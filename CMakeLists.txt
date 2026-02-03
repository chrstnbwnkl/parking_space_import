cmake_minimum_required(VERSION 3.20)
project(ndw_traffic LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(PS_BUILD_TESTS "Build tests" ON)
option(PS_BUILD_TOOLS "Build CLI tools" ON)

set(PS_TRAFFIC_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(PS_TRAFFIC_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(PS_TRAFFIC_ROOT ${PS_TRAFFIC_ROOT} CACHE PATH "Project root directory" FORCE)
set(PS_TRAFFIC_BUILD_DIR ${PS_TRAFFIC_BUILD_DIR} CACHE PATH "Build directory" FORCE)

include(FindPkgConfig)
include(GNUInstallDirs)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(GDAL REQUIRED)
pkg_check_modules(libprime_server IMPORTED_TARGET libprime_server>=0.6.3)
pkg_check_modules(libvalhalla REQUIRED IMPORTED_TARGET libvalhalla>=3.5.1)
pkg_check_modules(PUGIXML REQUIRED pugixml)
pkg_check_modules(LZ4 REQUIRED IMPORTED_TARGET liblz4)
pkg_check_modules(HIREDIS REQUIRED hiredis)
find_package(TIFF REQUIRED)
find_package(GeoTIFF REQUIRED)
find_package(ZLIB REQUIRED)
set(GTIFF_TARGETS TIFF::TIFF geotiff_library)

set(protobuf_MODULE_COMPATIBLE ON CACHE BOOL "")
find_package(Protobuf REQUIRED)
message(STATUS "Using protoc from ${Protobuf_PROTOC_EXECUTABLE}")
message(STATUS "Using pbf headers from ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Using pbf libs from ${PROTOBUF_LIBRARIES}")

if(TARGET protobuf::libprotobuf)
  message(STATUS "Using pbf")
endif()

add_library(ndw_traffic STATIC
    # sources here
    src/control_socket.cc
)

target_include_directories(ndw_traffic PUBLIC include ${HIREDIS_INCLUDE_DIRS})
target_link_libraries(ndw_traffic
  PUBLIC
    PkgConfig::libvalhalla 
    PkgConfig::LZ4
    ${PUGIXML_LIBRARIES}
    ${GTIFF_TARGETS}
    GDAL::GDAL
    ZLIB::ZLIB
    ${HIREDIS_LIBRARIES}
)

install(TARGETS ndw_traffic
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shared NAMELINK_SKIP
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT development)

install(DIRECTORY include/ndw
  DESTINATION include
)

if(PS_BUILD_TOOLS)
  set(cxxopts_include_dir ${CMAKE_SOURCE_DIR}/third_party/cxxopts/include)
  file(GLOB EXEC_FILES
      "${PS_TRAFFIC_ROOT}/tools/*.cc")

  foreach(src ${EXEC_FILES})
    get_filename_component(exec_name "${src}" NAME_WE)
    add_executable("${exec_name}" "${src}")
    target_link_libraries("${exec_name}" PRIVATE PkgConfig::libvalhalla PkgConfig::LZ4 ndw_traffic)
    target_include_directories("${exec_name}" PRIVATE ${PS_TRAFFIC_ROOT}/tools ${cxxopts_include_dir})
    install(TARGETS "${exec_name}" DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT runtime)
  endforeach()
endif()

if(PS_BUILD_TESTS)
    enable_testing()
    include(GoogleTest)
    add_subdirectory(test)
endif()

