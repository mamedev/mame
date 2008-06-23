/***************************************************************************

TODO:
- Find out how layers are enabled\disabled
- dangar input ports
- wrong title screen in ninjemak
- bit 3 of ninjemak_gfxbank_w, there currently is a kludge to clear text RAM
  but it should really copy stuff from the extra ROM.


Galivan
(C) 1985 Nihon Bussan

driver by

Luca Elia (l.elia@tin.it)
Olivier Galibert


Ninja Emaki (US)
(c)1986 NihonBussan Co.,Ltd.

Youma Ninpou Chou (Japan)
(c)1986 NihonBussan Co.,Ltd.

Driver by Takahiro Nogi (nogi@kt.rim.or.jp) 1999/12/17 -

***************************************************************************/

#include "driver.h"
#include "sound/dac.h"
#include "sound/3812intf.h"

WRITE8_HANDLER( galivan_scrollx_w );
WRITE8_HANDLER( galivan_scrolly_w );
WRITE8_HANDLER( galivan_videoram_w );
WRITE8_HANDLER( galivan_colorram_w );
WRITE8_HANDLER( galivan_gfxbank_w );
PALETTE_INIT( galivan );
VIDEO_START( galivan );
VIDEO_UPDATE( galivan );

WRITE8_HANDLER( ninjemak_scrollx_w );
WRITE8_HANDLER( ninjemak_scrolly_w );
WRITE8_HANDLER( ninjemak_gfxbank_w );
VIDEO_START( ninjemak );
VIDEO_UPDATE( ninjemak );



static MACHINE_RESET( galivan )
{
	UINT8 *RAM = memory_region(machine, REGION_CPU1);

	memory_set_bankptr(1,&RAM[0x10000]);
//  layers = 0x60;
}

static WRITE8_HANDLER( galivan_sound_command_w )
{
	soundlatch_w(machine,offset,(data << 1) | 1);
}

static READ8_HANDLER( galivan_sound_command_r )
{
	int data;

	data = soundlatch_r(machine,offset);
	soundlatch_clear_w(machine,0,0);
	return data;
}

static READ8_HANDLER( IO_port_c0_r )
{
  return (0x58); /* To Avoid Reset on Ufo Robot dangar */
}


/* the scroll registers are memory mapped in ninjemak, I/O ports in the others */
static WRITE8_HANDLER( ninjemak_videoreg_w )
{
	switch (offset)
	{
		case	0x0b:
			ninjemak_scrolly_w(machine, 0, data);
			break;
		case	0x0c:
			ninjemak_scrolly_w(machine, 1, data);
			break;
		case	0x0d:
			ninjemak_scrollx_w(machine, 0, data);
			break;
		case	0x0e:
			ninjemak_scrollx_w(machine, 1, data);
			break;
		default:
			break;
	}
}



static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_READ(SMH_ROM)
	AM_RANGE(0xc000, 0xdfff) AM_READ(SMH_BANK1)
	AM_RANGE(0xe000, 0xffff) AM_READ(SMH_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_WRITE(SMH_ROM)
	AM_RANGE(0xd800, 0xdbff) AM_WRITE(galivan_videoram_w) AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0xdc00, 0xdfff) AM_WRITE(galivan_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0xe000, 0xe0ff) AM_WRITE(SMH_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xe100, 0xffff) AM_WRITE(SMH_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ninjemak_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_WRITE(SMH_ROM)
	AM_RANGE(0xd800, 0xd81f) AM_WRITE(ninjemak_videoreg_w)
	AM_RANGE(0xd800, 0xdbff) AM_WRITE(galivan_videoram_w) AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0xdc00, 0xdfff) AM_WRITE(galivan_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0xe000, 0xe1ff) AM_WRITE(SMH_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xe200, 0xffff) AM_WRITE(SMH_RAM)
ADDRESS_MAP_END


static ADDRESS_MAP_START( readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(input_port_0_r)
	AM_RANGE(0x01, 0x01) AM_READ(input_port_1_r)
	AM_RANGE(0x02, 0x02) AM_READ(input_port_2_r)
	AM_RANGE(0x03, 0x03) AM_READ(input_port_3_r)
	AM_RANGE(0x04, 0x04) AM_READ(input_port_4_r)
	AM_RANGE(0xc0, 0xc0) AM_READ(IO_port_c0_r) /* dangar needs to return 0x58 */
ADDRESS_MAP_END

static ADDRESS_MAP_START( writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x40) AM_WRITE(galivan_gfxbank_w)
	AM_RANGE(0x41, 0x42) AM_WRITE(galivan_scrollx_w)
	AM_RANGE(0x43, 0x44) AM_WRITE(galivan_scrolly_w)
	AM_RANGE(0x45, 0x45) AM_WRITE(galivan_sound_command_w)
/*  AM_RANGE(0x46, 0x46) AM_WRITE(SMH_NOP) */
/*  AM_RANGE(0x47, 0x47) AM_WRITE(SMH_NOP) */
ADDRESS_MAP_END

static ADDRESS_MAP_START( ninjemak_readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x80) AM_READ(input_port_0_r)
	AM_RANGE(0x81, 0x81) AM_READ(input_port_1_r)
	AM_RANGE(0x82, 0x82) AM_READ(input_port_2_r)
	AM_RANGE(0x83, 0x83) AM_READ(input_port_3_r)
	AM_RANGE(0x84, 0x84) AM_READ(input_port_4_r)
	AM_RANGE(0x85, 0x85) AM_READ(input_port_5_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ninjemak_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x80) AM_WRITE(ninjemak_gfxbank_w)
	AM_RANGE(0x85, 0x85) AM_WRITE(galivan_sound_command_w)
//  AM_RANGE(0x86, 0x86) AM_WRITE(SMH_NOP)         // ??
//  AM_RANGE(0x87, 0x87) AM_WRITE(SMH_NOP)         // ??
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_READ(SMH_ROM)
	AM_RANGE(0xc000, 0xc7ff) AM_READ(SMH_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_WRITE(SMH_ROM)
	AM_RANGE(0xc000, 0xc7ff) AM_WRITE(SMH_RAM)
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
/*  AM_RANGE(0x04, 0x04) AM_READ(SMH_NOP)    value read and *discarded*    */
	AM_RANGE(0x06, 0x06) AM_READ(galivan_sound_command_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(YM3526_control_port_0_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(YM3526_write_port_0_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(DAC_0_data_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(DAC_1_data_w)
ADDRESS_MAP_END


/***************
   Dip Sitches
 ***************/

#define NIHON_JOYSTICK(_n_) \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(_n_) \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(_n_) \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(_n_) \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(_n_) \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(_n_)

#define NIHON_SYSTEM \
	PORT_START_TAG("IN2")  /* TEST, COIN, START */ \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 ) \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 ) \
	PORT_SERVICE_NO_TOGGLE(0x20, IP_ACTIVE_LOW)\
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

#define NIHON_COINAGE_A \
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) ) \
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

#define NIHON_COINAGE_B \
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) ) \
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) ) \
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )

#define NIHON_COINAGE_B_ALT \
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) ) \
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) ) \
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ) )

	/* This is how the Bonus Life are defined in Service Mode */
	/* However, to keep the way Bonus Life are defined in MAME, */
	/* below are the same values, but using the MAME way */
//  PORT_DIPNAME( 0x04, 0x04, "1st Bonus Life" )
//  PORT_DIPSETTING(    0x04, "20k" )
//  PORT_DIPSETTING(    0x00, "50k" )
//  PORT_DIPNAME( 0x08, 0x08, "2nd Bonus Life" )
//  PORT_DIPSETTING(    0x08, "every 60k" )
//  PORT_DIPSETTING(    0x00, "every 90k" )
#define NIHON_BONUS_LIFE \
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) ) \
	PORT_DIPSETTING(    0x0c, "20k and every 60k" ) \
	PORT_DIPSETTING(    0x08, "50k and every 60k" ) \
	PORT_DIPSETTING(    0x04, "20k and every 90k" ) \
	PORT_DIPSETTING(    0x00, "50k and every 90k" )

#define NIHON_LIVES \
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) ) \
	PORT_DIPSETTING(    0x03, "3" ) \
	PORT_DIPSETTING(    0x02, "4" ) \
	PORT_DIPSETTING(    0x01, "5" ) \
	PORT_DIPSETTING(    0x00, "6" )

static INPUT_PORTS_START( galivan )
	PORT_START_TAG("IN0")
	NIHON_JOYSTICK(1)
	PORT_START_TAG("IN1")
	NIHON_JOYSTICK(2)
	NIHON_SYSTEM

	PORT_START_TAG("DSW1")
	NIHON_LIVES
	NIHON_BONUS_LIFE
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, "Power Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Life Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW2")
	NIHON_COINAGE_A
	NIHON_COINAGE_B_ALT
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dangar )
	PORT_START_TAG("IN0")
	NIHON_JOYSTICK(1)
	PORT_START_TAG("IN1")
	NIHON_JOYSTICK(2)
	NIHON_SYSTEM

	PORT_START_TAG("DSW1")
	NIHON_LIVES
	NIHON_BONUS_LIFE
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Alternate Enemies")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW2")
	NIHON_COINAGE_A
	NIHON_COINAGE_B
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* two switches to allow continue... both work */
    	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Allow_Continue ) )
    	PORT_DIPSETTING(    0xc0, DEF_STR( No ) )
    	PORT_DIPSETTING(    0x80, "3 Times" )
    	PORT_DIPSETTING(    0x40, "5 Times" )
    	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

/* different Lives values and last different the last two dips */
static INPUT_PORTS_START( dangar2 )
	PORT_START_TAG("IN0")
	NIHON_JOYSTICK(1)
	PORT_START_TAG("IN1")
	NIHON_JOYSTICK(2)
	NIHON_SYSTEM

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	NIHON_BONUS_LIFE
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Alternate Enemies")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW2")
	NIHON_COINAGE_A
	NIHON_COINAGE_B
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Complete Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Base Ship Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/* the last two dip switches are different */
static INPUT_PORTS_START( dangarb )
	PORT_START_TAG("IN0")
	NIHON_JOYSTICK(1)
	PORT_START_TAG("IN1")
	NIHON_JOYSTICK(2)
	NIHON_SYSTEM

	PORT_START_TAG("DSW1")
	NIHON_LIVES
	NIHON_BONUS_LIFE
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Alternate Enemies")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW2")
	NIHON_COINAGE_A
	NIHON_COINAGE_B
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Complete Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Base Ship Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( ninjemak )
	PORT_START_TAG("IN0")
	NIHON_JOYSTICK(1)
	PORT_START_TAG("IN1")
	NIHON_JOYSTICK(2)
	NIHON_SYSTEM

	PORT_START_TAG("IN3")	/* IN3 - TEST */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW )
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
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("IN4")	/* IN4 - TEST */
	NIHON_LIVES
	NIHON_BONUS_LIFE
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_START_TAG("IN5")	/* IN5 - TEST */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Allow_Continue ) )
    	PORT_DIPSETTING(    0xc0, DEF_STR( No ) )
    	PORT_DIPSETTING(    0x80, "3 Times" )
    	PORT_DIPSETTING(    0x40, "5 Times" )
    	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END


#define CHARLAYOUT(NUM) static const gfx_layout charlayout_##NUM =  \
{																	\
	8,8,	/* 8*8 characters */									\
	NUM,	/* NUM characters */									\
	4,	/* 4 bits per pixel */										\
	{ 0, 1, 2, 3 },													\
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4 },						\
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },				\
	32*8	/* every char takes 32 consecutive bytes */				\
}

CHARLAYOUT(512);
CHARLAYOUT(1024);

static const gfx_layout tilelayout =
{
	16,16,
	1024,
	4,
	{ 0, 1, 2, 3 },
	{ 4,0,12,8,20,16,28,24,36,32,44,40,52,48,60,56 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
	  8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	16*16*4
};

#define SPRITELAYOUT(NUM) static const gfx_layout spritelayout_##NUM =  \
{																		\
	16,16,	/* 16*16 sprites */											\
	NUM,	/* NUM sprites */											\
	4,	/* 4 bits per pixel */											\
	{ 0, 1, 2, 3 },														\
	{ 1*4, 0*4, 1*4+NUM*64*8, 0*4+NUM*64*8, 3*4, 2*4, 3*4+NUM*64*8, 2*4+NUM*64*8,			\
			5*4, 4*4, 5*4+NUM*64*8, 4*4+NUM*64*8, 7*4, 6*4, 7*4+NUM*64*8, 6*4+NUM*64*8 },	\
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,					\
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },		\
	64*8	/* every sprite takes 64 consecutive bytes */				\
}

SPRITELAYOUT(512);
SPRITELAYOUT(1024);


static GFXDECODE_START( galivan )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout_512,            0,   8 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, tilelayout,             8*16,  16 )
	GFXDECODE_ENTRY( REGION_GFX3, 0, spritelayout_512, 8*16+16*16, 256 )
GFXDECODE_END

static GFXDECODE_START( ninjemak )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout_1024,            0,   8 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, tilelayout,              8*16,  16 )
	GFXDECODE_ENTRY( REGION_GFX3, 0, spritelayout_1024, 8*16+16*16, 256 )
GFXDECODE_END



static MACHINE_DRIVER_START( galivan )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80,12000000/2)		/* 6 MHz? */
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
	MDRV_CPU_IO_MAP(readport,writeport)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_CPU_ADD(Z80,8000000/2)
	/* audio CPU */		/* 4 MHz? */
	MDRV_CPU_PROGRAM_MAP(sound_readmem,sound_writemem)
	MDRV_CPU_IO_MAP(sound_readport,sound_writeport)
	MDRV_CPU_PERIODIC_INT(irq0_line_hold, 7250)  /* timed interrupt, ?? Hz */

	MDRV_MACHINE_RESET(galivan)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(galivan)
	MDRV_PALETTE_LENGTH(8*16+16*16+256*16)

	MDRV_PALETTE_INIT(galivan)
	MDRV_VIDEO_START(galivan)
	MDRV_VIDEO_UPDATE(galivan)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(YM3526, 8000000/2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( ninjemak )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80,12000000/2)		/* 6 MHz? */
	MDRV_CPU_PROGRAM_MAP(readmem,ninjemak_writemem)
	MDRV_CPU_IO_MAP(ninjemak_readport,ninjemak_writeport)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_CPU_ADD(Z80,8000000/2)
	/* audio CPU */		/* 4 MHz? */
	MDRV_CPU_PROGRAM_MAP(sound_readmem,sound_writemem)
	MDRV_CPU_IO_MAP(sound_readport,sound_writeport)
	MDRV_CPU_PERIODIC_INT(irq0_line_hold, 7250)	/* timed interrupt, ?? Hz */

	MDRV_MACHINE_RESET(galivan)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(ninjemak)
	MDRV_PALETTE_LENGTH(8*16+16*16+256*16)

	MDRV_PALETTE_INIT(galivan)
	MDRV_VIDEO_START(ninjemak)
	MDRV_VIDEO_UPDATE(ninjemak)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD(YM3526, 8000000/2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( galivan )
	ROM_REGION( 0x14000, REGION_CPU1, 0 )	/* main cpu code */
	ROM_LOAD( "gv1.1b",       0x00000, 0x8000, CRC(5e480bfc) SHA1(f444de27d3d8aff579cf196a25b7f0c906617172) )
	ROM_LOAD( "gv2.3b",       0x08000, 0x4000, CRC(0d1b3538) SHA1(aa1ee04ff3516e0121db0cf50cee849ba5058fd5) )
	ROM_LOAD( "gv3.4b",       0x10000, 0x4000, CRC(82f0c5e6) SHA1(77dd3927c2161e4fce9e0adba81dc0c875d7e2f4) ) /* 2 banks at c000 */

	ROM_REGION( 0x10000, REGION_CPU2, 0 )		/* sound cpu code */
	ROM_LOAD( "gv11.14b",     0x0000, 0x4000, CRC(05f1a0e3) SHA1(c0f579130d64123c889c77d8f2f474ebcc3ba649) )
	ROM_LOAD( "gv12.15b",     0x4000, 0x8000, CRC(5b7a0d6d) SHA1(0c15def9be8014aeb4e14b6967efe8f5abac51f2) )

	ROM_REGION( 0x04000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "gv4.13d",      0x00000, 0x4000, CRC(162490b4) SHA1(55592865f208bf1b8f49c8eedc22a3d91ca3578d) ) /* chars */

	ROM_REGION( 0x20000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "gv7.14f",      0x00000, 0x8000, CRC(eaa1a0db) SHA1(ed3b125a7472c0c0a458b28df6476cb4c64b4aa3) ) /* tiles */
	ROM_LOAD( "gv8.15f",      0x08000, 0x8000, CRC(f174a41e) SHA1(38aa7aa3d6ba026478d30b5e404614a0cc7aed52) )
	ROM_LOAD( "gv9.17f",      0x10000, 0x8000, CRC(edc60f5d) SHA1(c743f4af0e0e2c60f59fd01ce0a153108e9f5414) )
	ROM_LOAD( "gv10.19f",     0x18000, 0x8000, CRC(41f27fca) SHA1(3674dbecc2eb1c837159a8dfbb0086088631b2a5) )

	ROM_REGION( 0x10000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "gv14.4f",      0x00000, 0x8000, CRC(03e2229f) SHA1(9dace9e04867d1140eb3c794bd4ae54ec3bb4a83) ) /* sprites */
	ROM_LOAD( "gv13.1f",      0x08000, 0x8000, CRC(bca9e66b) SHA1(d84840943748a7b9fd6e141be9971431f69ce1f9) )

	ROM_REGION( 0x8000, REGION_GFX4, 0 )	/* background tilemaps */
	ROM_LOAD( "gv6.19d",      0x0000, 0x4000, CRC(da38168b) SHA1(a12decd55fd1cf32fd192f13bd33d2f1f4129d2c) )
	ROM_LOAD( "gv5.17d",      0x4000, 0x4000, CRC(22492d2a) SHA1(c8d36949abc2fcc8f2b12276eb82b330a940bc38) )

	ROM_REGION( 0x0400, REGION_PROMS, 0 )
	ROM_LOAD( "mb7114e.9f",   0x0000, 0x0100, CRC(de782b3e) SHA1(c76da7d5cbd9170be93c9591e525646a4360203c) )	/* red */
	ROM_LOAD( "mb7114e.10f",  0x0100, 0x0100, CRC(0ae2a857) SHA1(cdf84c0c75d483a81013dbc050e7aa8c8503c74c) )	/* green */
	ROM_LOAD( "mb7114e.11f",  0x0200, 0x0100, CRC(7ba8b9d1) SHA1(5942b403eda046e2f2584062443472cbf559db5c) )	/* blue */
	ROM_LOAD( "mb7114e.2d",   0x0300, 0x0100, CRC(75466109) SHA1(6196d12ab7103f6ef991b826d8b93303a61d4c48) )	/* sprite lookup table */

	ROM_REGION( 0x0100, REGION_USER1, 0 )
	ROM_LOAD( "mb7114e.7f",   0x0000, 0x0100, CRC(06538736) SHA1(a2fb2ecb768686839f3087e691102e2dc2eb65b5) )	/* sprite palette bank */
ROM_END

ROM_START( galivan2 )
	ROM_REGION( 0x14000, REGION_CPU1, 0 )		/* main cpu code */
	ROM_LOAD( "e-1",          0x00000, 0x8000, CRC(d8cc72b8) SHA1(73a46cd7dda3a912b14075b9b4ebc81a175a1461) )
	ROM_LOAD( "e-2",          0x08000, 0x4000, CRC(9e5b3157) SHA1(1aa5f7f382468af815c929c63866bd39e7a9ac18) )
	ROM_LOAD( "gv3.4b",       0x10000, 0x4000, CRC(82f0c5e6) SHA1(77dd3927c2161e4fce9e0adba81dc0c875d7e2f4) ) /* 2 banks at c000 */

	ROM_REGION( 0x10000, REGION_CPU2, 0 )		/* sound cpu code */
	ROM_LOAD( "gv11.14b",     0x0000, 0x4000, CRC(05f1a0e3) SHA1(c0f579130d64123c889c77d8f2f474ebcc3ba649) )
	ROM_LOAD( "gv12.15b",     0x4000, 0x8000, CRC(5b7a0d6d) SHA1(0c15def9be8014aeb4e14b6967efe8f5abac51f2) )

	ROM_REGION( 0x04000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "gv4.13d",      0x00000, 0x4000, CRC(162490b4) SHA1(55592865f208bf1b8f49c8eedc22a3d91ca3578d) ) /* chars */

	ROM_REGION( 0x20000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "gv7.14f",      0x00000, 0x8000, CRC(eaa1a0db) SHA1(ed3b125a7472c0c0a458b28df6476cb4c64b4aa3) ) /* tiles */
	ROM_LOAD( "gv8.15f",      0x08000, 0x8000, CRC(f174a41e) SHA1(38aa7aa3d6ba026478d30b5e404614a0cc7aed52) )
	ROM_LOAD( "gv9.17f",      0x10000, 0x8000, CRC(edc60f5d) SHA1(c743f4af0e0e2c60f59fd01ce0a153108e9f5414) )
	ROM_LOAD( "gv10.19f",     0x18000, 0x8000, CRC(41f27fca) SHA1(3674dbecc2eb1c837159a8dfbb0086088631b2a5) )

	ROM_REGION( 0x10000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "gv14.4f",      0x00000, 0x8000, CRC(03e2229f) SHA1(9dace9e04867d1140eb3c794bd4ae54ec3bb4a83) ) /* sprites */
	ROM_LOAD( "gv13.1f",      0x08000, 0x8000, CRC(bca9e66b) SHA1(d84840943748a7b9fd6e141be9971431f69ce1f9) )

	ROM_REGION( 0x8000, REGION_GFX4, 0 )	/* background tilemaps */
	ROM_LOAD( "gv6.19d",      0x0000, 0x4000, CRC(da38168b) SHA1(a12decd55fd1cf32fd192f13bd33d2f1f4129d2c) )
	ROM_LOAD( "gv5.17d",      0x4000, 0x4000, CRC(22492d2a) SHA1(c8d36949abc2fcc8f2b12276eb82b330a940bc38) )

	ROM_REGION( 0x0400, REGION_PROMS, 0 )
	ROM_LOAD( "mb7114e.9f",   0x0000, 0x0100, CRC(de782b3e) SHA1(c76da7d5cbd9170be93c9591e525646a4360203c) )	/* red */
	ROM_LOAD( "mb7114e.10f",  0x0100, 0x0100, CRC(0ae2a857) SHA1(cdf84c0c75d483a81013dbc050e7aa8c8503c74c) )	/* green */
	ROM_LOAD( "mb7114e.11f",  0x0200, 0x0100, CRC(7ba8b9d1) SHA1(5942b403eda046e2f2584062443472cbf559db5c) )	/* blue */
	ROM_LOAD( "mb7114e.2d",   0x0300, 0x0100, CRC(75466109) SHA1(6196d12ab7103f6ef991b826d8b93303a61d4c48) )	/* sprite lookup table */

	ROM_REGION( 0x0100, REGION_USER1, 0 )
	ROM_LOAD( "mb7114e.7f",   0x0000, 0x0100, CRC(06538736) SHA1(a2fb2ecb768686839f3087e691102e2dc2eb65b5) )	/* sprite palette bank */
ROM_END

ROM_START( dangar )
	ROM_REGION( 0x14000, REGION_CPU1, 0 )		/* main cpu code */
	ROM_LOAD( "dangar08.1b",  0x00000, 0x8000, CRC(e52638f2) SHA1(6dd3ccb4574a410abf1ac35b4f9518ee21ecac91) )
	ROM_LOAD( "dangar09.3b",  0x08000, 0x4000, CRC(809d280f) SHA1(931f811f1fe3c71ba82fc44f69ef461bdd9cd2d8) )
	ROM_LOAD( "dangar10.5b",  0x10000, 0x4000, CRC(99a3591b) SHA1(45011043ff5620524d79076542bd8c602fe90cf4) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )		/* sound cpu code */
	ROM_LOAD( "dangar13.b14", 0x0000, 0x4000, CRC(3e041873) SHA1(8f9e1ec64509c8a7e9e45add9efc95f98f35fcfc) )
	ROM_LOAD( "dangar14.b15", 0x4000, 0x8000, CRC(488e3463) SHA1(73ff7ab061be54162f3a548f6bd9ef55b9dec5d9) )

	ROM_REGION( 0x04000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "dangar05.13d", 0x00000, 0x4000, CRC(40cb378a) SHA1(764596f6845fc0b787b653a87a1778a56ce4f3f8) )	/* chars */

	ROM_REGION( 0x20000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "dangar01.14f", 0x00000, 0x8000, CRC(d59ed1f1) SHA1(e55314b5a078145ad7a5e95cb792b4fd32cfb05d) )  /* tiles */
	ROM_LOAD( "dangar02.15f", 0x08000, 0x8000, CRC(dfdb931c) SHA1(33563160239f221f24ca0cb652d14550e9941afe) )
	ROM_LOAD( "dangar03.17f", 0x10000, 0x8000, CRC(6954e8c3) SHA1(077bcbe9f80df011c9110d8cf6e08b53d035d1c8) )
	ROM_LOAD( "dangar04.19f", 0x18000, 0x8000, CRC(4af6a8bf) SHA1(d004b10b9b8559d1d6d26af35999df2857d87c53) )

	ROM_REGION( 0x10000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "dangarxx.f4",  0x00000, 0x8000, CRC(55711884) SHA1(2682ebc8d88d0d6c430b7df34ed362bc81047072) )  /* sprites */
	ROM_LOAD( "dangarxx.f1",  0x08000, 0x8000, CRC(8cf11419) SHA1(79e7a3046878724fde248100ad55a305a427cd46) )

	ROM_REGION( 0x8000, REGION_GFX4, 0 )	/* background tilemaps */
	ROM_LOAD( "dangar07.19d", 0x0000, 0x4000, CRC(6dba32cf) SHA1(e6433f291364202c1291b137d6ee1840ecf7d72d) )
	ROM_LOAD( "dangar06.17d", 0x4000, 0x4000, CRC(6c899071) SHA1(9a776aae897d57e66ebdbcf79f3c673da8b78b05) )

	ROM_REGION( 0x0400, REGION_PROMS, 0 )
	ROM_LOAD( "82s129.9f",    0x0000, 0x0100, CRC(b29f6a07) SHA1(17c82f439f314c212470bafd917b3f7e12462d16) )	/* red */
	ROM_LOAD( "82s129.10f",   0x0100, 0x0100, CRC(c6de5ecb) SHA1(d5b6cb784b5df16332c5e2b19b763c8858a0b6a7) )	/* green */
	ROM_LOAD( "82s129.11f",   0x0200, 0x0100, CRC(a5bbd6dc) SHA1(5587844900a24d833500d204f049c05493c4a25a) )	/* blue */
	ROM_LOAD( "82s129.2d",    0x0300, 0x0100, CRC(a4ac95a5) SHA1(3b31cd3fd6caedd89d1bedc606a978081fc5431f) )	/* sprite lookup table */

	ROM_REGION( 0x0100, REGION_USER1, 0 )
	ROM_LOAD( "82s129.7f",    0x0000, 0x0100, CRC(29bc6216) SHA1(1d7864ad06ad0cd5e3d1905fc6066bee1cd90995) )	/* sprite palette bank */
ROM_END

ROM_START( dangar2 )
	ROM_REGION( 0x14000, REGION_CPU1, 0 )		/* main cpu code */
	ROM_LOAD( "dangar2.016",  0x00000, 0x8000, CRC(743fa2d4) SHA1(55539796967532b57279801374b2f0cf82cfe1ae) )
	ROM_LOAD( "dangar2.017",  0x08000, 0x4000, CRC(1cdc60a5) SHA1(65f776d14c9461f1a6939ad512eacf6a1a9da2c6) )
	ROM_LOAD( "dangar2.018",  0x10000, 0x4000, CRC(db7f6613) SHA1(c55d1f2fdb86e2b9fbdfad0b156d4d084677b750) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )		/* sound cpu code */
	ROM_LOAD( "dangar13.b14", 0x0000, 0x4000, CRC(3e041873) SHA1(8f9e1ec64509c8a7e9e45add9efc95f98f35fcfc) )
	ROM_LOAD( "dangar14.b15", 0x4000, 0x8000, CRC(488e3463) SHA1(73ff7ab061be54162f3a548f6bd9ef55b9dec5d9) )

	ROM_REGION( 0x04000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "dangar2.011",  0x00000, 0x4000, CRC(e804ffe1) SHA1(22f16c23b9a82f104dda24bc8fccc08f3f69cf97) )	/* chars */

	ROM_REGION( 0x20000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "dangar01.14f", 0x00000, 0x8000, CRC(d59ed1f1) SHA1(e55314b5a078145ad7a5e95cb792b4fd32cfb05d) )  /* tiles */
	ROM_LOAD( "dangar02.15f", 0x08000, 0x8000, CRC(dfdb931c) SHA1(33563160239f221f24ca0cb652d14550e9941afe) )
	ROM_LOAD( "dangar03.17f", 0x10000, 0x8000, CRC(6954e8c3) SHA1(077bcbe9f80df011c9110d8cf6e08b53d035d1c8) )
	ROM_LOAD( "dangar04.19f", 0x18000, 0x8000, CRC(4af6a8bf) SHA1(d004b10b9b8559d1d6d26af35999df2857d87c53) )

	ROM_REGION( 0x10000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "dangarxx.f4",  0x00000, 0x8000, CRC(55711884) SHA1(2682ebc8d88d0d6c430b7df34ed362bc81047072) )  /* sprites */
	ROM_LOAD( "dangarxx.f1",  0x08000, 0x8000, CRC(8cf11419) SHA1(79e7a3046878724fde248100ad55a305a427cd46) )

	ROM_REGION( 0x8000, REGION_GFX4, 0 )	/* background tilemaps */
	ROM_LOAD( "dangar07.19d", 0x0000, 0x4000, CRC(6dba32cf) SHA1(e6433f291364202c1291b137d6ee1840ecf7d72d) )
	ROM_LOAD( "dangar06.17d", 0x4000, 0x4000, CRC(6c899071) SHA1(9a776aae897d57e66ebdbcf79f3c673da8b78b05) )

	ROM_REGION( 0x0400, REGION_PROMS, 0 )
	ROM_LOAD( "82s129.9f",    0x0000, 0x0100, CRC(b29f6a07) SHA1(17c82f439f314c212470bafd917b3f7e12462d16) )	/* red */
	ROM_LOAD( "82s129.10f",   0x0100, 0x0100, CRC(c6de5ecb) SHA1(d5b6cb784b5df16332c5e2b19b763c8858a0b6a7) )	/* green */
	ROM_LOAD( "82s129.11f",   0x0200, 0x0100, CRC(a5bbd6dc) SHA1(5587844900a24d833500d204f049c05493c4a25a) )	/* blue */
	ROM_LOAD( "82s129.2d",    0x0300, 0x0100, CRC(a4ac95a5) SHA1(3b31cd3fd6caedd89d1bedc606a978081fc5431f) )	/* sprite lookup table */

	ROM_REGION( 0x0100, REGION_USER1, 0 )
	ROM_LOAD( "82s129.7f",    0x0000, 0x0100, CRC(29bc6216) SHA1(1d7864ad06ad0cd5e3d1905fc6066bee1cd90995) )	/* sprite palette bank */
ROM_END

ROM_START( dangarb )
	ROM_REGION( 0x14000, REGION_CPU1, 0 )		/* main cpu code */
	ROM_LOAD( "8",            0x00000, 0x8000, CRC(8136fd10) SHA1(5f2ca08fab0d9431af38ef66922fdb6bd9a132e2) )
	ROM_LOAD( "9",            0x08000, 0x4000, CRC(3ce5ec11) SHA1(bcc0df6167d0b84b9f260435c1999b9d3605fcd4) )
	ROM_LOAD( "dangar2.018",  0x10000, 0x4000, CRC(db7f6613) SHA1(c55d1f2fdb86e2b9fbdfad0b156d4d084677b750) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )		/* sound cpu code */
	ROM_LOAD( "dangar13.b14", 0x0000, 0x4000, CRC(3e041873) SHA1(8f9e1ec64509c8a7e9e45add9efc95f98f35fcfc) )
	ROM_LOAD( "dangar14.b15", 0x4000, 0x8000, CRC(488e3463) SHA1(73ff7ab061be54162f3a548f6bd9ef55b9dec5d9) )

	ROM_REGION( 0x04000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "dangar2.011",  0x00000, 0x4000, CRC(e804ffe1) SHA1(22f16c23b9a82f104dda24bc8fccc08f3f69cf97) )	/* chars */

	ROM_REGION( 0x20000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "dangar01.14f", 0x00000, 0x8000, CRC(d59ed1f1) SHA1(e55314b5a078145ad7a5e95cb792b4fd32cfb05d) )  /* tiles */
	ROM_LOAD( "dangar02.15f", 0x08000, 0x8000, CRC(dfdb931c) SHA1(33563160239f221f24ca0cb652d14550e9941afe) )
	ROM_LOAD( "dangar03.17f", 0x10000, 0x8000, CRC(6954e8c3) SHA1(077bcbe9f80df011c9110d8cf6e08b53d035d1c8) )
	ROM_LOAD( "dangar04.19f", 0x18000, 0x8000, CRC(4af6a8bf) SHA1(d004b10b9b8559d1d6d26af35999df2857d87c53) )

	ROM_REGION( 0x10000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "dangarxx.f4",  0x00000, 0x8000, CRC(55711884) SHA1(2682ebc8d88d0d6c430b7df34ed362bc81047072) )  /* sprites */
	ROM_LOAD( "dangarxx.f1",  0x08000, 0x8000, CRC(8cf11419) SHA1(79e7a3046878724fde248100ad55a305a427cd46) )

	ROM_REGION( 0x8000, REGION_GFX4, 0 )	/* background tilemaps */
	ROM_LOAD( "dangar07.19d", 0x0000, 0x4000, CRC(6dba32cf) SHA1(e6433f291364202c1291b137d6ee1840ecf7d72d) )
	ROM_LOAD( "dangar06.17d", 0x4000, 0x4000, CRC(6c899071) SHA1(9a776aae897d57e66ebdbcf79f3c673da8b78b05) )

	ROM_REGION( 0x0400, REGION_PROMS, 0 )
	ROM_LOAD( "82s129.9f",    0x0000, 0x0100, CRC(b29f6a07) SHA1(17c82f439f314c212470bafd917b3f7e12462d16) )	/* red */
	ROM_LOAD( "82s129.10f",   0x0100, 0x0100, CRC(c6de5ecb) SHA1(d5b6cb784b5df16332c5e2b19b763c8858a0b6a7) )	/* green */
	ROM_LOAD( "82s129.11f",   0x0200, 0x0100, CRC(a5bbd6dc) SHA1(5587844900a24d833500d204f049c05493c4a25a) )	/* blue */
	ROM_LOAD( "82s129.2d",    0x0300, 0x0100, CRC(a4ac95a5) SHA1(3b31cd3fd6caedd89d1bedc606a978081fc5431f) )	/* sprite lookup table */

	ROM_REGION( 0x0100, REGION_USER1, 0 )
	ROM_LOAD( "82s129.7f",    0x0000, 0x0100, CRC(29bc6216) SHA1(1d7864ad06ad0cd5e3d1905fc6066bee1cd90995) )	/* sprite palette bank */
ROM_END

ROM_START( ninjemak )
	ROM_REGION( 0x18000, REGION_CPU1, 0 )	/* main cpu code */
	ROM_LOAD( "ninjemak.1",   0x00000, 0x8000, CRC(12b0a619) SHA1(7b42097be6423931256d5b7fdafb98bee1b42e64) )
	ROM_LOAD( "ninjemak.2",   0x08000, 0x4000, CRC(d5b505d1) SHA1(53935549754e8a71f0620630c2e59c21d52edcba) )
	ROM_LOAD( "ninjemak.3",   0x10000, 0x8000, CRC(68c92bf6) SHA1(90633622dab0e450a29230b600e0d60a42f407f4) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* sound cpu code */
	ROM_LOAD( "ninjemak.12",  0x0000, 0x4000, CRC(3d1cd329) SHA1(6abd8e0dbecddfd67c4d358b958c850136fd3c29) )
	ROM_LOAD( "ninjemak.13",  0x4000, 0x8000, CRC(ac3a0b81) SHA1(39f2c305706e313d5256c357a3c8b57bbe45d3d7) )

	ROM_REGION( 0x08000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "ninjemak.4",   0x00000, 0x8000, CRC(83702c37) SHA1(c063288cf74dee74005c6d0dea57e9ec3adebc83) )	/* chars */

	ROM_REGION( 0x20000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "ninjemak.8",   0x00000, 0x8000, CRC(655f0a58) SHA1(8ffe73cec68d52c7b09651b546289613d6d4dde4) ) /* tiles */
	ROM_LOAD( "ninjemak.9",   0x08000, 0x8000, CRC(934e1703) SHA1(451f8d01d9035d91c969cdc3fb582a00007da7df) )
	ROM_LOAD( "ninjemak.10",  0x10000, 0x8000, CRC(955b5c45) SHA1(936bfe2599228dd0861bbcfe15152ac5e9b906d1) )
	ROM_LOAD( "ninjemak.11",  0x18000, 0x8000, CRC(bbd2e51c) SHA1(51bc266cf8161610204e5d98e56346b1d8d3c009) )

	ROM_REGION( 0x20000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "ninjemak.16",  0x00000, 0x8000, CRC(8df93fed) SHA1(ef37c78d4abbdbe9f427e3d9345f52464261116d) )  /* sprites */
	ROM_LOAD( "ninjemak.17",  0x08000, 0x8000, CRC(a3efd0fc) SHA1(69d40707b0570c2f1be6247f0209ba9e60a83ed0) )
	ROM_LOAD( "ninjemak.14",  0x10000, 0x8000, CRC(bff332d3) SHA1(d277ba18034b083eaafa969d90685563994416fa) )
	ROM_LOAD( "ninjemak.15",  0x18000, 0x8000, CRC(56430ed4) SHA1(68356a0f68404ef70d8dc17d5cbdf5e1f28badcf) )

	ROM_REGION( 0x8000, REGION_GFX4, 0 )	/* background tilemaps */
	ROM_LOAD( "ninjemak.7",   0x0000, 0x4000, CRC(80c20d36) SHA1(f20724754824030d62059388f3ea2224f5b7a60e) )
	ROM_LOAD( "ninjemak.6",   0x4000, 0x4000, CRC(1da7a651) SHA1(5307452058164a0bc39d144dd204627a9ead7543) )

	ROM_REGION( 0x4000, REGION_GFX5, 0 )	/* data for mcu/blitter? */
	ROM_LOAD( "ninjemak.5",   0x0000, 0x4000, CRC(5f91dd30) SHA1(3513c0a2e4ca83f602cacad6af9c07fe9e4b16a1) )	/* text layer data */

	ROM_REGION( 0x0400, REGION_PROMS, 0 )	/* Region 3 - color data */
	ROM_LOAD( "ninjemak.pr1", 0x0000, 0x0100, CRC(8a62d4e4) SHA1(99ca4da01ea1b5585f6e3ebf162c3f988ab317e5) )	/* red */
	ROM_LOAD( "ninjemak.pr2", 0x0100, 0x0100, CRC(2ccf976f) SHA1(b804ee761793697087fbe3372352f301a22feeab) )	/* green */
	ROM_LOAD( "ninjemak.pr3", 0x0200, 0x0100, CRC(16b2a7a4) SHA1(53c410b439c8a835447f15f2ab250b363b3f7888) )	/* blue */
	ROM_LOAD( "yncp-2d.bin",  0x0300, 0x0100, BAD_DUMP CRC(23bade78) SHA1(7e2de5eb08d888f97830807b6dbe85d09bb3b7f8)  )	/* sprite lookup table */

	ROM_REGION( 0x0100, REGION_USER1, 0 )
	ROM_LOAD( "yncp-7f.bin",  0x0000, 0x0100, BAD_DUMP CRC(262d0809) SHA1(a67281af02cef082023c0d7d57e3824aeef67450)  )	/* sprite palette bank */
ROM_END

ROM_START( youma )
	ROM_REGION( 0x18000, REGION_CPU1, 0 )	/* main cpu code */
	ROM_LOAD( "ync-1.bin",    0x00000, 0x8000, CRC(0552adab) SHA1(183cf88d288875fbb2b60e2712e5a1671511351d) )
	ROM_LOAD( "ync-2.bin",    0x08000, 0x4000, CRC(f961e5e6) SHA1(cbf9d3a256937da9e17734f89652e049242910b8) )
	ROM_LOAD( "ync-3.bin",    0x10000, 0x8000, CRC(9ad50a5e) SHA1(2532b10e2468b1c74440fd8090489142e5fc240b) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* sound cpu code */
	ROM_LOAD( "ninjemak.12",  0x0000, 0x4000, CRC(3d1cd329) SHA1(6abd8e0dbecddfd67c4d358b958c850136fd3c29) )
	ROM_LOAD( "ninjemak.13",  0x4000, 0x8000, CRC(ac3a0b81) SHA1(39f2c305706e313d5256c357a3c8b57bbe45d3d7) )

	ROM_REGION( 0x08000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "ync-4.bin",    0x00000, 0x8000, CRC(a1954f44) SHA1(b10a22b51bd1a02c0d7b116b4d7390003c41decf) )	/* chars */

	ROM_REGION( 0x20000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "ninjemak.8",   0x00000, 0x8000, CRC(655f0a58) SHA1(8ffe73cec68d52c7b09651b546289613d6d4dde4) ) /* tiles */
	ROM_LOAD( "ninjemak.9",   0x08000, 0x8000, CRC(934e1703) SHA1(451f8d01d9035d91c969cdc3fb582a00007da7df) )
	ROM_LOAD( "ninjemak.10",  0x10000, 0x8000, CRC(955b5c45) SHA1(936bfe2599228dd0861bbcfe15152ac5e9b906d1) )
	ROM_LOAD( "ninjemak.11",  0x18000, 0x8000, CRC(bbd2e51c) SHA1(51bc266cf8161610204e5d98e56346b1d8d3c009) )

	ROM_REGION( 0x20000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "ninjemak.16",  0x00000, 0x8000, CRC(8df93fed) SHA1(ef37c78d4abbdbe9f427e3d9345f52464261116d) )  /* sprites */
	ROM_LOAD( "ninjemak.17",  0x08000, 0x8000, CRC(a3efd0fc) SHA1(69d40707b0570c2f1be6247f0209ba9e60a83ed0) )
	ROM_LOAD( "ninjemak.14",  0x10000, 0x8000, CRC(bff332d3) SHA1(d277ba18034b083eaafa969d90685563994416fa) )
	ROM_LOAD( "ninjemak.15",  0x18000, 0x8000, CRC(56430ed4) SHA1(68356a0f68404ef70d8dc17d5cbdf5e1f28badcf) )

	ROM_REGION( 0x8000, REGION_GFX4, 0 )	/* background tilemaps */
	ROM_LOAD( "ninjemak.7",   0x0000, 0x4000, CRC(80c20d36) SHA1(f20724754824030d62059388f3ea2224f5b7a60e) )
	ROM_LOAD( "ninjemak.6",   0x4000, 0x4000, CRC(1da7a651) SHA1(5307452058164a0bc39d144dd204627a9ead7543) )

	ROM_REGION( 0x4000, REGION_GFX5, 0 )	/* data for mcu/blitter? */
	ROM_LOAD( "ync-5.bin",    0x0000, 0x4000, CRC(993e4ab2) SHA1(aceafc83b36db4db923d27f77ad045e626678bae) )	/* text layer data */

	ROM_REGION( 0x0400, REGION_PROMS, 0 )	/* Region 3 - color data */
	ROM_LOAD( "yncp-6e.bin",  0x0000, 0x0100, CRC(ea47b91a) SHA1(9921aa1ef882fb664d85d3e065223610262ca112) )	/* red */
	ROM_LOAD( "yncp-7e.bin",  0x0100, 0x0100, CRC(e94c0fed) SHA1(68581c91e9aa485f78af6b6a5c98612372cd5b17) )	/* green */
	ROM_LOAD( "yncp-8e.bin",  0x0200, 0x0100, CRC(ffb4b287) SHA1(c3c7018e6d5e18cc2db135812d0dc3824710ab4c) )	/* blue */
	ROM_LOAD( "yncp-2d.bin",  0x0300, 0x0100, CRC(23bade78) SHA1(7e2de5eb08d888f97830807b6dbe85d09bb3b7f8) )	/* sprite lookup table */

	ROM_REGION( 0x0100, REGION_USER1, 0 )
	ROM_LOAD( "yncp-7f.bin",  0x0000, 0x0100, CRC(262d0809) SHA1(a67281af02cef082023c0d7d57e3824aeef67450) )	/* sprite palette bank */
ROM_END

ROM_START( youmab )
	ROM_REGION( 0x18000, REGION_CPU1, 0 )	/* main cpu code */
	ROM_LOAD( "electric1.3u", 0x00000, 0x8000, CRC(cc4fdb92) SHA1(9ce963db23f91f91e775a0b9a819f00db869120f) )
	ROM_LOAD( "electric3.3r", 0x10000, 0x8000, CRC(c1bc7387) SHA1(ad05bff02ece515465a9506e09c252c446c8f81d) )

	ROM_REGION( 0x10000, REGION_USER2, 0 )	/* main cpu code */
	/* This rom is double the size of the original one, appears to have extra (banked) code for 0x8000 */
	ROM_LOAD( "electric2.3t", 0x00000, 0x8000, CRC(99aee3bc) SHA1(5ffd60b959dda3fd41609c89a3486a989b1e2530) )


	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* sound cpu code */
	ROM_LOAD( "electric12.5e",  0x0000, 0x4000, CRC(3d1cd329) SHA1(6abd8e0dbecddfd67c4d358b958c850136fd3c29) )
	ROM_LOAD( "electric13.5d",  0x4000, 0x8000, CRC(ac3a0b81) SHA1(39f2c305706e313d5256c357a3c8b57bbe45d3d7) )

	ROM_REGION( 0x08000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "electric4.3m",    0x00000, 0x8000, CRC(a1954f44) SHA1(b10a22b51bd1a02c0d7b116b4d7390003c41decf) )	/* chars */

	ROM_REGION( 0x20000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "electric8.1f",  0x00000, 0x8000, CRC(655f0a58) SHA1(8ffe73cec68d52c7b09651b546289613d6d4dde4) ) /* tiles */
	ROM_LOAD( "electric9.1d",  0x08000, 0x8000, CRC(77a964c1) SHA1(47eb2d4df240e5493951b0a170cd07b2d5ecc18a) ) // different (bad?)
	ROM_LOAD( "electric10.1b", 0x10000, 0x8000, CRC(955b5c45) SHA1(936bfe2599228dd0861bbcfe15152ac5e9b906d1) )
	ROM_LOAD( "electric11.1a", 0x18000, 0x8000, CRC(bbd2e51c) SHA1(51bc266cf8161610204e5d98e56346b1d8d3c009) )

	ROM_REGION( 0x20000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "electric16.1p",  0x00000, 0x8000, CRC(8df93fed) SHA1(ef37c78d4abbdbe9f427e3d9345f52464261116d) )  /* sprites */
	ROM_LOAD( "electric17.1m",  0x08000, 0x8000, CRC(a3efd0fc) SHA1(69d40707b0570c2f1be6247f0209ba9e60a83ed0) )
	ROM_LOAD( "electric14.1t",  0x10000, 0x8000, CRC(bff332d3) SHA1(d277ba18034b083eaafa969d90685563994416fa) )
	ROM_LOAD( "electric15.1r",  0x18000, 0x8000, CRC(56430ed4) SHA1(68356a0f68404ef70d8dc17d5cbdf5e1f28badcf) )

	ROM_REGION( 0x8000, REGION_GFX4, 0 )	/* background tilemaps */
	ROM_LOAD( "electric7.3a",   0x0000, 0x4000, CRC(80c20d36) SHA1(f20724754824030d62059388f3ea2224f5b7a60e) )
	ROM_LOAD( "electric6.3b",   0x4000, 0x4000, CRC(1da7a651) SHA1(5307452058164a0bc39d144dd204627a9ead7543) )

	ROM_REGION( 0x0400, REGION_PROMS, 0 )	/* Region 3 - color data */
	ROM_LOAD( "prom82s129.2n",  0x0000, 0x0100, CRC(ea47b91a) SHA1(9921aa1ef882fb664d85d3e065223610262ca112) )	/* red */
	ROM_LOAD( "prom82s129.2m",  0x0100, 0x0100, CRC(e94c0fed) SHA1(68581c91e9aa485f78af6b6a5c98612372cd5b17) )	/* green */
	ROM_LOAD( "prom82s129.2l",  0x0200, 0x0100, CRC(ffb4b287) SHA1(c3c7018e6d5e18cc2db135812d0dc3824710ab4c) )	/* blue */
	ROM_LOAD( "prom82s129.3s",  0x0300, 0x0100, CRC(23bade78) SHA1(7e2de5eb08d888f97830807b6dbe85d09bb3b7f8) )	/* sprite lookup table */

	ROM_REGION( 0x0100, REGION_USER1, 0 )
	ROM_LOAD( "prom82s129.1l",  0x0000, 0x0100, CRC(262d0809) SHA1(a67281af02cef082023c0d7d57e3824aeef67450) )	/* sprite palette bank */
ROM_END


ROM_START( youmab2 )
	ROM_REGION( 0x18000, REGION_CPU1, 0 )	/* main cpu code */
	ROM_LOAD( "1.1d",	  0x00000, 0x8000, CRC(692ae497) SHA1(572e5a1eae9b0bb48f65dce5de2df5c5ae95a3bd) )
	ROM_LOAD( "3.4d",     0x10000, 0x8000, CRC(ebf61afc) SHA1(30235a90e8316f5033d44d31f02cca97c64f2d5e) )

	ROM_REGION( 0x10000, REGION_USER2, 0 )	/* main cpu code */
	/* This rom is double the size of the original one, appears to have extra (banked) code for 0x8000 */
	ROM_LOAD( "2.2d", 0x00000, 0x8000, CRC(99aee3bc) SHA1(5ffd60b959dda3fd41609c89a3486a989b1e2530) ) // same as first bootleg

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* sound cpu code */
	ROM_LOAD( "11.13b",  0x0000, 0x4000, CRC(3d1cd329) SHA1(6abd8e0dbecddfd67c4d358b958c850136fd3c29) )
	ROM_LOAD( "12.15b",  0x4000, 0x8000, CRC(ac3a0b81) SHA1(39f2c305706e313d5256c357a3c8b57bbe45d3d7) )

	ROM_REGION( 0x08000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "4.7d",    0x00000, 0x8000, CRC(a1954f44) SHA1(b10a22b51bd1a02c0d7b116b4d7390003c41decf) )	/* chars */

	ROM_REGION( 0x20000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "7.13f",   0x00000, 0x8000, CRC(655f0a58) SHA1(8ffe73cec68d52c7b09651b546289613d6d4dde4) ) /* tiles */
	ROM_LOAD( "8.15f",   0x08000, 0x8000, CRC(934e1703) SHA1(451f8d01d9035d91c969cdc3fb582a00007da7df) )
	ROM_LOAD( "9.16f",   0x10000, 0x8000, CRC(955b5c45) SHA1(936bfe2599228dd0861bbcfe15152ac5e9b906d1) )
	ROM_LOAD( "10.18f",  0x18000, 0x8000, CRC(bbd2e51c) SHA1(51bc266cf8161610204e5d98e56346b1d8d3c009) )

	ROM_REGION( 0x20000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "15.4h",  0x00000, 0x8000, CRC(8df93fed) SHA1(ef37c78d4abbdbe9f427e3d9345f52464261116d) )  /* sprites */
	ROM_LOAD( "16.6h",  0x08000, 0x8000, CRC(a3efd0fc) SHA1(69d40707b0570c2f1be6247f0209ba9e60a83ed0) )
	ROM_LOAD( "13.1h",  0x10000, 0x8000, CRC(bff332d3) SHA1(d277ba18034b083eaafa969d90685563994416fa) )
	ROM_LOAD( "14.3h",  0x18000, 0x8000, CRC(56430ed4) SHA1(68356a0f68404ef70d8dc17d5cbdf5e1f28badcf) )

	ROM_REGION( 0x8000, REGION_GFX4, 0 )	/* background tilemaps */
	ROM_LOAD( "6.18d",   0x0000, 0x4000, CRC(80c20d36) SHA1(f20724754824030d62059388f3ea2224f5b7a60e) )
	ROM_LOAD( "5.17d",   0x4000, 0x4000, CRC(1da7a651) SHA1(5307452058164a0bc39d144dd204627a9ead7543) )

	ROM_REGION( 0x0400, REGION_PROMS, 0 )	/* Region 3 - color data */
	ROM_LOAD( "pr.6e",  0x0000, 0x0100, CRC(ea47b91a) SHA1(9921aa1ef882fb664d85d3e065223610262ca112) )	/* red */
	ROM_LOAD( "pr.7e",  0x0100, 0x0100, CRC(6d66da81) SHA1(ffdd1778ce5b7614b90b5da85589c5871405d3fe) )	/* green */ // different (bad?)
	ROM_LOAD( "pr.8e",  0x0200, 0x0100, CRC(ffb4b287) SHA1(c3c7018e6d5e18cc2db135812d0dc3824710ab4c) )	/* blue */
	ROM_LOAD( "pr.2e",  0x0300, 0x0100, CRC(23bade78) SHA1(7e2de5eb08d888f97830807b6dbe85d09bb3b7f8) )	/* sprite lookup table */

	ROM_REGION( 0x0100, REGION_USER1, 0 )
	ROM_LOAD( "pr.7h",  0x0000, 0x0100, CRC(262d0809) SHA1(a67281af02cef082023c0d7d57e3824aeef67450) )	/* sprite palette bank */
ROM_END


static WRITE8_HANDLER( youmab_extra_bank_w )
{
	if (data==0xff)
	{
		memory_set_bankptr( 2, memory_region(machine, REGION_USER2)+0x4000 );
	}
	else if (data==0x00)
	{
		memory_set_bankptr( 2, memory_region(machine, REGION_USER2) );
	}
	else
	{
		printf("data %03x\n",data);
	}
}

static READ8_HANDLER( youmab_8a_r )
{
	return mame_rand(machine);
}

static WRITE8_HANDLER( youmab_81_w )
{
	// ??
}

static WRITE8_HANDLER( youmab_84_w )
{
	// ??
}

static DRIVER_INIT( youmab )
{
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_IO, 0x82, 0x82, 0, 0, youmab_extra_bank_w); // banks rom at 0x8000? writes 0xff and 0x00 before executing code there
	memory_install_read8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x8000, 0xbfff, 0, 0, SMH_BANK2);
	memory_set_bankptr( 2, memory_region(machine, REGION_USER2) );

	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_IO, 0x81, 0x81, 0, 0, youmab_81_w); // ?? often, alternating values
	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_IO, 0x84, 0x84, 0, 0, youmab_84_w); // ?? often, sequence..

	memory_install_write8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xd800, 0xd81f, 0, 0, SMH_NOP); // scrolling isn't here..

	memory_install_read8_handler(machine, 0, ADDRESS_SPACE_IO, 0x8a, 0x8a, 0, 0, youmab_8a_r); // ???

}

GAME( 1985, galivan,  0,        galivan,  galivan,  0, ROT270, "Nichibutsu", "Galivan - Cosmo Police (12/16/1985)", GAME_SUPPORTS_SAVE )
GAME( 1985, galivan2, galivan,  galivan,  galivan,  0, ROT270, "Nichibutsu", "Galivan - Cosmo Police (12/11/1985)", GAME_SUPPORTS_SAVE )
GAME( 1986, dangar,   0,        galivan,  dangar,   0, ROT270, "Nichibutsu", "Dangar - Ufo Robo (12/1/1986)", GAME_SUPPORTS_SAVE )
GAME( 1986, dangar2,  dangar,   galivan,  dangar2,  0, ROT270, "Nichibutsu", "Dangar - Ufo Robo (9/26/1986)", GAME_SUPPORTS_SAVE )
GAME( 1986, dangarb,  dangar,   galivan,  dangarb,  0, ROT270, "bootleg", "Dangar - Ufo Robo (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1986, ninjemak, 0,        ninjemak, ninjemak, 0, ROT270, "Nichibutsu", "Ninja Emaki (US)", GAME_SUPPORTS_SAVE )
GAME( 1986, youma,    ninjemak, ninjemak, ninjemak, 0, ROT270, "Nichibutsu", "Youma Ninpou Chou (Japan)", GAME_SUPPORTS_SAVE )
GAME( 1986, youmab,   ninjemak, ninjemak, ninjemak, youmab, ROT270, "bootleg", "Youma Ninpou Chou (Game Electronics bootleg, set 1)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE ) // scrolling doesn't work
GAME( 1986, youmab2,  ninjemak, ninjemak, ninjemak, youmab, ROT270, "bootleg", "Youma Ninpou Chou (Game Electronics bootleg, set 2)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE ) // scrolling doesn't work

