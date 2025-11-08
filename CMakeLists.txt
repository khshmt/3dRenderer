cmake_minimum_required(VERSION 3.22.0)
set(PROJECT_NAME "3dRenderer")
project(${PROJECT_NAME} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)
find_package(Tracy CONFIG REQUIRED)


add_library(Renderer SHARED ${CMAKE_SOURCE_DIR}/src/renderer.cpp)
target_link_libraries(Renderer PUBLIC 
                        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
                        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
                         $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
                        $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
                        Eigen3::Eigen
)
target_include_directories(Renderer PUBLIC ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/thirdparty/upng)
target_compile_definitions(Renderer PUBLIC RENDERER_EXPORTS)

if(ENABLE_PROFILING) 
    message(STATUS "Profiling enabled")
    target_link_libraries(Renderer PUBLIC Tracy::TracyClient)
    target_compile_definitions(Renderer PUBLIC TRACY_ENABLE TRACY_CALLSTACK)
endif()

add_executable(${PROJECT_NAME} 
    #WIN32
    ${CMAKE_SOURCE_DIR}/src/main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE Renderer)

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/assets/ttf/Roboto-Regular.ttf 
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Roboto-Regular.ttf
)

if(ENABLE_PROFILING) 
    add_subdirectory(benchmark)
endif()