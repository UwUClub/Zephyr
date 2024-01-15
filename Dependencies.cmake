include(cmake/CPM.cmake)

# Done as a function so that updates to variables like
# CMAKE_CXX_FLAGS don't propagate out to other
# targets

find_package(Catch2 QUIET)
find_package(spdlog QUIET)
find_package(Boost QUIET)

message(STATUS "Catch2_FOUND: ${Catch2_FOUND}")
message(STATUS "spdlog_FOUND: ${spdlog_FOUND}")
message(STATUS "Boost_FOUND: ${Boost_FOUND}")

function(zephyr_setup_dependencies)

    # For each dependency, see if it's
    # already been provided to us by a parent project

    if(NOT TARGET Catch2::Catch2WithMain AND NOT Catch2_FOUND)
        cpmaddpackage("gh:catchorg/Catch2@3.3.2")
    endif()

    if(NOT TARGET spdlog::spdlog AND NOT spdlog_FOUND)
        cpmaddpackage("gh:gabime/spdlog@1.12.0")
    endif()

    if(NOT TARGET Boost::boost AND NOT Boost_FOUND)
        set(BOOST_VERSION "1.84.0")
        set(BOOST_LIBS system serialization align assert config core static_assert throw_exception array bind chrono integer move mpl predef asio ratio type_traits typeof utility coroutine date_time function regex smart_ptr preprocessor io uuid)
        set(BOOST_INCLUDE_LIBRARIES ${BOOST_LIBS})

        FetchContent_Declare(
            Boost
            URL https://github.com/boostorg/boost/releases/download/boost-${BOOST_VERSION}/boost-${BOOST_VERSION}.7z
            USES_TERMINAL_DOWNLOAD TRUE
            GIT_PROGRESS TRUE
            DOWNLOAD_NO_EXTRACT FALSE
        )
        set(Boost_USE_STATIC_LIBS ON)
        set(Boost_USE_MULTITHREADED ON)
        set(Boost_USE_STATIC_RUNTIME OFF)
        FetchContent_MakeAvailable(Boost)
    endif()

endfunction()
