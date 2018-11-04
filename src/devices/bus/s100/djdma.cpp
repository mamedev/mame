// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Morrow Designs Disk Jockey/DMA floppy controller board emulation

**********************************************************************/

#include "emu.h"
#include "djdma.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define Z80_TAG     "14a"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(S100_DJDMA, s100_djdma_device, "s100_djdma", "Morrow Disk Jockey/DMA")


//-------------------------------------------------
//  ROM( djdma )
//-------------------------------------------------

ROM_START( djdma )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_LOAD( "djdma 2.5 26c2.16d", 0x0000, 0x1000, CRC(71ff1924) SHA1(6907575954836364826b8fdef3c108bb93bf3d25) )

	ROM_REGION( 0x500, "proms", 0 )
	ROM_LOAD( "djdma2x.3d", 0x000, 0x200, CRC(f9b1648b) SHA1(1ebe6dc8ccfbfa6c7dc98cb65fbc9fa21e3b687f) )
	ROM_LOAD( "dj-11c-a.11c", 0x200, 0x200, CRC(0c6c4af0) SHA1(8fdcd34e3d07add793ff9ba27c77af864e1731bb) )
	ROM_LOAD( "dja-12b.12b", 0x400, 0x100, CRC(040044af) SHA1(d069dc0e6b680cb8848d165aff6681ed2d750961) )

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "djdma-2b.2b", 0x000, 0x104, CRC(d6925f2c) SHA1(1e58dfb7b8a2a5bbaa6589d4018042626fd5ceaf) ) // PAL16R4
	ROM_LOAD( "djdma 2c 81d5.2c", 0x0000, 0x10, NO_DUMP ) // ?
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *s100_djdma_device::device_rom_region() const
{
	return ROM_NAME( djdma );
}


//-------------------------------------------------
//  ADDRESS_MAP( djdma_mem )
//-------------------------------------------------

ADDRESS_MAP_START(s100_djdma_device::djdma_mem)
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION("14a", 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( djdma_io )
//-------------------------------------------------

ADDRESS_MAP_START(s100_djdma_device::djdma_io)
ADDRESS_MAP_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(s100_djdma_device::device_add_mconfig)
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL(4'000'000))
	MCFG_CPU_PROGRAM_MAP(djdma_mem)
	MCFG_CPU_IO_MAP(djdma_io)
MACHINE_CONFIG_END



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  s100_djdma_device - constructor
//-------------------------------------------------

s100_djdma_device::s100_djdma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, S100_DJDMA, tag, owner, clock),
	device_s100_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s100_djdma_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void s100_djdma_device::device_reset()
{
}
