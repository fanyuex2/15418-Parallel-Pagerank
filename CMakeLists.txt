cmake_minimum_required(VERSION 3.5)
project(mtmetis-test)

file(GLOB SOURCES "src/*.cpp")

add_subdirectory(src/graph)

add_executable(mtmetis-test ${SOURCES})

target_include_directories(mtmetis-test
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

find_package(OpenMP REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX ${CMAKE_SOURCE_DIR}/lib/libmtmetis.a SnapFile) 