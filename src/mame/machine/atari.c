/***************************************************************************

    Atari 400/800

    Machine driver

    Juergen Buchmueller, June 1998

***************************************************************************/

#include "driver.h"
#include "cpu/m6502/m6502.h"
#include "includes/atari.h"
#include "sound/pokey.h"
#include "machine/6821pia.h"
#include "sound/dac.h"
#include "video/gtia.h"

#define VERBOSE_POKEY	0
#define VERBOSE_SERIAL	0
#define VERBOSE_TIMERS	0

//#define LOG(x) if (errorlog) fprintf x

static int atari = 0;
#define ATARI_5200	0
#define ATARI_400	1
#define ATARI_800	2
#define ATARI_600XL 3
#define ATARI_800XL 4

#ifdef MESS
static int a800_cart_loaded = 0;
static int a800_cart_is_16k = 0;
#endif

static void a800xl_mmu(running_machine *machine, UINT8 new_mmu);
static void a600xl_mmu(running_machine *machine, UINT8 new_mmu);

static void pokey_reset(running_machine *machine);

void atari_interrupt_cb(const device_config *device, int mask)
{

	if (VERBOSE_POKEY)
	{
		if (mask & 0x80)
			logerror("atari interrupt_cb BREAK\n");
		if (mask & 0x40)
			logerror("atari interrupt_cb KBCOD\n");
	}
	if (VERBOSE_SERIAL)
	{
		if (mask & 0x20)
			logerror("atari interrupt_cb SERIN\n");
		if (mask & 0x10)
			logerror("atari interrupt_cb SEROR\n");
		if (mask & 0x08)
			logerror("atari interrupt_cb SEROC\n");
	}
	if (VERBOSE_TIMERS)
	{
		if (mask & 0x04)
			logerror("atari interrupt_cb TIMR4\n");
		if (mask & 0x02)
			logerror("atari interrupt_cb TIMR2\n");
		if (mask & 0x01)
			logerror("atari interrupt_cb TIMR1\n");
	}

	cpu_set_input_line(device->machine->cpu[0], 0, HOLD_LINE);
}

/**************************************************************
 *
 * PIA interface
 *
 **************************************************************/

static READ8_DEVICE_HANDLER(atari_pia_pa_r)
{
	return atari_input_disabled() ? 0xFF : input_port_read_safe(device->machine, "djoy_0_1", 0);
}

static READ8_DEVICE_HANDLER(atari_pia_pb_r)
{
	return atari_input_disabled() ? 0xFF : input_port_read_safe(device->machine, "djoy_2_3", 0);
}

static WRITE8_DEVICE_HANDLER(a600xl_pia_pb_w) { a600xl_mmu(device->machine, data); }
static WRITE8_DEVICE_HANDLER(a800xl_pia_pb_w) { a800xl_mmu(device->machine, data); }

#ifndef MESS
WRITE_LINE_DEVICE_HANDLER(atari_pia_cb2_w) { }
#endif

const pia6821_interface atari_pia_interface =
{
	DEVCB_HANDLER(atari_pia_pa_r),		/* port A in */
	DEVCB_HANDLER(atari_pia_pb_r),	/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_LINE(atari_pia_cb2_w),		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

const pia6821_interface a600xl_pia_interface =
{
	DEVCB_HANDLER(atari_pia_pa_r),		/* port A in */
	DEVCB_HANDLER(atari_pia_pb_r),	/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_HANDLER(a600xl_pia_pb_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_LINE(atari_pia_cb2_w),		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

const pia6821_interface a800xl_pia_interface =
{
	DEVCB_HANDLER(atari_pia_pa_r),		/* port A in */
	DEVCB_HANDLER(atari_pia_pb_r),	/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_HANDLER(a800xl_pia_pb_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_LINE(atari_pia_cb2_w),		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};




/**************************************************************
 *
 * Reset hardware
 *
 **************************************************************/

void a600xl_mmu(running_machine *machine, UINT8 new_mmu)
{
	read8_space_func rbank2;
	write8_space_func wbank2;

	/* check if self-test ROM changed */
	if ( new_mmu & 0x80 )
	{
		logerror("%s MMU SELFTEST RAM\n", machine->gamedrv->name);
		rbank2 = SMH_NOP;
		wbank2 = SMH_NOP;
	}
	else
	{
		logerror("%s MMU SELFTEST ROM\n", machine->gamedrv->name);
		rbank2 = SMH_BANK2;
		wbank2 = SMH_UNMAP;
	}
	memory_install_readwrite8_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0x5000, 0x57ff, 0, 0, rbank2, wbank2);
	if (rbank2 == SMH_BANK2)
		memory_set_bankptr(machine, 2, memory_region(machine, "maincpu") + 0x5000);
}

void a800xl_mmu(running_machine *machine, UINT8 new_mmu)
{
	read8_space_func rbank1, rbank2, rbank3, rbank4;
	write8_space_func wbank1, wbank2, wbank3, wbank4;
	UINT8 *base1, *base2, *base3, *base4;

	/* check if memory C000-FFFF changed */
	if( new_mmu & 0x01 )
	{
		logerror("%s MMU BIOS ROM\n", machine->gamedrv->name);
		rbank3 = SMH_BANK3;
		wbank3 = SMH_UNMAP;
		base3 = memory_region(machine, "maincpu") + 0x14000;  /* 8K lo BIOS */
		rbank4 = SMH_BANK4;
		wbank4 = SMH_UNMAP;
		base4 = memory_region(machine, "maincpu") + 0x15800;  /* 4K FP ROM + 8K hi BIOS */
	}
	else
	{
		logerror("%s MMU BIOS RAM\n", machine->gamedrv->name);
		rbank3 = SMH_BANK3;
		wbank3 = SMH_BANK3;
		base3 = memory_region(machine, "maincpu") + 0x0c000;  /* 8K RAM */
		rbank4 = SMH_BANK4;
		wbank4 = SMH_BANK4;
		base4 = memory_region(machine, "maincpu") + 0x0d800;  /* 4K RAM + 8K RAM */
	}
	memory_install_readwrite8_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0xc000, 0xcfff, 0, 0, rbank3, wbank3);
	memory_install_readwrite8_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0xd800, 0xffff, 0, 0, rbank4, wbank4);
	memory_set_bankptr(machine, 3, base3);
	memory_set_bankptr(machine, 4, base4);

	/* check if BASIC changed */
	if( new_mmu & 0x02 )
	{
		logerror("%s MMU BASIC RAM\n", machine->gamedrv->name);
		rbank1 = SMH_BANK1;
		wbank1 = SMH_BANK1;
		base1 = memory_region(machine, "maincpu") + 0x0a000;  /* 8K RAM */
	}
	else
	{
		logerror("%s MMU BASIC ROM\n", machine->gamedrv->name);
		rbank1 = SMH_BANK1;
		wbank1 = SMH_UNMAP;
		base1 = memory_region(machine, "maincpu") + 0x10000;  /* 8K BASIC */
	}
	memory_install_readwrite8_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0xa000, 0xbfff, 0, 0, rbank1, wbank1);
	memory_set_bankptr(machine, 1, base1);

	/* check if self-test ROM changed */
	if( new_mmu & 0x80 )
	{
		logerror("%s MMU SELFTEST RAM\n", machine->gamedrv->name);
		rbank2 = SMH_BANK2;
		wbank2 = SMH_BANK2;
		base2 = memory_region(machine, "maincpu") + 0x05000;  /* 0x0800 bytes */
	}
	else
	{
		logerror("%s MMU SELFTEST ROM\n", machine->gamedrv->name);
		rbank2 = SMH_BANK2;
		wbank2 = SMH_UNMAP;
		base2 = memory_region(machine, "maincpu") + 0x15000;  /* 0x0800 bytes */
	}
	memory_install_readwrite8_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0x5000, 0x57ff, 0, 0, rbank2, wbank2);
	memory_set_bankptr(machine, 2, base2);
}



/**************************************************************
 *
 * Keyboard
 *
 **************************************************************/

#define AKEY_BREAK		0x03	/* this not really a scancode */
#define AKEY_NONE		0x09

/**************************************************************

	Keyboard inputs use 6bits to read the 64keys in the key matrix.
	We currently read the key matrix by lines and convert the input
	to the value expected by the POKEY (see the code below to 
	determine atari_code values).

	K2,K1,K0 | 000 | 001 | 010 | 011 | 100 | 101 | 110 | 111 |
	K5,K4,K3
	----------------------------------------------------------
		000	 |  L  |  J  |  ;  | (*) |     |  K  |  +  |  *  |
		001	 |  O  |     |  P  |  U  | Ret |  I  |  -  |  =  |
		010	 |  V  |     |  C  |     |     |  B  |  X  |  Z  |
		011	 |  4  |     |  3  |  6  | Esc |  5  |  2  |  1  |
		100	 |  ,  | Spc |  .  |  N  |     |  M  |  /  |Atari|
		101	 |  R  |     |  E  |  Y  | Tab |  T  |  W  |  Q  |
		110	 |  9  |     |  0  |  7  |Bkspc|  8  |  <  |  >  |
		111	 |  F  |  H  |  D  |     | Caps|  G  |  S  |  A  |

	(*) We use this value to read Break, but in fact it would be read
	    in KR2 bit. This has to be properly implemented for later
		Atari systems because here we would have F1.

	To Do: investigate implementation of KR2 to read accurately Break, 
	Shift and Control keys.

 **************************************************************/

void a800_handle_keyboard(running_machine *machine)
{
	const device_config *pokey = devtag_get_device(machine, "pokey");
	static int atari_last = 0xff;
	int atari_code, count, ipt, i;
	static const char *const tag[] = { "keyboard_0", "keyboard_1", "keyboard_2", "keyboard_3",
										"keyboard_4", "keyboard_5", "keyboard_6", "keyboard_7" };

	/* check keyboard */
	for( i = 0; i < 8; i++ )
	{
		ipt = input_port_read_safe(machine, tag[i], 0);

		if( ipt )
		{
			count = 0;
			while(ipt / 2)
			{
				ipt = ipt/2;
				count++;
			}

			atari_code = i*8 + count;

			/* SHIFT */
			if(input_port_read_safe(machine, "fake", 0) & 0x01)
				atari_code |= 0x40;

			/* CTRL */
			if(input_port_read_safe(machine, "fake", 0) & 0x02)
				atari_code |= 0x80;

			if( atari_code != AKEY_NONE )
			{
				if( atari_code == atari_last )
					return;
				atari_last = atari_code;

				if( (atari_code & 0x3f) == AKEY_BREAK )
				{
					pokey_break_w(pokey, atari_code & 0x40);
					return;
				}

				pokey_kbcode_w(pokey, atari_code, 1);
				return;
			}
		}

	}
	/* remove key pressed status bit from skstat */
	pokey_kbcode_w(pokey, AKEY_NONE, 0);
	atari_last = AKEY_NONE;
}

/**************************************************************
 *
 * Keypad
 *
 **************************************************************/

/**************************************************************

	A5200 keypad inputs use 4bits to read the 16keys in the key 
	matrix. We currently read the key matrix by lines and convert 
	the input to the value expected by the POKEY (see the code 
	below to determine atari_code values).

	K2,K1,K0 | 00x | 01x | 10x | 11x |
	K5,K4,K3
	----------------------------------
		x00	 |     |  #  |  0  |  *  |
		x01	 |Reset|  9  |  8  |  7  |
		x10	 |Pause|  6  |  5  |  4  |
		x11	 |Start|  3  |  2  |  1  |

	K0 & K5 are ignored (we send them as 1, see the code below where
	we pass "(atari_code << 1) | 0x21" )

	To Do: investigate implementation of KR2 to read accurately the 
	secondary Fire button (primary read through GTIA).

 **************************************************************/

void a5200_handle_keypads(running_machine *machine)
{
	const device_config *pokey = devtag_get_device(machine, "pokey");
	static int atari_last = 0xff;
	int atari_code, count, ipt, i;
	static const char *const tag[] = { "keypad_0", "keypad_1", "keypad_2", "keypad_3" };

	/* check keypad */
	for( i = 0; i < 4; i++ )
	{
		ipt = input_port_read_safe(machine, tag[i], 0);

		if( ipt )
		{
			count = 0;
			while(ipt / 2)
			{
				ipt = ipt/2;
				count++;
			}

			atari_code = i*4 + count;

			if( atari_code == atari_last )
				return;
			atari_last = atari_code;

			if( atari_code == 0 )
			{
				pokey_break_w(pokey, atari_code & 0x40);
				return;
			}

			pokey_kbcode_w(pokey, (atari_code << 1) | 0x21, 1);
			return;
		}

	}

	/* check top button */
	if ((input_port_read(machine, "djoy_b") & 0x10) == 0)
	{
		if (atari_last == 0xfe)
			return;
		pokey_kbcode_w(pokey, 0x61, 1);
		//pokey_break_w(pokey, 0x40);
		atari_last = 0xfe;
		return;
	}
	else if (atari_last == 0xfe)
		pokey_kbcode_w(pokey, 0x21, 1);

	/* remove key pressed status bit from skstat */
	pokey_kbcode_w(pokey, 0xff, 0);
	atari_last = 0xff;
}

#ifdef MESS
DRIVER_INIT( atari )
{
	offs_t ram_top;
	offs_t ram_size;

	if (!strcmp(machine->gamedrv->name, "a400")
		|| !strcmp(machine->gamedrv->name, "a400pal")
		|| !strcmp(machine->gamedrv->name, "a800")
		|| !strcmp(machine->gamedrv->name, "a800pal")
		|| !strcmp(machine->gamedrv->name, "a800xl"))
	{
		ram_size = 0xA000;
	}
	else
	{
		ram_size = 0x8000;
	}

	/* install RAM */
	ram_top = MIN(mess_ram_size, ram_size) - 1;
	memory_install_read8_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM),
		0x0000, ram_top, 0, 0, SMH_BANK2);
	memory_install_write8_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM),
		0x0000, ram_top, 0, 0, SMH_BANK2);
	memory_set_bankptr(machine, 2, mess_ram);
}
#endif



/*************************************
 *
 *  Generic Atari Code
 *
 *************************************/

#ifdef MESS
static void a800_setbank(running_machine *machine, int n)
{
	void *read_addr;
	void *write_addr;
	UINT8 *mem = memory_region(machine, "maincpu");

	switch (n)
	{
		case 1:
			read_addr = &mem[0x10000];
			write_addr = NULL;
			break;
		default:
			if( atari <= ATARI_400 )
			{
				/* Atari 400 has no RAM here, so install the NOP handler */
				read_addr = NULL;
				write_addr = NULL;
			}
			else
			{
				read_addr = &mess_ram[0x08000];
				write_addr = &mess_ram[0x08000];
			}
			break;
	}

	memory_install_read8_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0x8000, 0xbfff, 0, 0,
		read_addr ? SMH_BANK1 : SMH_NOP);
	memory_install_write8_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0x8000, 0xbfff, 0, 0,
		write_addr ? SMH_BANK1 : SMH_NOP);
	if (read_addr)
		memory_set_bankptr(machine, 1, read_addr);
	if (write_addr)
		memory_set_bankptr(machine, 1, write_addr);
}
#endif



static void pokey_reset(running_machine *machine)
{
	const device_config *pokey = devtag_get_device(machine, "pokey");
	pokey_w(pokey,15,0);
}



static void cart_reset(running_machine *machine)
{
#ifdef MESS
	if (a800_cart_loaded)
		a800_setbank(machine, 1);
#endif /* MESS */
}



static UINT8 console_read(const address_space *space)
{
	return input_port_read(space->machine, "console");
}



static void console_write(const address_space *space, UINT8 data)
{
	const device_config *dac = devtag_get_device(space->machine, "dac");
	if (data & 0x08)
		dac_data_w(dac, -120);
	else
		dac_data_w(dac, +120);
}


static void _antic_reset(running_machine *machine)
{
	antic_reset();
}


static void atari_machine_start(running_machine *machine, int type, int has_cart)
{
	gtia_interface gtia_intf;

	atari = type;

	/* GTIA */
	memset(&gtia_intf, 0, sizeof(gtia_intf));
	if (input_port_by_tag(machine->portconfig, "console") != NULL)
		gtia_intf.console_read = console_read;
	if (devtag_get_device(machine, "dac") != NULL)
		gtia_intf.console_write = console_write;
	gtia_init(machine, &gtia_intf);

	/* pokey */
	add_reset_callback(machine, pokey_reset);

	/* ANTIC */
	add_reset_callback(machine, _antic_reset);

	/* cartridge */
	if (has_cart)
		add_reset_callback(machine, cart_reset);

#ifdef MESS
	{
		offs_t ram_top;
		offs_t ram_size;

		if (!strcmp(machine->gamedrv->name, "a400")
			|| !strcmp(machine->gamedrv->name, "a400pal")
			|| !strcmp(machine->gamedrv->name, "a800")
			|| !strcmp(machine->gamedrv->name, "a800pal")
			|| !strcmp(machine->gamedrv->name, "a800xl"))
		{
			ram_size = 0xA000;
		}
		else
		{
			ram_size = 0x8000;
		}

		/* install RAM */
		ram_top = MIN(mess_ram_size, ram_size) - 1;
		memory_install_read8_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM),
			0x0000, ram_top, 0, 0, SMH_BANK2);
		memory_install_write8_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM),
			0x0000, ram_top, 0, 0, SMH_BANK2);
		memory_set_bankptr(machine, 2, mess_ram);
	}
#endif /* MESS */

	/* save states */
	state_save_register_global_pointer(machine, ((UINT8 *) &antic.r), sizeof(antic.r));
	state_save_register_global_pointer(machine, ((UINT8 *) &antic.w), sizeof(antic.w));
}



/*************************************
 *
 *  Atari 400
 *  Atari 600XL
 *
 *************************************/

MACHINE_START( a400 )
{
	atari_machine_start(machine, ATARI_400, TRUE);
}

MACHINE_START( a600xl )
{
	atari_machine_start(machine, ATARI_600XL, TRUE);
}



/*************************************
 *
 *  Atari 800
 *
 *************************************/

MACHINE_START( a800 )
{
	atari_machine_start(machine, ATARI_800, TRUE);
}



#ifdef MESS
DEVICE_IMAGE_LOAD( a800_cart )
{
	UINT8 *mem = memory_region(image->machine, "maincpu");
	int size;

	/* load an optional (dual) cartridge (e.g. basic.rom) */
	if( strcmp(image->tag,"cart2") == 0 )
	{
		size = image_fread(image, &mem[0x12000], 0x2000);
		a800_cart_is_16k = (size == 0x2000);
		logerror("%s loaded right cartridge '%s' size 16K\n", image->machine->gamedrv->name, image_filename(image) );
	}
	else
	{
		size = image_fread(image, &mem[0x10000], 0x2000);
		a800_cart_loaded = size > 0x0000;
		size = image_fread(image, &mem[0x12000], 0x2000);
		a800_cart_is_16k = size > 0x2000;
		logerror("%s loaded left cartridge '%s' size %s\n", image->machine->gamedrv->name, image_filename(image) , (a800_cart_is_16k) ? "16K":"8K");
	}
	return INIT_PASS;
}

DEVICE_IMAGE_UNLOAD( a800_cart )
{
	if( strcmp(image->tag,"cart2") == 0 )
	{
		a800_cart_is_16k = 0;
		a800_setbank(image->machine, 1);
    }
	else
	{
		a800_cart_loaded = 0;
		a800_setbank(image->machine, 0);
    }
}
#endif



/*************************************
 *
 *  Atari 800XL
 *
 *************************************/

MACHINE_START( a800xl )
{
	atari_machine_start(machine, ATARI_800XL, TRUE);
}



#ifdef MESS
DEVICE_IMAGE_LOAD( a800xl_cart )
{
	UINT8 *mem = memory_region(image->machine, "maincpu");
	astring *fname;
	mame_file *basic_fp;
	file_error filerr;
	unsigned size;

	fname = astring_assemble_3(astring_alloc(), image->machine->gamedrv->name, PATH_SEPARATOR, "basic.rom");
	filerr = mame_fopen(SEARCHPATH_ROM, astring_c(fname), OPEN_FLAG_READ, &basic_fp);
	astring_free(fname);

	if (filerr != FILERR_NONE)
	{
		size = mame_fread(basic_fp, &mem[0x14000], 0x2000);
		if( size < 0x2000 )
		{
			logerror("%s image '%s' load failed (less than 8K)\n", image->machine->gamedrv->name, astring_c(fname));
			mame_fclose(basic_fp);
			return 2;
		}
	}

	/* load an optional (dual) cartidge (e.g. basic.rom) */
	if (filerr != FILERR_NONE)
	{
		{
			size = image_fread(image, &mem[0x14000], 0x2000);
			a800_cart_loaded = size / 0x2000;
			size = image_fread(image, &mem[0x16000], 0x2000);
			a800_cart_is_16k = size / 0x2000;
			logerror("%s loaded cartridge '%s' size %s\n",
					image->machine->gamedrv->name, image_filename(image), (a800_cart_is_16k) ? "16K":"8K");
		}
		mame_fclose(basic_fp);
	}

	return INIT_PASS;
}
#endif



/*************************************
 *
 *  Atari 5200 console
 *
 *************************************/

MACHINE_START( a5200 )
{
	atari_machine_start(machine, ATARI_800XL, FALSE);
}



#ifdef MESS
DEVICE_IMAGE_LOAD( a5200_cart )
{
	UINT8 *mem = memory_region(image->machine, "maincpu");
	int size;

	/* load an optional (dual) cartidge */
	size = image_fread(image, &mem[0x4000], 0x8000);
	if (size<0x8000) memmove(mem+0x4000+0x8000-size, mem+0x4000, size);
	// mirroring of smaller cartridges
	if (size <= 0x1000) memcpy(mem+0xa000, mem+0xb000, 0x1000);
	if (size <= 0x2000) memcpy(mem+0x8000, mem+0xa000, 0x2000);
	if (size <= 0x4000)
	{
		const char *info;
		memcpy(&mem[0x4000], &mem[0x8000], 0x4000);
		info = image_extrainfo(image);
		if (info!=NULL && strcmp(info, "A13MIRRORING")==0)
		{
			memcpy(&mem[0x8000], &mem[0xa000], 0x2000);
			memcpy(&mem[0x6000], &mem[0x4000], 0x2000);
		}
	}
	logerror("%s loaded cartridge '%s' size %dK\n",
		image->machine->gamedrv->name, image_filename(image) , size/1024);
	return INIT_PASS;
}

DEVICE_IMAGE_UNLOAD( a5200_cart )
{
	UINT8 *mem = memory_region(image->machine, "maincpu");
	/* zap the cartridge memory (again) */
	memset(&mem[0x4000], 0x00, 0x8000);
}
#endif

