// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple Taos video
    Emulation by R. Belmont

    Taos is a video controller used exclusively in the Pippin, but it shares similarities
    to the DAFB used in the Quadra series.  It has a 1 MiB framebuffer (and supports 2)
    and a programmable CRTC with convolution for NTSC and PAL modes.
*/

#include "emu.h"
#include "taos.h"

#include "endianness.h"

#define LOG_SWATCH          (1U << 1)       // CRTC parameter calculations
#define LOG_RAMDAC          (1U << 2)       // palette
#define LOG_REGISTERS       (1U << 3)       // register writes
#define LOG_REGISTERS_READ  (1U << 4)       // register reads (separate because spammy)

#define VERBOSE (0)

#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

DEFINE_DEVICE_TYPE(APPLE_TAOS, taos_device, "appletaos", "Apple Taos video")

void taos_device::map(address_map & map)
{
	map(0x0000'0000, 0x000f'ffff).rw(FUNC(taos_device::vram_r), FUNC(taos_device::vram_w));
	map(0x0080'0000, 0x0080'03ff).rw(FUNC(taos_device::taos_r), FUNC(taos_device::taos_w));
	map(0x0100'0000, 0x0100'07ff).rw(FUNC(taos_device::clut_r), FUNC(taos_device::clut_w));
}

taos_device::taos_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, APPLE_TAOS, tag, owner, clock),
	m_screen(*this, "screen"),
	m_palette(*this, "palette"),
	m_hres(640),
	m_vres(480),
	m_htotal(800),
	m_vtotal(525),
	m_pixel_clock(31334400)
{
	std::fill(std::begin(m_taos_regs), std::end(m_taos_regs), 0);
}

void taos_device::device_start()
{
	m_vram = std::make_unique<u32[]>(0x100000/4);

	save_item(NAME(m_hres));
	save_item(NAME(m_vres));
	save_item(NAME(m_htotal));
	save_item(NAME(m_vtotal));
	save_item(NAME(m_pixel_clock));
	save_item(NAME(m_taos_regs));
	save_item(NAME(m_pal_idx));
	save_pointer(NAME(m_vram), 0x100000/4);
}

void taos_device::device_reset()
{
}

void taos_device::device_add_mconfig(machine_config &config)
{
	PALETTE(config, m_palette).set_entries(256);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(25175000, 800, 0, 640, 525, 0, 480);
	m_screen->set_screen_update(FUNC(taos_device::screen_update_taos));
}

void taos_device::set_pixclock(u32 pclock)
{
	m_pixel_clock = pclock;
}

u32 taos_device::screen_update_taos(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const u32 fb_base = m_taos_regs[FB_BASE] >> 24;

	if (BIT(m_taos_regs[COLOR_DEPTH], 31))
	{
		auto const vram16 = util::big_endian_cast<u16 const>(&m_vram[fb_base >> 1]);
		const u32 stride = (m_taos_regs[ROW_WORDS] >> 24) << 3;

		for (int y = 0; y < 480; y++)
		{
			u32 *scanline = &bitmap.pix(y);
			for (int x = 0; x < 640; x++)
			{
				u16 const pixels = vram16[(y * stride) + x];
				*scanline++ = rgb_t(((pixels >> 10) & 0x1f) << 3, ((pixels >> 5) & 0x1f) << 3, (pixels & 0x1f) << 3);
			}
		}
	}
	else
	{
		auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[fb_base >> 2]);
		const u32 stride = (m_taos_regs[ROW_WORDS] >> 24) << 3;
		const pen_t *pens = m_palette->pens();
		for (int y = 0; y < 480; y++)
		{
			u32 *scanline = &bitmap.pix(y);

			for (int x = 0; x < 640; x++)
			{
				u8 const pixels = vram8[(y * stride) + x + fb_base];
				*scanline++ = pens[pixels];
			}
		}
	}

	return 0;
}

u32 taos_device::vram_r(offs_t offset)
{
	return m_vram[offset];
}

void taos_device::vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
}

u32 taos_device::taos_r(offs_t offset)
{
	LOGMASKED(LOG_REGISTERS_READ, "TAOS read reg %02X (%s)\n", offset, machine().describe_context().c_str());
	switch (offset)
	{
	case VERSION:
		return 0xa500'0000;

	// bits 31-29: Apple monitor ID (3 for PAL, 5 for NTSC, 6 for traditional Mac 640x480 30 kHz)
	// bit 27: CD tray status (1 = closed)
	// bit 25: vblank status (0 = in vblank)
	case GPIO_IN:
	{
		u32 result = (5 << 29) | (1 << 27);
		result |= (m_screen->vblank()) ? 0 : (1 << 25);
		return result;
	}
	}
	return m_taos_regs[offset];
}

void taos_device::taos_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (offset >= TAOS_NUM_REGS)
	{
		logerror("taos_w: offset %x out of range\n", offset);
		return;
	}

	LOGMASKED(LOG_REGISTERS, "TAOS write reg %02X = %08x  (%s)\n", offset, data, machine().describe_context().c_str());
	COMBINE_DATA(&m_taos_regs[offset]);

	if (offset == CONTROL)
	{
		rebuild_params();
	}
}

u32 taos_device::clut_r(offs_t offset)
{
	LOGMASKED(LOG_RAMDAC, "CLUT read offset %08x\n", offset);
	return 0;
}

void taos_device::clut_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_RAMDAC, "CLUT write offset %08x = %08x\n", offset, data);
	m_palette->set_pen_red_level(offset, (data >> 24) & 0xff);
	m_palette->set_pen_green_level(offset, (data >> 16) & 0xff);
	m_palette->set_pen_blue_level(offset, (data >> 8) & 0xff);
}

void taos_device::rebuild_params()
{
	// almost the same derivation as DAFB, except vres is a little different
	m_hres = (m_taos_regs[HFP] >> 20) - (m_taos_regs[HAL] >> 20);
	m_vres = (m_taos_regs[VFPEQ] >> 20) - (m_taos_regs[VAL] >> 20);
	m_htotal = m_taos_regs[HPIX] >> 20;
	m_vtotal = m_taos_regs[VHLINE] >> 20;

	if ((m_htotal > 0) && (m_vtotal > 0))
	{
		const double refresh = double(m_pixel_clock) / double(m_htotal * m_vtotal);
		LOGMASKED(LOG_SWATCH, "hres %d vres %d htotal %d vtotal %d pclk %d refresh %f\n", m_hres, m_vres, m_htotal, m_vtotal, m_pixel_clock, refresh);
		if ((m_hres != 0) && (m_vres != 0))
		{
			rectangle visarea(0, m_hres - 1, 0, m_vres - 1);
			m_screen->configure(m_htotal, m_vtotal, visarea, attotime::from_ticks(m_htotal * m_vtotal, m_pixel_clock).as_attoseconds());
		}
	}
}

