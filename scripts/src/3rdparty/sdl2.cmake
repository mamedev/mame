# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##################################################
## SDL2 library
##################################################

if (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
	add_library(SDL2 SHARED EXCLUDE_FROM_ALL)

	target_link_libraries(SDL2 PUBLIC
			dl
			GLESv1_CM
			GLESv2
			log
			android
			OpenSLES
	)
	target_link_options(SDL2 PRIVATE "-Wl,-soname,libSDL2.so")
	target_compile_definitions(SDL2 PRIVATE GL_GLEXT_PROTOTYPES)

	target_compile_options(SDL2 PRIVATE -Wno-strict-prototypes)
	target_compile_options(SDL2 PRIVATE -Wno-implicit-function-declaration)
	target_compile_options(SDL2 PRIVATE -Wno-unneeded-internal-declaration)
	target_compile_options(SDL2 PRIVATE -Wno-pointer-to-int-cast)
else()
	add_library(SDL2 STATIC EXCLUDE_FROM_ALL)
endif()

target_sources(SDL2 PRIVATE
	${MAME_DIR}/3rdparty/SDL2/include/begin_code.h
	${MAME_DIR}/3rdparty/SDL2/include/close_code.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_assert.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_atomic.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_audio.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_bits.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_blendmode.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_clipboard.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_config.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_config_windows.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_copying.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_cpuinfo.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_egl.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_endian.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_error.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_events.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_filesystem.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_gamecontroller.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_gesture.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_haptic.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_hints.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_joystick.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_keyboard.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_keycode.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_loadso.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_log.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_main.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_messagebox.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_mouse.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_mutex.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_name.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_opengl.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_opengl_glext.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_opengles.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_opengles2.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_opengles2_gl2.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_opengles2_gl2ext.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_opengles2_gl2platform.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_opengles2_khrplatform.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_pixels.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_platform.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_power.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_quit.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_rect.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_render.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_revision.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_rwops.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_scancode.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_shape.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_stdinc.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_surface.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_system.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_syswm.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_test.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_test_assert.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_test_common.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_test_compare.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_test_crc32.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_test_font.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_test_fuzzer.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_test_harness.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_test_images.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_test_log.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_test_md5.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_test_random.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_thread.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_timer.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_touch.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_types.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_version.h
	${MAME_DIR}/3rdparty/SDL2/include/SDL_video.h


	${MAME_DIR}/3rdparty/SDL2/src/atomic/SDL_atomic.c
	${MAME_DIR}/3rdparty/SDL2/src/atomic/SDL_spinlock.c
	${MAME_DIR}/3rdparty/SDL2/src/audio/disk/SDL_diskaudio.c
	${MAME_DIR}/3rdparty/SDL2/src/audio/disk/SDL_diskaudio.h
	${MAME_DIR}/3rdparty/SDL2/src/audio/dummy/SDL_dummyaudio.c
	${MAME_DIR}/3rdparty/SDL2/src/audio/dummy/SDL_dummyaudio.h
	${MAME_DIR}/3rdparty/SDL2/src/audio/SDL_audio.c
	${MAME_DIR}/3rdparty/SDL2/src/audio/SDL_audio_c.h
	${MAME_DIR}/3rdparty/SDL2/src/audio/SDL_audiocvt.c
	${MAME_DIR}/3rdparty/SDL2/src/audio/SDL_audiodev.c
	${MAME_DIR}/3rdparty/SDL2/src/audio/SDL_audiodev_c.h
	${MAME_DIR}/3rdparty/SDL2/src/audio/SDL_audiotypecvt.c
	${MAME_DIR}/3rdparty/SDL2/src/audio/SDL_mixer.c
	${MAME_DIR}/3rdparty/SDL2/src/audio/SDL_sysaudio.h
	${MAME_DIR}/3rdparty/SDL2/src/audio/SDL_wave.c
	${MAME_DIR}/3rdparty/SDL2/src/audio/SDL_wave.h
	${MAME_DIR}/3rdparty/SDL2/src/cpuinfo/SDL_cpuinfo.c
	${MAME_DIR}/3rdparty/SDL2/src/dynapi/SDL_dynapi.c
	${MAME_DIR}/3rdparty/SDL2/src/dynapi/SDL_dynapi.h
	${MAME_DIR}/3rdparty/SDL2/src/dynapi/SDL_dynapi_overrides.h
	${MAME_DIR}/3rdparty/SDL2/src/dynapi/SDL_dynapi_procs.h
	${MAME_DIR}/3rdparty/SDL2/src/events/blank_cursor.h
	${MAME_DIR}/3rdparty/SDL2/src/events/default_cursor.h
	${MAME_DIR}/3rdparty/SDL2/src/events/SDL_clipboardevents.c
	${MAME_DIR}/3rdparty/SDL2/src/events/SDL_clipboardevents_c.h
	${MAME_DIR}/3rdparty/SDL2/src/events/SDL_dropevents.c
	${MAME_DIR}/3rdparty/SDL2/src/events/SDL_dropevents_c.h
	${MAME_DIR}/3rdparty/SDL2/src/events/SDL_events.c
	${MAME_DIR}/3rdparty/SDL2/src/events/SDL_events_c.h
	${MAME_DIR}/3rdparty/SDL2/src/events/SDL_gesture.c
	${MAME_DIR}/3rdparty/SDL2/src/events/SDL_gesture_c.h
	${MAME_DIR}/3rdparty/SDL2/src/events/SDL_keyboard.c
	${MAME_DIR}/3rdparty/SDL2/src/events/SDL_keyboard_c.h
	${MAME_DIR}/3rdparty/SDL2/src/events/SDL_mouse.c
	${MAME_DIR}/3rdparty/SDL2/src/events/SDL_mouse_c.h
	${MAME_DIR}/3rdparty/SDL2/src/events/SDL_quit.c
	${MAME_DIR}/3rdparty/SDL2/src/events/SDL_sysevents.h
	${MAME_DIR}/3rdparty/SDL2/src/events/SDL_touch.c
	${MAME_DIR}/3rdparty/SDL2/src/events/SDL_touch_c.h
	${MAME_DIR}/3rdparty/SDL2/src/events/SDL_windowevents.c
	${MAME_DIR}/3rdparty/SDL2/src/events/SDL_windowevents_c.h
	${MAME_DIR}/3rdparty/SDL2/src/file/SDL_rwops.c
	${MAME_DIR}/3rdparty/SDL2/src/haptic/SDL_haptic.c
	${MAME_DIR}/3rdparty/SDL2/src/haptic/SDL_syshaptic.h
	${MAME_DIR}/3rdparty/SDL2/src/joystick/SDL_gamecontroller.c
	${MAME_DIR}/3rdparty/SDL2/src/joystick/SDL_joystick.c
	${MAME_DIR}/3rdparty/SDL2/src/joystick/SDL_joystick_c.h
	${MAME_DIR}/3rdparty/SDL2/src/joystick/SDL_sysjoystick.h
	${MAME_DIR}/3rdparty/SDL2/src/loadso/windows/SDL_sysloadso.c
	${MAME_DIR}/3rdparty/SDL2/src/power/SDL_power.c
	${MAME_DIR}/3rdparty/SDL2/src/power/windows/SDL_syspower.c
	${MAME_DIR}/3rdparty/SDL2/src/render/direct3d/SDL_render_d3d.c
	${MAME_DIR}/3rdparty/SDL2/src/render/direct3d11/SDL_render_d3d11.c
	${MAME_DIR}/3rdparty/SDL2/src/render/mmx.h
	${MAME_DIR}/3rdparty/SDL2/src/render/opengl/SDL_render_gl.c
	${MAME_DIR}/3rdparty/SDL2/src/render/opengl/SDL_shaders_gl.c
	${MAME_DIR}/3rdparty/SDL2/src/render/opengl/SDL_shaders_gl.h
	${MAME_DIR}/3rdparty/SDL2/src/render/opengles2/SDL_render_gles2.c
	${MAME_DIR}/3rdparty/SDL2/src/render/opengles2/SDL_shaders_gles2.c
	${MAME_DIR}/3rdparty/SDL2/src/render/SDL_d3dmath.c
	${MAME_DIR}/3rdparty/SDL2/src/render/SDL_d3dmath.h
	${MAME_DIR}/3rdparty/SDL2/src/render/SDL_render.c
	${MAME_DIR}/3rdparty/SDL2/src/render/SDL_sysrender.h
	${MAME_DIR}/3rdparty/SDL2/src/render/SDL_yuv_mmx.c
	${MAME_DIR}/3rdparty/SDL2/src/render/SDL_yuv_sw.c
	${MAME_DIR}/3rdparty/SDL2/src/render/SDL_yuv_sw_c.h
	${MAME_DIR}/3rdparty/SDL2/src/render/software/SDL_blendfillrect.c
	${MAME_DIR}/3rdparty/SDL2/src/render/software/SDL_blendfillrect.h
	${MAME_DIR}/3rdparty/SDL2/src/render/software/SDL_blendline.c
	${MAME_DIR}/3rdparty/SDL2/src/render/software/SDL_blendline.h
	${MAME_DIR}/3rdparty/SDL2/src/render/software/SDL_blendpoint.c
	${MAME_DIR}/3rdparty/SDL2/src/render/software/SDL_blendpoint.h
	${MAME_DIR}/3rdparty/SDL2/src/render/software/SDL_draw.h
	${MAME_DIR}/3rdparty/SDL2/src/render/software/SDL_drawline.c
	${MAME_DIR}/3rdparty/SDL2/src/render/software/SDL_drawline.h
	${MAME_DIR}/3rdparty/SDL2/src/render/software/SDL_drawpoint.c
	${MAME_DIR}/3rdparty/SDL2/src/render/software/SDL_drawpoint.h
	${MAME_DIR}/3rdparty/SDL2/src/render/software/SDL_render_sw.c
	${MAME_DIR}/3rdparty/SDL2/src/render/software/SDL_render_sw_c.h
	${MAME_DIR}/3rdparty/SDL2/src/render/software/SDL_rotate.c
	${MAME_DIR}/3rdparty/SDL2/src/render/software/SDL_rotate.h
	${MAME_DIR}/3rdparty/SDL2/src/SDL.c
	${MAME_DIR}/3rdparty/SDL2/src/SDL_assert.c
	${MAME_DIR}/3rdparty/SDL2/src/SDL_error.c
	${MAME_DIR}/3rdparty/SDL2/src/SDL_error_c.h
	${MAME_DIR}/3rdparty/SDL2/src/SDL_hints.c
	${MAME_DIR}/3rdparty/SDL2/src/SDL_log.c
	${MAME_DIR}/3rdparty/SDL2/src/stdlib/SDL_getenv.c
	${MAME_DIR}/3rdparty/SDL2/src/stdlib/SDL_iconv.c
	${MAME_DIR}/3rdparty/SDL2/src/stdlib/SDL_malloc.c
	${MAME_DIR}/3rdparty/SDL2/src/stdlib/SDL_qsort.c
	${MAME_DIR}/3rdparty/SDL2/src/stdlib/SDL_stdlib.c
	${MAME_DIR}/3rdparty/SDL2/src/stdlib/SDL_string.c
	${MAME_DIR}/3rdparty/SDL2/src/thread/SDL_systhread.h
	${MAME_DIR}/3rdparty/SDL2/src/thread/SDL_thread.c
	${MAME_DIR}/3rdparty/SDL2/src/thread/SDL_thread_c.h
	${MAME_DIR}/3rdparty/SDL2/src/timer/SDL_timer.c
	${MAME_DIR}/3rdparty/SDL2/src/timer/SDL_timer_c.h
	${MAME_DIR}/3rdparty/SDL2/src/video/dummy/SDL_nullevents.c
	${MAME_DIR}/3rdparty/SDL2/src/video/dummy/SDL_nullevents_c.h
	${MAME_DIR}/3rdparty/SDL2/src/video/dummy/SDL_nullframebuffer.c
	${MAME_DIR}/3rdparty/SDL2/src/video/dummy/SDL_nullframebuffer_c.h
	${MAME_DIR}/3rdparty/SDL2/src/video/dummy/SDL_nullvideo.c
	${MAME_DIR}/3rdparty/SDL2/src/video/dummy/SDL_nullvideo.h
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_blit.c
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_blit.h
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_blit_0.c
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_blit_1.c
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_blit_A.c
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_blit_auto.c
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_blit_auto.h
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_blit_copy.c
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_blit_copy.h
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_blit_N.c
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_blit_slow.c
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_blit_slow.h
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_bmp.c
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_clipboard.c
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_egl.c
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_fillrect.c
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_pixels.c
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_pixels_c.h
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_rect.c
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_rect_c.h
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_RLEaccel.c
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_RLEaccel_c.h
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_shape.c
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_shape_internals.h
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_stretch.c
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_surface.c
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_sysvideo.h
	${MAME_DIR}/3rdparty/SDL2/src/video/SDL_video.c
)

if((${CMAKE_SYSTEM_NAME} STREQUAL "Darwin") OR  (${CMAKE_SYSTEM_NAME} STREQUAL "Windows"))
	target_sources(SDL2 PRIVATE
		${MAME_DIR}/3rdparty/SDL2/src/libm/e_atan2.c
		${MAME_DIR}/3rdparty/SDL2/src/libm/e_log.c
		${MAME_DIR}/3rdparty/SDL2/src/libm/e_pow.c
		${MAME_DIR}/3rdparty/SDL2/src/libm/e_rem_pio2.c
		${MAME_DIR}/3rdparty/SDL2/src/libm/e_sqrt.c
		${MAME_DIR}/3rdparty/SDL2/src/libm/k_cos.c
		${MAME_DIR}/3rdparty/SDL2/src/libm/k_rem_pio2.c
		${MAME_DIR}/3rdparty/SDL2/src/libm/k_sin.c
		${MAME_DIR}/3rdparty/SDL2/src/libm/k_tan.c
		${MAME_DIR}/3rdparty/SDL2/src/libm/math_private.h
		${MAME_DIR}/3rdparty/SDL2/src/libm/s_atan.c
		${MAME_DIR}/3rdparty/SDL2/src/libm/s_copysign.c
		${MAME_DIR}/3rdparty/SDL2/src/libm/s_cos.c
		${MAME_DIR}/3rdparty/SDL2/src/libm/s_fabs.c
		${MAME_DIR}/3rdparty/SDL2/src/libm/s_floor.c
		${MAME_DIR}/3rdparty/SDL2/src/libm/s_scalbn.c
		${MAME_DIR}/3rdparty/SDL2/src/libm/s_sin.c
		${MAME_DIR}/3rdparty/SDL2/src/libm/s_tan.c
	)
endif()

if(NOT (${CMAKE_SYSTEM_NAME} STREQUAL "Windows"))
	target_sources(SDL2 PRIVATE
		${MAME_DIR}/3rdparty/SDL2/src/render/opengles/SDL_render_gles.c
		${MAME_DIR}/3rdparty/SDL2/src/render/opengles/SDL_glesfuncs.h
	)
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
	target_sources(SDL2 PRIVATE
		${MAME_DIR}/3rdparty/SDL2/src/audio/android/opensl_io.h
		${MAME_DIR}/3rdparty/SDL2/src/audio/android/opensl_io.c
		${MAME_DIR}/3rdparty/SDL2/src/audio/android/SDL_androidaudio.h
		${MAME_DIR}/3rdparty/SDL2/src/audio/android/SDL_androidaudio.c
		${MAME_DIR}/3rdparty/SDL2/src/core/android/SDL_android.c
		${MAME_DIR}/3rdparty/SDL2/src/core/android/SDL_android.h
		${MAME_DIR}/3rdparty/SDL2/src/filesystem/android/SDL_sysfilesystem.c
		${MAME_DIR}/3rdparty/SDL2/src/haptic/dummy/SDL_syshaptic.c
		${MAME_DIR}/3rdparty/SDL2/src/joystick/android/SDL_sysjoystick.c
		${MAME_DIR}/3rdparty/SDL2/src/loadso/dlopen/SDL_sysloadso.c
		${MAME_DIR}/3rdparty/SDL2/src/power/android/SDL_syspower.c
		${MAME_DIR}/3rdparty/SDL2/src/thread/pthread/SDL_syscond.c
		${MAME_DIR}/3rdparty/SDL2/src/thread/pthread/SDL_sysmutex.c
		${MAME_DIR}/3rdparty/SDL2/src/thread/pthread/SDL_sysmutex_c.h
		${MAME_DIR}/3rdparty/SDL2/src/thread/pthread/SDL_syssem.c
		${MAME_DIR}/3rdparty/SDL2/src/thread/pthread/SDL_systhread.c
		${MAME_DIR}/3rdparty/SDL2/src/thread/pthread/SDL_systhread_c.h
		${MAME_DIR}/3rdparty/SDL2/src/thread/pthread/SDL_systls.c
		${MAME_DIR}/3rdparty/SDL2/src/timer/unix/SDL_systimer.c
		${MAME_DIR}/3rdparty/SDL2/src/video/android/SDL_androidclipboard.c
		${MAME_DIR}/3rdparty/SDL2/src/video/android/SDL_androidclipboard.h
		${MAME_DIR}/3rdparty/SDL2/src/video/android/SDL_androidevents.c
		${MAME_DIR}/3rdparty/SDL2/src/video/android/SDL_androidevents.h
		${MAME_DIR}/3rdparty/SDL2/src/video/android/SDL_androidgl.c
		${MAME_DIR}/3rdparty/SDL2/src/video/android/SDL_androidkeyboard.c
		${MAME_DIR}/3rdparty/SDL2/src/video/android/SDL_androidkeyboard.h
		${MAME_DIR}/3rdparty/SDL2/src/video/android/SDL_androidmessagebox.c
		${MAME_DIR}/3rdparty/SDL2/src/video/android/SDL_androidmessagebox.h
		${MAME_DIR}/3rdparty/SDL2/src/video/android/SDL_androidmouse.c
		${MAME_DIR}/3rdparty/SDL2/src/video/android/SDL_androidmouse.h
		${MAME_DIR}/3rdparty/SDL2/src/video/android/SDL_androidtouch.c
		${MAME_DIR}/3rdparty/SDL2/src/video/android/SDL_androidtouch.h
		${MAME_DIR}/3rdparty/SDL2/src/video/android/SDL_androidvideo.c
		${MAME_DIR}/3rdparty/SDL2/src/video/android/SDL_androidvideo.h
		${MAME_DIR}/3rdparty/SDL2/src/video/android/SDL_androidwindow.c
		${MAME_DIR}/3rdparty/SDL2/src/video/android/SDL_androidwindow.h
	)
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	target_sources(SDL2 PRIVATE
		${MAME_DIR}/3rdparty/SDL2/src/audio/coreaudio/SDL_coreaudio.m
		${MAME_DIR}/3rdparty/SDL2/src/audio/coreaudio/SDL_coreaudio.h
		${MAME_DIR}/3rdparty/SDL2/src/file/cocoa/SDL_rwopsbundlesupport.m
		${MAME_DIR}/3rdparty/SDL2/src/file/cocoa/SDL_rwopsbundlesupport.h
		${MAME_DIR}/3rdparty/SDL2/src/filesystem/cocoa/SDL_sysfilesystem.m
		${MAME_DIR}/3rdparty/SDL2/src/haptic/darwin/SDL_syshaptic.c
		${MAME_DIR}/3rdparty/SDL2/src/haptic/darwin/SDL_syshaptic_c.h
		${MAME_DIR}/3rdparty/SDL2/src/joystick/darwin/SDL_sysjoystick.c
		${MAME_DIR}/3rdparty/SDL2/src/joystick/darwin/SDL_sysjoystick_c.h
		${MAME_DIR}/3rdparty/SDL2/src/loadso/dlopen/SDL_sysloadso.c
		${MAME_DIR}/3rdparty/SDL2/src/power/macosx/SDL_syspower.c
		${MAME_DIR}/3rdparty/SDL2/src/thread/pthread/SDL_syscond.c
		${MAME_DIR}/3rdparty/SDL2/src/thread/pthread/SDL_sysmutex.c
		${MAME_DIR}/3rdparty/SDL2/src/thread/pthread/SDL_sysmutex_c.h
		${MAME_DIR}/3rdparty/SDL2/src/thread/pthread/SDL_syssem.c
		${MAME_DIR}/3rdparty/SDL2/src/thread/pthread/SDL_systhread.c
		${MAME_DIR}/3rdparty/SDL2/src/thread/pthread/SDL_systhread_c.h
		${MAME_DIR}/3rdparty/SDL2/src/thread/pthread/SDL_systls.c
		${MAME_DIR}/3rdparty/SDL2/src/timer/unix/SDL_systimer.c
		${MAME_DIR}/3rdparty/SDL2/src/video/cocoa/SDL_cocoaclipboard.m
		${MAME_DIR}/3rdparty/SDL2/src/video/cocoa/SDL_cocoaclipboard.h
		${MAME_DIR}/3rdparty/SDL2/src/video/cocoa/SDL_cocoaevents.m
		${MAME_DIR}/3rdparty/SDL2/src/video/cocoa/SDL_cocoaevents.h
		${MAME_DIR}/3rdparty/SDL2/src/video/cocoa/SDL_cocoakeyboard.m
		${MAME_DIR}/3rdparty/SDL2/src/video/cocoa/SDL_cocoakeyboard.h
		${MAME_DIR}/3rdparty/SDL2/src/video/cocoa/SDL_cocoamessagebox.m
		${MAME_DIR}/3rdparty/SDL2/src/video/cocoa/SDL_cocoamessagebox.h
		${MAME_DIR}/3rdparty/SDL2/src/video/cocoa/SDL_cocoamodes.m
		${MAME_DIR}/3rdparty/SDL2/src/video/cocoa/SDL_cocoamodes.h
		${MAME_DIR}/3rdparty/SDL2/src/video/cocoa/SDL_cocoamouse.m
		${MAME_DIR}/3rdparty/SDL2/src/video/cocoa/SDL_cocoamouse.h
		${MAME_DIR}/3rdparty/SDL2/src/video/cocoa/SDL_cocoamousetap.m
		${MAME_DIR}/3rdparty/SDL2/src/video/cocoa/SDL_cocoamousetap.h
		${MAME_DIR}/3rdparty/SDL2/src/video/cocoa/SDL_cocoaopengl.m
		${MAME_DIR}/3rdparty/SDL2/src/video/cocoa/SDL_cocoaopengl.h
		${MAME_DIR}/3rdparty/SDL2/src/video/cocoa/SDL_cocoashape.m
		${MAME_DIR}/3rdparty/SDL2/src/video/cocoa/SDL_cocoashape.h
		${MAME_DIR}/3rdparty/SDL2/src/video/cocoa/SDL_cocoavideo.m
		${MAME_DIR}/3rdparty/SDL2/src/video/cocoa/SDL_cocoavideo.h
		${MAME_DIR}/3rdparty/SDL2/src/video/cocoa/SDL_cocoawindow.m
		${MAME_DIR}/3rdparty/SDL2/src/video/cocoa/SDL_cocoawindow.h
	)
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	target_sources(SDL2 PRIVATE
		${MAME_DIR}/3rdparty/SDL2/src/thread/generic/SDL_syscond.c
		${MAME_DIR}/3rdparty/SDL2/src/audio/directsound/SDL_directsound.c
		${MAME_DIR}/3rdparty/SDL2/src/audio/directsound/SDL_directsound.h
		${MAME_DIR}/3rdparty/SDL2/src/audio/winmm/SDL_winmm.c
		${MAME_DIR}/3rdparty/SDL2/src/audio/winmm/SDL_winmm.h
		${MAME_DIR}/3rdparty/SDL2/src/core/windows/SDL_directx.h
		${MAME_DIR}/3rdparty/SDL2/src/core/windows/SDL_windows.c
		${MAME_DIR}/3rdparty/SDL2/src/core/windows/SDL_windows.h
		${MAME_DIR}/3rdparty/SDL2/src/core/windows/SDL_xinput.c
		${MAME_DIR}/3rdparty/SDL2/src/core/windows/SDL_xinput.h
		${MAME_DIR}/3rdparty/SDL2/src/filesystem/windows/SDL_sysfilesystem.c
		${MAME_DIR}/3rdparty/SDL2/src/haptic/windows/SDL_dinputhaptic.c
		${MAME_DIR}/3rdparty/SDL2/src/haptic/windows/SDL_dinputhaptic_c.h
		${MAME_DIR}/3rdparty/SDL2/src/haptic/windows/SDL_windowshaptic.c
		${MAME_DIR}/3rdparty/SDL2/src/haptic/windows/SDL_windowshaptic_c.h
		${MAME_DIR}/3rdparty/SDL2/src/haptic/windows/SDL_xinputhaptic.c
		${MAME_DIR}/3rdparty/SDL2/src/haptic/windows/SDL_xinputhaptic_c.h
		${MAME_DIR}/3rdparty/SDL2/src/joystick/windows/SDL_dinputjoystick.c
		${MAME_DIR}/3rdparty/SDL2/src/joystick/windows/SDL_dinputjoystick_c.h
		${MAME_DIR}/3rdparty/SDL2/src/joystick/windows/SDL_mmjoystick.c
		${MAME_DIR}/3rdparty/SDL2/src/joystick/windows/SDL_windowsjoystick.c
		${MAME_DIR}/3rdparty/SDL2/src/joystick/windows/SDL_windowsjoystick_c.h
		${MAME_DIR}/3rdparty/SDL2/src/joystick/windows/SDL_xinputjoystick.c
		${MAME_DIR}/3rdparty/SDL2/src/joystick/windows/SDL_xinputjoystick_c.h
		${MAME_DIR}/3rdparty/SDL2/src/thread/windows/SDL_sysmutex.c
		${MAME_DIR}/3rdparty/SDL2/src/thread/windows/SDL_syssem.c
		${MAME_DIR}/3rdparty/SDL2/src/thread/windows/SDL_systhread.c
		${MAME_DIR}/3rdparty/SDL2/src/thread/windows/SDL_systhread_c.h
		${MAME_DIR}/3rdparty/SDL2/src/thread/windows/SDL_systls.c
		${MAME_DIR}/3rdparty/SDL2/src/timer/windows/SDL_systimer.c
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_vkeys.h
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_windowsclipboard.c
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_windowsclipboard.h
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_windowsevents.c
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_windowsevents.h
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_windowsframebuffer.c
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_windowsframebuffer.h
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_windowskeyboard.c
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_windowskeyboard.h
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_windowsmessagebox.c
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_windowsmessagebox.h
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_windowsmodes.c
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_windowsmodes.h
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_windowsmouse.c
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_windowsmouse.h
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_windowsopengl.c
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_windowsopengl.h
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_windowsopengles.c
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_windowsshape.c
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_windowsshape.h
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_windowsvideo.c
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_windowsvideo.h
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_windowswindow.c
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/SDL_windowswindow.h
		${MAME_DIR}/3rdparty/SDL2/src/video/windows/wmmsg.h
		${MAME_DIR}/3rdparty/SDL2/src/main/windows/version.rc
		${MAME_DIR}/3rdparty/SDL2/src/main/windows/SDL_windows_main.c
	)
endif()

if (MINGW)
	target_include_directories(SDL2 PRIVATE ${MAME_DIR}/3rdparty/SDL2-override/mingw)
	target_include_directories(SDL2 PRIVATE ${MAME_DIR}/3rdparty/bgfx/3rdparty/khronos)

	target_compile_options(SDL2 PRIVATE
		-Wno-bad-function-cast
		-Wno-discarded-qualifiers
		-Wno-format
		-Wno-format-security
		-Wno-pointer-to-int-cast
		-Wno-strict-prototypes
		-Wno-undef
		-Wno-unused-but-set-variable
	)
	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		target_compile_options(SDL2 PRIVATE -Wno-incompatible-pointer-types-discards-qualifiers)
	endif()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_sources(SDL2 PRIVATE ${MAME_DIR}/3rdparty/SDL2/src/audio/xaudio2/SDL_xaudio2.c)
	target_compile_options(SDL2 PRIVATE /wd4200) # warning C4200: nonstandard extension used: zero-sized array in struct/union
	target_compile_options(SDL2 PRIVATE /wd4055) # warning C4055: 'type cast': from data pointer 'void *' to function pointer 'xxx'
	target_compile_options(SDL2 PRIVATE /wd4152) # warning C4152: nonstandard extension, function/data pointer conversion in expression
	target_compile_options(SDL2 PRIVATE /wd4057) # warning C4057: 'function': 'xxx' differs in indirection to slightly different base types from 'xxx'
	target_compile_options(SDL2 PRIVATE /wd4701) # warning C4701: potentially uninitialized local variable 'xxx' used
	target_compile_options(SDL2 PRIVATE /wd4204) # warning C4204: nonstandard extension used: non-constant aggregate initializer
	target_compile_options(SDL2 PRIVATE /wd4054) # warning C4054: 'type cast': from function pointer 'xxx' to data pointer 'xxx'
	target_compile_options(SDL2 PRIVATE /wd4131) # warning C4131: 'xxx' : uses old-style declarator
	target_compile_options(SDL2 PRIVATE /wd4312) # warning C4312: 'type cast': conversion from 'xxx' to 'xxx' of greater size

	target_compile_definitions(SDL2 PRIVATE "HAVE_LIBC")
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	target_link_libraries(SDL2 PUBLIC
		imm32
		version
		winmm
	)
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	target_compile_options(SDL2 PRIVATE
		-Wno-undef
		-Wno-bad-function-cast
		-Wno-strict-prototypes
	)
	target_link_libraries(SDL2 PUBLIC
		"-framework AudioToolbox"
		"-framework CoreAudio"
		"-framework CoreVideo"
		"-framework ForceFeedback"
		"-framework IOKit"
		"-framework AppKit"
	)
endif()

target_include_directories(SDL2 PUBLIC ${MAME_DIR}/3rdparty/SDL2/include)
