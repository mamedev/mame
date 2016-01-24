// license:BSD-3-Clause
// copyright-holders:Kevin Horton,Jonathan Gevaryahu,Sandro Ronco,hap
/******************************************************************************
 
    Fidelity Electronics 6502 based board driver
    See drivers/fidelz80.cpp for hardware description

    TODO:
    - x

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6821pia.h"
#include "sound/speaker.h"

#include "includes/fidelz80.h"

// internal artwork
extern const char layout_fidel_vsc[]; // same layout as fidelz80/vsc


class fidel6502_state : public fidelz80base_state
{
public:
	fidel6502_state(const machine_config &mconfig, device_type type, const char *tag)
		: fidelz80base_state(mconfig, type, tag),
		m_6821pia(*this, "6821pia"),
		m_speaker(*this, "speaker")
	{ }

	// devices/pointers
	optional_device<pia6821_device> m_6821pia;
	optional_device<speaker_sound_device> m_speaker;

	// model CSC
	void csc_prepare_display();
	DECLARE_READ8_MEMBER(csc_speech_r);
	DECLARE_WRITE8_MEMBER(csc_pia0_pa_w);
	DECLARE_WRITE8_MEMBER(csc_pia0_pb_w);
	DECLARE_READ8_MEMBER(csc_pia0_pb_r);
	DECLARE_WRITE_LINE_MEMBER(csc_pia0_ca2_w);
	DECLARE_WRITE8_MEMBER(csc_pia1_pa_w);
	DECLARE_WRITE8_MEMBER(csc_pia1_pb_w);
	DECLARE_READ8_MEMBER(csc_pia1_pa_r);
	DECLARE_WRITE_LINE_MEMBER(csc_pia1_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(csc_pia1_cb2_w);
	DECLARE_READ_LINE_MEMBER(csc_pia1_ca1_r);
	DECLARE_READ_LINE_MEMBER(csc_pia1_cb1_r);

	TIMER_DEVICE_CALLBACK_MEMBER(irq_timer);

protected:
	virtual void machine_start() override;
};



// Devices, I/O

/******************************************************************************
    CSC
******************************************************************************/

// misc handlers

void fidel6502_state::csc_prepare_display()
{
	// 7442 output, also update input mux (9 is unused)
	m_inp_mux = (1 << m_led_select) & 0x1ff;
	
	// 4 7seg leds + H
	for (int i = 0; i < 4; i++)
	{
		m_display_segmask[i] = 0x7f;
		m_display_state[i] = (m_inp_mux >> i & 1) ? m_7seg_data : 0;
	}
	
	// 8*8 chessboard leds
	for (int i = 0; i < 8; i++)
		m_display_state[i+4] = (m_inp_mux >> i & 1) ? m_led_data : 0;

	set_display_size(8, 12);
	display_update();
}

READ8_MEMBER(fidel6502_state::csc_speech_r)
{
	return m_speech_rom[m_speech_bank << 12 | offset];
}


// 6821 PIA 0

WRITE8_MEMBER(fidel6502_state::csc_pia0_pa_w)
{
	// d0-d5: TSI C0-C5
	m_speech->data_w(space, 0, data & 0x3f);

	// d0-d7: data for the 4 7seg leds, bits are ABFGHCDE (H is extra led)
	m_7seg_data = BITSWAP8(data,0,1,5,6,7,2,3,4);
	csc_prepare_display();
}

WRITE8_MEMBER(fidel6502_state::csc_pia0_pb_w)
{
	// d0: speech ROM A12
	m_speech->force_update(); // update stream to now
	m_speech_bank = data & 1;

	// d1: TSI START line
	m_speech->start_w(data >> 1 & 1);

	// d4: tone line
	m_speaker->level_w(data >> 4 & 1);
}

READ8_MEMBER(fidel6502_state::csc_pia0_pb_r)
{
	// d2: printer?
	UINT8 data = 0x04;

	// d3: TSI BUSY line
	if (m_speech->busy_r())
		data |= 0x08;

	// d5: button row 8 (active low)
	if (!(read_inputs(9) & 0x100))
		data |= 0x20;
	
	// d6,d7: language switches
	data|=0xc0;

	return data;
}

WRITE_LINE_MEMBER(fidel6502_state::csc_pia0_ca2_w)
{
	// printer?
}


// 6821 PIA 1

READ8_MEMBER(fidel6502_state::csc_pia1_pa_r)
{
	// d0-d5: button row 0-5 (active low)
	return (read_inputs(9) & 0x3f) ^ 0xff;
}

WRITE8_MEMBER(fidel6502_state::csc_pia1_pa_w)
{
	// d6,d7: 7442 A0,A1
	m_led_select = (m_led_select & ~3) | (data >> 6 & 3);
	csc_prepare_display();
}

WRITE8_MEMBER(fidel6502_state::csc_pia1_pb_w)
{
	// d0-d7: led row data
	m_led_data = data;
	csc_prepare_display();
}

READ_LINE_MEMBER(fidel6502_state::csc_pia1_ca1_r)
{
	// button row 6 (active low)
	return ~read_inputs(9) >> 6 & 1;
}

READ_LINE_MEMBER(fidel6502_state::csc_pia1_cb1_r)
{
	// button row 7 (active low)
	return ~read_inputs(9) >> 7 & 1;
}

WRITE_LINE_MEMBER(fidel6502_state::csc_pia1_cb2_w)
{
	// 7442 A2
	m_led_select = (m_led_select & ~4) | (state ? 4 : 0);
	csc_prepare_display();
}

WRITE_LINE_MEMBER(fidel6502_state::csc_pia1_ca2_w)
{
	// 7442 A3
	m_led_select = (m_led_select & ~8) | (state ? 8 : 0);
	csc_prepare_display();
}




TIMER_DEVICE_CALLBACK_MEMBER(fidel6502_state::irq_timer)
{
	m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
}

/******************************************************************************
    Address Maps
******************************************************************************/

static ADDRESS_MAP_START( csc_map, AS_PROGRAM, 8, fidel6502_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x07ff) AM_RAM AM_MIRROR(0x4000)
	AM_RANGE( 0x0800, 0x0bff) AM_RAM AM_MIRROR(0x4400)
	AM_RANGE( 0x1000, 0x1003) AM_DEVREADWRITE("pia0", pia6821_device, read, write) AM_MIRROR(0x47fc)
	AM_RANGE( 0x1800, 0x1803) AM_DEVREADWRITE("pia1", pia6821_device, read, write) AM_MIRROR(0x47fc)
	AM_RANGE( 0x2000, 0x3fff) AM_ROM AM_MIRROR(0x4000)
	AM_RANGE( 0xa000, 0xffff) AM_ROM
ADDRESS_MAP_END



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( csc )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x100,IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Speak") PORT_CODE(KEYCODE_SPACE)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x100,IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RV") PORT_CODE(KEYCODE_V)

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x100,IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("TM") PORT_CODE(KEYCODE_T)

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x100,IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("LV") PORT_CODE(KEYCODE_L) // level

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x100,IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("DM") PORT_CODE(KEYCODE_M)

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x100,IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("ST") PORT_CODE(KEYCODE_S)

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x100,IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x100,IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Pawn") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Rook") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Knight") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Bishop") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Queen") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("King") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_DEL) // clear
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RE") PORT_CODE(KEYCODE_R) // reset
	PORT_BIT(0x100,IP_ACTIVE_HIGH, IPT_UNUSED) PORT_UNUSED
INPUT_PORTS_END


void fidel6502_state::machine_start()
{
	fidelz80base_state::machine_start();
}


/******************************************************************************
    Machine Drivers
******************************************************************************/

static MACHINE_CONFIG_START( csc, fidel6502_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 3900000/2)
	MCFG_CPU_PROGRAM_MAP(csc_map)


	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_timer", fidel6502_state, irq_timer, attotime::from_hz(38400/64))

	MCFG_DEVICE_ADD("pia0", PIA6821, 0)
	MCFG_PIA_READPB_HANDLER(READ8(fidel6502_state, csc_pia0_pb_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(fidel6502_state, csc_pia0_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(fidel6502_state, csc_pia0_pb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(fidel6502_state, csc_pia0_ca2_w))

	MCFG_DEVICE_ADD("pia1", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(fidel6502_state, csc_pia1_pa_r))
	MCFG_PIA_READCA1_HANDLER(READLINE(fidel6502_state, csc_pia1_ca1_r))
	MCFG_PIA_READCB1_HANDLER(READLINE(fidel6502_state, csc_pia1_cb1_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(fidel6502_state, csc_pia1_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(fidel6502_state, csc_pia1_pb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(fidel6502_state, csc_pia1_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(fidel6502_state, csc_pia1_cb2_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", fidelz80base_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_fidel_vsc)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speech", S14001A, 25000) // R/C circuit, around 25khz
	MCFG_S14001A_EXT_READ_HANDLER(READ8(fidel6502_state, csc_speech_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)

	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( csc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-64109.bin", 0x2000, 0x2000, CRC(08a3577c) SHA1(69fe379d21a9d4b57c84c3832d7b3e7431eec341) )
	ROM_LOAD("1025a03.bin",   0xa000, 0x2000, CRC(63982c07) SHA1(5ed4356323d5c80df216da55994abe94ba4aa94c) )
	ROM_LOAD("1025a02.bin",   0xc000, 0x2000, CRC(9e6e7c69) SHA1(4f1ed9141b6596f4d2b1217d7a4ba48229f3f1b0) )
	ROM_LOAD("1025a01.bin",   0xe000, 0x2000, CRC(57f068c3) SHA1(7d2ac4b9a2fba19556782863bdd89e2d2d94e97b) )
	ROM_LOAD("74s474.bin",    0xfe00, 0x0200, CRC(4511ba31) SHA1(e275b1739f8c3aa445cccb6a2b597475f507e456) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-32107.bin", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d) )
	ROM_RELOAD(               0x1000, 0x1000)
ROM_END

ROM_START( fexcelv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1080a01.ic5", 0x0000, 0x8000, CRC(846f8e40) SHA1(4e1d5b08d5ff3422192b54fa82cb3f505a69a971) )

	ROM_REGION( 0x8000, "speech", 0 )
	ROM_LOAD("101-1081a01.ic2", 0x0000, 0x8000, CRC(c8ae1607) SHA1(6491ce6be60ed77f3dd931c0ca17616f13af943e) )
ROM_END

/******************************************************************************
    Drivers
******************************************************************************/

/*    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT     INIT              COMPANY, FULLNAME, FLAGS */
COMP( 1981, csc,     0,      0,      csc,  csc, driver_device,   0, "Fidelity Electronics", "Champion Sensory Chess Challenger", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

COMP( 1987, fexcelv,     0,      0,      csc,  csc, driver_device,   0, "Fidelity Electronics", "Voice Excellence", MACHINE_NOT_WORKING )
