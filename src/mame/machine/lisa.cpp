// license:GPL-2.0+
// copyright-holders:Raphael Nabet
/*
    experimental LISA driver

    This driver runs most ROM test code successfully, and it loads the boot
    file, but it locks when booting from Lisa OS (probably because of floppy
    write bug).  MacWorks boots fine, though (I think it looks when we
    attempt to write to floppy, too).

    TODO :
    * fix floppy write bug
    * *** Boot and run LisaTest !!! ***
    * finish MMU (does not switch to bank 0 on 68k trap) --- fixed in June 2003???
    * support hard disk to boot office system
    * finish keyboard/mouse support
    * finish clock support
    * write SCC support
    * finalize sound support (involves adding new features to the 6522 VIA core)
    * fix warm-reset (I think I need to use a callback when 68k RESET
      instruction is called)
    * write support for additionnal hardware (hard disk, etc...)
    * emulate LISA1 (?)
    * optimize MMU emulation !

    DONE (just a reminder to uplift my spirit) :
    * the lion's share of MMU (spring 2000)
    * video hardware (except contrast control) (spring 2000)
    * LISA2/Mac XL floppy hardware (november 2000)

    Credits :
    * the lisaemu project (<http://www.sundernet.com/>) has gathered much
       hardware information (books, schematics...) without which this driver
       could never have been written
    * The driver raised the interest of several MESS regulars (Paul Lunga,
      Dennis Munsie...) who supported my feeble efforts

    Raphael Nabet, 2000-2003
*/

#include "includes/lisa.h"


/*
    pointers with RAM & ROM location
*/

/* up to 2MB of 68k RAM (normally 1MB or 512kb), generally 16kb of ROM */

/* offsets in "maincpu" */
#define RAM_OFFSET 0x004000
#define ROM_OFFSET 0x000000

/* 1kb of RAM for 6504 floppy disk controller (shared with 68000), and 4kb of
ROM (8kb on some boards, but then only one 4kb bank is selected, according to
the drive type (TWIGGY or 3.5'')) */

/* special ROM (includes S/N) */


/*
    MMU regs
*/





/*
    parity logic - only hard errors are emulated for now, since
    a) the ROMs power-up test only tests hard errors
    b) most memory boards do not use soft errors (i.e. they only generate 1
      parity bit to detect errors, instead of generating several ECC bits to
      fix errors)
*/




/*
    video
*/


/*
    2 vias : one is used for communication with COPS ; the other may be used to interface
    a hard disk
*/

/*
    floppy disk interface
*/




/*
    lisa model identification
*/
enum lisa_model_t
{
	/*lisa1,*/      /* twiggy floppy drive */
	lisa2,      /* 3.5'' Sony floppy drive */
	lisa210,    /* modified I/O board, and internal 10Meg drive */
	mac_xl      /* same as above with modified video */
};



/*
    protos
*/





/*
    Interrupt handling
*/

void lisa_state::field_interrupts()
{
	if (m_parity_error_pending)
		return; /* don't touch anything... */

#if 0
	if (RSIR)
		// serial interrupt
		m_maincpu->set_input_line_and_vector(M68K_IRQ_6, ASSERT_LINE, M68K_INT_ACK_AUTOVECTOR);
	else if (int0)
		// external interrupt
		m_maincpu->set_input_line_and_vector(M68K_IRQ_5, ASSERT_LINE, M68K_INT_ACK_AUTOVECTOR);
	else if (int1)
		// external interrupt
		m_maincpu->set_input_line_and_vector(M68K_IRQ_4, ASSERT_LINE, M68K_INT_ACK_AUTOVECTOR);
	else if (int2)
		// external interrupt
		m_maincpu->set_input_line_and_vector(M68K_IRQ_3, ASSERT_LINE, M68K_INT_ACK_AUTOVECTOR);
	else
#endif
	if (m_KBIR)
		/* COPS VIA interrupt */
		m_maincpu->set_input_line_and_vector(M68K_IRQ_2, ASSERT_LINE, M68K_INT_ACK_AUTOVECTOR);
	else if (m_FDIR || m_VTIR)
		/* floppy disk or VBl */
		m_maincpu->set_input_line_and_vector(M68K_IRQ_1, ASSERT_LINE, M68K_INT_ACK_AUTOVECTOR);
	else
		/* clear all interrupts */
		m_maincpu->set_input_line_and_vector(M68K_IRQ_1, CLEAR_LINE, M68K_INT_ACK_AUTOVECTOR);
}

void lisa_state::set_parity_error_pending(int value)
{
#if 1
	/* does not work well due to bugs in 68k cores */
	m_parity_error_pending = value;
	if (m_parity_error_pending)
	{
		m_maincpu->set_input_line_and_vector(M68K_IRQ_7, ASSERT_LINE, M68K_INT_ACK_AUTOVECTOR);
	}
	else
	{
		m_maincpu->set_input_line(M68K_IRQ_7, CLEAR_LINE);
	}
#else
	/* work-around... */
	if ((! m_parity_error_pending) && value)
	{
		m_parity_error_pending = 1;
		m_maincpu->set_input_line_and_vector(M68K_IRQ_7, PULSE_LINE, M68K_INT_ACK_AUTOVECTOR);
	}
	else if (m_parity_error_pending && (! value))
	{
		m_parity_error_pending = 0;
		field_interrupts();
	}
#endif
}

void lisa_state::set_VTIR(int value)
{
	if (m_VTIR != value)
	{
		m_VTIR = value;
		if (m_VTIR==1)
			field_interrupts();
	}
}



/*
    keyboard interface (COPS simulation; our COPS CPU core is too broken and too esoteric to emulate this correctly, I tried)
*/

void lisa_state::COPS_send_data_if_possible()
{
	if ((! m_hold_COPS_data) && m_fifo_size && (! m_COPS_Ready))
	{
//        printf("COPsim: sending %02x to VIA\n", m_fifo_data[m_fifo_head]);

		UINT8 data = m_fifo_data[m_fifo_head];/* output data */
		m_via0->write_pa0((data>>0)&1);
		m_via0->write_pa1((data>>1)&1);
		m_via0->write_pa2((data>>2)&1);
		m_via0->write_pa3((data>>3)&1);
		m_via0->write_pa4((data>>4)&1);
		m_via0->write_pa5((data>>5)&1);
		m_via0->write_pa6((data>>6)&1);
		m_via0->write_pa7((data>>7)&1);

		if (m_fifo_head == m_mouse_data_offset)
			m_mouse_data_offset = -1;    /* we just phased out the mouse data in buffer */
		m_fifo_head = (m_fifo_head+1) & 0x7;
		m_fifo_size--;
		m_via0->write_ca1(1);        /* pulse ca1 so that VIA reads it */
		m_via0->write_ca1(0);        /* BTW, I have no idea how a real COPS does it ! */
	}
}

/* send data (queue it into the FIFO if needed) */
void lisa_state::COPS_queue_data(const UINT8 *data, int len)
{
#if 0
	if (m_fifo_size + len <= 8)
#else
	/* trash old data */
	while (m_fifo_size > 8 - len)
	{
		if (m_fifo_head == m_mouse_data_offset)
			m_mouse_data_offset = -1;    /* we just phased out the mouse data in buffer */
		m_fifo_head = (m_fifo_head+1) & 0x7;
		m_fifo_size--;
	}
#endif

	{
//      printf("Adding %d bytes of data to FIFO\n", len);

		while (len--)
		{
			m_fifo_data[m_fifo_tail] = * (data++);
			m_fifo_tail = (m_fifo_tail+1) & 0x7;
			m_fifo_size++;
		}

		// do not try to send immediately here, the VIA is set to latch mode so
		// sending too soon after a command can result in the DDR not yet being
		// returned to input and the first byte gets trashed
	}
}

/*
    scan_keyboard()

    scan the keyboard, and add key transition codes to buffer as needed
*/
/* shamelessly stolen from machine/mac.c :-) */

/* keyboard matrix to detect transition */

void lisa_state::scan_keyboard()
{
	int i, j;
	UINT8 keycode;
	UINT32 keybuf[8] = { m_io_line0->read(), m_io_line1->read(), m_io_line2->read(), m_io_line3->read(),
						m_io_line4->read(), m_io_line5->read(), m_io_line6->read(), m_io_line7->read() };

	if (! m_COPS_force_unplug)
		for (i=0; i<8; i++)
		{
			if (keybuf[i] != m_key_matrix[i])
			{   /* if state has changed, find first bit which has changed */
				/*logerror("keyboard state changed, %d %X\n", i, keybuf);*/

				for (j=0; j<16; j++)
				{
					if (((keybuf[i] ^ m_key_matrix[i]) >> j) & 1)
					{
						/* update key_matrix */
						m_key_matrix[i] = (m_key_matrix[i] & ~ (1 << j)) | (keybuf[i] & (1 << j));

						/* create key code */
						keycode = (i << 4) | j;
						if (keybuf[i] & (1 << j))
						{   /* key down */
							keycode |= 0x80;
						}
#if 0
						if (keycode == m_NMIcode)
						{   /* generate NMI interrupt */
							m_maincpu->set_input_line(M68K_IRQ_7, PULSE_LINE);
							m_maincpu->set_input_line_vector(M68K_IRQ_7, M68K_INT_ACK_AUTOVECTOR);
						}
#endif
						COPS_queue_data(&keycode, 1);
					}
				}
			}
		}
}

/* handle mouse moves */
/* shamelessly stolen from machine/mac.c :-) */
TIMER_CALLBACK_MEMBER(lisa_state::handle_mouse)
{
	int diff_x = 0, diff_y = 0;
	int new_mx, new_my;

#if 0
	if (m_COPS_force_unplug)
		return; /* ???? */
#endif

	new_mx = m_io_mouse_x->read();
	new_my = m_io_mouse_y->read();

	/* see if it moved in the x coord */
	if (new_mx != m_last_mx)
	{
		diff_x = new_mx - m_last_mx;

		/* check for wrap */
		if (diff_x > 0x80)
			diff_x = 0x100-diff_x;
		if  (diff_x < -0x80)
			diff_x = -0x100-diff_x;

		m_last_mx = new_mx;
	}
	/* see if it moved in the y coord */
	if (new_my != m_last_my)
	{
		diff_y = new_my - m_last_my;

		/* check for wrap */
		if (diff_y > 0x80)
			diff_y = 0x100-diff_y;
		if  (diff_y < -0x80)
			diff_y = -0x100-diff_y;

		m_last_my = new_my;
	}

	/* update any remaining count and then return */
	if (diff_x || diff_y)
	{
		if (m_mouse_data_offset != -1)
		{
			m_fifo_data[m_mouse_data_offset] += diff_x;
			m_fifo_data[(m_mouse_data_offset+1) & 0x7] += diff_y;
		}
		else
		{
#if 0
			if (m_fifo_size <= 5)
#else
			/* trash old data */
			while (m_fifo_size > 5)
			{
				m_fifo_head = (m_fifo_head+1) & 0x7;
				m_fifo_size--;
			}
#endif

			{
				/*logerror("Adding 3 bytes of mouse data to FIFO\n");*/

				m_fifo_data[m_fifo_tail] = 0;
				m_mouse_data_offset = m_fifo_tail = (m_fifo_tail+1) & 0x7;
				m_fifo_data[m_fifo_tail] = diff_x;
				m_fifo_tail = (m_fifo_tail+1) & 0x7;
				m_fifo_data[m_fifo_tail] = diff_y;
				m_fifo_tail = (m_fifo_tail+1) & 0x7;
				m_fifo_size += 3;

				/*logerror("handle_mouse : trying to send data to VIA\n");*/
				COPS_send_data_if_possible();
			}
			/* else, mouse data is lost forever (correct ??) */
		}
	}
}

/* read command from the VIA port A */
TIMER_CALLBACK_MEMBER(lisa_state::read_COPS_command)
{
	int command;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	m_COPS_Ready = 0;
	m_via0->write_pb6(m_COPS_Ready);

	/*logerror("read_COPS_command : trying to send data to VIA\n");*/
	COPS_send_data_if_possible();

	/* some pull-ups allow the COPS to read 1s when the VIA port is not set as output */
	command = (m_COPS_command | (~ m_via0->read(space, via6522_device::VIA_DDRA))) & 0xff;

//    printf("Dropping Ready, command = %02x\n", command);

	if (command & 0x80)
		return; /* NOP */

	if (command & 0xF0)
	{   /* commands with 4-bit immediate operand */
		int immediate = command & 0xf;

		switch ((command & 0xF0) >> 4)
		{
		case 0x1:   /* write clock data */
			if (m_clock_regs.clock_write_ptr != -1)
			{
				switch (m_clock_regs.clock_write_ptr)
				{
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
					/* alarm */
					m_clock_regs.alarm &= ~ (0xf << (4 * (4 - m_clock_regs.clock_write_ptr)));
					m_clock_regs.alarm |= immediate << (4 * (4 - m_clock_regs.clock_write_ptr));
					break;
				case 5:
					/* year */
					m_clock_regs.years = immediate;
					break;
				case 6:
					/* day */
					m_clock_regs.days1 = immediate;
					break;
				case 7:
					/* day */
					m_clock_regs.days2 = immediate;
					break;
				case 8:
					/* day */
					m_clock_regs.days3 = immediate;
					break;
				case 9:
					/* hours */
					m_clock_regs.hours1 = immediate;
					break;
				case 10:
					/* hours */
					m_clock_regs.hours2 = immediate;
					break;
				case 11:
					/* minutes */
					m_clock_regs.minutes1 = immediate;
					break;
				case 12:
					/* minutes */
					m_clock_regs.minutes1 = immediate;
					break;
				case 13:
					/* seconds */
					m_clock_regs.seconds1 = immediate;
					break;
				case 14:
					/* seconds */
					m_clock_regs.seconds2 = immediate;
					break;
				case 15:
					/* tenth */
					m_clock_regs.tenths = immediate;
					break;
				}
				m_clock_regs.clock_write_ptr++;
				if (m_clock_regs.clock_write_ptr == 16)
					m_clock_regs.clock_write_ptr = -1;
			}

			break;

		case 0x2:   /* set clock mode */
			if (immediate & 0x8)
			{   /* start setting the clock */
				m_clock_regs.clock_write_ptr = 0;
			}
			else
			{   /* clock write disabled */
				m_clock_regs.clock_write_ptr = -1;
			}

			if (! (immediate & 0x4))
			{   /* enter sleep mode */
				/* ... */
			}
			else
			{   /* wake up */
				/* should never happen */
			}

			m_clock_regs.clock_mode = (clock_mode_t)(immediate & 0x3);
			break;

#if 0
		/* LED commands - not implemented in production LISAs */
		case 0x3:   /* write 4 keyboard LEDs */
			keyboard_leds = (keyboard_leds & 0x0f) | (immediate << 4);
			break;

		case 0x4:   /* write next 4 keyboard LEDs */
			keyboard_leds = (keyboard_leds & 0xf0) | immediate;
			break;
#endif

		case 0x5:   /* set high nibble of NMI character to nnnn */
			m_NMIcode = (m_NMIcode & 0x0f) | (immediate << 4);
			break;

		case 0x6:   /* set low nibble of NMI character to nnnn */
			m_NMIcode = (m_NMIcode & 0xf0) | immediate;
			break;

		case 0x7:   /* send mouse command */
			if (immediate & 0x8)
				m_mouse_timer->adjust(attotime::zero, 0, attotime::from_msec((immediate & 0x7)*4)); /* enable mouse */
			else
				m_mouse_timer->reset();
			break;
		}
	}
	else
	{   /* operand-less commands */
		switch (command)
		{
		case 0x0:   /*Turn I/O port on (???) */

			break;

		case 0x1:   /*Turn I/O port off (???) */

			break;

		case 0x2:   /* Read clock data */
			{
				/* format and send reply */

				UINT8 reply[7];

				reply[0] = 0x80;
				reply[1] = 0xE0 | m_clock_regs.years;
				reply[2] = (m_clock_regs.days1 << 4) | m_clock_regs.days2;
				reply[3] = (m_clock_regs.days3 << 4) | m_clock_regs.hours1;
				reply[4] = (m_clock_regs.hours2 << 4) | m_clock_regs.minutes1;
				reply[5] = (m_clock_regs.minutes2 << 4) | m_clock_regs.seconds1;
				reply[6] = (m_clock_regs.seconds2 << 4) | m_clock_regs.tenths;

				COPS_queue_data(reply, 7);
			}
			break;
		}
	}
}

/* this timer callback raises the COPS Ready line, which tells the COPS is about to read a command */
TIMER_CALLBACK_MEMBER(lisa_state::set_COPS_ready)
{
	m_COPS_Ready = 1;
	m_via0->write_pb6(m_COPS_Ready);

	/* impulsion width : +/- 20us */
	machine().scheduler().timer_set(attotime::from_usec(20), timer_expired_delegate(FUNC(lisa_state::read_COPS_command),this));
}

void lisa_state::reset_COPS()
{
	int i;

	m_fifo_size = 0;
	m_fifo_head = 0;
	m_fifo_tail = 0;
	m_mouse_data_offset = -1;

	for (i=0; i<8; i++)
		m_key_matrix[i] = 0;

	m_mouse_timer->reset();
}

void lisa_state::unplug_keyboard()
{
	static const UINT8 cmd[2] =
	{
		0x80,   /* RESET code */
		0xFD    /* keyboard unplugged */
	};

	COPS_queue_data(cmd, 2);
}

void lisa_state::plug_keyboard()
{
	/*
	    possible keyboard IDs according to Lisa Hardware Manual and boot ROM source code

	    2 MSBs : "mfg code" (-> 0x80 for Keytronics, 0x00 for "APD")
	    6 LSBs :
	        0x0x : "old US keyboard"
	        0x3f : US keyboard
	        0x3d : Canadian keyboard
	        0x2f : UK
	        0x2e : German
	        0x2d : French
	        0x27 : Swiss-French
	        0x26 : Swiss-German
	        unknown : spanish, US dvorak, italian & swedish
	*/

	static const UINT8 cmd[2] =
	{
		0x80,   /* RESET code */
		0x3f    /* keyboard ID - US for now */
	};

	COPS_queue_data(cmd, 2);
}


/* called at power-up */
void lisa_state::init_COPS()
{
	m_COPS_Ready = 0;
	m_via0->write_pb6(m_COPS_Ready);

	reset_COPS();
}



/* VIA1 accessors (COPS, sound, and 2 hard disk lines) */

/*
    PA0-7 (I/O) : VIA <-> COPS data bus
    CA1 (I) : COPS sending valid data
    CA2 (O) : VIA -> COPS handshake
*/
WRITE8_MEMBER(lisa_state::COPS_via_out_a)
{
//    printf("VIA A = %02x\n", data);
	m_COPS_command = data;
}

WRITE_LINE_MEMBER(lisa_state::COPS_via_out_ca2)
{
	m_hold_COPS_data = state;

	/*logerror("COPS CA2 line state : %d\n", val);*/

	/*logerror("COPS_via_out_ca2 : trying to send data to VIA\n");*/
	COPS_send_data_if_possible();
}

/*
    PB7 (O) : CR* ("Controller Reset", used by hard disk interface) ???
    PB6 (I) : CRDY ("COPS ready") : set low by the COPS for 20us when it is reading a command
        from the data bus (command latched at low-to-high transition)
    PB5 (I/O) : PR* ; as output : "parity error latch reset" (only when CR* and RESET* are
        inactive) ; as input : low when CR* or RESET are low.
    PB4 (I) : FDIR (floppy disk interrupt request - the fdc shared RAM should not be accessed
        unless this bit is 1)
    PB1-3 (O) : sound volume
    PB0 (O) : forces disconnection of keyboard and mouse (allows to retrive keyboard ID, etc.)

    CB1 : not used
    CB2 (O) : sound output
*/

WRITE8_MEMBER(lisa_state::COPS_via_out_b)
{
	/* pull-up */
	data |= (~ m_via0->read(space, via6522_device::VIA_DDRA)) & 0x01;

	if (data & 0x01)
	{
		if (m_COPS_force_unplug)
		{
			m_COPS_force_unplug = 0;
			plug_keyboard();
		}
	}
	else
	{
		if (! m_COPS_force_unplug)
		{
			m_COPS_force_unplug = 1;
			unplug_keyboard();
			//reset_COPS(state);
		}
	}
}

WRITE_LINE_MEMBER(lisa_state::COPS_via_out_cb2)
{
	m_speaker->level_w(state);
}

void lisa_state::COPS_via_irq_func(int val)
{
	if (m_KBIR != val)
	{
		m_KBIR = val;
		field_interrupts();
	}
}

/* VIA2 accessors (hard disk, and a few floppy disk lines) */

/*
    PA0-7 (I/O) : VIA <-> hard disk data bus (cf PB3)
    CA1 (I) : hard disk BSY line
    CA2 (O) : hard disk PSTRB* line
*/

/*
    PB7 (O) : WCNT line : set contrast latch on low-to-high transition ?
    PB6 (I) : floppy disk DISK DIAG line
    PB5 (I) : hard disk data DD0-7 current parity (does not depend on PB2)
    PB4 (O) : hard disk CMD* line
    PB3 (O) : hard disk DR/W* line ; controls the direction of the drivers on the data bus
    PB2 (O) : when low, disables hard disk interface drivers
    PB1 (I) : hard disk BSY line
    PB0 (I) : hard disk OCD (Open Cable Detect) line : 0 when hard disk attached
    CB1 : not used
    CB2 (I) : current parity latch value
*/



/*
    LISA video emulation
*/

void lisa_state::video_start()
{
}

/*
    Video update
*/
UINT32 lisa_state::screen_update_lisa(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 *v;
	int x, y;
	/* resolution is 720*364 on lisa, vs 608*431 on mac XL */
	int resx = (m_features.has_mac_xl_video) ? 608 : 720;   /* width */
	int resy = (m_features.has_mac_xl_video) ? 431 : 364;   /* height */

	UINT8 line_buffer[720];

	v = m_videoram_ptr;

	for (y = 0; y < resy; y++)
	{
		for (x = 0; x < resx; x++)
//          line_buffer[x] = (v[(x+y*resx)>>4] & (0x8000 >> ((x+y*resx) & 0xf))) ? 0 : 1;
			line_buffer[x] = (v[(x+y*resx)>>4] & (0x8000 >> (x & 0xf))) ? 0 : 1;
		draw_scanline8(bitmap, 0, y, resx, line_buffer, m_palette->pens());
	}
	return 0;
}

#if 0   // we can execute directly out of read handlers now, so this shouldn't be necessary any more.  #if 0'd for documentation until we get everything working.
DIRECT_UPDATE_HANDLER (lisa_OPbaseoverride)
{
	lisa_state *state = machine.driver_data<lisa_state>();
	/* upper 7 bits -> segment # */
	int segment = (address >> 17) & 0x7f;
	int the_seg = m_seg;

	address &= 0xffffff;

	printf("lisa: logical address %x\n", address);

	if (m_setup)
	{
		if (address & 0x004000)
		{
//          the_seg = 0;    /* correct ??? */
		}
		else
		{
			if (address & 0x008000)
			{   /* MMU register : BUS error ??? */
				printf("illegal opbase address %lX\n", (long) address);
			}
			else
			{   /* system ROMs */
				direct.explicit_configure((address & 0xffc000), (address & 0xffc000) + 0x003fff, 0xffffff, m_rom_ptr - (address & 0x3fff));
			}

			return -1;
		}

	}

	if (m_maincpu->state_int(M68K_SR) & 0x2000)
	{
		/* supervisor mode -> force register file 0 */
		the_seg = 0;
	}

	{
		int seg_offset = address & 0x01ffff;

		/* add relevant origin -> address */
		offs_t mapped_address = (m_mmu_regs[the_seg][segment].sorg + seg_offset) & 0x1fffff;

		switch ((mmu_entry_t)m_mmu_regs[the_seg][segment].type)
		{
		case RAM_r:
		case RAM_rw:
			if (seg_offset > m_mmu_regs[the_seg][segment].slim)
			{
				/* out of segment limits : bus error */
				printf("illegal opbase address%lX\n", (long) address);
			}
			direct.explicit_configure((address & 0xffc000), (address & 0xffc000) + 0x003fff, 0xffffff, m_ram_ptr + mapped_address - address);
			printf("RAM\n");
			break;

		case RAM_stack_r:
		case RAM_stack_rw:  /* stack : bus error ??? */
		case IO:            /* I/O : bus error ??? */
		case invalid:       /* unmapped segment */
			/* bus error */
			printf("illegal opbase address%lX\n", (long) address);
			break;

		case special_IO:
			direct.explicit_configure((address & 0xffc000), (address & 0xffc000) + 0x003fff, 0xffffff, m_rom_ptr + (mapped_address & 0x003fff) - address);
			printf("ROM\n");
			break;
		}
	}

	return -1;
}
#endif

/* should save PRAM to file */
/* TODO : save time difference with host clock, set default date, etc */
void lisa_state::nvram_init(nvram_device &nvram, void *data, size_t size)
{
	memset(data, 0x00, size);

	{
		/* Now we copy the host clock into the Lisa clock */
		system_time systime;
		machine().base_datetime(systime);

		m_clock_regs.alarm = 0xfffffL;
		/* The clock count starts on 1st January 1980 */
		m_clock_regs.years = (systime.local_time.year - 1980) & 0xf;
		m_clock_regs.days1 = (systime.local_time.day + 1) / 100;
		m_clock_regs.days2 = ((systime.local_time.day + 1) / 10) % 10;
		m_clock_regs.days3 = (systime.local_time.day + 1) % 10;
		m_clock_regs.hours1 = systime.local_time.hour / 10;
		m_clock_regs.hours2 = systime.local_time.hour % 10;
		m_clock_regs.minutes1 = systime.local_time.minute / 10;
		m_clock_regs.minutes2 = systime.local_time.minute % 10;
		m_clock_regs.seconds1 = systime.local_time.second / 10;
		m_clock_regs.seconds2 = systime.local_time.second % 10;
		m_clock_regs.tenths = 0;

		m_clock_regs.clock_mode = timer_disable;
		m_clock_regs.clock_write_ptr = -1;
	}
#if 0
	UINT32 temp32;
	SINT8 temp8;
	temp32 = (m_clock_regs.alarm << 12) | (m_clock_regs.years << 8) | (m_clock_regs.days1 << 4)
	| m_clock_regs.days2;

	temp32 = (m_clock_regs.days3 << 28) | (m_clock_regs.hours1 << 24) | (m_clock_regs.hours2 << 20)
	| (m_clock_regs.minutes1 << 16) | (m_clock_regs.minutes2 << 12)
	| (m_clock_regs.seconds1 << 8) | (m_clock_regs.seconds2 << 4) | m_clock_regs.tenths;

	temp8 = clock_mode;         /* clock mode */

	temp8 = m_clock_regs.clock_write_ptr;    /* clock byte to be written next (-1 if clock write disabled) */
#endif

}


#ifdef UNUSED_FUNCTION
void lisa_state::init_lisa1(void)
{
	m_model = lisa1;
	m_features.has_fast_timers = 0;
	m_features.floppy_hardware = twiggy;
	m_features.has_double_sided_floppy = 1;
	m_features.has_mac_xl_video = 0;
}
#endif

DRIVER_INIT_MEMBER(lisa_state,lisa2)
{
	m_ram_ptr = memregion("maincpu")->base() + RAM_OFFSET;
	m_rom_ptr = memregion("maincpu")->base() + ROM_OFFSET;
	m_model = lisa2;
	m_features.has_fast_timers = 0;
	m_features.floppy_hardware = sony_lisa2;
	m_features.has_double_sided_floppy = 0;
	m_features.has_mac_xl_video = 0;

	m_bad_parity_table = std::make_unique<UINT8[]>(0x40000);  /* 1 bit per byte of CPU RAM */
}

DRIVER_INIT_MEMBER(lisa_state,lisa210)
{
	m_ram_ptr = memregion("maincpu")->base() + RAM_OFFSET;
	m_rom_ptr = memregion("maincpu")->base() + ROM_OFFSET;
	m_model = lisa210;
	m_features.has_fast_timers = 1;
	m_features.floppy_hardware = sony_lisa210;
	m_features.has_double_sided_floppy = 0;
	m_features.has_mac_xl_video = 0;

	m_bad_parity_table = std::make_unique<UINT8[]>(0x40000);  /* 1 bit per byte of CPU RAM */
}

DRIVER_INIT_MEMBER(lisa_state,mac_xl)
{
	m_ram_ptr = memregion("maincpu")->base() + RAM_OFFSET;
	m_rom_ptr = memregion("maincpu")->base() + ROM_OFFSET;
	m_model = mac_xl;
	m_features.has_fast_timers = 1;
	m_features.floppy_hardware = sony_lisa210;
	m_features.has_double_sided_floppy = 0;
	m_features.has_mac_xl_video = 1;

	m_bad_parity_table = std::make_unique<UINT8[]>(0x40000);  /* 1 bit per byte of CPU RAM */
}

void lisa_state::machine_start()
{
	m_mouse_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(lisa_state::handle_mouse),this));

	/* read command every ms (don't know the real value) */
	machine().scheduler().timer_pulse(attotime::from_msec(1), timer_expired_delegate(FUNC(lisa_state::set_COPS_ready),this));

	m_nvram->set_base(m_fdc_ram, 1024);
}

void lisa_state::machine_reset()
{
	m_ram_ptr = memregion("maincpu")->base() + RAM_OFFSET;
	m_rom_ptr = memregion("maincpu")->base() + ROM_OFFSET;
	m_videoROM_ptr = memregion("gfx1")->base();

//  m_maincpu->space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate_create_static(lisa_OPbaseoverride, *machine()));
//  m68k_set_reset_callback(m_maincpu, /*lisa_reset_instr_callback*/NULL);

	/* init MMU */
	m_setup = 1;
	m_seg = 0;

	/* init parity */
	m_diag2 = 0;
	m_test_parity = 0;
	m_parity_error_pending = 0;

	m_bad_parity_count = 0;
	memset(m_bad_parity_table.get(), 0, 0x40000); /* Clear */

	/* init video */

	m_VTMSK = 0;
	set_VTIR(0);

	m_video_address_latch = 0;
	m_videoram_ptr = (UINT16 *) m_ram_ptr;

	m_FDIR = 0;
	m_via0->write_pb4(m_FDIR);

	/* tell there is no hard disk : */
	m_via1->write_pb0(1);

	/* keep busy high to work around a bug??? */
	//m_via1->write_pb1(1);

	m_DISK_DIAG = 0;
	m_via1->write_pb6(m_DISK_DIAG);

	/* reset COPS keyboard/mouse controller */
	init_COPS();

	{
		COPS_via_out_ca2(0);    /* VIA core forgets to do so */
	}

	/* initialize floppy */
	{
		if (m_features.floppy_hardware == sony_lisa2)
		{
			sony_set_enable_lines(m_fdc, 1);   /* on lisa2, drive unit 1 is always selected (?) */
		}
	}

	/* reset 68k to pick up proper vectors from MMU */
	m_maincpu->reset();
}

INTERRUPT_GEN_MEMBER(lisa_state::lisa_interrupt)
{
	if ((++m_frame_count) == 6)
	{   /* increment clock every 1/10s */
		m_frame_count = 0;

		if (m_clock_regs.clock_mode != clock_timer_disable)
		{
			if ((++m_clock_regs.tenths) == 10)
			{
				m_clock_regs.tenths = 0;

				if (m_clock_regs.clock_mode != timer_disable)
				{
					if (m_clock_regs.alarm == 0)
					{
						/* generate reset (should cause a VIA interrupt...) */
						static const UINT8 cmd[2] =
						{
							0x80,   /* RESET code */
							0xFC    /* timer time-out */
						};
						COPS_queue_data(cmd, 2);

						m_clock_regs.alarm = 0xfffffL;
					}
					else
					{
						m_clock_regs.alarm--;
					}
				}

				if ((++m_clock_regs.seconds2) == 10)
				{
					m_clock_regs.seconds2 = 0;

					if ((++m_clock_regs.seconds1) == 6)
					{
						m_clock_regs.seconds1 = 0;

						if ((++m_clock_regs.minutes2) == 10)
						{
							m_clock_regs.minutes2 = 0;

							if ((++m_clock_regs.minutes1) == 6)
							{
								m_clock_regs.minutes1 = 0;

								if ((++m_clock_regs.hours2) == 10)
								{
									m_clock_regs.hours2 = 0;

									m_clock_regs.hours1++;
								}

								if ((m_clock_regs.hours1*10 + m_clock_regs.hours2) == 24)
								{
									m_clock_regs.hours1 = m_clock_regs.hours2 = 0;

									if ((++m_clock_regs.days3) == 10)
									{
										m_clock_regs.days3 = 0;

										if ((++m_clock_regs.days2) == 10)
										{
											m_clock_regs.days2 = 0;

											m_clock_regs.days1++;
										}
									}

									if ((m_clock_regs.days1*100 + m_clock_regs.days2*10 + m_clock_regs.days3) ==
										((m_clock_regs.years % 4) ? 366 : 367))
									{
										m_clock_regs.days1 = m_clock_regs.days2 = m_clock_regs.days3 = 0;

										m_clock_regs.years = (m_clock_regs.years + 1) & 0xf;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	/* set VBI */
	if (m_VTMSK)
		set_VTIR(1);
	else
		set_VTIR(0);

	/* do keyboard scan */
	scan_keyboard();
}

/*
    Lots of fun with the Lisa fdc hardware

    The iwm floppy select line is connected to the drive SEL line in Lisa2 (which is why Lisa 2
    cannot support 2 floppy drives)...
*/

void lisa_state::lisa_fdc_ttl_glue_access(offs_t offset)
{
	switch ((offset & 0x000E) >> 1)
	{
	case 0:
		/*stop = offset & 1;*/  /* stop/run motor pulse generation */
		break;
	case 2:
		/*MT0 = offset & 1;*/   /* ???? */
		break;
	case 3:
		/* enable/disable the motor on Lisa 1 */
		/* can disable the motor on Lisa 2/10, too (although it is not useful) */
		/* On lisa 2, commands the loading of the speed register on lisalite board */
		if (m_features.floppy_hardware == sony_lisa2)
		{
			int oldMT1 = m_MT1;
			m_MT1 = offset & 1;
			if (m_MT1 && ! oldMT1)
			{
				m_PWM_floppy_motor_speed = (m_PWM_floppy_motor_speed << 1) & 0xff;
				if (m_fdc->get_lines() & APPLEFDC_PH0)
					m_PWM_floppy_motor_speed |= 1;
				sony_set_speed(((256-m_PWM_floppy_motor_speed) * 1.3) + 237);
			}
		}
		/*else
		    m_MT1 = offset & 1;*/
		break;
	case 4:
		/*DIS = offset & 1;*/   /* forbids access from the 68000 to our RAM */
		break;
	case 5:
		/*HDS = offset & 1;*/       /* head select (-> disk side) on twiggy */
#if 0
		if (m_features.floppy_hardware == twiggy)
			twiggy_set_head_line(offset & 1);
		else
#endif
		if (m_features.floppy_hardware == sony_lisa210)
			sony_set_sel_line(m_fdc, offset & 1);
		break;
	case 6:
		m_DISK_DIAG = offset & 1;
		m_via1->write_pb6(m_DISK_DIAG);
		break;
	case 7:
		m_FDIR = offset & 1; /* Interrupt request to 68k */
		m_via0->write_pb4(m_FDIR);
		field_interrupts();
		break;
	}
}

READ8_MEMBER(lisa_state::lisa_fdc_io_r)
{
	int answer=0;

	switch ((offset & 0x0030) >> 4)
	{
	case 0: /* IWM */
		answer = m_fdc->read(offset);
		break;

	case 1: /* TTL glue */
		lisa_fdc_ttl_glue_access(offset);
		answer = 0; /* ??? */
		break;

	case 2: /* pulses the PWM LOAD line (bug!) */
		answer = 0; /* ??? */
		break;

	case 3: /* not used */
		answer = 0; /* ??? */
		break;
	}

	return answer;
}

WRITE8_MEMBER(lisa_state::lisa_fdc_io_w)
{
	switch ((offset & 0x0030) >> 4)
	{
	case 0: /* IWM */
		m_fdc->write(offset, data);
		break;

	case 1: /* TTL glue */
		lisa_fdc_ttl_glue_access(offset);
		break;

	case 2: /* writes the PWM register */
		/* the written value is used to generate the motor speed control signal */
#if 0
		if (m_features.floppy_hardware == twiggy)
			twiggy_set_speed((256-data) * 1.3 /* ??? */ + 237 /* ??? */);
		else
#endif
		if (m_features.floppy_hardware == sony_lisa210)
			sony_set_speed(((256-data) * 1.3) + 237);
		break;

	case 3: /* not used */
		break;
	}
}

READ8_MEMBER(lisa_state::lisa_fdc_r)
{
	if (! (offset & 0x1000))
	{
		if (! (offset & 0x0800))
			if (! (offset & 0x0400))
				return m_fdc_ram[offset & 0x03ff];
			else
				return lisa_fdc_io_r(space, offset & 0x03ff);
		else
			return 0;   /* ??? */
	}
	else
		return m_fdc_rom[offset & 0x0fff];
}

READ8_MEMBER(lisa_state::lisa210_fdc_r)
{
	if (! (offset & 0x1000))
	{
		if (! (offset & 0x0400))
			if (! (offset & 0x0800))
				return m_fdc_ram[offset & 0x03ff];
			else
				return lisa_fdc_io_r(space, offset & 0x03ff);
		else
			return 0;   /* ??? */
	}
	else
		return m_fdc_rom[offset & 0x0fff];
}

WRITE8_MEMBER(lisa_state::lisa_fdc_w)
{
	if (! (offset & 0x1000))
	{
		if (! (offset & 0x0800))
		{
			if (! (offset & 0x0400))
				m_fdc_ram[offset & 0x03ff] = data;
			else
				lisa_fdc_io_w(space, offset & 0x03ff, data);
		}
	}
}

WRITE8_MEMBER(lisa_state::lisa210_fdc_w)
{
	if (! (offset & 0x1000))
	{
		if (! (offset & 0x0400))
		{
			if (! (offset & 0x0800))
				m_fdc_ram[offset & 0x03ff] = data;
			else
				lisa_fdc_io_w(space, offset & 0x03ff, data);
		}
	}
}

READ16_MEMBER(lisa_state::lisa_r)
{
	int answer=0;

	/* segment register set */
	int the_seg = m_seg;

	/* upper 7 bits -> segment # */
	int segment = (offset >> 16) & 0x7f;

	/*logerror("read, logical address%lX\n", offset);*/

	if (m_setup)
	{   /* special setup mode */
		if (offset & 0x002000)
		{
			the_seg = 0;        // TRUSTED by Lisa Hardware Manual section 2.3.3 and MMU startup test
		}
		else
		{
			if (offset & 0x004000)
			{   /* read MMU register */
				/*logerror("read from segment registers (%X:%X) ", the_seg, segment);*/
				if (offset & 0x000004)
				{   /* sorg register */
					answer = m_real_mmu_regs[the_seg][segment].sorg;
					/*logerror("sorg, data = %X\n", answer);*/
				}
				else
				{   /* slim register */
					answer = m_real_mmu_regs[the_seg][segment].slim;
					/*logerror("slim, data = %X\n", answer);*/
				}
			}
			else
			{   /* system ROMs */
				answer = ((UINT16*)m_rom_ptr)[(offset & 0x001fff)];
				/*logerror("dst address in ROM (setup mode)\n");*/
			}

			return answer;
		}
	}

	if (m_maincpu->state_int(M68K_SR) & 0x2000)
		/* supervisor mode -> force register file 0 */
		the_seg = 0;

	{
		/* offset in segment */
		int seg_offset = (offset & 0x00ffff) << 1;

		/* add relevant origin -> address */
		offs_t address = (m_mmu_regs[the_seg][segment].sorg + seg_offset) & 0x1fffff;

		/*logerror("read, logical address%lX\n", offset);
		logerror("physical address%lX\n", address);*/

		switch (m_mmu_regs[the_seg][segment].type)
		{
		case RAM_stack_r:
		case RAM_stack_rw:
			if (address <= m_mmu_regs[the_seg][segment].slim)
			{
				/* out of segment limits : bus error */

			}
			answer = *(UINT16 *)(m_ram_ptr + address);

			if (m_bad_parity_count && m_test_parity
					&& (m_bad_parity_table[address >> 3] & (0x3 << (address & 0x7))))
			{
				m_mem_err_addr_latch = address >> 5;
				set_parity_error_pending(1);
			}

			break;

		case RAM_r:
		case RAM_rw:
			if (address > m_mmu_regs[the_seg][segment].slim)
			{
				/* out of segment limits : bus error */

			}
			answer = *(UINT16 *)(m_ram_ptr + address);

			if (m_bad_parity_count && m_test_parity
					&& (m_bad_parity_table[address >> 3] & (0x3 << (address & 0x7))))
			{
				m_mem_err_addr_latch = address >> 5;
				set_parity_error_pending(1);
			}

			break;

		case IO:
			answer = lisa_IO_r(space, (address & 0x00ffff) >> 1, mem_mask);

			break;

		case invalid:       /* unmapped segment */
			/* bus error */

			answer = 0;

			break;

		case special_IO:
			if (! (address & 0x008000))
				answer = *(UINT16 *)(m_rom_ptr + (address & 0x003fff));
			else
			{   /* read serial number from ROM */
				/* this has to be be the least efficient way to read a ROM :-) */
				/* this emulation is not guaranteed accurate */

				/* problem : due to collisions with video, timings of the LISA CPU
				are slightly different from timings of a bare 68k */
				/* so we use a kludge... */
				int time_in_frame = machine().first_screen()->vpos();

				/* the BOOT ROM only reads 56 bits, so there must be some wrap-around for
				videoROM_address <= 56 */
				/* pixel clock 20MHz, memory access rate 1.25MHz, horizontal clock 22.7kHz
				according to Apple, which must stand for 1.25MHz/55 = 22727kHz, vertical
				clock approximately 60Hz, which means there are about 380 lines, including VBlank */
				/* The values are different on the Mac XL, and I don't know the correct values
				for sure. */

				/* Something appears to be wrong with the timings, since we expect to read the
				2nd half when v-syncing, i.e. for lines beyond the 431th or 364th one (provided
				there are no additionnal margins).
				This is caused by the fact that 68k timings are wrong (memory accesses are
				interlaced with the video hardware, which is not emulated). */
				if (m_features.has_mac_xl_video)
				{
					if ((time_in_frame >= 374) && (time_in_frame <= 392))   /* these values have not been tested */
						answer = m_videoROM_ptr[m_videoROM_address|0x80] << 8;
					else
						answer = m_videoROM_ptr[m_videoROM_address] << 8;
				}
				else
				{
					if ((time_in_frame >= 364) && (time_in_frame <= 375))
					{
						answer = m_videoROM_ptr[m_videoROM_address|0x80] << 8;
				logerror("reading1 %06X=%04x %s time=%d\n", address, answer, machine().describe_context(), time_in_frame);
					}
					else
					{
						answer = m_videoROM_ptr[m_videoROM_address] << 8;
				logerror("reading2 %06X=%04x %s time=%d\n", address, answer, machine().describe_context(), time_in_frame);
					}
				}


				m_videoROM_address = (m_videoROM_address + 1) & 0x7f;
				if (m_videoROM_address == ((m_features.has_mac_xl_video) ? 48 : 56)) {
					logerror("loop %d\n", m_videoROM_address);
					m_videoROM_address = 0;
				}

			}

			break;
		}
	}

	/*logerror("result %X\n", answer);*/

	return answer;
}

WRITE16_MEMBER(lisa_state::lisa_w)
{
	/* segment register set */
	int the_seg = m_seg;

	/* upper 7 bits -> segment # */
	int segment = (offset >> 16) & 0x7f;


	if (m_setup)
	{
		if (offset & 0x002000)
		{
			the_seg = 0;        // TRUSTED by Lisa Hardware Manual section 2.3.3 and MMU startup test
		}
		else
		{
			if (offset & 0x004000)
			{   /* write to MMU register */
				logerror("write to segment registers (%X:%X) ", the_seg, segment);
				if (offset & 0x000004)
				{   /* sorg register */
					logerror("sorg, data = %X\n", data);
					m_real_mmu_regs[the_seg][segment].sorg = data & 0xFFF;
					m_mmu_regs[the_seg][segment].sorg = (data & 0x0fff) << 9;
				}
				else
				{   /* slim register */
					logerror("slim, data = %X\n", data);
					m_real_mmu_regs[the_seg][segment].slim = data & 0xFFF;
					m_mmu_regs[the_seg][segment].slim = (~ (data << 9)) & 0x01ffff;
					switch ((data & 0x0f00) >> 8)
					{
					case 0x4:
						/*logerror("type : RAM stack r\n");*/
						m_mmu_regs[the_seg][segment].type = RAM_stack_r;
						break;
					case 0x5:
						/*logerror("type : RAM r\n");*/
						m_mmu_regs[the_seg][segment].type = RAM_r;
						break;
					case 0x6:
						/*logerror("type : RAM stack rw\n");*/
						m_mmu_regs[the_seg][segment].type = RAM_stack_rw;
						break;
					case 0x7:
						/*logerror("type : RAM rw\n");*/
						m_mmu_regs[the_seg][segment].type = RAM_rw;
						break;
					case 0x8:
					case 0x9:   /* not documented, but used by ROMs (?) */
						/*logerror("type : I/O\n");*/
						m_mmu_regs[the_seg][segment].type = IO;
						break;
					case 0xC:
						/*logerror("type : invalid\n");*/
						m_mmu_regs[the_seg][segment].type = invalid;
						break;
					case 0xF:
						logerror("type : special I/O\n");
						m_mmu_regs[the_seg][segment].type = special_IO;
						break;
					default:    /* "unpredictable results" */
						logerror("type : unknown\n");
						m_mmu_regs[the_seg][segment].type = invalid;
						break;
					}
				}
			}
			else
			{   /* system ROMs : read-only ??? */
				/* bus error ??? */
			}
			return;
		}
	}

	if (m_maincpu->state_int(M68K_SR) & 0x2000)
		/* supervisor mode -> force register file 0 */
		the_seg = 0;

	{
		/* offset in segment */
		int seg_offset = (offset & 0x00ffff) << 1;

		/* add relevant origin -> address */
		offs_t address = (m_mmu_regs[the_seg][segment].sorg + seg_offset) & 0x1fffff;

		switch (m_mmu_regs[the_seg][segment].type)
		{
		case RAM_stack_rw:
			if (address <= m_mmu_regs[the_seg][segment].slim)
			{
				/* out of segment limits : bus error */

			}
			COMBINE_DATA((UINT16 *) (m_ram_ptr + address));
			if (m_diag2)
			{
				if ((ACCESSING_BITS_0_7)
					&& ! (m_bad_parity_table[address >> 3] & (0x1 << (address & 0x7))))
				{
					m_bad_parity_table[address >> 3] |= 0x1 << (address & 0x7);
					m_bad_parity_count++;
				}
				if ((ACCESSING_BITS_8_15)
					&& ! (m_bad_parity_table[address >> 3] & (0x2 << (address & 0x7))))
				{
					m_bad_parity_table[address >> 3] |= 0x2 << (address & 0x7);
					m_bad_parity_count++;
				}
			}
			else if (m_bad_parity_table[address >> 3] & (0x3 << (address & 0x7)))
			{
				if ((ACCESSING_BITS_0_7)
					&& (m_bad_parity_table[address >> 3] & (0x1 << (address & 0x7))))
				{
					m_bad_parity_table[address >> 3] &= ~ (0x1 << (address & 0x7));
					m_bad_parity_count--;
				}
				if ((ACCESSING_BITS_8_15)
					&& (m_bad_parity_table[address >> 3] & (0x2 << (address & 0x7))))
				{
					m_bad_parity_table[address >> 3] &= ~ (0x2 << (address & 0x7));
					m_bad_parity_count--;
				}
			}
			break;

		case RAM_rw:
			if (address > m_mmu_regs[the_seg][segment].slim)
			{
				/* out of segment limits : bus error */

			}
			COMBINE_DATA((UINT16 *) (m_ram_ptr + address));
			if (m_diag2)
			{
				if ((ACCESSING_BITS_0_7)
					&& ! (m_bad_parity_table[address >> 3] & (0x1 << (address & 0x7))))
				{
					m_bad_parity_table[address >> 3] |= 0x1 << (address & 0x7);
					m_bad_parity_count++;
				}
				if ((ACCESSING_BITS_8_15)
					&& ! (m_bad_parity_table[address >> 3] & (0x2 << (address & 0x7))))
				{
					m_bad_parity_table[address >> 3] |= 0x2 << (address & 0x7);
					m_bad_parity_count++;
				}
			}
			else if (m_bad_parity_table[address >> 3] & (0x3 << (address & 0x7)))
			{
				if ((ACCESSING_BITS_0_7)
					&& (m_bad_parity_table[address >> 3] & (0x1 << (address & 0x7))))
				{
					m_bad_parity_table[address >> 3] &= ~ (0x1 << (address & 0x7));
					m_bad_parity_count--;
				}
				if ((ACCESSING_BITS_8_15)
					&& (m_bad_parity_table[address >> 3] & (0x2 << (address & 0x7))))
				{
					m_bad_parity_table[address >> 3] &= ~ (0x2 << (address & 0x7));
					m_bad_parity_count--;
				}
			}
			break;

		case IO:
			lisa_IO_w(space, (address & 0x00ffff) >> 1, data, mem_mask);
			break;

		case RAM_stack_r:   /* read-only */
		case RAM_r:         /* read-only */
		case special_IO:    /* system ROMs : read-only ??? */
		case invalid:       /* unmapped segment */
			/* bus error */

			break;
		}
	}
}


/**************************************************************************************\
* I/O Slot Memory                                                                      *
*                                                                                      *
* 000000 - 001FFF Slot 0 Low  Decode                                                   *
* 002000 - 003FFF Slot 0 High Decode                                                   *
* 004000 - 005FFF Slot 1 Low  Decode                                                   *
* 006000 - 007FFF Slot 1 High Decode                                                   *
* 008000 - 009FFF Slot 2 Low  Decode                                                   *
* 00A000 - 00BFFF Slot 2 High Decode                                                   *
* 00C000 - 00CFFF Floppy Disk Controller shared RAM                                    *
*   00c001-00c7ff floppy disk control                                                  *
* 00D000 - 00DFFF I/O Board Devices                                                    *
*   00d000-00d3ff serial ports control                                                 *
*   00d800-00dbff paralel port                                                         *
*   00dc00-00dfff keyboard/mouse cops via                                              *
* 00E000 - 00FFFF CPU Board Devices                                                    *
*   00e000-00e01e cpu board control                                                    *
*   00e01f-00e7ff unused                                                               *
*   00e8xx-video address latch                                                         *
*   00f0xx memory error address latch                                                  *
*   00f8xx status register                                                             *
*                                                                                      *
\**************************************************************************************/

void lisa_state::cpu_board_control_access(offs_t offset)
{
	switch ((offset & 0x03ff) << 1)
	{
	case 0x0002:    /* Set DIAG1 Latch */
	case 0x0000:    /* Reset DIAG1 Latch */
		break;
	case 0x0006:    /* Set Diag2 Latch */
		m_diag2 = 1;
		break;
	case 0x0004:    /* ReSet Diag2 Latch */
		m_diag2 = 0;
		break;
	case 0x000A:    /* SEG1 Context Selection bit SET */
		/*logerror("seg bit 0 set\n");*/
		m_seg |= 1;
		break;
	case 0x0008:    /* SEG1 Context Selection bit RESET */
		/*logerror("seg bit 0 clear\n");*/
		m_seg &= ~1;
		break;
	case 0x000E:    /* SEG2 Context Selection bit SET */
		/*logerror("seg bit 1 set\n");*/
		m_seg |= 2;
		break;
	case 0x000C:    /* SEG2 Context Selection bit RESET */
		/*logerror("seg bit 1 clear\n");*/
		m_seg &= ~2;
		break;
	case 0x0010:    /* SETUP register SET */
		logerror("setup SET %s\n", machine().describe_context());
		m_setup = 1;
		break;
	case 0x0012:    /* SETUP register RESET */
		logerror("setup UNSET %s\n", machine().describe_context());
		m_setup = 0;
		break;
	case 0x001A:    /* Enable Vertical Retrace Interrupt */
		logerror("enable retrace %s\n", machine().describe_context());
		m_VTMSK = 1;
		break;
	case 0x0018:    /* Disable Vertical Retrace Interrupt */
		logerror("disable retrace %s\n", machine().describe_context());
		m_VTMSK = 0;
		set_VTIR(2);
		break;
	case 0x0016:    /* Enable Soft Error Detect. */
	case 0x0014:    /* Disable Soft Error Detect. */
		break;
	case 0x001E:    /* Enable Hard Error Detect */
		m_test_parity = 1;
		break;
	case 0x001C:    /* Disable Hard Error Detect */
		m_test_parity = 0;
		set_parity_error_pending(0);
		break;
	}
}

READ16_MEMBER(lisa_state::lisa_IO_r)
{
	int answer=0;

	switch ((offset & 0x7000) >> 12)
	{
	case 0x0:
		/* Slot 0 Low */
		break;

	case 0x1:
		/* Slot 0 High */
		break;

	case 0x2:
		/* Slot 1 Low */
		break;

	case 0x3:
		/* Slot 1 High */
		break;

	case 0x4:
		/* Slot 2 Low */
		break;

	case 0x5:
		/* Slot 2 High */
		break;

	case 0x6:
		if (! (offset & 0x800))
		{
			if (! (offset & 0x400))
			{
				/*if (ACCESSING_BITS_0_7)*/ /* Geez, who cares ? */
					answer = m_fdc_ram[offset & 0x03ff] & 0xff; /* right ??? */
			}
		}
		else
		{
			/* I/O Board Devices */
			switch ((offset & 0x0600) >> 9)
			{
			case 0: /* serial ports control */
				answer = m_scc->reg_r(space, offset&7);
				break;

			case 2: /* parallel port */
				/* 1 VIA located at 0xD901 */
				if (ACCESSING_BITS_0_7)
					answer = m_via1->read(space, (offset >> 2) & 0xf);
				break;

			case 3: /* keyboard/mouse cops via */
				/* 1 VIA located at 0xDD81 */
				if (ACCESSING_BITS_0_7)
					answer = m_via0->read(space, offset & 0xf);
				break;
			}
		}
		break;

	case 0x7:
		/* CPU Board Devices */
		switch ((offset & 0x0C00) >> 10)
		{
		case 0x0:   /* cpu board control */
			cpu_board_control_access(offset & 0x03ff);
			break;

		case 0x1:   /* Video Address Latch */
			answer = m_video_address_latch;
			break;

		case 0x2:   /* Memory Error Address Latch */
			answer = m_mem_err_addr_latch;
			break;

		case 0x3:   /* Status Register */
			answer = 0;
			if (! m_parity_error_pending)
				answer |= 0x02;
			if (m_VTIR<=1)
// GFE : needs to be in phase with Serial NUM
			{
				int time_in_frame = machine().first_screen()->vpos();
				if (m_features.has_mac_xl_video)
				{
					if ((time_in_frame >= 374) && (time_in_frame <= 392))   /* these values have not been tested */
					{   /* if VSyncing, read ROM 2nd half ? */
					}
					else
					{
						m_VTIR=0;
						answer |= 0x04;
					}
				}
				else
				{
					logerror("read status time=%x\n", time_in_frame);
					if ((time_in_frame >= 364) && (time_in_frame <= 383))   /* these values are approximative */
					{   /* if VSyncing, read ROM 2nd half ? */
					}
					else
					{
						m_VTIR=0;
						answer |= 0x04;
					}
				}

			}
			else
						answer |= 0x04;
			/* huh... we need to emulate some other bits */
			logerror("read status %s val=%x\n", machine().describe_context(), answer);

			break;
		}
		break;
	}

	return answer;
}

WRITE16_MEMBER(lisa_state::lisa_IO_w)
{
	switch ((offset & 0x7000) >> 12)
	{
	case 0x0:
		/* Slot 0 Low */
		break;

	case 0x1:
		/* Slot 0 High */
		break;

	case 0x2:
		/* Slot 1 Low */
		break;

	case 0x3:
		/* Slot 1 High */
		break;

	case 0x4:
		/* Slot 2 Low */
		break;

	case 0x5:
		/* Slot 2 High */
		break;

	case 0x6:
		if (! (offset & 0x0800))
		{
			/* Floppy Disk Controller shared RAM */
			if (! (offset & 0x0400))
			{
				if (ACCESSING_BITS_0_7)
					m_fdc_ram[offset & 0x03ff] = data & 0xff;
			}
		}
		else
		{
			/* I/O Board Devices */
			switch ((offset & 0x0600) >> 9)
			{
			case 0: /* serial ports control */
				m_scc->reg_w(space, offset&7, data);
				break;

			case 2: /* paralel port */
				if (ACCESSING_BITS_0_7)
					m_via1->write(space, (offset >> 2) & 0xf, data & 0xff);
				break;

			case 3: /* keyboard/mouse cops via */
				if (ACCESSING_BITS_0_7)
					m_via0->write(space, offset & 0xf, data & 0xff);
				break;
			}
		}
		break;

	case 0x7:
		/* CPU Board Devices */
		switch ((offset & 0x0C00) >> 10)
		{
		case 0x0:   /* cpu board control */
			cpu_board_control_access(offset & 0x03ff);
			break;

		case 0x1:   /* Video Address Latch */
			/*logerror("video address latch write offs=%X, data=%X\n", offset, data);*/
			COMBINE_DATA(& m_video_address_latch);
			m_videoram_ptr = ((UINT16 *)m_ram_ptr) + ((m_video_address_latch << 6) & 0xfc000);
			/*logerror("video address latch %X -> base address %X\n", m_video_address_latch,
			                (m_video_address_latch << 7) & 0x1f8000);*/
			break;
		}
		break;
	}
}
