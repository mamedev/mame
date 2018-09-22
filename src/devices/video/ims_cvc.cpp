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
 *   - everything (skeleton only)
 */

#include "emu.h"
#include "ims_cvc.h"

#include "screen.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(G300, g300_device, "g300", "INMOS G300 Colour Video Controller")
DEFINE_DEVICE_TYPE(G332, g332_device, "g332", "INMOS G332 Colour Video Controller")
DEFINE_DEVICE_TYPE(G364, g364_device, "g364", "INMOS G364 Colour Video Controller")

ims_cvc_device::ims_cvc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_palette_interface(mconfig, *this)
	, m_space_config("shared", ENDIANNESS_LITTLE, 32, 24)
{
}

g300_device::g300_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ims_cvc_device(mconfig, G300, tag, owner, clock)
{
}

g332_device::g332_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: ims_cvc_device(mconfig, type, tag, owner, clock)
{
}

g332_device::g332_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ims_cvc_device(mconfig, G332, tag, owner, clock)
{
}

g364_device::g364_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: g332_device(mconfig, G364, tag, owner, clock)
{
}

device_memory_interface::space_config_vector ims_cvc_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(0, &m_space_config)
	};
}

void g300_device::map(address_map &map)
{
	// datasheet gives unshifted addresses
	const int shift = 2;

	// colour palette
	map(0x000 << shift, (0x0ff << shift) | 0x3).rw(FUNC(g300_device::colour_palette_r), FUNC(g300_device::colour_palette_w));

	// data path registers
	map(0x121, (0x121 << shift) | 0x3).rw(FUNC(g300_device::halfsync_r), FUNC(g300_device::halfsync_w));
	map(0x122, (0x122 << shift) | 0x3).rw(FUNC(g300_device::backporch_r), FUNC(g300_device::backporch_w));
	map(0x123, (0x123 << shift) | 0x3).rw(FUNC(g300_device::display_r), FUNC(g300_device::display_w));
	map(0x124, (0x124 << shift) | 0x3).rw(FUNC(g300_device::shortdisplay_r), FUNC(g300_device::shortdisplay_w));
	map(0x125, (0x125 << shift) | 0x3).rw(FUNC(g300_device::broadpulse_r), FUNC(g300_device::broadpulse_w));
	map(0x126, (0x126 << shift) | 0x3).rw(FUNC(g300_device::vsync_r), FUNC(g300_device::vsync_w));
	map(0x127, (0x127 << shift) | 0x3).rw(FUNC(g300_device::vblank_r), FUNC(g300_device::vblank_w));
	map(0x128, (0x128 << shift) | 0x3).rw(FUNC(g300_device::vdisplay_r), FUNC(g300_device::vdisplay_w));
	map(0x129, (0x129 << shift) | 0x3).rw(FUNC(g300_device::linetime_r), FUNC(g300_device::linetime_w));
	map(0x12a, (0x12a << shift) | 0x3).rw(FUNC(g300_device::tos_r), FUNC(g300_device::tos_w));
	map(0x12b, (0x12b << shift) | 0x3).rw(FUNC(g300_device::meminit_r), FUNC(g300_device::meminit_w));
	map(0x12c, (0x12c << shift) | 0x3).rw(FUNC(g300_device::transferdelay_r), FUNC(g300_device::transferdelay_w));

	map(0x140 << shift, (0x140 << shift) | 0x3).rw(FUNC(g300_device::mask_r), FUNC(g300_device::mask_w));
	map(0x160 << shift, (0x160 << shift) | 0x3).rw(FUNC(g300_device::control_r), FUNC(g300_device::control_w));
	map(0x180 << shift, (0x180 << shift) | 0x3).rw(FUNC(g300_device::tos_r), FUNC(g300_device::tos_w));
	map(0x1a0 << shift, (0x1a0 << shift) | 0x3).w(FUNC(g300_device::boot_w));
}

void g332_device::map(address_map &map)
{
	// datasheet gives unshifted addresses
	const int shift = 2; // TODO: 64 bit mode

	map(0x000 << shift, (0x000 << shift) | 0x3).w(FUNC(g332_device::boot_w));

	// data path registers
	map(0x021, (0x021 << shift) | 0x3).rw(FUNC(g332_device::halfsync_r), FUNC(g332_device::halfsync_w));
	map(0x022, (0x022 << shift) | 0x3).rw(FUNC(g332_device::backporch_r), FUNC(g332_device::backporch_w));
	map(0x023, (0x023 << shift) | 0x3).rw(FUNC(g332_device::display_r), FUNC(g332_device::display_w));
	map(0x024, (0x024 << shift) | 0x3).rw(FUNC(g332_device::shortdisplay_r), FUNC(g332_device::shortdisplay_w));
	map(0x025, (0x025 << shift) | 0x3).rw(FUNC(g332_device::broadpulse_r), FUNC(g332_device::broadpulse_w));
	map(0x026, (0x026 << shift) | 0x3).rw(FUNC(g332_device::vsync_r), FUNC(g332_device::vsync_w));
	map(0x027, (0x027 << shift) | 0x3).rw(FUNC(g332_device::vpreequalise_r), FUNC(g332_device::vpreequalise_w));
	map(0x028, (0x028 << shift) | 0x3).rw(FUNC(g332_device::vpostequalise_r), FUNC(g332_device::vpostequalise_w));
	map(0x029, (0x029 << shift) | 0x3).rw(FUNC(g332_device::vblank_r), FUNC(g332_device::vblank_w));
	map(0x02a, (0x02a << shift) | 0x3).rw(FUNC(g332_device::vdisplay_r), FUNC(g332_device::vdisplay_w));
	map(0x02b, (0x02b << shift) | 0x3).rw(FUNC(g332_device::linetime_r), FUNC(g332_device::linetime_w));
	map(0x02c, (0x02c << shift) | 0x3).rw(FUNC(g332_device::linestart_r), FUNC(g332_device::linestart_w));
	map(0x02d, (0x02d << shift) | 0x3).rw(FUNC(g332_device::meminit_r), FUNC(g332_device::meminit_w));
	map(0x02e, (0x02e << shift) | 0x3).rw(FUNC(g332_device::transferdelay_r), FUNC(g332_device::transferdelay_w));

	map(0x040 << shift, (0x040 << shift) | 0x3).rw(FUNC(g332_device::mask_r), FUNC(g332_device::mask_w));
	map(0x060 << shift, (0x060 << shift) | 0x3).rw(FUNC(g332_device::control_a_r), FUNC(g332_device::control_a_w));
	map(0x070 << shift, (0x070 << shift) | 0x3).rw(FUNC(g332_device::control_b_r), FUNC(g332_device::control_b_w));
	map(0x080 << shift, (0x080 << shift) | 0x3).rw(FUNC(g332_device::tos_r), FUNC(g332_device::tos_w));

	// cursor palette (0a1-0a3)
	// checksum registers (0c0-0c2)

	// colour palette
	map(0x100 << shift, (0x1ff << shift) | 0x3).rw(FUNC(g332_device::colour_palette_r), FUNC(g332_device::colour_palette_w));

	// cursor store (200-3ff)
	// cursor position (0c7)
}

void ims_cvc_device::device_start()
{
	m_space = &space(0);
}

void ims_cvc_device::device_reset()
{
}

void ims_cvc_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
}
