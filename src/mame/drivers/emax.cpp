// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

    Skeleton driver for E-mu Emax & Emax II samplers.

***********************************************************************************************************************************/

#include "emu.h"
#include "cpu/ns32000/ns32000.h"
#include "machine/6850acia.h"
#include "machine/eepromser.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"

class emax_state : public driver_device
{
public:
	emax_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "lcdc")
	{
	}

	void emax(machine_config &config);
	void emaxp(machine_config &config);
	void emax2(machine_config &config);

private:
	HD44780_PIXEL_UPDATE(pixel_update);

	void palette_init(palette_device &palette);

	void emax_map(address_map &map);
	void emaxp_map(address_map &map);
	void emax2_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
};

HD44780_PIXEL_UPDATE(emax_state::pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 16)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}


void emax_state::emax_map(address_map &map)
{
	map(0x000000, 0x000fff).rom().region("bootprom", 0);
	map(0x008100, 0x0081ff).ram();
	map(0x010000, 0x017fff).ram();
	map(0x822400, 0x822400).nopr();
	map(0x824004, 0x824004).nopw();
	map(0x824006, 0x824006).nopw();
	map(0x890000, 0x890000).w(m_lcdc, FUNC(hd44780_device::control_w));
	map(0x890002, 0x890002).r(m_lcdc, FUNC(hd44780_device::control_r));
	map(0x890004, 0x890004).w(m_lcdc, FUNC(hd44780_device::data_w));
	map(0x890006, 0x890006).r(m_lcdc, FUNC(hd44780_device::data_r));
}

void emax_state::emaxp_map(address_map &map)
{
	map(0x000000, 0x001fff).rom().region("bootprom", 0);
	map(0x008100, 0x0081ff).ram();
	map(0x010000, 0x017fff).ram();
	map(0x822400, 0x822400).nopr();
	map(0x824004, 0x824004).nopw();
	map(0x824006, 0x824006).nopw();
	map(0x890000, 0x890000).w(m_lcdc, FUNC(hd44780_device::control_w));
	map(0x890002, 0x890002).r(m_lcdc, FUNC(hd44780_device::control_r));
	map(0x890004, 0x890004).w(m_lcdc, FUNC(hd44780_device::data_w));
	map(0x890006, 0x890006).r(m_lcdc, FUNC(hd44780_device::data_r));
}

void emax_state::emax2_map(address_map &map)
{
	map(0x000000, 0x003fff).rom().region("bootprom", 0);
	map(0x008000, 0x01ffff).ram();
	map(0x0a8018, 0x0a801b).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);
	map(0x0a801e, 0x0a801f).nopr();
	map(0x0b0000, 0x0b0000).w(m_lcdc, FUNC(hd44780_device::control_w));
	map(0x0b0002, 0x0b0002).r(m_lcdc, FUNC(hd44780_device::control_r));
	map(0x0b0004, 0x0b0004).w(m_lcdc, FUNC(hd44780_device::data_w));
	map(0x0b0006, 0x0b0006).r(m_lcdc, FUNC(hd44780_device::data_r));
	map(0x8e8000, 0x8e82ff).ram();
}


static INPUT_PORTS_START(emax)
INPUT_PORTS_END

static INPUT_PORTS_START(emax2)
INPUT_PORTS_END

void emax_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(131, 136, 139));
	palette.set_pen_color(1, rgb_t( 92,  83,  88));
}

void emax_state::emax(machine_config &config)
{
	NS32008(config, m_maincpu, 8'000'000); // NS32008D-8
	m_maincpu->set_addrmap(AS_PROGRAM, &emax_state::emax_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_screen_update(m_lcdc, FUNC(hd44780_device::screen_update));
	screen.set_size(16*6, 16);
	screen.set_visarea(0, 16*6-1, 0, 16-1);
	screen.set_palette("palette");

	HD44780(config, m_lcdc);
	m_lcdc->set_lcd_size(2, 16);
	m_lcdc->set_pixel_update_cb(FUNC(emax_state::pixel_update));

	PALETTE(config, "palette", FUNC(emax_state::palette_init), 2);

	// TODO: add other peripherals
}

void emax_state::emaxp(machine_config &config)
{
	emax(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &emax_state::emaxp_map);

	// TODO: add SCSI
}

void emax_state::emax2(machine_config &config)
{
	NS32016(config, m_maincpu, 20_MHz_XTAL / 2); // NS32CG16V-10 (FIXME)
	m_maincpu->set_addrmap(AS_PROGRAM, &emax_state::emax2_map);

	EEPROM_93C06_16BIT(config, "eeprom"); // NMC93C06N

	ACIA6850(config, "acia");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_screen_update(m_lcdc, FUNC(hd44780_device::screen_update));
	screen.set_size(16*6, 16);
	screen.set_visarea(0, 16*6-1, 0, 16-1);
	screen.set_palette("palette");

	HD44780(config, m_lcdc);
	m_lcdc->set_lcd_size(2, 16);
	m_lcdc->set_pixel_update_cb(FUNC(emax_state::pixel_update));

	PALETTE(config, "palette", FUNC(emax_state::palette_init), 2);

	// TODO: add other unknown peripherals
}

ROM_START(emax)
	ROM_REGION(0x1000, "bootprom", 0) // v2, Rev C mainboard, non-SE/HD version
	ROM_LOAD("emax.bin", 0x0000, 0x1000, CRC(b55210aa) SHA1(9b02dfc28700e07be5e044d53035041a54732927))

	ROM_REGION(0x104, "cspal", 0)
	ROM_LOAD("ip345c.bin", 0x000, 0x104, CRC(7bae1347) SHA1(a49ab0bae41132e60c113d2117c5a042c2a1e44d))
ROM_END

ROM_START(emaxp)
	ROM_REGION(0x2000, "bootprom", 0) // SCSI upgrade
	ROM_LOAD("ip424a3089.bin", 0x0000, 0x2000, CRC(3abd3a16) SHA1(8d7ac39c8147bdc2ead9fedee463d1bbe94332c5))

	ROM_REGION(0x104, "cspal", 0)
	ROM_LOAD("ip345c.bin", 0x000, 0x104, CRC(7bae1347) SHA1(a49ab0bae41132e60c113d2117c5a042c2a1e44d))

	ROM_REGION(0x104, "timpal", 0)
	ROM_LOAD("ip379a.bin", 0x000, 0x104, CRC(fb50f8bd) SHA1(5b8b7904736188c4cf8b36a4bf5ad685422ec760))
ROM_END

ROM_START(emax2)
	ROM_REGION16_LE(0x4000, "bootprom", 0)
	ROM_LOAD16_BYTE("ip43aemu_3891.ic20", 0x0000, 0x2000, CRC(51fdccb8) SHA1(0cab6540ed5d03ba202569b8730e0ec6dce1a477)) // Am27C64-250DC
	ROM_LOAD16_BYTE("ip43bemu_4291.ic19", 0x0001, 0x2000, CRC(810160b3) SHA1(6f490f9014bc221e047ccd77428b002d0a3c3168)) // Am27C64-250DC

	ROM_REGION16_LE(0x20, "eeprom", 0)
	ROM_LOAD("93c06n.ic24", 0x00, 0x20, CRC(403ef05b) SHA1(893ef614127ac1898d8ac529521f87ff62207138))
ROM_END

SYST(1986, emax,  0,    0, emax,  emax,  emax_state, empty_init, "E-mu Systems", "Emax Digital Sampling Keyboard", MACHINE_IS_SKELETON)
SYST(198?, emaxp, emax, 0, emaxp, emax,  emax_state, empty_init, "E-mu Systems", "Emax Plus Digital Sampling Keyboard", MACHINE_IS_SKELETON)
SYST(1989, emax2, 0,    0, emax2, emax2, emax_state, empty_init, "E-mu Systems", "Emax II 16-Bit Digital Sound System", MACHINE_IS_SKELETON)
