# finalize.exe
add_executable(finalize finalize.cpp misc.cpp)
target_link_libraries(finalize ${OpenCV_LIBS} winmm vfw32 shlwapi)
