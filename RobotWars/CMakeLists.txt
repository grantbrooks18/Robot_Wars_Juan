cmake_minimum_required(VERSION 3.12)
project(RobotWars C)

set(CMAKE_C_STANDARD 99)

INCLUDE_DIRECTORIES( C:\\MinGW-Allegro-4.4.2\\include )
LINK_DIRECTORIES( C:\\MinGW-Allegro-4.4.2\\lib )

include_directories("precompiled")
include_directories("resources")
include_directories("src")

set(SOURCE_FILES
        src/competition.c
        src/graphics.c
        src/ll.c
        src/particles.c
        src/physics.c
        src/rockemsockem.c
        robots/bender.c
        robots/maximilian.c
        robots/Teemo.c
		"robots/precompiled/6R4V3 D1663R.o"
        robots/precompiled/capsule1337.o
        robots/precompiled/JR.o
        robots/precompiled/Lopez.o
		robots/Juan.c robots/Juan.h)

add_library(allegro STATIC IMPORTED C:\\MinGW-Allegro-4.4.2\\lib)

add_executable(RobotWars ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(RobotWars liballeg44.dll.a)

