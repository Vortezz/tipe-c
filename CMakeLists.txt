cmake_minimum_required(VERSION 3.22)
project(tipe C)

set(CMAKE_C_STANDARD 99)

add_executable(tipe main.c draw.c
        typings.c
        misc.c
        grid.c)
