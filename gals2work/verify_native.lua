-- Check the real 68000 B1C6 decoder against decode_bg15.py's output.
-- Run on galpani2j, whose disassembly and task addresses are in this folder.
-- Supply the verified Asia B5 bytes in volatile ROM memory for this test only.
-- Use an isolated cfg/nvram directory.
local machine = manager.machine
assert(machine.system.name == "galpani2j", "this harness uses Japanese program addresses")
local space = machine.devices[":sub"].spaces.program
local source = assert(io.open("gals2work/galpani2_subdata_logical.bin", "rb"))
source:seek("set", 0x2cfb1e)
local asset = assert(source:read(61820))
source:close()
local region = machine.memory.regions[":subdata"]
for byte = 1, #asset, 2 do
 region:write_u16(0x2cfb1e + byte - 1, string.unpack(">I2", asset, byte))
end
local frames, active = 0, false
local function on_frame()
 frames = frames + 1
 if frames == 300 then
  local base = space:read_u32(0x1094a8)
  assert(base >= 0x100000 and base < 0x140000, "task table not initialized")
  for slot = 0, 0x1f0, 0x10 do
   if space:read_u16(base + slot + 2) == 0x5a then
    local block = 0x13f000
    space:write_u32(block, 0x2cfb1e + 6) -- Asia image B5, AFTER its header
    space:write_u32(block + 4, 0x4c0000)
    space:write_u16(block + 8, 0xff)
    space:write_u16(block + 10, 0xff)
    space:write_u16(block + 14, 0xff) -- D2=0: write all pixels
    space:write_u16(0x10b6d0, 0)
    space:write_u32(base + slot + 8, block)
    -- The scheduler normally assigns this when resolving a zero code pointer.
    -- This harness enters B1C6 directly, so it must supply the coroutine save area.
    space:write_u32(base + slot + 12, block + 0x100)
    space:write_u32(base + slot + 4, 0xb1c6)
    space:write_u8(base + slot, 0x40)
    active = true
    print("Started native B1C6 with verified Asia B5 payload")
    break
   end
  end
  assert(active, "task 0x5a not found")
 elseif active and space:read_u16(0x10b6d0) == 0xffff then
  local out = assert(io.open("gals2work/native_b5_words.bin", "wb"))
  for line = 0, 255 do
   for pixel = 0, 255 do
    out:write(string.pack(">I2", space:read_u16(0x4c0000 + line * 0x400 + pixel * 2) & 0x7fff))
   end
  end
  out:close()
  print("Native B1C6 completed without MCU reactivation; exported 65536 words")
  machine:exit()
 elseif frames == 600 then
  error("native decoder did not complete within five seconds")
 end
end
native_verification_subscription = emu.add_machine_frame_notifier(on_frame)
