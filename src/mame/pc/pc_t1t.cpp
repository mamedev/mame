// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    IBM PCjr and Tandy 1000 graphics emulation

    Note that in the IBM PC Junior world, the term VGA refers to the Video
    Gate Array, which is unrelated to the more well-known Video Graphics
    Array introduced with the PS/2.

    The Tandy 1000 used an actual 6845 CRT controller, while later models
    integrated equivalent functionality into custom chips.
    * The Tandy 1000A integrates the functionality into the 8079010 Custom
      Address Array.
    * The Tandy 1000 SX and Tandy 1000 HX integrate the functionality into
      the BIGBLUE video array.

    TODO:
    * Extended addressing for Tandy 1000 machines supporting more than
      128K VRAM.
    * Extended page register for later Tandy 1000 machines.
    * Tandy 1000 640*200 secondary pixel organisation.
    * Composite output colours.
    * Correct output colours for monochrome modes.
    * Border colours.
    * Confirm whether colours are translated via the palette registers for
      Tandy 1000 CGA compatibility features.

***************************************************************************/

#include "emu.h"
#include "pc_t1t.h"

#include "screen.h"

#include <algorithm>


namespace {

enum
{
	T1000_TEXT_INTEN = 0,
	T1000_TEXT_BLINK,
	T1000_GFX_1BPP,
	T1000_GFX_2BPP,
	T1000_GFX_2BPP_HIGH,
	T1000_GFX_4BPP,
	PCJX_TEXT
};


ROM_START(pcvideo_t1000)
	// ROM dump from a 16-bit model with external character ROM marked:
	// 8079027
	// NCR
	// 609-2495004
	// F841030 A9025
	ROM_REGION(0x8000, "chrgen", 0)
	ROM_LOAD("video-array.chr", 0x0000, 0x4000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450))
ROM_END

ROM_START(pcvideo_t1000x)
	// ROM dump from a 16-bit model with external character ROM marked:
	// 8079027
	// NCR
	// 609-2495004
	// F841030 A9025
	ROM_REGION(0x8000, "chrgen", 0)
	ROM_LOAD("bigblue.chr", 0x0000, 0x4000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450))
ROM_END


static gfx_layout const t1000_charlayout =
{
	8, 16,
	256,
	1,
	{ 0 },
	{ STEP8(0, 1) },
	{ STEP16(0, 2048) },
	8
};

static GFXDECODE_START(gfx_t1000)
	GFXDECODE_DEVICE("chrgen", 0x0000, t1000_charlayout, 3, 1)
GFXDECODE_END

} // anonymous namespace


DEFINE_DEVICE_TYPE(PCVIDEO_T1000,  pcvideo_t1000_device,  "tandy_1000_graphics",  "Tandy 1000 built-in video (VIDEO-ARRAY)")
DEFINE_DEVICE_TYPE(PCVIDEO_T1000X, pcvideo_t1000x_device, "tandy_1000x_graphics", "Tandy 1000 X built-in videeo (BIGBLUE)")
DEFINE_DEVICE_TYPE(PCVIDEO_PCJR,   pcvideo_pcjr_device,   "pcjr_graphics",        "IBM PCjr built-in video")


pc_t1t_device::pc_t1t_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		uint32_t clock,
		uint8_t databits,
		uint8_t addrbits,
		unsigned chr_stride,
		unsigned ra_stride) :
	device_t(mconfig, type, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_palette_interface(mconfig, *this),
	device_memory_interface(mconfig, *this),
	m_vram_config("vram", ENDIANNESS_LITTLE, databits, addrbits, 0, address_map_constructor(FUNC(pc_t1t_device::default_map), this)),
	m_chr_gen(*this, finder_base::DUMMY_TAG),
	m_mc6845(*this, "mc6845_t1000"),
	m_chr_stride(chr_stride),
	m_ra_stride(ra_stride),
	m_display_base(0),
	m_window_base(0),
	m_base_mask(0),
	m_ra_mask(0),
	m_offset_mask(0),
	m_ra_shift(0),
	m_status(0),
	m_vga_addr(0),
	m_palette_mask(0),
	m_border_color(0),
	m_page(0),
	m_pc_framecnt(0),
	m_update_row_type(-1),
	m_display_enable(0),
	m_vsync(0),
	m_palette_base(0)
{
	std::fill(std::begin(m_palette_reg), std::end(m_palette_reg), 0);
}

pcvideo_t1000_device::pcvideo_t1000_device(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		uint32_t clock) :
	pcvideo_t1000_device(mconfig, PCVIDEO_T1000, tag, owner, clock, 8, 17)
{
}

pcvideo_t1000_device::pcvideo_t1000_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		uint32_t clock,
		uint8_t databits,
		uint8_t addrbits) :
	pc_t1t_device(mconfig, type, tag, owner, clock, databits, addrbits, 0, 8),
	device_gfx_interface(mconfig, *this, gfx_t1000, DEVICE_SELF),
	m_mode_sel(0),
	m_color_sel(0),
	m_mode_ctrl(0),
	m_disable(false)
{
}

pcvideo_t1000x_device::pcvideo_t1000x_device(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		uint32_t clock) :
	pcvideo_t1000x_device(mconfig, PCVIDEO_T1000X, tag, owner, clock, 16, 18)
{
}

pcvideo_t1000x_device::pcvideo_t1000x_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		uint32_t clock,
		uint8_t databits,
		uint8_t addrbits) :
	pcvideo_t1000_device(mconfig, type, tag, owner, clock, databits, addrbits),
	m_ext_page(0)
{
}

pcvideo_pcjr_device::pcvideo_pcjr_device(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		uint32_t clock) :
	pc_t1t_device(mconfig, PCVIDEO_PCJR, tag, owner, clock, 8, 17, 3, 0),
	m_jxkanji(*this, finder_base::DUMMY_TAG),
	m_vsync_cb(*this),
	m_address_data_ff(0),
	m_mode_ctrl_1(0),
	m_mode_ctrl_2(0)
{
}


void pc_t1t_device::device_start()
{
	// Initialise the CGA palette
	static constexpr rgb_t tga_palette[16] =
	{
		{ 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0xaa }, { 0x00, 0xaa, 0x00 }, { 0x00, 0xaa, 0xaa },
		{ 0xaa, 0x00, 0x00 }, { 0xaa, 0x00, 0xaa }, { 0xaa, 0x55, 0x00 }, { 0xaa, 0xaa, 0xaa },
		{ 0x55, 0x55, 0x55 }, { 0x55, 0x55, 0xff }, { 0x55, 0xff, 0x55 }, { 0x55, 0xff, 0xff },
		{ 0xff, 0x55, 0x55 }, { 0xff, 0x55, 0xff }, { 0xff, 0xff, 0x55 }, { 0xff, 0xff, 0xff }
	};

	// colors
	for (int i = 0; i < 16; i++)
		set_indirect_color(i, tga_palette[i]);

	// b/w mode shades
	for (int i = 0; i < 16; i++)
		set_indirect_color(16 + i, rgb_t(pal4bit(i), pal4bit(i), pal4bit(i)));

	for (int i = 0; i < 16; i++)
		set_pen_indirect(i, 0);

	save_item(NAME(m_vga_addr));
	save_item(NAME(m_palette_mask));
	save_item(NAME(m_border_color));
	save_item(NAME(m_palette_reg));
	save_item(NAME(m_page));
}

void pcvideo_t1000_device::device_start()
{
	pc_t1t_device::device_start();

	save_item(NAME(m_mode_sel));
	save_item(NAME(m_color_sel));
	save_item(NAME(m_mode_ctrl));
	save_item(NAME(m_disable));
}

void pcvideo_t1000x_device::device_start()
{
	pcvideo_t1000_device::device_start();

	save_item(NAME(m_ext_page));
}

void pcvideo_pcjr_device::device_start()
{
	pc_t1t_device::device_start();

	space(0).cache(m_vram);

	if ((m_jxkanji.finder_tag() != finder_base::DUMMY_TAG) && !m_jxkanji)
		throw emu_fatalerror("%s: kanji ROM region %s configured but not found", tag(), m_jxkanji.finder_tag());

	save_item(NAME(m_address_data_ff));
	save_item(NAME(m_mode_ctrl_1));
	save_item(NAME(m_mode_ctrl_2));

	page_w(0);
}


void pcvideo_t1000x_device::device_reset()
{
	pcvideo_t1000_device::device_reset();

	ext_page_w(0);
}


void pc_t1t_device::device_post_load()
{
	// reconstruct VRAM addressing parameters
	page_w(m_page);
}

void pcvideo_t1000_device::device_post_load()
{
	pc_t1t_device::device_post_load();

	// ensure correct video update function is selected;
	mode_switch();
}

void pcvideo_t1000x_device::device_post_load()
{
	pcvideo_t1000_device::device_post_load();

	// patch up extended VRAM addressing
	page_w(m_page);
}

void pcvideo_pcjr_device::device_post_load()
{
	pc_t1t_device::device_post_load();

	// ensure correct video update function is selected;
	pc_pcjr_mode_switch();
}


device_memory_interface::space_config_vector pc_t1t_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(0, &m_vram_config) };
}


void pc_t1t_device::default_map(address_map &map)
{
	map.unmap_value_high();
}


/***************************************************************************

    Static declarations

***************************************************************************/

tiny_rom_entry const *pcvideo_t1000_device::device_rom_region() const
{
	return ROM_NAME(pcvideo_t1000);
}


tiny_rom_entry const *pcvideo_t1000x_device::device_rom_region() const
{
	return ROM_NAME(pcvideo_t1000x);
}


void pcvideo_t1000_device::device_add_mconfig(machine_config &config)
{
	m_chr_gen.set_tag("chrgen");

	screen_device &screen(SCREEN(config, T1000_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(14'318'181),912,0,640,262,0,200);
	screen.set_screen_update(m_mc6845, FUNC(mc6845_device::screen_update));

	MC6845(config, m_mc6845, XTAL(14'318'181) / 16);
	m_mc6845->set_screen(T1000_SCREEN_NAME);
	m_mc6845->set_show_border_area(false);
	m_mc6845->set_char_width(8);
	m_mc6845->set_update_row_callback(FUNC(pcvideo_t1000_device::crtc_update_row));
	m_mc6845->out_de_callback().set(FUNC(pcvideo_t1000_device::de_changed));
	m_mc6845->out_vsync_callback().set(FUNC(pcvideo_t1000_device::vsync_changed));
}


void pcvideo_pcjr_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, T1000_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(14'318'181), 912, 0, 640, 262, 0, 200);
	screen.set_screen_update(m_mc6845, FUNC(mc6845_device::screen_update));

	MC6845(config, m_mc6845, XTAL(14'318'181) / 16);
	m_mc6845->set_screen(T1000_SCREEN_NAME);
	m_mc6845->set_show_border_area(false);
	m_mc6845->set_char_width(8);
	m_mc6845->set_update_row_callback(FUNC(pcvideo_pcjr_device::crtc_update_row));
	m_mc6845->out_de_callback().set(FUNC(pcvideo_pcjr_device::de_changed));
	m_mc6845->out_vsync_callback().set(FUNC(pcvideo_pcjr_device::pcjr_vsync_changed));
}


/***************************************************************************

    Methods

***************************************************************************/

MC6845_UPDATE_ROW( pc_t1t_device::text_inten_update_row )
{
	uint32_t const rowbase = row_base(ra);
	uint32_t *p = &bitmap.pix(y);

	if (y == 0) logerror("t1000_text_inten_update_row\n");
	for (int i = 0; i < x_count; i++)
	{
		uint16_t const offset = ((ma + i) << 1) & m_offset_mask;
		uint16_t const vram = space(0).read_word(rowbase | offset);
		uint8_t const chr = vram & 0x00ff;
		uint8_t const attr = vram >> 8;
		auto const fg = palette_r(BIT(attr, 0, 4));
		auto const bg = palette_r(BIT(attr, 4, 4));

		uint8_t data = chr_gen_r(chr, ra);
		if (i == cursor_x && (m_pc_framecnt & 0x08))
			data = 0xff;

		*p++ = BIT(data, 7) ? fg : bg;
		*p++ = BIT(data, 6) ? fg : bg;
		*p++ = BIT(data, 5) ? fg : bg;
		*p++ = BIT(data, 4) ? fg : bg;
		*p++ = BIT(data, 3) ? fg : bg;
		*p++ = BIT(data, 2) ? fg : bg;
		*p++ = BIT(data, 1) ? fg : bg;
		*p++ = BIT(data, 0) ? fg : bg;
	}
}


MC6845_UPDATE_ROW( pcvideo_t1000_device::text_blink_update_row )
{
	uint32_t const rowbase = row_base(ra);
	uint32_t *p = &bitmap.pix(y);

	for (int i = 0; i < x_count; i++)
	{
		// Documentation says colour select register bit 4 sets the background
		// intensity, but the BIOS sets this bit, resulting in a grey background
		// in DOS (rather than black).  CGA documentation says the same thing,
		// but according to the schematics, it doesn't actually use it, either.
		uint16_t const offset = ((ma + i) << 1) & m_offset_mask;
		uint16_t const vram = space(0).read_word(rowbase | offset);
		uint8_t const chr = vram & 0x00ff;
		uint8_t const attr = vram >> 8;
		auto const fg = palette_r(BIT(attr, 0, 4));
		//auto const bg = palette_r(BIT(attr, 4, 3) | (BIT(m_color_sel, 4) << 3));
		auto const bg = palette_r(BIT(attr, 4, 3));

		uint8_t data = chr_gen_r(chr, ra);
		if (i == cursor_x)
		{
			if (m_pc_framecnt & 0x08)
				data = 0xff;
		}
		else
		{
			if ((attr & 0x80) && (m_pc_framecnt & 0x10))
				data = 0x00;
		}

		*p++ = BIT(data, 7) ? fg : bg;
		*p++ = BIT(data, 6) ? fg : bg;
		*p++ = BIT(data, 5) ? fg : bg;
		*p++ = BIT(data, 4) ? fg : bg;
		*p++ = BIT(data, 3) ? fg : bg;
		*p++ = BIT(data, 2) ? fg : bg;
		*p++ = BIT(data, 1) ? fg : bg;
		*p++ = BIT(data, 0) ? fg : bg;
	}
}

MC6845_UPDATE_ROW( pcvideo_pcjr_device::text_blink_update_row )
{
	uint32_t const rowbase = row_base(ra);
	uint32_t *p = &bitmap.pix(y);

	for (int i = 0; i < x_count; i++)
	{
		uint16_t const offset = ((ma + i) << 1) & m_offset_mask;
		uint16_t const vram = space(0).read_word(rowbase | offset);
		uint8_t const chr = vram & 0x00ff;
		uint8_t const attr = vram >> 8;
		auto const fg = palette_r(BIT(attr, 0, 4));
		auto const bg = palette_r(BIT(attr, 4, 3));

		uint8_t data = chr_gen_r(chr, ra);
		if (i == cursor_x)
		{
			if (m_pc_framecnt & 0x08)
				data = 0xff;
		}
		else
		{
			if ((attr & 0x80) && (m_pc_framecnt & 0x10))
				data = 0x00;
		}

		*p++ = BIT(data, 7) ? fg : bg;
		*p++ = BIT(data, 6) ? fg : bg;
		*p++ = BIT(data, 5) ? fg : bg;
		*p++ = BIT(data, 4) ? fg : bg;
		*p++ = BIT(data, 3) ? fg : bg;
		*p++ = BIT(data, 2) ? fg : bg;
		*p++ = BIT(data, 1) ? fg : bg;
		*p++ = BIT(data, 0) ? fg : bg;
	}
}


MC6845_UPDATE_ROW( pcvideo_pcjr_device::pcjx_text_update_row )
{
	uint32_t const rowbase = row_base(ra);
	uint32_t *p = &bitmap.pix(y);

	for (int i = 0; i < x_count; i++)
	{
		uint16_t const offset = ((ma + i) << 1) & 0x3fff;
		uint8_t chr = space(0).read_byte(rowbase + offset);
		uint8_t const attr = space(0).read_byte(rowbase + offset + 1);
		auto const fg = palette_r(BIT(attr, 0, 3));
		auto const bg = palette_r(BIT(attr, 4, 3));
		uint16_t code = chr & 0x1f;
		if((attr & 0x88) == 0x88)
		{
			code = space(0).read_byte(rowbase + offset - 2) & 0x1f;
			code = (code << 8) + chr;
		}
		else if(attr & 0x80)
			code = (code << 8) + space(0).read_byte(rowbase + offset + 2);
		else
			code = chr;

		uint8_t data;
		if(ra < 16)
			data = m_jxkanji[code * 16 * 2 + (ra * 2) + ((attr & 8)?1:0)];
		else
			data = ((i == cursor_x) && (m_pc_framecnt & 8)) ? 0xff: 0;

		*p++ = BIT(data, 7) ? fg : bg;
		*p++ = BIT(data, 6) ? fg : bg;
		*p++ = BIT(data, 5) ? fg : bg;
		*p++ = BIT(data, 4) ? fg : bg;
		*p++ = BIT(data, 3) ? fg : bg;
		*p++ = BIT(data, 2) ? fg : bg;
		*p++ = BIT(data, 1) ? fg : bg;
		*p++ = BIT(data, 0) ? fg : bg;
	}
}

MC6845_UPDATE_ROW( pc_t1t_device::gfx_4bpp_update_row )
{
	uint32_t *p = &bitmap.pix(y);
	uint32_t const rowbase = row_base(ra);

	for (int i = 0; i < x_count; i++)
	{
		uint16_t const offset = ((ma + i) << 1) & m_offset_mask;
		uint16_t const data = space(0).read_word(rowbase | offset);

		*p++ = palette_r(BIT(data,  4, 4));
		*p++ = palette_r(BIT(data,  0, 4));
		*p++ = palette_r(BIT(data, 12, 4));
		*p++ = palette_r(BIT(data,  8, 4));
	}
}


MC6845_UPDATE_ROW( pcvideo_t1000_device::gfx_2bpp_update_row )
{
	// Unlike the PCjr, the Tandy 1000 ignores the VRAM addressing configuration
	// in this mode.
	uint32_t *p = &bitmap.pix(y);
	uint32_t const rowbase = m_display_base | ((uint32_t(ra) << 13) & 0x02000);

	for (int i = 0; i < x_count; i++)
	{
		uint16_t const offset = ((ma + i) << 1) & 0x01fff;
		uint16_t const data = space(0).read_word(rowbase | offset);

		*p++ = palette_cga_2bpp_r(BIT(data,  6, 2));
		*p++ = palette_cga_2bpp_r(BIT(data,  4, 2));
		*p++ = palette_cga_2bpp_r(BIT(data,  2, 2));
		*p++ = palette_cga_2bpp_r(BIT(data,  0, 2));
		*p++ = palette_cga_2bpp_r(BIT(data, 14, 2));
		*p++ = palette_cga_2bpp_r(BIT(data, 12, 2));
		*p++ = palette_cga_2bpp_r(BIT(data, 10, 2));
		*p++ = palette_cga_2bpp_r(BIT(data,  8, 2));
	}
}

MC6845_UPDATE_ROW( pcvideo_pcjr_device::gfx_2bpp_update_row )
{
	uint32_t *p = &bitmap.pix(y);
	uint32_t const rowbase = row_base(ra);

	for (int i = 0; i < x_count; i++)
	{
		uint16_t const offset = ((ma + i) << 1) & m_offset_mask;
		uint16_t const data = space(0).read_word(rowbase | offset);

		*p++ = palette_r(BIT(data,  6, 2));
		*p++ = palette_r(BIT(data,  4, 2));
		*p++ = palette_r(BIT(data,  2, 2));
		*p++ = palette_r(BIT(data,  0, 2));
		*p++ = palette_r(BIT(data, 14, 2));
		*p++ = palette_r(BIT(data, 12, 2));
		*p++ = palette_r(BIT(data, 10, 2));
		*p++ = palette_r(BIT(data,  8, 2));
	}
}


MC6845_UPDATE_ROW( pc_t1t_device::gfx_2bpp_high_update_row )
{
	uint32_t *p = &bitmap.pix(y);
	uint32_t const rowbase = row_base(ra);

	for (int i = 0; i < x_count; i++)
	{
		uint16_t const offset = ((ma + i) << 1) & m_offset_mask;
		uint16_t const data = space(0).read_word(rowbase | offset);

		*p++ = palette_r(bitswap<2>(data, 15, 7));
		*p++ = palette_r(bitswap<2>(data, 14, 6));
		*p++ = palette_r(bitswap<2>(data, 13, 5));
		*p++ = palette_r(bitswap<2>(data, 12, 4));
		*p++ = palette_r(bitswap<2>(data, 11, 3));
		*p++ = palette_r(bitswap<2>(data, 10, 2));
		*p++ = palette_r(bitswap<2>(data,  9, 1));
		*p++ = palette_r(bitswap<2>(data,  8, 0));
	}
}


MC6845_UPDATE_ROW( pcvideo_t1000_device::gfx_1bpp_update_row )
{
	// Unlike the PCjr, the Tandy 1000 ignores the VRAM addressing configuration
	// in this mode.
	uint32_t *p = &bitmap.pix(y);
	uint32_t const rowbase = m_display_base | ((uint32_t(ra) << 13) & 0x02000);
	auto const fg = palette_r(BIT(m_color_sel, 0, 4));
	auto const bg = palette_r(0);

	for (int i = 0; i < x_count; i++)
	{
		uint16_t const offset = ((ma + i) << 1) & 0x01fff;
		uint16_t const data = space(0).read_word(rowbase | offset);

		*p++ = BIT(data,  7) ? fg : bg;
		*p++ = BIT(data,  6) ? fg : bg;
		*p++ = BIT(data,  5) ? fg : bg;
		*p++ = BIT(data,  4) ? fg : bg;
		*p++ = BIT(data,  3) ? fg : bg;
		*p++ = BIT(data,  2) ? fg : bg;
		*p++ = BIT(data,  1) ? fg : bg;
		*p++ = BIT(data,  0) ? fg : bg;
		*p++ = BIT(data, 15) ? fg : bg;
		*p++ = BIT(data, 14) ? fg : bg;
		*p++ = BIT(data, 13) ? fg : bg;
		*p++ = BIT(data, 12) ? fg : bg;
		*p++ = BIT(data, 11) ? fg : bg;
		*p++ = BIT(data, 10) ? fg : bg;
		*p++ = BIT(data,  9) ? fg : bg;
		*p++ = BIT(data,  8) ? fg : bg;
	}
}

MC6845_UPDATE_ROW( pcvideo_pcjr_device::gfx_1bpp_update_row )
{
	uint32_t *p = &bitmap.pix(y);
	uint32_t const rowbase = row_base(ra);
	auto const fg = palette_r(1);
	auto const bg = palette_r(0);

	for (int i = 0; i < x_count; i++)
	{
		uint16_t const offset = ((ma + i) << 1) & m_offset_mask;
		uint16_t const data = space(0).read_word(rowbase | offset);

		*p++ = BIT(data,  7) ? fg : bg;
		*p++ = BIT(data,  6) ? fg : bg;
		*p++ = BIT(data,  5) ? fg : bg;
		*p++ = BIT(data,  4) ? fg : bg;
		*p++ = BIT(data,  3) ? fg : bg;
		*p++ = BIT(data,  2) ? fg : bg;
		*p++ = BIT(data,  1) ? fg : bg;
		*p++ = BIT(data,  0) ? fg : bg;
		*p++ = BIT(data, 15) ? fg : bg;
		*p++ = BIT(data, 14) ? fg : bg;
		*p++ = BIT(data, 13) ? fg : bg;
		*p++ = BIT(data, 12) ? fg : bg;
		*p++ = BIT(data, 11) ? fg : bg;
		*p++ = BIT(data, 10) ? fg : bg;
		*p++ = BIT(data,  9) ? fg : bg;
		*p++ = BIT(data,  8) ? fg : bg;
	}
}


MC6845_UPDATE_ROW( pc_t1t_device::crtc_update_row )
{
	switch (m_update_row_type)
	{
		case T1000_TEXT_INTEN:
			text_inten_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case T1000_GFX_2BPP_HIGH:
			gfx_2bpp_high_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case T1000_GFX_4BPP:
			gfx_4bpp_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
	}
}

MC6845_UPDATE_ROW( pcvideo_t1000_device::crtc_update_row )
{
	switch (m_update_row_type)
	{
		case T1000_TEXT_BLINK:
			text_blink_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case T1000_GFX_1BPP:
			gfx_1bpp_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case T1000_GFX_2BPP:
			gfx_2bpp_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		default:
			pc_t1t_device::crtc_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
	}
}

MC6845_UPDATE_ROW( pcvideo_pcjr_device::crtc_update_row )
{
	switch (m_update_row_type)
	{
		case T1000_TEXT_BLINK:
			text_blink_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case T1000_GFX_1BPP:
			gfx_1bpp_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case T1000_GFX_2BPP:
			gfx_2bpp_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case PCJX_TEXT:
			pcjx_text_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		default:
			pc_t1t_device::crtc_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
	}
}


void pc_t1t_device::set_palette_base(uint8_t base)
{
	if (base != m_palette_base)
	{
		m_palette_base = base;
		for (unsigned i = 0; 16 > i; ++i)
			set_pen_indirect(i, base + m_palette_reg[i]);
	}
}


void pcvideo_t1000_device::mode_switch()
{
	bool const blink    = BIT(m_mode_sel, 5);
	bool const hresad   = BIT(m_mode_sel, 4);
	bool const videbcr  = BIT(m_mode_sel, 3);
	bool const grph     = BIT(m_mode_sel, 1);
	bool const hresck   = BIT(m_mode_sel, 0);
	bool const c16col   = BIT(m_mode_ctrl, 4);
	bool const c4colhr  = BIT(m_mode_ctrl, 3);

	if (!videbcr)
	{
		m_update_row_type = -1;
	}
	else if (!grph)
	{
		if (!blink)
			m_update_row_type = T1000_TEXT_INTEN;
		else
			m_update_row_type = T1000_TEXT_BLINK;
	}
	else if (!hresad)
	{
		if (!c16col)
			m_update_row_type = T1000_GFX_2BPP;
		else
			m_update_row_type = T1000_GFX_4BPP;
	}
	else
	{
		if (!c4colhr)
			m_update_row_type = T1000_GFX_1BPP;
		else
			m_update_row_type = T1000_GFX_2BPP_HIGH;
	}

	// determine MC6845 input clock
	if (hresck)
		m_mc6845->set_clock(XTAL(14'318'181) / 8);
	else
		m_mc6845->set_clock(XTAL(14'318'181) / 16);
	m_mc6845->set_hpixels_per_column(!grph ? 8 : (!hresad && c16col) ? 4 : (hresad && !c4colhr) ? 16 : 8);
}


// mode control 1 (VGA register 0)
// bit0 - 0 = low bandwidth, 1 = high bandwidth
// bit1 - 0 = alpha, 1 = graphics
// bit2 - 0 = color, 1 = b/w
// bit3 - 0 = video disable, 1 = video enable
// bit4 - 1 = 16 color graphics
// mode control 2 (VGA register 3)
// bit1 - 1 = enable blink
// bit3 - 1 = 2 color graphics

void pcvideo_pcjr_device::pc_pcjr_mode_switch()
{
	bool const high_bw  = BIT(m_mode_ctrl_1, 0);
	bool const graphics = BIT(m_mode_ctrl_1, 1);
	bool const mono     = BIT(m_mode_ctrl_1, 2);
	bool const enable   = BIT(m_mode_ctrl_1, 3);
	bool const color16  = BIT(m_mode_ctrl_1, 4);
	bool const blink    = BIT(m_mode_ctrl_2, 1);
	bool const color2   = BIT(m_mode_ctrl_2, 3);

	if (!enable)
	{
		m_update_row_type = -1;
	}
	else if (!graphics)
	{
		// text mode
		if (m_jxkanji)
			m_update_row_type = PCJX_TEXT;
		else if (blink)
			m_update_row_type = T1000_TEXT_BLINK;
		else
			m_update_row_type = T1000_TEXT_INTEN;
	}
	else if (color16)
	{
		m_update_row_type = T1000_GFX_4BPP;
	}
	else if (color2)
	{
		m_update_row_type = T1000_GFX_1BPP;
	}
	else if (high_bw)
	{
		m_update_row_type = T1000_GFX_2BPP_HIGH;
	}
	else
	{
		m_update_row_type = T1000_GFX_2BPP;
	}

	// determine MC6845 input clock
	if (high_bw)
		m_mc6845->set_clock(XTAL(14'318'181) / 8);
	else
		m_mc6845->set_clock(XTAL(14'318'181) / 16);
	m_mc6845->set_hpixels_per_column(!graphics ? 8 : color16 ? 4 : color2 ? 16 : 8);

	// color or b/w?
	set_palette_base(mono ? 16 : 0);
}


/*
 * 3d8 rW   T1T mode control register (see #P138)
 */
void pcvideo_t1000_device::mode_select_w(uint8_t data)
{
	m_mode_sel = data;

	mode_switch();
}


/*
 * 3d9 ?W   color select register on color adapter
 */
void pcvideo_t1000_device::color_select_w(uint8_t data)
{
	m_color_sel = data & 0x3f;
}


/*  Bitfields for T1T status register:
 *  Bit(s)  Description (Table P179)
 *  7-6 not used
 *  5-4 color EGA, color ET4000: diagnose video display feedback, select
 *      from color plane enable
 *  4   holds current dot being displayed
 *  3   in vertical retrace
 *  2   (CGA,color EGA) light pen switch is off
 *      (MCGA,color ET4000) reserved (0)
 *  1   (CGA,color EGA) positive edge from light pen has set trigger
 *      (MCGA,color ET4000) reserved (0)
 *  0   display enabled
 *      =0  do not use memory
 *      =1  memory access without interfering with display
 *      (Genoa SuperEGA) horizontal or vertical retrace
 */
int pc_t1t_device::status_r()
{
	int data = m_vsync | m_status | m_display_enable;
	/* HACK HACK HACK */
	data |= ( m_display_enable ? 0x10 : 0x00 );
	/* end HACK */
	return data;
}

/*
 * 3db -W   light pen strobe reset (on any value)
 */
void pc_t1t_device::lightpen_strobe_w(int data)
{
//  pc_port[0x3db] = data;
}


/*
 * 3da -W   (mono EGA/mono VGA) feature control register
 *          (see PORT 03DAh-W for details; VGA, see PORT 03CAh-R)
 */
void pc_t1t_device::vga_addr_w(uint8_t data)
{
	m_vga_addr = data & 0x1f;
}

void pc_t1t_device::vga_data_w(uint8_t data)
{
	if (BIT(m_vga_addr, 4))
	{
		m_palette_reg[m_vga_addr & 0x0f] = data & 0x0f;
		set_pen_indirect(m_vga_addr & 0x0f, m_palette_base + (data & 0x0f));
	}
	else switch (m_vga_addr & 0x0f)
	{
		case 0x01:
			m_palette_mask = data & 0x0f;
			break;
		case 0x2:
			m_border_color = data & 0x0f;
			break;
	}
}

void pcvideo_t1000_device::vga_data_w(uint8_t data)
{
	switch (m_vga_addr)
	{
		case 0x03:
			m_mode_ctrl = data & 0x1f;
			mode_switch();
			break;
		default:
			pc_t1t_device::vga_data_w(data);
	}
}

void pcvideo_pcjr_device::vga_data_w(uint8_t data)
{
	switch (m_vga_addr)
	{
		case 0x00:
			m_mode_ctrl_1 = data & 0x1f;
			pc_pcjr_mode_switch();
			break;
		case 0x03:
			m_mode_ctrl_2 = data & 0x0f;
			pc_pcjr_mode_switch();
			break;
		case 0x04:  // reset register
			break;
		default:
			pc_t1t_device::vga_data_w(data);
	}
}


/*
 * 03DF RW   display bank, access bank, mode
 * bit 0-2  Identifies the page of main memory being displayed in units of 16K.
 *          0: 0K, 1: 16K...7: 112K. In 32K modes (bits 6-7 = 2) only 0,2,4 and
 *          6 are valid, as the next page will also be used.
 *     3-5  Identifies the page of main memory that can be read/written at B8000h
 *          in units of 16K. 0: 0K, 1: 16K...7: 112K. In 32K modes (bits 6-7 = 2)
 *          only 0,2,4 and 6 are valid, as the next page will also be used.
 *     6-7  Display mode. 0: Text, 1: 16K graphics mode (4,5,6,8)
 *          2: 32K graphics mode (9,Ah)
 */
void pc_t1t_device::page_w(uint8_t data)
{
	m_display_base = uint32_t(BIT(data, 0, 3)) << 14;
	m_window_base = uint32_t(BIT(data, 3, 3)) << 14;

	m_ra_mask = uint32_t(BIT(data, 6, 2)) << 13;
	m_base_mask = (uint32_t(0x07) << 14) & ~m_ra_mask;
	m_offset_mask = 0x3fff & ~(m_base_mask | m_ra_mask);
	m_ra_shift = 13;

	m_page = data;
}

void pcvideo_t1000x_device::page_w(uint8_t data)
{
	pc_t1t_device::page_w(data);

	if (!BIT(m_ext_page, 0))
	{
		if (BIT(data, 6, 2) == 0x02)
			m_ra_shift = 14; // new feature - two 16K segments switched on RA bit 0
	}
	else
	{
		// new feature - 32K segment modes
		// TODO: what actually happens if bit 7 (ADRM1) is set?
		m_ra_mask = uint32_t(BIT(data, 6)) << 15;
		m_base_mask = (uint32_t(0x06) << 14) & ~m_ra_mask;
		m_offset_mask = 0x7fff;
		m_ra_shift = 15;
	}

	// extra page bit for 256K VRAM
	m_base_mask |= uint32_t(0x01) << 17;
}


void pcvideo_t1000x_device::ext_page_w(uint8_t data)
{
	// TODO: implement PG17 and VPG17 bits when 256K VRAM addressing is understood
	m_ext_page = data;

	page_w(m_page);
}


/*************************************************************************
 *
 *      T1T
 *      Tandy 1000 / PCjr
 *
 *************************************************************************/

uint8_t pcvideo_t1000_device::read(address_space &space, offs_t offset)
{
	uint8_t data = space.unmap();

	switch (offset)
	{
		case 0x01: case 0x03: case 0x05: case 0x07: // only 0x03D5 is documented, write-only according to Tandy 1000SX manual
			data = m_mc6845->register_r();
			break;

		case 0x0a:
			data = status_r();
			break;
	}
	return data;
}

uint8_t pcvideo_pcjr_device::read(address_space &space, offs_t offset)
{
	uint8_t data = space.unmap();

	switch (offset)
	{
		case 0x01: case 0x05:
			data = m_mc6845->register_r();
			break;

		case 0x0a:
			if (!machine().side_effects_disabled())
				m_address_data_ff = 0;
			data = status_r();
			break;
	}
	return data;
}


void pcvideo_t1000_device::write(offs_t offset, uint8_t data)
{
	switch (offset & 0x0f)
	{
		case 0x00: case 0x02: case 0x04: case 0x06: // only 03D4 is documented
			m_mc6845->address_w(data);
			break;
		case 0x01: case 0x03: case 0x05: case 0x07: // only 03D5 is documented
			m_mc6845->register_w(data);
			break;
		case 0x08:
			mode_select_w(data);
			break;
		case 0x09:
			color_select_w(data);
			break;
		case 0x0a:
			vga_addr_w(data);
			break;
		case 0x0b: // FIXME: clear light pen latch
			lightpen_strobe_w(data);
			break;
		case 0x0c: // FIXME: set light pen latch
			break;
		case 0x0e:
			vga_data_w(data);
			break;
		case 0x0f:
			page_w(data);
			break;
	}
}

void pcvideo_t1000x_device::write(offs_t offset, uint8_t data)
{
	switch (offset & 0x0f)
	{
		case 0x0d:
			ext_page_w(data);
			break;
		case 0x0f:
			page_w(data);
			break;
		default:
			pcvideo_t1000_device::write(offset, data);
	}
}

void pcvideo_pcjr_device::write(offs_t offset, uint8_t data)
{
	switch (offset & 0x0f)
	{
		case 0x00: case 0x04:
			m_mc6845->address_w(data);
			break;
		case 0x01: case 0x05:
			m_mc6845->register_w(data);
			break;
		case 0x0a:
			if (m_address_data_ff & 0x01)
				vga_data_w(data);
			else
				vga_addr_w(data);
			m_address_data_ff ^= 0x01;
			break;
		case 0x0b: // FIXME: clear light pen latch
			lightpen_strobe_w(data);
			break;
		case 0x0c: // FIXME: preset light pen latch
			break;
		case 0x0f:
			page_w(data);
			break;
	}
}


uint8_t pcvideo_t1000_device::vram_window8_r(address_space &space, offs_t offset)
{
	if (!m_disable)
		return this->space(0).read_byte(m_window_base | (offset & 0x7fff));
	else
		return space.unmap();
}

uint16_t pcvideo_t1000_device::vram_window16_r(address_space &space, offs_t offset)
{
	if (!m_disable)
		return this->space(0).read_word(m_window_base | ((offset << 1) & 0x7fff));
	else
		return space.unmap();
}

uint8_t pcvideo_pcjr_device::vram_window_r(offs_t offset)
{
	return m_vram.read_byte(m_window_base | (offset & 0x7fff));
}


void pcvideo_t1000_device::vram_window8_w(offs_t offset, uint8_t data)
{
	if (!m_disable)
		space(0).write_byte(m_window_base | (offset & 0x7fff), data);
}

void pcvideo_t1000_device::vram_window16_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (!m_disable)
		space(0).write_word(m_window_base | ((offset << 1) & 0x7fff), data, mem_mask);
}

void pcvideo_pcjr_device::vram_window_w(offs_t offset, uint8_t data)
{
	m_vram.write_byte(m_window_base | (offset & 0x7fff), data);
}


void pcvideo_t1000_device::de_changed(int state)
{
	m_display_enable = state ? 0 : 1;
}

void pcvideo_pcjr_device::de_changed(int state)
{
	m_display_enable = state ? 1 : 0;
}


void pcvideo_t1000_device::vsync_changed(int state)
{
	m_vsync = state ? 8 : 0;
	if (state)
	{
		m_pc_framecnt++;
	}
}

void pcvideo_t1000_device::disable_w(int state)
{
	m_disable = state ? true : false;
}

void pcvideo_pcjr_device::pcjr_vsync_changed(int state)
{
	m_vsync = state ? 8 : 0;
	if (state)
	{
		m_pc_framecnt++;
	}
	m_vsync_cb(state);
}
