cmake_minimum_required (VERSION 3.5)

project(ctsp_schd)

add_subdirectory(util)

add_subdirectory(sync_lib/sync_IO)
add_subdirectory(sync_lib/sync_verify)
add_subdirectory(sync_lib/sync_checker_solver)
add_subdirectory(sync_lib/sync_checker)
add_subdirectory(sync_lib/sync_path_finder)

add_subdirectory(CTSP/IO)
add_subdirectory(CTSP/interface)

add_subdirectory(main)
