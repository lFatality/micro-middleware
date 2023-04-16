# Micro-Middleware

Micro-Middleware is a basic, header-only middleware.
It provides two ways of communication:
- topics: Publish messages to (multiple) subscribers of a topic
- services: Send a request to a server and get a response back

Objectives:
- provide topics & services
- use callback functions for the topic / service
- be able to provide class members as callback functions
- header-only
- minimal dependencies, easy to use and setup
- usable on embedded devices

## How to build the examples

In the root folder of the project, execute the following steps:

```
mkdir build
cmake ..
make
```

You can then find and execute the compiled examples in the `build` folder.

## How to use

You can check out the `examples` folder to see how to use it.
Next to that, here are a few instructions.

### Subscribe

Decide on a type that will be published on the topic. For example:

```
struct MyTopicMessage {
    uint8_t value1;
    uint32_t value2;
};
```

Create a callback function which will be called when the subscriber
receives a message. Notice that the callbackFunction gets a `const void*` and returns `void`.

```
void callbackFunction(const void* msg) {
    // cast the message to the published type of the topic
    const MyTopicMessage* message = reinterpret_cast<const MyTopicMessage*>(msg);

    printf("Received message with value1 = %d, value2 = %d\n",
        message->value1, message->value2);
}
```

Subscribe to the topic:
```
MicroMiddleware::subscribe("MyTopicName", callbackFunction);
```

If your callback function is a class member instead, use this syntax:
```
// if you subscribe from within the class
MicroMiddleware::subscribe("MyTopicName", &MyClass::callbackFunction, this);

// if you subscribe from outside the class
MicroMiddleware::subscribe("MyTopicName", &MyClass::callbackFunction, &myClassObject);
```

### Publish

Create the message, fill in the values, and publish it:

```
MyTopicMessage msg;
msg.value1 = 5;
msg.value2 = 1224;

MicroMiddleware::publish("MyTopicName", &msg);
```

A note on multi-threading:
Publishing to a topic is basically just a function call. The publishing thread
simply calls the callback function of the subscriber. This means that the
callback function will be executed by the publishing thread! Keep that in mind.
E.g. for long running callback functions, you might want to put the message into
a queue and resume a different thread so that the publishing thread doesn't hang
and lets other subscribers wait too long for the message.

### Register service

Decide on the types that will be used for the request and response of the service. For example:
```
struct MyServiceRequest {
    uint8_t value1;
    uint8_t value2;
};

struct MyServiceResponse {
    uint16_t value3;
};
```

The request is sent to the service and it will answer with the response.

Define the callback function that is called whenever a request is made to the service.
Note that it gets a `const void* request` and `void* response` as parameters and returns `void`.

```
void callbackFunction(const void* request, void* response) {
    // cast the request & response to the published type of the service
    const MyServiceRequest* req = reinterpret_cast<const MyServiceRequest*>(request);
    MyServiceResponse* res = reinterpret_cast<MyServiceResponse*>(response);

    // use the request to create the response
    res->value3 = req->value1 + req->value2;
}
```

Then you can register the service like this:
```
MicroMiddleware::registerService("MyServiceName", callbackFunction);
```

If your callback function is a class member instead, use this syntax:
```
// if you want to register the service from inside the class
MicroMiddleware::registerService("MyServiceName", &MyClass::callbackFunction, this);

// if you want to register the service from outside the class
MicroMiddleware::registerService("MyServiceName", &MyClass::callbackFunction, &myClassObject);
```

Note: A service with the same name can only be registered once.
Trying to register the same service again is not possible.

### Call service

Create the request and response, fill the request with data, then call the service:

```
MyServiceRequest req;
req.value1 = 5;
req.value2 = 10;

MyServiceResponse res;

MicroMiddleware::callService("MyServiceName", &req, &res);
```

After calling the service, you can immediately access the response. E.g. like this:

```
printf("Received response with value3 = %d\n", res.value3);
```

For multi-threading, the same note as for publishing applies (see section `Publish`).
In summary: The callback function is executed by the thread calling the service!