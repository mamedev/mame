--
-- Premake 4.x build configuration script
--

--
-- Define the project. Put the release configuration first so it will be the
-- default when folks build using the makefile. That way they don't have to
-- worry about the /scripts argument and all that.
--
	premake.make.override = { "TARGET" }

	solution "genie"
		configurations {
			"Release",
			"Debug"
		}
		location (_OPTIONS["to"])

	project "genie"
		targetname "genie"
		language "C"
		kind "ConsoleApp"
		flags {
			"ExtraWarnings",
			"No64BitChecks",
			"StaticRuntime"
		}

		includedirs {
			"../src/host/lua-5.3.0/src"
		}

		files {
			"../**.lua",
			"../src/**.h",
			"../src/**.c",
			"../src/host/scripts.c",
		}

		excludes {
			"../src/premake.lua",
			"../src/host/lua-5.3.0/src/lua.c",
			"../src/host/lua-5.3.0/src/luac.c",
			"../src/host/lua-5.3.0/**.lua",
			"../src/host/lua-5.3.0/etc/*.c",
		}

		buildoptions {
			"-m64",
		}

		configuration "Debug"
			defines     { "_DEBUG", "LUA_COMPAT_MODULE" }
			flags       { "Symbols" }

		configuration "Release"
			defines     { "NDEBUG", "LUA_COMPAT_MODULE" }
			flags       { "OptimizeSize" }

		configuration "vs*"
			defines     { "_CRT_SECURE_NO_WARNINGS" }

		configuration "windows"
			targetdir   "../bin/windows"
			links { "ole32" }

		configuration "linux"
			targetdir   "../bin/linux"
			links       { "dl" }

		configuration "bsd"
			targetdir   "../bin/bsd"

		configuration "linux or bsd"
			defines      { "LUA_USE_POSIX", "LUA_USE_DLOPEN" }
			buildoptions { "-Wno-implicit-fallthrough" }
			links        { "m" }
			linkoptions  { "-rdynamic" }

		configuration "linux"
			define       { "_FILE_OFFSET_BITS=64" }

		configuration "macosx"
			targetdir   "../bin/darwin"
			defines     { "LUA_USE_MACOSX" }
			links       { "CoreServices.framework" }

		configuration { "macosx", "gmake" }
			buildoptions { "-mmacosx-version-min=10.6" }
			linkoptions  { "-mmacosx-version-min=10.6" }

		configuration {}


--
-- A more thorough cleanup.
--

	if _ACTION == "clean" then
		os.rmdir("bin")
		os.rmdir("build")
	end



--
-- Use the --to=path option to control where the project files get generated. I use
-- this to create project files for each supported toolset, each in their own folder,
-- in preparation for deployment.
--

	newoption {
		trigger = "to",
		value   = "path",
		description = "Set the output location for the generated files"
	}



--
-- Use the embed action to convert all of the Lua scripts into C strings, which
-- can then be built into the executable. Always embed the scripts before creating
-- a release build.
--

	dofile("embed.lua")

	newaction {
		trigger     = "embed",
		description = "Embed scripts in scripts.c; required before release builds",
		execute     = doembed
	}


--
-- Use the release action to prepare source and binary packages for a new release.
-- This action isn't complete yet; a release still requires some manual work.
--


	dofile("release.lua")

	newaction {
		trigger     = "release",
		description = "Prepare a new release (incomplete)",
		execute     = dorelease
	}
