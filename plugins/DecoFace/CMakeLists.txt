# DecoFace.yap
add_library(DecoFace SHARED DecoFace_yap.cpp DecoFace_yap.def DecoFace_yap_res.rc)
set_target_properties(DecoFace PROPERTIES OUTPUT_NAME "DecoFace.yap")
set_target_properties(DecoFace PROPERTIES PREFIX "")
set_target_properties(DecoFace PROPERTIES SUFFIX "")
target_link_libraries(DecoFace ${OpenCV_LIBS} shlwapi)
