# ChromaKey.yap
add_library(ChromaKey SHARED ChromaKey_yap.cpp ChromaKey_yap.def ChromaKey_yap_res.rc)
set_target_properties(ChromaKey PROPERTIES OUTPUT_NAME "ChromaKey.yap")
set_target_properties(ChromaKey PROPERTIES PREFIX "")
set_target_properties(ChromaKey PROPERTIES SUFFIX "")
target_link_libraries(ChromaKey shlwapi ${OpenCV_LIBS})
