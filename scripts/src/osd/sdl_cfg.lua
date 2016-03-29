-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

forcedincludes {
	MAME_DIR .. "src/osd/sdl/sdlprefix.h"
}

if SDL_NETWORK~="" and not _OPTIONS["DONT_USE_NETWORK"] then
	defines {
		"USE_NETWORK",
		"OSD_NET_USE_" .. string.upper(SDL_NETWORK),
	}
end

if _OPTIONS["NO_OPENGL"]~="1" and _OPTIONS["USE_DISPATCH_GL"]~="1" and _OPTIONS["MESA_INSTALL_ROOT"] then
	includedirs {
		path.join(_OPTIONS["MESA_INSTALL_ROOT"],"include"),
	}
end

if _OPTIONS["SDL_INI_PATH"]~=nil then
	defines {
		"'INI_PATH=\"" .. _OPTIONS["SDL_INI_PATH"] .. "\"'",
	}
end

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

if _OPTIONS["NO_USE_MIDI"]~="1" and _OPTIONS["targetos"]=="linux" then
	buildoptions {
		backtick("pkg-config --cflags alsa"),
	}
end

defines {
	"SDLMAME_SDL2=1",
}
if _OPTIONS["SDL2_MULTIAPI"]=="1" then
	defines {
		"SDL2_MULTIAPI",
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
		if _OPTIONS["with-bundled-sdl2"]==nil then
			if _OPTIONS["USE_LIBSDL"]~="1" then
				buildoptions {
					"-F" .. _OPTIONS["SDL_FRAMEWORK_PATH"],
				}
			else
				defines {
					"MACOSX_USE_LIBSDL",
				}
				buildoptions {
					backtick(sdlconfigcmd() .. " --cflags | sed 's:/SDL::'"),
				}
			end
			end
	else
		buildoptions {
			backtick(sdlconfigcmd() .. " --cflags"),
		}
		if _OPTIONS["targetos"]~="asmjs" then
			buildoptions {
				backtick("pkg-config --cflags fontconfig"),
			}
		end
	end
end

if _OPTIONS["targetos"]=="windows" then
	configuration { "mingw* or vs*" }
		defines {
			"UNICODE",
			"_UNICODE",
			"main=utf8_main",
		}

	configuration { "Debug" }
		defines {
			"MALLOC_DEBUG",
		}
	configuration { }

elseif _OPTIONS["targetos"]=="linux" then
	if _OPTIONS["QT_HOME"]~=nil then
		buildoptions {
			"-I" .. backtick(_OPTIONS["QT_HOME"] .. "/bin/qmake -query QT_INSTALL_HEADERS"),
		}
	else
		buildoptions {
			backtick("pkg-config --cflags Qt5Widgets"),
		}
	end
elseif _OPTIONS["targetos"]=="macosx" then
	defines {
		"SDLMAME_MACOSX",
		"SDLMAME_DARWIN",
	}
elseif _OPTIONS["targetos"]=="freebsd" then
	buildoptions {
		-- /usr/local/include is not considered a system include director on FreeBSD.  GL.h resides there and throws warnings
		"-isystem /usr/local/include",
	}
end

configuration { "osx*" }
	includedirs {
		MAME_DIR .. "3rdparty/bx/include/compat/osx",
	}

configuration { "freebsd" }
	includedirs {
		MAME_DIR .. "3rdparty/bx/include/compat/freebsd",
	}

configuration { "netbsd" }
	includedirs {
		MAME_DIR .. "3rdparty/bx/include/compat/freebsd",
	}

configuration { }

