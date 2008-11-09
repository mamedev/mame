/***********************************************************************

    DECO Cassette System driver

(01) Highway Chase (12/80)
(02) Sengoku Ninja tai (12/80)
(03) Manhattan (1/81)
(04) Terranean (81)
(??) Missile Sprinter (81)
(06) Nebula (3/81)
(07) Astro Fantasia (3/81)
(08) The Tower (2/81)
(09) Super Astro Fighter (5/81)
(??) Buramzon (81)
(11) Lock N Chase (4/81)
(12) The DECO BOY (Flash Boy) (8/81)
(13) Pro Golf (9/81)
(14) DS TeleJang (6/81)
(15) Lucky Poker (2/81)
(16) Treasure Island (1/82)
(18) Explorer (11/82)
(19) Disco No. 1 (3/82)
(20) Tornado (82)
(21) Mission X (3/82)
(22) Pro Tennis (5/82)
(??) Fishing (7/82)
(25) Angler Dandler
(26) Burger Time (6/83)
(27) Burnin' Rubber (11/82)
(27) Bump N Jump (4/83)
(28) Graplop (11/82)
(28) Cluster Buster (9/83)
(29) La-pa-pa (11/82)
(30) Skater (3/83)
(31) Pro Bowling (10/83)
(32) Night Star (4/83)
(33) Pro Soccer (11/83)
(34) Super Doubles Tennis
(??) Genesis (11/83) <I think this may be Boomer Rangr'>
(??) Bambolin (83)
(37) Zeroize (10/83)
(38) Scrum Try (3/84)
(39) Peter Pepper's Ice Cream Factory (2/84)
(40) Fighting Ice Hockey (4/84)
(41) Oh Zumou (5/84)
(42) Hello Gate Ball (8/84)
(??) Yellow Cab (84)
(44) Boulder Dash (8/85)
(??) Tokyo Mie Sinryohjyo (10/84)
(??) Tokyo Mie Sinryohyo2 (1/85)
(??) Geinohijin Sikaku Siken (5/85)

 ***********************************************************************/

#include "driver.h"
#include "machine/decocass.h"
#include "sound/ay8910.h"

static UINT8 *decocass_rambase;

static UINT8 *decrypted;

/***************************************************************************
 *
 *  write decrypted opcodes
 *
 ***************************************************************************/

INLINE int swap_bits_5_6(int data)
{
	return (data & 0x9f) | ((data & 0x20) << 1) | ((data & 0x40) >> 1);
}

static WRITE8_HANDLER( ram_w )        { decrypted[0x0000 + offset] = swap_bits_5_6(data); decocass_rambase[0x0000 + offset] = data;  }
static WRITE8_HANDLER( charram_w )    { decrypted[0x6000 + offset] = swap_bits_5_6(data); decocass_charram_w(machine, offset, data); }
static WRITE8_HANDLER( fgvideoram_w ) { decrypted[0xc000 + offset] = swap_bits_5_6(data); decocass_fgvideoram_w(machine, offset, data); }
static WRITE8_HANDLER( fgcolorram_w ) { decrypted[0xc400 + offset] = swap_bits_5_6(data); decocass_colorram_w(machine, offset, data); }
static WRITE8_HANDLER( tileram_w )    { decrypted[0xd000 + offset] = swap_bits_5_6(data); decocass_tileram_w(machine, offset, data); }
static WRITE8_HANDLER( objectram_w )  { decrypted[0xd800 + offset] = swap_bits_5_6(data); decocass_objectram_w(machine, offset, data); }

static WRITE8_HANDLER( mirrorvideoram_w ) { offset = ((offset >> 5) & 0x1f) | ((offset & 0x1f) << 5); fgvideoram_w(machine, offset, data); }
static WRITE8_HANDLER( mirrorcolorram_w ) { offset = ((offset >> 5) & 0x1f) | ((offset & 0x1f) << 5); fgcolorram_w(machine, offset, data); }
static READ8_HANDLER( mirrorvideoram_r ) { offset = ((offset >> 5) & 0x1f) | ((offset & 0x1f) << 5); return decocass_fgvideoram[offset]; }
static READ8_HANDLER( mirrorcolorram_r ) { offset = ((offset >> 5) & 0x1f) | ((offset & 0x1f) << 5); return decocass_colorram[offset]; }


static ADDRESS_MAP_START( decocass_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_RAM_WRITE(ram_w) AM_BASE(&decocass_rambase)
	AM_RANGE(0x6000, 0xbfff) AM_RAM_WRITE(charram_w) AM_BASE(&decocass_charram) /* still RMS3 RAM */
	AM_RANGE(0xc000, 0xc3ff) AM_RAM_WRITE(fgvideoram_w) AM_BASE(&decocass_fgvideoram) AM_SIZE(&decocass_fgvideoram_size)  /* DSP3 RAM */
	AM_RANGE(0xc400, 0xc7ff) AM_RAM_WRITE(fgcolorram_w) AM_BASE(&decocass_colorram) AM_SIZE(&decocass_colorram_size)
	AM_RANGE(0xc800, 0xcbff) AM_READWRITE(mirrorvideoram_r, mirrorvideoram_w)
	AM_RANGE(0xcc00, 0xcfff) AM_READWRITE(mirrorcolorram_r, mirrorcolorram_w)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(tileram_w) AM_BASE(&decocass_tileram) AM_SIZE(&decocass_tileram_size)
	AM_RANGE(0xd800, 0xdbff) AM_RAM_WRITE(objectram_w) AM_BASE(&decocass_objectram) AM_SIZE(&decocass_objectram_size)
	AM_RANGE(0xe000, 0xe0ff) AM_RAM_WRITE(decocass_paletteram_w) AM_BASE(&paletteram)
	AM_RANGE(0xe300, 0xe300) AM_READ_PORT("DSW1") AM_WRITE(decocass_watchdog_count_w)
	AM_RANGE(0xe301, 0xe301) AM_READ_PORT("DSW2") AM_WRITE(decocass_watchdog_flip_w)
	AM_RANGE(0xe302, 0xe302) AM_WRITE(decocass_color_missiles_w)
	AM_RANGE(0xe400, 0xe400) AM_WRITE(decocass_reset_w)

/* BIO-3 board */
	AM_RANGE(0xe402, 0xe402) AM_WRITE(decocass_mode_set_w)
	AM_RANGE(0xe403, 0xe403) AM_WRITE(decocass_back_h_shift_w)
	AM_RANGE(0xe404, 0xe404) AM_WRITE(decocass_back_vl_shift_w)
	AM_RANGE(0xe405, 0xe405) AM_WRITE(decocass_back_vr_shift_w)
	AM_RANGE(0xe406, 0xe406) AM_WRITE(decocass_part_h_shift_w)
	AM_RANGE(0xe407, 0xe407) AM_WRITE(decocass_part_v_shift_w)

	AM_RANGE(0xe410, 0xe410) AM_WRITE(decocass_color_center_bot_w)
	AM_RANGE(0xe411, 0xe411) AM_WRITE(decocass_center_h_shift_space_w)
	AM_RANGE(0xe412, 0xe412) AM_WRITE(decocass_center_v_shift_w)
	AM_RANGE(0xe413, 0xe413) AM_WRITE(decocass_coin_counter_w)
	AM_RANGE(0xe414, 0xe414) AM_WRITE(decocass_sound_command_w)
	AM_RANGE(0xe415, 0xe416) AM_WRITE(decocass_quadrature_decoder_reset_w)
	AM_RANGE(0xe417, 0xe417) AM_WRITE(decocass_nmi_reset_w)
	AM_RANGE(0xe420, 0xe42f) AM_WRITE(decocass_adc_w)

	AM_RANGE(0xe500, 0xe5ff) AM_READWRITE(decocass_e5xx_r, decocass_e5xx_w)	/* read data from 8041/status */

	AM_RANGE(0xe600, 0xe6ff) AM_READ(decocass_input_r)		/* inputs */
	AM_RANGE(0xe700, 0xe700) AM_READ(decocass_sound_data_r)	/* read sound CPU data */
	AM_RANGE(0xe701, 0xe701) AM_READ(decocass_sound_ack_r)	/* read sound CPU ack status */

	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( decocass_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x17ff) AM_READWRITE(decocass_sound_nmi_enable_r, decocass_sound_nmi_enable_w)
	AM_RANGE(0x1800, 0x1fff) AM_READWRITE(decocass_sound_data_ack_reset_r, decocass_sound_data_ack_reset_w)
	AM_RANGE(0x2000, 0x2fff) AM_WRITE(ay8910_write_port_0_w)
	AM_RANGE(0x4000, 0x4fff) AM_WRITE(ay8910_control_port_0_w)
	AM_RANGE(0x6000, 0x6fff) AM_WRITE(ay8910_write_port_1_w)
	AM_RANGE(0x8000, 0x8fff) AM_WRITE(ay8910_control_port_1_w)
	AM_RANGE(0xa000, 0xafff) AM_READ(decocass_sound_command_r)
	AM_RANGE(0xc000, 0xcfff) AM_WRITE(decocass_sound_data_w)
	AM_RANGE(0xf800, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( decocass_mcu_portmap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x01, 0x01) AM_READWRITE(i8041_p1_r, i8041_p1_w)
	AM_RANGE(0x02, 0x02) AM_READWRITE(i8041_p2_r, i8041_p2_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( decocass )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH,IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH,IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH,IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH,IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH,IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH,IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH,IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH,IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH,IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH,IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH,IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH,IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH,IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH,IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH,IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH,IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH,IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH,IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH,IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH,IPT_START1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH,IPT_START2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH,IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("AN0")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("AN1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("AN2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("AN3")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(	0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(	0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x30, "Board type" )    /* used by the "bios" */
	PORT_DIPSETTING(	0x00, "old" )
	PORT_DIPSETTING(	0x10, "invalid?" )
	PORT_DIPSETTING(	0x20, "invalid?" )
	PORT_DIPSETTING(	0x30, "new" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK	)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cbtime )
	PORT_INCLUDE( decocass )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(	0x01, "3" )
	PORT_DIPSETTING(	0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(	0x06, "20000" )
	PORT_DIPSETTING(	0x04, "30000" )
	PORT_DIPSETTING(	0x02, "40000"  )
	PORT_DIPSETTING(	0x00, "50000"  )
	PORT_DIPNAME( 0x08, 0x08, "Enemies" )
	PORT_DIPSETTING(	0x08, "4" )
	PORT_DIPSETTING(	0x00, "6" )
	PORT_DIPNAME( 0x10, 0x10, "End of Level Pepper" )
	PORT_DIPSETTING(	0x10, DEF_STR( No ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x80, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	1024,	/* 1024 characters */
	3,		/* 3 bits per pixel */
	{ 2*1024*8*8, 1024*8*8, 0 },	/* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 	/* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,	/* 16*16 sprites */
	256,	/* 256 sprites */
	3,		/* 3 bits per pixel */
	{ 2*256*16*16, 256*16*16, 0 },	/* the bitplanes are separated */
	{ 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
	  0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	  8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8	/* every sprite takes 32 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	16,16,	/* 16*16 characters */
	16+1,	/* 16 tiles (+1 empty tile used in the half-width bg tilemaps) */
	3,	/* 3 bits per pixel */
	{ 2*16*16*16+4, 2*16*16*16+0, 4 },
	{ 3*16*8+0, 3*16*8+1, 3*16*8+2, 3*16*8+3,
	  2*16*8+0, 2*16*8+1, 2*16*8+2, 2*16*8+3,
	  16*8+0, 16*8+1, 16*8+2, 16*8+3,
	  0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	  8*8, 9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	2*16*16 /* every tile takes 64 consecutive bytes */
};

static const UINT32 objlayout_xoffset[64] =
{
	7*8+0,7*8+1,7*8+2,7*8+3,7*8+4,7*8+5,7*8+6,7*8+7,
	6*8+0,6*8+1,6*8+2,6*8+3,6*8+4,6*8+5,6*8+6,6*8+7,
	5*8+0,5*8+1,5*8+2,5*8+3,5*8+4,5*8+5,5*8+6,5*8+7,
	4*8+0,4*8+1,4*8+2,4*8+3,4*8+4,4*8+5,4*8+6,4*8+7,
	3*8+0,3*8+1,3*8+2,3*8+3,3*8+4,3*8+5,3*8+6,3*8+7,
	2*8+0,2*8+1,2*8+2,2*8+3,2*8+4,2*8+5,2*8+6,2*8+7,
	1*8+0,1*8+1,1*8+2,1*8+3,1*8+4,1*8+5,1*8+6,1*8+7,
	0*8+0,0*8+1,0*8+2,0*8+3,0*8+4,0*8+5,0*8+6,0*8+7
};

static const UINT32 objlayout_yoffset[64] =
{
	63*2*64,62*2*64,61*2*64,60*2*64,59*2*64,58*2*64,57*2*64,56*2*64,
	55*2*64,54*2*64,53*2*64,52*2*64,51*2*64,50*2*64,49*2*64,48*2*64,
	47*2*64,46*2*64,45*2*64,44*2*64,43*2*64,42*2*64,41*2*64,40*2*64,
	39*2*64,38*2*64,37*2*64,36*2*64,35*2*64,34*2*64,33*2*64,32*2*64,
	31*2*64,30*2*64,29*2*64,28*2*64,27*2*64,26*2*64,25*2*64,24*2*64,
	23*2*64,22*2*64,21*2*64,20*2*64,19*2*64,18*2*64,17*2*64,16*2*64,
	15*2*64,14*2*64,13*2*64,12*2*64,11*2*64,10*2*64, 9*2*64, 8*2*64,
	 7*2*64, 6*2*64, 5*2*64, 4*2*64, 3*2*64, 2*2*64, 1*2*64, 0*2*64
};

static const gfx_layout objlayout =
{
	64,64,	/* 64x64 object */
	2,		/* 2 objects */
	1,		/* 1 bits per pixel */
	{ 0 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	8*8, /* object takes 8 consecutive bytes */
	objlayout_xoffset,
	objlayout_yoffset
};

static GFXDECODE_START( decocass )
	GFXDECODE_ENTRY( NULL, 0x6000, charlayout,		 0, 4 )  /* char set #1 */
	GFXDECODE_ENTRY( NULL, 0x6000, spritelayout, 	 0, 4 )  /* sprites */
	GFXDECODE_ENTRY( NULL, 0xd000, tilelayout,		32, 2 )  /* background tiles */
	GFXDECODE_ENTRY( NULL, 0xd800, objlayout,		48, 4 )  /* object */
GFXDECODE_END

static PALETTE_INIT( decocass )
{
	int i;

	machine->colortable = colortable_alloc(machine, 32);

	/* set up 32 colors 1:1 pens */
	for (i = 0; i < 32; i++)
		colortable_entry_set_value(machine->colortable, i, i);

	/* setup straight/flipped colors for background tiles (D7 of color_center_bot ?) */
	for (i = 0; i < 8; i++)
	{
		colortable_entry_set_value(machine->colortable, 32+i, 3*8+i);
		colortable_entry_set_value(machine->colortable, 40+i, 3*8+((i << 1) & 0x04) + ((i >> 1) & 0x02) + (i & 0x01));
	}

	/* setup 4 colors for 1bpp object */
	colortable_entry_set_value(machine->colortable, 48+0*2+0, 0);
	colortable_entry_set_value(machine->colortable, 48+0*2+1, 25);	/* testtape red from 4th palette section? */
	colortable_entry_set_value(machine->colortable, 48+1*2+0, 0);
	colortable_entry_set_value(machine->colortable, 48+1*2+1, 28);	/* testtape blue from 4th palette section? */
	colortable_entry_set_value(machine->colortable, 48+2*2+0, 0);
	colortable_entry_set_value(machine->colortable, 48+2*2+1, 26);	/* testtape green from 4th palette section? */
	colortable_entry_set_value(machine->colortable, 48+3*2+0, 0);
	colortable_entry_set_value(machine->colortable, 48+3*2+1, 23);	/* ???? */
}


static MACHINE_DRIVER_START( decocass )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", M6502,750000)
	MDRV_CPU_PROGRAM_MAP(decocass_map,0)

	MDRV_CPU_ADD("audio", M6502,500000) /* 500 kHz */
	MDRV_CPU_PROGRAM_MAP(decocass_sound_map,0)

	MDRV_CPU_ADD("mcu", I8041,500000*15) /* 500 kHz ( I doubt it is 400kHz Al! )*/
	MDRV_CPU_IO_MAP(decocass_mcu_portmap,0)

	MDRV_INTERLEAVE(70)				/* interleave CPUs */

	MDRV_MACHINE_RESET(decocass)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(57)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(3072)		/* frames per second, vblank duration */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(decocass)
	MDRV_PALETTE_LENGTH(32+2*8+2*4)

	MDRV_PALETTE_INIT(decocass)
	MDRV_VIDEO_START(decocass)
	MDRV_VIDEO_UPDATE(decocass)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay1", AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MDRV_SOUND_ADD("ay2", AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( ctsttape )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(ctsttape)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( chwy )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(chwy)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( clocknch )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(clocknch)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( ctisland )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(ctisland)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( csuperas )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(csuperas)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( castfant )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(castfant)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cluckypo )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(cluckypo)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cterrani )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(cterrani)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cexplore )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(cexplore)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cprogolf )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(cprogolf)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cmissnx )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(cmissnx)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cdiscon1 )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(cdiscon1)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cptennis )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(cptennis)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( ctornado )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(ctornado)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cbnj )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(cbnj)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cburnrub )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(cburnrub)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cbtime )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(cbtime)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cgraplop )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(cgraplop)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cgraplp2 )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(cgraplp2)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( clapapa )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(clapapa)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cfghtice )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(cfghtice)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cprobowl )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(cprobowl)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cnightst )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(cnightst)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cprosocc )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(cprosocc)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cppicf )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(cppicf)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cbdash )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(cbdash)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cscrtry )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(cscrtry)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cflyball )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(cflyball)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( czeroize )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(decocass)
	MDRV_MACHINE_RESET(czeroize)
	MDRV_SCREEN_MODIFY("main")
	MDRV_SCREEN_VISIBLE_AREA(1*8, 32*8-1, 1*8, 31*8-1)
MACHINE_DRIVER_END


#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios+1)) /* Note '+1' */


#define DECOCASS_BIOS \
	ROM_REGION( 0x10000, "main", 0 ) \
	ROM_SYSTEM_BIOS( 0, "bios0", "rms8.cpu" ) \
	ROM_LOAD_BIOS( 0, "rms8.cpu",     0xf000, 0x1000, CRC(23d929b7) SHA1(063f83020ba3d6f43ab8471f95ca919767b93aa4) ) \
	ROM_SYSTEM_BIOS( 1, "bios1", "dsp3.p0b & dsp3.p1b" ) \
	ROM_LOAD_BIOS( 1, "dsp3.p0b",     0xf000, 0x0800, CRC(b67a91d9) SHA1(681c040be0f0ed1ba0a50161b36d0ad8e1c8c5cb) ) \
	ROM_LOAD_BIOS( 1, "dsp3.p1b",     0xf800, 0x0800, CRC(3bfff5f3) SHA1(4e9437cb1b76d64da6b37f01bd6e879fb399e8ce) ) \

#define DECOCASS_COMMON_ROMS	\
	DECOCASS_BIOS \
\
	ROM_REGION( 0x10000, "audio", 0 )	  \
	ROM_LOAD( "rms8.snd",     0xf800, 0x0800, CRC(b66b2c2a) SHA1(0097f38beb4872e735e560148052e258a26b08fd) ) \
\
	ROM_REGION( 0x10000, "mcu", 0 )	  /* 4k for the MCU (actually 1K ROM + 64 bytes RAM @ 0x800) */ \
	ROM_LOAD( "cass8041.bin", 0x0000, 0x0400, CRC(a6df18fd) SHA1(1f9ea47e372d31767c936c15852b43df2b0ee8ff) ) \
\
	ROM_REGION( 0x00060, "proms", 0 )	  /* PROMS */ \
	ROM_LOAD( "dsp8.3m",      0x0000, 0x0020, CRC(238fdb40) SHA1(b88e8fabb82092105c3828154608ea067acbf2e5) ) \
	ROM_LOAD( "dsp8.10d",     0x0020, 0x0020, CRC(3b5836b4) SHA1(b630bb277d9ec09d46ef26b944014dd6165b35d8) ) \
	ROM_LOAD( "rms8.j3",      0x0040, 0x0020, CRC(51eef657) SHA1(eaedce5caf55624ad6ae706aedf82c5717c60f1f) ) /* DRAM banking and timing */ \


ROM_START( decocass )
	DECOCASS_COMMON_ROMS

ROM_END

/* The Following use Dongle Type 1 (DE-0061)
    (dongle data same for each game)         */

ROM_START( ctsttape )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x00020, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "de-0061.pro", 0x0000, 0x0020, CRC(e09ae5de) SHA1(7dec067d0739a6dad2607132641b66880a5b7751) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "testtape.cas", 0x0000, 0x2000, CRC(4f9d8efb) SHA1(5b77747dad1033e5703f06c0870441b54b4256c5) )
ROM_END

ROM_START( chwy )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x00020, "user1", 0 )	  /* dongle data */
	/* The dongle data is reverse engineered from manual decryption */
	ROM_LOAD( "chwy.pro",   0x0000, 0x0020, BAD_DUMP CRC(2fae678e) SHA1(4a7de851442d4c1d690de03262f0e136a52fca35) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "chwy.cas",   0x0000, 0x8000, CRC(68a48064) SHA1(7e389737972fd0c54f398d296159c561f5ec3a93) )
ROM_END

ROM_START( clocknch )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x00020, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "de-0061.pro", 0x0000, 0x0020, CRC(e09ae5de) SHA1(7dec067d0739a6dad2607132641b66880a5b7751) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "clocknch.cas", 0x0000, 0x8000, CRC(c9d163a4) SHA1(3ef55a8d8f603059e263776c08eb81f2cf18b75c) )
ROM_END

ROM_START( ctisland )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x00020, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "de-0061.pro", 0x0000, 0x0020, CRC(e09ae5de) SHA1(7dec067d0739a6dad2607132641b66880a5b7751) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "ctisland.cas", 0x0000, 0x8000, CRC(3f63b8f8) SHA1(2fd0679ef9750a228ebb098672ab6091fda75804) )

	ROM_REGION( 0x4000, "user3", 0 )	  /* roms from the overlay pcb */
	ROM_LOAD( "deco-ti.x1",   0x0000, 0x1000, CRC(a7f8aeba) SHA1(0c9ba1a46d0636b36f40fad31638db89f374f778) )
	ROM_LOAD( "deco-ti.x2",   0x1000, 0x1000, CRC(2a0d3c91) SHA1(552d08fcddddbea5b52fa1e8decd188ae49c86ea) )
	ROM_LOAD( "deco-ti.x3",   0x2000, 0x1000, CRC(3a26b97c) SHA1(f57e76077806e149a9e455c85e5431eac2d42bc3) )
	ROM_LOAD( "deco-ti.x4",   0x3000, 0x1000, CRC(1cbe43de) SHA1(8f26ad224e96c87da810c60d3dd88d415400b9fc) )
ROM_END

ROM_START( ctislnd2 )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x00020, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "de-0061.pro", 0x0000, 0x0020, CRC(e09ae5de) SHA1(7dec067d0739a6dad2607132641b66880a5b7751) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "ctislnd2.cas", 0x0000, 0x8000, CRC(2854b4c0) SHA1(d3b4e0031dbb2340fbbe396a1ff9b8fbfd63663e) )

	ROM_REGION( 0x4000, "user3", 0 )	  /* roms from the overlay pcb */
	ROM_LOAD( "deco-ti.x1",   0x0000, 0x1000, CRC(a7f8aeba) SHA1(0c9ba1a46d0636b36f40fad31638db89f374f778) )
	ROM_LOAD( "deco-ti.x2",   0x1000, 0x1000, CRC(2a0d3c91) SHA1(552d08fcddddbea5b52fa1e8decd188ae49c86ea) )
	ROM_LOAD( "deco-ti.x3",   0x2000, 0x1000, CRC(3a26b97c) SHA1(f57e76077806e149a9e455c85e5431eac2d42bc3) )
	ROM_LOAD( "deco-ti.x4",   0x3000, 0x1000, CRC(1cbe43de) SHA1(8f26ad224e96c87da810c60d3dd88d415400b9fc) )
ROM_END

ROM_START( ctislnd3 )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x00020, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "de-0061.pro", 0x0000, 0x0020, CRC(e09ae5de) SHA1(7dec067d0739a6dad2607132641b66880a5b7751) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "ctislnd3.cas", 0x0000, 0x8000, CRC(45464e1e) SHA1(03275694d963c7ab0e0f5525e248e69da5f9b591) )

	ROM_REGION( 0x4000, "user3", 0 )	  /* roms from the overlay pcb */
	ROM_LOAD( "deco-ti.x1",   0x0000, 0x1000, CRC(a7f8aeba) SHA1(0c9ba1a46d0636b36f40fad31638db89f374f778) )
	ROM_LOAD( "deco-ti.x2",   0x1000, 0x1000, CRC(2a0d3c91) SHA1(552d08fcddddbea5b52fa1e8decd188ae49c86ea) )
	ROM_LOAD( "deco-ti.x3",   0x2000, 0x1000, CRC(3a26b97c) SHA1(f57e76077806e149a9e455c85e5431eac2d42bc3) )
	ROM_LOAD( "deco-ti.x4",   0x3000, 0x1000, CRC(1cbe43de) SHA1(8f26ad224e96c87da810c60d3dd88d415400b9fc) )
ROM_END

ROM_START( csuperas )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x00020, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "de-0061.pro", 0x0000, 0x0020, CRC(e09ae5de) SHA1(7dec067d0739a6dad2607132641b66880a5b7751) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "csuperas.cas", 0x0000, 0x8000, CRC(fabcd07f) SHA1(4070c668ad6725f0710cf7fe6df0d5f80272a449) )
ROM_END

ROM_START( castfant )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x00020, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "de-0061.pro", 0x0000, 0x0020, CRC(e09ae5de) SHA1(7dec067d0739a6dad2607132641b66880a5b7751) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "castfant.cas", 0x0000, 0x8000, CRC(6d77d1b5) SHA1(821bd65fbe887cbeac9281a2ad3f88595918f886) )
ROM_END

ROM_START( cluckypo )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x00020, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "de-0061.pro", 0x0000, 0x0020, CRC(e09ae5de) SHA1(7dec067d0739a6dad2607132641b66880a5b7751) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "cluckypo.cas", 0x0000, 0x8000, CRC(2070c243) SHA1(cd3af309af8eb27937756c1fe6fd0504be5aaaf5) )
ROM_END

ROM_START( cterrani )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x00020, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "de-0061.pro", 0x0000, 0x0020, CRC(e09ae5de) SHA1(7dec067d0739a6dad2607132641b66880a5b7751) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "cterrani.cas", 0x0000, 0x8000, CRC(eb71adbc) SHA1(67becfde39c034d4b8edc2eb100050de102773da) )
ROM_END

ROM_START( cexplore )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x00020, "user1", 0 )	  /* dongle data */
	/* The dongle data is reverse engineered by table analysis */
	ROM_LOAD( "cexplore.pro", 0x0000, 0x0020, BAD_DUMP CRC(c7a9ac8f) SHA1(b0a566d948f71a4eddcde0dd5e9e69ca96f71c36) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "cexplore.cas", 0x0000, 0x8000, CRC(fae49c66) SHA1(4ae69e2f706fdf30204f0aa1277619395cacc21b) )

	ROM_REGION( 0x4000, "user3", 0 )	  /* roms from the overlay pcb */
	ROM_LOAD( "cexplore_overlay_roms", 0x0000, 0x4000, NO_DUMP )
ROM_END

ROM_START( cprogolf )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x00020, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "de-0061.pro", 0x0000, 0x0020, CRC(e09ae5de) SHA1(7dec067d0739a6dad2607132641b66880a5b7751) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "cprogolf.cas", 0x0000, 0x8000, CRC(02123cd1) SHA1(e4c630ed293725f23d539cb43beb97953558dabd) )
ROM_END

/* The Following use Dongle Type 2 (CS82-007)
    (dongle data differs for each game)      */

ROM_START( cmissnx )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x00800, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "cmissnx.pro",  0x0000, 0x0800, CRC(8a41c071) SHA1(7b16d933707bf21d25dcd11db6a6c28834b11c5b) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "cmissnx.cas",  0x0000, 0x8000, CRC(3a094e11) SHA1(c355fe14838187cbde19a799e5c60083c82615ac) )
ROM_END

ROM_START( cdiscon1 )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x00800, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "cdiscon1.pro", 0x0000, 0x0800, CRC(0f793fab) SHA1(331f1b1b482fcd10f42c388a503f9af62d705401) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "cdiscon1.cas", 0x0000, 0x8000, CRC(1429a397) SHA1(12f9e03fcda31dc6161a39bf5c3315a1e9e94565) )
ROM_END

ROM_START( csweetht )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x00800, "user1", 0 )   /* dongle data */
	ROM_LOAD( "cdiscon1.pro", 0x0000, 0x0800, CRC(0f793fab) SHA1(331f1b1b482fcd10f42c388a503f9af62d705401) )

	ROM_REGION( 0x10000, "user2", 0 )   /* (max) 64k for cassette image */
	ROM_LOAD( "csweetht.cas", 0x0000, 0x8000, CRC(175ef706) SHA1(49b86233f69d0daf54a6e59b86e69b8159e8f6cc) )
ROM_END

ROM_START( cptennis )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x00800, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "cptennis.pro", 0x0000, 0x0800, CRC(59b8cede) SHA1(514861a652b5256a11477fc357bc01dfd87f712b) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "cptennis.cas", 0x0000, 0x8000, CRC(6bb257fe) SHA1(7554bf1996bc9e9c04a276aab050708d70103f54) )
ROM_END

ROM_START( ctornado )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x00800, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "ctornado.pro", 0x0000, 0x0800, CRC(c9a91697) SHA1(3f7163291edbdf1a596e3cd2b7a16bbb140ffb36) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "ctornado.cas", 0x0000, 0x8000, CRC(e4e36ce0) SHA1(48a11823121fb2e3de31ae08e453c0124fc4f7f3) )
ROM_END

/* The Following use Dongle Type 3 (unknown part number?)
    (dongle data differs for each game)      */

ROM_START( cburnrub )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x01000, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "cburnrub.pro",   0x0000, 0x1000, CRC(9f396832) SHA1(0e302fd094474ac792882948a018c73ce76e0759) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "cburnrub.cas",   0x0000, 0x8000, CRC(4528ac22) SHA1(dc0fcc5e5fd21c1c858a90f43c175e36a24b3c3d) )
ROM_END

ROM_START( cburnrb2 )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x01000, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "cburnrub.pro",   0x0000, 0x1000, CRC(9f396832) SHA1(0e302fd094474ac792882948a018c73ce76e0759) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "cburnrb2.cas",   0x0000, 0x8000, CRC(84a9ed66) SHA1(a9c536e46b89fc6b9c6271776292fed1241d2f3f) )
ROM_END

ROM_START( cbnj )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x01000, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "cburnrub.pro",   0x0000, 0x1000, CRC(9f396832) SHA1(0e302fd094474ac792882948a018c73ce76e0759) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "cbnj.cas",       0x0000, 0x8000, CRC(eed41560) SHA1(85d5df76efac33cd10427f659c4259afabb3daaf) )
ROM_END

ROM_START( cbtime )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x01000, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "cbtime.pro",   0x0000, 0x1000, CRC(25bec0f0) SHA1(9fb1f9699f37937421e26d4fb8fdbcd21a5ddc5c) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "cbtime.cas",   0x0000, 0x8000, CRC(56d7dc58) SHA1(34b2513c9ca7ab40f532b6d6d911aa3012113632) )
ROM_END

ROM_START( cgraplop )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x01000, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "cgraplop.pro", 0x0000, 0x1000, CRC(ee93787d) SHA1(0c753d62fdce2fdbd5b329a5aa259a967d07a651) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "cgraplop.cas", 0x0000, 0x8000, CRC(d2c1c1bb) SHA1(db67304caa11540363735e7d4bf03507ccbe9980) )
ROM_END

ROM_START( cgraplp2 )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x01000, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "cgraplop.pro", 0x0000, 0x1000, CRC(ee93787d) SHA1(0c753d62fdce2fdbd5b329a5aa259a967d07a651) ) /* is this right for this set? */

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "cgraplp2.cas", 0x0000, 0x8000, CRC(2e728981) SHA1(83ba90d95858d647315a1c311b8643672afea5f7) )
ROM_END

ROM_START( clapapa )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x01000, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "clapapa.pro",  0x0000, 0x1000, CRC(e172819a) SHA1(3492775f4f0a0b31ce5a1a998076829b3f264e98) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "clapapa.cas",  0x0000, 0x8000, CRC(4ffbac24) SHA1(1ec0d7ac1886d4b430dc12be27f387e9d952d235) )
ROM_END

ROM_START( clapapa2 )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x01000, "user1", 0 )   /* dongle data */
	ROM_LOAD( "clapapa.pro",  0x0000, 0x1000, CRC(e172819a) SHA1(3492775f4f0a0b31ce5a1a998076829b3f264e98) )

	ROM_REGION( 0x10000, "user2", 0 )   /* (max) 64k for cassette image */
	ROM_LOAD( "clapapa2.cas",  0x0000, 0x8000, CRC(069dd3c4) SHA1(5a19392c7ac5aea979187c96267e73bf5126307e) )
ROM_END

ROM_START( cfghtice )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x01000, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "cfghtice.pro", 0x0000, 0x1000, CRC(5abd27b5) SHA1(2ab1c171adffd491759036d6ce2433706654aad2) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "cfghtice.cas", 0x0000, 0x10000, CRC(906dd7fb) SHA1(894a7970d5476ed035edd15656e5cf10d6ddcf57) )
ROM_END

ROM_START( cprobowl )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x01000, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "cprobowl.pro", 0x0000, 0x1000, CRC(e3a88e60) SHA1(e6e9a2e5ab26e0463c63201a15f7d5a429ec836e) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "cprobowl.cas", 0x0000, 0x8000, CRC(cb86c5e1) SHA1(66c467418cff2ed6d7c121a8b1650ee97ae48fe9) )
ROM_END

ROM_START( cnightst )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x01000, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "cnightst.pro", 0x0000, 0x1000, CRC(553b0fbc) SHA1(2cdf4560992b62e59b6de760d7996be4ed25f505) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "cnightst.cas", 0x0000, 0x8000, CRC(c6f844cb) SHA1(5fc6154c20ee4e2f4049a78df6f3cacbb96b0dc0) )
ROM_END

ROM_START( cnights2 )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x01000, "user1", 0 )   /* dongle data */
	ROM_LOAD( "cnightst.pro", 0x0000, 0x1000, CRC(553b0fbc) SHA1(2cdf4560992b62e59b6de760d7996be4ed25f505) )

	ROM_REGION( 0x10000, "user2", 0 )   /* (max) 64k for cassette image */
	ROM_LOAD( "cnights2.cas", 0x0000, 0x8000, CRC(1a28128c) SHA1(4b620a1919d02814f734aba995115c09dc2db930) )
ROM_END

ROM_START( cprosocc )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x01000, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "cprosocc.pro", 0x0000, 0x1000,  CRC(919fabb2) SHA1(3d6a0676cea7b0be0fe69d06e04ca08c36b2851a) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "cprosocc.cas", 0x0000, 0x10000, CRC(76b1ad2c) SHA1(6188667e5bc001dfdf83deaf7251eae794de4702) )
ROM_END

ROM_START( cppicf )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x01000, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "cppicf.pro",   0x0000, 0x1000, CRC(0b1a1ecb) SHA1(2106da6837c78812c102b0eaaa1127fcc21ea780) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "cppicf.cas",   0x0000, 0x8000, CRC(8c02f160) SHA1(03430dd8d4b2e6ca931986dac4d39be6965ffa6f) )
ROM_END

ROM_START( cppicf2 )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x01000, "user1", 0 )   /* dongle data */
	ROM_LOAD( "cppicf.pro",   0x0000, 0x1000, CRC(0b1a1ecb) SHA1(2106da6837c78812c102b0eaaa1127fcc21ea780) )

	ROM_REGION( 0x10000, "user2", 0 )   /* (max) 64k for cassette image */
	ROM_LOAD( "cppicf2.cas",   0x0000, 0x8000, CRC(78ffa1bc) SHA1(d15f2a240ae7b45885d32b5f507243f82e820d4b) )
ROM_END

/* The Following use Dongle Type 4 (unknown part number?)
    (dongle data probably differs for each game, but only one is known using it atm) */

ROM_START( cscrtry )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x08000, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "cscrtry.pro",  0x0000, 0x8000, CRC(7bc3460b) SHA1(7c5668ff9a5073e27f4a83b02d79892eb4df6b92) )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "cscrtry.cas",  0x0000, 0x8000, CRC(5625f0ca) SHA1(f4b0a6f2ca908880386838f06b626479b4b74134) )
ROM_END

ROM_START( cscrtry2 )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x08000, "user1", 0 )   /* dongle data */
	ROM_LOAD( "cscrtry.pro",  0x0000, 0x8000, CRC(7bc3460b) SHA1(7c5668ff9a5073e27f4a83b02d79892eb4df6b92) )

	ROM_REGION( 0x10000, "user2", 0 )   /* (max) 64k for cassette image */
	ROM_LOAD( "cscrtry2.cas",  0x0000, 0x8000, CRC(04597842) SHA1(7f1fc3e06b61df880debe9056bdfbbb8600af739) )
ROM_END

/* The Following use Dongle Type 5 (unknown part number?)
    (dongle data not read)       */

ROM_START( cbdash )
	DECOCASS_COMMON_ROMS

/*  ROM_REGION( 0x01000, "user1", 0 ) */ /* (max) 4k for dongle data */
	/* no proms */

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "cbdash.cas",   0x0000, 0x8000, CRC(cba4c1af) SHA1(5d163d8e31c58b20679c6be06b1aa02df621822b) )
ROM_END

/* The Following have no Dongles at all */

ROM_START( cflyball )
	DECOCASS_COMMON_ROMS

	/* no dongle data */

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "cflyball.cas",   0x0000, 0x10000, CRC(cb40d043) SHA1(57698bac7e0d552167efa99d08116bf19a3b29c9) )
ROM_END

/* The Following have unknown Dongles
    (dongle data not read)       */

ROM_START( czeroize )
	DECOCASS_COMMON_ROMS

	ROM_REGION( 0x01000, "user1", 0 )	  /* dongle data */
	ROM_LOAD( "czeroize.pro",  0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x10000, "user2", 0 )	  /* (max) 64k for cassette image */
	ROM_LOAD( "czeroize.cas",   0x0000, 0x10000, CRC(3ef0a406) SHA1(645b34cd477e0bb5539c8fe937a7a2dbd8369003) )
ROM_END


static DRIVER_INIT( decocass )
{
	UINT8 *rom = memory_region(machine, "main");
	int A;

	/* allocate memory and mark all RAM regions with their decrypted pointers */
	decrypted = auto_malloc(0x10000);
	memory_set_decrypted_region(0, 0x0000, 0xc7ff, &decrypted[0x0000]);
	memory_set_decrypted_region(0, 0xd000, 0xdbff, &decrypted[0xd000]);
	memory_set_decrypted_region(0, 0xf000, 0xffff, &decrypted[0xf000]);

	/* Swap bits 5 & 6 for opcodes */
	for (A = 0xf000;A < 0x10000;A++)
		decrypted[A] = swap_bits_5_6(rom[A]);

	/* Call the state save setup code in machine/decocass.c */
	decocass_machine_state_save_init(machine);
	/* and in video/decocass.c, too */
	decocass_video_state_save_init();
}

static DRIVER_INIT( decocrom )
{
	int romlength = memory_region_length(machine, "user3");
	UINT8 *rom = memory_region(machine, "user3");
	UINT8 *decrypted2 = auto_malloc(romlength);
	int i;

	/* standard init */
	DRIVER_INIT_CALL(decocass);

	/* decrypt the ROMs */
	for (i = 0; i < romlength; i++)
		decrypted2[i] = swap_bits_5_6(rom[i]);

	/* convert charram to a banked ROM */
	memory_install_readwrite8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x6000, 0xafff, 0, 0, SMH_BANK1, decocass_de0091_w);
	memory_configure_bank(1, 0, 1, decocass_charram, 0);
	memory_configure_bank(1, 1, 1, memory_region(machine, "user3"), 0);
	memory_configure_bank_decrypted(1, 0, 1, &decrypted[0x6000], 0);
	memory_configure_bank_decrypted(1, 1, 1, decrypted2, 0);
	memory_set_bank(1, 0);

	/* install the bank selector */
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xe900, 0xe900, 0, 0, decocass_e900_w);
}


         GAME( 1981, decocass, 0, decocass, decocass, decocass, ROT270, "Data East Corporation", "DECO Cassette System", GAME_IS_BIOS_ROOT )
         GAME( 1981, ctsttape, decocass, ctsttape, decocass, decocass, ROT270, "Data East Corporation", "Test Tape (Cassette)", 0 )
/* 01 */ GAME( 1980, chwy,     decocass, chwy,     decocass, decocass, ROT270, "Data East Corporation", "Highway Chase (Cassette)", 0 )
/* 04 */ GAME( 1981, cterrani, decocass, cterrani, decocass, decocass, ROT270, "Data East Corporation", "Terranean (Cassette)", 0 )
/* 07 */ GAME( 1981, castfant, decocass, castfant, decocass, decocass, ROT270, "Data East Corporation", "Astro Fantasia (Cassette)", 0 )
/* 09 */ GAME( 1981, csuperas, decocass, csuperas, decocass, decocass, ROT270, "Data East Corporation", "Super Astro Fighter (Cassette)", 0 )
/* 11 */ GAME( 1981, clocknch, decocass, clocknch, decocass, decocass, ROT270, "Data East Corporation", "Lock'n'Chase (Cassette)", 0 )
/* 13 */ GAME( 1981, cprogolf, decocass, cprogolf, decocass, decocass, ROT270, "Data East Corporation", "Pro Golf (Cassette)", 0 )
/* 15 */ GAME( 1981, cluckypo, decocass, cluckypo, decocass, decocass, ROT270, "Data East Corporation", "Lucky Poker (Cassette)", 0 )
/* 16 */ GAME( 1981, ctisland, decocass, ctisland, decocass, decocrom, ROT270, "Data East Corporation", "Treasure Island (Cassette, set 1)", 0 )
/* 16 */ GAME( 1981, ctislnd2, ctisland, ctisland, decocass, decocrom, ROT270, "Data East Corporation", "Treasure Island (Cassette, set 2)", 0 )
/* 16 */ GAME( 1981, ctislnd3, ctisland, ctisland, decocass, decocrom, ROT270, "Data East Corporation", "Treasure Island (Cassette, set 3)", GAME_NOT_WORKING ) /* Different Bitswap? */
/* 18 */ GAME( 1982, cexplore, decocass, cexplore, decocass, decocass, ROT270, "Data East Corporation", "Explorer (Cassette)", GAME_NOT_WORKING )
/* 19 */ GAME( 1982, cdiscon1, decocass, cdiscon1, decocass, decocass, ROT270, "Data East Corporation", "Disco No.1 (Cassette)", 0 )
/* 19 */ GAME( 1982, csweetht, cdiscon1, cdiscon1, decocass, decocass, ROT270, "Data East Corporation", "Sweet Heart (Cassette)", 0 )
/* 20 */ GAME( 1982, ctornado, decocass, ctornado, decocass, decocass, ROT270, "Data East Corporation", "Tornado (Cassette)", 0 )
/* 21 */ GAME( 1982, cmissnx,  decocass, cmissnx,  decocass, decocass, ROT270, "Data East Corporation", "Mission-X (Cassette)", 0 )
/* 22 */ GAME( 1982, cptennis, decocass, cptennis, decocass, decocass, ROT270, "Data East Corporation", "Pro Tennis (Cassette)", 0 )
/* 26 */ GAME( 1983, cbtime,   decocass, cbtime,   cbtime,   decocass, ROT270, "Data East Corporation", "Burger Time (Cassette)", 0 )
/* 27 */ GAME( 1982, cburnrub, decocass, cburnrub, decocass, decocass, ROT270, "Data East Corporation", "Burnin' Rubber (Cassette, set 1)", 0 )
/* 27 */ GAME( 1982, cburnrb2, cburnrub, cburnrub, decocass, decocass, ROT270, "Data East Corporation", "Burnin' Rubber (Cassette, set 2)", 0 )
/* 27 */ GAME( 1982, cbnj,     cburnrub, cbnj,     decocass, decocass, ROT270, "Data East Corporation", "Bump N Jump (Cassette)", 0 )
/* 28 */ GAME( 1983, cgraplop, decocass, cgraplop, decocass, decocass, ROT270, "Data East Corporation", "Cluster Buster / Graplop (Cassette, set 1)", 0 )
/* 28 */ GAME( 1983, cgraplp2, cgraplop, cgraplp2, decocass, decocass, ROT270, "Data East Corporation", "Cluster Buster / Graplop (Cassette, set 2)", GAME_NOT_WORKING )
/* 29 */ GAME( 1983, clapapa,  decocass, clapapa,  decocass, decocass, ROT270, "Data East Corporation", "Rootin' Tootin' / La-Pa-Pa (Cassette)" , 0) /* Displays 'La-Pa-Pa during attract */
/* 29 */ GAME( 1983, clapapa2, clapapa, clapapa,  decocass, decocass, ROT270, "Data East Corporation", "Rootin' Tootin' (Cassette)" , 0)			/* Displays 'Rootin' Tootin' during attract */
/* 31 */ GAME( 1983, cprobowl, decocass, cprobowl, decocass, decocass, ROT270, "Data East Corporation", "Pro Bowling (Cassette)", 0 )
/* 32 */ GAME( 1983, cnightst, decocass, cnightst, decocass, decocass, ROT270, "Data East Corporation", "Night Star (Cassette, set 1)", 0 )
/* 32 */ GAME( 1983, cnights2, cnightst, cnightst, decocass, decocass, ROT270, "Data East Corporation", "Night Star (Cassette, set 2)", 0 )
/* 33 */ GAME( 1983, cprosocc, decocass, cprosocc, decocass, decocass, ROT270, "Data East Corporation", "Pro Soccer (Cassette)", 0 )
/* 37 */ GAME( 1983, czeroize, decocass, czeroize, decocass, decocass, ROT270, "Data East Corporation", "Zeroize (Cassette)", 0 )
/* 38 */ GAME( 1984, cscrtry,  decocass, cscrtry,  decocass, decocass, ROT270, "Data East Corporation", "Scrum Try (Cassette, set 1)", 0 )
/* 38 */ GAME( 1984, cscrtry2, cscrtry, cscrtry,  decocass, decocass, ROT270, "Data East Corporation", "Scrum Try (Cassette, set 2)", 0 )
/* 39 */ GAME( 1984, cppicf,   decocass, cppicf,   decocass, decocass, ROT270, "Data East Corporation", "Peter Pepper's Ice Cream Factory (Cassette, set 1)", 0 )
/* 39 */ GAME( 1984, cppicf2,  cppicf, cppicf,   decocass, decocass, ROT270, "Data East Corporation", "Peter Pepper's Ice Cream Factory (Cassette, set 2)", 0 )
/* 40 */ GAME( 1984, cfghtice, decocass, cfghtice, decocass, decocass, ROT270, "Data East Corporation", "Fighting Ice Hockey (Cassette)", 0 )
/* 44 */ GAME( 1985, cbdash,   decocass, cbdash,   decocass, decocass, ROT270, "Data East Corporation", "Boulder Dash (Cassette)", 0 )
         GAME( 1985, cflyball, decocass, cflyball, decocass, decocass, ROT270, "Data East Corporation", "Flying Ball (Cassette)", GAME_NO_SOUND )
