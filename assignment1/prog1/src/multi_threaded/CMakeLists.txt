cmake_minimum_required(VERSION 3.0)
project(prog1)

# Add all your source files
set(SOURCES
    character_utils.c
    cli_utils.c
    main.c
    shared_region.c
)

# Add all your header files
set(HEADERS
    character_utils.h
    cli_utils.h
    constants.h
    shared_region.h
)

# Create an executable target
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Set compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -O3)
