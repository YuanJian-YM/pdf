cmake_minimum_required(VERSION 3.10.2) #版本要求

PROJECT(Pdf)#工程名

SET(CMAKE_CXX_FLAGS "-std=c++11")#添加c++ 11标准支持

include_directories(${PROJECT_SOURCE_DIR}/include)

include_directories(/usr/local/include/freetype2/)

AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/source DIR_SRCS)

SET(SOURCE_CPP ${DIR_SRCS})

ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCE_CPP})

find_package(Threads)

target_link_libraries(${PROJECT_NAME} 
    ${CMAKE_THREAD_LIBS_INIT}
    glog
    gflags_nothreads
    freetype
    jpeg
    z)
