--
-- codeblocks_workspace.lua
-- Generate a Code::Blocks workspace.
-- Copyright (c) 2009 Jason Perkins and the Premake project
--

	function premake.codeblocks.workspace(sln)
		_p('<?xml version="1.0" encoding="UTF-8" standalone="yes" ?>')
		_p('<CodeBlocks_workspace_file>')
		_p(1,'<Workspace title="%s">', sln.name)
		
		for prj in premake.solution.eachproject(sln) do
			local fname = path.join(path.getrelative(sln.location, prj.location), prj.name)
			local active = iif(prj.project == sln.projects[1], ' active="1"', '')
			
			_p(2,'<Project filename="%s.cbp"%s>', fname, active)
			for _,dep in ipairs(premake.getdependencies(prj)) do
				_p(3,'<Depends filename="%s.cbp" />', path.join(path.getrelative(sln.location, dep.location), dep.name))
			end
		
			_p(2,'</Project>')
		end
		
		_p(1,'</Workspace>')
		_p('</CodeBlocks_workspace_file>')
	end

