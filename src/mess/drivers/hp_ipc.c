// license:BSD-3-Clause
// copyright-holders:
/******************************************************************************

HP Integral Personal Computer (IPC) skeleton driver 

Interrupt levels:
 7 - Soft reset from keyboard
 6 - Real-time clock or NBIR3 (external)
 5 - Disc drive or NBIR2 (external)
 4 - GPU or NBIR1 (external)
 3 - HP-IB, printer, or NBIR0 (external)
 2 - HP-HIL devices (keyboard, mouse, etc)
 1 - Real-time clock

******************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"


class hp_ipc_state : public driver_device
{
public:
	hp_ipc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	DECLARE_WRITE16_MEMBER(mmu_w);
	DECLARE_READ16_MEMBER(ram_r);
	DECLARE_WRITE16_MEMBER(ram_w);

private:
	required_device<m68000_device> m_maincpu;

	UINT32 m_mmu[4];
	UINT16 m_internal_ram[0x40000];

	inline UINT32 get_ram_address(offs_t offset) { return (m_mmu[(m_maincpu->get_fc() >> 1) & 3] + offset) & 0x3FFFFF; }
};


/*
Physical Address Map:
 000000 - 07FFFF Internal ROM (operating system PCA)
 080000 - 0FFFFF Internal ROM (option ROM PCA)
 100000 - 4FFFFF External ROM modules
 500000 - 5FFFFF Reserved
 600000 - 6FFFFF Internal I/O
  600000 - 60FFFF MMU
  610000 - 61FFFF Disc Drive
  620000 - 62FFFF Display
  630000 - 63FFFF HP-IB
  640000 - 64FFFF Real-Time Clock
  650000 - 65FFFF Printer
  660000 - 66FFFF Keyboard
  670000 - 67FFFF Speaker
  680000 - 68FFFF Reserved
  690000 - 69FFFF Reserved
  6A0000 - 6AFFFF Reserved
  6B0000 - 6BFFFF Reserved
  6C0000 - 6CFFFF Reserved
  6D0000 - 6DFFFF Reserved
  6E0000 - 6EFFFF Reserved
  6F0000 - 6FFFFF Reserved
 700000 - 7FFFFF External I/O
  700000 - 70FFFF Mainframe Port A
  710000 - 71FFFF Mainframe Port B
  720000 - 72FFFF Bus Expander Port A1
  730000 - 73FFFF Bus Expander Port A2
  740000 - 74FFFF Bus Expander Port A3
  750000 - 75FFFF Bus Expander Port A4
  760000 - 76FFFF Bus Expander Port A5
  770000 - 77FFFF Reserved
  780000 - 78FFFF Reserved
  790000 - 79FFFF Reserved
  7A0000 - 7AFFFF Bus Expander Port B1
  7B0000 - 7BFFFF Bus Expander Port B2
  7C0000 - 7CFFFF Bus Expander Port B3
  7D0000 - 7DFFFF Bus Expander Port B4
  7E0000 - 7EFFFF Bus Expander Port B5
  7F0000 - 7FFFFF Reserved
 800000 - EFFFFF External RAM modules
 F00000 - F7FFFF Internal RAM
 F80000 - FFFFFF Reserved

All accesses to 800000-FFFFFF go through the "MMU" to form a final physical address

*/
static ADDRESS_MAP_START(hp_ipc_mem, AS_PROGRAM, 16, hp_ipc_state)
	AM_RANGE(0x000000, 0x07FFFF) AM_ROM
	AM_RANGE(0x600000, 0x60FFFF) AM_WRITE(mmu_w)
	AM_RANGE(0x800000, 0xFFFFFF) AM_READWRITE(ram_r, ram_w)
ADDRESS_MAP_END


static INPUT_PORTS_START(hp_ipc)
INPUT_PORTS_END


WRITE16_MEMBER(hp_ipc_state::mmu_w)
{
	logerror("mmu_w: offset = %08x, data = %04x, register = %d, data_to_add = %08x\n", offset, data, offset & 3, (data & 0xFFF) << 10);
	m_mmu[offset & 3] = (data & 0xFFF) << 10;
}


READ16_MEMBER(hp_ipc_state::ram_r)
{
	UINT32 ram_address = get_ram_address(offset);

	//logerror("RAM read, offset = %08x, ram address = %08X\n", offset, ram_address);

	if (ram_address < 0x380000)
	{
		// External RAM modules
	}
	else if (ram_address < 0x3c0000)
	{
		// Internal RAM
		return m_internal_ram[offset & 0x3ffff];
	}
	return 0xffff;
}


WRITE16_MEMBER(hp_ipc_state::ram_w)
{
	UINT32 ram_address = get_ram_address(offset);

	//logerror("RAM write, offset = %08x, ram address = %08X, data = %04x\n", offset, ram_address, data);

	if (ram_address < 0x380000)
	{
		// External RAM modules
	}
	else if (ram_address < 0x3c0000)
	{
		// Internal RAM
		m_internal_ram[offset & 0x3ffff] = data;
	}
}


static MACHINE_CONFIG_START(hp_ipc, hp_ipc_state)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 15920000 / 2)
	MCFG_CPU_PROGRAM_MAP(hp_ipc_mem)
MACHINE_CONFIG_END


ROM_START(hp_ipc)
	ROM_REGION(0x100000, "maincpu" , 0)
	ROM_LOAD("hp ipc os 82991A.bin", 0x00000, 0x80000, BAD_DUMP CRC(df45a37b) SHA1(476af9923bca0d2d0f40aeb81be5145ca76fddf5)) // Should be spread across 4 x 128K ROMs
ROM_END


COMP(198?, hp_ipc, 0, 0, hp_ipc, hp_ipc, driver_device, 0, "HP", "Integral Personal Computer", MACHINE_IS_SKELETON)

