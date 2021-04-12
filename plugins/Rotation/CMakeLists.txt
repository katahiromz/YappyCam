# Rotation.yap
add_library(Rotation SHARED Rotation_yap.cpp Rotation_yap.def Rotation_yap_res.rc)
set_target_properties(Rotation PROPERTIES OUTPUT_NAME "Rotation.yap")
set_target_properties(Rotation PROPERTIES PREFIX "")
set_target_properties(Rotation PROPERTIES SUFFIX "")
target_link_libraries(Rotation ${OpenCV_LIBS})
