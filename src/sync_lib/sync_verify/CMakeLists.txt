##
## sync_verify - CTSP Solution to Scheduling Converter
##
## This library converts CTSP solutions into temporal schedules with time windows.
## It verifies synchronization constraints and generates practical schedules for implementation.
##

# Set the project name
project (sync_verify C CXX)
    
# Source files
file(GLOB SOURCES 
    "src/sol_2_scheduling.cpp" 
)


# Add a library with the above sources
add_library(${PROJECT_NAME} 
    ${SOURCES})

# Link dependencies
# - sync_model_a: For model building and problem data
# - sync_checker: For synchronization constraint verification
target_link_libraries(${PROJECT_NAME}
    sub::sync_model_a
    sub::sync_checker
)

# Public include directory
target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

# Export as sub::sync_verify for use in parent project
add_library(sub::sync_verify ALIAS ${PROJECT_NAME})   
