
#include <cstdio>
#include "micro_middleware.hpp"

struct MyTopicMessage {
    uint8_t value1;
    uint32_t value2;
};

Topic<MyTopicMessage> myTopicName(1, "myTopicName");

// this function is called when the subscriber receives a message
void callbackFunction(const MyTopicMessage& message) {
    printf("Received message with value1 = %d, value2 = %d\n",
        message.value1, message.value2);
}

int main () {
    myTopicName.Subscribe(callbackFunction);

    MyTopicMessage msg;
    msg.value1 = 5;
    msg.value2 = 1337;

    myTopicName.Publish(msg);
}