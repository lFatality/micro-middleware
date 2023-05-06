#include "micro_middleware.hpp"
#include <cassert>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>

// example serializer - deserializer that:
// 1) subscribes to a topic as usual
// 2) converts each published topic message to a simple protocol as such
//    [topic id (16 bits)] [message size (16 bits)] [message...]
// 3) publishes these serialized messages to the `transport` topic
// 4) the deserializer, which subscribes to the `tranport` topic, gets those
// messages, decodes them,
//    and publishes them to the corresponding topic (this is not done yet, needs
//    a ay to look up topics by ID)

// begin section transport api
Topic<std::string> transport(99);

template <typename M> class SimpleTopicTransport {
  typename Topic<M>::Subscription serialize_subscription_;
  Topic<M> &topic_;

  M decoded_message_;

public:
  SimpleTopicTransport(Topic<M> &topic, bool deserializer = true)
      : topic_(topic) {

    serialize_subscription_ =
        topic.Subscribe(&SimpleTopicTransport::Serialize, this);

    if (deserializer) {
      transport.Subscribe(&SimpleTopicTransport::Deserialize, this);
    }
  }

private:
  void Serialize(Topic<M> &topic, const M &msg) {
    // we wouldn't use stringstream/strings in an embedded system, just to
    // illustrate the point
    std::stringstream ss;

    ss.put((uint8_t)(topic.Id()) & 0xff);
    ss.put((uint8_t)(topic.Id()) >> 8);

    ss.put((uint8_t)(sizeof(M)) & 0xff);
    ss.put((uint8_t)(sizeof(M)) >> 8);

    const uint8_t *ptr = (const uint8_t *)(&msg);
    for (uint16_t i = 0; i < sizeof(M); i++) {
      ss.put((uint8_t)(ptr[i]));
    }

    std::string mystr = ss.str();

    std::cout << mystr << std::endl;

    printf("transmitting message with size %ld on topic id %d: %x\n", sizeof(M),
           topic.Id(), ptr[0]);

    transport.Publish(mystr);
  }

  void Deserialize(const std::string &msg) {
    const char *data = msg.c_str();
    uint16_t data_idx = 0;

    uint16_t received_topic_id =
        (uint16_t)(data[data_idx++]) | ((uint16_t)(data[data_idx++]) << 8);

    printf("deserializer instance for topic id %d ", topic_.Id());
    printf(" - decoding message on transport topic, id %d", received_topic_id);

    if (topic_.Id() != received_topic_id) {
      printf(" - ignoring. \n");
      return;
    }

    uint16_t message_length =
        (uint16_t)(data[data_idx++]) | ((uint16_t)(data[data_idx++]) << 8);
    printf(" - processing %d bytes.\n", message_length);
    assert(message_length == sizeof(M));

    uint8_t *ptr = (uint8_t *)(&decoded_message_);
    memcpy(ptr, data + data_idx, message_length);

    // publish to topic, skipping the serialization subscription
    topic_.Publish(decoded_message_, &serialize_subscription_, 1);
  }
};
// end section transport api

// declare two example topics with message types of different sizes

struct MsgTypeA {
  uint64_t foo;
};

struct MsgTypeB {
  uint8_t bar;
};

Topic<MsgTypeA> topicA(1);
Topic<MsgTypeB> topicB(2);

void a_subscriber(const MsgTypeA &message) {
  printf("a_subscriber: %ld\n", message.foo);
}

void b_subscriber(const MsgTypeB &message) {
  printf("b_subscriber: %d\n", message.bar);
}

// begin section user example
int main() {
  printf("hello\n");
  auto transportA = SimpleTopicTransport(topicA);
  auto transportB = SimpleTopicTransport(topicB);

  topicA.Subscribe(a_subscriber);
  topicB.Subscribe(b_subscriber);

  topicA.Publish({.foo = 13});
  topicB.Publish({.bar = 37});
}