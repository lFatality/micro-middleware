#include "micro_middleware.hpp"

struct RandomNumber {
  uint64_t sender_pid;
  uint64_t random_number;
};

struct RandomASCIIData {
  uint64_t sender_pid;
  char data[256];
};

inline Topic<RandomNumber> topic_random_number(1);
inline Topic<RandomASCIIData> topic_random_ascii_data(2);