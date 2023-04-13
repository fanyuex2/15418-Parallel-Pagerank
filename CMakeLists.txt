cmake_minimum_required(VERSION 3.5)
project(page_rank)

file(GLOB SOURCES "src/*.cpp")

add_subdirectory(src/graph)

add_executable(page_rank ${SOURCES})

target_include_directories(page_rank
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

find_package(OpenMP REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX SnapFile) 