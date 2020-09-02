
find_package(ROOT REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)

list(APPEND SOURCES
        task_main.cpp
        TaskRegistry.cpp
        UserTask.cpp
        )

list(APPEND HEADERS
        TaskRegistry.h
        UserTask.h
        Task.h
        )

add_library(task_main STATIC ${SOURCES} ${HEADERS})
target_link_libraries(task_main PUBLIC AnalysisTreeBase AnalysisTreeInfra
        Boost::program_options
        ROOT::Core)
add_custom_command(TARGET task_main POST_BUILD
        COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${CMAKE_BINARY_DIR}/include/at_task
        COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${HEADERS} ${CMAKE_BINARY_DIR}/include/at_task
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
target_include_directories(task_main
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
            $<INSTALL_INTERFACE:include>
        )