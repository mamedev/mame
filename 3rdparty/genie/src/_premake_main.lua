--
-- _premake_main.lua
-- Script-side entry point for the main program logic.
-- Copyright (c) 2002-2011 Jason Perkins and the Premake project
--


	_WORKING_DIR        = os.getcwd()


--
-- Inject a new target platform into each solution; called if the --platform
-- argument was specified on the command line.
--

	local function injectplatform(platform)
		if not platform then return true end
		platform = premake.checkvalue(platform, premake.fields.platforms.allowed)

		for sln in premake.solution.each() do
			local platforms = sln.platforms or { }

			-- an empty table is equivalent to a native build
			if #platforms == 0 then
				table.insert(platforms, "Native")
			end

			-- the solution must provide a native build in order to support this feature
			if not table.contains(platforms, "Native") then
				return false, sln.name .. " does not target native platform\nNative platform settings are required for the --platform feature."
			end

			-- add it to the end of the list, if it isn't in there already
			if not table.contains(platforms, platform) then
				table.insert(platforms, platform)
			end

			sln.platforms = platforms
		end

		return true
	end

--
-- Script-side program entry point.
--

	function _premake_main(scriptpath)

		-- if running off the disk (in debug mode), load everything
		-- listed in _manifest.lua; the list divisions make sure
		-- everything gets initialized in the proper order.

		if (scriptpath) then
			local scripts  = dofile(scriptpath .. "/_manifest.lua")
			for _,v in ipairs(scripts) do
				dofile(scriptpath .. "/" .. v)
			end
		end


		-- Now that the scripts are loaded, I can use path.getabsolute() to properly
		-- canonicalize the executable path.

		_PREMAKE_COMMAND = path.getabsolute(_PREMAKE_COMMAND)


		-- Set up the environment for the chosen action early, so side-effects
		-- can be picked up by the scripts.

		premake.action.set(_ACTION)


		-- Seed the random number generator so actions don't have to do it themselves

		math.randomseed(os.time())


		-- If there is a project script available, run it to get the
		-- project information, available options and actions, etc.


		if (nil ~= _OPTIONS["file"]) then
			local fname = _OPTIONS["file"]
			if (os.isfile(fname)) then
				dofile(fname)
			else
				error("No genie script '" .. fname .. "' found!", 2)
			end
		else
			local dir, name = premake.findDefaultScript(path.getabsolute("./"))
			if dir ~= nil then
				os.chdir(dir)
				dofile(name)
			end
		end

		-- Process special options

		if (_OPTIONS["version"] or _OPTIONS["help"] or not _ACTION) then
			printf("GENie - Project generator tool %s", _GENIE_VERSION_STR)
			printf("https://github.com/bkaradzic/GENie")
			if (not _OPTIONS["version"]) then
				premake.showhelp()
			end
			return 1
		end

		-- Validate the command-line arguments. This has to happen after the
		-- script has run to allow for project-specific options

		action = premake.action.current()
		if (not action) then
			error("Error: no such action '" .. _ACTION .. "'", 0)
		end

		ok, err = premake.option.validate(_OPTIONS)
		if (not ok) then error("Error: " .. err, 0) end


		-- Sanity check the current project setup

		ok, err = premake.checktools()
		if (not ok) then error("Error: " .. err, 0) end


		-- If a platform was specified on the command line, inject it now

		ok, err = injectplatform(_OPTIONS["platform"])
		if (not ok) then error("Error: " .. err, 0) end

		local profiler = newProfiler()
		if (nil ~= _OPTIONS["debug-profiler"]) then
			profiler:start()
		end

		-- work-in-progress: build the configurations
		print("Building configurations...")
		premake.bake.buildconfigs()

		if (nil ~= _OPTIONS["debug-profiler"]) then
			profiler:stop()

			local filePath = path.getabsolute("GENie-profiler-bake.txt")
			print("Writing debug-profiler report " .. filePath .. ".")

			local outfile = io.open(filePath, "w+")
			profiler:report(outfile)
			outfile:close()
		end

		ok, err = premake.checkprojects()
		if (not ok) then error("Error: " .. err, 0) end

		-- Hand over control to the action
		printf("Running action '%s'...", action.trigger)
		premake.action.call(action.trigger)

		print("Done.")
		return 0

	end
