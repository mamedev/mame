-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

dofile('modules.lua')

forcedincludes {
	MAME_DIR .. "src/osd/sdl/sdlprefix.h"
}

if _OPTIONS["USE_TAPTUN"]=="1" or _OPTIONS["USE_PCAP"]=="1" or _OPTIONS["USE_VMNET"]=="1" or _OPTIONS["USE_VMNET_HELPER"]=="1" then
	defines {
		"USE_NETWORK",
	}
	if _OPTIONS["USE_TAPTUN"]=="1" then
		defines {
			"OSD_NET_USE_TAPTUN",
		}
	end
	if _OPTIONS["USE_PCAP"]=="1" then
		defines {
			"OSD_NET_USE_PCAP",
		}
	end
	if _OPTIONS["USE_VMNET"]=="1" then
		defines {
			"OSD_NET_USE_VMNET",
		}
	end
	if _OPTIONS["USE_VMNET_HELPER"]=="1" then
		defines {
			"OSD_NET_USE_VMNET_HELPER",
		}
	end
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

if _OPTIONS["USE_WAYLAND"]=="1" then
	defines {
		"SDLMAME_USE_WAYLAND",
	}
	if _OPTIONS["targetos"]=="linux" then
		buildoptions {
			backtick(pkgconfigcmd() .. " --cflags wayland-egl"),
		}
	end
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

if _OPTIONS["NO_USE_XINPUT_WII_LIGHTGUN_HACK"]=="1" then
	defines {
		"USE_XINPUT_WII_LIGHTGUN_HACK=0",
	}
else
	defines {
		"USE_XINPUT_WII_LIGHTGUN_HACK=1",
	}
end

if _OPTIONS["NO_USE_MIDI"]~="1" and _OPTIONS["targetos"]=="linux" then
	buildoptions {
		backtick(pkgconfigcmd() .. " --cflags alsa"),
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
		if _OPTIONS["USE_LIBSDL"]~="1" then
			buildoptions {
				"-F" .. _OPTIONS["SDL_FRAMEWORK_PATH"],
			}
		else
			defines {
				"MACOSX_USE_LIBSDL",
			}
			buildoptions {
				backtick(sdlconfigcmd() .. " --cflags | sed 's:/SDL2::'"),
			}
		end
	elseif _OPTIONS["targetos"]=="android" then
		buildoptions {
			backtick(sdlconfigcmd() .. " --cflags | sed 's:/SDL2::'"),
		}
	else
		buildoptions {
			backtick(sdlconfigcmd() .. " --cflags"),
		}
		if _OPTIONS["targetos"]~="asmjs" then
			buildoptions {
				backtick(pkgconfigcmd() .. " --cflags fontconfig"),
			}
		end
	end
end

if _OPTIONS["targetos"]=="windows" then
	configuration { "mingw* or vs*" }
		defines {
			"UNICODE",
			"_UNICODE",
			"_WIN32_WINNT=0x0600",
			"WIN32_LEAN_AND_MEAN",
			"NOMINMAX",
		}

	configuration { }

elseif _OPTIONS["targetos"]=="linux" then
	if _OPTIONS["QT_HOME"]~=nil then
		buildoptions {
			"-I" .. backtick(_OPTIONS["QT_HOME"] .. "/bin/qmake -query QT_INSTALL_HEADERS"),
		}
	else
		buildoptions {
			backtick(pkgconfigcmd() .. " --cflags Qt5Widgets"),
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

