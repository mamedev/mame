--
-- vs2010_vcxproj_filters.lua
-- Generate a Visual Studio 2010 C/C++ filters file.
-- Copyright (c) 2009-2011 Jason Perkins and the Premake project
--

	local vc2010 = premake.vstudio.vc2010
	local project = premake.project
	

--
-- The first portion of the filters file assigns unique IDs to each
-- directory or virtual group. Would be cool if we could automatically
-- map vpaths like "**.h" to an <Extensions>h</Extensions> element.
--

	function vc2010.filteridgroup(prj)
		local filters = { }
		local filterfound = false

		for file in project.eachfile(prj) do
			-- split the path into its component parts
			local folders = string.explode(file.vpath, "/", true)
			local path = ""
			for i = 1, #folders - 1 do
				-- element is only written if there *are* filters
				if not filterfound then
					filterfound = true
					_p(1,'<ItemGroup>')
				end
				
				path = path .. folders[i]

				-- have I seen this path before?
				if not filters[path] then
					filters[path] = true
					_p(2, '<Filter Include="%s">', path)
					_p(3, '<UniqueIdentifier>{%s}</UniqueIdentifier>', os.uuid(path))
					_p(2, '</Filter>')
				end

				-- prepare for the next subfolder
				path = path .. "\\"
			end
		end
		
		for _, custombuildtask in ipairs(prj.custombuildtask or {}) do
			for _, buildtask in ipairs(custombuildtask or {}) do
				local folders = string.explode(path.trimdots(path.getrelative(prj.location,buildtask[1])), "/", true)
				local path = ""
				for i = 1, #folders - 1 do
					-- element is only written if there *are* filters
					if not filterfound then
						filterfound = true
						_p(1,'<ItemGroup>')
					end
					
					path = path .. folders[i]

					-- have I seen this path before?
					if not filters[path] then
						filters[path] = true
						_p(2, '<Filter Include="%s">', path)
						_p(3, '<UniqueIdentifier>{%s}</UniqueIdentifier>', os.uuid(path))
						_p(2, '</Filter>')
					end

					-- prepare for the next subfolder
					path = path .. "\\"
				end	
			end
		end
		if filterfound then
			_p(1,'</ItemGroup>')
		end
	end


--
-- The second portion of the filters file assigns filters to each source
-- code file, as needed. Section is one of "ClCompile", "ClInclude", 
-- "ResourceCompile", or "None".
--

	function vc2010.filefiltergroup(prj, section, kind)
		local files = vc2010.getfilegroup(prj, section) or {}
		
		if kind == nill then
			kind = section
		end
		
		if (section == "CustomBuild") then
			for _, custombuildtask in ipairs(prj.custombuildtask or {}) do
				for _, buildtask in ipairs(custombuildtask or {}) do
					local fcfg = { }
					fcfg.name = path.getrelative(prj.location,buildtask[1])
					fcfg.vpath = path.trimdots(fcfg.name)
					table.insert(files, fcfg)
				end
			end
		end
		if #files > 0 then
			_p(1,'<ItemGroup>')
			for _, file in ipairs(files) do
				local filter
				if file.name ~= file.vpath then
					filter = path.getdirectory(file.vpath)
				else
					filter = path.getdirectory(file.name)
				end				
				
				if filter ~= "." then
					_p(2,'<%s Include=\"%s\">', kind, path.translate(file.name, "\\"))
						_p(3,'<Filter>%s</Filter>', path.translate(filter, "\\"))
					_p(2,'</%s>', kind)
				else
					_p(2,'<%s Include=\"%s\" />', kind, path.translate(file.name, "\\"))
				end
			end
			_p(1,'</ItemGroup>')
		end
	end


--
-- Output the VC2010 filters file
--
	
	function vc2010.generate_filters(prj)
		io.indent = "  "
		vc2010.header()
			vc2010.filteridgroup(prj)
			vc2010.filefiltergroup(prj, "None")
			vc2010.filefiltergroup(prj, "ClInclude")
			vc2010.filefiltergroup(prj, "ClCompile")
			vc2010.filefiltergroup(prj, "ResourceCompile")
			vc2010.filefiltergroup(prj, "CustomBuild")
			vc2010.filefiltergroup(prj, "AppxManifest")
			vc2010.filefiltergroup(prj, "Image")
			vc2010.filefiltergroup(prj, "DeploymentContent", "None")
		_p('</Project>')
	end
