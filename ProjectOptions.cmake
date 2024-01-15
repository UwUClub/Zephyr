include(cmake/SystemLink.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)


macro(zephyr_supports_sanitizers)
  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)
    set(SUPPORTS_UBSAN ON)
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    set(SUPPORTS_ASAN ON)
  endif()
endmacro()

macro(zephyr_setup_options)
  option(zephyr_ENABLE_HARDENING "Enable hardening" ON)
  option(zephyr_ENABLE_COVERAGE "Enable coverage reporting" ON)
  cmake_dependent_option(
    zephyr_ENABLE_GLOBAL_HARDENING
    "Attempt to push hardening options to built dependencies"
    ON
    zephyr_ENABLE_HARDENING
    OFF)

  zephyr_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR zephyr_PACKAGING_MAINTAINER_MODE)
    option(zephyr_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(zephyr_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(zephyr_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(zephyr_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(zephyr_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(zephyr_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(zephyr_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(zephyr_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(zephyr_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(zephyr_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(zephyr_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(zephyr_ENABLE_PCH "Enable precompiled headers" OFF)
    option(zephyr_ENABLE_CACHE "Enable ccache" OFF)
  else()
    option(zephyr_ENABLE_IPO "Enable IPO/LTO" ON)
    option(zephyr_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(zephyr_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(zephyr_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
    option(zephyr_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(zephyr_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
    option(zephyr_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(zephyr_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(zephyr_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(zephyr_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
    option(zephyr_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
    option(zephyr_ENABLE_PCH "Enable precompiled headers" OFF)
    option(zephyr_ENABLE_CACHE "Enable ccache" ON)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      zephyr_ENABLE_IPO
      zephyr_WARNINGS_AS_ERRORS
      zephyr_ENABLE_USER_LINKER
      zephyr_ENABLE_SANITIZER_ADDRESS
      zephyr_ENABLE_SANITIZER_LEAK
      zephyr_ENABLE_SANITIZER_UNDEFINED
      zephyr_ENABLE_SANITIZER_THREAD
      zephyr_ENABLE_SANITIZER_MEMORY
      zephyr_ENABLE_UNITY_BUILD
      zephyr_ENABLE_CLANG_TIDY
      zephyr_ENABLE_CPPCHECK
      zephyr_ENABLE_COVERAGE
      zephyr_ENABLE_PCH
      zephyr_ENABLE_CACHE)
  endif()

endmacro()

macro(zephyr_global_options)
  if(zephyr_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    zephyr_enable_ipo()
  endif()

  zephyr_supports_sanitizers()

  if(zephyr_ENABLE_HARDENING AND zephyr_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR zephyr_ENABLE_SANITIZER_UNDEFINED
       OR zephyr_ENABLE_SANITIZER_ADDRESS
       OR zephyr_ENABLE_SANITIZER_THREAD
       OR zephyr_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    message("${zephyr_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${zephyr_ENABLE_SANITIZER_UNDEFINED}")
    zephyr_enable_hardening(zephyr_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()
endmacro()

macro(zephyr_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(zephyr_warnings INTERFACE)
  add_library(zephyr_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  zephyr_set_project_warnings(
    zephyr_warnings
    ${zephyr_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  if(zephyr_ENABLE_USER_LINKER)
    include(cmake/Linker.cmake)
    configure_linker(zephyr_options)
  endif()

  include(cmake/Sanitizers.cmake)
  zephyr_enable_sanitizers(
    zephyr_options
    ${zephyr_ENABLE_SANITIZER_ADDRESS}
    ${zephyr_ENABLE_SANITIZER_LEAK}
    ${zephyr_ENABLE_SANITIZER_UNDEFINED}
    ${zephyr_ENABLE_SANITIZER_THREAD}
    ${zephyr_ENABLE_SANITIZER_MEMORY})

  set_target_properties(zephyr_options PROPERTIES UNITY_BUILD ${zephyr_ENABLE_UNITY_BUILD})

  if(zephyr_ENABLE_PCH)
    target_precompile_headers(
      zephyr_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(zephyr_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    zephyr_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(zephyr_ENABLE_CLANG_TIDY)
    zephyr_enable_clang_tidy(zephyr_options ${zephyr_WARNINGS_AS_ERRORS})
  endif()

  if(zephyr_ENABLE_CPPCHECK)
    zephyr_enable_cppcheck(${zephyr_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(zephyr_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    zephyr_enable_coverage(zephyr_options)
  endif()

  if(zephyr_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(zephyr_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

  if(zephyr_ENABLE_HARDENING AND NOT zephyr_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR zephyr_ENABLE_SANITIZER_UNDEFINED
       OR zephyr_ENABLE_SANITIZER_ADDRESS
       OR zephyr_ENABLE_SANITIZER_THREAD
       OR zephyr_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    zephyr_enable_hardening(zephyr_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()

endmacro()
