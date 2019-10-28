# CMakeLists.txt --- CMake project settings
#    ex) cmake -G "Visual Studio 9 2008"
#    ex) cmake -DCMAKE_BUILD_TYPE=Release -G "MSYS Makefiles"
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# project name and language
project(YappyCam CXX)

# enable Win32 resource
enable_language(RC)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # using Clang
    set(CMAKE_C_FLAGS "-static")
    set(CMAKE_CXX_FLAGS "-static")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # using GCC
    set(CMAKE_C_FLAGS "-static")
    set(CMAKE_CXX_FLAGS "-static")
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

##############################################################################

add_definitions(-DUNICODE -D_UNICODE)

# YappyCam.exe
add_executable(YappyCam WIN32
    YappyCam.cpp
    PluginFramework.cpp
    hotkeys_dlg.cpp
    misc.cpp
    multires_dlg.cpp
    sound.cpp
    sound_dlg.cpp
    pic_dlg.cpp
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

# tests
subdirs(tests)

##############################################################################

# Plugin1.yap
add_library(Plugin1 SHARED Plugin1.cpp Plugin1.def)
target_compile_definitions(Plugin1 PRIVATE -DPLUGIN_BUILD)
set_target_properties(Plugin1 PROPERTIES OUTPUT_NAME "Plugin1.yap")
set_target_properties(Plugin1 PROPERTIES PREFIX "")
set_target_properties(Plugin1 PROPERTIES SUFFIX "")
target_link_libraries(Plugin1 ${OpenCV_LIBS})
