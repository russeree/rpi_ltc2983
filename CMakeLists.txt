#Minimum CMake version required
cmake_minimum_required (VERSION 2.6)
#Project Name
project (LTC2983_TCP_SERVER)
# Locate libraries and headers
find_package(WiringPi REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost 1.40 COMPONENTS system thread REQUIRED)
#Include header files
set(HEADER_FILES $("include/*.hpp"))
include_directories(${Boost_INCLUDE_DIR})
include_directories(include)
include_directories(include ${WIRINGPI_INCLUDE_DIRS}) 
#Get sources
file(GLOB SOURCES "src/*.cc")
add_executable(LTC2983_SERV ${SOURCES})
add_definitions(-std=gnu++11)
# Link against libraries
target_link_libraries(LTC2983_SERV ${WIRINGPI_LIBRARIES}) 
target_link_libraries(LTC2983_SERV ${CMAKE_THREAD_LIBS_INIT}) 
target_link_libraries(LTC2983_SERV ${Boost_LIBRARIES}) 
