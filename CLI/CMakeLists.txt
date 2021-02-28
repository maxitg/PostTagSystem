option(POST_TAG_SYSTEM_CLI_STATIC_BUILD "Build command-line interface as a statically linked executable." OFF)

if(POST_TAG_SYSTEM_CLI_STATIC_BUILD)
  set(Boost_USE_STATIC_LIBS ON)
endif()

find_package(Boost 1.67
  COMPONENTS program_options
  REQUIRED)

set(_link_libraries
  PostTagSystem
  ${Boost_LIBRARIES}
  )

add_executable(wolfram-postproject
  main.cpp arguments.cpp
  files/PostTagFileReader.cpp files/PostTagFileWriter.cpp
  files/PostTagCribFile.cpp files/PostTagResultFile.cpp
  )

target_include_directories(wolfram-postproject PRIVATE
  ${Boost_INCLUDE_DIR})

target_compile_options(wolfram-postproject PRIVATE
  ${POST_TAG_SYSTEM_COMPILE_OPTIONS})

target_compile_definitions(wolfram-postproject PRIVATE
  POST_TAG_VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR}
  POST_TAG_VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR}
  POST_TAG_VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH}
  )

if(POST_TAG_SYSTEM_CLI_STATIC_BUILD)
  target_link_options(wolfram-postproject PUBLIC "-static")
endif()

target_link_libraries(wolfram-postproject ${_link_libraries})
