--
-- vs2005_solution.lua
-- Generate a Visual Studio 2005-2010 solution.
-- Copyright (c) 2009-2011 Jason Perkins and the Premake project
--

	premake.vstudio.sln2005 = { }
	local vstudio = premake.vstudio
	local sln2005 = premake.vstudio.sln2005


	function sln2005.generate(sln)
		io.eol = '\r\n'

		-- Precompute Visual Studio configurations
		sln.vstudio_configs = premake.vstudio.buildconfigs(sln)

		-- Mark the file as Unicode
		_p('\239\187\191')

		sln2005.reorderProjects(sln)

		sln2005.header(sln)

		for grp in premake.solution.eachgroup(sln) do
			sln2005.group(grp)
		end

		for prj in premake.solution.eachproject(sln) do
			sln2005.project(prj)
		end

		
		_p('Global')
		sln2005.platforms(sln)
		sln2005.project_platforms(sln)
		sln2005.properties(sln)
		sln2005.project_groups(sln)
		_p('EndGlobal')
	end

--
-- If a startup project is specified, move it to the front of the project list. 
-- This will make Visual Studio treat it like a startup project.
--

		function sln2005.reorderProjects(sln)
				if sln.startproject then
						for i, prj in ipairs(sln.projects) do
							if sln.startproject == prj.name then
								-- Move group tree containing the project to start of group list
								local cur = prj.group
								while cur ~= nil do
									-- Remove group from array
									for j, group in ipairs(sln.groups) do
										if group == cur then
											table.remove(sln.groups, j)
											break
										end
									end

									-- Add back at start
									table.insert(sln.groups, 1, cur)
									cur = cur.parent
								end

								-- Move the project itself to start
								table.remove(sln.projects, i)
								table.insert(sln.projects, 1, prj)
								break
							end
						end
				end
		end

--
-- Generate the solution header
--

	function sln2005.header(sln)
		local action = premake.action.current()
		_p('Microsoft Visual Studio Solution File, Format Version %d.00', action.vstudio.solutionVersion)
		_p('# Visual Studio %s', _ACTION:sub(3))
	end


--
-- Write out an entry for a project
--

	function sln2005.project(prj)
		-- Build a relative path from the solution file to the project file
		local projpath = path.translate(path.getrelative(prj.solution.location, vstudio.projectfile(prj)), "\\")

		_p('Project("{%s}") = "%s", "%s", "{%s}"', vstudio.tool(prj), prj.name, projpath, prj.uuid)
		sln2005.projectdependencies(prj)
		_p('EndProject')
	end

--
-- Write out an entry for a solution folder
--
	
	function sln2005.group(grp)
		_p('Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = "%s", "%s", "{%s}"', grp.name, grp.name, grp.uuid)
		_p('EndProject')
	end


--
-- Write out the list of project dependencies for a particular project.
--

	function sln2005.projectdependencies(prj)
		local deps = premake.getdependencies(prj)
		if #deps > 0 then
			_p('\tProjectSection(ProjectDependencies) = postProject')
			for _, dep in ipairs(deps) do
				_p('\t\t{%s} = {%s}', dep.uuid, dep.uuid)
			end
			_p('\tEndProjectSection')
		end
	end


--
-- Write out the contents of the SolutionConfigurationPlatforms section, which
-- lists all of the configuration/platform pairs that exist in the solution.
--

	function sln2005.platforms(sln)
		_p('\tGlobalSection(SolutionConfigurationPlatforms) = preSolution')
		for _, cfg in ipairs(sln.vstudio_configs) do
			_p('\t\t%s = %s', cfg.name, cfg.name)
		end
		_p('\tEndGlobalSection')
	end



--
-- Write out the contents of the ProjectConfigurationPlatforms section, which maps
-- the configuration/platform pairs into each project of the solution.
--

	function sln2005.project_platforms(sln)
		_p('\tGlobalSection(ProjectConfigurationPlatforms) = postSolution')
		for prj in premake.solution.eachproject(sln) do
			for _, cfg in ipairs(sln.vstudio_configs) do

				-- .NET projects always map to the "Any CPU" platform (for now, at
				-- least). For C++, "Any CPU" and "Mixed Platforms" map to the first
				-- C++ compatible target platform in the solution list.
				local mapped
				if premake.isdotnetproject(prj) then
					mapped = "Any CPU"
				else
					if cfg.platform == "Any CPU" or cfg.platform == "Mixed Platforms" then
						mapped = sln.vstudio_configs[3].platform
					else
						mapped = cfg.platform
					end
				end

				_p('\t\t{%s}.%s.ActiveCfg = %s|%s', prj.uuid, cfg.name, cfg.buildcfg, mapped)
				if mapped == cfg.platform or cfg.platform == "Mixed Platforms" then
					_p('\t\t{%s}.%s.Build.0 = %s|%s',  prj.uuid, cfg.name, cfg.buildcfg, mapped)
				end

				if premake.vstudio.iswinrt() and prj.kind == "WindowedApp" then
					_p('\t\t{%s}.%s.Deploy.0 = %s|%s',  prj.uuid, cfg.name, cfg.buildcfg, mapped)
				end
			end
		end
		_p('\tEndGlobalSection')
	end



--
-- Write out contents of the SolutionProperties section; currently unused.
--

	function sln2005.properties(sln)
		_p('\tGlobalSection(SolutionProperties) = preSolution')
		_p('\t\tHideSolutionNode = FALSE')
		_p('\tEndGlobalSection')
	end

--
-- Write out list of project nestings
--
	function sln2005.project_groups(sln)
		_p('\tGlobalSection(NestedProjects) = preSolution')

		for grp in premake.solution.eachgroup(sln) do
			if grp.parent ~= nil then
				_p('\t\t{%s} = {%s}', grp.uuid, grp.parent.uuid)
			end
		end

		for prj in premake.solution.eachproject(sln) do
			if prj.group ~= nil then
				_p('\t\t{%s} = {%s}', prj.uuid, prj.group.uuid)
			end
		end

		_p('\tEndGlobalSection')
	end