# Install script for directory: /Users/juliechien/Dropbox/Julie/Columbia/CS6901 Nayar Lab/dlib/examples/cpr/opt/curl

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
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
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

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE FILES "/Users/juliechien/Dropbox/Julie/Columbia/CS6901 Nayar Lab/dlib/examples/buildcpr/cpr/opt/curl/curl-config")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/juliechien/Dropbox/Julie/Columbia/CS6901 Nayar Lab/dlib/examples/buildcpr/cpr/opt/curl/libcurl.pc")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/curl" TYPE FILE FILES "/Users/juliechien/Dropbox/Julie/Columbia/CS6901 Nayar Lab/dlib/examples/buildcpr/cpr/opt/curl/include/curl/curlbuild.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE DIRECTORY FILES "/Users/juliechien/Dropbox/Julie/Columbia/CS6901 Nayar Lab/dlib/examples/cpr/opt/curl/include/curl" FILES_MATCHING REGEX "/[^/]*\\.h$" REGEX "/curlbuild\\.h$" EXCLUDE)
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/juliechien/Dropbox/Julie/Columbia/CS6901 Nayar Lab/dlib/examples/buildcpr/cpr/opt/curl/lib/cmake_install.cmake")
  include("/Users/juliechien/Dropbox/Julie/Columbia/CS6901 Nayar Lab/dlib/examples/buildcpr/cpr/opt/curl/tests/cmake_install.cmake")

endif()

