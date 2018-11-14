cmake_minimum_required(VERSION 3.12)
project(balda)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost COMPONENTS system filesystem thread regex)
find_package(OpenSSL)

add_executable(balda main.cpp)

target_include_directories(balda PUBLIC ${Boost_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(balda ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} -lpthread)