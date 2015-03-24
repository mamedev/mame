--
-- tests/actions/xcode/test_xcode4_project.lua
-- Automated test suite for Xcode project generation.
-- Copyright (c) 2011 Jason Perkins and the Premake project
--

	T.xcode4_project = { }
	
	local suite = T.xcode4_project
	local xcode = premake.xcode


---------------------------------------------------------------------------
-- Setup/Teardown
---------------------------------------------------------------------------

	local sln, tr
	function suite.setup()
		_OS = "macosx"
		_ACTION = "xcode4"
		io.eol = "\n"
		xcode.used_ids = { } -- reset the list of generated IDs
		sln = test.createsolution()
	end

	local function prepare()
		premake.bake.buildconfigs()
		xcode.preparesolution(sln)
		local prj = premake.solution.getproject(sln, 1)
		tr = xcode.buildprjtree(prj)
	end


---------------------------------------------------------------------------
-- XCBuildConfiguration_Project tests
---------------------------------------------------------------------------

	function suite.XCBuildConfigurationProject_OnSymbols()
		flags { "Symbols" }
		prepare()
		xcode.XCBuildConfiguration_Project(tr, tr.configs[1])
		test.capture [[
		[MyProject:Debug(2)] /* Debug */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				ARCHS = "$(NATIVE_ARCH_ACTUAL)";
				CONFIGURATION_TEMP_DIR = "$(OBJROOT)";
				COPY_PHASE_STRIP = NO;
				GCC_C_LANGUAGE_STANDARD = gnu99;
				GCC_OPTIMIZATION_LEVEL = 0;
				GCC_SYMBOLS_PRIVATE_EXTERN = NO;
				GCC_WARN_ABOUT_RETURN_TYPE = YES;
				GCC_WARN_UNUSED_VARIABLE = YES;
				OBJROOT = "obj/Debug";
				ONLY_ACTIVE_ARCH = YES;
			};
			name = "Debug";
		};
		]]
	end
