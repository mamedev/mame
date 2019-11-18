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
    - improve sound DAC writes;
    - subclass this for VIDC20 emulation (RiscPC);
    - Are CRTC values correct? VGA modes have a +1 in display line;

**********************************************************************************************/

#include "emu.h"
#include "acorn_vidc.h"
#include "screen.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(ACORN_VIDC10, acorn_vidc10_device, "acorn_vidc10", "Acorn VIDC10")
DEFINE_DEVICE_TYPE(ACORN_VIDC10_LCD, acorn_vidc10_lcd_device, "acorn_vidc10_lcd", "Acorn VIDC10 with LCD monitor")


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


acorn_vidc10_device::acorn_vidc10_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_palette_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, m_space_config("regs_space", ENDIANNESS_LITTLE, 32, 8, 0, address_map_constructor(FUNC(acorn_vidc10_device::regs_map), this))
	, m_lspeaker(*this, "lspeaker")
	, m_rspeaker(*this, "rspeaker")
	, m_dac(*this, "dac%u", 0)
	, m_vblank_cb(*this)
	, m_sound_drq_cb(*this)
{
}

acorn_vidc10_device::acorn_vidc10_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: acorn_vidc10_device(mconfig, ACORN_VIDC10, tag, owner, clock)
{
}


acorn_vidc10_lcd_device::acorn_vidc10_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: acorn_vidc10_device(mconfig, ACORN_VIDC10_LCD, tag, owner, clock)
{
}

device_memory_interface::space_config_vector acorn_vidc10_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_IO, &m_space_config)
	};
}


//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------

void acorn_vidc10_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, m_lspeaker).front_left();
	SPEAKER(config, m_rspeaker).front_right();
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref", 0));
	for (int i = 0; i < m_sound_max_channels; i++)
	{
		// custom DAC
		DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_dac[i], 0).add_route(0, m_lspeaker, m_sound_input_gain).add_route(0, m_rspeaker, m_sound_input_gain);
		vref.add_route(0, m_dac[i], 1.0, DAC_VREF_POS_INPUT); vref.add_route(0, m_dac[i], -1.0, DAC_VREF_NEG_INPUT);
	}
}

void acorn_vidc10_lcd_device::device_add_mconfig(machine_config &config)
{
	acorn_vidc10_device::device_add_mconfig(config);
	// TODO: verify !Configure with automatic type detection, there must be an ID telling this is a LCD machine.
}

uint32_t acorn_vidc10_device::palette_entries() const
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
	m_vblank_cb.resolve_safe();
	m_sound_drq_cb.resolve_safe();

	for (int i = 0; i < entries(); i++)
		set_pen_color(i, rgb_t::black());

	save_item(NAME(m_bpp_mode));
	save_item(NAME(m_crtc_interlace));
	save_item(NAME(m_pixel_clock));
	save_item(NAME(m_sound_frequency_latch));
	save_item(NAME(m_sound_frequency_test_bit));
	save_item(NAME(m_cursor_enable));
	save_pointer(NAME(m_crtc_regs), CRTC_VCER+1);
	m_data_vram = auto_alloc_array_clear(machine(), u8, m_data_vram_size);
	m_cursor_vram = auto_alloc_array_clear(machine(), u8, m_cursor_vram_size);
	save_pointer(NAME(m_data_vram), m_data_vram_size);
	save_pointer(NAME(m_cursor_vram), m_cursor_vram_size);
	save_pointer(NAME(m_stereo_image), m_sound_max_channels);

	m_video_timer = timer_alloc(TIMER_VIDEO);
	m_sound_timer = timer_alloc(TIMER_SOUND);

	// generate u255 law lookup table
	// cfr. page 48 of the VIDC20 manual, page 33 of the VIDC manual
	// TODO: manual mentions a format difference between VIDC10 revisions
	for (int rawval = 0; rawval < 256; rawval++)
	{
		uint8_t chord = rawval >> 5;
		uint8_t point = (rawval & 0x1e) >> 1;
		bool sign = rawval & 1;
		int16_t result = ((16+point)<<chord)-16;

		if (sign)
			result = -result;

		m_ulaw_lookup[rawval] = result*8;
	}
	save_pointer(NAME(m_ulaw_lookup), 256);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void acorn_vidc10_device::device_reset()
{
	m_cursor_enable = false;
	memset(m_data_vram, 0, m_data_vram_size);
	memset(m_cursor_vram, 0, m_cursor_vram_size);
	memset(m_stereo_image, 4, m_sound_max_channels);
	for (int ch=0;ch<m_sound_max_channels;ch++)
		refresh_stereo_image(ch);
	m_video_timer->adjust(attotime::never);
	m_sound_timer->adjust(attotime::never);
}

//-------------------------------------------------
//  device_timer - device-specific timer
//-------------------------------------------------

void acorn_vidc10_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_VIDEO:
			m_vblank_cb(ASSERT_LINE);
			screen_vblank_line_update();
			break;
		case TIMER_SOUND:
			m_sound_drq_cb(ASSERT_LINE);
			break;
	}
}

//**************************************************************************
//  CRTC section
//**************************************************************************

inline void acorn_vidc10_device::screen_vblank_line_update()
{
	int vline = (m_crtc_regs[CRTC_VDER]) * (m_crtc_interlace + 1);
	m_video_timer->adjust((vline > 2) ? screen().time_until_pos(vline) : attotime::never);
}

void acorn_vidc10_device::screen_dynamic_res_change()
{
	const int32_t pixel_rate[4] = { 8000000, 12000000, 16000000, 24000000};

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

	attoseconds_t const refresh = HZ_TO_ATTOSECONDS(pixel_rate[m_pixel_clock]) * m_crtc_regs[CRTC_HCR] * m_crtc_regs[CRTC_VCR];

	screen().configure(m_crtc_regs[CRTC_HCR], m_crtc_regs[CRTC_VCR] * (m_crtc_interlace+1), visarea, refresh);
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE32_MEMBER( acorn_vidc10_device::write )
{
	// TODO: check against mem_mask not 32-bit wide
	uint8_t reg = data >> 24;
	uint32_t val = data & 0xffffff;

	this->space(AS_IO).write_dword(reg, val);
}

inline void acorn_vidc10_device::update_4bpp_palette(uint16_t index, uint32_t paldata)
{
	int r,g,b;

	// TODO: for TV Tuner we need to output this, also check if cursor mode actually sets this up for offset = 0
//  i = (paldata & 0x1000) >> 12; //supremacy bit
	b = (paldata & 0x0f00) >> 8;
	g = (paldata & 0x00f0) >> 4;
	r = (paldata & 0x000f) >> 0;

	set_pen_color(index, pal4bit(r), pal4bit(g), pal4bit(b) );
	screen().update_partial(screen().vpos());
}

WRITE32_MEMBER( acorn_vidc10_device::pal_data_display_w )
{
	update_4bpp_palette(offset+0x100, data);
	//printf("%02x: %01x %01x %01x [%d]\n",offset,r,g,b,screen().vpos());

	// 8bpp
	for(int idx=0;idx<0x100;idx+=0x10)
	{
		int b = ((data & 0x700) >> 8) | ((idx & 0x80) >> 4);
		int g = ((data & 0x030) >> 4) | ((idx & 0x60) >> 3);
		int r = ((data & 0x007) >> 0) | ((idx & 0x10) >> 1);

		set_pen_color(offset + idx, pal4bit(r), pal4bit(g), pal4bit(b) );
	}
}

WRITE32_MEMBER( acorn_vidc10_device::pal_data_cursor_w )
{
	update_4bpp_palette(offset+0x110, data);
}

WRITE32_MEMBER( acorn_vidc10_device::control_w )
{
	// TODO: not sure what the commented out bits do
	m_pixel_clock = (data & 0x03);
	m_bpp_mode = ((data & 0x0c) >> 2);
	//m_dma_request_mode = ((data & 0x30) >> 4);
	m_crtc_interlace = ((data & 0x40) >> 6);
	//m_composite_sync = BIT(data, 7);
	//m_test_mode = (data & 0xc100) != 0xc100;

	//todo: vga/svga modes sets 0x1000?
	screen_vblank_line_update();
	screen_dynamic_res_change();
}

WRITE32_MEMBER( acorn_vidc10_device::crtc_w )
{
	switch(offset)
	{
		case CRTC_HCR:  m_crtc_regs[CRTC_HCR] =  ((data >> 14)<<1)+2;       break;
//      case CRTC_HSWR: m_crtc_regs[CRTC_HSWR] = (data >> 14)+1;            break;
		case CRTC_HBSR: m_crtc_regs[CRTC_HBSR] = ((data >> 14)<<1)+1;       break;
		case CRTC_HDSR: m_crtc_regs[CRTC_HDSR] = (data >> 14);              break;
		case CRTC_HDER: m_crtc_regs[CRTC_HDER] = (data >> 14);              break;
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

inline void acorn_vidc10_device::refresh_stereo_image(uint8_t channel)
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

	m_lspeaker->set_input_gain(channel,left_gain[m_stereo_image[channel]]*m_sound_input_gain);
	m_rspeaker->set_input_gain(channel,right_gain[m_stereo_image[channel]]*m_sound_input_gain);
	//printf("%d %f %f\n",channel,m_lspeaker->input_gain(channel),m_rspeaker->input_gain(channel));
}


WRITE32_MEMBER( acorn_vidc10_device::stereo_image_w )
{
	uint8_t channel = (offset + 7) & 0x7;
	m_stereo_image[channel] = data & 0x7;
	refresh_stereo_image(channel);
}

WRITE32_MEMBER( acorn_vidc10_device::sound_frequency_w )
{
	m_sound_frequency_test_bit = BIT(data, 8);
	m_sound_frequency_latch = data & 0xff;
	if (m_sound_mode == true)
		refresh_sound_frequency();
}

//**************************************************************************
//  MEMC comms
//**************************************************************************

void acorn_vidc10_device::write_dac(uint8_t channel, uint8_t data)
{
	int16_t res;
	res = m_ulaw_lookup[data];
	m_dac[channel & 7]->write(res);
}

void acorn_vidc10_device::refresh_sound_frequency()
{
	// TODO: check against test bit (reloads sound frequency if 0)
	if (m_sound_mode == true)
	{
		// TODO: Range is between 3 and 256 usecs
		double sndhz = 1e6 / ((m_sound_frequency_latch & 0xff) + 2);
		sndhz /= 8.0;
		m_sound_timer->adjust(attotime::zero, 0, attotime::from_hz(sndhz));
		//printf("VIDC: audio DMA start, sound freq %d, sndhz = %f\n", (m_crtc_regs[0xc0] & 0xff)-2, sndhz);
	}
	else
		m_sound_timer->adjust(attotime::never);
}

//**************************************************************************
//  Screen Update / VBlank / HBlank
//**************************************************************************

void acorn_vidc10_device::draw(bitmap_rgb32 &bitmap, const rectangle &cliprect, u8 *vram, uint8_t bpp, int xstart, int ystart, int xsize, int ysize, bool is_cursor)
{
	const u16 pen_base = (bpp == 3 ? 0 : 0x100) + (is_cursor == true ? 0x10 : 0);
	const u16 pen_masks[4] = { 1, 3, 0xf, 0xff };
	const u16 pen_mask = pen_masks[bpp];
	const u16 xchar_size = 1 << (3 - bpp);
	const u8 pen_byte_sizes[4] = { 1, 2, 4, 1 };
	const u16 pen_byte_size = pen_byte_sizes[bpp];
	const int raster_ystart = std::max(0, cliprect.min_y-ystart);
	xsize >>= 3-bpp;

	//printf("%d %d %d %d\n",ystart, ysize, cliprect.min_y, cliprect.max_y);

	for (int srcy = raster_ystart; srcy<ysize; srcy++)
	{
		int dsty = (srcy + ystart)*(m_crtc_interlace+1);
		for (int srcx = 0; srcx<xsize; srcx++)
		{
			u8 pen = vram[srcx + srcy * xsize];
			int dstx = (srcx*xchar_size) + xstart;

			for (int xi=0;xi<xchar_size;xi++)
			{
				u16 dot = ((pen>>(xi*pen_byte_size)) & pen_mask);
				if (is_cursor == true && dot == 0)
					continue;
				dot += pen_base;
				bitmap.pix32(dsty, dstx+xi) = this->pen(dot);
				if (m_crtc_interlace)
					bitmap.pix32(dsty+1, dstx+xi) = this->pen(dot);
			}
		}
	}
}

u32 acorn_vidc10_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int xstart,ystart,xend,yend;
	int xsize,ysize;
	int calc_dxs = 0,calc_dxe = 0;
	const uint8_t x_step[4] = { 19, 11, 7, 5 };

	/* border color */
	bitmap.fill(pen(0x110), cliprect);

	/* define X display area through BPP mode register */
	calc_dxs = (m_crtc_regs[CRTC_HDSR]*2)+x_step[m_bpp_mode & 3];
	calc_dxe = (m_crtc_regs[CRTC_HDER]*2)+x_step[m_bpp_mode & 3];

	/* now calculate display clip rectangle start/end areas */
	xstart = (calc_dxs)-m_crtc_regs[CRTC_HBSR];
	ystart = (m_crtc_regs[CRTC_VDSR]-m_crtc_regs[CRTC_VBSR]);
	xend = (calc_dxe)+xstart;
	yend = (m_crtc_regs[CRTC_VDER] * (m_crtc_interlace+1))+ystart;

	/* disable the screen if display params are invalid */
	if(xstart > xend || ystart > yend)
		return 0;

	xsize = calc_dxe-calc_dxs;
	ysize = m_crtc_regs[CRTC_VDER]-m_crtc_regs[CRTC_VDSR];

	if (xsize <= 0 || ysize <= 0)
		return 0;

	draw(bitmap, cliprect, m_data_vram, m_bpp_mode, xstart, ystart, xsize, ysize, false);
	if (m_cursor_enable == true)
	{
		xstart = m_crtc_regs[CRTC_HCSR] - m_crtc_regs[CRTC_HBSR];
		ystart = m_crtc_regs[CRTC_VCSR] - m_crtc_regs[CRTC_VBSR];
		xsize = 32;
		ysize = m_crtc_regs[CRTC_VCER] - m_crtc_regs[CRTC_VCSR];
		if (ysize > 0)
			draw(bitmap, cliprect, m_cursor_vram, 1, xstart, ystart, xsize, ysize, true);
	}

	return 0;
}

READ_LINE_MEMBER(acorn_vidc10_device::flyback_r )
{
	int vert_pos = screen().vpos();
	bool flyback = (vert_pos <= m_crtc_regs[CRTC_VDSR] || vert_pos >= m_crtc_regs[CRTC_VDER]);
	return flyback;
}
