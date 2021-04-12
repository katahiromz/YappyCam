# FaceDetection.yap
add_library(FaceDetection SHARED FaceDetection_yap.cpp FaceDetection_yap.def FaceDetection_yap_res.rc)
set_target_properties(FaceDetection PROPERTIES OUTPUT_NAME "FaceDetection.yap")
set_target_properties(FaceDetection PROPERTIES PREFIX "")
set_target_properties(FaceDetection PROPERTIES SUFFIX "")
target_link_libraries(FaceDetection ${OpenCV_LIBS})
