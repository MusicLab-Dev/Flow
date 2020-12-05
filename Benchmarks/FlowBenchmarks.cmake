project(AudioBenchmarks)

find_package(benchmark REQUIRED)

get_filename_component(AudioBenchmarksDir ${CMAKE_CURRENT_LIST_FILE} PATH)

set(AudioBenchmarksSources
    ${AudioBenchmarksDir}/Main.cpp
)

add_executable(${PROJECT_NAME} ${AudioBenchmarksSources})

target_link_libraries(${PROJECT_NAME}
PUBLIC
    Audio
    benchmark::benchmark
)
