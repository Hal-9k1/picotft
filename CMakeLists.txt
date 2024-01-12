# mostly for target_link_options, but pico-sdk requires 3.13 anyway so:
cmake_minimum_required(VERSION 3.13)

if (NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  message("Building picotft as top level project.")
  include(pico_sdk_import.cmake)

  project(picotft LANGUAGES C CXX ASM)
  set(CMAKE_C_STANDARD 11)
  set(CMAKE_C_STANDARD_REQUIRED YES)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED YES)
  pico_sdk_init()
endif()

# pico_sdk_init includes pico_sdk_version
if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# not sure if this was specific to pico-examples:
#if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
#  add_compile_options(-Wno-maybe-uninitialized)
#endif()

add_library(picotft
  Display.cpp DisplayIO.cpp)

target_include_directories(picotft
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include-private
  )

target_compile_features(picotft PUBLIC
  cxx_std_17)

set(PICOTFT_UNIVERSAL_WARNINGS  # warn when...
  -Wsuggest-override              # overriding method isn't marked as such
  -Wdisabled-optimization         # an optimization is disabled because code is too complex
  -Wunsafe-loop-optimizations     # loop optimization is disabled because bounds can't be detected
  -Wcast-qual                     # const/volatile qualifiers are casted away
  -Wcast-align=strict             # a pointer is cast to type with greater alignment requirement
  -Waggressive-loop-optimizations # loop optimization is disabled because of undefined behavior
  -Woverloaded-virtual            # a virtual method is HIDDEN by one with same name, diff args
  -Wold-style-cast                # a c-style cast is used
  -Wuseless-cast                  # a redundant cast is used
  -Wpedantic                      # ansi standards are violated
  -Wall                           # any of a list of warnings are triggered
  -Wextra                         # any of a longer list of warnings are triggered
  -Werror                       # error instead of warn
  -pedantic-errors)             # error instead of warn for ansi violations

target_compile_options(picotft PRIVATE
  ${PICOTFT_UNIVERSAL_WARNINGS})

target_link_options(picotft PRIVATE
  ${PICOTFT_UNIVERSAL_WARNINGS})

target_link_libraries(picotft PUBLIC
  pico_stdlib
  pico_cyw43_arch_none # gpio is tied to wifi chip on the pico w
  )
