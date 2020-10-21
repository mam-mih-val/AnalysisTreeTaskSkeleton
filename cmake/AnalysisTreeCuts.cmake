include(FetchContent)
FetchContent_Declare(
        AnalysisTreeCuts
        GIT_REPOSITORY https://git.cbm.gsi.de/pwg-c2f/analysis/cuts.git
)

FetchContent_GetProperties(AnalysisTreeCuts)

if (NOT analysistreecuts_POPULATED)
    FetchContent_Populate(AnalysisTreeCuts)
    add_subdirectory(${analysistreecuts_SOURCE_DIR} ${analysistreecuts_BINARY_DIR})
endif ()