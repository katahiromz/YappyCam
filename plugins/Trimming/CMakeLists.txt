# Trimming.yap
add_library(Trimming SHARED Trimming_yap.cpp Trimming_yap.def Trimming_yap_res.rc)
set_target_properties(Trimming PROPERTIES OUTPUT_NAME "Trimming.yap")
set_target_properties(Trimming PROPERTIES PREFIX "")
set_target_properties(Trimming PROPERTIES SUFFIX "")
target_link_libraries(Trimming ${OpenCV_LIBS})
