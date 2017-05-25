--
-- solution.lua
-- Work with the list of solutions loaded from the script.
-- Copyright (c) 2002-2009 Jason Perkins and the Premake project
--

	premake.solution = { }


-- The list of defined solutions (which contain projects, etc.)

	premake.solution.list = { }


--
-- Create a new solution and add it to the session.
--
-- @param name
--    The new solution's name.
--

	function premake.solution.new(name)
		local sln = { }

		-- add to master list keyed by both name and index
		table.insert(premake.solution.list, sln)
		premake.solution.list[name] = sln
			
		-- attach a type descriptor
		setmetatable(sln, { __type="solution" })

		sln.name           = name
		sln.basedir        = os.getcwd()			
		sln.projects       = { }
		sln.blocks         = { }
		sln.configurations = { }
		sln.groups         = { }
		sln.importedprojects = { }
		return sln
	end


--
-- Iterate over the collection of solutions in a session.
--
-- @returns
--    An iterator function.
--

	function premake.solution.each()
		local i = 0
		return function ()
			i = i + 1
			if i <= #premake.solution.list then
				return premake.solution.list[i]
			end
		end
	end


--
-- Iterate over the projects of a solution.
--
-- @param sln
--    The solution.
-- @returns
--    An iterator function.
--

	function premake.solution.eachproject(sln)
		local i = 0
		return function ()
			i = i + 1
			if (i <= #sln.projects) then
				return premake.solution.getproject(sln, i)
			end
		end
	end

--
-- Iterate over the groups of a solution
--
-- @param sln
--    The solution.
-- @returns
--    An iterator function.

	function premake.solution.eachgroup(sln)
		local i = 0
		return function()
			i = i + 1
			if(i <= #sln.groups) then
				return premake.solution.getgroup(sln, i)
			end
		end
	end


--
-- Retrieve a solution by name or index.
--
-- @param key
--    The solution key, either a string name or integer index.
-- @returns
--    The solution with the provided key.
--

	function premake.solution.get(key)
		return premake.solution.list[key]
	end


--
-- Retrieve the project at a particular index.
--
-- @param sln
--    The solution.
-- @param idx
--    An index into the array of projects.
-- @returns
--    The project at the given index.
--

	function premake.solution.getproject(sln, idx)
		-- retrieve the root configuration of the project, with all of
		-- the global (not configuration specific) settings collapsed
		local prj = sln.projects[idx]
		local cfg = premake.getconfig(prj)
		
		-- root configuration doesn't have a name; use the project's
		cfg.name = prj.name
		return cfg
	end

--
-- Retrieve the group at a particular index
-- 
-- @param sln
--   The solution.
-- @param idx
--   The index into the array of groups
-- @returns
--    The group at the given index
	
	function premake.solution.getgroup(sln, idx)
		local grp = sln.groups[idx]
		return grp
	end