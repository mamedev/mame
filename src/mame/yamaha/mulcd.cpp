// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert

// HD44780B04/LCD image combo used in the yamaha mus, vl70m, fs1r and
// probably others
//
// LCD layout and font validated against a real mu50
//
// Yamaha module name: DM113Z-5BL3

#include "emu.h"
#include "mulcd.h"

#include "mulcd.lh"

DEFINE_DEVICE_TYPE(MULCD,   mulcd_device,   "mulcd",   "Yamaha MU/VL70/FS1R common LCD")

ROM_START( mulcd )
	ROM_REGION( 525261, "screen", 0)
	ROM_LOAD( "mulcd.svg", 0, 525261, CRC(fb3c68ed) SHA1(e18bd29d25b8e5d025ec107adc37021e1f5e85e1))

	ROM_REGION( 0x1000, "cgrom", 0)
	ROM_LOAD( "hd44780u_b04.bin", 0x0000, 0x1000, CRC(126ed6da) SHA1(2ff0899bfee7795ba52a3d56c96edf31d9e6a3f9))
ROM_END

const tiny_rom_entry *mulcd_device::device_rom_region() const
{
	return ROM_NAME(mulcd);
}

mulcd_device::mulcd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	hd44780_base_device(mconfig, MULCD, tag, owner, clock),
	m_outputs(*this, "%03x.%d.%d", 0U, 0U, 0U),
	m_contrast(*this, "contrast"),
	m_led_outputs(*this, "LED%d", 0U)
{
	set_lcd_size(4, 20);
}

void mulcd_device::device_start()
{
	hd44780_base_device::device_start();
	m_outputs.resolve();
	m_contrast.resolve();
	m_led_outputs.resolve();
}

void mulcd_device::device_reset()
{
	hd44780_base_device::device_reset();
	set_contrast(0);
	set_leds(0);
}

void mulcd_device::set_contrast(u8 contrast)
{
	// 0 to 7
	m_contrast = contrast;
}

void mulcd_device::set_leds(u16 leds)
{
	for(int x=0; x != 10; x++)
		m_led_outputs[x] = (leds >> x) & 1;
}

u32 mulcd_device::mu_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const u8 *image = render();
	for(int x=0; x != 64; x++) {
		for(int y=0; y != 8; y++) {
			u8 v = *image++;
			for(int z=0; z != 5; z++)
				m_outputs[x][y][z] = (v >> z) & 1;
		}
		image += 8;
	}

	return 0;
}

void mulcd_device::device_add_mconfig(machine_config &config)
{
	auto &screen = SCREEN(config, "screen", SCREEN_TYPE_SVG);
	screen.set_refresh_hz(60);
	screen.set_size(1920/1.5, 580/1.5);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(mulcd_device::mu_screen_update));

	config.set_default_layout(layout_mulcd);
}
