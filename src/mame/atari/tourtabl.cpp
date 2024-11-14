// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

  Atari Tournament Table driver

  Hardware is identical to the VCS2600 except for an extra 6532 chip.

***************************************************************************/

#include "emu.h"

#include "machine/mos6530.h"
#include "cpu/m6502/m6507.h"
#include "machine/watchdog.h"
#include "sound/tiaintf.h"
#include "tia.h"

#include "screen.h"
#include "speaker.h"


namespace {

class tourtabl_state : public driver_device
{
public:
	tourtabl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_riot(*this, "riot%u", 0),
		m_tia(*this, "tia_video"),
		m_tia_inputs(*this, { "PADDLE1", "PADDLE2", "PADDLE3", "PADDLE4", "TIA_IN4", "TIA_IN5" }),
		m_screen(*this, "screen"),
		m_leds(*this, "led%u", 0U)
	{ }

	void tourtabl(machine_config &config);

protected:
	virtual void machine_start() override { m_leds.resolve(); }

private:
	required_device<cpu_device> m_maincpu;
	required_device_array<mos6532_device, 2> m_riot;
	required_device<tia_video_device> m_tia;
	required_ioport_array<6> m_tia_inputs;
	required_device<screen_device> m_screen;
	output_finder<4> m_leds;

	void tourtabl_led_w(uint8_t data);
	uint16_t tourtabl_read_input_port(offs_t offset) { return m_tia_inputs[offset]->read(); }
	uint8_t tourtabl_get_databus_contents(offs_t offset) { return offset; }

	void main_map(address_map &map) ATTR_COLD;
};


void tourtabl_state::tourtabl_led_w(uint8_t data)
{
	m_leds[0] = BIT(data, 6); // start 1
	m_leds[1] = BIT(data, 5); // start 2
	m_leds[2] = BIT(data, 4); // start 4
	m_leds[3] = BIT(data, 7); // select game

	machine().bookkeeping().coin_lockout_global_w(!(data & 0x80));
}


void tourtabl_state::main_map(address_map &map)
{
	map(0x0000, 0x007f).mirror(0x0100).rw(m_tia, FUNC(tia_video_device::read), FUNC(tia_video_device::write));
	map(0x0080, 0x00ff).mirror(0x0100).m(m_riot[0], FUNC(mos6532_device::ram_map));
	map(0x0280, 0x029f).m(m_riot[0], FUNC(mos6532_device::io_map));
	map(0x0400, 0x047f).m(m_riot[1], FUNC(mos6532_device::ram_map));
	map(0x0500, 0x051f).m(m_riot[1], FUNC(mos6532_device::io_map));
	map(0x0800, 0x1fff).rom();
}


static INPUT_PORTS_START( tourtabl )
	PORT_START("PADDLE1") // P1 white
	PORT_BIT( 0xff, 0x8f, IPT_PADDLE ) PORT_MINMAX(0x1f, 0xff) PORT_SENSITIVITY(40) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("PADDLE2") // P2 black
	PORT_BIT( 0xff, 0x8f, IPT_PADDLE ) PORT_MINMAX(0x1f, 0xff) PORT_SENSITIVITY(40) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("PADDLE3") // P3 white, or Breakout P2
	PORT_BIT( 0xff, 0x8f, IPT_PADDLE ) PORT_MINMAX(0x1f, 0xff) PORT_SENSITIVITY(40) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(3)

	PORT_START("PADDLE4") // P4 black, or Breakout P1
	PORT_BIT( 0xff, 0x8f, IPT_PADDLE ) PORT_MINMAX(0x1f, 0xff) PORT_SENSITIVITY(40) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE PORT_PLAYER(4)

	PORT_START("TIA_IN4") // TIA INPT4
	PORT_DIPNAME( 0x80, 0x80, "Breakout Replay" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("TIA_IN5") // TIA INPT5
	PORT_DIPNAME( 0x80, 0x80, "Game Length" )
	PORT_DIPSETTING(    0x00, "11 points (3 balls)" )
	PORT_DIPSETTING(    0x80, "15 points (5 balls)" )

	PORT_START("RIOT0_SWA") // RIOT #0 SWCHA
	PORT_DIPNAME( 0x0f, 0x0e, "Replay Level" )
	PORT_DIPSETTING(    0x0b, "200 points" )
	PORT_DIPSETTING(    0x0c, "250 points" )
	PORT_DIPSETTING(    0x0d, "300 points" )
	PORT_DIPSETTING(    0x0e, "400 points" )
	PORT_DIPSETTING(    0x0f, "450 points" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("RIOT0_SWB") // RIOT #0 SWCHB
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Game Select")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("RIOT1_SWA") // RIOT #1 SWCHA
	PORT_DIPNAME( 0x0f, 0x07, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x00, "Mode A" )
	PORT_DIPSETTING(    0x01, "Mode B" )
	PORT_DIPSETTING(    0x02, "Mode C" )
	PORT_DIPSETTING(    0x03, "Mode D" )
	PORT_DIPSETTING(    0x04, "Mode E" )
	PORT_DIPSETTING(    0x05, "Mode F" )
	PORT_DIPSETTING(    0x06, "Mode G" )
	PORT_DIPSETTING(    0x07, "Mode H" )
	PORT_DIPSETTING(    0x08, "Mode I" )
	PORT_DIPSETTING(    0x09, "Mode J" )
	PORT_DIPSETTING(    0x0a, "Mode K" )
	PORT_DIPSETTING(    0x0b, "Mode L" )
	PORT_DIPSETTING(    0x0c, "Mode M" )
	PORT_DIPSETTING(    0x0d, "Mode N" )
	PORT_DIPSETTING(    0x0e, "Mode O" )
	PORT_DIPSETTING(    0x0f, "Mode P" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x10, DEF_STR( French ) )
	PORT_DIPSETTING(    0x20, DEF_STR( German ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Spanish ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_HIGH )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("RIOT1_SWB") // RIOT #1 SWCHB
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
INPUT_PORTS_END


void tourtabl_state::tourtabl(machine_config &config)
{
	// basic machine hardware
	constexpr XTAL MASTER_CLOCK(3.579575_MHz_XTAL);
	M6507(config, m_maincpu, MASTER_CLOCK / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &tourtabl_state::main_map);

	MOS6532(config, m_riot[0], MASTER_CLOCK / 3);
	m_riot[0]->pa_rd_callback().set_ioport("RIOT0_SWA");
	m_riot[0]->pb_rd_callback().set_ioport("RIOT0_SWB");
	m_riot[0]->pb_wr_callback().set("watchdog", FUNC(watchdog_timer_device::reset_line_w)).bit(0);

	MOS6532(config, m_riot[1], MASTER_CLOCK / 3);
	m_riot[1]->pa_rd_callback().set_ioport("RIOT1_SWA");
	m_riot[1]->pb_rd_callback().set_ioport("RIOT1_SWB");
	m_riot[1]->pb_wr_callback().set(FUNC(tourtabl_state::tourtabl_led_w));

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	TIA_NTSC_VIDEO(config, m_tia, 0, "tia");
	m_tia->read_input_port_callback().set(FUNC(tourtabl_state::tourtabl_read_input_port));
	m_tia->databus_contents_callback().set(FUNC(tourtabl_state::tourtabl_get_databus_contents));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK, 228, 34, 34 + 160, 262, 46, 46 + 200);
	m_screen->set_screen_update(m_tia, FUNC(tia_video_device::screen_update));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	TIA(config, "tia", MASTER_CLOCK/114).add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( tourtabl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "030751.ab2", 0x0800, 0x0800, CRC(4479a6f7) SHA1(bf3fd859614533a592f831e3539ea0a9d1964c82) )
	ROM_LOAD( "030752.ab3", 0x1000, 0x0800, CRC(c92c49dc) SHA1(cafcf13e1b1087b477a667d1e785f5e2be187b0d) )
	ROM_LOAD( "030753.ab4", 0x1800, 0x0800, CRC(3978b269) SHA1(4fa05c655bb74711eb99428f36df838ec70da699) )
ROM_END


ROM_START( tourtab2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "030929.ab2", 0x0800, 0x0800, CRC(fcdfafa2) SHA1(f35ab83366a334a110fbba0cef09f4db950dbb68) )
	ROM_LOAD( "030752.ab3", 0x1000, 0x0800, CRC(c92c49dc) SHA1(cafcf13e1b1087b477a667d1e785f5e2be187b0d) )
	ROM_LOAD( "030753.ab4", 0x1800, 0x0800, CRC(3978b269) SHA1(4fa05c655bb74711eb99428f36df838ec70da699) )
ROM_END

} // anonymous namespace


GAME( 1978, tourtabl, 0,        tourtabl, tourtabl, tourtabl_state, empty_init, ROT0, "Atari", "Tournament Table (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1978, tourtab2, tourtabl, tourtabl, tourtabl, tourtabl_state, empty_init, ROT0, "Atari", "Tournament Table (set 2)", MACHINE_SUPPORTS_SAVE )
