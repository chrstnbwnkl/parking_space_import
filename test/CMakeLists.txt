file(MAKE_DIRECTORY ${PS_BUILD_DIR}/test/data)
find_package(ValhallaTest REQUIRED)

find_package(GTest REQUIRED)
file(GLOB_RECURSE TEST_FILES "${CMAKE_CURRENT_LIST_DIR}/test_*.cc")

# Test executable
add_executable(tests 
  ${TEST_FILES}
)

target_compile_definitions(tests PRIVATE
  PS_ROOT="${PS_ROOT}"
  PS_BUILD_DIR="${PS_BUILD_DIR}"
)

target_link_libraries(tests PRIVATE parking_spaces GTest::gtest GTest::gtest_main ${VALHALLA_TEST_LIB})

# Discover tests automatically
gtest_discover_tests(tests)
