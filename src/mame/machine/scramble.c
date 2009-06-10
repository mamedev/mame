/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "machine/8255ppi.h"
#include "includes/galaxold.h"

static UINT8 cavelon_bank;

static UINT8 security_2B_counter;

MACHINE_RESET( scramble )
{
	MACHINE_RESET_CALL(galaxold);

	if (cputag_get_cpu(machine, "audiocpu") != NULL)
		scramble_sh_init(machine);

  security_2B_counter = 0;
}

MACHINE_RESET( explorer )
{
	UINT8 *RAM = memory_region(machine, "maincpu");
	RAM[0x47ff] = 0; /* If not set, it doesn't reset after the 1st time */

	MACHINE_RESET_CALL(galaxold);
}


CUSTOM_INPUT( darkplnt_custom_r )
{
	static const UINT8 remap[] = {0x03, 0x02, 0x00, 0x01, 0x21, 0x20, 0x22, 0x23,
							  0x33, 0x32, 0x30, 0x31, 0x11, 0x10, 0x12, 0x13,
							  0x17, 0x16, 0x14, 0x15, 0x35, 0x34, 0x36, 0x37,
							  0x3f, 0x3e, 0x3c, 0x3d, 0x1d, 0x1c, 0x1e, 0x1f,
							  0x1b, 0x1a, 0x18, 0x19, 0x39, 0x38, 0x3a, 0x3b,
							  0x2b, 0x2a, 0x28, 0x29, 0x09, 0x08, 0x0a, 0x0b,
							  0x0f, 0x0e, 0x0c, 0x0d, 0x2d, 0x2c, 0x2e, 0x2f,
							  0x27, 0x26, 0x24, 0x25, 0x05, 0x04, 0x06, 0x07 };
	UINT8 val = input_port_read(field->port->machine, (const char *)param);

	return remap[val >> 2];
}

/* state of the security PAL (6J) */
static UINT8 xb;

static WRITE8_DEVICE_HANDLER( scramble_protection_w )
{
	xb = data;
}

static READ8_DEVICE_HANDLER( scramble_protection_r )
{
	switch (cpu_get_pc(cputag_get_cpu(device->machine, "maincpu")))
	{
	case 0x00a8: return 0xf0;
	case 0x00be: return 0xb0;
	case 0x0c1d: return 0xf0;
	case 0x0c6a: return 0xb0;
	case 0x0ceb: return 0x40;
	case 0x0d37: return 0x60;
	case 0x1ca2: return 0x00;  /* I don't think it's checked */
	case 0x1d7e: return 0xb0;
	default:
		logerror("%s: read protection\n",cpuexec_describe_context(device->machine));
		return 0;
	}
}


static READ8_HANDLER( mariner_protection_1_r )
{
	return 7;
}

static READ8_HANDLER( mariner_protection_2_r )
{
	return 3;
}


READ8_HANDLER( triplep_pip_r )
{
	logerror("PC %04x: triplep read port 2\n",cpu_get_pc(space->cpu));
	if (cpu_get_pc(space->cpu) == 0x015a) return 0xff;
	else if (cpu_get_pc(space->cpu) == 0x0886) return 0x05;
	else return 0;
}

READ8_HANDLER( triplep_pap_r )
{
	logerror("PC %04x: triplep read port 3\n",cpu_get_pc(space->cpu));
	if (cpu_get_pc(space->cpu) == 0x015d) return 0x04;
	else return 0;
}



static void cavelon_banksw(running_machine *machine)
{
	/* any read/write access in the 0x8000-0xffff region causes a bank switch.
       Only the lower 0x2000 is switched but we switch the whole region
       to keep the CPU core happy at the boundaries */

	cavelon_bank = !cavelon_bank;
	memory_set_bank(machine, 1, cavelon_bank);
}

static READ8_HANDLER( cavelon_banksw_r )
{
	cavelon_banksw(space->machine);

	if      ((offset >= 0x0100) && (offset <= 0x0103))
		return ppi8255_r(devtag_get_device(space->machine, "ppi8255_0"), offset - 0x0100);
	else if ((offset >= 0x0200) && (offset <= 0x0203))
		return ppi8255_r(devtag_get_device(space->machine, "ppi8255_1"), offset - 0x0200);

	return 0xff;
}

static WRITE8_HANDLER( cavelon_banksw_w )
{
	cavelon_banksw(space->machine);

	if      ((offset >= 0x0100) && (offset <= 0x0103))
		ppi8255_w(devtag_get_device(space->machine, "ppi8255_0"), offset - 0x0100, data);
	else if ((offset >= 0x0200) && (offset <= 0x0203))
		ppi8255_w(devtag_get_device(space->machine, "ppi8255_1"), offset - 0x0200, data);
}


READ8_HANDLER( hunchbks_mirror_r )
{
	return memory_read_byte(space, 0x1000+offset);
}

WRITE8_HANDLER( hunchbks_mirror_w )
{
	memory_write_byte(space, 0x1000+offset,data);
}

const ppi8255_interface scramble_ppi_0_intf =
{
	DEVCB_INPUT_PORT("IN0"),		/* Port A read */
	DEVCB_INPUT_PORT("IN1"),		/* Port B read */
	DEVCB_INPUT_PORT("IN2"),		/* Port C read */
	DEVCB_NULL,						/* Port A write */
	DEVCB_NULL,						/* Port B write */
	DEVCB_NULL 						/* Port C write */
};

const ppi8255_interface scramble_ppi_1_intf =
{
	DEVCB_NULL,												/* Port A read */
	DEVCB_NULL,												/* Port B read */
	DEVCB_NULL,												/* Port C read */
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, soundlatch_w),	/* Port A write */
	DEVCB_HANDLER(scramble_sh_irqtrigger_w),				/* Port B write */
	DEVCB_NULL												/* Port C write */
};


const ppi8255_interface stratgyx_ppi_1_intf =
{
	DEVCB_NULL,												/* Port A read */
	DEVCB_NULL,												/* Port B read */
	DEVCB_INPUT_PORT("IN3"),								/* Port C read */
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, soundlatch_w),	/* Port A write */
	DEVCB_HANDLER(scramble_sh_irqtrigger_w),				/* Port B write */
	DEVCB_NULL												/* Port C write */
};


const ppi8255_interface scramble_protection_ppi_1_intf =
{
	DEVCB_NULL,												/* Port A read */
	DEVCB_NULL,												/* Port B read */
	DEVCB_HANDLER(scramble_protection_r),					/* Port C read */
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, soundlatch_w),	/* Port A write */
	DEVCB_HANDLER(scramble_sh_irqtrigger_w),				/* Port B write */
	DEVCB_HANDLER(scramble_protection_w)					/* Port C write */
};


const ppi8255_interface mrkougar_ppi_1_intf =
{
	DEVCB_NULL,												/* Port A read */
	DEVCB_NULL,												/* Port B read */
	DEVCB_NULL,												/* Port C read */
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, soundlatch_w),	/* Port A write */
	DEVCB_HANDLER(mrkougar_sh_irqtrigger_w),				/* Port B write */
	DEVCB_NULL												/* Port C write */
};


DRIVER_INIT( scramble_ppi )
{
}

DRIVER_INIT( scobra )
{
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa803, 0xa803, 0, 0, scrambold_background_enable_w);
}

DRIVER_INIT( atlantis )
{
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x6803, 0x6803, 0, 0, scrambold_background_enable_w);
}

DRIVER_INIT( scramble )
{
	DRIVER_INIT_CALL(atlantis);
}

DRIVER_INIT( stratgyx )
{
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xb000, 0xb000, 0, 0, scrambold_background_green_w);
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xb002, 0xb002, 0, 0, scrambold_background_blue_w);
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xb00a, 0xb00a, 0, 0, scrambold_background_red_w);
}

DRIVER_INIT( tazmani2 )
{
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xb002, 0xb002, 0, 0, scrambold_background_enable_w);
}

DRIVER_INIT( ckongs )
{
}

DRIVER_INIT( mariner )
{
	/* extra ROM */
	memory_install_readwrite8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x5800, 0x67ff, 0, 0, (read8_space_func)SMH_BANK(1), (write8_space_func)SMH_UNMAP);
	memory_set_bankptr(machine, 1, memory_region(machine, "maincpu") + 0x5800);

	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x9008, 0x9008, 0, 0, mariner_protection_2_r);
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xb401, 0xb401, 0, 0, mariner_protection_1_r);

	/* ??? (it's NOT a background enable) */
	/*memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x6803, 0x6803, 0, 0, (write8_space_func)SMH_NOP);*/
}

DRIVER_INIT( frogger )
{
	offs_t A;
	UINT8 *ROM;

	/* the first ROM of the second CPU has data lines D0 and D1 swapped. Decode it. */
	ROM = memory_region(machine, "audiocpu");
	for (A = 0;A < 0x0800;A++)
		ROM[A] = BITSWAP8(ROM[A],7,6,5,4,3,2,0,1);

	/* likewise, the 2nd gfx ROM has data lines D0 and D1 swapped. Decode it. */
	ROM = memory_region(machine, "gfx1");
	for (A = 0x0800;A < 0x1000;A++)
		ROM[A] = BITSWAP8(ROM[A],7,6,5,4,3,2,0,1);
}

DRIVER_INIT( froggers )
{
	offs_t A;
	UINT8 *ROM;

	/* the first ROM of the second CPU has data lines D0 and D1 swapped. Decode it. */
	ROM = memory_region(machine, "audiocpu");
	for (A = 0;A < 0x0800;A++)
		ROM[A] = BITSWAP8(ROM[A],7,6,5,4,3,2,0,1);
}

DRIVER_INIT( devilfsh )
{
	offs_t i;
	UINT8 *RAM;

	/* Address lines are scrambled on the main CPU */

	/* A0 -> A2 */
	/* A1 -> A0 */
	/* A2 -> A3 */
	/* A3 -> A1 */

	RAM = memory_region(machine, "maincpu");
	for (i = 0; i < 0x10000; i += 16)
	{
		offs_t j;
		UINT8 swapbuffer[16];

		for (j = 0; j < 16; j++)
		{
			offs_t newval = BITSWAP8(j,7,6,5,4,2,0,3,1);

			swapbuffer[j] = RAM[i + newval];
		}

		memcpy(&RAM[i], swapbuffer, 16);
	}
}

DRIVER_INIT( mars )
{
	DRIVER_INIT_CALL(devilfsh);
}

DRIVER_INIT( hotshock )
{
	/* protection??? The game jumps into never-neverland here. I think
       it just expects a RET there */
	memory_region(machine, "maincpu")[0x2ef9] = 0xc9;
}

DRIVER_INIT( cavelon )
{
	UINT8 *ROM = memory_region(machine, "maincpu");

	/* banked ROM */
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x0000, 0x3fff, 0, 0, (read8_space_func)SMH_BANK(1));
	memory_configure_bank(machine, 1, 0, 2, &ROM[0x00000], 0x10000);
	cavelon_banksw(machine);

	/* A15 switches memory banks */
	memory_install_readwrite8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x8000, 0xffff, 0, 0, cavelon_banksw_r, cavelon_banksw_w);

	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x2000, 0x2000, 0, 0, (write8_space_func)SMH_NOP);	/* ??? */
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x3800, 0x3801, 0, 0, (write8_space_func)SMH_NOP);  /* looks suspicously like
                                                               an AY8910, but not sure */
	state_save_register_global(machine, cavelon_bank);
}



DRIVER_INIT( darkplnt )
{
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xb00a, 0xb00a, 0, 0, darkplnt_bullet_color_w);
}

DRIVER_INIT( mimonkey )
{
	static const UINT8 xortable[16][16] =
	{
		{ 0x03,0x03,0x05,0x07,0x85,0x00,0x85,0x85,0x80,0x80,0x06,0x03,0x03,0x00,0x00,0x81 },
		{ 0x83,0x87,0x03,0x87,0x06,0x00,0x06,0x04,0x02,0x00,0x84,0x84,0x04,0x00,0x01,0x83 },
		{ 0x82,0x82,0x84,0x02,0x04,0x00,0x00,0x03,0x82,0x00,0x06,0x80,0x03,0x00,0x81,0x07 },
		{ 0x06,0x06,0x82,0x81,0x85,0x00,0x04,0x07,0x81,0x05,0x04,0x00,0x03,0x00,0x82,0x84 },
		{ 0x07,0x07,0x80,0x07,0x07,0x00,0x85,0x86,0x00,0x07,0x06,0x04,0x85,0x00,0x86,0x85 },
		{ 0x81,0x83,0x02,0x02,0x87,0x00,0x86,0x03,0x04,0x06,0x80,0x05,0x87,0x00,0x81,0x81 },
		{ 0x01,0x01,0x00,0x07,0x07,0x00,0x01,0x01,0x07,0x07,0x06,0x00,0x06,0x00,0x07,0x07 },
		{ 0x80,0x87,0x81,0x87,0x83,0x00,0x84,0x01,0x01,0x86,0x86,0x80,0x86,0x00,0x86,0x86 },
		{ 0x03,0x03,0x05,0x07,0x85,0x00,0x85,0x85,0x80,0x80,0x06,0x03,0x03,0x00,0x00,0x81 },
		{ 0x83,0x87,0x03,0x87,0x06,0x00,0x06,0x04,0x02,0x00,0x84,0x84,0x04,0x00,0x01,0x83 },
		{ 0x82,0x82,0x84,0x02,0x04,0x00,0x00,0x03,0x82,0x00,0x06,0x80,0x03,0x00,0x81,0x07 },
		{ 0x06,0x06,0x82,0x81,0x85,0x00,0x04,0x07,0x81,0x05,0x04,0x00,0x03,0x00,0x82,0x84 },
		{ 0x07,0x07,0x80,0x07,0x07,0x00,0x85,0x86,0x00,0x07,0x06,0x04,0x85,0x00,0x86,0x85 },
		{ 0x81,0x83,0x02,0x02,0x87,0x00,0x86,0x03,0x04,0x06,0x80,0x05,0x87,0x00,0x81,0x81 },
		{ 0x01,0x01,0x00,0x07,0x07,0x00,0x01,0x01,0x07,0x07,0x06,0x00,0x06,0x00,0x07,0x07 },
		{ 0x80,0x87,0x81,0x87,0x83,0x00,0x84,0x01,0x01,0x86,0x86,0x80,0x86,0x00,0x86,0x86 }
	};

	UINT8 *ROM = memory_region(machine, "maincpu");
	int A, ctr = 0, line, col;

	for( A = 0; A < 0x4000; A++ )
	{
		line = (ctr & 0x07) | ((ctr & 0x200) >> 6);
		col = ((ROM[A] & 0x80) >> 4) | (ROM[A] & 0x07);
		ROM[A] = ROM[A] ^ xortable[line][col];
		ctr++;
	}

	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa804, 0xa804, 0, 0, scrambold_background_enable_w);
}

DRIVER_INIT( mimonsco )
{
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa804, 0xa804, 0, 0, scrambold_background_enable_w);
}

DRIVER_INIT( mimonscr )
{
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x6804, 0x6804, 0, 0, scrambold_background_enable_w);
}


static int bit(int i,int n)
{
	return ((i >> n) & 1);
}


DRIVER_INIT( anteater )
{
	offs_t i, len;
	UINT8 *RAM;
	UINT8 *scratch;


	DRIVER_INIT_CALL(scobra);

	/*
    *   Code To Decode Lost Tomb by Mirko Buffoni
    *   Optimizations done by Fabio Buffoni
    */

	RAM = memory_region(machine, "gfx1");
	len = memory_region_length(machine, "gfx1");

	scratch = alloc_array_or_die(UINT8, len);

		memcpy(scratch, RAM, len);

		for (i = 0; i < len; i++)
		{
			int j;


			j = i & 0x9bf;
			j |= ( bit(i,4) ^ bit(i,9) ^ ( bit(i,2) & bit(i,10) ) ) << 6;
			j |= ( bit(i,2) ^ bit(i,10) ) << 9;
			j |= ( bit(i,0) ^ bit(i,6) ^ 1 ) << 10;

			RAM[i] = scratch[j];
		}

		free(scratch);
}

DRIVER_INIT( rescue )
{
	offs_t i, len;
	UINT8 *RAM;
	UINT8 *scratch;


	DRIVER_INIT_CALL(scobra);

	/*
    *   Code To Decode Lost Tomb by Mirko Buffoni
    *   Optimizations done by Fabio Buffoni
    */

	RAM = memory_region(machine, "gfx1");
	len = memory_region_length(machine, "gfx1");

	scratch = alloc_array_or_die(UINT8, len);

		memcpy(scratch, RAM, len);

		for (i = 0; i < len; i++)
		{
			int j;


			j = i & 0xa7f;
			j |= ( bit(i,3) ^ bit(i,10) ) << 7;
			j |= ( bit(i,1) ^ bit(i,7) ) << 8;
			j |= ( bit(i,0) ^ bit(i,8) ) << 10;

			RAM[i] = scratch[j];
		}

		free(scratch);
}

DRIVER_INIT( minefld )
{
	offs_t i, len;
	UINT8 *RAM;
	UINT8 *scratch;


	DRIVER_INIT_CALL(scobra);

	/*
    *   Code To Decode Minefield by Mike Balfour and Nicola Salmoria
    */

	RAM = memory_region(machine, "gfx1");
	len = memory_region_length(machine, "gfx1");

	scratch = alloc_array_or_die(UINT8, len);

		memcpy(scratch, RAM, len);

		for (i = 0; i < len; i++)
		{
			int j;


			j  = i & 0xd5f;
			j |= ( bit(i,3) ^ bit(i,7) ) << 5;
			j |= ( bit(i,2) ^ bit(i,9) ^ ( bit(i,0) & bit(i,5) ) ^
				 ( bit(i,3) & bit(i,7) & ( bit(i,0) ^ bit(i,5) ))) << 7;
			j |= ( bit(i,0) ^ bit(i,5) ^ ( bit(i,3) & bit(i,7) ) ) << 9;

			RAM[i] = scratch[j];
		}

		free(scratch);
}

DRIVER_INIT( losttomb )
{
	offs_t i, len;
	UINT8 *RAM;
	UINT8 *scratch;


	DRIVER_INIT_CALL(scramble);

	/*
    *   Code To Decode Lost Tomb by Mirko Buffoni
    *   Optimizations done by Fabio Buffoni
    */

	RAM = memory_region(machine, "gfx1");
	len = memory_region_length(machine, "gfx1");

	scratch = alloc_array_or_die(UINT8, len);

		memcpy(scratch, RAM, len);

		for (i = 0; i < len; i++)
		{
			int j;


			j = i & 0xa7f;
			j |= ( (bit(i,1) & bit(i,8)) | ((1 ^ bit(i,1)) & (bit(i,10)))) << 7;
			j |= ( bit(i,7) ^ (bit(i,1) & ( bit(i,7) ^ bit(i,10) ))) << 8;
			j |= ( (bit(i,1) & bit(i,7)) | ((1 ^ bit(i,1)) & (bit(i,8)))) << 10;

			RAM[i] = scratch[j];
		}

		free(scratch);
}


DRIVER_INIT( hustler )
{
	offs_t A;
	UINT8 *rom = memory_region(machine, "maincpu");


	for (A = 0;A < 0x4000;A++)
	{
		UINT8 xormask;
		int bits[8];
		int i;


		for (i = 0;i < 8;i++)
			bits[i] = (A >> i) & 1;

		xormask = 0xff;
		if (bits[0] ^ bits[1]) xormask ^= 0x01;
		if (bits[3] ^ bits[6]) xormask ^= 0x02;
		if (bits[4] ^ bits[5]) xormask ^= 0x04;
		if (bits[0] ^ bits[2]) xormask ^= 0x08;
		if (bits[2] ^ bits[3]) xormask ^= 0x10;
		if (bits[1] ^ bits[5]) xormask ^= 0x20;
		if (bits[0] ^ bits[7]) xormask ^= 0x40;
		if (bits[4] ^ bits[6]) xormask ^= 0x80;

		rom[A] ^= xormask;
	}

	/* the first ROM of the second CPU has data lines D0 and D1 swapped. Decode it. */
	{
		rom = memory_region(machine, "audiocpu");


		for (A = 0;A < 0x0800;A++)
			rom[A] = BITSWAP8(rom[A],7,6,5,4,3,2,0,1);
	}
}

DRIVER_INIT( billiard )
{
	offs_t A;
	UINT8 *rom = memory_region(machine, "maincpu");


	for (A = 0;A < 0x4000;A++)
	{
		UINT8 xormask;
		int bits[8];
		int i;


		for (i = 0;i < 8;i++)
			bits[i] = (A >> i) & 1;

		xormask = 0x55;
		if (bits[2] ^ (( bits[3]) & ( bits[6]))) xormask ^= 0x01;
		if (bits[4] ^ (( bits[5]) & ( bits[7]))) xormask ^= 0x02;
		if (bits[0] ^ (( bits[7]) & (!bits[3]))) xormask ^= 0x04;
		if (bits[3] ^ ((!bits[0]) & ( bits[2]))) xormask ^= 0x08;
		if (bits[5] ^ ((!bits[4]) & ( bits[1]))) xormask ^= 0x10;
		if (bits[6] ^ ((!bits[2]) & (!bits[5]))) xormask ^= 0x20;
		if (bits[1] ^ ((!bits[6]) & (!bits[4]))) xormask ^= 0x40;
		if (bits[7] ^ ((!bits[1]) & ( bits[0]))) xormask ^= 0x80;

		rom[A] ^= xormask;

		rom[A] = BITSWAP8(rom[A],6,1,2,5,4,3,0,7);
	}

	/* the first ROM of the second CPU has data lines D0 and D1 swapped. Decode it. */
	{
		rom = memory_region(machine, "audiocpu");


		for (A = 0;A < 0x0800;A++)
			rom[A] = BITSWAP8(rom[A],7,6,5,4,3,2,0,1);
	}
}

/************************************************************
 mr kougar protected main cpu - by HIGHWAYMAN
 mr kougar contains a steel module at location S7,
 this module contains a Z80c cpu with the following changes:
 IOREQ pin cut, RD & WR pins swapped and the following
 address lines swapped - a0-a2,a1-a0,a2-a3,a3-a1.
*************************************************************/

DRIVER_INIT( mrkougar )
{
	DRIVER_INIT_CALL(devilfsh);
}

DRIVER_INIT( mrkougb )
{
}

DRIVER_INIT( ad2083 )
{
	UINT8 c;
	int i, len = memory_region_length(machine, "maincpu");
	UINT8 *ROM = memory_region(machine, "maincpu");

	for (i=0; i<len; i++)
	{
		c = ROM[i] ^ 0x35;
		c = BITSWAP8(c, 6,2,5,1,7,3,4,0); /* also swapped inside of the bigger module */
		ROM[i] = c;
	}
}
