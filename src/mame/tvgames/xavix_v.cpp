// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "xavix.h"

#include <cmath>

// #define VERBOSE 1
#include "logmacro.h"


inline void xavix_state::set_data_address(int address, int bit)
{
	m_tmp_dataaddress = address;
	m_tmp_databit = bit;
}

inline uint8_t xavix_state::get_next_bit()
{
	// going through memory is slow, try not to do it too often!
	if (m_tmp_databit == 0)
	{
		if (m_disable_memory_bypass)
			m_bit = m_maincpu->space(AS_PROGRAM).read_byte(m_tmp_dataaddress);
		else
			m_bit = read_full_data_sp_bypass(m_tmp_dataaddress);
	}

	uint8_t ret = m_bit >> m_tmp_databit;
	ret &= 1;

	m_tmp_databit++;

	if (m_tmp_databit == 8)
	{
		m_tmp_databit = 0;
		m_tmp_dataaddress++;
	}

	return ret;
}

inline uint8_t superxavix_state::get_next_bit_sx()
{
	if (m_tmp_databit == 0)
	{
		m_bit = m_extra[m_tmp_dataaddress&0x7fffff];
	}

	uint8_t ret = m_bit >> m_tmp_databit;
	ret &= 1;

	m_tmp_databit++;

	if (m_tmp_databit == 8)
	{
		m_tmp_databit = 0;
		m_tmp_dataaddress++;
	}

	return ret;
}


inline uint8_t xavix_state::get_next_byte()
{
	uint8_t dat = 0;
	for (int i = 0; i < 8; i++)
	{
		dat |= (get_next_bit() << i);
	}
	return dat;
}

inline int xavix_state::get_current_address_byte()
{
	return m_tmp_dataaddress;
}


void xavix_state::video_start()
{
	m_screen->register_screen_bitmap(m_zbuffer);
}


void xavix_state::palram_sh_w(offs_t offset, uint8_t data)
{
	m_palram_sh[offset] = data;
	update_pen(offset, m_palram_sh[offset], m_palram_l[offset]);
}

void xavix_state::palram_l_w(offs_t offset, uint8_t data)
{
	m_palram_l[offset] = data;
	update_pen(offset, m_palram_sh[offset], m_palram_l[offset]);
}

void superxavix_state::bmp_palram_sh_w(offs_t offset, uint8_t data)
{
	m_bmp_palram_sh[offset] = data;
	update_pen(offset+256, m_bmp_palram_sh[offset], m_bmp_palram_l[offset]);
}

void superxavix_state::bmp_palram_l_w(offs_t offset, uint8_t data)
{
	m_bmp_palram_l[offset] = data;
	update_pen(offset+256, m_bmp_palram_sh[offset], m_bmp_palram_l[offset]);
}


void xavix_state::spriteram_w(offs_t offset, uint8_t data)
{
	if (offset < 0x100)
	{
		m_fragment_sprite[offset] = data;
		m_fragment_sprite[offset + 0x400] = data & 0x01;
	}
	else if (offset < 0x400)
	{
		m_fragment_sprite[offset] = data;
	}
	else if (offset < 0x500)
	{
		m_fragment_sprite[offset] = data & 1;
		m_fragment_sprite[offset - 0x400] = (m_fragment_sprite[offset - 0x400] & 0xfe) | (data & 0x01);
		m_sprite_xhigh_ignore_hack = false; // still doesn't help monster truck test mode case, which writes here, but still expects values to be ignored
	}
	else
	{
		m_fragment_sprite[offset] = data;
	}
}

void superxavix_state::superxavix_crtc_1_w(offs_t offset, uint8_t data)
{
	logerror("%s superxavix_crtc_1_w %02x %02x\n", machine().describe_context(), offset, data);
	m_sx_crtc_1[offset] = data;
}

uint8_t superxavix_state::superxavix_crtc_1_r(offs_t offset)
{
	return m_sx_crtc_1[offset];
}

void superxavix_state::bitmap_params_w(offs_t offset, uint8_t data)
{
	logerror("%s bitmap_params_w %02x %02x\n", machine().describe_context(), offset, data);
	m_bmp_base[offset] = data;
	// if anything changes these mid-screen we may need to trigger a partial update here
}

uint8_t superxavix_state::bitmap_params_r(offs_t offset)
{
	return m_bmp_base[offset];
}

void superxavix_state::superxavix_crtc_2_w(offs_t offset, uint8_t data)
{
	logerror("%s superxavix_crtc_2_w %02x %02x\n", machine().describe_context(), offset, data);
	m_sx_crtc_2[offset] = data;
}

uint8_t superxavix_state::superxavix_crtc_2_r(offs_t offset)
{
	return m_sx_crtc_2[offset];
}

void superxavix_state::ext_segment_regs_w(offs_t offset, uint8_t data)
{
	logerror("%s ext_segment_regs_w %02x %02x\n", machine().describe_context(), offset, data);
	m_ext_segment_regs[offset] = data;
}

void superxavix_state::superxavix_plt_flush_w(uint8_t data)
{
	// flush current write buffer (as we might not have filled a byte when plotting)
	// and set write mode?
	// do we need to cache the previous write address for the flush? (probably not, this seems to be explicitly
	// written both after every transfer, and before the next one.
	if (m_plotter_has_byte)
	{
		m_maincpu->space(6).write_byte((m_sx_plt_address >> 3) & 0x7fffff, m_plotter_current_byte);
	}

	m_plotter_has_byte = 0;
	m_plotter_current_byte = 0x00;
	//printf("%s: superxavix_plt_flush_w %02x\n", machine().describe_context().c_str(), data);
	m_sx_plt_mode = data;
}

uint8_t superxavix_state::superxavix_plt_dat_r()
{
	return machine().rand();
}

void superxavix_state::superxavix_plt_dat_w(uint8_t data)
{
	uint8_t pixels = m_sx_plt_mode + 1;

	while (pixels)
	{
		if (!m_plotter_has_byte)
		{
			m_plotter_current_byte = m_maincpu->space(6).read_byte((m_sx_plt_address>>3) & 0x7fffff);
			m_plotter_has_byte = 1;
		}

		uint8_t bit_to_write = data & 1;
		data >>= 1;

		uint8_t destbit = m_sx_plt_address & 0x7;
		m_plotter_current_byte &= ~(1 << destbit);
		m_plotter_current_byte |= (bit_to_write << destbit);

		m_sx_plt_address++;
		if (!(m_sx_plt_address & 0x7))
		{
			m_maincpu->space(6).write_byte(((m_sx_plt_address-1)>>3) & 0x7fffff, m_plotter_current_byte);
			m_plotter_has_byte = 0;
		}

		pixels--;
	}
}

void superxavix_state::superxavix_plt_loc_w(offs_t offset, uint8_t data)
{
	logerror("%s superxavix_plt_loc_w %02x %02x\n", machine().describe_context(), offset, data);

	m_sx_plt_loc[offset] = data;
	if (offset == 3)
	{
		m_sx_plt_address = (m_sx_plt_loc[3] << 24) | (m_sx_plt_loc[2] << 16) | (m_sx_plt_loc[1] << 8) | (m_sx_plt_loc[0] << 0);
		logerror("%s SuperXavix Bitmap Plotter set to address %08x\n", machine().describe_context(), m_sx_plt_address);
	}
}

uint8_t superxavix_state::superxavix_plt_loc_r(offs_t offset)
{
	return m_sx_plt_loc[offset];
}

void superxavix_state::superxavix_bitmap_pal_index_w(uint8_t data)
{
	m_superxavix_bitmap_pal_index = data;
}

uint8_t superxavix_state::superxavix_bitmap_pal_index_r()
{
	return m_superxavix_bitmap_pal_index;
}

void superxavix_state::superxavix_chr_pal_index_w(uint8_t data)
{
	m_superxavix_pal_index = data;
}

uint8_t superxavix_state::superxavix_chr_pal_index_r()
{
	return m_superxavix_pal_index;
}


uint8_t superxavix_state::superxavix_bitmap_pal_hue_r()
{
	return superxavix_pal_hue_r(true);
}

uint8_t superxavix_state::superxavix_chr_pal_hue_r()
{
	return superxavix_pal_hue_r(false);
}

uint8_t superxavix_state::superxavix_pal_hue_r(bool bitmap)
{
	u8 *sh, *sl;
	u8 ind;

	if (bitmap)
	{
		sh = m_bmp_palram_sh;
		sl = m_bmp_palram_l;
		ind = m_superxavix_bitmap_pal_index;
	}
	else
	{
		sh = m_palram_sh;
		sl = m_palram_l;
		ind = m_superxavix_pal_index;
	}

	uint16_t dat = (sh[ind]) | (sl[ind] << 8);
	uint16_t val = get_pen_hue_from_dat(dat);
	return val << 2;
}

uint8_t superxavix_state::superxavix_bitmap_pal_saturation_r()
{
	return superxavix_pal_saturation_r(true);
}

uint8_t superxavix_state::superxavix_chr_pal_saturation_r()
{
	return superxavix_pal_saturation_r(false);
}

uint8_t superxavix_state::superxavix_pal_saturation_r(bool bitmap)
{
	u8 *sh, *sl;
	u8 ind;

	if (bitmap)
	{
		sh = m_bmp_palram_sh;
		sl = m_bmp_palram_l;
		ind = m_superxavix_bitmap_pal_index;
	}
	else
	{
		sh = m_palram_sh;
		sl = m_palram_l;
		ind = m_superxavix_pal_index;
	}

	uint16_t dat = (sh[ind]) | (sl[ind] << 8);
	uint16_t val = get_pen_saturation_from_dat(dat);
	return val << 4;
}

uint8_t superxavix_state::superxavix_bitmap_pal_lightness_r()
{
	return superxavix_pal_lightness_r(true);
}

uint8_t superxavix_state::superxavix_chr_pal_lightness_r()
{
	return superxavix_pal_lightness_r(false);
}

uint8_t superxavix_state::superxavix_pal_lightness_r(bool bitmap)
{
	u8 *sh, *sl;
	u8 ind;

	if (bitmap)
	{
		sh = m_bmp_palram_sh;
		sl = m_bmp_palram_l;
		ind = m_superxavix_bitmap_pal_index;
	}
	else
	{
		sh = m_palram_sh;
		sl = m_palram_l;
		ind = m_superxavix_pal_index;
	}

	uint16_t dat = (sh[ind]) | (sl[ind] << 8);
	uint16_t val = get_pen_lightness_from_dat(dat);
	return val << 2;
}

void superxavix_state::superxavix_bitmap_pal_hue_w(uint8_t data)
{
	superxavix_pal_hue_w(data, true);
}

void superxavix_state::superxavix_chr_pal_hue_w(uint8_t data)
{
	superxavix_pal_hue_w(data, false);
}

void superxavix_state::superxavix_pal_hue_w(uint8_t data, bool bitmap)
{
	u8 *sh, *sl;
	u8 ind;
	int offset;

	if (bitmap)
	{
		sh = m_bmp_palram_sh;
		sl = m_bmp_palram_l;
		ind = m_superxavix_bitmap_pal_index;
		offset = 256;
	}
	else
	{
		sh = m_palram_sh;
		sl = m_palram_l;
		ind = m_superxavix_pal_index;
		offset = 0;
	}

	uint16_t olddat = (sh[ind]) | (sl[ind] << 8);
	uint16_t newdata = apply_pen_hue_to_dat(olddat, data >> 2);
	sh[ind] = newdata & 0xff;
	sl[ind] = (newdata >> 8) & 0xff;
	update_pen(ind+offset, sh[ind], sl[ind]);
}

void superxavix_state::superxavix_bitmap_pal_saturation_w(uint8_t data)
{
	superxavix_pal_saturation_w(data, true);
}

void superxavix_state::superxavix_chr_pal_saturation_w(uint8_t data)
{
	superxavix_pal_saturation_w(data, false);
}

void superxavix_state::superxavix_pal_saturation_w(uint8_t data, bool bitmap)
{
	u8 *sh, *sl;
	u8 ind;
	int offset;

	if (bitmap)
	{
		sh = m_bmp_palram_sh;
		sl = m_bmp_palram_l;
		ind = m_superxavix_bitmap_pal_index;
		offset = 256;
	}
	else
	{
		sh = m_palram_sh;
		sl = m_palram_l;
		ind = m_superxavix_pal_index;
		offset = 0;
	}

	uint16_t olddat = (sh[ind]) | (sl[ind] << 8);
	uint16_t newdata = apply_pen_saturation_to_dat(olddat, data >> 4);
	sh[ind] = newdata & 0xff;
	sl[ind] = (newdata >> 8) & 0xff;
	update_pen(ind+offset, sh[ind], sl[ind]);
}

void superxavix_state::superxavix_bitmap_pal_lightness_w(uint8_t data)
{
	superxavix_pal_lightness_w(data, true);
}

void superxavix_state::superxavix_chr_pal_lightness_w(uint8_t data)
{
	superxavix_pal_lightness_w(data, false);
}

void superxavix_state::superxavix_pal_lightness_w(uint8_t data, bool bitmap)
{
	u8 *sh, *sl;
	u8 ind;
	int offset;

	if (bitmap)
	{
		sh = m_bmp_palram_sh;
		sl = m_bmp_palram_l;
		ind = m_superxavix_bitmap_pal_index;
		offset = 256;
	}
	else
	{
		sh = m_palram_sh;
		sl = m_palram_l;
		ind = m_superxavix_pal_index;
		offset = 0;
	}

	uint16_t olddat = (sh[ind]) | (sl[ind] << 8);
	uint16_t newdata = apply_pen_lightness_to_dat(olddat, data >> 2);
	sh[ind] = newdata & 0xff;
	sl[ind] = (newdata >> 8) & 0xff;
	update_pen(ind+offset, sh[ind], sl[ind]);
}

uint16_t xavix_state::apply_pen_lightness_to_dat(uint16_t dat, uint16_t lightness)
{
	lightness &= 0x3f;
	dat &= ~0x1f00;
	dat &= ~0x8000;
	dat |= (lightness & 0x3e) << 7;
	dat |= (lightness & 0x01) << 15;
	return dat;
}

uint16_t xavix_state::apply_pen_saturation_to_dat(uint16_t dat, uint16_t saturation)
{
	saturation &= 0x0f;
	dat &= ~0x00e0;
	dat &= ~0x4000;
	dat |= (saturation & 0x0e) << 4;
	dat |= (saturation & 0x01) << 14;
	return dat;
}

uint16_t xavix_state::apply_pen_hue_to_dat(uint16_t dat, uint16_t hue)
{
	hue &= 0x3f;
	dat &= ~0x001f;
	dat &= ~0x2000;
	dat |= (hue & 0x3e) >> 1;
	dat |= (hue & 0x01) << 13;
	return dat;
}

uint8_t xavix_state::get_pen_lightness_from_dat(uint16_t dat)
{
	uint8_t y_raw = (dat & 0x1f00) >> 7;
	y_raw |= (dat & 0x8000) >> 15; // currently we don't make use of this bit
	return y_raw;
}

uint8_t xavix_state::get_pen_saturation_from_dat(uint16_t dat)
{
	uint8_t c_raw = (dat & 0x00e0) >> 4;
	c_raw |= (dat & 0x4000) >> 14; // currently we don't make use of this bit
	return c_raw;
}

uint8_t xavix_state::get_pen_hue_from_dat(uint16_t dat)
{
	uint8_t h_raw = (dat & 0x001f) << 1;
	h_raw |= (dat & 0x2000) >> 13; // currently we don't make use of this bit
	return h_raw;
}

void xavix_state::update_pen(int pen, uint8_t shval, uint8_t lval)
{
	uint16_t dat;
	dat = shval;
	dat |= lval << 8;

	int y_raw = get_pen_lightness_from_dat(dat) >> 1;
	int c_raw = get_pen_saturation_from_dat(dat) >> 1;
	int h_raw = get_pen_hue_from_dat(dat) >> 1;

	// The dividers may be dynamic
	double y = y_raw / 20.0;
	double c = c_raw /  5.0;

	// These weights may be dynamic too.  They're standard NTSC values, would they change on PAL?
	const double wr = 0.299;
	const double wg = 0.587;
	const double wb = 0.114;

	// Table of hues
	// Values 24+ are transparent

	const double hues[32][3] = {
		{ 1.00, 0.00, 0.00 },
		{ 1.00, 0.25, 0.00 },
		{ 1.00, 0.50, 0.00 },
		{ 1.00, 0.75, 0.00 },
		{ 1.00, 1.00, 0.00 },
		{ 0.75, 1.00, 0.00 },
		{ 0.50, 1.00, 0.00 },
		{ 0.25, 1.00, 0.00 },
		{ 0.00, 1.00, 0.00 },
		{ 0.00, 1.00, 0.25 },
		{ 0.00, 1.00, 0.50 },
		{ 0.00, 1.00, 0.75 },
		{ 0.00, 1.00, 1.00 },
		{ 0.00, 0.75, 1.00 },
		{ 0.00, 0.50, 1.00 },
		{ 0.00, 0.25, 1.00 },
		{ 0.00, 0.00, 1.00 },
		{ 0.25, 0.00, 1.00 },
		{ 0.50, 0.00, 1.00 },
		{ 0.75, 0.00, 1.00 },
		{ 1.00, 0.00, 1.00 },
		{ 1.00, 0.00, 0.75 },
		{ 1.00, 0.00, 0.50 },
		{ 1.00, 0.00, 0.25 },

		{ 0   , 0   , 0    },
		{ 0   , 0   , 0    },
		{ 0   , 0   , 0    },
		{ 0   , 0   , 0    },
		{ 0   , 0   , 0    },
		{ 0   , 0   , 0    },
		{ 0   , 0   , 0    },
		{ 0   , 0   , 0    },
	};

	double r0 = hues[h_raw][0];
	double g0 = hues[h_raw][1];
	double b0 = hues[h_raw][2];

	double z = wr * r0 + wg * g0 + wb * b0;

	if(y < z)
		c *= y/z;
	else if(z < 1)
		c *= (1-y) / (1-z);

	double r1 = (r0 - z) * c + y;
	double g1 = (g0 - z) * c + y;
	double b1 = (b0 - z) * c + y;


	// lower overall brightness slightly, or some palette entries in tak_gin end up washed out / with identical colours, losing details
	// the darkest colours in certain flags still look wrong however, and appear nearly black
	// this might suggest the overall palette conversion needs work
	r1 = r1 * 0.92f;
	g1 = g1 * 0.92f;
	b1 = b1 * 0.92f;

	if(r1 < 0)
		r1 = 0;
	else if(r1 > 1)
		r1 = 1.0;

	if(g1 < 0)
		g1 = 0;
	else if(g1 > 1)
		g1 = 1.0;

	if(b1 < 0)
		b1 = 0;
	else if(b1 > 1)
		b1 = 1.0;

	int r_real = r1 * 255.0f;
	int g_real = g1 * 255.0f;
	int b_real = b1 * 255.0f;

	m_palette->set_pen_color(pen, r_real, g_real, b_real);

	m_screen->update_partial(m_screen->vpos());
}



void xavix_state::draw_tilemap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int which)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		draw_tilemap_line(screen, bitmap, cliprect, which, y);
	}
}

void superxavix_state::draw_tilemap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int which)
{
	m_use_superxavix_extra = false;
	xavix_state::draw_tilemap(screen, bitmap, cliprect, which);
}

void xavix_state::decode_inline_header(int &flipx, int &flipy, int &test, int &pal, int debug_packets)
{
	uint8_t byte1 = 0;
	int done = 0;

	flipx = 0;
	flipy = 0;
	test = 0;

	int first = 1;

	do
	{
		byte1 = get_next_byte();

		// only the first byte matters when it comes to setting palette / flips, the rest are just ignored until we reach a 0x6 command, after which there is the tile data
		if (first == 1)
		{
			pal = (byte1 & 0xf0) >> 4;
			int cmd = (byte1 & 0x0f);

			switch (cmd)
			{
			// these cases haven't been seen
			case 0x0:
			case 0x2:
			case 0x4:
			case 0x8:
			case 0xa:
			case 0xc:
			case 0xe:

			// this is just the end command, changes nothing, can be pointed at directly tho
			case 0x6:
				break;

			// flip cases
			// does bit 0x02 have a meaning here, we have 2 values for each case

			case 0x1:
			case 0x3:
				flipx = 0; flipy = 0;
				break;

			case 0x5:
			case 0x7:
				flipx = 1; flipy = 0;
				break;

			case 0x9:
			case 0xb:
				flipx = 0; flipy = 1;
				break;

			case 0xd:
			case 0xf:
				flipx = 1; flipy = 1;
				break;
			}

			first = 0;
		}

		if ((byte1 & 0x0f) == 0x06)
		{
			// tile data will follow after this, always?
			done = 1;
			//if (debug_packets) LOG(" (setting palette)");
		}
	} while (done == 0);
	//if (debug_packets) LOG("\n");
}

void xavix_state::draw_tilemap_line(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int which, int line)
{
	uint8_t* tileregs;
	if (which == 0)
	{
		tileregs = m_tmap1_regs;
	}
	else
	{
		tileregs = m_tmap2_regs;
	}

	// bail if tilemap is disabled
	if (!(tileregs[0x7] & 0x80))
		return;

	int use_inline_header = 0;
	int alt_tileaddressing2 = 0;

	int ydimension = 0;
	int xdimension = 0;
	int ytilesize = 0;
	int xtilesize = 0;
	int yshift = 0;

	switch (tileregs[0x3] & 0x30)
	{
	case 0x00:
		ydimension = 32;
		xdimension = 32;
		ytilesize = 8;
		xtilesize = 8;
		yshift = 3;
		break;

	case 0x10:
		ydimension = 32;
		xdimension = 16;
		ytilesize = 8;
		xtilesize = 16;
		yshift = 3;
		break;

	case 0x20: // guess
		ydimension = 16;
		xdimension = 32;
		ytilesize = 16;
		xtilesize = 8;
		yshift = 4;
		break;

	case 0x30:
		ydimension = 16;
		xdimension = 16;
		ytilesize = 16;
		xtilesize = 16;
		yshift = 4;
		break;
	}

	if (tileregs[0x7] & 0x10)
		use_inline_header = 1;
	else
		use_inline_header = 0;

	if (tileregs[0x7] & 0x02)
		alt_tileaddressing2 = 1;

	if ((tileregs[0x7] & 0x7f) == 0x04)
		alt_tileaddressing2 = 2;

	// SuperXaviX only?
	if ((tileregs[0x7] & 0x7f) == 0x0d)
		alt_tileaddressing2 = 3;

	//LOG("draw tilemap %d, regs base0 %02x base1 %02x base2 %02x tilesize,bpp %02x scrollx %02x scrolly %02x pal %02x mode %02x\n", which, tileregs[0x0], tileregs[0x1], tileregs[0x2], tileregs[0x3], tileregs[0x4], tileregs[0x5], tileregs[0x6], tileregs[0x7]);

	// there's a tilemap register to specify base in main ram, although in the monster truck test mode it points to an unmapped region
	// and expected a fixed layout, we handle that in the memory map at the moment

	int drawline = line;
	int scrolly = tileregs[0x5];

	drawline += scrolly;

	drawline &= ((ydimension * ytilesize) - 1);

	int y = drawline >> yshift;
	int yyline = drawline & (ytilesize - 1);

	for (int x = 0; x < xdimension; x++)
	{
		int count = (y * xdimension) + x;

		int tile = 0;

		// the register being 0 probably isn't the condition here
		if (tileregs[0x0] != 0x00)
		{
			const offs_t realaddress = (tileregs[0x0] << 8) + count;
			if (m_disable_memory_bypass)
				tile |= m_maincpu->space(AS_PROGRAM).read_byte(realaddress);
			else
				tile |= read_full_data_sp_bypass(realaddress);
		}

		// only read the next byte if we're not in an 8-bit mode
		if (((tileregs[0x7] & 0x7f) != 0x00) && ((tileregs[0x7] & 0x7f) != 0x08))
		{
			const offs_t realaddress = (tileregs[0x1] << 8) + count;
			if (m_disable_memory_bypass)
				tile |= m_maincpu->space(AS_PROGRAM).read_byte(realaddress) << 8;
			else
				tile |= read_full_data_sp_bypass(realaddress) << 8;
		}

		// 24 bit modes can use reg 0x2, otherwise it gets used as extra attribute in other modes
		if (alt_tileaddressing2 == 2)
		{
			const offs_t realaddress = (tileregs[0x2] << 8) + count;
			if (m_disable_memory_bypass)
				tile |= m_maincpu->space(AS_PROGRAM).read_byte(realaddress) << 16;
			else
				tile |= read_full_data_sp_bypass(realaddress) << 16;
		}


		int bpp = (tileregs[0x3] & 0x0e) >> 1;
		bpp++;
		int pal = (tileregs[0x6] & 0xf0) >> 4;
		int zval = (tileregs[0x6] & 0x0f) >> 0;
		int scrollx = tileregs[0x4];

		int basereg;
		int flipx = (tileregs[0x03]&0x40)>>6;
		int flipy = (tileregs[0x03]&0x80)>>7;

		// epo_doka explicitly sets these registers on the XaviX logo
		// but expects no flipping.  Is code being executed out of order or
		// is this further evidence that they don't work as expected on SuperXaviX
		// hardware (see other hack for xavmusic needing sprite flip disabled)
		if (m_disable_tile_regs_flip)
		{
			flipx = flipy = 0;
		}

		int gfxbase;

		// tile 0 is always skipped, doesn't even point to valid data packets in alt mode
		// this is consistent with the top layer too
		// should we draw as solid in solid layer?
		if (tile == 0)
		{
			continue;
		}

		int debug_packets = 1;
		//if (line==128) debug_packets = 1;
		//else debug_packets = 0;

		int test = 0;

		if (!use_inline_header)
		{
			if (alt_tileaddressing2 == 0)
			{
				// Tile Based Addressing takes into account Tile Sizes and bpp
				const int offset_multiplier = (ytilesize * xtilesize) / 8;

				basereg = 0;
				gfxbase = (m_segment_regs[(basereg * 2) + 1] << 16) | (m_segment_regs[(basereg * 2)] << 8);

				tile = tile * (offset_multiplier * bpp);
				tile += gfxbase;
			}
			else if (alt_tileaddressing2 == 1)
			{
				if (tileregs[0x7] & 0x01)
				{
					// only epo_stad has been seen using this mode (box behind dialog after you allow multiple strikes against you)
					basereg = (tile & 0xf000) >> 12;
					tile = tile & 0x0fff;
					gfxbase = (m_segment_regs[(basereg * 2) + 1] << 16) | (m_segment_regs[(basereg * 2)] << 8);
					tile += gfxbase;
				}
				else
				{
					// 8-byte alignment Addressing Mode uses a fixed offset? (like sprites)
					basereg = (tile & 0xe000) >> 13;
					tile &= 0x1fff;
					tile = tile * 8;
					gfxbase = (m_segment_regs[(basereg * 2) + 1] << 16) | (m_segment_regs[(basereg * 2)] << 8);
					tile += gfxbase;
				}
			}
			else if (alt_tileaddressing2 == 3)
			{
				// SuperXaviX only?
				if (m_ext_segment_regs)
				{
					// Tile Based Addressing takes into account Tile Sizes and bpp
					const int offset_multiplier = (ytilesize * xtilesize) / 8;
					const int basereg = 0;
					gfxbase = (m_ext_segment_regs[(basereg * 4) + 3] << 24) |
						(m_ext_segment_regs[(basereg * 4) + 2] << 16) |
						(m_ext_segment_regs[(basereg * 4) + 1] << 8) |
						(m_ext_segment_regs[(basereg * 4) + 0] << 0);

					tile = tile * (offset_multiplier * bpp);
					tile += gfxbase;
				}
				else
				{
					tile = 0;
				}
			}

			// Tilemap specific mode extension with an 8-bit per tile attribute, works in all modes except 24-bit (no room for attribute) and header (not needed?)
			if (tileregs[0x7] & 0x08)
			{
				const offs_t realaddress = (tileregs[0x2] << 8) + count;
				uint8_t extraattr;

				if (m_disable_memory_bypass)
					extraattr = m_maincpu->space(AS_PROGRAM).read_byte(realaddress);
				else
					extraattr = read_full_data_sp_bypass(realaddress);

				// make use of the extraattr stuff?
				pal = (extraattr & 0xf0) >> 4;
				zval = (extraattr & 0x0f) >> 0;
			}
		}
		else
		{
			// Addressing Mode 2 (plus Inline Header)

			//if (debug_packets) LOG("for tile %04x (at %d %d): ", tile, (((x * 16) + scrollx) & 0xff), (((y * 16) + scrolly) & 0xff));

			basereg = (tile & 0xf000) >> 12;
			tile &= 0x0fff;
			gfxbase = (m_segment_regs[(basereg * 2) + 1] << 16) | (m_segment_regs[(basereg * 2)] << 8);

			tile += gfxbase;
			set_data_address(tile, 0);

			decode_inline_header(flipx, flipy, test, pal, debug_packets);

			tile = get_current_address_byte();
		}

		if (test == 1)
		{
			pal = machine().rand() & 0xf;
		}

		draw_tile_line(screen, bitmap, cliprect, tile, bpp, (x * xtilesize) + scrollx, line, ytilesize, xtilesize, flipx, flipy, pal, zval, yyline);
		draw_tile_line(screen, bitmap, cliprect, tile, bpp, ((x * xtilesize) + scrollx) - 256, line, ytilesize, xtilesize, flipx, flipy, pal, zval, yyline); // wrap-x
	}
}

void xavix_state::draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		draw_sprites_line(screen, bitmap, cliprect, y);
	}
}

void superxavix_state::draw_sprites(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
{
	if (m_extra && m_allow_superxavix_extra_rom_sprites)
		m_use_superxavix_extra = true;

	xavix_state::draw_sprites(screen, bitmap, cliprect);
	m_use_superxavix_extra = false;
}


void xavix_state::draw_sprites_line(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int line)
{
	int alt_addressing = 0;

	// some games have the top bit set, why?
	m_spritereg &= 0x7f;

	if ((m_spritereg == 0x00) || (m_spritereg == 0x01))
	{
		// 8-bit addressing  (Tile Number)
		// 16-bit addressing (Tile Number) (rad_rh)
		alt_addressing = 1;
	}
	else if (m_spritereg == 0x02)
	{
		// 16-bit addressing (8-byte alignment Addressing Mode)
		alt_addressing = 2;
	}
	else if (m_spritereg == 0x04)
	{
		// 24-bit addressing (Addressing Mode 2)
		alt_addressing = 0;
	}
	else if (m_spritereg == 0x07)
	{
		// used by anpanmdx - is this a specific 24-bit mode to enable SuperXaviX extra ROM access?
		alt_addressing = 3;
	}
	else
	{
		popmessage("unknown sprite reg %02x", m_spritereg);
	}


	//LOG("frame\n");
	// priority doesn't seem to be based on list order, there are bad sprite-sprite priorities with either forward or reverse

	for (int i = 0xff; i >= 0; i--)
	{

		uint8_t* spr_attr0 = m_fragment_sprite + 0x000;
		uint8_t* spr_attr1 = m_fragment_sprite + 0x100;
		uint8_t* spr_ypos = m_fragment_sprite + 0x200;
		uint8_t* spr_xpos = m_fragment_sprite + 0x300;
		uint8_t* spr_xposh = m_fragment_sprite + 0x400;
		uint8_t* spr_addr_lo = m_fragment_sprite + 0x500;
		uint8_t* spr_addr_md = m_fragment_sprite + 0x600;
		uint8_t* spr_addr_hi = m_fragment_sprite + 0x700;


		/* attribute 0 bits
		   pppp bbb-    p = palette, b = bpp

		   attribute 1 bits
		   zzzz ssFf    s = size, F = flipy f = flipx
		*/

		int ypos = spr_ypos[i];
		int xpos = spr_xpos[i];
		int tile = 0;

		// high 8-bits only used in 24-bit modes
		if (((m_spritereg & 0x7f) == 0x04) || ((m_spritereg & 0x7f) == 0x07) || ((m_spritereg & 0x7f) == 0x15))
			tile |= (spr_addr_hi[i] << 16);

		// mid 8-bits used in everything except 8-bit mode
		if ((m_spritereg & 0x7f) != 0x00)
			tile |= (spr_addr_md[i] << 8);

		// low 8-bits always read
		tile |= spr_addr_lo[i];

		int attr0 = spr_attr0[i];
		int attr1 = spr_attr1[i];

		int pal = (attr0 & 0xf0) >> 4;

		int zval = (attr1 & 0xf0) >> 4;
		int flipx = (attr1 & 0x01);
		int flipy = (attr1 & 0x02);

		// many elements, including the XaviX logo on xavmusic have yflip set, but don't want it, why?
		if (m_disable_sprite_yflip)
			flipy = 0;

		int drawheight = 16;
		int drawwidth = 16;

		int xpos_adjust = 0;
		int ypos_adjust = -16;

		// taito nost attr1 is 84 / 80 / 88 / 8c for the various elements of the xavix logo.  monster truck uses ec / fc / dc / 4c / 5c / 6c (final 6 sprites ingame are 00 00 f0 f0 f0 f0, radar?)

		drawheight = 8;
		drawwidth = 8;

		if (attr1 & 0x04) drawwidth = 16;
		if (attr1 & 0x08) drawheight = 16;

		xpos_adjust = -(drawwidth/2);
		ypos_adjust = -(drawheight/2);

		ypos ^= 0xff;

		if (ypos & 0x80)
		{
			ypos = -0x80 + (ypos & 0x7f);
		}
		else
		{
			ypos &= 0x7f;
		}

		ypos += 128 + 1;

		ypos += ypos_adjust;

		int spritelowy = ypos;
		int spritehighy = ypos + drawheight;

		if ((line >= spritelowy) && (line < spritehighy))
		{
			int drawline = line - spritelowy;


			/* coordinates are signed, based on screen position 0,0 being at the center of the screen
			   tile addressing likewise, point 0,0 is center of tile?
			   this makes the calculation a bit more annoying in terms of knowing when to apply offsets, when to wrap etc.
			   this is likely still incorrect

			   -- NOTE! HACK!

			   Use of additional x-bit is very confusing rad_snow, taitons1 (ingame) etc. clearly need to use it
			   but the taitons1 xavix logo doesn't even initialize the RAM for it and behavior conflicts with ingame?
			   maybe only works with certain tile sizes?

			   some code even suggests this should be bit 0 of attr0, but it never gets set there
			   (I'm mirroring the bits in the write handler at the moment)

			   there must be a register somewhere (or a side-effect of another mode) that enables / disables this
			   behavior, as we need to make use of xposh for the left side in cases that need it, but that
			   completely breaks the games that never set it at all (monster truck, xavix logo on taitons1)

			   monster truck hidden service mode ends up writing to the RAM, breaking the 'clock' display if
			   we use the values for anything.. again suggesting there must be a way to ignore it entirely?

			 */

			int xposh = spr_xposh[i] & 1;

			if (xpos & 0x80) // left side of center
			{
				xpos &= 0x7f;
				xpos = -0x80 + xpos;

				// this also breaks the epoch logo on suprtvpc, although some ingame elements need it
				if (!m_sprite_xhigh_ignore_hack)
					if (!xposh)
						xpos -= 0x80;

			}
			else // right side of center
			{
				xpos &= 0x7f;

				if (!m_sprite_xhigh_ignore_hack)
					if (xposh)
						xpos += 0x80;
			}

			xpos += 128;

			xpos += xpos_adjust;

			// galplus phalanx beam (sprite wraparound)
			if (xpos<-0x80)
				xpos += 256+128;

			int bpp = 1;

			bpp = (attr0 & 0x0e) >> 1;
			bpp += 1;

			// Everything except directdirect addressing (Addressing Mode 2) goes through the segment registers?
			if (alt_addressing != 0)
			{
				int basereg = 0;

				// tile based addressing takes into account tile size (and bpp?)
				if (alt_addressing == 1)
				{
					tile = (tile * drawheight * drawwidth * bpp) / 8;
					int gfxbase = (m_segment_regs[1] << 16) | (m_segment_regs[0] << 8); // always use segment 0
					tile += gfxbase;

					// ban_bkgj is in sprite mode 0x01 but seems to expect the extended SuperXaviX registers to apply too?
					//
					// Is the register being set correctly? I'd expect it to be a different value to enable this. It does
					// briefly set mode 4 and 5 on the startup logos, where it *doesn't* need this logic
					if (m_ext_segment_regs)
					{
						int gfxbase = (m_ext_segment_regs[3] << 24) |
									  (m_ext_segment_regs[2] << 16) |
									  (m_ext_segment_regs[1] << 8) |
									  (m_ext_segment_regs[0] << 0);
						tile += gfxbase;
					}


				}
				else if (alt_addressing == 2)
				{
					tile = tile * 8;
					basereg = (tile & 0xf0000) >> 16;
					tile &= 0xffff;
					int gfxbase = (m_segment_regs[(basereg * 2) + 1] << 16) | (m_segment_regs[(basereg * 2)] << 8);
					tile += gfxbase;
				}
				else if (alt_addressing == 3)
				{
					// 24-bit, with multiplier, no segment use?
					// seen in anpanmdx, might be superxavix specific to be able to access more memory?
					tile = tile * 8;
				}
				else // currently unused case
				{
					basereg = (tile & 0xf0000) >> 16;
					tile &= 0xffff;
					int gfxbase = (m_segment_regs[(basereg * 2) + 1] << 16) | (m_segment_regs[(basereg * 2)] << 8);
					tile += gfxbase;
				}

			}

			draw_tile_line(screen, bitmap, cliprect, tile, bpp, xpos , line, drawheight, drawwidth, flipx, flipy, pal, zval, drawline);

			/*
			if ((spr_ypos[i] != 0x81) && (spr_ypos[i] != 0x80) && (spr_ypos[i] != 0x00))
			{
			    LOG("sprite with enable? %02x attr0 %02x attr1 %02x attr3 %02x attr5 %02x attr6 %02x attr7 %02x\n", spr_ypos[i], spr_attr0[i], spr_attr1[i], spr_xpos[i], spr_addr_lo[i], spr_addr_md[i], spr_addr_hi[i] );
			}
			*/
		}
	}
}

void xavix_state::get_tile_pixel_dat(uint8_t &dat, int bpp)
{
	for (int i = 0; i < bpp; i++)
	{
		dat |= (get_next_bit() << i);
	}
}

void superxavix_state::get_tile_pixel_dat(uint8_t &dat, int bpp)
{
	if (m_use_superxavix_extra)
	{
		for (int i = 0; i < bpp; i++)
		{
			dat |= (get_next_bit_sx() << i);
		}
	}
	else
	{
		for (int i = 0; i < bpp; i++)
		{
			dat |= (get_next_bit() << i);
		}
	}
}

void xavix_state::draw_tile_line(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int tile, int bpp, int xpos, int ypos, int drawheight, int drawwidth, int flipx, int flipy, int pal, int zval, int line)
{
	const pen_t *paldata = m_palette->pens();
	if (ypos > cliprect.max_y || ypos < cliprect.min_y)
		return;

	if (((xpos * m_video_hres_multiplier) > cliprect.max_x) || (((xpos * m_video_hres_multiplier) + drawwidth * m_video_hres_multiplier) < cliprect.min_x))
		return;

	if ((ypos >= cliprect.min_y && ypos <= cliprect.max_y))
	{
		// if bpp>4 then ignore unaligned palette selects bits based on bpp
		// ttv_lotr uses 5bpp graphics (so 32 colour alignment) but sets palette 0xf (a 16 colour boundary) when it expects palette 0xe
		if (bpp>4)
			pal &= (0xf<<(bpp-4));

		int bits_per_tileline = drawwidth * bpp;

		// set the address here so we can increment in bits in the draw function
		set_data_address(tile, 0);

		if (flipy)
			line = (drawheight - 1) - line;

		m_tmp_dataaddress = m_tmp_dataaddress + ((line * bits_per_tileline) / 8);
		m_tmp_databit = (line * bits_per_tileline) % 8;

		for (int x = 0; x < drawwidth; x++)
		{
			int col;

			if (flipx)
			{
				col = xpos + (drawwidth - 1) - x;
			}
			else
			{
				col = xpos + x;
			}

			uint8_t dat = 0;

			get_tile_pixel_dat(dat, bpp);

			col = col * m_video_hres_multiplier;

			if ((col >= cliprect.min_x && col <= cliprect.max_x))
			{
				uint16_t *const zyposptr = &m_zbuffer.pix(ypos);

				if (zval >= zyposptr[col])
				{
					int pen = (dat + (pal << 4)) & 0xff;

					if ((m_palram_sh[pen] & 0x1f) < 24) // hue values 24-31 are transparent
					{
						uint32_t *const yposptr = &bitmap.pix(ypos);
						yposptr[col] = paldata[pen];
						if (m_video_hres_multiplier == 2)
							yposptr[col+1] = paldata[pen];

						zyposptr[col] = zval;

						if (m_video_hres_multiplier == 2)
							zyposptr[col+1] = zval;

					}
				}
			}
		}
	}
}

rectangle xavix_state::do_arena(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *paldata = m_palette->pens();
	bitmap.fill(m_palette->black_pen(), cliprect);

	rectangle clip = cliprect;

	clip.min_y = cliprect.min_y;
	clip.max_y = cliprect.max_y;
	clip.min_x = cliprect.min_x;
	clip.max_x = cliprect.max_x;

	if (m_arena_control & 0x01)
	{
		/* Controls the clipping area (for all layers?) used for effect at start of Slap Fight and to add black borders in other cases
		   based on Slap Fight Tiger lives display (and reference videos) this is slightly offset as all status bar gfx must display
		   Monster Truck black bars are wider on the right hand side, but this matches with the area in which the tilemap is incorrectly rendered so seems to be intentional
		   Snowboard hides garbage sprites on the right hand side with this, confirming the right hand side offset
		   Taito Nostalgia 1 'Gladiator' portraits in demo mode are cut slightly due to the area specified, again the cut-off points for left and right are confirmed as correct on hardware

		   some games enable it with both regs as 00, which causes a problem, likewise ping pong sets both to 0xff
		   but Slap Fight Tiger has both set to 0x82 at a time when everything should be clipped
		*/
		if (((m_arena_start != 0x00) && (m_arena_end != 0x00)) && ((m_arena_start != 0xff) && (m_arena_end != 0xff)))
		{
			clip.max_x = (m_arena_start - 3) * m_video_hres_multiplier; // must be -3 to hide garbage on the right hand side of snowboarder
			clip.min_x = (m_arena_end - 2) * m_video_hres_multiplier; // must be -2 to render a single pixel line of the left border on Mappy remix (verified to render), although this creates a single pixel gap on the left of snowboarder status bar (need to verify)

			if (clip.min_x < cliprect.min_x)
				clip.min_x = cliprect.min_x;

			if (clip.max_x > cliprect.max_x)
				clip.max_x = cliprect.max_x;
		}
	}

	bitmap.fill(paldata[0], clip);
	m_zbuffer.fill(0, clip);

	return clip;
}

void xavix_state::draw_regular_layers(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &clip)
{
	draw_tilemap(screen, bitmap, clip, 0);
	draw_tilemap(screen, bitmap, clip, 1); // epo_golf requires this layer in front when priorities are equal or menu doesn't show?
	draw_sprites(screen, bitmap, clip);
}

uint32_t xavix_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rectangle clip = do_arena(screen, bitmap, cliprect);
	draw_regular_layers(screen, bitmap, clip);
	return 0;
}

void superxavix_state::draw_bitmap_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *paldata = m_palette->pens();
	// incomplete!
	if (m_bmp_base)
	{
		// screen position for the bitmap?
		uint16_t top = ((m_bmp_base[0x01] << 8) | m_bmp_base[0x00]);
		uint16_t bot = ((m_bmp_base[0x03] << 8) | m_bmp_base[0x02]);
		uint16_t lft = ((m_bmp_base[0x05] << 8) | m_bmp_base[0x04]);
		uint16_t rgt = ((m_bmp_base[0x07] << 8) | m_bmp_base[0x06]);

		// and can specify base address relative start / end positions with these for data reading to be cut off?
		uint16_t topadr = ((m_bmp_base[0x09] << 8) | m_bmp_base[0x08]);
		uint16_t botadr = ((m_bmp_base[0x0b] << 8) | m_bmp_base[0x0a]);
		uint16_t lftadr = ((m_bmp_base[0x0d] << 8) | m_bmp_base[0x0c]);
		uint16_t rgtadr = ((m_bmp_base[0x0f] << 8) | m_bmp_base[0x0e]);

		uint16_t start = ((m_bmp_base[0x11] << 8) | m_bmp_base[0x10]);
		uint8_t step = m_bmp_base[0x12]; // step value to next line base
		uint8_t size = m_bmp_base[0x13]; // some kind of additional scaling?
		uint8_t mode = m_bmp_base[0x14]; // enable,bpp, zval etc.

		uint32_t unused = ((m_bmp_base[0x15] << 16) | (m_bmp_base[0x16] << 8) | (m_bmp_base[0x17] << 0));

		if (mode & 0x01)
		{
			if (0)
			{
				popmessage("bitmap t:%04x b:%04x l:%04x r:%04x  -- -- ta: %04x ba:%04x la:%04x ra:%04x   -- -- start %04x (%08x) step:%02x - size:%02x unused:%08x\n",
					top, bot, lft, rgt,
					topadr, botadr, lftadr, rgtadr,
					start, start * 0x800, step, size, unused);
			}

			// anpanmdx title screen ends up with a seemingly incorrect value for start
			// when it does the scroller.  There is presumably an opcode or math bug causing this.
			//if (start >= 0x7700)
			/// start -= 0x3c00;

			int base = start * 0x800;
			int base2 = topadr * 0x8;

			int bpp = ((mode & 0x0e) >> 1) + 1;
			int zval = ((mode & 0xf0) >> 4);

			int hposadjust, vpostadjust;
			bool is_highres = true;

			// SuperXaviX seems to have CRTC registers (0x6f80 - 0x6faf area), so in reality this probably
			// comes from there due to different overall screen positioning; might also depend on bitmap
			// bpp mode (so this is probably not the correct way to detect this)
			// there could also be XaviX2000+ opcode bugs causing wrong register values!
			if ((size & 0x70) == 0x70)
				is_highres = false;

			if (is_highres)
				hposadjust = 0x90; // good for xavtenni, xavbowl, tmy_thom, tmy_rkmj, but not xavbaseb
			else
				hposadjust = 0x46; // good for udance, ban_ordj, not quite correct for ban_kksj in demo

			vpostadjust = 0x06; // probably can vary (see XaviX boot-up logos, logo vs text positions)

			top += vpostadjust;
			bot += vpostadjust;

			for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
			{
				int line = y - top;

				if ((line > 0) && (y < bot)) // should also clip if out of range for botadr
				{
					set_data_address(base + base2 + ((line * (step * 64)) / 8), 0);

					// also needs to take into account 'lftadr' address from the line base
					// and clip against 'rgtadr' address if reads go beyond linebase + that for the line
					for (int x = lft - hposadjust; x < rgt - hposadjust; x++)
					{
						uint32_t* const yposptr = &bitmap.pix(y);
						uint16_t* const zyposptr = &m_zbuffer.pix(y);

						uint8_t dat = 0;

						if (m_extra)
						{
							for (int i = 0; i < bpp; i++)
							{
								dat |= (get_next_bit_sx() << i);
							}
						}
						else
						{
							for (int i = 0; i < bpp; i++)
							{
								dat |= (get_next_bit() << i);
							}
						}

						int realx;
						realx = x;

						if (!is_highres)
							realx *= 2;

						if (((realx <= cliprect.max_x) && (realx >= cliprect.min_x)) && ((y <= cliprect.max_y) && (y >= cliprect.min_y)))
						{
							if ((m_bmp_palram_sh[dat] & 0x1f) < 24) // same transparency logic as everything else? (baseball title)
							{
								if (zval >= zyposptr[realx])
								{
									yposptr[realx] = paldata[dat + 0x100];
									zyposptr[realx] = zval;
								}
							}
						}

						// draw doubled pixel for low-res bitmap
						if (!is_highres)
							realx += 1;

						if (((realx <= cliprect.max_x) && (realx >= cliprect.min_x)) && ((y <= cliprect.max_y) && (y >= cliprect.min_y)))
						{
							if ((m_bmp_palram_sh[dat] & 0x1f) < 24) // same transparency logic as everything else? (baseball title)
							{
								if (zval >= zyposptr[realx])
								{
									yposptr[realx] = paldata[dat + 0x100];
									zyposptr[realx] = zval;
								}
							}
						}

					}
				}
			}

		}
	}
}

uint32_t superxavix_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rectangle clip = do_arena(screen, bitmap, cliprect);
	draw_bitmap_layer(screen, bitmap, clip); // maxheart suggests bitmap is drawn first or you get a black box over the display
	draw_regular_layers(screen, bitmap, clip);
	return 0;
}

void xavix_state::spritefragment_dma_params_1_w(offs_t offset, uint8_t data)
{
	m_spritefragment_dmaparam1[offset] = data;
}

void xavix_state::spritefragment_dma_params_2_w(offs_t offset, uint8_t data)
{
	m_spritefragment_dmaparam2[offset] = data;
}

void xavix_state::spritefragment_dma_trg_w(uint8_t data)
{
	uint16_t len = data & 0x07;
	uint16_t src = (m_spritefragment_dmaparam1[1] << 8) | m_spritefragment_dmaparam1[0];
	uint16_t dst = (m_spritefragment_dmaparam2[0] << 8);

	uint8_t src_block_size = m_spritefragment_dmaparam2[1];

	LOG("%s: spritefragment_dma_trg_w with trg %02x size %04x src %04x dest %04x unk (%02x)\n", machine().describe_context(), data & 0xf8, len, src, dst, src_block_size);

	src_block_size--; // 0x00 is maximum

	if (len == 0x00)
	{
		len = 0x08;
		LOG(" (length was 0x0, assuming 0x8)\n");
	}

	len = len << 8;

	if (data & 0x40)
	{
		int readaddress = 0;

		for (int i = 0; i < len; i++)
		{
			// tak_chq explicitly sets unk & 0x80
			// and the source data is in 0x80 byte blocks, not 0x100
			// epo_doka uses 0xb4, which seems to confirm this
			if ((i & 0xff) > src_block_size)
			{
				// do nothing, maybe unk is the length of each section to copy?
			}
			else
			{
				//uint8_t dat = m_maincpu->read_full_data_sp(src + i);
				const offs_t realaddress = src + readaddress;
				uint8_t dat;
				if (m_disable_memory_bypass)
					dat = m_maincpu->space(AS_PROGRAM).read_byte(realaddress);
				else
					dat = read_full_data_sp_bypass(realaddress);
				//m_fragment_sprite[(dst + i) & 0x7ff] = dat;
				spriteram_w((dst + i) & 0x7ff, dat);
				readaddress++;
			}
		}
	}
}

uint8_t xavix_state::spritefragment_dma_status_r()
{
	// expects bit 0x40 to clear in most cases
	return 0x00;
}

uint8_t xavix_state::pal_ntsc_r()
{
	// only seen 0x10 checked in code
	// in monster truck the tile base address gets set based on this, there are 2 copies of the test screen in rom, one for pal, one for ntsc, see 1854c
	// likewise card night has entirely different tilesets for each region title
	return m_region->read();
}


void xavix_state::tmap1_regs_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	/*
	   0x0 pointer to low tile bits
	   0x1 pointer to middle tile bits
	   0x2 pointer to upper tile bits

	   0x3 Fftt bbb-  Ff = flip Y,X
	                  tt = tile/tilemap size
	                  b = bpp
	                  - = unused

	   0x4 scroll
	   0x5 scroll

	   0x6 pppp zzzz  p = palette
	                  z = priority

	   0x7 e--m mmmm  e = enable
	                  m = mode

	   modes are
	    ---0 0000 (00) 8-bit addressing  (Tile Number)
	    ---0 0001 (01) 16-bit addressing (Tile Number) (monster truck, ekara)
	    ---0 0010 (02) 16-bit addressing (8-byte alignment Addressing Mode) (boxing)
	    ---0 0011 (03) 16-bit addressing (Addressing Mode 2)
	    ---0 0100 (04) 24-bit addressing (Addressing Mode 2) (epo_efdx)

	    ---0 1000 (08) 8-bit+8 addressing  (Tile Number + 8-bit Attribute)
	    ---0 1001 (09) 16-bit+8 addressing (Tile Number + 8-bit Attribute) (Taito Nostalgia 2)
	    ---0 1010 (0a) 16-bit+8 addressing (8-byte alignment Addressing Mode + 8-bit Attribute) (boxing, Snowboard)
	    ---0 1011 (0b) 16-bit+8 addressing (Addressing Mode 2 + 8-bit Attribute)

	    ---0 1100 (0d) ban_bkgj - seems to be a SuperXaviX only mode using the extended 32-bit segment regs

	    ---1 0011 (13) 16-bit addressing (Addressing Mode 2 + Inline Header)  (monster truck)
	    ---1 0100 (14) 24-bit addressing (Addressing Mode 2 + Inline Header)

	*/

	if ((offset != 0x4) && (offset != 0x5))
	{
		LOG("%s: tmap1_regs_w offset %02x data %02x\n", machine().describe_context(), offset, data);
	}

	COMBINE_DATA(&m_tmap1_regs[offset]);
}

void xavix_state::tmap2_regs_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	// same as above but for 2nd tilemap
	if ((offset != 0x4) && (offset != 0x5))
	{
		LOG("%s: tmap2_regs_w offset %02x data %02x\n", machine().describe_context(), offset, data);
	}

	COMBINE_DATA(&m_tmap2_regs[offset]);
}


void xavix_state::spriteregs_w(uint8_t data)
{
	LOG("%s: spriteregs_w data %02x\n", machine().describe_context(), data);
	/*
	    This is similar to Tilemap reg 7 and is used to set the addressing mode for sprite data

	    ---0 -000 (00) 8-bit addressing  (Tile Number)
	    ---0 -001 (01) 16-bit addressing (Tile Number)
	    ---0 -010 (02) 16-bit addressing (8-byte alignment Addressing Mode)
	    ---0 -011 (03) 16-bit addressing (Addressing Mode 2)
	    ---0 -100 (04) 24-bit addressing (Addressing Mode 2)

	    ---1 -011 (13) 16-bit addressing (Addressing Mode 2 + Inline Header)
	    ---1 -100 (14) 24-bit addressing (Addressing Mode 2 + Inline Header)
	*/
	m_spritereg = data;
}

uint8_t xavix_state::tmap1_regs_r(offs_t offset)
{
	LOG("%s: tmap1_regs_r offset %02x\n", offset, machine().describe_context());
	return m_tmap1_regs[offset];
}

uint8_t xavix_state::tmap2_regs_r(offs_t offset)
{
	LOG("%s: tmap2_regs_r offset %02x\n", offset, machine().describe_context());
	return m_tmap2_regs[offset];
}

// The Text Array / Memory Emulator acts as a memory area that you can point the tilemap sources at to get a fixed pattern of data
void xavix_state::xavix_memoryemu_txarray_w(offs_t offset, uint8_t data)
{
	if (offset < 0x400)
	{
		// nothing
	}
	else if (offset < 0x800)
	{
		m_txarray[0] = data;
	}
	else if (offset < 0xc00)
	{
		m_txarray[1] = data;
	}
	else if (offset < 0x1000)
	{
		m_txarray[2] = data;
	}
}

uint8_t xavix_state::xavix_memoryemu_txarray_r(offs_t offset)
{
	return txarray_r(offset);
}
