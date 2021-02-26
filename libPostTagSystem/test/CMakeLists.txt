set(_link_libraries PostTagSystem ${GTEST_LIBRARIES})

add_executable(PostTagSystem_test PostTagSystem_test.cpp)
add_executable(CheckpointsTrie_test CheckpointsTrie_test.cpp)

target_link_libraries(PostTagSystem_test ${_link_libraries})
target_link_libraries(CheckpointsTrie_test ${_link_libraries})

gtest_discover_tests(PostTagSystem_test)
gtest_discover_tests(CheckpointsTrie_test)
