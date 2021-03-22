project(FlowExternal)

find_package(Threads)

get_filename_component(FlowExternalDir ${CMAKE_CURRENT_LIST_FILE} PATH)


# Atomic wait
include(${FlowExternalDir}/AtomicWait/AtomicWait.cmake)