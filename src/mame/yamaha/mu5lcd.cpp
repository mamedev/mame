// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert

// LC7985/LCD image combo used in the yamaha mu5 and mu15

#include "emu.h"
#include "mu5lcd.h"

DEFINE_DEVICE_TYPE(MU5LCD,   mu5lcd_device,   "mu5lcd",   "Yamaha MU5/MU15 common LCD")

ROM_START( mu5lcd )
	ROM_REGION(261774, "screen", 0)
	ROM_LOAD("mu5lcd.svg", 0, 261774, CRC(3cccbb88) SHA1(3db0b16f27b501ff8d8ac3fb631dd315571230d3))
ROM_END

const tiny_rom_entry *mu5lcd_device::device_rom_region() const
{
	return ROM_NAME(mu5lcd);
}

mu5lcd_device::mu5lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MU5LCD, tag, owner, clock),
	m_lcd(*this, "lcd"),
	m_outputs(*this, "%x.%x.%x.%x", 0U, 0U, 0U, 0U)
{
}

void mu5lcd_device::device_start()
{
	m_outputs.resolve();
}

void mu5lcd_device::device_reset()
{
}

u32 mu5lcd_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const u8 *render = m_lcd->render();
	for(int y=0; y != 2; y++)
		for(int x=0; x != 8; x++)
			for(int yy=0; yy != 8; yy++) {
				u8 v = render[8 * y + 16 * x + yy];
				for(int xx=0; xx != 5; xx++)
					m_outputs[y][x][yy][xx] = (v >> xx) & 1;
			}

	return 0;
}

void mu5lcd_device::device_add_mconfig(machine_config &config)
{
	LC7985(config, m_lcd);

	auto &screen = SCREEN(config, "screen", SCREEN_TYPE_SVG);
	screen.set_refresh_hz(60);
	screen.set_size(800, 435);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(mu5lcd_device::screen_update));
}
