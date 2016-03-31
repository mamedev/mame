-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team,Jeffrey Clark

local extlibs = {
--
--      3rdparty       system        3rdparty
--      lib name:      lib name,     include dir
--
	expat      = { "expat",     "3rdparty/expat/lib" },
	zlib       = { "z",         "3rdparty/zlib" },
	jpeg       = { "jpeg",      "3rdparty/libjpeg" },
	flac       = { "FLAC",      "3rdparty/libflac/include" },
	sqlite3    = { "sqlite3",   "3rdparty/sqlite3" },
	portmidi   = { "portmidi",  "3rdparty/portmidi/pm_common" },
	portaudio  = { "portaudio", "3rdparty/portaudio/include" },
	lua        = { "lua",       "3rdparty/lua/src" },
	uv         = { "uv" ,       "3rdparty/libuv/include" },
}

-- system lib options
newoption {
	trigger = 'with-system-expat',
	description = 'Use system Expat library',
}

newoption {
	trigger = 'with-system-zlib',
	description = 'Use system Zlib library',
}

newoption {
	trigger = 'with-system-jpeg',
	description = 'Use system JPEG library',
}

newoption {
	trigger = 'with-system-flac',
	description = 'Use system FLAC library',
}

newoption {
	trigger = 'with-system-sqlite3',
	description = 'Use system SQLite library',
}

newoption {
	trigger = 'with-system-portmidi',
	description = 'Use system PortMidi library',
}

newoption {
	trigger = 'with-system-portaudio',
	description = 'Use system PortAudio library',
}

newoption {
	trigger = "with-system-lua",
	description = "Use system LUA library",
}

newoption {
	trigger = 'with-system-uv',
	description = 'Use system uv library',
}

-- build helpers
function ext_lib(lib)
	local opt = _OPTIONS["with-system-" .. lib]
	if (opt~=nil and opt=="1") then
		default = extlibs[lib][1]
	else
		default = lib
	end
	return ext_best(lib, default, 1)
end

function ext_includedir(lib)
	local opt = _OPTIONS["with-system-" .. lib]
	if (opt==nil or opt=="0") then
		-- using bundled, prepend MAME_DIR
		default = MAME_DIR .. extlibs[lib][2]
	else
		default = ""
	end
	return ext_best(lib, default, 2)
end

function ext_best(lib, default, idx)
	local opt = _OPTIONS["with-system-" .. lib]
	local found = default
	if (opt~=nil and opt~="0" and opt~="1") then
		-- override default if provided (format <libname:includedir>)
		local x = opt:explode(":")
		if x[idx]~=nil then
			local y = x[idx]:explode(",")
			if y[1]~=nil then
				found = y
			else
				found = x[idx]
			end
		end
	end
	return found
end
