#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "transport_api.hpp"

constexpr int MAX_SERIALIZED_MESSAGE_SIZE = 1024;
class UDPSendReceive : public SendReceive {
  uint8_t data_[MAX_SERIALIZED_MESSAGE_SIZE] = {0};
  struct sockaddr_in addr_;
  int sock_;

public:
  UDPSendReceive(const char *address, int port) {
    if ((sock_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      throw std::runtime_error("Failed to create socket");
    }
    // Setup address
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    if (inet_pton(AF_INET, address, &addr_.sin_addr) <= 0) {
      throw std::runtime_error("Failed to convert IP address");
    }
  }

  virtual void Send(const RawTopicMessage &message) {
    std::stringstream ss;

    ss.put((uint8_t)(message.topic_id) & 0xff);
    ss.put((uint8_t)(message.topic_id) >> 8);

    ss.put((uint8_t)(message.data.length) & 0xff);
    ss.put((uint8_t)(message.data.length) >> 8);

    for (uint16_t i = 0; i < message.data.length; i++) {
      ss.put((uint8_t)(message.data.ptr[i]));
    }

    std::string mystr = ss.str();

    sendto(sock_, mystr.c_str(), mystr.length(), 0, (struct sockaddr *)&addr_,
           sizeof(addr_));
  }
  virtual void ReceiveLoop() {}
};
