--
-- io.lua
-- Additions to the I/O namespace.
-- Copyright (c) 2008-2009 Jason Perkins and the Premake project
--


--
-- Prepare to capture the output from all subsequent calls to io.printf(), 
-- used for automated testing of the generators.
--

	function io.capture()
		io.captured = ''
	end
	
	
	
--
-- Returns the captured text and stops capturing.
--

	function io.endcapture()
		local captured = io.captured
		io.captured = nil
		return captured
	end
	
	
--
-- Open an overload of the io.open() function, which will create any missing
-- subdirectories in the filename if "mode" is set to writeable.
--

	local builtin_open = io.open
	function io.open(fname, mode)
		if (mode) then
			if (mode:find("w")) then
				local dir = path.getdirectory(fname)
				ok, err = os.mkdir(dir)
				if (not ok) then
					error(err, 0)
				end
			end
		end
		return builtin_open(fname, mode)
	end



-- 
-- A shortcut for printing formatted output to an output stream.
--

	function io.printf(msg, ...)
		local arg={...}
		if not io.eol then
			io.eol = "\n"
		end

		if not io.indent then
			io.indent = "\t"
		end

		if type(msg) == "number" then
			s = string.rep(io.indent, msg) .. string.format(table.unpack(arg))
		else
			s = string.format(msg, table.unpack(arg))
		end
		
		if io.captured then
			io.captured = io.captured .. s .. io.eol
		else
			io.write(s)
			io.write(io.eol)
		end
	end


--
-- Because I use io.printf() so often in the generators, create a terse shortcut
-- for it. This saves me typing, and also reduces the size of the executable.
--

	_p = io.printf
