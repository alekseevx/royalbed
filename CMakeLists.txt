cmake_minimum_required(VERSION 3.11)

include(cmake/CMakeRC.cmake)
include(cmake/enable_warnings.cmake)
include(cmake/sanitizer.cmake)
add_compile_options(-Wall -Wextra -Wpedantic)

include(cmake/bastard_setup.cmake)
bastard_setup()
project(${BASTARD_PACKAGE_NAME})

option(ROYALBED_COVERAGE_ENABLED "enable coverage compiler flags" OFF)

option(ROYALBED_ADDRESS_SANITIZER_ENABLED "enable address sanitizer" OFF)
option(ROYALBED_THREAD_SANITIZER_ENABLED "enable thread sanitizer" OFF)
option(ROYALBED_MEMORY_SANITIZER_ENABLED "enable memory sanitizer" OFF)

cmrc_add_resource_library(swaggerFiles
    NAMESPACE royalbed::swagger
    swagger/index.html
    # swagger-ui v4.6.1
    swagger/ui/favicon-16x16.png
    swagger/ui/favicon-32x32.png
    swagger/ui/swagger-ui-bundle.js.gz
    swagger/ui/swagger-ui.css.gz
    swagger/ui/swagger-ui-standalone-preset.js.gz
)
target_compile_features(swaggerFiles PRIVATE cxx_std_17)
target_link_libraries(${BASTARD_PACKAGE_NAME} swaggerFiles)

cmrc_add_resource_library(redocFiles
    NAMESPACE royalbed::redoc
    redoc/index.html
    # redoc 2.0.0-rc.64
    redoc/redoc.standalone.js.gz
)
target_compile_features(redocFiles PRIVATE cxx_std_17)
target_link_libraries(${BASTARD_PACKAGE_NAME} redocFiles)

if(ROYALBED_THREAD_SANITIZER_ENABLED)
    enable_thread_sanitizer(
        blacklist ${CMAKE_CURRENT_LIST_DIR}/sanitize-blacklist)
endif()

if(ROYALBED_COVERAGE_ENABLED)
    message(STATUS "royalbed enabling coverage")
    include(cmake/CodeCoverage.cmake)
    append_coverage_compiler_flags()
    setup_target_for_coverage_gcovr_html(
          NAME royal_bed_coverage
          EXECUTABLE ./tests --gtest_output=xml:junit-report.xml --gtest_print_file_info
          EXCLUDE ".deps" "tests" "build" "examples" "lib/3rdparty/*"
          )
endif()

if(ROYALBED_ADDRESS_SANITIZER_ENABLED)
    enable_address_sanitizer(TARGET ${BASTARD_PACKAGE_NAME})
endif()

if(ROYALBED_MEMORY_SANITIZER_ENABLED)
    enable_memory_sanitizer(
        TARGET ${BASTARD_PACKAGE_NAME} 
        blacklist ${CMAKE_CURRENT_LIST_DIR}/sanitize-blacklist)
endif()

if (WIN32)
    target_compile_definitions(${BASTARD_PACKAGE_NAME} PUBLIC -D_WIN32_WINNT=0x0601)
endif()

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    add_subdirectory(examples)
endif()

# EnableWarnings(${BASTARD_PACKAGE_NAME})
