// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple "Valkyrie" - very low-cost video framebuffer
    Emulation by R. Belmont

    This was the bonus awfulness in the Quadra 630/LC 580 machines.  While the
    ROM software and MacOS don't support many monitor types, the Linux driver indicates
    that's more of a software problem.

    https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/drivers/video/fbdev/valkyriefb.c?h=v6.2.10

    Also valkyrie.c in the Marathon Infinity open source release from Bungie.  (They use Valkyrie's video upscaler
    to only have to render 320x240!)

    TODO:
        - YUV video-in support
        - Determine if the I2C clock is internal or external

    Video in buffer 0 is at 0xf9300000
    Video in buffer 1 is at 0xf9340000
    Both buffers are 320x240 with a stride of 1024 bytes, data is 16 bits per pixel (YUV?)

*/

#include "emu.h"
#include "valkyrie.h"

#define LOG_MODE        (1U << 1)
#define LOG_MONSENSE    (1U << 2)
#define LOG_RAMDAC      (1U << 3)
#define LOG_CLOCKGEN    (1U << 4)

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(VALKYRIE, valkyrie_device, "apvalkyrie", "Apple Valkyrie video")

//-------------------------------------------------
//  ADDRESS_MAP
//-------------------------------------------------

void valkyrie_device::map(address_map &map)
{
	map(0x50f2a000, 0x50f2bfff).rw(FUNC(valkyrie_device::regs_r), FUNC(valkyrie_device::regs_w));
	map(0x50f24000, 0x50f25fff).rw(FUNC(valkyrie_device::ramdac_r), FUNC(valkyrie_device::ramdac_w));

	map(0xf9000000, 0xf90fffff).rw(FUNC(valkyrie_device::vram_r), FUNC(valkyrie_device::vram_w));
}

valkyrie_device::valkyrie_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, VALKYRIE, tag, owner, clock),
	i2c_hle_interface(mconfig, *this, 0x28),
	m_vram_size(0x100000),
	m_pixel_clock(31334400),
	m_pal_address(0), m_pal_idx(0), m_mode(0),
	m_screen(*this, "screen"),
	m_palette(*this, "palette"),
	m_monitor_config(*this, "monitor"),
	m_irq(*this),
	m_vram_offset(0), m_monitor_id(0),
	m_base(0), m_stride(1024), m_video_timing(0x80), m_int_status(0), m_hres(0), m_vres(0), m_htotal(0), m_vtotal(0),
	m_config(0),
	m_M(1), m_N(1), m_P(1)
{
}

void valkyrie_device::device_start()
{
	m_vram = std::make_unique<u32[]>(m_vram_size);

	m_vbl_timer = timer_alloc(FUNC(valkyrie_device::vbl_tick), this);
	m_vbl_timer->adjust(attotime::never);

	save_item(NAME(m_video_timing));
	save_item(NAME(m_vram_offset));
	save_item(NAME(m_mode));
	save_item(NAME(m_monitor_id));
	save_item(NAME(m_base));
	save_item(NAME(m_stride));
	save_item(NAME(m_pal_address));
	save_item(NAME(m_pal_idx));
	save_item(NAME(m_hres));
	save_item(NAME(m_vres));
	save_item(NAME(m_htotal));
	save_item(NAME(m_vtotal));
	save_item(NAME(m_pixel_clock));
	save_item(NAME(m_config));
	save_item(NAME(m_int_status));
	save_item(NAME(m_M));
	save_item(NAME(m_N));
	save_item(NAME(m_P));
	save_pointer(NAME(m_vram), m_vram_size);

	machine().save().register_postload(save_prepost_delegate(FUNC(valkyrie_device::recalc_mode), this));
}

void valkyrie_device::device_reset()
{
	m_video_timing = 0x80;

	// clear the MAME default palette so we don't get a weird red screen on initial startup
	for (int i = 0; i < 256; i++)
	{
		m_palette->set_pen_red_level(i, 0);
		m_palette->set_pen_green_level(i, 0);
		m_palette->set_pen_blue_level(i, 0);
	}
}

void valkyrie_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// dot clock, htotal, hstart, hend, vtotal, vstart, vend
	m_screen->set_raw(31334400, 896, 0, 640, 525, 0, 480);
	m_screen->set_screen_update(FUNC(valkyrie_device::screen_update));

	PALETTE(config, m_palette).set_entries(256);
}

static constexpr u8 ext(u8 bc, u8 ac, u8 ab)
{
	return 0x40 | (bc << 4) | (ac << 2) | ab;
}

static INPUT_PORTS_START(monitor_config)
	PORT_START("monitor")
	PORT_CONFNAME(0x7f, 6, "Monitor type")
	PORT_CONFSETTING(0x02, u8"Mac RGB Display (12\" 512\u00d7384)")             // "Rubik" (modified IIgs AppleColor RGB)
	PORT_CONFSETTING(0x06, u8"Mac Hi-Res Display (12-14\" 640\u00d7480)")       // "High Res" 640x480 @ 67 Hz
	PORT_CONFSETTING(ext(1, 1, 3), "640x480 VGA")                               // VGA 640x480 @ 60 Hz
	PORT_CONFSETTING(ext(2, 3, 1), "832x624 16\" RGB")                          // "Goldfish" or "16 inch RGB"

INPUT_PORTS_END

ioport_constructor valkyrie_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(monitor_config);
}

u32 valkyrie_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]) + 0x1000;
	const pen_t *pens = m_palette->pens();

	if (m_video_timing & 0x80)
	{
		return 0;
	}

	const u32 stride = (m_stride << m_mode);
	switch (m_mode)
	{
		case 0: // 1bpp
		{
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres/8; x++)
				{
					u8 const pixels = vram8[(y * stride) + x];

					*scanline++ = pens[(pixels>>7)&1];
					*scanline++ = pens[(pixels>>6)&1];
					*scanline++ = pens[(pixels>>5)&1];
					*scanline++ = pens[(pixels>>4)&1];
					*scanline++ = pens[(pixels>>3)&1];
					*scanline++ = pens[(pixels>>2)&1];
					*scanline++ = pens[(pixels>>1)&1];
					*scanline++ = pens[(pixels&1)];
				}
			}
		}
		break;

		case 1: // 2bpp
		{
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres/4; x++)
				{
					u8 const pixels = vram8[(y * stride) + x];

					*scanline++ = pens[((pixels>>6)&3)];
					*scanline++ = pens[((pixels>>4)&3)];
					*scanline++ = pens[((pixels>>2)&3)];
					*scanline++ = pens[(pixels&3)];
				}
			}
		}
		break;

		case 2: // 4bpp
		{
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres/2; x++)
				{
					u8 const pixels = vram8[(y * stride) + x];

					*scanline++ = pens[(pixels>>4)];
					*scanline++ = pens[(pixels&0xf)];
				}
			}
		}
		break;

		case 3: // 8bpp
		{
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres; x++)
				{
					u8 const pixels = vram8[(y * stride) + x];
					*scanline++ = pens[pixels];
				}
			}
		}
		break;

		case 4: // 16bpp x555
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres; x++)
				{
					u16 const pixels = (vram8[(y * stride) + (x<<1)] << 8) | vram8[(y * stride) + (x<<1) + 1];
					*scanline++ = rgb_t(((pixels >> 10) & 0x1f) << 3, ((pixels >> 5) & 0x1f) << 3, (pixels & 0x1f) << 3);
				}
			}
			break;
	}

	return 0;
}

u8 valkyrie_device::regs_r(offs_t offset)
{
	switch (offset)
	{
		case 0:
			return m_video_timing;

		case 4:
			return m_mode;

		case 0x10: // config
			return m_config;

		case 0x14:
			return m_screen->vblank();

		case 0x1c:  // monitor sense in upper nibble, write monitor sense in lower nibble
			{
				u8 mon = m_monitor_config->read();
				u8 res;
				LOGMASKED(LOG_MONSENSE, "mon = %02x, m_monitor_id = %02x\n", mon, m_monitor_id);
				if (mon & 0x40)
				{
					res = 7;
					if (m_monitor_id == 0x4)
					{
						res &= 4 | (BIT(mon, 5) << 1) | BIT(mon, 4);
					}
					if (m_monitor_id == 0x2)
					{
						res &= (BIT(mon, 3) << 2) | 2 | BIT(mon, 2);
					}
					if (m_monitor_id == 0x1)
					{
						res &= (BIT(mon, 1) << 2) | (BIT(mon, 0) << 1) | 1;
					}
				}
				else
				{
					res = mon;
				}

				LOGMASKED(LOG_MONSENSE, "sense result = %x\n", res);
				return res<<4;
			}
			break;
	}

	return 0;
}

void valkyrie_device::regs_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0: // video mode.  hardcoded!
			m_video_timing = data & 0xff;
			if (!(data & 0x80))
			{
				recalc_mode();
			}
			break;

		case 4: // video depth: 0=1bpp, 1=2bpp, 2=4bpp, 3=8bpp, 4=16bpp
			LOG("Mode set to %d\n", data & 7);
			m_mode = data & 7;
			break;

		case 0xc:   // subsystem configuration register
			if (data == 0x1)
			{
				recalc_mode();
			}
			break;

		case 0x10:
			m_config = data;

			m_int_status &= ~1;
			recalc_ints();

			m_vbl_timer->adjust(m_screen->time_until_pos(m_vres, 0), 0);
			break;

		case 0x18: // screen enable
			// at startup, the screen isn't wanted on until 0x81 is written.
			// if the Video Startup extension is installed, it later sets this to 0x02.
			if ((data & 0x80) || (data == 0x02))
			{
				m_vbl_timer->adjust(m_screen->time_until_pos(m_vres, 0), 0);
			}
			else
			{
				m_vbl_timer->adjust(attotime::never);
			}
			break;

		case 0x1c: // drive monitor sense lines. 1 = drive, 0 = tri-state
			m_monitor_id = (data & 0x7);
			LOGMASKED(LOG_MONSENSE, "%x to sense drive\n", data & 0xf);
			break;

		case 0x20: // video in control
			break;

		case 0x60: // video window X position
		case 0x61:
			break;

		case 0x64: // video window Y position
		case 0x65:
			break;

		case 0x70: // video window width
		case 0x71:
			break;

		case 0x74: // video window height
		case 0x75:
			break;

		case 0x80: // video field starting X pixel
		case 0x81:
			break;

		case 0x84: // video field starting Y pixel
		case 0x85:
			break;

		default:
			LOG("Valkyrie: Unk write %08x @ %x\n", data, offset<<2);
			break;
	}
}

u32 valkyrie_device::ramdac_r(offs_t offset)
{
	switch (offset)
	{
		case 0:
			if (!machine().side_effects_disabled())
			{
				m_pal_idx = 0;
			}
			return m_pal_address<<24;

		case 1:
			{
				pen_t const entry = m_palette->pen(m_pal_address);
				u8 const idx = m_pal_idx;
				if (!machine().side_effects_disabled())
				{
					m_pal_idx++;
				}
				switch (idx)
				{
					case 0:
						return ((entry >> 16) & 0xff) << 24;
					case 1:
						return ((entry >> 8) & 0xff) << 24;
					case 2:
						return (entry & 0xff) << 24;
				}
			}
			break;
	}
	return 0;
}

void valkyrie_device::ramdac_w(offs_t offset, u32 data)
{
	data >>= 24;
	switch (offset)
	{
	case 0:
		m_pal_address = data & 0xff;
		m_pal_idx = 0;
		break;

	case 1:
		switch (m_pal_idx)
		{
			case 0:
				m_palette->set_pen_red_level(m_pal_address, data & 0xff);
				break;
			case 1:
				m_palette->set_pen_green_level(m_pal_address, data & 0xff);
				break;
			case 2:
				m_palette->set_pen_blue_level(m_pal_address, data & 0xff);
				break;
		}

		m_pal_idx++;
		if (m_pal_idx == 3)
		{
			m_pal_idx = 0;
			m_pal_address++;
		}
		break;

	case 2:
		LOGMASKED(LOG_RAMDAC, "%02x to DAC @ %x\n", data, offset);
		break;
	}
}

void valkyrie_device::recalc_mode()
{
	// mode parameters taken from the Quadra 630 Developer Note
	// m_video_timing numbers:
	//  2 =  512x384, 60 Hz ("Rubik")
	//  6 =  640x480, 72 Hz (Mac 13")
	//  9 =  832x624, 75 Hz (Mac 16")
	// 11 =  640x480, 60 Hz (VGA)
	// 12 =  800x600, 60 Hz (SVGA)
	// 13 =  800x600, 72 Hz (SVGA)
	// 14 = 1024x768, 60 Hz (SVGA)
	// 15 = 1024x768, 72 Hz (SVGA)

	if (m_video_timing & 0x80)
	{
		return;
	}

	switch (m_video_timing & 0x7f)
	{
		case 2: // Rubik 512x384
			m_hres = 512;
			m_vres = 384;
			m_htotal = 640;
			m_vtotal = 407;
			m_stride = 64;
			break;

		case 6: // 13" 640x480
			m_hres = 640;
			m_vres = 480;
			m_htotal = 864;
			m_vtotal = 525;
			m_stride = 80;
			break;

		case 9: // 16" RGB 832x624?
			m_hres = 832;
			m_vres = 624;
			m_htotal = 1072;
			m_vtotal = 690;
			m_stride = 104;
			break;

		case 11: // VGA 640x480
			m_hres = 640;
			m_vres = 480;
			m_htotal = 800;
			m_vtotal = 525;
			m_stride = 80;
			break;
	}

	const double refresh = (double)m_pixel_clock / (double)(m_htotal * m_vtotal);
	LOGMASKED(LOG_MODE, "hres %d vres %d htotal %d vtotal %d refresh %f stride %d mode %d\n", m_hres, m_vres, m_htotal, m_vtotal, refresh, m_stride, m_mode);

	if ((m_hres != 0) && (m_vres != 0))
	{
		rectangle visarea(0, m_hres - 1, 0, m_vres - 1);
		m_screen->configure(m_htotal, m_vtotal, visarea, attotime::from_ticks(m_htotal * m_vtotal, m_pixel_clock).as_attoseconds());
	}
}

u32 valkyrie_device::vram_r(offs_t offset)
{
	return m_vram[offset & (m_vram_size - 1)];
}

void valkyrie_device::vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_vram[offset & (m_vram_size - 1)]);
}

void valkyrie_device::recalc_ints()
{
	if (m_int_status != 0)
	{
		m_irq(ASSERT_LINE);
	}
	else
	{
		m_irq(CLEAR_LINE);
	}
}

TIMER_CALLBACK_MEMBER(valkyrie_device::vbl_tick)
{
	m_int_status |= 1;
	recalc_ints();

	m_vbl_timer->adjust(m_screen->time_until_pos(480, 0), 0);
}

void valkyrie_device::write_data(u16 offset, u8 data)
{
	switch (offset)
	{
		case 0:
			return;

		case 1:
			m_M = data;
			break;

		case 2:
			m_N = data;
			break;

		case 3:
			m_P = data;
			break;
	}

	const double clock = (3986400.0f * (double)(1 << m_P) * (double)m_N) / (double)m_M;
	m_pixel_clock = (u32)clock;

	// TODO: Apple documents these machines as supporting the 512x384 monitor, but selecting it
	// programs garbage to the clock generator.  Allow that configuration to work until we determine
	// if the Apple developer note is lying.
	if ((m_M == 0) && (m_N == 0) && (m_P = 98))
	{
		m_pixel_clock = 15670000;
	}

	LOGMASKED(LOG_CLOCKGEN, "Valkyrie: M = %d %02x, N = %d %02x P = %d, pixel clock %d\n", m_M, m_M, m_N, m_N, m_P, m_pixel_clock);
	recalc_mode();
}
