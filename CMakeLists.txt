cmake_minimum_required (VERSION 2.6)
project (MicroMiddleware)

# uncomment this to use the clang-tidy linter; helpful but slow
#set(CMAKE_CXX_CLANG_TIDY "clang-tidy")

# add root folder to include paths
include_directories(PUBLIC .)

# build as debug
# set(CMAKE_BUILD_TYPE Debug)

# prefer pthread library if there are multiple thread libraries
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
# prefer -pthread compiler and linker flag (to avoid skipping the -pthread flag
# during compiling or linking)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
# use threads library
find_package(Threads REQUIRED)

# use C++ 17
set (CMAKE_CXX_STANDARD 17)

# add single executables
#add_executable(basic_service examples/basic_service.cpp)
#add_executable(class_service examples/class_service.cpp)
add_executable(basic_subscription examples/basic_subscription.cpp)
add_executable(class_subscription examples/class_subscription.cpp)
add_executable(transport examples/transport.cpp)
add_executable(transport_udp examples/transport_udp/main.cpp)

# add libraries
target_link_libraries(basic_subscription Threads::Threads)
target_link_libraries(class_subscription Threads::Threads)
target_link_libraries(transport Threads::Threads)
target_link_libraries(transport_udp Threads::Threads)