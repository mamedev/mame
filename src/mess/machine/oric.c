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
static void oric_refresh_ints(running_machine &machine)
{
	oric_state *state = machine.driver_data<oric_state>();
	/* telestrat has floppy hardware built-in! */
	if (state->m_is_telestrat==0)
	{
		/* oric 1 or oric atmos */

		/* if floppy disc hardware is disabled, do not allow interrupts from it */
		if ((machine.root_device().ioport("FLOPPY")->read() & 0x07) == ORIC_FLOPPY_INTERFACE_NONE)
			state->m_irqs &=~(1<<1);
	}

	/* any irq set? */
	if (state->m_irqs & 0x0f)
		machine.device("maincpu")->execute().set_input_line(0, HOLD_LINE);
	else
		machine.device("maincpu")->execute().set_input_line(0, CLEAR_LINE);
}



/* index of keyboard line to scan */
/* sense result */
/* mask to read keys */





/* refresh keyboard sense */
static void oric_keyboard_sense_refresh(running_machine &machine)
{
	oric_state *state = machine.driver_data<oric_state>();
	/* The following assumes that if a 0 is written, it can be used to detect if any key has been pressed.. */
	/* for each bit that is 0, it combines it's pressed state with the pressed state so far */

	int i;
	unsigned char key_bit = 0;

	/* what if data is 0, can it sense if any of the keys on a line are pressed? */
	int input_port_data;
	static const char *const keynames[] = { "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7" };

	input_port_data = machine.root_device().ioport(keynames[state->m_keyboard_line])->read();

	/* go through all bits in line */
	for (i=0; i<8; i++)
	{
		/* sense this bit? */
		if (((~state->m_keyboard_mask) & (1<<i)) != 0)
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
	state->m_key_sense_bit = 0;

	/* any keys pressed on this line? */
	if (key_bit!=0)
	{
		/* set sense result */
		state->m_key_sense_bit = (1<<3);
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
static READ8_DEVICE_HANDLER ( oric_via_in_a_func )
{
	oric_state *state = device->machine().driver_data<oric_state>();

	/*logerror("port a read\r\n"); */

	/* access psg? */
	if (state->m_psg_control!=0)
	{
		/* if psg is in read register state return reg data */
		if (state->m_psg_control==0x01)
			return ay8910_r(space.machine().device("ay8912"), space, 0);

		/* return high-impedance */
		return 0x0ff;
	}

	/* correct?? */
	return state->m_via_port_a_data;
}

static READ8_DEVICE_HANDLER ( oric_via_in_b_func )
{
	oric_state *state = device->machine().driver_data<oric_state>();
	int data;

	oric_keyboard_sense_refresh(device->machine());

	data = state->m_key_sense_bit;
	data |= state->m_keyboard_line & 0x07;

	return data;
}


/* read/write data depending on state of bdir, bc1 pins and data output to psg */
static void oric_psg_connection_refresh(address_space &space)
{
	oric_state *state = space.machine().driver_data<oric_state>();
	if (state->m_psg_control!=0)
	{
		switch (state->m_psg_control)
		{
			/* PSG inactive */
			case 0:
			break;
			/* read register data */
			case 1:
			{
				//state->m_via_port_a_data = ay8910_read_port_0_r(space, 0);
			}
			break;
			/* write register data */
			case 2:
			{
				device_t *ay8912 = space.machine().device("ay8912");
				ay8910_data_w(ay8912, space, 0, state->m_via_port_a_data);
			}
			break;
			/* write register index */
			case 3:
			{
				device_t *ay8912 = space.machine().device("ay8912");
				ay8910_address_w(ay8912, space, 0, state->m_via_port_a_data);
			}
			break;

			default:
				break;
		}

		return;
	}
}

static WRITE8_DEVICE_HANDLER ( oric_via_out_a_func )
{
	oric_state *state = device->machine().driver_data<oric_state>();
	state->m_via_port_a_data = data;

	oric_psg_connection_refresh(space);


	if (state->m_psg_control==0)
	{
		/* if psg not selected, write to printer */
		centronics_device *centronics = device->machine().device<centronics_device>("centronics");
		centronics->write(*device->machine().memory().first_space(), 0, data);
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


static cassette_image_device *cassette_device_image(running_machine &machine)
{
	return machine.device<cassette_image_device>(CASSETTE_TAG);
}

/* not called yet - this will update the via with the state of the tape data.
This allows the via to trigger on bit changes and issue interrupts */
static TIMER_CALLBACK(oric_refresh_tape)
{
	int data;
	int input_port_9;
	via6522_device *via_0 = machine.device<via6522_device>("via6522_0");

	data = 0;

	if ((cassette_device_image(machine))->input() > 0.0038)
		data |= 1;

	/* "A simple cable to catch the vertical retrace signal !
        This cable connects the video output for the television/monitor
    to the via cb1 input. Interrupts can be generated from the vertical
    sync, and flicker free games can be produced */

	input_port_9 = machine.root_device().ioport("FLOPPY")->read();
	/* cable is enabled? */
	if ((input_port_9 & 0x08)!=0)
	{
		/* return state of vsync */
		data = input_port_9>>4;
	}

	via_0->write_cb1(data);
}

static WRITE8_DEVICE_HANDLER ( oric_via_out_b_func )
{
	oric_state *state = device->machine().driver_data<oric_state>();
	centronics_device *centronics = device->machine().device<centronics_device>("centronics");

	/* KEYBOARD */
	state->m_keyboard_line = data & 0x07;

	/* CASSETTE */
	/* cassette motor control */
	cassette_device_image(device->machine())->change_state(
		(data & 0x40) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED,
		CASSETTE_MOTOR_DISABLED);

	/* cassette data out */
	cassette_device_image(device->machine())->output((data & (1<<7)) ? -1.0 : +1.0);

	/* centronics STROBE is connected to PB4 */
	centronics->strobe_w(BIT(data, 4));

	oric_psg_connection_refresh(space);
	state->m_previous_portb_data = data;
}


static READ8_DEVICE_HANDLER ( oric_via_in_ca2_func )
{
	oric_state *state = device->machine().driver_data<oric_state>();
	return state->m_psg_control & 1;
}

static READ8_DEVICE_HANDLER ( oric_via_in_cb2_func )
{
	oric_state *state = device->machine().driver_data<oric_state>();
	return (state->m_psg_control>>1) & 1;
}

static WRITE8_DEVICE_HANDLER ( oric_via_out_ca2_func )
{
	oric_state *state = device->machine().driver_data<oric_state>();
	state->m_psg_control &=~1;

	if (data)
		state->m_psg_control |=1;

	oric_psg_connection_refresh(space);
}

static WRITE8_DEVICE_HANDLER ( oric_via_out_cb2_func )
{
	oric_state *state = device->machine().driver_data<oric_state>();
	state->m_psg_control &=~2;

	if (data)
		state->m_psg_control |=2;

	oric_psg_connection_refresh(space);
}


static void oric_via_irq_func(device_t *device, int state)
{
	oric_state *drvstate = device->machine().driver_data<oric_state>();
	drvstate->m_irqs &= ~(1<<0);

	if (state)
		drvstate->m_irqs |=(1<<0);

	oric_refresh_ints(device->machine());
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

const via6522_interface oric_6522_interface=
{
	DEVCB_HANDLER(oric_via_in_a_func),
	DEVCB_HANDLER(oric_via_in_b_func),
	DEVCB_NULL,				/* printer acknowledge - handled by callback*/
	DEVCB_NULL,				/* tape input - handled by timer */
	DEVCB_HANDLER(oric_via_in_ca2_func),
	DEVCB_HANDLER(oric_via_in_cb2_func),
	DEVCB_HANDLER(oric_via_out_a_func),
	DEVCB_HANDLER(oric_via_out_b_func),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(oric_via_out_ca2_func),
	DEVCB_HANDLER(oric_via_out_cb2_func),
	DEVCB_LINE(oric_via_irq_func),
};




/*********************/
/* APPLE 2 INTERFACE */

/*
apple2 disc drive accessed through 0x0310-0x031f (read/write)
oric via accessed through 0x0300-0x030f. (read/write)
disk interface rom accessed through 0x0320-0x03ff (read only)

CALL &320 to start, or use BOBY rom.
*/

static void oric_install_apple2_interface(running_machine &machine)
{
	oric_state *state = machine.driver_data<oric_state>();
	device_t *fdc = machine.device("fdc");
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	if (state->m_is_telestrat)
		return;

	space->install_read_handler(0x0300, 0x030f, read8_delegate(FUNC(oric_state::oric_IO_r),state));
	space->install_legacy_read_handler(*fdc, 0x0310, 0x031f, FUNC(applefdc_r));
	space->install_read_bank(0x0320, 0x03ff, "bank4");

	space->install_write_handler(0x0300, 0x030f, write8_delegate(FUNC(oric_state::oric_IO_w),state));
	space->install_legacy_write_handler(*fdc, 0x0310, 0x031f, FUNC(applefdc_w));
	state->membank("bank4")->set_base(	state->memregion("maincpu")->base() + 0x014000 + 0x020);
}


static void oric_enable_memory(running_machine &machine, int low, int high, int rd, int wr)
{
	oric_state *state = machine.driver_data<oric_state>();
	int i;
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	if (state->m_is_telestrat)
		return;
	for (i = low; i <= high; i++)
	{
		switch(i) {
		case 1:
			if (rd) {
				space->install_read_bank(0xc000, 0xdfff, "bank1");
			} else {
				space->nop_read(0xc000, 0xdfff);
			}
			if (wr) {
				space->install_write_bank(0xc000, 0xdfff, "bank5");
			} else {
				space->unmap_write(0xc000, 0xdfff);
			}
			break;
		case 2:
			if (rd) {
				space->install_read_bank(0xe000, 0xf7ff, "bank2");
			} else {
				space->nop_read(0xe000, 0xf7ff);
			}
			if (wr) {
				space->install_write_bank(0xe000, 0xf7ff, "bank6");
			} else {
				space->unmap_write(0xe000, 0xf7ff);
			}
			break;
		case 3:
			if (rd) {
				space->install_read_bank(0xf800, 0xffff, "bank3");
			} else {
				space->nop_read(0xf800, 0xffff);
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
	membank("bank4")->set_base(machine().root_device().memregion("maincpu")->base() + 0x014000 + 0x0100 + (((offset & 0x02)>>1)<<8));

	oric_enable_memory(machine(), 1, 3, TRUE, TRUE);

	/* bit 1 is 0, rom enabled, bit 1 is 1 ram enabled */
	if ((offset & 0x01)==0)
	{
		unsigned char *rom_ptr;

		/* logerror("apple 2 interface v2: rom enabled\n"); */

		/* enable rom */
		rom_ptr = memregion("maincpu")->base() + 0x010000;
		membank("bank1")->set_base(rom_ptr);
		membank("bank2")->set_base(rom_ptr+0x02000);
		membank("bank3")->set_base(rom_ptr+0x03800);
		membank("bank5")->set_base(m_ram_0x0c000);
		membank("bank6")->set_base(m_ram_0x0c000+0x02000);
		membank("bank7")->set_base(m_ram_0x0c000+0x03800);
	}
	else
	{
		/*logerror("apple 2 interface v2: ram enabled\n"); */

		/* enable ram */
		membank("bank1")->set_base(m_ram_0x0c000);
		membank("bank2")->set_base(m_ram_0x0c000+0x02000);
		membank("bank3")->set_base(m_ram_0x0c000+0x03800);
		membank("bank5")->set_base(m_ram_0x0c000);
		membank("bank6")->set_base(m_ram_0x0c000+0x02000);
		membank("bank7")->set_base(m_ram_0x0c000+0x03800);
	}
}


/* APPLE 2 INTERFACE V2 */
static void oric_install_apple2_v2_interface(running_machine &machine)
{
	oric_state *state = machine.driver_data<oric_state>();
	device_t *fdc = machine.device("fdc");
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	space->install_read_handler(0x0300, 0x030f, read8_delegate(FUNC(oric_state::oric_IO_r),state));
	space->install_legacy_read_handler(*fdc, 0x0310, 0x031f, FUNC(applefdc_r));
	space->install_read_bank(0x0320, 0x03ff, "bank4");

	space->install_write_handler(0x0300, 0x030f, write8_delegate(FUNC(oric_state::oric_IO_w),state));
	space->install_legacy_write_handler(*fdc, 0x0310, 0x031f, FUNC(applefdc_w));
	space->install_write_handler(0x0380, 0x0383, write8_delegate(FUNC(oric_state::apple2_v2_interface_w),state));

	state->apple2_v2_interface_w(*space, 0, 0);
}

/********************/
/* JASMIN INTERFACE */


/* bit 0: overlay ram access (1 means overlay ram enabled) */

/* bit 0: ROMDIS (1 means internal Basic rom disabled) */


static void oric_jasmin_set_mem_0x0c000(running_machine &machine)
{
	oric_state *state = machine.driver_data<oric_state>();
	/* assumption:
    1. It is possible to access all 16k overlay ram.
    2. If os is enabled, and overlay ram is enabled, all 16k can be accessed.
    3. if os is disabled, and overlay ram is enabled, jasmin rom takes priority.
    */
	if (state->m_is_telestrat)
		return;

	/* the ram is disabled in the jasmin rom which indicates that jasmin takes
    priority over the ram */

	/* basic rom disabled? */
	if ((state->m_port_3fb_w & 0x01)==0)
	{
		/* no, it is enabled! */

		/* overlay ram enabled? */
		if ((state->m_port_3fa_w & 0x01)==0)
		{
			unsigned char *rom_ptr;

			/* no it is disabled */
			/*logerror("&c000-&ffff is os rom\n"); */

			oric_enable_memory(machine, 1, 3, TRUE, FALSE);

			rom_ptr = state->memregion("maincpu")->base() + 0x010000;
			state->membank("bank1")->set_base(rom_ptr);
			state->membank("bank2")->set_base(rom_ptr+0x02000);
			state->membank("bank3")->set_base(rom_ptr+0x03800);
		}
		else
		{
			/*logerror("&c000-&ffff is ram\n"); */

			oric_enable_memory(machine, 1, 3, TRUE, TRUE);

			state->membank("bank1")->set_base(state->m_ram_0x0c000);
			state->membank("bank2")->set_base(state->m_ram_0x0c000+0x02000);
			state->membank("bank3")->set_base(state->m_ram_0x0c000+0x03800);
			state->membank("bank5")->set_base(state->m_ram_0x0c000);
			state->membank("bank6")->set_base(state->m_ram_0x0c000+0x02000);
			state->membank("bank7")->set_base(state->m_ram_0x0c000+0x03800);
		}
	}
	else
	{
		/* yes, basic rom is disabled */

		if ((state->m_port_3fa_w & 0x01)==0)
		{
			/* overlay ram disabled */

			/*logerror("&c000-&f8ff is nothing!\n"); */
			oric_enable_memory(machine, 1, 2, FALSE, FALSE);
		}
		else
		{
			/*logerror("&c000-&f8ff is ram!\n"); */
			oric_enable_memory(machine, 1, 2, TRUE, TRUE);

			state->membank("bank1")->set_base(state->m_ram_0x0c000);
			state->membank("bank2")->set_base(state->m_ram_0x0c000+0x02000);
			state->membank("bank5")->set_base(state->m_ram_0x0c000);
			state->membank("bank6")->set_base(state->m_ram_0x0c000+0x02000);
		}

		{
			/* basic rom disabled */
			unsigned char *rom_ptr;

			/*logerror("&f800-&ffff is jasmin rom\n"); */
			/* jasmin rom enabled */
			oric_enable_memory(machine, 3, 3, TRUE, TRUE);
			rom_ptr = machine.root_device().memregion("maincpu")->base() + 0x010000+0x04000+0x02000;
			state->membank("bank3")->set_base(rom_ptr);
			state->membank("bank7")->set_base(rom_ptr);
		}
	}
}

/* DRQ is connected to interrupt */
static WRITE_LINE_DEVICE_HANDLER( oric_jasmin_wd179x_drq_w )
{
	oric_state *drvstate = device->machine().driver_data<oric_state>();
	if (state)
		drvstate->m_irqs |= (1<<1);
	else
		drvstate->m_irqs &=~(1<<1);

	oric_refresh_ints(device->machine());
}

READ8_MEMBER(oric_state::oric_jasmin_r)
{
	via6522_device *via_0 = machine().device<via6522_device>("via6522_0");
	device_t *fdc = machine().device("wd179x");
	unsigned char data = 0x0ff;

	switch (offset & 0x0f)
	{
		/* jasmin floppy disc interface */
		case 0x04:
			data = wd17xx_status_r(fdc, space, 0);
			break;
		case 0x05:
			data =wd17xx_track_r(fdc, space, 0);
			break;
		case 0x06:
			data = wd17xx_sector_r(fdc, space, 0);
			break;
		case 0x07:
			data = wd17xx_data_r(fdc, space, 0);
			break;
		default:
			data = via_0->read(space,offset & 0x0f);
			//logerror("unhandled io read: %04x %02x\n", offset, data);
			break;

	}

	return data;
}

WRITE8_MEMBER(oric_state::oric_jasmin_w)
{
	via6522_device *via_0 = machine().device<via6522_device>("via6522_0");
	device_t *fdc = machine().device("wd179x");
	switch (offset & 0x0f)
	{
		/* microdisc floppy disc interface */
		case 0x04:
			wd17xx_command_w(fdc, space, 0, data);
			break;
		case 0x05:
			wd17xx_track_w(fdc, space, 0, data);
			break;
		case 0x06:
			wd17xx_sector_w(fdc, space, 0, data);
			break;
		case 0x07:
			wd17xx_data_w(fdc, space, 0, data);
			break;
		/* bit 0 = side */
		case 0x08:
			wd17xx_set_side(fdc,data & 0x01);
			break;
		/* any write will cause wd179x to reset */
		case 0x09:
			wd17xx_reset(fdc);
			break;
		case 0x0a:
			//logerror("jasmin overlay ram w: %02x PC: %04x\n", data, machine().device("maincpu")->safe_pc());
			m_port_3fa_w = data;
			oric_jasmin_set_mem_0x0c000(machine());
			break;
		case 0x0b:
			//logerror("jasmin romdis w: %02x PC: %04x\n", data, machine().device("maincpu")->safe_pc());
			m_port_3fb_w = data;
			oric_jasmin_set_mem_0x0c000(machine());
			break;
		/* bit 0,1 of addr is the drive */
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
			wd17xx_set_drive(fdc,offset & 0x03);
			break;

		default:
			via_0->write(space,offset & 0x0f, data);
			break;
	}
}


static void oric_install_jasmin_interface(running_machine &machine)
{
	oric_state *state = machine.driver_data<oric_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	/* romdis */
	state->m_port_3fb_w = 1;
	oric_jasmin_set_mem_0x0c000(machine);

	space->install_read_handler(0x0300, 0x03ef, read8_delegate(FUNC(oric_state::oric_IO_r),state));
	space->install_read_handler(0x03f0, 0x03ff, read8_delegate(FUNC(oric_state::oric_jasmin_r),state));

	space->install_write_handler(0x0300, 0x03ef, write8_delegate(FUNC(oric_state::oric_IO_w),state));
	space->install_write_handler(0x03f0, 0x03ff, write8_delegate(FUNC(oric_state::oric_jasmin_w),state));
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


static void oric_microdisc_refresh_wd179x_ints(running_machine &machine)
{
	oric_state *state = machine.driver_data<oric_state>();
	state->m_irqs &=~(1<<1);

	if ((state->m_wd179x_int_state) && (state->m_port_314_w & (1<<0)))
	{
		/*logerror("oric microdisc interrupt\n"); */

		state->m_irqs |=(1<<1);
	}

	oric_refresh_ints(machine);
}

static WRITE_LINE_DEVICE_HANDLER( oric_microdisc_wd179x_intrq_w )
{
	oric_state *drvstate = device->machine().driver_data<oric_state>();
	drvstate->m_wd179x_int_state = state;

	if (state)
		drvstate->m_port_314_r &= ~(1<<7);
	else
		drvstate->m_port_314_r |=(1<<7);

	oric_microdisc_refresh_wd179x_ints(device->machine());
}

static WRITE_LINE_DEVICE_HANDLER( oric_microdisc_wd179x_drq_w )
{
	oric_state *drvstate = device->machine().driver_data<oric_state>();
	if (state)
		drvstate->m_port_318_r &=~(1<<7);
	else
		drvstate->m_port_318_r |= (1<<7);
}

static void oric_microdisc_set_mem_0x0c000(running_machine &machine)
{
	oric_state *state = machine.driver_data<oric_state>();
	if (state->m_is_telestrat)
		return;

	/* for 0x0c000-0x0dfff: */
	/* if os disabled, ram takes priority */
	/* /ROMDIS */
	if ((state->m_port_314_w & (1<<1))==0)
	{
		/*logerror("&c000-&dfff is ram\n"); */
		/* rom disabled enable ram */
		oric_enable_memory(machine, 1, 1, TRUE, TRUE);
		state->membank("bank1")->set_base(state->m_ram_0x0c000);
		state->membank("bank5")->set_base(state->m_ram_0x0c000);
	}
	else
	{
		unsigned char *rom_ptr;
		/*logerror("&c000-&dfff is os rom\n"); */
		/* basic rom */
		oric_enable_memory(machine, 1, 1, TRUE, FALSE);
		rom_ptr = machine.root_device().memregion("maincpu")->base() + 0x010000;
		state->membank("bank1")->set_base(rom_ptr);
		state->membank("bank5")->set_base(rom_ptr);
	}

	/* for 0x0e000-0x0ffff */
	/* if not disabled, os takes priority */
	if ((state->m_port_314_w & (1<<1))!=0)
	{
		unsigned char *rom_ptr;
		/*logerror("&e000-&ffff is os rom\n"); */
		/* basic rom */
		oric_enable_memory(machine, 2, 3, TRUE, FALSE);
		rom_ptr = machine.root_device().memregion("maincpu")->base() + 0x010000;
		state->membank("bank2")->set_base(rom_ptr+0x02000);
		state->membank("bank3")->set_base(rom_ptr+0x03800);
		state->membank("bank6")->set_base(rom_ptr+0x02000);
		state->membank("bank7")->set_base(rom_ptr+0x03800);

	}
	else
	{
		/* if eprom is enabled, it takes priority over ram */
		if ((state->m_port_314_w & (1<<7))==0)
		{
			unsigned char *rom_ptr;
			/*logerror("&e000-&ffff is disk rom\n"); */
			oric_enable_memory(machine, 2, 3, TRUE, FALSE);
			/* enable rom of microdisc interface */
			rom_ptr = machine.root_device().memregion("maincpu")->base() + 0x014000;
			state->membank("bank2")->set_base(rom_ptr);
			state->membank("bank3")->set_base(rom_ptr+0x01800);
		}
		else
		{
			/*logerror("&e000-&ffff is ram\n"); */
			/* rom disabled enable ram */
			oric_enable_memory(machine, 2, 3, TRUE, TRUE);
			state->membank("bank2")->set_base(state->m_ram_0x0c000+0x02000);
			state->membank("bank3")->set_base(state->m_ram_0x0c000+0x03800);
			state->membank("bank6")->set_base(state->m_ram_0x0c000+0x02000);
			state->membank("bank7")->set_base(state->m_ram_0x0c000+0x03800);
		}
	}
}



READ8_MEMBER(oric_state::oric_microdisc_r)
{
	unsigned char data = 0x0ff;
	device_t *fdc = machine().device("wd179x");

	switch (offset & 0x0ff)
	{
		/* microdisc floppy disc interface */
		case 0x00:
			data = wd17xx_status_r(fdc, space, 0);
			break;
		case 0x01:
			data =wd17xx_track_r(fdc, space, 0);
			break;
		case 0x02:
			data = wd17xx_sector_r(fdc, space, 0);
			break;
		case 0x03:
			data = wd17xx_data_r(fdc, space, 0);
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
			{
				via6522_device *via_0 = machine().device<via6522_device>("via6522_0");
				data = via_0->read(space, offset & 0x0f);
			}
			break;

	}

	return data;
}

WRITE8_MEMBER(oric_state::oric_microdisc_w)
{
	device_t *fdc = machine().device("wd179x");
	switch (offset & 0x0ff)
	{
		/* microdisc floppy disc interface */
		case 0x00:
			wd17xx_command_w(fdc, space, 0, data);
			break;
		case 0x01:
			wd17xx_track_w(fdc, space, 0, data);
			break;
		case 0x02:
			wd17xx_sector_w(fdc, space, 0, data);
			break;
		case 0x03:
			wd17xx_data_w(fdc, space, 0, data);
			break;
		case 0x04:
		{
			m_port_314_w = data;

			//logerror("port_314_w: %02x\n",data);

			/* bit 6,5: drive */
			/* bit 4: side */
			/* bit 3: double density enable */
			/* bit 0: enable FDC IRQ to trigger IRQ on CPU */
			wd17xx_set_drive(fdc,(data>>5) & 0x03);
			wd17xx_set_side(fdc,(data>>4) & 0x01);
			wd17xx_dden_w(fdc, !BIT(data, 3));

			oric_microdisc_set_mem_0x0c000(machine());
			oric_microdisc_refresh_wd179x_ints(machine());
		}
		break;

		default:
			{
				via6522_device *via_0 = machine().device<via6522_device>("via6522_0");
				via_0->write(space, offset & 0x0f, data);
			}
			break;
	}
}

static void oric_install_microdisc_interface(running_machine &machine)
{
	oric_state *state = machine.driver_data<oric_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	space->install_read_handler(0x0300, 0x030f, read8_delegate(FUNC(oric_state::oric_IO_r),state));
	space->install_read_handler(0x0310, 0x031f, read8_delegate(FUNC(oric_state::oric_microdisc_r),state));
	space->install_read_handler(0x0320, 0x03ff, read8_delegate(FUNC(oric_state::oric_IO_r),state));

	space->install_write_handler(0x0300, 0x030f, write8_delegate(FUNC(oric_state::oric_IO_w),state));
	space->install_write_handler(0x0310, 0x031f, write8_delegate(FUNC(oric_state::oric_microdisc_w),state));
	space->install_write_handler(0x0320, 0x03ff, write8_delegate(FUNC(oric_state::oric_IO_w),state));

	/* disable os rom, enable microdisc rom */
	/* 0x0c000-0x0dfff will be ram, 0x0e000-0x0ffff will be microdisc rom */
	state->m_port_314_w = 0x0ff^((1<<7) | (1<<1));

	oric_microdisc_set_mem_0x0c000(machine);
}



/*********************************************************/

static WRITE_LINE_DEVICE_HANDLER( oric_wd179x_intrq_w )
{
	if ((device->machine().root_device().ioport("FLOPPY")->read() & 0x07) == ORIC_FLOPPY_INTERFACE_MICRODISC)
		oric_microdisc_wd179x_intrq_w(device, state);
}

static WRITE_LINE_DEVICE_HANDLER( oric_wd179x_drq_w )
{
	switch (device->machine().root_device().ioport("FLOPPY")->read() &  0x07)
	{
		default:
		case ORIC_FLOPPY_INTERFACE_NONE:
		case ORIC_FLOPPY_INTERFACE_APPLE2:
			return;
		case ORIC_FLOPPY_INTERFACE_MICRODISC:
			oric_microdisc_wd179x_drq_w(device, state);
			return;
		case ORIC_FLOPPY_INTERFACE_JASMIN:
			oric_jasmin_wd179x_drq_w(device, state);
			return;
	}
}

const wd17xx_interface oric_wd17xx_interface =
{
	DEVCB_NULL,
	DEVCB_LINE(oric_wd179x_intrq_w),
	DEVCB_LINE(oric_wd179x_drq_w),
	{FLOPPY_0, FLOPPY_1, FLOPPY_2, FLOPPY_3}
};

static void oric_common_init_machine(running_machine &machine)
{
	oric_state *state = machine.driver_data<oric_state>();
	/* clear all irqs */
	state->m_irqs = 0;
	state->m_ram_0x0c000 = NULL;
	state->m_keyboard_line = 0;
	state->m_key_sense_bit = 0;
	state->m_keyboard_mask = 0;
	state->m_via_port_a_data = 0;
	state->m_psg_control = 0;
	state->m_previous_portb_data = 0;
	state->m_port_3fa_w = 0;
	state->m_port_3fb_w = 0;
	state->m_wd179x_int_state = 0;
	state->m_port_314_r = 0;
	state->m_port_318_r = 0;
	state->m_port_314_w = 0;
	machine.scheduler().timer_pulse(attotime::from_hz(4800), FUNC(oric_refresh_tape));
}

void oric_state::machine_start()
{
	oric_common_init_machine(machine());

	m_is_telestrat = 0;

	m_ram_0x0c000 = auto_alloc_array(machine(), UINT8, 16384);
}


void oric_state::machine_reset()
{
	int disc_interface_id = machine().root_device().ioport("FLOPPY")->read() & 0x07;
	address_space *space = machine().device("maincpu")->memory().space(AS_PROGRAM);
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
			oric_enable_memory(machine(), 1, 3, TRUE, FALSE);
			rom_ptr = memregion("maincpu")->base() + 0x010000;
			membank("bank1")->set_base(rom_ptr);
			membank("bank2")->set_base(rom_ptr+0x02000);
			membank("bank3")->set_base(rom_ptr+0x03800);
			membank("bank5")->set_base(rom_ptr);
			membank("bank6")->set_base(rom_ptr+0x02000);
			membank("bank7")->set_base(rom_ptr+0x03800);


			if (disc_interface_id==ORIC_FLOPPY_INTERFACE_APPLE2)
			{
				oric_install_apple2_interface(machine());
			}
			else
			{
				space->install_read_handler(0x0300, 0x03ff, read8_delegate(FUNC(oric_state::oric_IO_r),this));
				space->install_write_handler(0x0300, 0x03ff, write8_delegate(FUNC(oric_state::oric_IO_w),this));
			}
		}
		break;

		case ORIC_FLOPPY_INTERFACE_APPLE2_V2:
		{
			oric_install_apple2_v2_interface(machine());
		}
		break;


		case ORIC_FLOPPY_INTERFACE_MICRODISC:
		{
			oric_install_microdisc_interface(machine());
		}
		break;

		case ORIC_FLOPPY_INTERFACE_JASMIN:
		{
			oric_install_jasmin_interface(machine());
		}
		break;
	}
	machine().device("maincpu")->reset();
}


READ8_MEMBER(oric_state::oric_IO_r)
{
	via6522_device *via_0 = machine().device<via6522_device>("via6522_0");
	switch (ioport("FLOPPY")->read() & 0x07)
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
	return via_0->read(space, offset & 0x0f);
}

WRITE8_MEMBER(oric_state::oric_IO_w)
{
	via6522_device *via_0 = machine().device<via6522_device>("via6522_0");
	switch (ioport("FLOPPY")->read() & 0x07)
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

	via_0->write(space, offset & 0x0f, data);
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


static void telestrat_refresh_mem(running_machine &machine)
{
	oric_state *state = machine.driver_data<oric_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	telestrat_mem_block *mem_block = &state->m_telestrat_blocks[state->m_telestrat_bank_selection];

	switch (mem_block->MemType)
	{
		case TELESTRAT_MEM_BLOCK_RAM:
		{
			state->membank("bank1")->set_base(mem_block->ptr);
			state->membank("bank2")->set_base(mem_block->ptr);
			space->install_read_bank(0xc000, 0xffff, "bank1");
			space->install_write_bank(0xc000, 0xffff, "bank2");
		}
		break;

		case TELESTRAT_MEM_BLOCK_ROM:
		{
			state->membank("bank1")->set_base(mem_block->ptr);
			space->install_read_bank(0xc000, 0xffff, "bank1");
			space->nop_write(0xc000, 0xffff);
		}
		break;

		default:
		case TELESTRAT_MEM_BLOCK_UNDEFINED:
		{
			space->nop_readwrite(0xc000, 0xffff);
		}
		break;
	}
}

static READ8_DEVICE_HANDLER(telestrat_via2_in_a_func)
{
	oric_state *state = device->machine().driver_data<oric_state>();
	//logerror("via 2 - port a %02x\n",state->m_telestrat_via2_port_a_data);
	return state->m_telestrat_via2_port_a_data;
}


static WRITE8_DEVICE_HANDLER(telestrat_via2_out_a_func)
{
	oric_state *state = device->machine().driver_data<oric_state>();
	//logerror("via 2 - port a w: %02x\n",data);

	state->m_telestrat_via2_port_a_data = data;

	if (((data^state->m_telestrat_bank_selection) & 0x07)!=0)
	{
		state->m_telestrat_bank_selection = data & 0x07;

		telestrat_refresh_mem(device->machine());
	}
}

static READ8_DEVICE_HANDLER(telestrat_via2_in_b_func)
{
	oric_state *state = device->machine().driver_data<oric_state>();
	unsigned char data = 0x01f;

	/* left joystick selected? */
	if (state->m_telestrat_via2_port_b_data & (1<<6))
	{
		data &= state->ioport("JOY0")->read();
	}

	/* right joystick selected? */
	if (state->m_telestrat_via2_port_b_data & (1<<7))
	{
		data &= device->machine().root_device().ioport("JOY1")->read();
	}

	data |= state->m_telestrat_via2_port_b_data & ((1<<7) | (1<<6) | (1<<5));

	return data;
}

static WRITE8_DEVICE_HANDLER(telestrat_via2_out_b_func)
{
	oric_state *state = device->machine().driver_data<oric_state>();
	state->m_telestrat_via2_port_b_data = data;
}


static void telestrat_via2_irq_func(device_t *device, int state)
{
	oric_state *drvstate = device->machine().driver_data<oric_state>();
	drvstate->m_irqs &=~(1<<2);

	if (state)
	{
		//logerror("telestrat via2 interrupt\n");

		drvstate->m_irqs |=(1<<2);
	}

	oric_refresh_ints(device->machine());
}

const via6522_interface telestrat_via2_interface=
{
	DEVCB_HANDLER(telestrat_via2_in_a_func),
	DEVCB_HANDLER(telestrat_via2_in_b_func),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(telestrat_via2_out_a_func),
	DEVCB_HANDLER(telestrat_via2_out_b_func),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_LINE(telestrat_via2_irq_func),
};

#if 0
/* interrupt state from acia6551 */
static void telestrat_acia_callback(running_machine &machine, int irq_state)
{
	oric_state *state = machine.driver_data<oric_state>();
	state->m_irqs&=~(1<<3);

	if (irq_state)
	{
		state->m_irqs |= (1<<3);
	}

	oric_refresh_ints(machine);
}
#endif

MACHINE_START_MEMBER(oric_state,telestrat)
{
	UINT8 *mem = memregion("maincpu")->base();

	oric_common_init_machine(machine());

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
	telestrat_refresh_mem(machine());

	/* disable os rom, enable microdisc rom */
	/* 0x0c000-0x0dfff will be ram, 0x0e000-0x0ffff will be microdisc rom */
	m_port_314_w = 0x0ff^((1<<7) | (1<<1));
}
