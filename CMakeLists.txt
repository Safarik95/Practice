cmake_minimum_required(VERSION 3.22)

project(MusicProgram VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Test)
qt_standard_project_setup()

if(MSVC)
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(music_core STATIC
    src/result.h
    src/track.h
    src/program.h
    src/program.cpp
    src/storage.h
    src/storage.cpp
)
target_include_directories(music_core PUBLIC src)
target_link_libraries(music_core PUBLIC Qt6::Core)

qt_add_executable(MusicProgram WIN32
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
)
target_link_libraries(MusicProgram PRIVATE music_core Qt6::Widgets)

enable_testing()

qt_add_executable(test_program tests/test_program.cpp)
target_link_libraries(test_program PRIVATE music_core Qt6::Test)
add_test(NAME test_program COMMAND test_program)
set_tests_properties(test_program PROPERTIES
    ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")

qt_add_executable(test_storage tests/test_storage.cpp)
target_link_libraries(test_storage PRIVATE music_core Qt6::Test)
target_compile_definitions(test_storage PRIVATE DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
add_test(NAME test_storage COMMAND test_storage)
set_tests_properties(test_storage PROPERTIES
    ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")
