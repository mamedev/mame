--
-- tests/actions/make/test_make_linking.lua
-- Validate library references in makefiles.
-- Copyright (c) 2010-2013 Jason Perkins and the Premake project
--

	T.gcc_linking = {}
	local suite = T.gcc_linking
	local cpp = premake.make.cpp

--
-- Setup
--

	local sln, prj

	function suite.setup()
		_OS = "linux"
		sln, prj = test.createsolution()
	end

	local function prepare()
		premake.bake.buildconfigs()
		cfg = premake.getconfig(prj, "Debug")
		cpp.linker(cfg, premake.gcc)
	end


--
-- Check linking to a shared library sibling project. In order to support
-- custom target prefixes and extensions, use the full, relative path
-- to the library.
--

	function suite.onSharedLibrarySibling()
		links { "MyProject2" }
		test.createproject(sln)
		kind "SharedLib"
		targetdir "libs"
		prepare()
		test.capture [[
  ALL_LDFLAGS   += $(LDFLAGS) -Llibs -s
  LDDEPS    += libs/libMyProject2.so
  LIBS      += $(LDDEPS)
		]]
	end


--
-- Check linking to a static library sibling project. As with shared
-- libraries, it should list out the full relative path.
--

	function suite.onStaticLibrarySibling()
		links { "MyProject2" }
		test.createproject(sln)
		kind "StaticLib"
		targetdir "libs"
		prepare()
		test.capture [[
  ALL_LDFLAGS   += $(LDFLAGS) -Llibs -s
  LDDEPS    += libs/libMyProject2.a
  LIBS      += $(LDDEPS)
		]]
	end


--
-- If an executable is listed in the links, no linking should happen (a
-- build dependency would have been created at the solution level)
--

	function suite.onConsoleAppSibling()
		links { "MyProject2" }
		test.createproject(sln)
		kind "ConsoleApp"
		targetdir "libs"
		prepare()
		test.capture [[
  ALL_LDFLAGS   += $(LDFLAGS) -s
  LDDEPS    +=
  LIBS      += $(LDDEPS)
		]]
	end


--
-- Make sure that project locations are taken into account when building
-- the path to the library.
--


	function suite.onProjectLocations()
		location "MyProject"
		links { "MyProject2" }

		test.createproject(sln)
		kind "SharedLib"
		location "MyProject2"
		targetdir "MyProject2"

		prepare()
		test.capture [[
  ALL_LDFLAGS   += $(LDFLAGS) -L../MyProject2 -s
  LDDEPS    += ../MyProject2/libMyProject2.so
  LIBS      += $(LDDEPS)
		]]
	end


--
-- When referencing an external library via a path, the directory
-- should be added to the library search paths, and the library
-- itself included via an -l flag.
--

 function suite.onExternalLibraryWithPath()
 	location "MyProject"
	links { "libs/SomeLib" }
	prepare()
	test.capture [[
  ALL_LDFLAGS   += $(LDFLAGS) -L../libs -s
  LDDEPS    +=
  LIBS      += $(LDDEPS) -lSomeLib
	]]
 end
