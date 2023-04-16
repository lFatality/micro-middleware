
#include <cstdio>
#include "micro_middleware.hpp"

struct MyServiceRequest {
    uint8_t value1;
    uint8_t value2;
};

struct MyServiceResponse {
    uint16_t value3;
};

class MyClass {
public:
    MyClass(const char* name): name(name) {
        // subscribe to topic upon construction
        MicroMiddleware::registerService("MyServiceName", &MyClass::callbackFunction, this);
    }

private:
    const char* name; // just used for the print

    // this function is called when the subscriber receives a message
    void callbackFunction(const void* request, void* response) {

        // cast the request & response to the published type of the service
        const MyServiceRequest* req = reinterpret_cast<const MyServiceRequest*>(request);
        MyServiceResponse* res = reinterpret_cast<MyServiceResponse*>(response);

        // use the request to create the response
        res->value3 = req->value1 + req->value2;

        printf("Server with name %s received request with value1 = %d, value2 = %d\n",
            name, req->value1, req->value2);
    }
};

int main () {

    MyClass service("MyServer");

    MyServiceRequest req;
    req.value1 = 5;
    req.value2 = 10;

    MyServiceResponse res;

    MicroMiddleware::callService("MyServiceName", &req, &res);

    printf("Received response with value3 = %d\n", res.value3);
}