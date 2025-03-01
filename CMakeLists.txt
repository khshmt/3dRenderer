cmake_minimum_required(VERSION 3.22.0)
set(PROJECT_NAME "renderer")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/static)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)
find_package(Thrust REQUIRED CONFIG)

add_library(Renderer STATIC ${CMAKE_SOURCE_DIR}/src/renderer.cpp)

target_link_libraries(Renderer PUBLIC 
                        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
                        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
                         $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
                         Thrust::Thrust
)
target_include_directories(Renderer PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME}_exec 
    #WIN32
    ${CMAKE_SOURCE_DIR}/main.cpp
)
target_link_libraries(${PROJECT_NAME}_exec
    PRIVATE
    Renderer
)

add_subdirectory(benchmark)