add_executable(paloopback
    src/audio_analyzer.c
    src/audio_analyzer.h
    src/biquad_filter.c
    src/biquad_filter.h
    src/paqa.c
    src/paqa_tools.c
    src/paqa_tools.h
    src/qa_tools.h
    src/test_audio_analyzer.c
    src/test_audio_analyzer.h
    src/write_wav.c
    src/write_wav.h
)
target_include_directories(paloopback PRIVATE ..)
target_link_libraries(paloopback PortAudio)
if(UNIX)
  target_link_libraries(paloopback m)
endif()
