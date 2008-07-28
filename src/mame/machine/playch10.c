#include "driver.h"
#include "deprecat.h"
#include "video/ppu2c0x.h"
#include "machine/rp5h01.h"
#include "includes/playch10.h"

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

/*************************************
 *
 *  Init machine
 *
 *************************************/
MACHINE_RESET( pc10 )
{
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
	RP5H01_enable_w( 0, 0 );
	RP5H01_0_reset_w( machine, 0, 0 );
	RP5H01_0_reset_w( machine, 0, 1 );
	RP5H01_enable_w( 0, 1 );

	/* reset the ppu */
	ppu2c0x_reset( machine, 0, /* video_screen_get_scan_period(machine->primary_screen) * */ 1 );

	ppu2c0x_set_mirroring( 0, mirroring );
}

/*************************************
 *
 *  BIOS ports handling
 *
 *************************************/
READ8_HANDLER( pc10_port_0_r )
{
	return input_port_read(machine, "BIOS") | ( ( ~pc10_int_detect & 1 ) << 3 );
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
	cpunum_set_input_line(machine, 1, INPUT_LINE_RESET, ( data & 1 ) ? CLEAR_LINE : ASSERT_LINE );
}

WRITE8_HANDLER( pc10_GAMESTOP_w )
{
	cpunum_set_input_line(machine, 1, INPUT_LINE_HALT, ( data & 1 ) ? CLEAR_LINE : ASSERT_LINE );
}

WRITE8_HANDLER( pc10_PPURES_w )
{
	if ( data & 1 )
		ppu2c0x_reset( machine, 0, /* video_screen_get_scan_period(machine->primary_screen) * */ 1 );
}

READ8_HANDLER( pc10_detectclr_r )
{
	pc10_int_detect = 0;

	return 0;
}

WRITE8_HANDLER( pc10_CARTSEL_w )
{
	cart_sel &= ~( 1 << offset );
	cart_sel |= ( data & 1 ) << offset;
}


/*************************************
 *
 *  RP5H01 handling
 *
 *************************************/
READ8_HANDLER( pc10_prot_r )
{
	int data = 0xe7;

	/* we only support a single cart connected at slot 0 */
	if ( cart_sel == 0 )
	{
		RP5H01_0_enable_w( machine, 0, 0 );
		data |= ( ( ~RP5H01_counter_r( 0 ) ) << 4 ) & 0x10;	/* D4 */
		data |= ( ( RP5H01_data_r( 0 ) ) << 3 ) & 0x08;		/* D3 */
		RP5H01_0_enable_w( machine, 0, 1 );
	}
	return data;
}

WRITE8_HANDLER( pc10_prot_w )
{
	/* we only support a single cart connected at slot 0 */
	if ( cart_sel == 0 )
	{
		RP5H01_0_enable_w( machine, 0, 0 );
		RP5H01_0_test_w( machine, 0, data & 0x10 );		/* D4 */
		RP5H01_0_clock_w( machine, 0, data & 0x08 );		/* D3 */
		RP5H01_0_reset_w( machine, 0, ~data & 0x01 );	/* D0 */
		RP5H01_0_enable_w( machine, 0, 1 );

		/* this thing gets dense at some point                      */
		/* it wants to jump and execute an opcode at $ffff, wich    */
		/* is the actual protection memory area                     */
		/* setting the whole 0x2000 region every time is a waste    */
		/* so we just set $ffff with the current value              */
		memory_region( machine, "main" )[0xffff] = pc10_prot_r(machine,0);
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
	if ( data & 1 )
		return;

	/* load up the latches */
	input_latch[0] = input_port_read(machine, "P1");
	input_latch[1] = input_port_read(machine, "P2");

	/* apply any masking from the BIOS */
	if ( cntrl_mask )
	{
		/* mask out select and start */
		input_latch[0] &= ~0x0c;
	}
}

READ8_HANDLER( pc10_in0_r )
{
	int ret = ( input_latch[0] ) & 1;

	/* shift */
	input_latch[0] >>= 1;

	/* some games expect bit 6 to be set because the last entry on the data bus shows up */
	/* in the unused upper 3 bits, so typically a read from $4016 leaves 0x40 there. */
	ret |= 0x40;

	return ret;
}

READ8_HANDLER( pc10_in1_r )
{
	int ret = ( input_latch[1] ) & 1;

	/* shift */
	input_latch[1] >>= 1;

	/* do the gun thing */
	if ( pc10_gun_controller )
	{
		int trigger = input_port_read(machine, "P1");
		int x = input_port_read(machine, "GUNX");
		int y = input_port_read(machine, "GUNY");
		UINT32 pix, color_base;

		/* no sprite hit (yet) */
		ret |= 0x08;

		/* get the pixel at the gun position */
		pix = ppu2c0x_get_pixel( 0, x, y );

		/* get the color base from the ppu */
		color_base = ppu2c0x_get_colorbase( 0 );

		/* look at the screen and see if the cursor is over a bright pixel */
		if ( ( pix == color_base+0x20 ) || ( pix == color_base+0x30 ) ||
			 ( pix == color_base+0x33 ) || ( pix == color_base+0x34 ) )
		{
			ret &= ~0x08; /* sprite hit */
		}

		/* now, add the trigger if not masked */
		if ( !cntrl_mask )
		{
			ret |= ( trigger & 2 ) << 3;
		}
	}

	/* some games expect bit 6 to be set because the last entry on the data bus shows up */
	/* in the unused upper 3 bits, so typically a read from $4016 leaves 0x40 there. */
	ret |= 0x40;

	return ret;
}

/* RP5H01 interface */
static const struct RP5H01_interface rp5h01_interface =
{
	1,
	{ "user1" },
	{ 0 }
};

/*************************************
 *
 *  Common init for all games
 *
 *************************************/
DRIVER_INIT( playch10 )
{
	/* initialize the security chip */
	if ( RP5H01_init( &rp5h01_interface ) )
	{
		fatalerror("rp5h01_interface initualization failed");
	}

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

	int reg = ( offset >> 13 );

	/* reset mapper */
	if ( data & 0x80 )
	{
		mmc1_shiftreg = mmc1_shiftcount = 0;

		size16k = 1;
		switchlow = 1;
		vrom4k = 0;

		return;
	}

	/* see if we need to clock in data */
	if ( mmc1_shiftcount < 5 )
	{
		mmc1_shiftreg >>= 1;
		mmc1_shiftreg |= ( data & 1 ) << 4;
		mmc1_shiftcount++;
	}

	/* are we done shifting? */
	if ( mmc1_shiftcount == 5 )
	{
		/* reset count */
		mmc1_shiftcount = 0;

		/* apply data to registers */
		switch( reg )
		{
			case 0:		/* mirroring and options */
				{
					int _mirroring;

					vrom4k = mmc1_shiftreg & 0x10;
					size16k = mmc1_shiftreg & 0x08;
					switchlow = mmc1_shiftreg & 0x04;

					switch( mmc1_shiftreg & 3 )
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
					ppu2c0x_set_mirroring( 0, _mirroring );
				}
			break;

			case 1:	/* video rom banking - bank 0 - 4k or 8k */
				ppu2c0x_set_videorom_bank( 0, 0, ( vrom4k ) ? 4 : 8, ( mmc1_shiftreg & 0x1f ), 256 );
			break;

			case 2: /* video rom banking - bank 1 - 4k only */
				if ( vrom4k )
					ppu2c0x_set_videorom_bank( 0, 4, 4, ( mmc1_shiftreg & 0x1f ), 256 );
			break;

			case 3:	/* program banking */
				{
					int bank = ( mmc1_shiftreg & mmc1_rom_mask ) * 0x4000;
					UINT8 *prg = memory_region( machine, "cart" );

					if ( !size16k )
					{
						/* switch 32k */
						memcpy( &prg[0x08000], &prg[0x010000+bank], 0x8000 );
					}
					else
					{
						/* switch 16k */
						if ( switchlow )
						{
							/* low */
							memcpy( &prg[0x08000], &prg[0x010000+bank], 0x4000 );
						}
						else
						{
							/* high */
							memcpy( &prg[0x0c000], &prg[0x010000+bank], 0x4000 );
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
	ppu2c0x_set_videorom_bank( 0, 0, 8, ( data & 3 ), 512 );
}

DRIVER_INIT( pcaboard )
{
	/* switches vrom with writes to the $803e-$8041 area */
	memory_install_write8_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x8000, 0x8fff, 0, 0, aboard_vrom_switch_w );

	/* common init */
	DRIVER_INIT_CALL(playch10);

	/* set the mirroring here */
	mirroring = PPU_MIRROR_VERT;
}

/**********************************************************************************/

/* B Board games (Contra, Rush N' Attach, Pro Wrestling) */

static WRITE8_HANDLER( bboard_rom_switch_w )
{
	int bankoffset = 0x10000 + ( ( data & 7 ) * 0x4000 );
	UINT8 *prg = memory_region( machine, "cart" );

	memcpy( &prg[0x08000], &prg[bankoffset], 0x4000 );
}

DRIVER_INIT( pcbboard )
{
	UINT8 *prg = memory_region( machine, "cart" );

	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	memcpy( &prg[0x08000], &prg[0x28000], 0x8000 );

	/* Roms are banked at $8000 to $bfff */
	memory_install_write8_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x8000, 0xffff, 0, 0, bboard_rom_switch_w );

	/* common init */
	DRIVER_INIT_CALL(playch10);

	/* set the mirroring here */
	mirroring = PPU_MIRROR_VERT;
}

/**********************************************************************************/

/* C Board games (The Goonies) */

static WRITE8_HANDLER( cboard_vrom_switch_w )
{
	ppu2c0x_set_videorom_bank( 0, 0, 8, ( ( data >> 1 ) & 1 ), 512 );
}

DRIVER_INIT( pccboard )
{
	/* switches vrom with writes to $6000 */
	memory_install_write8_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x6000, 0x6000, 0, 0, cboard_vrom_switch_w );

	/* common init */
	DRIVER_INIT_CALL(playch10);
}

/**********************************************************************************/

/* D Board games (Rad Racer) */

DRIVER_INIT( pcdboard )
{
	UINT8 *prg = memory_region( machine, "cart" );

	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	memcpy( &prg[0x08000], &prg[0x28000], 0x8000 );

	mmc1_rom_mask = 0x07;

	/* MMC mapper at writes to $8000-$ffff */
	memory_install_write8_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x8000, 0xffff, 0, 0, mmc1_rom_switch_w );

	/* common init */
	DRIVER_INIT_CALL(playch10);
}

/* D Board games with extra ram (Metroid) */

DRIVER_INIT( pcdboard_2 )
{
	/* extra ram at $6000-$7fff */
	memory_install_readwrite8_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x6000, 0x7fff, 0, 0, SMH_BANK1, SMH_BANK1 );
	memory_set_bankptr(1, auto_malloc(0x2000));

	/* common init */
	DRIVER_INIT_CALL(pcdboard);
}

/**********************************************************************************/

/* E Board games (Mike Tyson's Punchout) - BROKEN - FIX ME */

/* callback for the ppu_latch */
static void mapper9_latch( offs_t offset )
{
	if( (offset & 0x1ff0) == 0x0fd0 && MMC2_bank_latch[0] != 0xfd )
	{
		MMC2_bank_latch[0] = 0xfd;
		ppu2c0x_set_videorom_bank( 0, 0, 4, MMC2_bank[0], 256 );
	}
	else if( (offset & 0x1ff0) == 0x0fe0 && MMC2_bank_latch[0] != 0xfe )
	{
		MMC2_bank_latch[0] = 0xfe;
		ppu2c0x_set_videorom_bank( 0, 0, 4, MMC2_bank[1], 256 );
	}
	else if( (offset & 0x1ff0) == 0x1fd0 && MMC2_bank_latch[1] != 0xfd )
	{
		MMC2_bank_latch[1] = 0xfd;
		ppu2c0x_set_videorom_bank( 0, 4, 4, MMC2_bank[2], 256 );
	}
	else if( (offset & 0x1ff0) == 0x1fe0 && MMC2_bank_latch[1] != 0xfe )
	{
		MMC2_bank_latch[1] = 0xfe;
		ppu2c0x_set_videorom_bank( 0, 4, 4, MMC2_bank[3], 256 );
	}
}

static WRITE8_HANDLER( eboard_rom_switch_w )
{
	/* a variation of mapper 9 on a nes */
	switch( offset & 0x7000 )
	{
		case 0x2000: /* code bank switching */
			{
				int bankoffset = 0x10000 + ( data & 0x0f ) * 0x2000;
				UINT8 *prg = memory_region( machine, "cart" );
				memcpy( &prg[0x08000], &prg[bankoffset], 0x2000 );
			}
		break;

		case 0x3000: /* gfx bank 0 - 4k */
			MMC2_bank[0] = data;
			if( MMC2_bank_latch[0] == 0xfd )
				ppu2c0x_set_videorom_bank( 0, 0, 4, data, 256 );
		break;

		case 0x4000: /* gfx bank 0 - 4k */
			MMC2_bank[1] = data;
			if( MMC2_bank_latch[0] == 0xfe )
				ppu2c0x_set_videorom_bank( 0, 0, 4, data, 256 );
		break;

		case 0x5000: /* gfx bank 1 - 4k */
			MMC2_bank[2] = data;
			if( MMC2_bank_latch[1] == 0xfd )
				ppu2c0x_set_videorom_bank( 0, 4, 4, data, 256 );
		break;

		case 0x6000: /* gfx bank 1 - 4k */
			MMC2_bank[3] = data;
			if( MMC2_bank_latch[1] == 0xfe )
				ppu2c0x_set_videorom_bank( 0, 4, 4, data, 256 );
		break;

		case 0x7000: /* mirroring */
			ppu2c0x_set_mirroring( 0, data ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT );

		break;
	}
}

DRIVER_INIT( pceboard )
{
	UINT8 *prg = memory_region( machine, "cart" );

	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	memcpy( &prg[0x08000], &prg[0x28000], 0x8000 );

	/* basically a mapper 9 on a nes */
	memory_install_write8_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x8000, 0xffff, 0, 0, eboard_rom_switch_w );

	/* ppu_latch callback */
	ppu_latch = mapper9_latch;

	/* nvram at $6000-$6fff */
	memory_install_readwrite8_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x6000, 0x6fff, 0, 0, SMH_BANK1, SMH_BANK1 );
	memory_set_bankptr(1, auto_malloc(0x1000));

	/* common init */
	DRIVER_INIT_CALL(playch10);
}

/**********************************************************************************/

/* F Board games (Ninja Gaiden, Double Dragon) */

DRIVER_INIT( pcfboard )
{
	UINT8 *prg = memory_region( machine, "cart" );

	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	memcpy( &prg[0x08000], &prg[0x28000], 0x8000 );

	mmc1_rom_mask = 0x07;

	/* MMC mapper at writes to $8000-$ffff */
	memory_install_write8_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x8000, 0xffff, 0, 0, mmc1_rom_switch_w );

	/* common init */
	DRIVER_INIT_CALL(playch10);
}

/* F Board games with extra ram (Baseball Stars) */

DRIVER_INIT( pcfboard_2 )
{
	/* extra ram at $6000-$6fff */
	memory_install_readwrite8_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x6000, 0x6fff, 0, 0, SMH_BANK1, SMH_BANK1 );
	memory_set_bankptr(1, auto_malloc(0x1000));

	/* common init */
	DRIVER_INIT_CALL(pcfboard);
}

/**********************************************************************************/

/* G Board games (Super Mario Bros. 3) */

static int gboard_scanline_counter;
static int gboard_scanline_latch;
static int gboard_banks[2];
static int gboard_4screen;
static int gboard_last_bank = 0xff;
static int gboard_command;

static void gboard_scanline_cb( int num, int scanline, int vblank, int blanked )
{
	if ( !vblank && !blanked )
	{
		if ( --gboard_scanline_counter == -1 )
		{
			gboard_scanline_counter = gboard_scanline_latch;
			cpunum_set_input_line(Machine, 1, 0, PULSE_LINE );
		}
	}
}

static WRITE8_HANDLER( gboard_rom_switch_w )
{
	/* basically, a MMC3 mapper from the nes */

	switch( offset & 0x7001 )
	{
		case 0x0000:
			gboard_command = data;

			if ( gboard_last_bank != ( data & 0xc0 ) )
			{
				int bank;
				UINT8 *prg = memory_region( machine, "cart" );

				/* reset the banks */
				if ( gboard_command & 0x40 )
				{
					/* high bank */
					bank = gboard_banks[0] * 0x2000 + 0x10000;

					memcpy( &prg[0x0c000], &prg[bank], 0x2000 );
					memcpy( &prg[0x08000], &prg[0x4c000], 0x2000 );
				}
				else
				{
					/* low bank */
					bank = gboard_banks[0] * 0x2000 + 0x10000;

					memcpy( &prg[0x08000], &prg[bank], 0x2000 );
					memcpy( &prg[0x0c000], &prg[0x4c000], 0x2000 );
				}

				/* mid bank */
				bank = gboard_banks[1] * 0x2000 + 0x10000;
				memcpy( &prg[0x0a000], &prg[bank], 0x2000 );

				gboard_last_bank = data & 0xc0;
			}
		break;

		case 0x0001:
			{
				UINT8 cmd = gboard_command & 0x07;
				int page = ( gboard_command & 0x80 ) >> 5;
				int bank;

				switch( cmd )
				{
					case 0:	/* char banking */
					case 1: /* char banking */
						data &= 0xfe;
						page ^= ( cmd << 1 );
						ppu2c0x_set_videorom_bank( 0, page, 2, data, 64 );
					break;

					case 2: /* char banking */
					case 3: /* char banking */
					case 4: /* char banking */
					case 5: /* char banking */
						page ^= cmd + 2;
						ppu2c0x_set_videorom_bank( 0, page, 1, data, 64 );
					break;

					case 6: /* program banking */
					{
						UINT8 *prg = memory_region( machine, "cart" );
						if ( gboard_command & 0x40 )
						{
							/* high bank */
							gboard_banks[0] = data & 0x1f;
							bank = ( gboard_banks[0] ) * 0x2000 + 0x10000;

							memcpy( &prg[0x0c000], &prg[bank], 0x2000 );
							memcpy( &prg[0x08000], &prg[0x4c000], 0x2000 );
						}
						else
						{
							/* low bank */
							gboard_banks[0] = data & 0x1f;
							bank = ( gboard_banks[0] ) * 0x2000 + 0x10000;

							memcpy( &prg[0x08000], &prg[bank], 0x2000 );
							memcpy( &prg[0x0c000], &prg[0x4c000], 0x2000 );
						}
					}
					break;

					case 7: /* program banking */
						{
							/* mid bank */
							UINT8 *prg = memory_region( machine, "cart" );
							gboard_banks[1] = data & 0x1f;
							bank = gboard_banks[1] * 0x2000 + 0x10000;

							memcpy( &prg[0x0a000], &prg[bank], 0x2000 );
						}
					break;
				}
			}
		break;

		case 0x2000: /* mirroring */
			if( !gboard_4screen )
			{
				if ( data & 0x40 )
					ppu2c0x_set_mirroring( 0, PPU_MIRROR_HIGH );
				else
					ppu2c0x_set_mirroring( 0, ( data & 1 ) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT );
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
			ppu2c0x_set_scanline_callback( 0, 0 );
		break;

		case 0x6001: /* enable irqs */
			ppu2c0x_set_scanline_callback( 0, gboard_scanline_cb );
		break;
	}
}

DRIVER_INIT( pcgboard )
{
	UINT8 *prg = memory_region( machine, "cart" );

	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	memcpy( &prg[0x08000], &prg[0x4c000], 0x4000 );
	memcpy( &prg[0x0c000], &prg[0x4c000], 0x4000 );

	/* MMC3 mapper at writes to $8000-$ffff */
	memory_install_write8_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x8000, 0xffff, 0, 0, gboard_rom_switch_w );

	/* extra ram at $6000-$7fff */
	memory_install_readwrite8_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x6000, 0x7fff, 0, 0, SMH_BANK1, SMH_BANK1 );
	memory_set_bankptr(1, auto_malloc(0x2000));

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
	UINT8 *prg = memory_region( machine, "cart" );

	if ( data & 0x10 )
		ppu2c0x_set_mirroring( 0, PPU_MIRROR_HIGH );
	else
		ppu2c0x_set_mirroring( 0, PPU_MIRROR_LOW );

	memcpy( &prg[0x08000], &prg[bank * 0x8000 + 0x10000], 0x8000 );
}

DRIVER_INIT( pciboard )
{
	UINT8 *prg = memory_region( machine, "cart" );

	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	memcpy( &prg[0x08000], &prg[0x10000], 0x8000 );

	/* Roms are banked at $8000 to $bfff */
	memory_install_write8_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x8000, 0xffff, 0, 0, iboard_rom_switch_w );

	/* common init */
	DRIVER_INIT_CALL(playch10);
}

/**********************************************************************************/

/* H Board games (PinBot) */

static WRITE8_HANDLER( hboard_rom_switch_w )
{
	switch( offset & 0x7001 )
	{
		case 0x0001:
			{
				UINT8 cmd = gboard_command & 0x07;
				int page = ( gboard_command & 0x80 ) >> 5;

				switch( cmd )
				{
					case 0:	/* char banking */
					case 1: /* char banking */
						data &= 0xfe;
						page ^= ( cmd << 1 );
						if ( data & 0x20 )
						{
							ppu2c0x_set_videoram_bank( 0, page, 2, data, 64 );
						}
						else
						{
							ppu2c0x_set_videorom_bank( 0, page, 2, data, 64 );
						}
					return;

					case 2: /* char banking */
					case 3: /* char banking */
					case 4: /* char banking */
					case 5: /* char banking */
						page ^= cmd + 2;
						if ( data & 0x40 )
						{
							ppu2c0x_set_videoram_bank( 0, page, 1, data, 64 );
						}
						else
						{
							ppu2c0x_set_videorom_bank( 0, page, 1, data, 64 );
						}
					return;
				}
			}
	};
	gboard_rom_switch_w(machine,offset,data);
};


DRIVER_INIT( pchboard )
{
	UINT8 *prg = memory_region( machine, "cart" );
	memcpy( &prg[0x08000], &prg[0x4c000], 0x4000 );
	memcpy( &prg[0x0c000], &prg[0x4c000], 0x4000 );

	/* Roms are banked at $8000 to $bfff */
	memory_install_write8_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x8000, 0xffff, 0, 0, hboard_rom_switch_w );

	/* extra ram at $6000-$7fff */
	memory_install_readwrite8_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x6000, 0x7fff, 0, 0, SMH_BANK1, SMH_BANK1 );
	memory_set_bankptr(1, auto_malloc(0x2000));

	gboard_banks[0] = 0x1e;
	gboard_banks[1] = 0x1f;
	gboard_scanline_counter = 0;
	gboard_scanline_latch = 0;

	/* common init */
	DRIVER_INIT_CALL(playch10);
}

/**********************************************************************************/

/* K Board games (Mario Open Golf) */

DRIVER_INIT( pckboard )
{
	UINT8 *prg = memory_region( machine, "cart" );

	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	memcpy( &prg[0x08000], &prg[0x48000], 0x8000 );

	mmc1_rom_mask = 0x0f;

	/* extra ram at $6000-$7fff */
	memory_install_readwrite8_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x6000, 0x7fff, 0, 0, SMH_BANK1, SMH_BANK1 );
	memory_set_bankptr(1, auto_malloc(0x2000));

	/* Roms are banked at $8000 to $bfff */
	memory_install_write8_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x8000, 0xffff, 0, 0, mmc1_rom_switch_w );

	/* common init */
	DRIVER_INIT_CALL(playch10);
}
