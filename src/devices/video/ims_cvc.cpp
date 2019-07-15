// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An implementation of the INMOS G300, G332 and G364 CVC (Colour Video
 * Controller) devices.
 *
 * References:
 *
 *   http://bitsavers.org/components/inmos/graphics/72-TRN-204-01_Graphics_Databook_Second_Edition_1990.pdf
 *
 * TODO
 *   - cursor
 */

#include "emu.h"
#include "ims_cvc.h"

#include "screen.h"

#define LOG_GENERAL (1U << 0)
#define LOG_CONFIG  (1U << 1)

//#define VERBOSE (LOG_GENERAL|LOG_CONFIG)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(G300, g300_device, "g300", "INMOS G300 Colour Video Controller")
DEFINE_DEVICE_TYPE(G332, g332_device, "g332", "INMOS G332 Colour Video Controller")
DEFINE_DEVICE_TYPE(G364, g364_device, "g364", "INMOS G364 Colour Video Controller")

ims_cvc_device::ims_cvc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_palette_interface(mconfig, *this)
	, m_screen(*this, finder_base::DUMMY_TAG)
	, m_vram(*this, finder_base::DUMMY_TAG)
{
}

g300_device::g300_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ims_cvc_device(mconfig, G300, tag, owner, clock)
{
}

g332_device::g332_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: ims_cvc_device(mconfig, type, tag, owner, clock)
	, m_microport(*this, "microport")
{
}

g332_device::g332_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: g332_device(mconfig, G332, tag, owner, clock)
{
}

g364_device::g364_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: g332_device(mconfig, G364, tag, owner, clock)
{
}

void g332_device::device_add_mconfig(machine_config &config)
{
	ADDRESS_MAP_BANK(config, m_microport).set_map(&g332_device::microport_map).set_options(ENDIANNESS_LITTLE, 32, 32);
}

void g300_device::map(address_map &map)
{
	// datasheet gives unshifted addresses
	const int shift = 2;

	// colour palette
	map(0x000 << shift, (0x0ff << shift) | 0x3).rw(FUNC(g300_device::colour_palette_r), FUNC(g300_device::colour_palette_w));

	// data path registers
	map(0x121 << shift, (0x121 << shift) | 0x3).rw(FUNC(g300_device::halfsync_r), FUNC(g300_device::halfsync_w));
	map(0x122 << shift, (0x122 << shift) | 0x3).rw(FUNC(g300_device::backporch_r), FUNC(g300_device::backporch_w));
	map(0x123 << shift, (0x123 << shift) | 0x3).rw(FUNC(g300_device::display_r), FUNC(g300_device::display_w));
	map(0x124 << shift, (0x124 << shift) | 0x3).rw(FUNC(g300_device::shortdisplay_r), FUNC(g300_device::shortdisplay_w));
	map(0x125 << shift, (0x125 << shift) | 0x3).rw(FUNC(g300_device::broadpulse_r), FUNC(g300_device::broadpulse_w));
	map(0x126 << shift, (0x126 << shift) | 0x3).rw(FUNC(g300_device::vsync_r), FUNC(g300_device::vsync_w));
	map(0x127 << shift, (0x127 << shift) | 0x3).rw(FUNC(g300_device::vblank_r), FUNC(g300_device::vblank_w));
	map(0x128 << shift, (0x128 << shift) | 0x3).rw(FUNC(g300_device::vdisplay_r), FUNC(g300_device::vdisplay_w));
	map(0x129 << shift, (0x129 << shift) | 0x3).rw(FUNC(g300_device::linetime_r), FUNC(g300_device::linetime_w));
	map(0x12a << shift, (0x12a << shift) | 0x3).rw(FUNC(g300_device::tos_r), FUNC(g300_device::tos_w));
	map(0x12b << shift, (0x12b << shift) | 0x3).rw(FUNC(g300_device::meminit_r), FUNC(g300_device::meminit_w));
	map(0x12c << shift, (0x12c << shift) | 0x3).rw(FUNC(g300_device::transferdelay_r), FUNC(g300_device::transferdelay_w));

	map(0x140 << shift, (0x140 << shift) | 0x3).rw(FUNC(g300_device::mask_r), FUNC(g300_device::mask_w));
	map(0x160 << shift, (0x160 << shift) | 0x3).rw(FUNC(g300_device::control_r), FUNC(g300_device::control_w));
	map(0x180 << shift, (0x180 << shift) | 0x3).rw(FUNC(g300_device::tos_r), FUNC(g300_device::tos_w));
	map(0x1a0 << shift, (0x1a0 << shift) | 0x3).w(FUNC(g300_device::boot_w));
}

void g332_device::map(address_map &map)
{
	map(0x0000, 0x1fff).m(m_microport, FUNC(address_map_bank_device::amap32));
}

void g332_device::microport_map(address_map &map)
{
	// datasheet uses unshifted addresses: configure the device map for 64 bit
	// address mode, bank device does handles additional shift for 32 bit mode
	const int shift = 3;

	map(0x000 << shift, (0x000 << shift) | 0x7).w(FUNC(g332_device::boot_w));

	// data path registers
	map(0x021 << shift, (0x021 << shift) | 0x7).rw(FUNC(g332_device::halfsync_r), FUNC(g332_device::halfsync_w));
	map(0x022 << shift, (0x022 << shift) | 0x7).rw(FUNC(g332_device::backporch_r), FUNC(g332_device::backporch_w));
	map(0x023 << shift, (0x023 << shift) | 0x7).rw(FUNC(g332_device::display_r), FUNC(g332_device::display_w));
	map(0x024 << shift, (0x024 << shift) | 0x7).rw(FUNC(g332_device::shortdisplay_r), FUNC(g332_device::shortdisplay_w));
	map(0x025 << shift, (0x025 << shift) | 0x7).rw(FUNC(g332_device::broadpulse_r), FUNC(g332_device::broadpulse_w));
	map(0x026 << shift, (0x026 << shift) | 0x7).rw(FUNC(g332_device::vsync_r), FUNC(g332_device::vsync_w));
	map(0x027 << shift, (0x027 << shift) | 0x7).rw(FUNC(g332_device::vpreequalise_r), FUNC(g332_device::vpreequalise_w));
	map(0x028 << shift, (0x028 << shift) | 0x7).rw(FUNC(g332_device::vpostequalise_r), FUNC(g332_device::vpostequalise_w));
	map(0x029 << shift, (0x029 << shift) | 0x7).rw(FUNC(g332_device::vblank_r), FUNC(g332_device::vblank_w));
	map(0x02a << shift, (0x02a << shift) | 0x7).rw(FUNC(g332_device::vdisplay_r), FUNC(g332_device::vdisplay_w));
	map(0x02b << shift, (0x02b << shift) | 0x7).rw(FUNC(g332_device::linetime_r), FUNC(g332_device::linetime_w));
	map(0x02c << shift, (0x02c << shift) | 0x7).rw(FUNC(g332_device::linestart_r), FUNC(g332_device::linestart_w));
	map(0x02d << shift, (0x02d << shift) | 0x7).rw(FUNC(g332_device::meminit_r), FUNC(g332_device::meminit_w));
	map(0x02e << shift, (0x02e << shift) | 0x7).rw(FUNC(g332_device::transferdelay_r), FUNC(g332_device::transferdelay_w));

	map(0x040 << shift, (0x040 << shift) | 0x7).rw(FUNC(g332_device::mask_r), FUNC(g332_device::mask_w));
	map(0x060 << shift, (0x060 << shift) | 0x7).rw(FUNC(g332_device::control_a_r), FUNC(g332_device::control_a_w));
	map(0x070 << shift, (0x070 << shift) | 0x7).rw(FUNC(g332_device::control_b_r), FUNC(g332_device::control_b_w));
	map(0x080 << shift, (0x080 << shift) | 0x7).rw(FUNC(g332_device::tos_r), FUNC(g332_device::tos_w));

	// cursor palette (0a1-0a3)
	map(0x0a1 << shift, (0x0a3 << shift) | 0x7).rw(FUNC(g332_device::cursor_palette_r), FUNC(g332_device::cursor_palette_w));

	// checksum registers (0c0-0c2)

	// colour palette
	map(0x100 << shift, (0x1ff << shift) | 0x7).rw(FUNC(g332_device::colour_palette_r), FUNC(g332_device::colour_palette_w));

	// cursor store (200-3ff)
	// cursor position (0c7)
}

void ims_cvc_device::device_start()
{
	save_item(NAME(m_halfsync));
	save_item(NAME(m_backporch));
	save_item(NAME(m_display));
	save_item(NAME(m_shortdisplay));
	save_item(NAME(m_broadpulse));
	save_item(NAME(m_vsync));
	save_item(NAME(m_vblank));
	save_item(NAME(m_vdisplay));
	save_item(NAME(m_linetime));
	save_item(NAME(m_meminit));
	save_item(NAME(m_transferdelay));

	save_item(NAME(m_mask));
	save_item(NAME(m_tos));
	save_item(NAME(m_boot));
}

void ims_cvc_device::device_reset()
{
}

void g300_device::device_start()
{
	ims_cvc_device::device_start();

	save_item(NAME(m_control));
}

void g332_device::device_start()
{
	ims_cvc_device::device_start();

	save_item(NAME(m_vpreequalise));
	save_item(NAME(m_vpostequalise));
	save_item(NAME(m_linestart));
	save_item(NAME(m_control_a));
	save_item(NAME(m_control_b));
}

void g332_device::device_reset()
{
	m_control_a = 0;
}

u32 g300_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	offs_t address = m_tos;

	for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
		for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x++)
			bitmap.pix(y, x) = pen_color(m_vram->read(address++) & m_mask);

	return 0;
}

u32 g332_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	offs_t address = m_tos;

	switch (m_control_a & PIXEL_BITS)
	{
	case BPP_1:
		for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
			for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x += 8)
			{
				u8 pixel_data = m_vram->read(address++);

				bitmap.pix(y, x + 0) = pen_color(pixel_data & 0x1 & m_mask); pixel_data >>= 1;
				bitmap.pix(y, x + 1) = pen_color(pixel_data & 0x1 & m_mask); pixel_data >>= 1;
				bitmap.pix(y, x + 2) = pen_color(pixel_data & 0x1 & m_mask); pixel_data >>= 1;
				bitmap.pix(y, x + 3) = pen_color(pixel_data & 0x1 & m_mask); pixel_data >>= 1;
				bitmap.pix(y, x + 4) = pen_color(pixel_data & 0x1 & m_mask); pixel_data >>= 1;
				bitmap.pix(y, x + 5) = pen_color(pixel_data & 0x1 & m_mask); pixel_data >>= 1;
				bitmap.pix(y, x + 6) = pen_color(pixel_data & 0x1 & m_mask); pixel_data >>= 1;
				bitmap.pix(y, x + 7) = pen_color(pixel_data & 0x1 & m_mask);
			}
		break;

	case BPP_2:
		for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
			for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x += 4)
			{
				u8 pixel_data = m_vram->read(address++);

				bitmap.pix(y, x + 0) = pen_color(pixel_data & 0x3 & m_mask); pixel_data >>= 2;
				bitmap.pix(y, x + 1) = pen_color(pixel_data & 0x3 & m_mask); pixel_data >>= 2;
				bitmap.pix(y, x + 2) = pen_color(pixel_data & 0x3 & m_mask); pixel_data >>= 2;
				bitmap.pix(y, x + 3) = pen_color(pixel_data & 0x3 & m_mask);
			}
		break;

	case BPP_4:
		for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
			for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x += 2)
			{
				u8 pixel_data = m_vram->read(address++);

				bitmap.pix(y, x + 0) = pen_color(pixel_data & 0xf & m_mask); pixel_data >>= 4;
				bitmap.pix(y, x + 1) = pen_color(pixel_data & 0xf & m_mask);
			}
		break;

	case BPP_8:
		for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
			for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x++)
				bitmap.pix(y, x) = pen_color(m_vram->read(address++) & m_mask);
		break;
	}

	return 0;
}

u32 ims_cvc_device::colour_palette_r(offs_t offset)
{
	return 0;
}

void ims_cvc_device::colour_palette_w(offs_t offset, u32 data, u32 mem_mask)
{
	set_pen_color(offset >> 1, data >> 16, data >> 8, data >> 0);
}

u32 g332_device::cursor_palette_r(offs_t offset)
{
	return 0;
}

void g332_device::cursor_palette_w(offs_t offset, u32 data, u32 mem_mask)
{
	set_pen_color(256 + (offset >> 1), data >> 16, data >> 8, data >> 0);
}

void g332_device::boot_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOG("boot_w %s clock, multiplier %d, %d bit alignment (%s)\n",
		(data & PLL_SELECT) ? "PLL" : "external", (data & PLL_MULTIPLIER),
		(data & ALIGN_64) ? 64 : 32, machine().describe_context());

	m_microport->set_shift((data & ALIGN_64) ? 0 : 1);

	mem_mask &= 0x00ffffffU;
	COMBINE_DATA(&m_boot);
}

void g332_device::control_a_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOG("control_a_w 0x%08x (%s)\n", data, machine().describe_context());

	mem_mask &= 0x00ffffffU;
	COMBINE_DATA(&m_control_a);

	if (data & VTG_ENABLE)
	{
		LOGMASKED(LOG_CONFIG, "VTG %s, %s, %s mode\n",
			(data & VTG_ENABLE) ? "enabled" : "disabled",
			(data & INTL_ENABLE) ? ((data & INTL_FORMAT) ? "interlaced (CCIR)" : "interlaced (EIA)") : "non-interlaced",
			(data & SLAVE_MODE) ? "slave" : "master");

		LOGMASKED(LOG_CONFIG, "%s sync, %s digital sync, analogue %s\n",
			(data & SYNC_PATTERN) ? "plain" : "tesselated",
			(data & SYNC_FORMAT) ? "separate" : "composite",
			(data & VIDEO_FORMAT) ? "video only" : "composite video + sync");

		LOGMASKED(LOG_CONFIG, "%s, CBlank is %s, %s, %s\n",
			(data & BLANK_LEVEL) ? "blanking pedestal" : "no blank pedestal",
			(data & BLANK_IO) ? "ouput" : "input",
			(data & BLANK_FUNC) ? "undelayed ClkDisable" : "delayed CBlank",
			(data & BLANK_FORCE) ? "screen blanked" : (data & BLANK_DISABLE) ? "blanking disabled" : "blanking enabled");

		LOGMASKED(LOG_CONFIG, "address increment %d, DMA %s, sync delay %d cycles\n",
			(data & ADDR_INC) == INC_1 ? 1 :
			(data & ADDR_INC) == INC_256 ? (data & INTL_ENABLE) ? 2 : 256 :
			(data & ADDR_INC) == INC_512 ? 512 : 1024,
			(data & DMA_DISABLE) ? "disabled" : "enabled",
			(data & SYNC_DELAY) >> 15);

		LOGMASKED(LOG_CONFIG, "interleave %s, pixel sampling %s, %s bits per pixel, cursor %s\n",
			(data & INTERLEAVE) ? "enabled" : "disabled",
			(data & SAMPLE_DELAY) ? "delayed" : "standard",
			(data & PIXEL_BITS) == BPP_1 ? "1" :
			(data & PIXEL_BITS) == BPP_2 ? "2" :
			(data & PIXEL_BITS) == BPP_4 ? "4" :
			(data & PIXEL_BITS) == BPP_8 ? "8" :
			(data & PIXEL_BITS) == BPP_15 ? "15" :
			(data & PIXEL_BITS) == BPP_16 ? "16" : "unknown",
			(data & CURSOR_DISABLE) ? "disabled" : "enabled");

		LOG("display %d vdisplay %d\n", m_display, m_vdisplay);
		LOG("linetime %d halfsync %d backporch %d broadpulse %d\n", m_linetime, m_halfsync, m_backporch, m_broadpulse);
		LOG("vsync %d vpreequalise %d vpostequalise %d vblank %d\n", m_vsync, m_vpreequalise, m_vpostequalise, m_vblank);

		int const hbend = (m_halfsync + m_halfsync + m_backporch) << 2;
		int const vbend = (m_vpreequalise + m_vpostequalise + m_vsync + m_vblank) >> 1;
		int const width = m_linetime << 2;
		int const height = vbend + (m_vdisplay >> 1);

		rectangle const visarea(hbend, hbend + (m_display << 2) - 1, vbend, height - 1);

		u32 const dotclock = (m_boot & PLL_SELECT) ? clock() * (m_boot & PLL_MULTIPLIER) : clock();
		attotime const refresh = attotime::from_hz(dotclock / (width * height));

		m_screen->configure(width, height, visarea, refresh.as_attoseconds());
		m_screen->reset_origin();
	}
}
