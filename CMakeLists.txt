cmake_minimum_required(VERSION 3.23)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR "In-source builds are not allowed")
endif()

project(
  presets
  VERSION 2.0.0
  DESCRIPTION "arguments presets"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTING "Build tests" ON)

include(cmake/get_cpm.cmake)
# include(CMakeDependentOption) find_package(vcpkg CONFIG QUIET)

# if(NOT vcpkg_FOUND)
cpmaddpackage(NAME simple_enum GITHUB_REPOSITORY arturbac/simple_enum GIT_TAG
              v0.8.5)
find_package(simple_enum REQUIRED)
cpmaddpackage(NAME glaze GITHUB_REPOSITORY stephenberry/glaze GIT_TAG v4.1.0)
# endif()
find_package(glaze REQUIRED)
add_library(presets)
add_library(presets::presets ALIAS presets)

target_sources(presets PRIVATE src/presets.cc)

# Include directories
target_include_directories(
  presets
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:include>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Link dependencies
target_link_libraries(presets PUBLIC simple_enum::simple_enum)
target_link_libraries(presets PRIVATE  glaze::glaze)

# Installation
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(
  TARGETS presets
  EXPORT presetsTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# install(
#   EXPORT presetsTargets
#   FILE presetsTargets.cmake
#   NAMESPACE presets::
#   DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/presets)

configure_package_config_file(
  cmake/presetsConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/presetsConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/presets)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/presetsConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/presetsConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/presetsConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/presets)

if(BUILD_TESTING)
  enable_testing()
  include(cmake/unit_test_integration.cmake)
  add_subdirectory(tests)
endif()
