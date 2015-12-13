// license:BSD-3-Clause
// copyright-holders:Robbbert
/* Super80.c written by Robbbert, 2005-2009. See driver source for documentation. */

#include "includes/super80.h"
#include "machine/z80bin.h"

/**************************** PIO ******************************************************************************/


WRITE8_MEMBER( super80_state::pio_port_a_w )
{
	m_keylatch = data;
	m_pio->port_b_write(pio_port_b_r(generic_space(),0,0xff)); // refresh kbd int
}

READ8_MEMBER( super80_state::pio_port_b_r )
{
	UINT8 data = 0xff;

	for (int i = 0; i < 8; i++)
	{
		if (!BIT(m_keylatch, i))
			data &= m_io_keyboard[i]->read();
	}

	m_key_pressed = 3;

	return data;
}

/**************************** CASSETTE ROUTINES *****************************************************************/

void super80_state::super80_cassette_motor( UINT8 data )
{
	if (data)
		m_cassette->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
	else
		m_cassette->change_state(CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);

	/* does user want to hear the sound? */
	if BIT(m_io_config->read(), 3)
		m_cassette->change_state(CASSETTE_SPEAKER_ENABLED,CASSETTE_MASK_SPEAKER);
	else
		m_cassette->change_state(CASSETTE_SPEAKER_MUTED,CASSETTE_MASK_SPEAKER);
}

/********************************************* TIMER ************************************************/


// If normal keyboard scan has stopped, then do a scan to allow the interrupt key sequence
TIMER_DEVICE_CALLBACK_MEMBER( super80_state::timer_k )
{
	if (m_key_pressed)
		m_key_pressed--;
	else
	if (!m_key_pressed)
		m_pio->port_b_write(pio_port_b_r(generic_space(),0,0xff));
}

/* cassette load circuit
    This "emulates" U79 CD4046BCN PLL chip and U1 LM311P op-amp. U79 converts a frequency to a voltage,
    and U1 amplifies that voltage to digital levels. U1 has a trimpot connected, to set the midpoint.

    The MDS homebrew input circuit consists of 2 op-amps followed by a D-flipflop.
    The "read-any-system" cassette circuit is a CA3140 op-amp, the smarts being done in software.

    bit 0 = original system (U79 and U1)
    bit 1 = MDS fast system
    bit 2 = CA3140 */
TIMER_DEVICE_CALLBACK_MEMBER( super80_state::timer_p )
{
	UINT8 cass_ws=0;

	m_cass_data[1]++;
	cass_ws = ((m_cassette)->input() > +0.03) ? 4 : 0;

	if (cass_ws != m_cass_data[0])
	{
		if (cass_ws) m_cass_data[3] ^= 2;       // the MDS flipflop
		m_cass_data[0] = cass_ws;
		m_cass_data[2] = ((m_cass_data[1] < 0x0d) ? 1 : 0) | cass_ws | m_cass_data[3];
		m_cass_data[1] = 0;
	}
}

/* after the first 4 bytes have been read from ROM, switch the ram back in */
TIMER_CALLBACK_MEMBER(super80_state::super80_reset)
{
	membank("boot")->set_entry(0);
}

TIMER_DEVICE_CALLBACK_MEMBER( super80_state::timer_h )
{
	UINT8 go_fast = 0;
	if ( (!BIT(m_portf0, 2)) | (!BIT(m_io_config->read(), 1)) )    /* bit 2 of port F0 is low, OR user turned on config switch */
		go_fast++;

	/* code to slow down computer to 1 MHz by halting cpu on every second frame */
	if (!go_fast)
	{
		if (!m_int_sw)
			m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);    // if going, stop it

		m_int_sw++;
		if (m_int_sw > 1)
		{
			m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);     // if stopped, start it
			m_int_sw = 0;
		}
	}
	else
	{
		if (m_int_sw < 8)                               // @2MHz, reset just once
		{
			m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			m_int_sw = 8;                           // ...not every time
		}
	}
}

/*************************************** PRINTER ********************************************************/

/* The Super80 had an optional I/O card that plugged into the S-100 slot. The card had facility for running
    an 8-bit Centronics printer, and a serial device at 300, 600, or 1200 baud. The I/O address range
    was selectable via 4 dipswitches. The serial parameters (baud rate, parity, stop bits, etc) was
    chosen with more dipswitches. Regretably, no parameters could be set by software. Currently, the
    Centronics printer is emulated; the serial side of things may be done later, as will the dipswitches.

    The most commonly used I/O range is DC-DE (DC = centronics, DD = serial data, DE = serial control
    All the home-brew roms use this, except for super80e which uses BC-BE. */

/**************************** I/O PORTS *****************************************************************/

READ8_MEMBER( super80_state::port3e_r )
{
	return 0xF8 | (m_fdc->intrq_r() << 0) | (m_fdc->drq_r() << 1) | 4;
}

// UFDC board can support 4 drives; we support 2
WRITE8_MEMBER( super80_state::port3f_w )
{
	// m_fdc->58(BIT(data, 0));   5/8 pin not emulated in wd_fdc
	m_fdc->enmf_w(BIT(data,1));

	floppy_image_device *floppy = nullptr;
	if (BIT(data, 2)) floppy = m_floppy0->get_device();
	if (BIT(data, 3)) floppy = m_floppy1->get_device();
	//if (BIT(data, 4)) floppy = m_floppy2->get_device();
	//if (BIT(data, 5)) floppy = m_floppy3->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		floppy->mon_w(0);
		floppy->ss_w(!BIT(data, 6));
	}

	m_fdc->dden_w(BIT(data, 7));
}

READ8_MEMBER( super80_state::super80_f2_r )
{
	UINT8 data = m_io_dsw->read() & 0xf0;  // dip switches on pcb
	data |= m_cass_data[2];         // bit 0 = output of U1, bit 1 = MDS cass state, bit 2 = current wave_state
	data |= 0x08;               // bit 3 - not used
	return data;
}

WRITE8_MEMBER( super80_state::super80_dc_w )
{
	/* hardware strobe driven from port select, bit 7..0 = data */
	m_cent_data_out->write(space, 0, data);
	m_centronics->write_strobe(0);
	m_centronics->write_strobe(1);
}


WRITE8_MEMBER( super80_state::super80_f0_w )
{
	UINT8 bits = data ^ m_last_data;
	m_portf0 = data;
	m_speaker->level_w(BIT(data, 3));               /* bit 3 - speaker */
	if (BIT(bits, 1)) super80_cassette_motor(BIT(data, 1));  /* bit 1 - cassette motor */
	m_cassette->output( BIT(data, 0) ? -1.0 : +1.0);    /* bit 0 - cass out */

	m_last_data = data;
}

WRITE8_MEMBER( super80_state::super80r_f0_w )
{
	UINT8 bits = data ^ m_last_data;
	m_portf0 = data | 0x14;
	m_speaker->level_w(BIT(data, 3));               /* bit 3 - speaker */
	if (BIT(bits, 1)) super80_cassette_motor(BIT(data, 1));  /* bit 1 - cassette motor */
	m_cassette->output( BIT(data, 0) ? -1.0 : +1.0);    /* bit 0 - cass out */

	m_last_data = data;
}

/**************************** BASIC MACHINE CONSTRUCTION ***********************************************************/

void super80_state::machine_reset()
{
	m_portf0 = 0xff;
	m_keylatch = 0xff;
	m_key_pressed = 0;
	m_palette_index = 0;
	machine().scheduler().timer_set(attotime::from_usec(10), timer_expired_delegate(FUNC(super80_state::super80_reset),this));
	membank("boot")->set_entry(1);
}

DRIVER_INIT_MEMBER( super80_state,super80 )
{
	UINT8 *RAM = memregion("maincpu")->base();
	membank("boot")->configure_entries(0, 2, &RAM[0x0000], 0xc000);
}


/*-------------------------------------------------
    QUICKLOAD_LOAD_MEMBER( super80_state, super80 )
-------------------------------------------------*/

QUICKLOAD_LOAD_MEMBER( super80_state, super80 )
{
	UINT16 exec_addr, start_addr, end_addr;

	/* load the binary into memory */
	if (z80bin_load_file(&image, m_maincpu->space(AS_PROGRAM), file_type, &exec_addr, &start_addr, &end_addr) == IMAGE_INIT_FAIL)
		return IMAGE_INIT_FAIL;

	/* is this file executable? */
	if (exec_addr != 0xffff)
		/* check to see if autorun is on */
		if BIT(m_io_config->read(), 0)
			m_maincpu->set_pc(exec_addr);

	return IMAGE_INIT_PASS;
}
