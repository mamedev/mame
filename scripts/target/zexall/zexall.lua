STANDALONE = true

CPUS["Z80"] = true

MACHINES["Z80DAISY"] = true

function standalone()
	files{
		MAME_DIR .. "src/zexall/main.cpp",
		MAME_DIR .. "src/zexall/zexall.cpp",
		MAME_DIR .. "src/zexall/zexall.h",
		MAME_DIR .. "src/zexall/interface.h",
	}
end

