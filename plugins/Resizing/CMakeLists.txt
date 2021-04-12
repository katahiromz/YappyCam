# Resizing.yap
add_library(Resizing SHARED Resizing_yap.cpp Resizing_yap.def Resizing_yap_res.rc)
set_target_properties(Resizing PROPERTIES OUTPUT_NAME "Resizing.yap")
set_target_properties(Resizing PROPERTIES PREFIX "")
set_target_properties(Resizing PROPERTIES SUFFIX "")
target_link_libraries(Resizing ${OpenCV_LIBS})
