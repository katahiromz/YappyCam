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
target_link_libraries(YappyCam ${OpenCV_LIBS} comctl32 shlwapi winmm vfw32 ole32 avrt ksuser)
