--
-- tests/base/test_action.lua
-- Automated test suite for the action list.
-- Copyright (c) 2009 Jason Perkins and the Premake project
--

	T.action = { }


--
-- Setup/teardown
--

	local fake = {
		trigger = "fake",
		description = "Fake action used for testing",
	}
	
	function T.action.setup()
		premake.action.list["fake"] = fake
		solution "MySolution"
		configurations "Debug"
		project "MyProject"
		premake.bake.buildconfigs()
	end

	function T.action.teardown()
		premake.action.list["fake"] = nil
	end



--
-- Tests for call()
--

	function T.action.CallCallsExecuteIfPresent()
		local called = false
		fake.execute = function () called = true end
		premake.action.call("fake")
		test.istrue(called)
	end

	function T.action.CallCallsOnSolutionIfPresent()
		local called = false
		fake.onsolution = function () called = true end
		premake.action.call("fake")
		test.istrue(called)
	end

	function T.action.CallCallsOnProjectIfPresent()
		local called = false
		fake.onproject = function () called = true end
		premake.action.call("fake")
		test.istrue(called)
	end
	
	function T.action.CallSkipsCallbacksIfNotPresent()
		test.success(premake.action.call, "fake")
	end


--
-- Tests for set()
--

	function T.action.set_SetsActionOS()
		local oldos = _OS
		_OS = "linux"
		premake.action.set("vs2008")
		test.isequal(_OS, "windows")
		_OS = oldos
	end
