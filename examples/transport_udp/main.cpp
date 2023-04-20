#include "micro_middleware.hpp"
#include <chrono>
#include <cstdio>
#include <random>
#include <thread>
#include <unistd.h>

#include "simple_topic_transport.hpp"
#include "topics.hpp"
#include "transport_api.hpp"
#include "udp_sendreceive.hpp"

bool should_exit = false;

// a thread that generates random numbers and random hex data and publishes it
// to the topics
void RandomGenerator() {
  std::mt19937_64 rng;
  std::uniform_int_distribution<uint64_t> num_dist;
  std::uniform_int_distribution<int> ascii_dist(32, 126);

  while (!should_exit) {
    RandomNumber number_message = {.sender_pid = (uint64_t)getpid(),
                                   .random_number = num_dist(rng)};

    topic_random_number.Publish(number_message);

    RandomASCIIData hex_data_message = {.sender_pid = (uint64_t)getpid()};
    for (int i = 0; i < 256; i++) {
      hex_data_message.data[i] = static_cast<char>(ascii_dist(rng));
    }
    topic_random_ascii_data.Publish(hex_data_message);

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void NumberPrinter(const RandomNumber &message) {
  printf("received random number from pid %u: %llu\n", message.sender_pid,
         message.random_number);
}

void DataPrinter(const RandomASCIIData &message) {
  printf("received random data from pid %u: ", message.sender_pid);
  for (int i = 0; i < 256; i++) {
    printf("%c", message.data[i]);
  }
  printf("\n");
}

int main() {
  printf("hello world\n");

  /*
  auto number_transport =
      SimpleTopicTransport(udp_send_receive, topic_random_number);
  auto ascii_data_transport =
      SimpleTopicTransport(udp_send_receive, topic_random_ascii_data);
  */

  UDPSendReceive udp_send_receive;
  TransportManager<10, 1024, SimpleTopicTransport> my_manager(udp_send_receive);
  my_manager.AddTopic(topic_random_number);
  my_manager.AddTopic(topic_random_ascii_data);

  topic_random_number.Subscribe(NumberPrinter);
  topic_random_ascii_data.Subscribe(DataPrinter);

  std::thread generator_thread(RandomGenerator);
  generator_thread.join();

  return 0;
}