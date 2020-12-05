project(FlowTests)

find_package(GTest REQUIRED)

get_filename_component(FlowTestsDir ${CMAKE_CURRENT_LIST_FILE} PATH)

set(FlowTestsSources
    ${FlowTestsDir}/tests_Scheduler.cpp
)

add_executable(${PROJECT_NAME} ${FlowTestsSources})

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
PUBLIC
    Flow
    GTest::GTest GTest::Main
)