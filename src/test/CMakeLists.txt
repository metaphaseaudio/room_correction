message("-- RoCor Plugin Test Suite build enabled")

add_executable(run_rocor_tests AllTests.cpp)
target_link_libraries(run_rocor_tests
        ${GTEST_LIBRARIES}
        ${JUCE_LIBS}
        ${META_SHARED_LIBS}
        juce
        metaphase_core
        rocorlib)

set_target_properties(run_rocor_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_BINARY_DIR}/test/metaphase_core)
