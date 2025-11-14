# ==============================================================================
# sync_IO Library - CMake Configuration
# ==============================================================================
# 
# This library provides:
# - Data structures for CTSP solutions, schedules, and time windows
# - Model builders for creating operation-based and arc-based formulations
# - I/O utilities for reading/writing solutions in JSON format
# - Conversion interfaces between different solution representations
#
# Dependencies:
# - gomautil: Matrix operations and utility functions
# ==============================================================================

# Set the project name
project(sync_model_a)

# Add a library with all source files
add_library(${PROJECT_NAME} 
    # Core data structures
    src/sync_operations.cpp       # Operations, arcs, subsets, and partitions
    src/sync_solution.cpp          # Solution representation (routes)
    src/sync_scheduling.cpp        # Scheduling with timing information
    src/sync_infeasible.cpp
    src/sync_tw.cpp                # Time windows representation
    
    # Model builders
    src/sync_model_builder.cpp     # Base model builder (operations & partitions)
    src/sync_model_a_builder.cpp   # Model A builder (arc-based formulation)
    
    # Utilities
    src/sync_mapping.cpp           # Pair-to-index mappings
    src/json_format_io.cpp         # JSON I/O utilities
    src/model_a_solution_interface.cpp  # Solution conversion interface
)
    
# Create an alias for use in subdirectories
add_library(sub::sync_model_a ALIAS ${PROJECT_NAME})

# Specify include directories for this library
target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

# Link dependencies
target_link_libraries(${PROJECT_NAME}
    sub::gomautil  # Matrix utilities
) 

