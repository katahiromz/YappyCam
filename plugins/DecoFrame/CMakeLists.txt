# DecoFrame.yap
add_library(DecoFrame SHARED DecoFrame_yap.cpp DecoFrame_yap.def DecoFrame_yap_res.rc)
set_target_properties(DecoFrame PROPERTIES OUTPUT_NAME "DecoFrame.yap")
set_target_properties(DecoFrame PROPERTIES PREFIX "")
set_target_properties(DecoFrame PROPERTIES SUFFIX "")
target_link_libraries(DecoFrame ${OpenCV_LIBS} shlwapi)
