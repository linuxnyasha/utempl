cmake_minimum_required(VERSION 3.30)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")

project(utempl
  VERSION 0.1
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_MODULE_STD 1)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(FMT_MODULE ON)
set(BOOST_INCLUDE_LIBRARIES "pfr")

option(CPM_USE_LOCAL_PACKAGES "Use local packages" ON)
option(BUILD_SHARED_LIBS "Build shared libraries (.so)" ON)
option(BUILD_STATIC_LIBS "Build static libraries (.a)" OFF)
file(GLOB_RECURSE SOURCES "src/*.cpp")

set_target_properties(__cmake_cxx23 PROPERTIES CXX_EXTENSIONS OFF)


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

set(TMP ${CPM_USE_LOCAL_PACKAGES})
set(CPM_USE_LOCAL_PACKAGES OFF)

CPMAddPackage(
  NAME fmt
  URL
  "https://github.com/linuxnyasha/fmt_import/archive/refs/heads/linuxnyasha-patch-1.zip"
  # Fixes for fmt module and constexpr fmt::formatted_size
  OPTIONS "CMAKE_CXX_FLAGS -DFMT_ATTACH_TO_GLOBAL_MODULE"
)

set(CPM_USE_LOCAL_PACKAGES ${TMP})

function(configure target)
  target_compile_features(${target} INTERFACE cxx_std_23)

  target_include_directories(${target} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

  target_sources(${target} PUBLIC FILE_SET ${target} TYPE CXX_MODULES
    FILES ${SOURCES})



  install(TARGETS ${target}
    EXPORT utemplTargets
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      FILE_SET ${target} DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )


  if(TARGET Boost::pfr)
    target_link_libraries(${target} PUBLIC fmt::fmt
      Boost::pfr)
  else()
    find_package(Boost 1.85.0 REQUIRED)
    target_link_libraries(${target} PUBLIC fmt::fmt
      ${Boost_LIBRARIES})
  endif()
endfunction(configure)



cmake_policy(SET CMP0079 NEW)

include(GNUInstallDirs)

if(BUILD_SHARED_LIBS)
  add_library(utempl_shared)
  add_library(utempl::utempl ALIAS utempl_shared)
  configure(utempl_shared)
endif()

if(BUILD_STATIC_LIBS)
  add_library(utempl_static)
  if(NOT BUILD_SHARED_LIBS)
    add_library(utempl ALIAS utempl_static)
    add_library(utempl::utempl ALIAS utempl_static)
  endif()
  configure(utempl_static)
endif()



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

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/utempl DESTINATION include)



if(ENABLE_TESTS) 
  find_package(GTest REQUIRED)
  enable_testing()
  file(GLOB SOURCES tests/* tests/*/* tests/*/*/*)
  add_executable(utempl_tests)
  target_sources(utempl_tests PUBLIC FILE_SET utempl_tests TYPE CXX_MODULES
    FILES ${SOURCES})
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
