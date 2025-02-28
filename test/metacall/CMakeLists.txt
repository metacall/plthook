cmake_minimum_required(VERSION 3.10)

project(MetaCallPoC VERSION 1.0)
set(CMAKE_C_STANDARD 99)

# Debug
set(CMAKE_BUILD_TYPE Debug)
if(NOT WIN32)
    add_compile_options(-fsanitize=address)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=address)
    add_link_options(-fsanitize=undefined)
endif()

# Windows exports
if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# PLTHook
set(PLTHOOK_DIR "${CMAKE_SOURCE_DIR}/../..")
set(PLTHOOK_INCLUDE_DIR "${PLTHOOK_DIR}")
if(APPLE)
    set(PLTHOOK_SOURCE "${PLTHOOK_DIR}/plthook_osx.c")
elseif(WIN32)
    set(PLTHOOK_SOURCE "${PLTHOOK_DIR}/plthook_win32.c")
else()
    set(PLTHOOK_SOURCE "${PLTHOOK_DIR}/plthook_elf.c")
endif()

# MetaCall Library
add_library(metacall SHARED metacall.c ${PLTHOOK_SOURCE})
target_include_directories(metacall PRIVATE ${PLTHOOK_INCLUDE_DIR})
target_link_libraries(metacall ${CMAKE_DL_LIBS})

# NodeJS library of NodeJS static
add_library(libnode-static libnode.c)
target_compile_definitions(libnode-static PUBLIC NODE_STATIC=1) # Add definition for compiling as "node-static"

# NodeJS static
add_executable(node-static node-static.c)
target_link_libraries(node-static libnode-static ${CMAKE_DL_LIBS} metacall)
set_property(TARGET node-static PROPERTY ENABLE_EXPORTS ON) # Required to export the symbols on the executable

# NodeJS library of NodeJS dynamic
add_library(libnode SHARED libnode.c)

# NodeJS dynamic
add_executable(node-dynamic node-dynamic.c)
target_link_libraries(node-dynamic ${CMAKE_DL_LIBS} metacall)

# Normal Executable
add_executable(normal-executable normal-executable.c)
target_link_libraries(normal-executable ${CMAKE_DL_LIBS} metacall)

# NodeJS Loader is linked weakly to libnode2
add_library(node_loader SHARED node_loader.c)
target_link_options(node_loader
    PRIVATE
    # TODO: Add flags for gcc?
    $<$<AND:$<BOOL:${APPLE}>,$<CXX_COMPILER_ID:AppleClang,Clang>>:-Wl,-undefined,dynamic_lookup>
    $<$<CXX_COMPILER_ID:MSVC>:/FORCE:UNRESOLVED>
)

# NodeJS library of NodeJS Loader
add_library(libnode2 SHARED libnode2.c)

# Additional dlinfo test (unrelated to the PoC)
if(NOT WIN32)
    add_executable(dlinfo-test dlinfo-test.c)
    target_link_libraries(dlinfo-test ${CMAKE_DL_LIBS})
endif()
