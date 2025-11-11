# ==============================================================================
# sync_checker Library - CMake Configuration
# ==============================================================================
#
# This library provides synchronization constraint checking for CTSP
# (Consistent Traveling Salesman Problem) using LP-based verification.
#
# Key Components:
# - ctsp_sync_checker: Base synchronization constraint checker
# - ctsp_primal_model: Primal LP model for synchronization verification
# - ctsp_lb_sync_checker: Specialized checker for lower bound computation
# - ctsp_lb_primal_model: Primal model for lower bounds
# - sync_iterative_checker: Template wrapper for iterative checking
#
# Functionality:
# - Verify if routing solutions satisfy temporal synchronization constraints
# - Extract dual variables (α, β, γ) for cut generation in branch-and-cut
# - Compute lower bounds for optimality certification
# - Support both CTSP1 (time window sync) and CTSP2 (exact sync)
#
# Dependencies:
# - gomautil: Utility functions and matrix operations
# - sync_model_a: CTSP model building and arc representations
# - sync_checker_solver: LP solver wrapper (CPLEX or alternatives)
#
# ==============================================================================

# Set the project name
project(sync_checker C CXX)

# Collect all source files
file(GLOB SOURCES 
    "src/ctsp_sync_checker.cpp"       # Base synchronization checker
    "src/ctsp_primal_model.cpp"       # Primal LP model construction
    "src/ctsp_lb_primal_model.cpp"    # Lower bound primal model
    "src/ctsp_lb_sync_checker.cpp"    # Lower bound checker
)

# Create the library
add_library(${PROJECT_NAME} 
    ${SOURCES}
)

# Link required libraries
target_link_libraries(${PROJECT_NAME}
    sub::gomautil              # Utility functions
    sub::sync_model_a          # CTSP model representation
    sub::sync_checker_solver   # LP solver interface
)

# Specify include directories
target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

# Create alias for use in parent projects
add_library(sub::sync_checker ALIAS ${PROJECT_NAME})   

