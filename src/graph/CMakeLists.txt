

add_library(SnapFile TextFile.cpp SNAPFile.cpp)


find_package(OpenMP REQUIRED)

target_include_directories(SnapFile
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(SnapFile  PUBLIC OpenMP::OpenMP_CXX ${CMAKE_SOURCE_DIR}/lib/libmtmetis.a)