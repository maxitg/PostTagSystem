cmake_minimum_required(VERSION 3.13)
project(PostTagSystem
    VERSION 0.3.0
    LANGUAGES CXX)
message(STATUS "${PROJECT_NAME} version: ${PROJECT_VERSION}")

option(POST_TAG_SYSTEM_BUILD_TESTING "Enable cpp testing." OFF)
include(GNUInstallDirs) # Define CMAKE_INSTALL_xxx: LIBDIR, INCLUDEDIR
set(PostTagSystem_export_file "${PROJECT_BINARY_DIR}/PostTagSystemTargets.cmake")

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
        STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY
        STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
    message(STATUS "${PROJECT_NAME} using CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
endif()

# Set extra compile options
option(POST_TAG_SYSTEM_ENABLE_ALLWARNINGS
       "Add all compiler warnings to the C++ targets"
       OFF)
if(MSVC)
  list(APPEND POST_TAG_SYSTEM_DEFAULT_COMPILE_OPTIONS /EHsc)
  if(POST_TAG_SYSTEM_ENABLE_ALLWARNINGS)
    list(APPEND POST_TAG_SYSTEM_DEFAULT_COMPILE_OPTIONS /W4 /WX)
  endif()
else()
  if(POST_TAG_SYSTEM_ENABLE_ALLWARNINGS)
    list(APPEND POST_TAG_SYSTEM_DEFAULT_COMPILE_OPTIONS
         -Wall -Wextra -Werror -pedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2
         -Winit-self -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow
         -Wsign-promo -Wswitch-default -Wundef -Wno-unused)
  endif()
endif()

set(POST_TAG_SYSTEM_EXTRA_COMPILE_OPTIONS CACHE STRING "Add extra compile options to the C++ targets")
set(POST_TAG_SYSTEM_COMPILE_OPTIONS_OVERRIDE CACHE STRING "Overwrite compile options for the C++ targets")
if(POST_TAG_SYSTEM_COMPILE_OPTIONS_OVERRIDE)
  set(POST_TAG_SYSTEM_COMPILE_OPTIONS ${POST_TAG_SYSTEM_COMPILE_OPTIONS_OVERRIDE})
else()
  set(POST_TAG_SYSTEM_COMPILE_OPTIONS
      ${POST_TAG_SYSTEM_DEFAULT_COMPILE_OPTIONS}
      ${POST_TAG_SYSTEM_EXTRA_COMPILE_OPTIONS})
endif()

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

message(STATUS "POST_TAG_SYSTEM_BUILD_TESTING: ${POST_TAG_SYSTEM_BUILD_TESTING}")
message(STATUS "POST_TAG_SYSTEM_COMPILE_OPTIONS: ${POST_TAG_SYSTEM_COMPILE_OPTIONS}")

set(libPostTagSystem_headers
    WolframLanguageAPI.hpp
    PostTagState.hpp
    PostTagMultihistory.hpp
    PostTagHistory.hpp
    )
set(libPostTagSystem_sources
    WolframLanguageAPI.cpp
    PostTagMultihistory.cpp
    PostTagHistory.cpp
    )
list(TRANSFORM libPostTagSystem_headers PREPEND "libPostTagSystem/")
list(TRANSFORM libPostTagSystem_sources PREPEND "libPostTagSystem/")

add_library(PostTagSystem ${libPostTagSystem_sources})
target_include_directories(PostTagSystem PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libPostTagSystem>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
target_compile_options(PostTagSystem PRIVATE ${POST_TAG_SYSTEM_COMPILE_OPTIONS})

set(POST_TAG_SYSTEM_LIBRARIES PostTagSystem)

install(TARGETS ${POST_TAG_SYSTEM_LIBRARIES}
        EXPORT PostTagSystemTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT development
        )

install(FILES ${libPostTagSystem_headers}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PostTagSystem
  COMPONENT development
  )

# export to the build tree
export( TARGETS ${POST_TAG_SYSTEM_LIBRARIES}
  NAMESPACE PostTagSystem::
  APPEND FILE ${PostTagSystem_export_file})

if(POST_TAG_SYSTEM_BUILD_TESTING)
  target_compile_definitions(PostTagSystem PUBLIC LIBPOSTTAGSYSTEM_BUILD_TESTING)

  enable_testing()
  set(INSTALL_GTEST OFF)
  set(CMAKE_POLICY_DEFAULT_CMP0077 NEW) # Propagate INSTALL_GTEST=OFF to subproject
  set(GTEST_LIBRARIES gtest gtest_main)
  include(GoogleTest)
  #############################################################################
  # Fetch GTest
  include(FetchContent)

  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.10.x
  )

  set(CMAKE_POLICY_DEFAULT_CMP0048 NEW) # google test raises warning about it
  FetchContent_GetProperties(googletest)
  if(NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
  endif()
  #############################################################################

  add_subdirectory(libPostTagSystem/test)
endif()

# INSTALL
set(install_cmake_dir "${CMAKE_INSTALL_LIBDIR}/cmake/PostTagSystem")

install (EXPORT PostTagSystemTargets
  NAMESPACE PostTagSystem::
  DESTINATION ${install_cmake_dir} )

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/PostTagSystemConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/PostTagSystemConfigVersion.cmake
  DESTINATION ${install_cmake_dir} )

include(CMakePackageConfigHelpers)

write_basic_package_version_file(PostTagSystemConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

# Build tree
set(PostTagSystem_TARGETS_FILE ${PostTagSystem_export_file})
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/PostTagSystemConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/PostTagSystemConfig.cmake
  INSTALL_DESTINATION ${install_cmake_dir}
  PATH_VARS PostTagSystem_TARGETS_FILE
  NO_CHECK_REQUIRED_COMPONENTS_MACRO # PostTagSystem does not provide components
  INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}
  )

# Install tree
set(PostTagSystem_TARGETS_FILE ${CMAKE_INSTALL_PREFIX}/${install_cmake_dir}/PostTagSystemTargets.cmake)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/PostTagSystemConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/PostTagSystemConfig.cmake
  INSTALL_DESTINATION ${install_cmake_dir}
  PATH_VARS PostTagSystem_TARGETS_FILE
  NO_CHECK_REQUIRED_COMPONENTS_MACRO # PostTagSystem does not provide components
  )

# Add custom target to only install component: runtime (libraries)
add_custom_target(set-replace-install-runtime
  ${CMAKE_COMMAND}
  -DCMAKE_INSTALL_COMPONENT=runtime
  -P "${PROJECT_BINARY_DIR}/cmake_install.cmake"
  DEPENDS ${POST_TAG_SYSTEM_LIBRARIES}
  )
message(STATUS "POST_TAG_SYSTEM_LIBRARIES: ${POST_TAG_SYSTEM_LIBRARIES}")
add_dependencies(set-replace-install-runtime ${POST_TAG_SYSTEM_LIBRARIES})
