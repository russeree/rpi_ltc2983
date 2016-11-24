#Minimum CMake version required
cmake_minimum_required (VERSION 2.6)
#Project Name
project (LTC2983_TCP_SERVER)
# Locate libraries and headers
find_package(WiringPi REQUIRED)
find_package(Threads REQUIRED) 
#Include header files
set(HEADER_FILES $("include/*.h"))
include_directories(include)
include_directories(include ${WIRINGPI_INCLUDE_DIRS}) 
#Get sources
file(GLOB SOURCES "src/*.cc")
add_executable(LTC2983_SERV ${SOURCES})
add_definitions(-L/usr/local/lib/ -lwiringPi -lpthread -std=gnu++11)
# Link against libraries
target_link_libraries(LTC2983_SERV ${WIRINGPI_LIBRARIES}) 
target_link_libraries(LTC2983_SERV ${CMAKE_THREAD_LIBS_INIT}) 
