local xml = {}

-- basic xml parser for mamecheat only
local function xml_parse(data)
	local function fix_gt(str)
		str = str:gsub(">=", " ge ")
		str = str:gsub(">", " gt ")
		return str
	end
	data = data:gsub("(condition=%b\"\")", fix_gt)
	local cheat_str = data:match("<mamecheat.->(.*)</ *mamecheat>")

	local function get_tags(str)
		local arr = {}
		while str ~= "" do
			local tag, attr, stop
			tag, attr, stop, str = str:match("<([%w!-]+) ?(.-)(/?)[ -]->(.*)")

			if not tag then
				return arr
			end
			if tag:sub(0, 3) ~= "!--" then
				local block = {}
				if stop ~= "/" then
					local nest
					nest, str = str:match("(.-)</ *" .. tag .. " *>(.*)")
					local children = get_tags(nest)
					if not next(children) then
						nest = nest:gsub("<!--.--->", "")
						nest = nest:gsub("^%s*(.-)%s*$", "%1")
						block["text"] = nest
					else
						block = children
					end
				end
				if attr then
					for name, value in attr:gmatch("(%w-)=\"(.-)\"") do
						block[name] = value:gsub("^%s*(.-)%s*$", "%1")
					end
				end
				if not arr[tag] then
					arr[tag] = {}
				end
				arr[tag][#arr[tag] + 1] = block
			end
		end
		return arr
	end
	local xml_table = get_tags(cheat_str)
	return xml_table
end

function xml.conv_cheat(data)
	local spaces, regions, output
	data = xml_parse(data)
	local function convert_expr(data)
		local write = false
	
		local function convert_memref(cpu, space, width, addr, rw)
			local direct = ""
			local count
			if space == "p" then
				fullspace = "program"
			elseif space == "d" then
				fullspace = "data"
			elseif space == "i" then
				fullspace = "io"
			elseif space == "r" or space == "o" then
				fullspace = "program"
				direct = "direct_"
				space = "p"
			end
			if width == "b" then
				width = "u8"
			elseif width == "w" then
				width = "u16"
			elseif width == "d" then
				width = "u32"
			elseif width == "q" then
				width = "u64"
			end
			local cpuname = cpu:gsub(":", "_")
			if space == "m" then
				regions[cpuname .. space] = ":" .. cpu
			else
				spaces[cpuname .. space] = { tag = ":" .. cpu, type = fullspace }
			end
			if rw == "=" then
				write = true
				ret = cpuname .. space .. ":" .. "write_" .. direct .. width .. "(" .. addr .. ","
			else
				ret = cpuname .. space .. ":" .. "read_" .. direct .. width .. "(" .. addr .. ")"
			end
			if rw == "==" then
				ret = ret .. "=="
			end
			return ret
		end

		local function frame()
			output = true
			return "screen:frame_number()"
		end

		data = data:lower()
		data = data:gsub("^[(](.-)[)]$", "%1")
		data = data:gsub("%f[%w]lt%f[%W]", "<")
		data = data:gsub("%f[%w]ge%f[%W]", ">=")
		data = data:gsub("%f[%w]gt%f[%W]", ">")
		data = data:gsub("%f[%w]le%f[%W]", "<=")
		data = data:gsub("%f[%w]eq%f[%W]", "==")
		data = data:gsub("%f[%w]ne%f[%W]", "~=")
		data = data:gsub("!=", "~=")
		data = data:gsub("||", " or ")
		data = data:gsub("%f[%w]frame%f[%W]", frame)
		data = data:gsub("%f[%w]band%f[%W]", "&")
		data = data:gsub("%f[%w]bor%f[%W]", "|")
		data = data:gsub("%f[%w]rshift%f[%W]", ">>")
		data = data:gsub("%f[%w]lshift%f[%W]", "<<")
		data = data:gsub("(%w-)%+%+", "%1 = %1 + 1")
		data = data:gsub("%f[%w](%x+)%f[%W]", "0x%1")
		data = data:gsub("([%w_:]-)%.([pmrodi3]-)([bwdq])@(%w+) *(=*)", convert_memref)
		repeat
			data, count = data:gsub("([%w_:]-)%.([pmrodi3]-)([bwdq])@(%b()) *(=*)", convert_memref)
		until count == 0
		if write then
			data = data .. ")"
		end
		return data
	end

	local function convert_output(data)
		local str = "draw_text(screen,"
		if data["align"] then
			str = str .. data["align"]
		else
			str = str .. "\"left\""
		end
		if data["line"] then
			str = str .. ",\"" .. data["line"] .. "\""
		else
			str = str .. ", \"auto\""
		end
		str = str .. ", nil,\"" .. data["format"] .. "\""
		if data["argument"] then
			for count, block in pairs(data["argument"]) do
				local expr = convert_expr(block["text"])
				if block["count"] then
					for i = 0, block["count"] - 1 do
						str = str .. "," .. expr:gsub("argindex", i)
					end
				else
					str = str .. "," .. expr
				end
			end
		end
		return str .. ")"
	end

	local function convert_script(data)
		local str = ""
		local state = "run"
		for tag, block in pairs(data) do
			if tag == "state" then
				state = block
			elseif tag == "action" then
				for count, action in pairs(block) do
					if action["condition"] then
						str = str .. " if (" .. convert_expr(action["condition"]) .. ") then "
						for expr in action["text"]:gmatch("([^,]+)") do
							str = str .. convert_expr(expr) .. " "
						end
						str = str .. "end"
					else
						for expr in action["text"]:gmatch("([^,]+)") do
							str = str .. " " .. convert_expr(expr) .. " "
						end
					end
				end
			elseif tag == "output" then
				output = true
				for count, output in pairs(block) do
					if output["condition"] then
						str = str .. " if " .. convert_expr(output["condition"]) .. " then "
						str = str .. convert_output(output) .. " end "
					else
						str = str .. " " .. convert_output(output) .. " "
					end
				end
			end
		end
		return state, str
	end

	for count, cheat in pairs(data["cheat"]) do
		spaces = {}
		regions = {}
		output = false
		for tag, block in pairs(cheat) do
			if tag == "comment" then
				data["cheat"][count]["comment"] = block[1]["text"]
			elseif tag == "script" then
				local scripts = {}
				for count2, script in pairs(block) do
					local state, str = convert_script(script)
					scripts[state] = str
				end
				data["cheat"][count]["script"] = scripts
			elseif tag == "parameter" then
				if block[1]["min"] then
					block[1]["min"] = block[1]["min"]:gsub("%$","0x")
				end
				if block[1]["max"] then
					block[1]["max"] = block[1]["max"]:gsub("%$","0x")
				end
				if block[1]["step"] then
					block[1]["step"] = block[1]["step"]:gsub("%$","0x")
				end
			data["cheat"][count]["parameter"] = block[1]
			end
		end
		if next(spaces) then
			data["cheat"][count]["space"] = {} 
			for name, space in pairs(spaces) do
				data["cheat"][count]["space"] = {} 
				data["cheat"][count]["space"][name] = { type = space["type"], tag = space["tag"] }
			end
		end
		if next(regions) then
			data["cheat"][count]["region"] = {} 
			for name, region in pairs(regions) do
				data["cheat"][count]["region"] = {} 
				data["cheat"][count]["region"][name] = region
			end
		end
		if output then
			data["cheat"][count]["screen"] = {}
			data["cheat"][count]["screen"]["screen"] = ":screen"
		end
	end
	return data["cheat"]
end

return xml
