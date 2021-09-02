cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

get_filename_component(FlowDir ${CMAKE_CURRENT_LIST_FILE} PATH)

include(${FlowDir}/External/External.cmake)

project(Flow)

set(FlowPrecompiledHeaders
    ${FlowDir}/Graph.hpp
    ${FlowDir}/Node.hpp
    ${FlowDir}/NodeType.hpp
    ${FlowDir}/Scheduler.hpp
    ${FlowDir}/Task.hpp
    ${FlowDir}/Worker.hpp
    ${FlowDir}/AtomicWait.hpp
)

set(FlowSources
    ${FlowPrecompiledHeaders}
    ${FlowDir}/Graph.ipp
    ${FlowDir}/Graph.cpp
    ${FlowDir}/Scheduler.cpp
    ${FlowDir}/Scheduler.ipp
    ${FlowDir}/Task.ipp
    ${FlowDir}/Worker.cpp
    ${FlowDir}/Worker.ipp
)

add_library(${PROJECT_NAME} ${FlowSources})

if (CMAKE_CXX_COMPILER_ID MATCHES "Apple")
elseif()
    target_precompile_headers(${PROJECT_NAME} PUBLIC ${FlowPrecompiledHeaders})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${FlowDir}/..)

target_link_libraries(${PROJECT_NAME} PUBLIC Core AtomicWait)

if(CODE_COVERAGE)
    target_compile_options(${PROJECT_NAME} PUBLIC --coverage)
    target_link_options(${PROJECT_NAME} PUBLIC --coverage)
endif()
