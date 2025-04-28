// license:BSD-3-Clause
// copyright-holders:F. Ulivi

/*********************************************************************

    VIDEO80 homebrew 80-column video card

    This is a 80-column video card that I designed and built in 1988.
    Here's a summary of its features:
    - 80 columns by 24 rows of text video
    - 8x8 character cell
    - 16 MHz dot clock, 50 Hz frame rate
    - Based on MC6845 CRT controller
    - 4k of SRAM: half for the framebuffer & half for the character
      generator.
    - 4k of EPROM: it contains the firmware and the compressed font.
      FW supports seamless operation in both BASIC and MSXDOS
      environments. No driver or additional sw is needed.

*********************************************************************/

#include "emu.h"
#include "video80.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"

// Debugging
#define VERBOSE 0
#include "logmacro.h"

namespace {

// Dot clock
constexpr auto DOT_CLOCK = 16_MHz_XTAL;

class msx_cart_video80_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_video80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, MSX_CART_VIDEO80, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_palette(*this, "palette")
		, m_crtc(*this, "crtc")
		, m_screen(*this, "screen1")
		, m_cpu_waiting(false)
	{
	}

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void de_w(int state);
	uint8_t vram_r(offs_t addr);
	void vram_w(offs_t addr, uint8_t data);
	uint8_t crtc_r(offs_t addr);
	void crtc_w(offs_t addr, uint8_t data);

private:
	static inline constexpr unsigned VRAM_SIZE = 4096;

	required_device<palette_device> m_palette;
	required_device<mc6845_device> m_crtc;
	required_device<screen_device> m_screen;

	uint8_t m_vram[VRAM_SIZE];
	bool m_cpu_waiting;

	MC6845_UPDATE_ROW(crtc_update_row);
};

std::error_condition msx_cart_video80_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_video80_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	if (cart_rom_region()->bytes() != 0x1000)
	{
		message = "msx_cart_video80_device: Region 'rom' has invalid size.";
		return image_error::INVALIDLENGTH;
	}

	page(1)->install_rom(0x4000, 0x4fff, cart_rom_region()->base());
	page(1)->install_read_handler(0x5000, 0x5fff, emu::rw_delegate(*this, FUNC(msx_cart_video80_device::vram_r)));
	page(1)->install_write_handler(0x5000, 0x5fff, emu::rw_delegate(*this, FUNC(msx_cart_video80_device::vram_w)));
	page(1)->install_read_handler(0x6000, 0x6fff, emu::rw_delegate(*this, FUNC(msx_cart_video80_device::crtc_r)));
	page(1)->install_write_handler(0x6000, 0x6fff, emu::rw_delegate(*this, FUNC(msx_cart_video80_device::crtc_w)));

	return std::error_condition();
}

void msx_cart_video80_device::device_start()
{
	save_item(NAME(m_vram));
	save_item(NAME(m_cpu_waiting));
}

void msx_cart_video80_device::device_add_mconfig(machine_config &config)
{
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER, rgb_t::green());
	m_screen->set_raw(DOT_CLOCK, 1024, 0, 640, 312, 0, 192);
	m_screen->set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));

	MC6845(config, m_crtc, DOT_CLOCK / 8);
	m_crtc->set_char_width(8);
	m_crtc->set_show_border_area(false);
	m_crtc->set_screen(m_screen);
	m_crtc->set_update_row_callback(FUNC(msx_cart_video80_device::crtc_update_row));
	m_crtc->out_de_callback().set(FUNC(msx_cart_video80_device::de_w));
}

void msx_cart_video80_device::de_w(int state)
{
	if (m_cpu_waiting && !state)
	{
		maincpu().set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);
		m_cpu_waiting = false;
	}
}

uint8_t msx_cart_video80_device::vram_r(offs_t addr)
{
	if (!machine().side_effects_disabled() && m_crtc->de_r())
	{
		maincpu().set_input_line(Z80_INPUT_LINE_WAIT, ASSERT_LINE);
		maincpu().defer_access();
		m_cpu_waiting = true;
		return 0;
	}
	else
	{
		return m_vram[addr];
	}
}

void msx_cart_video80_device::vram_w(offs_t addr, uint8_t data)
{
	if (!machine().side_effects_disabled() && m_crtc->de_r())
	{
		maincpu().set_input_line(Z80_INPUT_LINE_WAIT, ASSERT_LINE);
		maincpu().defer_access();
		m_cpu_waiting = true;
	}
	else
	{
		m_vram[addr] = data;
	}
}

uint8_t msx_cart_video80_device::crtc_r(offs_t addr)
{
	uint8_t res = 0;

	if (BIT(addr, 1))
	{
		if (BIT(addr, 0))
			res = m_crtc->register_r();
		else
			LOG("Reading from non-existing reg!\n");
	}
	else
	{
		LOG("Reading from a write-only address!\n");
	}

	return res;
}

void msx_cart_video80_device::crtc_w(offs_t addr, uint8_t data)
{
	if (BIT(addr, 1))
	{
		LOG("Writing to a read-only address!\n");
	}
	else
	{
		if (BIT(addr, 0))
			m_crtc->register_w(data);
		else
			m_crtc->address_w(data);
	}
}

MC6845_UPDATE_ROW(msx_cart_video80_device::crtc_update_row)
{
	pen_t const *const pen = m_palette->pens();
	for (int i = 0; i < x_count; i++)
	{
		uint8_t const char_code = m_vram[(ma + i) & 0x7ff];
		uint8_t pixels = m_vram[(unsigned(char_code) << 3) | 0x800 | (ra & 7)];
		bool const cursor = cursor_x == i;
		if (cursor)
			pixels = ~pixels;

		for (unsigned col = 0; col < 8; col++)
			bitmap.pix(y, i * 8 + col) = pen[BIT(pixels, 7 - col)];
	}
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_VIDEO80, msx_cart_interface, msx_cart_video80_device, "msx_cart_video80", "MSX Cartridge - VIDEO80")
