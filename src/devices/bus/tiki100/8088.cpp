// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TIKI-100 8/16 8088/8087 expansion card emulation

**********************************************************************/

#include "emu.h"
#include "8088.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define I8088_TAG   "u3"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TIKI100_8088, tiki100_8088_device, "tiki100_8088", "TIKI-100 8/16")


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

const tiny_rom_entry *tiki100_8088_device::device_rom_region() const
{
	return ROM_NAME( tiki100_8088 );
}


void tiki100_8088_device::i8088_mem(address_map &map)
{
	map(0x00000, 0xbffff).ram();
	//map(0xc0000, 0xcffff).rw(m_bus, FUNC(tiki100_bus_device::exin_mrq_r), FUNC(tiki100_bus_device::exin_mrq_w)); don't have m_bus until start
	map(0xff000, 0xfffff).rom().region(I8088_TAG, 0);
}


void tiki100_8088_device::i8088_io(address_map &map)
{
	map(0x7f, 0x7f).rw(FUNC(tiki100_8088_device::read), FUNC(tiki100_8088_device::write));
}


//-------------------------------------------------
//  device_add_mconfig()
//-------------------------------------------------

void tiki100_8088_device::device_add_mconfig(machine_config &config)
{
	I8088(config, m_maincpu, 6000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &tiki100_8088_device::i8088_mem);
	m_maincpu->set_addrmap(AS_IO, &tiki100_8088_device::i8088_io);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tiki100_8088_device - constructor
//-------------------------------------------------

tiki100_8088_device::tiki100_8088_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TIKI100_8088, tag, owner, clock),
	device_tiki100bus_card_interface(mconfig, *this),
	m_maincpu(*this, I8088_TAG),
	m_data(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tiki100_8088_device::device_start()
{
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(
			0xc0000, 0xcffff,
			read8sm_delegate(*m_bus, FUNC(tiki100_bus_device::exin_mrq_r)),
			write8sm_delegate(*m_bus, FUNC(tiki100_bus_device::exin_mrq_w)));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tiki100_8088_device::device_reset()
{
	m_maincpu->reset();

	m_data = 0;
}


//-------------------------------------------------
//  tiki100bus_iorq_r - I/O read
//-------------------------------------------------

uint8_t tiki100_8088_device::iorq_r(offs_t offset, uint8_t data)
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

void tiki100_8088_device::iorq_w(offs_t offset, uint8_t data)
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

READ8_MEMBER( tiki100_8088_device::read )
{
	return m_busak << 4 | m_data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( tiki100_8088_device::write )
{
	m_data = data & 0x0f;

	m_bus->busrq_w(BIT(data, 4));
}
