// license:BSD-3-Clause
// copyright-holders:hap, Lord Nightmare
/***************************************************************************

  Texas Instruments Speak & Spell hardware

***************************************************************************/

#include "emu.h"
#include "cpu/tms0980/tms0980.h"
#include "sound/tms5110.h"
#include "machine/tms6100.h"

#include "tispeak.lh"


class tispeak_state : public driver_device
{
public:
	tispeak_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tms5100(*this, "tms5100"),
		m_tms6100(*this, "tms6100"),
		m_filoff_timer(*this, "filoff"),
		m_button_matrix(*this, "IN")
	{ }

	required_device<tms0270_cpu_device> m_maincpu;
	required_device<tms5100_device> m_tms5100;
	required_device<tms6100_device> m_tms6100;
	required_device<timer_device> m_filoff_timer;
	required_ioport_array<9> m_button_matrix;

	UINT16 m_r;
	UINT16 m_o;
	int m_filament_on;
	int m_power_on;

	UINT16 m_digit_state[9];
	void display_update();
	TIMER_DEVICE_CALLBACK_MEMBER(delayed_filament_off);

	DECLARE_READ8_MEMBER(snspell_read_k);
	DECLARE_WRITE16_MEMBER(snmath_write_o);
	DECLARE_WRITE16_MEMBER(snspell_write_o);
	DECLARE_WRITE16_MEMBER(snspell_write_r);

	DECLARE_INPUT_CHANGED_MEMBER(power_button);
	DECLARE_WRITE_LINE_MEMBER(auto_power_off);
	void power_off();

	virtual void machine_reset();
	virtual void machine_start();
};



/***************************************************************************

  VFD Display

***************************************************************************/

// The device strobes the filament-enable very fast, it is unnoticeable to the user.
// To prevent flickering here, we need to simulate a decay.

// decay time in milliseconds
#define FILOFF_DECAY_TIME 20

TIMER_DEVICE_CALLBACK_MEMBER(tispeak_state::delayed_filament_off)
{
	// turn off display
	m_filament_on = 0;
	display_update();
}

void tispeak_state::display_update()
{
	// filament on/off
	if (m_r & 0x8000)
	{
		m_filament_on = 1;
		m_filoff_timer->reset();
	}
	else if (m_filament_on && m_filoff_timer->time_left() == attotime::never)
	{
		// schedule delayed filament-off
		m_filoff_timer->adjust(attotime::from_msec(FILOFF_DECAY_TIME));
	}
	
	// update digit state
	for (int i = 0; i < 9; i++)
		if (m_r >> i & 1)
			m_digit_state[i] = m_o;

	// send to output
	for (int i = 0; i < 9; i++)
	{
		// standard led14seg
		output_set_digit_value(i, m_filament_on ? m_digit_state[i] & 0x3fff : 0);
		
		// DP(display point) and AP(apostrophe) segments as lamps
		output_set_lamp_value(i*10 + 0, m_digit_state[i] >> 14 & m_filament_on);
		output_set_lamp_value(i*10 + 1, m_digit_state[i] >> 15 & m_filament_on);
	}
}



/***************************************************************************

  I/O

***************************************************************************/

// common/snspell

READ8_MEMBER(tispeak_state::snspell_read_k)
{
	// the Vss row is always on
	UINT8 k = m_button_matrix[8]->read();

	// read selected button rows
	for (int i = 0; i < 8; i++)
		if (m_r >> i & 1)
			k |= m_button_matrix[i]->read();

	return k;
}

WRITE16_MEMBER(tispeak_state::snspell_write_r)
{
	// R0-R7: input mux and select digit (+R8 if the device has 9 digits)
	// R15: filament on
	// other bits: MCU internal use
	m_r = data;
	display_update();
}

WRITE16_MEMBER(tispeak_state::snspell_write_o)
{
	// reorder opla to led14seg, plus DP as d14 and AP as d15:
	// E,D,C,G,B,A,I,M,L,K,N,J,[AP],H,F,[DP] (sidenote: TI KLMN = MAME MLNK)
	m_o = BITSWAP16(data,12,15,10,7,8,9,11,6,13,3,14,0,1,2,4,5);

	display_update();
}


void tispeak_state::power_off()
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_tms5100->reset();
	m_tms6100->reset();

	m_power_on = 0;
}

WRITE_LINE_MEMBER(tispeak_state::auto_power_off)
{
	// power-off request from the MCU, when [OFF] is pressed, also typically after a couple of minutes of idling
	if (state)
		power_off();
}


// snmath specific

WRITE16_MEMBER(tispeak_state::snmath_write_o)
{
	// reorder opla to led14seg, plus DP as d14 and AP as d15:
	// [DP],D,C,H,F,B,I,M,L,K,N,J,[AP],E,G,A (sidenote: TI KLMN = MAME MLNK)
	m_o = BITSWAP16(data,12,0,10,7,8,9,11,6,3,14,4,13,1,2,5,15);

	display_update();
}



/***************************************************************************

  Inputs

***************************************************************************/

INPUT_CHANGED_MEMBER(tispeak_state::power_button)
{
	int on = (int)(FPTR)param;
	
	if (on)
	{
		m_power_on = 1;
		m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	}
	else if (m_power_on)
		power_off();
}

static INPUT_PORTS_START( snspell )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_NAME("Module")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Erase")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Enter")

	PORT_START("IN.6") // R6
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.7") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Off") // -> auto_power_off
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_NAME("Go")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_NAME("Replay")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_NAME("Repeat")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_NAME("Clue")

	PORT_START("IN.8") // Vss!
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_NAME("Mystery Word")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_NAME("Secret Code")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_NAME("Letter")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_NAME("Say It")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Spell/On") PORT_CHANGED_MEMBER(DEVICE_SELF, tispeak_state, power_button, (void *)1)
INPUT_PORTS_END


static INPUT_PORTS_START( snmath )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Go")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Off") // -> auto_power_off
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Clear")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("<")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_NAME(">")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_NAME("Repeat")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE) // /
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_NAME("Mix It")

	PORT_START("IN.6") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_NAME("Number Stumper")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_NAME("Write It")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_NAME("Greater/Less")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_NAME("Word Problems")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Solve It/On") PORT_CHANGED_MEMBER(DEVICE_SELF, tispeak_state, power_button, (void *)1)

	PORT_START("IN.7")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.8")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void tispeak_state::machine_reset()
{
	m_filament_on = 0;
	m_power_on = 1;
}

void tispeak_state::machine_start()
{
	// zerofill
	memset(m_digit_state, 0, sizeof(m_digit_state));
	m_r = 0;
	m_o = 0;
	m_filament_on = 0;
	m_power_on = 0;

	// register for savestates
	save_item(NAME(m_digit_state));
	save_item(NAME(m_r));
	save_item(NAME(m_o));
	save_item(NAME(m_filament_on));
	save_item(NAME(m_power_on));
}


static MACHINE_CONFIG_START( snspell, tispeak_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0270, XTAL_640kHz/2)
	MCFG_TMS1XXX_READ_K_CB(READ8(tispeak_state, snspell_read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(tispeak_state, snspell_write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(tispeak_state, snspell_write_r))
	MCFG_TMS1XXX_POWER_OFF_CB(WRITELINE(tispeak_state, auto_power_off))

	MCFG_TMS0270_READ_CTL_CB(DEVREAD8("tms5100", tms5100_device, ctl_r))
	MCFG_TMS0270_WRITE_CTL_CB(DEVWRITE8("tms5100", tms5100_device, ctl_w))
	MCFG_TMS0270_WRITE_PDC_CB(DEVWRITELINE("tms5100", tms5100_device, pdc_w))

	MCFG_TIMER_DRIVER_ADD("filoff", tispeak_state, delayed_filament_off)
	MCFG_DEFAULT_LAYOUT(layout_tispeak) // max 9 digits

	/* no video! */

	/* sound hardware */
	MCFG_DEVICE_ADD("tms6100", TMS6100, 0)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("tms5100", TMS5100, XTAL_640kHz)
	MCFG_TMS5110_M0_CB(DEVWRITELINE("tms6100", tms6100_device, tms6100_m0_w))
	MCFG_TMS5110_M1_CB(DEVWRITELINE("tms6100", tms6100_device, tms6100_m1_w))
	MCFG_TMS5110_ADDR_CB(DEVWRITE8("tms6100", tms6100_device, tms6100_addr_w))
	MCFG_TMS5110_DATA_CB(DEVREADLINE("tms6100", tms6100_device, tms6100_data_r))
	MCFG_TMS5110_ROMCLK_CB(DEVWRITELINE("tms6100", tms6100_device, tms6100_romclock_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( snmath, snspell )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(tispeak_state, snmath_write_o))
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( snspell )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "us4189779_tmc0271", 0x0000, 0x1000, BAD_DUMP CRC(d3f5a37d) SHA1(f75ab617a6067d4d3a954a9f86126d2089554df8) ) // typed in from patent 4189779, may have errors

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_default_ipla.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_cd2708_mpla.pla", 0, 2127, BAD_DUMP CRC(504b96bb) SHA1(67b691e7c0b97239410587e50e5182bf46475b43) ) // taken from cd2708, need to verify if it's same as tmc0271
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_tmc0271_opla.pla", 0, 1246, CRC(9ebe12ab) SHA1(acb4e07ba26f2daca5f1c234885ac0371c7ce87f) )

	ROM_REGION( 0x8000, "tms6100", 0 )
	ROM_LOAD( "tmc0351.vsm", 0x0000, 0x4000, CRC(beea3373) SHA1(8b0f7586d2f12c3d4a885fdb528cf23feffa1a3b) )
	ROM_LOAD( "tmc0352.vsm", 0x4000, 0x4000, CRC(d51f0587) SHA1(ddaa484be1bba5fef46b481cafae517e4acaa8ed) )
ROM_END

ROM_START( snmath )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "us4946391_t2074", 0x0000, 0x1000, BAD_DUMP CRC(011f0c2d) SHA1(d2e14d72e03ca864abd51da78ffb71a9da82f624) ) // typed in from patent 4946391, verified with source code (mark BAD_DUMP just to be unsure)

	ROM_REGION( 1246, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0980_default_ipla.pla", 0, 1246, CRC(42db9a38) SHA1(2d127d98028ec8ec6ea10c179c25e447b14ba4d0) )
	ROM_REGION( 2127, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0270_cd2708_mpla.pla", 0, 2127, BAD_DUMP CRC(504b96bb) SHA1(67b691e7c0b97239410587e50e5182bf46475b43) ) // taken from cd2708, need to verify if it's same as cd2704
	ROM_REGION( 1246, "maincpu:opla", 0 )
	ROM_LOAD( "tms0270_cd2708_opla.pla", 0, 1246, BAD_DUMP CRC(1abad753) SHA1(53d20b519ed73ce248368047a056836afbe3cd46) ) // "

	ROM_REGION( 0x8000, "tms6100", 0 )
	ROM_LOAD( "cd2392.vsm", 0x0000, 0x4000, CRC(4ed2e920) SHA1(8896f29e25126c1e4d9a47c9a325b35dddecc61f) )
	ROM_LOAD( "cd2393.vsm", 0x4000, 0x4000, CRC(571d5b5a) SHA1(83284755d9b77267d320b5b87fdc39f352433715) )
ROM_END


COMP( 1978, snspell, 0, 0, snspell, snspell, driver_device, 0, "Texas Instruments", "Speak & Spell (US, prototype)", GAME_NOT_WORKING )
COMP( 1980, snmath,  0, 0, snmath,  snmath,  driver_device, 0, "Texas Instruments", "Speak & Math (US, prototype)", GAME_NOT_WORKING )
