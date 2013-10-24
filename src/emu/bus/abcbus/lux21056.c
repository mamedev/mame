// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor 55 21056-00 Xebec Interface Host Adapter emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "lux21056.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define Z80_TAG         "5a"
#define Z80DMA_TAG      "6a"
#define SASIBUS_TAG     "sasi"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type LUXOR_55_21056 = &device_creator<luxor_55_21056_device>;


//-------------------------------------------------
//  ROM( luxor_55_21056 )
//-------------------------------------------------

ROM_START( luxor_55_21056 )
	ROM_REGION( 0x800, Z80_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "st4038", "Seagate ST4038 (CHS: 733,5,17,512)" )
	ROMX_LOAD( "st4038.6c", 0x000, 0x800, CRC(4c803b87) SHA1(1141bb51ad9200fc32d92a749460843dc6af8953), ROM_BIOS(1) ) // Seagate ST4038 (http://stason.org/TULARC/pc/hard-drives-hdd/seagate/ST4038-1987-31MB-5-25-FH-MFM-ST412.html)
	ROM_SYSTEM_BIOS( 1, "st225", "Seagate ST225 (CHS: 615,4,17,512)" )
	ROMX_LOAD( "st225.6c",  0x000, 0x800, CRC(c9f68f81) SHA1(7ff8b2a19f71fe0279ab3e5a0a5fffcb6030360c), ROM_BIOS(2) ) // Seagate ST225 (http://stason.org/TULARC/pc/hard-drives-hdd/seagate/ST225-21MB-5-25-HH-MFM-ST412.html)
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *luxor_55_21056_device::device_rom_region() const
{
	return ROM_NAME( luxor_55_21056 );
}


//-------------------------------------------------
//  ADDRESS_MAP( luxor_55_21056_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( luxor_55_21056_mem, AS_PROGRAM, 8, luxor_55_21056_device )
	AM_RANGE(0x0000, 0x07ff) AM_ROM AM_REGION(Z80_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( luxor_55_21056_io )
//-------------------------------------------------

static ADDRESS_MAP_START( luxor_55_21056_io, AS_IO, 8, luxor_55_21056_device )
ADDRESS_MAP_END


//-------------------------------------------------
//  z80_daisy_config daisy_chain
//-------------------------------------------------

static const z80_daisy_config daisy_chain[] =
{
	{ Z80DMA_TAG }
};


//-------------------------------------------------
//  Z80DMA_INTERFACE( dma_intf )
//-------------------------------------------------

READ8_MEMBER(luxor_55_21056_device::memory_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(luxor_55_21056_device::memory_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.write_byte(offset, data);
}

READ8_MEMBER(luxor_55_21056_device::io_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(luxor_55_21056_device::io_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.write_byte(offset, data);
}

static Z80DMA_INTERFACE( dma_intf )
{
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_HALT),
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0),
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, luxor_55_21056_device, memory_read_byte),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, luxor_55_21056_device, memory_write_byte),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, luxor_55_21056_device, io_read_byte),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, luxor_55_21056_device, io_write_byte),
};


//-------------------------------------------------
//  MACHINE_DRIVER( luxor_55_21056 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( luxor_55_21056 )
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_8MHz/2)
	MCFG_CPU_PROGRAM_MAP(luxor_55_21056_mem)
	MCFG_CPU_IO_MAP(luxor_55_21056_io)
	MCFG_CPU_CONFIG(daisy_chain)

	MCFG_Z80DMA_ADD(Z80DMA_TAG, XTAL_8MHz/2, dma_intf)

	MCFG_SCSIBUS_ADD(SASIBUS_TAG)
	MCFG_SCSIDEV_ADD(SASIBUS_TAG ":harddisk0", SCSIHD, SCSI_ID_0)
	MCFG_SCSICB_ADD(SASIBUS_TAG ":host")
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor luxor_55_21056_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( luxor_55_21056 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  luxor_55_21056_device - constructor
//-------------------------------------------------

luxor_55_21056_device::luxor_55_21056_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, LUXOR_55_21056, "ABC XEBEC", tag, owner, clock, "luxor_55_21056", __FILE__),
		device_abcbus_card_interface(mconfig, *this),
		m_maincpu(*this, Z80_TAG),
		m_sasibus(*this, SASIBUS_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void luxor_55_21056_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void luxor_55_21056_device::device_reset()
{
}



//**************************************************************************
//  ABC BUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  abcbus_cs -
//-------------------------------------------------

void luxor_55_21056_device::abcbus_cs(UINT8 data)
{
}
