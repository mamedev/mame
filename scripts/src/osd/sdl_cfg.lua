forcedincludes {
	MAME_DIR .. "src/osd/sdl/sdlprefix.h"
}

if _OPTIONS["NO_X11"]=="1" then
	defines {
		"SDLMAME_NO_X11",
	}
else
	defines {
		"SDLMAME_X11",
	}
	includedirs {
		"/usr/X11/include",
		"/usr/X11R6/include",
		"/usr/openwin/include",
	}
end

if _OPTIONS["NO_USE_XINPUT"]=="1" then
	defines {
		"USE_XINPUT=0",
	}
else
	defines {
		"USE_XINPUT=1",
		"USE_XINPUT_DEBUG=0",
	}
end

if _OPTIONS["SDL_LIBVER"]=="sdl2" then
	defines {
		"SDLMAME_SDL2=1",
	}
else
	defines {
		"SDLMAME_SDL2=0",
		"SYNC_IMPLEMENTATION=" .. SYNC_IMPLEMENTATION,
	}
end

if USE_BGFX == 1 then
	defines {
		"USE_BGFX"
	}
end

defines {
	"OSD_SDL",
}

if BASE_TARGETOS=="unix" then
	defines {
		"SDLMAME_UNIX",
	}
	if _OPTIONS["targetos"]=="macosx" then
		if _OPTIONS["MACOSX_USE_LIBSDL"]~="1" then
			buildoptions {
				"-F" .. _OPTIONS["SDL_FRAMEWORK_PATH"],
			}
		else
			defines {
				"NO_SDL_GLEXT",
				"MACOSX_USE_LIBSDL",
			}
			buildoptions {
				string.gsub(os.outputof(sdlconfigcmd() .. " --cflags | sed 's:/SDL::'"), '[\r\n]+', ' '),
			}
		end
	else
		buildoptions {
			string.gsub(os.outputof(sdlconfigcmd() .. " --cflags"), '[\r\n]+', ' '),
		}
		if _OPTIONS["targetos"]~="emscripten" then
			buildoptions {
				string.gsub(os.outputof("pkg-config --cflags fontconfig"), '[\r\n]+', ' '),
			}
		end
	end
end

if _OPTIONS["targetos"]=="windows" then
	defines {
		"SDLMAME_WIN32",
		"UNICODE",
		"_UNICODE",
		"USE_OPENGL=1",
		"USE_QTDEBUG=" .. USE_QT,
		"SDLMAME_NET_PCAP",
		"main=utf8_main",
	}
	configuration { "mingw*" }
		buildoptions {
			"-I$(shell qmake -query QT_INSTALL_HEADERS)/QtCore",
			"-I$(shell qmake -query QT_INSTALL_HEADERS)/QtGui",
			"-I$(shell qmake -query QT_INSTALL_HEADERS)",
		}
		
	configuration { "vs*" }
		includedirs {
			path.join(_OPTIONS["SDL_INSTALL_ROOT"],"include")
		}
	configuration { }

elseif _OPTIONS["targetos"]=="linux" then
	defines {
		"USE_OPENGL=1",
		"USE_QTDEBUG=" .. USE_QT,
		"SDLMAME_NET_TAPTUN",
	}

	buildoptions {
		'$(shell pkg-config --cflags QtGui)',
	}
elseif _OPTIONS["targetos"]=="macosx" then
	defines {
		"SDLMAME_MACOSX",
		"SDLMAME_DARWIN",
		"USE_OPENGL=1",
		"USE_QTDEBUG=0",
		"SDLMAME_NET_PCAP",
	}
elseif _OPTIONS["targetos"]=="os2" then
	defines {
		"SDLMAME_OS2",
	}
end
