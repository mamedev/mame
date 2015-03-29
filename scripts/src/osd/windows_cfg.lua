forcedincludes {
	MAME_DIR .. "src/osd/windows/winprefix.h"
}

defines {
	"UNICODE",
	"_UNICODE",
	"OSD_WINDOWS",
	"USE_SDL=0",
	"USE_QTDEBUG=0",
	"USE_OPENGL=1",
	"USE_DISPATCH_GL=1",
	"DIRECTINPUT_VERSION=0x0800"
}
