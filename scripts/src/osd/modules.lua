-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   modules.lua
--
--   Rules for the building of modules
--
---------------------------------------------------------------------------

function string.starts(String,Start)
	return string.sub(String,1,string.len(Start))==Start
end

function addlibfromstring(str)
	if (str==nil) then return  end
	for w in str:gmatch("%S+") do
		if string.starts(w,"-l")==true then
			links {
				string.sub(w,3)
			}
		end
	end
end

function addoptionsfromstring(str)
	if (str==nil) then return  end
	for w in str:gmatch("%S+") do
		if string.starts(w,"-l")==false then
			linkoptions {
				w
			}
		end
	end
end

function pkgconfigcmd()
	local pkgconfig = os.getenv("PKG_CONFIG")
	if pkgconfig == nil then
		return "pkg-config"
	end
	return pkgconfig
end

function osdmodulesbuild()

	removeflags {
		"SingleOutputDir",
	}

	files {
		MAME_DIR .. "src/osd/osdnet.cpp",
		MAME_DIR .. "src/osd/osdnet.h",
		MAME_DIR .. "src/osd/watchdog.cpp",
		MAME_DIR .. "src/osd/watchdog.h",
		MAME_DIR .. "src/osd/interface/inputcode.h",
		MAME_DIR .. "src/osd/interface/inputdev.h",
		MAME_DIR .. "src/osd/interface/inputfwd.h",
		MAME_DIR .. "src/osd/interface/inputman.h",
		MAME_DIR .. "src/osd/interface/inputseq.cpp",
		MAME_DIR .. "src/osd/interface/inputseq.h",
		MAME_DIR .. "src/osd/interface/midiport.h",
		MAME_DIR .. "src/osd/interface/nethandler.cpp",
		MAME_DIR .. "src/osd/interface/nethandler.h",
		MAME_DIR .. "src/osd/interface/uievents.h",
		MAME_DIR .. "src/osd/modules/debugger/debug_module.h",
		MAME_DIR .. "src/osd/modules/debugger/debuggdbstub.cpp",
		MAME_DIR .. "src/osd/modules/debugger/debugimgui.cpp",
		MAME_DIR .. "src/osd/modules/debugger/debugwin.cpp",
		MAME_DIR .. "src/osd/modules/debugger/none.cpp",
		MAME_DIR .. "src/osd/modules/debugger/xmlconfig.cpp",
		MAME_DIR .. "src/osd/modules/debugger/xmlconfig.h",
		MAME_DIR .. "src/osd/modules/diagnostics/diagnostics_module.h",
		MAME_DIR .. "src/osd/modules/diagnostics/diagnostics_win32.cpp",
		MAME_DIR .. "src/osd/modules/diagnostics/none.cpp",
		MAME_DIR .. "src/osd/modules/font/font_dwrite.cpp",
		MAME_DIR .. "src/osd/modules/font/font_module.h",
		MAME_DIR .. "src/osd/modules/font/font_none.cpp",
		MAME_DIR .. "src/osd/modules/font/font_osx.cpp",
		MAME_DIR .. "src/osd/modules/font/font_sdl.cpp",
		MAME_DIR .. "src/osd/modules/font/font_windows.cpp",
		MAME_DIR .. "src/osd/modules/input/assignmenthelper.cpp",
		MAME_DIR .. "src/osd/modules/input/assignmenthelper.h",
		MAME_DIR .. "src/osd/modules/input/input_common.cpp",
		MAME_DIR .. "src/osd/modules/input/input_common.h",
		MAME_DIR .. "src/osd/modules/input/input_dinput.cpp",
		MAME_DIR .. "src/osd/modules/input/input_dinput.h",
		MAME_DIR .. "src/osd/modules/input/input_mac.cpp",
		MAME_DIR .. "src/osd/modules/input/input_module.h",
		MAME_DIR .. "src/osd/modules/input/input_none.cpp",
		MAME_DIR .. "src/osd/modules/input/input_rawinput.cpp",
		MAME_DIR .. "src/osd/modules/input/input_sdl.cpp",
		MAME_DIR .. "src/osd/modules/input/input_win32.cpp",
		MAME_DIR .. "src/osd/modules/input/input_wincommon.h",
		MAME_DIR .. "src/osd/modules/input/input_windows.cpp",
		MAME_DIR .. "src/osd/modules/input/input_windows.h",
		MAME_DIR .. "src/osd/modules/input/input_winhybrid.cpp",
		MAME_DIR .. "src/osd/modules/input/input_x11.cpp",
		MAME_DIR .. "src/osd/modules/input/input_xinput.cpp",
		MAME_DIR .. "src/osd/modules/input/input_xinput.h",
		MAME_DIR .. "src/osd/modules/lib/osdobj_common.cpp",
		MAME_DIR .. "src/osd/modules/lib/osdobj_common.h",
		MAME_DIR .. "src/osd/modules/midi/midi_module.h",
		MAME_DIR .. "src/osd/modules/midi/none.cpp",
		MAME_DIR .. "src/osd/modules/midi/portmidi.cpp",
		MAME_DIR .. "src/osd/modules/monitor/monitor_common.cpp",
		MAME_DIR .. "src/osd/modules/monitor/monitor_common.h",
		MAME_DIR .. "src/osd/modules/monitor/monitor_dxgi.cpp",
		MAME_DIR .. "src/osd/modules/monitor/monitor_mac.cpp",
		MAME_DIR .. "src/osd/modules/monitor/monitor_module.h",
		MAME_DIR .. "src/osd/modules/monitor/monitor_sdl.cpp",
		MAME_DIR .. "src/osd/modules/monitor/monitor_win32.cpp",
		MAME_DIR .. "src/osd/modules/netdev/netdev_module.h",
		MAME_DIR .. "src/osd/modules/netdev/none.cpp",
		MAME_DIR .. "src/osd/modules/netdev/pcap.cpp",
		MAME_DIR .. "src/osd/modules/netdev/taptun.cpp",
		MAME_DIR .. "src/osd/modules/netdev/vmnet.cpp",
		MAME_DIR .. "src/osd/modules/netdev/vmnet_helper.cpp",
		MAME_DIR .. "src/osd/modules/netdev/vmnet_common.cpp",
		MAME_DIR .. "src/osd/modules/output/console.cpp",
		MAME_DIR .. "src/osd/modules/output/network.cpp",
		MAME_DIR .. "src/osd/modules/output/none.cpp",
		MAME_DIR .. "src/osd/modules/output/output_module.h",
		MAME_DIR .. "src/osd/modules/output/win32_output.cpp",
		MAME_DIR .. "src/osd/modules/output/win32_output.h",
		MAME_DIR .. "src/osd/modules/render/blit13.ipp",
		MAME_DIR .. "src/osd/modules/render/draw13.cpp",
		MAME_DIR .. "src/osd/modules/render/drawgdi.cpp",
		MAME_DIR .. "src/osd/modules/render/drawnone.cpp",
		MAME_DIR .. "src/osd/modules/render/drawogl.cpp",
		MAME_DIR .. "src/osd/modules/render/drawsdl.cpp",
		MAME_DIR .. "src/osd/modules/render/render_module.h",
		MAME_DIR .. "src/osd/modules/sound/coreaudio_sound.cpp",
		MAME_DIR .. "src/osd/modules/sound/direct_sound.cpp",
		MAME_DIR .. "src/osd/modules/sound/js_sound.cpp",
		MAME_DIR .. "src/osd/modules/sound/none.cpp",
		MAME_DIR .. "src/osd/modules/sound/pa_sound.cpp",
		MAME_DIR .. "src/osd/modules/sound/pulse_sound.cpp",
		MAME_DIR .. "src/osd/modules/sound/sdl_sound.cpp",
		MAME_DIR .. "src/osd/modules/sound/sound_module.h",
		MAME_DIR .. "src/osd/modules/sound/xaudio2_sound.cpp",
	}
	includedirs {
		MAME_DIR .. "src/osd",
		ext_includedir("asio"),
	}

	if _OPTIONS["gcc"]~=nil and string.find(_OPTIONS["gcc"], "clang") then
		buildoptions {
			"-Wno-unused-private-field",
		}
	end

	if _OPTIONS["targetos"]=="windows" then
		includedirs {
			MAME_DIR .. "3rdparty/winpcap/Include",
			MAME_DIR .. "3rdparty/compat/mingw",
			MAME_DIR .. "3rdparty/portaudio/include",
		}

		includedirs {
			MAME_DIR .. "3rdparty/compat/winsdk-override",
		}
	end

	if _OPTIONS["NO_OPENGL"]=="1" then
		defines {
			"USE_OPENGL=0",
		}
	else
		files {
			MAME_DIR .. "src/osd/modules/opengl/gl_shader_tool.cpp",
			MAME_DIR .. "src/osd/modules/opengl/gl_shader_mgr.cpp",
			MAME_DIR .. "src/osd/modules/opengl/gl_shader_mgr.h",
			MAME_DIR .. "src/osd/modules/opengl/gl_shader_tool.h",
			MAME_DIR .. "src/osd/modules/opengl/osd_opengl.h",
		}
		defines {
			"USE_OPENGL=1",
		}
		if _OPTIONS["USE_DISPATCH_GL"]=="1" then
			defines {
				"USE_DISPATCH_GL=1",
			}
		end
	end

	defines {
		"__STDC_LIMIT_MACROS",
		"__STDC_FORMAT_MACROS",
		"__STDC_CONSTANT_MACROS",
		"BX_CONFIG_DEBUG=0",
	}

	files {
		MAME_DIR .. "src/osd/modules/render/drawbgfx.cpp",
		MAME_DIR .. "src/osd/modules/render/aviwrite.cpp",
		MAME_DIR .. "src/osd/modules/render/aviwrite.h",
		MAME_DIR .. "src/osd/modules/render/bgfxutil.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfxutil.h",
		MAME_DIR .. "src/osd/modules/render/binpacker.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/blendreader.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/blendreader.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/chain.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/chain.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/chainentry.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/chainentry.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/chainentryreader.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/chainentryreader.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/chainmanager.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/chainmanager.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/chainreader.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/chainreader.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/clear.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/clear.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/clearreader.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/clearreader.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/cullreader.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/cullreader.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/depthreader.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/depthreader.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/effect.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/effect.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/effectmanager.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/effectmanager.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/effectreader.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/effectreader.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/entryuniformreader.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/entryuniformreader.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/inputpair.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/inputpair.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/frameparameter.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/frameparameter.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/timeparameter.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/timeparameter.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/paramreader.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/paramreader.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/paramuniform.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/paramuniform.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/paramuniformreader.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/paramuniformreader.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/shadermanager.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/shadermanager.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/slider.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/slider.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/sliderreader.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/sliderreader.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/slideruniform.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/slideruniform.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/slideruniformreader.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/slideruniformreader.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/statereader.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/statereader.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/suppressor.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/suppressor.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/suppressorreader.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/suppressorreader.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/target.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/target.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/targetreader.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/targetreader.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/targetmanager.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/targetmanager.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/texture.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/texture.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/texturehandleprovider.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/texturemanager.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/texturemanager.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/uniform.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/uniform.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/uniformreader.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/uniformreader.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/valueuniform.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/valueuniform.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/valueuniformreader.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/valueuniformreader.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/view.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/view.h",
		MAME_DIR .. "src/osd/modules/render/bgfx/writereader.cpp",
		MAME_DIR .. "src/osd/modules/render/bgfx/writereader.h",
	}
	includedirs {
		MAME_DIR .. "3rdparty/bgfx/examples/common",
		MAME_DIR .. "3rdparty/bgfx/include",
		MAME_DIR .. "3rdparty/bgfx/3rdparty",
		MAME_DIR .. "3rdparty/bgfx/3rdparty/khronos",
		MAME_DIR .. "3rdparty/bx/include",
		ext_includedir("rapidjson")
	}

	if _OPTIONS["NO_USE_PORTAUDIO"]=="1" then
		defines {
			"NO_USE_PORTAUDIO",
		}
	else
		includedirs {
			ext_includedir("portaudio"),
		}
	end

	if _OPTIONS["NO_USE_PULSEAUDIO"]=="1" then
		defines {
			"NO_USE_PULSEAUDIO",
		}
	end

	if _OPTIONS["NO_USE_MIDI"]=="1" then
		defines {
			"NO_USE_MIDI",
		}
	else
		includedirs {
			ext_includedir("portmidi"),
		}
	end

	if _OPTIONS["USE_QTDEBUG"]=="1" then
		defines {
			"USE_QTDEBUG=1",
		}
	else
		defines {
			"USE_QTDEBUG=0",
		}
	end

end


function qtdebuggerbuild()

	removeflags {
		"SingleOutputDir",
	}
	local version = str_to_version(_OPTIONS["gcc_version"])
	if _OPTIONS["gcc"]~=nil and (string.find(_OPTIONS["gcc"], "clang") or string.find(_OPTIONS["gcc"], "asmjs")) then
		configuration { "gmake or ninja" }
			buildoptions {
				"-Wno-error=inconsistent-missing-override",
			}
		configuration { }
	end

	files {
		MAME_DIR .. "src/osd/modules/debugger/debugqt.cpp",
	}

	if _OPTIONS["USE_QTDEBUG"]=="1" then
		files {
			MAME_DIR .. "src/osd/modules/debugger/qt/debuggerview.cpp",
			MAME_DIR .. "src/osd/modules/debugger/qt/debuggerview.h",
			MAME_DIR .. "src/osd/modules/debugger/qt/windowqt.cpp",
			MAME_DIR .. "src/osd/modules/debugger/qt/windowqt.h",
			MAME_DIR .. "src/osd/modules/debugger/qt/logwindow.cpp",
			MAME_DIR .. "src/osd/modules/debugger/qt/logwindow.h",
			MAME_DIR .. "src/osd/modules/debugger/qt/dasmwindow.cpp",
			MAME_DIR .. "src/osd/modules/debugger/qt/dasmwindow.h",
			MAME_DIR .. "src/osd/modules/debugger/qt/mainwindow.cpp",
			MAME_DIR .. "src/osd/modules/debugger/qt/mainwindow.h",
			MAME_DIR .. "src/osd/modules/debugger/qt/memorywindow.cpp",
			MAME_DIR .. "src/osd/modules/debugger/qt/memorywindow.h",
			MAME_DIR .. "src/osd/modules/debugger/qt/breakpointswindow.cpp",
			MAME_DIR .. "src/osd/modules/debugger/qt/breakpointswindow.h",
			MAME_DIR .. "src/osd/modules/debugger/qt/deviceswindow.cpp",
			MAME_DIR .. "src/osd/modules/debugger/qt/deviceinformationwindow.cpp",
			MAME_DIR .. "src/osd/modules/debugger/qt/deviceinformationwindow.h",
			MAME_DIR .. "src/osd/modules/debugger/qt/deviceswindow.h",
			GEN_DIR .. "osd/modules/debugger/qt/debuggerview.moc.cpp",
			GEN_DIR .. "osd/modules/debugger/qt/windowqt.moc.cpp",
			GEN_DIR .. "osd/modules/debugger/qt/logwindow.moc.cpp",
			GEN_DIR .. "osd/modules/debugger/qt/dasmwindow.moc.cpp",
			GEN_DIR .. "osd/modules/debugger/qt/mainwindow.moc.cpp",
			GEN_DIR .. "osd/modules/debugger/qt/memorywindow.moc.cpp",
			GEN_DIR .. "osd/modules/debugger/qt/breakpointswindow.moc.cpp",
			GEN_DIR .. "osd/modules/debugger/qt/deviceswindow.moc.cpp",
			GEN_DIR .. "osd/modules/debugger/qt/deviceinformationwindow.moc.cpp",
		}
		defines {
			"USE_QTDEBUG=1",
		}

		local MOC = ""
		if (os.is("windows")) then
			MOC = "moc"
		else
			if _OPTIONS["QT_HOME"]~=nil then
				MOCTST = backtick(_OPTIONS["QT_HOME"] .. "/bin/moc --version 2>/dev/null")
				if (MOCTST=='') then
					MOCTST = backtick(_OPTIONS["QT_HOME"] .. "/libexec/moc --version 2>/dev/null")
					if (MOCTST=='') then
						print("Qt's Meta Object Compiler (moc) wasn't found!")
						os.exit(1)
					else
						MOC = _OPTIONS["QT_HOME"] .. "/libexec/moc"
					end
				else
					MOC = _OPTIONS["QT_HOME"] .. "/bin/moc"
				end
			else
				MOCTST = backtick("which moc-qt5 2>/dev/null")
				if (MOCTST=='') then
					MOCTST = backtick("which moc 2>/dev/null")
				end
				if (MOCTST=='') then
					print("Qt's Meta Object Compiler (moc) wasn't found!")
					os.exit(1)
				end
				MOC = MOCTST
			end
		end


		custombuildtask {
			{ MAME_DIR .. "src/osd/modules/debugger/qt/debuggerview.h",             GEN_DIR .. "osd/modules/debugger/qt/debuggerview.moc.cpp", { },             { MOC .. "$(MOCINCPATH) -b emu.h $(<) -o $(@)" } },
			{ MAME_DIR .. "src/osd/modules/debugger/qt/windowqt.h",                 GEN_DIR .. "osd/modules/debugger/qt/windowqt.moc.cpp", { },                 { MOC .. "$(MOCINCPATH) -b emu.h $(<) -o $(@)" } },
			{ MAME_DIR .. "src/osd/modules/debugger/qt/logwindow.h",                GEN_DIR .. "osd/modules/debugger/qt/logwindow.moc.cpp", { },                { MOC .. "$(MOCINCPATH) -b emu.h $(<) -o $(@)" } },
			{ MAME_DIR .. "src/osd/modules/debugger/qt/dasmwindow.h",               GEN_DIR .. "osd/modules/debugger/qt/dasmwindow.moc.cpp", { },               { MOC .. "$(MOCINCPATH) -b emu.h $(<) -o $(@)" } },
			{ MAME_DIR .. "src/osd/modules/debugger/qt/mainwindow.h",               GEN_DIR .. "osd/modules/debugger/qt/mainwindow.moc.cpp", { },               { MOC .. "$(MOCINCPATH) -b emu.h $(<) -o $(@)" } },
			{ MAME_DIR .. "src/osd/modules/debugger/qt/memorywindow.h",             GEN_DIR .. "osd/modules/debugger/qt/memorywindow.moc.cpp", { },             { MOC .. "$(MOCINCPATH) -b emu.h $(<) -o $(@)" } },
			{ MAME_DIR .. "src/osd/modules/debugger/qt/breakpointswindow.h",        GEN_DIR .. "osd/modules/debugger/qt/breakpointswindow.moc.cpp", { },        { MOC .. "$(MOCINCPATH) -b emu.h $(<) -o $(@)" } },
			{ MAME_DIR .. "src/osd/modules/debugger/qt/deviceswindow.h",            GEN_DIR .. "osd/modules/debugger/qt/deviceswindow.moc.cpp", { },            { MOC .. "$(MOCINCPATH) -b emu.h $(<) -o $(@)" } },
			{ MAME_DIR .. "src/osd/modules/debugger/qt/deviceinformationwindow.h",  GEN_DIR .. "osd/modules/debugger/qt/deviceinformationwindow.moc.cpp", { },  { MOC .. "$(MOCINCPATH) -b emu.h $(<) -o $(@)" } },

		}

		if _OPTIONS["targetos"]=="windows" then
			configuration { "mingw*" }
				buildoptions {
					"-I$(shell qmake -query QT_INSTALL_HEADERS)",
				}
			configuration { }
		elseif _OPTIONS["targetos"]=="macosx" then
			buildoptions {
				"-F" .. backtick("qmake -query QT_INSTALL_LIBS"),
			}
		else
			if _OPTIONS["QT_HOME"]~=nil then
				buildoptions {
					"-I" .. backtick(_OPTIONS["QT_HOME"] .. "/bin/qmake -query QT_INSTALL_HEADERS"),
				}
			else
				buildoptions {
					backtick(pkgconfigcmd() .. " --cflags Qt5Widgets"),
				}
			end
		end
	else
		defines {
			"USE_QTDEBUG=0",
		}
	end

end


function osdmodulestargetconf()

	if _OPTIONS["NO_OPENGL"]~="1" then
		if _OPTIONS["targetos"]=="macosx" then
			links {
				"OpenGL.framework",
			}
		elseif _OPTIONS["USE_DISPATCH_GL"]~="1" then
			if _OPTIONS["targetos"]=="windows" then
				links {
					"opengl32",
				}
			else
				links {
					"GL",
				}
			end
		end
	end

	if _OPTIONS["NO_USE_MIDI"]~="1" then
		if _OPTIONS["targetos"]=="linux" then
			local str = backtick(pkgconfigcmd() .. " --libs alsa")
			addlibfromstring(str)
			addoptionsfromstring(str)
		elseif _OPTIONS["targetos"]=="macosx" then
			links {
				"CoreMIDI.framework",
			}
		end
	end

	if _OPTIONS["USE_QTDEBUG"]=="1" then
		if _OPTIONS["targetos"]=="windows" then
			linkoptions {
				"-L$(shell qmake -query QT_INSTALL_LIBS)",
			}
			links {
				"Qt5Core.dll",
				"Qt5Gui.dll",
				"Qt5Widgets.dll",
			}
		elseif _OPTIONS["targetos"]=="macosx" then
			linkoptions {
				"-F" .. backtick("qmake -query QT_INSTALL_LIBS"),
			}
			links {
				"Qt5Core.framework",
				"Qt5Gui.framework",
				"Qt5Widgets.framework",
			}
		else
			if _OPTIONS["QT_HOME"]~=nil then
				local qt_version = str_to_version(backtick(_OPTIONS["QT_HOME"] .. "/bin/qmake -query QT_VERSION"))
				linkoptions {
					"-L" .. backtick(_OPTIONS["QT_HOME"] .. "/bin/qmake -query QT_INSTALL_LIBS"),
				}
				if qt_version < 60000 then
					links {
						"Qt5Core",
						"Qt5Gui",
						"Qt5Widgets",
					}
				else
					links {
						"Qt6Core",
						"Qt6Gui",
						"Qt6Widgets",
					}
				end
			else
				local str = backtick(pkgconfigcmd() .. " --libs Qt5Widgets")
				addlibfromstring(str)
				addoptionsfromstring(str)
			end
		end
	end

	if _OPTIONS["USE_VMNET"]=="1" then
		links {
			"vmnet.framework",
			"Security.framework"
		}
	end

	if _OPTIONS["targetos"]=="windows" then
		links {
			"gdi32",
			"dsound",
			"dxguid",
			"oleaut32",
			"winmm",
		}
	elseif _OPTIONS["targetos"]=="macosx" then
		links {
			"AudioUnit.framework",
			"AudioToolbox.framework",
			"CoreAudio.framework",
			"CoreServices.framework",
		}
	end

	if _OPTIONS["NO_USE_PULSEAUDIO"]=="0" then
		links {
			ext_lib("pulse"),
		}
	end
end


newoption {
	trigger = "USE_TAPTUN",
	description = "Include tap/tun network module",
	allowed = {
		{ "0",  "Don't include tap/tun network module" },
		{ "1",  "Include tap/tun network module" },
	},
}

newoption {
	trigger = "USE_PCAP",
	description = "Include pcap network module",
	allowed = {
		{ "0",  "Don't include pcap network module" },
		{ "1",  "Include pcap network module" },
	},
}

newoption {
	trigger = "USE_VMNET",
	description = "Include vmnet network module (macOS).  This builds vmnet support into MAME, so MAME must be run as root to access the network.",
	allowed = {
		{ "0",  "Don't include vmnet network module" },
		{ "1",  "Include vmnet network module" },
	},
}

newoption {
	trigger = "USE_VMNET_HELPER",
	description = "Include vmnet helper network module (macOS).  This works with the external vmnet_helper program, so that MAME doesn't have to run as root.",
	allowed = {
		{ "0",  "Don't include vmnet helper network module" },
		{ "1",  "Include vmnet network helper module" },
	},
}

newoption {
	trigger = "NO_OPENGL",
	description = "Disable use of OpenGL",
	allowed = {
		{ "0",  "Enable OpenGL"  },
		{ "1",  "Disable OpenGL" },
	},
}

if not _OPTIONS["NO_OPENGL"] then
	if _OPTIONS["targetos"] == "android" then
		_OPTIONS["NO_OPENGL"] = "1"
	else
		_OPTIONS["NO_OPENGL"] = "0"
	end
end

newoption {
	trigger = "USE_DISPATCH_GL",
	description = "Use GL-dispatching",
	allowed = {
		{ "0",  "Link to OpenGL library"  },
		{ "1",  "Use GL-dispatching"      },
	},
}

if not _OPTIONS["USE_DISPATCH_GL"] then
	_OPTIONS["USE_DISPATCH_GL"] = "0"
end

newoption {
	trigger = "NO_USE_MIDI",
	description = "Disable MIDI I/O",
	allowed = {
		{ "0",  "Enable MIDI"  },
		{ "1",  "Disable MIDI" },
	},
}

if not _OPTIONS["NO_USE_MIDI"] then
	if _OPTIONS["targetos"]=="freebsd" or _OPTIONS["targetos"]=="openbsd" or _OPTIONS["targetos"]=="netbsd" or _OPTIONS["targetos"]=="solaris" or _OPTIONS["targetos"]=="haiku" or _OPTIONS["targetos"] == "asmjs" or _OPTIONS["targetos"] == "android" then
		_OPTIONS["NO_USE_MIDI"] = "1"
	else
		_OPTIONS["NO_USE_MIDI"] = "0"
	end
end

newoption {
	trigger = "NO_USE_PORTAUDIO",
	description = "Disable PortAudio interface",
	allowed = {
		{ "0",  "Enable PortAudio"  },
		{ "1",  "Disable PortAudio" },
	},
}

if not _OPTIONS["NO_USE_PORTAUDIO"] then
	if _OPTIONS["targetos"]=="windows" or _OPTIONS["targetos"]=="linux" or _OPTIONS["targetos"]=="macosx" then
		_OPTIONS["NO_USE_PORTAUDIO"] = "0"
	else
		_OPTIONS["NO_USE_PORTAUDIO"] = "1"
	end
end

newoption {
	trigger = "NO_USE_PULSEAUDIO",
	description = "Disable PulseAudio interface",
	allowed = {
		{ "0",  "Enable PulseAudio"  },
		{ "1",  "Disable PulseAudio" },
	},
}

if not _OPTIONS["NO_USE_PULSEAUDIO"] then
	if _OPTIONS["targetos"]=="linux" then
		_OPTIONS["NO_USE_PULSEAUDIO"] = "0"
	else
		_OPTIONS["NO_USE_PULSEAUDIO"] = "1"
	end
end

newoption {
	trigger = "MODERN_WIN_API",
	description = "Use Modern Windows APIs",
	allowed = {
		{ "0",  "Use classic Windows APIs - allows support for XP and later"   },
		{ "1",  "Use Modern Windows APIs - support for Windows 8.1 and later"  },
	},
}

newoption {
	trigger = "USE_QTDEBUG",
	description = "Use QT debugger",
	allowed = {
		{ "0",  "Don't use Qt debugger" },
		{ "1",  "Use Qt debugger" },
	},
}

newoption {
	trigger = "QT_HOME",
	description = "QT lib location",
}


if not _OPTIONS["USE_TAPTUN"] then
	if _OPTIONS["targetos"]=="linux" or _OPTIONS["targetos"]=="windows" then
		_OPTIONS["USE_TAPTUN"] = "1"
	else
		_OPTIONS["USE_TAPTUN"] = "0"
	end
end

if not _OPTIONS["USE_PCAP"] then
	if _OPTIONS["targetos"]=="macosx" or _OPTIONS["targetos"]=="netbsd" then
		_OPTIONS["USE_PCAP"] = "1"
	else
		_OPTIONS["USE_PCAP"] = "0"
	end
end

if not _OPTIONS["USE_VMNET"] then
	if _OPTIONS["targetos"]=="macosx" then
		_OPTIONS["USE_VMNET"] = "1"
	else
		_OPTIONS["USE_VMNET"] = "0"
	end
end

if not _OPTIONS["USE_VMNET_HELPER"] then
	if _OPTIONS["targetos"]=="macosx" then
		_OPTIONS["USE_VMNET_HELPER"] = "1"
	else
		_OPTIONS["USE_VMNET_HELPER"] = "0"
	end
end

if not _OPTIONS["USE_QTDEBUG"] then
	if _OPTIONS["targetos"]=="windows" or _OPTIONS["targetos"]=="macosx" or _OPTIONS["targetos"]=="solaris" or _OPTIONS["targetos"]=="haiku" or _OPTIONS["targetos"]=="asmjs" or _OPTIONS["targetos"]=="android" then
		_OPTIONS["USE_QTDEBUG"] = "0"
	else
		_OPTIONS["USE_QTDEBUG"] = "1"
	end
end
