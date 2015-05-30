--
-- tests/actions/vstudio/vc2010/test_debugdir.lua
-- Validate handling of the working directory for debugging.
-- Copyright (c) 2011 Jason Perkins and the Premake project
--

	T.vstudio_vs2010_debugdir = { }
	local suite = T.vstudio_vs2010_debugdir
	local vc2010 = premake.vstudio.vc2010


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
		vc2010.debugdir(prj)
	end


--
-- Tests
--

	function suite.PrintsNothing_OnDebugDirSet()
		prepare()
		test.capture [[
		]]
	end

	function suite.IsFormattedCorrectly_OnRelativePath()
		debugdir "bin/debug"
		prepare()
		test.capture [[
    <LocalDebuggerWorkingDirectory>bin\debug</LocalDebuggerWorkingDirectory>
    <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
		]]
	end

	function suite.Arguments()
		debugargs { "arg1", "arg2" }
		prepare()
		test.capture [[
    <LocalDebuggerCommandArguments>arg1 arg2</LocalDebuggerCommandArguments>
		]]
	end



	T.vs2010_debug_environment = { }
	local vs10_debug_environment = T.vs2010_debug_environment
	local vs2010 = premake.vstudio.vc2010

	function vs10_debug_environment.config_noDebugEnvsTable_bufferDoesNotContainLocalDebuggerEnvironment()
		vs2010.debugenvs( {flags={}} )
		test.string_does_not_contain(io.endcapture(),'<LocalDebuggerEnvironment>')
	end

	function vs10_debug_environment.config_NoneEmtpyDebugEnvTable_bufferContainsLocalDebuggerEnvironment()
		vs2010.debugenvs({flags={},debugenvs ={'key=value'}} )
		test.string_contains(io.endcapture(),'<LocalDebuggerEnvironment>')
	end

	function vs10_debug_environment.format_listContainsOneEntry_openTagKeyValuePairCloseTag()
		vs2010.debugenvs({flags={},debugenvs ={'key=value'}} )
		test.string_contains(io.endcapture(),'<LocalDebuggerEnvironment>key=value</LocalDebuggerEnvironment>')
	end
	
	function vs10_debug_environment.format_listContainsTwoEntries_openTagFirstPairNewLineSecondPairCloseTag()
		vs2010.debugenvs({flags={},debugenvs ={'key=value','foo=bar'}} )
		test.string_contains(io.endcapture(),'<LocalDebuggerEnvironment>key=value\nfoo=bar</LocalDebuggerEnvironment>')
	end

	function vs10_debug_environment.flags_withOutEnvironmentArgsInherit_doesNotContainLocalDebuggerEnvironmentArg()
		vs2010.debugenvs({flags={},environmentargs ={'key=value'}} )
		test.string_does_not_contain(io.endcapture(),'%$%(LocalDebuggerEnvironment%)')
	end

	function vs10_debug_environment.flags_withDebugEnvsInherit_endsWithNewLineLocalDebuggerEnvironmentFollowedByClosedTag()
		vs2010.debugenvs({flags={DebugEnvsInherit=1},debugenvs ={'key=value'}} )
		test.string_contains(io.endcapture(),'\n%$%(LocalDebuggerEnvironment%)</LocalDebuggerEnvironment>')
	end
	
	function vs10_debug_environment.flags_withDebugEnvsDontMerge_localDebuggerMergeEnvironmentSetToFalse()
		vs2010.debugenvs({flags={DebugEnvsDontMerge=1},debugenvs ={'key=value'}} )
		test.string_contains(io.endcapture(),'<LocalDebuggerMergeEnvironment>false</LocalDebuggerMergeEnvironment>')
	end
