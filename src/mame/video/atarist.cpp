// license:BSD-3-Clause
// copyright-holders:Curt Coder, Olivier Galibert
/*

    TODO:

    - rewrite shifter
    - STe pixelofs
    - blitter hog
    - high resolution

*/

#include "emu.h"
#include "video/atarist.h"
#include "cpu/m68000/m68000.h"
#include "screen.h"



//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

DEFINE_DEVICE_TYPE(ST_VIDEO, st_video_device, "st_video", "Atari ST Video ASICs")
DEFINE_DEVICE_TYPE(STE_VIDEO, ste_video_device, "ste_video", "Atari STe Video ASICs")
//DEFINE_DEVICE_TYPE(STBOOK_VIDEO, stbook_video_device, "stbook_video", "Atari STbook Video ASICs")
//DEFINE_DEVICE_TYPE(TT_VIDEO, tt_video_device, "tt_video", "Atari TT Video ASICs")

static const int BLITTER_NOPS[16][4] =
{
	{ 1, 1, 1, 1 },
	{ 2, 2, 3, 3 },
	{ 2, 2, 3, 3 },
	{ 1, 1, 2, 2 },
	{ 2, 2, 3, 3 },
	{ 2, 2, 2, 2 },
	{ 2, 2, 3, 3 },
	{ 2, 2, 3, 3 },
	{ 2, 2, 3, 3 },
	{ 2, 2, 3, 3 },
	{ 2, 2, 2, 2 },
	{ 2, 2, 3, 3 },
	{ 1, 1, 2, 2 },
	{ 2, 2, 3, 3 },
	{ 2, 2, 3, 3 },
	{ 1, 1, 1, 1 }
};


//**************************************************************************
//  SHIFTER
//**************************************************************************

//-------------------------------------------------
//  shift_mode_0 -
//-------------------------------------------------

inline pen_t st_video_device::shift_mode_0()
{
	int color = (BIT(m_shifter_rr[3], 15) << 3) | (BIT(m_shifter_rr[2], 15) << 2) | (BIT(m_shifter_rr[1], 15) << 1) | BIT(m_shifter_rr[0], 15);

	m_shifter_rr[0] <<= 1;
	m_shifter_rr[1] <<= 1;
	m_shifter_rr[2] <<= 1;
	m_shifter_rr[3] <<= 1;

	return pen(color);
}


//-------------------------------------------------
//  shift_mode_1 -
//-------------------------------------------------

inline pen_t st_video_device::shift_mode_1()
{
	int color = (BIT(m_shifter_rr[1], 15) << 1) | BIT(m_shifter_rr[0], 15);

	m_shifter_rr[0] <<= 1;
	m_shifter_rr[1] <<= 1;
	m_shifter_shift++;

	if (m_shifter_shift == 16)
	{
		m_shifter_rr[0] = m_shifter_rr[2];
		m_shifter_rr[1] = m_shifter_rr[3];
		m_shifter_rr[2] = m_shifter_rr[3] = 0;
		m_shifter_shift = 0;
	}

	return pen(color);
}


//-------------------------------------------------
//  shift_mode_2 -
//-------------------------------------------------

inline pen_t st_video_device::shift_mode_2()
{
	int color = BIT(m_shifter_rr[0], 15);

	m_shifter_rr[0] <<= 1;
	m_shifter_shift++;

	switch (m_shifter_shift)
	{
	case 16:
		m_shifter_rr[0] = m_shifter_rr[1];
		m_shifter_rr[1] = m_shifter_rr[2];
		m_shifter_rr[2] = m_shifter_rr[3];
		m_shifter_rr[3] = 0;
		break;

	case 32:
		m_shifter_rr[0] = m_shifter_rr[1];
		m_shifter_rr[1] = m_shifter_rr[2];
		m_shifter_rr[2] = 0;
		break;

	case 48:
		m_shifter_rr[0] = m_shifter_rr[1];
		m_shifter_rr[1] = 0;
		m_shifter_shift = 0;
		break;
	}

	return pen(color);
}


//-------------------------------------------------
//  shifter_tick -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(st_video_device::shifter_tick)
{
	int y = screen().vpos();
	int x = screen().hpos();

	pen_t pen;

	switch (m_shifter_mode)
	{
	case 0:
		pen = shift_mode_0();
		break;

	case 1:
		pen = shift_mode_1();
		break;

	case 2:
		pen = shift_mode_2();
		break;

	default:
		pen = black_pen();
		break;
	}

	m_bitmap.pix(y, x) = pen;
}


//-------------------------------------------------
//  shifter_load -
//-------------------------------------------------

inline void st_video_device::shifter_load()
{
	uint16_t data = m_ram_space->read_word(m_shifter_ofs);

	m_shifter_ir[m_shifter_bitplane] = data;
	m_shifter_bitplane++;
	m_shifter_ofs += 2;

	if (m_shifter_bitplane == 4)
	{
		m_shifter_bitplane = 0;

		m_shifter_rr[0] = m_shifter_ir[0];
		m_shifter_rr[1] = m_shifter_ir[1];
		m_shifter_rr[2] = m_shifter_ir[2];
		m_shifter_rr[3] = m_shifter_ir[3];
	}
}


//-------------------------------------------------
//  glue_tick -
//-------------------------------------------------

void st_video_device::draw_pixel(int x, int y, u32 pen)
{
	if(x < m_bitmap.width() && y < m_bitmap.height())
		m_bitmap.pix(y, x) = pen;
}

TIMER_CALLBACK_MEMBER(st_video_device::glue_tick)
{
	int y = screen().vpos();
	int x = screen().hpos();

	int v = (y >= m_shifter_y_start) && (y < m_shifter_y_end);
	int h = (x >= m_shifter_x_start) && (x < m_shifter_x_end);

	int de = h && v;

	if(!x) {
		m_shifter_bitplane = 0;
		m_shifter_shift = 0;
	}

	if (de != m_shifter_de)
	{
		m_de_callback(de);
		m_shifter_de = de;
	}

	if (de)
	{
		shifter_load();
	}

	if ((y == m_shifter_vblank_start) && (x == 0))
	{
		m_maincpu->set_input_line(M68K_IRQ_4, HOLD_LINE); // FIXME: make this a callback instead
		m_shifter_ofs = m_shifter_base;
	}

	if (x == m_shifter_hblank_start)
	{
		m_maincpu->set_input_line(M68K_IRQ_2, HOLD_LINE); // FIXME: make this a callback instead
//      m_shifter_ofs += (m_shifter_lineofs * 2); // STe
	}

	pen_t pen;

	switch (m_shifter_mode)
	{
	case 0:
		pen = shift_mode_0();
		draw_pixel(x, y, pen);
		draw_pixel(x+1, y, pen);
		pen = shift_mode_0();
		draw_pixel(x+2, y, pen);
		draw_pixel(x+3, y, pen);
		pen = shift_mode_0();
		draw_pixel(x+4, y, pen);
		draw_pixel(x+5, y, pen);
		pen = shift_mode_0();
		draw_pixel(x+6, y, pen);
		draw_pixel(x+7, y, pen);
		break;

	case 1:
		pen = shift_mode_1();
		draw_pixel(x, y, pen);
		pen = shift_mode_1();
		draw_pixel(x+1, y, pen);
		pen = shift_mode_1();
		draw_pixel(x+2, y, pen);
		pen = shift_mode_1();
		draw_pixel(x+3, y, pen);
		pen = shift_mode_1();
		draw_pixel(x+4, y, pen);
		pen = shift_mode_1();
		draw_pixel(x+5, y, pen);
		pen = shift_mode_1();
		draw_pixel(x+6, y, pen);
		pen = shift_mode_1();
		draw_pixel(x+7, y, pen);
		break;

	case 2:
		pen = shift_mode_2();
		break;

	default:
		pen = black_pen();
		break;
	}
}


//-------------------------------------------------
//  set_screen_parameters -
//-------------------------------------------------

void st_video_device::set_screen_parameters()
{
	if (m_shifter_sync & 0x02)
	{
		m_shifter_x_start = ATARIST_HBDEND_PAL*2;
		m_shifter_x_end = ATARIST_HBDSTART_PAL*2;
		m_shifter_y_start = ATARIST_VBDEND_PAL;
		m_shifter_y_end = ATARIST_VBDSTART_PAL;
		m_shifter_hblank_start = ATARIST_HBSTART_PAL*2;
		m_shifter_vblank_start = ATARIST_VBSTART_PAL;
	}
	else
	{
		m_shifter_x_start = ATARIST_HBDEND_NTSC*2;
		m_shifter_x_end = ATARIST_HBDSTART_NTSC*2;
		m_shifter_y_start = ATARIST_VBDEND_NTSC;
		m_shifter_y_end = ATARIST_VBDSTART_NTSC;
		m_shifter_hblank_start = ATARIST_HBSTART_NTSC*2;
		m_shifter_vblank_start = ATARIST_VBSTART_NTSC;
	}
}


//-------------------------------------------------
//  shifter_base_r -
//-------------------------------------------------

uint8_t st_video_device::shifter_base_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
	case 0x00:
		data = (m_shifter_base >> 16) & 0x3f;
		break;

	case 0x01:
		data = (m_shifter_base >> 8) & 0xff;
		break;
	}

	return data;
}


//-------------------------------------------------
//  shifter_base_w -
//-------------------------------------------------

void st_video_device::shifter_base_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x00:
		m_shifter_base = (m_shifter_base & 0x00ff00) | (data & 0x3f) << 16;
		logerror("SHIFTER Video Base Address %06x\n", m_shifter_base);
		break;

	case 0x01:
		m_shifter_base = (m_shifter_base & 0x3f0000) | (data << 8);
		logerror("SHIFTER Video Base Address %06x\n", m_shifter_base);
		break;
	}
}


//-------------------------------------------------
//  shifter_counter_r -
//-------------------------------------------------

uint8_t st_video_device::shifter_counter_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
	case 0x00:
		data = (m_shifter_ofs >> 16) & 0x3f;
		break;

	case 0x01:
		data = (m_shifter_ofs >> 8) & 0xff;
		break;

	case 0x02:
		data = m_shifter_ofs & 0xff;
		break;
	}

	return data;
}


//-------------------------------------------------
//  shifter_sync_r -
//-------------------------------------------------

uint8_t st_video_device::shifter_sync_r()
{
	return m_shifter_sync;
}


//-------------------------------------------------
//  shifter_sync_w -
//-------------------------------------------------

void st_video_device::shifter_sync_w(uint8_t data)
{
	m_shifter_sync = data;
	logerror("SHIFTER Sync %x\n", m_shifter_sync);
	set_screen_parameters();
}


//-------------------------------------------------
//  shifter_mode_r -
//-------------------------------------------------

uint8_t st_video_device::shifter_mode_r()
{
	return m_shifter_mode;
}


//-------------------------------------------------
//  shifter_mode_w -
//-------------------------------------------------

void st_video_device::shifter_mode_w(uint8_t data)
{
	m_shifter_mode = data;
	logerror("SHIFTER Mode %x\n", m_shifter_mode);
}


//-------------------------------------------------
//  shifter_palette_r -
//-------------------------------------------------

uint16_t st_video_device::shifter_palette_r(offs_t offset)
{
	return m_shifter_palette[offset] | 0xf888;
}


//-------------------------------------------------
//  shifter_palette_w -
//-------------------------------------------------

void st_video_device::shifter_palette_w(offs_t offset, uint16_t data)
{
	m_shifter_palette[offset] = data;
	//  logerror("SHIFTER Palette[%x] = %x\n", offset, data);

	set_pen_color(offset, pal3bit(data >> 8), pal3bit(data >> 4), pal3bit(data));
}



//**************************************************************************
//  STE SHIFTER
//**************************************************************************

//-------------------------------------------------
//  shifter_base_low_r -
//-------------------------------------------------

uint8_t ste_video_device::shifter_base_low_r()
{
	return m_shifter_base & 0xfe;
}


//-------------------------------------------------
//  shifter_base_low_w -
//-------------------------------------------------

void ste_video_device::shifter_base_low_w(uint8_t data)
{
	m_shifter_base = (m_shifter_base & 0x3fff00) | (data & 0xfe);
	logerror("SHIFTER Video Base Address %06x\n", m_shifter_base);
}


//-------------------------------------------------
//  shifter_counter_r -
//-------------------------------------------------

uint8_t ste_video_device::shifter_counter_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
	case 0x00:
		data = (m_shifter_ofs >> 16) & 0x3f;
		break;

	case 0x01:
		data = (m_shifter_ofs >> 8) & 0xff;
		break;

	case 0x02:
		data = m_shifter_ofs & 0xfe;
		break;
	}

	return data;
}


//-------------------------------------------------
//  shifter_counter_w -
//-------------------------------------------------

void ste_video_device::shifter_counter_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x00:
		m_shifter_ofs = (m_shifter_ofs & 0x00fffe) | (data & 0x3f) << 16;
		logerror("SHIFTER Video Address Counter %06x\n", m_shifter_ofs);
		break;

	case 0x01:
		m_shifter_ofs = (m_shifter_ofs & 0x3f00fe) | (data << 8);
		logerror("SHIFTER Video Address Counter %06x\n", m_shifter_ofs);
		break;

	case 0x02:
		m_shifter_ofs = (m_shifter_ofs & 0x3fff00) | (data & 0xfe);
		logerror("SHIFTER Video Address Counter %06x\n", m_shifter_ofs);
		break;
	}
}


//-------------------------------------------------
//  shifter_palette_w -
//-------------------------------------------------

void ste_video_device::shifter_palette_w(offs_t offset, uint16_t data)
{
	int r = ((data >> 7) & 0x0e) | BIT(data, 11);
	int g = ((data >> 3) & 0x0e) | BIT(data, 7);
	int b = ((data << 1) & 0x0e) | BIT(data, 3);

	m_shifter_palette[offset] = data;
	logerror("SHIFTER palette %x = %x\n", offset, data);

	set_pen_color(offset, r, g, b);
}


//-------------------------------------------------
//  shifter_lineofs_r -
//-------------------------------------------------

uint8_t ste_video_device::shifter_lineofs_r()
{
	return m_shifter_lineofs;
}


//-------------------------------------------------
//  shifter_lineofs_w -
//-------------------------------------------------

void ste_video_device::shifter_lineofs_w(uint8_t data)
{
	m_shifter_lineofs = data;
	logerror("SHIFTER Line Offset %x\n", m_shifter_lineofs);
}


//-------------------------------------------------
//  shifter_pixelofs_r -
//-------------------------------------------------

uint8_t ste_video_device::shifter_pixelofs_r()
{
	return m_shifter_pixelofs;
}


//-------------------------------------------------
//  shifter_pixelofs_w -
//-------------------------------------------------

void ste_video_device::shifter_pixelofs_w(uint8_t data)
{
	m_shifter_pixelofs = data & 0x0f;
	logerror("SHIFTER Pixel Offset %x\n", m_shifter_pixelofs);
}



//**************************************************************************
//  VIDEO
//**************************************************************************

st_video_device::st_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_palette_interface(mconfig, *this)
	, device_video_interface(mconfig, *this, true)
	, m_ram_space(*this, finder_base::DUMMY_TAG, 0)
	, m_maincpu(*this, "^m68000") // FIXME: use callbacks instead
	, m_de_callback(*this)
{
}

st_video_device::st_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: st_video_device(mconfig, ST_VIDEO, tag, owner, clock)
{
}

ste_video_device::ste_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: st_video_device(mconfig, STE_VIDEO, tag, owner, clock)
{
}


void st_video_device::device_resolve_objects()
{
	m_de_callback.resolve_safe();
}


void st_video_device::device_start()
{
	m_shifter_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(st_video_device::shifter_tick), this));
	m_glue_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(st_video_device::glue_tick), this));

//  m_shifter_timer->adjust(screen().time_until_pos(0), 0, clocks_to_attotime(4)); // 125 ns
	m_glue_timer->adjust(screen().time_until_pos(0), 0, clocks_to_attotime(16)); // 500 ns

	screen().register_screen_bitmap(m_bitmap);

	// register for state saving
	save_item(NAME(m_shifter_base));
	save_item(NAME(m_shifter_ofs));
	save_item(NAME(m_shifter_sync));
	save_item(NAME(m_shifter_mode));
	save_item(NAME(m_shifter_palette));
	save_item(NAME(m_shifter_rr));
	save_item(NAME(m_shifter_ir));
	save_item(NAME(m_shifter_bitplane));
	save_item(NAME(m_shifter_shift));
	save_item(NAME(m_shifter_h));
	save_item(NAME(m_shifter_v));
	save_item(NAME(m_shifter_de));

	m_shifter_base = 0;
	m_shifter_ofs = 0;
	m_shifter_mode = 0;

	set_screen_parameters();
}

void ste_video_device::device_start()
{
	st_video_device::device_start();

	// register for state saving
	save_item(NAME(m_shifter_lineofs));
	save_item(NAME(m_shifter_pixelofs));
}


void st_video_device::device_reset()
{
	// TODO: reset glue chip
}


uint32_t st_video_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}
