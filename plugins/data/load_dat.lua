local datfile = {}

function datfile.open(file, vertag)
	local data = {}
	local ver
	local filepath
	local fh
	for path in mame_manager:ui():options().entries.historypath:value():gmatch("([^;]+)") do
		filepath = lfs.env_replace(path) .. "/" .. file
		fh = io.open(filepath, "rb")
		if fh then
			break
		end
	end
	if not fh then
		return nil
	end
	do
		local inblock = false
		local buffer = fh:read("a")
		if vertag then
			local match = buffer:match(vertag .. "%s*([^%s]+)")
			if match then
				ver = match
			end
		end
		local function gmatchpos()
			local pos = 1
			local function iter()
				local spos, epos = buffer:find("\n$", pos, true)
				if not spos then
					return nil
				end
				spos = spos + 1
				local spos, epos, match = buffer:find("([^\n]+)", spos)
				pos = epos + 1
				return match, pos, iter
			end
			return iter
		end
		for line, epos, iter in gmatchpos() do

			local flag = line:sub(1, 1)
			if flag ~= "#" then
				if flag == "$" then
					if line:sub(1, 4) == "$end" then
						inblock = false
					elseif not inblock then
						local tag, set = line:match("^%$([^%s=]+)=?([^%s]*)")
						if set and set ~= "" then
							local tags = {}
							local sets = {}
							local tag1 = ""
							tag:gsub("([^,]+)", function(s) tags[#tags + 1] = s end)
							set:gsub("([^,]+)", function(s) sets[#sets + 1] = s end)
							repeat
								tag1, epos = iter()
							until tag1:sub(1, 1) == "$"
							tag1 = tag1:match("^$([^%s]*)")
							if not data[tag1] then
								data[tag1] = {}
							end
							for num1, tag2 in pairs(tags) do
								if not data[tag1][tag2] then
									data[tag1][tag2] = {}
								end
								for num2, set in pairs(sets) do
									data[tag1][tag2][set] = epos
								end
							end
						end
						inblock = true
					end
				end
			end
		end
	end
	fh:close()
	fh = io.open(filepath, "r")
	local function read(tag1, tag2, set)
		local output = {}
		if not data[tag1][tag2][set] then
			return nil
		end
		fh:seek("set", data[tag1][tag2][set])
		for line in fh:lines() do
			if line:sub(1, 4) == "$end" then
				return table.concat(output, "\n")
			end
			output[#output + 1] = line
		end
	end

	return read, ver
end

return datfile
