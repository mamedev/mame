// license:BSD-3-Clause
// copyright-holders:Robbbert and unknown others
/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

MAX_LUMPS   192     crude storage units - don't know much about it
MAX_GRANULES    8       lumps consisted of granules.. aha
MAX_SECTORS     5       and granules of sectors

***************************************************************************/

#include "includes/trs80.h"


#define IRQ_M1_RTC      0x80    /* RTC on Model I */
#define IRQ_M1_FDC      0x40    /* FDC on Model I */
#define IRQ_M4_RTC      0x04    /* RTC on Model 4 */
#define CASS_RISE       0x01    /* high speed cass on Model III/4) */
#define CASS_FALL       0x02    /* high speed cass on Model III/4) */
#define MODEL4_MASTER_CLOCK 20275200


TIMER_CALLBACK_MEMBER(trs80_state::cassette_data_callback)
{
/* This does all baud rates. 250 baud (trs80), and 500 baud (all others) set bit 7 of "cassette_data".
    1500 baud (trs80m3, trs80m4) is interrupt-driven and uses bit 0 of "cassette_data" */

	double new_val = (m_cassette->input());

	/* Check for HI-LO transition */
	if ( m_old_cassette_val > -0.2 && new_val < -0.2 )
	{
		m_cassette_data |= 0x80;        /* 500 baud */
		if (m_mask & CASS_FALL) /* see if 1500 baud */
		{
			m_cassette_data = 0;
			m_irq |= CASS_FALL;
			m_maincpu->set_input_line(0, HOLD_LINE);
		}
	}
	else
	if ( m_old_cassette_val < -0.2 && new_val > -0.2 )
	{
		if (m_mask & CASS_RISE) /* 1500 baud */
		{
			m_cassette_data = 1;
			m_irq |= CASS_RISE;
			m_maincpu->set_input_line(0, HOLD_LINE);
		}
	}

	m_old_cassette_val = new_val;
}


/*************************************
 *
 *              Port handlers.
 *
 *************************************/


READ8_MEMBER( trs80_state::trs80m4_e0_r )
{
/* Indicates which devices are interrupting - d6..d3 not emulated.
    Whenever an interrupt occurs, this port is immediately read
    to find out which device requires service. Lowest-numbered
    bit takes precedence. We take this opportunity to clear the
    cpu INT line.

    d6 RS232 Error (Any of {FE, PE, OR} errors has occurred)
    d5 RS232 Rcv (DAV indicates a char ready to be picked up from uart)
    d4 RS232 Xmit (TBMT indicates ready to accept another char from cpu)
    d3 I/O Bus
    d2 RTC
    d1 Cass 1500 baud Falling
    d0 Cass 1500 baud Rising */

	m_maincpu->set_input_line(0, CLEAR_LINE);
	return ~(m_mask & m_irq);
}

READ8_MEMBER( trs80_state::trs80m4_e4_r )
{
/* Indicates which devices are interrupting - d6..d5 not emulated.
    Whenever an NMI occurs, this port is immediately read
    to find out which device requires service. Lowest-numbered
    bit takes precedence. We take this opportunity to clear the
    cpu NMI line.

    d7 status of FDC INTREQ (0=true)
    d6 status of Motor Timeout (0=true)
    d5 status of Reset signal (0=true - this will reboot the computer) */

	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	return ~(m_nmi_mask & m_nmi_data);
}

READ8_MEMBER( trs80_state::trs80m4_e8_r )
{
/* not emulated
    d7 Clear-to-Send (CTS), Pin 5
    d6 Data-Set-Ready (DSR), pin 6
    d5 Carrier Detect (CD), pin 8
    d4 Ring Indicator (RI), pin 22
    d3,d2,d0 Not used
    d1 UART Receiver Input, pin 20 (pin 20 is also DTR) */

	return 0;
}

READ8_MEMBER( trs80_state::trs80m4_ea_r )
{
/* UART Status Register
    d7 Data Received ('1'=condition true)
    d6 Transmitter Holding Register empty ('1'=condition true)
    d5 Overrun Error ('1'=condition true)
    d4 Framing Error ('1'=condition true)
    d3 Parity Error ('1'=condition true)
    d2..d0 Not used */

	UINT8 data=7;
	m_ay31015->set_input_pin(AY31015_SWE, 0);
	data |= m_ay31015->get_output_pin(AY31015_TBMT) ? 0x40 : 0;
	data |= m_ay31015->get_output_pin(AY31015_DAV ) ? 0x80 : 0;
	data |= m_ay31015->get_output_pin(AY31015_OR  ) ? 0x20 : 0;
	data |= m_ay31015->get_output_pin(AY31015_FE  ) ? 0x10 : 0;
	data |= m_ay31015->get_output_pin(AY31015_PE  ) ? 0x08 : 0;
	m_ay31015->set_input_pin(AY31015_SWE, 1);

	return data;
}

READ8_MEMBER( trs80_state::trs80m4_eb_r )
{
/* UART received data */
	UINT8 data = m_ay31015->get_received_data();
	m_ay31015->set_input_pin(AY31015_RDAV, 0);
	m_ay31015->set_input_pin(AY31015_RDAV, 1);
	return data;
}

READ8_MEMBER( trs80_state::trs80m4_ec_r )
{
/* Reset the RTC interrupt */
	m_irq &= ~IRQ_M4_RTC;
	return 0;
}

READ8_MEMBER( trs80_state::sys80_f9_r )
{
/* UART Status Register - d6..d4 not emulated
    d7 Transmit buffer empty (inverted)
    d6 CTS pin
    d5 DSR pin
    d4 CD pin
    d3 Parity Error
    d2 Framing Error
    d1 Overrun
    d0 Data Available */

	UINT8 data = 70;
	m_ay31015->set_input_pin(AY31015_SWE, 0);
	data |= m_ay31015->get_output_pin(AY31015_TBMT) ? 0 : 0x80;
	data |= m_ay31015->get_output_pin(AY31015_DAV ) ? 0x01 : 0;
	data |= m_ay31015->get_output_pin(AY31015_OR  ) ? 0x02 : 0;
	data |= m_ay31015->get_output_pin(AY31015_FE  ) ? 0x04 : 0;
	data |= m_ay31015->get_output_pin(AY31015_PE  ) ? 0x08 : 0;
	m_ay31015->set_input_pin(AY31015_SWE, 1);

	return data;
}

READ8_MEMBER( trs80_state::lnw80_fe_r )
{
	return ((m_mode & 0x78) >> 3) | 0xf0;
}

READ8_MEMBER( trs80_state::trs80_ff_r )
{
/* ModeSel and cassette data
    d7 cassette data from tape
    d2 modesel setting */

	UINT8 data = (~m_mode & 1) << 5;
	return data | m_cassette_data;
}

READ8_MEMBER( trs80_state::trs80m4_ff_r )
{
/* Return of cassette data stream from tape
    d7 Low-speed data
    d6..d1 info from write of port EC
    d0 High-speed data */

	m_irq &= 0xfc;  /* clear cassette interrupts */

	return m_port_ec | m_cassette_data;
}


WRITE8_MEMBER( trs80_state::trs80m4_84_w )
{
/* Hi-res graphics control - d6..d4 not emulated
    d7 Page Control
    d6 Fix upper memory
    d5 Memory bit 1
    d4 Memory bit 0
    d3 Invert Video
    d2 80/64 width
    d1 Select bit 1
    d0 Select bit 0 */

	/* get address space instead of io space */
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	UINT8 *base = m_region_maincpu->base();

	m_mode = (m_mode & 0x73) | (data & 0x8c);

	m_model4 &= 0xce;
	m_model4 |= (data & 3) << 4;

	switch (data & 3)
	{
		case 0: /* normal operation */

			if (m_model4 & 4)   /* Model 4P gets RAM while Model 4 gets ROM */
			{
				if (m_model4 & 8)
					m_bank1->set_base(base);
				else
					m_bank1->set_base(base + 0x10000);

				m_bank2->set_base(base + 0x11000);
				m_bank4->set_base(base + 0x137ea);
			}
			else
			{
				m_bank1->set_base(base);
				m_bank2->set_base(base + 0x01000);
				m_bank4->set_base(base + 0x037ea);
			}

			m_bank7->set_base(base + 0x14000);
			m_bank8->set_base(base + 0x1f400);
			m_bank9->set_base(base + 0x1f800);
			m_bank11->set_base(base + 0x05000);
			m_bank12->set_base(base + 0x06000);
			m_bank14->set_base(base + 0x09000);
			m_bank15->set_base(base + 0x0a000);
			m_bank17->set_base(base + 0x14000);
			m_bank18->set_base(base + 0x1f400);
			m_bank19->set_base(base + 0x1f800);
			mem.install_readwrite_handler (0x37e8, 0x37e9, read8_delegate(FUNC(trs80_state::trs80_printer_r), this), write8_delegate(FUNC(trs80_state::trs80_printer_w), this));    /* 3 & 13 */
			mem.install_read_handler (0x3800, 0x3bff, read8_delegate(FUNC(trs80_state::trs80_keyboard_r), this));   /* 5 */
			mem.install_readwrite_handler (0x3c00, 0x3fff, read8_delegate(FUNC(trs80_state::trs80_videoram_r), this), write8_delegate(FUNC(trs80_state::trs80_videoram_w), this));  /* 6 & 16 */
			break;

		case 1: /* write-only ram backs up the rom */

			if (m_model4 & 4)   /* Model 4P gets RAM while Model 4 gets ROM */
			{
				if (m_model4 & 8)
					m_bank1->set_base(base);
				else
					m_bank1->set_base(base + 0x10000);

				m_bank2->set_base(base + 0x11000);
				m_bank3->set_base(base + 0x137e8);
				m_bank4->set_base(base + 0x137ea);
			}
			else
			{
				m_bank1->set_base(base);
				m_bank2->set_base(base + 0x01000);
				m_bank3->set_base(base + 0x037e8);
				m_bank4->set_base(base + 0x037ea);
			}

			m_bank7->set_base(base + 0x14000);
			m_bank8->set_base(base + 0x1f400);
			m_bank9->set_base(base + 0x1f800);
			m_bank11->set_base(base + 0x10000);
			m_bank12->set_base(base + 0x11000);
			m_bank13->set_base(base + 0x137e8);
			m_bank14->set_base(base + 0x137ea);
			m_bank15->set_base(base + 0x0a000);
			m_bank17->set_base(base + 0x14000);
			m_bank18->set_base(base + 0x1f400);
			m_bank19->set_base(base + 0x1f800);
			mem.install_read_handler (0x3800, 0x3bff, read8_delegate(FUNC(trs80_state::trs80_keyboard_r), this));   /* 5 */
			mem.install_readwrite_handler (0x3c00, 0x3fff, read8_delegate(FUNC(trs80_state::trs80_videoram_r), this), write8_delegate(FUNC(trs80_state::trs80_videoram_w), this));  /* 6 & 16 */
			break;

		case 2: /* keyboard and video are moved to high memory, and the rest is ram */
			m_bank1->set_base(base + 0x10000);
			m_bank2->set_base(base + 0x11000);
			m_bank3->set_base(base + 0x137e8);
			m_bank4->set_base(base + 0x137ea);
			m_bank5->set_base(base + 0x13800);
			m_bank6->set_base(base + 0x13c00);
			m_bank7->set_base(base + 0x14000);
			m_bank11->set_base(base + 0x10000);
			m_bank12->set_base(base + 0x11000);
			m_bank13->set_base(base + 0x137e8);
			m_bank14->set_base(base + 0x137ea);
			m_bank15->set_base(base + 0x13800);
			m_bank16->set_base(base + 0x13c00);
			m_bank17->set_base(base + 0x14000);
			m_bank18->set_base(base + 0x0a000);
			mem.install_read_handler (0xf400, 0xf7ff, read8_delegate(FUNC(trs80_state::trs80_keyboard_r), this));   /* 8 */
			mem.install_readwrite_handler (0xf800, 0xffff, read8_delegate(FUNC(trs80_state::trs80_videoram_r), this), write8_delegate(FUNC(trs80_state::trs80_videoram_w), this));  /* 9 & 19 */
			m_model4++;
			break;

		case 3: /* 64k of ram */
			m_bank1->set_base(base + 0x10000);
			m_bank2->set_base(base + 0x11000);
			m_bank3->set_base(base + 0x137e8);
			m_bank4->set_base(base + 0x137ea);
			m_bank5->set_base(base + 0x13800);
			m_bank6->set_base(base + 0x13c00);
			m_bank7->set_base(base + 0x14000);
			m_bank8->set_base(base + 0x1f400);
			m_bank9->set_base(base + 0x1f800);
			m_bank11->set_base(base + 0x10000);
			m_bank12->set_base(base + 0x11000);
			m_bank13->set_base(base + 0x137e8);
			m_bank14->set_base(base + 0x137ea);
			m_bank15->set_base(base + 0x13800);
			m_bank16->set_base(base + 0x13c00);
			m_bank17->set_base(base + 0x14000);
			m_bank18->set_base(base + 0x1f400);
			m_bank19->set_base(base + 0x1f800);
			break;
	}
}

WRITE8_MEMBER( trs80_state::trs80m4_90_w )
{
	m_speaker->level_w(~data & 1);
}

WRITE8_MEMBER( trs80_state::trs80m4p_9c_w )     /* model 4P only - swaps the ROM with read-only RAM */
{
	/* Meaning of model4 variable:
	    d5..d4 memory mode (as described in section above)
	    d3 rom switch (1=enabled) only effective in mode0 and 1
	    d2 this is a Model 4P
	    d1 this is a Model 4
	    d0 Video banking exists yes/no (1=not banked) */

	m_model4 &= 0xf7;
	m_model4 |= (data << 3);

	if ((m_model4) && (~m_model4 & 0x20))
	{
		switch (m_model4 & 8)
		{
			case 0:     /* Read-only RAM replaces rom */
				m_bank1->set_base(m_region_maincpu->base() + 0x10000);
				break;
			case 8:     /* Normal setup - rom enabled */
				m_bank1->set_base(m_region_maincpu->base());
				break;
		}
	}
}

WRITE8_MEMBER( trs80_state::trs80m4_e0_w )
{
/* Interrupt settings - which devices are allowed to interrupt - bits align with read of E0
    d6 Enable Rec Err
    d5 Enable Rec Data
    d4 Enable Xmit Emp
    d3 Enable I/O int
    d2 Enable RT int
    d1 C fall Int
    d0 C Rise Int */

	m_mask = data;
}

WRITE8_MEMBER( trs80_state::trs80m4_e4_w )
{
/* Disk to NMI interface
    d7 1=enable disk INTRQ to generate NMI
    d6 1=enable disk Motor Timeout to generate NMI */

	m_nmi_mask = data;
}

WRITE8_MEMBER( trs80_state::trs80m4_e8_w )
{
/* d1 when '1' enables control register load (see below) */

	m_reg_load = data & 2;
}

WRITE8_MEMBER( trs80_state::trs80m4_e9_w )
{
/* UART set baud rate. Rx = bits 0..3, Tx = bits 4..7
    00h    50
    11h    75
    22h    110
    33h    134.5
    44h    150
    55h    300
    66h    600
    77h    1200
    88h    1800
    99h    2000
    AAh    2400
    BBh    3600
    CCh    4800
    DDh    7200
    EEh    9600
    FFh    19200 */

	static const int baud_clock[]={ 800, 1200, 1760, 2152, 2400, 4800, 9600, 19200, 28800, 32000, 38400, 57600, 76800, 115200, 153600, 307200 };
	m_ay31015->set_receiver_clock(baud_clock[data & 0x0f]);
	m_ay31015->set_transmitter_clock(baud_clock[data >> 4]);
}

WRITE8_MEMBER( trs80_state::trs80m4_ea_w )
{
	if (m_reg_load)

/* d2..d0 not emulated
    d7 Even Parity Enable ('1'=even, '0'=odd)
    d6='1',d5='1' for 8 bits
    d6='0',d5='1' for 7 bits
    d6='1',d5='0' for 6 bits
    d6='0',d5='0' for 5 bits
    d4 Stop Bit Select ('1'=two stop bits, '0'=one stop bit)
    d3 Parity Inhibit ('1'=disable; No parity, '0'=parity enabled)
    d2 Break ('0'=disable transmit data; continuous RS232 'SPACE' condition)
    d1 Request-to-Send (RTS), pin 4
    d0 Data-Terminal-Ready (DTR), pin 20 */

	{
		m_ay31015->set_input_pin(AY31015_CS, 0);
		m_ay31015->set_input_pin(AY31015_NB1, BIT(data, 6));
		m_ay31015->set_input_pin(AY31015_NB2, BIT(data, 5));
		m_ay31015->set_input_pin(AY31015_TSB, BIT(data, 4));
		m_ay31015->set_input_pin(AY31015_EPS, BIT(data, 7));
		m_ay31015->set_input_pin(AY31015_NP,  BIT(data, 3));
		m_ay31015->set_input_pin(AY31015_CS, 1);
	}
	else
	{
/* not emulated
    d7,d6 Not used
    d5 Secondary Unassigned, pin 18
    d4 Secondary Transmit Data, pin 14
    d3 Secondary Request-to-Send, pin 19
    d2 Break ('0'=disable transmit data; continuous RS232 'SPACE' condition)
    d1 Data-Terminal-Ready (DTR), pin 20
    d0 Request-to-Send (RTS), pin 4 */

	}
}

WRITE8_MEMBER( trs80_state::trs80m4_eb_w )
{
	m_ay31015->set_transmit_data(data);
}

WRITE8_MEMBER( trs80_state::trs80m4_ec_w )
{
/* Hardware settings - d5..d4 not emulated
    d6 CPU fast (1=4MHz, 0=2MHz)
    d5 1=Enable Video Wait
    d4 1=Enable External I/O bus
    d3 1=Enable Alternate Character Set
    d2 Mode Select (0=64 chars, 1=32chars)
    d1 Cassette Motor (1=On) */

	m_maincpu->set_unscaled_clock(data & 0x40 ? MODEL4_MASTER_CLOCK/5 : MODEL4_MASTER_CLOCK/10);

	m_mode = (m_mode & 0xde) | ((data & 4) ? 1 : 0) | ((data & 8) ? 0x20 : 0);

	m_cassette->change_state(( data & 2 ) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR );

	m_port_ec = data & 0x7e;
}

/* Selection of drive and parameters - d6..d5 not emulated.
 A write also causes the selected drive motor to turn on for about 3 seconds.
 When the motor turns off, the drive is deselected.
    d7 1=MFM, 0=FM
    d6 1=Wait
    d5 1=Write Precompensation enabled
    d4 0=Side 0, 1=Side 1
    d3 1=select drive 3
    d2 1=select drive 2
    d1 1=select drive 1
    d0 1=select drive 0 */
WRITE8_MEMBER( trs80_state::trs80m4_f4_w )
{
	floppy_image_device *floppy = NULL;

	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	if (BIT(data, 1)) floppy = m_floppy1->get_device();
	if (BIT(data, 2)) floppy = m_floppy2->get_device();
	if (BIT(data, 3)) floppy = m_floppy3->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		floppy->mon_w(0);
		floppy->ss_w(BIT(data, 4));
	}

	m_fdc->dden_w(!BIT(data, 7));
}

WRITE8_MEMBER( trs80_state::sys80_f8_w )
{
/* not emulated
    d2 reset UART (XR pin)
    d1 DTR
    d0 RTS */
}

WRITE8_MEMBER( trs80_state::sys80_fe_w )
{
/* not emulated
    d4 select internal or external cassette player */

	m_tape_unit = (data & 0x10) ? 2 : 1;
}

/* lnw80 can switch out all the devices, roms and video ram to be replaced by graphics ram. */
WRITE8_MEMBER( trs80_state::lnw80_fe_w )
{
/* lnw80 video options
    d3 bankswitch lower 16k between roms and hires ram (1=hires)
    d2 enable colour    \
    d1 hres         /   these 2 are the bits from the MODE command of LNWBASIC
    d0 inverse video (entire screen) */

	/* get address space instead of io space */
	address_space &mem = m_maincpu->space(AS_PROGRAM);

	m_mode = (m_mode & 0x87) | ((data & 0x0f) << 3);

	if (data & 8)
	{
		mem.unmap_readwrite (0x0000, 0x3fff);
		mem.install_readwrite_handler (0x0000, 0x3fff, read8_delegate(FUNC(trs80_state::trs80_gfxram_r), this), write8_delegate(FUNC(trs80_state::trs80_gfxram_w), this));
	}
	else
	{
		mem.unmap_readwrite (0x0000, 0x3fff);
		mem.install_read_bank (0x0000, 0x2fff, "bank1");
		membank("bank1")->set_base(m_region_maincpu->base());
		mem.install_readwrite_handler (0x37e0, 0x37e3, read8_delegate(FUNC(trs80_state::trs80_irq_status_r), this), write8_delegate(FUNC(trs80_state::trs80_motor_w), this));
		mem.install_readwrite_handler (0x37e8, 0x37eb, read8_delegate(FUNC(trs80_state::trs80_printer_r), this), write8_delegate(FUNC(trs80_state::trs80_printer_w), this));
		mem.install_read_handler (0x37ec, 0x37ec, read8_delegate(FUNC(trs80_state::trs80_wd179x_r), this));
		mem.install_write_handler (0x37ec, 0x37ec, write8_delegate(FUNC(fd1793_t::cmd_w),(fd1793_t*)m_fdc));
		mem.install_readwrite_handler (0x37ed, 0x37ed, read8_delegate(FUNC(fd1793_t::track_r),(fd1793_t*)m_fdc), write8_delegate(FUNC(fd1793_t::track_w),(fd1793_t*)m_fdc));
		mem.install_readwrite_handler (0x37ee, 0x37ee, read8_delegate(FUNC(fd1793_t::sector_r),(fd1793_t*)m_fdc), write8_delegate(FUNC(fd1793_t::sector_w),(fd1793_t*)m_fdc));
		mem.install_readwrite_handler (0x37ef, 0x37ef, read8_delegate(FUNC(fd1793_t::data_r),(fd1793_t*)m_fdc),write8_delegate( FUNC(fd1793_t::data_w),(fd1793_t*)m_fdc));
		mem.install_read_handler (0x3800, 0x38ff, 0, 0x0300, read8_delegate(FUNC(trs80_state::trs80_keyboard_r), this));
		mem.install_readwrite_handler (0x3c00, 0x3fff, read8_delegate(FUNC(trs80_state::trs80_videoram_r), this), write8_delegate(FUNC(trs80_state::trs80_videoram_w), this));
	}
}

WRITE8_MEMBER( trs80_state::trs80_ff_w )
{
/* Standard output port of Model I
    d3 ModeSel bit
    d2 Relay
    d1, d0 Cassette output */

	static const double levels[4] = { 0.0, -1.0, 0.0, 1.0 };
	static int init = 0;

	m_cassette->change_state(( data & 4 ) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR );
	m_cassette->output(levels[data & 3]);
	m_cassette_data &= ~0x80;

	m_mode = (m_mode & 0xfe) | ((data & 8) >> 3);

	if (!init)
	{
		init = 1;
		static INT16 speaker_levels[4] = { 0, -32768, 0, 32767 };
		m_speaker->static_set_levels(*m_speaker, 4, speaker_levels);

	}
	/* Speaker for System-80 MK II - only sounds if relay is off */
	if (~data & 4)
		m_speaker->level_w(data & 3);
}

WRITE8_MEMBER( trs80_state::trs80m4_ff_w )
{
/* Cassette port
    d1, d0 Cassette output */

	static const double levels[4] = { 0.0, -1.0, 0.0, 1.0 };
	m_cassette->output(levels[data & 3]);
	m_cassette_data &= ~0x80;
}


/*************************************
 *
 *      Interrupt handlers.
 *
 *************************************/

INTERRUPT_GEN_MEMBER(trs80_state::trs80_rtc_interrupt)
{
/* This enables the processing of interrupts for the clock and the flashing cursor.
    The OS counts one tick for each interrupt. The Model I has 40 ticks per
    second, while the Model III/4 has 30. */

	if (m_model4)   // Model 4
	{
		if (m_mask & IRQ_M4_RTC)
		{
			m_irq |= IRQ_M4_RTC;
			device.execute().set_input_line(0, HOLD_LINE);
		}
	}
	else        // Model 1
	{
		m_irq |= IRQ_M1_RTC;
		device.execute().set_input_line(0, HOLD_LINE);
	}
}

void trs80_state::trs80_fdc_interrupt_internal()
{
	if (m_model4)
	{
		if (m_nmi_mask & 0x80)   // Model 4 does a NMI
		{
			m_nmi_data = 0x80;
			m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
		}
	}
	else        // Model 1 does a IRQ
	{
		m_irq |= IRQ_M1_FDC;
		m_maincpu->set_input_line(0, HOLD_LINE);
	}
}

INTERRUPT_GEN_MEMBER(trs80_state::trs80_fdc_interrupt)/* not used - should it be? */
{
	trs80_fdc_interrupt_internal();
}

WRITE_LINE_MEMBER(trs80_state::trs80_fdc_intrq_w)
{
	if (state)
	{
		trs80_fdc_interrupt_internal();
	}
	else
	{
		if (m_model4)
			m_nmi_data = 0;
		else
			m_irq &= ~IRQ_M1_FDC;
	}
}


/*************************************
 *                                   *
 *      Memory handlers              *
 *                                   *
 *************************************/

READ8_MEMBER( trs80_state::trs80_wd179x_r )
{
	UINT8 data = 0xff;
	if (BIT(m_io_config->read(), 7))
		data = m_fdc->status_r(space, offset);

	return data;
}

READ8_MEMBER( trs80_state::trs80_printer_r )
{
	return m_cent_status_in->read();
}

WRITE8_MEMBER( trs80_state::trs80_printer_w )
{
	m_cent_data_out->write(space, 0, data);
	m_centronics->write_strobe(0);
	m_centronics->write_strobe(1);
}

WRITE8_MEMBER( trs80_state::trs80_cassunit_w )
{
/* not emulated
    01 for unit 1 (default)
    02 for unit 2 */

	m_tape_unit = data;
}

READ8_MEMBER( trs80_state::trs80_irq_status_r )
{
/* (trs80l2) Whenever an interrupt occurs, 37E0 is read to see what devices require service.
    d7 = RTC
    d6 = FDC
    d2 = Communications (not emulated)
    All interrupting devices are serviced in a single interrupt. There is a mask byte,
    which is dealt with by the DOS. We take the opportunity to reset the cpu INT line. */

	int result = m_irq;
	m_maincpu->set_input_line(0, CLEAR_LINE);
	m_irq = 0;
	return result;
}


WRITE8_MEMBER( trs80_state::trs80_motor_w )
{
	floppy_image_device *floppy = NULL;

	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	if (BIT(data, 1)) floppy = m_floppy1->get_device();
	if (BIT(data, 2)) floppy = m_floppy2->get_device();
	if (BIT(data, 3)) floppy = m_floppy3->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		floppy->mon_w(0);
		floppy->ss_w(BIT(data, 4));
	}

	// switch to fm
	m_fdc->dden_w(1);
}

/*************************************
 *      Keyboard         *
 *************************************/
READ8_MEMBER( trs80_state::trs80_keyboard_r )
{
	UINT8 result = 0;

	if (offset & 1)
		result |= m_io_line0->read();
	if (offset & 2)
		result |= m_io_line1->read();
	if (offset & 4)
		result |= m_io_line2->read();
	if (offset & 8)
		result |= m_io_line3->read();
	if (offset & 16)
		result |= m_io_line4->read();
	if (offset & 32)
		result |= m_io_line5->read();
	if (offset & 64)
		result |= m_io_line6->read();
	if (offset & 128)
		result |= m_io_line7->read();

	return result;
}


/*************************************
 *  Machine              *
 *************************************/

void trs80_state::machine_start()
{
	m_tape_unit=1;
	m_reg_load=1;
	m_nmi_data=0xff;

	m_cassette_data_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(trs80_state::cassette_data_callback),this));
	m_cassette_data_timer->adjust( attotime::zero, 0, attotime::from_hz(11025) );
}

void trs80_state::machine_reset()
{
	m_cassette_data = 0;
}

MACHINE_RESET_MEMBER(trs80_state,trs80m4)
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	m_cassette_data = 0;

	mem.install_read_bank (0x0000, 0x0fff, "bank1");
	m_bank1 = membank("bank1");
	mem.install_read_bank (0x1000, 0x37e7, "bank2");
	m_bank2 = membank("bank2");
	mem.install_read_bank (0x37e8, 0x37e9, "bank3");
	m_bank3 = membank("bank3");
	mem.install_read_bank (0x37ea, 0x37ff, "bank4");
	m_bank4 = membank("bank4");
	mem.install_read_bank (0x3800, 0x3bff, "bank5");
	m_bank5 = membank("bank5");
	mem.install_read_bank (0x3c00, 0x3fff, "bank6");
	m_bank6 = membank("bank6");
	mem.install_read_bank (0x4000, 0xf3ff, "bank7");
	m_bank7 = membank("bank7");
	mem.install_read_bank (0xf400, 0xf7ff, "bank8");
	m_bank8 = membank("bank8");
	mem.install_read_bank (0xf800, 0xffff, "bank9");
	m_bank9 = membank("bank9");

	mem.install_write_bank (0x0000, 0x0fff, "bank11");
	m_bank11 = membank("bank11");
	mem.install_write_bank (0x1000, 0x37e7, "bank12");
	m_bank12 = membank("bank12");
	mem.install_write_bank (0x37e8, 0x37e9, "bank13");
	m_bank13 = membank("bank13");
	mem.install_write_bank (0x37ea, 0x37ff, "bank14");
	m_bank14 = membank("bank14");
	mem.install_write_bank (0x3800, 0x3bff, "bank15");
	m_bank15 = membank("bank15");
	mem.install_write_bank (0x3c00, 0x3fff, "bank16");
	m_bank16 = membank("bank16");
	mem.install_write_bank (0x4000, 0xf3ff, "bank17");
	m_bank17 = membank("bank17");
	mem.install_write_bank (0xf400, 0xf7ff, "bank18");
	m_bank18 = membank("bank18");
	mem.install_write_bank (0xf800, 0xffff, "bank19");
	m_bank19 = membank("bank19");
	trs80m4p_9c_w(mem, 0, 1);   /* Enable the ROM */
	trs80m4_84_w(mem, 0, 0);    /* switch in devices at power-on */
}

MACHINE_RESET_MEMBER(trs80_state,lnw80)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	m_cassette_data = 0;
	m_reg_load = 1;
	lnw80_fe_w(space, 0, 0);
}


/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG 1

#define CMD_TYPE_OBJECT_CODE                            0x01
#define CMD_TYPE_TRANSFER_ADDRESS                       0x02
#define CMD_TYPE_END_OF_PARTITIONED_DATA_SET_MEMBER     0x04
#define CMD_TYPE_LOAD_MODULE_HEADER                     0x05
#define CMD_TYPE_PARTITIONED_DATA_SET_HEADER            0x06
#define CMD_TYPE_PATCH_NAME_HEADER                      0x07
#define CMD_TYPE_ISAM_DIRECTORY_ENTRY                   0x08
#define CMD_TYPE_END_OF_ISAM_DIRECTORY_ENTRY            0x0a
#define CMD_TYPE_PDS_DIRECTORY_ENTRY                    0x0c
#define CMD_TYPE_END_OF_PDS_DIRECTORY_ENTRY             0x0e
#define CMD_TYPE_YANKED_LOAD_BLOCK                      0x10
#define CMD_TYPE_COPYRIGHT_BLOCK                        0x1f

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

QUICKLOAD_LOAD_MEMBER( trs80_state, trs80_cmd )
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	UINT8 type, length;
	UINT8 data[0x100];
	UINT8 addr[2];
	void *ptr;

	while (!image.image_feof())
	{
		image.fread( &type, 1);
		image.fread( &length, 1);

		length -= 2;
		int block_length = length ? length : 256;

		switch (type)
		{
		case CMD_TYPE_OBJECT_CODE:
			{
			image.fread( &addr, 2);
			UINT16 address = (addr[1] << 8) | addr[0];
			if (LOG) logerror("/CMD object code block: address %04x length %u\n", address, block_length);
			ptr = program.get_write_ptr(address);
			image.fread( ptr, block_length);
			}
			break;

		case CMD_TYPE_TRANSFER_ADDRESS:
			{
			image.fread( &addr, 2);
			UINT16 address = (addr[1] << 8) | addr[0];
			if (LOG) logerror("/CMD transfer address %04x\n", address);
			m_maincpu->set_state_int(Z80_PC, address);
			}
			break;

		case CMD_TYPE_LOAD_MODULE_HEADER:
			image.fread( &data, block_length);
			if (LOG) logerror("/CMD load module header '%s'\n", data);
			break;

		case CMD_TYPE_COPYRIGHT_BLOCK:
			image.fread( &data, block_length);
			if (LOG) logerror("/CMD copyright block '%s'\n", data);
			break;

		default:
			image.fread( &data, block_length);
			logerror("/CMD unsupported block type %u!\n", type);
		}
	}

	return IMAGE_INIT_PASS;
}
