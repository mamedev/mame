local lfs = require("lfs")

-- Returns true if dirname is an existing directory, false if not a directory,
-- or nil, an error message and a system dependent error code on error.
local function is_dir(dirname)
	local ret, err, code = lfs.attributes(dirname, "mode")
	if ret == nil then
		return ret, err, code
	else
		return ret == "directory"
	end
end

-- Get the directory name for the file.
local function dirname(filename)
	if filename == "/" then
		return "/"
	end
	local parent = filename:match("(.*)/.")
	if parent then
		if parent == "" then
			parent = "/"
		end
		return parent
	end
	return "."
end

-- Create dir and parents for dir if needed. Returns true on success,
-- nil, an error message and a system dependent error code on error.
local function mkdir_recursive(dir)
	local ret, err, code = is_dir(dir)
	if ret == true then
		return true
	end
	local parent = dirname(dir)
	local ret, err, code = mkdir_recursive(parent)
	if not ret then
		return ret, err, code
	end
	return lfs.mkdir(dir)
end

-- Create the parents of the file recursively if needed, returns true on success,
-- or nil, an error message and a system dependent error code on error.
local function create_parent_dirs(filename)
	local parent = dirname(filename)
	return mkdir_recursive(parent)
end

return {
	dirname = dirname,
	mkdir_recursive = mkdir_recursive,
	create_parent_dirs = create_parent_dirs
}
