--
-- validate.lua
-- Tests to validate the run-time environment before starting the action.
-- Copyright (c) 2002-2009 Jason Perkins and the Premake project
--


--
-- Performs a sanity check of all of the solutions and projects 
-- in the session to be sure they meet some minimum requirements.
--

	function premake.checkprojects()
		local action = premake.action.current()
		
		for sln in premake.solution.each() do
		
			-- every solution must have at least one project
			if (#sln.projects == 0) then
				return nil, "solution '" .. sln.name .. "' needs at least one project"
			end
			
			-- every solution must provide a list of configurations
			if (#sln.configurations == 0) then
				return nil, "solution '" .. sln.name .. "' needs configurations"
			end
			
			for prj in premake.solution.eachproject(sln) do

				-- every project must have a language
				if (not prj.language) then
					return nil, "project '" ..prj.name .. "' needs a language"
				end
				
				-- and the action must support it
				if (action.valid_languages) then
					if (not table.contains(action.valid_languages, prj.language)) then
						return nil, "the " .. action.shortname .. " action does not support " .. prj.language .. " projects"
					end
				end

				for cfg in premake.eachconfig(prj) do								
					
					-- every config must have a kind
					if (not cfg.kind) then
						return nil, "project '" ..prj.name .. "' needs a kind in configuration '" .. cfg.name .. "'"
					end
				
					-- and the action must support it
					if (action.valid_kinds) then
						if (not table.contains(action.valid_kinds, cfg.kind)) then
							return nil, "the " .. action.shortname .. " action does not support " .. cfg.kind .. " projects"
						end
					end
					
				end
				
				-- some actions have custom validation logic
				if action.oncheckproject then
					action.oncheckproject(prj)
				end
				
			end
		end		
		return true
	end


--
-- Check the specified tools (/cc, /dotnet, etc.) against the current action
-- to make sure they are compatible and supported.
--

	function premake.checktools()
		local action = premake.action.current()
		if (not action.valid_tools) then 
			return true 
		end
		
		for tool, values in pairs(action.valid_tools) do
			if (_OPTIONS[tool]) then
				if (not table.contains(values, _OPTIONS[tool])) then
					return nil, "the " .. action.shortname .. " action does not support /" .. tool .. "=" .. _OPTIONS[tool] .. " (yet)"
				end
			else
				_OPTIONS[tool] = values[1]
			end
		end
		
		return true
	end
