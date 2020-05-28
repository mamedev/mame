--
-- _xcode.lua
-- Define the Apple XCode action and support functions.
-- Copyright (c) 2009 Jason Perkins and the Premake project
--

	premake.xcode = { }

--
-- Verify only single target kind for Xcode project
--
-- @param prj
--    Project to be analyzed
--

	function premake.xcode.checkproject(prj)
		-- Xcode can't mix target kinds within a project
		local last
		for cfg in premake.eachconfig(prj) do
			if last and last ~= cfg.kind then
				error("Project '" .. prj.name .. "' uses more than one target kind; not supported by Xcode", 0)
			end
			last = cfg.kind
		end
	end

--
-- Set default toolset
--

	premake.xcode.toolset = "macosx"
