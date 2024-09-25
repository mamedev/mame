// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, hap
// thanks-to:Berger
/*******************************************************************************

SciSys/Saitek Stratos chesscomputer family (1987-1990)
(SciSys renamed themselves to Saitek in 1987)

- Stratos
- Turbo King
- Corona --> corona.cpp

IMPORTANT: The user is expected to press the STOP button to turn off the computer.
When not using -autosave, press that button before exiting MAME, or NVRAM can get
corrupt. If that happens, the chesscomputer will become unresponsive on next boot.
To force a cold boot, hold the PLAY button and trigger a power on/reset (F3).

TODO:
- emulate LCD at lower level, probably an MCU with embedded LCDC
- LCD status bit handling is guessed. stratos expects it to be high after lcd
  command 0xf, but tking2 won't work if it's done that way, and corona differs too
- irq timing is derived from the main XTAL, but result should be similar with 5MHz
  and 5.67MHz, there are a couple of "FREQ. SEL" nodes on the PCB, maybe related
  (not the ones in input ports). irq source should be from HELIOS pin 2
- tking(old revisions) and stratos slow responsive buttons, related to irq timing,
  but if that's changed, the led blinking and in-game clock is too fast
- does nvram.u7 work? it's cleared during boot, but not used after

================================================================================

Hardware notes:
- W65C02 or R65C02 at 5MHz or ~5.6MHz (for latter, box says 6MHz but that's a marketing lie)
- 2*32KB ROM + optional 32KB Endgame ROM sold separately
- 8KB RAM + another 8KB RAM(latter not populated on every PCB)
- NEC gate array for all I/O, Saitek calls it HELIOS
- side leds are tri-color (red + green, combined = yellow)
- unknown LCDC under epoxy blob, suspected to be an MCU

Stratos/Turbo King are identical.
Corona has magnet sensors and two HELIOS chips.

There is no official Saitek program versioning for these. The D/D+ versions are
known since they're the same chess engine as later Saitek modules, such as the
Analyst module. Likewise, officially there isn't a "Turbo King II" or "Corona II",
these 'sequels' are titled as such by the chesscomputer community. Saitek simply
advertised them as an improved program.

The initial Stratos/Turbo King (PRG ROM labels known: M,K,L,P) are probably
engine version B, very few bytes difference between revisions. The first Corona
is engine version C.

*******************************************************************************/

#include "emu.h"
#include "stratos.h"

#include "cpu/m6502/m65c02.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "softlist_dev.h"
#include "speaker.h"

// internal artwork
#include "saitek_stratos.lh"
#include "saitek_tking.lh"


class stratos_state : public stratos_base_state
{
public:
	stratos_state(const machine_config &mconfig, device_type type, const char *tag) :
		stratos_base_state(mconfig, type, tag),
		m_banked_nvram(*this, "nvram.u7", 0x2000, ENDIANNESS_LITTLE),
		m_nvrambank(*this, "nvrambank"),
		m_rombank(*this, "rombank"),
		m_extrom(*this, "extrom"),
		m_board(*this, "board"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	int lcd_ready_r() { return m_lcd_ready ? 1 : 0; }

	// machine configs
	void stratos(machine_config &config);
	void tking(machine_config &config);
	void tking2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices/pointers
	memory_share_creator<u8> m_banked_nvram;
	required_memory_bank m_nvrambank;
	required_memory_bank m_rombank;
	required_device<generic_slot_device> m_extrom;
	required_device<sensorboard_device> m_board;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<8+2> m_inputs;

	u8 m_select = 0;
	u8 m_control = 0;
	u8 m_led_data = 0;

	void main_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void update_leds();
	void select_w(u8 data);
	u8 chessboard_r();
	void sound_w(u8 data);
	void leds_w(u8 data);
	u8 control_r();
	void control_w(u8 data);
	u8 lcd_data_r();
	u8 extrom_r(offs_t offset);
};


// stratos_base_state

void stratos_base_state::machine_start()
{
	// resolve handlers
	m_out_digit.resolve();
	m_out_lcd.resolve();

	// register for savestates
	save_item(NAME(m_power));
	save_item(NAME(m_lcd_ready));
	save_item(NAME(m_lcd_count));
	save_item(NAME(m_lcd_command));
	save_item(NAME(m_lcd_data));
}

void stratos_base_state::machine_reset()
{
	m_power = true;
	m_lcd_ready = false;
	m_lcd_count = 0;
	clear_lcd();
}

INPUT_CHANGED_MEMBER(stratos_base_state::change_cpu_freq)
{
	// known officially* released CPU speeds: 5MHz, 5.626MHz, 5.67MHz
	// *not including reseller overclocks, user mods, or the "Turbo Kit"
	static const XTAL xtal[3] = { 5_MHz_XTAL, 5.626_MHz_XTAL, 5.67_MHz_XTAL };
	m_maincpu->set_unscaled_clock(xtal[newval % 3]);
}


// stratos_state

void stratos_state::machine_start()
{
	stratos_base_state::machine_start();

	// init banks
	m_rombank->configure_entries(0, 2, memregion("maincpu")->base(), 0x8000);
	m_nvrambank->configure_entries(0, 2, m_banked_nvram, 0x1000);

	// register for savestates
	save_item(NAME(m_select));
	save_item(NAME(m_control));
	save_item(NAME(m_led_data));
}

void stratos_state::machine_reset()
{
	stratos_base_state::machine_reset();

	m_control = 0;
	m_rombank->set_entry(0);
	m_nvrambank->set_entry(0);
}



/*******************************************************************************
    I/O
*******************************************************************************/

// soft power on/off

INPUT_CHANGED_MEMBER(stratos_base_state::go_button)
{
	if (newval && !m_power)
	{
		m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		machine_reset();
	}
}

void stratos_base_state::power_off()
{
	m_power = false;
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	// clear display
	m_display->clear();
	clear_lcd();
	update_lcd();
}


// LCD HLE

void stratos_base_state::update_lcd()
{
	// output individual segments
	for (int i = 0; i < 0x40; i++)
		for (int j = 0; j < 4; j++)
			m_out_lcd[i >> 4][i & 0xf][j] = BIT(m_lcd_data[i], j);

	m_out_digit[0] = (m_lcd_data[0] & 4) ? 6 : 0; // "1"

	// upper digits (colon is at 0x00)
	for (int i = 0; i < 4; i++)
		m_out_digit[i + 1] = (m_lcd_data[0x01 + i * 2] << 4 | m_lcd_data[0x01 + i * 2 + 1]) & 0x7f;

	// lower digits (colon is at 0x10)
	for (int i = 0; i < 4; i++)
		m_out_digit[i + 5] = (m_lcd_data[0x11 + i * 2] << 4 | m_lcd_data[0x11 + i * 2 + 1]) & 0x7f;
}

void stratos_base_state::lcd_data_w(u8 data)
{
	// d0-d3: lcd data
	// d4-d7: unused?
	data &= 0xf;

	if (m_lcd_count == 0)
		m_lcd_command = data;
	else
	{
		// write to lcd row
		if (m_lcd_command > 0 && m_lcd_command <= 4)
			m_lcd_data[(((m_lcd_command - 1) << 4) + (m_lcd_count - 1)) & 0x3f] = data;
	}

	// it expects a specific number of writes for each row
	const u8 maxcount[5] = { 0, 9, 9, 1, 12 };
	if (m_lcd_command > 4 || m_lcd_count == maxcount[m_lcd_command])
	{
		// reset/start?
		if (m_lcd_command & 8)
			m_lcd_ready = true;

		m_lcd_count = 0;
		update_lcd();
	}
	else
		m_lcd_count++;
}


// HELIOS

void stratos_state::update_leds()
{
	m_display->matrix_partial(0, 2, 1 << (m_control >> 5 & 1), (~m_led_data & 0xff) | (~m_control << 6 & 0x100));
}

void stratos_state::leds_w(u8 data)
{
	// d0-d7: button led data
	m_led_data = data;
	update_leds();

	m_dac->write(0); // guessed
}

void stratos_state::sound_w(u8 data)
{
	m_dac->write(1);
}

void stratos_state::select_w(u8 data)
{
	// d0-d3: input/led mux
	// d4-d7: chessboard led data
	m_select = data;
	m_display->matrix_partial(2, 4, ~m_select >> 4 & 0xf, 1 << (m_select & 0xf));
}

u8 stratos_state::chessboard_r()
{
	// d0-d7: chessboard sensors
	return ~m_board->read_file(m_select & 0xf);
}

u8 stratos_state::control_r()
{
	u8 data = 0;
	u8 sel = m_select & 0xf;

	if (sel == 8)
	{
		// d5: lcd status flag?
		data |= m_inputs[9]->read();

		if (!machine().side_effects_disabled())
			m_lcd_ready = false;

		// d7: battery low
		data |= m_inputs[8]->read() << 7;
	}

	// read button panel
	if (sel < 8)
		data |= m_inputs[sel]->read() << 5;

	return data;
}

void stratos_state::control_w(u8 data)
{
	u8 prev = m_control;
	m_control = data;

	// d0: main rom bank
	// d1: ext rom bank
	// d1: nvram bank
	m_rombank->set_entry(data & 1);
	m_nvrambank->set_entry(data >> 1 & 1);

	// d2: mode led state
	// d5: button led select
	update_leds();

	// d6 falling edge: power-off request
	if (~data & prev & 0x40)
		power_off();
}

u8 stratos_state::extrom_r(offs_t offset)
{
	u16 bank = BIT(m_control, 1) * 0x4000;
	return (m_extrom->exists()) ? m_extrom->read_rom(offset | bank) : 0xff;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void stratos_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram.u6");
	map(0x2000, 0x2000).w(FUNC(stratos_state::select_w));
	map(0x2200, 0x2200).rw(FUNC(stratos_state::chessboard_r), FUNC(stratos_state::sound_w));
	map(0x2400, 0x2400).w(FUNC(stratos_state::leds_w));
	map(0x2600, 0x2600).rw(FUNC(stratos_state::control_r), FUNC(stratos_state::control_w));
	map(0x2800, 0x37ff).bankrw("nvrambank");
	map(0x3800, 0x3800).w(FUNC(stratos_state::lcd_data_w));
	map(0x4000, 0x7fff).r(FUNC(stratos_state::extrom_r));
	map(0x8000, 0xffff).bankr("rombank");
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

INPUT_PORTS_START( saitek_stratos )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Set Up")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM) // freq sel

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Sound")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Stop")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Play")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Tab / Color")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_EQUALS) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Function")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM) // freq sel

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Library")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Info")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM) // freq sel

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Analysis")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Normal")

	PORT_START("IN.8")
	PORT_CONFNAME( 0x01, 0x01, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Low" )
	PORT_CONFSETTING(    0x01, DEF_STR( Normal ) )

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CHANGED_MEMBER(DEVICE_SELF, stratos_base_state, go_button, 0) PORT_NAME("Go")

	PORT_START("CPU")
	PORT_CONFNAME( 0x03, 0x00, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, stratos_base_state, change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x00, "5MHz" )
	PORT_CONFSETTING(    0x01, "5.626MHz" )
	PORT_CONFSETTING(    0x02, "5.67MHz" )
INPUT_PORTS_END

static INPUT_PORTS_START( stratos )
	PORT_INCLUDE( saitek_stratos )

	PORT_MODIFY("IN.6")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_CUSTOM)

	PORT_START("IN.9")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, stratos_state, lcd_ready_r)
INPUT_PORTS_END

static INPUT_PORTS_START( tking ) // same buttons, but different locations
	PORT_INCLUDE( stratos )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Set Up")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_L) PORT_NAME("Level")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Sound")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Stop")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")

	PORT_MODIFY("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Play Normal")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Tab / Color")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")

	PORT_MODIFY("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_CODE(KEYCODE_EQUALS) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Function")

	PORT_MODIFY("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Library")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Info")

	PORT_MODIFY("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Analysis")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Normal")

	PORT_MODIFY("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_CHANGED_MEMBER(DEVICE_SELF, stratos_base_state, go_button, 0) PORT_NAME("Go")
INPUT_PORTS_END

static INPUT_PORTS_START( tking2 )
	PORT_INCLUDE( tking )

	PORT_MODIFY("IN.5")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_CUSTOM)

	PORT_MODIFY("IN.9")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_CUSTOM)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void stratos_state::stratos(machine_config &config)
{
	// basic machine hardware
	M65C02(config, m_maincpu, 5_MHz_XTAL); // see change_cpu_freq
	m_maincpu->set_addrmap(AS_PROGRAM, &stratos_state::main_map);
	m_maincpu->set_periodic_int(FUNC(stratos_state::irq0_line_hold), attotime::from_hz(76));

	NVRAM(config, "nvram.u6", nvram_device::DEFAULT_ALL_0);
	NVRAM(config, "nvram.u7", nvram_device::DEFAULT_ALL_0);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(350));
	m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(2+4, 8+1);
	config.set_default_layout(layout_saitek_stratos);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);

	// extension rom
	GENERIC_SOCKET(config, "extrom", generic_plain_slot, "saitek_egr");
	SOFTWARE_LIST(config, "cart_list").set_original("saitek_egr");
}

void stratos_state::tking(machine_config &config)
{
	stratos(config);
	config.set_default_layout(layout_saitek_tking);
}

void stratos_state::tking2(machine_config &config)
{
	tking(config);
	m_maincpu->set_periodic_int(FUNC(stratos_state::irq0_line_hold), attotime::from_hz(107));

	// seems much more responsive (not just because of higher irq rate)
	m_board->set_delay(attotime::from_msec(200));
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( stratos )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("w1yo1f_728m_u3.u3", 0x0000, 0x8000, CRC(b58a7256) SHA1(75b3a3a65f4ca8d52aa5b17a06319bff59d9014f) )
	ROM_LOAD("bw1_819n_u4.u4", 0x8000, 0x8000, CRC(cb0de631) SHA1(f78d40213be21775966cbc832d64acd9b73de632) )
ROM_END

ROM_START( stratosa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("w1yo1f_728l_u3.u3", 0x0000, 0x8000, CRC(19a22058) SHA1(a5ca54d870c70b7ce9c7be2951800bf49cc57527) )
	ROM_LOAD("bw1_819n_u4.u4", 0x8000, 0x8000, CRC(cb0de631) SHA1(f78d40213be21775966cbc832d64acd9b73de632) )
ROM_END


ROM_START( tking ) // PCB rev. 10
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("yo1f_713d_u3.u3", 0x0000, 0x8000, CRC(b8c6d853) SHA1(98923f44bbbd2ea17c269850971d3df229e6057e) )
	ROM_LOAD("yo1f_712a_u4.u4", 0x8000, 0x8000, CRC(7d3f8f7b) SHA1(8be5d8d988ff0577ccfec0a773bfd94599f2534f) )
ROM_END

ROM_START( tkinga ) // PCB rev. 7
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("w1yo1f_728p_u3.u3", 0x0000, 0x8000, CRC(ad77f83e) SHA1(598fdb1e40267d9d43a3d8f287723070b9afa349) )
	ROM_LOAD("yo1f-b_819o_u4.u4", 0x8000, 0x8000, CRC(336040d4) SHA1(aca662b8cc4d6bafd61ca158c768ba8896117169) )
ROM_END

ROM_START( tkingb ) // PCB rev. 3, also PCB rev. 5
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("w1yo1f_728l_u3.u3", 0x0000, 0x8000, CRC(19a22058) SHA1(a5ca54d870c70b7ce9c7be2951800bf49cc57527) )
	ROM_LOAD("yo1f-b_819o_u4.u4", 0x8000, 0x8000, CRC(336040d4) SHA1(aca662b8cc4d6bafd61ca158c768ba8896117169) )
ROM_END



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT   COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1987, stratos,  0,       0,      stratos, stratos, stratos_state, empty_init, "SciSys / Heuristic Software", "Kasparov Stratos (set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1987, stratosa, stratos, 0,      stratos, stratos, stratos_state, empty_init, "SciSys / Heuristic Software", "Kasparov Stratos (set 2)", MACHINE_SUPPORTS_SAVE )

SYST( 1990, tking,    stratos, 0,      tking2,  tking2,  stratos_state, empty_init, "Saitek / Heuristic Software", "Kasparov Turbo King (ver. D)", MACHINE_SUPPORTS_SAVE ) // aka Turbo King II
SYST( 1988, tkinga,   stratos, 0,      tking,   tking,   stratos_state, empty_init, "Saitek / Heuristic Software", "Kasparov Turbo King (ver. B, set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1988, tkingb,   stratos, 0,      tking,   tking,   stratos_state, empty_init, "Saitek / Heuristic Software", "Kasparov Turbo King (ver. B, set 2)", MACHINE_SUPPORTS_SAVE )
