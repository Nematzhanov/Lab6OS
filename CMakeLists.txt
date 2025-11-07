cmake_minimum_required(VERSION 3.5)
project(Lab6OS)

add_executable( Lab6OS
       Code/main.cpp )

enable_testing()


add_test(NAME Lab6_Test
        COMMAND bash -c "echo '5 10 50' | ./Lab6OS")

set_tests_properties(Lab6_Test PROPERTIES
        PASS_REGULAR_EXPRESSION "Отсортированный массив"
)

target_compile_options(Lab6OS PRIVATE -Wall -Wextra -pedantic)