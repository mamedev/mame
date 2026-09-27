// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CBM 500/600/700 High Resolution Graphics cartridge emulation

**********************************************************************/

/*

    TODO:

    http://www.wfking.de/hires.htm

    - version A (EF9365, 512x512 interlaced, 1 page)
    - version B (EF9366, 512x256 non-interlaced, 2 pages)
    - 256KB version ROM

*/

#include "emu.h"
#include "hrg.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define EF9365_TAG  "ef9365"
#define EF9366_TAG  EF9365_TAG
#define SCREEN_TAG  "screen"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CBM2_HRG_A, cbm2_hrg_a_device, "cbm2_hrga", "CBM 500/600/700 High Resolution Graphics (A)")
DEFINE_DEVICE_TYPE(CBM2_HRG_B, cbm2_hrg_b_device, "cbm2_hrgb", "CBM 500/600/700 High Resolution Graphics (B)")


//-------------------------------------------------
//  ROM( cbm2_hrg )
//-------------------------------------------------

ROM_START( cbm2_hrg )
	ROM_REGION( 0x2000, "bank3", 0 )
	ROM_LOAD( "324688-01 sw gr 600.bin", 0x0000, 0x2000, CRC(863e9ef8) SHA1(d75ffa97b2dd4e1baefe4acaa130daae866ab0e8) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *cbm2_hrg_device::device_rom_region() const
{
	return ROM_NAME( cbm2_hrg );
}


//-------------------------------------------------
//  ADDRESS_MAP( hrg_a_map )
//-------------------------------------------------

void cbm2_hrg_a_device::hrg_a_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x7fff).rw(FUNC(cbm2_hrg_a_device::ram_r), FUNC(cbm2_hrg_a_device::ram_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( hrg_b_map )
//-------------------------------------------------

void cbm2_hrg_b_device::hrg_b_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x3fff).rw(FUNC(cbm2_hrg_b_device::ram_r), FUNC(cbm2_hrg_b_device::ram_w));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void cbm2_hrg_a_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, SCREEN_TAG).set_color(rgb_t::green()));
	screen.set_screen_update(FUNC(cbm2_hrg_a_device::screen_update));
	screen.set_size(512, 512);
	screen.set_visarea(0, 512-1, 0, 512-1);
	screen.set_refresh_hz(25);
	PALETTE(config, m_palette, palette_device::MONOCHROME_INVERTED);

	EF9365(config, m_gdc, 1750000);
	m_gdc->set_screen(SCREEN_TAG);
	m_gdc->set_addrmap(0, &cbm2_hrg_a_device::hrg_a_map);
	m_gdc->set_palette_tag(m_palette);
	m_gdc->write_msl().set(FUNC(cbm2_hrg_a_device::msl_w));
	m_gdc->set_nb_bitplanes(1);
	m_gdc->set_display_mode(ef9365_device::DISPLAY_MODE_512x512);
}

void cbm2_hrg_b_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, SCREEN_TAG).set_color(rgb_t::green()));
	screen.set_screen_update(FUNC(cbm2_hrg_b_device::screen_update));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_refresh_hz(50);
	PALETTE(config, m_palette, palette_device::MONOCHROME_INVERTED);

	EF9365(config, m_gdc, 1750000); //EF9366
	m_gdc->set_screen(SCREEN_TAG);
	m_gdc->set_addrmap(0, &cbm2_hrg_b_device::hrg_b_map);
	m_gdc->set_palette_tag(m_palette);
	m_gdc->write_msl().set(FUNC(cbm2_hrg_b_device::msl_w));
	m_gdc->set_nb_bitplanes(1);
	m_gdc->set_display_mode(ef9365_device::DISPLAY_MODE_512x256);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cbm2_hrg_device - constructor
//-------------------------------------------------

cbm2_hrg_device::cbm2_hrg_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, offs_t page_mask) :
	device_t(mconfig, type, tag, owner, clock),
	device_cbm2_expansion_card_interface(mconfig, *this),
	m_gdc(*this, EF9366_TAG),
	m_palette(*this, "palette"),
	m_bank3(*this, "bank3"),
	m_page_mask(page_mask),
	m_control(0),
	m_readback(0),
	m_msl(0)
{
}

cbm2_hrg_a_device::cbm2_hrg_a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	cbm2_hrg_device(mconfig, CBM2_HRG_A, tag, owner, clock, 0)
{
}

cbm2_hrg_b_device::cbm2_hrg_b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	cbm2_hrg_device(mconfig, CBM2_HRG_B, tag, owner, clock, 0x4000)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cbm2_hrg_device::device_start()
{
	m_ram = std::make_unique<uint8_t[]>(0x8000);
	std::fill_n(m_ram.get(), 0x8000, 0);

	save_pointer(NAME(m_ram), 0x8000);
	save_item(NAME(m_control));
	save_item(NAME(m_readback));
	save_item(NAME(m_msl));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cbm2_hrg_device::device_reset()
{
	m_control = 0;

	m_gdc->reset();

	m_slot->bank3().install_rom(0x0000, 0x1f7f, m_bank3->base());
	m_slot->bank3().install_write_handler(0x1f80, 0x1f80, write8smo_delegate(*this, FUNC(cbm2_hrg_device::control_w)));
	m_slot->bank3().install_read_handler(0x1fa0, 0x1fa0, read8smo_delegate(*this, FUNC(cbm2_hrg_device::readback_r)));
	m_slot->bank3().install_readwrite_handler(0x1ff0, 0x1fff, read8sm_delegate(*m_gdc, FUNC(ef9365_device::data_r)), write8sm_delegate(*m_gdc, FUNC(ef9365_device::data_w)));
}


//-------------------------------------------------
//  ram_r -
//-------------------------------------------------

uint8_t cbm2_hrg_device::ram_r(offs_t offset)
{
	uint8_t const data = m_ram[page_offset(1) | offset];

	if (!BIT(m_control, 0) && !machine().side_effects_disabled())
		m_readback = data;

	return data;
}


//-------------------------------------------------
//  ram_w -
//-------------------------------------------------

void cbm2_hrg_device::ram_w(offs_t offset, uint8_t data)
{
	offs_t const addr = page_offset(1) | offset;

	if (BIT(m_control, 2))
		m_ram[addr] ^= 0x80 >> (m_msl & 7);
	else
		m_ram[addr] = data;
}


//-------------------------------------------------
//  readback_r -
//-------------------------------------------------

uint8_t cbm2_hrg_device::readback_r()
{
	return bitswap<8>(m_readback, 0, 1, 2, 3, 4, 5, 6, 7);
}


//-------------------------------------------------
//  screen_update -
//-------------------------------------------------

uint32_t cbm2_hrg_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t const *const pens = m_palette->pens();
	offs_t const base = page_offset(4);

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			uint8_t const data = m_ram[base | ((y * 512 + x) >> 3)];

			bitmap.pix(y, x) = pens[BIT(data, ~x & 7)];
		}
	}

	return 0;
}


//-------------------------------------------------
//  control_w -
//-------------------------------------------------

void cbm2_hrg_device::control_w(uint8_t data)
{
	/*

	    bit     description

	    0       memory readback latch enable (0=active)
	    1       operating page select (version B)
	    2       invert pixels (1=active)
	    3       display switch (1=graphic)
	    4       display page select (version B)
	    5
	    6
	    7

	*/

	m_control = data;
}
