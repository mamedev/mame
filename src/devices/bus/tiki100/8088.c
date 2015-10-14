// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TIKI-100 8/16 8088/8087 expansion card emulation

**********************************************************************/

#include "8088.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define I8088_TAG	"u3"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type TIKI100_8088 = &device_creator<tiki100_8088_t>;


//-------------------------------------------------
//  ROM( tiki100_8088 )
//-------------------------------------------------

ROM_START( tiki100_8088 )
	ROM_REGION( 0x1000, I8088_TAG, 0 )
	ROM_LOAD( "boot 1.0.u3", 0x0000, 0x1000, CRC(436974aa) SHA1(837087b3ab982d047e4f15799fef3daa37dd6c01) )

	ROM_REGION( 0x100, "u26", 0 )
	ROM_LOAD( "53ls140.u26", 0x000, 0x100, CRC(fc5902e1) SHA1(afb9cb54ab6fc449e7544ddb3cbebc3770c4f937) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *tiki100_8088_t::device_rom_region() const
{
	return ROM_NAME( tiki100_8088 );
}


static ADDRESS_MAP_START( i8088_mem, AS_PROGRAM, 8, tiki100_8088_t )
	AM_RANGE(0xff000, 0xfffff) AM_ROM AM_REGION(I8088_TAG, 0)
ADDRESS_MAP_END


static ADDRESS_MAP_START( i8088_io, AS_IO, 8, tiki100_8088_t )
	AM_RANGE(0x7f, 0x7f) AM_READWRITE(read, write)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( tiki100_8088 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( tiki100_8088 )
	MCFG_CPU_ADD(I8088_TAG, I8088, 6000000)
	MCFG_CPU_PROGRAM_MAP(i8088_mem)
	MCFG_CPU_IO_MAP(i8088_io)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor tiki100_8088_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( tiki100_8088 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tiki100_8088_t - constructor
//-------------------------------------------------

tiki100_8088_t::tiki100_8088_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, TIKI100_8088, "TIKI-100 8/16", tag, owner, clock, "tiki100_8088", __FILE__),
	device_tiki100bus_card_interface(mconfig, *this),
	m_maincpu(*this, I8088_TAG),
	m_data(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tiki100_8088_t::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tiki100_8088_t::device_reset()
{
	m_maincpu->reset();

	m_data = 0;
}


//-------------------------------------------------
//  tiki100bus_iorq_r - I/O read
//-------------------------------------------------

UINT8 tiki100_8088_t::iorq_r(address_space &space, offs_t offset, UINT8 data)
{
	if ((offset & 0xff) == 0x7f)
	{
		data = m_data;
	}

	return data;
}


//-------------------------------------------------
//  tiki100bus_iorq_w - I/O write
//-------------------------------------------------

void tiki100_8088_t::iorq_w(address_space &space, offs_t offset, UINT8 data)
{
	if ((offset & 0xff) == 0x7f)
	{
		m_data = data & 0x0f;

		if (BIT(data, 5))
		{
			device_reset();
		}
	}
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( tiki100_8088_t::read )
{
	return m_data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( tiki100_8088_t::write )
{
	m_data = data & 0x0f;
}
