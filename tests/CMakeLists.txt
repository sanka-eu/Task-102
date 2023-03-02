add_executable(protobuf_parser_test
    test_DelimitedMessagesStreamParser.cpp
    test_helpers.cpp
)

target_include_directories(protobuf_parser_test PRIVATE "${CMAKE_SOURCE_DIR}")
target_include_directories(protobuf_parser_test PRIVATE "${CMAKE_BINARY_DIR}")

target_link_libraries(protobuf_parser_test
    gtest
    gtest_main
    proto
)

include(GoogleTest)
gtest_discover_tests(protobuf_parser_test)
