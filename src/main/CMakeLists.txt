# ==============================================================================
# CTSP Scheduler Application
# ==============================================================================
# This CMake configuration builds the main executable that converts CTSP
# routing solutions into temporal schedules with time windows.
#
# Executable: ctsp_scheduler
# Purpose: Load instance and solution files, generate schedules, write JSON
# Dependencies: All project libraries (util, CTSP, sync_lib)
# ==============================================================================

project(ctsp_scheduler)

# Link directories for libraries
LINK_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib )

# Configure output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Multi-configuration generators (MSVC, Xcode)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/lib )
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/lib )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

# Debug builds get "_d" suffix
set (CMAKE_DEBUG_POSTFIX "_d")


# ==============================================================================
# Executable Definition
# ==============================================================================
# Creates the ctsp_scheduler executable from:
# - main.cpp: Entry point and command-line parsing
# - schedulers.cpp: CTSP scheduling algorithms
# - sch_io.cpp: Input/output utilities
# ==============================================================================
add_executable(${PROJECT_NAME}
src/main.cpp 
src/schedulers.cpp
src/sch_io.cpp
)

# Include directories
target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)
 
# ==============================================================================
# Library Dependencies
# ==============================================================================
# Links all necessary libraries:
# - sub::gomautil: Matrix class and LP solver interface
# - sub::ctsp_io: TSPLIB parser and instance management
# - sub::ctsp_interfaz: CTSP model builder adapter
# - sub::sync_model_a: Synchronization model I/O
# - sub::sync_verify: Solution to schedule converter (sol_2_scheduling)
# - sub::sync_checker: Synchronization constraint verification
# - sub::sync_checker_solver: LP solver wrapper
# ==============================================================================
target_link_libraries(
    ${PROJECT_NAME}
    sub::gomautil
    sub::ctsp_io
    sub::ctsp_interfaz
    sub::sync_model_a
    sub::sync_verify
    sub::sync_checker
    sub::sync_checker_solver
)  

