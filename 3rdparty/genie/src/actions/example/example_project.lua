-- An example project generator; see _example.lua for action description

-- 
-- The project generation function, attached to the action in _example.lua.
-- By now, premake.generate() has created the project file using the name
-- provided in _example.lua, and redirected input to this new file.
--

	function premake.example.project(prj)
		-- If necessary, set an explicit line ending sequence
		-- io.eol = '\r\n'
	
		-- Let's start with a header
		_p('-- Example project file version 1.0')
		_p('Name: %s', prj.name)
		_p('Kind: %s', prj.kind)
		_p('Language: %s', prj.language)
		_p('ID: {%s}', prj.uuid)
		_p('')

		
		-- List the build configurations, and the settings for each
		for cfg in premake.eachconfig(prj) do
			_p('Configuration %s:', cfg.name)
			_p(1, 'Objects directory: %s', cfg.objectsdir)

			_p(1, 'Build target:')
			_p(2, 'Full path: %s', cfg.buildtarget.fullpath)
			_p(2, 'Directory: %s', cfg.buildtarget.directory)
			_p(2, 'Name: %s', cfg.buildtarget.name)
			_p(2, 'Base name: %s', cfg.buildtarget.basename)
			_p(2, 'Prefix: %s', cfg.buildtarget.prefix)
			_p(2, 'Suffix: %s', cfg.buildtarget.suffix)
			_p('')

			_p(1, 'Compiling:')
			_p(2, 'Defines: %s', table.concat(cfg.defines, ";"))
			_p(2, 'Include paths: %s', table.concat(cfg.includedirs, ";"))
			_p(2, 'Flags: %s', table.concat(cfg.flags, ", "))
			if not cfg.flags.NoPCH and cfg.pchheader then
				_p(2, 'Precompiled header: %s (%s)', cfg.pchheader, cfg.pchsource)
			end
			_p(2, 'Options: %s', table.concat(cfg.buildoptions, " "))
			_p('')
			
			_p(1, 'Linking:')
			_p(2, 'Library paths: %s', table.concat(cfg.libdirs, ";"))
			_p(2, 'Options: %s', table.concat(cfg.linkoptions, " "))
			_p(2, 'Libraries: %s', table.concat(premake.getlinks(cfg, "all", "fullpath")))
			_p('')
			
			if #cfg.prebuildcommands > 0 then
				_p(1, 'Prebuild commands:')
				for _, cmd in ipairs(cfg.prebuildcommands) do
					_p(2, cmd)
				end
				_p('')
			end
			
			if #cfg.prelinkcommands > 0 then
				_p(1, 'Prelink commands:')
				for _, cmd in ipairs(cfg.prelinkcommands) do
					_p(2, cmd)
				end
				_p('')
			end
			
			if #cfg.postbuildcommands > 0 then
				_p(1, 'Postbuild commands:')
				for _, cmd in ipairs(cfg.postbuildcommands) do
					_p(2, cmd)
				end
				_p('')
			end
		end

		
		-- List out the folders and files that make up the build
		local tr = premake.project.buildsourcetree(prj)
		premake.tree.sort(tr)
		premake.tree.traverse(tr, {
			onbranch = function(node, depth)
				_p(depth, path.getname(node.name) .. "/")
			end,
			
			onleaf = function(node, depth)
				_p(depth, path.getname(node.name))
			end
		})
		
	end
