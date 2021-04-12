# sound2wav.exe
add_executable(sound2wav sound2wav.cpp misc.cpp)
target_link_libraries(sound2wav winmm vfw32)
