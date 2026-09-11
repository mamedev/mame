-- license:BSD-3-Clause
-- Assemble tms9901_irq.a99 with xas99.py -R -b -o irq-test.bin.
-- Set TMS9901_IRQ_TEST to that binary and run ti99_4a with console ROMs,
-- -autoboot_delay 1 -autoboot_script tests/emu/tms9901_irq.lua.
-- No cartridge or game data is required. Completion takes about 5 seconds
-- of emulated time. The caller must check for the PASS line, not just exit 0.
local m = manager.machine
local cpu = m.devices[':maincpu']
local symbols = emu.symbol_table(m)
local file = assert(io.open(assert(os.getenv('TMS9901_IRQ_TEST')), 'rb'))
local code = file:read('a')
file:close()
assert(#code == 78, 'Unexpected test binary size')
for i = 1,#code,2 do
    symbols:set_memory_value(':maincpu','p',0x8320+i-1,2,code:byte(i)*256+code:byte(i+1),true)
end
cpu.state.ST.value = 0
cpu.state.WP.value = 0x8300
cpu.state.PC.value = 0x8320
local frames = 0
emu.register_frame_done(function()
    frames = frames+1
    local pc = cpu.state.PC.value
    local count = symbols:memory_value(':maincpu','p',0x830e,2,true)
    if cpu.state.WP.value ~= 0x8300 or pc < 0x8320 or pc > 0x836e then
        print(string.format('FAIL: stale interrupt after %d iterations; PC=%04X WP=%04X',count,pc,cpu.state.WP.value))
        m:exit()
    elseif count == 256 then
        print('PASS: 256 VDP interrupt acknowledgements across 64 delay phases')
        m:exit()
    elseif frames > 600 then
        print('FAIL: interrupt acknowledgement test timed out')
        m:exit()
    end
end,'TMS9901 IRQ regression')
