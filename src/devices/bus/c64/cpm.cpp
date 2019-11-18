// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 64 CP/M cartridge emulation

    http://www.baltissen.org/newhtm/c64_cpm.htm

**********************************************************************/

/*

    TODO:

    - Z80 clock speed

*/

#include "emu.h"
#include "cpm.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define Z80_TAG     "z80"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_CPM, c64_cpm_cartridge_device, "c64_cpm", "C64 CP/M cartridge")


//-------------------------------------------------
//  ADDRESS_MAP( z80_mem )
//-------------------------------------------------

void c64_cpm_cartridge_device::z80_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(c64_cpm_cartridge_device::dma_r), FUNC(c64_cpm_cartridge_device::dma_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( z80_io )
//-------------------------------------------------

void c64_cpm_cartridge_device::z80_io(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(c64_cpm_cartridge_device::dma_r), FUNC(c64_cpm_cartridge_device::dma_w));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_cpm_cartridge_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_maincpu, 3000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &c64_cpm_cartridge_device::z80_mem);
	m_maincpu->set_addrmap(AS_IO, &c64_cpm_cartridge_device::z80_io);
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  update_signals -
//-------------------------------------------------

inline void c64_cpm_cartridge_device::update_signals()
{
	if (m_enabled)
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_slot->dma_w(ASSERT_LINE);

		if (m_reset)
		{
			m_maincpu->reset();
			m_maincpu->set_state_int(Z80_PC, 0);
			m_reset = 0;
		}
	}
	else
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_slot->dma_w(CLEAR_LINE);
	}

/*
    // NOTE: the following is how it actually works once the Z80 core has been rewritten

    // C64 DMA
    m_slot->dma_w(m_enabled ? ASSERT_LINE : CLEAR_LINE);

    // Z80 BUSRQ
    int busrq = !(m_enabled & !m_ba) ? CLEAR_LINE : ASSERT_LINE;
    m_maincpu->set_input_line(Z80_INPUT_LINE_BUSRQ, busrq);

    // Z80 WAIT
    m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, m_enabled ? CLEAR_LINE : ASSERT_LINE);
*/
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_cpm_cartridge_device - constructor
//-------------------------------------------------

c64_cpm_cartridge_device::c64_cpm_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_CPM, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_maincpu(*this, Z80_TAG),
	m_enabled(0),
	m_ba(1), m_reset(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_cpm_cartridge_device::device_start()
{
	// state saving
	save_item(NAME(m_enabled));
	save_item(NAME(m_ba));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_cpm_cartridge_device::device_reset()
{
	m_enabled = 0;
	m_reset = 1;

	update_signals();
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_cpm_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		m_enabled = !BIT(data, 0);

		update_signals();
	}
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_cpm_cartridge_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	if (m_ba != ba)
	{
		m_ba = ba;

		update_signals();
	}

	return 1;
}


//-------------------------------------------------
//  dma_r -
//-------------------------------------------------

READ8_MEMBER( c64_cpm_cartridge_device::dma_r )
{
	uint8_t data = 0xff;

	if (m_enabled)
	{
		offs_t addr = (offset + 0x1000) & 0xffff;

		data = m_slot->dma_cd_r(addr);
	}

	return data;
}


//-------------------------------------------------
//  dma_w -
//-------------------------------------------------

WRITE8_MEMBER( c64_cpm_cartridge_device::dma_w )
{
	if (m_enabled)
	{
		offs_t addr = (offset + 0x1000) & 0xffff;

		m_slot->dma_cd_w(addr, data);
	}
}
