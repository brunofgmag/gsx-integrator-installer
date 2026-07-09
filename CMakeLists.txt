cmake_minimum_required(VERSION 3.21)

project(gsx-integrator-installer LANGUAGES CXX)

include(cmake/ProjectOptions.cmake)
include(cmake/Dependencies.cmake)
include(cmake/Sources.cmake)
include(cmake/AppTarget.cmake)

if (BUILD_TESTING)
    include(cmake/Tests.cmake)
endif ()

include(cmake/Deployment.cmake)
