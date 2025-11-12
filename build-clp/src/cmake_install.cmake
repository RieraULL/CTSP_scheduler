# Install script for directory: /home/riera/Documentos/GitHub/CTSP_scheduler/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/riera/Documentos/GitHub/CTSP_scheduler/build-clp/src/util/cmake_install.cmake")
  include("/home/riera/Documentos/GitHub/CTSP_scheduler/build-clp/src/sync_lib/sync_IO/cmake_install.cmake")
  include("/home/riera/Documentos/GitHub/CTSP_scheduler/build-clp/src/sync_lib/sync_verify/cmake_install.cmake")
  include("/home/riera/Documentos/GitHub/CTSP_scheduler/build-clp/src/sync_lib/sync_checker_solver/cmake_install.cmake")
  include("/home/riera/Documentos/GitHub/CTSP_scheduler/build-clp/src/sync_lib/sync_checker/cmake_install.cmake")
  include("/home/riera/Documentos/GitHub/CTSP_scheduler/build-clp/src/CTSP/IO/cmake_install.cmake")
  include("/home/riera/Documentos/GitHub/CTSP_scheduler/build-clp/src/CTSP/interface/cmake_install.cmake")
  include("/home/riera/Documentos/GitHub/CTSP_scheduler/build-clp/src/main/cmake_install.cmake")

endif()

