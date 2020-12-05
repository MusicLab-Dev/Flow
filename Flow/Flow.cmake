cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(Flow)

get_filename_component(FlowDir ${CMAKE_CURRENT_LIST_FILE} PATH)

set(FlowSources
    ${FlowDir}/Graph.hpp
    ${FlowDir}/Graph.ipp
    ${FlowDir}/Node.hpp
    ${FlowDir}/NodeType.hpp
    ${FlowDir}/Scheduler.cpp
    ${FlowDir}/Scheduler.hpp
    ${FlowDir}/Scheduler.ipp
    ${FlowDir}/Task.hpp
    ${FlowDir}/Task.ipp
    ${FlowDir}/Worker.cpp
    ${FlowDir}/Worker.hpp
    ${FlowDir}/Worker.ipp
)

add_library(${PROJECT_NAME} ${FlowSources})

target_include_directories(${PROJECT_NAME} PUBLIC ${FlowDir}/..)

target_link_libraries(${PROJECT_NAME} PUBLIC Core)

if(CODE_COVERAGE)
    target_compile_options(${PROJECT_NAME} PUBLIC --coverage)
    target_link_options(${PROJECT_NAME} PUBLIC --coverage)
endif()