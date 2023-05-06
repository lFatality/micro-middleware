#include "micro_middleware.hpp"
#include <cassert>
#include <cstddef>
#include <mutex>
#include <sstream>

#pragma once

struct SizedBuffer {
  const uint8_t *ptr;
  std::size_t length;
};

struct RawTopicMessage {
  uint16_t topic_id;
  SizedBuffer data;
};

class SendReceive {
  Topic<RawTopicMessage> receive_topic_;

public:
  SendReceive() : receive_topic_(0) {} /* topic ID is irrelevant */

  Topic<RawTopicMessage> &ReceiveTopic() { return receive_topic_; }

  virtual void Send(const RawTopicMessage &message) = 0;
  virtual void ReceiveLoop() = 0;
};

template <size_t SIZE> class MutexProtectedBuffer {
  uint8_t buffer_[SIZE];
  std::mutex mutex_;

public:
  class Lock {
  public:
    Lock(MutexProtectedBuffer &buffer) : buffer(buffer), lock(buffer.mutex_) {}
    uint8_t *Get() { return buffer.buffer_; }

  private:
    MutexProtectedBuffer &buffer;
    std::lock_guard<std::mutex> lock;
  };

  friend class Lock;
};

// ---

template <size_t MAX_SERIALIZED_MESSAGE_SIZE, typename M>
class TopicTransport final {
protected:
  using BufferType = MutexProtectedBuffer<MAX_SERIALIZED_MESSAGE_SIZE>;

  typename Topic<M>::Subscription serialize_subscription_;
  Topic<M> &topic_;
  SendReceive &send_receive_;
  BufferType &buffer_;

public:
  TopicTransport(BufferType &buffer, SendReceive &transport, Topic<M> &topic,
                 bool deserializer = true)
      : buffer_(buffer), send_receive_(transport), topic_(topic) {

    serialize_subscription_ = topic.Subscribe(&TopicTransport::Serialize, this);

    if (deserializer) {
      send_receive_.ReceiveTopic().Subscribe(&TopicTransport::Deserialize,
                                             this);
    }
  }

  void Serialize(const M &msg) {
    SizedBuffer buf = {.ptr = reinterpret_cast<const uint8_t *>(&msg),
                       .length = sizeof(M)};
    send_receive_.Send({.topic_id = topic_.Id(), .data = buf});
  };

  void Deserialize(const RawTopicMessage &message) {
    if (message.topic_id != topic_.Id()) {
      return;
    }

    typename BufferType::Lock lock(buffer_);
    uint8_t *buf = lock.Get();
    memcpy(buf, message.data.ptr, message.data.length);
    M &deserialized_message = reinterpret_cast<M &>(buf);

    // publish to the topic, skipping the `Serialize` subscription
    topic_.Publish(deserialized_message, &serialize_subscription_, 1);

    // deserialized_message is invalid after this function ends
    // (although it may not yet have been overwritten in the `buffer_`)
    // how to deal with this?
  };
};

template <size_t MAX_TRANSPORTED_TOPICS, size_t MAX_SERIALIZED_MESSAGE_SIZE>
class TransportManager {
  /* The size of the specific transport type we are using should not change
   * based on the message type, so use an arbitrary value type (uint8_t)  */
  static constexpr size_t TRANSPORT_OBJECT_SIZE =
      sizeof(TopicTransport<MAX_SERIALIZED_MESSAGE_SIZE, uint8_t>);

  std::array<
      std::aligned_storage_t<TRANSPORT_OBJECT_SIZE, alignof(std::max_align_t)>,
      MAX_TRANSPORTED_TOPICS>
      transport_array_;
  size_t next_transport_idx_ = 0;

  MutexProtectedBuffer<MAX_SERIALIZED_MESSAGE_SIZE> buffer_;
  SendReceive &send_receive_;

public:
  TransportManager(SendReceive &send_receive) : send_receive_(send_receive) {}

  template <typename M> void AddTopic(Topic<M> &topic) {
    using ConcreteTransportType =
        TopicTransport<MAX_SERIALIZED_MESSAGE_SIZE, M>;
    static_assert(sizeof(ConcreteTransportType) <= TRANSPORT_OBJECT_SIZE);

    // allocate the transport object in transport_array_.
    // this causes it to subscribe to `topic` and the receive topic of
    // `send_receive`.
    // the resulting transport object lives for the lifetime of the
    // `TransportManager` object.
    assert(next_transport_idx_ < MAX_TRANSPORTED_TOPICS);
    auto *transport = new (&transport_array_[next_transport_idx_])
        ConcreteTransportType(buffer_, send_receive_, topic);

    next_transport_idx_ += 1;
  }
};