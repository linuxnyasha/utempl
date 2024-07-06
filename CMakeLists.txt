cmake_minimum_required(VERSION 3.27)
project(utempl
  VERSION 0.1)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDART 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CXX_EXTENSIONS NO)
set(BOOST_INCLUDE_LIBRARIES "pfr")
option(CPM_USE_LOCAL_PACKAGES "Use local packages" ON)


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

CPMAddPackage("gh:fmtlib/fmt#10.2.1")





cmake_policy(SET CMP0079 NEW)

include(GNUInstallDirs)
add_library(utempl INTERFACE)
target_include_directories(
  utempl
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

if(TARGET Boost::pfr)
  target_link_libraries(utempl INTERFACE fmt::fmt-header-only Boost::pfr)
else()
  find_package(Boost 1.85.0 REQUIRED)
  target_link_libraries(utempl INTERFACE fmt::fmt-header-only ${Boost_LIBRARIES})
endif()
target_compile_features(utempl INTERFACE cxx_std_20)

install(TARGETS utempl
  EXPORT utemplTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
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

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/utempl DESTINATION include)



if(ENABLE_TESTS) 
  find_package(GTest REQUIRED)
  enable_testing()
  file(GLOB SOURCES tests/* tests/*/* tests/*/*/*)
  add_executable(utempl_tests ${SOURCES})
  target_link_libraries(utempl_tests GTest::gtest_main utempl)
  set_property(TARGET utempl_tests PROPERTY CXX_STANDARD 20)
  include(GoogleTest)
  gtest_discover_tests(utempl_tests)
endif()
if(ENABLE_EXAMPLES)
  file(GLOB EXAMPLES_SRC "examples/src/*.cpp")
  foreach(EXAMPLE_SRC ${EXAMPLES_SRC})
      get_filename_component(EXAMPLE_NAME ${EXAMPLE_SRC} NAME_WE)
      add_executable(${EXAMPLE_NAME} ${EXAMPLE_SRC})
      target_link_libraries(${EXAMPLE_NAME} utempl)
      set_property(TARGET ${EXAMPLE_NAME} PROPERTY CXX_STANDARD 20)
      set_target_properties(${EXAMPLE_NAME} PROPERTIES
          RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/examples/")
endforeach()
endif()
