cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

get_filename_component(FlowRoot ${CMAKE_CURRENT_LIST_FILE} PATH)

include(${FlowRoot}/CompileOptions/CompileOptions.cmake)

project(Flow)

add_submodule_directory(${FlowRoot}/Core)

include(${FlowRoot}/Flow/Flow.cmake)

if(TESTS)
    enable_testing()
    include(${FlowRoot}/Tests/FlowTests.cmake)
endif()

if(BENCHMARKS)
    include(${FlowRoot}/Benchmarks/FlowBenchmarks.cmake)
endif()