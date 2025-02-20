// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont, Juergen Buchmueller
/**********************************************************************************************

    Acorn VIDC10 (VIDeo Controller) device chip

    based off legacy AA VIDC implementation by Angelo Salese, R. Belmont, Juergen Buchmueller

    TODO:
    - subclass screen_device, derive h/vsync signals out there;
    - improve timings for raster effects:
      * nebulus: 20 lines off with aa310;
      * lotustc2: abuses color flipping;
      * quazer: needs in-flight DMA;
    - move DAC handling into a separate sub-device(s),
      particularly needed for proper VIDC20 mixing and likely for fixing aliasing
      issues in VIDC10;
    - complete VIDC20 emulation (RiscPC/ssfindo.cpp);
    - Are CRTC values correct? VGA modes have a +1 in display line;

**********************************************************************************************/

#include "emu.h"
#include "acorn_vidc.h"
#include "screen.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(ACORN_VIDC1, acorn_vidc1_device, "acorn_vidc1", "Acorn VIDC1")
DEFINE_DEVICE_TYPE(ACORN_VIDC1A, acorn_vidc1a_device, "acorn_vidc1a", "Acorn VIDC1a")
DEFINE_DEVICE_TYPE(ARM_VIDC20, arm_vidc20_device, "arm_vidc20", "ARM VIDC20")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  acorn_vidc10_device - constructor
//-------------------------------------------------

void acorn_vidc10_device::regs_map(address_map &map)
{
	map(0x00, 0x3f).w(FUNC(acorn_vidc10_device::pal_data_display_w));
	map(0x40, 0x4f).w(FUNC(acorn_vidc10_device::pal_data_cursor_w));
	map(0x60, 0x7f).w(FUNC(acorn_vidc10_device::stereo_image_w));
	map(0x80, 0xbf).w(FUNC(acorn_vidc10_device::crtc_w));
	map(0xc0, 0xc3).w(FUNC(acorn_vidc10_device::sound_frequency_w));
	map(0xe0, 0xe3).w(FUNC(acorn_vidc10_device::control_w));
}


acorn_vidc10_device::acorn_vidc10_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int dac_type)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_palette_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, m_bpp_mode(0)
	, m_crtc_interlace(0)
	, m_sound_frequency_latch(0)
	, m_sound_mode(false)
	, m_dac(*this, "dac%u", 0)
	, m_dac_type(dac_type)
	, m_lspeaker(*this, "lspeaker")
	, m_rspeaker(*this, "rspeaker")
	, m_vblank_cb(*this)
	, m_sound_drq_cb(*this)
	, m_pixel_clock(0)
	, m_cursor_enable(false)
	, m_sound_frequency_test_bit(false)
{
	std::fill(std::begin(m_crtc_regs), std::end(m_crtc_regs), 0);
	std::fill(std::begin(m_stereo_image), std::end(m_stereo_image), 0);
}

acorn_vidc1_device::acorn_vidc1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: acorn_vidc10_device(mconfig, ACORN_VIDC1, tag, owner, clock, 1)
{
	m_space_config = address_space_config("regs_space", ENDIANNESS_LITTLE, 32, 8, 0, address_map_constructor(FUNC(acorn_vidc1_device::regs_map), this));
	m_pal_4bpp_base = 0x100;
	m_pal_cursor_base = 0x10;
	m_pal_border_base = 0x110;
}

acorn_vidc1a_device::acorn_vidc1a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: acorn_vidc10_device(mconfig, ACORN_VIDC1A, tag, owner, clock, 2)
{
	m_space_config = address_space_config("regs_space", ENDIANNESS_LITTLE, 32, 8, 0, address_map_constructor(FUNC(acorn_vidc1a_device::regs_map), this));
	m_pal_4bpp_base = 0x100;
	m_pal_cursor_base = 0x10;
	m_pal_border_base = 0x110;
}

device_memory_interface::space_config_vector acorn_vidc10_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_IO, &m_space_config)
	};
}


//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration additions
//-------------------------------------------------

void acorn_vidc10_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, m_lspeaker).front_left();
	SPEAKER(config, m_rspeaker).front_right();
	for (int i = 0; i < m_sound_max_channels; i++)
	{
		// custom DAC
		DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_dac[i], 0).add_route(0, m_lspeaker, m_sound_input_gain).add_route(0, m_rspeaker, m_sound_input_gain);
	}
}

u32 acorn_vidc10_device::palette_entries() const noexcept
{
	return 0x100+0x10+4; // 8bpp + 1/2/4bpp + 2bpp for cursor
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void acorn_vidc10_device::device_config_complete()
{
	if (!has_screen())
		return;

	if (!screen().refresh_attoseconds())
		screen().set_raw(clock() * 2 / 3, 1024,0,735, 624/2,0,292); // RiscOS 3 default screen settings

	if (!screen().has_screen_update())
		screen().set_screen_update(*this, FUNC(acorn_vidc10_device::screen_update));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acorn_vidc10_device::device_start()
{
	for (int i = 0; i < entries(); i++)
		set_pen_color(i, rgb_t::black());

	save_item(NAME(m_bpp_mode));
	save_item(NAME(m_crtc_interlace));
	save_item(NAME(m_pixel_clock));
	save_item(NAME(m_sound_frequency_latch));
	save_item(NAME(m_sound_frequency_test_bit));
	save_item(NAME(m_cursor_enable));
	save_pointer(NAME(m_crtc_regs), CRTC_VCER+1);
	save_pointer(NAME(m_crtc_raw_horz), 2);
	m_data_vram = make_unique_clear<u8[]>(m_data_vram_size);
	m_cursor_vram = make_unique_clear<u8[]>(m_cursor_vram_size);
	save_pointer(NAME(m_data_vram), m_data_vram_size);
	save_pointer(NAME(m_cursor_vram), m_cursor_vram_size);
	save_pointer(NAME(m_stereo_image), m_sound_max_channels);

	m_video_timer = timer_alloc(FUNC(acorn_vidc10_device::vblank_timer), this);
	m_sound_timer = timer_alloc(FUNC(acorn_vidc10_device::sound_drq_timer), this);

	// generate u255 law lookup table
	// cfr. page 48 of the VIDC20 manual, page 33 of the VIDC manual
	for (int rawval = 0; rawval < 256; rawval++)
	{
		u8 chord, point;
		bool sign;
		if (m_dac_type == 1)
		{
			chord = (rawval & 0x70) >> 4;
			point = rawval & 0x0f;
			sign = rawval >> 7;
		}
		else
		{
			chord = rawval >> 5;
			point = (rawval & 0x1e) >> 1;
			sign = rawval & 1;
		}
		int16_t result = ((16+point)<<chord)-16;

		if (sign)
			result = -result;

		m_ulaw_lookup[rawval] = result*8;
	}

	// saved for debugging purposes
	save_pointer(NAME(m_ulaw_lookup), 256);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void acorn_vidc10_device::device_reset()
{
	m_cursor_enable = false;
	memset(m_stereo_image, 4, m_sound_max_channels);
	for (int ch = 0; ch < m_sound_max_channels; ch++)
		refresh_stereo_image(ch);
	m_video_timer->adjust(attotime::never);
	m_sound_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(acorn_vidc10_device::vblank_timer)
{
	m_vblank_cb(ASSERT_LINE);
	screen_vblank_line_update();
}

TIMER_CALLBACK_MEMBER(acorn_vidc10_device::sound_drq_timer)
{
	m_sound_drq_cb(ASSERT_LINE);
}

//**************************************************************************
//  CRTC section
//**************************************************************************

inline void acorn_vidc10_device::screen_vblank_line_update()
{
	int vline = (m_crtc_regs[CRTC_VDER]) * (m_crtc_interlace + 1);
	m_video_timer->adjust((vline > 2) ? screen().time_until_pos(vline) : attotime::never);
}

u32 acorn_vidc10_device::get_pixel_clock()
{
	const int32_t pixel_rate[4] = { 8000000, 12000000, 16000000, 24000000};
	return pixel_rate[m_pixel_clock];
}

inline void acorn_vidc10_device::screen_dynamic_res_change()
{
	const u32 pixel_clock = get_pixel_clock();

	// sanity checks
	if (m_crtc_regs[CRTC_HCR] <= 1 || m_crtc_regs[CRTC_VCR] <= 1)
		return;

	if (m_crtc_regs[CRTC_HBER] <= 1 || m_crtc_regs[CRTC_VBER] <= 1)
		return;

	//  total cycles >= border end >= border start
	if (m_crtc_regs[CRTC_HCR] < m_crtc_regs[CRTC_HBER])
		return;

	if (m_crtc_regs[CRTC_HBER] < m_crtc_regs[CRTC_HBSR])
		return;

	if (m_crtc_regs[CRTC_VBER] < m_crtc_regs[CRTC_VBSR])
		return;

	rectangle const visarea(
			0, m_crtc_regs[CRTC_HBER] - m_crtc_regs[CRTC_HBSR] - 1,
			0, (m_crtc_regs[CRTC_VBER] - m_crtc_regs[CRTC_VBSR]) * (m_crtc_interlace + 1));

#if 0
	// TODO: move to debugger custom command
	const int m_vidc_vblank_time = m_crtc_regs[CRTC_VDER] * (m_crtc_interlace+1);
	printf("Configuring: htotal %d vtotal %d border %d x %d display origin %d x %d vblank = %d\n",
		m_crtc_regs[CRTC_HCR], m_crtc_regs[CRTC_VCR],
		visarea.right(), visarea.bottom(),
		m_crtc_regs[CRTC_HDER]-m_crtc_regs[CRTC_HDSR],m_crtc_regs[CRTC_VDER]-m_crtc_regs[CRTC_VDSR]+1,
		m_vidc_vblank_time);
#endif

	attoseconds_t const refresh = HZ_TO_ATTOSECONDS(pixel_clock) * m_crtc_regs[CRTC_HCR] * m_crtc_regs[CRTC_VCR];

	screen().configure(m_crtc_regs[CRTC_HCR], m_crtc_regs[CRTC_VCR] * (m_crtc_interlace+1), visarea, refresh);
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

void acorn_vidc10_device::write(offs_t offset, u32 data, u32 mem_mask)
{
	// TODO: check against mem_mask not 32-bit wide
	u8 reg = data >> 24;
	u32 val = data & 0xffffff;

	this->space(AS_IO).write_dword(reg, val);
}

inline void acorn_vidc10_device::update_4bpp_palette(u16 index, u32 paldata)
{
	// TODO: for TV Tuner we need to output this, also check if cursor mode actually sets this up for offset = 0
//  i = (paldata & 0x1000) >> 12; //supremacy bit
	int b = (paldata & 0x0f00) >> 8;
	int g = (paldata & 0x00f0) >> 4;
	int r = (paldata & 0x000f) >> 0;

	set_pen_color(index, pal4bit(r), pal4bit(g), pal4bit(b) );
	screen().update_partial(screen().vpos());
}

void acorn_vidc10_device::pal_data_display_w(offs_t offset, u32 data)
{
	update_4bpp_palette(offset + 0x100, data);
	//printf("%02x: %01x %01x %01x [%d]\n",offset,r,g,b,screen().vpos());

	// 8bpp
	for (int idx = 0; idx < 0x100; idx += 0x10)
	{
		int b = ((data & 0x700) >> 8) | ((idx & 0x80) >> 4);
		int g = ((data & 0x030) >> 4) | ((idx & 0x60) >> 3);
		int r = ((data & 0x007) >> 0) | ((idx & 0x10) >> 1);

		set_pen_color(offset + idx, pal4bit(r), pal4bit(g), pal4bit(b) );
	}
}

void acorn_vidc10_device::pal_data_cursor_w(offs_t offset, u32 data)
{
	update_4bpp_palette(offset+0x110, data);
}

void acorn_vidc10_device::control_w(u32 data)
{
	// TODO: not sure what the commented out bits do
	m_pixel_clock = (data & 0x03);
	m_bpp_mode = ((data & 0x0c) >> 2);
	//m_dma_request_mode = ((data & 0x30) >> 4);
	m_crtc_interlace = ((data & 0x40) >> 6);
	//m_composite_sync = BIT(data, 7);
	//m_test_mode = (data & 0xc100) != 0xc100;

	// TODO: vga/svga modes sets 0x1000?
	m_crtc_regs[CRTC_HDSR] = convert_crtc_hdisplay(0);
	m_crtc_regs[CRTC_HDER] = convert_crtc_hdisplay(1);
	screen_vblank_line_update();
	screen_dynamic_res_change();
}

inline u32 acorn_vidc10_device::convert_crtc_hdisplay(u8 index)
{
	const u8 x_step[4] = { 19, 11, 7, 5 };
	return (m_crtc_raw_horz[index]*2)+x_step[m_bpp_mode];
}

void acorn_vidc10_device::crtc_w(offs_t offset, u32 data)
{
	switch(offset)
	{
		case CRTC_HCR:  m_crtc_regs[CRTC_HCR] =  ((data >> 14)<<1)+2;       break;
//      case CRTC_HSWR: m_crtc_regs[CRTC_HSWR] = (data >> 14)+1;            break;
		case CRTC_HBSR: m_crtc_regs[CRTC_HBSR] = ((data >> 14)<<1)+1;       break;
		case CRTC_HDSR:
			m_crtc_raw_horz[0] = (data >> 14);
			m_crtc_regs[CRTC_HDSR] = convert_crtc_hdisplay(0);
			break;
		case CRTC_HDER:
			m_crtc_raw_horz[1] = (data >> 14);
			m_crtc_regs[CRTC_HDER] = convert_crtc_hdisplay(1);
			break;
		case CRTC_HBER: m_crtc_regs[CRTC_HBER] = ((data >> 14)<<1)+1;       break;
		case CRTC_HCSR: m_crtc_regs[CRTC_HCSR] = ((data >> 13) & 0x7ff) + 6; return;
//      case CRTC_HIR: // ...

		case CRTC_VCR:  m_crtc_regs[CRTC_VCR] = (data >> 14)+1;             break;
		case CRTC_VSWR: m_crtc_regs[CRTC_VSWR] = (data >> 14)+1;            break;
		case CRTC_VBSR:
			m_crtc_regs[CRTC_VBSR] = (data >> 14)+1;
			break;
		case CRTC_VDSR:
			m_crtc_regs[CRTC_VDSR] = (data >> 14)+1;
			break;
		case CRTC_VDER:
			m_crtc_regs[CRTC_VDER] = (data >> 14)+1;
			screen_vblank_line_update();
			break;
		case CRTC_VBER:
			m_crtc_regs[CRTC_VBER] = (data >> 14)+1;
			break;
		case CRTC_VCSR: m_crtc_regs[CRTC_VCSR] = ((data >> 14) & 0x3ff) + 1; return;
		case CRTC_VCER: m_crtc_regs[CRTC_VCER] = ((data >> 14) & 0x3ff) + 1; return;
	}

	screen_dynamic_res_change();
}

inline void acorn_vidc10_device::refresh_stereo_image(u8 channel)
{
	/*
	    -111 full right
	    -110 83% right, 17% left
	    -101 67% right, 33% left
	    -100 center
	    -011 67% left, 33% right
	    -010 83% left, 17% right
	    -001 full left
	    -000 "undefined" TODO: verify what it actually means
	*/
	const float left_gain[8] = { 1.0f, 2.0f, 1.66f, 1.34f, 1.0f, 0.66f, 0.34f, 0.0f };
	const float right_gain[8] = { 1.0f, 0.0f, 0.34f, 0.66f, 1.0f, 1.34f, 1.66f, 2.0f };

	m_lspeaker->set_input_gain(channel, left_gain[m_stereo_image[channel]] * m_sound_input_gain);
	m_rspeaker->set_input_gain(channel, right_gain[m_stereo_image[channel]] * m_sound_input_gain);
	//printf("%d %f %f\n",channel,m_lspeaker->input(channel).gain(),m_rspeaker->input(channel).gain());
}


void acorn_vidc10_device::stereo_image_w(offs_t offset, u32 data)
{
	u8 channel = (offset + 7) & 0x7;
	m_stereo_image[channel] = data & 0x7;
	refresh_stereo_image(channel);
}

void acorn_vidc10_device::sound_frequency_w(u32 data)
{
	m_sound_frequency_test_bit = BIT(data, 8);
	m_sound_frequency_latch = data & 0xff;
	if (m_sound_mode == true)
		refresh_sound_frequency();
}

//**************************************************************************
//  MEMC comms
//**************************************************************************

void acorn_vidc10_device::write_dac(u8 channel, u8 data)
{
	int16_t res = m_ulaw_lookup[data];
	m_dac[channel & 7]->write(res);
}

void acorn_vidc10_device::refresh_sound_frequency()
{
	// TODO: check against test bit (reloads sound frequency if 0)
	if (m_sound_mode == true)
	{
		// TODO: Range is between 3 and 256 usecs
		double sndhz = 1e6 / ((m_sound_frequency_latch & 0xff) + 2);
		sndhz /= get_dac_mode() == true ? 2.0 : 8.0;
		m_sound_timer->adjust(attotime::zero, 0, attotime::from_hz(sndhz));
		//printf("VIDC: audio DMA start, sound freq %d, sndhz = %f\n", (m_crtc_regs[0xc0] & 0xff)-2, sndhz);
	}
	else
		m_sound_timer->adjust(attotime::never);
}

//**************************************************************************
//  Screen Update / VBlank / HBlank
//**************************************************************************

void acorn_vidc10_device::draw(bitmap_rgb32 &bitmap, const rectangle &cliprect, u8 *vram, u8 bpp, int xstart, int ystart, int xsize, int ysize, bool is_cursor)
{
	const u16 pen_base = (bpp == 3 ? 0 : m_pal_4bpp_base) + (is_cursor == true ? m_pal_cursor_base : 0);
	const u16 pen_masks[4] = { 1, 3, 0xf, 0xff };
	const u16 pen_mask = pen_masks[bpp];
	const u16 xchar_size = 1 << (3 - bpp);
	const u8 pen_byte_sizes[4] = { 1, 2, 4, 1 };
	const u16 pen_byte_size = pen_byte_sizes[bpp];
	const int raster_ystart = std::max(0, cliprect.min_y-ystart);
	xsize >>= 3-bpp;

	//printf("%d %d %d %d\n",ystart, ysize, cliprect.min_y, cliprect.max_y);

	for (int srcy = raster_ystart; srcy < ysize; srcy++)
	{
		int dsty = (srcy + ystart) * (m_crtc_interlace+1);
		for (int srcx = 0; srcx < xsize; srcx++)
		{
			u8 pen = vram[srcx + srcy * xsize];
			int dstx = (srcx*xchar_size) + xstart;

			for (int xi = 0; xi < xchar_size; xi++)
			{
				u16 dot = (pen >> (xi * pen_byte_size)) & pen_mask;
				if (is_cursor == true && dot == 0)
					continue;
				dot += pen_base;
				bitmap.pix(dsty, dstx+xi) = this->pen(dot);
				if (m_crtc_interlace)
					bitmap.pix(dsty+1, dstx+xi) = this->pen(dot);
			}
		}
	}
}

u32 acorn_vidc10_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* border color */
	bitmap.fill(pen(m_pal_border_base), cliprect);

	/* define X display area through BPP mode register */
	int calc_dxs = m_crtc_regs[CRTC_HDSR];
	int calc_dxe = m_crtc_regs[CRTC_HDER];

	/* now calculate display clip rectangle start/end areas */
	int xstart = (calc_dxs)-m_crtc_regs[CRTC_HBSR];
	int ystart = (m_crtc_regs[CRTC_VDSR]-m_crtc_regs[CRTC_VBSR]);
	int xend = (calc_dxe)+xstart;
	int yend = (m_crtc_regs[CRTC_VDER] * (m_crtc_interlace+1))+ystart;

	/* disable the screen if display params are invalid */
	if(xstart > xend || ystart > yend)
		return 0;

	int xsize = calc_dxe-calc_dxs;
	int ysize = m_crtc_regs[CRTC_VDER]-m_crtc_regs[CRTC_VDSR];

	if (xsize <= 0 || ysize <= 0)
		return 0;

	draw(bitmap, cliprect, m_data_vram.get(), m_bpp_mode, xstart, ystart, xsize, ysize, false);
	if (m_cursor_enable == true)
	{
		xstart = m_crtc_regs[CRTC_HCSR] - m_crtc_regs[CRTC_HBSR];
		ystart = m_crtc_regs[CRTC_VCSR] - m_crtc_regs[CRTC_VBSR];
		xsize = 32;
		ysize = m_crtc_regs[CRTC_VCER] - m_crtc_regs[CRTC_VCSR];
		if (ysize > 0)
			draw(bitmap, cliprect, m_cursor_vram.get(), 1, xstart, ystart, xsize, ysize, true);
	}

	return 0;
}

int acorn_vidc10_device::flyback_r()
{
	int vert_pos = screen().vpos();
	if (vert_pos <= m_crtc_regs[CRTC_VDSR] * (m_crtc_interlace+1))
		return true;

	if (vert_pos >= m_crtc_regs[CRTC_VDER] * (m_crtc_interlace+1))
		return true;

	return false;
}

/*
 *
 * VIDC20 overrides
 *
 */

void arm_vidc20_device::regs_map(address_map &map)
{
	map(0x00, 0x0f).w(FUNC(arm_vidc20_device::vidc20_pal_data_display_w));
	map(0x10, 0x1f).w(FUNC(arm_vidc20_device::vidc20_pal_data_index_w));
	map(0x40, 0x7f).w(FUNC(arm_vidc20_device::vidc20_pal_data_cursor_w));
	map(0x80, 0x9f).w(FUNC(arm_vidc20_device::vidc20_crtc_w));
//  map(0xa0, 0xa7) stereo image
	map(0xb0, 0xb0).w(FUNC(arm_vidc20_device::vidc20_sound_frequency_w));
	map(0xb1, 0xb1).w(FUNC(arm_vidc20_device::vidc20_sound_control_w));
	map(0xd0, 0xdf).w(FUNC(arm_vidc20_device::fsynreg_w));
	map(0xe0, 0xef).w(FUNC(arm_vidc20_device::vidc20_control_w));
}

arm_vidc20_device::arm_vidc20_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: acorn_vidc10_device(mconfig, ARM_VIDC20, tag, owner, clock, 2)
	, m_pixel_source(0)
	, m_pixel_rate(0)
	, m_dac32(*this, "serial_dac_%u", 0)
{
	m_space_config = address_space_config("regs_space", ENDIANNESS_LITTLE, 32, 8, -2, address_map_constructor(FUNC(arm_vidc20_device::regs_map), this));
	m_pal_4bpp_base = 0x000;
	m_pal_cursor_base = 0x100;
	m_pal_border_base = 0x100;
}


void arm_vidc20_device::device_add_mconfig(machine_config &config)
{
	acorn_vidc10_device::device_add_mconfig(config);

	// FIXME: disable DACs for the time being
	// so that it won't mixin with QS1000 source for ssfindo.cpp games
	for (int i = 0; i < m_sound_max_channels; i++)
	{
		m_dac[i]->reset_routes();
		m_dac[i]->add_route(0, m_lspeaker, 0.0);
		m_dac[i]->add_route(0, m_rspeaker, 0.0);
	}

	// For simplicity we separate DACs for 32-bit mode
	// TODO: how stereo image copes with this if at all?
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_dac32[0], 0).add_route(ALL_OUTPUTS, m_lspeaker, 0.25);
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_dac32[1], 0).add_route(ALL_OUTPUTS, m_rspeaker, 0.25);
}

void arm_vidc20_device::device_config_complete()
{
	if (!has_screen())
		return;

	if (!screen().refresh_attoseconds())
		screen().set_raw(clock() * 2 / 3, 1024,0,735, 624/2,0,292); // RiscOS 3 default screen settings

	if (!screen().has_screen_update())
		screen().set_screen_update(*this, FUNC(arm_vidc20_device::screen_update));
}

u32 arm_vidc20_device::palette_entries() const noexcept
{
	return 0x100+4; // 8bpp + 2bpp for cursor
}

void arm_vidc20_device::device_start()
{
	acorn_vidc10_device::device_start();

	save_item(NAME(m_vco_r_modulo));
	save_item(NAME(m_vco_v_modulo));
	save_item(NAME(m_pal_data_index));
	save_item(NAME(m_dac_serial_mode));
	save_item(NAME(m_pixel_source));
	save_item(NAME(m_pixel_rate));
}

void arm_vidc20_device::device_reset()
{
	acorn_vidc10_device::device_reset();

	// TODO: sensible defaults
	m_vco_r_modulo = 1;
	m_vco_v_modulo = 1;

	// make sure DACs don't output any undefined behaviour for now
	// (will cause wild DC offset in ssfindo.cpp games)
	for (int ch = 0; ch < 8; ch ++)
		write_dac(ch, 0);

	write_dac32(0, 0);
	write_dac32(1, 0);
}

inline void arm_vidc20_device::refresh_stereo_image(u8 channel)
{
	// TODO: set_input_gain hampers with both QS1000 and serial DAC mode
	// Best option is to move the legacy DAC handling into a separate device,
	// make it proper 8 channel output while clients are responsible of mix-ins
}


inline void arm_vidc20_device::update_8bpp_palette(u16 index, u32 paldata)
{
	// TODO: ext hookup, supremacy plus other stuff according to the manual
//  ext = (paldata & 0x0f000000) >> 24;
	int b =   (paldata & 0x00ff0000) >> 16;
	int g =   (paldata & 0x0000ff00) >> 8;
	int r =   (paldata & 0x000000ff) >> 0;

	set_pen_color(index, r, g, b );
	screen().update_partial(screen().vpos());
}

void arm_vidc20_device::vidc20_pal_data_display_w(offs_t offset, u32 data)
{
	u8 ext_data = offset & 0xf;
	update_8bpp_palette(m_pal_data_index, (ext_data<<24) | data);
	m_pal_data_index ++;
	m_pal_data_index &= 0xff;
}

void arm_vidc20_device::vidc20_pal_data_index_w(u32 data)
{
	m_pal_data_index = data & 0xff;
}

void arm_vidc20_device::vidc20_pal_data_cursor_w(offs_t offset, u32 data)
{
	u8 ext_data = offset & 0xf;
	u8 cursor_pal_index = (offset >> 4) & 3;
	update_8bpp_palette(m_pal_cursor_base + cursor_pal_index, (ext_data<<24) | data);
}

u32 arm_vidc20_device::get_pixel_clock()
{
	// RCLK source: passes thru a r-modulus and a phase frequency (PCOMP), the full story is interesting if you're into maths.
	// TODO: for now we just multiply source clock by 2, enough for ssfindo.cpp games.
	//printf("%d %02x %02x %d %d\n",this->clock(), 1 << m_pixel_rate, m_pixel_source, m_vco_v_modulo, m_vco_r_modulo);
	if (m_pixel_source == 2) // RCLK
		return (this->clock() << 1) >> m_pixel_rate;

	// VCLK source is just an external connection
	// TODO: get clock from outside world, understand how the modulos are really used,
	//       understand if SW do some VCO testing before setting CRTC params,
	//       if there isn't a monitor ID mechanism that copes with this
	if (m_pixel_source == 0) // VCLK
		return (25175000);

	throw emu_fatalerror("%s unhandled pixel source %02x selected",this->tag(), m_pixel_source);
}

void arm_vidc20_device::vidc20_crtc_w(offs_t offset, u32 data)
{
	if (offset & 0x8)
		throw emu_fatalerror("%s accessing CRTC test register %02x, please call the ambulance",this->tag(),offset+0x80);

	const u8 crtc_offset = (offset & 0x7) | ((offset & 0x10) >> 1);

	switch(crtc_offset)
	{
		case CRTC_HCR:  m_crtc_regs[CRTC_HCR] = (data&0x7ffc) + 8; break;
		case CRTC_HSWR: m_crtc_regs[CRTC_HSWR] = (data&0x7ffe) + 8; break;
		case CRTC_HBSR: m_crtc_regs[CRTC_HBSR] = (data&0x7ffe) + 12; break;
		case CRTC_HDSR: m_crtc_regs[CRTC_HDSR] = (data&0x7ffe) + 18; break;
		case CRTC_HDER: m_crtc_regs[CRTC_HDER] = (data&0x7ffe) + 18; break;
		case CRTC_HBER: m_crtc_regs[CRTC_HBER] = (data&0x7ffe) + 12; break;
		case CRTC_HCSR: m_crtc_regs[CRTC_HCSR] = (data&0x7fff) + 17; return;
//      case CRTC_HIR:
		case CRTC_VCR:  m_crtc_regs[CRTC_VCR] = (data&0x3fff) + 2; break;
		case CRTC_VSWR: m_crtc_regs[CRTC_VSWR] = (data&0x3fff) + 1; break;
		case CRTC_VBSR: m_crtc_regs[CRTC_VBSR] = (data&0x3fff) + 1; break;
		case CRTC_VDSR: m_crtc_regs[CRTC_VDSR] = (data&0x3fff) + 1; break;
		case CRTC_VDER:
			m_crtc_regs[CRTC_VDER] = (data&0x3fff) + 1;
			screen_vblank_line_update();
			break;
		case CRTC_VBER: m_crtc_regs[CRTC_VBER] = (data&0x3fff) + 1; break;
		// TODO: bits 15-14 specific for duplex LCD mode
		case CRTC_VCSR:
			m_crtc_regs[CRTC_VCSR] = (data&0x3fff) + 1;
			return;
		case CRTC_VCER: m_crtc_regs[CRTC_VCER] = (data&0x3fff) + 1; return;
	}

	screen_dynamic_res_change();
}

void arm_vidc20_device::fsynreg_w(u32 data)
{
	m_vco_r_modulo = data & 0x3f;
	m_vco_v_modulo = (data >> 8) & 0x3f;
	// bits 15-14 and 7-6 are test bits

	screen_dynamic_res_change();
}

void arm_vidc20_device::vidc20_control_w(u32 data)
{
	// ---- --00: VCLK
	// ---- --01: HCLK
	// ---- --10: RCLK ("recommended" 24 MHz)
	// ---- --11: undefined, probably same as RCLK
	m_pixel_source = data & 3;
	m_pixel_rate = (data & 0x1c) >> 2;
	// (data & 0x700) >> 8 FIFO load
	// BIT(data, 13) enables Duplex LCD mode
	// BIT(data, 14) power down
	// (data & 0xf0000) >> 16 test mode
	m_bpp_mode = (data & 0xe0) >> 5;
	m_crtc_interlace = BIT(data, 12);

	screen_vblank_line_update();
	screen_dynamic_res_change();
}

void arm_vidc20_device::vidc20_sound_control_w(u32 data)
{
	// TODO: VIDC10 mode, ext clock bit 0
	m_dac_serial_mode = BIT(data, 1);

	if (m_dac_serial_mode)
	{
		m_dac32[0]->set_output_gain(0, 1.0);
		m_dac32[1]->set_output_gain(0, 1.0);

		for (int ch = 0; ch < m_sound_max_channels; ch++)
			m_dac[ch]->set_output_gain(0, 0.0);
	}
	else
	{
		m_dac32[0]->set_output_gain(0, 0.0);
		m_dac32[1]->set_output_gain(0, 0.0);

		for (int ch = 0; ch < m_sound_max_channels; ch++)
		{
			//m_dac[ch]->set_output_gain(0, 0.0);
			refresh_stereo_image(ch);
		}
	}
}

void arm_vidc20_device::vidc20_sound_frequency_w(u32 data)
{
	m_sound_frequency_latch = data & 0xff;
	if (m_sound_mode == true)
		refresh_sound_frequency();
}

void arm_vidc20_device::write_dac32(u8 channel, u16 data)
{
	m_dac32[channel & 1]->write(data);
}

bool arm_vidc20_device::get_dac_mode()
{
	return m_dac_serial_mode;
}

u32 arm_vidc20_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// TODO: support for true color modes
	return acorn_vidc10_device::screen_update(screen, bitmap, cliprect);
}
