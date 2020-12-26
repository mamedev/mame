-- to use this get the package from http://greatstone.free.fr/hi2txt/
-- extract the hi2txt.zip and place it in your history path

local dat = {}
local env = {}
local output
local curset

function env.open(file, size)
	if file == ".hi" then
		local path = "hi"
		local ini = emu.file(emu.subst_env(manager.options.entries.inipath:value()), 1)
		local ret = ini:open("hiscore.ini")
		if not ret then
			local inifile = ini:read(ini:size())
			for line in inifile:gmatch("[^\n\r]") do
				token, value = string.match(line, '([^ ]+) ([^ ]+)');
				if token == "hi_path" then
					path = value
					break
				end
			end
		end
		file = path .. "/" .. curset .. ".hi"
	else
		file = emu.subst_env(manager.options.entries.nvram_directory:value()) .. "/" .. curset .. "/" .. file
	end
	local f = io.open(file, "rb")
	local content = f:read("*all")
	f:close()
	if #content < size then
		content = content .. string.rep("\0", size - #content)
	end
	return content
end

function env.endianness(bytes, endian)
	local newbytes = {}
	if endian == "little_endian" then
		for i = 1, #bytes do
			newbytes[i] = bytes[#bytes - i + 1]
		end
	else
		newbytes = bytes
	end
	return newbytes
end

function env.byte_skip(bytes, skip)
	local newbytes = {}
	if skip == "odd" then
		-- lua lists are 1 based so use even indexes
		for i = 2, #bytes, 2 do
			newbytes[i/2] = bytes[i]
		end
	elseif skip == "even" then
		for i = 1, #bytes, 2 do
			newbytes[(i+1)/2] = bytes[i]
		end
	elseif skip == "1000" then
		for i = 1, #bytes, 4 do
			newbytes[(i+3)/4] = bytes[i]
		end
	elseif skip == "0100" then
		for i = 2, #bytes, 4 do
			newbytes[(i+2)/4] = bytes[i]
		end
	elseif skip == "0010" then
		for i = 3, #bytes, 4 do
			newbytes[(i+1)/4] = bytes[i]
		end
	elseif skip == "0001" then
		for i = 4, #bytes, 4 do
			newbytes[i/4] = bytes[i]
		end
	else
		skip = tonumber(skip)
		for i = 1, #bytes do
			if bytes[i] ~= skip then
				newbytes[#newbytes + 1] = bytes[i]
			end
		end
	end
	return newbytes
end

function env.byte_trim(bytes, val)
	val = tonumber(val)
	len = #bytes
	for i = 1, len do
		if bytes[1] ~= val then
			return bytes
		end
		table.remove(bytes, 1)
	end
	return bytes
end

function env.byte_trunc(bytes, val)
	val = tonumber(val)
	for i = 1, #bytes do
		if bytes[i] == val then
			break
		end
	end
	while #bytes >= i do
		table.remove(bytes)
	end
	return bytes
end

function env.byte_swap(bytes, val)
	local newbytes = {}
	val = tonumber(val)
	for i = 1, #bytes do
		local off = i + val - 1 - 2 * ((i - 1) % val)
		if off > #bytes then -- ??
			break
		end
		newbytes[i] = bytes[off]
	end
	return newbytes
end

function env.nibble_skip(bytes, skip)
	local newbytes = {}
	if skip == "odd" then
		for i = 1, #bytes, 2 do
			val1 = bytes[i]:byte(1)
			val2 = bytes[i+1]:byte(1)
			newbytes[(i+1)/2] = string.char(((val1 & 0x0f) << 4) | (val2 & 0x0f))
		end
	elseif skip == "even" then
		for i = 1, #bytes, 2 do
			val1 = bytes[i]:byte(1)
			val2 = bytes[i+1]:byte(1)
			newbytes[(i+1)/2] = string.char((val1 & 0xf0) | ((val2 & 0xf0) >> 4))
		end
	end
	return newbytes
end

function env.bit_swap(bytes, swap)
	if swap == "yes" then
		for i = 1, #bytes do
			val = bytes[i]:byte(1)
			bytes[i] = string.char(((val & 1) << 7) | ((val & 2) << 5) | ((val & 4) << 3) | ((val & 8) << 1) | ((val & 0x10) >> 1) | ((val & 0x20) >> 3) | ((val & 0x40) >> 5) | ((val & 0x80) >> 7))
		end
	end
	return bytes
end

function env.bitmask(bytes, mask)
	local newbytes = 0
	bytes = string.unpack(">I" .. #bytes, table.concat(bytes))
	for i = 1, #mask do
		newbytes = newbytes | (((bytes >> mask.ishift) & mask.mask) << mask.oshift)
	end
	bytes = {}
	while newbytes ~= 0 do
		bytes[#bytes + 1] = newbytes & 0xff
		newbytes = newbytes >> 8
	end
	newbytes = {}
	for i = 1, #bytes do
		newbytes[i] = string.char(bytes[#bytes + 1 - i])
	end
	return newbytes
end

function env.frombcd(val)
	local result = 0
	local mul = 1
	while val ~= 0 do
		result = result + ((val % 16) * mul)
		val = val >> 4
		mul = mul * 10
	end
	return result
end

function env.basechar(bytes, base)
	emu.print_verbose("data_hiscore: basechar " .. base .. " unimplemented\n")
	if base == "32" then
	elseif base == "40" then
	end
	return bytes
end

function env.charset_conv(bytes, charset)
	if type(charset) == "string" then
		local chartype, offset, delta = charset:match("CS_(%w*)%[?(%-?%d?%d?),?(%d?%d?)%]?")
		if chartype == "NUMBER" then

		end
		emu.print_verbose("data_hiscore: charset " .. chartype .. " unimplemented\n")
		return bytes
	end
	for num, char in ipairs(bytes) do
		char = string.byte(char)
		if charset[char] then
			bytes[num] = charset[char]
		elseif charset.default then
			bytes[num] = charset.default
		end
	end
	return bytes
end

function env.ascii_step(bytes, step)
	for num, char in ipairs(bytes) do
		bytes[num] = string.char(char:byte() / step)
	end
	return bytes
end

function env.ascii_offset(bytes, offset)
	for num, char in ipairs(bytes) do
		bytes[num] = string.char(char:byte() + offset)
	end
	return bytes
end

env.tostring = tostring
env.type = type
env.table = { pack = table.pack, concat = table.concat }
env.string = { unpack = string.unpack, format = string.format, rep = string.rep, gsub = string.gsub, lower = string.lower, upper = string.upper }
env.math = { min = math.min, max = math.max, floor = math.floor }

do
	local function readonly(t)
		local mt = { __index = t, __newindex = function(t, k, v) return end }
		return setmetatable({}, mt)
	end
	env.table = readonly(env.table)
	env.string = readonly(env.string)
	env.math = readonly(env.math)
	env = readonly(env)
end

function dat.check(set, softlist)
	if softlist then
		return nil
	end
	local datpath
	local function xml_parse(file)
		local table
		datpath = file:fullpath():gsub(".zip", "/")
		local data = file:read(file:size())
		data = data:match("<hi2txt.->(.*)</ *hi2txt>")
		local function get_tags(str, parent)
			local arr = {}
			while str ~= "" do
				local tag, attr, stop
				tag, attr, stop, str = str:match("<([%w!_%-]+) ?(.-)(/?)[ %-]->(.*)")

				if not tag then
					return arr
				end
				if tag:sub(0, 3) ~= "!--" then
					local block = {}
					if stop ~= "/" then
						local nest
						nest, str = str:match("(.-)</ *" .. tag .. " *>(.*)")
						local children = get_tags(nest, tag)
						if not next(children) then
							nest = nest:gsub("<!--.-%-%->", "")
							nest = nest:gsub("^%s*(.-)%s*$", "%1")
							block["text"] = nest
						else
							block = children
						end
					end
					if attr then
						for name, value in attr:gmatch("([-%w]-)=\"(.-)\"") do
							block[name] = value:gsub("^(.-)$", "%1")
						end
					end
					if not arr[tag] then
						arr[tag] = {}
					end
					if parent == "structure" or parent == "output" or parent == "format" or parent == "loop" or parent == "concat" then
						block["tag"] = tag
						arr[#arr + 1] = block
					elseif tag == "charset" or tag == "bitmask" then
						arr[tag][block["id"]] = block
					elseif tag == "case" or tag == "char" then
						arr[tag][block["src"]] = block
					else
						arr[tag][#arr[tag] + 1] = block
					end
				end
			end
			return arr
		end
		return get_tags(data, "")
	end

	local function parse_table(xml)
		local total_size = 0
		local s = { "local data = open('" .. xml.structure[1].file .. "', size)\nlocal offset = 1\nlocal arr = {}",
				"local elem, bytes, offset, value, lastindex, output"}
		local fparam = {}
		if xml.bitmask then
			local bitmask = "local bitmask = {"
			for id, masks in pairs(xml.bitmask) do
				bitmask = bitmask .. "['" .. id .. "'] = {"
				for num, mask in ipairs(masks.character) do
					mask = mask:gsub("( )", "")
					local newmask = tonumber(mask, 2)
					local shift = 0
					local count = 8
					while (newmask & 1) == 0 do
						newmask = newmask >> 1
						shift = shift + 1
					end
					if masks.byte-completion and masks.byte-completion == "no" then
						count = 0
						while (newmask >> count) & 1 == 1 do
							count = count + 1
						end
					end
					bitmask = bitmask .. "{ mask = " .. newmask .. ", ishift = " .. shift .. ", oshift = " .. count .. "},"
				end
				bitmask = bitmask .. "},"
			end
			s[#s + 1] = bitmask .. "}"
		end
		if xml.charset then
			local charset = "local charset = {"
			for id, set in pairs(xml.charset) do
				local default
				charset = charset .. "['" .. id .. "'] = {"
				for src, char in pairs(set.char) do
					if char.default and char.default == "yes" then
						default = char.dst
					end
					charset = charset .. "[" .. src .. "]" .. " = '" .. char.dst .. "',"
				end
				if default then
					charset = charset .. "default = " .. default
				end
				charset = charset .. "},"
			end
			s[#s + 1] = charset .. "}"
		end

		local function check_format(formstr)
			local formats = {}
			local ret = "local function tempform(val)"
			formstr = formstr:gsub("&gt;", ">")
			formstr:gsub("([^;]+)", function(s) formats[#formats + 1] = s end)
			for num, form in ipairs(formats) do
				local oper
				local first, rest = form:match("^(.)(.*)$")
				if first == "*" and tonumber(rest) then
					oper = " val = val * " .. rest
				elseif first == "/" and tonumber(rest) then
					oper = " val = val / " .. rest
				elseif first == "d" and tonumber(rest) then
					oper = " val = math.floor(val / " .. rest .. ")"
				elseif first == "D" and tonumber(rest) then
					oper = " val = math.floor((val / " .. rest .. ") + 0.5)"
				elseif first == "-" and tonumber(rest) then
					oper = " val = val - " .. rest
				elseif first == "+" and tonumber(rest) then
					oper = " val = val + " .. rest
				elseif first == "%" and tonumber(rest) then
					oper = " val = val % " .. rest
				elseif first == ">" and tonumber(rest) then
					oper = " val = val << " .. rest
				elseif first == "L" and (rest == "C" or rest == "owercase") then
					oper = " val = val:lower()"
				elseif first == "U" and (rest == "C" or rest == "ppercase") then
					oper = " val = val:upper()"
				elseif first == "C" and rest == "apitalize" then
					oper = " val = val:gsub('^(.)', function(s) return s:upper() end)"
				elseif first == "R" and (rest == "" or rest == "ound") then
					oper = " val = math.floor(" .. var .. " + 0.5)"
				elseif first == "T" then
					local trim, char = rest:match("rim(L?R?)(.)$")
					if trim == "L" or trim == "" then
						oper = " val = val:gsub('^(" .. char .. "*)', '')"
					end
					if trim == "R" or trim == "" then
						oper = " val = val:gsub('(" .. char .. "*)$', '')"
					end
				elseif first == "P" then
					local pad, count, char = rest:match("ad(L?R?)(%d-)(.)$")
					if pad == "L" then
						oper = " val = string.rep('" .. char .. "', " .. count .. " - #val) .. val"
					elseif pad == "R" then
						oper = "val = val .. string.rep('" .. char .. "', " .. count .. " - #val)"
					elseif pad == nil then
						local prefix = rest:match("refix(.*)")
						if prefix then
							oper = " val = '" .. rest .. "' .. val"
						end
					end
				elseif first == "S" then
					local suffix = rest:match("uffix(.*)")
					if suffix then
						oper = " val = val .. '" .. rest .. "'"
					end
				elseif (first == "h" and rest == "exadecimal_string") or (first == "0" and rest == "x") then
					oper = " val = string.format('0x%x', val)"
				elseif (first == "L" and rest == "oopIndex") then
					oper = " val = index"
				end
				if not oper then
					oper = " val = format['" .. form .. "'](val, {"
					for num1, colpar in ipairs(fparam[form]) do
						oper = oper .. "arr['" .. colpar .. "'][i].val or arr['" .. colpar .. "'][1].val,"
					end
					oper = oper .. "})"
				end
				ret = ret .. oper
			end
			return ret .. " return val\nend"
		end

		if xml.format then
			local format = { "local format = {" }
			for num, form in ipairs(xml.format) do
				local param = {}
				format[#format + 1] = "['" .. form["id"] .. "'] = "
				if form["input-as-subcolumns-input"] then
					--format[#format + 1] = "input_as_subcolumns_input = '" .. form["input-as-subcolumns-input"] .. "',"
					emu.print_verbose("data_hiscore: input-as-subcolumns-input unimplemented\n")
				end
				format[#format + 1] = "function(val, param) "
				if form["formatter"] then
					format[#format + 1] = "local function tempform(val) "
				end
				if form["apply-to"]  == "char" then
					format[#format + 1] = "val = val:gsub('(.)', function(val) "
				end
				format[#format + 1] = "local temp = val"
				for num1, op in ipairs(form) do
					if op.tag == "add" then
						format[#format + 1] = "val = val + " .. op.text
					elseif op.tag == "prefix" then
						format[#format + 1] = "val = '" .. op.text .. "' .. val "
					elseif op.tag == "suffix" then
						format[#format + 1] = "val = val .. '" .. op.text .. "'"
					elseif op.tag == "multiply" then
						format[#format + 1] = "val = val * " .. op.text
					elseif op.tag == "divide" then
						format[#format + 1] = "val = val / " .. op.text
					elseif op.tag == "sum" then
						format[#format + 1] = "val = 0"
						for num2, col in ipairs(op) do
							param[#param + 1] = col["id"]
							if col["format"] then
								local colform = check_format(col["format"])
								format[#format + 1] = colform .. " val = val + tempform(val)"
							else
								format[#format + 1] = "val = val + param[" .. #param .. "]"
							end
						end
					elseif op.tag == "concat" then
						format[#format + 1] = "val = ''"
						for num2, col in ipairs(op) do
							if col["tag"] == "txt" then
								format[#format + 1] = "val = val .. '" .. col["text"] .. "'"
							elseif col["format"] then
								param[#param + 1] = col["id"]
								local n = #param
								format[#format + 1] = function() return " " .. check_format(col["format"]) .. " val = val .. tempform(param[" .. n .. "])" end
							else
								param[#param + 1] = col["id"]
								format[#format + 1] = "val = val .. param[" .. #param .. "]"
							end
						end
					elseif op.tag == "min" then
						format[#format + 1] = "val = 0x7fffffffffffffff"
						for num2, col in ipairs(op) do
							param[#param + 1] = col["id"]
							format[#format + 1] = "val = math.min(val, param[" .. #param .. "])"
						end
					elseif op.tag == "max" then
						format[#format + 1] = "val = 0"
						for num2, col in ipairs(op) do
							param[#param + 1] = col["id"]
							format[#format + 1] = "val = math.max(val, param[" .. #param .. "])"
						end
					elseif op.tag == "pad" then
						format[#format + 1] = "if type(val) == 'number' then val = tostring(val) end"
						format[#format + 1] = "if #val < " .. op.max .. " then"
						if op.direction == "left" then
							format[#format + 1] = "val = string.rep('" .. op.text .. "', " .. op.max .. " - #val) .. val"
						elseif op.direction == "right" then
							format[#format + 1] = "val = val .. string.rep('" .. op.text .. "', " .. op.max .. " - #val)"
						end
						format[#format + 1] = "end"
					elseif op.tag == "trim" then
						if op.direction == "left" or op.direction == "both" then
							format[#format + 1] = "val = val:gsub('^(" .. op.text .. "*)', '')"
						end
						if op.direction == "right" or op.direction == "both" then
							format[#format + 1] = "val = val:gsub('(" .. op.text .. "*)$', '')"
						end
					elseif op.tag == "substract" then
						format[#format + 1] = "val = val - " .. op.text
					elseif op.tag == "remainder" then
						format[#format + 1] = "val = val % " .. op.text
					elseif op.tag == "trunc" then
						format[#format + 1] = "val = math.floor(val)"
					elseif op.tag == "round" then
						format[#format + 1] = "val = math.floor(val + 0.5)"
					elseif op.tag == "divide_trunc" then
						format[#format + 1] = "val = math.floor(val / " .. op.text .. ")"
					elseif op.tag == "divide_round" then
						format[#format + 1] = "val = math.floor((val / " .. op.text .. ") + 0.5)"
					elseif op.tag == "replace" then
						format[#format + 1] = "val = val:gsub('" .. op.src .. "', '" .. op.dst .. "')"
					elseif op.tag == "shift" then
						format[#format + 1] = "val = val << " .. op.text
					elseif op.tag == "lowercase" then
						format[#format + 1] = "val = val:lower()"
					elseif op.tag == "uppercase" then
						format[#format + 1] = "val = val:upper()"
					elseif op.tag == "capitalize" then
						format[#format + 1] = "val = val:gsub('^(.)', function(s) return s:upper() end)"
					elseif op.tag == "loopindex" then
						param[#param + 1] = "loopindex"
						format[#format + 1] = "val = param[" .. #param .. "]"
					elseif op.tag == "case" then
						format[#format + 1] = "val = temp"
						if not tonumber(op["src"]) then
							op["src"] = "'" .. op["src"] .. "'"
						end
						if not tonumber(op["dst"]) then
							op["dst"] = "'" .. op["dst"] .. "'"
						end
						if op["default"] == "yes" then
							format[#format + 1] = "local default = " .. op["dst"]
						end
						if op["operator-format"] then
							format[#format + 1] = function() return " val = ".. check_format(col["operator-format"]) end
						end
						if not op["operator"] then
							op["operator"] = "=="
						else
							op["operator"] = op["operator"]:gsub("&lt;", "<")
							op["operator"] = op["operator"]:gsub("&gt;", ">")
						end
						format[#format + 1] = "if val " .. op["operator"] .. " " .. op["src"] .. " then"
						format[#format + 1] = "val = " .. op["dst"]
						if op["format"] then
							format[#format + 1] = function() return " val = ".. check_format(col["operator-format"]) end
						end
						format[#format + 1] = "return val\n end"
					end

				end
				fparam[form["id"]] = param
				if form["apply-to"]  == "char" then
					format[#format + 1] = "if default then\nreturn default\nend\nreturn val\nend)\nreturn val\nend"
				else
					format[#format + 1] = "if default then\nreturn default\nend\nreturn val\nend"
				end
				if form["formatter"] then
					format[#format + 1] = "return string.format('" .. form["formatter"] .. "', tempform(val))\nend"
				end
				format[#format + 1] = ","
			end
			for num, line in ipairs(format) do
				if type(line) == "string" then
					s[#s + 1] = line
				elseif type(line) == "function" then
					s[#s + 1] = line()
				end
			end
			s[#s + 1] = "}"
		end
		local function parse_elem(elem, loopelem)
			local ret = 0
			if elem["tag"] == "loop" then
				if elem["skip-first-bytes"] then
					s[#s + 1] = "offset = offset + " .. elem["skip-first-bytes"]
				end
				s[#s + 1] = "for i = 1, " .. elem["count"] .. " do"
				for num, elt in ipairs(elem) do
					index = parse_elem(elt, elem)
				end
				s[#s + 1] = "end"
				if elem["skip-last-bytes"] then
					s[#s + 1] = "offset = offset + " .. elem["skip-last-bytes"]
				end
			elseif elem["tag"] == "elt" then
				s[#s + 1] = "if not arr['" .. elem["id"] .. "'] then arr['" .. elem["id"] .. "'] = {} end\nelem = {}"
				s[#s + 1] = "bytes = table.pack(string.unpack('" .. string.rep("c1", elem["size"]) .. "', data, offset))"
				if loopelem then
					total_size = total_size + elem["size"] * loopelem["count"]
				else
					total_size = total_size + elem["size"]
				end
				s[#s + 1] = "offset = bytes[#bytes]\nbytes[#bytes] = nil"
				if elem["decoding-profile"] then
					if elem["decoding-profile"] == "base-40" then
						elem["src-unit-size"] = 16
						elem["base"] = "40"
						elem["dst-unit-size"] = 3
						elem["ascii-offset"] = 64
					elseif elem["decoding-profile"] == "base-32" then
						elem["src-unit-size"] = 16
						elem["base"] = "32"
						elem["dst-unit-size"] = 3
						elem["ascii-offset"] = 64
					elseif elem["decoding-profile"] == "bcd" then
						elem["endianness"] = "big-endian"
						elem["nibble-skip"] = "odd"
						elem["base"] = "16"
					elseif elem["decoding-profile"] == "bcd-le" then
						elem["endianness"] = "big-endian"
						elem["nibble-skip"] = "odd"
						elem["base"] = "16"
					end
				end
				local bytedec = elem["swap-skip-order"] or "byte-swap;bit-swap;byte-skip;endianness;byte-trim;nibble-skip;bitmask"
				local bytedecl = {}
				bytedec:gsub("([^;]*)", function(c) bytedecl[#bytedecl + 1] = c end)
				for num, func in ipairs(bytedecl) do
					if elem[func] then
						fixfunc = func:gsub("-", "_")
						if func == "bitmask" then
							s[#s + 1] = "bytes = " .. fixfunc .. "(bytes, bitmask['" .. elem[func] .. "'])"
						else
							s[#s + 1] = "bytes = " .. fixfunc .. "(bytes, '" .. elem[func] .. "')"
						end
					end
				end
				if elem["type"] == "int" then
					s[#s + 1] = "value = string.unpack('>I' .. #bytes, table.concat(bytes))"
					elem["base"] = elem["base"] or "10"
					if elem["base"] == "16" then
						s[#s + 1] = "value = frombcd(value)"
					end
					s[#s + 1] = "elem.val = value"
				elseif elem["type"] == "text" then
					if elem["ascii-step"] then
						s[#s + 1] = "bytes = ascii_step(bytes, " .. elem["ascii-step"] .. ")"
					end
					if elem["ascii-offset"] then
						s[#s + 1] = "bytes = ascii_offset(bytes, " .. elem["ascii-offset"] .. ")"
					end
					if elem["base"] then
						s[#s + 1] = "bytes = basechar(bytes, " .. elem["base"] .. ")"
					end
					if elem["charset"] then
						local charsets = {}
						elem["charset"]:gsub("([^;]*)", function(s) charsets[#charsets + 1] = s return "" end)
						for num, charset in pairs(charsets) do
							if charset:match("^CS_") then
								s[#s + 1] = "bytes = charset_conv(bytes, " .. charset .. ")"
							elseif charset ~= "" then
								s[#s + 1] = "bytes = charset_conv(bytes, charset['" .. charset .. "'])"
							end
						end
					end
					s[#s + 1] = "elem.val = table.concat(bytes)"
				end
				local index
				if elem["table-index"] then
					index = tonumber(elem["table-index"])
				end
				if not index and loopelem then
					local total = loopelem["count"]
					local start = loopelem["start"] or 0
					local step = loopelem["step"] or 1
					local ref, reftype
					if elem["table-index"] then
						elem["table-index"]:match("([%w ]*):([%a_]*)")
					end
					if not elem["table-index"] or elem["table-index"] == "loop_index" then
						index = "(i - 1) * " .. step .. " + " .. start
					elseif elem["table-index"] == "loop_reverse_index" then
						index = "(" .. total  .. "- i) * " .. step .. " + " .. start
					elseif elem["table-index"] == "itself" then
						index = "value"
					elseif elem["table-index"] == "last" then
						index = "lastindex"
					elseif reftype then
						index = reftype .. "(arr, '" .. ref .. "')"
					end
				end
				if index then
					s[#s + 1] = "elem.index = " .. index
					s[#s + 1] = "lastindex = elem.index"
				end
				s[#s + 1] = "arr['" .. elem["id"] .. "'][#arr['" .. elem["id"] .. "'] + 1] = elem"
			end
			return index
		end
		for num, elem in ipairs(xml.structure[1]) do
			if elem["tag"] == "loop" or elem["tag"] == "elt" then
				parse_elem(elem)
			end
		end
		table.insert(s, 1, "local size = " .. total_size)

		s[#s + 1] = "output = ''"

		for num1, fld in ipairs(xml.output[1]) do
			if not fld["display"] or fld["display"] == "always" then
				if fld["tag"] == "field" then
					if not fld["src"] then
						fld["src"] = fld["id"]
					end
					s[#s + 1] = "output = output .. '" .. fld["id"] .. " '"
					s[#s + 1] = "value = arr['" .. fld["src"] .. "'][1]"
					if fld["format"] then
						s[#s + 1] = check_format(fld["format"])
						s[#s + 1] = "value = tempform(value)"
					end
					s[#s + 1] = "output = output .. value .. '\\n'"
				elseif fld["tag"] == "table" then
					local head = {}
					local dat = {}
					local loopcnt
					local igncol, ignval
					if fld["line-ignore"] then
						igncol, ignval = fld["line-ignore"]:match("([^:]*):(.*)")
					end
					if fld["field"] and not fld["column"] then -- ????
						fld["column"] = fld["field"]
					end
					for num2, col in ipairs(fld["column"]) do
						if not col["display"] or col["display"] == "always" then
							if not col["src"] then
								col["src"] = col["id"]
							end
							if not loopcnt and col["src"] ~= "index" then
								table.insert(dat, 1, "for i = 1, #arr['" .. col["src"] .. "'] do")
								table.insert(dat, 2, "local index = arr['" .. col["src"] .. "'][i].index or i - 1")
								table.insert(dat, 3, "local line = ''")
								loopcnt = true
							end
							head[#head + 1] = "output = output .. '" .. col["id"] .. "\\t'"
							if col["src"] == "index" then
								dat[#dat + 1] = "value = index"
							else
								dat[#dat + 1] = "if arr['"  .. col["src"] .. "'] then value = arr['" .. col["src"] .. "'][i].val end"
							end
							if col["format"] then
								dat[#dat + 1] = check_format(col["format"])
								dat[#dat + 1] = "value = tempform(value)"
							end
							if igncol == col["id"] then
								dat[#dat + 1] = "local checkval = value"
							end
							dat[#dat + 1] = "line = line .. value .. '\\t'"
						end
					end
					if igncol then
						dat[#dat + 1] = "if checkval ~= " .. ignval .. " then output = output .. line .. '\\n' end\nend"
					else
						dat[#dat + 1] = "output = output .. line .. '\\n'\nend"
					end
					s[#s + 1] = table.concat(head, "\n") .. "\noutput = output .. '\\n'"
					s[#s + 1] = table.concat(dat, "\n")
				end
			end
		end
		s[#s + 1] = "return output"

		-- cache script
		local script = table.concat(s, "\n")
		local scrpath = datpath .. "/"
		local scrfile = io.open(scrpath .. set .. ".lua", "w+")
		if not scrfile then
			lfs.mkdir(scrpath)
			scrfile = io.open(scrpath .. set .. ".lua", "w+")
		end
		if scrfile then
			scrfile:write(script)
		end
		return script
	end

	if curset == set then
		if output then
			return _("High Scores")
		else
			return nil
		end
	end
	output = nil
	curset = set

	local scrfile = emu.file(emu.subst_env(mame_manager.ui.options.entries.historypath:value():gsub("([^;]+)", "%1/hi2txt")), 1)
	local ret = scrfile:open(set .. ".lua")
	local script
	if ret then
		function get_xml_table(fileset)
			local file = emu.file(emu.subst_env(mame_manager.ui.options.entries.historypath:value():gsub("([^;]+)", "%1/hi2txt")), 1)
			local ret = file:open(fileset .. ".xml")
			if ret then
				return nil
			end
			local xml = xml_parse(file)
			return xml
		end
		local xml = get_xml_table(set)
		if not xml then
			return nil
		end
		if xml.sameas then
			xml = get_xml_table(xml.sameas[1].id)
		end
		local status
		status, script = pcall(parse_table, xml)
		if not status then
			emu.print_verbose("error creating hi score parse script: " .. script)
			return nil
		end
	else
		script = scrfile:read(scrfile:size())
	end
	local scr, err = load(script, script, "t", env)
	if err then
		emu.print_verbose("error loading hi score script file: " .. err)
	else
		status, output = pcall(scr, xml_table)
		if not status then
			emu.print_verbose("error in hi score parse script: " .. output)
			output = nil
		end
	end
	if output then
		return _("High Scores")
	else
		return nil
	end
end

function dat.get()
	return output
end

return dat
