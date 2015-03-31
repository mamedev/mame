BASE_TARGETOS = "unix"
SDLOS_TARGETOS = "unix"
SYNC_IMPLEMENTATION = "tc"
if _OPTIONS["targetos"]=="openbsd" then
	SYNC_IMPLEMENTATION = "ntc"
elseif _OPTIONS["targetos"]=="netbsd" then
	SYNC_IMPLEMENTATION = "ntc"
elseif _OPTIONS["targetos"]=="haiku" then
	SYNC_IMPLEMENTATION = "ntc"
elseif _OPTIONS["targetos"]=="emscripten" then
	SYNC_IMPLEMENTATION = "mini"
elseif _OPTIONS["targetos"]=="windows" then
	BASE_TARGETOS = "win32"
	SDLOS_TARGETOS = "win32"
	SYNC_IMPLEMENTATION = "windows"
elseif _OPTIONS["targetos"]=="macosx" then
	SDLOS_TARGETOS = "macosx"
	SYNC_IMPLEMENTATION = "ntc"
elseif _OPTIONS["targetos"]=="os2" then
	BASE_TARGETOS = "os2"
	SDLOS_TARGETOS = "os2"
	SYNC_IMPLEMENTATION = "os2"
end

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

	buildoptions {
		"-I$(shell qmake -query QT_INSTALL_HEADERS)/QtCore",
		"-I$(shell qmake -query QT_INSTALL_HEADERS)/QtGui",
		"-I$(shell qmake -query QT_INSTALL_HEADERS)",
	}
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
