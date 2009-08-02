#include "driver.h"
#include "video/ppu2c0x.h"
#include "machine/rp5h01.h"
#include "includes/playch10.h"

/* prototypes */
static void pc10_set_mirroring( int mirroring );
static WRITE8_HANDLER( pc10_nt_w );
static READ8_HANDLER( pc10_nt_r );
static WRITE8_HANDLER( pc10_chr_w );
static READ8_HANDLER( pc10_chr_r );
static void pc10_set_videorom_bank( running_machine *machine, int first, int count, int bank, int size );
static void set_videoram_bank( running_machine *machine, int first, int count, int bank, int size );

typedef struct
{
	int writable;	// 1 for RAM, 0 for ROM
	UINT8* chr;		// direct access to the memory
} chr_bank;


/* Globals */
int pc10_sdcs;			/* ShareD Chip Select */
int pc10_dispmask;		/* Display Mask */
int pc10_nmi_enable;	/* nmi enable */
int pc10_dog_di;		/* watchdog disable */
int pc10_int_detect;	/* interrupt detect */
static int pc10_gun_controller;
int pc10_game_mode;		/* Used in Single monitor version to display Game or PPU */
int pc10_dispmask_old;

/* Locals */
static int cart_sel;
static int cntrl_mask;
static int input_latch[2];
static int mirroring;

static int MMC2_bank[4], MMC2_bank_latch[2];

static UINT8* vrom;				// used for games with cart gfx loaded (a,c,e,f,g,h)
static UINT8* vram = NULL;		// used for boards h, b, d, i, k (Presumably, 8K. 16K didn't work in MESS)
static UINT8* nametable[4];		// For non-mirroring boards, this can be moved to a direct mapping
static UINT8* nt_ram = NULL;	// Per-board size! Some boards clearly need 4K. Some MAY use 2K. Research needed.
static chr_bank chr_page[8];	// Simple wrapper for ROM/RAM, since we could be banking either (Hboard)

/*************************************
 *
 *  Init machine
 *
 *************************************/

MACHINE_RESET( pc10 )
{
	const device_config *rp5h01 = devtag_get_device(machine, "rp5h01");

	/* initialize latches and flip-flops */
	pc10_nmi_enable = pc10_dog_di = pc10_dispmask = pc10_sdcs = pc10_int_detect = 0;

	pc10_game_mode = pc10_dispmask_old = 0;

	cart_sel = 0;
	cntrl_mask = 1;

	input_latch[0] = input_latch[1] = 0;

	/* variables used only in MMC2 game (mapper 9)  */
	MMC2_bank[0] = MMC2_bank[1] = MMC2_bank[2] = MMC2_bank[3] = 0;
	MMC2_bank_latch[0] = MMC2_bank_latch[1] = 0xfe;

	/* reset the security chip */
	rp5h01_enable_w(rp5h01, 0, 0);
	rp5h01_reset_w(rp5h01, 0, 0);
	rp5h01_reset_w(rp5h01, 0, 1);
	rp5h01_enable_w(rp5h01, 0, 1);

	pc10_set_mirroring(mirroring);
}

MACHINE_START( pc10 )
{
	vrom = memory_region(machine, "gfx2");

	/* allocate 4K of nametable ram here */
	/* move to individual boards as documentation of actual boards allows */
	nt_ram = auto_alloc_array(machine, UINT8, 0x1000);

	memory_install_readwrite8_handler(cpu_get_address_space(cputag_get_cpu(machine, "ppu"), ADDRESS_SPACE_PROGRAM), 0, 0x1fff, 0, 0, pc10_chr_r, pc10_chr_w);
	memory_install_readwrite8_handler(cpu_get_address_space(cputag_get_cpu(machine, "ppu"), ADDRESS_SPACE_PROGRAM), 0x2000, 0x3eff, 0, 0, pc10_nt_r, pc10_nt_w);

	if (NULL != vram)
		set_videoram_bank(machine, 0, 8, 0, 8);
	else pc10_set_videorom_bank(machine, 0, 8, 0, 8);

}

MACHINE_START( playch10_hboard )
{
	vrom = memory_region(machine, "gfx2");

	/* allocate 4K of nametable ram here */
	/* move to individual boards as documentation of actual boards allows */
	nt_ram = auto_alloc_array(machine, UINT8, 0x1000);
	/* allocate vram */

	vram = auto_alloc_array(machine, UINT8, 0x2000);

	memory_install_readwrite8_handler(cpu_get_address_space(cputag_get_cpu(machine, "ppu"), ADDRESS_SPACE_PROGRAM), 0, 0x1fff, 0, 0, pc10_chr_r, pc10_chr_w);
	memory_install_readwrite8_handler(cpu_get_address_space(cputag_get_cpu(machine, "ppu"), ADDRESS_SPACE_PROGRAM), 0x2000, 0x3eff, 0, 0, pc10_nt_r, pc10_nt_w);
}

/*************************************
 *
 *  BIOS ports handling
 *
 *************************************/

CUSTOM_INPUT( pc10_int_detect_r )
{
	return ~pc10_int_detect & 1;
}

WRITE8_HANDLER( pc10_SDCS_w )
{
	/*
        Hooked to CLR on LS194A - Sheet 2, bottom left.
        Drives character and color code to 0.
        It's used to keep the screen black during redraws.
        Also hooked to the video sram. Prevent writes.
    */
	pc10_sdcs = ~data & 1;
}

WRITE8_HANDLER( pc10_CNTRLMASK_w )
{
	cntrl_mask = ~data & 1;
}

WRITE8_HANDLER( pc10_DISPMASK_w )
{
	pc10_dispmask = ~data & 1;
}

WRITE8_HANDLER( pc10_SOUNDMASK_w )
{
	/* should mute the APU - unimplemented yet */
}

WRITE8_HANDLER( pc10_NMIENABLE_w )
{
	pc10_nmi_enable = data & 1;
}

WRITE8_HANDLER( pc10_DOGDI_w )
{
	pc10_dog_di = data & 1;
}

WRITE8_HANDLER( pc10_GAMERES_w )
{
	cputag_set_input_line(space->machine, "cart", INPUT_LINE_RESET, (data & 1) ? CLEAR_LINE : ASSERT_LINE );
}

WRITE8_HANDLER( pc10_GAMESTOP_w )
{
	cputag_set_input_line(space->machine, "cart", INPUT_LINE_HALT, (data & 1) ? CLEAR_LINE : ASSERT_LINE );
}

WRITE8_HANDLER( pc10_PPURES_w )
{
	if (data & 1)
		devtag_reset(space->machine, "ppu");
}

READ8_HANDLER( pc10_detectclr_r )
{
	pc10_int_detect = 0;

	return 0;
}

WRITE8_HANDLER( pc10_CARTSEL_w )
{
	cart_sel &= ~(1 << offset);
	cart_sel |= (data & 1) << offset;
}


/*************************************
 *
 *  RP5H01 handling
 *
 *************************************/

READ8_HANDLER( pc10_prot_r )
{
	const device_config *rp5h01 = devtag_get_device(space->machine, "rp5h01");
	int data = 0xe7;

	/* we only support a single cart connected at slot 0 */
	if (cart_sel == 0)
	{
		rp5h01_enable_w(rp5h01, 0, 0);
		data |= ((~rp5h01_counter_r(rp5h01, 0)) << 4) & 0x10;	/* D4 */
		data |= ((rp5h01_data_r(rp5h01, 0)) << 3) & 0x08;		/* D3 */
		rp5h01_enable_w(rp5h01, 0, 1);
	}
	return data;
}

WRITE8_HANDLER( pc10_prot_w )
{
	const device_config *rp5h01 = devtag_get_device(space->machine, "rp5h01");
	/* we only support a single cart connected at slot 0 */
	if (cart_sel == 0)
	{
		rp5h01_enable_w(rp5h01, 0, 0);
		rp5h01_test_w(rp5h01, 0, data & 0x10);		/* D4 */
		rp5h01_clock_w(rp5h01, 0, data & 0x08);		/* D3 */
		rp5h01_reset_w(rp5h01, 0, ~data & 0x01);	/* D0 */
		rp5h01_enable_w(rp5h01, 0, 1);

		/* this thing gets dense at some point                      */
		/* it wants to jump and execute an opcode at $ffff, wich    */
		/* is the actual protection memory area                     */
		/* setting the whole 0x2000 region every time is a waste    */
		/* so we just set $ffff with the current value              */
		memory_region(space->machine, "maincpu")[0xffff] = pc10_prot_r(space, 0);
	}
}


/*************************************
 *
 *  Input Ports
 *
 *************************************/

WRITE8_HANDLER( pc10_in0_w )
{
	/* Toggling bit 0 high then low resets both controllers */
	if (data & 1)
		return;

	/* load up the latches */
	input_latch[0] = input_port_read(space->machine, "P1");
	input_latch[1] = input_port_read(space->machine, "P2");

	/* apply any masking from the BIOS */
	if (cntrl_mask)
	{
		/* mask out select and start */
		input_latch[0] &= ~0x0c;
	}
}

READ8_HANDLER( pc10_in0_r )
{
	int ret = (input_latch[0]) & 1;

	/* shift */
	input_latch[0] >>= 1;

	/* some games expect bit 6 to be set because the last entry on the data bus shows up */
	/* in the unused upper 3 bits, so typically a read from $4016 leaves 0x40 there. */
	ret |= 0x40;

	return ret;
}

READ8_HANDLER( pc10_in1_r )
{
	int ret = (input_latch[1]) & 1;

	/* shift */
	input_latch[1] >>= 1;

	/* do the gun thing */
	if (pc10_gun_controller)
	{
		const device_config *ppu = devtag_get_device(space->machine, "ppu");
		int trigger = input_port_read(space->machine, "P1");
		int x = input_port_read(space->machine, "GUNX");
		int y = input_port_read(space->machine, "GUNY");
		UINT32 pix, color_base;

		/* no sprite hit (yet) */
		ret |= 0x08;

		/* get the pixel at the gun position */
		pix = ppu2c0x_get_pixel(ppu, x, y);

		/* get the color base from the ppu */
		color_base = ppu2c0x_get_colorbase(ppu);

		/* look at the screen and see if the cursor is over a bright pixel */
		if ((pix == color_base + 0x20) || (pix == color_base + 0x30) ||
			(pix == color_base + 0x33) || (pix == color_base + 0x34))
		{
			ret &= ~0x08; /* sprite hit */
		}

		/* now, add the trigger if not masked */
		if (!cntrl_mask)
		{
			ret |= (trigger & 2) << 3;
		}
	}

	/* some games expect bit 6 to be set because the last entry on the data bus shows up */
	/* in the unused upper 3 bits, so typically a read from $4016 leaves 0x40 there. */
	ret |= 0x40;

	return ret;
}
/*************************************
 *
 *  PPU External bus handlers
 *
 *************************************/

static WRITE8_HANDLER( pc10_nt_w )
{
	int page = ((offset & 0xc00) >> 10);
	nametable[page][offset & 0x3ff] = data;
}

static READ8_HANDLER( pc10_nt_r )
{
	int page = ((offset & 0xc00) >> 10);
	return nametable[page][offset & 0x3ff];
}

static WRITE8_HANDLER( pc10_chr_w )
{
	int bank = offset >> 10;
	if (chr_page[bank].writable)
	{
		chr_page[bank].chr[offset & 0x3ff] = data;
	}
}

static READ8_HANDLER( pc10_chr_r )
{
	int bank = offset >> 10;
	return chr_page[bank].chr[offset & 0x3ff];
}

static void pc10_set_mirroring( int mirroring )
{
	switch (mirroring)
	{
	case PPU_MIRROR_LOW:
		nametable[0] = nametable[1] = nametable[2] = nametable[3] = nt_ram;
		break;
	case PPU_MIRROR_HIGH:
		nametable[0] = nametable[1] = nametable[2] = nametable[3] = nt_ram + 0x400;
		break;
	case PPU_MIRROR_HORZ:
		nametable[0] = nt_ram;
		nametable[1] = nt_ram;
		nametable[2] = nt_ram + 0x400;
		nametable[3] = nt_ram + 0x400;
		break;
	case PPU_MIRROR_VERT:
		nametable[0] = nt_ram;
		nametable[1] = nt_ram + 0x400;
		nametable[2] = nt_ram;
		nametable[3] = nt_ram + 0x400;
		break;
	case PPU_MIRROR_NONE:
	default:
		nametable[0] = nt_ram;
		nametable[1] = nt_ram + 0x400;
		nametable[2] = nt_ram + 0x800;
		nametable[3] = nt_ram + 0xc00;
		break;
	}
}

/* SIZE MAPPINGS *\
 * old       new *
 * 512         8 *
 * 256         4 *
 * 128         2 *
 *  64         1 *
\*****************/

static void pc10_set_videorom_bank( running_machine *machine, int first, int count, int bank, int size )
{
	int i, len;
	/* first = first bank to map */
	/* count = number of 1K banks to map */
	/* bank = index of the bank */
	/* size = size of indexed banks (in KB) */
	/* note that this follows the original PPU banking and might be overly complex */

	/* yeah, this is probably a horrible assumption to make.*/
	/* but the driver is 100% consistant */

	len = memory_region_length(machine, "gfx2");
	len /= 0x400;	// convert to KB
	len /= size;	// convert to bank resolution
	len--;			// convert to mask
	bank &= len;	// should be the right mask

	for (i = 0; i < count; i++)
	{
		chr_page[i + first].writable = 0;
		chr_page[i + first].chr=vrom + (i * 0x400) + (bank * size * 0x400);
	}
}

static void set_videoram_bank( running_machine *machine, int first, int count, int bank, int size )
{
	int i;
	/* first = first bank to map */
	/* count = number of 1K banks to map */
	/* bank = index of the bank */
	/* size = size of indexed banks (in KB) */
	/* note that this follows the original PPU banking and might be overly complex */

	/* assumes 8K of vram */
	/* need 8K to fill address space */
	/* only pinbot (8k) banks at all */

	for (i = 0; i < count; i++)
	{
		chr_page[i + first].writable = 1;
		chr_page[i + first].chr = vram + (((i * 0x400) + (bank * size * 0x400)) & 0x1fff);
	}
}

/*************************************
 *
 *  Common init for all games
 *
 *************************************/

DRIVER_INIT( playch10 )
{
	vram = NULL;

	/* set the controller to default */
	pc10_gun_controller = 0;

	/* default mirroring */
	mirroring = PPU_MIRROR_NONE;
}

/**********************************************************************************
 *
 *  Game and Board-specific initialization
 *
 **********************************************************************************/

/* Gun games */

DRIVER_INIT( pc_gun )
{
	/* common init */
	DRIVER_INIT_CALL(playch10);

	/* we have no vram, make sure switching games doesn't point to an old allocation */
	vram = NULL;

	/* set the control type */
	pc10_gun_controller = 1;
}


/* Horizontal mirroring */

DRIVER_INIT( pc_hrz )
{
	/* common init */
	DRIVER_INIT_CALL(playch10);

	/* setup mirroring */
	mirroring = PPU_MIRROR_HORZ;
}

/* MMC1 mapper, used by D and F boards */

static int mmc1_shiftreg;
static int mmc1_shiftcount;
static int mmc1_rom_mask;

static WRITE8_HANDLER( mmc1_rom_switch_w )
{
	/* basically, a MMC1 mapper from the nes */
	static int size16k, switchlow, vrom4k;

	int reg = (offset >> 13);

	/* reset mapper */
	if (data & 0x80)
	{
		mmc1_shiftreg = mmc1_shiftcount = 0;

		size16k = 1;
		switchlow = 1;
		vrom4k = 0;

		return;
	}

	/* see if we need to clock in data */
	if (mmc1_shiftcount < 5)
	{
		mmc1_shiftreg >>= 1;
		mmc1_shiftreg |= (data & 1) << 4;
		mmc1_shiftcount++;
	}

	/* are we done shifting? */
	if (mmc1_shiftcount == 5)
	{
		/* reset count */
		mmc1_shiftcount = 0;

		/* apply data to registers */
		switch (reg)
		{
			case 0:		/* mirroring and options */
				{
					int _mirroring;

					vrom4k = mmc1_shiftreg & 0x10;
					size16k = mmc1_shiftreg & 0x08;
					switchlow = mmc1_shiftreg & 0x04;

					switch (mmc1_shiftreg & 3)
					{
						case 0:
							_mirroring = PPU_MIRROR_LOW;
						break;

						case 1:
							_mirroring = PPU_MIRROR_HIGH;
						break;

						case 2:
							_mirroring = PPU_MIRROR_VERT;
						break;

						default:
						case 3:
							_mirroring = PPU_MIRROR_HORZ;
						break;
					}

					/* apply mirroring */
					pc10_set_mirroring(_mirroring);
				}
			break;

			case 1:	/* video rom banking - bank 0 - 4k or 8k */
				pc10_set_videorom_bank(space->machine, 0, (vrom4k) ? 4 : 8, (mmc1_shiftreg & 0x1f), 4);
			break;

			case 2: /* video rom banking - bank 1 - 4k only */
				if (vrom4k)
					pc10_set_videorom_bank(space->machine, 4, 4, (mmc1_shiftreg & 0x1f), 4);
			break;

			case 3:	/* program banking */
				{
					int bank = (mmc1_shiftreg & mmc1_rom_mask) * 0x4000;
					UINT8 *prg = memory_region(space->machine, "cart");

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
	}
}

/**********************************************************************************/
/* A Board games (Track & Field, Gradius) */

static WRITE8_HANDLER( aboard_vrom_switch_w )
{
	pc10_set_videorom_bank(space->machine, 0, 8, (data & 3), 8);
}

DRIVER_INIT( pcaboard )
{
	/* switches vrom with writes to the $803e-$8041 area */
	memory_install_write8_handler(cputag_get_address_space(machine, "cart", ADDRESS_SPACE_PROGRAM), 0x8000, 0x8fff, 0, 0, aboard_vrom_switch_w );

	/* common init */
	DRIVER_INIT_CALL(playch10);

	/* set the mirroring here */
	mirroring = PPU_MIRROR_VERT;

	/* we have no vram, make sure switching games doesn't point to an old allocation */
	vram = NULL;
}

/**********************************************************************************/
/* B Board games (Contra, Rush N' Attach, Pro Wrestling) */

static WRITE8_HANDLER( bboard_rom_switch_w )
{
	int bankoffset = 0x10000 + ((data & 7) * 0x4000);
	UINT8 *prg = memory_region(space->machine, "cart");

	memcpy(&prg[0x08000], &prg[bankoffset], 0x4000);
}

DRIVER_INIT( pcbboard )
{
	UINT8 *prg = memory_region(machine, "cart");

	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	memcpy(&prg[0x08000], &prg[0x28000], 0x8000);

	/* Roms are banked at $8000 to $bfff */
	memory_install_write8_handler(cputag_get_address_space(machine, "cart", ADDRESS_SPACE_PROGRAM), 0x8000, 0xffff, 0, 0, bboard_rom_switch_w );

	/* common init */
	DRIVER_INIT_CALL(playch10);

	/* allocate vram */
	vram = auto_alloc_array(machine, UINT8, 0x2000);

	/* set the mirroring here */
	mirroring = PPU_MIRROR_VERT;
	/* special init */
	set_videoram_bank(machine, 0, 8, 0, 8);
}

/**********************************************************************************/
/* C Board games (The Goonies) */

static WRITE8_HANDLER( cboard_vrom_switch_w )
{
	pc10_set_videorom_bank(space->machine, 0, 8, ((data >> 1) & 1), 8);
}

DRIVER_INIT( pccboard )
{
	/* switches vrom with writes to $6000 */
	memory_install_write8_handler(cputag_get_address_space(machine, "cart", ADDRESS_SPACE_PROGRAM), 0x6000, 0x6000, 0, 0, cboard_vrom_switch_w );

	/* we have no vram, make sure switching games doesn't point to an old allocation */
	vram = NULL;

	/* common init */
	DRIVER_INIT_CALL(playch10);
}

/**********************************************************************************/
/* D Board games (Rad Racer) */

DRIVER_INIT( pcdboard )
{
	UINT8 *prg = memory_region(machine, "cart");

	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	memcpy(&prg[0x08000], &prg[0x28000], 0x8000);

	mmc1_rom_mask = 0x07;

	/* MMC mapper at writes to $8000-$ffff */
	memory_install_write8_handler(cputag_get_address_space(machine, "cart", ADDRESS_SPACE_PROGRAM), 0x8000, 0xffff, 0, 0, mmc1_rom_switch_w );


	/* common init */
	DRIVER_INIT_CALL(playch10);
	/* allocate vram */
	vram = auto_alloc_array(machine, UINT8, 0x2000);
	/* special init */
	set_videoram_bank(machine, 0, 8, 0, 8);
}

/* D Board games with extra ram (Metroid) */

DRIVER_INIT( pcdboard_2 )
{
	/* extra ram at $6000-$7fff */
	memory_install_readwrite8_handler(cputag_get_address_space(machine, "cart", ADDRESS_SPACE_PROGRAM), 0x6000, 0x7fff, 0, 0, (read8_space_func)SMH_BANK(1), (write8_space_func)SMH_BANK(1) );
	memory_set_bankptr(machine, 1, auto_alloc_array(machine, UINT8, 0x2000));

	/* common init */
	DRIVER_INIT_CALL(pcdboard);

	/* allocate vram */
	vram = auto_alloc_array(machine, UINT8, 0x2000);
	/* special init */
	set_videoram_bank(machine, 0, 8, 0, 8);
}

/**********************************************************************************/
/* E Board games (Mike Tyson's Punchout) - BROKEN - FIX ME */

/* callback for the ppu_latch */
static void mapper9_latch( const device_config *ppu, offs_t offset )
{

	if((offset & 0x1ff0) == 0x0fd0 && MMC2_bank_latch[0] != 0xfd)
	{
		MMC2_bank_latch[0] = 0xfd;
		pc10_set_videorom_bank(ppu->space[0]->machine, 0, 4, MMC2_bank[0], 4);
	}
	else if((offset & 0x1ff0) == 0x0fe0 && MMC2_bank_latch[0] != 0xfe)
	{
		MMC2_bank_latch[0] = 0xfe;
		pc10_set_videorom_bank(ppu->space[0]->machine, 0, 4, MMC2_bank[1], 4);
	}
	else if((offset & 0x1ff0) == 0x1fd0 && MMC2_bank_latch[1] != 0xfd)
	{
		MMC2_bank_latch[1] = 0xfd;
		pc10_set_videorom_bank(ppu->space[0]->machine, 4, 4, MMC2_bank[2], 4);
	}
	else if((offset & 0x1ff0) == 0x1fe0 && MMC2_bank_latch[1] != 0xfe)
	{
		MMC2_bank_latch[1] = 0xfe;
		pc10_set_videorom_bank(ppu->space[0]->machine, 4, 4, MMC2_bank[3], 4);
	}
}

static WRITE8_HANDLER( eboard_rom_switch_w )
{
	/* a variation of mapper 9 on a nes */
	switch (offset & 0x7000)
	{
		case 0x2000: /* code bank switching */
			{
				int bankoffset = 0x10000 + (data & 0x0f) * 0x2000;
				UINT8 *prg = memory_region(space->machine, "cart");
				memcpy(&prg[0x08000], &prg[bankoffset], 0x2000);
			}
		break;

		case 0x3000: /* gfx bank 0 - 4k */
			MMC2_bank[0] = data;
			if (MMC2_bank_latch[0] == 0xfd)
				pc10_set_videorom_bank(space->machine, 0, 4, data, 4);
		break;

		case 0x4000: /* gfx bank 0 - 4k */
			MMC2_bank[1] = data;
			if (MMC2_bank_latch[0] == 0xfe)
				pc10_set_videorom_bank(space->machine, 0, 4, data, 4);
		break;

		case 0x5000: /* gfx bank 1 - 4k */
			MMC2_bank[2] = data;
			if (MMC2_bank_latch[1] == 0xfd)
				pc10_set_videorom_bank(space->machine, 4, 4, data, 4);
		break;

		case 0x6000: /* gfx bank 1 - 4k */
			MMC2_bank[3] = data;
			if (MMC2_bank_latch[1] == 0xfe)
				pc10_set_videorom_bank(space->machine, 4, 4, data, 4);
		break;

		case 0x7000: /* mirroring */
			pc10_set_mirroring(data ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);

		break;
	}
}

DRIVER_INIT( pceboard )
{
	UINT8 *prg = memory_region(machine, "cart");

	/* we have no vram, make sure switching games doesn't point to an old allocation */
	vram = NULL;

	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	memcpy(&prg[0x08000], &prg[0x28000], 0x8000);

	/* basically a mapper 9 on a nes */
	memory_install_write8_handler(cputag_get_address_space(machine, "cart", ADDRESS_SPACE_PROGRAM), 0x8000, 0xffff, 0, 0, eboard_rom_switch_w );

	/* ppu_latch callback */
	ppu_latch = mapper9_latch;

	/* nvram at $6000-$6fff */
	memory_install_readwrite8_handler(cputag_get_address_space(machine, "cart", ADDRESS_SPACE_PROGRAM), 0x6000, 0x6fff, 0, 0, (read8_space_func)SMH_BANK(1), (write8_space_func)SMH_BANK(1) );
	memory_set_bankptr(machine, 1, auto_alloc_array(machine, UINT8, 0x1000));

	/* common init */
	DRIVER_INIT_CALL(playch10);
}

/**********************************************************************************/
/* F Board games (Ninja Gaiden, Double Dragon) */

DRIVER_INIT( pcfboard )
{
	UINT8 *prg = memory_region(machine, "cart");

	/* we have no vram, make sure switching games doesn't point to an old allocation */
	vram = NULL;

	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	memcpy(&prg[0x08000], &prg[0x28000], 0x8000);

	mmc1_rom_mask = 0x07;

	/* MMC mapper at writes to $8000-$ffff */
	memory_install_write8_handler(cputag_get_address_space(machine, "cart", ADDRESS_SPACE_PROGRAM), 0x8000, 0xffff, 0, 0, mmc1_rom_switch_w );

	/* common init */
	DRIVER_INIT_CALL(playch10);
}

/* F Board games with extra ram (Baseball Stars) */

DRIVER_INIT( pcfboard_2 )
{
	/* extra ram at $6000-$6fff */
	memory_install_readwrite8_handler(cputag_get_address_space(machine, "cart", ADDRESS_SPACE_PROGRAM), 0x6000, 0x6fff, 0, 0, (read8_space_func)SMH_BANK(1), (write8_space_func)SMH_BANK(1) );
	memory_set_bankptr(machine, 1, auto_alloc_array(machine, UINT8, 0x1000));

	vram = NULL;

	/* common init */
	DRIVER_INIT_CALL(pcfboard);
}

/**********************************************************************************/
/* G Board games (Super Mario Bros. 3) */

static int gboard_scanline_counter;
static int gboard_scanline_latch;
static int gboard_banks[2];
static int gboard_4screen;
static int gboard_last_bank;
static int gboard_command;

static void gboard_scanline_cb( const device_config *device, int scanline, int vblank, int blanked )
{
	if (!vblank && !blanked)
	{
		if (--gboard_scanline_counter == -1)
		{
			gboard_scanline_counter = gboard_scanline_latch;
			generic_pulse_irq_line(cputag_get_cpu(device->machine, "cart"), 0);
		}
	}
}

static WRITE8_HANDLER( gboard_rom_switch_w )
{
	const device_config *ppu = devtag_get_device(space->machine, "ppu");

	/* basically, a MMC3 mapper from the nes */

	switch (offset & 0x7001)
	{
		case 0x0000:
			gboard_command = data;

			if (gboard_last_bank != (data & 0xc0))
			{
				int bank;
				UINT8 *prg = memory_region(space->machine, "cart");

				/* reset the banks */
				if (gboard_command & 0x40)
				{
					/* high bank */
					bank = gboard_banks[0] * 0x2000 + 0x10000;

					memcpy(&prg[0x0c000], &prg[bank], 0x2000);
					memcpy(&prg[0x08000], &prg[0x4c000], 0x2000);
				}
				else
				{
					/* low bank */
					bank = gboard_banks[0] * 0x2000 + 0x10000;

					memcpy(&prg[0x08000], &prg[bank], 0x2000);
					memcpy(&prg[0x0c000], &prg[0x4c000], 0x2000);
				}

				/* mid bank */
				bank = gboard_banks[1] * 0x2000 + 0x10000;
				memcpy(&prg[0x0a000], &prg[bank], 0x2000);

				gboard_last_bank = data & 0xc0;
			}
		break;

		case 0x0001:
			{
				UINT8 cmd = gboard_command & 0x07;
				int page = (gboard_command & 0x80) >> 5;
				int bank;

				switch (cmd)
				{
					case 0:	/* char banking */
					case 1: /* char banking */
						data &= 0xfe;
						page ^= (cmd << 1);
						pc10_set_videorom_bank(space->machine, page, 2, data, 1);
					break;

					case 2: /* char banking */
					case 3: /* char banking */
					case 4: /* char banking */
					case 5: /* char banking */
						page ^= cmd + 2;
						pc10_set_videorom_bank(space->machine, page, 1, data, 1);
					break;

					case 6: /* program banking */
					{
						UINT8 *prg = memory_region(space->machine, "cart");
						if (gboard_command & 0x40)
						{
							/* high bank */
							gboard_banks[0] = data & 0x1f;
							bank = (gboard_banks[0]) * 0x2000 + 0x10000;

							memcpy(&prg[0x0c000], &prg[bank], 0x2000);
							memcpy(&prg[0x08000], &prg[0x4c000], 0x2000);
						}
						else
						{
							/* low bank */
							gboard_banks[0] = data & 0x1f;
							bank = (gboard_banks[0]) * 0x2000 + 0x10000;

							memcpy(&prg[0x08000], &prg[bank], 0x2000);
							memcpy(&prg[0x0c000], &prg[0x4c000], 0x2000);
						}
					}
					break;

					case 7: /* program banking */
						{
							/* mid bank */
							UINT8 *prg = memory_region(space->machine, "cart");
							gboard_banks[1] = data & 0x1f;
							bank = gboard_banks[1] * 0x2000 + 0x10000;

							memcpy(&prg[0x0a000], &prg[bank], 0x2000);
						}
					break;
				}
			}
		break;

		case 0x2000: /* mirroring */
			if (!gboard_4screen)
			{
				if (data & 0x40)
					pc10_set_mirroring(PPU_MIRROR_HIGH);
				else
					pc10_set_mirroring((data & 1) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			}
		break;

		case 0x2001: /* enable ram at $6000 */
			/* ignored - we always enable it */
		break;

		case 0x4000: /* scanline counter */
			gboard_scanline_counter = data;
		break;

		case 0x4001: /* scanline latch */
			gboard_scanline_latch = data;
		break;

		case 0x6000: /* disable irqs */
			ppu2c0x_set_scanline_callback(ppu, 0);
		break;

		case 0x6001: /* enable irqs */
			ppu2c0x_set_scanline_callback(ppu, gboard_scanline_cb);
		break;
	}
}

DRIVER_INIT( pcgboard )
{
	UINT8 *prg = memory_region(machine, "cart");
	vram = NULL;

	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	memcpy(&prg[0x08000], &prg[0x4c000], 0x4000);
	memcpy(&prg[0x0c000], &prg[0x4c000], 0x4000);

	/* MMC3 mapper at writes to $8000-$ffff */
	memory_install_write8_handler(cputag_get_address_space(machine, "cart", ADDRESS_SPACE_PROGRAM), 0x8000, 0xffff, 0, 0, gboard_rom_switch_w );

	/* extra ram at $6000-$7fff */
	memory_install_readwrite8_handler(cputag_get_address_space(machine, "cart", ADDRESS_SPACE_PROGRAM), 0x6000, 0x7fff, 0, 0, (read8_space_func)SMH_BANK(1), (write8_space_func)SMH_BANK(1) );
	memory_set_bankptr(machine, 1, auto_alloc_array(machine, UINT8, 0x2000));

	gboard_banks[0] = 0x1e;
	gboard_banks[1] = 0x1f;
	gboard_scanline_counter = 0;
	gboard_scanline_latch = 0;
	gboard_4screen = 0;

	/* common init */
	DRIVER_INIT_CALL(playch10);
}

DRIVER_INIT( pcgboard_type2 )
{
	vram = NULL;
	/* common init */
	DRIVER_INIT_CALL(pcgboard);

	/* enable 4 screen mirror */
	gboard_4screen = 1;
}

/**********************************************************************************/
/* i Board games (Captain Sky Hawk, Solar Jetman) */

static WRITE8_HANDLER( iboard_rom_switch_w )
{
	int bank = data & 7;
	UINT8 *prg = memory_region(space->machine, "cart");

	if (data & 0x10)
		pc10_set_mirroring(PPU_MIRROR_HIGH);
	else
		pc10_set_mirroring(PPU_MIRROR_LOW);

	memcpy(&prg[0x08000], &prg[bank * 0x8000 + 0x10000], 0x8000);
}

DRIVER_INIT( pciboard )
{
	UINT8 *prg = memory_region(machine, "cart");

	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	memcpy(&prg[0x08000], &prg[0x10000], 0x8000);

	/* Roms are banked at $8000 to $bfff */
	memory_install_write8_handler(cputag_get_address_space(machine, "cart", ADDRESS_SPACE_PROGRAM), 0x8000, 0xffff, 0, 0, iboard_rom_switch_w );

	/* common init */
	DRIVER_INIT_CALL(playch10);

	/* allocate vram */
	vram = auto_alloc_array(machine, UINT8, 0x2000);
	/* special init */
	set_videoram_bank(machine, 0, 8, 0, 8);
}

/**********************************************************************************/
/* H Board games (PinBot) */

static WRITE8_HANDLER( hboard_rom_switch_w )
{
	switch (offset & 0x7001)
	{
		case 0x0001:
			{
				UINT8 cmd = gboard_command & 0x07;
				int page = (gboard_command & 0x80) >> 5;

				switch (cmd)
				{
					case 0:	/* char banking */
					case 1: /* char banking */
						data &= 0xfe;
						page ^= (cmd << 1);
						if (data & 0x40)
						{
							set_videoram_bank(space->machine, page, 2, data, 1);
						}
						else
						{
							pc10_set_videorom_bank(space->machine, page, 2, data, 1);
						}
					return;

					case 2: /* char banking */
					case 3: /* char banking */
					case 4: /* char banking */
					case 5: /* char banking */
						page ^= cmd + 2;
						if (data & 0x40)
						{
							set_videoram_bank(space->machine, page, 1, data, 1);
						}
						else
						{
							pc10_set_videorom_bank(space->machine, page, 1, data, 1);
						}
					return;
				}
			}
	};
	gboard_rom_switch_w(space,offset,data);
};


DRIVER_INIT( pchboard )
{
	UINT8 *prg = memory_region(machine, "cart");
	memcpy(&prg[0x08000], &prg[0x4c000], 0x4000);
	memcpy(&prg[0x0c000], &prg[0x4c000], 0x4000);

	/* Roms are banked at $8000 to $bfff */
	memory_install_write8_handler(cputag_get_address_space(machine, "cart", ADDRESS_SPACE_PROGRAM), 0x8000, 0xffff, 0, 0, hboard_rom_switch_w );

	/* extra ram at $6000-$7fff */
	memory_install_readwrite8_handler(cputag_get_address_space(machine, "cart", ADDRESS_SPACE_PROGRAM), 0x6000, 0x7fff, 0, 0, (read8_space_func)SMH_BANK(1), (write8_space_func)SMH_BANK(1) );
	memory_set_bankptr(machine, 1, auto_alloc_array(machine, UINT8, 0x2000));

	gboard_banks[0] = 0x1e;
	gboard_banks[1] = 0x1f;
	gboard_scanline_counter = 0;
	gboard_scanline_latch = 0;
	gboard_last_bank = 0xff;
	gboard_command = 0;

	/* common init */
	DRIVER_INIT_CALL(playch10);
}

/**********************************************************************************/
/* K Board games (Mario Open Golf) */

DRIVER_INIT( pckboard )
{
	UINT8 *prg = memory_region(machine, "cart");

	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	memcpy(&prg[0x08000], &prg[0x48000], 0x8000);

	mmc1_rom_mask = 0x0f;

	/* extra ram at $6000-$7fff */
	memory_install_readwrite8_handler(cputag_get_address_space(machine, "cart", ADDRESS_SPACE_PROGRAM), 0x6000, 0x7fff, 0, 0, (read8_space_func)SMH_BANK(1), (write8_space_func)SMH_BANK(1) );
	memory_set_bankptr(machine, 1, auto_alloc_array(machine, UINT8, 0x2000));

	/* Roms are banked at $8000 to $bfff */
	memory_install_write8_handler(cputag_get_address_space(machine, "cart", ADDRESS_SPACE_PROGRAM), 0x8000, 0xffff, 0, 0, mmc1_rom_switch_w );

	/* common init */
	DRIVER_INIT_CALL(playch10);

	/* allocate vram */
	vram = auto_alloc_array(machine, UINT8, 0x2000);
	/* special init */
	set_videoram_bank(machine, 0, 8, 0, 8);
}
