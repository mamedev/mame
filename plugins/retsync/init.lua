-- license:BSD-3-Clause
-- copyright-holders: Carl (based on gdbstub), hjanetzek
--
-- What works?
-- In Ghidra: While stopped in debugger address/cursor location is synced with MAME
-- F5  go
-- F2  toggle breakpoint
-- F10,F11 step instruction

-- This need to match the name of the imported ROM in Ghidra:
--local PROGRAM_NAME = 'cpu_a1_c1.bin'
--local PROGRAM_NAME = 'ustudio_dump.bin'
--local PROGRAM_NAME = 'ustudio2.bin'

local PROGRAM_NAME = '5322-02-4481186.bin'

local socket = require("socket")

local exports = {
	name = "retsync",
	version = "0.0.1",
	description = "ret-sync plugin",
	license = "BSD-3-Clause",
	author = { name = "Carl, hjanetzek" } }

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
	},
	i8085a = {
		togdb = {
			AF = 1, BC = 2, DE = 3, HL = 4, PC = 5 },
		fromgdb = {
			"AF", "BC", "DE", "HL", "PC" },
		regsize = 2,
		addrsize = 2,
		pcreg = "PC"
	},
	i80186 = {
		togdb = {
			IP = 1, AX = 2, CX = 3, DX = 4, SP = 5, BP = 6, SI = 7, DI = 8, ES = 9, CS = 10, SS = 11, DS = 12 }, -- V?
		fromgdb = {
			"IP", "AX", "CX", "DX", "SP", "BP", "SI", "DI", "ES", "CS", "SS", "DS" },
		regsize = 2,
		addrsize = 2,
		pcreg = "PC"
	},
}
regmaps.i486 = regmaps.i386
regmaps.pentium = regmaps.i386

local reset_subscription, stop_subscription

-- Translated by GPT4 from ret-sync ext_gdb/sync.py to Lua
-- In case MAME takes over the world and wants us to play a game - This is the place where it probably started...
local Tunnel = {}
Tunnel.__index = Tunnel

function Tunnel.new(host, port)
	local self = setmetatable({}, Tunnel)
	print("initializing retsync tunnel using " .. host .. ":" .. port .. "...")
	self.sock = nil
	self.sync = false

	self.sock = socket.tcp()
	self.sock:settimeout(4)

	local success, err = self.sock:connect(host, port)
	if not success then
		self.sock:close()
		self.sock = nil
		print("tunnel initialization error: " .. err)
		return nil
	end

	self.sync = true

	return self
end

function Tunnel:is_up()
	return (self.sock ~= nil and self.sync == true)
end

function Tunnel:poll()
	if not self:is_up() then
		return nil
	end

	self.sock:settimeout(0)

	local msg, err, partial = self.sock:receive()
	if not msg and err == "timeout" then
		return ''
	elseif not msg then
		self:close()
		return nil
	end

	self.sock:settimeout(4)
	return msg
end

function Tunnel:send(msg)
	if not self.sock then
		print("tunnel_send: tunnel is unavailable (did you forget to sync ?)")
		return
	end

	local success, err = self.sock:send(msg)
	if not success then
		print(err)
		self.sync = false
		self:close()
		print("tunnel_send error: " .. err)
	end
end

function Tunnel:close()
	if self:is_up() then
		self:send("[notice]{\"type\":\"dbg_quit\",\"msg\":\"dbg disconnected\"}\n")
	end

	if self.sock then
		self.sock:close()
		self.sock = nil
	end

	self.sync = false
end

function gdbstub.startplugin()
	local debugger
	local debug
	local cpu
	local breaks
	local watches
	local consolelog
	local consolelast
	local running
	local prev_pc

	reset_subscription = emu.add_machine_reset_notifier(function ()
		debugger = manager.machine.debugger
		if not debugger then
			print("retsync: debugger not enabled")
			return
		end
		cpu = manager.machine.devices[":maincpu"]
		if not cpu then
			print("retsync: maincpu not found")
		end
		if not regmaps[cpu.shortname] then
			print("retsync: no register map for cpu " .. cpu.shortname)
			cpu = nil
		end
		consolelog = debugger.consolelog
		consolelast = 0
		breaks = {byaddr = {}, byidx = {}}
		watches = {byaddr = {}, byidx = {}}
		running = false
	end)

	stop_subscription = emu.add_machine_stop_notifier(function ()
		consolelog = nil
		cpu = nil
		debug = nil
	end)

	local socket = Tunnel.new("localhost", 9100)
    if not socket or not socket:is_up() then
       return
    end
	local id = "MAME"
	socket:send("[notice]{\"type\":\"new_dbg\",\"msg\":\"dbg connect - " .. id .. "\",\"dialect\":\"gdb\"}\n")

	local arg = 'on'
	socket:send("[notice]{\"type\":\"sync_mode\",\"auto\":\"" .. arg .. "\"}\n")
	local sync = true

	local modules = {}
	socket:send("[notice]{\"type\":\"module\",\"path\":\"" .. PROGRAM_NAME .. "\",\"modules\":[" .. table.concat(modules, ",") .. "]}\n")

	--debugger:command("log retsync started")
	emu.register_periodic(function ()
		if not cpu then
			return
		end

		if running and debugger.execution_state == "stop" then
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
			local map = regmaps[cpu.shortname]
			running = false
			if not point then
  			    point = tonumber(msg:match("Stopped at watchpoint ([0-9]+)"))
				if not point then
					return -- ??
				end
				local wp = watches.byidx[point]
				if wp then
					print("retsync stop wp TODO")
				end
				return
			else
				-- local bp = breaks.byidx[point]
				-- if bp then
					print("retsync stop bp")
					local base = 0
					local offset = cpu.state[map.pcreg].value
					socket:send("[sync]{\"type\":\"loc\",\"base\":" .. base .. ",\"offset\":" .. offset .. "}\n")
				-- else
				--    print("retsync stop no bp")
				-- end
				return
			end
			print("retsync stop errp")
		end


		if sync and not running then
		   -- update on next instruction step
		   local map = regmaps[cpu.shortname]
		   local pc = cpu.state[map.pcreg].value
		   if pc ~= prev_pc then
			  prev_pc = pc
			  local base = 0
			  socket:send("[sync]{\"type\":\"loc\",\"base\":" .. base .. ",\"offset\":" .. pc .. "}\n")
			  return
		   end
		end

		local data = ""
		repeat
		   local read = socket:poll()
		   if not read then
			  return
			end
			data = data .. read
		until #read == 0
		if #data == 0 then
			return
		end
		print(">>> '" .. data .. "'")
		local cmd, address = data:match("(%a) %* 0x(%x+)")
		if cmd then
		   print("cmd:" .. cmd .. " addr:" .. address)
		   if cmd == "b" then
			  addr = tonumber(address, 16)
			  print("cmd:" .. cmd .. " addr:" .. addr)
			  if breaks.byaddr[addr] then
				 print("clear breakpoint")
				 local idx = breaks.byaddr[addr]
				 debugger:command("printf 'clear breakpoint " .. idx .. " @0x" .. address .. "'")
				 cpu.debug:bpclear(idx)
				 breaks.byaddr[addr] = nil
				 breaks.byidx[idx] = nil
				 return
			  end
			  print("set breakpoint")
			  local idx = cpu.debug:bpset(addr,'','')
			  debugger:command("printf 'set breakpoint " .. idx .. " @0x" .. address .. "'")
			  breaks.byaddr[addr] = idx
			  breaks.byidx[idx] = addr
		   end
		   return
		end
		if data == 'ni' or data == 'si' then
		   cpu.debug:step()
		   local base = 0
		   local map = regmaps[cpu.shortname]
		   local offset = cpu.state[map.pcreg].value
		   socket:send("[sync]{\"type\":\"loc\",\"base\":" .. base .. ",\"offset\":" .. offset .. "}\n")
		   return
		end
		if data == 'continue' then
		   cpu.debug:go(0) -- TODO get the right cpu?
		   return
		end
	end)
end

return exports
