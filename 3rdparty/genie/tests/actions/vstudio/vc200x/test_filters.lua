--
-- tests/actions/vstudio/vc200x/test_filters.lua
-- Validate generation of filter blocks in Visual Studio 200x C/C++ projects.
-- Copyright (c) 2011 Jason Perkins and the Premake project
--

T.vs200x_filters = { }
local suite = T.vs200x_filters
local vc200x = premake.vstudio.vc200x


--
-- Setup/teardown
--

	local sln, prj
	local os_uuid

	function suite.setup()
		os_uuid = os.uuid
		os.uuid = function() return "00112233-4455-6677-8888-99AABBCCDDEE" end

		_ACTION = "vs2008"
		sln, prj = test.createsolution()
	end

	function suite.teardown()
		os.uuid = os_uuid
	end

	local function prepare()
		premake.bake.buildconfigs()
		sln.vstudio_configs = premake.vstudio.buildconfigs(sln)
		prj = premake.solution.getproject(sln,1)
	end


--
-- File/filter assignment tests
--

	function suite.Filter_UsesVirtualForm_OnVpath()
		files { "src/hello.cpp" }
		vpaths { ["Source Files"] = "**.cpp" }		
		prepare()
		vc200x.Files(prj)
		test.capture [[
		<Filter
			Name="Source Files"
			Filter=""
			>
			<File
				RelativePath="src\hello.cpp"
				>
			</File>
		</Filter>
		]]
	end
