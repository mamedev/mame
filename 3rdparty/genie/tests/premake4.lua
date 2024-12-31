--
-- tests/premake4.lua
-- Automated test suite for Premake 4.x
-- Copyright (c) 2008-2011 Jason Perkins and the Premake project
--

	dofile("testfx.lua")

--
-- Some helper functions
--

	test.createsolution = function()
		local sln = solution "MySolution"
		configurations { "Debug", "Release" }

		local prj = project "MyProject"
		language "C++"
		kind "ConsoleApp"

		return sln, prj
	end


	test.createproject = function(sln)
		local n = #sln.projects + 1
		if n == 1 then n = "" end

		local prj = project ("MyProject" .. n)
		language "C++"
		kind "ConsoleApp"
		return prj
	end


--
-- The test suites
--

	dofile("test_dofile.lua")
	dofile("test_string.lua")
	dofile("test_premake.lua")
	dofile("test_platforms.lua")
	dofile("test_targets.lua")
	dofile("test_keywords.lua")
	dofile("test_gmake_cpp.lua")
	dofile("test_gmake_cs.lua")
	dofile("base/test_api.lua")
	dofile("base/test_action.lua")
	dofile("base/test_config.lua")
	dofile("base/test_location.lua")
	dofile("base/test_os.lua")
	dofile("base/test_path.lua")
	dofile("base/test_premake_command.lua")
	dofile("base/test_table.lua")
	dofile("base/test_tree.lua")
	dofile("tools/test_gcc.lua")
	dofile("base/test_config_bug.lua")

	-- Project API tests
	dofile("test_project.lua")
	dofile("project/test_eachfile.lua")
	dofile("project/test_vpaths.lua")

	-- Baking tests
	dofile("base/test_baking.lua")
	dofile("baking/test_merging.lua")

	-- Clean tests
	dofile("actions/test_clean.lua")

	-- Visual Studio tests
	dofile("actions/vstudio/test_vs200x_vcproj.lua")
	dofile("actions/vstudio/test_vs200x_vcproj_linker.lua")
	dofile("actions/vstudio/test_vs2010_vcxproj.lua")
	dofile("actions/vstudio/test_vs2010_flags.lua")
	dofile("actions/vstudio/test_vs2010_project_kinds.lua")

	-- Visual Studio 2002-2003 C# projects
	dofile("actions/vstudio/cs2002/test_files.lua")

	-- Visual Studio 2005-2010 C# projects
	dofile("actions/vstudio/cs2005/test_files.lua")
	dofile("actions/vstudio/cs2005/projectelement.lua")
	dofile("actions/vstudio/cs2005/projectsettings.lua")
	dofile("actions/vstudio/cs2005/propertygroup.lua")

	-- Visual Studio 2005-2010 solutions
	dofile("actions/vstudio/sln2005/dependencies.lua")
	dofile("actions/vstudio/sln2005/header.lua")
	dofile("actions/vstudio/sln2005/layout.lua")
	dofile("actions/vstudio/sln2005/platforms.lua")
	dofile("actions/vstudio/sln2005/projectplatforms.lua")
	dofile("actions/vstudio/sln2005/projects.lua")

	-- Visual Studio 2002-2008 C/C++ projects
	dofile("actions/vstudio/vc200x/debugdir.lua")
	dofile("actions/vstudio/vc200x/header.lua")
	dofile("actions/vstudio/vc200x/test_files.lua")
	dofile("actions/vstudio/vc200x/test_filters.lua")

	-- Visual Studio 2010 C/C++ projects
	dofile("actions/vstudio/vc2010/test_config_props.lua")
	dofile("actions/vstudio/vc2010/test_debugdir.lua")
	dofile("actions/vstudio/vc2010/test_header.lua")
	dofile("actions/vstudio/vc2010/test_files.lua")
	dofile("actions/vstudio/vc2010/test_filters.lua")
	dofile("actions/vstudio/vc2010/test_link_settings.lua")
	dofile("actions/vstudio/vc2010/test_links.lua")
	dofile("actions/vstudio/vc2010/test_output_props.lua")
	dofile("actions/vstudio/vc2010/test_pch.lua")
	dofile("actions/vstudio/vc2010/test_project_refs.lua")

	-- Makefile tests
	dofile("actions/make/test_make_escaping.lua")
	dofile("actions/make/test_make_pch.lua")
	dofile("actions/make/test_make_linking.lua")
	-- dofile("actions/make/test_makesettings.lua")
	dofile("actions/make/test_wiidev.lua")

--
-- Register a test action
--

	newoption {
		trigger     = "test",
		description = "A suite or test to run"
	}

	newaction {
		trigger     = "test",
		description = "Run the automated test suite",

		execute = function ()
			if _OPTIONS["test"] then
				local t = string.explode(_OPTIONS["test"] or "", ".", true)
				passed, failed = test.runall(t[1], t[2])
			else
				passed, failed = test.runall()
			end

			msg = string.format("%d tests passed, %d failed", passed, failed)
			if (failed > 0) then
				-- should probably return an error code here somehow
				print(msg)
			else
				print(msg)
			end
		end
	}
