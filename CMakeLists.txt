cmake_minimum_required(VERSION 3.7)
project(balda)

set(CMAKE_CXX_STANDARD 14)

SET(Boost_USE_STATIC_LIBS ON)

find_package(Boost COMPONENTS system filesystem thread regex log)
find_package(OpenSSL)

add_executable(balda main.cpp src/logger.cpp)
add_executable(daemon src/daemon.cpp src/logger.cpp)

target_include_directories(balda PUBLIC ${Boost_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${CMAKE_SOURCE_DIR})
target_link_libraries(balda ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} -lpthread)

target_include_directories(daemon PUBLIC ${Boost_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${CMAKE_SOURCE_DIR})
target_link_libraries(daemon ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} -lpthread -lboost_system -lboost_filesystem)