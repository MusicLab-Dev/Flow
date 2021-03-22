cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(AtomicWait)

get_filename_component(AtomicWaitDir ${CMAKE_CURRENT_LIST_FILE} PATH)

find_package(Threads)

set(AtomicSources
    ${AtomicWaitDir}/atomic_wait
    ${AtomicWaitDir}/atomic.cpp
)

add_library(${PROJECT_NAME} ${AtomicSources})

target_link_libraries(${PROJECT_NAME} PUBLIC
    Threads::Threads
    m
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${AtomicWaitDir}
)
