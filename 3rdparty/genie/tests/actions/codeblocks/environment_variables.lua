--
-- tests/actions/codeblocks/environment_variables.lua
-- Validate generation of files block in CodeLite C/C++ projects.
-- Copyright (c) 2011 Jason Perkins and the Premake project
--

	T.codeblocks_environment = { }
	local suite = T.codeblocks_environment
	local codeblocks = premake.codeblocks

	local old_eol = io.eol
	function suite.setup()
		_OPTIONS.cc = 'gcc'
		old_eol = io.eol
		io.eol = '\n'
	end

	function suite.teardown()
		io.eol = old_eol
	end

	function suite.withVar_bufferContainsDebugger()
		local MockName = 'MockName'
		codeblocks.debugenvs( {debugenvs = {'foo=bar'},language='C',longname=MockName} )
		test.string_contains(io.endcapture(),
												'\t\t\t<debugger>\n' ..
												'\t\t\t\t<remote_debugging target="'..MockName..'">\n' ..
												'\t\t\t\t\t<options additional_cmds_before=.- />\n' ..
												'\t\t\t\t</remote_debugging>\n' ..
												'\t\t\t</debugger>'
							)
	end

	function suite.format_SetEnvKeyValuePair()
		local env_arg = 'foo=bar'
		codeblocks.debugenvs( {debugenvs = {env_arg},language='C',longname='DontCare'} )
		test.string_contains(io.endcapture(),'<options additional_cmds_before="set env '..env_arg..'" />')
	end

	function suite.format_mutipleValues_setEnvKeyValuePairEscapeSetEnvKeyValuePair()
		local env_arg = { 'foo=bar','baz=qux'}
		codeblocks.debugenvs( {debugenvs = env_arg,language='C',longname='DontCare'} )
		test.string_contains(io.endcapture(),'<options additional_cmds_before="set env '.. env_arg[1]
											..'&#x0A;set env ' .. env_arg[2] .. '" />')
	end

	--Why is this an error and not silent? Because I feel if you are setting environment variables
	--and they are not getting set in your IDE, than that is a problem which premake should not be
	--quite about.
	--See codeblocks project generator source for the assumption that gcc has anything to do with this setting.
	function suite.withVar_noneGccCompiler_willCallError()
		_OPTIONS.cc = 'msc'
		local called = 0
		local real_error = error
		error = function() called = 1 end
		codeblocks.debugenvs( {debugenvs = {'foo=bar'},language='C',longname='foo'} )
		error = real_error
		test.isequal(1,called)
	end