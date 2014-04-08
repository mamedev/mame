/*********************************************************************

    machine/oric.c

    Paul Cook
    Kev Thacker

    Thankyou to Fabrice Frances for his ORIC documentation which helped with this driver
    http://oric.ifrance.com/oric/

    TODO:
    - there are problems loading some .wav's. Try to fix these.
    - fix more graphics display problems
    - check the printer works
    - fix more disc drive/wd179x problems so more software will run

*********************************************************************/


#include "includes/oric.h"




/* ==0 if oric1 or oric atmos, !=0 if telestrat */

/* This does not exist in the real hardware. I have used it to
know which sources are interrupting */
/* bit 2 = telestrat 2nd via interrupt,
1 = microdisc interface,
0 = oric 1st via interrupt */

enum
{
	ORIC_FLOPPY_NONE,
	ORIC_FLOPPY_MFM_DISK,
	ORIC_FLOPPY_BASIC_DISK
};

/* type of disc interface connected to oric/oric atmos */
/* telestrat always has a microdisc interface */
enum
{
	ORIC_FLOPPY_INTERFACE_NONE = 0,
	ORIC_FLOPPY_INTERFACE_MICRODISC = 1,
	ORIC_FLOPPY_INTERFACE_JASMIN = 2,
	ORIC_FLOPPY_INTERFACE_APPLE2 = 3,
	ORIC_FLOPPY_INTERFACE_APPLE2_V2 = 4
};

/* called when ints are changed - cleared/set */
void oric_state::oric_refresh_ints()
{
	/* telestrat has floppy hardware built-in! */
	if (m_is_telestrat==0)
	{
		/* oric 1 or oric atmos */

		/* if floppy disc hardware is disabled, do not allow interrupts from it */
		if ((m_io_floppy->manager().safe_to_read()) && ((m_io_floppy->read() & 0x07) == ORIC_FLOPPY_INTERFACE_NONE))
		{
			m_irqs &=~(1<<1);
		}
	}

	/* any irq set? */
	if (m_irqs & 0x0f)
	{
		m_maincpu->set_input_line(0, HOLD_LINE);
	}
	else
	{
		m_maincpu->set_input_line(0, CLEAR_LINE);
	}
}



/* index of keyboard line to scan */
/* sense result */
/* mask to read keys */





/* refresh keyboard sense */
void oric_state::oric_keyboard_sense_refresh()
{
	/* The following assumes that if a 0 is written, it can be used to detect if any key has been pressed.. */
	/* for each bit that is 0, it combines it's pressed state with the pressed state so far */

	int i;
	unsigned char key_bit = 0;

	/* what if data is 0, can it sense if any of the keys on a line are pressed? */
	int input_port_data = 0;

	switch ( m_keyboard_line )
	{
		case 0: input_port_data = m_io_row0->read(); break;
		case 1: input_port_data = m_io_row1->read(); break;
		case 2: input_port_data = m_io_row2->read(); break;
		case 3: input_port_data = m_io_row3->read(); break;
		case 4: input_port_data = m_io_row4->read(); break;
		case 5: input_port_data = m_io_row5->read(); break;
		case 6: input_port_data = m_io_row6->read(); break;
		case 7: input_port_data = m_io_row7->read(); break;
	}

	/* go through all bits in line */
	for (i=0; i<8; i++)
	{
		/* sense this bit? */
		if (((~m_keyboard_mask) & (1<<i)) != 0)
		{
			/* is key pressed? */
			if (input_port_data & (1<<i))
			{
				/* yes */
				key_bit |= 1;
			}
		}
	}

	/* clear sense result */
	m_key_sense_bit = 0;

	/* any keys pressed on this line? */
	if (key_bit!=0)
	{
		/* set sense result */
		m_key_sense_bit = (1<<3);
	}
}


/* this is executed when a write to psg port a is done */
WRITE8_MEMBER(oric_state::oric_psg_porta_write)
{
	m_keyboard_mask = data;
}


/* PSG control pins */
/* bit 1 = BDIR state */
/* bit 0 = BC1 state */

/* this port is also used to read printer data */
READ8_MEMBER(oric_state::oric_via_in_a_func)
{
	/*logerror("port a read\r\n"); */

	/* access psg? */
	if (m_psg_control!=0)
	{
		/* if psg is in read register state return reg data */
		if (m_psg_control==0x01)
		{
			return m_ay8912->data_r(space, 0);
		}

		/* return high-impedance */
		return 0x0ff;
	}

	/* correct?? */
	return m_via_port_a_data;
}

READ8_MEMBER(oric_state::oric_via_in_b_func)
{
	int data;

	oric_keyboard_sense_refresh();

	data = m_key_sense_bit;
	data |= m_keyboard_line & 0x07;

	return data;
}


/* read/write data depending on state of bdir, bc1 pins and data output to psg */
void oric_state::oric_psg_connection_refresh(address_space &space)
{
	if (m_psg_control!=0)
	{
		switch (m_psg_control)
		{
			/* PSG inactive */
			case 0:
				break;

			/* read register data */
			case 1:
				//m_via_port_a_data = ay8910_read_port_0_r(space, 0);
				break;

			/* write register data */
			case 2:
				m_ay8912->data_w(space, 0, m_via_port_a_data);
				break;

			/* write register index */
			case 3:
				m_ay8912->address_w(space, 0, m_via_port_a_data);
				break;

			default:
				break;
		}

		return;
	}
}

WRITE8_MEMBER(oric_state::oric_via_out_a_func)
{
	m_via_port_a_data = data;

	oric_psg_connection_refresh(space);

	if (m_psg_control==0)
	{
		/* if psg not selected, write to printer */
		m_cent_data_out->write(space, 0, data);
	}
}

/*
PB0..PB2
 keyboard lines-demultiplexer line 7

PB3
 keyboard sense line 0

PB4
 printer strobe line 1

PB5
 (not connected) ?? 1

PB6
 tape connector motor control 0

PB7
 tape connector output high 1

 */


/* not called yet - this will update the via with the state of the tape data.
This allows the via to trigger on bit changes and issue interrupts */
TIMER_CALLBACK_MEMBER(oric_state::oric_refresh_tape)
{
	int data;
	int input_port_9;

	data = 0;

	if (m_cassette->input() > 0.0038)
	{
		data |= 1;
	}

	/* "A simple cable to catch the vertical retrace signal !
	    This cable connects the video output for the television/monitor
	to the via cb1 input. Interrupts can be generated from the vertical
	sync, and flicker free games can be produced */

	input_port_9 = m_io_floppy->read();
	/* cable is enabled? */
	if ((input_port_9 & 0x08)!=0)
	{
		/* return state of vsync */
		data = input_port_9>>4;
	}

	m_via6522_0->write_cb1(data);
}

WRITE8_MEMBER(oric_state::oric_via_out_b_func)
{
	/* KEYBOARD */
	m_keyboard_line = data & 0x07;

	/* CASSETTE */
	/* cassette motor control */
	m_cassette->change_state(
		(data & 0x40) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED,
		CASSETTE_MOTOR_DISABLED);

	/* cassette data out */
	m_cassette->output((data & (1<<7)) ? -1.0 : +1.0);

	/* centronics STROBE is connected to PB4 */
	m_centronics->write_strobe(BIT(data, 4));

	oric_psg_connection_refresh(space);
	m_previous_portb_data = data;
}


WRITE_LINE_MEMBER(oric_state::oric_via_out_ca2_func)
{
	if (state)
		m_psg_control |=1;
	else
		m_psg_control &=~1;

	oric_psg_connection_refresh(generic_space());
}

WRITE_LINE_MEMBER(oric_state::oric_via_out_cb2_func)
{
	if (state)
		m_psg_control |=2;
	else
		m_psg_control &=~2;

	oric_psg_connection_refresh(generic_space());
}


WRITE_LINE_MEMBER(oric_state::oric_via_irq_func)
{
	m_irqs &= ~(1<<0);

	if (state)
	{
		m_irqs |=(1<<0);
	}

	oric_refresh_ints();
}


/*
VIA Lines
 Oric usage

PA0..PA7
 PSG data bus, printer data lines

CA1
 printer acknowledge line

CA2
 PSG BC1 line

PB0..PB2
 keyboard lines-demultiplexer

PB3
 keyboard sense line

PB4
 printer strobe line

PB5
 (not connected)

PB6
 tape connector motor control

PB7
 tape connector output

CB1
 tape connector input

CB2
 PSG BDIR line

*/




/*********************/
/* APPLE 2 INTERFACE */

/*
apple2 disc drive accessed through 0x0310-0x031f (read/write)
oric via accessed through 0x0300-0x030f. (read/write)
disk interface rom accessed through 0x0320-0x03ff (read only)

CALL &320 to start, or use BOBY rom.
*/

void oric_state::oric_install_apple2_interface()
{
	applefdc_base_device *fdc = machine().device<applefdc_base_device>("fdc");
	address_space &space = m_maincpu->space(AS_PROGRAM);

	if (m_is_telestrat)
	{
		return;
	}

	space.install_read_handler(0x0300, 0x030f, read8_delegate(FUNC(oric_state::oric_IO_r), this));
	space.install_read_handler(0x0310, 0x031f, read8_delegate(FUNC(applefdc_base_device::read), fdc));
	space.install_read_bank(0x0320, 0x03ff, "bank4");
	m_bank4 = membank("bank4");

	space.install_write_handler(0x0300, 0x030f, write8_delegate(FUNC(oric_state::oric_IO_w), this));
	space.install_write_handler(0x0310, 0x031f, write8_delegate(FUNC(applefdc_base_device::write), fdc));
	m_bank4->set_base(  m_region_maincpu->base() + 0x014000 + 0x020);
}


void oric_state::oric_enable_memory(int low, int high, int rd, int wr)
{
	int i;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	if (m_is_telestrat)
	{
		return;
	}

	for (i = low; i <= high; i++)
	{
		switch(i) {
		case 1:
			if (rd) {
				space.install_read_bank(0xc000, 0xdfff, "bank1");
			} else {
				space.nop_read(0xc000, 0xdfff);
			}
			if (wr) {
				space.install_write_bank(0xc000, 0xdfff, "bank5");
			} else {
				space.unmap_write(0xc000, 0xdfff);
			}
			break;
		case 2:
			if (rd) {
				space.install_read_bank(0xe000, 0xf7ff, "bank2");
			} else {
				space.nop_read(0xe000, 0xf7ff);
			}
			if (wr) {
				space.install_write_bank(0xe000, 0xf7ff, "bank6");
			} else {
				space.unmap_write(0xe000, 0xf7ff);
			}
			break;
		case 3:
			if (rd) {
				space.install_read_bank(0xf800, 0xffff, "bank3");
			} else {
				space.nop_read(0xf800, 0xffff);
			}
			break;
		}
	}
}



/************************/
/* APPLE 2 INTERFACE V2 */

/*
apple2 disc drive accessed through 0x0310-0x031f (read/write)
oric via accessed through 0x0300-0x030f. (read/write)
disk interface rom accessed through 0x0320-0x03ff (read only)
v2 registers accessed through 0x0380-0x0383 (write only)

CALL &320 to start, or use BOBY rom.
*/

WRITE8_MEMBER(oric_state::apple2_v2_interface_w)
{
	/* data is ignored, address is used to decode operation */
	if (m_is_telestrat)
		return;

/*  logerror("apple 2 interface v2 rom page: %01x\n",(offset & 0x02)>>1); */

	/* bit 0 is 0 for page 0, 1 for page 1 */
	m_bank4->set_base(m_region_maincpu->base() + 0x014000 + 0x0100 + (((offset & 0x02)>>1)<<8));

	oric_enable_memory(1, 3, TRUE, TRUE);

	/* bit 1 is 0, rom enabled, bit 1 is 1 ram enabled */
	if ((offset & 0x01)==0)
	{
		unsigned char *rom_ptr;

		/* logerror("apple 2 interface v2: rom enabled\n"); */

		/* enable rom */
		rom_ptr = m_region_maincpu->base() + 0x010000;
		m_bank1->set_base(rom_ptr);
		m_bank2->set_base(rom_ptr+0x02000);
		m_bank3->set_base(rom_ptr+0x03800);
		m_bank5->set_base(m_ram_0x0c000);
		m_bank6->set_base(m_ram_0x0c000+0x02000);
		m_bank7->set_base(m_ram_0x0c000+0x03800);
	}
	else
	{
		/*logerror("apple 2 interface v2: ram enabled\n"); */

		/* enable ram */
		m_bank1->set_base(m_ram_0x0c000);
		m_bank2->set_base(m_ram_0x0c000+0x02000);
		m_bank3->set_base(m_ram_0x0c000+0x03800);
		m_bank5->set_base(m_ram_0x0c000);
		m_bank6->set_base(m_ram_0x0c000+0x02000);
		m_bank7->set_base(m_ram_0x0c000+0x03800);
	}
}


/* APPLE 2 INTERFACE V2 */
void oric_state::oric_install_apple2_v2_interface()
{
	applefdc_base_device *fdc = machine().device<applefdc_base_device>("fdc");
	address_space &space = m_maincpu->space(AS_PROGRAM);

	space.install_read_handler(0x0300, 0x030f, read8_delegate(FUNC(oric_state::oric_IO_r), this));
	space.install_read_handler(0x0310, 0x031f, read8_delegate(FUNC(applefdc_base_device::read), fdc));
	space.install_read_bank(0x0320, 0x03ff, "bank4");
	m_bank4 = membank("bank4");

	space.install_write_handler(0x0300, 0x030f, write8_delegate(FUNC(oric_state::oric_IO_w), this));
	space.install_write_handler(0x0310, 0x031f, write8_delegate(FUNC(applefdc_base_device::write), fdc));
	space.install_write_handler(0x0380, 0x0383, write8_delegate(FUNC(oric_state::apple2_v2_interface_w),this));

	apple2_v2_interface_w(space, 0, 0);
}

/********************/
/* JASMIN INTERFACE */


/* bit 0: overlay ram access (1 means overlay ram enabled) */

/* bit 0: ROMDIS (1 means internal Basic rom disabled) */


void oric_state::oric_jasmin_set_mem_0x0c000()
{
	/* assumption:
	1. It is possible to access all 16k overlay ram.
	2. If os is enabled, and overlay ram is enabled, all 16k can be accessed.
	3. if os is disabled, and overlay ram is enabled, jasmin rom takes priority.
	*/
	if (m_is_telestrat)
	{
		return;
	}

	/* the ram is disabled in the jasmin rom which indicates that jasmin takes
	priority over the ram */

	/* basic rom disabled? */
	if ((m_port_3fb_w & 0x01)==0)
	{
		/* no, it is enabled! */

		/* overlay ram enabled? */
		if ((m_port_3fa_w & 0x01)==0)
		{
			unsigned char *rom_ptr;

			/* no it is disabled */
			/*logerror("&c000-&ffff is os rom\n"); */

			oric_enable_memory(1, 3, TRUE, FALSE);

			rom_ptr = m_region_maincpu->base() + 0x010000;
			m_bank1->set_base(rom_ptr);
			m_bank2->set_base(rom_ptr+0x02000);
			m_bank3->set_base(rom_ptr+0x03800);
		}
		else
		{
			/*logerror("&c000-&ffff is ram\n"); */

			oric_enable_memory(1, 3, TRUE, TRUE);

			m_bank1->set_base(m_ram_0x0c000);
			m_bank2->set_base(m_ram_0x0c000+0x02000);
			m_bank3->set_base(m_ram_0x0c000+0x03800);
			m_bank5->set_base(m_ram_0x0c000);
			m_bank6->set_base(m_ram_0x0c000+0x02000);
			m_bank7->set_base(m_ram_0x0c000+0x03800);
		}
	}
	else
	{
		/* yes, basic rom is disabled */

		if ((m_port_3fa_w & 0x01)==0)
		{
			/* overlay ram disabled */

			/*logerror("&c000-&f8ff is nothing!\n"); */
			oric_enable_memory(1, 2, FALSE, FALSE);
		}
		else
		{
			/*logerror("&c000-&f8ff is ram!\n"); */
			oric_enable_memory(1, 2, TRUE, TRUE);

			m_bank1->set_base(m_ram_0x0c000);
			m_bank2->set_base(m_ram_0x0c000+0x02000);
			m_bank5->set_base(m_ram_0x0c000);
			m_bank6->set_base(m_ram_0x0c000+0x02000);
		}

		{
			/* basic rom disabled */
			unsigned char *rom_ptr;

			/*logerror("&f800-&ffff is jasmin rom\n"); */
			/* jasmin rom enabled */
			oric_enable_memory(3, 3, TRUE, TRUE);
			rom_ptr = m_region_maincpu->base() + 0x010000+0x04000+0x02000;
			m_bank3->set_base(rom_ptr);
			m_bank7->set_base(rom_ptr);
		}
	}
}

/* DRQ is connected to interrupt */
WRITE_LINE_MEMBER(oric_state::oric_jasmin_wd179x_drq_w)
{
	if (state)
		m_irqs |= (1<<1);
	else
		m_irqs &=~(1<<1);

	oric_refresh_ints();
}

READ8_MEMBER(oric_state::oric_jasmin_r)
{
	wd1770_device *fdc = machine().device<wd1770_device>("wd179x");
	unsigned char data = 0x0ff;

	switch (offset & 0x0f)
	{
		/* jasmin floppy disc interface */
		case 0x04:
			data = fdc->status_r(space, 0);
			break;
		case 0x05:
			data = fdc->track_r(space, 0);
			break;
		case 0x06:
			data = fdc->sector_r(space, 0);
			break;
		case 0x07:
			data = fdc->data_r(space, 0);
			break;
		default:
			data = m_via6522_0->read(space,offset & 0x0f);
			//logerror("unhandled io read: %04x %02x\n", offset, data);
			break;

	}

	return data;
}

WRITE8_MEMBER(oric_state::oric_jasmin_w)
{
	wd1770_device *fdc = machine().device<wd1770_device>("wd179x");
	switch (offset & 0x0f)
	{
		/* microdisc floppy disc interface */
		case 0x04:
			fdc->command_w( space, 0, data);
			break;
		case 0x05:
			fdc->track_w(space, 0, data);
			break;
		case 0x06:
			fdc->sector_w(space, 0, data);
			break;
		case 0x07:
			fdc->data_w(space, 0, data);
			break;
		/* bit 0 = side */
		case 0x08:
			fdc->set_side(data & 0x01);
			break;
		/* any write will cause wd179x to reset */
		case 0x09:
			fdc->reset();
			break;
		case 0x0a:
			//logerror("jasmin overlay ram w: %02x PC: %04x\n", data, m_maincpu->pc());
			m_port_3fa_w = data;
			oric_jasmin_set_mem_0x0c000();
			break;
		case 0x0b:
			//logerror("jasmin romdis w: %02x PC: %04x\n", data, m_maincpu->pc());
			m_port_3fb_w = data;
			oric_jasmin_set_mem_0x0c000();
			break;
		/* bit 0,1 of addr is the drive */
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
			fdc->set_drive(offset & 0x03);
			break;

		default:
			m_via6522_0->write(space,offset & 0x0f, data);
			break;
	}
}


void oric_state::oric_install_jasmin_interface()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	/* romdis */
	m_port_3fb_w = 1;
	oric_jasmin_set_mem_0x0c000();

	space.install_read_handler(0x0300, 0x03ef, read8_delegate(FUNC(oric_state::oric_IO_r),this));
	space.install_read_handler(0x03f0, 0x03ff, read8_delegate(FUNC(oric_state::oric_jasmin_r),this));

	space.install_write_handler(0x0300, 0x03ef, write8_delegate(FUNC(oric_state::oric_IO_w),this));
	space.install_write_handler(0x03f0, 0x03ff, write8_delegate(FUNC(oric_state::oric_jasmin_w),this));
}

/*********************************/
/* MICRODISC INTERFACE variables */

/* used by Microdisc interfaces */

/* bit 7 is intrq state */
/* bit 7 is drq state (active low) */
/* bit 6,5: drive */
/* bit 4: side */
/* bit 3: double density enable */
/* bit 0: enable FDC IRQ to trigger IRQ on CPU */


void oric_state::oric_microdisc_refresh_wd179x_ints()
{
	m_irqs &=~(1<<1);

	if ((m_wd179x_int_state) && (m_port_314_w & (1<<0)))
	{
		/*logerror("oric microdisc interrupt\n"); */

		m_irqs |=(1<<1);
	}

	oric_refresh_ints();
}

WRITE_LINE_MEMBER(oric_state::oric_microdisc_wd179x_intrq_w)
{
	m_wd179x_int_state = state;

	if (state)
		m_port_314_r &= ~(1<<7);
	else
		m_port_314_r |=(1<<7);

	oric_microdisc_refresh_wd179x_ints();
}

WRITE_LINE_MEMBER(oric_state::oric_microdisc_wd179x_drq_w)
{
	if (state)
		m_port_318_r &=~(1<<7);
	else
		m_port_318_r |= (1<<7);
}

void oric_state::oric_microdisc_set_mem_0x0c000()
{
	if (m_is_telestrat)
	{
		return;
	}

	/* for 0x0c000-0x0dfff: */
	/* if os disabled, ram takes priority */
	/* /ROMDIS */
	if ((m_port_314_w & (1<<1))==0)
	{
		/*logerror("&c000-&dfff is ram\n"); */
		/* rom disabled enable ram */
		oric_enable_memory(1, 1, TRUE, TRUE);
		m_bank1->set_base(m_ram_0x0c000);
		m_bank5->set_base(m_ram_0x0c000);
	}
	else
	{
		unsigned char *rom_ptr;
		/*logerror("&c000-&dfff is os rom\n"); */
		/* basic rom */
		oric_enable_memory(1, 1, TRUE, FALSE);
		rom_ptr = m_region_maincpu->base() + 0x010000;
		m_bank1->set_base(rom_ptr);
		m_bank5->set_base(rom_ptr);
	}

	/* for 0x0e000-0x0ffff */
	/* if not disabled, os takes priority */
	if ((m_port_314_w & (1<<1))!=0)
	{
		unsigned char *rom_ptr;
		/*logerror("&e000-&ffff is os rom\n"); */
		/* basic rom */
		oric_enable_memory(2, 3, TRUE, FALSE);
		rom_ptr = m_region_maincpu->base() + 0x010000;
		m_bank2->set_base(rom_ptr+0x02000);
		m_bank3->set_base(rom_ptr+0x03800);
		m_bank6->set_base(rom_ptr+0x02000);
		m_bank7->set_base(rom_ptr+0x03800);

	}
	else
	{
		/* if eprom is enabled, it takes priority over ram */
		if ((m_port_314_w & (1<<7))==0)
		{
			unsigned char *rom_ptr;
			/*logerror("&e000-&ffff is disk rom\n"); */
			oric_enable_memory(2, 3, TRUE, FALSE);
			/* enable rom of microdisc interface */
			rom_ptr = m_region_maincpu->base() + 0x014000;
			m_bank2->set_base(rom_ptr);
			m_bank3->set_base(rom_ptr+0x01800);
		}
		else
		{
			/*logerror("&e000-&ffff is ram\n"); */
			/* rom disabled enable ram */
			oric_enable_memory(2, 3, TRUE, TRUE);
			m_bank2->set_base(m_ram_0x0c000+0x02000);
			m_bank3->set_base(m_ram_0x0c000+0x03800);
			m_bank6->set_base(m_ram_0x0c000+0x02000);
			m_bank7->set_base(m_ram_0x0c000+0x03800);
		}
	}
}



READ8_MEMBER(oric_state::oric_microdisc_r)
{
	unsigned char data = 0x0ff;
	wd1770_device *fdc = machine().device<wd1770_device>("wd179x");

	switch (offset & 0x0ff)
	{
		/* microdisc floppy disc interface */
		case 0x00:
			data = fdc->status_r(space, 0);
			break;
		case 0x01:
			data = fdc->track_r(space, 0);
			break;
		case 0x02:
			data = fdc->sector_r(space, 0);
			break;
		case 0x03:
			data = fdc->data_r(space, 0);
			break;
		case 0x04:
			data = m_port_314_r | 0x07f;
/*          logerror("port_314_r: %02x\n",data); */
			break;
		case 0x08:
			data = m_port_318_r | 0x07f;
/*          logerror("port_318_r: %02x\n",data); */
			break;

		default:
			data = m_via6522_0->read(space, offset & 0x0f);
			break;

	}

	return data;
}

WRITE8_MEMBER(oric_state::oric_microdisc_w)
{
	wd1770_device *fdc = machine().device<wd1770_device>("wd179x");
	switch (offset & 0x0ff)
	{
		/* microdisc floppy disc interface */
		case 0x00:
			fdc->command_w(space, 0, data);
			break;
		case 0x01:
			fdc->track_w(space, 0, data);
			break;
		case 0x02:
			fdc->sector_w(space, 0, data);
			break;
		case 0x03:
			fdc->data_w(space, 0, data);
			break;
		case 0x04:
		{
			m_port_314_w = data;

			//logerror("port_314_w: %02x\n",data);

			/* bit 6,5: drive */
			/* bit 4: side */
			/* bit 3: double density enable */
			/* bit 0: enable FDC IRQ to trigger IRQ on CPU */
			fdc->set_drive((data>>5) & 0x03);
			fdc->set_side((data>>4) & 0x01);
			fdc->dden_w(!BIT(data, 3));

			oric_microdisc_set_mem_0x0c000();
			oric_microdisc_refresh_wd179x_ints();
		}
		break;

		default:
			m_via6522_0->write(space, offset & 0x0f, data);
			break;
	}
}

void oric_state::oric_install_microdisc_interface()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	space.install_read_handler(0x0300, 0x030f, read8_delegate(FUNC(oric_state::oric_IO_r),this));
	space.install_read_handler(0x0310, 0x031f, read8_delegate(FUNC(oric_state::oric_microdisc_r),this));
	space.install_read_handler(0x0320, 0x03ff, read8_delegate(FUNC(oric_state::oric_IO_r),this));

	space.install_write_handler(0x0300, 0x030f, write8_delegate(FUNC(oric_state::oric_IO_w),this));
	space.install_write_handler(0x0310, 0x031f, write8_delegate(FUNC(oric_state::oric_microdisc_w),this));
	space.install_write_handler(0x0320, 0x03ff, write8_delegate(FUNC(oric_state::oric_IO_w),this));

	/* disable os rom, enable microdisc rom */
	/* 0x0c000-0x0dfff will be ram, 0x0e000-0x0ffff will be microdisc rom */
	m_port_314_w = 0x0ff^((1<<7) | (1<<1));

	oric_microdisc_set_mem_0x0c000();
}



/*********************************************************/

WRITE_LINE_MEMBER(oric_state::oric_wd179x_intrq_w)
{
	if ((m_io_floppy->read() & 0x07) == ORIC_FLOPPY_INTERFACE_MICRODISC)
	{
		oric_microdisc_wd179x_intrq_w(state);
	}
}

WRITE_LINE_MEMBER(oric_state::oric_wd179x_drq_w)
{
	switch (m_io_floppy->read() &  0x07)
	{
		default:
		case ORIC_FLOPPY_INTERFACE_NONE:
		case ORIC_FLOPPY_INTERFACE_APPLE2:
			return;
		case ORIC_FLOPPY_INTERFACE_MICRODISC:
			oric_microdisc_wd179x_drq_w(state);
			return;
		case ORIC_FLOPPY_INTERFACE_JASMIN:
			oric_jasmin_wd179x_drq_w(state);
			return;
	}
}

const wd17xx_interface oric_wd17xx_interface =
{
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(oric_state,oric_wd179x_intrq_w),
	DEVCB_DRIVER_LINE_MEMBER(oric_state,oric_wd179x_drq_w),
	{FLOPPY_0, FLOPPY_1, FLOPPY_2, FLOPPY_3}
};

void oric_state::oric_common_init_machine()
{
	/* clear all irqs */
	m_irqs = 0;
	m_ram_0x0c000 = NULL;
	m_keyboard_line = 0;
	m_key_sense_bit = 0;
	m_keyboard_mask = 0;
	m_via_port_a_data = 0;
	m_psg_control = 0;
	m_previous_portb_data = 0;
	m_port_3fa_w = 0;
	m_port_3fb_w = 0;
	m_wd179x_int_state = 0;
	m_port_314_r = 0;
	m_port_318_r = 0;
	m_port_314_w = 0;
	machine().scheduler().timer_pulse(attotime::from_hz(4800), timer_expired_delegate(FUNC(oric_state::oric_refresh_tape),this));
}

void oric_state::machine_start()
{
	oric_common_init_machine();

	m_is_telestrat = 0;

	m_ram_0x0c000 = auto_alloc_array(machine(), UINT8, 16384);
}


void oric_state::machine_reset()
{
	int disc_interface_id = m_io_floppy->read() & 0x07;
	address_space &space = m_maincpu->space(AS_PROGRAM);
	if (m_is_telestrat)
		return;

	switch (disc_interface_id)
	{
		default:

		case ORIC_FLOPPY_INTERFACE_APPLE2:
		case ORIC_FLOPPY_INTERFACE_NONE:
		{
			/* setup memory when there is no disc interface */
			unsigned char *rom_ptr;

			/* os rom */
			oric_enable_memory(1, 3, TRUE, FALSE);
			rom_ptr = m_region_maincpu->base() + 0x010000;
			m_bank1->set_base(rom_ptr);
			m_bank2->set_base(rom_ptr+0x02000);
			m_bank3->set_base(rom_ptr+0x03800);
			m_bank5->set_base(rom_ptr);
			m_bank6->set_base(rom_ptr+0x02000);
			m_bank7->set_base(rom_ptr+0x03800);


			if (disc_interface_id==ORIC_FLOPPY_INTERFACE_APPLE2)
			{
				oric_install_apple2_interface();
			}
			else
			{
				space.install_read_handler(0x0300, 0x03ff, read8_delegate(FUNC(oric_state::oric_IO_r),this));
				space.install_write_handler(0x0300, 0x03ff, write8_delegate(FUNC(oric_state::oric_IO_w),this));
			}
		}
		break;

		case ORIC_FLOPPY_INTERFACE_APPLE2_V2:
		{
			oric_install_apple2_v2_interface();
		}
		break;


		case ORIC_FLOPPY_INTERFACE_MICRODISC:
		{
			oric_install_microdisc_interface();
		}
		break;

		case ORIC_FLOPPY_INTERFACE_JASMIN:
		{
			oric_install_jasmin_interface();
		}
		break;
	}
	m_maincpu->reset();
}


READ8_MEMBER(oric_state::oric_IO_r)
{
	switch (m_io_floppy->read() & 0x07)
	{
		default:
		case ORIC_FLOPPY_INTERFACE_NONE:
			break;

		case ORIC_FLOPPY_INTERFACE_MICRODISC:
		{
			if ((offset>=0x010) && (offset<=0x01f))
			{
				return oric_microdisc_r(space, offset);
			}
		}
		break;

		case ORIC_FLOPPY_INTERFACE_JASMIN:
		{
			if ((offset>=0x0f4) && (offset<=0x0ff))
			{
				return oric_jasmin_r(space, offset);
			}
		}
		break;
	}

	/* it is repeated */
	return m_via6522_0->read(space, offset & 0x0f);
}

WRITE8_MEMBER(oric_state::oric_IO_w)
{
	switch (m_io_floppy->read() & 0x07)
	{
		default:
		case ORIC_FLOPPY_INTERFACE_NONE:
			break;

		case ORIC_FLOPPY_INTERFACE_MICRODISC:
		{
			if ((offset >= 0x010) && (offset <= 0x01f))
			{
				oric_microdisc_w(space, offset, data);
				return;
			}
		}
		break;

		case ORIC_FLOPPY_INTERFACE_JASMIN:
		{
			if ((offset >= 0x0f4) && (offset <= 0x0ff))
			{
				oric_jasmin_w(space, offset, data);
				return;
			}

		}
		break;
	}

	m_via6522_0->write(space, offset & 0x0f, data);
}



/**** TELESTRAT ****/

/*
VIA lines
 Telestrat usage

PA0..PA2
 Memory bank selection

PA3
 "Midi" port pin 3

PA4
 RS232/Minitel selection

PA5
 Third mouse button (right joystick port pin 5)

PA6
 "Midi" port pin 5

PA7
 Second mouse button (right joystick port pin 9)

CA1
 "Midi" port pin 1

CA2
 not used ?

PB0..PB4
 Joystick ports

PB5
 Joystick doubler switch

PB6
 Select Left Joystick port

PB7
 Select Right Joystick port

CB1
 Phone Ring detection

CB2
 "Midi" port pin 4

*/


void oric_state::telestrat_refresh_mem()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	telestrat_mem_block *mem_block = &m_telestrat_blocks[m_telestrat_bank_selection];

	switch (mem_block->MemType)
	{
		case TELESTRAT_MEM_BLOCK_RAM:
		{
			m_bank1->set_base(mem_block->ptr);
			m_bank2->set_base(mem_block->ptr);
			space.install_read_bank(0xc000, 0xffff, "bank1");
			space.install_write_bank(0xc000, 0xffff, "bank2");
		}
		break;

		case TELESTRAT_MEM_BLOCK_ROM:
		{
			m_bank1->set_base(mem_block->ptr);
			space.install_read_bank(0xc000, 0xffff, "bank1");
			space.nop_write(0xc000, 0xffff);
		}
		break;

		default:
		case TELESTRAT_MEM_BLOCK_UNDEFINED:
		{
			space.nop_readwrite(0xc000, 0xffff);
		}
		break;
	}
}

READ8_MEMBER(oric_state::telestrat_via2_in_a_func)
{
	//logerror("via 2 - port a %02x\n",m_telestrat_via2_port_a_data);
	return m_telestrat_via2_port_a_data;
}


WRITE8_MEMBER(oric_state::telestrat_via2_out_a_func)
{
	//logerror("via 2 - port a w: %02x\n",data);

	m_telestrat_via2_port_a_data = data;

	if (((data^m_telestrat_bank_selection) & 0x07)!=0)
	{
		m_telestrat_bank_selection = data & 0x07;

		telestrat_refresh_mem();
	}
}

READ8_MEMBER(oric_state::telestrat_via2_in_b_func)
{
	unsigned char data = 0x01f;

	/* left joystick selected? */
	if (m_telestrat_via2_port_b_data & (1<<6))
	{
		data &= ioport("JOY0")->read();
	}

	/* right joystick selected? */
	if (m_telestrat_via2_port_b_data & (1<<7))
	{
		data &= ioport("JOY1")->read();
	}

	data |= m_telestrat_via2_port_b_data & ((1<<7) | (1<<6) | (1<<5));

	return data;
}

WRITE8_MEMBER(oric_state::telestrat_via2_out_b_func)
{
	m_telestrat_via2_port_b_data = data;
}


WRITE_LINE_MEMBER(oric_state::telestrat_via2_irq_func)
{
	m_irqs &=~(1<<2);

	if (state)
	{
		//logerror("telestrat via2 interrupt\n");

		m_irqs |=(1<<2);
	}

	oric_refresh_ints();
}

/* interrupt state from acia6551 */
WRITE_LINE_MEMBER(oric_state::telestrat_acia_callback)
{
	m_irqs&=~(1<<3);

	if (state)
	{
		m_irqs |= (1<<3);
	}

	oric_refresh_ints();
}

MACHINE_START_MEMBER(oric_state,telestrat)
{
	UINT8 *mem = m_region_maincpu->base();

	oric_common_init_machine();

	m_telestrat_via2_port_a_data = 0;
	m_telestrat_via2_port_b_data = 0;
	m_is_telestrat = 1;

	/* initialise overlay ram */
	m_telestrat_blocks[0].MemType = TELESTRAT_MEM_BLOCK_RAM;
	m_telestrat_blocks[0].ptr = mem+0x020000; //auto_alloc_array(machine(), UINT8, 16384);

	m_telestrat_blocks[1].MemType = TELESTRAT_MEM_BLOCK_RAM;
	m_telestrat_blocks[1].ptr = mem+0x024000; //auto_alloc_array(machine(), UINT8, 16384);

	m_telestrat_blocks[2].MemType = TELESTRAT_MEM_BLOCK_RAM;
	m_telestrat_blocks[2].ptr = mem+0x028000; //auto_alloc_array(machine(), UINT8, 16384);

	/* initialise default cartridge */
	m_telestrat_blocks[3].MemType = TELESTRAT_MEM_BLOCK_ROM;
	m_telestrat_blocks[3].ptr = mem+0x010000; // telmatic.rom

	m_telestrat_blocks[4].MemType = TELESTRAT_MEM_BLOCK_RAM;
	m_telestrat_blocks[4].ptr = mem+0x02c000; //auto_alloc_array(machine(), UINT8, 16384);

	/* initialise default cartridge */
	m_telestrat_blocks[5].MemType = TELESTRAT_MEM_BLOCK_ROM;
	m_telestrat_blocks[5].ptr = mem+0x014000;  // teleass.rom

	/* initialise default cartridge */
	m_telestrat_blocks[6].MemType = TELESTRAT_MEM_BLOCK_ROM;
	m_telestrat_blocks[6].ptr = mem+0x018000; // hyperbas.rom

	/* initialise default cartridge */
	m_telestrat_blocks[7].MemType = TELESTRAT_MEM_BLOCK_ROM;
	m_telestrat_blocks[7].ptr = mem+0x01c000; // telmon24.rom

	m_telestrat_bank_selection = 7;
	telestrat_refresh_mem();

	/* disable os rom, enable microdisc rom */
	/* 0x0c000-0x0dfff will be ram, 0x0e000-0x0ffff will be microdisc rom */
	m_port_314_w = 0x0ff^((1<<7) | (1<<1));
}
