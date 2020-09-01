
find_package(ROOT REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)

add_library(task_main STATIC
        task_main.cpp
        TaskRegistry.cpp
        TaskRegistry.h
        UserTask.cpp
        UserTask.h
        Task.h)
target_link_libraries(task_main PUBLIC AnalysisTreeBase AnalysisTreeInfra
        Boost::program_options
        ROOT::Core)
target_include_directories(task_main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})