-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

--------------------------------------------------
-- romcmp
--------------------------------------------------

project("romcmp")
uuid ("1b40275b-194c-497b-8abd-9338775a21b8")
kind "ConsoleApp"	

options {
	"ForceCPP",
}

flags {
	"Symbols", -- always include minimum symbols for executables 	
}

if _OPTIONS["SEPARATE_BIN"]~="1" then 
	targetdir(MAME_DIR)
end

links {
	"utils",
	"expat",
	"ocore_" .. _OPTIONS["osd"],
}

if _OPTIONS["with-bundled-zlib"] then
	links {
		"zlib",
	}
else
	links {
		"z",
	}
end

includedirs {
	MAME_DIR .. "src/osd",
	MAME_DIR .. "src/lib/util",
}

files {
	MAME_DIR .. "src/tools/romcmp.c",
}

--------------------------------------------------
-- chdman
--------------------------------------------------

project("chdman")
uuid ("7d948868-42db-432a-9bb5-70ce5c5f4620")
kind "ConsoleApp"	

options {
	"ForceCPP",
}

flags {
	"Symbols", -- always include minimum symbols for executables 	
}

if _OPTIONS["SEPARATE_BIN"]~="1" then 
	targetdir(MAME_DIR)
end

links {
	"utils",
	"expat",
	"7z",
	"ocore_" .. _OPTIONS["osd"],
}

if _OPTIONS["with-bundled-zlib"] then
	links {
		"zlib",
	}
else
	links {
		"z",
	}
end

if _OPTIONS["with-bundled-flac"] then
	links {
		"flac",
	}
else
	links {
		"FLAC",
	}
end

includedirs {
	MAME_DIR .. "src/osd",
	MAME_DIR .. "src/lib/util",
	MAME_DIR .. "3rdparty",
}

files {
	MAME_DIR .. "src/tools/chdman.c",
	MAME_DIR .. "src/version.c",
}

--------------------------------------------------
-- jedutil
--------------------------------------------------

project("jedutil")
uuid ("bda60edb-f7f5-489f-b232-23d33c43dda1")
kind "ConsoleApp"	

options {
	"ForceCPP",
}

flags {
	"Symbols", -- always include minimum symbols for executables 	
}

if _OPTIONS["SEPARATE_BIN"]~="1" then 
	targetdir(MAME_DIR)
end

links {
	"utils",
	"expat",
	"ocore_" .. _OPTIONS["osd"],
}

if _OPTIONS["with-bundled-zlib"] then
	links {
		"zlib",
	}
else
	links {
		"z",
	}
end

includedirs {
	MAME_DIR .. "src/osd",
	MAME_DIR .. "src/lib/util",
}

files {
	MAME_DIR .. "src/tools/jedutil.c",
}

--------------------------------------------------
-- unidasm
--------------------------------------------------

project("unidasm")
uuid ("65f81d3b-299a-4b08-a3fa-d5241afa9fd1")
kind "ConsoleApp"	

options {
	"ForceCPP",
}

flags {
	"Symbols", -- always include minimum symbols for executables 	
}

if _OPTIONS["SEPARATE_BIN"]~="1" then 
	targetdir(MAME_DIR)
end

links {
	"dasm",
	"emu",
	"utils",
	"expat",
	"7z",	
	"ocore_" .. _OPTIONS["osd"],
}

if _OPTIONS["with-bundled-zlib"] then
	links {
		"zlib",
	}
else
	links {
		"z",
	}
end

if _OPTIONS["with-bundled-flac"] then
	links {
		"flac",
	}
else
	links {
		"FLAC",
	}
end

includedirs {
	MAME_DIR .. "src/osd",
	MAME_DIR .. "src/emu",
	MAME_DIR .. "src/lib/util",
	MAME_DIR .. "3rdparty",
}

files {
	MAME_DIR .. "src/tools/unidasm.c",
}


--------------------------------------------------
-- ldresample
--------------------------------------------------

project("ldresample")
uuid ("3401561a-4407-4e13-9c6d-c0801330f7cc")
kind "ConsoleApp"	

options {
	"ForceCPP",
}

flags {
	"Symbols", -- always include minimum symbols for executables 	
}

if _OPTIONS["SEPARATE_BIN"]~="1" then 
	targetdir(MAME_DIR)
end

links {
	"utils",
	"expat",
	"7z",	
	"ocore_" .. _OPTIONS["osd"],
}

if _OPTIONS["with-bundled-zlib"] then
	links {
		"zlib",
	}
else
	links {
		"z",
	}
end

if _OPTIONS["with-bundled-flac"] then
	links {
		"flac",
	}
else
	links {
		"FLAC",
	}
end

includedirs {
	MAME_DIR .. "src/osd",
	MAME_DIR .. "src/lib/util",
	MAME_DIR .. "3rdparty",
}

files {
	MAME_DIR .. "src/tools/ldresample.c",
}

--------------------------------------------------
-- ldverify
--------------------------------------------------

project("ldverify")
uuid ("3e66560d-b928-4227-928b-eadd0a10f00a")
kind "ConsoleApp"	

options {
	"ForceCPP",
}

flags {
	"Symbols", -- always include minimum symbols for executables 	
}

if _OPTIONS["SEPARATE_BIN"]~="1" then 
	targetdir(MAME_DIR)
end

links {
	"utils",
	"expat",
	"7z",	
	"ocore_" .. _OPTIONS["osd"],
}

if _OPTIONS["with-bundled-zlib"] then
	links {
		"zlib",
	}
else
	links {
		"z",
	}
end

if _OPTIONS["with-bundled-flac"] then
	links {
		"flac",
	}
else
	links {
		"FLAC",
	}
end

includedirs {
	MAME_DIR .. "src/osd",
	MAME_DIR .. "src/lib/util",
	MAME_DIR .. "3rdparty",
}

files {
	MAME_DIR .. "src/tools/ldverify.c",
}

--------------------------------------------------
-- regrep
--------------------------------------------------

project("regrep")
uuid ("7f6de580-d800-4e8d-bed6-9fc86829584d")
kind "ConsoleApp"	

options {
	"ForceCPP",
}

flags {
	"Symbols", -- always include minimum symbols for executables 	
}

if _OPTIONS["SEPARATE_BIN"]~="1" then 
	targetdir(MAME_DIR)
end

links {
	"utils",
	"expat",
	"ocore_" .. _OPTIONS["osd"],
}

if _OPTIONS["with-bundled-zlib"] then
	links {
		"zlib",
	}
else
	links {
		"z",
	}
end

includedirs {
	MAME_DIR .. "src/osd",
	MAME_DIR .. "src/lib/util",
}

files {
	MAME_DIR .. "src/tools/regrep.c",
}

--------------------------------------------------
-- srcclean
---------------------------------------------------

project("srcclean")
uuid ("4dd58139-313a-42c5-965d-f378bdeed220")
kind "ConsoleApp"	

options {
	"ForceCPP",
}

flags {
	"Symbols", -- always include minimum symbols for executables 	
}

if _OPTIONS["SEPARATE_BIN"]~="1" then 
	targetdir(MAME_DIR)
end

links {
	"utils",
	"expat",
	"ocore_" .. _OPTIONS["osd"],
}

if _OPTIONS["with-bundled-zlib"] then
	links {
		"zlib",
	}
else
	links {
		"z",
	}
end

includedirs {
	MAME_DIR .. "src/osd",
	MAME_DIR .. "src/lib/util",
}

files {
	MAME_DIR .. "src/tools/srcclean.c",
}

--------------------------------------------------
-- src2html
--------------------------------------------------

project("src2html")
uuid ("b31e963a-09ef-4696-acbd-e663e35ce6f7")
kind "ConsoleApp"	

options {
	"ForceCPP",
}

flags {
	"Symbols", -- always include minimum symbols for executables 	
}

if _OPTIONS["SEPARATE_BIN"]~="1" then 
	targetdir(MAME_DIR)
end

links {
	"utils",
	"expat",
	"ocore_" .. _OPTIONS["osd"],
}

if _OPTIONS["with-bundled-zlib"] then
	links {
		"zlib",
	}
else
	links {
		"z",
	}
end

includedirs {
	MAME_DIR .. "src/osd",
	MAME_DIR .. "src/lib/util",
}

files {
	MAME_DIR .. "src/tools/src2html.c",
}

--------------------------------------------------
-- split
--------------------------------------------------

project("split")
uuid ("8ef6ff18-3199-4cc2-afd0-d64033070faa")
kind "ConsoleApp"	

options {
	"ForceCPP",
}

flags {
	"Symbols", -- always include minimum symbols for executables 	
}

if _OPTIONS["SEPARATE_BIN"]~="1" then 
	targetdir(MAME_DIR)
end

links {
	"utils",
	"expat",
	"7z",
	"ocore_" .. _OPTIONS["osd"],
}

if _OPTIONS["with-bundled-zlib"] then
	links {
		"zlib",
	}
else
	links {
		"z",
	}
end

if _OPTIONS["with-bundled-flac"] then
	links {
		"flac",
	}
else
	links {
		"FLAC",
	}
end

includedirs {
	MAME_DIR .. "src/osd",
	MAME_DIR .. "src/lib/util",
}

files {
	MAME_DIR .. "src/tools/split.c",
}

--------------------------------------------------
-- pngcmp
--------------------------------------------------

project("pngcmp")
uuid ("61f647d9-b129-409b-9c62-8acf98ed39be")
kind "ConsoleApp"	

options {
	"ForceCPP",
}

flags {
	"Symbols", -- always include minimum symbols for executables 	
}

if _OPTIONS["SEPARATE_BIN"]~="1" then 
	targetdir(MAME_DIR)
end

links {
	"utils",
	"expat",
	"ocore_" .. _OPTIONS["osd"],
}

if _OPTIONS["with-bundled-zlib"] then
	links {
		"zlib",
	}
else
	links {
		"z",
	}
end

includedirs {
	MAME_DIR .. "src/osd",
	MAME_DIR .. "src/lib/util",
}

files {
	MAME_DIR .. "src/tools/pngcmp.c",
}

--------------------------------------------------
-- nltool
--------------------------------------------------

project("nltool")
uuid ("853a03b7-fa37-41a8-8250-0dc23dd935d6")
kind "ConsoleApp"	

options {
	"ForceCPP",
}

flags {
	"Symbols", -- always include minimum symbols for executables 	
}

if _OPTIONS["SEPARATE_BIN"]~="1" then 
	targetdir(MAME_DIR)
end

links {
	"utils",
	"expat",
	"7z",
	"ocore_" .. _OPTIONS["osd"],
	"netlist",
}

if _OPTIONS["with-bundled-zlib"] then
	links {
		"zlib",
	}
else
	links {
		"z",
	}
end

if _OPTIONS["with-bundled-flac"] then
	links {
		"flac",
	}
else
	links {
		"FLAC",
	}
end

includedirs {
	MAME_DIR .. "src/osd",
	MAME_DIR .. "src/lib/util",
	MAME_DIR .. "src/lib/netlist",
}

files {
	MAME_DIR .. "src/lib/netlist/prg/nltool.c",
}

--------------------------------------------------
-- nlwav
--------------------------------------------------

project("nlwav")
uuid ("7c5396d1-2a1a-4c93-bed6-6b8fa182054a")
kind "ConsoleApp" 

options {
  "ForceCPP",
}

flags {
  "Symbols", -- always include minimum symbols for executables  
}

if _OPTIONS["SEPARATE_BIN"]~="1" then 
  targetdir(MAME_DIR)
end

links {
  "utils",
  "ocore_" .. _OPTIONS["osd"],
  "netlist",
}

includedirs {
  MAME_DIR .. "src/osd",
  MAME_DIR .. "src/lib/util",
  MAME_DIR .. "src/lib/netlist",
}

files {
  MAME_DIR .. "src/lib/netlist/prg/nlwav.c",
}

--------------------------------------------------
-- castool
--------------------------------------------------

project("castool")
uuid ("7d9ed428-e2ba-4448-832d-d882a64d5c22")
kind "ConsoleApp"	

options {
	"ForceCPP",
}

flags {
	"Symbols", -- always include minimum symbols for executables 	
}

if _OPTIONS["SEPARATE_BIN"]~="1" then 
	targetdir(MAME_DIR)
end

links {
	"formats",
	"utils",
	"expat",
	"7z",
	"ocore_" .. _OPTIONS["osd"],
}

if _OPTIONS["with-bundled-zlib"] then
	links {
		"zlib",
	}
else
	links {
		"z",
	}
end

if _OPTIONS["with-bundled-flac"] then
	links {
		"flac",
	}
else
	links {
		"FLAC",
	}
end

includedirs {
	MAME_DIR .. "src/osd",
	MAME_DIR .. "src/lib",	
	MAME_DIR .. "src/lib/util",
}

files {
	MAME_DIR .. "src/mame/tools/castool/main.c",
}

--------------------------------------------------
-- floptool
--------------------------------------------------

project("floptool")
uuid ("85d8e3a6-1661-4ac9-8c21-281d20cbaf5b")
kind "ConsoleApp"	

options {
	"ForceCPP",
}

flags {
	"Symbols", -- always include minimum symbols for executables 	
}

if _OPTIONS["SEPARATE_BIN"]~="1" then 
	targetdir(MAME_DIR)
end

links {
	"formats",
	"emu",
	"utils",
	"expat",
	"7z",
	"ocore_" .. _OPTIONS["osd"],
}

if _OPTIONS["with-bundled-zlib"] then
	links {
		"zlib",
	}
else
	links {
		"z",
	}
end

if _OPTIONS["with-bundled-flac"] then
	links {
		"flac",
	}
else
	links {
		"FLAC",
	}
end

includedirs {
	MAME_DIR .. "src/osd",
	MAME_DIR .. "src/lib",	
	MAME_DIR .. "src/lib/util",
}

files {
	MAME_DIR .. "src/mame/tools/floptool/main.c",
}

--------------------------------------------------
-- imgtool
--------------------------------------------------

project("imgtool")
uuid ("f3707807-e587-4297-a5d8-bc98f3d0b1ca")
kind "ConsoleApp"	

options {
	"ForceCPP",
}

flags {
	"Symbols", -- always include minimum symbols for executables 	
}

if _OPTIONS["SEPARATE_BIN"]~="1" then 
	targetdir(MAME_DIR)
end

links {
	"formats",
	"emu",
	"utils",
	"expat",
	"7z",
	"ocore_" .. _OPTIONS["osd"],
}

if _OPTIONS["with-bundled-zlib"] then
	links {
		"zlib",
	}
else
	links {
		"z",
	}
end

if _OPTIONS["with-bundled-flac"] then
	links {
		"flac",
	}
else
	links {
		"FLAC",
	}
end

includedirs {
	MAME_DIR .. "src/osd",
	MAME_DIR .. "src/lib",	
	MAME_DIR .. "src/lib/util",
	MAME_DIR .. "3rdparty/zlib",
	MAME_DIR .. "src/mame/tools/imgtool",	
}

files {
	MAME_DIR .. "src/mame/tools/imgtool/main.c",
	MAME_DIR .. "src/mame/tools/imgtool/stream.c",
	MAME_DIR .. "src/mame/tools/imgtool/library.c",
	MAME_DIR .. "src/mame/tools/imgtool/modules.c",
	MAME_DIR .. "src/mame/tools/imgtool/iflopimg.c",
	MAME_DIR .. "src/mame/tools/imgtool/filter.c",
	MAME_DIR .. "src/mame/tools/imgtool/filteoln.c",
	MAME_DIR .. "src/mame/tools/imgtool/filtbas.c",
	MAME_DIR .. "src/mame/tools/imgtool/imgtool.c",
	MAME_DIR .. "src/mame/tools/imgtool/imgterrs.c",
	MAME_DIR .. "src/mame/tools/imgtool/imghd.c", 
	MAME_DIR .. "src/mame/tools/imgtool/charconv.c",
	MAME_DIR .. "src/mame/tools/imgtool/formats/vt_dsk.c",
	MAME_DIR .. "src/mame/tools/imgtool/formats/vt_dsk.h",
	MAME_DIR .. "src/mame/tools/imgtool/formats/coco_dsk.c",
	MAME_DIR .. "src/mame/tools/imgtool/formats/coco_dsk.h",	
	MAME_DIR .. "src/mame/tools/imgtool/modules/amiga.c",
	MAME_DIR .. "src/mame/tools/imgtool/modules/macbin.c",
	MAME_DIR .. "src/mame/tools/imgtool/modules/rsdos.c",
	MAME_DIR .. "src/mame/tools/imgtool/modules/os9.c",
	MAME_DIR .. "src/mame/tools/imgtool/modules/mac.c",
	MAME_DIR .. "src/mame/tools/imgtool/modules/ti99.c", 
	MAME_DIR .. "src/mame/tools/imgtool/modules/ti990hd.c",
	MAME_DIR .. "src/mame/tools/imgtool/modules/concept.c",
	MAME_DIR .. "src/mame/tools/imgtool/modules/fat.c",
	MAME_DIR .. "src/mame/tools/imgtool/modules/pc_flop.c",
	MAME_DIR .. "src/mame/tools/imgtool/modules/pc_hard.c",
	MAME_DIR .. "src/mame/tools/imgtool/modules/prodos.c",
	MAME_DIR .. "src/mame/tools/imgtool/modules/vzdos.c",
	MAME_DIR .. "src/mame/tools/imgtool/modules/thomson.c",
	MAME_DIR .. "src/mame/tools/imgtool/modules/macutil.c",
	MAME_DIR .. "src/mame/tools/imgtool/modules/cybiko.c",
	MAME_DIR .. "src/mame/tools/imgtool/modules/cybikoxt.c",
	MAME_DIR .. "src/mame/tools/imgtool/modules/psion.c",
	MAME_DIR .. "src/mame/tools/imgtool/modules/bml3.c",
	MAME_DIR .. "src/mame/tools/imgtool/modules/hp48.c",
}
