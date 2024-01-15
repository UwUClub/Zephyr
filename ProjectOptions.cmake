include(cmake/SystemLink.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)


macro(patatocs_supports_sanitizers)
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

macro(patatocs_setup_options)
  option(patatocs_ENABLE_HARDENING "Enable hardening" ON)
  option(patatocs_ENABLE_COVERAGE "Enable coverage reporting" ON)
  cmake_dependent_option(
    patatocs_ENABLE_GLOBAL_HARDENING
    "Attempt to push hardening options to built dependencies"
    ON
    patatocs_ENABLE_HARDENING
    OFF)

  patatocs_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR patatocs_PACKAGING_MAINTAINER_MODE)
    option(patatocs_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(patatocs_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(patatocs_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(patatocs_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(patatocs_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(patatocs_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(patatocs_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(patatocs_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(patatocs_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(patatocs_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(patatocs_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(patatocs_ENABLE_PCH "Enable precompiled headers" OFF)
    option(patatocs_ENABLE_CACHE "Enable ccache" OFF)
  else()
    option(patatocs_ENABLE_IPO "Enable IPO/LTO" ON)
    option(patatocs_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(patatocs_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(patatocs_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
    option(patatocs_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(patatocs_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
    option(patatocs_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(patatocs_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(patatocs_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(patatocs_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
    option(patatocs_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
    option(patatocs_ENABLE_PCH "Enable precompiled headers" OFF)
    option(patatocs_ENABLE_CACHE "Enable ccache" ON)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      patatocs_ENABLE_IPO
      patatocs_WARNINGS_AS_ERRORS
      patatocs_ENABLE_USER_LINKER
      patatocs_ENABLE_SANITIZER_ADDRESS
      patatocs_ENABLE_SANITIZER_LEAK
      patatocs_ENABLE_SANITIZER_UNDEFINED
      patatocs_ENABLE_SANITIZER_THREAD
      patatocs_ENABLE_SANITIZER_MEMORY
      patatocs_ENABLE_UNITY_BUILD
      patatocs_ENABLE_CLANG_TIDY
      patatocs_ENABLE_CPPCHECK
      patatocs_ENABLE_COVERAGE
      patatocs_ENABLE_PCH
      patatocs_ENABLE_CACHE)
  endif()

endmacro()

macro(patatocs_global_options)
  if(patatocs_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    patatocs_enable_ipo()
  endif()

  patatocs_supports_sanitizers()

  if(patatocs_ENABLE_HARDENING AND patatocs_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR patatocs_ENABLE_SANITIZER_UNDEFINED
       OR patatocs_ENABLE_SANITIZER_ADDRESS
       OR patatocs_ENABLE_SANITIZER_THREAD
       OR patatocs_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    message("${patatocs_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${patatocs_ENABLE_SANITIZER_UNDEFINED}")
    patatocs_enable_hardening(patatocs_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()
endmacro()

macro(patatocs_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(patatocs_warnings INTERFACE)
  add_library(patatocs_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  patatocs_set_project_warnings(
    patatocs_warnings
    ${patatocs_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  if(patatocs_ENABLE_USER_LINKER)
    include(cmake/Linker.cmake)
    configure_linker(patatocs_options)
  endif()

  include(cmake/Sanitizers.cmake)
  patatocs_enable_sanitizers(
    patatocs_options
    ${patatocs_ENABLE_SANITIZER_ADDRESS}
    ${patatocs_ENABLE_SANITIZER_LEAK}
    ${patatocs_ENABLE_SANITIZER_UNDEFINED}
    ${patatocs_ENABLE_SANITIZER_THREAD}
    ${patatocs_ENABLE_SANITIZER_MEMORY})

  set_target_properties(patatocs_options PROPERTIES UNITY_BUILD ${patatocs_ENABLE_UNITY_BUILD})

  if(patatocs_ENABLE_PCH)
    target_precompile_headers(
      patatocs_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(patatocs_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    patatocs_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(patatocs_ENABLE_CLANG_TIDY)
    patatocs_enable_clang_tidy(patatocs_options ${patatocs_WARNINGS_AS_ERRORS})
  endif()

  if(patatocs_ENABLE_CPPCHECK)
    patatocs_enable_cppcheck(${patatocs_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(patatocs_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    patatocs_enable_coverage(patatocs_options)
  endif()

  if(patatocs_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(patatocs_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

  if(patatocs_ENABLE_HARDENING AND NOT patatocs_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR patatocs_ENABLE_SANITIZER_UNDEFINED
       OR patatocs_ENABLE_SANITIZER_ADDRESS
       OR patatocs_ENABLE_SANITIZER_THREAD
       OR patatocs_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    patatocs_enable_hardening(patatocs_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()

endmacro()
