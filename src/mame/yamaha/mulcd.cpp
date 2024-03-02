// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert

// HD44780/LCD image combo used in the yamaha mus, vl70m, fs1r and
// probably others
//
// Yamaha module name: DM113Z-5BL3

#include "emu.h"
#include "mulcd.h"

#include "mulcd.lh"

DEFINE_DEVICE_TYPE(MULCD,   mulcd_device,   "mulcd",   "Yamaha MU/VL70/FS1R common LCD")

ROM_START( mulcd )
	ROM_REGION( 525021, "screen", 0)
	ROM_LOAD( "mulcd.svg", 0, 525021, CRC(81eba091) SHA1(d998f4b508555ddd56b187a868311cd34f28e077))

	ROM_REGION( 0x1000, "hd44780", 0)
	// Hand made, 3 characters unused
	ROM_LOAD( "mu100-font.bin", 0x0000, 0x1000, BAD_DUMP CRC(a7d6c1d6) SHA1(9f0398d678bdf607cb34d83ee535f3b7fcc97c41) )
ROM_END

const tiny_rom_entry *mulcd_device::device_rom_region() const
{
	return ROM_NAME(mulcd);
}

mulcd_device::mulcd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MULCD, tag, owner, clock),
	m_lcd(*this, "hd44780"),
	m_outputs(*this, "%03x.%d.%d", 0U, 0U, 0U),
	m_contrast(*this, "contrast"),
	m_led_outputs(*this, "LED%d", 0U)
{
}

void mulcd_device::device_start()
{
	m_outputs.resolve();
	m_contrast.resolve();
	m_led_outputs.resolve();
}

void mulcd_device::device_reset()
{
	set_contrast(0);
	set_leds(0);
}

void mulcd_device::set_contrast(u8 contrast)
{
	// 0 to 7
	m_contrast = contrast;
}

void mulcd_device::set_leds(u8 leds)
{
	for(int x=0; x != 6; x++)
		m_led_outputs[x] = (leds >> x) & 1;
}

void mulcd_device::render_w(int state)
{
	if(!state)
		return;

	const u8 *render = m_lcd->render();
	for(int x=0; x != 64; x++) {
		for(int y=0; y != 8; y++) {
			u8 v = *render++;
			for(int z=0; z != 5; z++)
				m_outputs[x][y][z] = (v >> z) & 1;
		}
		render += 8;
	}
}

void mulcd_device::device_add_mconfig(machine_config &config)
{
	HD44780(config, m_lcd, 270'000); // HD44780B04; 91K surface-mount resistor connected to OSC
	m_lcd->set_lcd_size(4, 20);

	auto &screen = SCREEN(config, "screen", SCREEN_TYPE_SVG);
	screen.set_refresh_hz(60);
	screen.set_size(1920/1.5, 580/1.5);
	screen.set_visarea_full();
	screen.screen_vblank().set(FUNC(mulcd_device::render_w));

	config.set_default_layout(layout_mulcd);
}
