cmake_minimum_required(VERSION 3.5)
project(mtmetis-test)

add_library(GKlib SHARED IMPORTED)
set_property(TARGET GKlib PROPERTY IMPORTED_LOCATION "/jet/home/xiaf/build/lib/libGKlib.a")

add_library(metis SHARED IMPORTED)
set_property(TARGET metis PROPERTY IMPORTED_LOCATION "/jet/home/xiaf/build/lib/libmetis.a")
target_link_libraries(metis INTERFACE GKlib)

file(GLOB SOURCES "src/*.cpp")

add_subdirectory(src/graph)

add_executable(mtmetis-test ${SOURCES})

target_include_directories(mtmetis-test
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        /jet/home/xiaf/build/include
)

find_package(OpenMP REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX GKlib metis SnapFile) 