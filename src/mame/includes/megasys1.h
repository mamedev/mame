/***************************************************************************

                            -= Jaleco Mega System 1 =-

                    driver by   Luca Elia (l.elia@tin.it)


    This file contains definitions used across multiple megasys1
    and non megasys1 Jaleco games:

    * Input ports
    * Scrolling layers handling
    * Code decryption handling

***************************************************************************/


/***************************************************************************

                                Input Ports

***************************************************************************/


#define COINS \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )\
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )\
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )\
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )\
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )\
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

#define JOY_4BUTTONS(_flag_) \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(_flag_)\
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(_flag_)\
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(_flag_)\
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(_flag_)\
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(_flag_)\
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(_flag_)\
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(_flag_)\
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(_flag_)

#define JOY_3BUTTONS(_flag_) \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(_flag_)\
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(_flag_)\
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(_flag_)\
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(_flag_)\
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(_flag_)\
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(_flag_)\
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(_flag_)\
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN)

#define JOY_2BUTTONS(_flag_) \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(_flag_)\
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(_flag_)\
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(_flag_)\
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(_flag_)\
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(_flag_)\
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(_flag_)\
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN)

#define RESERVE \
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* Reserve 1P */\
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* Reserve 2P */\
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

/* Coinage DSWs */
//  1]  01-41 02-31 03-21 07-11 06-12 05-13 04-14 00-FC * 2
//  2]  04-31 02-21 07-11 03-12 05-13 01-14 06-15 00-FC
//      00-41 20-31 10-21 38-11 18-12 28-13 08-14 30-15


#define COINAGE_6BITS \
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )\
	PORT_DIPSETTING(      0x0400, DEF_STR( 3C_1C ) )\
	PORT_DIPSETTING(      0x0200, DEF_STR( 2C_1C ) )\
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )\
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_2C ) )\
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )\
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_4C ) )\
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_5C ) )\
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )\
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )\
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )\
	PORT_DIPSETTING(      0x2000, DEF_STR( 3C_1C ) )\
	PORT_DIPSETTING(      0x1000, DEF_STR( 2C_1C ) )\
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )\
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_2C ) )\
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_3C ) )\
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_4C ) )\
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_5C ) )\

#define COINAGE_6BITS_2 \
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )\
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_1C ) )\
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )\
	PORT_DIPSETTING(      0x0300, DEF_STR( 2C_1C ) )\
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )\
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )\
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )\
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )\
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )\
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )\
	PORT_DIPSETTING(      0x0800, DEF_STR( 4C_1C ) )\
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )\
	PORT_DIPSETTING(      0x1800, DEF_STR( 2C_1C ) )\
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )\
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_2C ) )\
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_3C ) )\
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )\
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )\

#define COINAGE_8BITS \
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coin_A ) )\
	PORT_DIPSETTING(      0x0007, DEF_STR( 4C_1C ) )\
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )\
	PORT_DIPSETTING(      0x0009, DEF_STR( 2C_1C ) )\
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )\
/*  PORT_DIPSETTING(      0x0005, DEF_STR( 1C_1C ) )*/\
/*  PORT_DIPSETTING(      0x0004, DEF_STR( 1C_1C ) )*/\
/*  PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )*/\
/*  PORT_DIPSETTING(      0x0002, DEF_STR( 1C_1C ) )*/\
/*  PORT_DIPSETTING(      0x0001, DEF_STR( 1C_1C ) )*/\
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_3C ) )\
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_2C ) )\
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_3C ) )\
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_4C ) )\
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )\
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )\
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )\
	PORT_DIPNAME( 0x00f0, 0x00f0, DEF_STR( Coin_B ) )\
	PORT_DIPSETTING(      0x0070, DEF_STR( 4C_1C ) )\
	PORT_DIPSETTING(      0x0080, DEF_STR( 3C_1C ) )\
	PORT_DIPSETTING(      0x0090, DEF_STR( 2C_1C ) )\
	PORT_DIPSETTING(      0x00f0, DEF_STR( 1C_1C ) )\
/*  PORT_DIPSETTING(      0x0050, DEF_STR( 1C_1C ) )*/\
/*  PORT_DIPSETTING(      0x0040, DEF_STR( 1C_1C ) )*/\
/*  PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )*/\
/*  PORT_DIPSETTING(      0x0020, DEF_STR( 1C_1C ) )*/\
/*  PORT_DIPSETTING(      0x0010, DEF_STR( 1C_1C ) )*/\
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_3C ) )\
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_2C ) )\
	PORT_DIPSETTING(      0x00d0, DEF_STR( 1C_3C ) )\
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_4C ) )\
	PORT_DIPSETTING(      0x00b0, DEF_STR( 1C_5C ) )\
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_6C ) )\
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )


/***************************************************************************

                            Scrolling Layers Handling

***************************************************************************/

/*----------- defined in video/megasys1.c -----------*/

/* Variables */
extern tilemap *megasys1_tmap[3];

extern UINT16 *megasys1_scrollram[3];
extern UINT16 *megasys1_objectram, *megasys1_vregs, *megasys1_ram;

extern int megasys1_scrollx[3], megasys1_scrolly[3];
extern int megasys1_active_layers;
//extern int megasys1_screen_flag, megasys1_sprite_flag;
extern int megasys1_bits_per_color_code;


/* Functions */
VIDEO_START( megasys1 );
VIDEO_UPDATE( megasys1 );

PALETTE_INIT( megasys1 );

READ16_HANDLER( megasys1_vregs_C_r );

WRITE16_HANDLER( megasys1_vregs_A_w );
WRITE16_HANDLER( megasys1_vregs_C_w );
WRITE16_HANDLER( megasys1_vregs_D_w );

WRITE16_HANDLER( megasys1_scrollram_0_w );
WRITE16_HANDLER( megasys1_scrollram_1_w );
WRITE16_HANDLER( megasys1_scrollram_2_w );

void megasys1_set_vreg_flag(int which, int data);


/*----------- defined in drivers/megasys1.c -----------*/

void astyanax_rom_decode(running_machine *machine, const char *region);
void phantasm_rom_decode(running_machine *machine, const char *region);
void rodland_rom_decode (running_machine *machine, const char *region);
