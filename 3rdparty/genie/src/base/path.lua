--
-- path.lua
-- Path manipulation functions.
-- Copyright (c) 2002-2010 Jason Perkins and the Premake project
--


--
-- Get the absolute file path from a relative path. The requested
-- file path doesn't actually need to exist.
--
	
	function path.getabsolute(p)
		-- normalize the target path
		p = path.translate(p, "/")
		if (p == "") then p = "." end
		
		-- if the directory is already absolute I don't need to do anything
		local result = iif (path.isabsolute(p), nil, os.getcwd())

		-- split up the supplied relative path and tackle it bit by bit
		for n, part in ipairs(p:explode("/", true)) do
			if (part == "" and n == 1) then
				result = "/"
			elseif (part == "..") then
				result = path.getdirectory(result)
			elseif (part ~= ".") then
				-- Environment variables embedded in the path need to be treated
				-- as relative paths; path.join() makes them absolute
				if (part:startswith("$") and n > 1) then
					result = result .. "/" .. part
				else
					result = path.join(result, part)
				end
			end
		end
		
		-- if I end up with a trailing slash remove it
		result = iif(result:endswith("/"), result:sub(1, -2), result)
		
		return result
	end
	

--
-- Retrieve the filename portion of a path, without any extension.
--

	function path.getbasename(p)
		local name = path.getname(p)
		local i = name:findlast(".", true)
		if (i) then
			return name:sub(1, i - 1)
		else
			return name
		end
	end

--
-- Retrieve the path, without any extension.
--

	function path.removeext(name)
		local i = name:findlast(".", true)
		if (i) then
			return name:sub(1, i - 1)
		else
			return name
		end
	end

--
-- Retrieve the directory portion of a path, or an empty string if 
-- the path does not include a directory.
--

	function path.getdirectory(p)
		local i = p:findlast("/", true)
		if (i) then
			if i > 1 then i = i - 1 end
			return p:sub(1, i)
		else
			return "."
		end
	end


--
-- Retrieve the drive letter, if a Windows path.
--

	function path.getdrive(p)
		local ch1 = p:sub(1,1)
		local ch2 = p:sub(2,2)
		if ch2 == ":" then
			return ch1
		end
	end



--
-- Retrieve the file extension.
--

	function path.getextension(p)
		local i = p:findlast(".", true)
		if (i) then
			return p:sub(i)
		else
			return ""
		end
	end
	
	
	
--
-- Retrieve the filename portion of a path.
--

	function path.getname(p)
		local i = p:findlast("[/\\]")
		if (i) then
			return p:sub(i + 1)
		else
			return p
		end
	end
	
	
--
-- Returns the relative path from src to dest.
--

	function path.getrelative(src, dst)
		-- normalize the two paths
		src = path.getabsolute(src)
		dst = path.getabsolute(dst)

		-- same directory?
		if (src == dst) then
			return "."
		end
		
		-- dollar macro? Can't tell what the real path is; use absolute
		-- This enables paths like $(SDK_ROOT)/include to work correctly.
		if dst:startswith("$") then
			return dst
		end
		
		src = src .. "/"
		dst = dst .. "/"

		-- find the common leading directories
		local idx = 0
		while (true) do
			local tst = src:find("/", idx + 1, true)
			if tst then
				if src:sub(1,tst) == dst:sub(1,tst) then
					idx = tst
				else
					break
				end
			else
				break
			end
		end
		
		-- if they have nothing in common return absolute path
		local first = src:find("/", 0, true)
		if idx <= first then
			return dst:sub(1, -2)
		end
		
		-- trim off the common directories from the front 
		src = src:sub(idx + 1)
		dst = dst:sub(idx + 1)
		
		-- back up from dst to get to this common parent
		local result = ""		
		idx = src:find("/")
		while (idx) do
			result = result .. "../"
			idx = src:find("/", idx + 1)
		end

		-- tack on the path down to the dst from here
		result = result .. dst

		-- remove the trailing slash
		return result:sub(1, -2)
	end
	

--
-- Returns true if the filename represents a C/C++ source code file. This check
-- is used to prevent passing non-code files to the compiler in makefiles. It is
-- not foolproof, but it has held up well. I'm open to better suggestions.
--

	function path.iscfile(fname)
		local extensions = { ".c", ".s", ".m" }
		local ext = path.getextension(fname):lower()
		return table.contains(extensions, ext)
	end
	
	function path.iscppfile(fname)
		local extensions = { ".cc", ".cpp", ".cxx", ".c", ".s", ".m", ".mm" }
		local ext = path.getextension(fname):lower()
		return table.contains(extensions, ext)
	end
	
	function path.isobjcfile(fname)
		local extensions = { ".m", ".mm" }
		local ext = path.getextension(fname):lower()
		return table.contains(extensions, ext)
	end

	function path.iscppheader(fname)
		local extensions = { ".h", ".hh", ".hpp", ".hxx" }
		local ext = path.getextension(fname):lower()
		return table.contains(extensions, ext)
	end

	function path.isappxmanifest(fname)
		local extensions = { ".appxmanifest" }
		local ext = path.getextension(fname):lower()
		return table.contains(extensions, ext)
	end


--
-- Returns true if the filename represents a Windows resource file. This check
-- is used to prevent passing non-resources to the compiler in makefiles.
--

	function path.isresourcefile(fname)
		local extensions = { ".rc" }
		local ext = path.getextension(fname):lower()
		return table.contains(extensions, ext)
	end

	
--
-- Join one or more pieces of a path together into a single path.
-- 
-- @param ...
--    One or more path strings.
-- @return
--    The joined path.
--

	function path.join(...)
		local arg={...}
		local numargs = select("#", ...)
		if numargs == 0 then
			return "";
		end
		
		local allparts = {}
		for i = numargs, 1, -1 do
			local part = select(i, ...)
			if part and #part > 0 and part ~= "." then
				-- trim off trailing slashes
				while part:endswith("/") do
					part = part:sub(1, -2)
				end
				
				table.insert(allparts, 1, part)
				if path.isabsolute(part) then
					break
				end
			end
		end
		
		return table.concat(allparts, "/")
	end


--
-- Takes a path which is relative to one location and makes it relative
-- to another location instead.
--

	function path.rebase(p, oldbase, newbase)
		p = path.getabsolute(path.join(oldbase, p))
		p = path.getrelative(newbase, p)
		return p
	end
	
	
--
-- Convert the separators in a path from one form to another. If `sep`
-- is nil, then a platform-specific separator is used.
--

	function path.translate(p, sep)
		if (type(p) == "table") then
			local result = { }
			for _, value in ipairs(p) do
				table.insert(result, path.translate(value))
			end
			return result
		else
			if (not sep) then
				if (os.is("windows")) then
					sep = "\\"
				else
					sep = "/"
				end
			end
			local result = p:gsub("[/\\]", sep)
			return result
		end
	end


--
-- Converts from a simple wildcard syntax, where * is "match any"
-- and ** is "match recursive", to the corresponding Lua pattern.
--
-- @param pattern
--    The wildcard pattern to convert.
-- @returns
--    The corresponding Lua pattern.
--

	function path.wildcards(pattern)
		-- Escape characters that have special meanings in Lua patterns
		pattern = pattern:gsub("([%+%.%-%^%$%(%)%%])", "%%%1")

		-- Replace wildcard patterns with special placeholders so I don't
		-- have competing star replacements to worry about
		pattern = pattern:gsub("%*%*", "\001")
		pattern = pattern:gsub("%*", "\002")
		
		-- Replace the placeholders with their Lua patterns
		pattern = pattern:gsub("\001", ".*")
		pattern = pattern:gsub("\002", "[^/]*")
		
		return pattern
	end

--
-- remove any dot ("./", "../") patterns from the start of the path
--
	function path.trimdots(p)
		local changed
		repeat
			changed = true
			if p:startswith("./") then
				p = p:sub(3)
			elseif p:startswith("../") then
				p = p:sub(4)
			else
				changed = false
			end
		until not changed

		return p
	end
