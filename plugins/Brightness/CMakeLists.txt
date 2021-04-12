# Brightness.yap
add_library(Brightness SHARED Brightness_yap.cpp Brightness_yap.def Brightness_yap_res.rc)
set_target_properties(Brightness PROPERTIES OUTPUT_NAME "Brightness.yap")
set_target_properties(Brightness PROPERTIES PREFIX "")
set_target_properties(Brightness PROPERTIES SUFFIX "")
target_link_libraries(Brightness ${OpenCV_LIBS})
