// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**************************************************************************

	VeriFone Tranz 330

	All information gleaned from:
	http://www.bigmessowires.com/2011/05/10/mapping-the-tranz-330/

	Currently sits in a loop doing very little, based on the disassembly
	it presumably needs some kind of interrupt in order to kick it into
	running.

    Interrupt Vectors are located at 0200-02FF.
    Display ram at 9000-90FF says GRAMING ERR 0 (part of PROGRAMING ERR message)

    TODO:
    - get working, driver needs a Z80 peripheral expert to look at it.
    - hook up magstripe reader

****************************************************************************/

#include "includes/tranz330.h"
#include "tranz330.lh"

static ADDRESS_MAP_START( tranz330_mem, AS_PROGRAM, 8, tranz330_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( tranz330_io, AS_IO, 8, tranz330_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE(PIO_TAG, z80pio_device, read_alt, write_alt)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE(CTC_TAG, z80ctc_device, read, write)
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE(DART_TAG, z80dart_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x30, 0x3f) AM_DEVREADWRITE(RTC_TAG, msm6242_device, read, write)
ADDRESS_MAP_END


static INPUT_PORTS_START( tranz330 )
	PORT_START("COL.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_1)			PORT_NAME("1 QZ.")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_4)			PORT_NAME("4 GHI")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_7)			PORT_NAME("7 PRS")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_ASTERISK) 	PORT_NAME("* ,'\"")

	PORT_START("COL.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_2)			PORT_NAME("2 ABC")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_5)			PORT_NAME("5 JKL")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_CODE(KEYCODE_8)			PORT_NAME("8 TUV")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_CODE(KEYCODE_0)			PORT_NAME("0 -SP")

	PORT_START("COL.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_CODE(KEYCODE_3)			PORT_NAME("3 DEF")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_CODE(KEYCODE_6)			PORT_NAME("6 MNO")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_CODE(KEYCODE_9)			PORT_NAME("9 WXY")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_CODE(KEYCODE_H)			PORT_NAME("#") // KEYCODE_H for 'hash mark'

	PORT_START("COL.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_CODE(KEYCODE_C)			PORT_NAME("CLEAR") // KEYCODE_C so as to not collide with potentially-used UI keys
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_CODE(KEYCODE_BACKSPACE)	PORT_NAME("BACK SPACE")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON16 ) PORT_CODE(KEYCODE_A)			PORT_NAME("ALPHA")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) PORT_CODE(KEYCODE_ENTER) 		PORT_NAME("FUNC | ENTER")
INPUT_PORTS_END


void tranz330_state::machine_start()
{
}

void tranz330_state::machine_reset()
{
}

WRITE_LINE_MEMBER( tranz330_state::syncb_w )
{
}

WRITE_LINE_MEMBER( tranz330_state::sound_w )
{
	m_speaker->level_w(state);
	m_ctc->trg3(state);
}

WRITE_LINE_MEMBER( tranz330_state::clock_w )
{
	// Ch 0 and 1 might be DART Ch A & B baud clocks
	//m_ctc->trg0(state);
	//m_ctc->trg1(state);
	// Ch 2 speaker clock
	m_ctc->trg2(state);
}

READ8_MEMBER( tranz330_state::card_r )
{
	// return 0xff for a magstripe 0, return 0x00 for a magstripe 1.
	// an interrupt should be triggered on the Z80 when magstripe reading begins.
	// external contributors are encouraged to hook this up.
	return 0xff;
}

WRITE8_MEMBER( tranz330_state::pio_a_w )
{
	m_keypad_col_mask = data & 0xf;
	m_vfd->por ((data >> 4) & 1);
	m_vfd->data((data >> 5) & 1);
	m_vfd->sclk((data >> 6) & 1);
}

READ8_MEMBER( tranz330_state::pio_b_r )
{
	UINT8 input_mask = 0xf;
	for (int i = 0; i < 4; i++)
	{
		if (!BIT(m_keypad_col_mask, i))
		{
			input_mask &= m_keypad[i]->read();
		}
	}
	return input_mask;
}

static const z80_daisy_config tranz330_daisy_chain[] =
{
	{ DART_TAG },
	{ CTC_TAG },
	{ PIO_TAG },
	{ nullptr }
};

// * - check clocks
// ? - check purported RS232 hookup, inconsistent information found at the relevant webpage vs. user-submitted errata
static MACHINE_CONFIG_START( tranz330, tranz330_state )
	MCFG_CPU_ADD(CPU_TAG, Z80, XTAL_7_15909MHz/2) //*
	MCFG_CPU_PROGRAM_MAP(tranz330_mem)
	MCFG_CPU_IO_MAP(tranz330_io)
	MCFG_Z80_DAISY_CHAIN(tranz330_daisy_chain)

	MCFG_DEVICE_ADD("ctc_clock", CLOCK, XTAL_7_15909MHz/4) // ?
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(tranz330_state, clock_w))

	MCFG_DEVICE_ADD(RTC_TAG, MSM6242, XTAL_32_768kHz)

	MCFG_DEVICE_ADD(PIO_TAG, Z80PIO, XTAL_7_15909MHz/2) //*
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(CPU_TAG, INPUT_LINE_IRQ0)) //*
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(tranz330_state, pio_a_w))
	MCFG_Z80PIO_IN_PA_CB(READ8(tranz330_state, card_r))
	MCFG_Z80PIO_IN_PB_CB(READ8(tranz330_state, pio_b_r))

	MCFG_Z80DART_ADD(DART_TAG, XTAL_7_15909MHz/2, 0, 0, 0, 0 ) //*
	MCFG_Z80DART_OUT_SYNCB_CB(WRITELINE(tranz330_state, syncb_w))
	MCFG_Z80DART_OUT_TXDB_CB(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd)) //?
	MCFG_Z80DART_OUT_DTRB_CB(DEVWRITELINE(RS232_TAG, rs232_port_device, write_dtr)) //?
	MCFG_Z80DART_OUT_RTSB_CB(DEVWRITELINE(RS232_TAG, rs232_port_device, write_rts)) //?
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(CPU_TAG, INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD(CTC_TAG, Z80CTC, XTAL_7_15909MHz/2) //*
	MCFG_Z80CTC_ZC2_CB(WRITELINE(tranz330_state, sound_w))
	MCFG_Z80CTC_INTR_CB(INPUTLINE(CPU_TAG, INPUT_LINE_IRQ0))

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(DART_TAG, z80dart_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(DART_TAG, z80dart_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(DART_TAG, z80dart_device, ctsb_w))

	// video
	MCFG_MIC10937_ADD(VFD_TAG, 0)
	MCFG_DEFAULT_LAYOUT( layout_tranz330 )

	// sound
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


ROM_START( tranz330 )
	ROM_REGION( 0x8000, CPU_TAG, 0 )
	ROM_LOAD( "tranz330-original.bin", 0x0000, 0x8000, CRC(af2bf474) SHA1(7896ed23b22b8e1730b689df83eb7bbf7b8dd130))
ROM_END


//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS             INIT    COMPANY     FULLNAME        FLAGS
COMP( 1985, tranz330, 0,      0,      tranz330, tranz330, driver_device,    0,     "VeriFone",  "Tranz 330",    MACHINE_CLICKABLE_ARTWORK )
