--
-- tests/base/test_api.lua
-- Automated test suite for the project API support functions.
-- Copyright (c) 2008-2011 Jason Perkins and the Premake project
--

	T.api = { }
	local suite = T.api

	local sln
	function suite.setup()
		sln = solution "MySolution"
	end


--
-- premake.getobject() tests
--

	function suite.getobject_RaisesError_OnNoContainer()
		premake.CurrentContainer = nil
		c, err = premake.getobject("container")
		test.istrue(c == nil)
		test.isequal("no active solution or project", err)
	end
	
	function suite.getobject_RaisesError_OnNoActiveSolution()
		premake.CurrentContainer = { }
		c, err = premake.getobject("solution")
		test.istrue(c == nil)
		test.isequal("no active solution", err)
	end
	
	function suite.getobject_RaisesError_OnNoActiveConfig()
		premake.CurrentConfiguration = nil
		c, err = premake.getobject("config")
		test.istrue(c == nil)
		test.isequal("no active solution, project, or configuration", err)
	end
		
	
--
-- premake.setarray() tests
--

	function suite.setarray_Inserts_OnStringValue()
		premake.CurrentConfiguration = { }
		premake.CurrentConfiguration.myfield = { }
		premake.setarray("config", "myfield", "hello")
		test.isequal("hello", premake.CurrentConfiguration.myfield[1])
	end

	function suite.setarray_Inserts_OnTableValue()
		premake.CurrentConfiguration = { }
		premake.CurrentConfiguration.myfield = { }
		premake.setarray("config", "myfield", { "hello", "goodbye" })
		test.isequal("hello", premake.CurrentConfiguration.myfield[1])
		test.isequal("goodbye", premake.CurrentConfiguration.myfield[2])
	end

	function suite.setarray_Appends_OnNewValues()
		premake.CurrentConfiguration = { }
		premake.CurrentConfiguration.myfield = { "hello" }
		premake.setarray("config", "myfield", "goodbye")
		test.isequal("hello", premake.CurrentConfiguration.myfield[1])
		test.isequal("goodbye", premake.CurrentConfiguration.myfield[2])
	end

	function suite.setarray_FlattensTables()
		premake.CurrentConfiguration = { }
		premake.CurrentConfiguration.myfield = { }
		premake.setarray("config", "myfield", { {"hello"}, {"goodbye"} })
		test.isequal("hello", premake.CurrentConfiguration.myfield[1])
		test.isequal("goodbye", premake.CurrentConfiguration.myfield[2])
	end
	
	function suite.setarray_RaisesError_OnInvalidValue()
		premake.CurrentConfiguration = { }
		premake.CurrentConfiguration.myfield = { }
		ok, err = pcall(function () premake.setarray("config", "myfield", "bad", { "Good", "Better", "Best" }) end)
		test.isfalse(ok)
	end
		
	function suite.setarray_CorrectsCase_OnConstrainedValue()
		premake.CurrentConfiguration = { }
		premake.CurrentConfiguration.myfield = { }
		premake.setarray("config", "myfield", "better", { "Good", "Better", "Best" })
		test.isequal("Better", premake.CurrentConfiguration.myfield[1])
	end
	
	

--
-- premake.setstring() tests
--

	function suite.setstring_Sets_OnNewProperty()
		premake.CurrentConfiguration = { }
		premake.setstring("config", "myfield", "hello")
		test.isequal("hello", premake.CurrentConfiguration.myfield)
	end

	function suite.setstring_Overwrites_OnExistingProperty()
		premake.CurrentConfiguration = { }
		premake.CurrentConfiguration.myfield = "hello"
		premake.setstring("config", "myfield", "goodbye")
		test.isequal("goodbye", premake.CurrentConfiguration.myfield)
	end
	
	function suite.setstring_RaisesError_OnInvalidValue()
		premake.CurrentConfiguration = { }
		ok, err = pcall(function () premake.setstring("config", "myfield", "bad", { "Good", "Better", "Best" }) end)
		test.isfalse(ok)
	end
		
	function suite.setstring_CorrectsCase_OnConstrainedValue()
		premake.CurrentConfiguration = { }
		premake.setstring("config", "myfield", "better", { "Good", "Better", "Best" })
		test.isequal("Better", premake.CurrentConfiguration.myfield)
	end


--
-- premake.setkeyvalue() tests
--

	function suite.setkeyvalue_Inserts_OnStringValue()
		premake.CurrentConfiguration = { }
		premake.setkeyvalue("config", "vpaths", { ["Headers"] = "*.h" })
		test.isequal({"*.h"}, premake.CurrentConfiguration.vpaths["Headers"])
	end
	
	function suite.setkeyvalue_Inserts_OnTableValue()
		premake.CurrentConfiguration = { }
		premake.setkeyvalue("config", "vpaths", { ["Headers"] = {"*.h","*.hpp"} })
		test.isequal({"*.h","*.hpp"}, premake.CurrentConfiguration.vpaths["Headers"])
	end
	
	function suite.setkeyvalue_Inserts_OnEmptyStringKey()
		premake.CurrentConfiguration = { }
		premake.setkeyvalue("config", "vpaths", { [""] = "src" })
		test.isequal({"src"}, premake.CurrentConfiguration.vpaths[""])
	end
	
	function suite.setkeyvalue_RaisesError_OnString()
		premake.CurrentConfiguration = { }
		ok, err = pcall(function () premake.setkeyvalue("config", "vpaths", "Headers") end)
		test.isfalse(ok)
	end
	
	function suite.setkeyvalue_InsertsString_IntoExistingKey()
		premake.CurrentConfiguration = { }
		premake.setkeyvalue("config", "vpaths", { ["Headers"] = "*.h" })
		premake.setkeyvalue("config", "vpaths", { ["Headers"] = "*.hpp" })
		test.isequal({"*.h","*.hpp"}, premake.CurrentConfiguration.vpaths["Headers"])
	end
	
	function suite.setkeyvalue_InsertsTable_IntoExistingKey()
		premake.CurrentConfiguration = { }
		premake.setkeyvalue("config", "vpaths", { ["Headers"] = {"*.h"} })
		premake.setkeyvalue("config", "vpaths", { ["Headers"] = {"*.hpp"} })
		test.isequal({"*.h","*.hpp"}, premake.CurrentConfiguration.vpaths["Headers"])
	end


--
-- accessor tests
--

	function suite.accessor_CanRetrieveString()
		sln.blocks[1].kind = "ConsoleApp"
		test.isequal("ConsoleApp", kind())
	end


--
-- solution() tests
--

	function suite.solution_SetsCurrentContainer_OnName()
		test.istrue(sln == premake.CurrentContainer)
	end

	function suite.solution_CreatesNewObject_OnNewName()
		solution "MySolution2"
		test.isfalse(sln == premake.CurrentContainer)
	end

	function suite.solution_ReturnsPrevious_OnExistingName()
		solution "MySolution2"
		local sln2 = solution "MySolution"
		test.istrue(sln == sln2)
	end

	function suite.solution_SetsCurrentContainer_OnExistingName()
		solution "MySolution2"
		solution "MySolution"
		test.istrue(sln == premake.CurrentContainer)
	end

	function suite.solution_ReturnsNil_OnNoActiveSolutionAndNoName()
		premake.CurrentContainer = nil
		test.isnil(solution())
	end
	
	function suite.solution_ReturnsCurrentSolution_OnActiveSolutionAndNoName()
		test.istrue(sln == solution())
	end
	
	function suite.solution_ReturnsCurrentSolution_OnActiveProjectAndNoName()
		project "MyProject"
		test.istrue(sln == solution())
	end

	function suite.solution_LeavesProjectActive_OnActiveProjectAndNoName()
		local prj = project "MyProject"
		solution()
		test.istrue(prj == premake.CurrentContainer)
	end

	function suite.solution_LeavesConfigActive_OnActiveSolutionAndNoName()
		local cfg = configuration "windows"
		solution()
		test.istrue(cfg == premake.CurrentConfiguration)
	end	

	function suite.solution_LeavesConfigActive_OnActiveProjectAndNoName()
		project "MyProject"
		local cfg = configuration "windows"
		solution()
		test.istrue(cfg == premake.CurrentConfiguration)
	end	
	
	function suite.solution_SetsName_OnNewName()
		test.isequal("MySolution", sln.name)
	end
	
	function suite.solution_AddsNewConfig_OnNewName()
		test.istrue(#sln.blocks == 1)
	end

	function suite.solution_AddsNewConfig_OnName()
		local num = #sln.blocks
		solution "MySolution"
		test.istrue(#sln.blocks == num + 1)
	end
	


--
-- configuration() tests
--
		
	function suite.configuration_RaisesError_OnNoContainer()
		premake.CurrentContainer = nil
		local fn = function() configuration{"Debug"} end
		ok, err = pcall(fn)
		test.isfalse(ok)
	end

	function suite.configuration_SetsCurrentConfiguration_OnKeywords()
		local cfg = configuration {"Debug"}
		test.istrue(premake.CurrentConfiguration == cfg)
	end

	function suite.configuration_AddsToContainer_OnKeywords()
		local cfg = configuration {"Debug"}
		test.istrue(cfg == sln.blocks[#sln.blocks])
	end
	
	function suite.configuration_ReturnsCurrent_OnNoKeywords()
		local cfg = configuration()
		test.istrue(cfg == sln.blocks[1])
	end

	function suite.configuration_SetsTerms()
		local cfg = configuration {"aa", "bb"}
		test.isequal({"aa", "bb"},  cfg.terms)
	end

	function suite.configuration_SetsTermsWithNestedTables()
		local cfg = configuration { {"aa", "bb"}, "cc" }
		test.isequal({"aa", "bb", "cc"},  cfg.terms)
	end

	function suite.configuration_CanReuseTerms()
		local cfg = configuration { "aa", "bb" }
		local cfg2 = configuration { cfg.terms, "cc" }
		test.isequal({"aa", "bb", "cc"},  cfg2.terms)
	end


--
-- project() tests
--
		
	function suite.project_RaisesError_OnNoSolution()
		premake.CurrentContainer = nil
		local fn = function() project("MyProject") end
		ok, err = pcall(fn)
		test.isfalse(ok)
	end

	function suite.project_SetsCurrentContainer_OnName()
		local prj = project "MyProject"
		test.istrue(prj == premake.CurrentContainer)
	end
	
	function suite.project_CreatesNewObject_OnNewName()
		local prj = project "MyProject"
		local pr2 = project "MyProject2"
		test.isfalse(prj == premake.CurrentContainer)
	end

	function suite.project_AddsToSolution_OnNewName()
		local prj = project "MyProject"
		test.istrue(prj == sln.projects[1])
	end
	
	function suite.project_ReturnsPrevious_OnExistingName()
		local prj = project "MyProject"
		local pr2 = project "MyProject2"
		local pr3 = project "MyProject"
		test.istrue(prj == pr3)
	end
	
	function suite.project_SetsCurrentContainer_OnExistingName()
		local prj = project "MyProject"
		local pr2 = project "MyProject2"
		local pr3 = project "MyProject"
		test.istrue(prj == premake.CurrentContainer)
	end
		
	function suite.project_ReturnsNil_OnNoActiveProjectAndNoName()
		test.isnil(project())
	end
	
	function suite.project_ReturnsCurrentProject_OnActiveProjectAndNoName()
		local prj = project "MyProject"
		test.istrue(prj == project())
	end
	
	function suite.project_LeavesProjectActive_OnActiveProjectAndNoName()
		local prj = project "MyProject"
		project()
		test.istrue(prj == premake.CurrentContainer)
	end
	
	function suite.project_LeavesConfigActive_OnActiveProjectAndNoName()
		local prj = project "MyProject"
		local cfg = configuration "Windows"
		project()
		test.istrue(cfg == premake.CurrentConfiguration)
	end
		
	function suite.project_SetsName_OnNewName()
		prj = project("MyProject")
		test.isequal("MyProject", prj.name)
	end
	
	function suite.project_SetsSolution_OnNewName()
		prj = project("MyProject")
		test.istrue(sln == prj.solution)
	end

	function suite.project_SetsConfiguration()
		prj = project("MyProject")
		test.istrue(premake.CurrentConfiguration == prj.blocks[1])
	end

	function suite.project_SetsUUID()
		local prj = project "MyProject"
		test.istrue(prj.uuid)
	end
			


--
-- uuid() tests
--

	function suite.uuid_makes_uppercase()
		premake.CurrentContainer = {}
		uuid "7CBB5FC2-7449-497f-947F-129C5129B1FB"
		test.isequal(premake.CurrentContainer.uuid, "7CBB5FC2-7449-497F-947F-129C5129B1FB")
	end


--
-- Fields with allowed value lists should be case-insensitive.
--

	function suite.flags_onCaseMismatch()
		premake.CurrentConfiguration = {}
		flags "symbols"
		test.isequal(premake.CurrentConfiguration.flags[1], "Symbols")
	end

	function suite.flags_onCaseMismatchAndAlias()
		premake.CurrentConfiguration = {}
		flags "optimisespeed"
		test.isequal(premake.CurrentConfiguration.flags[1], "OptimizeSpeed")
	end
		