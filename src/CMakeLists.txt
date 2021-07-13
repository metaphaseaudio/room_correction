cmake_minimum_required(VERSION 3.2)

project(ROCOR)
set(ROCOR_VERSION_MAJOR 1)
set(ROCOR_VERSION_MINOR 0)
set(VERSION ${ROCOR_VERSION_MAJOR}.${ROCOR_VERSION_MINOR})
add_definitions(-DROCOR_VERSION="${ROCOR_VERSION_MAJOR}.${ROCOR_VERSION_MINOR}")
include(${CMAKE_CURRENT_SOURCE_DIR}/shared/MUDS/cmake_snippets/CPP14Boilerplate.cmake)

#------------------------------------------------------------------------------
# Build the VST SDK
#------------------------------------------------------------------------------
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/shared/vst3sdk)
#------------------------------------------------------------------------------
# Build the JUCE library
#------------------------------------------------------------------------------
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/shared/JUCE)
#------------------------------------------------------------------------------
# Build Metaphase Shared Libraries
#------------------------------------------------------------------------------
add_subdirectory(shared/metaphase_core)

#------------------------------------------------------------------------------
# Build ROCOR
#------------------------------------------------------------------------------
juce_add_plugin(ROCOR
        VERSION 1.0
        COMPANY_NAME Metaphase
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        VST_NUM_MIDI_INS 0
        PLUGIN_MANUFACTURER_CODE Mtap
        PLUGIN_CODE MROC
        COPY_PLUGIN_AFTER_BUILD TRUE
        FORMATS VST3 Standalone
        PRODUCT_NAME "ROCOR")

juce_generate_juce_header(ROCOR)

FILE(GLOB_RECURSE ROCOR_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rocor/*.cpp
                               ${CMAKE_CURRENT_SOURCE_DIR}/rocor/*.h)
LIST(SORT ROCOR_SOURCE)

target_sources(ROCOR PRIVATE ${ROCOR_SOURCE})
target_compile_definitions(ROCOR
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(ROCOR
    PUBLIC
        juce::juce_audio_utils
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
    PUBLIC
        metaphase_core
)

option(BUILD_TESTS "BUILD_TESTS" OFF)
if (BUILD_TESTS)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
endif()
