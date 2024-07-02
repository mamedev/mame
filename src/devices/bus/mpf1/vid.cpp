// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Bardehle Electronics VIDEO-MPF-I

***************************************************************************/

#include "emu.h"
#include "vid.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"


namespace {

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( mpf_vid )
	ROM_REGION(0x0800, "rom", 0)
	ROM_SYSTEM_BIOS(0, "20", "2.0")
	ROMX_LOAD("vidmon_v20.bin", 0x0000, 0x0800, CRC(3a2293a0) SHA1(281092d82a272383b584b31064421296add20c46), ROM_BIOS(0))

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD("vidchar.bin", 0x0000, 0x0800, CRC(d56211f9) SHA1(9cf717de353b86c38f94b805d31a16d8079896af))
ROM_END


//-------------------------------------------------
//  gfx_layout mpf_vid_charlayout
//-------------------------------------------------

static const gfx_layout mpf_vid_charlayout =
{
	8, 12,                  /* 8 x 12 characters */
	128,                    /* 128 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0 * 8,  1 * 8,  2 * 8,  3 * 8,  4 * 8,  5 * 8,  6 * 8,  7 * 8, 8 * 8,  9 * 8,  10 * 8,  11 * 8 },
	8 * 16                  /* every char takes 16 bytes */
};

//-------------------------------------------------
//  GFXDECODE( gfx_mpf_vid )
//-------------------------------------------------

static GFXDECODE_START(gfx_mpf_vid)
	GFXDECODE_ENTRY("chargen", 0, mpf_vid_charlayout, 0, 1)
GFXDECODE_END


//-------------------------------------------------
//  mpf_vid_device - constructor
//-------------------------------------------------

class mpf_vid_device : public device_t, public device_mpf1_exp_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	mpf_vid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, MPF_VID, tag, owner, clock)
		, device_mpf1_exp_interface(mconfig, *this)
		, m_rom(*this, "rom")
		, m_chargen(*this, "chargen")
		, m_crtc(*this, "mc6845")
		, m_palette(*this, "palette")
	{
	}

protected:
	virtual void device_add_mconfig(machine_config &config) override
	{
		screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
		screen.set_raw(8_MHz_XTAL, 512, 0, 320, 326, 0, 240);
		screen.set_screen_update("mc6845", FUNC(mc6845_device::screen_update));

		GFXDECODE(config, "gfxdecode", m_palette, gfx_mpf_vid);
		PALETTE(config, m_palette, palette_device::MONOCHROME);

		MC6845(config, m_crtc, 8_MHz_XTAL / 8); // UM6845
		m_crtc->set_screen("screen");
		m_crtc->set_show_border_area(false);
		m_crtc->set_char_width(8);
		m_crtc->set_update_row_callback(FUNC(mpf_vid_device::crtc_update_row));
	}

	virtual const tiny_rom_entry *device_rom_region() const override
	{
		return ROM_NAME( mpf_vid );
	}

	virtual void device_start() override
	{
		m_videoram = make_unique_clear<uint8_t[]>(0x0800);

		save_pointer(NAME(m_videoram), 0x0800);
	}

	virtual void device_reset() override
	{
		program_space().install_ram(0x4000, 0x47ff, 0x800, m_videoram.get());
		program_space().install_rom(0xa000, 0xa7ff, m_rom->base());

		io_space().nop_read(0xf0, 0xf0); // TODO: UM6845 Status Register (b7 must be high)
		io_space().install_write_handler(0xf0, 0xf0, emu::rw_delegate(*m_crtc, FUNC(mc6845_device::address_w)));
		io_space().install_readwrite_handler(0xf1, 0xf1, emu::rw_delegate(*m_crtc, FUNC(mc6845_device::register_r)), emu::rw_delegate(*m_crtc, FUNC(mc6845_device::register_w)));
	}

private:
	required_memory_region m_rom;
	required_memory_region m_chargen;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;

	MC6845_UPDATE_ROW(crtc_update_row);

	std::unique_ptr<uint8_t[]> m_videoram;
};


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

MC6845_UPDATE_ROW(mpf_vid_device::crtc_update_row)
{
	uint32_t *p = &bitmap.pix(y);

	for (int column = 0; column < x_count; column++)
	{
		uint8_t code = m_videoram[(ma + column) & 0x7ff];
		uint16_t addr = (code << 4) | (ra & 0x0f);
		uint8_t data = m_chargen->base()[addr & 0x7ff];

		if (column == cursor_x)
		{
			data = 0xff;
		}

		for (int bit = 0; bit < 8; bit++)
		{
			*p++ = m_palette->pen(BIT(code, 7) ? !BIT(data, 7) : BIT(data, 7));
			data <<= 1;
		}
	}
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(MPF_VID, device_mpf1_exp_interface, mpf_vid_device, "mpf1_vid", "Bardehle VIDEO-MPF-I (Video Board)")
