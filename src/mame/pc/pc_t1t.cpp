// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    IBM PC junior
    Tandy 1000 Graphics Adapter (T1T) section

    Note that in the IBM PC Junior world, the term 'vga' is not the 'vga' that
    most people think of

***************************************************************************/

#include "emu.h"
#include "pc_t1t.h"

#include "screen.h"


enum
{
	T1000_TEXT_INTEN = 0,
	T1000_TEXT_BLINK,
	T1000_GFX_1BPP,
	T1000_GFX_2BPP,
	T1000_GFX_4BPP,
	T1000_GFX_2BPP_TGA,
	PCJX_TEXT,
	PCJR_GFX_2BPP_HIGH
};


DEFINE_DEVICE_TYPE(PCVIDEO_T1000, pcvideo_t1000_device, "tandy_1000_graphics", "Tandy 1000 Graphics Adapter")
DEFINE_DEVICE_TYPE(PCVIDEO_PCJR,  pcvideo_pcjr_device,  "pcjr_graphics",       "PC Jr Graphics Adapter")


pc_t1t_device::pc_t1t_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		uint32_t clock,
		uint8_t addrbits) :
	device_t(mconfig, type, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_palette_interface(mconfig, *this),
	device_memory_interface(mconfig, *this),
	m_vram_config("vram", ENDIANNESS_LITTLE, 8, addrbits, 0, address_map_constructor(FUNC(pc_t1t_device::default_map), this)),
	m_chr_gen(*this, finder_base::DUMMY_TAG),
	m_mc6845(*this, "mc6845_t1000"),
	m_display_base(0),
	m_window_base(0),
	m_mode_control(0),
	m_color_select(0),
	m_status(0),
	m_bank(0),
	m_pc_framecnt(0),
	m_chr_size(0),
	m_ra_offset(0),
	m_address_data_ff(0),
	m_update_row_type(-1),
	m_display_enable(0),
	m_vsync(0),
	m_palette_base(0)
{
}

pcvideo_t1000_device::pcvideo_t1000_device(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		uint32_t clock) :
	pcvideo_t1000_device(mconfig, PCVIDEO_T1000, tag, owner, clock, 17)
{
}

pcvideo_t1000_device::pcvideo_t1000_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		uint32_t clock,
		uint8_t addrbits) :
	pc_t1t_device(mconfig, type, tag, owner, clock, addrbits),
	m_disable(false)
{
}

pcvideo_pcjr_device::pcvideo_pcjr_device(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		uint32_t clock) :
	pc_t1t_device(mconfig, PCVIDEO_PCJR, tag, owner, clock, 17),
	m_vsync_cb(*this),
	m_jxkanji(nullptr)
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
		set_pen_color(i, tga_palette[i]);

	// b/w mode shades
	for (int i = 0; i < 16; i++)
		set_pen_color(16 + i, pal4bit(i), pal4bit(i), pal4bit(i));
}

void pcvideo_t1000_device::device_start()
{
	pc_t1t_device::device_start();

	m_bank = 0;
	m_chr_size = 1;
	m_ra_offset = 256;
}

void pcvideo_pcjr_device::device_start()
{
	pc_t1t_device::device_start();

	space(0).cache(m_vram);

	m_bank = 0;
	m_mode_control = 0x08;
	m_chr_size = 8;
	m_ra_offset = 1;
	if(!strncmp(machine().system().name, "ibmpcjx", 7))
		m_jxkanji = machine().root_device().memregion("kanji")->base();
	else
		m_jxkanji = nullptr;
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

void pcvideo_t1000_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, T1000_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(14'318'181),912,0,640,262,0,200);
	screen.set_screen_update(m_mc6845, FUNC(mc6845_device::screen_update));

	MC6845(config, m_mc6845, XTAL(14'318'181)/8);
	m_mc6845->set_screen(T1000_SCREEN_NAME);
	m_mc6845->set_show_border_area(false);
	m_mc6845->set_char_width(8);
	m_mc6845->set_update_row_callback(FUNC(pc_t1t_device::crtc_update_row));
	m_mc6845->out_de_callback().set(FUNC(pc_t1t_device::t1000_de_changed));
	m_mc6845->out_vsync_callback().set(FUNC(pcvideo_t1000_device::t1000_vsync_changed));
}


void pcvideo_pcjr_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, T1000_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(14'318'181), 912, 0, 640, 262, 0, 200);
	screen.set_screen_update(m_mc6845, FUNC(mc6845_device::screen_update));

	MC6845(config, m_mc6845, XTAL(14'318'181)/16);
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

MC6845_UPDATE_ROW( pc_t1t_device::t1000_text_inten_update_row )
{
	rgb_t const *const pal = palette()->entry_list_raw();
	uint32_t *p = &bitmap.pix(y);

	if (y == 0) logerror("t1000_text_inten_update_row\n");
	for (int i = 0; i < x_count; i++)
	{
		uint16_t const offset = ((ma + i) << 1) & 0x3fff;
		uint16_t const vram = space(0).read_word(m_display_base | offset);
		uint8_t const chr = vram & 0x00ff;
		uint8_t const attr = vram >> 8;
		uint16_t const fg = m_palette_base + (attr & 0x0f);
		uint16_t const bg = m_palette_base + ((attr >> 4) & 0x07);

		uint8_t data = m_chr_gen[chr * m_chr_size + ra * m_ra_offset];
		if (i == cursor_x && (m_pc_framecnt & 0x08))
			data = 0xff;

		*p++ = pal[BIT(data, 7) ? fg : bg];
		*p++ = pal[BIT(data, 6) ? fg : bg];
		*p++ = pal[BIT(data, 5) ? fg : bg];
		*p++ = pal[BIT(data, 4) ? fg : bg];
		*p++ = pal[BIT(data, 3) ? fg : bg];
		*p++ = pal[BIT(data, 2) ? fg : bg];
		*p++ = pal[BIT(data, 1) ? fg : bg];
		*p++ = pal[BIT(data, 0) ? fg : bg];
	}
}


MC6845_UPDATE_ROW( pc_t1t_device::t1000_text_blink_update_row )
{
	rgb_t const *const pal = palette()->entry_list_raw();
	uint32_t *p = &bitmap.pix(y);

	for (int i = 0; i < x_count; i++)
	{
		uint16_t const offset = ((ma + i) << 1) & 0x3fff;
		uint16_t const vram = space(0).read_word(m_display_base | offset);
		uint8_t const chr = vram & 0x00ff;
		uint8_t const attr = vram >> 8;
		uint16_t const fg = m_palette_base + (attr & 0x0f);
		uint16_t const bg = m_palette_base + ((attr >> 4) & 0x07);

		uint8_t data = m_chr_gen[chr * m_chr_size + ra * m_ra_offset];
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

		*p++ = pal[BIT(data, 7) ? fg : bg];
		*p++ = pal[BIT(data, 6) ? fg : bg];
		*p++ = pal[BIT(data, 5) ? fg : bg];
		*p++ = pal[BIT(data, 4) ? fg : bg];
		*p++ = pal[BIT(data, 3) ? fg : bg];
		*p++ = pal[BIT(data, 2) ? fg : bg];
		*p++ = pal[BIT(data, 1) ? fg : bg];
		*p++ = pal[BIT(data, 0) ? fg : bg];
	}
}

MC6845_UPDATE_ROW( pcvideo_pcjr_device::pcjx_text_update_row )
{
	rgb_t const *const pal = palette()->entry_list_raw();
	uint32_t *p = &bitmap.pix(y);

	for (int i = 0; i < x_count; i++)
	{
		uint16_t const offset = ((ma + i) << 1) & 0x3fff;
		uint8_t chr = space(0).read_byte(m_display_base + offset);
		uint8_t const attr = space(0).read_byte(m_display_base + offset + 1);
		uint16_t const fg = m_palette_base + (attr & 0x07);
		uint16_t const bg = m_palette_base + ((attr >> 4) & 0x07);
		uint16_t code = chr & 0x1f;
		if((attr & 0x88) == 0x88)
		{
			code = space(0).read_byte(m_display_base + offset - 2) & 0x1f;
			code = (code << 8) + chr;
		}
		else if(attr & 0x80)
			code = (code << 8) + space(0).read_byte(m_display_base + offset + 2);
		else
			code = chr;

		uint8_t data;
		if(ra < 16)
			data = m_jxkanji[code * 16 * 2 + (ra * 2) + ((attr & 8)?1:0)];
		else
			data = ((i == cursor_x) && (m_pc_framecnt & 8)) ? 0xff: 0;

		*p++ = pal[BIT(data, 7) ? fg : bg];
		*p++ = pal[BIT(data, 6) ? fg : bg];
		*p++ = pal[BIT(data, 5) ? fg : bg];
		*p++ = pal[BIT(data, 4) ? fg : bg];
		*p++ = pal[BIT(data, 3) ? fg : bg];
		*p++ = pal[BIT(data, 2) ? fg : bg];
		*p++ = pal[BIT(data, 1) ? fg : bg];
		*p++ = pal[BIT(data, 0) ? fg : bg];
	}
}

MC6845_UPDATE_ROW( pc_t1t_device::t1000_gfx_4bpp_update_row )
{
	rgb_t const *const pal = palette()->entry_list_raw();
	uint32_t *p = &bitmap.pix(y);
	uint32_t const rowbase = m_display_base | ((ra << 13) & 0x7fff);

	for (int i = 0; i < x_count; i++)
	{
		uint16_t const offset = ((ma + i) << 1) & 0x1fff;
		uint16_t const data = space(0).read_word(rowbase | offset);

		*p++ = pal[m_palette_base + m_reg.data[0x10 | BIT(data,  4, 4)]];
		*p++ = pal[m_palette_base + m_reg.data[0x10 | BIT(data,  4, 4)]];
		*p++ = pal[m_palette_base + m_reg.data[0x10 | BIT(data,  0, 4)]];
		*p++ = pal[m_palette_base + m_reg.data[0x10 | BIT(data,  0, 4)]];
		*p++ = pal[m_palette_base + m_reg.data[0x10 | BIT(data, 12, 4)]];
		*p++ = pal[m_palette_base + m_reg.data[0x10 | BIT(data, 12, 4)]];
		*p++ = pal[m_palette_base + m_reg.data[0x10 | BIT(data,  8, 4)]];
		*p++ = pal[m_palette_base + m_reg.data[0x10 | BIT(data,  8, 4)]];
	}
}


MC6845_UPDATE_ROW( pc_t1t_device::t1000_gfx_2bpp_update_row )
{
	rgb_t const *const pal = palette()->entry_list_raw();
	uint32_t *p = &bitmap.pix(y);
	uint32_t const rowbase = m_display_base | ((ra << 13) & 0x7fff);

	for (int i = 0; i < x_count; i++)
	{
		uint16_t const offset = ((ma + i) << 1) & 0x1fff;
		uint16_t const data = space(0).read_word(rowbase | offset);

		*p++ = pal[m_palette_base + m_reg.data[0x10 | BIT(data,  6, 2)]];
		*p++ = pal[m_palette_base + m_reg.data[0x10 | BIT(data,  4, 2)]];
		*p++ = pal[m_palette_base + m_reg.data[0x10 | BIT(data,  2, 2)]];
		*p++ = pal[m_palette_base + m_reg.data[0x10 | BIT(data,  0, 2)]];
		*p++ = pal[m_palette_base + m_reg.data[0x10 | BIT(data, 14, 2)]];
		*p++ = pal[m_palette_base + m_reg.data[0x10 | BIT(data, 12, 2)]];
		*p++ = pal[m_palette_base + m_reg.data[0x10 | BIT(data, 10, 2)]];
		*p++ = pal[m_palette_base + m_reg.data[0x10 | BIT(data,  8, 2)]];
	}
}


MC6845_UPDATE_ROW( pcvideo_pcjr_device::pcjr_gfx_2bpp_high_update_row )
{
	rgb_t const *const pal = palette()->entry_list_raw();
	uint32_t *p = &bitmap.pix(y);
	uint32_t const rowbase = m_display_base | ((ra << 13) & 0x7fff);

	for (int i = 0; i < x_count; i++)
	{
		uint16_t const offset = ((ma + i) << 1) & 0x1fff;
		uint16_t const data = space(0).read_word(rowbase | offset);

		*p++ = pal[m_palette_base + m_reg.data[0x10 | bitswap<2>(data, 15, 7)]];
		*p++ = pal[m_palette_base + m_reg.data[0x10 | bitswap<2>(data, 14, 6)]];
		*p++ = pal[m_palette_base + m_reg.data[0x10 | bitswap<2>(data, 13, 5)]];
		*p++ = pal[m_palette_base + m_reg.data[0x10 | bitswap<2>(data, 12, 4)]];
		*p++ = pal[m_palette_base + m_reg.data[0x10 | bitswap<2>(data, 11, 3)]];
		*p++ = pal[m_palette_base + m_reg.data[0x10 | bitswap<2>(data, 10, 2)]];
		*p++ = pal[m_palette_base + m_reg.data[0x10 | bitswap<2>(data,  9, 1)]];
		*p++ = pal[m_palette_base + m_reg.data[0x10 | bitswap<2>(data,  8, 0)]];
	}
}


MC6845_UPDATE_ROW( pc_t1t_device::t1000_gfx_2bpp_tga_update_row )
{
	rgb_t const *const pal = palette()->entry_list_raw();
	uint32_t *p = &bitmap.pix(y);
	uint32_t const rowbase = m_display_base | ((ra << 13) & 0x7fff);

	if (y == 0) logerror("t1000_gfx_2bpp_tga_update_row\n");
	for (int i = 0; i < x_count; i++)
	{
		uint16_t const offset = ((ma + i) << 1) & 0x1fff;
		uint16_t const data = space(0).read_word(rowbase | offset);

		*p++ = pal[m_reg.data[0x10 | bitswap<2>(data, 15, 7)]];
		*p++ = pal[m_reg.data[0x10 | bitswap<2>(data, 14, 6)]];
		*p++ = pal[m_reg.data[0x10 | bitswap<2>(data, 13, 5)]];
		*p++ = pal[m_reg.data[0x10 | bitswap<2>(data, 12, 4)]];
		*p++ = pal[m_reg.data[0x10 | bitswap<2>(data, 11, 3)]];
		*p++ = pal[m_reg.data[0x10 | bitswap<2>(data, 10, 2)]];
		*p++ = pal[m_reg.data[0x10 | bitswap<2>(data,  9, 1)]];
		*p++ = pal[m_reg.data[0x10 | bitswap<2>(data,  8, 0)]];
	}
}


MC6845_UPDATE_ROW( pc_t1t_device::t1000_gfx_1bpp_update_row )
{
	rgb_t const *const pal = palette()->entry_list_raw();
	uint32_t *p = &bitmap.pix(y);
	uint32_t const rowbase = m_display_base | ((ra << 13) & 0x7fff);
	uint8_t const fg = m_palette_base + m_reg.data[0x11];
	uint8_t const bg = m_palette_base + m_reg.data[0x10];

	if (y == 0) logerror("t1000_gfx_1bpp_update_row\n");
	for (int i = 0; i < x_count; i++)
	{
		uint16_t const offset = ((ma + i) << 1) & 0x1fff;
		uint16_t const data = space(0).read_word(rowbase | offset);

		*p++ = pal[BIT(data,  7) ? fg : bg];
		*p++ = pal[BIT(data,  6) ? fg : bg];
		*p++ = pal[BIT(data,  5) ? fg : bg];
		*p++ = pal[BIT(data,  4) ? fg : bg];
		*p++ = pal[BIT(data,  3) ? fg : bg];
		*p++ = pal[BIT(data,  2) ? fg : bg];
		*p++ = pal[BIT(data,  1) ? fg : bg];
		*p++ = pal[BIT(data,  0) ? fg : bg];
		*p++ = pal[BIT(data, 15) ? fg : bg];
		*p++ = pal[BIT(data, 14) ? fg : bg];
		*p++ = pal[BIT(data, 13) ? fg : bg];
		*p++ = pal[BIT(data, 12) ? fg : bg];
		*p++ = pal[BIT(data, 11) ? fg : bg];
		*p++ = pal[BIT(data, 10) ? fg : bg];
		*p++ = pal[BIT(data,  9) ? fg : bg];
		*p++ = pal[BIT(data,  8) ? fg : bg];
	}
}

MC6845_UPDATE_ROW( pc_t1t_device::crtc_update_row )
{
	if (m_update_row_type == -1)
		return;

	switch (m_update_row_type)
	{
		case T1000_TEXT_INTEN:
			t1000_text_inten_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case T1000_TEXT_BLINK:
			t1000_text_blink_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case T1000_GFX_1BPP:
			t1000_gfx_1bpp_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case T1000_GFX_2BPP:
			t1000_gfx_2bpp_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case T1000_GFX_2BPP_TGA:
			t1000_gfx_2bpp_tga_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case T1000_GFX_4BPP:
			t1000_gfx_4bpp_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
	}
}


MC6845_UPDATE_ROW( pcvideo_pcjr_device::crtc_update_row )
{
	if (m_update_row_type == -1)
		return;

	switch (m_update_row_type)
	{
		case PCJX_TEXT:
			pcjx_text_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case PCJR_GFX_2BPP_HIGH:
			pcjr_gfx_2bpp_high_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		default:
			pc_t1t_device::crtc_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
	}
}


void pcvideo_t1000_device::mode_switch()
{
	switch (m_mode_control & 0x3B)
	{
	case 0x08: case 0x09:
		m_update_row_type = T1000_TEXT_INTEN;
		break;
	case 0x28: case 0x29:
		m_update_row_type = T1000_TEXT_BLINK;
		break;
	case 0x0A: case 0x0B: case 0x2A: case 0x2B:
		switch (m_bank & 0xc0)
		{
		case 0x00:
		case 0x40:
			//logerror("t1t_gfx_2bpp - 1\n");
			m_update_row_type = T1000_GFX_2BPP;
			if ( m_color_select )
			{
				m_reg.data[0x10] = 0x00;
				m_reg.data[0x11] = 0x0B;
				m_reg.data[0x12] = 0x0D;
				m_reg.data[0x13] = 0x0F;
			}
			else
			{
				m_reg.data[0x10] = 0x00;
				m_reg.data[0x11] = 0x0A;
				m_reg.data[0x12] = 0x0C;
				m_reg.data[0x13] = 0x0E;
			}
			break;
		case 0x80:
		case 0xc0:
			//logerror("t1t_gfx_4bpp\n");
			m_update_row_type = T1000_GFX_4BPP;
			break;
		}
		break;
	case 0x18: case 0x19: case 0x1A: case 0x1B:
	case 0x38: case 0x39: case 0x3A: case 0x3B:
		switch (m_bank & 0xc0)
		{
		case 0x00:
		case 0x40:
			//logerror("t1t_gfx_1bpp\n");
			m_update_row_type = T1000_GFX_1BPP;
			break;
		case 0x80:
		case 0xc0:
			//logerror("t1t_gfx_2bpp - 2\n");
			m_update_row_type = T1000_GFX_2BPP_TGA;
			break;
		}
		break;
	default:
		m_update_row_type = -1;
		break;
	}
}


/* mode control 1 ( m_reg.data[0] ) */
/* bit0 - 0 = low bandwidth, 1 = high bandwidth */
/* bit1 - 0 = alpha, 1 = graphics */
/* bit2 - 0 = color, 1 = b/w */
/* bit3 - 0 = video disable, 1 = video enable */
/* bit4 - 1 = 16 color graphics */
/* mode control 2 ( m_reg.data[3] ) */
/* bit1 - 1 = enable blink */
/* bit3 - 1 = 2 color graphics */

void pcvideo_pcjr_device::pc_pcjr_mode_switch()
{
	switch( m_reg.data[0] & 0x1A )
	{
	case 0x08:      /* 01x0x */
		if(m_jxkanji)
		{
			m_update_row_type = PCJX_TEXT;
			break;
		}
		if ( m_reg.data[3] & 0x02 )
		{
			m_update_row_type = T1000_TEXT_BLINK;
		}
		else
		{
			m_update_row_type = T1000_TEXT_INTEN;
		}
		break;
	case 0x0A:      /* 01x1x */
		/* By default use medium resolution mode */
		m_update_row_type = T1000_GFX_2BPP;

		/* Check for high resolution mode */
		if ((m_bank & 0xc0) == 0xc0)
			m_update_row_type = PCJR_GFX_2BPP_HIGH;

		/* Check for 640x200 b/w 2 shades mode */
		if ( ( m_reg.data[0] & 0x04 ) && ( m_reg.data[3] & 0x08 ) )
		{
			m_update_row_type = T1000_GFX_1BPP;
		}
		break;
	case 0x18:      /* 11x0x - invalid?? */
		m_update_row_type = -1;
		break;
	case 0x1A:      /* 11x1x */
		m_update_row_type = T1000_GFX_4BPP;
		break;
	default:
		m_update_row_type = -1;
		break;
	}

	/* Determine mc6845 input clock */
	if ( m_reg.data[0] & 0x01 )
	{
		m_mc6845->set_unscaled_clock( XTAL(14'318'181)/8 );
	}
	else
	{
		m_mc6845->set_unscaled_clock( XTAL(14'318'181)/16 );
	}

	/* color or b/w? */
	m_palette_base = ( m_reg.data[0] & 0x04 ) ? 16 : 0;
}


/*
 * 3d8 rW   T1T mode control register (see #P138)
 */
void pcvideo_t1000_device::mode_control_w(int data)
{
	m_mode_control = data;

	mode_switch();
}

int pc_t1t_device::mode_control_r()
{
	int data = m_mode_control;
	return data;
}

/*
 * 3d9 ?W   color select register on color adapter
 */
void pc_t1t_device::color_select_w(int data)
{
	if (m_color_select == data)
		return;
	m_color_select = data;
}

int pc_t1t_device::color_select_r()
{
	int data = m_color_select;
	return data;
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
void pc_t1t_device::vga_index_w(int data)
{
	m_reg.index = data;
}

void pcvideo_t1000_device::vga_data_w(int data)
{
	m_reg.data[m_reg.index] = data;

	switch (m_reg.index)
	{
		case 0x00: /* mode control 1 */
			break;
		case 0x01: /* palette mask (bits 3-0) */
			break;
		case 0x02: /* border color (bits 3-0) */
			break;
		case 0x03: /* mode control 2 */
			break;
		case 0x04: /* reset register */
			break;
		/* palette array */
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b:
		case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			m_reg.data[m_reg.index] = data & 0x0F;
			break;
	}
}


void pcvideo_pcjr_device::pc_pcjr_vga_data_w(int data)
{
	m_reg.data[m_reg.index] = data;

	switch (m_reg.index)
	{
		case 0x00:  /* mode control 1 */
					/* bit0 - 0 = low bandwidth, 1 = high bandwidth */
					/* bit1 - 0 = alpha, 1 = graphics */
					/* bit2 - 0 = color, 1 = b/w */
					/* bit3 - 0 = video disable, 1 = video enable */
					/* bit4 - 1 = 16 color graphics */
			pc_pcjr_mode_switch();
			break;
		case 0x01:  /* palette mask (bits 3-0) */
			break;
		case 0x02:  /* border color (bits 3-0) */
			break;
		case 0x03:  /* mode control 2 */
					/* bit1 - 1 = enable blink */
					/* bit3 - 1 = 2 color graphics */
			pc_pcjr_mode_switch();
			break;
		case 0x04:  /* reset register */
			break;
					/* palette array */
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b:
		case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			m_reg.data[m_reg.index] = data & 0x0F;
			break;
	}
}


int pc_t1t_device::vga_data_r()
{
	int data = m_reg.data[m_reg.index];

	switch (m_reg.index)
	{
		case 0x00: /* mode control 1 */
			break;
		case 0x01: /* palette mask (bits 3-0) */
			break;
		case 0x02: /* border color (bits 3-0) */
			break;
		case 0x03: /* mode control 2 */
			break;
		case 0x04: /* reset register */
			break;
		/* palette array */
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b:
		case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			break;
	}
	return data;
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
void pcvideo_t1000_device::bank_w(uint16_t data)
{
	if (m_disable)
		return;

	uint32_t dram, vram;
	if ((data&0xc0)==0xc0) /* needed for lemmings */
	{
		dram = (m_bank & 0x06);// | ((m_bank & 0x1800) >> 8);
		vram = ((m_bank & 0x30) >> 3);// | ((m_bank & 0x6000) >> 10);
	}
	else
	{
		dram = (m_bank & 0x07);// | ((m_bank & 0x1800) >> 8);
		vram = ((m_bank & 0x38) >> 3);// | ((m_bank & 0x6000) >> 10);
	}
	m_display_base = dram << 14;
	m_window_base = vram << 14;

	// FIXME: this won't ever get hit because write(...) updates m_bank before calling this function
	if ((m_bank & 0xc0) != (data & 0xc0))
		mode_switch();
}


void pcvideo_pcjr_device::pc_pcjr_bank_w(uint8_t data)
{
	uint32_t dram, vram;
	m_bank = data;
	/* it seems the video ram is mapped to the last 128K of main memory */
	if ((data&0xc0)==0xc0) /* needed for lemmings */
	{
		dram = (data & 0x06);
		vram = (data & 0x30) >> 3;
	}
	else
	{
		dram = (data & 0x07);
		vram = (data & 0x38) >> 3;
	}
	m_display_base = dram << 14;
	m_window_base = vram << 14;

	// FIXME: this won't ever get hit because m_bank was updated just a few lines ago
	if ((m_bank & 0xc0) != (data & 0xc0))
		pc_pcjr_mode_switch();
}

int pc_t1t_device::bank_r()
{
	return m_bank;
}

/*************************************************************************
 *
 *      T1T
 *      Tandy 1000 / PCjr
 *
 *************************************************************************/

void pcvideo_t1000_device::write(offs_t offset, uint8_t data)
{
	switch( offset )
	{
		case 0: case 2: case 4: case 6:
			m_mc6845->address_w(data);
			break;
		case 1: case 3: case 5: case 7:
			m_mc6845->register_w(data);
			break;
		case 8:
			mode_control_w(data);
			break;
		case 9:
			color_select_w(data);
			break;
		case 10:
			vga_index_w(data);
			break;
		case 11:
			lightpen_strobe_w(data);
			break;
		case 12:
			break;
		case 13:
			m_bank = (data << 8) | (m_bank & 0xff);
			bank_w(m_bank);
			break;
		case 14:
			vga_data_w(data);
			break;
		case 15:
			m_bank = (m_bank & 0xff00) | data;
			bank_w(data);
			break;
	}
}

void pcvideo_pcjr_device::write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0: case 4:
			m_mc6845->address_w(data);
			break;
		case 1: case 5:
			m_mc6845->register_w(data);
			break;
		case 10:
			if ( m_address_data_ff & 0x01 )
			{
				pc_pcjr_vga_data_w( data );
			}
			else
			{
				vga_index_w( data );
			}
			m_address_data_ff ^= 0x01;
			break;
		case 11:
			lightpen_strobe_w(data);
			break;
		case 12:
			break;
		case 15:
			pc_pcjr_bank_w(data);
			break;

		default:
			break;
	}
}


uint8_t pc_t1t_device::read(offs_t offset)
{
	int data = 0xff;

	switch (offset)
	{
		case 0: case 2: case 4: case 6:
			/* return last written mc6845 address value here? */
			break;

		case 1: case 3: case 5: case 7:
			data = m_mc6845->register_r();
			break;

		case 8:
			data = mode_control_r();
			break;

		case 9:
			data = color_select_r();
			break;

		case 10:
			m_address_data_ff = 0;
			data = status_r();
			break;

		case 11:
			/* -W lightpen strobe reset */
			break;

		case 12:
		case 13:
			break;

		case 14:
			data = vga_data_r();
			break;

		case 15:
			data = bank_r();
			break;
	}
	return data;
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


void pc_t1t_device::t1000_de_changed(int state)
{
	m_display_enable = state ? 0 : 1;
}


void pcvideo_pcjr_device::de_changed(int state)
{
	m_display_enable = state ? 1 : 0;
}


void pcvideo_t1000_device::t1000_vsync_changed(int state)
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
