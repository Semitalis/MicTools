cmake_minimum_required(VERSION 3.11)

project(MicTools)

if(MSVC)
  add_definitions(/MP)
endif()

set(SOURCES
  src/main.cpp
)
source_group(src FILES ${SOURCES})

add_executable(MicTools ${SOURCES})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MicTools)
