/***************************************************************************

    commodore c16 home computer

    peter.trauner@jk.uni-linz.ac.at
    documentation
     www.funet.fi

***************************************************************************/

#include "emu.h"
#include "audio/ted7360.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/cassette.h"
#include "imagedev/cartslot.h"
#include "machine/ram.h"
#include "includes/c16.h"
#include "machine/cbmiec.h"
#include "sound/sid6581.h"

#define VERBOSE_LEVEL 0
#define DBG_LOG( MACHINE, N, M, A ) \
	do { \
		if(VERBOSE_LEVEL >= N) \
		{ \
			if( M ) \
				logerror("%11.6f: %-24s", MACHINE.time().as_double(), (char*) M ); \
			logerror A; \
		} \
	} while (0)


/*
 * tia6523
 *
 * connector to floppy c1551 (delivered with c1551 as c16 expansion)
 * port a for data read/write
 * port b
 * 0 status 0
 * 1 status 1
 * port c
 * 6 dav output edge data on port a available
 * 7 ack input edge ready for next datum
 */

/*
  ddr bit 1 port line is output
  port bit 1 port line is high

  serial bus
  1 serial srq in (ignored)
  2 gnd
  3 atn out (pull up)
  4 clock in/out (pull up)
  5 data in/out (pull up)
  6 /reset (pull up) hardware


  p0 negated serial bus pin 5 /data out
  p1 negated serial bus pin 4 /clock out, cassette write
  p2 negated serial bus pin 3 /atn out
  p3 cassette motor out

  p4 cassette read
  p5 not connected (or not available on MOS7501?)
  p6 serial clock in
  p7 serial data in, serial bus 5
*/

WRITE8_DEVICE_HANDLER(c16_m7501_port_write)
{
	c16_state *state = device->machine().driver_data<c16_state>();

	/* bit zero then output 0 */
	state->m_iec->atn_w(!BIT(data, 2));
	state->m_iec->clk_w(!BIT(data, 1));
	state->m_iec->data_w(!BIT(data, 0));

	state->m_cassette->output(!BIT(data, 1) ? -(0x5a9e >> 1) : +(0x5a9e >> 1));

	state->m_cassette->change_state(BIT(data, 7) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
}

READ8_DEVICE_HANDLER(c16_m7501_port_read)
{
	c16_state *state = device->machine().driver_data<c16_state>();
	UINT8 data = 0xff;
	UINT8 c16_port7501 = m6510_get_port(state->m_maincpu);

	if (BIT(c16_port7501, 0) || !state->m_iec->data_r())
		data &= ~0x80;

	if (BIT(c16_port7501, 1) || !state->m_iec->clk_r())
		data &= ~0x40;

//  data &= ~0x20; // port bit not in pinout

	if (state->m_cassette->input() > +0.0)
		data |=  0x10;
	else
		data &= ~0x10;

	return data;
}

static void c16_bankswitch( running_machine &machine )
{
	c16_state *state = machine.driver_data<c16_state>();
	UINT8 *rom = state->memregion("maincpu")->base();
	state->membank("bank9")->set_base(state->m_messram->pointer());

	switch (state->m_lowrom)
	{
	case 0:
		state->membank("bank2")->set_base(rom + 0x10000);
		break;
	case 1:
		state->membank("bank2")->set_base(rom + 0x18000);
		break;
	case 2:
		state->membank("bank2")->set_base(rom + 0x20000);
		break;
	case 3:
		state->membank("bank2")->set_base(rom + 0x28000);
		break;
	}

	switch (state->m_highrom)
	{
	case 0:
		state->membank("bank3")->set_base(rom + 0x14000);
		state->membank("bank8")->set_base(rom + 0x17f20);
		break;
	case 1:
		state->membank("bank3")->set_base(rom + 0x1c000);
		state->membank("bank8")->set_base(rom + 0x1ff20);
		break;
	case 2:
		state->membank("bank3")->set_base(rom + 0x24000);
		state->membank("bank8")->set_base(rom + 0x27f20);
		break;
	case 3:
		state->membank("bank3")->set_base(rom + 0x2c000);
		state->membank("bank8")->set_base(rom + 0x2ff20);
		break;
	}
	state->membank("bank4")->set_base(rom + 0x17c00);
}

WRITE8_HANDLER( c16_switch_to_rom )
{
	c16_state *state = space->machine().driver_data<c16_state>();

	ted7360_rom_switch_w(state->m_ted7360, 1);
	c16_bankswitch(space->machine());
}

/* write access to fddX load data flipflop
 * and selects roms
 * a0 a1
 * 0  0  basic
 * 0  1  plus4 low
 * 1  0  c1 low
 * 1  1  c2 low
 *
 * a2 a3
 * 0  0  kernal
 * 0  1  plus4 hi
 * 1  0  c1 high
 * 1  1  c2 high */
WRITE8_HANDLER( c16_select_roms )
{
	c16_state *state = space->machine().driver_data<c16_state>();

	state->m_lowrom = offset & 0x03;
	state->m_highrom = (offset & 0x0c) >> 2;
	if (ted7360_rom_switch_r(state->m_ted7360))
		c16_bankswitch(space->machine());
}

WRITE8_HANDLER( c16_switch_to_ram )
{
	c16_state *state = space->machine().driver_data<c16_state>();
	UINT8 *ram = state->m_messram->pointer();
	UINT32 ram_size = state->m_messram->size();

	ted7360_rom_switch_w(state->m_ted7360, 0);

	state->membank("bank2")->set_base(ram + (0x8000 % ram_size));
	state->membank("bank3")->set_base(ram + (0xc000 % ram_size));
	state->membank("bank4")->set_base(ram + (0xfc00 % ram_size));
	state->membank("bank8")->set_base(ram + (0xff20 % ram_size));
}

UINT8 c16_read_keyboard( running_machine &machine, int databus )
{
	c16_state *state = machine.driver_data<c16_state>();
	UINT8 value = 0xff;
	int i;

	for (i = 0; i < 8; i++)
	{
		if (!BIT(state->m_port6529, i))
			value &= state->m_keyline[i];
	}

	/* looks like joy 0 needs dataline2 low
     * and joy 1 needs dataline1 low
     * write to 0xff08 (value on databus) reloads latches */
	if (!BIT(databus, 2))
		value &= state->m_keyline[8];

	if (!BIT(databus, 1))
		value &= state->m_keyline[9];

	return value;
}

/*
 * mos 6529
 * simple 1 port 8bit input output
 * output with pull up resistors, 0 means low
 * input, 0 means low
 */
/*
 * ic used as output,
 * output low means keyboard line selected
 * keyboard line is then read into the ted7360 latch
 */
WRITE8_HANDLER( c16_6529_port_w )
{
	c16_state *state = space->machine().driver_data<c16_state>();
	state->m_port6529 = data;
}

READ8_HANDLER( c16_6529_port_r )
{
	c16_state *state = space->machine().driver_data<c16_state>();
	return state->m_port6529 & (c16_read_keyboard (space->machine(), 0xff /*databus */ ) | (state->m_port6529 ^ 0xff));
}

/*
 * p0 Userport b
 * p1 Userport k
 * p2 Userport 4, cassette sense
 * p3 Userport 5
 * p4 Userport 6
 * p5 Userport 7
 * p6 Userport j
 * p7 Userport f
 */
WRITE8_HANDLER( plus4_6529_port_w )
{
}

READ8_HANDLER( plus4_6529_port_r )
{
	c16_state *state = space->machine().driver_data<c16_state>();
	int data = 0x00;

	if ((state->m_cassette->get_state() & CASSETTE_MASK_UISTATE) != CASSETTE_STOPPED)
		data &= ~0x04;
	else
		data |=  0x04;

	return data;
}

READ8_HANDLER( c16_fd1x_r )
{
	c16_state *state = space->machine().driver_data<c16_state>();
	int data = 0x00;

	if ((state->m_cassette->get_state() & CASSETTE_MASK_UISTATE) != CASSETTE_STOPPED)
		data &= ~0x04;
	else
		data |=  0x04;

	return data;
}

/**
 0 write: transmit data
 0 read: receiver data
 1 write: programmed rest (data is dont care)
 1 read: status register
 2 command register
 3 control register
 control register (offset 3)
  cleared by hardware reset, not changed by programmed reset
  7: 2 stop bits (0 1 stop bit)
  6,5: data word length
   00 8 bits
   01 7
   10 6
   11 5
  4: ?? clock source
   0 external receiver clock
   1 baud rate generator
  3-0: baud rate generator
   0000 use external clock
   0001 60
   0010 75
   0011
   0100
   0101
   0110 300
   0111 600
   1000 1200
   1001
   1010 2400
   1011 3600
   1100 4800
   1101 7200
   1110 9600
   1111 19200
 control register
  */
WRITE8_HANDLER( c16_6551_port_w )
{
	c16_state *state = space->machine().driver_data<c16_state>();

	offset &= 0x03;
	DBG_LOG(space->machine(), 3, "6551", ("port write %.2x %.2x\n", offset, data));
	state->m_port6529 = data;
}

READ8_HANDLER( c16_6551_port_r )
{
	int data = 0x00;

	offset &= 0x03;
	DBG_LOG(space->machine(), 3, "6551", ("port read %.2x %.2x\n", offset, data));
	return data;
}

int c16_dma_read( running_machine &machine, int offset )
{
	c16_state *state = machine.driver_data<c16_state>();
	return state->m_messram->pointer()[offset % state->m_messram->size()];
}

int c16_dma_read_rom( running_machine &machine, int offset )
{
	c16_state *state = machine.driver_data<c16_state>();

	/* should read real c16 system bus from 0xfd00 -ff1f */
	if (offset >= 0xc000)
	{								   /* rom address in rom */
		if ((offset >= 0xfc00) && (offset < 0xfd00))
			return state->m_mem10000[offset];

		switch (state->m_highrom)
		{
			case 0:
				return state->m_mem10000[offset & 0x7fff];
			case 1:
				return state->m_mem18000[offset & 0x7fff];
			case 2:
				return state->m_mem20000[offset & 0x7fff];
			case 3:
				return state->m_mem28000[offset & 0x7fff];
		}
	}

	if (offset >= 0x8000)
	{								   /* rom address in rom */
		switch (state->m_lowrom)
		{
			case 0:
				return state->m_mem10000[offset & 0x7fff];
			case 1:
				return state->m_mem18000[offset & 0x7fff];
			case 2:
				return state->m_mem20000[offset & 0x7fff];
			case 3:
				return state->m_mem28000[offset & 0x7fff];
		}
	}

	return state->m_messram->pointer()[offset % state->m_messram->size()];
}

void c16_interrupt( running_machine &machine, int level )
{
	c16_state *state = machine.driver_data<c16_state>();

	if (level != state->m_old_level)
	{
		DBG_LOG(machine, 3, "mos7501", ("irq %s\n", level ? "start" : "end"));
		device_set_input_line(state->m_maincpu, M6510_IRQ_LINE, level);
		state->m_old_level = level;
	}
}

static void c16_common_driver_init( running_machine &machine )
{
	c16_state *state = machine.driver_data<c16_state>();
	UINT8 *rom = state->memregion("maincpu")->base();

	/* initial bankswitch (notice that TED7360 is init to ROM) */
	state->membank("bank2")->set_base(rom + 0x10000);
	state->membank("bank3")->set_base(rom + 0x14000);
	state->membank("bank4")->set_base(rom + 0x17c00);
	state->membank("bank8")->set_base(rom + 0x17f20);

	state->m_mem10000 = rom + 0x10000;
	state->m_mem14000 = rom + 0x14000;
	state->m_mem18000 = rom + 0x18000;
	state->m_mem1c000 = rom + 0x1c000;
	state->m_mem20000 = rom + 0x20000;
	state->m_mem24000 = rom + 0x24000;
	state->m_mem28000 = rom + 0x28000;
	state->m_mem2c000 = rom + 0x2c000;
}

DRIVER_INIT_MEMBER(c16_state,c16)
{
	c16_common_driver_init(machine());

	m_sidcard = 0;
	m_pal = 1;
}

DRIVER_INIT_MEMBER(c16_state,plus4)
{
	c16_common_driver_init(machine());

	m_sidcard = 0;
	m_pal = 0;
}

DRIVER_INIT_MEMBER(c16_state,c16sid)
{
	c16_common_driver_init(machine());

	m_sidcard = 1;
	m_pal = 1;
}

DRIVER_INIT_MEMBER(c16_state,plus4sid)
{
	c16_common_driver_init(machine());

	m_sidcard = 1;
	m_pal = 0;
}

MACHINE_RESET( c16 )
{
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	c16_state *state = machine.driver_data<c16_state>();
	UINT8 *ram = state->m_messram->pointer();
	UINT32 ram_size = state->m_messram->size();

	memset(state->m_keyline, 0xff, ARRAY_LENGTH(state->m_keyline));

	state->m_lowrom = 0;
	state->m_highrom = 0;
	state->m_old_level = 0;
	state->m_port6529 = 0;

	if (state->m_pal)
	{
		state->membank("bank1")->set_base(ram + (0x4000 % ram_size));

		state->membank("bank5")->set_base(ram + (0x4000 % ram_size));
		state->membank("bank6")->set_base(ram + (0x8000 % ram_size));
		state->membank("bank7")->set_base(ram + (0xc000 % ram_size));

		space->install_write_bank(0xff20, 0xff3d,"bank10");
		space->install_write_bank(0xff40, 0xffff, "bank11");
		state->membank("bank10")->set_base(ram + (0xff20 % ram_size));
		state->membank("bank11")->set_base(ram + (0xff40 % ram_size));
	}
	else
	{
		space->install_write_bank(0x4000, 0xfcff, "bank10");
		state->membank("bank10")->set_base(ram + (0x4000 % ram_size));
	}
}

#if 0
// FIXME
// in very old MESS versions, we had these handlers to enable SID writes to 0xd400.
// would a real SID Card allow for this? If not, this should be removed completely
static WRITE8_HANDLER( c16_sidcart_16k )
{
	c16_state *state = space->machine().driver_data<c16_state>();
	UINT8 *ram = state->m_messram->pointer();

	ram[0x1400 + offset] = data;
	ram[0x5400 + offset] = data;
	ram[0x9400 + offset] = data;
	ram[0xd400 + offset] = data;

	sid6581_w(state->m_sid, offset, data);
}

static WRITE8_HANDLER( c16_sidcart_64k )
{
	c16_state *state = space->machine().driver_data<c16_state>();

	state->m_messram->pointer()[0xd400 + offset] = data;

	sid6581_w(state->m_sid, offset, data);
}

static TIMER_CALLBACK( c16_sidhack_tick )
{
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	c16_state *state = space->machine().driver_data<c16_state>();

	if (machine.root_device().ioport("SID")->read_safe(0x00) & 0x02)
	{
		if (state->m_pal)
			space->install_legacy_write_handler(0xd400, 0xd41f, FUNC(c16_sidcart_16k));
		else
			space->install_legacy_write_handler(0xd400, 0xd41f, FUNC(c16_sidcart_64k));
	}
	else
	{
		space->unmap_write(0xd400, 0xd41f);
	}
}
#endif

static TIMER_CALLBACK( c16_sidcard_tick )
{
	c16_state *state = machine.driver_data<c16_state>();
	address_space *space = state->m_maincpu->memory().space(AS_PROGRAM);

	if (machine.root_device().ioport("SID")->read_safe(0x00) & 0x01)
		space->install_legacy_readwrite_handler(*state->m_sid, 0xfe80, 0xfe9f, FUNC(sid6581_r), FUNC(sid6581_w));
	else
		space->install_legacy_readwrite_handler(*state->m_sid, 0xfd40, 0xfd5f, FUNC(sid6581_r), FUNC(sid6581_w));
}

INTERRUPT_GEN( c16_frame_interrupt )
{
	c16_state *state = device->machine().driver_data<c16_state>();
	int value, i;
	static const char *const c16ports[] = { "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7" };

	/* Lines 0-7 : common keyboard */
	for (i = 0; i < 8; i++)
	{
		value = 0xff;
		value &= ~device->machine().root_device().ioport(c16ports[i])->read();

		/* Shift Lock is mapped on Left/Right Shift */
		if ((i == 1) && (device->machine().root_device().ioport("SPECIAL")->read() & 0x80))
			value &= ~0x80;

		state->m_keyline[i] = value;
	}

	if (device->machine().root_device().ioport("CTRLSEL")->read() & 0x01)
	{
		value = 0xff;
		if (device->machine().root_device().ioport("JOY0")->read() & 0x10)			/* Joypad1_Button */
			{
				if (device->machine().root_device().ioport("SPECIAL")->read() & 0x40)
					value &= ~0x80;
				else
					value &= ~0x40;
			}

		value &= ~(device->machine().root_device().ioport("JOY0")->read() & 0x0f);	/* Other Inputs Joypad1 */

		if (device->machine().root_device().ioport("SPECIAL")->read() & 0x40)
			state->m_keyline[9] = value;
		else
			state->m_keyline[8] = value;
	}

	if (device->machine().root_device().ioport("CTRLSEL")->read() & 0x10)
	{
		value = 0xff;
		if (device->machine().root_device().ioport("JOY1")->read() & 0x10)			/* Joypad2_Button */
			{
				if (device->machine().root_device().ioport("SPECIAL")->read() & 0x40)
					value &= ~0x40;
				else
					value &= ~0x80;
			}

		value &= ~(device->machine().root_device().ioport("JOY1")->read() & 0x0f);	/* Other Inputs Joypad2 */

		if (device->machine().root_device().ioport("SPECIAL")->read() & 0x40)
			state->m_keyline[8] = value;
		else
			state->m_keyline[9] = value;
	}

	ted7360_frame_interrupt_gen(state->m_ted7360);

	if (state->m_sidcard)
	{
		/* if we are emulating the SID card, check which memory area should be accessed */
		device->machine().scheduler().timer_set(attotime::zero, FUNC(c16_sidcard_tick));
#if 0
		/* if we are emulating the SID card, check if writes to 0xd400 have been enabled */
		device->machine().scheduler().timer_set(attotime::zero, FUNC(c16_sidhack_tick));
#endif
	}

	set_led_status(device->machine(), 1, device->machine().root_device().ioport("SPECIAL")->read() & 0x80 ? 1 : 0);		/* Shift Lock */
	set_led_status(device->machine(), 0, device->machine().root_device().ioport("SPECIAL")->read() & 0x40 ? 1 : 0);		/* Joystick Swap */
}


/***********************************************

    C16 Cartridges

***********************************************/

static void plus4_software_list_cartridge_load(device_image_interface &image)
{
	UINT8 *mem = image.device().machine().root_device().memregion("maincpu")->base();

	size_t size = image.get_software_region_length("c1l");
	if (size)
		memcpy(mem + 0x20000, image.get_software_region("c1l"), size);

	size = image.get_software_region_length("c1h");
	if (size)
		memcpy(mem + 0x24000, image.get_software_region("c1h"), size);

	size = image.get_software_region_length("c2l");
	if (size)
		memcpy(mem + 0x28000, image.get_software_region("c2l"), size);

	size = image.get_software_region_length("c2h");
	if (size)
		memcpy(mem + 0x2c000, image.get_software_region("c2h"), size);
}

static int plus4_crt_load( device_image_interface &image )
{
	UINT8 *mem = image.device().machine().root_device().memregion("maincpu")->base();
	int size = image.length(), test;
	const char *filetype;
	int address = 0;

	/* magic lowrom at offset 7: $43 $42 $4d */
	/* if at offset 6 stands 1 it will immediatly jumped to offset 0 (0x8000) */
	static const unsigned char magic[] = {0x43, 0x42, 0x4d};
	unsigned char buffer[sizeof (magic)];

	image.fseek(7, SEEK_SET);
	image.fread( buffer, sizeof (magic));
	image.fseek(0, SEEK_SET);

	/* Check if our cart has the magic string, and set its loading address */
	if (!memcmp(buffer, magic, sizeof (magic)))
		address = 0x20000;

	/* Give a loading address to non .bin / non .rom carts as well */
	filetype = image.filetype();

	/* We would support .hi and .lo files, but currently I'm not sure where to load them.
       We simply load them at 0x20000 at this stage, even if it's probably wrong!
       It could also well be that they both need to be loaded at the same time, but this
       is now impossible since I reduced to 1 the number of cart slots.
       More investigations are in order if any .hi, .lo dump would surface!              */
	if (!mame_stricmp(filetype, "hi"))
		address = 0x20000;	/* FIX ME! */

	else if (!mame_stricmp(filetype, "lo"))
		address = 0x20000;	/* FIX ME! */

	/* As a last try, give a reasonable loading address also to .bin/.rom without the magic string */
	else if (!address)
	{
		logerror("Cart %s does not contain the magic string: it may be loaded at the wrong memory address!\n", image.filename());
		address = 0x20000;
	}

	logerror("Loading cart %s at %.5x size:%.4x\n", image.filename(), address, size);

	/* Finally load the cart */
	test = image.fread( mem + address, size);

	if (test != size)
		return IMAGE_INIT_FAIL;

	return IMAGE_INIT_PASS;
}

static DEVICE_IMAGE_LOAD( c16_cart )
{
	int result = IMAGE_INIT_PASS;

	if (image.software_entry() != NULL)
	{
		plus4_software_list_cartridge_load(image);
	}
	else
	{
		result = plus4_crt_load(image);
	}

	return result;
}

MACHINE_CONFIG_FRAGMENT( c16_cartslot )
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("bin,rom,hi,lo")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("plus4_cart")
	MCFG_CARTSLOT_LOAD(c16_cart)
	MCFG_SOFTWARE_LIST_ADD("cart_list", "plus4_cart")
MACHINE_CONFIG_END
