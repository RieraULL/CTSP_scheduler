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

option(USE_CLP "Use COIN-OR CLP instead of IBM CPLEX" OFF)
message(STATUS "USE_CLP=${USE_CLP}")

# Source files
set(SOURCES
    src/LP_solver.cpp
    src/model_description.cpp
    src/graph.cpp
)

if (USE_CLP)
    list(APPEND SOURCES
        src/CLP_solver.cpp
        src/CLP_model_structure.cpp
    )
else()
    # --- INCLUDE CPX LIBRARY TO PROJECT
    # WARNING: This path is system-specific and points to a commercial CPLEX installation
    # For open-source distribution, enable USE_CLP and link against CLP instead
    set (CPX_PATH /home/riera/Dropbox/Privado/Riera-Linux/ibm/ILOG/CPLEX_Studio221)

    list(APPEND SOURCES
        src/CPX_solver.cpp
        src/CPX_model_structure.cpp
    )
endif()

# Add a library with the above sources
add_library(${PROJECT_NAME} 
    ${SOURCES})

# Export as sub::gomautil for use in parent project
add_library(sub::gomautil ALIAS ${PROJECT_NAME})

# Public include directories
if (USE_CLP)
    # CLP headers come from the system installation
    # Try CMake config package first; if not present, fall back to manual discovery
    find_package(Clp CONFIG QUIET)

    if (Clp_FOUND)
        target_include_directories(${PROJECT_NAME}
            PUBLIC
                ${PROJECT_SOURCE_DIR}/include
                ${PROJECT_SOURCE_DIR}/include/CLP
        )
    else()
        message(STATUS "ClpConfig.cmake no encontrado. Intentando descubrimiento manual de CLP (headers y libs del sistema)...")

        # Headers típicamente en /usr/include/coin
        find_path(CLP_INCLUDE_DIR
            NAMES ClpSimplex.hpp ClpConfig.h
            PATHS /usr/include /usr/local/include
            PATH_SUFFIXES coin
        )
        if (NOT CLP_INCLUDE_DIR)
            message(FATAL_ERROR "No se encontraron los headers de CLP. Instala coinor-libclp-dev (y dependencias).")
        endif()

        # Librerías típicamente en /usr/lib/x86_64-linux-gnu
        set(_CLP_LIB_HINTS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)
        find_library(CLP_LIB       NAMES Clp       HINTS ${_CLP_LIB_HINTS})
        find_library(CLP_SOLVER_LIB NAMES ClpSolver HINTS ${_CLP_LIB_HINTS})
        find_library(COINUTILS_LIB NAMES CoinUtils HINTS ${_CLP_LIB_HINTS})
        find_library(OSI_LIB       NAMES Osi       HINTS ${_CLP_LIB_HINTS})
        find_library(OSICLP_LIB    NAMES OsiClp    HINTS ${_CLP_LIB_HINTS})

        if (NOT CLP_LIB OR NOT CLP_SOLVER_LIB OR NOT COINUTILS_LIB)
            message(FATAL_ERROR "No se encontraron las librerías de CLP/CoinUtils requeridas. Asegura instalación de coinor-libclp-dev y coinor-libcoinutils-dev.")
        endif()

        target_include_directories(${PROJECT_NAME}
            PUBLIC
                ${PROJECT_SOURCE_DIR}/include
                ${PROJECT_SOURCE_DIR}/include/CLP
                ${CLP_INCLUDE_DIR}
        )
    endif()
else()
    target_include_directories(${PROJECT_NAME}
        PUBLIC
            ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/include/CPX
        PRIVATE ${CPX_PATH}/cplex/include/
        PRIVATE ${CPX_PATH}/concert/include/
    )
endif()

# Link CPLEX and CONCERT libraries
# NOTE: For open-source distribution, replace with CLP/GLPK/HiGHS libraries
if (USE_CLP)
    # Prefer imported target if provided
    if (TARGET Clp::Clp)
        target_link_libraries(${PROJECT_NAME} PUBLIC Clp::Clp)
    elseif (Clp_FOUND)
        # Config package encontrado pero sin target importado estándar
        target_link_libraries(${PROJECT_NAME} PUBLIC Clp)
    else()
        # Descubrimiento manual: enlazar librerías básicas de CLP y dependencias
        set(_CLP_LINK_LIBS)
        if (CLP_LIB)
            list(APPEND _CLP_LINK_LIBS ${CLP_LIB})
        endif()
        if (CLP_SOLVER_LIB)
            list(APPEND _CLP_LINK_LIBS ${CLP_SOLVER_LIB})
        endif()
        if (OSI_LIB)
            list(APPEND _CLP_LINK_LIBS ${OSI_LIB})
        endif()
        if (OSICLP_LIB)
            list(APPEND _CLP_LINK_LIBS ${OSICLP_LIB})
        endif()
        if (COINUTILS_LIB)
            list(APPEND _CLP_LINK_LIBS ${COINUTILS_LIB})
        endif()
        target_link_libraries(${PROJECT_NAME} PUBLIC ${_CLP_LINK_LIBS})
    endif()
    target_compile_definitions(${PROJECT_NAME} PUBLIC USE_CLP)
else()
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
endif()
