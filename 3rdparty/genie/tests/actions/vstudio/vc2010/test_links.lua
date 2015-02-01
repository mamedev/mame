--
-- tests/actions/vstudio/vc2010/test_links.lua
-- Validate linking and project references in Visual Studio 2010 C/C++ projects.
-- Copyright (c) 2011 Jason Perkins and the Premake project
--

	T.vstudio_vs2010_links = { }
	local suite = T.vstudio_vs2010_links
	local vc2010 = premake.vstudio.vc2010


--
-- Setup
--

	local sln, prj, prj2

	function suite.setup()
		os_uuid = os.uuid
		os.uuid = function() return "00112233-4455-6677-8888-99AABBCCDDEE" end

		sln = test.createsolution()
		test.createproject(sln)
	end

	local function prepare()
		premake.bake.buildconfigs()
		prj = premake.solution.getproject(sln, 1)
		prj2 = premake.solution.getproject(sln, 2)
		sln.vstudio_configs = premake.vstudio.buildconfigs(sln)
	end


--
-- If there are no sibling projects listed in links(), then the
-- entire project references item group should be skipped.
--

	function suite.noProjectReferencesGroup_onNoSiblingReferences()
		prepare()
		vc2010.projectReferences(prj2)
		test.isemptycapture()
	end


--
-- If a sibling project is listed in links(), an item group should
-- be written with a reference to that sibling project.
--

	function suite.projectReferenceAdded_onSiblingProjectLink()
		links { "MyProject" }
		prepare()
		vc2010.projectReferences(prj2)
		test.capture [[
	<ItemGroup>
		<ProjectReference Include="MyProject.vcproj">
			<Project>{00112233-4455-6677-8888-99AABBCCDDEE}</Project>
		</ProjectReference>
	</ItemGroup>
		]]
	end


--
-- If a sibling library is listed in links(), it should NOT appear in
-- the additional dependencies element. Visual Studio will figure that
-- out from the project reference item group.
--

	function suite.noDependencies_onOnlySiblingProjectLinks()
		links { "MyProject" }
		prepare()
		vc2010.additionalDependencies(prj2)
		test.isemptycapture()
	end


--
-- If a mix of sibling and system links are listed, only the system
-- libraries should appear in the additional dependencies element.
--

	function suite.onlySystemDependencies_OnSiblingProjectLink()
		links { "MyProject", "kernel32" }
		prepare()
		vc2010.additionalDependencies(prj2)
		test.capture [[
			<AdditionalDependencies>kernel32;%(AdditionalDependencies)</AdditionalDependencies>
		]]
	end


