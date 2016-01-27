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
#include "cpu/m6502/r65c02.h"
#include "cpu/m6502/m65sc02.h"
#include "machine/6821pia.h"
#include "sound/speaker.h"

#include "includes/fidelz80.h"

// internal artwork
#include "fidel_sc12.lh"
#include "fidel_fev.lh"

extern const char layout_fidel_vsc[]; // same layout as fidelz80/vsc


class fidel6502_state : public fidelz80base_state
{
public:
	fidel6502_state(const machine_config &mconfig, device_type type, const char *tag)
		: fidelz80base_state(mconfig, type, tag),
		m_6821pia(*this, "6821pia"),
		m_speaker(*this, "speaker"),
		m_irq_off(*this, "irq_off")
	{ }

	// devices/pointers
	optional_device<pia6821_device> m_6821pia;
	optional_device<speaker_sound_device> m_speaker;
	optional_device<timer_device> m_irq_off;

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

	// model SC12
	TIMER_DEVICE_CALLBACK_MEMBER(irq_off);
	TIMER_DEVICE_CALLBACK_MEMBER(sc12_irq);
	DECLARE_WRITE8_MEMBER(sc12_control_w);
	DECLARE_READ8_MEMBER(sc12_input_r);
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



/******************************************************************************
    SC12
******************************************************************************/

// interrupt handling

TIMER_DEVICE_CALLBACK_MEMBER(fidel6502_state::irq_off)
{
	m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(fidel6502_state::sc12_irq)
{
	m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
	m_irq_off->adjust(attotime::from_nsec(15250)); // active low for 15.25us
}


// TTL

WRITE8_MEMBER(fidel6502_state::sc12_control_w)
{
	// d0-d3: 7442 a0-a3
	// 7442 0-8: led data, input mux
	UINT16 sel = 1 << (data & 0xf) & 0x3ff;
	m_inp_mux = sel & 0x1ff;

	// 7442 9: speaker out
	m_speaker->level_w(sel >> 9 & 1);

	// d6,d7: led select (active low)
	display_matrix(9, 2, sel & 0x1ff, ~data >> 6 & 3);

	// d4,d5: printer
	//..
}

READ8_MEMBER(fidel6502_state::sc12_input_r)
{
	// a0-a2,d7: multiplexed inputs (active low)
	return (read_inputs(9) << (offset^7) & 0x80) ^ 0xff;
}



/******************************************************************************
    Address Maps
******************************************************************************/

// CSC

static ADDRESS_MAP_START( csc_map, AS_PROGRAM, 8, fidel6502_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x4000) AM_RAM
	AM_RANGE(0x0800, 0x0bff) AM_MIRROR(0x4400) AM_RAM
	AM_RANGE(0x1000, 0x1003) AM_MIRROR(0x47fc) AM_DEVREADWRITE("pia0", pia6821_device, read, write)
	AM_RANGE(0x1800, 0x1803) AM_MIRROR(0x47fc) AM_DEVREADWRITE("pia1", pia6821_device, read, write)
	AM_RANGE(0x2000, 0x3fff) AM_MIRROR(0x4000) AM_ROM
	AM_RANGE(0xa000, 0xffff) AM_ROM
ADDRESS_MAP_END


// SC12

static ADDRESS_MAP_START( sc12_map, AS_PROGRAM, 8, fidel6502_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x1fff) AM_WRITE(sc12_control_w)
	AM_RANGE(0x8000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xa007) AM_MIRROR(0x1ff8) AM_READ(sc12_input_r)
	AM_RANGE(0xc000, 0xcfff) AM_MIRROR(0x1000) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END


// FEV

static ADDRESS_MAP_START( fev_map, AS_PROGRAM, 8, fidel6502_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROM
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

static INPUT_PORTS_START( sc12 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RV / Pawn") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("DM / Knight") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("TB / Bishop") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("LV / Rook") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("PV / Queen") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("PB / King") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_DEL) // clear
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RE") PORT_CODE(KEYCODE_R) // reset
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

static MACHINE_CONFIG_START( csc, fidel6502_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 3900000/2) // from 3.9MHz resonator
	MCFG_CPU_PROGRAM_MAP(csc_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(fidelz80base_state, irq0_line_hold, 600) // 38400kHz/64

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

static MACHINE_CONFIG_START( sc12, fidel6502_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", R65C02, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(sc12_map)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("sc12_irq", fidel6502_state, sc12_irq, attotime::from_hz(780)) // from 556 timer
	MCFG_TIMER_DRIVER_ADD("irq_off", fidel6502_state, irq_off)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", fidelz80base_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_fidel_sc12)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( fev, fidel6502_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M65SC02, XTAL_3MHz) // M65SC102 (CMD)
	MCFG_CPU_PROGRAM_MAP(fev_map)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", fidelz80base_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_fidel_fev)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speech", S14001A, 25000) // R/C circuit, around 25khz
	MCFG_S14001A_EXT_READ_HANDLER(READ8(fidel6502_state, csc_speech_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
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

ROM_START( fscc12 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1068a01",   0x8000, 0x2000, CRC(63c76cdd) SHA1(e0771c98d4483a6b1620791cb99a7e46b0db95c4) ) // SSS SCM23C65E4
	ROM_LOAD("tms2732ajl-45", 0xc000, 0x1000, CRC(45070a71) SHA1(8aeecff828f26fb7081902c757559903be272649) ) // TI TMS2732AJL-45
	ROM_LOAD("tmm2764d-2",    0xe000, 0x2000, CRC(183d3edc) SHA1(3296a4c3bce5209587d4a1694fce153558544e63) ) // Toshiba TMM2764D-2
ROM_END

ROM_START( fexcelv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1080a01.ic5", 0x8000, 0x8000, CRC(846f8e40) SHA1(4e1d5b08d5ff3422192b54fa82cb3f505a69a971) )

	ROM_REGION( 0x8000, "speech", 0 )
	ROM_LOAD("101-1081a01.ic2", 0x0000, 0x8000, CRC(c8ae1607) SHA1(6491ce6be60ed77f3dd931c0ca17616f13af943e) )
ROM_END

/******************************************************************************
    Drivers
******************************************************************************/

/*    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT     INIT              COMPANY, FULLNAME, FLAGS */
COMP( 1981, csc,     0,      0,      csc,  csc, driver_device,   0, "Fidelity Electronics", "Champion Sensory Chess Challenger", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

COMP( 1984, fscc12,     0,      0,      sc12,  sc12, driver_device,   0, "Fidelity Electronics", "Sensory Chess Challenger 12-B", MACHINE_NOT_WORKING )

COMP( 1987, fexcelv,     0,      0,      fev,  csc, driver_device,   0, "Fidelity Electronics", "Voice Excellence", MACHINE_NOT_WORKING )
