// license:BSD-3-Clause
// copyright-holders:Devin Acker

/*

    Skeleton driver for Yamaha YMW728-F (GEW12) keyboards

    TODO:
    - according to schematics, CPU XTAL is 14MHz, but that makes it run too fast.
      When comparing boot-up scroll speed to a video, it should be around 8MHz,
      7MHz (14/2) is too slow.
      Increasing tick_rate in gew12.cpp will improve scroll speed, but it won't fix
      graphics glitches due to writing to LCD before the busy flag is cleared.

*/

#include "emu.h"

#include "cpu/m6502/gew12.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class psr260_state : public driver_device
{
public:
	psr260_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "lcdc")
		, m_keys(*this, "KEY%u", 0U)
		, m_buttons(*this, "BTN%u", 0U)
	{ }

	void psr260(machine_config& config);

	void lcd_w(u8 data);

private:
	virtual void driver_start() override;

	HD44780_PIXEL_UPDATE(lcd_update);
	void palette_init(palette_device& palette);

	required_device<gew12_device> m_maincpu;
	required_device<ks0066_device> m_lcdc;

	optional_ioport_array<11> m_keys;
	optional_ioport_array<6> m_buttons;

	ioport_value m_key_sel{};
	ioport_value m_btn_sel{};
};

void psr260_state::driver_start()
{
	save_item(NAME(m_key_sel));
	save_item(NAME(m_btn_sel));
}

void psr260_state::lcd_w(u8 data)
{
	m_lcdc->db_w(BIT(data, 1, 4) << 4);
	m_lcdc->rs_w(BIT(data, 6));
	m_lcdc->e_w(BIT(data, 5));
}

HD44780_PIXEL_UPDATE(psr260_state::lcd_update)
{
	if (x < 6 && y < 8 && line < 2 && pos < 8)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}

void psr260_state::palette_init(palette_device& palette)
{
	palette.set_pen_color(0, rgb_t(255, 255, 255));
	palette.set_pen_color(1, rgb_t(0, 0, 0));
}


void psr260_state::psr260(machine_config &config)
{
	GEW12(config, m_maincpu, 8'000'000); // see TODO
	m_maincpu->port_out_cb<5>().set(FUNC(psr260_state::lcd_w));

	// TODO: MIDI in/out

	// LCD
	KS0066(config, m_lcdc, 270'000); // OSC = 91K resistor, TODO: actually KS0066U-10B
	m_lcdc->set_lcd_size(2, 8);
	m_lcdc->set_pixel_update_cb(FUNC(psr260_state::lcd_update));

	// screen (for testing only)
	// TODO: the actual LCD with custom segments
	screen_device& screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6 * 8, 8 * 2);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(psr260_state::palette_init), 2);
}


INPUT_PORTS_START(psr260)
INPUT_PORTS_END

ROM_START( psr79 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "xu66710.bin", 0x00000, 0x100000, CRC(d48f57f1) SHA1(587bfe518e83490e1885c88871c39ff042af9429))
ROM_END

ROM_START( psr260 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "xy43530.bin", 0x00000, 0x200000, CRC(51edbc10) SHA1(fcd2c5b247895c1bd4a185572a60115872763f63))
ROM_END

ROM_START( psr160 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "xy43430.bin", 0x00000, 0x200000, CRC(e234e79b) SHA1(4af36344c2dcc858db110d0ddc5535dbc47a25eb))
ROM_END

} // anonymous namespace

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS          INIT         COMPANY   FULLNAME   FLAGS
SYST( 1998, psr79,   0,      0,      psr260,  psr260, psr260_state,  empty_init,  "Yamaha", "PSR-79",  MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
SYST( 2000, psr260,  0,      0,      psr260,  psr260, psr260_state,  empty_init,  "Yamaha", "PSR-260", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
SYST( 2000, psr160,  psr260, 0,      psr260,  psr260, psr260_state,  empty_init,  "Yamaha", "PSR-160", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
