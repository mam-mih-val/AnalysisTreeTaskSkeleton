cmake_minimum_required(VERSION 3.17)
project(AnalysisTreeSkeleton)

set(CMAKE_CXX_STANDARD 17)


list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(AnalysisTree)


add_subdirectory(task_main)
add_subdirectory(sample_task)
