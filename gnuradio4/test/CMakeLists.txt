foreach(
  _tgt
  IN
  ITEMS qa_DigitalMods
        qa_AnalogMods
        qa_DigitalVoice
        qa_Dsss
        qa_Gdss
        qa_Support
        qa_PnSequenceGenerator
        qa_GrIdent)
  add_executable(${_tgt} ${_tgt}.cpp)
  target_compile_features(${_tgt} PRIVATE cxx_std_23)
  target_include_directories(${_tgt} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(${_tgt} PRIVATE gnuradio4::gr-qradiolink ut)
  add_test(NAME ${_tgt} COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:${_tgt}>)
endforeach()
