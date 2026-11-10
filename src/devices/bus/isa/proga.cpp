// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

	PROGA graphics card
	made by Institut Ivo Lola Ribar (Yugoslavia)

	HD63484P8 ACRTC, 25 MHz XTAL
	16x 41264 64Kx4 multiport VRAM (512KB)
	27256 EPROM, two DE-9 TTL video connectors
	HM65728K - 2Kx8 static RAM

	Option ROM only programs the ACRTC registers, card is used by
	drawing commands from software.

	ROM contains initialization data for HD63484 after which fonts are located.

	Board available is missing GAL/PALs.

	TODO:
		- Find software using this card

***************************************************************************/

#include "emu.h"
#include "proga.h"
#include "screen.h"

ROM_START( irl_proga )
	ROM_REGION(0x8000, "rom", 0)
	ROM_LOAD("proga.u2", 0x00000, 0x8000, CRC(4b40cee2) SHA1(93248d94dab9c1688d7421570c1d5fa02ef129fa) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_PROGA, isa8_proga_device, "irl_proga", "Ivo Lola Ribar PROGA")

void isa8_proga_device::videoram_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram(); // 16 x 41264 (256K words)
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_proga_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(25_MHz_XTAL, 1000, 168, 168 + 720, 500, 23, 23 + 460);
	screen.set_screen_update("acrtc", FUNC(hd63484_device::update_screen));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(isa8_proga_device::palette_init), 16);

	HD63484(config, m_acrtc, 25_MHz_XTAL / 4); // HD63484P8
	m_acrtc->set_screen("screen");
	m_acrtc->set_auto_configure_screen(false);
	m_acrtc->set_addrmap(0, &isa8_proga_device::videoram_map);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa8_proga_device::device_rom_region() const
{
	return ROM_NAME( irl_proga );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_proga_device - constructor
//-------------------------------------------------

isa8_proga_device::isa8_proga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA8_PROGA, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	m_acrtc(*this, "acrtc")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_proga_device::device_start()
{
	set_isa_device();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_proga_device::device_reset()
{
	remap(AS_PROGRAM, 0, 0xfffff);
}

void isa8_proga_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		// TODO: There is DIPSW on board, code is relocatable
		m_isa->install_rom(this, 0xc0000, 0xc6fff, "rom");
		// TODO: Assume static RAM is located right after ROM and before ACRTC mapping
		m_isa->install_memory(0xc7800, 0xc7802,	read8sm_delegate(*this, FUNC(isa8_proga_device::acrtc_r)), write8sm_delegate(*this, FUNC(isa8_proga_device::acrtc_w)));
	}
}

//-------------------------------------------------
//  ACRTC interface
//  +0 write: address register
//  +1 read:  status register
//  +2 r/w:   control register
//-------------------------------------------------

uint8_t isa8_proga_device::acrtc_r(offs_t offset)
{
	switch (offset)
	{
	case 1:
		return m_acrtc->read8(0);
	case 2:
		return m_acrtc->read8(1);
	default:
		return 0xff;
	}
}

void isa8_proga_device::acrtc_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0:
		m_acrtc->write8(0, data);
		break;
	case 2:
		m_acrtc->write8(1, data);
		break;
	}
}

//-------------------------------------------------
//  palette_init
//-------------------------------------------------

void isa8_proga_device::palette_init(palette_device &palette) const
{
	// TODO: palette is unknown
	static constexpr rgb_t proga_pens[16] = {
		{ 0x00, 0x00, 0x00 }, // 0x0  Black
		{ 0xaa, 0x00, 0x00 }, // 0x1  Red
		{ 0x00, 0xaa, 0x00 }, // 0x2  Green
		{ 0xaa, 0x55, 0x00 }, // 0x3  Yellow/Brown
		{ 0x00, 0x00, 0xaa }, // 0x4  Blue
		{ 0xaa, 0x00, 0xaa }, // 0x5  Magenta
		{ 0x00, 0xaa, 0xaa }, // 0x6  Cyan
		{ 0xaa, 0xaa, 0xaa }, // 0x7  Light Gray
		{ 0x55, 0x55, 0x55 }, // 0x8  Dark Gray
		{ 0xff, 0x55, 0x55 }, // 0x9  Light Red
		{ 0x55, 0xff, 0x55 }, // 0xA  Light Green
		{ 0xff, 0xff, 0x55 }, // 0xB  Yellow
		{ 0x55, 0x55, 0xff }, // 0xC  Light Blue
		{ 0xff, 0x55, 0xff }, // 0xD  Light Magenta
		{ 0x55, 0xff, 0xff }, // 0xE  Light Cyan
		{ 0xff, 0xff, 0xff }, // 0xF  White
	};
	palette.set_pen_colors(0, proga_pens);
}
