--
-- codelite_workspace.lua
-- Generate a CodeLite workspace file.
-- Copyright (c) 2009, 2011 Jason Perkins and the Premake project
--

	function premake.codelite.workspace(sln)
		_p('<?xml version="1.0" encoding="utf-8"?>')
		_p('<CodeLite_Workspace Name="%s" Database="./%s.tags">', premake.esc(sln.name), premake.esc(sln.name))
		
		for i,prj in ipairs(sln.projects) do
			local name = premake.esc(prj.name)
			local fname = path.join(path.getrelative(sln.location, prj.location), prj.name)
			local active = iif(i==1, "Yes", "No")
			_p('  <Project Name="%s" Path="%s.project" Active="%s" />', name, fname, active)
		end
		
		-- build a list of supported target platforms; I don't support cross-compiling yet
		local platforms = premake.filterplatforms(sln, premake[_OPTIONS.cc].platforms, "Native")
		for i = #platforms, 1, -1 do
			if premake.platforms[platforms[i]].iscrosscompiler then
				table.remove(platforms, i)
			end
		end 

		_p('  <BuildMatrix>')
		for _, platform in ipairs(platforms) do
			for _, cfgname in ipairs(sln.configurations) do
				local name = premake.getconfigname(cfgname, platform):gsub("|","_")
				_p('    <WorkspaceConfiguration Name="%s" Selected="yes">', name)
				for _,prj in ipairs(sln.projects) do
					_p('      <Project Name="%s" ConfigName="%s"/>', prj.name, name)
				end
				_p('    </WorkspaceConfiguration>')
			end
		end
		_p('  </BuildMatrix>')
		_p('</CodeLite_Workspace>')
	end

