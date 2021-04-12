# Quadruple.yap
add_library(Quadruple SHARED Quadruple_yap.cpp Quadruple_yap.def Quadruple_yap_res.rc)
set_target_properties(Quadruple PROPERTIES OUTPUT_NAME "Quadruple.yap")
set_target_properties(Quadruple PROPERTIES PREFIX "")
set_target_properties(Quadruple PROPERTIES SUFFIX "")
target_link_libraries(Quadruple ${OpenCV_LIBS})
