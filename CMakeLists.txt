cmake_minimum_required(VERSION 3.16)
project(RandomPasswordGenerator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Concurrent)

function(enable_warnings target)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE -Wall -Wextra)
    endif()
endfunction()

add_executable(PasswordGenerator main.cpp password_generator.h)
target_link_libraries(PasswordGenerator PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
enable_warnings(PasswordGenerator)

add_executable(password_generator_multithreaded multithreaded_main.cpp password_generator.h)
target_link_libraries(password_generator_multithreaded PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Concurrent)
enable_warnings(password_generator_multithreaded)

enable_testing()
add_executable(test_generator test_generator.cpp password_generator.h)
enable_warnings(test_generator)
add_test(NAME generator COMMAND test_generator)
