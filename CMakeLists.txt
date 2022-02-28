cmake_minimum_required(VERSION 3.13)
project(
  JLST # charls-tools is too long to type
  VERSION 0.1
  DESCRIPTION "CharLS Tools"
  HOMEPAGE_URL "https://github.com/malaterre/charls-tools"
  LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
file(TIMESTAMP ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt JLST_DATE "%Y-%m-%d"
     UTC)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# require c++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)
set(JLST_INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR})
configure_file(version.h.in version.h @ONLY)

option(JLST_USE_CLANG_TIDY "Use Clang tidy" OFF)
option(JLST_USE_IWYU "Use iwyu" OFF)
option(JLST_USE_LWYU "Use lwyu" OFF)

set(GNU_CXX_COMPILE_FLAGS
    -Wall #
    -Wextra #
    -pedantic #
    -Wold-style-cast #
    -Wfloat-equal #
    -Wlogical-op #
    -Wundef #
    -Wredundant-decls #
    -Wshadow #
    -Wwrite-strings #
    -Wpointer-arith #
    -Wcast-qual #
    -Wformat=2 #
    -Wmissing-include-dirs #
    -Wcast-align #
    -Wctor-dtor-privacy #
    -Wdisabled-optimization #
    -Winvalid-pch #
    -Woverloaded-virtual #
    -Wnon-virtual-dtor #
    -Wnoexcept #
    -Wdouble-promotion #
    -Wtrampolines #
    -Wzero-as-null-pointer-constant #
    -Wuseless-cast #
    -Wvector-operation-performance #
    -Wsized-deallocation #
)
set(CLANG_CXX_COMPILE_FLAGS
    -Weverything #
    -Wpedantic #
    -Wno-weak-vtables # Ignore, linker will remove the couple of extra vtables.
    -Wno-padded # Ignore, padding optimization is not needed.
    -Wno-c++98-compat # Ignore, CharLS 2.x targets C++14, ignore C++98
                      # compatibility.
    -Wno-c++98-compat-pedantic # Ignore, CharLS 2.x targets C++14, ignore C++98
                               # compatibility.
    -Wno-global-constructors # Ignore, by design CharLS uses types created at
                             # startup.
    -Wno-switch-enum # Ignore, cases are handled by default.
    -Wno-sign-conversion # Ignore, would just introduce ugly static_asserts.
    -Wno-exit-time-destructors # Ignore, by design exit-time destructors are
                               # used.
    -Wno-missing-braces # Ignore, False warning in clang 5.0, fixed in 6.0.
)

add_subdirectory(doc)

find_package(charls REQUIRED)
find_package(Boost 1.53 REQUIRED COMPONENTS program_options)

include(CheckSymbolExists)
check_symbol_exists(isatty "unistd.h" HAVE_ISATTY)

set_property(SOURCE options.cpp PROPERTY COMPILE_DEFINITIONS HAVE_ISATTY)

foreach(exe cjpls djpls jplsinfo jplstran)
  add_executable(
    ${exe}
    ${exe}.cpp
    ${exe}_options.cpp
    factory.cpp
    raw.cpp
    format.cpp
    pnm.cpp
    jls.cpp
    utils.cpp
    image.cpp
    source.cpp
    options.cpp
    dest.cpp
    crc32.cpp)
  target_compile_options(
    ${exe}
    PRIVATE $<$<CXX_COMPILER_ID:Clang>:${CLANG_CXX_COMPILE_FLAGS}>
            $<$<CXX_COMPILER_ID:GNU>:${GNU_CXX_COMPILE_FLAGS}>
            $<$<CXX_COMPILER_ID:MSVC>:${MSVC_CXX_COMPILE_FLAGS}>)
  # https://cliutils.gitlab.io/modern-cmake/chapters/features/utilities.html
  # https://blog.kitware.com/static-checks-with-cmake-cdash-iwyu-clang-tidy-lwyu-cpplint-and-cppcheck/
  if(JLST_USE_LWYU)
    set_target_properties(${exe} PROPERTIES LINK_WHAT_YOU_USE "TRUE")
  endif()
  if(JLST_USE_IWYU)
    set_target_properties(${exe} PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "iwyu")
  endif()
  if(JLST_USE_CLANG_TIDY)
    set_target_properties(
      ${exe}
      PROPERTIES CXX_CLANG_TIDY "clang-tidy"
                 "-checks=modernize-*,readability-*,performance-*" "-fix")
  endif()
  target_include_directories(${exe} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
  target_link_libraries(${exe} LINK_PRIVATE charls)
  target_link_libraries(${exe} LINK_PRIVATE ${Boost_LIBRARIES})
  install(
    TARGETS ${exe}
    DESTINATION ${JLST_INSTALL_BINDIR}
    COMPONENT libraries)
  # https://stackoverflow.com/questions/5096881/does-set-target-properties-in-cmake-override-cmake-cxx-flags
endforeach()

# locate charls root:
find_path(
  CHARLS_ROOT "test/SIEMENS-MR-RGB-16Bits.dcm"
  PATHS $ENV{CHARLS_ROOT} ${PROJECT_SOURCE_DIR}/../charls
  NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
mark_as_advanced(CHARLS_ROOT)

# locate charls test data root:
find_path(
  CHARLS_TEST_DATA "data/t87/T8SSE3.JLS"
  PATHS $ENV{CHARLS_TEST_DATA} ${PROJECT_SOURCE_DIR}/../charls-test-data
  NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
mark_as_advanced(CHARLS_TEST_DATA)

# test:
enable_testing()
include(CTest) # Memcheck
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
