cmake_minimum_required(VERSION 3.13)
project(LSM)

set(CMAKE_CXX_STANDARD 17)

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -Wall -Wno-unused-variable -pthread")
else( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    set(CMAKE_BUILD_TYPE "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O2 -pthread -fopenmp")
endif( CMAKE_BUILD_TYPE STREQUAL "Debug" )
message("Build Type:" ${CMAKE_BUILD_TYPE} ${CMAKE_CXX_FLAGS})

find_package(Boost REQUIRED COMPONENTS)

include_directories(
        include
        ${Boost_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME} SHARED
        include/kernel/Segment.h include/kernel/Common.h
        include/kernel/Manager.h src/kernel/Manager.cpp
        include/kernel/Exception.h src/kernel/Exception.cpp
        include/kernel/Time.h src/kernel/Time.cpp
        include/component/MemoryHub.h src/component/MemoryHub.cpp
        include/utils/NamedLock.h
        include/utils/String.h src/utils/String.cpp
        include/component/Channel.h src/component/Channel.cpp
        include/component/Connection.h src/component/Connection.cpp
        include/component/Message.h src/component/Message.cpp
        include/LSM
        include/client/LSM.h src/client/LSM.cpp src/kernel/Common.cpp)

add_executable(lsm_server
        include/server/Acceptor.h src/server/Acceptor.cpp
        include/server/Maintainer.h src/server/Maintainer.cpp
        src/server/Server.cpp)

add_executable(${PROJECT_NAME}_Demo
        src/client/Demo.cpp)

add_dependencies(lsm_server ${PROJECT_NAME})
add_dependencies(${PROJECT_NAME}_Demo ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
        -lpthread
        -lrt
        ${Boost_LIBRARIES}
)

target_link_libraries(lsm_server
        ${PROJECT_NAME}
        ${Boost_LIBRARIES})

target_link_libraries(${PROJECT_NAME}_Demo
        ${PROJECT_NAME}
        ${Boost_LIBRARIES})