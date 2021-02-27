find_package(Boost 1.67
  COMPONENTS program_options
  REQUIRED)

set(_link_libraries
  PostTagSystem
  ${Boost_LIBRARIES}
  )

add_executable(wolfram-postproject
  main.cpp arguments.cpp
  files/PostTagFileReader.cpp files/PostTagCribFile.cpp)

target_include_directories(wolfram-postproject
  PRIVATE ${Boost_INCLUDE_DIR})

target_compile_options(wolfram-postproject
  PRIVATE ${POST_TAG_SYSTEM_COMPILE_OPTIONS})

target_link_libraries(wolfram-postproject ${_link_libraries})
