if _OPTIONS["osd"]=="windows" then
	defines {
		"UNICODE",
		"_UNICODE",
		"X64_WINDOWS_ABI",
		"OSD_WINDOWS",
		"USE_SDL=0",
		"USE_QTDEBUG=0",
		"USE_OPENGL=1",
		"USE_DISPATCH_GL=1",
		"DIRECTINPUT_VERSION=0x0800"
	}
	--forcedincludes {
	--	MAME_DIR .. "src/osd/windows/winprefix.h"
	--}
elseif _OPTIONS["osd"]=="sdl" then
	--forcedincludes {
	--	MAME_DIR .. "src/osd/sdl/sdlprefix.h"
	--}
	if _OPTIONS["targetos"]=="windows" then
		defines {
			"OSD_SDL",
			"SDLMAME_WIN32",
			"X64_WINDOWS_ABI",
			"UNICODE",
			"_UNICODE",
			"SDLMAME_SDL2=1",
			"USE_XINPUT=0",
			"USE_OPENGL=1",
			"USE_QTDEBUG=1",
		}
		buildoptions {
			"-I$(shell qmake -query QT_INSTALL_HEADERS)/QtCore",
			"-I$(shell qmake -query QT_INSTALL_HEADERS)/QtGui",
			"-I$(shell qmake -query QT_INSTALL_HEADERS)",
		}
		
		linkoptions{
			"-L$(shell qmake -query QT_INSTALL_LIBS)",
		}
			
		links {
			"qtmain",
			"QtGui4",
			"QtCore4",
		}
	end
	if _OPTIONS["targetos"]=="linux" then
		defines {
			"OSD_SDL",
			"SDLMAME_UNIX",
			"SDLMAME_X11",
			"SDLMAME_SDL2=1",
			"USE_XINPUT=0",
			"USE_OPENGL=1",
			"USE_QTDEBUG=1",
		}
		if (USE_BGFX == 1) then
			defines {
				"USE_BGFX"
			}
		end
		buildoptions {
			'$(shell pkg-config --cflags QtGui)',
		}
		links {
			'QtGui',
			'QtCore',
		}
		--linkoptions {
		--	'$(shell pkg-config --libs QtGui)',
		--}
	end
	if _OPTIONS["targetos"]=="macosx" then
		defines {
			"OSD_SDL",
			"SDLMAME_UNIX",
			"SDLMAME_MACOSX",
			"SDLMAME_DARWIN",
			"SDLMAME_SDL2=1",
			"USE_XINPUT=0",
			"USE_OPENGL=1",
			"USE_QTDEBUG=0",
		}
		if (USE_BGFX == 1) then
			defines {
				"USE_BGFX"
			}
		end
	end
elseif _OPTIONS["osd"]=="osdmini" then
	defines {
		"OSD_MINI",
		"USE_QTDEBUG",
		"USE_SDL",
		"SDLMAME_NOASM=1",
		"USE_OPENGL=0",
	}
end
