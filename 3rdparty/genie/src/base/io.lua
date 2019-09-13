--
-- io.lua
-- Additions to the I/O namespace.
-- Copyright (c) 2008-2009 Jason Perkins and the Premake project
--

io.eol         = "\n"
io.indent      = "\t"
io.indentLevel = 0

-- default escaper function
local function _escaper(v) return v end
_esc = _escaper


--
-- Prepare to capture the output from all subsequent calls to io.printf(), 
-- used for automated testing of the generators. Returns the previously
-- captured text.
--

	function io.capture()
		local prev = io.captured
		io.captured = ''
		return prev
	end
	
	
	
--
-- Returns the captured text and stops capturing, optionally restoring a
-- previous capture.
--

	function io.endcapture(restore)
		local captured = io.captured
		io.captured = restore
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
-- Write a formatted string to the exported file, after passing all
-- arguments (except for the first, which is the formatting string)
-- through io.esc().
--

	function io.xprintf(msg, ...)
		local arg = {...}
		for i = 1, #arg do
			arg[i] = io.esc(arg[i])
		end
		io.printf(msg, unpack(arg))
	end

--
-- Handle escaping of strings for various outputs
--

	function io.esc(value)
		if type(value) == "table" then
			local result = {}
			local n = #value
			for i = 1, n do
				table.insert(result, io.esc(value[i]))
			end
			return result
		end

		return _esc(value or "")
	end

--
-- Set a new string escaping function
--

	function io.escaper(func)
		_esc = func or _escaper
	end

--
-- Because I use io.printf() so often in the generators, create a terse shortcut
-- for it. This saves me typing, and also reduces the size of the executable.
--

	_p = io.printf
	_x = io.xprintf
