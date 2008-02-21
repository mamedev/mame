/***************************************************************************

Nintendo VS UniSystem and DualSystem - (c) 1984 Nintendo of America

    Portions of this code are heavily based on
    Brad Oliver's MESS implementation of the NES.

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "video/ppu2c0x.h"

/* Globals */
static int vsnes_gun_controller;
static int vsnes_do_vrom_bank;

/* Locals */
static int input_latch[4];
static const UINT8 *remapped_colortable;

static int sound_fix=0;
/*************************************
 *
 *  Color Mapping
 *
 *************************************/

/* RP2C04-001 */
/* check 0x08 */
static const UINT8 rp2c04001_colortable[] =
{
	0x35, 0xff, 0x16, 0x22, 0x1c, 0xff, 0xff, 0x15, /* 0x00 - 0x07 */
	0x20, 0x00, 0x27, 0x05, 0x04, 0x27, 0x08, 0x30, /* 0x08 - 0x0f */
	0x21, 0xff, 0xff, 0x29, 0x3c, 0xff, 0x36, 0x12, /* 0x10 - 0x17 */
	0xff, 0x2b, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, /* 0x18 - 0x1f */
	0xff, 0x31, 0xff, 0x2a, 0x2c, 0x0c, 0xff, 0xff, /* 0x20 - 0x27 */
	0xff, 0x07, 0x34, 0x06, 0x13, 0xff, 0x26, 0x0f, /* 0x28 - 0x2f */
	0xff, 0x19, 0x10, 0x0a, 0xff, 0xff, 0xff, 0x17, /* 0x30 - 0x37 */
	0xff, 0x11, 0x09, 0xff, 0xff, 0x25, 0x18, 0xff  /* 0x38 - 0x3f */
};

/* RP2C04-002 */
/* 0x04, 0x24 */
static const UINT8 rp2c04002_colortable[] =
{
	0xff, 0x27, 0x18, 0xff, 0x3a, 0x25, 0x2b, 0x31, /* 0x00 - 0x07 */
	0x15, 0x13, 0x38, 0x34, 0x20, 0x23, 0xff, 0x0b, /* 0x08 - 0x0f */
	0xff, 0x21, 0x06, 0xff, 0x1b, 0x29, 0xff, 0x22, /* 0x10 - 0x17 */
	0xff, 0x24, 0xff, 0xff, 0xff, 0x08, 0xff, 0x03, /* 0x18 - 0x1f */
	0xff, 0x36, 0x26, 0x33, 0x11, 0xff, 0x10, 0x02, /* 0x20 - 0x27 */
	0x14, 0xff, 0x10, 0x09, 0x12, 0x0f, 0xff, 0x30, /* 0x28 - 0x2f */
	0xff, 0xff, 0x2a, 0x17, 0x0c, 0x11, 0x15, 0x19, /* 0x30 - 0x37 */
	0xff, 0x2c, 0x07, 0x37, 0xff, 0x05, 0x3a, 0xff  /* 0x38 - 0x3f */
};

/* RP2C04-003 */
/* check 0x0f, 0x2e, 0x34 */
static const UINT8 rp2c04003_colortable[] =
{
	0xff, 0xff, 0xff, 0x10, 0x1a, 0x30, 0x31, 0x09, /* 0x00 - 0x07 */
	0x01, 0x0f, 0x36, 0x08, 0x15, 0xff, 0xff, 0x30, /* 0x08 - 0x0f */
	0x22, 0x1c, 0xff, 0x12, 0x19, 0x18, 0x17, 0x2a, /* 0x10 - 0x17 */
	0x00, 0xff, 0xff, 0x02, 0x06, 0x07, 0xff, 0x35, /* 0x18 - 0x1f */
	0x23, 0xff, 0x8b, 0xf7, 0xff, 0x27, 0x26, 0x20, /* 0x20 - 0x27 */
	0x29, 0x03, 0x21, 0x24, 0x11, 0xff, 0xff, 0xff, /* 0x28 - 0x2f */
	0x2c, 0xff, 0xff, 0xff, 0x07, 0xf9, 0x28, 0xff, /* 0x30 - 0x37 */
	0x0a, 0xff, 0x32, 0x37, 0x13, 0x3a, 0xff, 0x0b  /* 0x38 - 0x3f */
};

/* RP2C05-004 */
/* check 0x03 0x1d, 0x38, 0x3b*/
static const UINT8 rp2c05004_colortable[] =
{
	0x18, 0xff, 0x1c, 0x89, 0xff, 0xff, 0x01, 0x17, /* 0x00 - 0x07 */
	0x10, 0x0f, 0x2a, 0xff, 0x36, 0x37, 0x1a, 0xff, /* 0x08 - 0x0f */
	0x25, 0xff, 0x12, 0xff, 0x0f, 0xff, 0xff, 0x26, /* 0x10 - 0x17 */
	0xff, 0xff, 0x22, 0xff, 0xff, 0x0f, 0x3a, 0x21, /* 0x18 - 0x1f */
	0x05, 0x0a, 0x07, 0xc2, 0x13, 0xff, 0x00, 0x15, /* 0x20 - 0x27 */
	0x0c, 0xff, 0x11, 0xff, 0xff, 0x38, 0xff, 0xff, /* 0x28 - 0x2f */
	0xff, 0xff, 0x08, 0x45, 0xff, 0xff, 0x30, 0x3c, /* 0x30 - 0x37 */
	0x0f, 0x27, 0xff, 0x60, 0x29, 0xff, 0x30, 0x09  /* 0x38 - 0x3f */
};



/* remap callback */
static int remap_colors( int num, int addr, int data )
{
	/* this is the protection. color codes are shuffled around */
	/* the ones with value 0xff are unknown */

	if ( addr >= 0x3f00 )
	{
		int newdata = remapped_colortable[ data & 0x3f ];

		if ( newdata != 0xff )
			data = newdata;

		#ifdef MAME_DEBUG
		else
			popmessage( "Unmatched color %02x, at address %04x", data & 0x3f, addr );
		#endif
	}

	return data;
}

/*************************************
 *
 *  Input Ports
 *
 *************************************/
WRITE8_HANDLER( vsnes_in0_w )
{
	/* Toggling bit 0 high then low resets both controllers */
	if ( data & 1 )
	{
		/* load up the latches */
		input_latch[0] = readinputport( 0 );
		input_latch[1] = readinputport( 1 );
	}
}

static READ8_HANDLER( gun_in0_r )
{
	int ret = ( input_latch[0] ) & 1;

	/* shift */
	input_latch[0] >>= 1;

	ret |= readinputport( 2 ); 				/* merge coins, etc */
	ret |= ( readinputport( 3 ) & 3 ) << 3; /* merge 2 dipswitches */

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

	int ret = ( input_latch[0] ) & 1;

	/* shift */
	input_latch[0] >>= 1;

	ret |= readinputport( 2 ); 				/* merge coins, etc */
	ret |= ( readinputport( 3 ) & 3 ) << 3; /* merge 2 dipswitches */

	return ret;

}


READ8_HANDLER( vsnes_in1_r )
{
	int ret = ( input_latch[1] ) & 1;

	ret |= readinputport( 3 ) & ~3;			/* merge the rest of the dipswitches */

	/* shift */
	input_latch[1] >>= 1;

	return ret;
}

WRITE8_HANDLER( vsnes_in0_1_w )
{
	/* Toggling bit 0 high then low resets both controllers */
	if ( data & 1 )
	{
		/* load up the latches */
		input_latch[2] = readinputport( 4 );
		input_latch[3] = readinputport( 5 );
	}
}

READ8_HANDLER( vsnes_in0_1_r )
{
	int ret = ( input_latch[2] ) & 1;

	/* shift */
	input_latch[2] >>= 1;

	ret |= readinputport( 6 ); 				/* merge coins, etc */
	ret |= ( readinputport( 7 ) & 3 ) << 3; /* merge 2 dipswitches */
	return ret;
}

READ8_HANDLER( vsnes_in1_1_r )
{
	int ret = ( input_latch[3] ) & 1;

	ret |= readinputport( 7 ) & ~3;			/* merge the rest of the dipswitches */

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
	input_latch[0] = input_latch[1] = 0;
	input_latch[2] = input_latch[3] = 0;

	/* reset the ppu */
	ppu2c0x_reset( 0, 1 );

	/* if we need to remap, install the callback */
	if ( remapped_colortable )
		ppu2c0x_set_vidaccess_callback( 0, remap_colors );
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

	/* reset the ppu */
	ppu2c0x_reset( 0,1);
	ppu2c0x_reset( 1,1 );

	/* if we need to remap, install the callback */
	if ( remapped_colortable )
	{
		ppu2c0x_set_vidaccess_callback( 0, remap_colors );
		ppu2c0x_set_vidaccess_callback( 1, remap_colors );
	}
}

/*************************************
 *
 *  Common init for all games
 *
 *************************************/
static void init_vsnes(running_machine *machine)
{
	/* set the controller to default */
	vsnes_gun_controller = 0;

	/* no color remapping */
	remapped_colortable = 0;
}

/**********************************************************************************
 *
 *  Game and Board-specific initialization
 *
 **********************************************************************************/

static WRITE8_HANDLER( vsnormal_vrom_banking )
{
	/* switch vrom */
	ppu2c0x_set_videorom_bank( 0, 0, 8, ( data & 4 ) ? 1 : 0, 512 );

	/* bit 1 ( data & 2 ) enables writes to extra ram, we ignore it */

	/* move along */
	vsnes_in0_w( offset, data );
}

/* Most games switch VROM Banks in controller 0 write */
/* they dont do any other trickery */
DRIVER_INIT( vsnormal )
{
	/* vrom switching is enabled with bit 2 of $4016 */
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x4016, 0x4016, 0, 0, vsnormal_vrom_banking );
}

static WRITE8_HANDLER( ppuRC2C05_protection )
{
	/* This PPU has registers mapped at $2000 and $2001 inverted */
	/* and no remapped color */

	if ( offset == 0 )
	{
		ppu2c0x_0_w( 1, data );
		return;
	}

	ppu2c0x_0_w( 0, data );
}

/**********************************************************************************/

/* Super Mario Bros. Extra ram at $6000 (NV?) and remapped colors */

DRIVER_INIT( suprmrio )
{
	/* common init */
	init_vsnes(machine);

	/* normal banking */
	DRIVER_INIT_CALL(vsnormal);

	/* extra ram at $6000 is enabled with bit 1 of $4016 */
	memory_install_readwrite8_handler(0, ADDRESS_SPACE_PROGRAM, 0x6000, 0x7fff, 0, 0, MRA8_BANK1, MWA8_BANK1 );
	memory_set_bankptr(1, auto_malloc(0x2000));

	/* now override the vidaccess callback */
	/* we need to remap color tables */
	/* this *is* the VS games protection, I guess */
	remapped_colortable = rp2c05004_colortable;
}

/**********************************************************************************/

/* Gun Games - VROM Banking in controller 0 write */

static WRITE8_HANDLER( gun_in0_w )
{
	static int zapstore;

	if (vsnes_do_vrom_bank)
	{
		/* switch vrom */
		ppu2c0x_set_videorom_bank( 0, 0, 8, ( data & 4 ) ? 1 : 0, 512 );
	}

	/* here we do things a little different */
	if ( data & 1 )
	{

		/* load up the latches */
		input_latch[0] = readinputport( 0 );

		/* do the gun thing */
		if ( vsnes_gun_controller )
		{
			int x = readinputport( 4 );
			int y = readinputport( 5 );
			UINT32 pix, color_base;

			/* get the pixel at the gun position */
			pix = ppu2c0x_get_pixel( 0, x, y );

			/* get the color base from the ppu */
			color_base = ppu2c0x_get_colorbase( 0 );

			/* look at the screen and see if the cursor is over a bright pixel */
			if ( ( pix == color_base+0x20 ) || ( pix == color_base+0x30 ) ||
				 ( pix == color_base+0x33 ) || ( pix == color_base+0x34 ) )
			{
				input_latch[0] |= 0x40;
			}
		}

		input_latch[1] = readinputport( 1 );
	}

    if ( ( zapstore & 1 ) && ( !( data & 1 ) ) )
	/* reset sound_fix to keep sound from hanging */
    {
		sound_fix = 0;
	}

    zapstore = data;
}

DRIVER_INIT( duckhunt )
{
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x4016, 0x4016, 0, 0, gun_in0_r);
	/* vrom switching is enabled with bit 2 of $4016 */
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x4016, 0x4016, 0, 0, gun_in0_w );

	/* common init */
	init_vsnes(machine);

	/* enable gun controller */
	vsnes_gun_controller = 1;
	vsnes_do_vrom_bank = 1;
}

/**********************************************************************************/

/* The Goonies, VS Gradius: ROMs bankings at $8000-$ffff */

static WRITE8_HANDLER( goonies_rom_banking )
{
	int reg = ( offset >> 12 ) & 0x07;
	int bankoffset = ( data & 7 ) * 0x2000 + 0x10000;

	switch( reg )
	{
		case 0: /* code bank 0 */
			memcpy( &memory_region( REGION_CPU1 )[0x08000], &memory_region( REGION_CPU1 )[bankoffset], 0x2000 );
		break;

		case 2: /* code bank 1 */
			memcpy( &memory_region( REGION_CPU1 )[0x0a000], &memory_region( REGION_CPU1 )[bankoffset], 0x2000 );
		break;

		case 4: /* code bank 2 */
			memcpy( &memory_region( REGION_CPU1 )[0x0c000], &memory_region( REGION_CPU1 )[bankoffset], 0x2000 );
		break;

		case 6: /* vrom bank 0 */
			ppu2c0x_set_videorom_bank( 0, 0, 4, data, 256 );
		break;

		case 7: /* vrom bank 1 */
			ppu2c0x_set_videorom_bank( 0, 4, 4, data, 256 );
		break;
	}
}

DRIVER_INIT( goonies )
{
	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	memcpy( &memory_region( REGION_CPU1 )[0x08000], &memory_region( REGION_CPU1 )[0x18000], 0x8000 );

	/* banking is done with writes to the $8000-$ffff area */
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x8000, 0xffff, 0, 0, goonies_rom_banking );

	/* common init */
	init_vsnes(machine);

	/* now override the vidaccess callback */
	remapped_colortable = rp2c04003_colortable;
}

DRIVER_INIT( vsgradus )
{
	/* We do manual banking, in case the code falls through */
	/* Copy the initial banks */
	memcpy( &memory_region( REGION_CPU1 )[0x08000], &memory_region( REGION_CPU1 )[0x18000], 0x8000 );

	/* banking is done with writes to the $8000-$ffff area */
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x8000, 0xffff, 0, 0, goonies_rom_banking );

	/* common init */
	init_vsnes(machine);

	/* now override the vidaccess callback */
	remapped_colortable = rp2c04001_colortable;
}

DRIVER_INIT( vspinbal )
{
	/* common init */
	init_vsnes(machine);

	/* normal banking */
	DRIVER_INIT_CALL(vsnormal);

	/* now override the vidaccess callback */
	remapped_colortable = rp2c04001_colortable;

}

DRIVER_INIT( hogalley )
{

	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x4016, 0x4016, 0, 0, gun_in0_r);
	/* vrom switching is enabled with bit 2 of $4016 */
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x4016, 0x4016, 0, 0, gun_in0_w );

	/* common init */
	init_vsnes(machine);

	/* enable gun controller */
	vsnes_gun_controller = 1;
	vsnes_do_vrom_bank = 1;

	/* now override the vidaccess callback */
	remapped_colortable = rp2c04001_colortable;
}

/***********************************************************************/

/* Vs. Gumshoe */

static READ8_HANDLER( vsgshoe_security_r )
{
	/* low part must be 0x1c */
	return ppu2c0x_0_r( 2 ) | 0x1c;
}

static WRITE8_HANDLER( vsgshoe_gun_in0_w )
{
	static int old_bank = 0;
	int addr;
	if((data & 0x04) != old_bank)
	{
		old_bank = data & 0x04;
		addr = old_bank ? 0x12000: 0x10000;
		memcpy (&memory_region( REGION_CPU1 )[0x08000], &memory_region( REGION_CPU1 )[addr], 0x2000);
	}

	gun_in0_w(offset, data);
}

DRIVER_INIT( vsgshoe )
{
	/* set up the default bank */
	memcpy (&memory_region( REGION_CPU1 )[0x08000], &memory_region( REGION_CPU1 )[0x12000], 0x2000);

	/* Protection */
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x2002, 0x2002, 0, 0, vsgshoe_security_r );
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x2000, 0x2001, 0, 0, ppuRC2C05_protection );

	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x4016, 0x4016, 0, 0, gun_in0_r);
	/* vrom switching is enabled with bit 2 of $4016 */
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x4016, 0x4016, 0, 0, vsgshoe_gun_in0_w );

	/* common init */
	init_vsnes(machine);

	vsnes_gun_controller = 1;
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

	int reg = ( offset >> 13 );

	/* reset mapper */
	if ( data & 0x80 )
	{
		drmario_shiftreg = drmario_shiftcount = 0;

		size16k = 1;

		switchlow = 1;
		vrom4k = 0;

		return;
	}

	/* see if we need to clock in data */
	if ( drmario_shiftcount < 5 )
	{
		drmario_shiftreg >>= 1;
		drmario_shiftreg |= ( data & 1 ) << 4;
		drmario_shiftcount++;
	}

	/* are we done shifting? */
	if ( drmario_shiftcount == 5 )
	{
		/* reset count */
		drmario_shiftcount = 0;

		/* apply data to registers */
		switch( reg )
		{
			case 0:		/* mirroring and options */
				{
					int mirroring;

					vrom4k = drmario_shiftreg & 0x10;
					size16k = drmario_shiftreg & 0x08;
					switchlow = drmario_shiftreg & 0x04;

					switch( drmario_shiftreg & 3 )
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
					ppu2c0x_set_mirroring( 0, mirroring );
				}
			break;

			case 1:	/* video rom banking - bank 0 - 4k or 8k */
				ppu2c0x_set_videorom_bank( 0, 0, ( vrom4k ) ? 4 : 8, drmario_shiftreg, ( vrom4k ) ? 256 : 512 );
			break;

			case 2: /* video rom banking - bank 1 - 4k only */
				if ( vrom4k )
					ppu2c0x_set_videorom_bank( 0, 4, 4, drmario_shiftreg, 256 );
			break;

			case 3:	/* program banking */
				{
					int bank = ( drmario_shiftreg & 0x03 ) * 0x4000;

					if ( !size16k )
					{
						/* switch 32k */
						memcpy( &memory_region( REGION_CPU1 )[0x08000], &memory_region( REGION_CPU1 )[0x010000+bank], 0x8000 );
					}
					else
					{
						/* switch 16k */
						if ( switchlow )
						{
							/* low */
							memcpy( &memory_region( REGION_CPU1 )[0x08000], &memory_region( REGION_CPU1 )[0x010000+bank], 0x4000 );
						}
						else
						{
							/* high */
							memcpy( &memory_region( REGION_CPU1 )[0x0c000], &memory_region( REGION_CPU1 )[0x010000+bank], 0x4000 );
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
	memcpy( &memory_region( REGION_CPU1 )[0x08000], &memory_region( REGION_CPU1 )[0x10000], 0x4000 );
	memcpy( &memory_region( REGION_CPU1 )[0x0c000], &memory_region( REGION_CPU1 )[0x1c000], 0x4000 );

	/* MMC1 mapper at writes to $8000-$ffff */
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x8000, 0xffff, 0, 0, drmario_rom_banking );

	drmario_shiftreg = 0;
	drmario_shiftcount = 0;

	/* common init */
	init_vsnes(machine);

	/* now override the vidaccess callback */
	remapped_colortable = rp2c04003_colortable;
}

/***********************************************************************/

/* Excite Bike */

DRIVER_INIT( excitebk )
{
	/* common init */
	init_vsnes(machine);

	/* normal banking */
	DRIVER_INIT_CALL(vsnormal);

	/* now override the vidaccess callback */
	/* we need to remap color tables */
	/* this *is* the VS games protection, I guess */
	remapped_colortable = rp2c04003_colortable;
}

DRIVER_INIT( excitbkj )
{
	/* common init */
	init_vsnes(machine);

	/* normal banking */
	DRIVER_INIT_CALL(vsnormal);

	/* now override the vidaccess callback */
	/* we need to remap color tables */
	/* this *is* the VS games protection, I guess */
	remapped_colortable = rp2c05004_colortable;
}

/**********************************************************************************/

/* Mach Rider */

DRIVER_INIT( machridr )
{

	/* common init */
	init_vsnes(machine);

	/* normal banking */
	DRIVER_INIT_CALL(vsnormal);

	/* now override the vidaccess callback */
	/* we need to remap color tables */
	/* this *is* the VS games protection, I guess */
	remapped_colortable = rp2c04002_colortable;
}

/**********************************************************************************/

/* VS Slalom */

DRIVER_INIT( vsslalom )
{
	/* common init */
	init_vsnes(machine);

	/* now override the vidaccess callback */
	/* we need to remap color tables */
	/* this *is* the VS games protection, I guess */
	remapped_colortable = rp2c04002_colortable;
}

/**********************************************************************************/

/* Castelvania: ROMs bankings at $8000-$ffff */

static WRITE8_HANDLER( castlevania_rom_banking )
{
	int rombank = 0x10000 + ( data & 7 ) * 0x4000;

	memcpy( &memory_region( REGION_CPU1 )[0x08000], &memory_region( REGION_CPU1 )[rombank], 0x4000 );
}

DRIVER_INIT( cstlevna )
{
	/* when starting the game, the 1st 16k and the last 16k are loaded into the 2 banks */
	memcpy( &memory_region( REGION_CPU1 )[0x08000], &memory_region( REGION_CPU1 )[0x28000], 0x8000 );

   	/* banking is done with writes to the $8000-$ffff area */
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x8000, 0xffff, 0, 0, castlevania_rom_banking );

	/* common init */
	init_vsnes(machine);

	/* now override the vidaccess callback */
	/* we need to remap color tables */
	/* this *is* the VS games protection, I guess */
	remapped_colortable = rp2c04002_colortable;
}

/**********************************************************************************/

/* VS Top Gun: ROMs bankings at $8000-$ffff, plus some protection */

static READ8_HANDLER( topgun_security_r )
{
	/* low part must be 0x1b */
	return ppu2c0x_0_r( 2 ) | 0x1b;
}

DRIVER_INIT( topgun )
{
	/* when starting the game, the 1st 16k and the last 16k are loaded into the 2 banks */
	memcpy( &memory_region( REGION_CPU1 )[0x08000], &memory_region( REGION_CPU1 )[0x28000], 0x8000 );

   	/* banking is done with writes to the $8000-$ffff area */
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x8000, 0xffff, 0, 0, castlevania_rom_banking );

	/* tap on the PPU, due to some tricky protection */
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x2002, 0x2002, 0, 0, topgun_security_r );
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x2000, 0x2001, 0, 0, ppuRC2C05_protection );

	/* common init */
	init_vsnes(machine);
}

/**********************************************************************************/

static int MMC3_cmd, MMC3_prg0, MMC3_prg1;
static int MMC3_chr[6];
static int MMC3_prg_chunks, MMC3_prg_mask;
static int IRQ_enable, IRQ_count, IRQ_count_latch;

static void mapper4_set_prg (void)
{
	MMC3_prg0 &= MMC3_prg_mask;
	MMC3_prg1 &= MMC3_prg_mask;

	if (MMC3_cmd & 0x40)
	{
		memcpy( &memory_region( REGION_CPU1 )[0x8000], &memory_region( REGION_CPU1 )[(MMC3_prg_chunks-1) * 0x4000 + 0x10000], 0x2000 );
		memcpy( &memory_region( REGION_CPU1 )[0xc000], &memory_region( REGION_CPU1 )[0x2000 * (MMC3_prg0) + 0x10000], 0x2000 );
	}
	else
	{
		memcpy( &memory_region( REGION_CPU1 )[0x8000], &memory_region( REGION_CPU1 )[0x2000 * (MMC3_prg0) + 0x10000], 0x2000 );
		memcpy( &memory_region( REGION_CPU1 )[0xc000], &memory_region( REGION_CPU1 )[(MMC3_prg_chunks-1) * 0x4000 + 0x10000], 0x2000 );
	}
	memcpy( &memory_region( REGION_CPU1 )[0xa000], &memory_region( REGION_CPU1 )[0x2000 * (MMC3_prg1) + 0x10000], 0x2000 );
}

static void mapper4_set_chr (void)
{
	UINT8 chr_page = (MMC3_cmd & 0x80) >> 5;
	ppu2c0x_set_videorom_bank(0, chr_page ^ 0, 2, MMC3_chr[0], 1);
	ppu2c0x_set_videorom_bank(0, chr_page ^ 2, 2, MMC3_chr[1], 1);
	ppu2c0x_set_videorom_bank(0, chr_page ^ 4, 1, MMC3_chr[2], 1);
	ppu2c0x_set_videorom_bank(0, chr_page ^ 5, 1, MMC3_chr[3], 1);
	ppu2c0x_set_videorom_bank(0, chr_page ^ 6, 1, MMC3_chr[4], 1);
	ppu2c0x_set_videorom_bank(0, chr_page ^ 7, 1, MMC3_chr[5], 1);
}

#define BOTTOM_VISIBLE_SCANLINE	239		/* The bottommost visible scanline */
#define NUM_SCANLINE 262

static void mapper4_irq ( int num, int scanline, int vblank, int blanked )
{
	mame_printf_debug("entra\n");
	if ((scanline < BOTTOM_VISIBLE_SCANLINE) || (scanline == NUM_SCANLINE-1))
	{
		if ((IRQ_enable) && !blanked)
		{
			if (IRQ_count == 0)
			{
				IRQ_count = IRQ_count_latch;
				cpunum_set_input_line (Machine, 0, 0, HOLD_LINE);
			}
			IRQ_count --;
		}
	}
}

static WRITE8_HANDLER( mapper4_w )
{
	static UINT8 last_bank = 0xff;

	switch (offset & 0x7001)
	{
		case 0x0000: /* $8000 */
			MMC3_cmd = data;

			/* Toggle between switching $8000 and $c000 */
			if (last_bank != (data & 0xc0))
			{
				/* Reset the banks */
				mapper4_set_prg ();
				mapper4_set_chr ();

			}
			last_bank = data & 0xc0;
			break;

		case 0x0001: /* $8001 */
		{
			UINT8 cmd = MMC3_cmd & 0x07;
			switch (cmd)
			{
				case 0: case 1:
					data &= 0xfe;
					MMC3_chr[cmd] = data * 64;
					mapper4_set_chr ();

					break;

				case 2: case 3: case 4: case 5:
					MMC3_chr[cmd] = data * 64;
					mapper4_set_chr ();

					break;

				case 6:
					MMC3_prg0 = data;
					mapper4_set_prg ();
					break;

				case 7:
					MMC3_prg1 = data;
					mapper4_set_prg ();
					break;
			}
			break;
		}
		case 0x2000: /* $a000 */
			if (data & 0x40)
				ppu2c0x_set_mirroring(0, PPU_MIRROR_HIGH);
			else
			{
				if (data & 0x01)
					ppu2c0x_set_mirroring(0, PPU_MIRROR_HORZ);
				else
					ppu2c0x_set_mirroring(0, PPU_MIRROR_VERT);
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

			ppu2c0x_set_scanline_callback (0, 0);

			break;

		case 0x6001: /* $e001 - Enable IRQs */
			IRQ_enable = 1;
			ppu2c0x_set_scanline_callback (0, mapper4_irq);

			break;

		default:
			logerror("mapper4_w uncaught: %04x value: %02x\n", offset + 0x8000, data);
			break;
	}
}

/* Common init for MMC3 games */

DRIVER_INIT( MMC3 )
{
	IRQ_enable = IRQ_count = IRQ_count_latch = 0;
	MMC3_prg0 = 0xfe;
	MMC3_prg1 = 0xff;
	MMC3_cmd = 0;

	MMC3_prg_chunks = (memory_region_length(REGION_CPU1) - 0x10000) / 0x4000;

	MMC3_prg_mask = ((MMC3_prg_chunks << 1) - 1);

	memcpy( &memory_region( REGION_CPU1 )[0x8000], &memory_region( REGION_CPU1 )[(MMC3_prg_chunks-1) * 0x4000 + 0x10000], 0x2000 );
	memcpy( &memory_region( REGION_CPU1 )[0xa000], &memory_region( REGION_CPU1 )[(MMC3_prg_chunks-1) * 0x4000 + 0x12000], 0x2000 );
	memcpy( &memory_region( REGION_CPU1 )[0xc000], &memory_region( REGION_CPU1 )[(MMC3_prg_chunks-1) * 0x4000 + 0x10000], 0x2000 );
	memcpy( &memory_region( REGION_CPU1 )[0xe000], &memory_region( REGION_CPU1 )[(MMC3_prg_chunks-1) * 0x4000 + 0x12000], 0x2000 );

	/* MMC3 mapper at writes to $8000-$ffff */
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x8000, 0xffff, 0, 0, mapper4_w );

	/* extra ram at $6000-$7fff */
	memory_install_readwrite8_handler(0, ADDRESS_SPACE_PROGRAM, 0x6000, 0x7fff, 0, 0, MRA8_BANK1, MWA8_BANK1 );
	memory_set_bankptr(1, auto_malloc(0x2000));

	/* common init */
	init_vsnes(machine);
}

/* Vs. RBI Baseball */

static READ8_HANDLER( rbi_hack_r)
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
			break;

			case 14:
				return 0x94;
			break;

   			default:
    			return 0xB4;
			break;
		}
	}
}

DRIVER_INIT( rbibb )
{
	DRIVER_INIT_CALL(MMC3);

	/* RBI Base ball hack */
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x5e00, 0x5e01, 0, 0, rbi_hack_r) ;

	remapped_colortable = rp2c04003_colortable;
}

/* Vs. Super Xevious */

static int supxevs_prot_index = 0;

static READ8_HANDLER( supxevs_read_prot_1_r )
{
	return 0x05;
}

static READ8_HANDLER( supxevs_read_prot_2_r )
{
	if( supxevs_prot_index )
		return 0;
	else
		return 0x01;
}

static READ8_HANDLER( supxevs_read_prot_3_r )
{
	if( supxevs_prot_index )
		return 0xd1;
	else
		return 0x89;
}

static READ8_HANDLER( supxevs_read_prot_4_r )
{
	if( supxevs_prot_index )
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
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x54ff, 0x54ff, 0, 0, supxevs_read_prot_1_r );
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x5678, 0x5678, 0, 0, supxevs_read_prot_2_r );
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x578f, 0x578f, 0, 0, supxevs_read_prot_3_r );
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x5567, 0x5567, 0, 0, supxevs_read_prot_4_r );

	remapped_colortable = rp2c04001_colortable;
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

	if ( offset == 0 )
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
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x5e00, 0x5e01, 0, 0, tko_security_r );

	/* now override the vidaccess callback */
	/* we need to remap color tables */
	/* this *is* the VS games protection, I guess */
	remapped_colortable = rp2c04003_colortable;
}

/* Vs. Freedom Force */

DRIVER_INIT( vsfdf )
{
	DRIVER_INIT_CALL(MMC3);

	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x4016, 0x4016, 0, 0, gun_in0_r );
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x4016, 0x4016, 0, 0, gun_in0_w );

	vsnes_gun_controller = 1;
	vsnes_do_vrom_bank = 0;

	remapped_colortable = rp2c04001_colortable;
}

/**********************************************************************************/
/* Platoon rom banking */

static WRITE8_HANDLER( mapper68_rom_banking ){

	switch (offset & 0x7000)
	{
		case 0x0000:
		ppu2c0x_set_videorom_bank(0,0,2,data,128);

		break;
		case 0x1000:
		ppu2c0x_set_videorom_bank(0,2,2,data,128);

		break;
		case 0x2000:
		ppu2c0x_set_videorom_bank(0,4,2,data,128);

		break;
		case 0x3000: /* ok? */
		ppu2c0x_set_videorom_bank(0,6,2,data,128);

		break;

		case 0x7000:
		memcpy( &memory_region( REGION_CPU1 )[0x08000], &memory_region( REGION_CPU1 )[0x10000 +data*0x4000], 0x4000 );
		break;

	}

}

DRIVER_INIT( platoon )
{

	/* when starting a mapper 68 game  the first 16K ROM bank in the cart is loaded into $8000
    the LAST 16K ROM bank is loaded into $C000. The last 16K of ROM cannot be swapped. */

	memcpy( &memory_region( REGION_CPU1 )[0x08000], &memory_region( REGION_CPU1 )[0x10000], 0x4000 );
	memcpy( &memory_region( REGION_CPU1 )[0x0c000], &memory_region( REGION_CPU1 )[0x2c000], 0x4000 );

	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x8000, 0xffff, 0, 0, mapper68_rom_banking );

	init_vsnes(machine);

 	remapped_colortable = rp2c04001_colortable;

}

/**********************************************************************************/
/* Vs. Raid on Bungeling Bay (Japan) */

static int ret;
static WRITE8_HANDLER ( set_bnglngby_irq_w )
{
	ret = data;
	cpunum_set_input_line(Machine, 0, 0, ( data & 2 ) ? ASSERT_LINE : CLEAR_LINE );
	/* other values ??? */
	/* 0, 4, 84 */
}

static READ8_HANDLER ( set_bnglngby_irq_r )
{
	return ret;
}

DRIVER_INIT( bnglngby )
{
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x0231, 0x0231, 0, 0, set_bnglngby_irq_r );
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x0231, 0x0231, 0, 0, set_bnglngby_irq_w );

	/* extra ram */
	memory_install_readwrite8_handler(0, ADDRESS_SPACE_PROGRAM, 0x6000, 0x7fff, 0, 0, MRA8_BANK1, MWA8_BANK1 );
	memory_set_bankptr(1, auto_malloc(0x2000));

	ret = 0;

	/* common init */
	init_vsnes(machine);

	/* normal banking */
	DRIVER_INIT_CALL(vsnormal);

	remapped_colortable = rp2c04002_colortable;
}

/**********************************************************************************/
/* Vs. Ninja Jajamaru Kun */

static READ8_HANDLER( jajamaru_security_r )
{
	/* low part must be 0x40 */
	return ppu2c0x_0_r( 2 ) | 0x40;
}

DRIVER_INIT( jajamaru )
{
	//It executes an illegal opcode: 0x04 at 0x9e67 and 0x9e1c
	//At 0x9e5d and 0x9e12 there is a conditional jump to it
	//Maybe it should be a DOP (double NOP)

	/* Protection */
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x2002, 0x2002, 0, 0, jajamaru_security_r );
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x2000, 0x2001, 0, 0, ppuRC2C05_protection );

	/* common init */
	init_vsnes(machine);

	/* normal banking */
	DRIVER_INIT_CALL(vsnormal);
}

/***********************************************************************/

/* Vs. Mighty Bomb Jack */

static READ8_HANDLER( mightybj_security_r )
{
	/* low part must be 0x3d */
	return ppu2c0x_0_r( 2 ) | 0x3d;
}

DRIVER_INIT( mightybj )
{
	/* Protection */
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x2002, 0x2002, 0, 0, mightybj_security_r );
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x2000, 0x2001, 0, 0, ppuRC2C05_protection );

	/* common init */
	init_vsnes(machine);
}

/**********************************************************************************/
/* VS Tennis */

static WRITE8_HANDLER( vstennis_vrom_banking )
{
	int other_cpu = cpu_getactivecpu() ^ 1;

	/* switch vrom */
	ppu2c0x_set_videorom_bank( cpu_getactivecpu(), 0, 8, ( data & 4 ) ? 1 : 0, 512 );

	/* bit 1 ( data & 2 ) triggers irq on the other cpu */
	cpunum_set_input_line(Machine, other_cpu, 0, ( data & 2 ) ? CLEAR_LINE : ASSERT_LINE );

	/* move along */
	if ( cpu_getactivecpu() == 0 )
		vsnes_in0_w( offset, data );
	else
		vsnes_in0_1_w( offset, data );
}

DRIVER_INIT( vstennis )
{
	/* vrom switching is enabled with bit 2 of $4016 */
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x4016, 0x4016, 0, 0, vstennis_vrom_banking );
	memory_install_write8_handler(1, ADDRESS_SPACE_PROGRAM, 0x4016, 0x4016, 0, 0, vstennis_vrom_banking );

	/* shared ram at $6000 */
	memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x6000, 0x7fff, 0, 0, MRA8_BANK1 );
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x6000, 0x7fff, 0, 0, MWA8_BANK1 );
	memory_install_read8_handler(1, ADDRESS_SPACE_PROGRAM, 0x6000, 0x7fff, 0, 0, MRA8_BANK1 );
	memory_install_write8_handler(1, ADDRESS_SPACE_PROGRAM, 0x6000, 0x7fff, 0, 0, MWA8_BANK1 );

	memory_set_bankptr(1, &memory_region(REGION_CPU1)[0x6000]);
}

/**********************************************************************/
/* VS Wrecking Crew */

DRIVER_INIT( wrecking )
{
	/* only differance between this and vstennis is the colors */

	DRIVER_INIT_CALL(vstennis);
	remapped_colortable = rp2c04002_colortable;
}

/**********************************************************************/
/* VS Balloon Fight */

DRIVER_INIT( balonfgt )
{
	/* only differance between this and vstennis is the colors */

	DRIVER_INIT_CALL(vstennis);

	remapped_colortable = rp2c04003_colortable;
}


/**********************************************************************/
/* VS Baseball */

DRIVER_INIT( vsbball )
{
	/* only differance between this and vstennis is the colors */

	DRIVER_INIT_CALL(vstennis);

	remapped_colortable = rp2c04001_colortable;

}


/**********************************************************************/
/* Dual Ice Climber Jpn */

DRIVER_INIT( iceclmrj )
{
	/* only differance between this and vstennis is the colors */

	DRIVER_INIT_CALL(vstennis);

	remapped_colortable = rp2c05004_colortable;

}

/**********************************************************************/
/* Battle City */
DRIVER_INIT( btlecity )
{
	init_vsnes(machine);
	DRIVER_INIT_CALL(vsnormal);
	remapped_colortable = rp2c04003_colortable;
}

/***********************************************************************/
/* Tetris */
DRIVER_INIT( vstetris )
{
	/* extra ram at $6000 is enabled with bit 1 of $4016 */
	memory_install_readwrite8_handler(0, ADDRESS_SPACE_PROGRAM, 0x6000, 0x7fff, 0, 0, MRA8_BANK1, MWA8_BANK1 );
	memory_set_bankptr(1, auto_malloc(0x2000));

	init_vsnes(machine);
	DRIVER_INIT_CALL(vsnormal);
	remapped_colortable = rp2c04003_colortable;
}
