cmake_minimum_required(VERSION 3.10)

project(MetaCallPoC VERSION 1.0)
set(CMAKE_C_STANDARD 99)

# Debug
set(CMAKE_BUILD_TYPE Debug)
if(NOT WIN32)
    # add_compile_options(-fsanitize=address)
    # add_compile_options(-fsanitize=undefined)
    # add_link_options(-fsanitize=address)
    # add_link_options(-fsanitize=undefined)
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

# NodeJS static
add_executable(node-static node-static.c)
target_link_libraries(node-static metacall)

# NodeJS library of NodeJS dynamic
add_library(libnode SHARED libnode.c)

# NodeJS dynamic
add_executable(node-dynamic node-dynamic.c)
target_link_libraries(node-dynamic metacall)

# Normal Executable
add_executable(normal-executable normal-executable.c)
target_link_libraries(normal-executable metacall)

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
