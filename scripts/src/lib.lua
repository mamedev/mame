project "utils"
	uuid "22489ad0-4cb2-4d91-ad81-24b0d80ca30a"
	kind "StaticLib"
	
	options {
		"ForceCPP",
	}

	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		MAME_DIR .. "3rdparty/expat/lib",
		MAME_DIR .. "3rdparty/zlib",
	}

	includeosd()

	files {
		MAME_DIR .. "src/osd/osdcore.*",
		MAME_DIR .. "src/lib/util/**.c",
		MAME_DIR .. "src/lib/util/**.h",
	}

	
project "formats"
	uuid "f69636b1-fcce-45ce-b09a-113e371a2d7a"
	kind "StaticLib"

	options {
		"ForceCPP",
		"ArchiveSplit",
	}	

	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		MAME_DIR .. "3rdparty/zlib",
	}

	includeosd()
	
	files {
		MAME_DIR .. "src/lib/formats/**.c",
		MAME_DIR .. "src/lib/formats/**.h",
	}

