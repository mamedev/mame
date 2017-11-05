-- license:BSD-3-Clause
-- copyright-holders: Carl
local exports = {}
exports.name = "gdbstub"
exports.version = "0.0.1"
exports.description = "GDB stub plugin"
exports.license = "The BSD 3-Clause License"
exports.author = { name = "Carl" }

local gdbstub = exports

-- percpu mapping of mame registers to gdb register order
local regmaps = {
	i386 = {
		togdb = {
			EAX = 1, ECX = 2, EDX = 3, EBX = 4, ESP = 5, EBP = 6, ESI = 7, EDI = 8, EIP = 9, EFLAGS = 10, CS = 11, SS = 12,
			DS = 13, ES = 14, FS = 15, GS = 16 },
		fromgdb = {
			"EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI", "EIP", "EFLAGS", "CS", "SS", "DS", "ES", "FS", "GS" },
		regsize = 4,
		addrsize = 4,
		pcreg = "EIP"
	}
}
regmaps.i486 = regmaps.i386
regmaps.pentium = regmaps.i386

function gdbstub.startplugin()
	local debugger
	local debug
	local cpu
	local breaks
	local watches
	local consolelog
	local consolelast
	local running

	emu.register_start(function ()
		debugger = manager:machine():debugger()
		if not debugger then
			print("gdbstub: debugger not enabled")
			return
		end
		cpu = manager:machine().devices[":maincpu"]
		if not cpu then
			print("gdbstub: maincpu not found")
		end
		if not regmaps[cpu:shortname()] then
			print("gdbstub: no register map for cpu " .. cpu:shortname())
			cpu = nil
		end
		consolelog = debugger.consolelog
		consolelast = 0
		breaks = {byaddr = {}, byidx = {}}
		watches = {byaddr = {}, byidx = {}}
		running = false
	end)

	emu.register_stop(function()
		consolelog = nil
		cpu = nil
		debug = nil
	end)

	local socket = emu.file("", 7)
	local connected = false
	socket:open("socket.127.0.0.1:2159")

	emu.register_periodic(function ()
		if not cpu then
			return
		end

		if running and debugger.execution_state == "stop" then
			socket:write("$S05#B8")
			running = false
			return
		elseif debugger.execution_state == "run" then
			running = true
		end

		local function chksum(str)
			local sum = 0
			str:gsub(".", function(s) sum = sum + s:byte() end)
			return string.format("%.2x", sum & 0xff)
		end

		local function makebestr(val, len)
			local str = ""
			for count = 0, len - 1 do
				str = str .. string.format("%.2x", (val >> (count * 8)) & 0xff)
			end
			return str
		end

		local last = consolelast
		local msg = consolelog[#consolelog]
		consolelast = #consolelog
		if #consolelog > last and msg:find("Stopped at", 1, true) then
			local point = tonumber(msg:match("Stopped at breakpoint ([0-9]+)"))
			local map = regmaps[cpu:shortname()]
			running = false
			if not point then
				point = tonumber(msg:match("Stopped at watchpoint ([0-9]+"))
				if not point then
					return -- ??
				end
				local wp = watches.byidx[point]
				if wp then
					local reply = "T05" .. wp.type .. ":" .. makebestr(wp.addr, map.addrsize)
					socket:write("$" .. reply .. "#" .. chksum(reply))
				else
					socket:write("$S05#B8")
				end
				return
			else
				local bp = breaks.byidx[point]
				if bp then
					local reply = "T05hwbreak:" .. makebestr(cpu.state[map.pcreg].value, map.regsize)
					socket:write("$" .. reply .. "#" .. chksum(reply))
				else
					socket:write("$S05#B8")
				end
				return
			end
		end

		if running and debugger.execution_state == "stop" then
			socket:write("$S05#B8")
			running = false
			return
		elseif debugger.execution_state == "run" then
			running = true
		end

		local data = ""

		repeat
			local read = socket:read(100)
			data = data .. read
		until #read == 0
		if #data == 0 then
			return
		end
		if data == "\x03" then
			debugger.execution_state = "stop"
			socket:write("$S05#B8")
			running = false
			return
		end
		local packet, checksum = data:match("%$([^#]+)#(%x%x)")
		if packet then
			packet:gsub("}(.)", function(s) return string.char(string.byte(s) ~ 0x20) end)
			local cmd = packet:sub(1, 1)
			local map = regmaps[cpu:shortname()]
			if cmd == "g" then
				local regs = {}
				for reg, idx in pairs(map.togdb) do
					regs[idx] = makebestr(cpu.state[reg].value, map.regsize)
				end
				local data = table.concat(regs)
				socket:write("+$" .. data .. "#" .. chksum(data))
			elseif cmd == "G" then
				local count = 0
				packet:sub(2):gsub(string.rep("%x", map.regsize * 2), function(s)
						count = count + 1
						cpu.state[map.fromgdb[count]].value = tonumber(s,16)
					end)
				socket:write("+$OK#9a")
			elseif cmd == "m" then
				local addr, len = packet:match("m(%x+),(%x+)")
				if addr and len then
					addr = tonumber(addr, 16)
					len = tonumber(len, 16)
					local data = ""
					local space = cpu.spaces["program"]
					for count = 1, len do
						data = data .. string.format("%.2x", space:read_log_u8(addr))
						addr = addr + 1
					end
					socket:write("+$" .. data .. "#" .. chksum(data))
				else
					socket:write("+$E00#a5") -- fix error
				end
			elseif cmd == "M" then
				local count = 0
				local addr, len, data = packet:match("M(%x+),(%x+),(%x+)")
				if addr and len and data then
					addr = tonumber(addr, 16)
					local space = cpu.spaces["program"]
					data:gsub("%x%x", function(s) space:write_log_u8(addr + count, tonumber(s, 16)) count = count + 1 end)
					socket:write("+$OK#9a")
				else
					socket:write("+$E00#a5")
				end
			elseif cmd == "s" then
				if #packet == 1 then
					cpu:debug():step()
					socket:write("+$OK#9a")
					socket:write("$S05#B8")
					running = false
				else
					socket:write("+$E00#a5")
				end
			elseif cmd == "c" then
				if #packet == 1 then
					cpu:debug():go()
					socket:write("+$OK#9a")
				else
					socket:write("+$E00#a5")
				end
			elseif cmd == "Z" then
				local btype, addr, kind = packet:match("Z([0-4]),(%x+),(.*)")
				addr = tonumber(addr, 16)
				if btype == "0" then
					socket:write("") -- is machine dependant
				elseif btype == "1" then
					if breaks.byaddr[addr] then
						socket:write("+$E00#a5")
						return
					end
					local idx = cpu:debug():bpset(addr)
					breaks.byaddr[addr] = idx
					breaks.byidx[idx] = addr
					socket:write("+$OK#9a")
				elseif btype == "2" then
					if watches.byaddr[addr] then
						socket:write("+$E00#a5")
						return
					end
					local idx = cpu:debug():wpset(cpu.spaces["program"], "w", addr, 1)
					watches.byaddr[addr] = idx
					watches.byidx[idx] = {addr = addr, type = "watch"}
					socket:write("+$OK#9a")
				elseif btype == "3" then
					if watches.byaddr[addr] then
						socket:write("+$E00#a5")
						return
					end
					local idx = cpu:debug():wpset(cpu.spaces["program"], "r", addr, 1)
					watches.byaddr[addr] = idx
					watches.byidx[idx] = {addr = addr, type = "rwatch"}
					socket:write("+$OK#9a")
				elseif btype == "4" then
					if watches.byaddr[addr] then
						socket:write("+$E00#a5")
						return
					end
					local idx = cpu:debug():wpset(cpu.spaces["program"], "rw", addr, 1)
					watches.byaddr[addr] = idx
					watches.byidx[idx] = {addr = addr, type = "awatch"}
					socket:write("+$OK#9a")
				end
			elseif cmd == "z" then
				local btype, addr, kind = packet:match("z([0-4]),(%x+),(.*)")
				addr = tonumber(addr, 16)
				if btype == "0" then
					socket:write("") -- is machine dependent
				elseif btype == "1" then
					if not breaks.byaddr[addr] then
						socket:write("+$E00#a5")
						return
					end
					local idx = breaks.byaddr[addr]
					cpu:debug():bpclr(idx)
					breaks.byaddr[addr] = nil
					breaks.byidx[idx] = nil
					socket:write("+$OK#9a")
				elseif btype == "2" or btype == "3" or btype == "4" then
					if not watches.byaddr[addr] then
						socket:write("+$E00#a5")
						return
					end
					local idx = watches.byaddr[addr]
					cpu:debug():wpclr(idx)
					watches.byaddr[addr] = nil
					watches.byidx[idx] = nil
					socket:write("+$OK#9a")
				end
			elseif cmd == "?" then
				socket:write("+$S05#B8")
			else
				socket:write("+$#00")
			end
		end
	end)
end

return exports
