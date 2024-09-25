// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Novag VIP & Super VIP

NOTE: Turn the power switch off before exiting MAME, otherwise NVRAM won't save
properly.

It's a portable chess computer with keypad input, no embedded chessboard. The
chess engine is similar to other HD6301/3Y ones by David Kittinger. The 'sequels'
of this portable design (Ruby, Sapphire, ..) are on H8.

TODO:
- if/when MAME supports an exit callback, hook up power-off switch to that
- add more Super System peripherals, each has their own MCU
- unmapped reads from 0x3* range, same as snova driver

BTANB:
- on nvip, if you press GO right after entering a move (during the opening book),
  it will reply by moving one of your pieces, taken from the board layout before
  your move
- just for fun, I'll mention that on many (not all) batches of Super VIP, the
  white text label under the Knight button says "Into" instead of "Info"

================================================================================

Novag VIP (aka V.I.P.) (model 872)
----------------------------------

Hardware notes:
- PCB label: 100078
- Hitachi HD6301Y0F (mode 2) @ 8MHz
- 2KB RAM (NEC D449G-15)
- LCD with 4 7segs and custom segments, no LCD chip
- 24 buttons, piezo

The LCD is the same as the one in Primo / Supremo / Super Nova.

Even though VIP was released first, the functionality of the Super VIP was
already planned according to advertisements. VIP is basically a stripped-down
version, running at lower CPU speed.

================================================================================

Novag Super VIP (aka Super V.I.P.) (model 895)
----------------------------------------------

Hardware notes:
- PCB label: 100115
- Hitachi HD6301Y0F (mode 2) @ 9.83MHz
- 32KB ROM(TC57256AD-12), 2KB RAM(TC5517CFL-20)
- same LCD and button panel as VIP, RJ-12 port

Super VIP is Novag's first chess computer with a proper serial interface. It
connects to the Novag Super System Distributor, which can then connect to an
external chessboard, TV interface, computer, etc. They are basically RS-232
devices, just with a different plug.

Serial transmission format for Novag Super System is 1 start bit, 8 data bits,
1 stop bit, no parity. On Super VIP, the baud rate is selectable 1200 or 9600,
default 9600 for v3.x, 1200 for v1.x.

Known official Novag Super System (or compatible) peripherals:
- Super System Chess Board Auto Sensory
- Super System Chess Board Touch Sensory
- Super System TV-Interface
- Universal Electronic Chess Board (aka UCB)

*******************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/m6800/m6801.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "novag_svip.lh"
#include "novag_vip.lh"


namespace {

class vip_state : public driver_device
{
public:
	vip_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_lcd_pwm(*this, "lcd_pwm"),
		m_dac(*this, "dac"),
		m_rs232(*this, "rs232"),
		m_inputs(*this, "IN.%u", 0),
		m_out_lcd(*this, "s%u.%u", 0U, 0U)
	{ }

	void vip(machine_config &config);
	void svip(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(power_off) { if (newval) m_power = false; }
	ioport_value power_r() { return m_power ? 1 : 0; }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override { m_power = true; }

private:
	// devices/pointers
	required_device<hd6301y0_cpu_device> m_maincpu;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<dac_2bit_ones_complement_device> m_dac;
	optional_device<rs232_port_device> m_rs232;
	required_ioport_array<4> m_inputs;
	output_finder<4, 10> m_out_lcd;

	bool m_power = false;
	bool m_lcd_strobe = false;
	u8 m_inp_mux = 0;
	u8 m_select = 0;

	void vip_map(address_map &map) ATTR_COLD;
	void svip_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void lcd_pwm_w(offs_t offset, u8 data);
	u8 p2_r();
	void p2_w(u8 data);
	void p5_w(u8 data);
	void p6_w(u8 data);
};

void vip_state::machine_start()
{
	m_out_lcd.resolve();

	// register for savestates
	save_item(NAME(m_power));
	save_item(NAME(m_lcd_strobe));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_select));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void vip_state::lcd_pwm_w(offs_t offset, u8 data)
{
	m_out_lcd[offset & 0x3f][offset >> 6] = data;
}

u8 vip_state::p2_r()
{
	u8 data = 0;

	// P20: keyboard lock switch
	// P23 (nvip) / P22 (nsvip): power switch
	data |= m_inputs[3]->read();

	// P23 (nsvip): serial rx
	if (m_rs232)
		data |= m_rs232->rxd_r() ? 0 : 8;

	// P25-P27: multiplexed inputs
	for (int i = 0; i < 3; i++)
		if (m_inp_mux & m_inputs[i]->read())
			data |= 0x20 << i;

	return ~data;
}

void vip_state::p2_w(u8 data)
{
	// P21: 4066 in/out to LCD
	if (m_lcd_strobe && ~data & 2)
	{
		u16 lcd_data = (~m_select << 2 & 0x300) | m_inp_mux;
		m_lcd_pwm->matrix(m_select & 0xf, lcd_data);
	}
	m_lcd_strobe = bool(data & 2);

	// P24 (nsvip): serial tx (TTL)
	if (m_rs232)
		m_rs232->write_txd(BIT(data, 4));
}

void vip_state::p5_w(u8 data)
{
	// P50-P53: 4066 control to LCD
	// P56,P57: lcd data
	m_select = data;

	// P54,P55: speaker out
	m_dac->write(data >> 4 & 3);
}

void vip_state::p6_w(u8 data)
{
	// P60-P67: input mux, lcd data
	m_inp_mux = ~data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void vip_state::vip_map(address_map &map)
{
	map(0x4000, 0x47ff).mirror(0x3800).ram().share("nvram");
}

void vip_state::svip_map(address_map &map)
{
	map(0x2000, 0x27ff).mirror(0x1800).ram().share("nvram");
	map(0x4000, 0xbfff).rom().region("eprom", 0);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( vip )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("GO")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Restore")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Hint")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_NAME("C/CB")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Color")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Verify/Setup")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("Random")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("NG")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Left / Nextbest")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Right / Auto")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Pawn / Level")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Knight / Info")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Bishop / Easy")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Rook / Solvemate")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Queen / Sound")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("King / Referee")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("A 1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("B 2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("C 3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("D 4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("E 5")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("F 6")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("G 7")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("H 8")

	PORT_START("IN.3")
	PORT_CONFNAME( 0x01, 0x00, "Keyboard Lock")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(vip_state, power_r)

	PORT_START("POWER") // needs to be triggered for nvram to work
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF) PORT_CHANGED_MEMBER(DEVICE_SELF, vip_state, power_off, 0)
INPUT_PORTS_END

static INPUT_PORTS_START( svip )
	PORT_INCLUDE( vip )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Restore / Replay")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Hint / Human")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Color / Video")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("Random / Autoclock")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Left / Next Best")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Right / Autoplay")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Pawn / Level")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Knight / Info / Echo")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Bishop / Easy / Moves")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Rook / Solvemate / Baud")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Queen / Sound / Game")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("King / Referee / Board")

	PORT_MODIFY("IN.3")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(vip_state, power_r)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void vip_state::vip(machine_config &config)
{
	// basic machine hardware
	HD6301Y0(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &vip_state::vip_map);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(hd6301y0_cpu_device::nvram_set_battery));
	m_maincpu->standby_cb().append([this](int state) { if (state) m_lcd_pwm->clear(); });
	m_maincpu->in_p2_cb().set(FUNC(vip_state::p2_r));
	m_maincpu->out_p2_cb().set(FUNC(vip_state::p2_w));
	m_maincpu->out_p5_cb().set(FUNC(vip_state::p5_w));
	m_maincpu->out_p6_cb().set(FUNC(vip_state::p6_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	PWM_DISPLAY(config, m_lcd_pwm).set_size(4, 10);
	m_lcd_pwm->output_x().set(FUNC(vip_state::lcd_pwm_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/3, 606/3);
	screen.set_visarea_full();

	config.set_default_layout(layout_novag_vip);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_2BIT_ONES_COMPLEMENT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.125);
}

void vip_state::svip(machine_config &config)
{
	vip(config);

	// basic machine hardware
	m_maincpu->set_clock(9.8304_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &vip_state::svip_map);

	config.set_default_layout(layout_novag_svip);

	// rs232 (configure after video)
	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( nvip )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("novag_872_31y0c81f", 0x0000, 0x4000, CRC(45f6d397) SHA1(c08ff786bcf8a37baa41a7401adf69a9d1bde81b) )

	ROM_REGION( 36256, "screen", 0 )
	ROM_LOAD("nvip.svg", 0, 36256, CRC(3373e0d5) SHA1(25bfbf0405017388c30f4529106baccb4723bc6b) )
ROM_END


ROM_START( nsvip ) // serial 7604xx
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("novag_895_31y0rk75f", 0x0000, 0x4000, CRC(11a3894e) SHA1(fa455181043ac3ee89412aae563171d87ac55596) )

	ROM_REGION( 0x8000, "eprom", 0 ) // ID = V3.7
	ROM_LOAD("yellow", 0x4000, 0x4000, CRC(278e98f0) SHA1(a080e2300c90b6daf42fef960642885eb00ee607) ) // no label
	ROM_CONTINUE(      0x0000, 0x4000 )

	ROM_REGION( 36256, "screen", 0 )
	ROM_LOAD("nvip.svg", 0, 36256, CRC(3373e0d5) SHA1(25bfbf0405017388c30f4529106baccb4723bc6b) )
ROM_END

ROM_START( nsvipa ) // serial 7569xx
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("novag_895_31y0rk75f", 0x0000, 0x4000, CRC(11a3894e) SHA1(fa455181043ac3ee89412aae563171d87ac55596) )

	ROM_REGION( 0x8000, "eprom", 0 ) // ID = V3.6
	ROM_LOAD("v_117", 0x4000, 0x4000, CRC(2a60a474) SHA1(5dd0a1871fcbbb2a6271f284562a62f98bf9bf93) )
	ROM_CONTINUE(     0x0000, 0x4000 )

	ROM_REGION( 36256, "screen", 0 )
	ROM_LOAD("nvip.svg", 0, 36256, CRC(3373e0d5) SHA1(25bfbf0405017388c30f4529106baccb4723bc6b) )
ROM_END

ROM_START( nsvipb ) // serial 7545xx
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("novag_895_31y0rk75f", 0x0000, 0x4000, CRC(11a3894e) SHA1(fa455181043ac3ee89412aae563171d87ac55596) )

	ROM_REGION( 0x8000, "eprom", 0 ) // ID = V1.03
	ROM_LOAD("sv_111", 0x4000, 0x4000, CRC(4be9a23f) SHA1(1fb8ccec112e485eedd899cbe8c0c2cb99e840a4) )
	ROM_CONTINUE(      0x0000, 0x4000 )

	ROM_REGION( 36256, "screen", 0 )
	ROM_LOAD("nvip.svg", 0, 36256, CRC(3373e0d5) SHA1(25bfbf0405017388c30f4529106baccb4723bc6b) )
ROM_END

ROM_START( nsvipc ) // serial 7524xx
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("novag_895_31y0rk75f", 0x0000, 0x4000, CRC(11a3894e) SHA1(fa455181043ac3ee89412aae563171d87ac55596) )

	ROM_REGION( 0x8000, "eprom", 0 ) // ID = V1.01
	ROM_LOAD("s_vip_920", 0x4000, 0x4000, CRC(6f61b1e1) SHA1(df74a599eb59b113ef60c9fc23f9c603a5de4f9e) )
	ROM_CONTINUE(         0x0000, 0x4000 )

	ROM_REGION( 36256, "screen", 0 )
	ROM_LOAD("nvip.svg", 0, 36256, CRC(3373e0d5) SHA1(25bfbf0405017388c30f4529106baccb4723bc6b) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY, FULLNAME, FLAGS
SYST( 1987, nvip,   0,      0,      vip,     vip,   vip_state, empty_init, "Novag Industries / Intelligent Heuristic Programming", "VIP (Novag)", MACHINE_SUPPORTS_SAVE )

SYST( 1989, nsvip,  0,      0,      svip,    svip,  vip_state, empty_init, "Novag Industries / Intelligent Heuristic Programming", "Super VIP (v3.7)", MACHINE_SUPPORTS_SAVE )
SYST( 1989, nsvipa, nsvip,  0,      svip,    svip,  vip_state, empty_init, "Novag Industries / Intelligent Heuristic Programming", "Super VIP (v3.6)", MACHINE_SUPPORTS_SAVE )
SYST( 1989, nsvipb, nsvip,  0,      svip,    svip,  vip_state, empty_init, "Novag Industries / Intelligent Heuristic Programming", "Super VIP (v1.03)", MACHINE_SUPPORTS_SAVE )
SYST( 1989, nsvipc, nsvip,  0,      svip,    svip,  vip_state, empty_init, "Novag Industries / Intelligent Heuristic Programming", "Super VIP (v1.01)", MACHINE_SUPPORTS_SAVE )
