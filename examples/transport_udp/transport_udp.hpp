#include "micro_middleware.hpp"
#include <sstream>

struct SizedBuffer {
  const uint8_t *ptr;
  std::size_t length;
};

class SendReceive {
  Topic<SizedBuffer> receive_topic_;

public:
  SendReceive() : receive_topic_(0) {} /* topic ID is irrelevant */

  Topic<SizedBuffer> &ReceiveTopic() { return receive_topic_; }

  virtual void Send(const SizedBuffer &buf) = 0;
  virtual void ReceiveLoop() = 0;
};

// ---

constexpr int MAX_SERIALIZED_MESSAGE_SIZE = 1024;
class UDPSendReceive : public SendReceive {
  uint8_t data[MAX_SERIALIZED_MESSAGE_SIZE] = {0};

public:
  virtual void Send(const SizedBuffer &buf) {}
  virtual void ReceiveLoop() {}
};

// ---

template <typename M> class TopicTransport {
protected:
  typename Topic<M>::Subscription serialize_subscription_;
  Topic<M> &topic_;
  SendReceive &send_receive_;

  M decoded_message_;

public:
  TopicTransport(SendReceive &transport, Topic<M> &topic,
                 bool deserializer = true)
      : send_receive_(transport), topic_(topic) {

    serialize_subscription_ = topic.Subscribe(&TopicTransport::Serialize, this);

    if (deserializer) {
      send_receive_.ReceiveTopic().Subscribe(&TopicTransport::Deserialize,
                                             this);
    }
  }

  virtual void Serialize(const M &msg) = 0;
  virtual void Deserialize(const SizedBuffer &buffer) = 0;
};

template <typename M> class SimpleTopicTransport : public TopicTransport<M> {
public:
  SimpleTopicTransport(SendReceive &transport, Topic<M> &topic,
                       bool deserializer = true)
      : TopicTransport<M>(transport, topic, deserializer) {}

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
