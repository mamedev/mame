--
-- tests/actions/vstudio/vc200x/header.lua
-- Validate generation of the project file header block.
-- Copyright (c) 2011 Jason Perkins and the Premake project
--

	T.vstudio_vs200x_header = { }
	local suite = T.vstudio_vs200x_header
	local vc200x = premake.vstudio.vc200x


--
-- Setup 
--

	local sln, prj
	
	function suite.setup()
		sln = test.createsolution()
	end
	
	local function prepare()
		premake.bake.buildconfigs()
		prj = premake.solution.getproject(sln, 1)
		sln.vstudio_configs = premake.vstudio.buildconfigs(sln)
		vc200x.header('VisualStudioProject')
	end


--
-- Tests
--

	function suite.On2008()
		_ACTION = 'vs2008'
		prepare()
		test.capture [[
<?xml version="1.0" encoding="Windows-1252"?>
<VisualStudioProject
	ProjectType="Visual C++"
	Version="9.00"
		]]
	end
