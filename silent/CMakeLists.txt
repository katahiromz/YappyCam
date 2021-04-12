# silent.exe
add_executable(silent silent.cpp silent_res.rc)
target_link_libraries(silent winmm)
