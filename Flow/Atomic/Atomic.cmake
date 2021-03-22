cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

get_filename_component(AtomicDir ${CMAKE_CURRENT_LIST_FILE} PATH)

find_package(Threads)

set(AtomicSources
    ${AtomicDir}/atomic_wait
    ${AtomicDir}/semaphore
    ${AtomicDir}/latch
    ${AtomicDir}/barrier
    ${AtomicDir}/atomic.cpp
)

set(AtomicLibraries
    Threads::Threads
    m
)
