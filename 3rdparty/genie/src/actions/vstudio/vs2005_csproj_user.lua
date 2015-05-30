--
-- vs2005_csproj_user.lua
-- Generate a Visual Studio 2005/2008 C# .user file.
-- Copyright (c) 2009 Jason Perkins and the Premake project
--

	local cs2005 = premake.vstudio.cs2005


	function cs2005.generate_user(prj)
		io.eol = "\r\n"
		
		_p('<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">')
		_p('  <PropertyGroup>')
		
		local refpaths = table.translate(prj.libdirs, function(v) return path.getabsolute(prj.location .. "/" .. v) end)
		_p('    <ReferencePath>%s</ReferencePath>', path.translate(table.concat(refpaths, ";"), "\\"))
		_p('  </PropertyGroup>')
		_p('</Project>')
		
	end
