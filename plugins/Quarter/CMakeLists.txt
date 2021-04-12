# Quarter.yap
add_library(Quarter SHARED Quarter_yap.cpp Quarter_yap.def Quarter_yap_res.rc)
set_target_properties(Quarter PROPERTIES OUTPUT_NAME "Quarter.yap")
set_target_properties(Quarter PROPERTIES PREFIX "")
set_target_properties(Quarter PROPERTIES SUFFIX "")
target_link_libraries(Quarter ${OpenCV_LIBS})
