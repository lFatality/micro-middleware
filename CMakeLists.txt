cmake_minimum_required (VERSION 2.6)
project (MicroMiddleware)

# add root folder to include paths
include_directories(PUBLIC .)

# build as debug
set(CMAKE_BUILD_TYPE Debug)

# use C++ 17
set (CMAKE_CXX_STANDARD 17)

# add single executables
add_executable(basic_subscription examples/basic_subscription.cpp)
add_executable(class_subscription examples/class_subscription.cpp)
add_executable(basic_service examples/basic_service.cpp)
add_executable(class_service examples/class_service.cpp)