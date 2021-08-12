project(FlowBenchmarks)

find_package(benchmark REQUIRED)

get_filename_component(FlowBenchmarksDir ${CMAKE_CURRENT_LIST_FILE} PATH)

set(FlowBenchmarksSources
    ${FlowBenchmarksDir}/Main.cpp
)

add_executable(${PROJECT_NAME} ${FlowBenchmarksSources})

target_link_libraries(${PROJECT_NAME}
PUBLIC
    Flow
    benchmark::benchmark
)
