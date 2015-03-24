-- An example solution generator; see _example.lua for action description

-- 
-- The solution generation function, attached to the action in _example.lua.
-- By now, premake.generate() has created the solution file using the name
-- provided in _example.lua, and redirected input to this new file.
--

	function premake.example.solution(sln)
		-- If necessary, set an explicit line ending sequence
		-- io.eol = '\r\n'
	
		-- Let's start with a header
		_p('-- Example solution file version 1.0')
		_p('Name: %s', sln.name)
		_p('')
		

		-- List the build configurations
		for _, cfgname in ipairs(sln.configurations) do
			_p('Config: %s', cfgname)
		end
		_p('')

		
		-- List the projects contained by the solution, with some info on each
		for prj in premake.solution.eachproject(sln) do
			_p('Project: %s', prj.name)
			_p(1, 'Kind: %s', prj.kind)
			_p(1, 'Language: %s', prj.language)
			_p(1, 'ID: {%s}', prj.uuid)
			_p(1, 'Relative path: %s', path.getrelative(sln.location, prj.location))
			
			-- List dependencies, if there are any
			local deps = premake.getdependencies(prj)
			if #deps > 0 then
				_p(1, 'Dependencies:')
				for _, depprj in ipairs(deps) do
					_p(2, '%s {%s}', depprj.name, depprj.uuid)
				end
			end

			_p('')
		end
		
	end
