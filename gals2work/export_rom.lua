-- Export logical big-endian ROM bytes; no dependency on the selected ROM bank.
local machine = manager.machine
for _, tag in ipairs({":subdata", ":sub"}) do
 local region = machine.memory.regions[tag]
 local path = "gals2work/" .. machine.system.name .. tag:gsub(":", "_") .. "_logical.bin"
 local out = assert(io.open(path, "wb"))
 local bytes = region:read(0, region.size)
 if region.bytewidth == 2 and region.endianness == "big" and string.pack("I2", 1):byte(1) == 1 then
  bytes = bytes:gsub("(..)", function(pair) return pair:reverse() end)
 end
 out:write(bytes)
 out:close()
 print("Exported " .. path .. " (" .. region.size .. " bytes)")
end
local space = machine.devices[":sub"].spaces.program
print(string.format("CPU bytes at 8eac7e: %02x %02x %02x %02x", space:read_u8(0x8eac7e), space:read_u8(0x8eac7f), space:read_u8(0x8eac80), space:read_u8(0x8eac81)))
machine:exit()
