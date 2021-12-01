cmake_minimum_required(VERSION 3.13)
 project(charls-tools
         VERSION 0.0.0
         DESCRIPTION "CharLS Tools"
         HOMEPAGE_URL "https://github.com/malaterre/charls-tools"
         LANGUAGES CXX)

configure_file(version.h.in version.h @ONLY)

find_package(charls REQUIRED)
foreach(exe cjpls djpls)
  add_executable(${exe} ${exe}.cxx)
  # https://stackoverflow.com/questions/5096881/does-set-target-properties-in-cmake-override-cmake-cxx-flags
  target_include_directories(${exe} PRIVATE ${CHARLS_INCLUDE_DIRS})
  target_include_directories(${exe} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
endforeach()
