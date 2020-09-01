
set(AnalysisTree_BUILD_TYPE RELEASE CACHE STRING "Build type of the AnalysisTree")
set(AnalysisTree_GIT_TAG v1.0.1 CACHE STRING "Git tag of the AnalysisTree")

include(FetchContent)
FetchContent_Declare(
        AnalysisTree
        GIT_REPOSITORY https://github.com/HeavyIonAnalysis/AnalysisTree.git
        GIT_TAG        ${AnalysisTree_GIT_TAG}
)

FetchContent_GetProperties(AnalysisTree)

if (NOT AnalysisTree_POPULATED)
    FetchContent_Populate(AnalysisTree)

    # there is no machinery to set variable for nested subdirectory only :C
    set(CMAKE_BUILD_TYPE_SAVED ${CMAKE_BUILD_TYPE})
    set(CMAKE_BUILD_TYPE ${AnalysisTree_BUILD_TYPE})
    add_subdirectory(${analysistree_SOURCE_DIR} ${analysistree_BINARY_DIR})
    set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE_SAVED})

    # Viktor, please, use modern CMake in AnalysisTree
    target_include_directories(AnalysisTreeBase INTERFACE
            $<BUILD_INTERFACE:${analysistree_SOURCE_DIR}>
            $<BUILD_INTERFACE:${analysistree_SOURCE_DIR}>/core # nasty
            )
    target_include_directories(AnalysisTreeInfra INTERFACE
            $<BUILD_INTERFACE:${analysistree_SOURCE_DIR}>
            $<BUILD_INTERFACE:${analysistree_SOURCE_DIR}>/infra
            )
endif ()
