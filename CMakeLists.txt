cmake_minimum_required(VERSION 3.20)
project(parking_spaces LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(PS_BUILD_TESTS "Build tests" ON)
option(PS_BUILD_TOOLS "Build CLI tools" ON)

set(PS_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(PS_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(PS_ROOT ${PS_ROOT} CACHE PATH "Project root directory" FORCE)
set(PS_BUILD_DIR ${PS_BUILD_DIR} CACHE PATH "Build directory" FORCE)

include(FindPkgConfig)
include(GNUInstallDirs)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(GDAL REQUIRED)
pkg_check_modules(libprime_server IMPORTED_TARGET libprime_server>=0.6.3)
pkg_check_modules(libvalhalla REQUIRED IMPORTED_TARGET libvalhalla>=3.5.1)
pkg_check_modules(LZ4 REQUIRED IMPORTED_TARGET liblz4)
find_package(TIFF REQUIRED)
find_package(GeoTIFF REQUIRED)
find_package(ZLIB REQUIRED)
set(GTIFF_TARGETS TIFF::TIFF geotiff_library)
set(libosmium_include_dirs ${PS_ROOT}/third_party/libosmium/include)

set(protobuf_MODULE_COMPATIBLE ON CACHE BOOL "")
find_package(Protobuf REQUIRED)
message(STATUS "Using protoc from ${Protobuf_PROTOC_EXECUTABLE}")
message(STATUS "Using pbf headers from ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Using pbf libs from ${PROTOBUF_LIBRARIES}")

if(TARGET protobuf::libprotobuf)
  message(STATUS "Using pbf")
endif()

add_library(parking_spaces STATIC
    # sources here
    src/parking_spaces.cc
)

target_include_directories(parking_spaces PUBLIC include ${libosmium_include_dirs})
target_link_libraries(parking_spaces
  PUBLIC
    PkgConfig::libvalhalla 
    PkgConfig::LZ4
    ${GTIFF_TARGETS}
    GDAL::GDAL
    ZLIB::ZLIB
)

install(TARGETS parking_spaces
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shared NAMELINK_SKIP
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT development)

install(DIRECTORY include/parking_spaces
  DESTINATION include
)

if(PS_BUILD_TOOLS)
  set(cxxopts_include_dir ${CMAKE_SOURCE_DIR}/third_party/cxxopts/include)
  file(GLOB EXEC_FILES
      "${PS_ROOT}/tools/*.cc")

  foreach(src ${EXEC_FILES})
    get_filename_component(exec_name "${src}" NAME_WE)
    add_executable("${exec_name}" "${src}")
    target_link_libraries("${exec_name}" PRIVATE PkgConfig::libvalhalla PkgConfig::LZ4 parking_spaces)
    target_include_directories("${exec_name}" PRIVATE ${PS_ROOT}/tools ${cxxopts_include_dir})
    install(TARGETS "${exec_name}" DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT runtime)
  endforeach()
endif()

if(PS_BUILD_TESTS)
    enable_testing()
    include(GoogleTest)
    add_subdirectory(test)
endif()

