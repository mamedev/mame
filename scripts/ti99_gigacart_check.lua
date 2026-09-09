-- license:BSD-3-Clause
-- Run with -autoboot_script and GIGACART_CHECKSUMS pointing to a local text
-- file containing BANK SUM hex pairs. Each sum covers all 4096 big-endian
-- words, modulo 65536. No game-specific reference data is distributed here.
-- Debugger memory access bypasses the TI's wait-state sequencing; ordinary
-- Lua space reads/writes do not initiate a complete datamux transaction.
local machine = manager.machine
local cpu = machine.devices[':maincpu']
local symbols = emu.symbol_table(machine)
local mem = {read_u16=function(self,a) return symbols:memory_value(':maincpu','p',a,2,true) end, write_u16=function(self,a,v) symbols:set_memory_value(':maincpu','p',a,2,v,true) end}
local cart = machine.devices[':gromport:single:cartridge']
local oldpc = cpu.state['PC'].value
local checked = 0
local reference = assert(os.getenv('GIGACART_CHECKSUMS'), 'Set GIGACART_CHECKSUMS to a local full-bank checksum reference')
for line in io.lines(reference) do
  local b,s = line:match('^(%x+)%s+(%x%x%x%x)')
  if b then
    local bank,expected = tonumber(b,16),tonumber(s,16)
    assert(bank == checked and bank < 0x100000, 'Expected consecutive banks starting at zero')
    mem:write_u16(0x6000 + (bank & 0xfff)*2, ((bank >> 12) & 255) << 8)
    local sum = 0
    for addr=0x6000,0x7ffe,2 do sum = (sum + mem:read_u16(addr)) & 0xffff end
    assert(sum == expected, string.format('bank %04X expected %04X got %04X',bank,expected,sum))
    checked = checked+1
  end
end
assert(checked > 0, 'No checksum rows found')
mem:write_u16(0x6000, 0x0102)
assert(emu.item(cart.items['0/m_rom_page']):read(0)==0x1000, 'TI word write order')
local state = machine:buffer_save()
local gromaddr = emu.item(cart.items['0/m_grom_address'])
local savedgrom = gromaddr:read(0)
gromaddr:write(0, 0x8081)
mem:write_u16(0x7ffe, 0x0300)
assert(emu.item(cart.items['0/m_rom_page']):read(0)~=0x1000)
assert(machine:buffer_load(state))
assert(emu.item(cart.items['0/m_rom_page']):read(0)==0x1000, 'saved bank latch')
assert(gromaddr:read(0)==savedgrom, 'saved GROM address and page')
assert(cpu.state['PC'].value == oldpc)
print('PASS: '..checked..' full-bank checksums, TI word write ordering, bank and GROM save/restore')
machine:exit()
