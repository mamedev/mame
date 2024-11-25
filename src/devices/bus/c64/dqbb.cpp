// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Brown Boxes Double Quick Brown Box emulation

**********************************************************************/

/*

    TODO:

    - 64/128 mode switch
    - dump of the initial NVRAM contents

*/

#include "emu.h"
#include "dqbb.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_DQBB, c64_dqbb_cartridge_device, "c64_dqbb", "C64 Double Quick Brown Box cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_dqbb_cartridge_device - constructor
//-------------------------------------------------

c64_dqbb_cartridge_device::c64_dqbb_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_DQBB, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	device_nvram_interface(mconfig, *this),
	m_cs(0),
	m_we(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_dqbb_cartridge_device::device_start()
{
	// allocate memory
	m_nvram = std::make_unique<uint8_t[]>(0x4000);
	save_pointer(NAME(m_nvram), 0x4000);

	// state saving
	save_item(NAME(m_cs));
	save_item(NAME(m_we));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_dqbb_cartridge_device::device_reset()
{
	m_exrom = 0; // TODO 1 in 128 mode
	m_game = 1;
	m_cs = 0;
	m_we = 0;
}


void c64_dqbb_cartridge_device::nvram_default()
{
}


bool c64_dqbb_cartridge_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = read(file, m_nvram.get(), 0x4000);
	return !err && (actual == 0x4000);
}


bool c64_dqbb_cartridge_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = write(file, m_nvram.get(), 0x4000);
	return !err;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_dqbb_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!m_cs && (!roml || !romh))
	{
		data = m_nvram[offset & 0x3fff];
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_dqbb_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!m_cs && m_we && (offset >= 0x8000 && offset < 0xc000))
	{
		m_nvram[offset & 0x3fff] = data;
	}
	else if (!io1)
	{
		/*

		    bit     description

		    0
		    1
		    2       GAME
		    3
		    4       WE
		    5
		    6       EXROM
		    7       _CS

		*/

		m_exrom = !BIT(data, 6);
		m_game = !BIT(data, 2);
		m_we = BIT(data, 4);
		m_cs = BIT(data, 7);
	}
}
