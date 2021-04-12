# ColorInvert.yap
add_library(ColorInvert SHARED ColorInvert_yap.cpp ColorInvert_yap.def ColorInvert_yap_res.rc)
set_target_properties(ColorInvert PROPERTIES OUTPUT_NAME "ColorInvert.yap")
set_target_properties(ColorInvert PROPERTIES PREFIX "")
set_target_properties(ColorInvert PROPERTIES SUFFIX "")
target_link_libraries(ColorInvert ${OpenCV_LIBS})
