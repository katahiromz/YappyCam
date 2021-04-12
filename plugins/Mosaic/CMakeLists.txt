# Mosaic.yap
add_library(Mosaic SHARED Mosaic_yap.cpp Mosaic_yap.def Mosaic_yap_res.rc)
set_target_properties(Mosaic PROPERTIES OUTPUT_NAME "Mosaic.yap")
set_target_properties(Mosaic PROPERTIES PREFIX "")
set_target_properties(Mosaic PROPERTIES SUFFIX "")
target_link_libraries(Mosaic ${OpenCV_LIBS})
