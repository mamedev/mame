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
#include "tranz330.h"

#include "machine/input_merger.h"
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

uint8_t tranz330_state::card_r()
{
	// return 0xff for a magstripe 0, return 0x00 for a magstripe 1.
	// an interrupt should be triggered on the Z80 when magstripe reading begins.
	// external contributors are encouraged to hook this up.
	return 0xff;
}

void tranz330_state::pio_a_w(uint8_t data)
{
	m_keypad_col_mask = data & 0xf;
	m_vfd->por ((data >> 4) & 1);
	m_vfd->data((data >> 5) & 1);
	m_vfd->sclk((data >> 6) & 1);
}

uint8_t tranz330_state::pio_b_r()
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
void tranz330_state::tranz330(machine_config &config)
{
	Z80(config, m_cpu, XTAL(7'159'090)/2); //*
	m_cpu->set_addrmap(AS_PROGRAM, &tranz330_state::tranz330_mem);
	m_cpu->set_addrmap(AS_IO, &tranz330_state::tranz330_io);
	m_cpu->set_daisy_config(tranz330_daisy_chain);

	CLOCK(config, "ctc_clock", XTAL(7'159'090)/4) // ?
			.signal_handler().set(FUNC(tranz330_state::clock_w));

	MSM6242(config, RTC_TAG, XTAL(32'768));

	INPUT_MERGER_ANY_HIGH(config, "irq")
			.output_handler().set_inputline(m_cpu, INPUT_LINE_IRQ0);

	Z80PIO(config, m_pio, XTAL(7'159'090)/2); //*
	m_pio->out_int_callback().set("irq", FUNC(input_merger_device::in_w<0>)); //*
	m_pio->out_pa_callback().set(FUNC(tranz330_state::pio_a_w));
	m_pio->in_pa_callback().set(FUNC(tranz330_state::card_r));
	m_pio->in_pb_callback().set(FUNC(tranz330_state::pio_b_r));

	Z80DART(config, m_dart, XTAL(7'159'090)/2); //*
	m_dart->out_syncb_callback().set(FUNC(tranz330_state::syncb_w));
	m_dart->out_txdb_callback().set(m_rs232, FUNC(rs232_port_device::write_txd)); //?
	m_dart->out_dtrb_callback().set(m_rs232, FUNC(rs232_port_device::write_dtr)); //?
	m_dart->out_rtsb_callback().set(m_rs232, FUNC(rs232_port_device::write_rts)); //?
	m_dart->out_int_callback().set("irq", FUNC(input_merger_device::in_w<1>));

	Z80CTC(config, m_ctc, XTAL(7'159'090)/2); //*
	m_ctc->zc_callback<2>().set(FUNC(tranz330_state::sound_w));
	m_ctc->intr_callback().set("irq", FUNC(input_merger_device::in_w<2>));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_dart, FUNC(z80dart_device::rxb_w));
	m_rs232->dcd_handler().set(m_dart, FUNC(z80dart_device::dcdb_w));
	m_rs232->cts_handler().set(m_dart, FUNC(z80dart_device::ctsb_w));

	// video
	MIC10937(config, VFD_TAG).set_port_value(0);
	config.set_default_layout(layout_tranz330);

	// sound
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker", 0)
			.add_route(ALL_OUTPUTS, "mono", 0.25);
}


ROM_START( tranz330 )
	ROM_REGION( 0x8000, CPU_TAG, 0 )
	ROM_LOAD( "tranz330-original.bin", 0x0000, 0x8000, CRC(af2bf474) SHA1(7896ed23b22b8e1730b689df83eb7bbf7b8dd130))
ROM_END


//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY      FULLNAME        FLAGS
COMP( 1985, tranz330, 0,      0,      tranz330, tranz330, tranz330_state, empty_init, "VeriFone",  "Tranz 330",    0 )
