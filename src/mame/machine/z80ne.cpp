// license:BSD-3-Clause
// copyright-holders:Roberto Lavarone
/***********************************************************************

    machine/z80ne.c

    Functions to emulate general aspects of the machine (RAM, ROM,
    interrupts, I/O ports)

***********************************************************************/

/* Core includes */
#include "emu.h"
#include "includes/z80ne.h"

/* Components */
#include "machine/ay31015.h"

/* Devices */
#include "imagedev/flopdrv.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)





/* timer to read cassette waveforms */

cassette_image_device* z80ne_state::cassette_device_image()
{
	if (m_lx385_ctrl & 0x08)
		return m_cassette2;
	else
		return m_cassette1;
}

TIMER_CALLBACK_MEMBER(z80ne_state::z80ne_cassette_tc)
{
	UINT8 cass_ws = 0;
	m_cass_data.input.length++;

	cass_ws = ((cassette_device_image())->input() > +0.02) ? 1 : 0;

	if ((cass_ws ^ m_cass_data.input.level) & cass_ws)
	{
		m_cass_data.input.level = cass_ws;
		m_cass_data.input.bit = ((m_cass_data.input.length < m_cass_data.wave_filter) || (m_cass_data.input.length > 0x20)) ? 1 : 0;
		m_cass_data.input.length = 0;
		m_ay31015->set_input_pin(AY31015_SI, m_cass_data.input.bit);
	}
	m_cass_data.input.level = cass_ws;

	/* saving a tape - convert the serial stream from the uart */

	m_cass_data.output.length--;

	if (!(m_cass_data.output.length))
	{
		if (m_cass_data.output.level)
			m_cass_data.output.level = 0;
		else
		{
			m_cass_data.output.level=1;
			cass_ws = m_ay31015->get_output_pin(AY31015_SO);
			m_cass_data.wave_length = cass_ws ? m_cass_data.wave_short : m_cass_data.wave_long;
		}
		cassette_device_image()->output(m_cass_data.output.level ? -1.0 : +1.0);
		m_cass_data.output.length = m_cass_data.wave_length;
	}
}


DRIVER_INIT_MEMBER(z80ne_state,z80ne)
{
	/* first two entries point to rom on reset */
	UINT8 *RAM = m_region_z80ne->base();
	m_bank1->configure_entry(0, &RAM[0x00000]); /* RAM   at 0x0000 */
	m_bank1->configure_entry(1, &RAM[0x14000]); /* ep382 at 0x0000 */
	m_bank2->configure_entry(0, &RAM[0x14000]); /* ep382 at 0x8000 */
}

DRIVER_INIT_MEMBER(z80ne_state,z80net)
{
	DRIVER_INIT_CALL(z80ne);
}

DRIVER_INIT_MEMBER(z80ne_state,z80netb)
{
}

DRIVER_INIT_MEMBER(z80ne_state,z80netf)
{
	/* first two entries point to rom on reset */
	UINT8 *RAM = m_region_z80ne->base();
	m_bank1->configure_entry(0, &RAM[0x00000]); /* RAM   at 0x0000-0x03FF */
	m_bank1->configure_entries(1, 3, &RAM[0x14400], 0x0400); /* ep390, ep1390, ep2390 at 0x0000-0x03FF */
	m_bank1->configure_entry(4, &RAM[0x14000]); /* ep382 at 0x0000-0x03FF */
	m_bank1->configure_entry(5, &RAM[0x10000]); /* ep548 at 0x0000-0x03FF */

	m_bank2->configure_entry(0, &RAM[0x00400]); /* RAM   at 0x0400 */
	m_bank2->configure_entry(1, &RAM[0x10400]); /* ep548 at 0x0400-0x3FFF */

	m_bank3->configure_entry(0, &RAM[0x08000]); /* RAM   at 0x8000 */
	m_bank3->configure_entry(1, &RAM[0x14000]); /* ep382 at 0x8000 */

	m_bank4->configure_entry(0, &RAM[0x0F000]); /* RAM   at 0xF000 */
	m_bank4->configure_entries(1, 3, &RAM[0x14400], 0x0400); /* ep390, ep1390, ep2390 at 0xF000 */

}

TIMER_CALLBACK_MEMBER(z80ne_state::z80ne_kbd_scan)
{
	/*
	 * NE555 is connected to a 74LS93 binary counter
	 * 74LS93 output:
	 *   QA-QC: column index for LEDs and keyboard
	 *   QD:    keyboard row select
	 *
	 * Port F0 input bit assignment:
	 *   0  QA  bits 0..3 of row counter
	 *   1  QB
	 *   2  QC
	 *   3  QD
	 *   4  Control button pressed, active high
	 *   5  Always low
	 *   6  Always low
	 *   7  Selected button pressed, active low
	 *
	 *
	 */

	UINT16 key_bits;
	UINT8 ctrl; //, rst;
	UINT8 i;

	/* 4-bit counter */
	--m_lx383_scan_counter;
	m_lx383_scan_counter &= 0x0f;

	if ( --m_lx383_downsampler == 0 )
	{
		m_lx383_downsampler = LX383_DOWNSAMPLING;
		key_bits = (m_io_row1->read() << 8) | m_io_row0->read();
//      rst = m_io_rst->read();
		ctrl = m_io_ctrl->read();

		for ( i = 0; i<LX383_KEYS; i++)
		{
			m_lx383_key[i] = ( i | (key_bits & 0x01 ? 0x80 : 0x00) | ~ctrl);
			key_bits >>= 1;
		}
	}
}

DIRECT_UPDATE_MEMBER(z80ne_state::z80ne_default)
{
	return address;
}
/*
 * Handle NMI delay for single step instruction
 */
DIRECT_UPDATE_MEMBER(z80ne_state::z80ne_nmi_delay_count)
{
	m_nmi_delay_counter--;

	if (!m_nmi_delay_counter)
	{
		m_maincpu->space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(z80ne_state::z80ne_default), this));
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
	return address;
}

/*
 * Handle delayed ROM/RAM banking at RESET
 * after the first reset_delay_counter bytes have been read from ROM, switch the RAM back in
 */
DIRECT_UPDATE_MEMBER(z80ne_state::z80ne_reset_delay_count)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	/*
	 * TODO: when debugger is active, his memory access causes this callback
	 *
	 */
	if(!space.debugger_access())
		m_reset_delay_counter--;

	if (!m_reset_delay_counter)
	{
		/* remove this callback */
		m_maincpu->space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(z80ne_state::z80ne_default), this));
		/* and switch to RAM bank at address 0x0000 */
		m_bank1->set_entry( 0 ); /* RAM at 0x0000 (bank 1) */
	}
	return address;
}

void z80ne_state::reset_lx388()
{
	m_lx388_kr2376->set_input_pin( KR2376_DSII, 0);
	m_lx388_kr2376->set_input_pin( KR2376_PII, 0);
}

void z80ne_state::reset_lx382_banking()
{
	/* switch to ROM bank at address 0x0000 */
	m_bank1->set_entry(1);
	m_bank2->set_entry(0);  /* ep382 at 0x8000 */

	/* after the first 3 bytes have been read from ROM, switch the RAM back in */
	m_reset_delay_counter = 2;
	m_maincpu->space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(z80ne_state::z80ne_reset_delay_count), this));
}

void z80ne_state::reset_lx390_banking()
{
	m_reset_delay_counter = 0;

	switch (m_io_config->read() & 0x07) {
	case 0x01: /* EP382 Hex Monitor */
		if (VERBOSE)
			logerror("reset_lx390_banking: banking ep382\n");
		m_bank1->set_entry(4);  /* ep382 at 0x0000 for 3 cycles, then RAM */
		m_bank2->set_entry(0);  /* RAM   at 0x0400 */
		m_bank3->set_entry(1);  /* ep382 at 0x8000 */
		m_bank4->set_entry(0);  /* RAM   at 0xF000 */
		/* after the first 3 bytes have been read from ROM, switch the RAM back in */
		m_reset_delay_counter = 2;
		m_maincpu->space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(z80ne_state::z80ne_reset_delay_count), this));
		break;
	case 0x02: /* EP548  16k BASIC */
		if (VERBOSE)
			logerror("reset_lx390_banking: banking ep548\n");
		m_bank1->set_entry(5);  /* ep548 at 0x0000-0x03FF */
		m_bank2->set_entry(1);  /* ep548 at 0x0400-0x3FFF */
		m_bank3->set_entry(0);  /* RAM   at 0x8000 */
		m_bank4->set_entry(0);  /* RAM   at 0xF000 */
		m_maincpu->space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(z80ne_state::z80ne_default), this));
		break;
	case 0x03: /* EP390  Boot Loader for 5.5k floppy BASIC */
		if (VERBOSE)
			logerror("reset_lx390_banking: banking ep390\n");
		m_bank1->set_entry(1);  /* ep390 at 0x0000-0 x03FF for 3 cycles, then RAM */
		m_bank2->set_entry(0);  /* RAM   at 0x0400-0x3FFF */
		m_bank3->set_entry(0);  /* RAM   at 0x8000 */
		m_bank4->set_entry(1);  /* ep390 at 0xF000 */
		m_maincpu->space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(z80ne_state::z80ne_default), this));
		break;
	case 0x04: /* EP1390 Boot Loader for NE DOS 1.0/1.5 */
		if (VERBOSE)
			logerror("reset_lx390_banking: banking ep1390\n");
		m_bank1->set_entry(2);  /* ep1390 at 0x0000-0x03FF for 3 cycles, then RAM */
		m_bank2->set_entry(0);  /* RAM   at 0x0400-0x3FFF */
		m_bank3->set_entry(0);  /* RAM   at 0x8000 */
		m_bank4->set_entry(2);  /* ep1390 at 0xF000 */
		m_maincpu->space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(z80ne_state::z80ne_default), this));
		break;
	case 0x05: /* EP2390 Boot Loader for NE DOS G.1 */
		if (VERBOSE)
			logerror("reset_lx390_banking: banking ep2390\n");
		m_bank1->set_entry(3);  /* ep2390 at 0x0000-0x03FF for 3 cycles, then RAM */
		m_bank2->set_entry(0);  /* RAM   at 0x0400-0x3FFF */
		m_bank3->set_entry(0);  /* RAM   at 0x8000 */
		m_bank4->set_entry(3);  /* ep2390 at 0xF000 */
		m_maincpu->space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(z80ne_state::z80ne_default), this));
		break;
	}

	/* TODO: in real hardware the ENH bus line is pulled down
	 * until a I/O read is performed on a address with A0 address bit low and A1 or A2 address bit high
	 */
}

MACHINE_RESET_MEMBER(z80ne_state,z80ne_base)
{
	int i;

	LOG(("In machine_reset z80ne_base\n"));

	for ( i=0; i<LX383_KEYS; i++)
		m_lx383_key[i] = 0xf0 | i;
	m_lx383_scan_counter = 0x0f;
	m_lx383_downsampler = LX383_DOWNSAMPLING;

	/* Initialize cassette interface */
	switch(m_io_lx_385->read() & 0x07)
	{
	case 0x01:
		m_cass_data.speed = TAPE_300BPS;
		m_cass_data.wave_filter = LX385_TAPE_SAMPLE_FREQ / 1600;
		m_cass_data.wave_short = LX385_TAPE_SAMPLE_FREQ / (2400 * 2);
		m_cass_data.wave_long = LX385_TAPE_SAMPLE_FREQ / (1200 * 2);
		break;
	case 0x02:
		m_cass_data.speed = TAPE_600BPS;
		m_cass_data.wave_filter = LX385_TAPE_SAMPLE_FREQ / 3200;
		m_cass_data.wave_short = LX385_TAPE_SAMPLE_FREQ / (4800 * 2);
		m_cass_data.wave_long = LX385_TAPE_SAMPLE_FREQ / (2400 * 2);
		break;
	case 0x04:
		m_cass_data.speed = TAPE_1200BPS;
		m_cass_data.wave_filter = LX385_TAPE_SAMPLE_FREQ / 6400;
		m_cass_data.wave_short = LX385_TAPE_SAMPLE_FREQ / (9600 * 2);
		m_cass_data.wave_long = LX385_TAPE_SAMPLE_FREQ / (4800 * 2);
	}
	m_cass_data.wave_length = m_cass_data.wave_short;
	m_cass_data.output.length = m_cass_data.wave_length;
	m_cass_data.output.level = 1;
	m_cass_data.input.length = 0;
	m_cass_data.input.bit = 1;

	m_ay31015->set_input_pin(AY31015_CS, 0);
	m_ay31015->set_input_pin(AY31015_NB1, 1);
	m_ay31015->set_input_pin(AY31015_NB2, 1);
	m_ay31015->set_input_pin(AY31015_TSB, 1);
	m_ay31015->set_input_pin(AY31015_EPS, 1);
	m_ay31015->set_input_pin(AY31015_NP, m_io_lx_385->read() & 0x80 ? 1 : 0);
	m_ay31015->set_input_pin(AY31015_CS, 1);
	m_ay31015->set_receiver_clock(m_cass_data.speed * 16.0);
	m_ay31015->set_transmitter_clock(m_cass_data.speed * 16.0);

	m_nmi_delay_counter = 0;
	lx385_ctrl_w(m_maincpu->space(AS_PROGRAM), 0, 0);

}

MACHINE_RESET_MEMBER(z80ne_state,z80ne)
{
	LOG(("In machine_reset z80ne\n"));
	reset_lx382_banking();
	MACHINE_RESET_CALL_MEMBER( z80ne_base );
}

MACHINE_RESET_MEMBER(z80ne_state,z80net)
{
	LOG(("In machine_reset z80net\n"));
	MACHINE_RESET_CALL_MEMBER( z80ne );
	reset_lx388();
}

MACHINE_RESET_MEMBER(z80ne_state,z80netb)
{
	LOG(("In machine_reset z80netb\n"));
	MACHINE_RESET_CALL_MEMBER( z80ne_base );
	reset_lx388();
}

MACHINE_RESET_MEMBER(z80ne_state,z80netf)
{
	LOG(("In machine_reset z80netf\n"));
	reset_lx390_banking();
	MACHINE_RESET_CALL_MEMBER( z80ne_base );
	reset_lx388();
}

INPUT_CHANGED_MEMBER(z80ne_state::z80ne_reset)
{
	UINT8 rst;
	rst = m_io_rst->read();

	if ( ! BIT(rst, 0))
	{
		machine().schedule_soft_reset();
	}
}

INPUT_CHANGED_MEMBER(z80ne_state::z80ne_nmi)
{
	UINT8 nmi;
	nmi = m_io_lx388_brk->read();

	if ( ! BIT(nmi, 0))
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
}

MACHINE_START_MEMBER(z80ne_state,z80ne)
{
	LOG(("In MACHINE_START z80ne\n"));
	m_lx385_ctrl = 0x1f;
	save_item(NAME(m_lx383_scan_counter));
	save_item(NAME(m_lx383_downsampler));
	save_item(NAME(m_lx383_key));
	save_item(NAME(m_nmi_delay_counter));
	m_cassette_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(z80ne_state::z80ne_cassette_tc),this));
	machine().scheduler().timer_pulse( attotime::from_hz(1000), timer_expired_delegate(FUNC(z80ne_state::z80ne_kbd_scan),this));
}

MACHINE_START_MEMBER(z80ne_state,z80net)
{
	MACHINE_START_CALL_MEMBER( z80ne );
	LOG(("In MACHINE_START z80net\n"));
}

MACHINE_START_MEMBER(z80ne_state,z80netb)
{
	MACHINE_START_CALL_MEMBER( z80net );
	LOG(("In MACHINE_START z80netb\n"));
}

MACHINE_START_MEMBER(z80ne_state,z80netf)
{
	MACHINE_START_CALL_MEMBER( z80net );
	LOG(("In MACHINE_START z80netf\n"));
}

/******************************************************************************
 Drivers
******************************************************************************/

/* LX.383 - LX.384 HEX keyboard and display */
READ8_MEMBER(z80ne_state::lx383_r)
{
	/*
	 * Keyboard scanning
	 *
	 * IC14 NE555 astable oscillator
	 * IC13 74LS93 binary counter
	 * IC5  74LS240 tri-state buffer
	 *
	 * l'oscillatore NE555 alimenta il clock del contatore 74LS93
	 *      D0 - Q(A) --\
	 *      D1 - Q(B)    |-- column
	 *      D2 - Q(C) --/
	 *      D3 - Q(D)        row
	 *      D4 - CTRL
	 *      D5 - 0
	 *      D6 - 0
	 *      D7 - ~KEY Pressed
	 */
	return m_lx383_key[m_lx383_scan_counter];
}

WRITE8_MEMBER(z80ne_state::lx383_w)
{
	/*
	 * First 8 locations (F0-F7) are mapped to a dual-port 8-byte RAM
	 * The 1KHz NE-555 astable oscillator circuit drive
	 * a 4-bit 74LS93 binary counter.
	 * The 3 least sigificant bits of the counter are connected
	 * both to the read addres of the dual-port ram and to
	 * a 74LS156 3 to 8 binary decoder driving the cathode
	 * of 8 7-segments LEDS.
	 * The data output of the dual-port ram drive the anodes
	 * of the LEDS through 74LS07 buffers.
	 * LED segments - dual-port RAM bit:
	 *   A   0x01
	 *   B   0x02
	 *   C   0x04
	 *   D   0x08
	 *   E   0x10
	 *   F   0x20
	 *   G   0x40
	 *   P   0x80 (represented by DP in original schematics)
	 *
	 *   A write in the range F0-FF starts a 74LS90 counter
	 *   that trigger the NMI line of the CPU afther 2 instruction
	 *   fetch cycles for single step execution.
	 */

	if ( offset < 8 )
		output().set_digit_value( offset, data ^ 0xff );
	else
	{
		/* after writing to port 0xF8 and the first ~M1 cycles strike a NMI for single step execution */
		m_nmi_delay_counter = 1;
		m_maincpu->space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(z80ne_state::z80ne_nmi_delay_count), this));
	}
}


/* LX.385 Cassette tape interface */
/*
 * NE555 is connected to a 74LS93 binary counter
 * 74LS93 output:
 *   QA-QC: column index for LEDs and keyboard
 *   QD:    keyboard row select
 *
 * Port EE: UART Data Read/Write
 * Port EF: Status/Control
 *     read, UART status bits read
 *         0 OR   Overrun
 *         1 FE   Framing Error
 *         2 PE   Parity Error
 *         3 TBMT Transmitter Buffer Empty
 *         4 DAV  Data Available
 *         5 EOC  End Of Character
 *         6 1
 *         7 1
 *     write, UART control bits / Tape Unit select / Modulation control
 *         0 bit1=0, bit0=0   UART Reset pulse
 *         1 bit1=0, bit0=1   UART RDAV (Reset Data Available) pulse
 *         2 Tape modulation enable
 *         3 *TAPEA Enable (active low) (at reset: low)
 *         4 *TAPEB Enable (active low) (at reset: low)
 *  Cassette is connected to the uart data input and output via the cassette
 *  interface hardware.
 *
 *  The cassette interface hardware converts square-wave pulses into bits which the uart receives.
 *
 *  1. the cassette format: "frequency shift" is converted
    into the uart data format "non-return to zero"

    2. on cassette a 1 data bit is stored as 8 2400 Hz pulses
    and a 0 data bit as 4 1200 Hz pulses
    - At 1200 baud, a logic 1 is 1 cycle of 1200 Hz and a logic 0 is 1/2 cycle of 600 Hz.
    - At 300 baud, a logic 1 is 8 cycles of 2400 Hz and a logic 0 is 4 cycles of 1200 Hz.

    Attenuation is applied to the signal and the square wave edges are rounded.

    A manchester encoder is used. A flip-flop synchronises input
    data on the positive-edge of the clock pulse.
 *
 */
READ8_MEMBER(z80ne_state::lx385_data_r)
{
	return m_ay31015->get_received_data();
}

READ8_MEMBER(z80ne_state::lx385_ctrl_r)
{
	/* set unused bits high */
	UINT8 data = 0xc0;

	m_ay31015->set_input_pin(AY31015_SWE, 0);
	data |= (m_ay31015->get_output_pin(AY31015_OR  ) ? 0x01 : 0);
	data |= (m_ay31015->get_output_pin(AY31015_FE  ) ? 0x02 : 0);
	data |= (m_ay31015->get_output_pin(AY31015_PE  ) ? 0x04 : 0);
	data |= (m_ay31015->get_output_pin(AY31015_TBMT) ? 0x08 : 0);
	data |= (m_ay31015->get_output_pin(AY31015_DAV ) ? 0x10 : 0);
	data |= (m_ay31015->get_output_pin(AY31015_EOC ) ? 0x20 : 0);
	m_ay31015->set_input_pin(AY31015_SWE, 1);

	return data;
}

WRITE8_MEMBER(z80ne_state::lx385_data_w)
{
	m_ay31015->set_transmit_data(data);
}

#define LX385_CASSETTE_MOTOR_MASK ((1<<3)|(1<<4))

WRITE8_MEMBER(z80ne_state::lx385_ctrl_w)
{
	/* Translate data to control signals
	 *     0 bit1=0, bit0=0   UART Reset pulse
	 *     1 bit1=0, bit0=1   UART RDAV (Reset Data Available) pulse
	 *     2 UART Tx Clock Enable (active high)
	 *     3 *TAPEA Enable (active low) (at reset: low)
	 *     4 *TAPEB Enable (active low) (at reset: low)
	 */
	UINT8 uart_reset, uart_rdav, uart_tx_clock;
	UINT8 motor_a, motor_b;
	UINT8 changed_bits = (m_lx385_ctrl ^ data) & 0x1C;
	m_lx385_ctrl = data;

	uart_reset = ((data & 0x03) == 0x00);
	uart_rdav  = ((data & 0x03) == 0x01);
	uart_tx_clock = ((data & 0x04) == 0x04);
	motor_a = ((data & 0x08) == 0x00);
	motor_b = ((data & 0x10) == 0x00);

	/* UART Reset and RDAV */
	if (uart_reset)
	{
		m_ay31015->set_input_pin(AY31015_XR, 1);
		m_ay31015->set_input_pin(AY31015_XR, 0);
	}

	if (uart_rdav)
	{
		m_ay31015->set_input_pin(AY31015_RDAV, 1);
		m_ay31015->set_input_pin(AY31015_RDAV, 0);
	}

	if (!changed_bits) return;

	/* UART Tx Clock enable/disable */
	if (changed_bits & 0x04)
		m_ay31015->set_transmitter_clock(uart_tx_clock ? m_cass_data.speed * 16.0 : 0.0);

	/* motors */
	if(changed_bits & 0x18)
	{
		m_cassette1->change_state(
			(motor_a) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);

		m_cassette2->change_state(
			(motor_b) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);

		if (motor_a || motor_b)
			m_cassette_timer->adjust(attotime::zero, 0, attotime::from_hz(LX385_TAPE_SAMPLE_FREQ));
		else
			m_cassette_timer->adjust(attotime::zero);
	}
}

READ8_MEMBER(z80ne_state::lx388_mc6847_videoram_r)
{
	if (offset == ~0) return 0xff;

	UINT8 *videoram = m_videoram;
	int d6 = BIT(videoram[offset], 6);
	int d7 = BIT(videoram[offset], 7);

	m_vdg->inv_w(d6 && d7);
	m_vdg->as_w(!d6 && d7);
	m_vdg->intext_w(!d6 && d7);

	return videoram[offset];
}

READ8_MEMBER(z80ne_state::lx388_data_r)
{
	UINT8 data;

	data = m_lx388_kr2376->data_r(space, 0) & 0x7f;
	data |= m_lx388_kr2376->get_output_pin(KR2376_SO) << 7;
	return data;
}

READ8_MEMBER(z80ne_state::lx388_read_field_sync)
{
	return m_vdg->fs_r() << 7;
}

/*
 * DRQ INTRQ IC9B.10 IC8B.*Q
 *  0    0     1       0
 *  0    1     0       x
 *  1    0     0       x
 *  1    1     0       x
 *
 */

WRITE8_MEMBER(z80ne_state::lx390_motor_w)
{
	/* Selection of drive and parameters
	 A write also causes the selected drive motor to turn on for about 3 seconds.
	 When the motor turns off, the drive is deselected.
	    d7 Unused             (trs80: 1=MFM, 0=FM)
	    d6 (trs80: 1=Wait)
	    d5 0=Side 0, 1=Side 1 (trs80: 1=Write Precompensation enabled)
	    d4 Unused             (trs80: 0=Side 0, 1=Side 1)
	    d3 1=select drive 3
	    d2 1=select drive 2
	    d1 1=select drive 1
	    d0 1=select drive 0 */

	floppy_image_device *floppy = nullptr;

	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	if (BIT(data, 1)) floppy = m_floppy1->get_device();
	if (BIT(data, 2)) floppy = m_floppy2->get_device();
	if (BIT(data, 3)) floppy = m_floppy3->get_device();

	m_wd1771->set_floppy(floppy);

	if (floppy)
	{
		floppy->ss_w(BIT(data, 5));
		floppy->mon_w(0);
	}

	m_wd17xx_state.head = (data & 32) ? 1 : 0;
	m_wd17xx_state.drive = data & 0x0F;

	/* no drive selected, turn off all leds */
	if (!m_wd17xx_state.drive)
	{
		output().set_value("drv0", 0);
		output().set_value("drv1", 0);
	}
}

READ8_MEMBER(z80ne_state::lx390_reset_bank)
{
	offs_t pc;

	/* if PC is not in range, we are under integrated debugger control, DON'T SWAP */
	pc = m_maincpu->pc();
	if((pc >= 0xf000) && (pc <=0xffff))
	{
		LOG(("lx390_reset_bank, reset memory bank 1\n"));
		m_bank1->set_entry(0); /* RAM at 0x0000 (bank 1) */
	}
	else
	{
		LOG(("lx390_reset_bank, bypass because in debugger\n"));
	}
	return 0xff;
}

READ8_MEMBER(z80ne_state::lx390_fdc_r)
{
	UINT8 d;

	switch(offset)
	{
	case 0:
		d = m_wd1771->status_r(space, 0) ^ 0xff;
		LOG(("lx390_fdc_r, WD17xx status: %02x\n", d));
		break;
	case 1:
		d = m_wd1771->track_r(space, 0) ^ 0xff;
		LOG(("lx390_fdc_r, WD17xx track:  %02x\n", d));
		break;
	case 2:
		d = m_wd1771->sector_r(space, 0) ^ 0xff;
		LOG(("lx390_fdc_r, WD17xx sector: %02x\n", d));
		break;
	case 3:
		d = m_wd1771->data_r(space, 0) ^ 0xff;
		LOG(("lx390_fdc_r, WD17xx data3:  %02x\n", d));
		break;
	case 6:
		d = 0xff;
		lx390_reset_bank(space, 0);
		break;
	case 7:
		d = m_wd1771->data_r(space, 3) ^ 0xff;
		LOG(("lx390_fdc_r, WD17xx data7, force:  %02x\n", d));
		break;
	default:
		d = 0x00;
	}
	return d;
}

WRITE8_MEMBER(z80ne_state::lx390_fdc_w)
{
	UINT8 d;

	d = data;
	switch(offset)
	{
	case 0:
		LOG(("lx390_fdc_w, WD17xx command: %02x\n", d));
		m_wd1771->cmd_w(space, offset, d ^ 0xff);
		if (m_wd17xx_state.drive & 1)
			output().set_value("drv0", 2);
		else if (m_wd17xx_state.drive & 2)
			output().set_value("drv1", 2);
		break;
	case 1:
		LOG(("lx390_fdc_w, WD17xx track:   %02x\n", d));
		m_wd1771->track_w(space, offset, d ^ 0xff);
		break;
	case 2:
		LOG(("lx390_fdc_w, WD17xx sector:  %02x\n", d));
		m_wd1771->sector_w(space, offset, d ^ 0xff);
		break;
	case 3:
		m_wd1771->data_w(space, 0, d ^ 0xff);
		LOG(("lx390_fdc_w, WD17xx data3:   %02x\n", d));
		break;
	case 6:
		LOG(("lx390_fdc_w, motor_w:   %02x\n", d));
		lx390_motor_w(space, 0, d);
		break;
	case 7:
		LOG(("lx390_fdc_w, WD17xx data7, force:   %02x\n", d));
		m_wd1771->data_w(space, 3, d ^ 0xff);
		break;
	}
}
