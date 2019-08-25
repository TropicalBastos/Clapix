#ifndef LIFX_H
#define LIFX_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#if !defined(ESP8266) && !defined(__AVR__)

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strings.h>
#include <fcntl.h>

#endif

#define SOURCE_ID 123

#define GET_SERVICE 2
#define TOGGLE_POWER 0
#define SET_POWER 21

#define BROADCAST_IP "255.255.255.255"
#define BROADCAST_PORT 56700

#define BUFSIZE 1024

#pragma pack(push, 1)
typedef struct 
{
  /* frame */
  uint16_t size;
  uint16_t protocol:12;
  uint8_t  addressable:1;
  uint8_t  tagged:1;
  uint8_t  origin:2;
  uint32_t source;
  /* frame address */
  uint8_t  target[8];
  uint8_t  reserved[6];
  uint8_t  res_required:1;
  uint8_t  ack_required:1;
  uint8_t  :6;
  uint8_t  sequence;
  /* protocol header */
  uint64_t :64;
  uint16_t type;
  uint16_t :16;
  /* variable length payload follows */
} lx_protocol_header_t;
#pragma pack(pop)

class LifxProtocol
{
public:
  LifxProtocol(){}
  ~LifxProtocol();
  void BuildLifxHeader(uint16_t type);
  int BuildPacket(void* payload, int payloadSize);
  void BroadcastMessage(void* payload, int payloadSize);
  inline lx_protocol_header_t GetHeader() const { return mHeader; }
  inline void SetHeader(lx_protocol_header_t header) { mHeader = header; }

private:
  lx_protocol_header_t mHeader;
  char* mPacket;
};

#endif // LIFX_H