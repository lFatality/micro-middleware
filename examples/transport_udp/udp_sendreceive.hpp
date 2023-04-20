#include "transport_api.hpp"

constexpr int MAX_SERIALIZED_MESSAGE_SIZE = 1021;
class UDPSendReceive : public SendReceive {
  uint8_t data[MAX_SERIALIZED_MESSAGE_SIZE] = {0};

public:
  virtual void Send(const SizedBuffer &buf) {}
  virtual void ReceiveLoop() {}
};
