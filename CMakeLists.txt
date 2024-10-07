cmake_minimum_required(VERSION 3.22.0)
set(PROJECT_NAME "renderer")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/static)


find_package(SDL2 CONFIG REQUIRED)

add_library(Renderer STATIC ${CMAKE_SOURCE_DIR}/src/renderer.cpp)
target_link_libraries(Renderer PUBLIC 
                        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
                        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>)
target_include_directories(Renderer PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME}_exec ${CMAKE_SOURCE_DIR}/main.cpp)
target_link_libraries(${PROJECT_NAME}_exec
    PRIVATE
    Renderer
)