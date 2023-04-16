
#include <cstdio>
#include "micro_middleware.hpp"

struct MyTopicMessage {
    uint8_t value1;
    uint32_t value2;
};

// this function is called when the subscriber receives a message
void callbackFunction(const void* msg) {
    // cast the message to the published type of the topic
    const MyTopicMessage* message = reinterpret_cast<const MyTopicMessage*>(msg);

    printf("Received message with value1 = %d, value2 = %d\n",
        message->value1, message->value2);
}

int main () {

    MicroMiddleware::subscribe("MyTopicName", callbackFunction);

    MyTopicMessage msg;
    msg.value1 = 5;
    msg.value2 = 1224;

    MicroMiddleware::publish("MyTopicName", &msg);
}