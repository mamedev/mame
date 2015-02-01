--
-- _clean.lua
-- The "clean" action: removes all generated files.
-- Copyright (c) 2002-2009 Jason Perkins and the Premake project
--

	premake.clean = { }


--
-- Clean a solution or project specific directory. Uses information in the
-- project object to build the target path.
--
-- @param obj
--    A solution or project object.
-- @param pattern
--    A filename pattern to clean; see premake.project.getfilename() for
--    a description of the format.
--

	function premake.clean.directory(obj, pattern)
		local fname = premake.project.getfilename(obj, pattern)
		os.rmdir(fname)
	end


--
-- Clean a solution or project specific file. Uses information in the project
-- object to build the target filename.
--
-- @param obj
--    A solution or project object.
-- @param pattern
--    A filename pattern to clean; see premake.project.getfilename() for
--    a description of the format.
--

	function premake.clean.file(obj, pattern)
		local fname = premake.project.getfilename(obj, pattern)
		os.remove(fname)
	end


--
-- Register the "clean" action.
--

	newaction {
		trigger     = "clean",
		description = "Remove all binaries and generated files",

		onsolution = function(sln)
			for action in premake.action.each() do
				if action.oncleansolution then
					action.oncleansolution(sln)
				end
			end
		end,
		
		onproject = function(prj)
			for action in premake.action.each() do
				if action.oncleanproject then
					action.oncleanproject(prj)
				end
			end

			if (prj.objectsdir) then
				premake.clean.directory(prj, prj.objectsdir)
			end

			-- build a list of supported target platforms that also includes a generic build
			local platforms = prj.solution.platforms or { }
			if not table.contains(platforms, "Native") then
				platforms = table.join(platforms, { "Native" })
			end

			for _, platform in ipairs(platforms) do
				for cfg in premake.eachconfig(prj, platform) do
					premake.clean.directory(prj, cfg.objectsdir)

					-- remove all permutations of the target binary
					premake.clean.file(prj, premake.gettarget(cfg, "build", "posix", "windows", "windows").fullpath)
					premake.clean.file(prj, premake.gettarget(cfg, "build", "posix", "posix", "linux").fullpath)
					premake.clean.file(prj, premake.gettarget(cfg, "build", "posix", "posix", "macosx").fullpath)
					premake.clean.file(prj, premake.gettarget(cfg, "build", "posix", "PS3", "windows").fullpath)
					if cfg.kind == "WindowedApp" then
						premake.clean.directory(prj, premake.gettarget(cfg, "build", "posix", "posix", "linux").fullpath .. ".app")
					end

					-- if there is an import library, remove that too
					premake.clean.file(prj, premake.gettarget(cfg, "link", "windows", "windows", "windows").fullpath)
					premake.clean.file(prj, premake.gettarget(cfg, "link", "posix", "posix", "linux").fullpath)

					-- call action.oncleantarget() with the undecorated target name
					local target = path.join(premake.project.getfilename(prj, cfg.buildtarget.directory), cfg.buildtarget.basename)
					for action in premake.action.each() do
						if action.oncleantarget then
							action.oncleantarget(target)
						end
					end
				end
			end
		end
	}
