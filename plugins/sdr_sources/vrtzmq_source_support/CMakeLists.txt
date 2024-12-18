cmake_minimum_required(VERSION 3.12)
project(vrtzmq_source_support)

message("Building with VRT ZMQ Source support")
find_package(ZMQ REQUIRED)
find_package(VRT REQUIRED)
include_directories(${ZMQ_INCLUDE_DIRS} ${VRT_INCLUDE_DIRS})
file(GLOB_RECURSE net_source_support_CPPS *.cpp)
add_library(vrtzmq_source_support SHARED ${net_source_support_CPPS})
target_link_libraries(vrtzmq_source_support PUBLIC satdump_core ${ZMQ_LIBRARIES} ${VRT_LIBRARIES})
target_include_directories(vrtzmq_source_support PUBLIC src)

if(MSVC)
    target_link_libraries(vrtzmq_source_support PUBLIC ws2_32)
endif()

install(TARGETS vrtzmq_source_support DESTINATION ${CMAKE_INSTALL_LIBDIR}/satdump/plugins)
