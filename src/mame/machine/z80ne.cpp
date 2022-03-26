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

//#define VERBOSE 1
#include "logmacro.h"





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
	uint8_t cass_ws = 0;
	m_cass_data.input.length++;

	cass_ws = ((cassette_device_image())->input() > +0.02) ? 1 : 0;

	if ((cass_ws ^ m_cass_data.input.level) & cass_ws)
	{
		m_cass_data.input.level = cass_ws;
		m_cass_data.input.bit = ((m_cass_data.input.length < m_cass_data.wave_filter) || (m_cass_data.input.length > 0x20)) ? 1 : 0;
		m_cass_data.input.length = 0;
		m_uart->write_si(m_cass_data.input.bit);
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
			cass_ws = m_uart->so_r();
			m_cass_data.wave_length = cass_ws ? m_cass_data.wave_short : m_cass_data.wave_long;
		}
		cassette_device_image()->output(m_cass_data.output.level ? -1.0 : +1.0);
		m_cass_data.output.length = m_cass_data.wave_length;
	}
}

void z80ne_state::save_state_vars()
{
	save_item(NAME(m_lx383_scan_counter));
	save_item(NAME(m_lx383_key));
	save_item(NAME(m_lx383_downsampler));
	save_item(NAME(m_lx385_ctrl));
}

void z80ne_state::init_z80ne()
{
	save_state_vars();
}

void z80netf_state::driver_init()
{
	save_state_vars();

	/* first two entries point to rom on reset */
	u8 *r = m_ram->pointer();
	m_bank1->configure_entry(0, r); /* RAM   at 0x0000-0x03FF */
	m_bank1->configure_entries(1, 3, m_rom+0x4400, 0x0400); /* ep390, ep1390, ep2390 at 0x0000-0x03FF */
	m_bank1->configure_entry(4, m_rom+0x4000); /* ep382 at 0x0000-0x03FF */
	m_bank1->configure_entry(5, m_rom); /* ep548 at 0x0000-0x03FF */

	m_bank2->configure_entry(0, r+0x0400); /* RAM   at 0x0400 */
	m_bank2->configure_entry(1, m_rom+0x0400); /* ep548 at 0x0400-0x3FFF */

	m_bank3->configure_entry(0, r+0x4000); /* RAM   at 0x8000 */
	m_bank3->configure_entry(1, m_rom+0x4000); /* ep382 at 0x8000 */

	m_bank4->configure_entry(0, r+0x5000); /* RAM   at 0xF000 */
	m_bank4->configure_entries(1, 3, m_rom+0x4400, 0x0400); /* ep390, ep1390, ep2390 at 0xF000 */

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

	uint16_t key_bits;
	uint8_t ctrl; //, rst;
	uint8_t i;

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

void z80ne_state::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
	case 0:
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
		break;
	default:
		printf("Invalid timer %d encountered\n",id);
	}
}

void z80net_state::reset_lx387()
{
	m_lx387_kr2376->set_input_pin( kr2376_device::KR2376_DSII, 0);
	m_lx387_kr2376->set_input_pin( kr2376_device::KR2376_PII, 0);
}

void z80netf_state::reset_lx390_banking()
{
	switch (m_io_config->read() & 0x07)
	{
	case 0x01: /* EP382 Hex Monitor */
		if (VERBOSE)
			logerror("reset_lx390_banking: banking ep382\n");
		m_bank1->set_entry(4);  /* ep382 at 0x0000 for 3 cycles, then RAM */
		m_bank2->set_entry(0);  /* RAM   at 0x0400 */
		m_bank3->set_entry(1);  /* ep382 at 0x8000 */
		m_bank4->set_entry(0);  /* RAM   at 0xF000 */
		break;
	case 0x02: /* EP548  16k BASIC */
		if (VERBOSE)
			logerror("reset_lx390_banking: banking ep548\n");
		m_bank1->set_entry(5);  /* ep548 at 0x0000-0x03FF */
		m_bank2->set_entry(1);  /* ep548 at 0x0400-0x3FFF */
		m_bank3->set_entry(0);  /* RAM   at 0x8000 */
		m_bank4->set_entry(0);  /* RAM   at 0xF000 */
		break;
	case 0x03: /* EP390  Boot Loader for 5.5k floppy BASIC */
		if (VERBOSE)
			logerror("reset_lx390_banking: banking ep390\n");
		m_bank1->set_entry(1);  /* ep390 at 0x0000-0 x03FF for 3 cycles, then RAM */
		m_bank2->set_entry(0);  /* RAM   at 0x0400-0x3FFF */
		m_bank3->set_entry(0);  /* RAM   at 0x8000 */
		m_bank4->set_entry(1);  /* ep390 at 0xF000 */
		break;
	case 0x04: /* EP1390 Boot Loader for NE DOS 1.0/1.5 */
		if (VERBOSE)
			logerror("reset_lx390_banking: banking ep1390\n");
		m_bank1->set_entry(2);  /* ep1390 at 0x0000-0x03FF for 3 cycles, then RAM */
		m_bank2->set_entry(0);  /* RAM   at 0x0400-0x3FFF */
		m_bank3->set_entry(0);  /* RAM   at 0x8000 */
		m_bank4->set_entry(2);  /* ep1390 at 0xF000 */
		break;
	case 0x05: /* EP2390 Boot Loader for NE DOS G.1 */
		if (VERBOSE)
			logerror("reset_lx390_banking: banking ep2390\n");
		m_bank1->set_entry(3);  /* ep2390 at 0x0000-0x03FF for 3 cycles, then RAM */
		m_bank2->set_entry(0);  /* RAM   at 0x0400-0x3FFF */
		m_bank3->set_entry(0);  /* RAM   at 0x8000 */
		m_bank4->set_entry(3);  /* ep2390 at 0xF000 */
		break;
	}

	/* TODO: in real hardware the ENH bus line is pulled down
	 * until a I/O read is performed on a address with A0 address bit low and A1 or A2 address bit high
	 */
}

void z80ne_state::base_reset()
{
	for (int i = 0; i < LX383_KEYS; i++)
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

	m_uart->write_cs(0);
	m_uart->write_nb1(1);
	m_uart->write_nb2(1);
	m_uart->write_tsb(1);
	m_uart->write_eps(1);
	m_uart->write_np(m_io_lx_385->read() & 0x80 ? 1 : 0);
	m_uart->write_cs(1);
	m_uart_clock->set_unscaled_clock(m_cass_data.speed * 16);

	lx385_ctrl_w(0);
}

void z80ne_state::machine_reset()
{
	base_reset();

	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x0000, 0x03ff, m_rom);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0x8000, 0x83ff,
			"rom_shadow_r",
			[this] (offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// reinstall RAM over the ROM shadow
					m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x03ff, m_mram);
				}
			},
			&m_rom_shadow_tap);
}

void z80net_state::machine_reset()
{
	reset_lx387();
	z80ne_state::machine_reset();
}

void z80netb_state::machine_reset()
{
	base_reset();
	reset_lx387();
}

void z80netf_state::machine_reset()
{
	reset_lx390_banking();
	base_reset();
	reset_lx387();

	// basic roms are exempt from memory tap
	if ((m_io_config->read() & 0x07) != 2)
	{
		address_space &program = m_maincpu->space(AS_PROGRAM);
		m_rom_shadow_tap.remove();
		m_rom_shadow_tap = program.install_read_tap(
				0x8000, 0xf3ff,
				"rom_shadow_r",
				[this] (offs_t offset, u8 &data, u8 mem_mask)
				{
					if (!machine().side_effects_disabled())
					{
						// delete this tap
						m_rom_shadow_tap.remove();

						// reinstall RAM over the ROM shadow
						m_bank1->set_entry(0);
					}
				},
				&m_rom_shadow_tap);
	}
}

INPUT_CHANGED_MEMBER(z80ne_state::z80ne_reset)
{
	uint8_t rst = m_io_rst->read();

	if ( ! BIT(rst, 0))
		machine().schedule_soft_reset();
}

INPUT_CHANGED_MEMBER(z80net_state::z80net_nmi)
{
	uint8_t nmi = m_io_lx387_brk->read();

	if ( ! BIT(nmi, 0))
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void z80ne_state::machine_start()
{
	m_timer_nmi = timer_alloc(0);

	m_lx383_digits.resolve();

	m_lx385_ctrl = 0x1f;
	m_cassette_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(z80ne_state::z80ne_cassette_tc), this));
	m_kbd_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(z80ne_state::z80ne_kbd_scan), this));
	m_kbd_timer->adjust(attotime::from_hz(1000), 0, attotime::from_hz(1000));
}

void z80netf_state::machine_start()
{
	z80ne_state::machine_start();
	m_drv_led.resolve();
}

/******************************************************************************
 Drivers
******************************************************************************/

/* LX.383 - LX.384 HEX keyboard and display */
uint8_t z80ne_state::lx383_r()
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

void z80ne_state::lx383_w(offs_t offset, uint8_t data)
{
	/*
	 * First 8 locations (F0-F7) are mapped to a dual-port 8-byte RAM
	 * The 1KHz NE-555 astable oscillator circuit drive
	 * a 4-bit 74LS93 binary counter.
	 * The 3 least significant bits of the counter are connected
	 * both to the read address of the dual-port ram and to
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
	 *   that trigger the NMI line of the CPU after 2 instruction
	 *   fetch cycles for single step execution.
	 */

	if ( offset < 8 )
		m_lx383_digits[offset] = data ^ 0xff;
	else
	{
		// after writing to port 0xF8 and the first ~M1 cycles strike a NMI for single step execution
		m_timer_nmi->adjust(m_maincpu->cycles_to_attotime(1));
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

    The UART is a RCA CDP1854 CMOS device with pin 2 jumpered to GND to select the
    AY-3-1015 compatibility mode. The jumper at P4 can be switched to place 12 V on
    pin 2 for an old PMOS UART.
 *
 */
uint8_t z80ne_state::lx385_ctrl_r()
{
	/* set unused bits high */
	uint8_t data = 0xc0;

	m_uart->write_swe(0);
	data |= (m_uart->or_r(  ) ? 0x01 : 0);
	data |= (m_uart->fe_r(  ) ? 0x02 : 0);
	data |= (m_uart->pe_r(  ) ? 0x04 : 0);
	data |= (m_uart->tbmt_r() ? 0x08 : 0);
	data |= (m_uart->dav_r( ) ? 0x10 : 0);
	data |= (m_uart->eoc_r( ) ? 0x20 : 0);
	m_uart->write_swe(1);

	return data;
}

#define LX385_CASSETTE_MOTOR_MASK ((1<<3)|(1<<4))

void z80ne_state::lx385_ctrl_w(uint8_t data)
{
	/* Translate data to control signals
	 *     0 bit1=0, bit0=0   UART Reset pulse
	 *     1 bit1=0, bit0=1   UART RDAV (Reset Data Available) pulse
	 *     2 UART Tx Clock Enable (active high)
	 *     3 *TAPEA Enable (active low) (at reset: low)
	 *     4 *TAPEB Enable (active low) (at reset: low)
	 */
	uint8_t uart_reset, uart_rdav;
	uint8_t motor_a, motor_b;
	uint8_t changed_bits = (m_lx385_ctrl ^ data) & 0x1C;
	m_lx385_ctrl = data;

	uart_reset = ((data & 0x03) == 0x00);
	uart_rdav  = ((data & 0x03) == 0x01);
	motor_a = ((data & 0x08) == 0x00);
	motor_b = ((data & 0x10) == 0x00);

	/* UART Reset and RDAV */
	if (uart_reset)
	{
		m_uart->write_xr(1);
		m_uart->write_xr(0);
	}

	if (uart_rdav)
	{
		m_uart->write_rdav(1);
		m_uart->write_rdav(0);
	}

	if (!changed_bits) return;

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

WRITE_LINE_MEMBER(z80ne_state::lx385_uart_tx_clock_w)
{
	if (BIT(m_lx385_ctrl, 2))
		m_uart->write_tcp(state);
}

READ_LINE_MEMBER(z80net_state::lx387_shift_r)
{
	return BIT(m_io_modifiers->read(), 0) || BIT(m_io_modifiers->read(), 2);
}

READ_LINE_MEMBER(z80net_state::lx387_control_r)
{
	return BIT(m_io_modifiers->read(), 1);
}

uint8_t z80net_state::lx388_mc6847_videoram_r(offs_t offset)
{
	if (offset == ~0) return 0xff;

	int d6 = BIT(m_videoram[offset], 6);
	int d7 = BIT(m_videoram[offset], 7);

	m_vdg->inv_w(d6 && d7);
	m_vdg->as_w(!d6 && d7);
	m_vdg->intext_w(!d6 && d7);

	return m_videoram[offset];
}

uint8_t z80net_state::lx387_data_r()
{
	uint8_t data = m_lx387_kr2376->data_r() & 0x7f;
	data |= m_lx387_kr2376->get_output_pin(kr2376_device::KR2376_SO) << 7;
	return data;
}

uint8_t z80net_state::lx388_read_field_sync()
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

void z80netf_state::lx390_motor_w(uint8_t data)
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

	for (u8 f = 0; f < 4; f++)
		if (BIT(data, f))
			floppy = m_floppy[f]->get_device();

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
		m_drv_led[0] = 0;
		m_drv_led[1] = 0;
	}
}

uint8_t z80netf_state::lx390_fdc_r(offs_t offset)
{
	uint8_t d;

	switch(offset)
	{
	case 0:
		d = m_wd1771->status_r() ^ 0xff;
		LOG("lx390_fdc_r, WD17xx status: %02x\n", d);
		break;
	case 1:
		d = m_wd1771->track_r() ^ 0xff;
		LOG("lx390_fdc_r, WD17xx track:  %02x\n", d);
		break;
	case 2:
		d = m_wd1771->sector_r() ^ 0xff;
		LOG("lx390_fdc_r, WD17xx sector: %02x\n", d);
		break;
	case 3:
		d = m_wd1771->data_r() ^ 0xff;
		LOG("lx390_fdc_r, WD17xx data3:  %02x\n", d);
		break;
	case 6:
		d = 0xff;
		m_bank1->set_entry(0);
		break;
	case 7:
		d = m_wd1771->data_r() ^ 0xff;
		LOG("lx390_fdc_r, WD17xx data7, force:  %02x\n", d);
		break;
	default:
		d = 0x00;
	}
	return d;
}

void z80netf_state::lx390_fdc_w(offs_t offset, uint8_t data)
{
	uint8_t d = data;
	switch(offset)
	{
	case 0:
		LOG("lx390_fdc_w, WD17xx command: %02x\n", d);
		m_wd1771->cmd_w(d ^ 0xff);
		if (m_wd17xx_state.drive & 1)
			m_drv_led[0] = 2;
		else if (m_wd17xx_state.drive & 2)
			m_drv_led[1] = 2;
		break;
	case 1:
		LOG("lx390_fdc_w, WD17xx track:   %02x\n", d);
		m_wd1771->track_w(d ^ 0xff);
		break;
	case 2:
		LOG("lx390_fdc_w, WD17xx sector:  %02x\n", d);
		m_wd1771->sector_w(d ^ 0xff);
		break;
	case 3:
		m_wd1771->data_w(d ^ 0xff);
		LOG("lx390_fdc_w, WD17xx data3:   %02x\n", d);
		break;
	case 6:
		LOG("lx390_fdc_w, motor_w:   %02x\n", d);
		lx390_motor_w(d);
		break;
	case 7:
		LOG("lx390_fdc_w, WD17xx data7, force:   %02x\n", d);
		m_wd1771->data_w(d ^ 0xff);
		break;
	}
}
