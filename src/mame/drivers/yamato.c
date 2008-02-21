/*

    T.S. 17.12.2005:

    Yamato:
    -------
     Added temporary bg gradient (bad colors/offset).

     Gradient table are stored in two(?) ROMs.
     Each table is 256 bytes long: 128 for normal
     and 128 bytes for flipped screen.
     Color format is unknown - probably direct RGB
     mapping of 8 or 16 (both roms) bits. Also table
     selection source is unknown.

     TODO:
      - bg gradient color decode & table selection


 Top Roller:
 ----------
     It's made by the same developers as Yamato and use
     probably the same encrypted SEGA cpu as Yamato.

     lives - $6155

     TODO:

       - COINB DSW is missing
       - few issues in cocktail mode
       - wrong colors (fg text layer) - game sometimes ("round" text , lives) updates only even columns of cell attribs...

-------------------------------------------------------------------


 Top Roller
 Jaleco

 Hardware : Original Jaleco board no 8307-B/8307-A(redump)

 Main CPU : Encrypted Z80 (probably 315-5018)
 Sound : AY-3-8910

 ROMS CRC32 + positions :

 [9894374d]  d5
 [ef789f00]  f5
 [d45494ba]  h5
 [1cb48ea0]  k5
 [84139f46]  l5
 [e30c1dd8]  m5
 [904fffb6]  d3
 [94371cfb]  f3
 [8a8032a7]  h3
 [1e8914a6]  k3
 [b20a9fa2]  l3
 [7f989dc9]  p3
 [89327329]  a4 bottom board 89327329
 [7a945733]  c4 bottom board
 [5f2c2a78]  h4 bottom board  bad dump / [1d9e3325] (8307-A)
 [ce3afe26]  j4 bottom board

*/


#include "driver.h"
#include "machine/segacrpt.h"
#include "sound/ay8910.h"
#include "includes/cclimber.h"

static PALETTE_INIT( yamato )
{
	int i;

	/* chars - 12 bits RGB */
	for (i = 0; i < 0x40; i++)
	{
		int bit0, bit1, bit2, bit3;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i + 0x00] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x00] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x00] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x00] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* green component */
		bit0 = (color_prom[i + 0x00] >> 4) & 0x01;
		bit1 = (color_prom[i + 0x00] >> 5) & 0x01;
		bit2 = (color_prom[i + 0x00] >> 6) & 0x01;
		bit3 = (color_prom[i + 0x00] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* blue component */
		bit0 = (color_prom[i + 0x40] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x40] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x40] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x40] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}

	/* big sprite - 8 bits RGB */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i + 0x80] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x80] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x80] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i + 0x80] >> 3) & 0x01;
		bit1 = (color_prom[i + 0x80] >> 4) & 0x01;
		bit2 = (color_prom[i + 0x80] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i + 0x80] >> 6) & 0x01;
		bit2 = (color_prom[i + 0x80] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i + 0x40, MAKE_RGB(r, g, b));
	}

	/* fake colors for bg gradient */
	for (i = 0; i < 0x100; i++)
		palette_set_color(machine, i + 0x60, MAKE_RGB(0, 0, i));
}

static PALETTE_INIT( toprollr )
{
	int i;

	for (i = 0; i < 32*5; i++)
	{
		int bit0,bit1,bit2;
		int r,g,b;

		/* red component */
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[0] >> 3) & 0x01;
		bit1 = (color_prom[0] >> 4) & 0x01;
		bit2 = (color_prom[0] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[0] >> 6) & 0x01;
		bit2 = (color_prom[0] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
		color_prom++;
	}

}


static int p0,p1;

static WRITE8_HANDLER( p0_w )
{
	p0 = data;
}
static WRITE8_HANDLER( p1_w )
{
	p1 = data;
}
static READ8_HANDLER( p0_r )
{
	return p0;
}
static READ8_HANDLER( p1_r )
{
	return p1;
}

static WRITE8_HANDLER( flip_screen_x_w )
{
	flip_screen_x_set(data);
}

static WRITE8_HANDLER( flip_screen_y_w )
{
	flip_screen_y_set(data);
}


static ADDRESS_MAP_START( yamato_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x6000, 0x6fff) AM_READ(MRA8_RAM)
	AM_RANGE(0x7000, 0x7fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x8800, 0x8bff) AM_READ(MRA8_RAM)
	AM_RANGE(0x9000, 0x93ff) AM_READ(MRA8_RAM)	/* video RAM */
	AM_RANGE(0x9800, 0x9bff) AM_READ(MRA8_RAM)	/* column scroll registers */
	AM_RANGE(0x9c00, 0x9fff) AM_READ(MRA8_RAM)	/* color RAM */
	AM_RANGE(0xa000, 0xa000) AM_READ(input_port_0_r)     /* IN0 */
	AM_RANGE(0xa800, 0xa800) AM_READ(input_port_1_r)     /* IN1 */
	AM_RANGE(0xb000, 0xb000) AM_READ(input_port_2_r)     /* DSW */
	AM_RANGE(0xb800, 0xb800) AM_READ(input_port_3_r)     /* IN2 */
	AM_RANGE(0xba00, 0xba00) AM_READ(input_port_4_r)     /* IN3 (maybe a mirror of b800) */
ADDRESS_MAP_END

static ADDRESS_MAP_START( yamato_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x6000, 0x6fff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x7000, 0x7fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x8800, 0x88ff) AM_WRITE(MWA8_RAM) AM_BASE(&cclimber_bsvideoram) AM_SIZE(&cclimber_bsvideoram_size)
	AM_RANGE(0x8900, 0x8bff) AM_WRITE(MWA8_RAM)  /* not used, but initialized */
	AM_RANGE(0x9000, 0x93ff) AM_WRITE(MWA8_RAM) AM_BASE(&videoram) AM_SIZE(&videoram_size)
//AM_RANGE(0x9400, 0x97ff) AM_WRITE(MWA8_RAM) /* mirror address, used by Crazy Climber to draw windows */
	/* 9800-9bff and 9c00-9fff share the same RAM, interleaved */
	/* (9800-981f for scroll, 9c20-9c3f for color RAM, and so on) */
	AM_RANGE(0x9800, 0x981f) AM_WRITE(MWA8_RAM) AM_BASE(&cclimber_column_scroll)
	AM_RANGE(0x9880, 0x989f) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x98dc, 0x98df) AM_WRITE(MWA8_RAM) AM_BASE(&cclimber_bigspriteram)
	AM_RANGE(0x9800, 0x9bff) AM_WRITE(MWA8_RAM)  /* not used, but initialized */
	AM_RANGE(0x9c00, 0x9fff) AM_WRITE(cclimber_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0xa000, 0xa000) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0xa001, 0xa001) AM_WRITE(flip_screen_x_w)
	AM_RANGE(0xa002, 0xa002) AM_WRITE(flip_screen_y_w)
//AM_RANGE(0xa004, 0xa004) AM_WRITE(cclimber_sample_trigger_w)
//AM_RANGE(0xa800, 0xa800) AM_WRITE(cclimber_sample_rate_w)
//AM_RANGE(0xb000, 0xb000) AM_WRITE(cclimber_sample_volume_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( yamato_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_WRITE(p0_w)	/* ??? */
	AM_RANGE(0x01, 0x01) AM_WRITE(p1_w)	/* ??? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( yamato_sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_READ(MRA8_ROM)
	AM_RANGE(0x5000, 0x53ff) AM_READ(MRA8_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( yamato_sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x5000, 0x53ff) AM_WRITE(MWA8_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( yamato_sound_readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x04, 0x04) AM_READ(p0_r)	/* ??? */
	AM_RANGE(0x08, 0x08) AM_READ(p1_r)	/* ??? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( yamato_sound_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(AY8910_control_port_1_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(AY8910_write_port_1_w)
ADDRESS_MAP_END

/* Top Roller */
static int jaleco_rombank=0;

static WRITE8_HANDLER(rombank_w)
{
	jaleco_rombank&=~(1<<offset);
	jaleco_rombank|=(data&1)<<offset;

	if(jaleco_rombank<3)
	{
		memory_set_bank(1, jaleco_rombank);
	}
}

static ADDRESS_MAP_START( toprollr_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_READ(MRA8_BANK1)
	AM_RANGE(0x6000, 0x6bff) AM_RAM
	AM_RANGE(0x8800, 0x88ff) AM_RAM AM_BASE(&cclimber_bsvideoram) AM_SIZE(&cclimber_bsvideoram_size)
	AM_RANGE(0x8c00, 0x8fff) AM_RAM AM_BASE(&toprollr_videoram3)
	AM_RANGE(0x9000, 0x93ff) AM_RAM AM_BASE(&videoram)
	AM_RANGE(0x9400, 0x97ff) AM_RAM AM_BASE(&toprollr_videoram4)
	AM_RANGE(0x9800, 0x987f) AM_RAM /* unused ? */
	AM_RANGE(0x9880, 0x995f) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x99dc, 0x99df) AM_RAM AM_BASE(&cclimber_bigspriteram)
	AM_RANGE(0x9c00, 0x9fff) AM_RAM AM_BASE(&toprollr_videoram2)
	AM_RANGE(0xa000, 0xa000) AM_WRITE(interrupt_enable_w) AM_READ(input_port_0_r)     /* IN0 */
	AM_RANGE(0xa001, 0xa001) AM_WRITE(flip_screen_x_w)
	AM_RANGE(0xa002, 0xa002) AM_WRITE(flip_screen_y_w)
	AM_RANGE(0xa004, 0xa004) AM_WRITE(cclimber_sample_trigger_w)
	AM_RANGE(0xa005, 0xa006) AM_WRITE(rombank_w)
	AM_RANGE(0xa800, 0xa800) AM_READ(input_port_1_r)  AM_WRITE(cclimber_sample_rate_w)   /* IN1 */
	AM_RANGE(0xb000, 0xb000) AM_READ(input_port_2_r)  AM_WRITE(cclimber_sample_volume_w)   /* DSW */
	AM_RANGE(0xb800, 0xb800) AM_READ(input_port_3_r)     /* IN2 */
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( toprollr_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x08, 0x08) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x09, 0x09) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x0d, 0x0d) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( yamato )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY

	PORT_START      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL

	PORT_START      /* DSW */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "Every 30000" )
	PORT_DIPSETTING(    0x20, "Every 50000" )
	PORT_DIPNAME( 0x40, 0x00, "Speed" )
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x40, "Fast" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START      /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )	/* set 1 only */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN3 )	/* set 1 only */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START      /* IN3 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( toprollr )
	PORT_START      /* IN0 */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY

	PORT_START      /* IN1 */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL

	PORT_START      /* DSW */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "Every 30000" )
	PORT_DIPSETTING(    0x20, "Every 50000" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START      /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )


INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	512,    /* 512 characters (256 in Crazy Climber) */
	2,      /* 2 bits per pixel */
	{ 0, 512*8*8 }, /* the two bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },     /* pretty straightforward layout */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     /* every char takes 8 consecutive bytes */
};
static const gfx_layout bscharlayout =
{
	8,8,    /* 8*8 characters */
	512,//256,    /* 256 characters */


	2,      /* 2 bits per pixel */
	{ 0, 512*8*8 }, /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },     /* pretty straightforward layout */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     /* every char takes 8 consecutive bytes */
};
static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	128,    /* 128 sprites (64 in Crazy Climber) */
	2,      /* 2 bits per pixel */
	{ 0, 128*16*16 },       /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,       /* pretty straightforward layout */
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8    /* every sprite takes 32 consecutive bytes */
};


static const gfx_layout toprollr_charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(1,2),0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout toprollr_spritelayout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,2),    /* 128 sprites (64 in Crazy Climber) */
	2,      /* 2 bits per pixel */
	{  RGN_FRAC(1,2),0 },       /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,       /* pretty straightforward layout */
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8    /* every sprite takes 32 consecutive bytes */
};


static GFXDECODE_START( yamato )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, charlayout,      0, 16 ) /* char set #1 */
	GFXDECODE_ENTRY( REGION_GFX1, 0x2000, charlayout,      0, 16 ) /* char set #2 */
	GFXDECODE_ENTRY( REGION_GFX2, 0x0000, bscharlayout, 16*4,  8 ) /* big sprite char set */
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, spritelayout,    0, 16 ) /* sprite set #1 */
	GFXDECODE_ENTRY( REGION_GFX1, 0x2000, spritelayout,    0, 16 ) /* sprite set #2 */
GFXDECODE_END

static GFXDECODE_START( toprollr )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, toprollr_charlayout,      0, 40 )
	GFXDECODE_ENTRY( REGION_GFX2, 0x0000, toprollr_charlayout,      0, 40 )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, toprollr_spritelayout,    0, 40 )
	GFXDECODE_ENTRY( REGION_GFX3, 0x0000, toprollr_charlayout,   	0, 40 )
GFXDECODE_END

static MACHINE_DRIVER_START( yamato )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, 3072000)	/* 3.072 MHz ? */
	MDRV_CPU_PROGRAM_MAP(yamato_readmem,yamato_writemem)
	MDRV_CPU_IO_MAP(0,yamato_writeport)
	MDRV_CPU_VBLANK_INT(nmi_line_pulse,1)

	MDRV_CPU_ADD(Z80, 3072000)
	/* audio CPU */	/* 3.072 MHz ? */
	MDRV_CPU_PROGRAM_MAP(yamato_sound_readmem,yamato_sound_writemem)
	MDRV_CPU_IO_MAP(yamato_sound_readport,yamato_sound_writeport)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(yamato)
	MDRV_PALETTE_LENGTH(96+256)

	MDRV_PALETTE_INIT(yamato)
	MDRV_VIDEO_START(generic)
	MDRV_VIDEO_UPDATE(yamato)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 1536000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD(AY8910, 1536000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( toprollr )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, 3072000)
	MDRV_CPU_PROGRAM_MAP(toprollr_map,0)
	MDRV_CPU_IO_MAP(toprollr_io_map,0)
	MDRV_CPU_VBLANK_INT(nmi_line_pulse,1)

	/* video hardware */


	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(toprollr)
	MDRV_PALETTE_LENGTH(32*5)
	MDRV_PALETTE_INIT(toprollr)

	MDRV_VIDEO_START(toprollr)
	MDRV_VIDEO_UPDATE(toprollr)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 1536000)
	MDRV_SOUND_CONFIG(cclimber_ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)

	MDRV_SOUND_ADD(SAMPLES, 0)
	MDRV_SOUND_CONFIG(cclimber_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_DRIVER_END


ROM_START( yamato )
	ROM_REGION( 2*0x10000, REGION_CPU1, 0 )	/* 64k for code + 64k for decrypted opcodes */
	ROM_LOAD( "2.5de",        0x0000, 0x2000, CRC(20895096) SHA1(af76786e3c519e710899f143d46c53087e9817c7) )
	ROM_LOAD( "3.5f",         0x2000, 0x2000, CRC(57a696f9) SHA1(28ea80fb100ac92295fc3eb318617d7cb014408d) )
	ROM_LOAD( "4.5jh",        0x4000, 0x2000, CRC(59a468e8) SHA1(a79cdee6efefd87a356cc8d710f8050bc12e07c3) )
	/* hole at 6000-6fff */
	ROM_LOAD( "11.5a",        0x7000, 0x1000, CRC(35987485) SHA1(1f0cb545bbd52982cbf801bc1dd2c4087af2f5f7) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "1.5v",         0x0000, 0x0800, CRC(3aad9e3c) SHA1(37b0414b265397881bb45b166ecab85880d1358d) )

	ROM_REGION( 0x4000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "10.11k",       0x0000, 0x1000, CRC(161121f5) SHA1(017c5c6b773b0ae1d0be52e4bac90b699ea196dd) )
	ROM_CONTINUE(             0x2000, 0x1000 )
	ROM_LOAD( "9.11h",        0x1000, 0x1000, CRC(56e84cc4) SHA1(c48e0e5460376d6b34173c42a27907ef12218182) )
	ROM_CONTINUE(             0x3000, 0x1000 )

	ROM_REGION( 0x2000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "8.11c",        0x0000, 0x1000, CRC(28024d9a) SHA1(c871c4d74be72a8bfea99e89d43f91922f4b734b) )
	ROM_LOAD( "7.11a",        0x1000, 0x1000, CRC(4a179790) SHA1(7fb6b033de939ff8bd13055c073311dca2c1a6fe) )

	ROM_REGION( 0x2000, REGION_USER1, 0 )
	ROM_LOAD( "5.5lm",        0x0000, 0x1000, CRC(7761ad24) SHA1(98878b19addd142d35718080eece05eaaee0388d) )	/* ?? */
	ROM_LOAD( "6.5n",         0x1000, 0x1000, CRC(da48444c) SHA1(a43e672ce262eb817fb4e5715ef4fb304a6a2815) )

	ROM_REGION( 0x00a0, REGION_PROMS, 0 )
	ROM_LOAD( "1.bpr",        0x0000, 0x0020, CRC(ef2053ab) SHA1(2006cbf003f90a8e75f39047a88a3bba85d78e80) )
	ROM_LOAD( "2.bpr",        0x0020, 0x0020, CRC(2281d39f) SHA1(e9b568bdacf7ab611801cf42ea5c7624f5440ef6) )
	ROM_LOAD( "3.bpr",        0x0040, 0x0020, CRC(9e6341e3) SHA1(2e7a4d3c1f40d6089735734b9d9de2ca57fb73c7) )
	ROM_LOAD( "4.bpr",        0x0060, 0x0020, CRC(1c97dc0b) SHA1(fe8e0a91172abdd2d14b199da144306a9b944372) )
	ROM_LOAD( "5.bpr",        0x0080, 0x0020, CRC(edd6c05f) SHA1(b95db8aaf74fe175d1179f0d85f79242b16f5fb4) )
ROM_END

ROM_START( yamato2 )
	ROM_REGION( 2*0x10000, REGION_CPU1, 0 )	/* 64k for code + 64k for decrypted opcodes */
	ROM_LOAD( "2-2.5de",      0x0000, 0x2000, CRC(93da1d52) SHA1(21b72856ebbd969e4e075b52719e6acdbd1bc4c5) )
	ROM_LOAD( "3-2.5f",       0x2000, 0x2000, CRC(31e73821) SHA1(e582c9fcea1b29d43f65b6aa67e1895c38d2736c) )
	ROM_LOAD( "4-2.5jh",      0x4000, 0x2000, CRC(fd7bcfc3) SHA1(5037170cb3a9824794e90d74def92b0b25d45caa) )
	/* hole at 6000-6fff */
	/* 7000-7fff not present here */

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "1.5v",         0x0000, 0x0800, CRC(3aad9e3c) SHA1(37b0414b265397881bb45b166ecab85880d1358d) )

	ROM_REGION( 0x4000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "10.11k",       0x0000, 0x1000, CRC(161121f5) SHA1(017c5c6b773b0ae1d0be52e4bac90b699ea196dd) )
	ROM_CONTINUE(             0x2000, 0x1000 )
	ROM_LOAD( "9.11h",        0x1000, 0x1000, CRC(56e84cc4) SHA1(c48e0e5460376d6b34173c42a27907ef12218182) )
	ROM_CONTINUE(             0x3000, 0x1000 )

	ROM_REGION( 0x2000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "8.11c",        0x0000, 0x1000, CRC(28024d9a) SHA1(c871c4d74be72a8bfea99e89d43f91922f4b734b) )
	ROM_LOAD( "7.11a",        0x1000, 0x1000, CRC(4a179790) SHA1(7fb6b033de939ff8bd13055c073311dca2c1a6fe) )

	ROM_REGION( 0x2000, REGION_USER1, 0 )
	ROM_LOAD( "5.5lm",        0x0000, 0x1000, CRC(7761ad24) SHA1(98878b19addd142d35718080eece05eaaee0388d) )	/* ?? */
	ROM_LOAD( "6.5n",         0x1000, 0x1000, CRC(da48444c) SHA1(a43e672ce262eb817fb4e5715ef4fb304a6a2815) )

	ROM_REGION( 0x00a0, REGION_PROMS, 0 )
	ROM_LOAD( "1.bpr",        0x0000, 0x0020, CRC(ef2053ab) SHA1(2006cbf003f90a8e75f39047a88a3bba85d78e80) )
	ROM_LOAD( "2.bpr",        0x0020, 0x0020, CRC(2281d39f) SHA1(e9b568bdacf7ab611801cf42ea5c7624f5440ef6) )
	ROM_LOAD( "3.bpr",        0x0040, 0x0020, CRC(9e6341e3) SHA1(2e7a4d3c1f40d6089735734b9d9de2ca57fb73c7) )
	ROM_LOAD( "4.bpr",        0x0060, 0x0020, CRC(1c97dc0b) SHA1(fe8e0a91172abdd2d14b199da144306a9b944372) )
	ROM_LOAD( "5.bpr",        0x0080, 0x0020, CRC(edd6c05f) SHA1(b95db8aaf74fe175d1179f0d85f79242b16f5fb4) )
ROM_END

ROM_START( toprollr )
	ROM_REGION( 0x10000*2, REGION_CPU1, 0 )
	ROM_LOAD( "10.k3", 0xc000, 0x2000, CRC(1e8914a6) SHA1(ec17f185f890d04ce75a5d8edf8b32da60e7a8d8) )
	ROM_LOAD( "11.l3", 0xe000, 0x2000, CRC(b20a9fa2) SHA1(accd3296447eca002b0808e7b02832f5e35407e8) )

	ROM_REGION( 0x04000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "15.h4", 0x0000, 0x2000, CRC(1d9e3325) SHA1(e7f6863aa2ba2aeec40cfcc5cf6c69e947c185b5) )
	ROM_LOAD( "16.j4", 0x2000, 0x2000, CRC(ce3afe26) SHA1(7de00720f091537c64cc0fec687c061de3a8b1a3) )

	ROM_REGION( 0x2000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "5.l5",  0x0000, 0x1000, CRC(84139f46) SHA1(976f781fb279dd540778708174b942a263f16443) )
	ROM_LOAD( "6.m5",  0x1000, 0x1000, CRC(e30c1dd8) SHA1(1777bf98625153c9b191020860e4e1839b46b998) )

	ROM_REGION( 0x04000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "13.a4", 0x0000, 0x2000, CRC(89327329) SHA1(555331a3136aa8c5bb35b97dd54bc59da067be57) )
	ROM_LOAD( "14.c4", 0x2000, 0x2000, CRC(7a945733) SHA1(14187ba303aecf0a812c425c34d8edda3deaa2b5) )

	ROM_REGION( 0x12000, REGION_USER1, 0 )
	ROM_LOAD( "2.f5",	0x00000, 0x02000, CRC(ef789f00) SHA1(424d69584d391ee7b9ad5db7ee6ced97d69897d4) )
	ROM_LOAD( "8.f3",	0x02000, 0x02000, CRC(94371cfb) SHA1(cb501c36b213c995a4048b3a96c85848c556cd05) )
	ROM_LOAD( "4.k5",	0x04000, 0x02000, CRC(1cb48ea0) SHA1(fdc75075112042ec84a7d1b3e5b5a6db1d1cb871) )
	ROM_COPY( REGION_USER1, 0x04000, 0x0a000, 0x02000 )
	ROM_COPY( REGION_USER1, 0x04000, 0x10000, 0x02000 )
	ROM_LOAD( "3.h5",	0x06000, 0x02000, CRC(d45494ba) SHA1(6e235b34f9457acadad6d4e27799978bc2e3db08) )
	ROM_LOAD( "9.h3",	0x08000, 0x02000, CRC(8a8032a7) SHA1(d6642d72645c613c21f65bbbe1560d0437d41f43) )
	ROM_LOAD( "1.d5",	0x0c000, 0x02000, CRC(9894374d) SHA1(173de4abbc3fb5d522aa6d6d5caf8e4d54f2a598) )
	ROM_LOAD( "7.d3",	0x0e000, 0x02000, CRC(904fffb6) SHA1(5528bc2a4d2fe8672428fd4725644265f0d57ded) )

	ROM_REGION( 0x2000, REGION_SOUND1, 0 )
	ROM_LOAD( "12.p3",  0x0000, 0x2000, CRC(7f989dc9) SHA1(3b4d18cbb992872b3cf8f5eaf5381ed3a9468cc1) )

	ROM_REGION( 0x01a0, REGION_PROMS, 0 )
	ROM_LOAD( "prom.a1",  0x0000, 0x0020, CRC(7d626d6c) SHA1(7c7202d0ec5bf0381e7104eef53afa5fa4596a29) ) //00-07 big sprites
	ROM_LOAD( "prom.p2",  0x0020, 0x0020, CRC(42e828fa) SHA1(81250b1f7c3956b3902324adbbaf3b5989e854ee) ) //08-0f sprites + fg (wrong?)
	ROM_LOAD( "prom.r2",  0x0040, 0x0020, CRC(99b87eed) SHA1(06c3164d681fe4aff0338c0dad1a921f7fe7369d) ) //10-17 sprites
	ROM_LOAD( "prom.p9",  0x0060, 0x0020, CRC(eb399c02) SHA1(bf3d6c6dd982cb54446cf8a010b7adb949514bdb) ) //18-1f bg
	ROM_LOAD( "prom.n9",  0x0080, 0x0020, CRC(fb03ea99) SHA1(4dcef86106cef713dfcbd965072bfa8fe4b68e15) ) //20-27 bg
	ROM_LOAD( "prom.s9",  0x00a0, 0x0100, CRC(abf4c5fb) SHA1(a953f14642d4b72328293b36bc3c65b13491ffff) ) //unknown prom (filled with 2 bit vals)

ROM_END

static DRIVER_INIT( yamato )
{
	yamato_decode();
}

static DRIVER_INIT( toprollr )
{
	toprollr_decode();
}

GAME( 1983, yamato,  0,      yamato, yamato, yamato, ROT90, "Sega", "Yamato (US)", GAME_IMPERFECT_GRAPHICS )
GAME( 1983, yamato2, yamato, yamato, yamato, yamato, ROT90, "Sega", "Yamato (World?)", GAME_IMPERFECT_GRAPHICS )
GAME( 1983, toprollr, 0, toprollr, toprollr, toprollr, ROT90, "Jaleco", "Top Roller", GAME_IMPERFECT_COLORS )
