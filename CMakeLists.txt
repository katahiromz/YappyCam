# CMakeLists.txt --- CMake project settings
#    ex) cmake -G "Visual Studio 9 2008"
#    ex) cmake -DCMAKE_BUILD_TYPE=Release -G "MSYS Makefiles"
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.0)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# project name and languages
project(YappyCam CXX RC)

# set output directory (build/)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # using Clang
    set(CMAKE_C_FLAGS "")
    set(CMAKE_CXX_FLAGS "")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # using GCC
    set(CMAKE_C_FLAGS "")
    set(CMAKE_CXX_FLAGS "")
elseif (MSVC)
    # replace "/MD" with "/MT" (building without runtime DLLs)
    set(CompilerFlags
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    foreach(CompilerFlags ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlags} "${${CompilerFlags}}")
    endforeach()
endif()

if (MSVC)
    add_definitions(-DMSVC)
endif()

##############################################################################
# OpenCV

if (MSVC)
    set(OpenCV_DIR "C:/opencv/build")
    include_directories("C:/opencv/build/include")
endif()

find_package(OpenCV REQUIRED)

add_definitions(-DUNICODE -D_UNICODE)

##############################################################################

# YappyCam.exe
add_executable(YappyCam WIN32
    YappyCam.cpp
    PluginFramework.cpp
    faces_dlg.cpp
    hotkeys_dlg.cpp
    misc.cpp
    multires_dlg.cpp
    sound.cpp
    sound_dlg.cpp
    pic_dlg.cpp
    plugins_dlg.cpp
    save_to_dlg.cpp
    YappyCam_res.rc)
target_link_libraries(YappyCam
    ${OpenCV_LIBS}
    comctl32
    shlwapi
    winmm
    vfw32
    ole32
    avrt
    ksuser)

# sound2wav.exe
add_executable(sound2wav sound2wav.cpp misc.cpp)
target_link_libraries(sound2wav winmm vfw32)

# finalize.exe
add_executable(finalize finalize.cpp misc.cpp)
target_link_libraries(finalize ${OpenCV_LIBS} winmm vfw32 shlwapi)

# silent.exe
add_executable(silent silent.cpp silent_res.rc)
target_link_libraries(silent winmm)

# tests and plugins
subdirs(tests plugins)

##############################################################################
