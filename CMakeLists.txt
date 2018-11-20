cmake_minimum_required(VERSION 3.7)
project(balda)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "-DBOOST_ALL_DYN_LINK")

find_package(Boost COMPONENTS system filesystem log thread)
find_package(OpenSSL REQUIRED)

message(${Boost_LIBRARIES})

add_executable(balda main.cpp src/logger.cpp)
add_executable(daemon src/daemon.cpp src/logger.cpp)

target_include_directories(balda PUBLIC ${Boost_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${CMAKE_SOURCE_DIR})
target_link_libraries(balda ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} -lpthread)

target_include_directories(daemon PUBLIC ${Boost_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${CMAKE_SOURCE_DIR})
target_link_libraries(daemon ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} -lpthread)