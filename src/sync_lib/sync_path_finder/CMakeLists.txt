# Set the project name
project (sync_path_finder C CXX)
    
file(GLOB SOURCES 
    "src/path_finder.cpp" 
)

# Add a library with the above sources
add_library(${PROJECT_NAME} 
    ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    sub::gomautil
    sub::sync_model_a
)

target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

add_library(sub::sync_path_finder ALIAS ${PROJECT_NAME})   
