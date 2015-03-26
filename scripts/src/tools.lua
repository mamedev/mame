--------------------------------------------------
-- romcmp
--------------------------------------------------

project("romcmp")
uuid ("1b40275b-194c-497b-8abd-9338775a21b8")
kind "ConsoleApp"	

options {
	"ForceCPP",
}

configuration { }
	targetdir(MAME_DIR)

links {
	"utils",
	"expat",
	"zlib",
	"ocore_" .. _OPTIONS["osd"],
}

includedirs {
	MAME_DIR .. "src/lib/util",
}

includeosd()

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

configuration { }
	targetdir(MAME_DIR)

links {
	"utils",
	"expat",
	"zlib",
	"flac",
	"7z",
	"ocore_" .. _OPTIONS["osd"],
}

includedirs {
	MAME_DIR .. "src/lib/util",
	MAME_DIR .. "3rdparty",
}

includeosd()

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

configuration { }
	targetdir(MAME_DIR)

links {
	"utils",
	"expat",
	"zlib",
	"ocore_" .. _OPTIONS["osd"],
}

includedirs {
	MAME_DIR .. "src/lib/util",
}

includeosd()

files {
	MAME_DIR .. "src/tools/jedutil.c",
}

--------------------------------------------------
-- unidasm
--------------------------------------------------

--------------------------------------------------
-- ldresample
--------------------------------------------------

project("ldresample")
uuid ("3401561a-4407-4e13-9c6d-c0801330f7cc")
kind "ConsoleApp"	

options {
	"ForceCPP",
}

configuration { }
	targetdir(MAME_DIR)

links {
	"utils",
	"expat",
	"zlib",
	"flac",
	"7z",	
	"ocore_" .. _OPTIONS["osd"],
}

includedirs {
	MAME_DIR .. "src/lib/util",
	MAME_DIR .. "3rdparty",
}

includeosd()

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

configuration { }
	targetdir(MAME_DIR)

links {
	"utils",
	"expat",
	"zlib",
	"flac",
	"7z",	
	"ocore_" .. _OPTIONS["osd"],
}

includedirs {
	MAME_DIR .. "src/lib/util",
	MAME_DIR .. "3rdparty",
}

includeosd()

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

configuration { }
	targetdir(MAME_DIR)

links {
	"utils",
	"expat",
	"zlib",
	"ocore_" .. _OPTIONS["osd"],
}

includedirs {
	MAME_DIR .. "src/lib/util",
}

includeosd()

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

configuration { }
	targetdir(MAME_DIR)

links {
	"utils",
	"expat",
	"zlib",
	"ocore_" .. _OPTIONS["osd"],
}

includedirs {
	MAME_DIR .. "src/lib/util",
}

includeosd()

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

configuration { }
	targetdir(MAME_DIR)

links {
	"utils",
	"expat",
	"zlib",
	"ocore_" .. _OPTIONS["osd"],
}

includedirs {
	MAME_DIR .. "src/lib/util",
}

includeosd()

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

configuration { }
	targetdir(MAME_DIR)

links {
	"utils",
	"expat",
	"zlib",
	"flac",
	"7z",
	"ocore_" .. _OPTIONS["osd"],
}

includedirs {
	MAME_DIR .. "src/lib/util",
}

includeosd()

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

configuration { }
	targetdir(MAME_DIR)

links {
	"utils",
	"expat",
	"zlib",
	"ocore_" .. _OPTIONS["osd"],
}

includedirs {
	MAME_DIR .. "src/lib/util",
}

includeosd()

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

configuration { }
	targetdir(MAME_DIR)

links {
	"utils",
	"expat",
	"zlib",
	"flac",
	"7z",
	"ocore_" .. _OPTIONS["osd"],
}

includedirs {
	MAME_DIR .. "src/lib/util",
	MAME_DIR .. "src/emu",
}

includeosd()

files {
	MAME_DIR .. "src/tools/nltool.c",
	MAME_DIR .. "src/emu/netlist/**.*",
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

configuration { }
	targetdir(MAME_DIR)

links {
	"formats",
	"utils",
	"expat",
	"zlib",
	"flac",
	"7z",
	"ocore_" .. _OPTIONS["osd"],
}

includedirs {
	MAME_DIR .. "src/lib",	
	MAME_DIR .. "src/lib/util",
}

includeosd()

files {
	MAME_DIR .. "src/mess/tools/castool/main.c",
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

configuration { }
	targetdir(MAME_DIR)

links {
	"formats",
	"emu",
	"utils",
	"expat",
	"zlib",
	"flac",
	"7z",
	"ocore_" .. _OPTIONS["osd"],
}

includedirs {
	MAME_DIR .. "src/lib",	
	MAME_DIR .. "src/lib/util",
}

includeosd()

files {
	MAME_DIR .. "src/mess/tools/floptool/main.c",
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

configuration { }
	targetdir(MAME_DIR)

links {
	"formats",
	"emu",
	"utils",
	"expat",
	"zlib",
	"flac",
	"7z",
	"ocore_" .. _OPTIONS["osd"],
}

includedirs {
	MAME_DIR .. "src/lib",	
	MAME_DIR .. "src/lib/util",
	MAME_DIR .. "3rdparty/zlib",
	MAME_DIR .. "src/mess/tools/imgtool",	
}

includeosd()

files {
	MAME_DIR .. "src/mess/tools/imgtool/**.*",
}

