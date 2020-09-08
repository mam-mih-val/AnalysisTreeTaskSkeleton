
find_package(ROOT REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)

list(APPEND SOURCES
        TaskRegistry.cpp
        UserTask.cpp
        )

list(APPEND HEADERS
        TaskRegistry.h
        UserTask.h
        Task.h
        )

add_library(at_task STATIC ${SOURCES} ${HEADERS})
target_link_libraries(at_task  PUBLIC AnalysisTreeBase AnalysisTreeInfra
        Boost::program_options
        ROOT::Core)
add_custom_command(TARGET at_task POST_BUILD
        COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${CMAKE_BINARY_DIR}/include/at_task
        COMMAND ${CMAKE_COMMAND} ARGS -E copy ${HEADERS} ${CMAKE_BINARY_DIR}/include/at_task
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${HEADERS}
)
target_include_directories(at_task
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
            $<INSTALL_INTERFACE:include>
        )

add_library(at_task_main STATIC task_main.cpp)
target_link_libraries(at_task_main PUBLIC at_task)
