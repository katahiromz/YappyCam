# GaussianBlur.yap
add_library(GaussianBlur SHARED GaussianBlur_yap.cpp GaussianBlur_yap.def GaussianBlur_yap_res.rc)
set_target_properties(GaussianBlur PROPERTIES OUTPUT_NAME "GaussianBlur.yap")
set_target_properties(GaussianBlur PROPERTIES PREFIX "")
set_target_properties(GaussianBlur PROPERTIES SUFFIX "")
target_link_libraries(GaussianBlur ${OpenCV_LIBS})
