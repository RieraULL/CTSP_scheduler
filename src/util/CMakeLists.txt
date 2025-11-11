##
## util - Linear Programming Utilities and Solver Wrappers
##
## This library provides:
## - Generic matrix class with 1-based indexing
## - Solver-independent model representation
## - CPLEX solver interface (can be replaced with CLP for open-source distribution)
##
## NOTE FOR OPEN-SOURCE DISTRIBUTION:
## This library currently uses IBM ILOG CPLEX (commercial license required).
## For open-source publication, consider replacing CPLEX with:
## - COIN-OR CLP (recommended, good performance)
## - GLPK (GNU Linear Programming Kit)
## - HiGHS (modern, excellent performance)
##
## To use this library with CLP instead of CPLEX:
## 1. Implement CLP_solver class following the LP_solver interface
## 2. Update this CMakeLists.txt to link against CLP libraries
## 3. Replace CPX_solver usage with CLP_solver in dependent code
##

# Set the project name
project (util)

# --- INCLUDE CPX LIBRARY TO PROJECT
# WARNING: This path is system-specific and points to a commercial CPLEX installation
# For open-source distribution, remove this section and link against open-source solvers
set (CPX_PATH /home/riera/Dropbox/Privado/Riera-Linux/ibm/ILOG/CPLEX_Studio221)

# Source files
file(GLOB SOURCES 
    "src/LP_solver.cpp"
    "src/CPX_solver.cpp"
    "src/CPX_model_structure.cpp"
    "src/model_description.cpp"
)

# Add a library with the above sources
add_library(${PROJECT_NAME} 
    ${SOURCES})

# Export as sub::gomautil for use in parent project
add_library(sub::gomautil ALIAS ${PROJECT_NAME})

# Public include directories
target_include_directories(${PROJECT_NAME}
    PUBLIC
        PUBLIC ${PROJECT_SOURCE_DIR}/include
        PUBLIC ${PROJECT_SOURCE_DIR}/include/CPX
        PRIVATE ${CPX_PATH}/cplex/include/
        PRIVATE ${CPX_PATH}/concert/include/
)

# Link CPLEX and CONCERT libraries
# NOTE: For open-source distribution, replace with CLP/GLPK/HiGHS libraries
set (CPX_LIB_PATH ${CPX_PATH}/cplex/lib/x86-64_linux/static_pic/)
set (ILO_LIB_PATH ${CPX_PATH}/concert/lib/x86-64_linux/static_pic/)

find_library(CPX_LIBRARY libcplex.a HINTS ${CPX_LIB_PATH})
find_library(ILO_LIBRARY libconcert.a HINTS ${ILO_LIB_PATH})

message(CPX_LIBRARY="${CPX_LIBRARY}")
message(ILO_LIBRARY="${ILO_LIBRARY}")

target_link_libraries(${PROJECT_NAME}
    ${ILO_LIBRARY}
    ${CPX_LIBRARY}
)
