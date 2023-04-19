
#include <cstdio>
#include "micro_middleware.hpp"

struct MyTopicMessage {
    uint8_t value1;
    uint32_t value2;
};

Topic<MyTopicMessage> myTopicName(1, "myTopicName");

class MyClass {
public:
    MyClass(const char* name): name(name) {
        // subscribe to topic upon construction
        myTopicName.Subscribe(&MyClass::callbackFunction, this);
    }

private:
    const char* name; // just used for the print

    // this function is called when the subscriber receives a message
    void callbackFunction(const MyTopicMessage& msg) {
        printf("Subscriber with name %s received message with value1 = %d, value2 = %d\n",
            name, msg.value1, msg.value2);
        }
};

int main () {
    MyClass sub1("Subscriber 1");
    MyClass sub2("Subscriber 2");

    MyTopicMessage msg;
    msg.value1 = 5;
    msg.value2 = 1224;

    myTopicName.Publish(msg);
}