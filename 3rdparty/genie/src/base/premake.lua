--
-- premake.lua
-- High-level processing functions.
-- Copyright (c) 2002-2009 Jason Perkins and the Premake project
--

	premake._filelevelconfig = false
	premake._checkgenerate = true

--
-- Open a file for output, and call a function to actually do the writing.
-- Used by the actions to generate solution and project files.
--
-- @param obj
--    A solution or project object; will be based to the callback function.
-- @param filename
--    The output filename; see the docs for premake.project.getfilename()
--    for the expected format.
-- @param callback
--    The function responsible for writing the file, should take a solution
--    or project as a parameters.
--

	function premake.generate(obj, filename, callback)
		local prev  = io.capture()
		local abort = (callback(obj) == false)
		local new   = io.endcapture(prev)

		if abort then
			premake.stats.num_skipped = premake.stats.num_skipped + 1
			return
		end

		filename = premake.project.getfilename(obj, filename)

		if (premake._checkgenerate) then
			local delta = false

			local f, err = io.open(filename, "rb")
			if (not f) then
				if string.find(err, "No such file or directory") then
					delta = true
				else
					error(err, 0)
				end
			else
				local existing = f:read("*all")
				if existing ~= new then
					delta = true
				end
				f:close()
			end

			if delta then
				printf("Generating %q", filename)
				local f, err = io.open(filename, "wb")
				if (not f) then
					error(err, 0)
				end

				f:write(new)
				f:close()

				premake.stats.num_generated = premake.stats.num_generated + 1
			else
	--			printf("Skipping %s as its contents would not change.", filename)
				premake.stats.num_skipped = premake.stats.num_skipped + 1
			end
		else
			printf("Generating %q", filename)

			local f, err = io.open(filename, "wb")
			if (not f) then
				error(err, 0)
			end

			f:write(new)
			f:close()
			premake.stats.num_generated = premake.stats.num_generated + 1
		end
	end


--
-- Finds a valid premake build file in the specified directory
-- Used by both the main genie process, and include commands
--
-- @param dir
--	  The path in which to start looking for the script
-- @param search_upwards
--    When the script was not found in the specified directory, does the
--    script need to look upwards in the file system
--

	function premake.findDefaultScript(dir, search_upwards)
		search_upwards = search_upwards or true

		local last = ""
		while dir ~= last do
			for _, name in ipairs({ "genie.lua", "solution.lua", "premake4.lua" }) do

				local script0 = dir .. "/" .. name
				if (os.isfile(script0)) then
					return dir, name
				end

				local script1 = dir .. "/scripts/" .. name
				if (os.isfile(script1)) then
					return dir .. "/scripts/", name
				end
			end

			last = dir
			dir  = path.getabsolute(dir .. "/..")

			if dir == "." or not search_upwards then break end
		end

		return nil, nil
	end
