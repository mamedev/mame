-- to use this get the package from http://greatstone.free.fr/hi2txt/
-- extract the hi2txt.zip and place it in your history path

local dat = {}
local env = {}
local output
local curset
local path = emu.subst_env(mame_manager.ui.options.entries.historypath:value():gsub("([^;]+)", "%1/hi2txt")) 
local htmlentites = {
	["amp"] = "&",
	["quot"] = utf8.char(34),
	["big-mid-dot"] = utf8.char(149),
	["copyright"] = utf8.char(169),
	["mid-dot"] = utf8.char(183),
	["one-on-two"] = utf8.char(189),
	["ring"] = utf8.char(214),
	["acute"] = utf8.char(225),
	["y-strike"] = utf8.char(590),
	["bridge1"] = utf8.char(761),
	["bridge2"] = utf8.char(765),
	["bridge3"] = utf8.char(766),
	["alpha"] = utf8.char(945),
	["beta"] = utf8.char(946),
	["gamma"] = utf8.char(947),
	["delta"] = utf8.char(948),
	["epsilon"] = utf8.char(949),
	["zeta"] = utf8.char(950),
	["eta"] = utf8.char(951),
	["theta"] = utf8.char(952),
	["iota"] = utf8.char(953),
	["kappa"] = utf8.char(954),
	["lambda"] = utf8.char(955),
	["mu"] = utf8.char(956),
	["nu"] = utf8.char(957),
	["xi"] = utf8.char(958),
	["omicron"] = utf8.char(959),
	["pi"] = utf8.char(960),
	["rho"] = utf8.char(961),
	["sigmaf"] = utf8.char(962),
	["sigma"] = utf8.char(963),
	["tau"] = utf8.char(964),
	["upsilon"] = utf8.char(965),
	["phi"] = utf8.char(966),
	["chi"] = utf8.char(967),
	["psi"] = utf8.char(968),
	["omega"] = utf8.char(969),
	["circle-line"] = utf8.char(984),
	["two-dots"] = utf8.char(1417),
	["inverted-question"] = utf8.char(1567),
	["rdquo"] = utf8.char(8221),
	["big-dot"] = utf8.char(8226),
	["three-dots"] = utf8.char(8230),
	["two-exclamations"] = utf8.char(8252),
	["broken-question"] = utf8.char(8253),
	["asterism"] = utf8.char(8258),
	["w-double-strike"] = utf8.char(8361),
	["square-2"] = utf8.char(8414),
	["roman-numeral-1"] = utf8.char(8544),
	["roman-numeral-2"] = utf8.char(8545),
	["roman-numeral-3"] = utf8.char(8546),
	["roman-numeral-4"] = utf8.char(8547),
	["roman-numeral-5"] = utf8.char(8548),
	["roman-numeral-6"] = utf8.char(8549),
	["roman-numeral-7"] = utf8.char(8550),
	["roman-numeral-8"] = utf8.char(8551),
	["roman-numeral-9"] = utf8.char(8552),
	["roman-numeral-10"] = utf8.char(8553),
	["roman-numeral-11"] = utf8.char(8554),
	["roman-numeral-12"] = utf8.char(8555),
	["small-roman-numeral-1"] = utf8.char(8560),
	["small-roman-numeral-2"] = utf8.char(8561),
	["small-roman-numeral-3"] = utf8.char(8562),
	["small-roman-numeral-4"] = utf8.char(8563),
	["small-roman-numeral-5"] = utf8.char(8564),
	["small-roman-numeral-6"] = utf8.char(8565),
	["small-roman-numeral-7"] = utf8.char(8566),
	["small-roman-numeral-8"] = utf8.char(8567),
	["small-roman-numeral-9"] = utf8.char(8568),
	["small-roman-numeral-10"] = utf8.char(8569),
	["small-roman-numeral-11"] = utf8.char(8570),
	["small-roman-numeral-12"] = utf8.char(8571),
	["left-arrow"] = utf8.char(8592),
	["right-double-arrow"] = utf8.char(8658),
	["four-lines"] = utf8.char(8803),
	["three-mid-dots"] = utf8.char(8943),
	["left-foot"] = utf8.char(8968),
	["right-foot"] = utf8.char(8968),
	["round-7"] = utf8.char(9318),
	["square"] = utf8.char(9633),
	["dot-in-square"] = utf8.char(9635),
	["h-lines-in-square"] = utf8.char(9636),
	["v-lines-in-square"] = utf8.char(9637),
	["slash-in-square"] = utf8.char(9639),
	["antislash-in-square"] = utf8.char(9640),
	["black-triangle-right"] = utf8.char(9658),
	["black-triangle-down"] = utf8.char(9660),
	["two-cubes"] = utf8.char(9707) ,
	["umbrella"] = utf8.char(9730),
	["snowman"] = utf8.char(9731) ,
	["black-star"] = utf8.char(9733),
	["star"] = utf8.char(9734),
	["headset"] = utf8.char(9738),
	["phone"] = utf8.char(9742),
	["hot-beverage"] = utf8.char(9749),
	["skull"] = utf8.char(9760),
	["ankh"] = utf8.char(9765),
	["cross-of-lorraine"] = utf8.char(9768),
	["cross-of-jerusalem"] = utf8.char(9769),
	["peace"] = utf8.char(9774),
	["angry-face"] = utf8.char(9785),
	["smiley"] = utf8.char(9786),
	["black-smiley"] = utf8.char(9787),
	["sun"] = utf8.char(9788),
	["moon"] = utf8.char(9789),
	["crescent-moon"] = utf8.char(9790),
	["woman"] = utf8.char(9792),
	["man"] = utf8.char(9794),
	["spaceship"] = utf8.char(9798),
	["aries"] = utf8.char(9800),
	["taurus"] = utf8.char(9801),
	["gemini"] = utf8.char(9802),
	["cancer"] = utf8.char(9803),
	["leo"] = utf8.char(9804),
	["virgo"] = utf8.char(9805),
	["libra"] = utf8.char(9806),
	["scorpio"] = utf8.char(9807),
	["sagitarius"] = utf8.char(9808),
	["capricorn"] = utf8.char(9809),
	["aquarius"] = utf8.char(9810),
	["pisces"] = utf8.char(9811),
	["amber"] = utf8.char(9816) ,
	["black-spade"] = utf8.char(9824) ,
	["heart"] = utf8.char(9825) ,
	["black-club"] = utf8.char(9827) ,
	["black-heart"] = utf8.char(9829) ,
	["black-diamond"] = utf8.char(9830) ,
	["single-music-note"] = utf8.char(9834),
	["double-music-note"] = utf8.char(9835),
	["crossed-swords"] = utf8.char(9876),
	["baseball"] = utf8.char(9918),
	["boat"] = utf8.char(9973),
	["scissors"] = utf8.char(9988),
	["airplane"] = utf8.char(9992),
	["multiplication"] = utf8.char(10005),
	["big-exclamation"] = utf8.char(10082),
	["left-black-heart"] = utf8.char(10085),
	["black-right-arrow-large"] = utf8.char(10152),
	["up-arrow-with-stroke"] = utf8.char(10505),
	["jp-h-a-small"] = utf8.char(12353),
	["jp-h-a"] = utf8.char(12354),
	["jp-h-i-small"] = utf8.char(12355),
	["jp-h-i"] = utf8.char(12356),
	["jp-h-u-small"] = utf8.char(12357),
	["jp-h-u"] = utf8.char(12358),
	["jp-h-e-small"] = utf8.char(12359),
	["jp-h-e"] = utf8.char(12360),
	["jp-h-o-small"] = utf8.char(12361),
	["jp-h-o"] = utf8.char(12362),
	["jp-h-ka"] = utf8.char(12363),
	["jp-h-ga"] = utf8.char(12364),
	["jp-h-ki"] = utf8.char(12365),
	["jp-h-gi"] = utf8.char(12366),
	["jp-h-ku"] = utf8.char(12367),
	["jp-h-gu"] = utf8.char(12368),
	["jp-h-ke"] = utf8.char(12369),
	["jp-h-ge"] = utf8.char(12370),
	["jp-h-ko"] = utf8.char(12371),
	["jp-h-go"] = utf8.char(12372),
	["jp-h-sa"] = utf8.char(12373),
	["jp-h-za"] = utf8.char(12374),
	["jp-h-si"] = utf8.char(12375),
	["jp-h-zi"] = utf8.char(12376),
	["jp-h-su"] = utf8.char(12377),
	["jp-h-zu"] = utf8.char(12378),
	["jp-h-se"] = utf8.char(12379),
	["jp-h-ze"] = utf8.char(12380),
	["jp-h-so"] = utf8.char(12381),
	["jp-h-zo"] = utf8.char(12382),
	["jp-h-ta"] = utf8.char(12383),
	["jp-h-da"] = utf8.char(12384),
	["jp-h-ti"] = utf8.char(12385),
	["jp-h-di"] = utf8.char(12386),
	["jp-h-tu-small"] = utf8.char(12387),
	["jp-h-sokuon"] = utf8.char(12387),
	["jp-h-tu"] = utf8.char(12388),
	["jp-h-du"] = utf8.char(12389),
	["jp-h-te"] = utf8.char(12390),
	["jp-h-de"] = utf8.char(12391),
	["jp-h-to"] = utf8.char(12392),
	["jp-h-do"] = utf8.char(12393),
	["jp-h-na"] = utf8.char(12394),
	["jp-h-ni"] = utf8.char(12395),
	["jp-h-nu"] = utf8.char(12396),
	["jp-h-ne"] = utf8.char(12397),
	["jp-h-no"] = utf8.char(12398),
	["jp-h-ha"] = utf8.char(12399),
	["jp-h-ba"] = utf8.char(12400),
	["jp-h-pa"] = utf8.char(12401),
	["jp-h-hi"] = utf8.char(12402),
	["jp-h-bi"] = utf8.char(12403),
	["jp-h-pi"] = utf8.char(12404),
	["jp-h-hu"] = utf8.char(12405),
	["jp-h-bu"] = utf8.char(12406),
	["jp-h-pu"] = utf8.char(12407),
	["jp-h-he"] = utf8.char(12408),
	["jp-h-be"] = utf8.char(12409),
	["jp-h-pe"] = utf8.char(12410),
	["jp-h-ho"] = utf8.char(12411),
	["jp-h-bo"] = utf8.char(12412),
	["jp-h-po"] = utf8.char(12413),
	["jp-h-ma"] = utf8.char(12414),
	["jp-h-mi"] = utf8.char(12415),
	["jp-h-mu"] = utf8.char(12416),
	["jp-h-me"] = utf8.char(12417),
	["jp-h-mo"] = utf8.char(12418),
	["jp-h-ya-small"] = utf8.char(12419),
	["jp-h-youon-a"] = utf8.char(12419),
	["jp-h-ya"] = utf8.char(12420),
	["jp-h-yu-small"] = utf8.char(12421),
	["jp-h-youon-u"] = utf8.char(12421),
	["jp-h-yu"] = utf8.char(12422),
	["jp-h-yo-small"] = utf8.char(12423),
	["jp-h-youon-o"] = utf8.char(12423),
	["jp-h-yo"] = utf8.char(12424),
	["jp-h-ra"] = utf8.char(12425),
	["jp-h-ri"] = utf8.char(12426),
	["jp-h-ru"] = utf8.char(12427),
	["jp-h-re"] = utf8.char(12428),
	["jp-h-ro"] = utf8.char(12429),
	["jp-h-wa-small"] = utf8.char(12430),
	["jp-h-wa"] = utf8.char(12431),
	["jp-h-wi"] = utf8.char(12432),
	["jp-h-we"] = utf8.char(12433),
	["jp-h-wo"] = utf8.char(12434),
	["jp-h-n"] = utf8.char(12435),
	["jp-h-vu"] = utf8.char(12436),
	["jp-h-ka-small"] = utf8.char(12437),
	["jp-h-ke-small"] = utf8.char(12438),
	["jp-h-dakuten"] = utf8.char(12443),
	["jp-h-handakuten"] = utf8.char(12444),
	["jp-k-a-small"] = utf8.char(12449),
	["jp-k-a"] = utf8.char(12450),
	["jp-k-i-small"] = utf8.char(12451),
	["jp-k-i"] = utf8.char(12452),
	["jp-k-u-small"] = utf8.char(12453),
	["jp-k-u"] = utf8.char(12454),
	["jp-k-e-small"] = utf8.char(12455),
	["jp-k-e"] = utf8.char(12456),
	["jp-k-o-small"] = utf8.char(12457),
	["jp-k-o"] = utf8.char(12458),
	["jp-k-ka"] = utf8.char(12459),
	["jp-k-ga"] = utf8.char(12460),
	["jp-k-ki"] = utf8.char(12461),
	["jp-k-gi"] = utf8.char(12462),
	["jp-k-ku"] = utf8.char(12463),
	["jp-k-gu"] = utf8.char(12464),
	["jp-k-ke"] = utf8.char(12465),
	["jp-k-ge"] = utf8.char(12466),
	["jp-k-ko"] = utf8.char(12467),
	["jp-k-go"] = utf8.char(12468),
	["jp-k-sa"] = utf8.char(12469),
	["jp-k-za"] = utf8.char(12470),
	["jp-k-si"] = utf8.char(12471),
	["jp-k-zi"] = utf8.char(12472),
	["jp-k-su"] = utf8.char(12473),
	["jp-k-zu"] = utf8.char(12474),
	["jp-k-se"] = utf8.char(12475),
	["jp-k-ze"] = utf8.char(12476),
	["jp-k-so"] = utf8.char(12477),
	["jp-k-zo"] = utf8.char(12478),
	["jp-k-ta"] = utf8.char(12479),
	["jp-k-da"] = utf8.char(12480),
	["jp-k-ti"] = utf8.char(12481),
	["jp-k-di"] = utf8.char(12482),
	["jp-k-tu-small"] = utf8.char(12483),
	["jp-k-tu"] = utf8.char(12484),
	["jp-k-du"] = utf8.char(12485),
	["jp-k-te"] = utf8.char(12486),
	["jp-k-de"] = utf8.char(12487),
	["jp-k-to"] = utf8.char(12488),
	["jp-k-do"] = utf8.char(12489),
	["jp-k-na"] = utf8.char(12490),
	["jp-k-ni"] = utf8.char(12491),
	["jp-k-nu"] = utf8.char(12492),
	["jp-k-ne"] = utf8.char(12493),
	["jp-k-no"] = utf8.char(12494),
	["jp-k-ha"] = utf8.char(12495),
	["jp-k-ba"] = utf8.char(12496),
	["jp-k-pa"] = utf8.char(12497),
	["jp-k-hi"] = utf8.char(12498),
	["jp-k-bi"] = utf8.char(12499),
	["jp-k-pi"] = utf8.char(12500),
	["jp-k-hu"] = utf8.char(12501),
	["jp-k-bu"] = utf8.char(12502),
	["jp-k-pu"] = utf8.char(12503),
	["jp-k-he"] = utf8.char(12504),
	["jp-k-be"] = utf8.char(12505),
	["jp-k-pe"] = utf8.char(12506),
	["jp-k-ho"] = utf8.char(12507),
	["jp-k-bo"] = utf8.char(12508),
	["jp-k-po"] = utf8.char(12509),
	["jp-k-ma"] = utf8.char(12510),
	["jp-k-mi"] = utf8.char(12511),
	["jp-k-mu"] = utf8.char(12512),
	["jp-k-me"] = utf8.char(12513),
	["jp-k-mo"] = utf8.char(12514),
	["jp-k-ya-small"] = utf8.char(12515),
	["jp-k-ya"] = utf8.char(12516),
	["jp-k-yu-small"] = utf8.char(12517),
	["jp-k-yu"] = utf8.char(12518),
	["jp-k-yo-small"] = utf8.char(12519),
	["jp-k-yo"] = utf8.char(12520),
	["jp-k-ra"] = utf8.char(12521),
	["jp-k-ri"] = utf8.char(12522),
	["jp-k-ru"] = utf8.char(12523),
	["jp-k-re"] = utf8.char(12524),
	["jp-k-ro"] = utf8.char(12525),
	["jp-k-wa-small"] = utf8.char(12526),
	["jp-k-wa"] = utf8.char(12527),
	["jp-k-wi"] = utf8.char(12528),
	["jp-k-we"] = utf8.char(12529),
	["jp-k-wo"] = utf8.char(12530),
	["jp-k-n"] = utf8.char(12531),
	["jp-k-vu"] = utf8.char(12532),
	["jp-k-ka-small"] = utf8.char(12533),
	["jp-k-ke-small"] = utf8.char(12534),
	["jp-k-va"] = utf8.char(12535),
	["jp-k-vi"] = utf8.char(12536),
	["jp-k-ve"] = utf8.char(12537),
	["jp-k-vo"] = utf8.char(12538),
	["jp-k-zero"] = utf8.char(38646),
	["jp-k-one"] = utf8.char(19968),
	["jp-k-two"] = utf8.char(20108),
	["jp-k-three"] = utf8.char(19977),
	["jp-k-four"] = utf8.char(22235),
	["jp-k-five"] = utf8.char(20116),
	["jp-k-six"] = utf8.char(20845),
	["jp-k-seven"] = utf8.char(19971),
	["jp-k-height"] = utf8.char(20843),
	["jp-k-nine"] = utf8.char(20061),
	["lama"] =  "lama",
	["cat-face"] = utf8.char(9786) ,
	["whale"] = "whale",
	["thumbs-up"] = utf8.char(8730) ,
	["shoe"] = "shoe",
	["kiss"] = utf8.char(9786),
	["heart-with-arrow"] = utf8.char(9829),
	["car-side"] = utf8.char(9936),
	["car-front"] = utf8.char(9936),
	["mens-symbol"] = utf8.char(9794),
	["womens-symbol"] = utf8.char(9792),
	["feet"] = utf8.char(128062)
}

function env.open(file, size, swap)
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
	if swap then
		if swap == 2 then
			content = content:gsub("(.)(.)", function(c1, c2) return c2 .. c1 end)
		elseif swap == 4 then
			content = content:gsub("(.)(.)(.)(.)", function(c1, c2, c3, c4) return c4 .. c3 .. c2 .. c1 end)
		else
			emu.print_verbose("swap " .. swap .. " not supported")
		end
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

function env.bitmask(bytes, masks)
	local newbytes = {}
	bytes = table.concat(bytes)
	bytes = table.pack(string.unpack(string.rep("c1", #bytes), bytes))
	bytes[#bytes] = nil
	for num, mask in ipairs(masks) do
		newbytes[#newbytes + 1] = ""
		for num2, bytemask in ipairs(mask.mask) do
			local val = bytes[num2]:byte() & bytemask
			if val ~= 0 then
				newbytes[#newbytes] = newbytes[#newbytes] .. string.char(val)
			end
		end
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
	if base == 32 or base == 40 then
		local newbytes = {}
		for num, char in ipairs(bytes) do
			local nchar = (char:byte(1) << 8) + char:byte(2)
			local pos = #newbytes
			for i = 1, 3 do
				table.insert(newbytes, pos + 1, string.char(nchar % base))
				nchar = nchar // base
			end
		end
		return newbytes
	end
	emu.print_verbose("data_hiscore: basechar " .. base .. " unimplemented\n")
	return bytes
end

function env.charset_conv(bytes, charset, aoffset)
	if not aoffset then aoffset = 0 end
	if type(charset) == "string" then
		local chartype, offset, delta = charset:match("CS_(%w*)%[?(%-?%d?%d?),?(%d?%d?)%]?")
		if offset then offset = tonumber(offset) else offset = 0 end
		if delta then delta = tonumber(delta) else delta = 1 end
		if chartype == "NUMBER" then
			for num, char in ipairs(bytes) do
				char = char:byte() - aoffset - offset
				if char >= 48 and char <= 57 then
					bytes[num] = string.char(char)
				end
			end
			return bytes
		end
		emu.print_verbose("data_hiscore: charset " .. chartype .. " unimplemented\n")
		return bytes
	end
	for num, char in ipairs(bytes) do
		char = char:byte() - aoffset
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

function env.index_from_value(col, index)
	for i = 0, #col - 1 do
		if col[i].val == index then
			return i
		end
	end
	return index
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
	local function xml_parse(file)
		local table
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
		local s = { "local data = open('" .. xml.structure[1].file .. "', size, " .. (xml.structure[1]['byte-swap'] or "nil") .. ")\nlocal offset = 1\nlocal arr = {}",
				"local elem, bytes, offset, value, index, output"}
		local fparam = {}
		if xml.bitmask then
			local bitmask = "local bitmasks = {"
			for id, masks in pairs(xml.bitmask) do
				bitmask = bitmask .. "['" .. id .. "'] = {"
				for num, mask in ipairs(masks.character) do
					local bytemasks = {}
					mask.mask:gsub("[01]*", function(s)
						local bytemask = 0
						for i = 1, 8 do
							bytemask = bytemask | (tonumber(s:sub(i, i)) << (8 - i))
						end
						bytemasks[#bytemasks + 1] = bytemask
					end)
					bitmask = bitmask .. "{ mask = {" .. table.concat(bytemasks, ", ") .. "}, complete = \"" .. (mask['byte-completion'] or "yes") .. "\"}"
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
					local subchar = char.dst:gsub("&([^;]*);", function(s)
						if htmlentites[s] then
							return htmlentites[s]
						elseif s[1] == "#" then
							return utf8.char(tonumber(s:sub(2)))
						else
							return s
						end
					end)
					if char.default and char.default == "yes" then
						default = subchar
					end
					charset = charset .. "[" .. src .. "]" .. " = '" .. subchar .. "',"
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
						oper = " val = val .. '" .. suffix .. "'"
					end
				elseif (first == "h" and rest == "exadecimal_string") or (first == "0" and rest == "x") then
					oper = " val = string.format('0x%x', val)"
				elseif (first == "L" and rest == "oopIndex") then
					oper = " val = index"
				end
				if not oper then
					oper = " val = format['" .. form .. "'](val, {"
					for num1, colpar in ipairs(fparam[form]) do
						if fparam[form].full then
							oper = oper .. "arr['" .. colpar .. "'],"
						else
							oper = oper .. "arr['" .. colpar .. "'][i].val or arr['" .. colpar .. "'][0].val,"
						end
					end
					oper = oper .. "})"
				end
				ret = ret .. oper
			end
			return ret .. " return val\nend"
		end

		if xml.format then
			local format = { "local format = {}" }
			for num, form in ipairs(xml.format) do
				local param = {}
				format[#format + 1] = "format['" .. form["id"] .. "'] = function(val, param)"
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
						if op.text then
							format[#format + 1] = "val = val / " .. op.text
						elseif op.field then
							if op.field[1].format then
								local fform = check_format(op.field[1].format)
								format[#format + 1] = fform .. " val = val / tempform(val)"
							end
						else
							format[#format + 1] = "val = 0"
							for num2, col in ipairs(op) do
								param[#param + 1] = col["id"]
								if col["format"] then
									local colform = check_format(col["format"])
									format[#format + 1] = colform .. " val = val / tempform(val)"
								else
									format[#format + 1] = "val = val / param[" .. #param .. "]"
								end
							end
						end
					elseif op.tag == "sum" then
						format[#format + 1] = "val = 0"
						param.full = true
						for num2, col in ipairs(op.column) do
							param[#param + 1] = col["id"]
							format[#format + 1] = "for i = 0, #param[" .. #param .. "] do val = val + param[" .. #param .. "][i].val end"
							if col["format"] then
								local colform = check_format(col["format"])
								format[#format + 1] = colform .. " val = val + tempform(val)"
							end
						end
					elseif op.tag == "concat" then
						format[#format + 1] = "val = ''"
						for num2, col in ipairs(op) do
							if col["tag"] == "txt" then
								format[#format + 1] = "val = val .. '" .. col["text"] .. "'"
							elseif col["format"] then
								if col["id"] then
									param[#param + 1] = col["id"]
									local n = #param
									format[#format + 1] = function() return " " .. check_format(col["format"]) .. " val = val .. tempform(param[" .. n .. "])" end
								else
									format[#format + 1] = function() return " " .. check_format(col["format"]) .. " val = val .. tempform(temp)" end
								end
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
							op["src"] = "'" .. op["src"]:gsub("'", "\\'") .. "'"
						end
						if not tonumber(op["dst"]) then
							op["dst"] = "'" .. op["dst"]:gsub("'", "\\'") .. "'"
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
						if form["apply-to"]  == "char" then
							format[#format + 1] = "if val:byte() " .. op["operator"] .. " " .. op["src"] .. " then"
						else
							format[#format + 1] = "if val " .. op["operator"] .. " " .. op["src"] .. " then"
						end
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
			end
			for num, line in ipairs(format) do
				if type(line) == "string" then
					s[#s + 1] = line
				elseif type(line) == "function" then
					s[#s + 1] = line()
				end
			end
		end
		local function parse_elem(elem, loopelem)
			local ret = 0
			if elem["tag"] == "loop" then
				if elem["skip-first-bytes"] then
					s[#s + 1] = "offset = offset + " .. elem["skip-first-bytes"]
				end
				s[#s + 1] = "for i = 0, " .. elem["count"] - 1 .. " do"
				for num, elt in ipairs(elem) do
					parse_elem(elt, elem)
				end
				s[#s + 1] = "end"
				if elem["skip-last-bytes"] then
					s[#s + 1] = "offset = offset + " .. elem["skip-last-bytes"]
				end
			elseif elem["tag"] == "elt" then
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
				local unitsize = 1
				if elem["src-unit-size"] then
					local size = elem["src-unit-size"] / 8
					if size ~= math.floor(size) then
						emu.print_verbose("src-unit-size " .. elem["src-unit-size"] .. " not suppoted)")
					else
						unitsize =  math.floor(size)
					end
				end
				s[#s + 1] = "if not arr['" .. elem["id"] .. "'] then arr['" .. elem["id"] .. "'] = {} end\nelem = {}"
				s[#s + 1] = "bytes = table.pack(string.unpack('" .. string.rep("c" .. unitsize, elem["size"] / unitsize) .. "', data, offset))"
				if loopelem then
					total_size = total_size + elem["size"] * loopelem["count"]
				else
					total_size = total_size + elem["size"]
				end
				s[#s + 1] = "offset = bytes[#bytes]\nbytes[#bytes] = nil"
				local bytedec = elem["swap-skip-order"] or "byte-swap;bit-swap;byte-skip;endianness;byte-trim;nibble-skip;bitmask"
				local bytedecl = {}
				bytedec:gsub("([^;]*)", function(c) bytedecl[#bytedecl + 1] = c end)
				for num, func in ipairs(bytedecl) do
					if elem[func] then
						fixfunc = func:gsub("-", "_")
						if func == "bitmask" then
							s[#s + 1] = "bytes = " .. fixfunc .. "(bytes, bitmasks['" .. elem[func] .. "'])"
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
					if elem["format"] then
						s[#s + 1] = check_format(elem["format"])
						s[#s + 1] = "value = tempform(value)"
					end
					s[#s + 1] = "elem.val = value"
				elseif elem["type"] == "text" then
					if elem["base"] then
						s[#s + 1] = "bytes = basechar(bytes, " .. elem["base"] .. ")"
					end
					if elem["ascii-step"] then
						s[#s + 1] = "bytes = ascii_step(bytes, " .. elem["ascii-step"] .. ")"
					end
					if elem["ascii-offset"] then
						s[#s + 1] = "bytes = ascii_offset(bytes, " .. elem["ascii-offset"] .. ")"
					end
					if elem["charset"] then
						local charsets = {}
						elem["charset"]:gsub("([^;]*)", function(s) charsets[#charsets + 1] = s return "" end)
						for num, charset in pairs(charsets) do
							if charset:match("^CS_") then
								s[#s + 1] = "bytes = charset_conv(bytes, \"" .. charset .. "\""
							elseif charset ~= "" then
								s[#s + 1] = "bytes = charset_conv(bytes, charset['" .. charset .. "']"
							end
							if elem["ascii-offset"] then
								s[#s] = s[#s] .. ", " .. elem["ascii-offset"]
							end
							s[#s] = s[#s] .. ")"
						end
					end
					s[#s + 1] = "elem.val = table.concat(bytes)"
					if elem["format"] then
						s[#s + 1] = check_format(elem["format"])
						s[#s + 1] = "elem.val = tempform(elem.val)"
					end
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
						ref, reftype = elem["table-index"]:match("([%w ]*):([%a_]*)")
					end
					if not elem["table-index"] or elem["table-index"] == "loop_index" then
						index = "i * " .. step .. " + " .. start
					elseif elem["table-index"] == "loop_reverse_index" then
						index = "(" .. total  .. "- i - 1) * " .. step .. " + " .. start
					elseif elem["table-index"] == "itself" then
						index = "value"
					elseif reftype then
						index = reftype .. "(arr['" .. ref .. "'], i)"
					end
				end
				if elem["table-index-format"] and index then
					s[#s + 1] = check_format(elem["table-index-format"])
					index = "tempform(" .. index .. ")"
				end
				if loopelem then
					if index then
						s[#s + 1] = "index = " .. index
					end
					s[#s + 1] = "arr['" .. elem["id"] .. "'][index] = elem"
				else
					s[#s + 1] = "arr['" .. elem["id"] .. "'] = elem"
				end
			end
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
					local cols = 0
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
							if not loopcnt and col["src"] ~= "index" and col["src"] ~= "unsorted_index" then
								table.insert(dat, 1, "for i = 0, #arr['" .. col["src"] .. "'] do")
								table.insert(dat, 2, "local line = ''")
								loopcnt = true
							end
							head[#head + 1] = "output = output .. '" .. col["id"] .. "\\t'"
							if col["src"] == "index" then
								dat[#dat + 1] = "value = i"
							elseif col["src"] == "unsorted_index" then
								dat[#dat + 1] = "value = i"
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
							cols = cols + 1
						end
					end
					if cols > 0 then
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
		end
		s[#s + 1] = "return output"

		-- cache script
		local script = table.concat(s, "\n")
		local scrpath = path:match("[^;]*") .. "/"
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

	local scrfile = emu.file(path, 1)
	local ret = scrfile:open(set .. ".lua")
	local script
	if ret then
		function get_xml_table(fileset)
			local file = emu.file(path, 1)
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
