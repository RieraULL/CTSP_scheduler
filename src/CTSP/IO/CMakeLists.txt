##
## ctsp_io - CTSP Instance I/O Library
##
## This library provides file I/O and instance management for:
## - TSPLIB format parsing (standard TSP benchmark format)
## - PTSP (Periodic TSP) instances
## - CTSP (Consistent TSP) instances with synchronization constraints
##
## The library handles multi-day routing instances with time windows and
## demand patterns.
##

# Set the project name
project (ctsp_io)

# Source files
file(GLOB SOURCES 
    "src/TSPLIB_instance.cpp"  # TSPLIB format parser
    "src/PTSP_instance.cpp"    # Periodic TSP instances
    "src/CTSP_instance.cpp"    # Consistent TSP instances
)


# Add a library with the above sources
add_library(${PROJECT_NAME} 
    ${SOURCES})

# Export as sub::ctsp_io for use in parent project
add_library(sub::ctsp_io ALIAS ${PROJECT_NAME})

# Public include directory
target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

# Link dependencies
target_link_libraries(${PROJECT_NAME}
    sub::gomautil  # For matrix class
)

# Add compiler warnings for code quality
target_compile_options(${PROJECT_NAME} PRIVATE
  -Wall 
  -Wextra
  -Wshadow
)
