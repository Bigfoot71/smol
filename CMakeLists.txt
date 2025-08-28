cmake_minimum_required(VERSION 3.8)
project(smol VERSION 0.1.0 LANGUAGES C)

# Used standard declaration

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Hot reload for MSVC compilers if supported

if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Determine the project's root directory and check if it's the main project

set(SL_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(SL_IS_MAIN ON)
else()
    set(SL_IS_MAIN OFF)
endif()

# Smol build config options

option(SL_BUILD_SHARED_LIBS "Build Smol as a shared library" OFF)
option(SL_BUILD_EXAMPLES "Enable building examples" ${SL_IS_MAIN})
option(SL_BUILD_DOCS "Build documentation with Doxygen" ${SL_IS_MAIN})
option(SL_INSTALL "Enable installation of the Smol library" ${SL_IS_MAIN})

# Shared build setup

if(SL_BUILD_SHARED_LIBS)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    add_definitions(-DSL_BUILD_SHARED)
endif()

# Set the cmake module path

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include standard cmake scripts

include(CheckLibraryExists)

# Global configuration

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Configure SDL

set(SDL_STATIC          ON CACHE BOOL "")

set(SDL_SHARED          OFF CACHE BOOL "")
set(SDL_INSTALL         OFF CACHE BOOL "")
set(SDL_EXAMPLES        OFF CACHE BOOL "")
set(SDL_TEST_LIBRARY    OFF CACHE BOOL "")

set(SDL_VIDEO           ON CACHE BOOL "")
set(SDL_AUDIO           ON CACHE BOOL "")
set(SDL_JOYSTICK        ON CACHE BOOL "")
set(SDL_OPENGLES        ON CACHE BOOL "")
set(SDL_OPENGL          ON CACHE BOOL "")

set(SDL_DIALOG          OFF CACHE BOOL "")
set(SDL_RENDER          OFF CACHE BOOL "")
set(SDL_CAMERA          OFF CACHE BOOL "")
set(SDL_HAPTIC          OFF CACHE BOOL "")
set(SDL_POWER           OFF CACHE BOOL "")
set(SDL_SENSOR          OFF CACHE BOOL "")
set(SDL_GPU             OFF CACHE BOOL "")
set(SDL_VULKAN          OFF CACHE BOOL "")
set(SDL_DIRECTX         OFF CACHE BOOL "")
set(SDL_OFFSCREEN       OFF CACHE BOOL "")

add_subdirectory("${SL_ROOT_PATH}/external/SDL")

# List all the smol sources

set(SL_SOURCES
    "${SL_ROOT_PATH}/src/internal/sl__render.c"
    "${SL_ROOT_PATH}/src/internal/sl__audio.c"
    "${SL_ROOT_PATH}/src/internal/sl__core.c"
    "${SL_ROOT_PATH}/src/sl_codepoint.c"
    "${SL_ROOT_PATH}/src/sl_texture.c"
    "${SL_ROOT_PATH}/src/sl_shader.c"
    "${SL_ROOT_PATH}/src/sl_render.c"
    "${SL_ROOT_PATH}/src/sl_canvas.c"
    "${SL_ROOT_PATH}/src/sl_audio.c"
    "${SL_ROOT_PATH}/src/sl_sound.c"
    "${SL_ROOT_PATH}/src/sl_music.c"
    "${SL_ROOT_PATH}/src/sl_image.c"
    "${SL_ROOT_PATH}/src/sl_mesh.c"
    "${SL_ROOT_PATH}/src/sl_math.c"
    "${SL_ROOT_PATH}/src/sl_font.c"
    "${SL_ROOT_PATH}/src/sl_text.c"
    "${SL_ROOT_PATH}/src/sl_rand.c"
    "${SL_ROOT_PATH}/src/sl_core.c"
)

# Create the Smol library target

if(SL_BUILD_SHARED_LIBS)
    add_library("${PROJECT_NAME}" SHARED ${SL_SOURCES})
else()
    add_library("${PROJECT_NAME}" STATIC ${SL_SOURCES})
endif()

# Plateform configuration

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SL_PLATFORM_WINDOWS)
elseif(APPLE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SL_PLATFORM_MACOS)
elseif(ANDROID)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SL_PLATFORM_ANDROID)
elseif(EMSCRIPTEN)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SL_PLATFORM_WEB)
elseif(UNIX)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SL_PLATFORM_LINUX)
endif()

# Adding SDL3 to Smol

target_sources(${PROJECT_NAME} PRIVATE $<TARGET_OBJECTS:SDL3-static>)

# Adding Glad to Smol

target_sources(${PROJECT_NAME} PRIVATE external/glad/src/gles2.c)
target_include_directories(${PROJECT_NAME} PRIVATE external/glad/include)

# Adding mojoAL to Smol

add_definitions(-DAL_LIBTYPE_STATIC)

target_sources(${PROJECT_NAME}
    PRIVATE
        "${SL_ROOT_PATH}/external/mojoAL/mojoal.c"
)

# Specify the include directories

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${SL_ROOT_PATH}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        "${SL_ROOT_PATH}/external/SDL/include"
        "${SL_ROOT_PATH}/external/mojoAL/AL"
        "${SL_ROOT_PATH}/external/dr_libs"
        "${SL_ROOT_PATH}/external/stb"
)

# Link project dependencies

check_library_exists(m cos "" SL_HAVE_LIB_M)

if(SL_HAVE_LIB_M)
    target_link_libraries(${PROJECT_NAME} PUBLIC m)
endif()

# Build documentation

if(SL_BUILD_DOCS)
    find_package(Doxygen REQUIRED)

    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()

# Install configuration

if(SL_INSTALL)

    include(GNUInstallDirs)

    # Installing the library (static or shared)
    install(TARGETS ${PROJECT_NAME}
        EXPORT SmolTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    # Installing public headers
    install(DIRECTORY "${SL_ROOT_PATH}/include/"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h"
    )

    # Export for find_package
    install(EXPORT SmolTargets
        FILE SmolTargets.cmake
        NAMESPACE smol::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/smol
    )

    # Minimal config for find_package
    include(CMakePackageConfigHelpers)
    
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/SmolConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )

    configure_package_config_file(
        "${SL_ROOT_PATH}/cmake/SmolConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/SmolConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/smol
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/SmolConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/SmolConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/smol
    )

endif()

# Example configuration

if(SL_BUILD_EXAMPLES)
    include("${SL_ROOT_PATH}/examples/CMakeLists.txt")
endif()
