// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:yoyo_chessboard
/******************************************************************************

    Fidelity Electronics generic MCS-48 based chess computer driver

    NOTE: MAME doesn't include a generalized implementation for boardpieces yet,
    greatly affecting user playability of emulated electronic board games.
    As workaround for the chess games, use an external chess GUI on the side,
    such as Arena(in editmode).

    TODO:
    - nothing

******************************************************************************

Sensory Chess Challenger 6 (model SC6):
- PCB label 510-1045B01
- INS8040N-11 MCU, 11MHz XTAL
- external 4KB ROM 2332 101-1035A01, in module slot
- buzzer, 2 7seg LEDs, 8*8 chessboard buttons

******************************************************************************/

#include "emu.h"
#include "includes/fidelbase.h"

#include "cpu/mcs48/mcs48.h"
#include "sound/volt_reg.h"
#include "speaker.h"

// internal artwork
#include "fidel_sc6.lh" // clickable


class fidelmcs48_state : public fidelbase_state
{
public:
	fidelmcs48_state(const machine_config &mconfig, device_type type, const char *tag)
		: fidelbase_state(mconfig, type, tag)
	{ }

	// SC6
	void sc6_prepare_display();
	DECLARE_WRITE8_MEMBER(sc6_mux_w);
	DECLARE_WRITE8_MEMBER(sc6_select_w);
	DECLARE_READ8_MEMBER(sc6_input_r);
	DECLARE_READ_LINE_MEMBER(sc6_input6_r);
	DECLARE_READ_LINE_MEMBER(sc6_input7_r);
	void sc6_map(address_map &map);
	void sc6(machine_config &config);
};



// Devices, I/O

/******************************************************************************
    SC6
******************************************************************************/

// MCU ports/generic

void fidelmcs48_state::sc6_prepare_display()
{
	// 2 7seg leds
	set_display_segmask(3, 0x7f);
	display_matrix(7, 2, m_7seg_data, m_led_select);
}

WRITE8_MEMBER(fidelmcs48_state::sc6_mux_w)
{
	// P24-P27: 7442 A-D
	u16 sel = 1 << (data >> 4 & 0xf) & 0x3ff;

	// 7442 0-8: input mux, 7seg data
	m_inp_mux = sel & 0x1ff;
	m_7seg_data = sel & 0x7f;
	sc6_prepare_display();

	// 7442 9: speaker out
	m_dac->write(BIT(sel, 9));
}

WRITE8_MEMBER(fidelmcs48_state::sc6_select_w)
{
	// P16,P17: digit select
	m_led_select = ~data >> 6 & 3;
	sc6_prepare_display();
}

READ8_MEMBER(fidelmcs48_state::sc6_input_r)
{
	// P10-P15: multiplexed inputs low
	return (~read_inputs(9) & 0x3f) | 0xc0;
}

READ_LINE_MEMBER(fidelmcs48_state::sc6_input6_r)
{
	// T0: multiplexed inputs bit 6
	return ~read_inputs(9) >> 6 & 1;
}

READ_LINE_MEMBER(fidelmcs48_state::sc6_input7_r)
{
	// T1: multiplexed inputs bit 7
	return ~read_inputs(9) >> 7 & 1;
}



/******************************************************************************
    Address Maps
******************************************************************************/

// SC6

ADDRESS_MAP_START(fidelmcs48_state::sc6_map)
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( sc6 )
	PORT_INCLUDE( fidel_cb_buttons )

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("RV / Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("DM / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("TB / Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("LV / Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("PV / Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("PB / King")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("CL")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("RE")
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

MACHINE_CONFIG_START(fidelmcs48_state::sc6)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8040, 11_MHz_XTAL)
	MCFG_CPU_PROGRAM_MAP(sc6_map)
	MCFG_MCS48_PORT_P2_OUT_CB(WRITE8(fidelmcs48_state, sc6_mux_w))
	MCFG_MCS48_PORT_P1_IN_CB(READ8(fidelmcs48_state, sc6_input_r))
	MCFG_MCS48_PORT_P1_OUT_CB(WRITE8(fidelmcs48_state, sc6_select_w))
	MCFG_MCS48_PORT_T0_IN_CB(READLINE(fidelmcs48_state, sc6_input6_r))
	MCFG_MCS48_PORT_T1_IN_CB(READLINE(fidelmcs48_state, sc6_input7_r))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", fidelbase_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_fidel_sc6)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("speaker")
	MCFG_SOUND_ADD("dac", DAC_1BIT, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.25)
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dac", 1.0, DAC_VREF_POS_INPUT)
MACHINE_CONFIG_END



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( fscc6 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("101-1035a01", 0x0000, 0x1000, CRC(0024971f) SHA1(76b16364913ada2fb94b9e6a8524b924e6832ddf) ) // 2332
ROM_END



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME      PARENT  CMP MACHINE    INPUT   STATE          INIT  COMPANY, FULLNAME, FLAGS
CONS( 1982, fscc6,    0,       0, sc6,       sc6,    fidelmcs48_state, 0, "Fidelity Electronics", "Sensory Chess Challenger 6", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
