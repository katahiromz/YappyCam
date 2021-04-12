# medianBlur.yap
add_library(medianBlur SHARED medianBlur_yap.cpp medianBlur_yap.def medianBlur_yap_res.rc)
set_target_properties(medianBlur PROPERTIES OUTPUT_NAME "medianBlur.yap")
set_target_properties(medianBlur PROPERTIES PREFIX "")
set_target_properties(medianBlur PROPERTIES SUFFIX "")
target_link_libraries(medianBlur ${OpenCV_LIBS})
