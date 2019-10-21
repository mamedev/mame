--
-- path.lua
-- Path manipulation functions.
-- Copyright (c) 2002-2010 Jason Perkins and the Premake project
--


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
-- Returns the common base directory of two paths.
--

	function path.getcommonbasedir(a, b)
		a = path.getdirectory(a)..'/'
		b = path.getdirectory(b)..'/'

		-- find the common leading directories
		local idx = 0
		while (true) do
			local tst = a:find('/', idx + 1, true)
			if tst then
				if a:sub(1,tst) == b:sub(1,tst) then
					idx = tst
				else
					break
				end
			else
				break
			end
		end
		-- idx is the index of the last sep before path string 'a' ran out or didn't match.
		local result = ''
		if idx > 1 then
			result = a:sub(1, idx - 1)	-- Remove the trailing slash to be consistent with other functions.
		end
		return result
	end


--
-- Returns true if the filename has a particular extension.
--
-- @param fname
--    The file name to test.
-- @param extensions
--    The extension(s) to test. Maybe be a string or table.
--

	function path.hasextension(fname, extensions)
		local fext = path.getextension(fname):lower()
		if type(extensions) == "table" then
			for _, extension in pairs(extensions) do
				if fext == extension then
					return true
				end
			end
			return false
		else
			return (fext == extensions)
		end
	end

--
-- Returns true if the filename represents a C/C++ source code file. This check
-- is used to prevent passing non-code files to the compiler in makefiles. It is
-- not foolproof, but it has held up well. I'm open to better suggestions.
--

	function path.iscfile(fname)
		return path.hasextension(fname, { ".c", ".m" })
	end

	function path.iscppfile(fname)
		return path.hasextension(fname, { ".cc", ".cpp", ".cxx", ".c++", ".c", ".m", ".mm" })
	end

	function path.iscxfile(fname)
		return path.hasextension(fname, ".cx")
	end

	function path.isobjcfile(fname)
		return path.hasextension(fname, { ".m", ".mm" })
	end

	function path.iscppheader(fname)
		return path.hasextension(fname, { ".h", ".hh", ".hpp", ".hxx" })
	end

	function path.isappxmanifest(fname)
		return path.hasextension(fname, ".appxmanifest")
	end

	function path.isandroidbuildfile(fname)
		return path.getname(fname) == "AndroidManifest.xml"
	end

	function path.isnatvis(fname)
		return path.hasextension(fname, ".natvis")
	end

	function path.isasmfile(fname)
		return path.hasextension(fname, { ".asm", ".s", ".S" })
	end

	function path.isvalafile(fname)
		return path.hasextension(fname, ".vala")
	end

	function path.isswiftfile(fname)
		return path.hasextension(fname, ".swift")
	end

	function path.issourcefile(fname)
		return path.iscfile(fname)
			or path.iscppfile(fname)
			or path.iscxfile(fname)
			or path.isasmfile(fname)
			or path.isvalafile(fname)
			or path.isswiftfile(fname)
	end

	function path.issourcefilevs(fname)
		return path.hasextension(fname, { ".cc", ".cpp", ".cxx", ".c++", ".c" })
			or path.iscxfile(fname)
	end

--
-- Returns true if the filename represents a compiled object file. This check
-- is used to support object files in the "files" list for archiving.
--

	function path.isobjectfile(fname)
		return path.hasextension(fname, { ".o", ".obj" })
	end

--
-- Returns true if the filename represents a Windows resource file. This check
-- is used to prevent passing non-resources to the compiler in makefiles.
--

	function path.isresourcefile(fname)
		return path.hasextension(fname, ".rc")
	end


--
-- Returns true if the filename represents a Windows image file. 
--

	function path.isimagefile(fname)
		local extensions = { ".png" }
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
-- Replace the file extension.
--

	function path.replaceextension(p, newext)
		local ext = path.getextension(p)

		if not ext then
			return p
		end

		if #newext > 0 and not newext:findlast(".", true) then
			newext = "."..newext
		end

		return p:match("^(.*)"..ext.."$")..newext
	end
