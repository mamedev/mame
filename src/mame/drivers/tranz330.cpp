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

#include "emu.h"
#include "includes/tranz330.h"

#include "speaker.h"

#include "tranz330.lh"


void tranz330_state::tranz330_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xffff).ram();
}

void tranz330_state::tranz330_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_pio,  FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x10, 0x13).rw(m_ctc,  FUNC(z80ctc_device::read),     FUNC(z80ctc_device::write));
	map(0x20, 0x23).rw(m_dart, FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	map(0x30, 0x3f).rw(m_rtc,  FUNC(msm6242_device::read),    FUNC(msm6242_device::write));
}

static void construct_ioport_tranz330(device_t &owner, ioport_list &portlist, std::string &errorbuf)
{
	ioport_configurer configurer(owner, portlist, errorbuf);

	configurer.port_alloc("COL.0");
	configurer.field_alloc(IPT_BUTTON1, IP_ACTIVE_LOW, 0x01).field_add_code(SEQ_TYPE_STANDARD, KEYCODE_1).field_set_name("1 QZ.");
	configurer.field_alloc(IPT_BUTTON2, IP_ACTIVE_LOW, 0x02).field_add_code(SEQ_TYPE_STANDARD, KEYCODE_4).field_set_name("4 GHI");
	configurer.field_alloc(IPT_BUTTON3, IP_ACTIVE_LOW, 0x04).field_add_code(SEQ_TYPE_STANDARD, KEYCODE_7).field_set_name("7 PRS");
	configurer.field_alloc(IPT_BUTTON4, IP_ACTIVE_LOW, 0x08).field_add_code(SEQ_TYPE_STANDARD, KEYCODE_ASTERISK).field_set_name("* ,'\"");

	configurer.port_alloc("COL.1");
	configurer.field_alloc(IPT_BUTTON5, IP_ACTIVE_LOW, 0x01).field_add_code(SEQ_TYPE_STANDARD, KEYCODE_2).field_set_name("2 ABC");
	configurer.field_alloc(IPT_BUTTON6, IP_ACTIVE_LOW, 0x02).field_add_code(SEQ_TYPE_STANDARD, KEYCODE_5).field_set_name("5 JKL");
	configurer.field_alloc(IPT_BUTTON7, IP_ACTIVE_LOW, 0x04).field_add_code(SEQ_TYPE_STANDARD, KEYCODE_8).field_set_name("8 TUV");
	configurer.field_alloc(IPT_BUTTON8, IP_ACTIVE_LOW, 0x08).field_add_code(SEQ_TYPE_STANDARD, KEYCODE_0).field_set_name("0 -SP");

	configurer.port_alloc("COL.2");
	configurer.field_alloc(IPT_BUTTON9,  IP_ACTIVE_LOW, 0x01).field_add_code(SEQ_TYPE_STANDARD, KEYCODE_3).field_set_name("3 DEF");
	configurer.field_alloc(IPT_BUTTON10, IP_ACTIVE_LOW, 0x02).field_add_code(SEQ_TYPE_STANDARD, KEYCODE_6).field_set_name("6 MNO");
	configurer.field_alloc(IPT_BUTTON11, IP_ACTIVE_LOW, 0x04).field_add_code(SEQ_TYPE_STANDARD, KEYCODE_9).field_set_name("9 WXY");
	configurer.field_alloc(IPT_BUTTON12, IP_ACTIVE_LOW, 0x08).field_add_code(SEQ_TYPE_STANDARD, KEYCODE_H).field_set_name("#");  // KEYCODE_H for 'hash mark'

	configurer.port_alloc("COL.3");
	configurer.field_alloc(IPT_BUTTON13, IP_ACTIVE_LOW, 0x01).field_add_code(SEQ_TYPE_STANDARD, KEYCODE_C)        .field_set_name("CLEAR"); // KEYCODE_C so as to not collide with potentially-used UI keys like DEL
	configurer.field_alloc(IPT_BUTTON14, IP_ACTIVE_LOW, 0x02).field_add_code(SEQ_TYPE_STANDARD, KEYCODE_BACKSPACE).field_set_name("BACK SPACE");
	configurer.field_alloc(IPT_BUTTON15, IP_ACTIVE_LOW, 0x04).field_add_code(SEQ_TYPE_STANDARD, KEYCODE_A)        .field_set_name("ALPHA");
	configurer.field_alloc(IPT_START1,   IP_ACTIVE_LOW, 0x08).field_add_code(SEQ_TYPE_STANDARD, KEYCODE_ENTER)    .field_set_name("FUNC | ENTER");  // KEYCODE_H for 'hash mark'
}


void tranz330_state::machine_start()
{
}

void tranz330_state::machine_reset()
{
}

void tranz330_state::syncb_w(int state)
{
}

void tranz330_state::sound_w(int state)
{
	m_speaker->level_w(state);
	m_ctc->trg3(state);
}

void tranz330_state::clock_w(int state)
{
	// Ch 0 and 1 might be DART Ch A & B baud clocks
	//m_ctc->trg0(state);
	//m_ctc->trg1(state);
	// Ch 2 speaker clock
	m_ctc->trg2(state);
}

uint8_t tranz330_state::card_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	// return 0xff for a magstripe 0, return 0x00 for a magstripe 1.
	// an interrupt should be triggered on the Z80 when magstripe reading begins.
	// external contributors are encouraged to hook this up.
	return 0xff;
}

void tranz330_state::pio_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_keypad_col_mask = data & 0xf;
	m_vfd->por ((data >> 4) & 1);
	m_vfd->data((data >> 5) & 1);
	m_vfd->sclk((data >> 6) & 1);
}

uint8_t tranz330_state::pio_b_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	uint8_t input_mask = 0xf;
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
MACHINE_CONFIG_START(tranz330_state::tranz330)
	MCFG_CPU_ADD(CPU_TAG, Z80, XTAL(7'159'090)/2) //*
	MCFG_CPU_PROGRAM_MAP(tranz330_mem)
	MCFG_CPU_IO_MAP(tranz330_io)
	MCFG_Z80_DAISY_CHAIN(tranz330_daisy_chain)

	MCFG_DEVICE_ADD("ctc_clock", CLOCK, XTAL(7'159'090)/4) // ?
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(tranz330_state, clock_w))

	MCFG_DEVICE_ADD(RTC_TAG, MSM6242, XTAL(32'768))

	MCFG_DEVICE_ADD(PIO_TAG, Z80PIO, XTAL(7'159'090)/2) //*
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(CPU_TAG, INPUT_LINE_IRQ0)) //*
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(tranz330_state, pio_a_w))
	MCFG_Z80PIO_IN_PA_CB(READ8(tranz330_state, card_r))
	MCFG_Z80PIO_IN_PB_CB(READ8(tranz330_state, pio_b_r))

	MCFG_DEVICE_ADD(DART_TAG, Z80DART, XTAL(7'159'090)/2) //*
	MCFG_Z80DART_OUT_SYNCB_CB(WRITELINE(tranz330_state, syncb_w))
	MCFG_Z80DART_OUT_TXDB_CB(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd)) //?
	MCFG_Z80DART_OUT_DTRB_CB(DEVWRITELINE(RS232_TAG, rs232_port_device, write_dtr)) //?
	MCFG_Z80DART_OUT_RTSB_CB(DEVWRITELINE(RS232_TAG, rs232_port_device, write_rts)) //?
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(CPU_TAG, INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD(CTC_TAG, Z80CTC, XTAL(7'159'090)/2) //*
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


//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS            INIT   COMPANY      FULLNAME        FLAGS
COMP( 1985, tranz330, 0,      0,      tranz330, tranz330, tranz330_state,  0,     "VeriFone",  "Tranz 330",    MACHINE_CLICKABLE_ARTWORK )
