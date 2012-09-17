/***************************************************************************

    a7800.c

    Machine file to handle emulation of the Atari 7800.

     5-Nov-2003 npwoods     Cleanups

    14-May-2002 kubecj      Fixed Fatal Run - adding simple riot timer helped.
                            maybe someone with knowledge should add full fledged
                            riot emulation?

    13-May-2002 kubecj      Fixed a7800_cart_type not to be too short ;-D
                            fixed for loading of bank6 cart (uh, I hope)
                            fixed for loading 64k supercarts
                            fixed for using PAL bios
                            cart not needed when in PAL mode
                            added F18 Hornet bank select type
                            added Activision bank select type
    19-Feb-2010 DanB        Added return values for TIA collision registers
***************************************************************************/

#include "emu.h"
#include "includes/a7800.h"
#include "cpu/m6502/m6502.h"
#include "sound/tiasound.h"
#include "machine/6532riot.h"
#include "sound/pokey.h"
#include "sound/tiaintf.h"



/* local */



/***************************************************************************
    6532 RIOT
***************************************************************************/

static READ8_DEVICE_HANDLER( riot_joystick_r )
{
	return device->machine().root_device().ioport("joysticks")->read();
}

static READ8_DEVICE_HANDLER( riot_console_button_r )
{
	return device->machine().root_device().ioport("console_buttons")->read();
}

static WRITE8_DEVICE_HANDLER( riot_button_pullup_w )
{
	a7800_state *state = device->machine().driver_data<a7800_state>();
	state->m_p1_one_button = data & 0x04; // pin 6 of the controller port is held high by the riot chip when reading two-button controllers (from schematic)
	state->m_p2_one_button = data & 0x10;
}

const riot6532_interface a7800_r6532_interface =
{
	DEVCB_HANDLER(riot_joystick_r),
	DEVCB_HANDLER(riot_console_button_r),
	DEVCB_NULL,
	DEVCB_HANDLER(riot_button_pullup_w),
	DEVCB_NULL
};


/***************************************************************************
    DRIVER INIT
***************************************************************************/

static void a7800_driver_init(running_machine &machine, int ispal, int lines)
{
	a7800_state *state = machine.driver_data<a7800_state>();
	address_space* space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	state->m_ROM = state->memregion("maincpu")->base();
	state->m_ispal = ispal;
	state->m_lines = lines;
	state->m_p1_one_button = 1;
	state->m_p2_one_button = 1;

	/* standard banks */
	state->membank("bank5")->set_base(&state->m_ROM[0x2040]);		/* RAM0 */
	state->membank("bank6")->set_base(&state->m_ROM[0x2140]);		/* RAM1 */
	state->membank("bank7")->set_base(&state->m_ROM[0x2000]);		/* MAINRAM */

	/* Brutal hack put in as a consequence of new memory system; fix this */
	space->install_readwrite_bank(0x0480, 0x04FF,"bank10");
	state->membank("bank10")->set_base(state->m_ROM + 0x0480);
	space->install_readwrite_bank(0x1800, 0x27FF, "bank11");
	state->membank("bank11")->set_base(state->m_ROM + 0x1800);
}


DRIVER_INIT_MEMBER(a7800_state,a7800_ntsc)
{
	a7800_driver_init(machine(), FALSE, 262);
}


DRIVER_INIT_MEMBER(a7800_state,a7800_pal)
{
	a7800_driver_init(machine(), TRUE, 312);
}


void a7800_state::machine_reset()
{
	UINT8 *memory;
	address_space* space = machine().device("maincpu")->memory().space(AS_PROGRAM);

	m_ctrl_lock = 0;
	m_ctrl_reg = 0;
	m_maria_flag = 0;

	/* set banks to default states */
	memory = memregion("maincpu")->base();
	membank("bank1")->set_base(memory + 0x4000 );
	membank("bank2")->set_base(memory + 0x8000 );
	membank("bank3")->set_base(memory + 0xA000 );
	membank("bank4")->set_base(memory + 0xC000 );

	/* pokey cartridge */
	if (m_cart_type & 0x01)
	{
		pokey_device *pokey = machine().device<pokey_device>("pokey");
		space->install_readwrite_handler(0x4000, 0x7FFF, read8_delegate(FUNC(pokey_device::read),pokey), write8_delegate(FUNC(pokey_device::write),pokey));
	}
}


/***************************************************************************
    CARTRIDGE HANDLING
***************************************************************************/

#define MBANK_TYPE_ATARI 0x0000
#define MBANK_TYPE_ACTIVISION 0x0100
#define MBANK_TYPE_ABSOLUTE 0x0200

/*  Header format
0     Header version     - 1 byte
1..16  "ATARI7800     "  - 16 bytes
17..48 Cart title        - 32 bytes
49..52 data length      - 4 bytes
53..54 cart type          - 2 bytes
    bit 0 0x01 - pokey cart
    bit 1 0x02 - supercart bank switched
    bit 2 0x04 - supercart RAM at $4000
    bit 3 0x08 - additional state->m_ROM at $4000

    bit 8-15 - Special
        0 = Normal cart
        1 = Absolute (F18 Hornet)
        2 = Activision

55   controller 1 type  - 1 byte
56   controller 2 type  - 1 byte
    0 = None
    1 = Joystick
    2 = Light Gun
57  0 = NTSC/1 = PAL

100..127 "ACTUAL CART DATA STARTS HERE" - 28 bytes

Versions:
    Version 0: Initial release
    Version 1: Added PAL/NTSC bit. Added Special cart byte.
               Changed 53 bit 2, added bit 3

*/
void a7800_partialhash(hash_collection &dest, const unsigned char *data,
	unsigned long length, const char *functions)
{
	if (length <= 128)
		return;
	dest.compute(&data[128], length - 128, functions);
}


static int a7800_verify_cart(char header[128])
{
	const char* tag = "ATARI7800";

	if( strncmp( tag, header + 1, 9 ) )
	{
		logerror("Not a valid A7800 image\n");
		return IMAGE_VERIFY_FAIL;
	}

	logerror("returning ID_OK\n");
	return IMAGE_VERIFY_PASS;
}


DEVICE_START( a7800_cart )
{
	a7800_state *state = device->machine().driver_data<a7800_state>();
	UINT8 *memory = state->memregion("maincpu")->base();

	state->m_bios_bkup = NULL;
	state->m_cart_bkup = NULL;

	/* Allocate memory for BIOS bank switching */
	state->m_bios_bkup = auto_alloc_array_clear(device->machine(), UINT8, 0x4000);
	state->m_cart_bkup = auto_alloc_array(device->machine(), UINT8, 0x4000);

	/* save the BIOS so we can switch it in and out */
	memcpy( state->m_bios_bkup, memory + 0xC000, 0x4000 );

	/* Initialize cart area to "no data" */
	memset( state->m_cart_bkup, 0xFF, 0x4000 );

	/* defaults for PAL bios without cart */
	state->m_cart_type = 0;
	state->m_stick_type = 1;
}

struct a7800_pcb
{
	const char *pcb_name;
	UINT16     type;
};

// sketchy support for a7800 cart types
// TODO: proper emulation of the banking based on xml
// (and on the real cart layout!)
static const a7800_pcb pcb_list[] =
{
	{ "ABSOLUTE", MBANK_TYPE_ABSOLUTE },
	{ "ACTIVISION", MBANK_TYPE_ACTIVISION },
	{ "TYPE-0", 0x0 },
	{ "TYPE-1", 0x1 },
	{ "TYPE-2", 0x2 },
	{ "TYPE-3", 0x3 },
	{ "TYPE-6", 0x6 },
	{ "TYPE-A", 0xa },
	{ 0 }
};

static UINT16 a7800_get_pcb_id(const char *pcb)
{
	int	i;

	for (i = 0; i < ARRAY_LENGTH(pcb_list); i++)
	{
		if (!mame_stricmp(pcb_list[i].pcb_name, pcb))
			return pcb_list[i].type;
	}

	return 0;
}

DEVICE_IMAGE_LOAD( a7800_cart )
{
	a7800_state *state = image.device().machine().driver_data<a7800_state>();
	UINT32 len = 0, start = 0;
	unsigned char header[128];
	UINT8 *memory = image.device().machine().root_device().memregion("maincpu")->base();
	const char	*pcb_name;

	// detect cart type either from xml or from header
	if (image.software_entry() == NULL)
	{
		/* Load and decode the header */
		image.fread(header, 128);

		/* Check the cart */
		if( a7800_verify_cart((char *)header) == IMAGE_VERIFY_FAIL)
			return IMAGE_INIT_FAIL;

		len =(header[49] << 24) |(header[50] << 16) |(header[51] << 8) | header[52];
		state->m_cart_size = len;

		state->m_cart_type =(header[53] << 8) | header[54];
		state->m_stick_type = header[55];
		logerror("Cart type: %x\n", state->m_cart_type);

		/* For now, if game support stick and gun, set it to stick */
		if (state->m_stick_type == 3)
			state->m_stick_type = 1;
	}
	else
	{
		len = image.get_software_region_length("rom");
		state->m_cart_size = len;
		// TODO: add stick/gun support to xml!
		state->m_stick_type = 1;
		if ((pcb_name = image.get_feature("pcb_type")) == NULL)
			state->m_cart_type = 0;
		else
			state->m_cart_type = a7800_get_pcb_id(pcb_name);
	}

	if (state->m_cart_type == 0 || state->m_cart_type == 1)
	{
		/* Normal Cart */
		start = 0x10000 - len;
		state->m_cartridge_rom = memory + start;
		if (image.software_entry() == NULL)
			image.fread(state->m_cartridge_rom, len);
		else
			memcpy(state->m_cartridge_rom, image.get_software_region("rom"), len);
	}
	else if (state->m_cart_type & 0x02)
	{
		/* Super Cart */
		/* Extra ROM at $4000 */
		if (state->m_cart_type & 0x08)
		{
			if (image.software_entry() == NULL)
				image.fread(memory + 0x4000, 0x4000);
			else
				memcpy(memory + 0x4000, image.get_software_region("rom"), 0x4000);
			len -= 0x4000;
			start = 0x4000;
		}

		state->m_cartridge_rom = memory + 0x10000;
		if (image.software_entry() == NULL)
			image.fread(state->m_cartridge_rom, len);
		else
			memcpy(state->m_cartridge_rom, image.get_software_region("rom") + start, len);

		/* bank 0 */
		memcpy(memory + 0x8000, memory + 0x10000, 0x4000);

		/* last bank */
		memcpy(memory + 0xC000, memory + 0x10000 + len - 0x4000, 0x4000);

		/* fixed 2002/05/13 kubecj
            there was 0x08, I added also two other cases.
            Now, it loads bank n-2 to $4000 if it's empty.
        */

		/* bank n-2 */
		if (!(state->m_cart_type & 0x0d))
		{
			memcpy(memory + 0x4000, memory + 0x10000 + len - 0x8000, 0x4000);
		}
	}
	else if (state->m_cart_type == MBANK_TYPE_ABSOLUTE)
	{
		/* F18 Hornet */

		logerror("Cart type: %x Absolute\n",state->m_cart_type);

		state->m_cartridge_rom = memory + 0x10000;
		if (image.software_entry() == NULL)
			image.fread(state->m_cartridge_rom, len);
		else
			memcpy(state->m_cartridge_rom, image.get_software_region("rom") + start, len);

		/* bank 0 */
		memcpy(memory + 0x4000, memory + 0x10000, 0x4000);

		/* last bank */
		memcpy(memory + 0x8000, memory + 0x18000, 0x8000);
	}
	else if (state->m_cart_type == MBANK_TYPE_ACTIVISION)
	{
		/* Activision */

		logerror("Cart type: %x Activision\n",state->m_cart_type);

		state->m_cartridge_rom = memory + 0x10000;
		if (image.software_entry() == NULL)
			image.fread(state->m_cartridge_rom, len);
		else
			memcpy(state->m_cartridge_rom, image.get_software_region("rom") + start, len);

		/* bank 0 */
		memcpy(memory + 0xa000, memory + 0x10000, 0x4000);

		/* bank6 hi */
		memcpy(memory + 0x4000, memory + 0x2a000, 0x2000);

		/* bank6 lo */
		memcpy(memory + 0x6000, memory + 0x28000, 0x2000);

		/* bank7 hi */
		memcpy(memory + 0x8000, memory + 0x2e000, 0x2000);

		/* bank7 lo */
		memcpy(memory + 0xe000, memory + 0x2c000, 0x2000);

	}

	memcpy(state->m_cart_bkup, memory + 0xc000, 0x4000);
	memcpy(memory + 0xc000, state->m_bios_bkup, 0x4000);
	return IMAGE_INIT_PASS;
}


WRITE8_MEMBER(a7800_state::a7800_RAM0_w)
{
	m_ROM[0x2040 + offset] = data;
	m_ROM[0x40 + offset] = data;
}


WRITE8_MEMBER(a7800_state::a7800_cart_w)
{
	UINT8 *memory = memregion("maincpu")->base();

	if(offset < 0x4000)
	{
		if(m_cart_type & 0x04)
		{
			m_ROM[0x4000 + offset] = data;
		}
		else if(m_cart_type & 0x01)
		{
			pokey_device *pokey = machine().device<pokey_device>("pokey");
			pokey->write(space, offset, data);
		}
		else
		{
			logerror("Undefined write A: %x",offset + 0x4000);
		}
	}

	if(( m_cart_type & 0x02 ) &&( offset >= 0x4000 ) )
	{
		/* fix for 64kb supercart */
		if( m_cart_size == 0x10000 )
		{
			data &= 0x03;
		}
		else
		{
			data &= 0x07;
		}
		membank("bank2")->set_base(memory + 0x10000 + (data << 14));
		membank("bank3")->set_base(memory + 0x12000 + (data << 14));
	/*  logerror("BANK SEL: %d\n",data); */
	}
	else if(( m_cart_type == MBANK_TYPE_ABSOLUTE ) &&( offset == 0x4000 ) )
	{
		/* F18 Hornet */
		/*logerror( "F18 BANK SEL: %d\n", data );*/
		if( data & 1 )
		{
			membank("bank1")->set_base(memory + 0x10000 );
		}
		else if( data & 2 )
		{
			membank("bank1")->set_base(memory + 0x14000 );
		}
	}
	else if(( m_cart_type == MBANK_TYPE_ACTIVISION ) &&( offset >= 0xBF80 ) )
	{
		/* Activision */
		data = offset & 7;

		/*logerror( "Activision BANK SEL: %d\n", data );*/

		membank("bank3")->set_base(memory + 0x10000 + ( data << 14 ) );
		membank("bank4")->set_base(memory + 0x12000 + ( data << 14 ) );
	}
}


/***************************************************************************
    TIA
***************************************************************************/

READ8_MEMBER(a7800_state::a7800_TIA_r)
{
	switch(offset & 0x0f)
	{
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		/* Even though the 7800 doesn't use the TIA graphics the collision registers should
           still return a reasonable value */
			return 0x00;
		case 0x08:
			  return((ioport("buttons")->read() & 0x02) << 6);
		case 0x09:
			  return((ioport("buttons")->read() & 0x08) << 4);
		case 0x0A:
			  return((ioport("buttons")->read() & 0x01) << 7);
		case 0x0B:
			  return((ioport("buttons")->read() & 0x04) << 5);
		case 0x0c:
			if(((ioport("buttons")->read() & 0x08) ||(ioport("buttons")->read() & 0x02)) && m_p1_one_button)
				return 0x00;
			else
				return 0x80;
		case 0x0d:
			if(((ioport("buttons")->read() & 0x01) ||(ioport("buttons")->read() & 0x04)) && m_p2_one_button)
				return 0x00;
			else
				return 0x80;
		default:
			logerror("undefined TIA read %x\n",offset);

	}
	return 0xFF;
}


WRITE8_MEMBER(a7800_state::a7800_TIA_w)
{
	switch(offset) {
	case 0x01:
		if(data & 0x01)
		{
			m_maria_flag=1;
		}
		if(!m_ctrl_lock)
		{
			m_ctrl_lock = data & 0x01;
			m_ctrl_reg = data;

			if (data & 0x04)
				memcpy( m_ROM + 0xC000, m_cart_bkup, 0x4000 );
			else
				memcpy( m_ROM + 0xC000, m_bios_bkup, 0x4000 );
		}
		break;
	}
	tia_sound_w(machine().device("tia"), space, offset, data);
	m_ROM[offset] = data;
}
