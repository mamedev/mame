/***************************************************************************

The FairyLand Story

  added Victorious Nine by BUT

TODO:
- game behaves VERY strangely in attract mode. Wait until the demo game will
  be shown. Insert a coin - game screen will be replaced with "start screen"
  but you can still see demo sprites and hear the in-game sounds.
- TA7630 emulation needs filter support (bass sounds from MSM5232 should be about 2 times louder)

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "cpu/m6805/m6805.h"
#include "sound/ay8910.h"
#include "sound/msm5232.h"
#include "sound/dac.h"

VIDEO_START( flstory );
VIDEO_UPDATE( flstory );
VIDEO_START( victnine );
VIDEO_UPDATE( victnine );

extern UINT8 *flstory_scrlram;

WRITE8_HANDLER( flstory_videoram_w );
READ8_HANDLER( flstory_palette_r );
WRITE8_HANDLER( flstory_palette_w );
WRITE8_HANDLER( flstory_gfxctrl_w );
READ8_HANDLER( flstory_scrlram_r );
WRITE8_HANDLER( flstory_scrlram_w );
READ8_HANDLER( victnine_gfxctrl_r );
WRITE8_HANDLER( victnine_gfxctrl_w );

READ8_HANDLER( flstory_68705_portA_r );
WRITE8_HANDLER( flstory_68705_portA_w );
READ8_HANDLER( flstory_68705_portB_r );
WRITE8_HANDLER( flstory_68705_portB_w );
READ8_HANDLER( flstory_68705_portC_r );
WRITE8_HANDLER( flstory_68705_portC_w );
WRITE8_HANDLER( flstory_68705_ddrA_w );
WRITE8_HANDLER( flstory_68705_ddrB_w );
WRITE8_HANDLER( flstory_68705_ddrC_w );
WRITE8_HANDLER( flstory_mcu_w );
READ8_HANDLER( flstory_mcu_r );
READ8_HANDLER( flstory_mcu_status_r );
WRITE8_HANDLER( onna34ro_mcu_w );
READ8_HANDLER( onna34ro_mcu_r );
READ8_HANDLER( onna34ro_mcu_status_r );
WRITE8_HANDLER( victnine_mcu_w );
READ8_HANDLER( victnine_mcu_r );
READ8_HANDLER( victnine_mcu_status_r );

UINT8 *onna34ro_workram;
UINT8 *victnine_workram;

static UINT8 snd_data;
static UINT8 snd_flag;

static READ8_HANDLER( from_snd_r )
{
	snd_flag = 0;
	return snd_data;
}

static READ8_HANDLER( snd_flag_r )
{
	return snd_flag | 0xfd;
}

static WRITE8_HANDLER( to_main_w )
{
	snd_data = data;
	snd_flag = 2;
}


static int sound_nmi_enable,pending_nmi;

static TIMER_CALLBACK( nmi_callback )
{
	if (sound_nmi_enable) cpunum_set_input_line(machine, 1,INPUT_LINE_NMI,PULSE_LINE);
	else pending_nmi = 1;
}

static WRITE8_HANDLER( sound_command_w )
{
	soundlatch_w(0,data);
	timer_call_after_resynch(NULL, data,nmi_callback);
}


static WRITE8_HANDLER( nmi_disable_w )
{
	sound_nmi_enable = 0;
}

static WRITE8_HANDLER( nmi_enable_w )
{
	sound_nmi_enable = 1;
	if (pending_nmi)
	{
		cpunum_set_input_line(Machine, 1,INPUT_LINE_NMI,PULSE_LINE);
		pending_nmi = 0;
	}
}

static ADDRESS_MAP_START( flstory_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_WRITE(flstory_videoram_w) AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0xc800, 0xcfff) AM_RAM /* unknown */
	AM_RANGE(0xd000, 0xd000) AM_READWRITE(flstory_mcu_r, flstory_mcu_w)
	AM_RANGE(0xd001, 0xd001) AM_WRITENOP	/* watchdog? */
	AM_RANGE(0xd002, 0xd002) AM_WRITENOP	/* coin lock out? */
	AM_RANGE(0xd400, 0xd400) AM_READWRITE(from_snd_r, sound_command_w)
	AM_RANGE(0xd401, 0xd401) AM_READ(snd_flag_r)
	AM_RANGE(0xd403, 0xd403) AM_NOP	/* unknown */
	AM_RANGE(0xd800, 0xd800) AM_READ(input_port_0_r)
	AM_RANGE(0xd801, 0xd801) AM_READ(input_port_1_r)
	AM_RANGE(0xd802, 0xd802) AM_READ(input_port_2_r)
	AM_RANGE(0xd803, 0xd803) AM_READ(input_port_3_r)
	AM_RANGE(0xd804, 0xd804) AM_READ(input_port_4_r)
	AM_RANGE(0xd805, 0xd805) AM_READ(flstory_mcu_status_r)
	AM_RANGE(0xd806, 0xd806) AM_READ(input_port_5_r)
//  AM_RANGE(0xda00, 0xda00) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xdc00, 0xdc9f) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xdca0, 0xdcbf) AM_RAM AM_WRITE(flstory_scrlram_w) AM_BASE(&flstory_scrlram)
	AM_RANGE(0xdcc0, 0xdcff) AM_RAM /* unknown */
	AM_RANGE(0xdd00, 0xdeff) AM_READWRITE(flstory_palette_r, flstory_palette_w)
	AM_RANGE(0xdf03, 0xdf03) AM_WRITE(flstory_gfxctrl_w)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM	/* work RAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( onna34ro_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_WRITE(flstory_videoram_w) AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0xc800, 0xcfff) AM_RAM	/* unknown */
	AM_RANGE(0xd000, 0xd000) AM_READWRITE(onna34ro_mcu_r, onna34ro_mcu_w)
	AM_RANGE(0xd001, 0xd001) AM_WRITENOP	/* watchdog? */
	AM_RANGE(0xd002, 0xd002) AM_WRITENOP	/* coin lock out? */
	AM_RANGE(0xd400, 0xd400) AM_READWRITE(from_snd_r, sound_command_w)
	AM_RANGE(0xd401, 0xd401) AM_READ(snd_flag_r)
	AM_RANGE(0xd403, 0xd403) AM_NOP	/* unknown */
	AM_RANGE(0xd800, 0xd800) AM_READ(input_port_0_r)
	AM_RANGE(0xd801, 0xd801) AM_READ(input_port_1_r)
	AM_RANGE(0xd802, 0xd802) AM_READ(input_port_2_r)
	AM_RANGE(0xd803, 0xd803) AM_READ(input_port_3_r)
	AM_RANGE(0xd804, 0xd804) AM_READ(input_port_4_r)
	AM_RANGE(0xd805, 0xd805) AM_READ(onna34ro_mcu_status_r)
	AM_RANGE(0xd806, 0xd806) AM_READ(input_port_5_r)
//  AM_RANGE(0xda00, 0xda00) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xdc00, 0xdc9f) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xdca0, 0xdcbf) AM_RAM AM_WRITE(flstory_scrlram_w) AM_BASE(&flstory_scrlram)
	AM_RANGE(0xdcc0, 0xdcff) AM_RAM /* unknown */
	AM_RANGE(0xdd00, 0xdeff) AM_READWRITE(flstory_palette_r, flstory_palette_w)
	AM_RANGE(0xdf03, 0xdf03) AM_WRITE(flstory_gfxctrl_w)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM	AM_BASE(&onna34ro_workram) /* work RAM */
ADDRESS_MAP_END

static READ8_HANDLER( victnine_port_5_r )
{
	return (victnine_mcu_status_r(0) & 3) | (readinputport(5) & ~3);
}

static ADDRESS_MAP_START( victnine_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_WRITE(flstory_videoram_w) AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0xc800, 0xcfff) AM_RAM	/* unknown */
	AM_RANGE(0xd000, 0xd000) AM_READWRITE(victnine_mcu_r, victnine_mcu_w)
	AM_RANGE(0xd001, 0xd001) AM_WRITENOP	/* watchdog? */
	AM_RANGE(0xd002, 0xd002) AM_READWRITE(MRA8_NOP, MWA8_NOP)	/* unknown read & coin lock out? */
	AM_RANGE(0xd400, 0xd400) AM_READWRITE(from_snd_r, sound_command_w)
	AM_RANGE(0xd401, 0xd401) AM_READ(snd_flag_r)
	AM_RANGE(0xd403, 0xd403) AM_READNOP /* unknown */
	AM_RANGE(0xd800, 0xd800) AM_READ(input_port_0_r)
	AM_RANGE(0xd801, 0xd801) AM_READ(input_port_1_r)
	AM_RANGE(0xd802, 0xd802) AM_READ(input_port_2_r)
	AM_RANGE(0xd803, 0xd803) AM_READ(input_port_3_r)
	AM_RANGE(0xd804, 0xd804) AM_READ(input_port_4_r)
	AM_RANGE(0xd805, 0xd805) AM_READ(victnine_port_5_r)
	AM_RANGE(0xd806, 0xd806) AM_READ(input_port_6_r)
	AM_RANGE(0xd807, 0xd807) AM_READ(input_port_7_r)
//  AM_RANGE(0xda00, 0xda00) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xdc00, 0xdc9f) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xdca0, 0xdcbf) AM_RAM AM_WRITE(flstory_scrlram_w) AM_BASE(&flstory_scrlram)
	AM_RANGE(0xdce0, 0xdce0) AM_READWRITE(victnine_gfxctrl_r, victnine_gfxctrl_w)
	AM_RANGE(0xdce1, 0xdce1) AM_WRITENOP	/* unknown */
	AM_RANGE(0xdd00, 0xdeff) AM_READWRITE(flstory_palette_r, flstory_palette_w)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM	AM_BASE(&victnine_workram) /* work RAM */
ADDRESS_MAP_END

static int vol_ctrl[16];

static MACHINE_RESET( ta7630 )
{
	int i;

	double db			= 0.0;
	double db_step		= 1.50;	/* 1.50 dB step (at least, maybe more) */
	double db_step_inc	= 0.125;
	for (i=0; i<16; i++)
	{
		double max = 100.0 / pow(10.0, db/20.0 );
		vol_ctrl[ 15-i ] = max;
		/*logerror("vol_ctrl[%x] = %i (%f dB)\n",15-i,vol_ctrl[ 15-i ],db);*/
		db += db_step;
		db_step += db_step_inc;
	}

	/* for (i=0; i<8; i++)
        logerror("SOUND Chan#%i name=%s\n", i, mixer_get_name(i) ); */
/*
  channels 0-2 AY#0
  channels 3,4 MSM5232 group1,group2
*/
}

static UINT8 snd_ctrl0=0;
static UINT8 snd_ctrl1=0;
static UINT8 snd_ctrl2=0;
static UINT8 snd_ctrl3=0;

static WRITE8_HANDLER( sound_control_0_w )
{
	snd_ctrl0 = data & 0xff;
//  popmessage("SND0 0=%02x 1=%02x 2=%02x 3=%02x", snd_ctrl0, snd_ctrl1, snd_ctrl2, snd_ctrl3);

	/* this definitely controls main melody voice on 2'-1 and 4'-1 outputs */
	sndti_set_output_gain(SOUND_MSM5232, 0, 0, vol_ctrl[ (snd_ctrl0>>4) & 15 ] / 100.0);	/* group1 from msm5232 */

}
static WRITE8_HANDLER( sound_control_1_w )
{
	snd_ctrl1 = data & 0xff;
//  popmessage("SND1 0=%02x 1=%02x 2=%02x 3=%02x", snd_ctrl0, snd_ctrl1, snd_ctrl2, snd_ctrl3);
	sndti_set_output_gain(SOUND_MSM5232, 0, 1, vol_ctrl[ (snd_ctrl1>>4) & 15 ] / 100.0);	/* group2 from msm5232 */
}

static WRITE8_HANDLER( sound_control_2_w )
{
	int i;

	snd_ctrl2 = data & 0xff;
//  popmessage("SND2 0=%02x 1=%02x 2=%02x 3=%02x", snd_ctrl0, snd_ctrl1, snd_ctrl2, snd_ctrl3);

	for (i=0; i<3; i++)
		sndti_set_output_gain (SOUND_AY8910, 0, i, vol_ctrl[ (snd_ctrl2>>4) & 15 ] / 100.0);	/* ym2149f all */
}

static WRITE8_HANDLER( sound_control_3_w ) /* unknown */
{
	snd_ctrl3 = data & 0xff;
//  popmessage("SND3 0=%02x 1=%02x 2=%02x 3=%02x", snd_ctrl0, snd_ctrl1, snd_ctrl2, snd_ctrl3);
}


static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xc800, 0xc800) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0xc801, 0xc801) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0xca00, 0xca0d) AM_WRITE(MSM5232_0_w)
	AM_RANGE(0xcc00, 0xcc00) AM_WRITE(sound_control_0_w)
	AM_RANGE(0xce00, 0xce00) AM_WRITE(sound_control_1_w)
	AM_RANGE(0xd800, 0xd800) AM_READWRITE(soundlatch_r, to_main_w)
	AM_RANGE(0xda00, 0xda00) AM_READWRITE(MRA8_NOP, nmi_enable_w)			/* unknown read*/
	AM_RANGE(0xdc00, 0xdc00) AM_WRITE(nmi_disable_w)
	AM_RANGE(0xde00, 0xde00) AM_READWRITE(MRA8_NOP, DAC_0_signed_data_w)	/* signed 8-bit DAC &  unknown read */
	AM_RANGE(0xe000, 0xefff) AM_ROM											/* space for diagnostics ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( flstory_m68705_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(11) )
	AM_RANGE(0x0000, 0x0000) AM_READWRITE(flstory_68705_portA_r, flstory_68705_portA_w)
	AM_RANGE(0x0001, 0x0001) AM_READWRITE(flstory_68705_portB_r, flstory_68705_portB_w)
	AM_RANGE(0x0002, 0x0002) AM_READWRITE(flstory_68705_portC_r, flstory_68705_portC_w)
	AM_RANGE(0x0004, 0x0004) AM_WRITE(flstory_68705_ddrA_w)
	AM_RANGE(0x0005, 0x0005) AM_WRITE(flstory_68705_ddrB_w)
	AM_RANGE(0x0006, 0x0006) AM_WRITE(flstory_68705_ddrC_w)
	AM_RANGE(0x0010, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x07ff) AM_ROM
ADDRESS_MAP_END



/* When "Debug Mode" Dip Switch is ON, keep IPT_SERVICE1 ('9') pressed to freeze the game.
   Once the game is frozen, you can press IPT_START1 ('5') to advance 1 frame, or IPT_START2
   ('6') to advance 6 frames.

   When "Continue" Dip Switch is ON, you can only continue in a 1 player game AND when level
   (0xe781) is between 8 and 98 (included).
*/

static INPUT_PORTS_START( flstory )
	PORT_START_TAG("DSW0")      /*D800*/
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30000 100000" )
	PORT_DIPSETTING(    0x01, "30000 150000" )
	PORT_DIPSETTING(    0x02, "50000 150000" )
	PORT_DIPSETTING(    0x03, "70000 150000" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")
	PORT_DIPNAME( 0x20, 0x20, "Debug Mode" )			// Check code at 0x0679
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START_TAG("DSW1")      /*D801*/
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START_TAG("IN0")      /* D802 */
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Attract Animation" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Leave Off")		// Check code at 0x7859
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )		// (must be OFF or the game will
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )			// hang after the game is over !)
	PORT_BIT(    0x40, 0x40, IPT_DIPSWITCH_NAME ) PORT_NAME("Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Slots" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )

	PORT_START_TAG("IN1")      /* D803 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* "BAD IO" if low */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* "BAD IO" if low */

	PORT_START_TAG("IN2")      /* D804:P1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN3")      /* D806:P2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( onna34ro )
	PORT_START_TAG("DSW0")      /* D800*/
	PORT_DIPNAME(0x03, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(   0x00, "200000 200000" )
	PORT_DIPSETTING(   0x01, "200000 300000" )
	PORT_DIPSETTING(   0x02, "100000 200000" )
	PORT_DIPSETTING(   0x03, "200000 100000" )
	PORT_DIPNAME(0x04, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x04, DEF_STR( On ) )
	PORT_DIPNAME(0x18, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(   0x10, "1" )
	PORT_DIPSETTING(   0x08, "2" )
	PORT_DIPSETTING(   0x00, "3" )
	PORT_DIPSETTING(   0x18, "Endless (Cheat)")
	PORT_DIPNAME(0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(   0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(   0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(   0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(   0x00, DEF_STR( Cocktail ) )

	PORT_START_TAG("DSW1")      /* D801 */
	PORT_DIPNAME(0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(   0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(   0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(   0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(   0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(   0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(   0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(   0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(   0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(   0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(   0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(   0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(   0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(   0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(   0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(   0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(   0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME(0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(   0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(   0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(   0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(   0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(   0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(   0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(   0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(   0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(   0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(   0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(   0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(   0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(   0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(   0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(   0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(   0x70, DEF_STR( 1C_8C ) )

	PORT_START_TAG("DSW2")      /* D802 */
	PORT_DIPNAME(0x01, 0x00, "Invulnerability (Cheat)")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x01, DEF_STR( On ) )
	PORT_DIPNAME(0x02, 0x00, "Rack Test" )
	PORT_DIPSETTING(   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x02, DEF_STR( On ) )
	PORT_DIPNAME(0x04, 0x00, DEF_STR( Unknown ) ) /* demo sounds */
	PORT_DIPSETTING(   0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x08, 0x00, "Freeze" )
	PORT_DIPSETTING(   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x08, DEF_STR( On ) )
	PORT_DIPNAME(0x10, 0x00, "Coinage Display" )
	PORT_DIPSETTING(   0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x60, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(   0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(   0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(   0x40, "Difficult" )
	PORT_DIPSETTING(   0x60, "Very Difficult" )
	PORT_DIPNAME(0x80, 0x80, DEF_STR( Coinage ) )
	PORT_DIPSETTING(   0x80, "A and B" )
	PORT_DIPSETTING(   0x00, "A only" )

	PORT_START_TAG("IN0")      /* D803: START BUTTONS */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* "BAD IO" if low */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* "BAD IO" if low */

	PORT_START_TAG("IN1")      /* D804: P1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN2")      /* D806: P2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( victnine )
	PORT_START_TAG("DSW0")      /* D800 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME(0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(   0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(   0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPNAME(0xa0, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(   0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(   0xa0, DEF_STR( Cocktail ) )
	PORT_DIPSETTING(   0x00, "MA / MB" )

	PORT_START_TAG("DSW1")      /* D801 */
	PORT_DIPNAME(0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(   0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(   0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(   0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(   0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(   0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(   0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(   0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(   0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(   0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(   0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(   0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(   0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(   0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(   0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(   0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(   0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME(0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(   0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(   0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(   0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(   0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(   0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(   0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(   0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(   0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(   0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(   0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(   0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(   0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(   0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(   0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(   0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(   0x70, DEF_STR( 1C_8C ) )

	PORT_START_TAG("DSW2")      /* D802 */
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME(0x10, 0x10, "Coinage Display" )
	PORT_DIPSETTING(   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Show Year" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME(0x40, 0x40, "No hit" )
	PORT_DIPSETTING(   0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x80, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(   0x80, "A and B" )
	PORT_DIPSETTING(   0x00, "A only" )

	PORT_START_TAG("IN0")      /* D803: START BUTTONS */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN1")      /* D804: P1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )	// A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )	// C
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN2")      /* D805: 1P a/b/c/d BUTTONS */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL )	// mcu is ready to receive data from main cpu
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL )	// mcu has sent data to the main cpu
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN3")      /* D806: P2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL	// A
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL	// C
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN4")      /* D807: 2P a/b/c/d BUTTONS */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0,
			16*8+3, 16*8+2, 16*8+1, 16*8+0, 16*8+8+3, 16*8+8+2, 16*8+8+1, 16*8+8+0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16 },
	64*8
};

static GFXDECODE_START( flstory )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout,     0, 16 )
	GFXDECODE_ENTRY( REGION_GFX1, 0, spritelayout, 256, 16 )
GFXDECODE_END


static const struct AY8910interface ay8910_interface =
{
	0,
	0,
	sound_control_2_w,
	sound_control_3_w
};

static const struct MSM5232interface msm5232_interface =
{
	{ 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6 }	/* 1.0 uF capacitors (verified on real PCB) */
};


static MACHINE_DRIVER_START( flstory )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80,10733000/2)		/* ??? */
	MDRV_CPU_PROGRAM_MAP(flstory_map,0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)

	MDRV_CPU_ADD(Z80,8000000/2)
	/* audio CPU */		/* 4 MHz */
	MDRV_CPU_PROGRAM_MAP(sound_map,0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,2)	/* IRQ generated by ??? */
						/* NMI generated by the main CPU */

	MDRV_CPU_ADD(M68705,4000000)	/* ??? */
	MDRV_CPU_PROGRAM_MAP(flstory_m68705_map,0)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_INTERLEAVE(100)	/* 100 CPU slices per frame - an high value to ensure proper */
							/* synchronization of the CPUs */
	MDRV_MACHINE_RESET(ta7630)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(flstory)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(flstory)
	MDRV_VIDEO_UPDATE(flstory)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 8000000/4)
	MDRV_SOUND_CONFIG(ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MDRV_SOUND_ADD(MSM5232, 2000000)
	MDRV_SOUND_CONFIG(msm5232_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( onna34ro )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80,10733000/2)		/* ??? */
	MDRV_CPU_PROGRAM_MAP(onna34ro_map,0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)

	MDRV_CPU_ADD(Z80,8000000/2)
	/* audio CPU */		/* 4 MHz */
	MDRV_CPU_PROGRAM_MAP(sound_map,0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,2)	/* IRQ generated by ??? */
						/* NMI generated by the main CPU */
//  MDRV_CPU_ADD(M68705,4000000)  /* ??? */
//  MDRV_CPU_PROGRAM_MAP(m68705_readmem,m68705_writemem)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_INTERLEAVE(100)	/* 100 CPU slices per frame - an high value to ensure proper */
							/* synchronization of the CPUs */
	MDRV_MACHINE_RESET(ta7630)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(flstory)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(flstory)
	MDRV_VIDEO_UPDATE(flstory)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 8000000/4)
	MDRV_SOUND_CONFIG(ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MDRV_SOUND_ADD(MSM5232, 2000000)
	MDRV_SOUND_CONFIG(msm5232_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( victnine )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80,8000000/2)		/* 4 MHz */
	MDRV_CPU_PROGRAM_MAP(victnine_map,0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)

	MDRV_CPU_ADD(Z80,8000000/2)
	/* audio CPU */		/* 4 MHz */
	MDRV_CPU_PROGRAM_MAP(sound_map,0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,2)	/* IRQ generated by ??? */
						/* NMI generated by the main CPU */
//  MDRV_CPU_ADD(M68705,4000000)  /* ??? */
//  MDRV_CPU_PROGRAM_MAP(m68705_readmem,m68705_writemem)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_INTERLEAVE(100)	/* 100 CPU slices per frame - an high value to ensure proper */
							/* synchronization of the CPUs */
	MDRV_MACHINE_RESET(ta7630)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(flstory)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(victnine)
	MDRV_VIDEO_UPDATE(victnine)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 8000000/4)
	MDRV_SOUND_CONFIG(ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MDRV_SOUND_ADD(MSM5232, 2000000)
	MDRV_SOUND_CONFIG(msm5232_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)
MACHINE_DRIVER_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( flstory )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for the first CPU */
	ROM_LOAD( "cpu-a45.15",   0x0000, 0x4000, CRC(f03fc969) SHA1(c8dd25ca25fd413b1a29bd4e58ce5820e5f852b2) )
	ROM_LOAD( "cpu-a45.16",   0x4000, 0x4000, CRC(311aa82e) SHA1(c2dd806f70ea917818ec844a275fb2fecc2e6c19) )
	ROM_LOAD( "cpu-a45.17",   0x8000, 0x4000, CRC(a2b5d17d) SHA1(0198d048aedcbd2498d490a5c0c506f8fc66ed03) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for the second CPU */
	ROM_LOAD( "snd.22",       0x0000, 0x2000, CRC(d58b201d) SHA1(1c9c2936ec95a8fa920d58668bea420c5e15008f) )
	ROM_LOAD( "snd.23",       0x2000, 0x2000, CRC(25e7fd9d) SHA1(b9237459e3d8acf8502a693914e50714a37d515e) )

	ROM_REGION( 0x0800, REGION_CPU3, 0 )	/* 2k for the microcontroller */
	ROM_LOAD( "a45.mcu",      0x0000, 0x0800, CRC(5378253c) SHA1(e1ae1ab01e470b896c1d74ad4088928602a21a1b) )

	ROM_REGION( 0x20000, REGION_GFX1, ROMREGION_DISPOSE | ROMREGION_INVERT )
	ROM_LOAD( "vid-a45.18",   0x00000, 0x4000, CRC(6f08f69e) SHA1(8f1b7e63a38f855cf26d57aed678da7cf1378fdf) )
	ROM_LOAD( "vid-a45.06",   0x04000, 0x4000, CRC(dc856a75) SHA1(6eedbf6b027c884502b6e7329f13829787138165) )
	ROM_LOAD( "vid-a45.08",   0x08000, 0x4000, CRC(d0b028ca) SHA1(c8bd9136ad3180002961ecfe600fc91a3c891539) )
	ROM_LOAD( "vid-a45.20",   0x0c000, 0x4000, CRC(1b0edf34) SHA1(e749c78053ed09bdb42c03cf4589b0fe122d9095) )
	ROM_LOAD( "vid-a45.19",   0x10000, 0x4000, CRC(2b572dc9) SHA1(9e14428663819e18829c625b4ae91a8a5530eb33) )
	ROM_LOAD( "vid-a45.07",   0x14000, 0x4000, CRC(aa4b0762) SHA1(6d4246753e80fe3ca05d47bd279f7ccc603f4700) )
	ROM_LOAD( "vid-a45.09",   0x18000, 0x4000, CRC(8336be58) SHA1(b92d37856870c4128a860d8ae02fa647743b99e3) )
	ROM_LOAD( "vid-a45.21",   0x1c000, 0x4000, CRC(fc382bd1) SHA1(a773c87454a3d7b80374a6d38ecb8633af2cd990) )
ROM_END

ROM_START( flstoryj )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for the first CPU */
	ROM_LOAD( "cpu-a45.15",   0x0000, 0x4000, CRC(f03fc969) SHA1(c8dd25ca25fd413b1a29bd4e58ce5820e5f852b2) )
	ROM_LOAD( "cpu-a45.16",   0x4000, 0x4000, CRC(311aa82e) SHA1(c2dd806f70ea917818ec844a275fb2fecc2e6c19) )
	ROM_LOAD( "cpu-a45.17",   0x8000, 0x4000, CRC(a2b5d17d) SHA1(0198d048aedcbd2498d490a5c0c506f8fc66ed03) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for the second CPU */
	ROM_LOAD( "a45_12.8",     0x0000, 0x2000, CRC(d6f593fb) SHA1(8551ef22c2cdd9df8d7949a178883f56ea56a4a2) )
	ROM_LOAD( "a45_13.9",     0x2000, 0x2000, CRC(451f92f9) SHA1(f4196e6d3420983b74001303936d086a48b10827) )

	ROM_REGION( 0x0800, REGION_CPU3, 0 )	/* 2k for the microcontroller */
	ROM_LOAD( "a45.mcu",      0x0000, 0x0800, CRC(5378253c) SHA1(e1ae1ab01e470b896c1d74ad4088928602a21a1b) )

	ROM_REGION( 0x20000, REGION_GFX1, ROMREGION_DISPOSE | ROMREGION_INVERT )
	ROM_LOAD( "vid-a45.18",   0x00000, 0x4000, CRC(6f08f69e) SHA1(8f1b7e63a38f855cf26d57aed678da7cf1378fdf) )
	ROM_LOAD( "vid-a45.06",   0x04000, 0x4000, CRC(dc856a75) SHA1(6eedbf6b027c884502b6e7329f13829787138165) )
	ROM_LOAD( "vid-a45.08",   0x08000, 0x4000, CRC(d0b028ca) SHA1(c8bd9136ad3180002961ecfe600fc91a3c891539) )
	ROM_LOAD( "vid-a45.20",   0x0c000, 0x4000, CRC(1b0edf34) SHA1(e749c78053ed09bdb42c03cf4589b0fe122d9095) )
	ROM_LOAD( "vid-a45.19",   0x10000, 0x4000, CRC(2b572dc9) SHA1(9e14428663819e18829c625b4ae91a8a5530eb33) )
	ROM_LOAD( "vid-a45.07",   0x14000, 0x4000, CRC(aa4b0762) SHA1(6d4246753e80fe3ca05d47bd279f7ccc603f4700) )
	ROM_LOAD( "vid-a45.09",   0x18000, 0x4000, CRC(8336be58) SHA1(b92d37856870c4128a860d8ae02fa647743b99e3) )
	ROM_LOAD( "vid-a45.21",   0x1c000, 0x4000, CRC(fc382bd1) SHA1(a773c87454a3d7b80374a6d38ecb8633af2cd990) )
ROM_END

ROM_START( onna34ro )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for the first CPU */
	ROM_LOAD( "a52-01-1.40c", 0x0000, 0x4000, CRC(ffddcb02) SHA1(d7002e8a577a5f9c2f63ec8d93076cd720443e05) )
	ROM_LOAD( "a52-02-1.41c", 0x4000, 0x4000, CRC(da97150d) SHA1(9b18f4d0bff811e332f6d2e151c7583400d60f23) )
	ROM_LOAD( "a52-03-1.42c", 0x8000, 0x4000, CRC(b9749a53) SHA1(15fd9624a500512f7b2c6766ed96f3734f61f160) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for the second CPU */
	ROM_LOAD( "a52-12.08s",   0x0000, 0x2000, CRC(28f48096) SHA1(20aa5041cd71003e0981c32e34005bcbad53f707) )
	ROM_LOAD( "a52-13.09s",   0x2000, 0x2000, CRC(4d3b16f3) SHA1(8687b76398da875f69e9565277f00478c2b82a99) )
	ROM_LOAD( "a52-14.10s",   0x4000, 0x2000, CRC(90a6f4e8) SHA1(101767a90e963f3031e0830fd25a537ca8296de9) )
	ROM_LOAD( "a52-15.37s",   0x6000, 0x2000, CRC(5afc21d0) SHA1(317d5fb3a48ce5e13e02c5c6431fa08ada115d27) )
	ROM_LOAD( "a52-16.38s",   0x8000, 0x2000, CRC(ccf42aee) SHA1(a6eb01c5384724999631b55700dade430b71ca95) )

	ROM_REGION( 0x0800, REGION_CPU3, 0 )	/* 2k for the microcontroller */
	ROM_LOAD( "a52-17.54c",   0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x20000, REGION_GFX1, ROMREGION_DISPOSE | ROMREGION_INVERT )
	ROM_LOAD( "a52-04.11v",   0x00000, 0x4000, CRC(5b126294) SHA1(fc31e062e665f7313f923e84d6497716f0658ac0) )
	ROM_LOAD( "a52-06.10v",   0x04000, 0x4000, CRC(78114721) SHA1(d0e52544e05ab4fd1b131ed49beb252048bcbe31) )
	ROM_LOAD( "a52-08.09v",   0x08000, 0x4000, CRC(4a293745) SHA1(a54c1cfced63306db0ba7ee635dce41134c91dc8) )
	ROM_LOAD( "a52-10.08v",   0x0c000, 0x4000, CRC(8be7b4db) SHA1(e7ab373942b8ce75b36d0c9f547902fe65a3964d) )
	ROM_LOAD( "a52-05.35v",   0x10000, 0x4000, CRC(a1a99588) SHA1(eae63ae89058da1a92065e1d352cf81a15b556bc) )
	ROM_LOAD( "a52-07.34v",   0x14000, 0x4000, CRC(0bf420f2) SHA1(367e76efbed772fc8a6d7ac854407b62f8897d78) )
	ROM_LOAD( "a52-09.33v",   0x18000, 0x4000, CRC(39c543b5) SHA1(978c42f5eb23c15a96dae3578e742ef41bac689b) )
	ROM_LOAD( "a52-11.32v",   0x1c000, 0x4000, CRC(d1dda6b3) SHA1(fadf1404e8a03ec7e3fafb6281d33bc73bb5c473) )
ROM_END

ROM_START( onna34ra )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for the first CPU */
	ROM_LOAD( "ry-08.rom",    0x0000, 0x4000, CRC(e4587b85) SHA1(2fc4439953dd086eac11ba6d7937d8075fc39639) )
	ROM_LOAD( "ry-07.rom",    0x4000, 0x4000, CRC(6ffda515) SHA1(429e7bb22c66eb3c6d31981c2021af61c44ed51b) )
	ROM_LOAD( "ry-06.rom",    0x8000, 0x4000, CRC(6fefcda8) SHA1(f532e254a8bd7372bd9f8f21c907e44e0f5f4f32) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for the second CPU */
	ROM_LOAD( "a52-12.08s",   0x0000, 0x2000, CRC(28f48096) SHA1(20aa5041cd71003e0981c32e34005bcbad53f707) )
	ROM_LOAD( "a52-13.09s",   0x2000, 0x2000, CRC(4d3b16f3) SHA1(8687b76398da875f69e9565277f00478c2b82a99) )
	ROM_LOAD( "a52-14.10s",   0x4000, 0x2000, CRC(90a6f4e8) SHA1(101767a90e963f3031e0830fd25a537ca8296de9) )
	ROM_LOAD( "a52-15.37s",   0x6000, 0x2000, CRC(5afc21d0) SHA1(317d5fb3a48ce5e13e02c5c6431fa08ada115d27) )
	ROM_LOAD( "a52-16.38s",   0x8000, 0x2000, CRC(ccf42aee) SHA1(a6eb01c5384724999631b55700dade430b71ca95) )

	ROM_REGION( 0x0800, REGION_CPU3, 0 )	/* 2k for the microcontroller */
	ROM_LOAD( "a52-17.54c",   0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x20000, REGION_GFX1, ROMREGION_DISPOSE | ROMREGION_INVERT )
	ROM_LOAD( "a52-04.11v",   0x00000, 0x4000, CRC(5b126294) SHA1(fc31e062e665f7313f923e84d6497716f0658ac0) )
	ROM_LOAD( "a52-06.10v",   0x04000, 0x4000, CRC(78114721) SHA1(d0e52544e05ab4fd1b131ed49beb252048bcbe31) )
	ROM_LOAD( "a52-08.09v",   0x08000, 0x4000, CRC(4a293745) SHA1(a54c1cfced63306db0ba7ee635dce41134c91dc8) )
	ROM_LOAD( "a52-10.08v",   0x0c000, 0x4000, CRC(8be7b4db) SHA1(e7ab373942b8ce75b36d0c9f547902fe65a3964d) )
	ROM_LOAD( "a52-05.35v",   0x10000, 0x4000, CRC(a1a99588) SHA1(eae63ae89058da1a92065e1d352cf81a15b556bc) )
	ROM_LOAD( "a52-07.34v",   0x14000, 0x4000, CRC(0bf420f2) SHA1(367e76efbed772fc8a6d7ac854407b62f8897d78) )
	ROM_LOAD( "a52-09.33v",   0x18000, 0x4000, CRC(39c543b5) SHA1(978c42f5eb23c15a96dae3578e742ef41bac689b) )
	ROM_LOAD( "a52-11.32v",   0x1c000, 0x4000, CRC(d1dda6b3) SHA1(fadf1404e8a03ec7e3fafb6281d33bc73bb5c473) )
ROM_END

ROM_START( victnine )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	/* 64k for the first CPU */
	ROM_LOAD( "a16-19.1",     0x0000, 0x2000, CRC(deb7c439) SHA1(e87c8f95bc31d8450a3deed7a14b5fe139778d47) )
	ROM_LOAD( "a16-20.2",     0x2000, 0x2000, CRC(60cdb6ae) SHA1(65f09ef624d758b138a87c4cc80bc3539cc89507) )
	ROM_LOAD( "a16-21.3",     0x4000, 0x2000, CRC(121bea03) SHA1(4925b56a3f5725f1e00bd6aa87949aca5caf476b) )
	ROM_LOAD( "a16-22.4",     0x6000, 0x2000, CRC(b20e3027) SHA1(fab83afd1010fe6cebbeee06099eb2be9b96ec8a) )
	ROM_LOAD( "a16-23.5",     0x8000, 0x2000, CRC(95fe9cb7) SHA1(cfd7c0123940f680365500a516c8435330ed5f60) )
	ROM_LOAD( "a16-24.6",     0xa000, 0x2000, CRC(32b5c155) SHA1(34d25f3d4fae580757b69431b8b58f6f86d2282e) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for the second CPU */
	ROM_LOAD( "a16-12.8",     0x0000, 0x2000, CRC(4b9bff43) SHA1(4bcd52d6d72213f8fa7b544dbdd344312a9e2115) )
	ROM_LOAD( "a16-13.9",     0x2000, 0x2000, CRC(355121b9) SHA1(69cbe31eed53456f49a81c37b6661f7ba4a72fa6) )
	ROM_LOAD( "a16-14.10",    0x4000, 0x2000, CRC(0f33ef4d) SHA1(6916016d7cf43870d2e19fc1e6f1b20e48e07d76) )
	ROM_LOAD( "a16-15.37",    0x6000, 0x2000, CRC(f91d63dc) SHA1(4585d0c7ed05249c17385f20b6557e2e4375a6bb) )
	ROM_LOAD( "a16-16.38",    0x8000, 0x2000, CRC(9395351b) SHA1(8f97bdf03dec47bcaaa62fb66c545566776116be) )
	ROM_LOAD( "a16-17.39",    0xa000, 0x2000, CRC(872270b3) SHA1(2298cb8ced6c3e9afb430faab1b38ba8f2fa93b5) )

	ROM_REGION( 0x0800, REGION_CPU3, 0 )	/* 2k for the microcontroller */
	ROM_LOAD( "a16-18.mcu",   0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x10000, REGION_GFX1, ROMREGION_DISPOSE | ROMREGION_INVERT )
	ROM_LOAD( "a16-06-1.7",   0x00000, 0x2000, CRC(b708134d) SHA1(9732be463cfbbe81ea0ad06da5a48b660ca429d0) )
	ROM_LOAD( "a16-07-2.8",   0x02000, 0x2000, CRC(cdaf7f83) SHA1(cf83af1655cb3ffce26c1b015b1e2249f7b12e3f) )
	ROM_LOAD( "a16-10.90",    0x04000, 0x2000, CRC(e8e42454) SHA1(c4923d4adfc0a48cf5a7d0145de5c9389495cac2) )
	ROM_LOAD( "a16-11-1.91",  0x06000, 0x2000, CRC(1f766661) SHA1(dfeecb587af7706e0e14539efc3386558f5d6da4) )
	ROM_LOAD( "a16-04.5",     0x08000, 0x2000, CRC(b2fae99f) SHA1(c8e56815159cd43a94c7e31b764d5bb996551a49) )
	ROM_LOAD( "a16-05-1.6",   0x0a000, 0x2000, CRC(85dfbb6e) SHA1(3643aab950d54eadded8d952033672aabb1e87c4) )
	ROM_LOAD( "a16-08.88",    0x0c000, 0x2000, CRC(1ddb6466) SHA1(0ea75c2fb584215f3cd4a7b7dfb3345a303e7e66) )
	ROM_LOAD( "a16-09-1.89",  0x0e000, 0x2000, CRC(23d4c43c) SHA1(ed0e059d3f97705331fdcc423a7c37aac9f07bb0) )
ROM_END


GAME( 1985, flstory,  0,        flstory,  flstory,  0, ROT180, "Taito", "The FairyLand Story", GAME_IMPERFECT_SOUND )
GAME( 1985, flstoryj, flstory,  flstory,  flstory,  0, ROT180, "Taito", "The FairyLand Story (Japan)", GAME_IMPERFECT_SOUND )
GAME( 1985, onna34ro, 0,        onna34ro, onna34ro, 0, ROT0,   "Taito", "Onna Sansirou - Typhoon Gal (set 1)", GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_SOUND )
GAME( 1985, onna34ra, onna34ro, onna34ro, onna34ro, 0, ROT0,   "Taito", "Onna Sansirou - Typhoon Gal (set 2)", GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_SOUND )
GAME( 1984, victnine, 0,        victnine, victnine, 0, ROT0,   "Taito", "Victorious Nine", GAME_IMPERFECT_SOUND )
