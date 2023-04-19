cmake_minimum_required(VERSION 3.5)
project(pagerank)

add_library(GKlib SHARED IMPORTED)
set_property(TARGET GKlib PROPERTY IMPORTED_LOCATION "/jet/home/xiaf/pagerank/lib/libGKlib.a")

add_library(mtmetis SHARED IMPORTED)
set_property(TARGET mtmetis PROPERTY IMPORTED_LOCATION "/jet/home/xiaf/pagerank/lib/libmtmetis.a")
target_link_libraries(mtmetis INTERFACE GKlib)

file(GLOB SOURCES "src/*.cpp")

add_subdirectory(src/graph)

add_executable(pagerank ${SOURCES})

target_include_directories(pagerank
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/fixgraph/
)

find_package(OpenMP REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX GKlib mtmetis SnapFile) 