--
-- vs200x_vcproj_user.lua
-- Generate a Visual Studio 2002-2008 C/C++ project .user file
-- Copyright (c) 2011 Jason Perkins and the Premake project
--


--
-- Set up namespaces
--

	local vc200x = premake.vstudio.vc200x


--
-- Generate the .vcproj.user file
--

	function vc200x.generate_user(prj)
		vc200x.header('VisualStudioUserFile')
		
		_p(1,'ShowAllFiles="false"')
		_p(1,'>')
		_p(1,'<Configurations>')
		
		for _, cfginfo in ipairs(prj.solution.vstudio_configs) do
			if cfginfo.isreal then
				local cfg = premake.getconfig(prj, cfginfo.src_buildcfg, cfginfo.src_platform)
		
				_p(2,'<Configuration')
				_p(3,'Name="%s"', premake.esc(cfginfo.name))
				_p(3,'>')
				
				vc200x.debugdir(cfg)
				
				_p(2,'</Configuration>')
			end
		end		
		
		_p(1,'</Configurations>')
		_p('</VisualStudioUserFile>')
	end


--
-- Output the debug settings element
--
	function vc200x.environmentargs(cfg)
		if cfg.environmentargs and #cfg.environmentargs > 0 then
			_p(4,'Environment="%s"', string.gsub(table.concat(cfg.environmentargs, "&#x0A;"),'"','&quot;'))
			if cfg.flags.EnvironmentArgsDontMerge then
				_p(4,'EnvironmentMerge="false"')
			end
		end
	end
	
	function vc200x.debugdir(cfg)
		_p(3,'<DebugSettings')
		
		if cfg.debugdir then
			_p(4,'WorkingDirectory="%s"', path.translate(cfg.debugdir, '\\'))
		end
		
		if #cfg.debugargs > 0 then
			_p(4,'CommandArguments="%s"', table.concat(cfg.debugargs, " "))
		end

			vc200x.environmentargs(cfg)
				
		_p(3,'/>')
	end
