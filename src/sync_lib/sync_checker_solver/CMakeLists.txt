# ==============================================================================
# sync_checker_solver Library - CMake Configuration
# ==============================================================================
#
# This library provides a high-level wrapper around LP solvers for
# solving linear programming problems in CTSP synchronization checking and
# constraint separation algorithms.
#
# Key Features:
# - Simplified interface to LP solvers for LP solving
# - Dynamic constraint addition (cut generation)
# - Dual variable extraction for reduced cost computation
# - Model modification capabilities (RHS, bounds, coefficients)
#
# Dependencies:
# - LP Solver (one of the following):
#   * CPLEX: Commercial LP/MIP solver (IBM ILOG CPLEX Optimization Studio)
#            Current implementation - requires commercial license
#   * CLP: Open-source alternative (COIN-OR Linear Programming)
#          Free alternative - requires implementing CLP_solver adapter
#   * Other: Gurobi, HiGHS, GLPK, etc. (require solver-specific adapters)
#
# - gomautil: Utility functions and data structures
# - sync_model_a: CTSP model building and representation
#
# Switching to CLP (Open-Source Alternative):
# To replace CPLEX with CLP:
# 1. Implement a CLP_solver class following the LP_solver interface
# 2. Replace CPX_solver with CLP_solver in sync_checker_solver.cpp
# 3. Update include directories and link libraries below to use CLP
#    Example: target_link_libraries(${PROJECT_NAME} Clp CoinUtils)
# 4. Remove CPLEX-specific includes and paths
#
# Note: Current implementation uses CPLEX. Update CPX_PATH below
# to point to your CPLEX installation directory.
# ==============================================================================

# Set the project name
project(sync_checker_solver C CXX)

# --- CPLEX LIBRARY PATH CONFIGURATION ---
# Update this path to match your CPLEX installation
# Example paths:
#   Linux:   /opt/ibm/ILOG/CPLEX_Studio221
#   Windows: C:/Program Files/IBM/ILOG/CPLEX_Studio221
#   macOS:   /Applications/CPLEX_Studio221
set(CPX_PATH /home/riera/Dropbox/Privado/Riera-Linux/ibm/ILOG/CPLEX_Studio221)
    
# Collect source files
file(GLOB SOURCES 
    "src/sync_checker_solver.cpp" 
)

# Create the library
add_library(${PROJECT_NAME} 
    ${SOURCES}
)

# Link required libraries
target_link_libraries(${PROJECT_NAME}
    sub::gomautil         # Utility functions
    sub::sync_model_a     # CTSP model representation
)

# Specify include directories
target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include          # Public headers for this library
    PRIVATE ${CPX_PATH}/cplex/include/           # CPLEX headers
    PRIVATE ${CPX_PATH}/concert/include/          # CPLEX Concert Technology
)

# Create alias for use in parent projects
add_library(sub::sync_checker_solver ALIAS ${PROJECT_NAME})   

