/***************************************************************************

Nintendo VS UniSystem and DualSystem - (c) 1984 Nintendo of America

    Portions of this code are heavily based on
    Brad Oliver's MESS implementation of the NES.

***************************************************************************/

#include "emu.h"
#include "video/ppu2c0x.h"
#include "includes/vsnes.h"

/* Globals */
static int vsnes_do_vrom_bank;

/* Locals */
static int input_latch[4];

static int sound_fix=0;
static UINT8 last_bank;

/* PPU notes */
/* nametable is, per Lord Nightmare, always 4K per PPU */
/* The vsnes system has relatively few banking options for CHR */
/* Each driver will use ROM or RAM for CHR, never both, and RAM is never banked */
/* This leads to the memory system being an optimal place to perform banking */

/* need double pointers for vsdual */
static UINT8* vram = NULL;
static UINT8* vrom[2];
static UINT8* nt_ram[2];
static UINT8* nt_page[2][4]; // because mirroring is used.
static UINT32 vrom_size[2];
static int vrom_banks;

/* Prototypes for mapping board components to PPU bus */
static WRITE8_HANDLER( vsnes_nt0_w );
static WRITE8_HANDLER( vsnes_nt1_w );
static READ8_HANDLER( vsnes_nt0_r );
static READ8_HANDLER( vsnes_nt1_r );

static const char * const chr_banknames[] = { "bank2", "bank3", "bank4", "bank5", "bank6", "bank7", "bank8", "bank9" };

/*************************************
 *
 *  Input Ports
 *
 *************************************/

WRITE8_HANDLER( vsnes_in0_w )
{
	/* Toggling bit 0 high then low resets both controllers */
	if (data & 1)
	{
		/* load up the latches */
		input_latch[0] = input_port_read(space->machine, "IN0");
		input_latch[1] = input_port_read(space->machine, "IN1");
	}
}

static READ8_HANDLER( gun_in0_r )
{
	int ret = (input_latch[0]) & 1;

	/* shift */
	input_latch[0] >>= 1;

	ret |= input_port_read(space->machine, "COINS");				/* merge coins, etc */
	ret |= (input_port_read(space->machine, "DSW0") & 3) << 3;		/* merge 2 dipswitches */

/* The gun games expect a 1 returned on every 5th read after sound_fix is reset*/
/* Info Supplied by Ben Parnell <xodnizel@home.com> of FCE Ultra fame */

	if (sound_fix == 4)
	{
		ret = 1;
	}

	sound_fix++;

	return ret;

}

READ8_HANDLER( vsnes_in0_r )
{

	int ret = (input_latch[0]) & 1;

	/* shift */
	input_latch[0] >>= 1;

	ret |= input_port_read(space->machine, "COINS");				/* merge coins, etc */
	ret |= (input_port_read(space->machine, "DSW0") & 3) << 3;		/* merge 2 dipswitches */

	return ret;

}

READ8_HANDLER( vsnes_in1_r )
{
	int ret = (input_latch[1]) & 1;

	ret |= input_port_read(space->machine, "DSW0") & ~3;			/* merge the rest of the dipswitches */

	/* shift */
	input_latch[1] >>= 1;

	return ret;
}

WRITE8_HANDLER( vsnes_in0_1_w )
{
	/* Toggling bit 0 high then low resets both controllers */
	if (data & 1)
	{
		/* load up the latches */
		input_latch[2] = input_port_read(space->machine, "IN2");
		input_latch[3] = input_port_read(space->machine, "IN3");
	}
}

READ8_HANDLER( vsnes_in0_1_r )
{
	int ret = (input_latch[2]) & 1;

	/* shift */
	input_latch[2] >>= 1;

	ret |= input_port_read(space->machine, "COINS2");				/* merge coins, etc */
	ret |= (input_port_read(space->machine, "DSW1") & 3) << 3;		/* merge 2 dipswitches */
	return ret;
}

READ8_HANDLER( vsnes_in1_1_r )
{
	int ret = (input_latch[3]) & 1;

	ret |= input_port_read(space->machine, "DSW1") & ~3;			/* merge the rest of the dipswitches */

	/* shift */
	input_latch[3] >>= 1;

	return ret;

}

/*************************************
 *
 *  Init machine
 *
 *************************************/

MACHINE_RESET( vsnes )
{

	last_bank = 0xff;
	sound_fix = 0;
	input_latch[0] = input_latch[1] = 0;
	input_latch[2] = input_latch[3] = 0;

}

/*************************************
 *
 *  Init machine
 *
 *************************************/

MACHINE_RESET( vsdual )
{

	input_latch[0] = input_latch[1] = 0;
	input_latch[2] = input_latch[3] = 0;

}

/*************************************
 *
 *  Machine start functions
 *
 *************************************/

static void v_set_videorom_bank( running_machine* machine, int start, int count, int vrom_start_bank )
{
	int i;

	assert(start + count <= 8);

	vrom_start_bank &= (vrom_banks - 1);
	assert(vrom_start_bank + count <= vrom_banks);

	/* bank_size_in_kb is used to determine how large the "bank" parameter is */
	/* count determines the size of the area mapped */
	for (i = 0; i < count; i++)
	{
		memory_set_bank(machine, chr_banknames[i + start], vrom_start_bank + i);
	}
}

MACHINE_START( vsnes )
{
	address_space *ppu1_space = cpu_get_address_space(machine->device("ppu1"), ADDRESS_SPACE_PROGRAM);
	int i;

	/* establish nametable ram */
	nt_ram[0] = auto_alloc_array(machine, UINT8, 0x1000);
	/* set mirroring */
	nt_page[0][0] = nt_ram[0];
	nt_page[0][1] = nt_ram[0] + 0x400;
	nt_page[0][2] = nt_ram[0] + 0x800;
	nt_page[0][3] = nt_ram[0] + 0xc00;

	memory_install_readwrite8_handler(ppu1_space, 0x2000, 0x3eff, 0, 0, vsnes_nt0_r, vsnes_nt0_w);

	vrom[0] = memory_region(machine, "gfx1");
	vrom_size[0] = memory_region_length(machine, "gfx1");
	vrom_banks = vrom_size[0] / 0x400;

	/* establish chr banks */
	/* bank 1 is used already! */
	/* DRIVER_INIT is called first - means we can handle this different for VRAM games! */
	if (NULL != vrom[0])
	{
		for (i = 0; i < 8; i++)
		{
			memory_install_read_bank(ppu1_space, 0x0400 * i, 0x0400 * i + 0x03ff, 0, 0, chr_banknames[i]);
			memory_configure_bank(machine, chr_banknames[i], 0, vrom_banks, vrom[0], 0x400);
		}
		v_set_videorom_bank(machine, 0, 8, 0);
	}
	else
	{
		memory_install_ram(ppu1_space, 0x0000, 0x1fff, 0, 0, vram);
	}
}

MACHINE_START( vsdual )
{
	vrom[0] = memory_region(machine, "gfx1");
	vrom[1] = memory_region(machine, "gfx2");
	vrom_size[0] = memory_region_length(machine, "gfx1");
	vrom_size[1] = memory_region_length(machine, "gfx2");

	/* establish nametable ram */
	nt_ram[0] = auto_alloc_array(machine, UINT8, 0x1000);
	nt_ram[1] = auto_alloc_array(machine, UINT8, 0x1000);
	/* set mirroring */
	nt_page[0][0] = nt_ram[0];
	nt_page[0][1] = nt_ram[0] + 0x400;
	nt_page[0][2] = nt_ram[0] + 0x800;
	nt_page[0][3] = nt_ram[0] + 0xc00;
	nt_page[1][0] = nt_ram[1];
	nt_page[1][1] = nt_ram[1] + 0x400;
	nt_page[1][2] = nt_ram[1] + 0x800;
	nt_page[1][3] = nt_ram[1] + 0xc00;

	memory_install_readwrite8_handler(cpu_get_address_space(machine->device("ppu1"), ADDRESS_SPACE_PROGRAM), 0x2000, 0x3eff, 0, 0, vsnes_nt0_r, vsnes_nt0_w);
	memory_install_readwrite8_handler(cpu_get_address_space(machine->device("ppu2"), ADDRESS_SPACE_PROGRAM), 0x2000, 0x3eff, 0, 0, vsnes_nt1_r, vsnes_nt1_w);
	// read only!
	memory_install_read_bank(cpu_get_address_space(machine->device("ppu1"), ADDRESS_SPACE_PROGRAM), 0x0000, 0x1fff, 0, 0, "bank2");
	// read only!
	memory_install_read_bank(cpu_get_address_space(machine->device("ppu2"), ADDRESS_SPACE_PROGRAM), 0x0000, 0x1fff, 0, 0, "bank3");
	memory_configure_bank(machine, "bank2", 0, vrom_size[0] / 0x2000, vrom[0], 0x2000);
	memory_configure_bank(machine, "bank3", 0, vrom_size[1] / 0x2000, vrom[1], 0x2000);
	memory_set_bank(machine, "bank2", 0);
	memory_set_bank(machine, "bank3", 0);
}

/*************************************
 *
 *  External mappings for PPU bus
 *
 *************************************/

static WRITE8_HANDLER( vsnes_nt0_w )
{
	int page = ((offset & 0xc00) >> 10);
	nt_page[0][page][offset & 0x3ff] = data;
}

static WRITE8_HANDLER( vsnes_nt1_w )
{
	int page = ((offset & 0xc00) >> 10);
	nt_page[1][page][offset & 0x3ff] = data;
}

static READ8_HANDLER( vsnes_nt0_r )
{
	int page = ((offset&0xc00) >> 10);
	return nt_page[0][page][offset & 0x3ff];
}

static READ8_HANDLER( vsnes_nt1_r )
{
	int page = ((offset & 0xc00) >> 10);
	return nt_page[1][page][offset & 0x3ff];
}

static void v_set_mirroring( int ppu, int mirroring )
{
	switch (mirroring)
	{
	case PPU_MIRROR_LOW:
		nt_page[ppu][0] = nt_page[ppu][1] = nt_page[ppu][2] = nt_page[ppu][3] = nt_ram[ppu];
		break;
	case PPU_MIRROR_HIGH:
		nt_page[ppu][0] = nt_page[ppu][1] = nt_page[ppu][2] = nt_page[ppu][3] = nt_ram[ppu] + 0x400;
		break;
	case PPU_MIRROR_HORZ:
		nt_page[ppu][0] = nt_ram[ppu];
		nt_page[ppu][1] = nt_ram[ppu];
		nt_page[ppu][2] = nt_ram[ppu] + 0x400;
		nt_page[ppu][3] = nt_ram[ppu] + 0x400;
		break;
	case PPU_MIRROR_VERT:
		nt_page[ppu][0] = nt_ram[ppu];
		nt_page[ppu][1] = nt_ram[ppu] + 0x400;
		nt_page[ppu][2] = nt_ram[ppu];
		nt_page[ppu][3] = nt_ram[ppu] + 0x400;
		break;
	case PPU_MIRROR_NONE:
	default:
		nt_page[ppu][0] = nt_ram[ppu];
		nt_page[ppu][1] = nt_ram[ppu] + 0x400;
		nt_page[ppu][2] = nt_ram[ppu] + 0x800;
		nt_page[ppu][3] = nt_ram[ppu] + 0xc00;
		break;
	}

}

/**********************************************************************************
 *
 *  Game and Board-specific initialization
 *
 **********************************************************************************/

/**********************************************************************************/
/* Most games: VROM Banking in controller 0 write */

static WRITE8_HANDLER( vsnormal_vrom_banking )
{
	/* switch vrom */
	v_set_videorom_bank(space->machine, 0, 8, (data & 4) ? 8 : 0);

	/* bit 1 ( data & 2 ) enables writes to extra ram, we ignore it */

	/* move along */
	vsnes_in0_w(space, offset, data);
}

DRIVER_INIT( vsnormal )
{
	/* vrom switching is enabled with bit 2 of $4016 */
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x4016, 0x4016, 0, 0, vsnormal_vrom_banking);
}

/**********************************************************************************/
/* Gun games: VROM Banking in controller 0 write */

static WRITE8_HANDLER( gun_in0_w )
{
	running_device *ppu1 = space->machine->device("ppu1");
	static int zapstore;

	if (vsnes_do_vrom_bank)
	{
		/* switch vrom */
		v_set_videorom_bank(space->machine, 0, 8, (data & 4) ? 8 : 0);
	}

	/* here we do things a little different */
	if (data & 1)
	{

		/* load up the latches */
		input_latch[0] = input_port_read(space->machine, "IN0");

		/* do the gun thing */
		int x = input_port_read(space->machine, "GUNX");
		int y = input_port_read(space->machine, "GUNY");
		UINT32 pix, color_base;

		/* get the pixel at the gun position */
		pix = ppu2c0x_get_pixel(ppu1, x, y);

		/* get the color base from the ppu */
		color_base = ppu2c0x_get_colorbase(ppu1);

		/* look at the screen and see if the cursor is over a bright pixel */
		if ((pix == color_base + 0x20 ) || (pix == color_base + 0x30) ||
			(pix == color_base + 0x33 ) || (pix == color_base + 0x34))
		{
			input_latch[0] |= 0x40;
		}

		input_latch[1] = input_port_read(space->machine, "IN1");
	}

    if ((zapstore & 1) && (!(data & 1)))
	/* reset sound_fix to keep sound from hanging */
    {
		sound_fix = 0;
	}

    zapstore = data;
}

DRIVER_INIT( vsgun )
{
	/* VROM switching is enabled with bit 2 of $4016 */
	memory_install_readwrite8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x4016, 0x4016, 0, 0, gun_in0_r, gun_in0_w);
	vsnes_do_vrom_bank = 1;
}

/**********************************************************************************/
/* Konami games: ROMs bankings at $8000-$ffff */

static WRITE8_HANDLER( vskonami_rom_banking )
{
	int reg = (offset >> 12) & 0x07;
	int bankoffset = (data & 7) * 0x2000 + 0x10000;

	switch (reg)
	{
		case 0: /* code bank 0 */
		case 2: /* code bank 1 */
		case 4: /* code bank 2 */
		{
			UINT8 *prg = memory_region(space->machine, "maincpu");
			memcpy(&prg[0x08000 + reg * 0x1000], &prg[bankoffset], 0x2000);
		}
		break;

		case 6: /* vrom bank 0 */
			v_set_videorom_bank(space->machine, 0, 4, data * 4);
		break;

		case 7: /* vrom bank 1 */
			v_set_videorom_bank(space->machine, 4, 4, data * 4);
		break;
	}
}

DRIVER_INIT( vskonami )
{
	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	UINT8 *prg = memory_region(machine, "maincpu");
	memcpy(&prg[0x08000], &prg[0x18000], 0x8000);

	/* banking is done with writes to the $8000-$ffff area */
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x8000, 0xffff, 0, 0, vskonami_rom_banking);
}

/***********************************************************************/
/* Vs. Gumshoe */

static WRITE8_HANDLER( vsgshoe_gun_in0_w )
{
	static int old_bank = 0;
	int addr;
	if((data & 0x04) != old_bank)
	{
		UINT8 *prg = memory_region(space->machine, "maincpu");
		old_bank = data & 0x04;
		addr = old_bank ? 0x12000: 0x10000;
		memcpy(&prg[0x08000], &prg[addr], 0x2000);
	}

	gun_in0_w(space, offset, data);
}

DRIVER_INIT( vsgshoe )
{
	/* set up the default bank */
	UINT8 *prg = memory_region(machine, "maincpu");
	memcpy (&prg[0x08000], &prg[0x12000], 0x2000);

	/* vrom switching is enabled with bit 2 of $4016 */
	memory_install_readwrite8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x4016, 0x4016, 0, 0, gun_in0_r, vsgshoe_gun_in0_w);

	vsnes_do_vrom_bank = 1;
}

/**********************************************************************************/
/* Dr Mario: ROMs bankings at $8000-$ffff */

static int drmario_shiftreg;
static int drmario_shiftcount;

static WRITE8_HANDLER( drmario_rom_banking )
{
	/* basically, a MMC1 mapper from the nes */
	static int size16k, switchlow, vrom4k;

	int reg = (offset >> 13);

	/* reset mapper */
	if (data & 0x80)
	{
		drmario_shiftreg = drmario_shiftcount = 0;

		size16k = 1;

		switchlow = 1;
		vrom4k = 0;

		return;
	}

	/* see if we need to clock in data */
	if (drmario_shiftcount < 5)
	{
		drmario_shiftreg >>= 1;
		drmario_shiftreg |= (data & 1) << 4;
		drmario_shiftcount++;
	}

	/* are we done shifting? */
	if (drmario_shiftcount == 5)
	{
		/* reset count */
		drmario_shiftcount = 0;

		/* apply data to registers */
		switch (reg)
		{
			case 0:		/* mirroring and options */
				{
					int mirroring;

					vrom4k = drmario_shiftreg & 0x10;
					size16k = drmario_shiftreg & 0x08;
					switchlow = drmario_shiftreg & 0x04;

					switch (drmario_shiftreg & 3)
					{
						case 0:
							mirroring = PPU_MIRROR_LOW;
						break;

						case 1:
							mirroring = PPU_MIRROR_HIGH;
						break;

						case 2:
							mirroring = PPU_MIRROR_VERT;
						break;

						default:
						case 3:
							mirroring = PPU_MIRROR_HORZ;
						break;
					}

					/* apply mirroring */
					v_set_mirroring(1, mirroring);
				}
			break;

			case 1:	/* video rom banking - bank 0 - 4k or 8k */
				if (!vram)
					v_set_videorom_bank(space->machine, 0, (vrom4k) ? 4 : 8, drmario_shiftreg * 4);
			break;

			case 2: /* video rom banking - bank 1 - 4k only */
				if (vrom4k && !vram)
					v_set_videorom_bank(space->machine, 4, 4, drmario_shiftreg * 4);
			break;

			case 3:	/* program banking */
				{
					int bank = (drmario_shiftreg & 0x03) * 0x4000;
					UINT8 *prg = memory_region(space->machine, "maincpu");

					if (!size16k)
					{
						/* switch 32k */
						memcpy(&prg[0x08000], &prg[0x010000 + bank], 0x8000);
					}
					else
					{
						/* switch 16k */
						if (switchlow)
						{
							/* low */
							memcpy(&prg[0x08000], &prg[0x010000 + bank], 0x4000);
						}
						else
						{
							/* high */
							memcpy(&prg[0x0c000], &prg[0x010000 + bank], 0x4000);
						}
					}
				}
			break;
		}

		drmario_shiftreg = 0;
	}
}

DRIVER_INIT( drmario )
{
	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	UINT8 *prg = memory_region(machine, "maincpu");
	memcpy(&prg[0x08000], &prg[0x10000], 0x4000);
	memcpy(&prg[0x0c000], &prg[0x1c000], 0x4000);

	/* MMC1 mapper at writes to $8000-$ffff */
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x8000, 0xffff, 0, 0, drmario_rom_banking);

	drmario_shiftreg = 0;
	drmario_shiftcount = 0;
}

/**********************************************************************************/
/* Games with VRAM instead of graphics ROMs: ROMs bankings at $8000-$ffff */

static WRITE8_HANDLER( vsvram_rom_banking )
{
	int rombank = 0x10000 + (data & 7) * 0x4000;
	UINT8 *prg = memory_region(space->machine, "maincpu");

	memcpy(&prg[0x08000], &prg[rombank], 0x4000);
}

DRIVER_INIT( vsvram )
{
	/* when starting the game, the 1st 16k and the last 16k are loaded into the 2 banks */
	UINT8 *prg = memory_region(machine, "maincpu");
	memcpy(&prg[0x08000], &prg[0x28000], 0x8000);

	/* banking is done with writes to the $8000-$ffff area */
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x8000, 0xffff, 0, 0, vsvram_rom_banking);

	/* allocate vram */
	vram = auto_alloc_array(machine, UINT8, 0x2000);
}

/**********************************************************************************/

static int MMC3_cmd;
static int MMC3_prg_bank[4];
static int MMC3_chr_bank[6];
static int MMC3_prg_mask;
static int IRQ_enable, IRQ_count, IRQ_count_latch;

static void mapper4_set_prg( running_machine *machine )
{
	UINT8 *prg = memory_region(machine, "maincpu");
	UINT8 prg_flip = (MMC3_cmd & 0x40) ? 2 : 0;

	memcpy(&prg[0x8000], &prg[0x2000 * (MMC3_prg_bank[0 ^ prg_flip] & MMC3_prg_mask) + 0x10000], 0x2000);
	memcpy(&prg[0xa000], &prg[0x2000 * (MMC3_prg_bank[1] & MMC3_prg_mask) + 0x10000], 0x2000);
	memcpy(&prg[0xc000], &prg[0x2000 * (MMC3_prg_bank[2 ^ prg_flip] & MMC3_prg_mask) + 0x10000], 0x2000);
	memcpy(&prg[0xe000], &prg[0x2000 * (MMC3_prg_bank[3] & MMC3_prg_mask) + 0x10000], 0x2000);
}

static void mapper4_set_chr( running_machine *machine )
{
	UINT8 chr_page = (MMC3_cmd & 0x80) >> 5;

	v_set_videorom_bank(machine, chr_page ^ 0, 1, MMC3_chr_bank[0] & ~0x01);
	v_set_videorom_bank(machine, chr_page ^ 1, 1, MMC3_chr_bank[0] |  0x01);
	v_set_videorom_bank(machine, chr_page ^ 2, 1, MMC3_chr_bank[1] & ~0x01);
	v_set_videorom_bank(machine, chr_page ^ 3, 1, MMC3_chr_bank[1] |  0x01);
	v_set_videorom_bank(machine, chr_page ^ 4, 1, MMC3_chr_bank[2]);
	v_set_videorom_bank(machine, chr_page ^ 5, 1, MMC3_chr_bank[3]);
	v_set_videorom_bank(machine, chr_page ^ 6, 1, MMC3_chr_bank[4]);
	v_set_videorom_bank(machine, chr_page ^ 7, 1, MMC3_chr_bank[5]);
}

#define BOTTOM_VISIBLE_SCANLINE	239		/* The bottommost visible scanline */
#define NUM_SCANLINE 262

static void mapper4_irq( running_device *device, int scanline, int vblank, int blanked )
{
	if (scanline < PPU_BOTTOM_VISIBLE_SCANLINE)
	{
		int priorCount = IRQ_count;
		if ((IRQ_count == 0))
		{
			IRQ_count = IRQ_count_latch;
		}
		else
			IRQ_count--;

		if (IRQ_enable && !blanked && (IRQ_count == 0) && priorCount)
		{
			cputag_set_input_line(device->machine, "maincpu", 0, HOLD_LINE);
		}
	}
}

static WRITE8_HANDLER( mapper4_w )
{
	running_device *ppu1 = space->machine->device("ppu1");
	UINT8 MMC3_helper, cmd;

	switch (offset & 0x6001)
	{
		case 0x0000: /* $8000 */
			MMC3_helper = MMC3_cmd ^ data;
			MMC3_cmd = data;

			/* Has PRG Mode changed? */
			if (MMC3_helper & 0x40)
				mapper4_set_prg(space->machine);

			/* Has CHR Mode changed? */
			if (MMC3_helper & 0x80)
				mapper4_set_chr(space->machine);
			break;

		case 0x0001: /* $8001 */
			cmd = MMC3_cmd & 0x07;
			switch (cmd)
			{
				case 0: case 1:	// these do not need to be separated: we take care of them in set_chr!
				case 2: case 3: case 4: case 5:
					MMC3_chr_bank[cmd] = data;
					mapper4_set_chr(space->machine);
					break;
				case 6:
				case 7:
					MMC3_prg_bank[cmd - 6] = data;
					mapper4_set_prg(space->machine);
					break;
			}
			break;

		case 0x2000: /* $a000 */
			if (data & 0x40)
				v_set_mirroring(1, PPU_MIRROR_HIGH);
			else
			{
				if (data & 0x01)
					v_set_mirroring(1, PPU_MIRROR_HORZ);
				else
					v_set_mirroring(1, PPU_MIRROR_VERT);
			}
			break;

		case 0x2001: /* $a001 - extra RAM enable/disable */
			/* ignored - we always enable it */

			break;
		case 0x4000: /* $c000 - IRQ scanline counter */
			IRQ_count = data;

			break;

		case 0x4001: /* $c001 - IRQ scanline latch */
			IRQ_count_latch = data;

			break;

		case 0x6000: /* $e000 - Disable IRQs */
			IRQ_enable = 0;
			IRQ_count = IRQ_count_latch;

			ppu2c0x_set_scanline_callback (ppu1, 0);

			break;

		case 0x6001: /* $e001 - Enable IRQs */
			IRQ_enable = 1;
			ppu2c0x_set_scanline_callback (ppu1, mapper4_irq);

			break;

		default:
			logerror("mapper4_w uncaught: %04x value: %02x\n", offset + 0x8000, data);
			break;
	}
}

/* Common init for MMC3 games */

DRIVER_INIT( MMC3 )
{
	UINT8 *prg = memory_region(machine, "maincpu");
	IRQ_enable = IRQ_count = IRQ_count_latch = 0;
	int MMC3_prg_chunks = (memory_region_length(machine, "maincpu") - 0x10000) / 0x4000;

	MMC3_prg_bank[0] = MMC3_prg_bank[2] = 0xfe;
	MMC3_prg_bank[1] = MMC3_prg_bank[3] = 0xff;
	MMC3_cmd = 0;

	MMC3_prg_mask = ((MMC3_prg_chunks << 1) - 1);

	memcpy(&prg[0x8000], &prg[(MMC3_prg_chunks - 1) * 0x4000 + 0x10000], 0x2000);
	memcpy(&prg[0xa000], &prg[(MMC3_prg_chunks - 1) * 0x4000 + 0x12000], 0x2000);
	memcpy(&prg[0xc000], &prg[(MMC3_prg_chunks - 1) * 0x4000 + 0x10000], 0x2000);
	memcpy(&prg[0xe000], &prg[(MMC3_prg_chunks - 1) * 0x4000 + 0x12000], 0x2000);

	/* MMC3 mapper at writes to $8000-$ffff */
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x8000, 0xffff, 0, 0, mapper4_w);

	/* extra ram at $6000-$7fff */
	memory_install_ram(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x6000, 0x7fff, 0, 0, NULL);
}

/* Vs. RBI Baseball */

static READ8_HANDLER( rbi_hack_r )
{
	/* Supplied by Ben Parnell <xodnizel@home.com> of FCE Ultra fame */

	static int VSindex;

	if (offset == 0)
	{
		VSindex=0;
		return 0xFF;

	}
	else
	{
		switch(VSindex++)
		{
			case 9:
    			return 0x6F;

			case 14:
				return 0x94;

			default:
    			return 0xB4;
		}
	}
}

DRIVER_INIT( rbibb )
{
	DRIVER_INIT_CALL(MMC3);

	/* RBI Base ball hack */
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x5e00, 0x5e01, 0, 0, rbi_hack_r) ;
}

/* Vs. Super Xevious */

static int supxevs_prot_index = 0;

static READ8_HANDLER( supxevs_read_prot_1_r )
{
	return 0x05;
}

static READ8_HANDLER( supxevs_read_prot_2_r )
{
	if (supxevs_prot_index)
		return 0;
	else
		return 0x01;
}

static READ8_HANDLER( supxevs_read_prot_3_r )
{
	if (supxevs_prot_index)
		return 0xd1;
	else
		return 0x89;
}

static READ8_HANDLER( supxevs_read_prot_4_r )
{
	if (supxevs_prot_index)
	{
		supxevs_prot_index = 0;
		return 0x3e;
	}
	else
	{
		supxevs_prot_index = 1;
		return 0x37;
	}
}


DRIVER_INIT( supxevs )
{
	DRIVER_INIT_CALL(MMC3);

	/* Vs. Super Xevious Protection */
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x54ff, 0x54ff, 0, 0, supxevs_read_prot_1_r);
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x5678, 0x5678, 0, 0, supxevs_read_prot_2_r);
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x578f, 0x578f, 0, 0, supxevs_read_prot_3_r);
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x5567, 0x5567, 0, 0, supxevs_read_prot_4_r);
}

/* Vs. TKO Boxing */

static READ8_HANDLER( tko_security_r )
{
	static int security_counter;
	static const UINT8 security_data[] = {
		0xff, 0xbf, 0xb7, 0x97, 0x97, 0x17, 0x57, 0x4f,
		0x6f, 0x6b, 0xeb, 0xa9, 0xb1, 0x90, 0x94, 0x14,
		0x56, 0x4e, 0x6f, 0x6b, 0xeb, 0xa9, 0xb1, 0x90,
		0xd4, 0x5c, 0x3e, 0x26, 0x87, 0x83, 0x13, 0x00
	};

	if (offset == 0)
	{
		security_counter = 0;
		return 0;
	}

	return security_data[(security_counter++)];

}

DRIVER_INIT( tkoboxng )
{
	DRIVER_INIT_CALL(MMC3);

	/* security device at $5e00-$5e01 */
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x5e00, 0x5e01, 0, 0, tko_security_r);
}

/* Vs. Freedom Force */

DRIVER_INIT( vsfdf )
{
	DRIVER_INIT_CALL(MMC3);

	memory_install_readwrite8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x4016, 0x4016, 0, 0, gun_in0_r, gun_in0_w);

	vsnes_do_vrom_bank = 0;
}

/**********************************************************************************/
/* Platoon rom banking */

static WRITE8_HANDLER( mapper68_rom_banking )
{
	switch (offset & 0x7000)
	{
		case 0x0000:
		v_set_videorom_bank(space->machine, 0, 2, data * 2);

		break;
		case 0x1000:
		v_set_videorom_bank(space->machine, 2, 2, data * 2);

		break;
		case 0x2000:
		v_set_videorom_bank(space->machine, 4, 2, data * 2);

		break;
		case 0x3000: /* ok? */
		v_set_videorom_bank(space->machine, 6, 2, data * 2);

		break;

		case 0x7000:
		{
			UINT8 *prg = memory_region(space->machine, "maincpu");
			memcpy(&prg[0x08000], &prg[0x10000 + data * 0x4000], 0x4000);
		}
		break;

	}

}

DRIVER_INIT( platoon )
{

	/* when starting a mapper 68 game  the first 16K ROM bank in the cart is loaded into $8000
    the LAST 16K ROM bank is loaded into $C000. The last 16K of ROM cannot be swapped. */

	UINT8 *prg = memory_region(machine, "maincpu");
	memcpy(&prg[0x08000], &prg[0x10000], 0x4000);
	memcpy(&prg[0x0c000], &prg[0x2c000], 0x4000);

	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x8000, 0xffff, 0, 0, mapper68_rom_banking);
}

/**********************************************************************************/
/* Vs. Raid on Bungeling Bay (Japan) */

static int ret;
static WRITE8_HANDLER( set_bnglngby_irq_w )
{
	ret = data;
	cputag_set_input_line(space->machine, "maincpu", 0, (data & 2) ? ASSERT_LINE : CLEAR_LINE);
	/* other values ??? */
	/* 0, 4, 84 */
}

static READ8_HANDLER( set_bnglngby_irq_r )
{
	return ret;
}

DRIVER_INIT( bnglngby )
{
	memory_install_readwrite8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x0231, 0x0231, 0, 0, set_bnglngby_irq_r, set_bnglngby_irq_w);

	/* extra ram */
	memory_install_ram(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x6000, 0x7fff, 0, 0, NULL);

	ret = 0;

	/* normal banking */
	DRIVER_INIT_CALL(vsnormal);
}

/**********************************************************************************/
/* VS Dualsystem */

static WRITE8_HANDLER( vsdual_vrom_banking )
{
	running_device *other_cpu = (space->cpu == space->machine->device("maincpu")) ? space->machine->device("sub") : space->machine->device("maincpu");

	/* switch vrom */
	(space->cpu == space->machine->device("maincpu")) ? memory_set_bank(space->machine, "bank2", BIT(data, 2)) : memory_set_bank(space->machine, "bank3", BIT(data, 2));

	/* bit 1 ( data & 2 ) triggers irq on the other cpu */
	cpu_set_input_line(other_cpu, 0, (data & 2) ? CLEAR_LINE : ASSERT_LINE);

	/* move along */
	if (space->cpu == space->machine->device("maincpu"))
		vsnes_in0_w(space, offset, data);
	else
		vsnes_in0_1_w(space, offset, data);
}

DRIVER_INIT( vsdual )
{
	UINT8 *prg = memory_region(machine, "maincpu");

	/* vrom switching is enabled with bit 2 of $4016 */
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x4016, 0x4016, 0, 0, vsdual_vrom_banking);
	memory_install_write8_handler(cputag_get_address_space(machine, "sub", ADDRESS_SPACE_PROGRAM), 0x4016, 0x4016, 0, 0, vsdual_vrom_banking);

	/* shared ram at $6000 */
	memory_install_ram(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x6000, 0x7fff, 0, 0, &prg[0x6000]);
	memory_install_ram(cputag_get_address_space(machine, "sub", ADDRESS_SPACE_PROGRAM), 0x6000, 0x7fff, 0, 0, &prg[0x6000]);
}

