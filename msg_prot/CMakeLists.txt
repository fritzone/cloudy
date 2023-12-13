project(test)
set(CMAKE_CXX_STANDARD 98)
include_directories(${CMAKE_SOURCE_DIR})
add_subdirectory(tests)
add_library(idl 
ezxml.c
    message.cpp
    connectr.cpp
    connecti.cpp
    authenti.cpp
    authen1.cpp
    statusre.cpp
    status.cpp
    director.cpp
    direct1.cpp
    direc2.cpp
    protocol.cpp

)