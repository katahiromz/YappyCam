# Monochrome.yap
add_library(Monochrome SHARED Monochrome_yap.cpp Monochrome_yap.def Monochrome_yap_res.rc)
set_target_properties(Monochrome PROPERTIES OUTPUT_NAME "Monochrome.yap")
set_target_properties(Monochrome PROPERTIES PREFIX "")
set_target_properties(Monochrome PROPERTIES SUFFIX "")
target_link_libraries(Monochrome ${OpenCV_LIBS})
