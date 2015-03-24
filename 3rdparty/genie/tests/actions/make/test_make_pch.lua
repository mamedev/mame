--
-- tests/actions/make/test_make_pch.lua
-- Validate the setup for precompiled headers in makefiles.
-- Copyright (c) 2010 Jason Perkins and the Premake project
--

	T.make_pch = { }
	local suite = T.make_pch
	local _ = premake.make.cpp


--
-- Setup and teardown
--

	local sln, prj, cfg
	function suite.setup()
		sln, prj = test.createsolution()
	end

	local function prepare()
		premake.bake.buildconfigs()
		prj = premake.getconfig(prj)
		cfg = premake.getconfig(prj, "Debug")
	end


--
-- Configuration block tests
--

	function suite.NoConfig_OnNoHeaderSet()
		prepare()
		_.pchconfig(cfg)
		test.capture [[]]
	end


	function suite.NoConfig_OnHeaderAndNoPCHFlag()
		pchheader "include/myproject.h"
		flags { NoPCH }
		prepare()
		_.pchconfig(cfg)
		test.capture [[]]
	end


	function suite.ConfigBlock_OnPchEnabled()
		pchheader "include/myproject.h"
		prepare()
		_.pchconfig(cfg)
		test.capture [[
  PCH        = include/myproject.h
  GCH        = $(OBJDIR)/$(notdir $(PCH)).gch
		]]
	end


--
-- Build rule tests
--

	function suite.BuildRules_OnCpp()
		pchheader "include/myproject.h"
		prepare()
		_.pchrules(prj)
		test.capture [[
ifneq (,$(PCH))
$(GCH): $(PCH)
	@echo $(notdir $<)
	$(SILENT) $(CXX) -x c++-header $(ALL_CXXFLAGS) -MMD -MP $(DEFINES) $(INCLUDES) -o "$@" -MF "$(@:%.gch=%.d)" -c "$<"
		]]
	end

	function suite.BuildRules_OnC()
		language "C"
		pchheader "include/myproject.h"
		prepare()
		_.pchrules(prj)
		test.capture [[
ifneq (,$(PCH))
$(GCH): $(PCH)
	@echo $(notdir $<)
	$(SILENT) $(CC) -x c-header $(ALL_CFLAGS) -MMD -MP $(DEFINES) $(INCLUDES) -o "$@" -MF "$(@:%.gch=%.d)" -c "$<"
		]]
	end


--
-- Ensure that PCH is included on all files that use it.
--

	function suite.includesPCH_onUse()
		pchheader "include/myproject.h"
		files { "main.cpp" }
		prepare()
		_.fileRules(prj)
		test.capture [[
$(OBJDIR)/main.o: main.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(ALL_CXXFLAGS) $(FORCE_INCLUDE) -o "$@" -MF $(@:%.o=%.d) -c "$<"
		]]
	end


--
-- If the header is located on one of the include file
-- search directories, it should get found automatically.
--

	function suite.findsPCH_onIncludeDirs()
		location "MyProject"
		pchheader "premake.h"
		includedirs { "../src/host" }
		prepare()
		_.pchconfig(cfg)
		test.capture [[
  PCH        = ../../src/host/premake.h
		]]
	end
