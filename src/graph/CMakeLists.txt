

add_library(SnapFile TextFile.cpp SNAPFile.cpp)

target_include_directories(SnapFile
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(SnapFile  PUBLIC)