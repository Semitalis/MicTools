cmake_minimum_required(VERSION 3.11)

project(MicTools VERSION 1.0)

if(NOT WIN32)
  message(FATAL_ERROR "Unsupported platform!")
endif()

if(WIN32)
  # at least Win 8
  set(WINVER 0x0602)
  add_definitions(-D_WIN32_WINNT=${WINVER} -DWINVER=${WINVER})
  # use MFC
  add_definitions(-D_AFXDLL)
  set(CMAKE_MFC_FLAG 2)
endif()

if(MSVC)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
  add_definitions(/MP -D_CRT_SECURE_NO_WARNINGS)
endif()

set(EXTERN
  extern/WinToast/wintoastlib.cpp
  extern/WinToast/wintoastlib.h
)
source_group(extern FILES ${EXTERN})

set(RESOURCES
  resources/icon.xcf
  resources/icon_off.ico
  resources/icon_on.ico
  resources/resources.rc
  impl/resources.hpp
)
source_group(resources FILES ${RESOURCES})

set(SOURCES
  impl/app.cpp
  impl/app.hpp
  impl/main.cpp
  impl/mic_controls.cpp
  impl/mic_controls.hpp
  impl/notifier.cpp
  impl/notifier.hpp
  impl/toasts.cpp
  impl/toasts.hpp
  impl/tray.cpp
  impl/tray.hpp
  impl/types.hpp
)
source_group(impl FILES ${SOURCES})

add_executable(MicTools
  ${EXTERN}
  ${RESOURCES}
  ${SOURCES}
)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MicTools)

target_include_directories(MicTools
  PRIVATE
    extern/WinToast
    impl
)

target_link_libraries(MicTools
  Winmm.lib
  Shell32.lib
  runtimeobject.lib
)
