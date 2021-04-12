# Sepia.yap
add_library(Sepia SHARED Sepia_yap.cpp Sepia_yap.def Sepia_yap_res.rc)
set_target_properties(Sepia PROPERTIES OUTPUT_NAME "Sepia.yap")
set_target_properties(Sepia PROPERTIES PREFIX "")
set_target_properties(Sepia PROPERTIES SUFFIX "")
target_link_libraries(Sepia ${OpenCV_LIBS})
