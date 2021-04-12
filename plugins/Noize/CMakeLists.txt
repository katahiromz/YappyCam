# Noize.yap
add_library(Noize SHARED Noize_yap.cpp Noize_yap.def Noize_yap_res.rc)
set_target_properties(Noize PROPERTIES OUTPUT_NAME "Noize.yap")
set_target_properties(Noize PROPERTIES PREFIX "")
set_target_properties(Noize PROPERTIES SUFFIX "")
target_link_libraries(Noize ${OpenCV_LIBS})
