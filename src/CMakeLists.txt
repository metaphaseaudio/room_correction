cmake_minimum_required(VERSION 3.2)

project(ROCOR)
set(ROCOR_VERSION_MAJOR 1)
set(ROCOR_VERSION_MINOR 0)
set(VERSION ${ROCOR_VERSION_MAJOR}.${ROCOR_VERSION_MINOR})
add_definitions(-DROCOR_VERSION="${ROCOR_VERSION_MAJOR}.${ROCOR_VERSION_MINOR}")
include(${CMAKE_CURRENT_SOURCE_DIR}/shared/MUDS/cmake_snippets/CPP14Boilerplate.cmake)

#------------------------------------------------------------------------------
# Build the JUCE library
#------------------------------------------------------------------------------
set(JUCE_APP_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/juce_includes/AppConfig.h)
set(JUCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/shared/JUCE)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/juce_includes)

set(JUCE_ENABLE_PLUGIN_VST ON)
set(JUCE_ENABLE_AUDIO_BASICS ON)
set(JUCE_ENABLE_AUDIO_FORMATS ON)
set(JUCE_ENABLE_AUDIO_PROCESSORS ON)
set(JUCE_ENABLE_AUDIO_DEVICES ON)
set(JUCE_ENABLE_AUDIO_UTILS ON)
set(JUCE_ENABLE_CRYPTOGRAPHY ON)
set(JUCE_ENABLE_DATA_STRUCTURES ON)
set(JUCE_ENABLE_DSP ON)
set(JUCE_ENABLE_EVENTS ON)
set(JUCE_ENABLE_GRAPHICS ON)
set(JUCE_ENABLE_GUI_BASICS ON)
set(JUCE_ENABLE_GUI_EXTRA ON)

include(${CMAKE_CURRENT_SOURCE_DIR}/shared/MUDS/cmake_snippets/CompileJUCE.cmake)

#------------------------------------------------------------------------------
# Build Metaphase Shared Libraries
#------------------------------------------------------------------------------
add_subdirectory(shared/metaphase_core)

#------------------------------------------------------------------------------
# Build ROCOR
#------------------------------------------------------------------------------
FILE(GLOB_RECURSE ROCOR_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rocor/*.cpp
                               ${CMAKE_CURRENT_SOURCE_DIR}/rocor/*.h)
LIST(SORT ROCOR_SOURCE)

add_library(rocorlib STATIC ${ROCOR_SOURCE})
set_target_properties(rocorlib PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(rocorlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/ROCOR)

# This corresponds with the VST plug-in itself.
add_library(RoCor MODULE ${JUCE_PLUGIN_SOURCE})

set(ROCOR_SOURCE
    ${ROCOR_SOURCE}
    ${JUCE_PLUGIN_SOURCE})

set(ROCOR_LIBS
    ${JUCE_LIBS}
    ${META_SHARED_LIBS}
    juce
    metaphase_core)

target_link_libraries(rocorlib ${ROCOR_LIBS} )
target_link_libraries(RoCor rocorlib)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/standalone)

if(CMAKE_THREAD_LIBS_INIT)
    target_link_libraries(RoCor ${CMAKE_THREAD_LIBS_INIT})
endif()

option(BUILD_TESTS "BUILD_TESTS" OFF)
if (BUILD_TESTS)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
endif()
