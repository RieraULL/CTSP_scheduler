##
## ctsp_interfaz - CTSP Model Builder Interface
##
## This library provides the bridge between CTSP problem instances and the
## synchronization constraint framework. It adapts CTSP-specific data to
## the generic sync_model_a_builder interface.
##
## Supports both CTSP1 (single-depot) and CTSP2 (multi-depot) variants.
##

# Set the project name
project (ctsp_interfaz)

# Add a library with the above sources
add_library(${PROJECT_NAME} 
    src/CTSP_model_a_builder.cpp 
    )

# Export as sub::ctsp_interfaz for use in parent project
add_library(sub::ctsp_interfaz ALIAS ${PROJECT_NAME})

# Public include directory
target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

# Link dependencies
target_link_libraries(${PROJECT_NAME}
    sub::gomautil          # Utility library (matrix, solvers)
    sub::sync_model_a      # Synchronization model builder
    sub::ctsp_io           # CTSP instance I/O
)

# Add compiler warnings and flags
target_compile_options(${PROJECT_NAME} PRIVATE
  -Wall 
  -Wextra
  -Wshadow
  -Wunreachable-code
 # -g                      # Debug symbols (uncomment for debugging)
 # -O3                     # Optimization (uncomment for release)
 # -DNDEBUG                # Disable assertions (uncomment for release)
)
