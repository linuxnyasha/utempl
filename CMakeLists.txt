cmake_minimum_required(VERSION 3.30)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")

project(utempl
  VERSION 0.1
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BOOST_INCLUDE_LIBRARIES "pfr")

option(CPM_USE_LOCAL_PACKAGES "Use local packages" ON)
option(UTEMPL_MODULE OFF)

if(UTEMPL_MODULE OR ENABLE_EXAMPLES OR ENABLE_TESTS)
  set(CMAKE_CXX_MODULE_STD 1)
endif()

file(GLOB_RECURSE SOURCES "src/*.cpp")

if(TARGET __cmake_cxx23)
  set_target_properties(__cmake_cxx23 PROPERTIES CXX_EXTENSIONS OFF)
endif()

file(
  DOWNLOAD
  https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.0/CPM.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
  EXPECTED_HASH
  SHA256=7b354f3a5976c4626c876850c93944e52c83ec59a159ae5de5be7983f0e17a2a
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

CPMAddPackage(
  NAME Boost
  VERSION 1.85.0
  URL "https://github.com/boostorg/boost/releases/download/boost-1.85.0/boost-1.85.0-cmake.tar.xz"  
  EXCLUDE_FROM_ALL ON
  OPTIONS "BOOST_SKIP_INSTALL_RULES OFF"
)

if(UTEMPL_MODULE)

set(TMP ${CPM_USE_LOCAL_PACKAGES})
set(CPM_USE_LOCAL_PACKAGES OFF)

CPMAddPackage(
  NAME fmt
  URL
  "https://github.com/linuxnyasha/fmt_import/archive/refs/heads/linuxnyasha-patch-1.zip"
  EXCLUDE_FROM_ALL ON
  OPTIONS "FMT_MODULE ON"
  # Fixes for fmt module and constexpr fmt::formatted_size
)

set(CPM_USE_LOCAL_PACKAGES ${TMP})

else()

CPMAddPackage("gh:fmtlib/fmt#11.0.2")


endif()

cmake_policy(SET CMP0079 NEW)

include(GNUInstallDirs)


file(MAKE_DIRECTORY "_headers")
file(MAKE_DIRECTORY "_headers/utempl")
file(MAKE_DIRECTORY "_headers/utempl/loopholes")
if(UTEMPL_MODULE)
  add_library(utempl STATIC)
  target_compile_definitions(utempl PUBLIC UTEMPL_MODULE)
  target_compile_options(utempl PUBLIC "-Wno-pragma-once-outside-header")
  target_sources(utempl PUBLIC FILE_SET utemplModules TYPE CXX_MODULES
  FILES ${SOURCES})
  file(GLOB_RECURSE HEADER_SOURCES "src/*.hpp")
  set(EXISTING_HEADERS)

  foreach(HPP_FILE ${HEADER_SOURCES})
    get_filename_component(REL_PATH ${HPP_FILE} DIRECTORY)
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/src" "" REL_PATH ${REL_PATH})
    get_filename_component(FILE_NAME ${HPP_FILE} NAME_WE)
    set(FILE_NAME "${REL_PATH}/${FILE_NAME}")
    set(HEADER_FILE "_headers/utempl/${FILE_NAME}.hpp")
    file(READ ${HPP_FILE} FILE_CONTENT)
    if(EXISTS ${HEADER_FILE}) 
      file(READ ${HEADER_FILE} HEADER_CONTENT)
      if(NOT FILE_CONTENT STREQUAL HEADER_CONTENT)
        file(WRITE ${HEADER_FILE} "${FILE_CONTENT}")
      endif()
    else()
      file(WRITE ${HEADER_FILE} "${FILE_CONTENT}")
    endif()
    list(APPEND EXISTING_HEADERS "${HEADER_FILE}")
  endforeach()
  file(GLOB HEADER_FILES "_headers/*.hpp")
  foreach(HEADER_FILE ${HEADER_FILES})
    if(NOT HEADER_FILE IN_LIST EXISTING_HEADERS)
      file(REMOVE ${HEADER_FILE})
    endif()
  endforeach()
  if(TARGET Boost::pfr)
    target_link_libraries(utempl PUBLIC fmt::fmt
      Boost::pfr)
  else()
    find_package(Boost 1.85.0 REQUIRED)
    target_link_libraries(utempl PUBLIC fmt::fmt
      ${Boost_LIBRARIES})
  endif()
  target_include_directories(utempl PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/_headers>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

else()
  add_library(utempl INTERFACE)
  file(GLOB_RECURSE HEADER_SOURCES "src/*.hpp")
  set(EXISTING_HEADERS)
  foreach(CPP_FILE ${SOURCES} ${HEADER_SOURCES})
    get_filename_component(REL_PATH ${CPP_FILE} DIRECTORY)
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/src" "" REL_PATH ${REL_PATH})
    get_filename_component(FILE_NAME ${CPP_FILE} NAME_WE)
    set(FILE_NAME "${REL_PATH}/${FILE_NAME}")
    set(HEADER_FILE "_headers/utempl/${FILE_NAME}.hpp")
    file(READ ${CPP_FILE} FILE_CONTENT)
    if(EXISTS ${HEADER_FILE}) 
      file(READ ${HEADER_FILE} HEADER_CONTENT)
      if(NOT FILE_CONTENT STREQUAL HEADER_CONTENT)
        file(WRITE ${HEADER_FILE} "${FILE_CONTENT}")
      endif()
    else()
      file(WRITE ${HEADER_FILE} "${FILE_CONTENT}")
    endif()
    list(APPEND EXISTING_HEADERS "${HEADER_FILE}")

  endforeach()
  file(GLOB HEADER_FILES "_headers/*.hpp")
  foreach(HEADER_FILE ${HEADER_FILES})
    if(NOT HEADER_FILE IN_LIST EXISTING_HEADERS)
      file(REMOVE ${HEADER_FILE})
    endif()
  endforeach()
  if(TARGET Boost::pfr)
    target_link_libraries(utempl INTERFACE fmt::fmt
      Boost::pfr)
  else()
    find_package(Boost 1.85.0 REQUIRED)
    target_link_libraries(utempl INTERFACE fmt::fmt
      ${Boost_LIBRARIES})
  endif()
  target_include_directories(utempl INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/_headers>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
endif()
add_library(utempl::utempl ALIAS utempl)
target_compile_features(utempl INTERFACE cxx_std_23)






install(TARGETS utempl
  EXPORT utemplTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    FILE_SET utemplModules DESTINATION ${CMAKE_INSTALL_LIBDIR}
)





include(CMakePackageConfigHelpers)
write_basic_package_version_file("utemplConfigVersion.cmake"
                                 VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY SameMajorVersion)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/utemplConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/utemplConfig.cmake"
  INSTALL_DESTINATION
  ${CMAKE_INSTALL_DATAROOTDIR}/utempl/cmake)

install(EXPORT utemplTargets
        FILE utemplTargets.cmake
        NAMESPACE utempl::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/utempl/cmake)

install(FILES "${PROJECT_BINARY_DIR}/utemplConfig.cmake"
              "${PROJECT_BINARY_DIR}/utemplConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/utempl/cmake)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/_headers/utempl DESTINATION include)



if(ENABLE_TESTS) 
  CPMAddPackage(
    NAME GTest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  )
  enable_testing()
  file(GLOB SOURCES tests/* tests/*/* tests/*/*/*)
  add_executable(utempl_tests)
  target_sources(utempl_tests PUBLIC ${SOURCES})
  target_link_libraries(utempl_tests GTest::gtest_main utempl::utempl)
  set_property(TARGET utempl_tests PROPERTY CXX_STANDARD 23)
  include(GoogleTest)
  gtest_discover_tests(utempl_tests)
endif()
if(ENABLE_EXAMPLES)
  file(GLOB EXAMPLES_SRC "examples/src/*.cpp")
  foreach(EXAMPLE_SRC ${EXAMPLES_SRC})
      get_filename_component(EXAMPLE_NAME ${EXAMPLE_SRC} NAME_WE)
      add_executable(${EXAMPLE_NAME} ${EXAMPLE_SRC})
      target_link_libraries(${EXAMPLE_NAME} utempl::utempl)
      set_property(TARGET ${EXAMPLE_NAME} PROPERTY CXX_STANDARD 23)
      set_target_properties(${EXAMPLE_NAME} PROPERTIES
          RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/examples/output")
endforeach()
endif()
