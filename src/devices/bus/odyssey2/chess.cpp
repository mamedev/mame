// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 Videopac Chess Module emulation

 TODO:
   - this code is just a stub... hence, almost everything is still to do!

 ***********************************************************************************************************/


#include "emu.h"
#include "chess.h"


//-------------------------------------------------
//  o2_chess_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(O2_ROM_CHESS, o2_chess_device, "o2_chess", "Odyssey 2 Videopac Chess Module")


o2_chess_device::o2_chess_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: o2_rom_device(mconfig, O2_ROM_CHESS, tag, owner, clock)
	, m_cpu(*this, "subcpu")
{
}


ADDRESS_MAP_START(o2_chess_device::chess_mem)
	AM_RANGE(0x0000, 0x07ff) AM_READ(read_rom04)
ADDRESS_MAP_END

ADDRESS_MAP_START(o2_chess_device::chess_io)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(o2_chess_device::device_add_mconfig)
	MCFG_CPU_ADD("subcpu", NSC800, XTAL(4'000'000))
	MCFG_CPU_PROGRAM_MAP(chess_mem)
	MCFG_CPU_IO_MAP(chess_io)
MACHINE_CONFIG_END
