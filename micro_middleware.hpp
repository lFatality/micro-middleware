/**
 * The MIT License (MIT)
 * Copyright © 2023 Fynn Boyer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the “Software”), to deal in the Software without restriction,
 * ncluding without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software
 * is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <cstdint>
#include <string.h>
#include <functional>

// *******************************
// ******** Configuration ********
// *******************************

// maximum number of topics
static constexpr uint16_t MAX_NUMBER_OF_TOPICS = 100;

// maximum number of subscriptions a single topic can have
static constexpr uint16_t MAX_NUMBER_OF_SUBS_PER_TOPIC = 20;

// maximum number of services
static constexpr uint16_t MAX_NUMBER_OF_SERVICES = 100;

// *******************************
// ************ Code *************
// *******************************

/**
 * This class provides a very simple middleware
 * to communicate between different parts of an application
 * via topics (publisher/subscriber) and services (client/server, request/response).
 */
class MicroMiddleware {

public:

    // TODO(FB): add unsubscribe / unregister functions (e.g. in case an object goes out of scope)

    /**
     * Subscribe to a topic
     *
     * @param topicName Name of the topic
     * @param callbackFunction Function to be called when a message is published on the topic
     * @return true on success, false otherwise
     */
    static bool subscribe(
        const char* topicName,
        std::function<void(const void*)> callbackFunction)
    {
        for (uint16_t i=0; i < MAX_NUMBER_OF_TOPICS; i++) {
            Subscription sub = {
                .callbackFunction = callbackFunction,
            };

            if (topicNames[i] == nullptr) {
                // add sub first, then add name (to avoid race conditions where
                // the topic name is "available" but the actual sub is not set up yet)
                bool ok = addSubToTopic(i, sub);
                topicNames[i] = topicName;
                return ok;
            }

            if (strcmp(topicNames[i], topicName) == 0) {
                // topic already exists, add yourself as a sub
                bool ok = addSubToTopic(i, sub);
                return ok;
            }
        }

        return false;
    }

    /**
     * Function to subscribe to a topic using a function of a class
     *
     * Example call:
     * MicroMiddleware::subscribe("TopicName", &MyAwesomeClass::myCallbackFunction, this);
     *
     * @tparam T The class of the function. Doesn't need to be provided, it will
     * be inferred via the parameters
     * @param topicName The name of the topic
     * @param callbackFunction Callback function of the subscriber
     * @param obj Class object on which to call the function.
     * WARNING: You MUST unsubscribe from the topic before you delete the object
     * (e.g. if it goes out of scope), otherwise undefined behavior will occur!
     * @return true on success, false otherwise
     */
    template<class T>
    static bool subscribe(const char* topicName, void(T::*callbackFunction)(const void*), T* obj) {
        std::function<void(const void*)> f = std::bind(callbackFunction, obj, std::placeholders::_1);
        return subscribe(topicName, f);
    }

    /**
     * Publish to a topic
     *
     * @param topicName Name of the topic
     * @param msg The message to publish. Make sure the type of object you publish
     * is the same as the object expected in the callback functions of the subscribers.
     * Otherwise this can lead to undefined behavior!
     * @return true on success, false otherwise
     */
    static bool publish(
        const char* topicName,
        void* msg)
    {
        for (uint16_t i=0; i < MAX_NUMBER_OF_TOPICS; i++) {

            if (topicNames[i] == nullptr) {
                // no more topics left
                break;
            }

            if (strcmp(topicNames[i], topicName) == 0) {
                // topic exists, publish to subs by calling their functions
                for (uint16_t j=0; j < MAX_NUMBER_OF_SUBS_PER_TOPIC; j++) {

                    if (subscriptions[i][j].callbackFunction == nullptr) {
                        // no more subs
                        break;
                    }

                    Subscription sub = subscriptions[i][j];
                    sub.callbackFunction(msg);
                }
            }
        }

        // topic not found
        return false;
    }

    /**
     * Call a service
     *
     * @param serviceName The name of the service
     * @param request The request to send. Make sure the type of object you send
     * is the same as the object expected in the callback functions of the service.
     * Otherwise this can lead to undefined behavior!
     * @param response Object in which the response will be stored. It will be filled
     * by the service. Make sure the type of object you send is the same as the object
     * expected in the callback functions of the service.
     * Otherwise this can lead to undefined behavior!
     * @return true on success, false otherwise
     */
    static bool callService(
        const char* serviceName,
        const void* request,
        void* response)
    {
        for (uint16_t i=0; i < MAX_NUMBER_OF_SERVICES; i++) {

            if (serviceNames[i] == nullptr) {
                break;
            }

            if (strcmp(serviceNames[i], serviceName) == 0) {
                // service exists, call function
                Service service = services[i];
                service.callbackFunction(request, response);

                return true;
            }
        }

        return false;
    }

    /**
     * Register a service for others to call.
     *
     * @param serviceName The name of the service.
     * Note: A service with the same name can only be registered once.
     * Attempting to register a service with the same name will fail.
     * @param callbackFunction The callback function which is executed
     * when someone calls the service.
     * @return true on success, false otherwise
     */
    static bool registerService(
        const char* serviceName,
        std::function<void(const void*, void*)> callbackFunction)
    {
        for (uint16_t i=0; i < MAX_NUMBER_OF_SERVICES; i++) {
            Service service = {
                .callbackFunction = callbackFunction,
            };

            if (serviceNames[i] == nullptr) {
                // add service first, then add name (to avoid race conditions where
                // the service name is "available" but the actual service is not set up yet)
                services[i] = service;
                serviceNames[i] = serviceName;
                return true;
            }

            if (strcmp(serviceNames[i], serviceName) == 0) {
                // service already exists.
                // a service with the same name can only be registered once.
                return false;
            }
        }

        return false;
    }

    /**
     * Function to register a service using a function of a class
     *
     * Example call:
     * MicroMiddleware::registerService("ServiceName", &MyAwesomeClass::myCallbackFunction, this);
     *
     * @tparam T The class of the function. Doesn't need to be provided, it will
     * be inferred via the parameters
     * @param serviceName The name of the service
     * @param callbackFunction Callback function of the service
     * @param obj Class object on which to call the function.
     * WARNING: You MUST unregister the service before you delete the object
     * (e.g. if it goes out of scope), otherwise undefined behavior will occur!
     * @return true on success, false otherwise
     */
    template<class T>
    static bool registerService(const char* serviceName, void(T::*callbackFunction)(const void*, void*), T* obj) {
        std::function<void(const void*, void*)> f = std::bind(callbackFunction, obj, std::placeholders::_1, std::placeholders::_2);
        return registerService(serviceName, f);
    }

private:

    /**
     * Storage for a single subscription
     */
    struct Subscription {
        // callback function of the subscription
        std::function<void (const void*)> callbackFunction;
    };

    /**
     * Storage for a single service
     */
    struct Service {
        // callback function of the service
        std::function<void (const void*, void*)> callbackFunction;
    };


    // TODO (FB): maybe this would be nicer with a map?
    /**
     * Array of all available topic names.
     * It's used to lookup topics and find out the index of them.
     * The index is then used in the subscriptions array to actually call the subscription.
     */
    static const char* topicNames[MAX_NUMBER_OF_TOPICS];

    /**
     * Array of all subscriptions on the different topics.
     * You can image it like this:
     * [
     *      "topicA": [SubscriptionA, SubscriptionB, nullptr, ...],
     *      "topicB": [SubscriptionC, SubscriptionD, SubscriptionE, nullptr, ...],
     *      ...
     * ]
     *
     * The topic names are stored in the topicNames array.
     */
    static Subscription subscriptions[MAX_NUMBER_OF_TOPICS][MAX_NUMBER_OF_SUBS_PER_TOPIC];

    /**
     * Array of all available service names.
     */
    static const char* serviceNames[MAX_NUMBER_OF_SERVICES];

    /**
     * Array of available services.
     * Services with the same name can only be registered once.
     * Therefore it's a simple one-dimensional array.
     */
    static Service services[MAX_NUMBER_OF_SERVICES];

    // private function
    static bool addSubToTopic(uint16_t topicIndex, Subscription sub) {
        for (uint16_t j=0; j < MAX_NUMBER_OF_SUBS_PER_TOPIC; j++) {
            // add sub to topic if space is still available
            if (subscriptions[topicIndex][j].callbackFunction == nullptr) {
                subscriptions[topicIndex][j] = sub;
                return true;
            }
        }

        return false;
    }
};

// Arrays of topics, subscriptions, services.
// These are defined inline so that the compiler doesn't complain about
// multiple definitions (since these variables are in the header file).
// This is only supported for C++17 or greater.
#if __cplusplus >= 201703L
inline const char* MicroMiddleware::topicNames[MAX_NUMBER_OF_TOPICS];
inline MicroMiddleware::Subscription MicroMiddleware::subscriptions[MAX_NUMBER_OF_TOPICS][MAX_NUMBER_OF_SUBS_PER_TOPIC];

inline const char* MicroMiddleware::serviceNames[MAX_NUMBER_OF_SERVICES];
inline MicroMiddleware::Service MicroMiddleware::services[MAX_NUMBER_OF_SERVICES];
#else
#error Library is header-only just for C++17 or greater. You can still compile it. Please see instructions at this error message.
// You can still compile the library, just not as header-only.
// Move the following lines into a .cpp file and compile it along your code.

// #include "micro_middleware.hpp"
// const char* MicroMiddleware::topicNames[MAX_NUMBER_OF_TOPICS];
// MicroMiddleware::Subscription MicroMiddleware::subscriptions[MAX_NUMBER_OF_TOPICS][MAX_NUMBER_OF_SUBS_PER_TOPIC];
// const char* MicroMiddleware::serviceNames[MAX_NUMBER_OF_SERVICES];
// MicroMiddleware::Service MicroMiddleware::services[MAX_NUMBER_OF_SERVICES];

#endif