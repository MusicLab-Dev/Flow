cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(Flow)

get_filename_component(FlowDir ${CMAKE_CURRENT_LIST_FILE} PATH)

include(${FlowDir}/Atomic/Atomic.cmake)

set(FlowSources
    ${FlowDir}/Graph.hpp
    ${FlowDir}/Graph.ipp
    ${FlowDir}/Graph.cpp
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

add_library(${PROJECT_NAME} ${FlowSources} ${AtomicSources})

target_include_directories(${PROJECT_NAME} PUBLIC ${FlowDir}/.. ${AtomicDir})

target_link_libraries(${PROJECT_NAME} PUBLIC Core ${AtomicLibraries})

if(CODE_COVERAGE)
    target_compile_options(${PROJECT_NAME} PUBLIC --coverage)
    target_link_options(${PROJECT_NAME} PUBLIC --coverage)
endif()