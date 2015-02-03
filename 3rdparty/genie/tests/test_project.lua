--
-- tests/test_project.lua
-- Automated test suite for the project support functions.
-- Copyright (c) 2008-2010 Jason Perkins and the Premake project
--

	local _project = premake.project
	
	T.project = { }

	local cfg, result
	function T.project.setup()
		_ACTION = "gmake"
		cfg = {}
		cfg.project = {}
		cfg.language = "C++"
		cfg.files = {}
		cfg.trimpaths = {}
		cfg.platform = "Native"
		result = "\n"
	end



--
-- findproject() tests
--

	function T.project.findproject_IsCaseSensitive()
		local sln = test.createsolution()
		local prj = test.createproject(sln)
		premake.bake.buildconfigs()
		test.isnil(premake.findproject("myproject"))
	end
	
	
--
-- getfilename() tests
--

	function T.project.getfilename_ReturnsRelativePath()
		local prj = { name = "project", location = "location" }
		local r = _project.getfilename(prj, path.join(os.getcwd(), "../filename"))
		test.isequal("../filename", r)
	end
	
	function T.project.getfilename_PerformsSubstitutions()
		local prj = { name = "project", location = "location" }
		local r = _project.getfilename(prj, "%%.prj")
		test.isequal("location/project.prj", r)
	end



--
-- premake.getlinks() tests
--

	function T.project.getlinks_OnMscSystemLibs()
		_OPTIONS.cc = "msc"
		cfg.links = { "user32", "gdi32" }
		result = premake.getlinks(cfg, "all", "fullpath")
		test.isequal("user32.lib gdi32.lib", table.concat(result, " "))
	end
