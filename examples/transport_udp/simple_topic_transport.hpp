#include "transport_api.hpp"

#pragma once

template <size_t MAX_SERIALIZED_MESSAGE_SIZE, typename M>
class SimpleTopicTransport
    : public TopicTransport<MAX_SERIALIZED_MESSAGE_SIZE, M> {

  static_assert(sizeof(M) <= MAX_SERIALIZED_MESSAGE_SIZE);

  using Base = TopicTransport<MAX_SERIALIZED_MESSAGE_SIZE, M>;
  using BufferType = typename Base::BufferType;

public:
  SimpleTopicTransport(BufferType &buffer, SendReceive &transport,
                       Topic<M> &topic, bool deserializer = true)
      : Base(buffer, transport, topic, deserializer) {}

private:
  virtual void Serialize(const M &msg) {
    // we wouldn't use stringstream/strings in an embedded system, just to
    // illustrate the point
    std::stringstream ss;

    ss.put((uint8_t)(this->topic_.Id()) & 0xff);
    ss.put((uint8_t)(this->topic_.Id()) >> 8);

    ss.put((uint8_t)(sizeof(M)) & 0xff);
    ss.put((uint8_t)(sizeof(M)) >> 8);

    const uint8_t *ptr = (const uint8_t *)(&msg);
    for (uint16_t i = 0; i < sizeof(M); i++) {
      ss.put((uint8_t)(ptr[i]));
    }

    std::string mystr = ss.str();

    printf("transmitting message with size %d on topic id %d: %x\n", sizeof(M),
           this->topic_.Id(), ptr[0]);

    SizedBuffer buf{reinterpret_cast<const uint8_t *>(mystr.c_str()),
                    mystr.length()};
    this->send_receive_.Send(buf);
  }

  virtual void Deserialize(const SizedBuffer &buffer) {}
};
