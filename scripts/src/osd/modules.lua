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

function osdmodulesbuild()

	removeflags {
		"SingleOutputDir",
	}

	files {
		MAME_DIR .. "src/osd/osdnet.cpp",
		MAME_DIR .. "src/osd/osdnet.h",
		MAME_DIR .. "src/osd/modules/debugger/debug_module.h",
		MAME_DIR .. "src/osd/modules/font/font_module.h",
		MAME_DIR .. "src/osd/modules/midi/midi_module.h",
		MAME_DIR .. "src/osd/modules/netdev/netdev_module.h",
		MAME_DIR .. "src/osd/modules/sound/sound_module.h",
		MAME_DIR .. "src/osd/modules/lib/osdobj_common.cpp",
		MAME_DIR .. "src/osd/modules/lib/osdobj_common.h",
		MAME_DIR .. "src/osd/modules/debugger/none.cpp",
		MAME_DIR .. "src/osd/modules/debugger/debugint.cpp",
		MAME_DIR .. "src/osd/modules/debugger/debugwin.cpp",
		MAME_DIR .. "src/osd/modules/font/font_sdl.cpp",
		MAME_DIR .. "src/osd/modules/font/font_windows.cpp",
		MAME_DIR .. "src/osd/modules/font/font_osx.cpp",
		MAME_DIR .. "src/osd/modules/font/font_none.cpp",
		MAME_DIR .. "src/osd/modules/netdev/taptun.cpp",
		MAME_DIR .. "src/osd/modules/netdev/pcap.cpp",
		MAME_DIR .. "src/osd/modules/netdev/none.cpp",
		MAME_DIR .. "src/osd/modules/midi/portmidi.cpp",
		MAME_DIR .. "src/osd/modules/midi/none.cpp",
		MAME_DIR .. "src/osd/modules/sound/js_sound.cpp",
		MAME_DIR .. "src/osd/modules/sound/direct_sound.cpp",
		MAME_DIR .. "src/osd/modules/sound/coreaudio_sound.cpp",
		MAME_DIR .. "src/osd/modules/sound/sdl_sound.cpp",
		MAME_DIR .. "src/osd/modules/sound/none.cpp",
	}

	if _OPTIONS["targetos"]=="windows" then
		includedirs {
			MAME_DIR .. "3rdparty/winpcap/Include",
		}
	end

	if _OPTIONS["NO_OPENGL"]=="1" then
		defines {
			"USE_OPENGL=0",
		}
	else
		files {
			MAME_DIR .. "src/osd/modules/render/drawogl.cpp",
			MAME_DIR .. "src/osd/modules/opengl/gl_shader_tool.cpp",
			MAME_DIR .. "src/osd/modules/opengl/gl_shader_mgr.cpp",
			MAME_DIR .. "src/osd/modules/opengl/gl_shader_mgr.h",
			MAME_DIR .. "src/osd/modules/opengl/gl_shader_tool.h",
			MAME_DIR .. "src/osd/modules/opengl/osd_opengl.h",
			MAME_DIR .. "src/osd/modules/opengl/SDL1211_opengl.h",
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

	if USE_BGFX == 1 then
		files {
			MAME_DIR .. "src/osd/modules/render/drawbgfx.cpp",
		}
		defines {
			"USE_BGFX"
		}
		includedirs {
			MAME_DIR .. "3rdparty/bgfx/include",
			MAME_DIR .. "3rdparty/bx/include",
		}
	end

	if _OPTIONS["NO_USE_MIDI"]=="1" then
		defines {
			"NO_USE_MIDI",
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
				QMAKETST = backtick(_OPTIONS["QT_HOME"] .. "/bin/qmake --version 2>/dev/null")
				if (QMAKETST=='') then
					print("Qt's Meta Object Compiler (moc) wasn't found!")
					os.exit(1)
				end	
				MOC = _OPTIONS["QT_HOME"] .. "/bin/moc"
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
			{ MAME_DIR .. "src/osd/modules/debugger/qt/debuggerview.h", 			GEN_DIR .. "osd/modules/debugger/qt/debuggerview.moc.cpp", { },			{ MOC .. "$(MOCINCPATH) $(<) -o $(@)" }},
			{ MAME_DIR .. "src/osd/modules/debugger/qt/windowqt.h", 				GEN_DIR .. "osd/modules/debugger/qt/windowqt.moc.cpp", { }, 				{ MOC .. "$(MOCINCPATH) $(<) -o $(@)" }},
			{ MAME_DIR .. "src/osd/modules/debugger/qt/logwindow.h", 				GEN_DIR .. "osd/modules/debugger/qt/logwindow.moc.cpp", { }, 				{ MOC .. "$(MOCINCPATH) $(<) -o $(@)" }},
			{ MAME_DIR .. "src/osd/modules/debugger/qt/dasmwindow.h", 				GEN_DIR .. "osd/modules/debugger/qt/dasmwindow.moc.cpp", { }, 			{ MOC .. "$(MOCINCPATH) $(<) -o $(@)" }},
			{ MAME_DIR .. "src/osd/modules/debugger/qt/mainwindow.h", 				GEN_DIR .. "osd/modules/debugger/qt/mainwindow.moc.cpp", { }, 			{ MOC .. "$(MOCINCPATH) $(<) -o $(@)" }},
			{ MAME_DIR .. "src/osd/modules/debugger/qt/memorywindow.h",				GEN_DIR .. "osd/modules/debugger/qt/memorywindow.moc.cpp", { }, 			{ MOC .. "$(MOCINCPATH) $(<) -o $(@)" }},
			{ MAME_DIR .. "src/osd/modules/debugger/qt/breakpointswindow.h",		GEN_DIR .. "osd/modules/debugger/qt/breakpointswindow.moc.cpp", { }, 		{ MOC .. "$(MOCINCPATH) $(<) -o $(@)" }},
			{ MAME_DIR .. "src/osd/modules/debugger/qt/deviceswindow.h", 			GEN_DIR .. "osd/modules/debugger/qt/deviceswindow.moc.cpp", { }, 			{ MOC .. "$(MOCINCPATH) $(<) -o $(@)" }},
			{ MAME_DIR .. "src/osd/modules/debugger/qt/deviceinformationwindow.h",  GEN_DIR .. "osd/modules/debugger/qt/deviceinformationwindow.moc.cpp", { },{ MOC .. "$(MOCINCPATH) $(<) -o $(@)" }},
			
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
					backtick("pkg-config --cflags Qt5Widgets"),
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
			local str = backtick("pkg-config --libs alsa")
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
				"qtmain",
				"Qt5Core",
				"Qt5Gui",
				"Qt5Widgets",
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
				linkoptions {
					"-L" .. backtick(_OPTIONS["QT_HOME"] .. "/bin/qmake -query QT_INSTALL_LIBS"),
				}
				links {
					"Qt5Core",
					"Qt5Gui",
					"Qt5Widgets",
				}
			else
				local str = backtick("pkg-config --libs Qt5Widgets")
				addlibfromstring(str)
				addoptionsfromstring(str)
			end
		end
	end

	if _OPTIONS["targetos"]=="windows" then
		links {
			"gdi32",
			"dsound",
			"dxguid",
		}
	elseif _OPTIONS["targetos"]=="macosx" then
		links {
			"AudioUnit.framework",
			"AudioToolbox.framework",
			"CoreAudio.framework",
			"CoreServices.framework",
		}
	end

end


newoption {
	trigger = "DONT_USE_NETWORK",
	description = "Disable network access",
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
	if _OPTIONS["targetos"]=="os2" then
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
	if USE_BGFX == 1 then
		_OPTIONS["USE_DISPATCH_GL"] = "0"
	else
		_OPTIONS["USE_DISPATCH_GL"] = "1"
	end
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
	if _OPTIONS["targetos"]=="freebsd" or _OPTIONS["targetos"]=="openbsd" or _OPTIONS["targetos"]=="netbsd" or _OPTIONS["targetos"]=="solaris" or _OPTIONS["targetos"]=="haiku" or _OPTIONS["targetos"] == "asmjs" or _OPTIONS["targetos"] == "os2" then
		_OPTIONS["NO_USE_MIDI"] = "1"
	else
		_OPTIONS["NO_USE_MIDI"] = "0"
	end
end

newoption {
	trigger = "USE_QTDEBUG",
	description = "Use QT debugger",
	allowed = {
		{ "0",  "Don't use Qt debugger"  },
		{ "1",  "Use Qt debugger" },
	},
}

newoption {
	trigger = "QT_HOME",
	description = "QT lib location",
}


if not _OPTIONS["USE_QTDEBUG"] then
	if _OPTIONS["targetos"]=="windows" or _OPTIONS["targetos"]=="macosx" or _OPTIONS["targetos"]=="solaris" or _OPTIONS["targetos"]=="haiku" or _OPTIONS["targetos"] == "asmjs" or _OPTIONS["targetos"] == "os2" then
		_OPTIONS["USE_QTDEBUG"] = "0"
	else
		_OPTIONS["USE_QTDEBUG"] = "1"
	end
end
