#include "lifx.h"

LifxProtocol::~LifxProtocol()
{
    free(mPacket);
}

void LifxProtocol::BuildLifxHeader(uint16_t type)
{
    memset(&mHeader, 0, sizeof(lx_protocol_header_t));

    /* frame */
    mHeader.protocol = 1024;
    mHeader.addressable = 1;
    mHeader.tagged = 1;
    mHeader.source = SOURCE_ID;

    /* frame address */
    mHeader.res_required = 1;
    mHeader.ack_required = 0;
    mHeader.sequence = 100;

    /* protocol header */
    mHeader.type = type;
}

// warning will need to be freed
// payload must be little endian
int LifxProtocol::BuildPacket(void* _payload, int payloadSize)
{
    char* packet = (char*) malloc(BUFSIZE);
    memset(packet, 0, BUFSIZE);

    int offset = sizeof(uint16_t);

    uint16_t next_bytes = (((mHeader.tagged << 1) | mHeader.addressable) << 12) | mHeader.protocol;
    memcpy(packet + offset, &next_bytes, sizeof(uint16_t));

    offset += sizeof(uint16_t); // origin / tagged /protocol

    memcpy(packet + offset, &mHeader.source, sizeof(uint32_t));

    offset += sizeof(uint32_t); // source 0's
    offset += sizeof(uint64_t); // target 0's
    offset += sizeof(uint32_t) + sizeof(uint16_t); // reserved

    uint8_t ack_res = (mHeader.ack_required << 1) | mHeader.res_required;
    memcpy(packet + offset, &ack_res, sizeof(uint8_t));

    offset += sizeof(uint8_t); //ack and res as one byte

    memcpy(packet + offset, &mHeader.sequence, sizeof(uint8_t));

    offset += sizeof(uint8_t); //sequence
    offset += sizeof(uint64_t); //reserved

    memcpy(packet + offset, &mHeader.type, sizeof(uint16_t));

    offset += sizeof(uint16_t); // type
    offset += sizeof(uint16_t); //reserved
    offset += sizeof(uint8_t); //reserved

    // payload
    memcpy(packet + offset, _payload, payloadSize);

    // now that we know the size we can set the size
    uint16_t size = offset + payloadSize;
    memcpy(packet, &size, sizeof(uint16_t));

    // and finally resize the packet
    mPacket = (char*) realloc(packet, size);

    printf("BEGIN PACKET\n");
    for (int i = 0; i < size; i++)
    {
        printf("%02X ", *(packet + i));
    }
    printf("\nEND PACKET\n");

    return size;
}

void LifxProtocol::BroadcastMessage(void* payload, int payloadSize)
{
#if !defined(ESP8266) && !defined(__AVR__)
    int size = BuildPacket(payload, payloadSize);

    struct sockaddr_in broadcast_addr;
    struct sockaddr_in listen_addr;
    struct sockaddr_in recv_addr;
    uint32_t socklen = sizeof(struct sockaddr_in); 

    memset(&listen_addr, 0, socklen);
    memset(&broadcast_addr, 0, socklen);
    memset(&recv_addr, 0, socklen);

    int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (fd == -1)
    {
        perror("Couldn't create socket\n");
        return;
    }

    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    broadcast_addr.sin_port = htons(BROADCAST_PORT);

    listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    listen_addr.sin_port = htons(0);
    listen_addr.sin_family = AF_INET;

    int status = bind(fd, (struct sockaddr*) &listen_addr, socklen);

    if (status == -1)
    {
        perror("Couldn't bind socket to port\n");
        return;
    }

    int broadcastEnable = 1;
    int ret = setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

    if (sendto(fd, mPacket, sizeof(lx_protocol_header_t), 0,
               (struct sockaddr*) &broadcast_addr, sizeof(broadcast_addr)) < 0)
    {
        perror("cannot broadcast message\n");
        close(fd);
        return;
    }

    lx_protocol_header_t lx;
    recvfrom(fd, (char*) &lx, sizeof(lx_protocol_header_t), 0,(struct sockaddr*) &recv_addr, &socklen);

    printf("RESPONSE FROM: %d\n", recv_addr.sin_addr.s_addr);

    close(fd);
#endif
}