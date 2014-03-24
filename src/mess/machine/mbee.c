/***************************************************************************

    microbee.c

    machine driver
    Juergen Buchmueller <pullmoll@t-online.de>, Jan 2000


****************************************************************************/

#include "includes/mbee.h"
#include "machine/z80bin.h"


void mbee_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_MBEE256_KBD:
		mbee256_kbd(ptr, param);
		break;
	case TIMER_MBEE_RTC_IRQ:
		mbee_rtc_irq(ptr, param);
		break;
	case TIMER_MBEE_RESET:
		mbee_reset(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in mbee_state::device_timer");
	}
}


/***********************************************************

    PIO

************************************************************/

WRITE_LINE_MEMBER( mbee_state::pio_ardy )
{
	m_centronics->write_strobe((state) ? 0 : 1);
}

WRITE8_MEMBER( mbee_state::pio_port_a_w )
{
	/* hardware strobe driven by PIO ARDY, bit 7..0 = data */
	m_pio->strobe_a(1); /* needed - otherwise nothing prints */
	m_cent_data_out->write(space, 0, data);
};

WRITE8_MEMBER( mbee_state::pio_port_b_w )
{
/*  PIO port B - d5..d2 not emulated
    d7 network interrupt (microbee network for classrooms)
    d6 speaker
    d5 rs232 output (1=mark)
    d4 rs232 input (0=mark)
    d3 rs232 CTS (0=clear to send)
    d2 rs232 clock or DTR
    d1 cass out and (on 256tc) keyboard irq
    d0 cass in */

	m_cassette->output((data & 0x02) ? -1.0 : +1.0);

	m_speaker->level_w(BIT(data, 6));
};

READ8_MEMBER( mbee_state::pio_port_b_r )
{
	UINT8 data = 0;

	if (m_cassette->input() > 0.03) data |= 1;

	data |= m_clock_pulse;
	data |= m_mbee256_key_available;

	m_clock_pulse = 0;

	return data;
};

const z80pio_interface mbee_z80pio_intf =
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(mbee_state, pio_port_a_w),
	DEVCB_DRIVER_LINE_MEMBER(mbee_state, pio_ardy),
	DEVCB_DRIVER_MEMBER(mbee_state, pio_port_b_r),
	DEVCB_DRIVER_MEMBER(mbee_state, pio_port_b_w),
	DEVCB_NULL
};

/*************************************************************************************

    Floppy DIsk

    The callback is quite simple, no interrupts are used.
    If either IRQ or DRQ activate, they set bit 7 of inport 0x48.

*************************************************************************************/

READ8_MEMBER( mbee_state::mbee_fdc_status_r )
{
/*  d7 indicate if IRQ or DRQ is occuring (1=happening)
    d6..d0 not used */

	return 0x7f | ((m_fdc->intrq_r() || m_fdc->drq_r()) ? 0x80 : 0);
}

WRITE8_MEMBER( mbee_state::mbee_fdc_motor_w )
{
/*  d7..d4 not used
    d3 density (1=MFM)
    d2 side (1=side 1)
    d1..d0 drive select (0 to 3) */

	floppy_image_device *floppy = NULL;
	if ((data&3)==0)
		floppy = m_floppy0->get_device();
	else
	if ((data&3)==1)
		floppy = m_floppy1->get_device();

	m_fdc->set_floppy(floppy);
	m_fdc->dden_w(!BIT(data, 3)); // /Q output of ic29

	if (floppy)
	{
		floppy->mon_w(0); // motor on
		floppy->ss_w(BIT(data, 2)); // inverted on the board
	}
}

/***********************************************************

    256TC Keyboard

************************************************************/


TIMER_CALLBACK_MEMBER(mbee_state::mbee256_kbd)
{
	/* Keyboard scanner is a Mostek M3870 chip. Its speed of operation is determined by a 15k resistor on
	pin 2 (XTL2) and is therefore unknown. If a key change is detected (up or down), the /strobe
	line activates, sending a high to bit 1 of port 2 (one of the pio input lines). The next read of
	port 18 will clear this line, and read the key scancode. It will also signal the 3870 that the key
	data has been read, on pin 38 (/extint). The 3870 can cache up to 9 keys. With no rom dump
	available, the following is a guess.

	The 3870 (MK3870) 8-bit microcontroller is a single chip implementation of Fairchild F8 (Mostek 3850).
	It includes up to 4 KB of mask-programmable ROM, 64 bytes of scratchpad RAM and up to 64 bytes
	of executable RAM. The MCU also integrates 32-bit I/O and a programmable timer. */

	UINT8 i, j;
	UINT8 pressed[15];


	/* see what is pressed */
	pressed[0] = m_io_x0->read();
	pressed[1] = m_io_x1->read();
	pressed[2] = m_io_x2->read();
	pressed[3] = m_io_x3->read();
	pressed[4] = m_io_x4->read();
	pressed[5] = m_io_x5->read();
	pressed[6] = m_io_x6->read();
	pressed[7] = m_io_x7->read();
	pressed[8] = m_io_x8->read();
	pressed[9] = m_io_x9->read();
	pressed[10] = m_io_x10->read();
	pressed[11] = m_io_x11->read();
	pressed[12] = m_io_x12->read();
	pressed[13] = m_io_x13->read();
	pressed[14] = m_io_x14->read();

	/* find what has changed */
	for (i = 0; i < 15; i++)
	{
		if (pressed[i] != m_mbee256_was_pressed[i])
		{
			/* get scankey value */
			for (j = 0; j < 8; j++)
			{
				if (BIT(pressed[i]^m_mbee256_was_pressed[i], j))
				{
					/* put it in the queue */
					m_mbee256_q[m_mbee256_q_pos] = (i << 3) | j | (BIT(pressed[i], j) ? 0x80 : 0);
					if (m_mbee256_q_pos < 19) m_mbee256_q_pos++;
				}
			}
			m_mbee256_was_pressed[i] = pressed[i];
		}
	}

	/* if anything queued, cause an interrupt */
	if (m_mbee256_q_pos)
		m_mbee256_key_available = 2; // set irq

	timer_set(attotime::from_hz(25), TIMER_MBEE256_KBD);
}

READ8_MEMBER( mbee_state::mbee256_18_r )
{
	UINT8 i, data = m_mbee256_q[0]; // get oldest key

	if (m_mbee256_q_pos)
	{
		m_mbee256_q_pos--;
		for (i = 0; i < m_mbee256_q_pos; i++) m_mbee256_q[i] = m_mbee256_q[i+1]; // ripple queue
	}

	m_mbee256_key_available = 0; // clear irq
	return data;
}


/***********************************************************

    256TC Change CPU speed

************************************************************/

READ8_MEMBER( mbee_state::mbee256_speed_low_r )
{
	m_maincpu->set_unscaled_clock(3375000);
	return 0xff;
}

READ8_MEMBER( mbee_state::mbee256_speed_high_r )
{
	m_maincpu->set_unscaled_clock(6750000);
	return 0xff;
}



/***********************************************************

    256TC Real Time Clock

************************************************************/

WRITE8_MEMBER( mbee_state::mbee_04_w )  // address
{
	m_rtc->write(space, 0, data);
}

WRITE8_MEMBER( mbee_state::mbee_06_w )  // write
{
	m_rtc->write(space, 1, data);
}

READ8_MEMBER( mbee_state::mbee_07_r )   // read
{
	return m_rtc->read(space, 1);
}

TIMER_CALLBACK_MEMBER(mbee_state::mbee_rtc_irq)
{
	UINT8 data = m_rtc->read(m_maincpu->space(AS_PROGRAM), 12);
	if (data) m_clock_pulse = 0x80;
	timer_set(attotime::from_hz(1), TIMER_MBEE_RTC_IRQ);
}


/***********************************************************

    256TC Memory Banking

    Bits 0, 1 and 5 select which bank goes into 0000-7FFF.
    Bit 2 disables ROM, replacing it with RAM.
    Bit 3 disables Video, replacing it with RAM.
    Bit 4 switches the video circuits between F000-FFFF and
          8000-8FFF.

    In case of a clash, video overrides ROM which overrides RAM.

************************************************************/

WRITE8_MEMBER( mbee_state::mbee256_50_w )
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);

	// primary low banks
	m_boot->set_entry((data & 3) | ((data & 0x20) >> 3));
	m_bank1->set_entry((data & 3) | ((data & 0x20) >> 3));

	// 9000-EFFF
	m_bank9->set_entry((data & 4) ? 1 : 0);

	// 8000-8FFF, F000-FFFF
	mem.unmap_readwrite (0x8000, 0x87ff);
	mem.unmap_readwrite (0x8800, 0x8fff);
	mem.unmap_readwrite (0xf000, 0xf7ff);
	mem.unmap_readwrite (0xf800, 0xffff);

	switch (data & 0x1c)
	{
		case 0x00:
			mem.install_read_bank (0x8000, 0x87ff, "bank8l");
			mem.install_read_bank (0x8800, 0x8fff, "bank8h");
			mem.install_readwrite_handler (0xf000, 0xf7ff, read8_delegate(FUNC(mbee_state::mbeeppc_low_r), this), write8_delegate(FUNC(mbee_state::mbeeppc_low_w), this));
			mem.install_readwrite_handler (0xf800, 0xffff, read8_delegate(FUNC(mbee_state::mbeeppc_high_r), this), write8_delegate(FUNC(mbee_state::mbeeppc_high_w), this));
			m_bank8l->set_entry(0); // rom
			m_bank8h->set_entry(0); // rom
			break;
		case 0x04:
			mem.install_read_bank (0x8000, 0x87ff, "bank8l");
			mem.install_read_bank (0x8800, 0x8fff, "bank8h");
			mem.install_readwrite_handler (0xf000, 0xf7ff, read8_delegate(FUNC(mbee_state::mbeeppc_low_r), this), write8_delegate(FUNC(mbee_state::mbeeppc_low_w), this));
			mem.install_readwrite_handler (0xf800, 0xffff, read8_delegate(FUNC(mbee_state::mbeeppc_high_r), this), write8_delegate(FUNC(mbee_state::mbeeppc_high_w), this));
			m_bank8l->set_entry(1); // ram
			m_bank8h->set_entry(1); // ram
			break;
		case 0x08:
		case 0x18:
			mem.install_read_bank (0x8000, 0x87ff, "bank8l");
			mem.install_read_bank (0x8800, 0x8fff, "bank8h");
			mem.install_read_bank (0xf000, 0xf7ff, "bankfl");
			mem.install_read_bank (0xf800, 0xffff, "bankfh");
			m_bank8l->set_entry(0); // rom
			m_bank8h->set_entry(0); // rom
			m_bankfl->set_entry(0); // ram
			m_bankfh->set_entry(0); // ram
			break;
		case 0x0c:
		case 0x1c:
			mem.install_read_bank (0x8000, 0x87ff, "bank8l");
			mem.install_read_bank (0x8800, 0x8fff, "bank8h");
			mem.install_read_bank (0xf000, 0xf7ff, "bankfl");
			mem.install_read_bank (0xf800, 0xffff, "bankfh");
			m_bank8l->set_entry(1); // ram
			m_bank8h->set_entry(1); // ram
			m_bankfl->set_entry(0); // ram
			m_bankfh->set_entry(0); // ram
			break;
		case 0x10:
		case 0x14:
			mem.install_readwrite_handler (0x8000, 0x87ff, read8_delegate(FUNC(mbee_state::mbeeppc_low_r), this), write8_delegate(FUNC(mbee_state::mbeeppc_low_w), this));
			mem.install_readwrite_handler (0x8800, 0x8fff, read8_delegate(FUNC(mbee_state::mbeeppc_high_r), this), write8_delegate(FUNC(mbee_state::mbeeppc_high_w), this));
			mem.install_read_bank (0xf000, 0xf7ff, "bankfl");
			mem.install_read_bank (0xf800, 0xffff, "bankfh");
			m_bankfl->set_entry(0); // ram
			m_bankfh->set_entry(0); // ram
			break;
	}
}

/***********************************************************

    128k Memory Banking

    The only difference to the 256TC is that bit 5 switches
    between rom2 and rom3. Since neither of these is dumped,
    this bit is not emulated. If it was, this scheme is used:

    Low - rom2 occupies C000-FFFF
    High - ram = C000-DFFF, rom3 = E000-FFFF.

************************************************************/

WRITE8_MEMBER( mbee_state::mbee128_50_w )
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);

	// primary low banks
	m_boot->set_entry((data & 3));
	m_bank1->set_entry((data & 3));

	// 9000-EFFF
	m_bank9->set_entry((data & 4) ? 1 : 0);

	// 8000-8FFF, F000-FFFF
	mem.unmap_readwrite (0x8000, 0x87ff);
	mem.unmap_readwrite (0x8800, 0x8fff);
	mem.unmap_readwrite (0xf000, 0xf7ff);
	mem.unmap_readwrite (0xf800, 0xffff);

	switch (data & 0x1c)
	{
		case 0x00:
			mem.install_read_bank (0x8000, 0x87ff, "bank8l");
			mem.install_read_bank (0x8800, 0x8fff, "bank8h");
			mem.install_readwrite_handler (0xf000, 0xf7ff, read8_delegate(FUNC(mbee_state::mbeeppc_low_r),this), write8_delegate(FUNC(mbee_state::mbeeppc_low_w),this));
			mem.install_readwrite_handler (0xf800, 0xffff, read8_delegate(FUNC(mbee_state::mbeeppc_high_r),this), write8_delegate(FUNC(mbee_state::mbeeppc_high_w),this));
			m_bank8l->set_entry(0); // rom
			m_bank8h->set_entry(0); // rom
			break;
		case 0x04:
			// these 2 lines were read_bank but readwrite is needed for bios 2,3,4,5 to boot
			mem.install_readwrite_bank (0x8000, 0x87ff, "bank8l");
			mem.install_readwrite_bank (0x8800, 0x8fff, "bank8h");
			mem.install_readwrite_handler (0xf000, 0xf7ff, read8_delegate(FUNC(mbee_state::mbeeppc_low_r),this), write8_delegate(FUNC(mbee_state::mbeeppc_low_w),this));
			mem.install_readwrite_handler (0xf800, 0xffff, read8_delegate(FUNC(mbee_state::mbeeppc_high_r),this), write8_delegate(FUNC(mbee_state::mbeeppc_high_w),this));
			m_bank8l->set_entry(1); // ram
			m_bank8h->set_entry(1); // ram
			break;
		case 0x08:
		case 0x18:
			mem.install_read_bank (0x8000, 0x87ff, "bank8l");
			mem.install_read_bank (0x8800, 0x8fff, "bank8h");
			mem.install_read_bank (0xf000, 0xf7ff, "bankfl");
			mem.install_read_bank (0xf800, 0xffff, "bankfh");
			m_bank8l->set_entry(0); // rom
			m_bank8h->set_entry(0); // rom
			m_bankfl->set_entry(0); // ram
			m_bankfh->set_entry(0); // ram
			break;
		case 0x0c:
		case 0x1c:
			mem.install_read_bank (0x8000, 0x87ff, "bank8l");
			mem.install_read_bank (0x8800, 0x8fff, "bank8h");
			mem.install_read_bank (0xf000, 0xf7ff, "bankfl");
			mem.install_read_bank (0xf800, 0xffff, "bankfh");
			m_bank8l->set_entry(1); // ram
			m_bank8h->set_entry(1); // ram
			m_bankfl->set_entry(0); // ram
			m_bankfh->set_entry(0); // ram
			break;
		case 0x10:
		case 0x14:
			mem.install_readwrite_handler (0x8000, 0x87ff, read8_delegate(FUNC(mbee_state::mbeeppc_low_r),this), write8_delegate(FUNC(mbee_state::mbeeppc_low_w),this));
			mem.install_readwrite_handler (0x8800, 0x8fff, read8_delegate(FUNC(mbee_state::mbeeppc_high_r),this), write8_delegate(FUNC(mbee_state::mbeeppc_high_w),this));
			mem.install_read_bank (0xf000, 0xf7ff, "bankfl");
			mem.install_read_bank (0xf800, 0xffff, "bankfh");
			m_bankfl->set_entry(0); // ram
			m_bankfh->set_entry(0); // ram
			break;
	}
}


/***********************************************************

    64k Memory Banking

    Bit 2 disables ROM, replacing it with RAM.

    Due to lack of documentation, it is not possible to know
    if other bits are used.

************************************************************/

WRITE8_MEMBER( mbee_state::mbee64_50_w )
{
	if BIT(data, 2)
	{
		m_boot->set_entry(0);
		m_bankl->set_entry(0);
		m_bankh->set_entry(0);
	}
	else
	{
		m_bankl->set_entry(1);
		m_bankh->set_entry(1);
	}
}


/***********************************************************

    ROM Banking on older models

    Set A to 0 or 1 then read the port to switch between the
    halves of the Telcom ROM.

    Output the PAK number to choose an optional PAK ROM.

    The bios will support 256 PAKs, although normally only
    8 are available in hardware. Each PAK is normally a 4K
    ROM. If 8K ROMs are used, the 2nd half becomes PAK+8,
    thus 16 PAKs in total. This is used in the PC85 models.

************************************************************/

READ8_MEMBER( mbee_state::mbeeic_0a_r )
{
	return m_0a;
}

WRITE8_MEMBER( mbee_state::mbeeic_0a_w )
{
	m_0a = data;
	m_pak->set_entry(data & 15);
}

READ8_MEMBER( mbee_state::mbeepc_telcom_low_r )
{
/* Read of port 0A - set Telcom rom to first half */
	m_telcom->set_entry(0);
	return m_0a;
}

READ8_MEMBER( mbee_state::mbeepc_telcom_high_r )
{
/* Read of port 10A - set Telcom rom to 2nd half */
	m_telcom->set_entry(1);
	return m_0a;
}


/***********************************************************

    Machine

************************************************************/

/*
  On reset or power on, a circuit forces rom 8000-8FFF to appear at 0000-0FFF, while ram is disabled.
  It gets set back to normal on the first attempt to write to memory. (/WR line goes active).
*/


/* after the first 4 bytes have been read from ROM, switch the ram back in */
TIMER_CALLBACK_MEMBER(mbee_state::mbee_reset)
{
	m_boot->set_entry(0);
}

void mbee_state::machine_reset_common_disk()
{
}

MACHINE_RESET_MEMBER(mbee_state,mbee)
{
	m_boot->set_entry(1);
	timer_set(attotime::from_usec(4), TIMER_MBEE_RESET);
}

MACHINE_RESET_MEMBER(mbee_state,mbee56)
{
	machine_reset_common_disk();
	m_boot->set_entry(1);
	timer_set(attotime::from_usec(4), TIMER_MBEE_RESET);
}

MACHINE_RESET_MEMBER(mbee_state,mbee64)
{
	machine_reset_common_disk();
	m_boot->set_entry(1);
	m_bankl->set_entry(1);
	m_bankh->set_entry(1);
}

MACHINE_RESET_MEMBER(mbee_state,mbee128)
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	machine_reset_common_disk();
	mbee128_50_w(mem,0,0); // set banks to default
	m_boot->set_entry(4); // boot time
}

MACHINE_RESET_MEMBER(mbee_state,mbee256)
{
	UINT8 i;
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	machine_reset_common_disk();
	for (i = 0; i < 15; i++) m_mbee256_was_pressed[i] = 0;
	m_mbee256_q_pos = 0;
	mbee256_50_w(mem,0,0); // set banks to default
	m_boot->set_entry(8); // boot time
	timer_set(attotime::from_usec(4), TIMER_MBEE_RESET);
}

MACHINE_RESET_MEMBER(mbee_state,mbeett)
{
	UINT8 i;
	for (i = 0; i < 15; i++) m_mbee256_was_pressed[i] = 0;
	m_mbee256_q_pos = 0;
	m_boot->set_entry(1);
	timer_set(attotime::from_usec(4), TIMER_MBEE_RESET);
}

INTERRUPT_GEN_MEMBER(mbee_state::mbee_interrupt)
{
// Due to the uncertainly and hackage here, this is commented out for now - Robbbert - 05-Oct-2010
#if 0

	//address_space &space = m_maincpu->space(AS_PROGRAM);
	/* The printer status connects to the pio ASTB pin, and the printer changing to not
	    busy should signal an interrupt routine at B61C, (next line) but this doesn't work.
	    The line below does what the interrupt should be doing. */
	/* But it would break any program loaded to that area of memory, such as CP/M programs */

	//m_z80pio->strobe_a(centronics_busy_r(m_centronics)); /* signal int when not busy (L->H) */
	//space.write_byte(0x109, centronics_busy_r(m_centronics));


	/* once per frame, pulse the PIO B bit 7 - it is in the schematic as an option,
	but need to find out what it does */
	m_clock_pulse = 0x80;
	irq0_line_hold(device);

#endif
}

DRIVER_INIT_MEMBER(mbee_state,mbee)
{
	UINT8 *RAM = memregion("maincpu")->base();
	m_boot->configure_entries(0, 2, &RAM[0x0000], 0x8000);
	m_size = 0x4000;
}

DRIVER_INIT_MEMBER(mbee_state,mbeeic)
{
	UINT8 *RAM = memregion("maincpu")->base();
	m_boot->configure_entries(0, 2, &RAM[0x0000], 0x8000);

	RAM = memregion("pakrom")->base();
	m_pak->configure_entries(0, 16, &RAM[0x0000], 0x2000);

	m_pak->set_entry(0);
	m_size = 0x8000;
}

DRIVER_INIT_MEMBER(mbee_state,mbeepc)
{
	UINT8 *RAM = memregion("maincpu")->base();
	m_boot->configure_entries(0, 2, &RAM[0x0000], 0x8000);

	RAM = memregion("telcomrom")->base();
	m_telcom->configure_entries(0, 2, &RAM[0x0000], 0x1000);

	RAM = memregion("pakrom")->base();
	m_pak->configure_entries(0, 16, &RAM[0x0000], 0x2000);

	m_pak->set_entry(0);
	m_telcom->set_entry(0);
	m_size = 0x8000;
}

DRIVER_INIT_MEMBER(mbee_state,mbeepc85)
{
	UINT8 *RAM = memregion("maincpu")->base();
	m_boot->configure_entries(0, 2, &RAM[0x0000], 0x8000);

	RAM = memregion("telcomrom")->base();
	m_telcom->configure_entries(0, 2, &RAM[0x0000], 0x1000);

	RAM = memregion("pakrom")->base();
	m_pak->configure_entries(0, 16, &RAM[0x0000], 0x2000);

	m_pak->set_entry(5);
	m_telcom->set_entry(0);
	m_size = 0x8000;
}

DRIVER_INIT_MEMBER(mbee_state,mbeeppc)
{
	UINT8 *RAM = memregion("maincpu")->base();
	m_boot->configure_entry(0, &RAM[0x0000]);

	RAM = memregion("basicrom")->base();
	m_basic->configure_entries(0, 2, &RAM[0x0000], 0x2000);
	m_boot->configure_entry(1, &RAM[0x0000]);

	RAM = memregion("telcomrom")->base();
	m_telcom->configure_entries(0, 2, &RAM[0x0000], 0x1000);

	RAM = memregion("pakrom")->base();
	m_pak->configure_entries(0, 16, &RAM[0x0000], 0x2000);

	m_pak->set_entry(5);
	m_telcom->set_entry(0);
	m_basic->set_entry(0);
	m_size = 0x8000;
}

DRIVER_INIT_MEMBER(mbee_state,mbee56)
{
	UINT8 *RAM = memregion("maincpu")->base();
	m_boot->configure_entries(0, 2, &RAM[0x0000], 0xe000);
	m_size = 0xe000;
}

DRIVER_INIT_MEMBER(mbee_state,mbee64)
{
	UINT8 *RAM = memregion("maincpu")->base();
	m_boot->configure_entry(0, &RAM[0x0000]);
	m_bankl->configure_entry(0, &RAM[0x1000]);
	m_bankl->configure_entry(1, &RAM[0x9000]);
	m_bankh->configure_entry(0, &RAM[0x8000]);

	RAM = memregion("bootrom")->base();
	m_bankh->configure_entry(1, &RAM[0x0000]);
	m_boot->configure_entry(1, &RAM[0x0000]);

	m_size = 0xf000;
}

DRIVER_INIT_MEMBER(mbee_state,mbee128)
{
	UINT8 *RAM = memregion("maincpu")->base();
	m_boot->configure_entries(0, 4, &RAM[0x0000], 0x8000); // standard banks 0000
	m_bank1->configure_entries(0, 4, &RAM[0x1000], 0x8000); // standard banks 1000
	m_bank8l->configure_entry(1, &RAM[0x0000]); // shadow ram
	m_bank8h->configure_entry(1, &RAM[0x0800]); // shadow ram
	m_bank9->configure_entry(1, &RAM[0x1000]); // shadow ram
	m_bankfl->configure_entry(0, &RAM[0xf000]); // shadow ram
	m_bankfh->configure_entry(0, &RAM[0xf800]); // shadow ram

	RAM = memregion("bootrom")->base();
	m_bank9->configure_entry(0, &RAM[0x1000]); // rom
	m_boot->configure_entry(4, &RAM[0x0000]); // rom at boot for 4usec
	m_bank8l->configure_entry(0, &RAM[0x0000]); // rom
	m_bank8h->configure_entry(0, &RAM[0x0800]); // rom

	m_size = 0x8000;
}

DRIVER_INIT_MEMBER(mbee_state,mbee256)
{
	UINT8 *RAM = memregion("maincpu")->base();
	m_boot->configure_entries(0, 8, &RAM[0x0000], 0x8000); // standard banks 0000
	m_bank1->configure_entries(0, 8, &RAM[0x1000], 0x8000); // standard banks 1000
	m_bank8l->configure_entry(1, &RAM[0x0000]); // shadow ram
	m_bank8h->configure_entry(1, &RAM[0x0800]); // shadow ram
	m_bank9->configure_entry(1, &RAM[0x1000]); // shadow ram
	m_bankfl->configure_entry(0, &RAM[0xf000]); // shadow ram
	m_bankfh->configure_entry(0, &RAM[0xf800]); // shadow ram

	RAM = memregion("bootrom")->base();
	m_bank9->configure_entry(0, &RAM[0x1000]); // rom
	m_boot->configure_entry(8, &RAM[0x0000]); // rom at boot for 4usec
	m_bank8l->configure_entry(0, &RAM[0x0000]); // rom
	m_bank8h->configure_entry(0, &RAM[0x0800]); // rom

	timer_set(attotime::from_hz(1), TIMER_MBEE_RTC_IRQ);   /* timer for rtc */
	timer_set(attotime::from_hz(25), TIMER_MBEE256_KBD);   /* timer for kbd */

	m_size = 0x8000;
}

DRIVER_INIT_MEMBER(mbee_state,mbeett)
{
	UINT8 *RAM = memregion("maincpu")->base();
	m_boot->configure_entries(0, 2, &RAM[0x0000], 0x8000);

	RAM = memregion("telcomrom")->base();
	m_telcom->configure_entries(0, 2, &RAM[0x0000], 0x1000);

	RAM = memregion("pakrom")->base();
	m_pak->configure_entries(0, 16, &RAM[0x0000], 0x2000);

	m_pak->set_entry(5);
	m_telcom->set_entry(0);

	timer_set(attotime::from_hz(1), TIMER_MBEE_RTC_IRQ);   /* timer for rtc */
	timer_set(attotime::from_hz(25), TIMER_MBEE256_KBD);   /* timer for kbd */

	m_size = 0x8000;
}


/***********************************************************

    Quickload

    These load the standard BIN format, as well
    as BEE, COM and MWB files.

************************************************************/

QUICKLOAD_LOAD_MEMBER( mbee_state, mbee )
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	UINT16 i, j;
	UINT8 data, sw = ioport("CONFIG")->read() & 1;   /* reading the dipswitch: 1 = autorun */

	if (!core_stricmp(image.filetype(), "mwb"))
	{
		/* mwb files - standard basic files */
		for (i = 0; i < quickload_size; i++)
		{
			j = 0x8c0 + i;

			if (image.fread(&data, 1) != 1)
			{
				image.message("Unexpected EOF");
				return IMAGE_INIT_FAIL;
			}

			if ((j < m_size) || (j > 0xefff))
				space.write_byte(j, data);
			else
			{
				image.message("Not enough memory in this microbee");
				return IMAGE_INIT_FAIL;
			}
		}

		if (sw)
		{
			space.write_word(0xa2,0x801e);  /* fix warm-start vector to get around some copy-protections */
			m_maincpu->set_pc(0x801e);
		}
		else
			space.write_word(0xa2,0x8517);
	}
	else if (!core_stricmp(image.filetype(), "com"))
	{
		/* com files - most com files are just machine-language games with a wrapper and don't need cp/m to be present */
		for (i = 0; i < quickload_size; i++)
		{
			j = 0x100 + i;

			if (image.fread(&data, 1) != 1)
			{
				image.message("Unexpected EOF");
				return IMAGE_INIT_FAIL;
			}

			if ((j < m_size) || (j > 0xefff))
				space.write_byte(j, data);
			else
			{
				image.message("Not enough memory in this microbee");
				return IMAGE_INIT_FAIL;
			}
		}

		if (sw) m_maincpu->set_pc(0x100);
	}
	else if (!core_stricmp(image.filetype(), "bee"))
	{
		/* bee files - machine-language games that start at 0900 */
		for (i = 0; i < quickload_size; i++)
		{
			j = 0x900 + i;

			if (image.fread(&data, 1) != 1)
			{
				image.message("Unexpected EOF");
				return IMAGE_INIT_FAIL;
			}

			if ((j < m_size) || (j > 0xefff))
				space.write_byte(j, data);
			else
			{
				image.message("Not enough memory in this microbee");
				return IMAGE_INIT_FAIL;
			}
		}

		if (sw) m_maincpu->set_pc(0x900);
	}

	return IMAGE_INIT_PASS;
}


/*-------------------------------------------------
    QUICKLOAD_LOAD_MEMBER( mbee_state, mbee_z80bin )
-------------------------------------------------*/

QUICKLOAD_LOAD_MEMBER( mbee_state, mbee_z80bin )
{
	UINT16 execute_address, start_addr, end_addr;
	int autorun;

	/* load the binary into memory */
	if (z80bin_load_file(&image, file_type, &execute_address, &start_addr, &end_addr) == IMAGE_INIT_FAIL)
		return IMAGE_INIT_FAIL;

	/* is this file executable? */
	if (execute_address != 0xffff)
	{
		/* check to see if autorun is on (I hate how this works) */
		autorun = ioport("CONFIG")->read_safe(0xFF) & 1;

		address_space &space = m_maincpu->space(AS_PROGRAM);

		space.write_word(0xa6, execute_address);            /* fix the EXEC command */

		if (autorun)
		{
			space.write_word(0xa2, execute_address);        /* fix warm-start vector to get around some copy-protections */
			m_maincpu->set_pc(execute_address);
		}
		else
		{
			space.write_word(0xa2, 0x8517);
		}
	}

	return IMAGE_INIT_PASS;
}
