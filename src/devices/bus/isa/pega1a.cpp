// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    WD Paradise PEGA1A

**********************************************************************/

#include "emu.h"
#include "pega1a.h"

#define LOG_EXT     (1U << 1)

//#define VERBOSE (LOG_EXT)

#include "logmacro.h"

#define LOGEXT(...) LOGMASKED(LOG_EXT, __VA_ARGS__)


static constexpr XTAL MDA_CLOCK(16'257'000);

// black, black, on, bright - the colour and mono monitor palettes name the
// same three greys by different indices
static const uint8_t s_mda_pen[2][4] =
{
	{ 0x00, 0x00, 0x07, 0x3f },
	{ 0x00, 0x00, 0x08, 0x18 }
};

// the 16 CGA colours as indices into the EGA's 64-entry palette
static const uint8_t s_cga_pen[16] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};


// isa8_ega_device keeps these to itself
#define EGA_MODE_GRAPHICS 1
#define EGA_MODE_TEXT     2


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pega1a_device - constructor
//-------------------------------------------------

pega1a_device::pega1a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: isa8_ega_device(mconfig, type, tag, owner, clock)
	, m_crtc_emu(*this, "crtc_emu")
	, m_ext_mode_color(0)
	, m_ext_mode_mono(0)
	, m_herc_control(0)
	, m_plantronics(0)
	, m_ext_3df(0)
	, m_mono_monitor(false)
	, m_mode_control(0)
	, m_color_select(0)
	, m_mode_control_mono(0)
	, m_crtc_index(0)
	, m_emu_x_off(0)
	, m_emu_frame_cnt(0)
	, m_emu_hsync(0)
	, m_emu_vsync(0)
{
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

// The emulation modes need 6845 register semantics, not EGA ones: crtc_ega
// reads R06-R09 as vertical total/overflow/preset row scan/max scan line, so a
// CGA register profile lands in the wrong registers. Hence a second MC6845.
void pega1a_device::device_add_mconfig(machine_config &config)
{
	isa8_ega_device::device_add_mconfig(config);

	// direct-colour: the pen lookup happens in screen_update()
	m_screen->set_screen_update(FUNC(pega1a_device::screen_update));
	m_screen->set_no_palette();

	// Neither CRTC owns the screen; both reach it through a reconfigure
	// callback instead. A CRTC that owns a screen resyncs its frame origin
	// every frame, so with two attached the idle one keeps postponing vblank.
	m_crtc_ega->set_screen(nullptr);
	m_crtc_ega->set_reconfigure_callback(FUNC(pega1a_device::ega_reconfigure));
	m_crtc_ega->set_row_update_callback(FUNC(pega1a_device::ega_update_row));

	MC6845(config, m_crtc_emu, 14.318181_MHz_XTAL / 16);
	m_crtc_emu->set_screen(nullptr);
	m_crtc_emu->set_show_border_area(false);
	m_crtc_emu->set_reconfigure_callback(FUNC(pega1a_device::emu_reconfigure));
	m_crtc_emu->set_char_width(8);
	m_crtc_emu->set_begin_update_callback(FUNC(pega1a_device::emu_begin_update));
	m_crtc_emu->set_update_row_callback(FUNC(pega1a_device::emu_update_row));
	m_crtc_emu->out_hsync_callback().set(FUNC(pega1a_device::emu_hsync_changed));
	m_crtc_emu->out_vsync_callback().set(FUNC(pega1a_device::emu_vsync_changed));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

// isa8_ega_device::device_start() is deliberately not called: it loads a
// generic EGA BIOS out of "user1"/"user2" and installs the 3b0-3df ranges
// unconditionally, both of which are the board's business here.
void pega1a_device::device_start()
{
	if (m_palette != nullptr && !m_palette->started())
		throw device_missing_dependencies();

	set_isa_device();

	set_monitor_palette(false);

	m_vram = make_unique_clear<uint8_t[]>(256 * 1024);

	m_videoram = m_vram.get();
	m_plane[0] = m_videoram + 0x00000;
	m_plane[1] = m_videoram + 0x10000;
	m_plane[2] = m_videoram + 0x20000;
	m_plane[3] = m_videoram + 0x30000;

	ega_save_state();

	save_item(NAME(m_misc_output));
	save_item(NAME(m_ext_mode_color));
	save_item(NAME(m_ext_mode_mono));
	save_item(NAME(m_herc_control));
	save_item(NAME(m_plantronics));
	save_item(NAME(m_ext_3df));
	save_item(NAME(m_mono_monitor));
	save_item(NAME(m_mode_control));
	save_item(NAME(m_color_select));
	save_item(NAME(m_mode_control_mono));
	save_item(NAME(m_crtc_index));
	save_item(NAME(m_palette_lut_2bpp));
	save_item(NAME(m_emu_x_off));
	save_item(NAME(m_emu_frame_cnt));
	save_item(NAME(m_emu_hsync));
	save_item(NAME(m_emu_vsync));

	m_screen->register_screen_bitmap(m_ega_bitmap);
}


//-------------------------------------------------
//  device_post_load - restore derived video state
//-------------------------------------------------

void pega1a_device::device_post_load()
{
	// Rebuild the video mode and font pointers from the restored registers.
	change_mode();
	install_banks();
}


//-------------------------------------------------
//  set_monitor_palette - pen table for the display
//-------------------------------------------------

// A mono monitor gets only video and intensity, so 0x10 - intensity without
// video - is the non-displaying MDA attribute rather than a dim white.
void pega1a_device::set_monitor_palette(bool mono)
{
	for (int i = 0; i < 64; i++)
	{
		uint8_t r, g, b;

		if (mono)
		{
			r = g = b = BIT(i, 3) ? (BIT(i, 4) ? 0xff : 0xaa) : 0x00;
		}
		else
		{
			r = ((i & 0x04) ? 0xaa : 0x00) + ((i & 0x20) ? 0x55 : 0x00);
			g = ((i & 0x02) ? 0xaa : 0x00) + ((i & 0x10) ? 0x55 : 0x00);
			b = ((i & 0x01) ? 0xaa : 0x00) + ((i & 0x08) ? 0x55 : 0x00);
		}

		m_palette->set_pen_color(i, r, g, b);
	}

	m_mono_monitor = mono;
}

uint8_t pega1a_device::mono_pen(int i) const
{
	return s_mda_pen[m_mono_monitor ? 1 : 0][i];
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pega1a_device::device_reset()
{
	isa8_ega_device::device_reset();

	m_ext_mode_color = 0;
	m_ext_mode_mono = 0;
	m_herc_control = 0;
	m_plantronics = 0;
	m_ext_3df = 0;
	m_mode_control = 0;
	m_color_select = 0;
	m_mode_control_mono = 0;
	m_crtc_index = 0;
	m_emu_x_off = 0;
	m_emu_frame_cnt = 0;
	m_emu_hsync = 0;
	m_emu_vsync = 0;
}


//-------------------------------------------------
//  pega_3b0_r/w, pega_3d0_r/w - Paradise extended
//  registers, falling through to the generic EGA
//  decode for everything else
//-------------------------------------------------

// The boot ROM writes 3DB and 3BB within a single Misc Output setting, so
// unlike the CRTC ports both are decoded regardless of Misc Output bit 0.

uint8_t pega1a_device::pega_3b0_r(offs_t offset)
{
	switch (offset)
	{
	case 0x08: case 0x0b: case 0x0f:
		return 0xff;

	case 0x01: case 0x03: case 0x05: case 0x07:
		if (mono_emulation())
			return m_crtc_emu->register_r();
		break;

	case 0x0a:
		if (mono_emulation())
			return mono_status_r();
		break;
	}

	return pc_ega8_3b0_r(offset);
}

void pega1a_device::pega_3b0_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x00: case 0x02: case 0x04: case 0x06:
		m_crtc_index = data & 0x1f;

		if (mono_emulation())
		{
			m_crtc_emu->address_w(data);
			return;
		}
		break;

	case 0x01: case 0x03: case 0x05: case 0x07:
		if (crtc_timing_locked() && m_crtc_index < 0x08)
			return;

		if (mono_emulation())
		{
			m_crtc_emu->register_w(data);
			return;
		}
		break;

	case 0x08:
		mode_control_mono_w(data);
		return;

	case 0x0b:
		ext_mode_mono_w(data);
		return;

	case 0x0f:
		herc_control_w(data);
		return;
	}

	pc_ega8_3b0_w(offset, data);
}

uint8_t pega1a_device::pega_3d0_r(offs_t offset)
{
	switch (offset)
	{
	case 0x08: case 0x09: case 0x0b: case 0x0d: case 0x0f:
		return 0xff;

	case 0x01: case 0x03: case 0x05: case 0x07:
		if (cga_emulation())
			return m_crtc_emu->register_r();
		break;

	case 0x0a:
		if (cga_emulation())
			return cga_status_r();
		break;
	}

	return pc_ega8_3d0_r(offset);
}

void pega1a_device::pega_3d0_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x00: case 0x02: case 0x04: case 0x06:
		m_crtc_index = data & 0x1f;

		if (cga_emulation())
		{
			m_crtc_emu->address_w(data);
			return;
		}
		break;

	case 0x01: case 0x03: case 0x05: case 0x07:
		if (crtc_timing_locked() && m_crtc_index < 0x08)
			return;

		if (cga_emulation())
		{
			m_crtc_emu->register_w(data);
			return;
		}
		break;

	case 0x08:
		mode_control_w(data);
		return;

	case 0x09:
		color_select_w(data);
		return;

	case 0x0b:
		ext_mode_color_w(data);
		return;

	case 0x0d:
		plantronics_w(data);
		return;

	case 0x0f:
		ext_3df_w(data);
		return;
	}

	pc_ega8_3d0_w(offset, data);
}


//-------------------------------------------------
//  extended registers
//-------------------------------------------------

void pega1a_device::ext_mode_color_w(uint8_t data)
{
	LOGEXT("%s: 3DB (Extended Mode Control, colour) <- %02x\n", machine().describe_context(), data);

	const uint8_t old_mode = ext_mode();
	const bool was = cga_emulation();
	m_ext_mode_color = data;

	if (was != cga_emulation())
		update_mode_timing();

	ext_mode_w(old_mode);
}

void pega1a_device::ext_mode_mono_w(uint8_t data)
{
	LOGEXT("%s: 3BB (Extended Mode Control, mono) <- %02x\n", machine().describe_context(), data);

	const uint8_t old_mode = ext_mode();
	const bool was = mono_emulation();
	m_ext_mode_mono = data;

	if (was != mono_emulation())
	{
		update_mode_timing();
		install_banks();
	}

	ext_mode_w(old_mode);
}


//-------------------------------------------------
//  ext_mode_w - the extended mode bits other than
//  the emulation-mode gate
//-------------------------------------------------

void pega1a_device::ext_mode_w(uint8_t old_mode)
{
	const uint8_t mode = ext_mode();

	if (BIT(mode ^ old_mode, 3))
		change_mode();

	if (BIT(mode, 1))
		logerror("%s: 132-column mode not supported\n", machine().describe_context());
}


//-------------------------------------------------
//  pega_3c0_w - the attribute controller, with the
//  palette registers lockable
//-------------------------------------------------

void pega1a_device::pega_3c0_w(offs_t offset, uint8_t data)
{
	if (offset == 0 && !m_attribute.index_write && palette_locked())
	{
		const uint8_t index = m_attribute.index & 0x1f;

		if (index <= 0x0f || index == 0x11)
		{
			m_attribute.index_write ^= 0x01;
			return;
		}
	}

	pc_ega8_3c0_w(offset, data);
}

//-------------------------------------------------
//  autoswitch_mono - the chip's AutoSwitch feature
//  dropping into mono emulation on its own
//-------------------------------------------------

void pega1a_device::autoswitch_mono()
{
	if (autoswitch_locked() || mono_emulation())
		return;

	LOGEXT("%s: AutoSwitch -> mono emulation\n", machine().describe_context());
	m_ext_mode_mono |= 0x40;
	update_mode_timing();
	install_banks();
}

void pega1a_device::herc_control_w(uint8_t data)
{
	LOGEXT("%s: 3BF (Hercules Control) <- %02x\n", machine().describe_context(), data);
	m_herc_control = data;

	// nothing on the EGA side ever writes 3BF, so a program that does is a
	// Hercules program and the chip switches over without being told
	autoswitch_mono();

	// as on a real Hercules card this only gates future writes to 3B8; it does
	// not restore a bit already masked out
	update_mono_timing();
	install_banks();
}


//-------------------------------------------------
//  install_banks - widen the mono aperture for
//  Hercules full mode
//-------------------------------------------------

// A Hercules program never touches the EGA graphics controller, so the mono
// aperture has to be reasserted over whatever window its memory map selected.
// Full mode's second page is selected with 3B8 bit 7 alone, so both pages are
// presented as one 64K aperture - which is also what puts page 1 at plane
// offset 0x8000, the EGA handlers indexing the plane by the offset within
// whatever range was installed.
void pega1a_device::install_banks()
{
	isa8_ega_device::install_banks();

	if (mono_emulation() && BIT(m_misc_output, 1))
	{
		m_isa->install_memory(0xb0000, BIT(m_herc_control, 1) ? 0xbffff : 0xb7fff,
				read8sm_delegate(*this, FUNC(isa8_ega_device::read)),
				write8sm_delegate(*this, FUNC(isa8_ega_device::write)));
	}
}

void pega1a_device::plantronics_w(uint8_t data)
{
	LOGEXT("%s: 3DD (Plantronics Control) <- %02x\n", machine().describe_context(), data);

	// only bits 6-4 exist: 320x200x16, 640x200x4, and a plane swap
	m_plantronics = data & 0x70;
	update_color_timing();
}

void pega1a_device::ext_3df_w(uint8_t data)
{
	LOGEXT("%s: 3DF (AutoSwitch Control) <- %02x\n", machine().describe_context(), data);
	m_ext_3df = data;
}

void pega1a_device::mode_control_w(uint8_t data)
{
	LOGEXT("%s: 3D8 (CGA Mode Control) <- %02x\n", machine().describe_context(), data);

	m_mode_control = data;

	update_color_timing();
	set_palette_luts();
}

void pega1a_device::color_select_w(uint8_t data)
{
	LOGEXT("%s: 3D9 (CGA Color Select) <- %02x\n", machine().describe_context(), data);
	m_color_select = data;

	set_palette_luts();
}


//-------------------------------------------------
//  set_palette_luts - 2bpp colour set, from 3D8/3D9
//-------------------------------------------------

// Colour 0 is whatever 3D9's low nibble says, except in 640x200 where it is
// forced black. Same table as isa8_cga_device.
void pega1a_device::set_palette_luts()
{
	m_palette_lut_2bpp[0] = cga_hires_gfx() ? 0 : (m_color_select & 0x0f);

	const uint8_t intensity = (m_color_select & 0x10) >> 1;

	if (BIT(m_mode_control, 2))
	{
		m_palette_lut_2bpp[1] = intensity | 3;
		m_palette_lut_2bpp[2] = intensity | 4;
		m_palette_lut_2bpp[3] = intensity | 7;
	}
	else if (BIT(m_color_select, 5))
	{
		m_palette_lut_2bpp[1] = intensity | 3;
		m_palette_lut_2bpp[2] = intensity | 5;
		m_palette_lut_2bpp[3] = intensity | 7;
	}
	else
	{
		m_palette_lut_2bpp[1] = intensity | 2;
		m_palette_lut_2bpp[2] = intensity | 4;
		m_palette_lut_2bpp[3] = intensity | 6;
	}
}

void pega1a_device::mode_control_mono_w(uint8_t data)
{
	LOGEXT("%s: 3B8 (MDA/Hercules Mode Control) <- %02x\n", machine().describe_context(), data);

	// bit 1 does not exist on an MDA, so a program setting it is asking for
	// Hercules graphics even if the write is masked out below for want of a
	// 3BF unlock - some programs set it, program the CRTC and only then write
	// 3BF, and waiting for 3BF would put the Hercules table into crtc_ega
	if (BIT(data, 1))
		autoswitch_mono();

	// a Hercules mode register only answers to the bits 3BF has unlocked
	if (!BIT(m_herc_control, 0))
		data &= ~0x02;
	if (!BIT(m_herc_control, 1))
		data &= ~0x80;

	m_mode_control_mono = data;

	update_mono_timing();
}


//-------------------------------------------------
//  update_*_timing - reclock the shared CRTC for
//  the current emulation mode
//-------------------------------------------------

// One CRTC serves every emulation mode, so update_mono_timing() and
// update_color_timing() are no-ops unless their half of the chip owns it.
// A CRTC only calls its reconfigure callback when its parameters change, so
// switching back to one has to restore its last configuration by hand.
void pega1a_device::update_mode_timing()
{
	if (mono_emulation())
		update_mono_timing();
	else if (cga_emulation())
		update_color_timing();

	const auto &timing = (cga_emulation() || mono_emulation()) ? m_emu_timing : m_ega_timing;
	if (timing.width > 0)
		m_screen->configure(timing.width, timing.height, timing.visarea, timing.frame_period);
}

int pega1a_device::color_hpixels_per_column() const
{
	if (!BIT(m_mode_control, 1))
		return 8;
	if (BIT(m_plantronics, 4))
		return 8;
	if (BIT(m_plantronics, 5))
		return 16;
	return cga_hires_gfx() ? 16 : 8;
}


void pega1a_device::update_mono_timing()
{
	if (!mono_emulation())
		return;

	m_crtc_emu->set_clock(MDA_CLOCK / (herc_gfx_mode() ? 16 : 9));
	m_crtc_emu->set_hpixels_per_column(herc_gfx_mode() ? 16 : 9);
}

void pega1a_device::update_color_timing()
{
	if (!cga_emulation() || mono_emulation())
		return;

	// 40- and 80-column CGA differ by a clock divisor, not by CRTC registers
	m_crtc_emu->set_clock(14.318181_MHz_XTAL / (BIT(m_mode_control, 0) ? 8 : 16));

	m_crtc_emu->set_hpixels_per_column(color_hpixels_per_column());
}


//-------------------------------------------------
//  mono_status_r - 3BA in MDA/Hercules mode
//-------------------------------------------------

uint8_t pega1a_device::mono_status_r()
{
	// bits 6-4 float high, which is how software tells a Hercules card from MDA
	return 0x70 | (m_emu_vsync ? 0x80 : 0x00) | (m_emu_vsync ? 0x08 : 0x00)
			| ((m_emu_hsync || m_emu_vsync) ? 0x01 : 0x00);
}


//**************************************************************************
//  CGA EMULATION MODE
//**************************************************************************

//-------------------------------------------------
//  cga_status_r - 3DA in CGA emulation mode
//-------------------------------------------------

uint8_t pega1a_device::cga_status_r()
{
	// bits 2-1 are the light pen, which the board does not fit
	return 0xf0 | (m_emu_vsync ? 0x08 : 0x00) | ((m_emu_hsync || m_emu_vsync) ? 0x01 : 0x00);
}

void pega1a_device::emu_vsync_changed(int state)
{
	m_emu_vsync = state;

	if (state)
		m_emu_frame_cnt++;
}


//-------------------------------------------------
//  screen_update - draw through the CRTC the
//  current mode selects
//-------------------------------------------------

// AR11 is the EGA's Overscan Color register; the emulation modes take their
// border from the CGA colour select instead, and the mono ones have none.
rgb_t pega1a_device::border_pen() const
{
	if (border_blanked())
		return rgb_t::black();

	const pen_t *const pens = m_palette->pens();

	if (mono_emulation())
		return pens[mono_pen(0)];

	if (cga_emulation())
		return pens[s_cga_pen[m_color_select & 0x0f]];

	return pens[m_attribute.data[0x11] & 0x3f];
}

uint32_t pega1a_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (cga_emulation() || mono_emulation())
		return m_crtc_emu->screen_update(screen, bitmap, cliprect);

	bitmap.fill(border_pen(), cliprect);

	// crtc_ega always draws the active display anchored at the bitmap origin,
	// so it renders into its own bitmap and is blitted to where the border
	// leaves room for it.
	const rectangle &active = m_ega_timing.active;
	m_crtc_ega->screen_update(screen, m_ega_bitmap, active);

	const pen_t *const pens = m_palette->pens();

	for (int y = active.min_y; y <= active.max_y; y++)
	{
		const int dst_y = y + m_ega_timing.y_off;

		if (dst_y < cliprect.min_y || dst_y > cliprect.max_y)
			continue;

		uint16_t const *src = &m_ega_bitmap.pix(y, active.min_x);

		for (int x = active.min_x; x <= active.max_x; x++, src++)
		{
			const int dst_x = x + m_ega_timing.x_off;

			if (dst_x >= cliprect.min_x && dst_x <= cliprect.max_x)
				bitmap.pix(dst_y, dst_x) = pens[*src & 0x3f];
		}
	}

	return 0;
}


//-------------------------------------------------
//  emu_reconfigure - hand a CRTC's timing to the
//  screen, but only while it owns the screen
//-------------------------------------------------

// A raster line runs active - front porch - sync - back porch, so what the
// monitor shows is everything but the sync pulse, with the active image sitting
// one back porch in from the left edge. A CRTC's own visible area is the active
// display alone, which is why neither reconfigure callback passes it on as is.
pega1a_device::screen_timing pega1a_device::bordered_timing(
		int width, int height, const rectangle &active, attotime frame_period,
		int hsync_on, int hsync_off, int vsync_on, int vsync_off) const
{
	screen_timing timing;

	timing.x_off = width - hsync_off;
	timing.y_off = height - vsync_off;
	timing.width = width;
	timing.height = height;
	timing.frame_period = frame_period;
	timing.active = active;

	const int visible_width = hsync_on + timing.x_off;
	const int visible_height = vsync_on + timing.y_off;

	// a CRTC programmed with no porch at all leaves nothing to draw a border in
	if (visible_width <= active.width() || visible_height <= active.height())
	{
		timing.x_off = 0;
		timing.y_off = 0;
		timing.visarea = active;
	}
	else
	{
		timing.visarea.set(0, visible_width - 1, 0, visible_height - 1);
	}

	return timing;
}

MC6845_RECONFIGURE( pega1a_device::emu_reconfigure )
{
	m_emu_timing = bordered_timing(width, height, visarea, frame_period,
			hsync_on, hsync_off, vsync_on, vsync_off);

	if (cga_emulation() || mono_emulation())
		m_screen->configure(width, height, m_emu_timing.visarea, frame_period);
}

CRTC_EGA_RECONFIGURE( pega1a_device::ega_reconfigure )
{
	m_ega_timing = bordered_timing(width, height, visarea, frame_period,
			hsync_on, hsync_off, vsync_on, vsync_off);

	if (!cga_emulation() && !mono_emulation())
		m_screen->configure(width, height, m_ega_timing.visarea, frame_period);
}


//-------------------------------------------------
//  emu_update_row - MC6845 row callback
//-------------------------------------------------

MC6845_BEGIN_UPDATE( pega1a_device::emu_begin_update )
{
	bitmap.fill(border_pen(), cliprect);
}

MC6845_UPDATE_ROW( pega1a_device::emu_update_row )
{
	// outside the active display emu_begin_update's border fill stands
	if (!de)
		return;

	// the porches are only in frame when the screen was configured with room
	// for them; bordered_timing() gives up on a CRTC programmed without any
	const bool bordered = m_emu_timing.visarea.width() > m_emu_timing.active.width();

	m_emu_x_off = bordered ? hbp : 0;
	y += bordered ? vbp : 0;

	if (mono_emulation())
	{
		// 3B8 bit 3 is the mono side's video enable
		if (!BIT(m_mode_control_mono, 3))
		{
			for (int i = 0; i < x_count * (herc_gfx_mode() ? 16 : 9); i++)
				bitmap.pix(y, m_emu_x_off + i) = rgb_t::black();
		}
		else if (herc_gfx_mode())
		{
			herc_gfx(bitmap, ma, ra, y, x_count);
		}
		else
		{
			mda_text(bitmap, ma, ra, y, x_count, cursor_x);
		}

		return;
	}

	if (!BIT(m_mode_control, 3) && !(blanking_disabled() && !BIT(m_mode_control, 1)))
	{
		for (int i = 0; i < x_count * color_hpixels_per_column(); i++)
			bitmap.pix(y, m_emu_x_off + i) = rgb_t::black();

		return;
	}

	if (BIT(m_mode_control, 1))
	{
		// a ColorPlus mode overrides the CGA one it is built on; both bits set
		// is undocumented, and cga.cpp let the low-resolution one win
		if (BIT(m_plantronics, 4))
			plantronics_gfx_4bpp(bitmap, ma, ra, y, x_count);
		else if (BIT(m_plantronics, 5))
			plantronics_gfx_2bpp(bitmap, ma, ra, y, x_count);
		else if (cga_hires_gfx())
			cga_gfx_1bpp(bitmap, ma, ra, y, x_count);
		else
			cga_gfx_2bpp(bitmap, ma, ra, y, x_count);

		return;
	}

	cga_text(bitmap, ma, ra, y, x_count, cursor_x);
}


//-------------------------------------------------
//  cga_text - CGA text mode row
//-------------------------------------------------

// Entering CGA emulation changes neither the memory map nor the character
// generator: the BIOS has already loaded the 8x8 font at the EGA's 32-byte
// stride and left odd/even addressing at 0xb8000, so a cell still reads as
// plane 0 = code, plane 1 = attribute.
void pega1a_device::cga_text(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count, int8_t cursor_x)
{
	const pen_t *const pens = m_palette->pens();
	const bool blink = BIT(m_mode_control, 5);

	for (int i = 0; i < x_count; i++)
	{
		// the MC6845 counts characters, the planes are addressed in the CPU's
		// interleaved character/attribute units
		const uint16_t offset = ((ma + i) << 1) & 0x3fff;
		const uint8_t chr = m_plane[0][offset];
		const uint8_t attr = m_plane[1][offset];

		uint8_t data = (m_charA == m_charB || BIT(attr, 3))
				? m_charA[chr * 32 + (ra & 0x1f)]
				: m_charB[chr * 32 + (ra & 0x1f)];

		const rgb_t fg = pens[s_cga_pen[attr & 0x0f]];
		const rgb_t bg = pens[s_cga_pen[(attr >> 4) & (blink ? 0x07 : 0x0f)]];

		if (i == cursor_x)
		{
			if (m_emu_frame_cnt & 0x08)
				data = 0xff;
		}
		else if (blink && BIT(attr, 7) && (m_emu_frame_cnt & 0x10))
		{
			data = 0x00;
		}

		uint32_t *p = &bitmap.pix(y, m_emu_x_off + i * 8);

		for (int b = 0; b < 8; b++)
			*p++ = BIT(data, 7 - b) ? fg : bg;
	}
}


//-------------------------------------------------
//  cga_gfx_1bpp / cga_gfx_2bpp - CGA graphics rows
//-------------------------------------------------

// Odd/even at 0xb8000 puts the CPU's even byte in plane 0 and its odd byte in
// plane 1 at the same index; each character time covers two bytes, and bit 0 of
// the raster address banks the odd scanlines 0x2000 up.
static constexpr uint16_t cga_gfx_offset(uint16_t ma, uint8_t ra, int i)
{
	return (((ma + i) << 1) & 0x1fff) | (BIT(ra, 0) << 13);
}

void pega1a_device::cga_gfx_1bpp(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count)
{
	const pen_t *const pens = m_palette->pens();
	const rgb_t fg = pens[s_cga_pen[m_color_select & 0x0f]];
	const rgb_t bg = pens[s_cga_pen[0]];

	uint32_t *p = &bitmap.pix(y, m_emu_x_off);

	for (int i = 0; i < x_count; i++)
	{
		const uint16_t offset = cga_gfx_offset(ma, ra, i);

		for (int plane = 0; plane < 2; plane++)
		{
			const uint8_t data = m_plane[plane][offset];

			for (int b = 0; b < 8; b++)
				*p++ = BIT(data, 7 - b) ? fg : bg;
		}
	}
}

void pega1a_device::cga_gfx_2bpp(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count)
{
	const pen_t *const pens = m_palette->pens();

	uint32_t *p = &bitmap.pix(y, m_emu_x_off);

	for (int i = 0; i < x_count; i++)
	{
		const uint16_t offset = cga_gfx_offset(ma, ra, i);

		for (int plane = 0; plane < 2; plane++)
		{
			const uint8_t data = m_plane[plane][offset];

			for (int b = 6; b >= 0; b -= 2)
				*p++ = pens[s_cga_pen[m_palette_lut_2bpp[(data >> b) & 0x03]]];
		}
	}
}


//-------------------------------------------------
//  ega_update_row - EGA-side row, with ColorPlus
//-------------------------------------------------

// ColorPlus is not one of the modes 3DB unlocks: it is an extra plane in the
// display path, and software reaches it by setting an ordinary CGA mode through
// the BIOS - which on this card is the EGA's own CGA-compatible mode - and then
// writing 3DD, never touching 3DB. So the second plane has to be honoured here
// as well as in the emulation-mode renderer.
CRTC_EGA_PIXEL_UPDATE( pega1a_device::ega_update_row )
{
	if (m_video_mode == EGA_MODE_GRAPHICS)
	{
		// the EGA's CGA-compatible mode is the graphics controller's odd/even
		// addressing, which is what the 16-colour mode is built on; the 640x200
		// one is built on the BIOS's 640x200, which this ROS programs as an
		// ordinary planar mode reading plane 0 alone, so it is the *absence* of
		// odd/even that picks it. Either way the second plane only exists when
		// the B8000 32K window is selected, which is what R6 bits 3-2 say.
		const uint8_t gc_mode = m_graphics_controller.data[5];
		const bool cga_window = ((m_graphics_controller.data[6] >> 2) & 0x03) == 0x03;

		if (BIT(m_plantronics, 4) && BIT(gc_mode, 4) && cga_window)
			ega_plantronics_gfx_4bpp(bitmap, ma, y, x);
		else if (BIT(m_plantronics, 5) && !BIT(gc_mode, 4) && cga_window)
			ega_plantronics_gfx_2bpp(bitmap, ma, y, x);
		else
			pc_ega_graphics(bitmap, cliprect, ma, ra, y, x, cursor_x);
	}
	else if (m_video_mode == EGA_MODE_TEXT)
	{
		pc_ega_text(bitmap, cliprect, ma, ra, y, x, cursor_x);
	}
}

// crtc_ega hands over an address that is already the CPU's byte offset into the
// 32K window, scanline bank and all, so unlike the emulation-mode renderer this
// one has no doubling of its own to do.
void pega1a_device::ega_plantronics_gfx_4bpp(bitmap_ind16 &bitmap, uint16_t ma, uint16_t y, uint8_t x)
{
	const int middle = BIT(m_plantronics, 6) ? 0x4000 : 0x0000;
	const int outer = middle ^ 0x4000;
	const uint16_t offset = ma & 0x3fff;

	uint16_t *p = &bitmap.pix(y, x * 8);

	for (int plane = 0; plane < 2; plane++)
	{
		const uint8_t m = m_plane[plane][offset | middle];
		const uint8_t o = m_plane[plane][offset | outer];

		for (int shift = 6; shift >= 0; shift -= 2)
		{
			const uint8_t mb = (m >> shift) & 0x03;
			const uint8_t ob = (o >> shift) & 0x03;

			*p++ = s_cga_pen[(mb << 1) | BIT(ob, 1) | (BIT(ob, 0) << 3)];
		}
	}

	m_last_pixel_value = *(p - 1);
}


// The 640x200 mode the second plane doubles here is one byte per character
// clock out of plane 0, not the odd/even pair the 16-colour mode reads, so both
// halves of the pixel come out of that same plane.
void pega1a_device::ega_plantronics_gfx_2bpp(bitmap_ind16 &bitmap, uint16_t ma, uint16_t y, uint8_t x)
{
	// bit 6 clear is the power-on order
	const int high = BIT(m_plantronics, 6) ? 0x0000 : 0x4000;
	const int low = high ^ 0x4000;
	const uint16_t offset = ma & 0x3fff;

	uint16_t *p = &bitmap.pix(y, x * 8);

	const uint8_t h = m_plane[0][offset | high];
	const uint8_t l = m_plane[0][offset | low];

	for (int bit = 7; bit >= 0; bit--)
		*p++ = s_cga_pen[m_palette_lut_2bpp[(BIT(h, bit) << 1) | BIT(l, bit)]];

	m_last_pixel_value = *(p - 1);
}


//-------------------------------------------------
//  plantronics_gfx_4bpp / _2bpp - ColorPlus
//  graphics rows
//-------------------------------------------------

// ColorPlus doubles CGA's colour depth with a second 16K plane at CGA offset
// 0x4000; nothing else about the mode changes. 3DD bit 6 swaps which plane
// supplies which half of the pixel, and the bit assembly below is not a natural
// packing - it is what the hardware did, preserved from cga.cpp.
void pega1a_device::plantronics_gfx_4bpp(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count)
{
	const pen_t *const pens = m_palette->pens();

	// bit 6 clear is the power-on order
	const int middle = BIT(m_plantronics, 6) ? 0x4000 : 0x0000;
	const int outer = middle ^ 0x4000;

	uint32_t *p = &bitmap.pix(y, m_emu_x_off);

	for (int i = 0; i < x_count; i++)
	{
		const uint16_t offset = cga_gfx_offset(ma, ra, i);

		for (int plane = 0; plane < 2; plane++)
		{
			const uint8_t m = m_plane[plane][offset | middle];
			const uint8_t o = m_plane[plane][offset | outer];

			for (int shift = 6; shift >= 0; shift -= 2)
			{
				const uint8_t mb = (m >> shift) & 0x03;
				const uint8_t ob = (o >> shift) & 0x03;

				*p++ = pens[s_cga_pen[(mb << 1) | BIT(ob, 1) | (BIT(ob, 0) << 3)]];
			}
		}
	}
}

void pega1a_device::plantronics_gfx_2bpp(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count)
{
	const pen_t *const pens = m_palette->pens();

	// bit 6 clear is the power-on order
	const int high = BIT(m_plantronics, 6) ? 0x0000 : 0x4000;
	const int low = high ^ 0x4000;

	uint32_t *p = &bitmap.pix(y, m_emu_x_off);

	for (int i = 0; i < x_count; i++)
	{
		const uint16_t offset = cga_gfx_offset(ma, ra, i);

		for (int plane = 0; plane < 2; plane++)
		{
			const uint8_t h = m_plane[plane][offset | high];
			const uint8_t l = m_plane[plane][offset | low];

			for (int bit = 7; bit >= 0; bit--)
				*p++ = pens[s_cga_pen[m_palette_lut_2bpp[(BIT(h, bit) << 1) | BIT(l, bit)]]];
		}
	}
}


//-------------------------------------------------
//  mda_text - MDA/Hercules text row
//-------------------------------------------------

// The character generator's second bank is not consulted: on an EGA attribute
// bit 3 selects it, on an MDA the same bit is intensity.
void pega1a_device::mda_text(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count, int8_t cursor_x)
{
	const pen_t *const pens = m_palette->pens();
	const bool blink = BIT(m_mode_control_mono, 5);

	uint32_t *p = &bitmap.pix(y, m_emu_x_off);

	for (int i = 0; i < x_count; i++)
	{
		const uint16_t offset = ((ma + i) << 1) & 0x0fff;
		const uint8_t chr = m_plane[0][offset];
		const uint8_t attr = m_plane[1][offset];

		uint8_t fg, bg;

		switch (attr)
		{
		case 0x00: case 0x08: case 0x80: case 0x88:
			fg = 0; bg = 0;                 // non-displaying
			break;

		case 0x70: case 0x78:
			fg = 0; bg = 2;                 // reverse video
			break;

		case 0xf0: case 0xf8:
			fg = 0; bg = blink ? 2 : 3;     // reverse video, blinking or bright
			break;

		default:
			fg = BIT(attr, 3) ? 3 : 2; bg = 0;
			break;
		}

		uint8_t data = m_charA[chr * 32 + (ra & 0x1f)];

		// the ninth column repeats the eighth only for the line-drawing range,
		// so that box characters join up and letters still get a gap
		bool duplicate = (chr & 0xe0) == 0xc0;

		if (ra == 12 && (attr & 0x07) == 0x01)
		{
			duplicate = true;
			data = 0xff;
		}

		if (blink && BIT(attr, 7) && (m_emu_frame_cnt & 0x10))
		{
			data = 0x00;
		}

		if (i == cursor_x && (m_emu_frame_cnt & 0x08))
		{
			duplicate = true;
			data = 0xff;
		}

		const rgb_t fg_pen = pens[mono_pen(fg)];
		const rgb_t bg_pen = pens[mono_pen(bg)];

		for (int b = 0; b < 8; b++)
			*p++ = BIT(data, 7 - b) ? fg_pen : bg_pen;

		*p++ = (duplicate && BIT(data, 0)) ? fg_pen : bg_pen;
	}
}


//-------------------------------------------------
//  herc_gfx - Hercules 720x348 graphics row
//-------------------------------------------------

// Hercules interleaves four scanlines across four 8K banks instead of CGA's
// two, and full mode has a second 32K page above the first.
void pega1a_device::herc_gfx(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count)
{
	const pen_t *const pens = m_palette->pens();
	const rgb_t fg = pens[mono_pen(2)];
	const rgb_t bg = pens[mono_pen(0)];

	const uint32_t page = BIT(m_mode_control_mono, 7) ? 0x8000 : 0x0000;

	uint32_t *p = &bitmap.pix(y, m_emu_x_off);

	for (int i = 0; i < x_count; i++)
	{
		const uint32_t offset = page | ((ra & 0x03) << 13) | (((ma + i) << 1) & 0x1fff);

		for (int plane = 0; plane < 2; plane++)
		{
			const uint8_t data = m_plane[plane][offset];

			for (int b = 0; b < 8; b++)
				*p++ = BIT(data, 7 - b) ? fg : bg;
		}
	}
}
