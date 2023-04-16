
#include <cstdio>
#include "micro_middleware.hpp"

struct MyServiceRequest {
    uint8_t value1;
    uint8_t value2;
};

struct MyServiceResponse {
    uint16_t value3;
};

// this function is called when the service receives a request
void callbackFunction(const void* request, void* response) {

    // cast the request & response to the published type of the service
    const MyServiceRequest* req = reinterpret_cast<const MyServiceRequest*>(request);
    MyServiceResponse* res = reinterpret_cast<MyServiceResponse*>(response);

    // use the request to create the response
    res->value3 = req->value1 + req->value2;

    printf("Received request with value1 = %d, value2 = %d\n", req->value1, req->value2);
}

int main () {

    MicroMiddleware::registerService("MyServiceName", callbackFunction);

    MyServiceRequest req;
    req.value1 = 5;
    req.value2 = 10;

    MyServiceResponse res;

    MicroMiddleware::callService("MyServiceName", &req, &res);

    printf("Received response with value3 = %d\n", res.value3);
}