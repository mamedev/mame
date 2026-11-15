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

#include "endianness.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>

#define LOG_MODE        (1U << 1)
#define LOG_MONSENSE    (1U << 2)
#define LOG_RAMDAC      (1U << 3)
#define LOG_CLOCKGEN    (1U << 4)

#define VERBOSE (0)
#include "logmacro.h"


namespace {

// Valkyrie aligns its (often byte-wide) registers to the left-hand side of 32-bit addresses.
// Valkyrie-AR is aligned to 64-bit addresses and not 32-bit ones.
enum
{
	VALKYRIEREG_VIDEO_TIMING        = 0x00,
	VALKYRIEREG_VIDEO_MODE          = 0x04,
	VALKYRIEREG_STATUS              = 0x08,
	VALKYRIEREG_VIDEO_DEPTH         = 0x0C,
	VALKYRIEREG_CONFIG              = 0x10,
	VALKYRIEREG_INT_STATUS          = 0x14,
	VALKYRIEREG_INT_ENABLE          = 0x18,
	VALKYRIEREG_MONITOR_SENSE       = 0x1C,

	VALKYRIEREG_VIDEO_IN_CONTROL    = 0x20,
	VALKYRIEREG_VIDEO_WINDOW_XPOS   = 0x60, // 16-bit
	VALKYRIEREG_VIDEO_WINDOW_YPOS   = 0x64, // 16-bit
	VALKYRIEREG_VIDEO_WINDOW_WIDTH  = 0x70, // 16-bit
	VALKYRIEREG_VIDEO_WINDOW_HEIGHT = 0x74, // 16-bit
	VALKYRIEREG_VIDEO_FIELD_X_PIXEL = 0x80, // 16-bit
	VALKYRIEREG_VIDEO_FIELD_Y_PIXEL = 0x84, // 16-bit
};

enum : u8
{
	VIDEO_IN_PIXEL_DOUBLE   = 0x02,
	VIDEO_IN_WINDOW_ACTIVE  = 0x20,
	VIDEO_IN_BUFFER1        = 0x40
};

constexpr u32 VIDEO_BUFFER0 = 0x97800;
constexpr u32 VIDEO_BUFFER1 = 0xc0000;
constexpr u32 VIDEO_BUFFER_WIDTH = 320;
constexpr u32 VIDEO_BUFFER_HEIGHT = 240;

constexpr offs_t video_buffer_offset(offs_t offset)
{
	u32 const row = BIT(offset, 8, 8);
	u32 const column = BIT(offset, 0, 8) << 2;
	u32 const base = BIT(offset, 16) ? VIDEO_BUFFER1 : VIDEO_BUFFER0;

	return (base + (row * (VIDEO_BUFFER_WIDTH << 1)) + column) >> 2;
}

} // anonymous namespace


DEFINE_DEVICE_TYPE(VALKYRIE, valkyrie_device, "apvalkyrie", "Apple Valkyrie video")


//-------------------------------------------------
//  ADDRESS_MAP
//-------------------------------------------------

void valkyrie_device::map(address_map &map)
{
	map(0x50f2a000, 0x50f2bfff).rw(FUNC(valkyrie_device::regs_r), FUNC(valkyrie_device::regs_w));
	map(0x50f24000, 0x50f25fff).rw(FUNC(valkyrie_device::ramdac_r), FUNC(valkyrie_device::ramdac_w));

	map(0xf9000000, 0xf90fffff).rw(FUNC(valkyrie_device::vram_r), FUNC(valkyrie_device::vram_w));
	map(0xf9300000, 0xf937ffff).rw(FUNC(valkyrie_device::video_buffer_r), FUNC(valkyrie_device::video_buffer_w));
}

void valkyrie_device::valkyriear_map(address_map &map)
{
	map(0xf130a000, 0xf130bfff).rw(FUNC(valkyrie_device::regs64_r), FUNC(valkyrie_device::regs64_w));
	map(0xf1304000, 0xf1305fff).rw(FUNC(valkyrie_device::ramdac_r), FUNC(valkyrie_device::ramdac_w));

	map(0xf1000000, 0xf10fffff).rw(FUNC(valkyrie_device::vram_r), FUNC(valkyrie_device::vram_w));
}


valkyrie_device::valkyrie_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, VALKYRIE, tag, owner, clock),
	i2c_hle_interface(mconfig, *this, 0x28),
	m_pixel_clock(31334400),
	m_pal_address(0), m_pal_idx(0), m_mode(0),
	m_screen(*this, "screen"),
	m_palette(*this, "palette"),
	m_monitor_config(*this, "monitor"),
	m_vram(*this, "vram", 0x100000, ENDIANNESS_BIG),
	m_irq(*this),
	m_vram_offset(0), m_monitor_id(0),
	m_base(0), m_stride(1024), m_video_timing(0x80), m_int_status(0), m_hres(0), m_vres(0), m_htotal(0), m_vtotal(0),
	m_config(0), m_int_enable(0),
	m_M(1), m_N(1), m_P(1),
	m_video_in_control(0), m_color_key(0),
	m_window_x(0), m_window_y(0), m_window_width(0), m_window_height(0), m_field_x(0), m_field_y(0),
	m_pal_latch{ 0, 0, 0 }
{
}

valkyrie_device::~valkyrie_device()
{
}

void valkyrie_device::device_start()
{
	save_item(NAME(m_video_timing));
	save_item(NAME(m_vram_offset));
	save_item(NAME(m_mode));
	save_item(NAME(m_monitor_id));
	save_item(NAME(m_base));
	save_item(NAME(m_stride));
	save_item(NAME(m_pal_address));
	save_item(NAME(m_pal_idx));
	save_item(NAME(m_pal_latch));
	save_item(NAME(m_hres));
	save_item(NAME(m_vres));
	save_item(NAME(m_htotal));
	save_item(NAME(m_vtotal));
	save_item(NAME(m_pixel_clock));
	save_item(NAME(m_config));
	save_item(NAME(m_int_enable));
	save_item(NAME(m_int_status));
	save_item(NAME(m_M));
	save_item(NAME(m_N));
	save_item(NAME(m_P));
	save_item(NAME(m_video_in_control));
	save_item(NAME(m_color_key));
	save_item(NAME(m_window_x));
	save_item(NAME(m_window_y));
	save_item(NAME(m_window_width));
	save_item(NAME(m_window_height));
	save_item(NAME(m_field_x));
	save_item(NAME(m_field_y));
	save_item(NAME(m_video_clut));

	machine().save().register_postload(save_prepost_delegate(FUNC(valkyrie_device::recalc_mode), this));
}

void valkyrie_device::device_reset()
{
	m_video_timing = 0x80;
	m_video_in_control = 0;
	for (auto &entry : m_video_clut)
		std::fill(std::begin(entry), std::end(entry), 0);

	// clear the MAME default palette so we don't get a weird red screen on initial startup
	for (int i = 0; i < 256; i++)
	{
		m_palette->set_pen_red_level(i, 0);
		m_palette->set_pen_green_level(i, 0);
		m_palette->set_pen_blue_level(i, 0);
	}
	load_pal_latch();
}

void valkyrie_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen);
	// dot clock, htotal, hstart, hend, vtotal, vstart, vend
	m_screen->set_raw(31334400, 896, 0, 640, 525, 0, 480);
	m_screen->set_screen_update(FUNC(valkyrie_device::screen_update));
	m_screen->screen_vblank().set(FUNC(valkyrie_device::vblank_w));

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
	if ((m_video_timing & 0x80) || timing_stopped() || (m_mode > 4))
	{
		bitmap.fill(rgb_t::black(), cliprect);
		return 0;
	}

	auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]) + 0x1000;
	auto const vram16 = util::big_endian_cast<u16 const>(&m_vram[0]) + 0x800;
	const pen_t *const pens = m_palette->pens();
	const u32 stride = m_stride << m_mode;
	const u32 depth = 1U << m_mode;
	const u32 pixel_mask = util::make_bitmask<u32>(depth);
	rectangle clip(0, m_hres - 1, 0, m_vres - 1);
	clip &= cliprect;

	for (int y = clip.top(); y <= clip.bottom(); y++)
	{
		u32 *const scanline = &bitmap.pix(y);
		if (m_mode < 4)
		{
			for (int x = clip.left(); x <= clip.right(); x++)
			{
				const u32 bit = x * depth;
				const u8 pixels = vram8[(y * stride) + (bit >> 3)];
				scanline[x] = pens[(pixels >> (8 - depth - (bit & 7))) & pixel_mask];
			}
		}
		else // 16bpp x555
		{
			for (int x = clip.left(); x <= clip.right(); x++)
			{
				const u16 pixels = vram16[(y * (stride >> 1)) + x];
				scanline[x] = rgb_t(BIT(pixels, 10, 5) << 3, BIT(pixels, 5, 5) << 3, BIT(pixels, 0, 5) << 3);
			}
		}
	}

	if (m_mode == 3)
		draw_video_window(bitmap, clip, stride);
	return 0;
}

void valkyrie_device::draw_video_window(bitmap_rgb32 &bitmap, rectangle const &cliprect, u32 stride)
{
	if (!(m_video_in_control & VIDEO_IN_WINDOW_ACTIVE))
		return;

	auto const graphics = util::big_endian_cast<u8 const>(&m_vram[0]) + 0x1000;
	auto const video = util::big_endian_cast<u16 const>(&m_vram[0]) + (((m_video_in_control & VIDEO_IN_BUFFER1) ? VIDEO_BUFFER1 : VIDEO_BUFFER0) >> 1);
	int const shift = (m_video_in_control & VIDEO_IN_PIXEL_DOUBLE) ? 1 : 0;
	rectangle clip(m_window_x, m_window_x + m_window_width - 1, m_window_y, m_window_y + m_window_height - 1);
	clip &= rectangle(0, m_hres - 1, 0, m_vres - 1);
	clip &= cliprect;

	for (s32 y = clip.top(); y <= clip.bottom(); y++)
	{
		u32 const vy = m_field_y + ((y - m_window_y) >> shift);
		if (vy >= VIDEO_BUFFER_HEIGHT)
			break;

		u32 *const scanline = &bitmap.pix(y);
		for (s32 x = clip.left(); x <= clip.right(); x++)
		{
			u32 const vx = m_field_x + ((x - m_window_x) >> shift);
			if (vx >= VIDEO_BUFFER_WIDTH)
				break;

			if (graphics[(y * stride) + x] == m_color_key)
			{
				u16 const pixel = video[(vy * VIDEO_BUFFER_WIDTH) + vx];
				scanline[x] = rgb_t(m_video_clut[BIT(pixel, 10, 5)][0], m_video_clut[BIT(pixel, 5, 5)][1], m_video_clut[BIT(pixel, 0, 5)][2]);
			}
		}
	}
}

u8 valkyrie_device::regs_r(offs_t offset)
{
	switch (offset)
	{
		case VALKYRIEREG_VIDEO_TIMING:
			return m_video_timing;

		case VALKYRIEREG_VIDEO_MODE:
			return m_mode;

		case VALKYRIEREG_CONFIG: // bit 7: blanking, bit 0: vertical blanking interrupt latch
			return (m_config & 0x7e) | ((m_screen->hblank() || m_screen->vblank() || timing_stopped()) ? 0x80 : 0) | BIT(m_int_status, 0);

		case VALKYRIEREG_INT_STATUS: // 0x81 while the enabled vertical blanking interrupt is pending
			return (BIT(m_int_status, 0) && BIT(m_int_enable, 0)) ? 0x81 : 0;

		case VALKYRIEREG_INT_ENABLE:
			return m_int_enable;

		case VALKYRIEREG_MONITOR_SENSE:  // monitor sense in upper nibble, write monitor sense in lower nibble
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

static void write_half(u16 &reg, offs_t offset, u8 data)
{
	if (BIT(offset, 0))
		reg = (reg & 0xff00) | data;
	else
		reg = (reg & 0x00ff) | (data << 8);
}

void valkyrie_device::regs_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case VALKYRIEREG_VIDEO_TIMING: // video mode.  hardcoded!
			m_video_timing = data & 0xff;
			if (!(data & 0x80))
			{
				recalc_mode();
			}
			break;

		case VALKYRIEREG_VIDEO_MODE: // video depth: 0=1bpp, 1=2bpp, 2=4bpp, 3=8bpp, 4=16bpp
			LOG("Mode set to %d\n", data & 7);
			m_mode = data & 7;
			break;

		case VALKYRIEREG_STATUS: // the chip is reset by any write
			m_video_timing = 0x9f;
			m_mode = 0;
			m_int_enable = 0;
			recalc_ints();
			break;

		case VALKYRIEREG_VIDEO_DEPTH:   // subsystem configuration register
			if (data == 0x1)
			{
				recalc_mode();
			}
			break;

		case VALKYRIEREG_CONFIG:
			m_config = data;

			// the vertical blanking interrupt latch is cleared when bit 0 is written as 1
			if (BIT(data, 0))
			{
				m_int_status &= ~1;
				recalc_ints();
			}
			break;

		case VALKYRIEREG_INT_ENABLE:
			// the bits written as 1 are set when bit 7 is 1 and cleared when it is 0
			// bit 0: vertical blanking interrupt enable, set by the ROM at startup with a write of 0x81
			if (BIT(data, 7))
			{
				m_int_enable |= data & 0x03;
			}
			else
			{
				m_int_enable &= ~data;
			}
			recalc_ints();
			break;

		case VALKYRIEREG_MONITOR_SENSE: // drive monitor sense lines. 1 = drive, 0 = tri-state
			m_monitor_id = (data & 0x7);
			LOGMASKED(LOG_MONSENSE, "%x to sense drive\n", data & 0xf);
			break;

		case VALKYRIEREG_VIDEO_IN_CONTROL:    // video in control
			m_video_in_control = data;
			break;

		case VALKYRIEREG_VIDEO_WINDOW_XPOS:   // video window X position
		case VALKYRIEREG_VIDEO_WINDOW_XPOS+1:
			write_half(m_window_x, offset, data);
			break;

		case VALKYRIEREG_VIDEO_WINDOW_YPOS:   // video window Y position
		case VALKYRIEREG_VIDEO_WINDOW_YPOS+1:
			write_half(m_window_y, offset, data);
			break;

		case VALKYRIEREG_VIDEO_WINDOW_WIDTH:   // video window width
		case VALKYRIEREG_VIDEO_WINDOW_WIDTH+1:
			write_half(m_window_width, offset, data);
			break;

		case VALKYRIEREG_VIDEO_WINDOW_HEIGHT:   // video window height
		case VALKYRIEREG_VIDEO_WINDOW_HEIGHT+1:
			write_half(m_window_height, offset, data);
			break;

		case VALKYRIEREG_VIDEO_FIELD_X_PIXEL:   // video field starting X pixel
		case VALKYRIEREG_VIDEO_FIELD_X_PIXEL+1:
			write_half(m_field_x, offset, data);
			break;

		case VALKYRIEREG_VIDEO_FIELD_Y_PIXEL:   // video field starting Y pixel
		case VALKYRIEREG_VIDEO_FIELD_Y_PIXEL+1:
			write_half(m_field_y, offset, data);
			break;

		default:
			LOG("Valkyrie: Unk write %08x @ %x\n", data, offset<<2);
			break;
	}
}

void valkyrie_device::regs64_w(offs_t offset, u8 data)
{
	// TODO: check that this 64-bit alignment really exists on real hardware.
	// Valkyrie-AR registers are 64-bit aligned, so it makes sense that they don't
	// get mirrored in 32-bit halves...
	if (offset & 4)
	{
		logerror("%s regs64_w(): unaligned write to 0x%02x\n", tag(), offset);
		return;
	}

	regs_w((offset >> 1) + (offset & 3), data);
}

u8 valkyrie_device::regs64_r(offs_t offset)
{
	// TODO: check that this 64-bit alignment really exists on real hardware
	if (offset & 4)
	{
		logerror("%s regs64_r(): unaligned read from reg at 8-bit address 0x%02x\n",
				 tag(),
				 offset);
		return 0;
	}

	return regs_r((offset >> 1) + (offset & 3));
}

void valkyrie_device::load_pal_latch()
{
	rgb_t const entry = m_palette->pen_color(m_pal_address);
	m_pal_latch[0] = entry.r();
	m_pal_latch[1] = entry.g();
	m_pal_latch[2] = entry.b();
}

u32 valkyrie_device::ramdac_r(offs_t offset)
{
	switch (offset)
	{
		case 0:
			return m_pal_address<<24;

		case 1:
			{
				u8 const data = m_pal_latch[m_pal_idx];
				if (!machine().side_effects_disabled())
				{
					// The address is incremented after green, and the latch is reloaded from the new address after blue
					if (m_pal_idx == 1)
						m_pal_address++;
					if (++m_pal_idx == 3)
					{
						m_pal_idx = 0;
						load_pal_latch();
					}
				}
				return u32(data) << 24;
			}
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
		load_pal_latch();
		break;

	case 1:
		// Components are collected in the latch shared with reads; the entry is written on blue
		m_pal_latch[m_pal_idx] = data;
		if (++m_pal_idx == 3)
		{
			m_pal_idx = 0;
			m_palette->set_pen_color(m_pal_address, rgb_t(m_pal_latch[0], m_pal_latch[1], m_pal_latch[2]));
			m_pal_address++;
		}
		break;

	case 2:
		if (m_pal_idx < 3)
			m_video_clut[m_pal_address][m_pal_idx] = data;

		m_pal_idx++;
		if (m_pal_idx == 3)
		{
			m_pal_idx = 0;
			m_pal_address++;
		}
		break;

	case 3:
		m_color_key = data;
		break;

	default:
		LOGMASKED(LOG_RAMDAC, "%02x to DAC @ %x\n", data, offset);
		break;
	}
}

void valkyrie_device::recalc_mode()
{
	// mode parameters taken from the Quadra 630 and Power Macintosh 5200/6200 Developer Notes
	// m_video_timing numbers:
	//  2 =  512x384, 60 Hz ("Rubik")
	//  6 =  640x480, 67 Hz (Mac 13")
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

		case 9: // 16" RGB 832x624
			m_hres = 832;
			m_vres = 624;
			m_htotal = 1152;
			m_vtotal = 667;
			m_stride = 104;
			break;

		case 11: // VGA 640x480
			m_hres = 640;
			m_vres = 480;
			m_htotal = 800;
			m_vtotal = 525;
			m_stride = 80;
			break;

		case 12: // SVGA 800x600, 60 Hz
			m_hres = 800;
			m_vres = 600;
			m_htotal = 1056;
			m_vtotal = 628;
			m_stride = 104;
			break;

		case 13: // SVGA 800x600, 72 Hz
			m_hres = 800;
			m_vres = 600;
			m_htotal = 1040;
			m_vtotal = 666;
			m_stride = 104;
			break;

		default:
			return;
	}

	const double refresh = (double)m_pixel_clock / (double)(m_htotal * m_vtotal);
	LOGMASKED(LOG_MODE, "hres %d vres %d htotal %d vtotal %d refresh %f stride %d mode %d\n", m_hres, m_vres, m_htotal, m_vtotal, refresh, m_stride, m_mode);

	if ((m_hres != 0) && (m_vres != 0))
	{
		rectangle visarea(0, m_hres - 1, 0, m_vres - 1);
		m_screen->configure(m_htotal, m_vtotal, visarea, attotime::from_ticks(m_htotal * m_vtotal, m_pixel_clock));
	}
}

u32 valkyrie_device::vram_r(offs_t offset)
{
	return m_vram[offset & (m_vram.length() - 1)];
}

void valkyrie_device::vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_vram[offset & (m_vram.length() - 1)]);
}

u32 valkyrie_device::video_buffer_r(offs_t offset)
{
	return m_vram[video_buffer_offset(offset)];
}

void valkyrie_device::video_buffer_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_vram[video_buffer_offset(offset)]);
}

void valkyrie_device::recalc_ints()
{
	if (BIT(m_int_status, 0) && BIT(m_int_enable, 0))
	{
		m_irq(ASSERT_LINE);
	}
	else
	{
		m_irq(CLEAR_LINE);
	}
}

void valkyrie_device::vblank_w(int state)
{
	if (state && !timing_stopped())
	{
		m_int_status |= 1;
		recalc_ints();
	}
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

	// Apple's driver and Linux both write P last
	if (offset != 3)
		return;

	// 512x384 writes M = 0, N = 0, P = 0x62 and runs at half the crystal clock
	if ((m_M == 0) || (m_N == 0))
	{
		m_pixel_clock = clock() / 2;
	}
	else
	{
		const double pixel_clock = std::ldexp(double(clock()) / 8.0, m_P) * m_N / m_M;
		if ((pixel_clock < 1.0) || (pixel_clock > std::numeric_limits<u32>::max()))
		{
			LOGMASKED(LOG_CLOCKGEN, "Valkyrie: ignoring out-of-range clock M=%u N=%u P=%u\n", m_M, m_N, m_P);
			return;
		}
		m_pixel_clock = u32(pixel_clock);
	}

	LOGMASKED(LOG_CLOCKGEN, "Valkyrie: M = %d %02x, N = %d %02x P = %d, pixel clock %d\n", m_M, m_M, m_N, m_N, m_P, m_pixel_clock);
	recalc_mode();
}

