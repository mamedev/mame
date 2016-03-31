--
-- premake.lua
-- High-level processing functions.
-- Copyright (c) 2002-2009 Jason Perkins and the Premake project
--

	premake._filelevelconfig = false

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
		filename = premake.project.getfilename(obj, filename)
		printf("Generating %s...", filename)

		local f, err = io.open(filename, "wb")
		if (not f) then
			error(err, 0)
		end

		io.output(f)
		callback(obj)
		f:close()
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
