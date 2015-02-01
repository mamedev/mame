--
-- action.lua
-- Work with the list of registered actions.
-- Copyright (c) 2002-2009 Jason Perkins and the Premake project
--

	premake.action = { }


--
-- The list of registered actions.
--

	premake.action.list = { }
	

--
-- Register a new action.
--
-- @param a
--    The new action object.
-- 

	function premake.action.add(a)
		-- validate the action object, at least a little bit
		local missing
		for _, field in ipairs({"description", "trigger"}) do
			if (not a[field]) then
				missing = field
			end
		end
		
		if (missing) then
			error("action needs a " .. missing, 3)
		end

		-- add it to the master list
		premake.action.list[a.trigger] = a		
	end


--
-- Trigger an action.
--
-- @param name
--    The name of the action to be triggered.
-- @returns
--    None.
--

	function premake.action.call(name)
		local a = premake.action.list[name]
		for sln in premake.solution.each() do
			if a.onsolution then
				a.onsolution(sln)
			end
			for prj in premake.solution.eachproject(sln) do
				if a.onproject then
					a.onproject(prj)
				end
			end
		end
		
		if a.execute then
			a.execute()
		end
	end


--
-- Retrieve the current action, as determined by _ACTION.
--
-- @return
--    The current action, or nil if _ACTION is nil or does not match any action.
--

	function premake.action.current()
		return premake.action.get(_ACTION)
	end
	
	
--
-- Retrieve an action by name.
--
-- @param name
--    The name of the action to retrieve.
-- @returns
--    The requested action, or nil if the action does not exist.
--

	function premake.action.get(name)
		return premake.action.list[name]
	end


--
-- Iterator for the list of actions.
--

	function premake.action.each()
		-- sort the list by trigger
		local keys = { }
		for _, action in pairs(premake.action.list) do
			table.insert(keys, action.trigger)
		end
		table.sort(keys)
		
		local i = 0
		return function()
			i = i + 1
			return premake.action.list[keys[i]]
		end
	end


--
-- Activates a particular action.
--
-- @param name
--    The name of the action to activate.
--

	function premake.action.set(name)
		_ACTION = name
		-- Some actions imply a particular operating system
		local action = premake.action.get(name)
		if action then
			_OS = action.os or _OS
		end
	end


--
-- Determines if an action supports a particular language or target type.
--
-- @param action
--    The action to test.
-- @param feature
--    The feature to check, either a programming language or a target type.
-- @returns
--    True if the feature is supported, false otherwise.
--

	function premake.action.supports(action, feature)
		if not action then
			return false
		end
		if action.valid_languages then
			if table.contains(action.valid_languages, feature) then
				return true
			end
		end
		if action.valid_kinds then
			if table.contains(action.valid_kinds, feature) then
				return true
			end
		end
		return false
	end
