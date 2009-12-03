/* Ganbare Chinsan Ooshoubu
 driver by David Haywood

ToDo:
Improve Inputs

Notes:
-ADPCM hook-up is virtually identical to the other Sanritsu games (Jantotsu, Appoooh, Dr. Micro etc.).

*/

/*

Ganbare Chinsan Ooshoubu
(c)1987 Sanritsu

C1-00114-B

CPU:    317-5012 (MC-8123A)
SOUND:  YM2203
        M5205
OSC:    10.000MHz
        384KHz
Chips:  8255AC-2


MM00.7D   prg.
MM01.8D

MM20.7K   chr.
MM21.8K
MM22.9K

MM40.13D  samples

MM60.2C   ?

MM61.9M   color
MM62.9N
MM63.10N

*/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "machine/mc8123.h"
#include "sound/2203intf.h"
#include "sound/msm5205.h"

static UINT8* chinsan_video;
static UINT8 chinsan_port_select;

static UINT32 adpcm_pos;
static UINT8 adpcm_idle,adpcm_data;

static VIDEO_START(chinsan)
{
}

static VIDEO_UPDATE(chinsan)
{
	int y,x, count;
	count = 0;
	for(y=0;y<32;y++)
	{
		for (x=0;x<64;x++)
		{
			int tileno,colour;
			tileno = chinsan_video[count] | (chinsan_video[count+0x800]<<8);
			colour = chinsan_video[count+0x1000]>>3;
			drawgfx_opaque(bitmap,cliprect,screen->machine->gfx[0],tileno,colour,0,0,x*8,y*8);
			count++;
		}
	}

	return 0;
}



static MACHINE_RESET( chinsan )
{
	memory_configure_bank(machine, "bank1", 0, 4, memory_region(machine, "maincpu") + 0x10000, 0x4000);

	adpcm_idle = 1;
}


static WRITE8_HANDLER(ctrl_w)
{
	memory_set_bank(space->machine, "bank1", data >> 6);
}

static WRITE8_DEVICE_HANDLER( ym_port_w1 )
{
	logerror("ym_write port 1 %02x\n",data);
}


static WRITE8_DEVICE_HANDLER( ym_port_w2 )
{
	logerror("ym_write port 2 %02x\n",data);
}


static const ym2203_interface ym2203_config =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_INPUT_PORT("DSW1"),
		DEVCB_INPUT_PORT("DSW2"),
		DEVCB_HANDLER(ym_port_w1),
		DEVCB_HANDLER(ym_port_w2)
	},
};

static WRITE8_HANDLER( chinsan_port00_w )
{

	chinsan_port_select = data;

	if (
	   (data!=0x40) &&
	   (data!=0x4f) &&
	   (data!=0x53) &&
	   (data!=0x57) &&
	   (data!=0x5b) &&
	   (data!=0x5d) &&
	   (data!=0x5e))
		logerror("write port 00 %02x\n",data);

}

static READ8_HANDLER( chinsan_input_port_0_r )
{

	//return 0xff; // the inputs don't seem to work, so just return ff for now

	switch (chinsan_port_select)
	{
		/* i doubt these are both really the same.. */
		case 0x40:
		case 0x4f:
			return input_port_read(space->machine, "MAHJONG_P2_1");

		case 0x53:
			return input_port_read(space->machine, "MAHJONG_P2_2");

		case 0x57:
			return input_port_read(space->machine, "MAHJONG_P2_3");

		case 0x5b:
			return input_port_read(space->machine, "MAHJONG_P2_4");

		case 0x5d:
			return input_port_read(space->machine, "MAHJONG_P2_5");

		case 0x5e:
			return input_port_read(space->machine, "MAHJONG_P2_6");
	}

	printf("chinsan_input_port_0_r unk_r %02x\n", chinsan_port_select);
	return mame_rand(space->machine);
}

static READ8_HANDLER( chinsan_input_port_1_r )
{
	switch (chinsan_port_select)
	{
		/* i doubt these are both really the same.. */
		case 0x40:
		case 0x4f:
			return input_port_read(space->machine, "MAHJONG_P1_1");

		case 0x53:
			return input_port_read(space->machine, "MAHJONG_P1_2");

		case 0x57:
			return input_port_read(space->machine, "MAHJONG_P1_3");

		case 0x5b:
			return input_port_read(space->machine, "MAHJONG_P1_4");

		case 0x5d:
			return input_port_read(space->machine, "MAHJONG_P1_5");

		case 0x5e:
			return input_port_read(space->machine, "MAHJONG_P1_6");
	}

	printf("chinsan_input_port_1_r unk_r %02x\n", chinsan_port_select);
	return mame_rand(space->machine);
}

static WRITE8_DEVICE_HANDLER( chin_adpcm_w )
{
	adpcm_pos = (data & 0xff) * 0x100;
	adpcm_idle = 0;
	msm5205_reset_w(device,0);
}

static ADDRESS_MAP_START( chinsan_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xf7ff) AM_RAM AM_BASE(&chinsan_video)
ADDRESS_MAP_END

static ADDRESS_MAP_START( chinsan_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(chinsan_port00_w)
	AM_RANGE(0x01, 0x01) AM_READ(chinsan_input_port_0_r)
	AM_RANGE(0x02, 0x02) AM_READ(chinsan_input_port_1_r)
	AM_RANGE(0x10, 0x11) AM_DEVREADWRITE("ymsnd", ym2203_r, ym2203_w)
	AM_RANGE(0x20, 0x20) AM_DEVWRITE("adpcm", chin_adpcm_w)
	AM_RANGE(0x30, 0x30) AM_WRITE(ctrl_w)	// ROM bank + unknown stuff (input mutliplex?)
ADDRESS_MAP_END


static INPUT_PORTS_START( chinsan )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
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
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
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
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )


	PORT_START("MAHJONG_P1_1")
	PORT_DIPNAME( 0x01, 0x01, "1-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "P1 FF" ) // labaled FF in test mode, is this coin1?
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
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) // adds coins?, but maybe its the service switch?

	PORT_START("MAHJONG_P1_2")
	PORT_DIPNAME( 0x01, 0x01, "1-2" )
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
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("MAHJONG_P1_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?

	PORT_START("MAHJONG_P1_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?

	PORT_START("MAHJONG_P1_5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?

	PORT_START("MAHJONG_P1_6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?

	PORT_START("MAHJONG_P2_1")
	PORT_DIPNAME( 0x01, 0x01, "2-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "P2 FF" ) // labaled FF in test mode, is this coin2 or some other button ?
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Test ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("MAHJONG_P2_2")
	PORT_DIPNAME( 0x01, 0x01, "2-2" )
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
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("MAHJONG_P2_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?

	PORT_START("MAHJONG_P2_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?

	PORT_START("MAHJONG_P2_5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?

	PORT_START("MAHJONG_P2_6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?
INPUT_PORTS_END



static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( chinsan )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 32 )
GFXDECODE_END

static void chin_adpcm_int(const device_config *device)
{
	static UINT8 trigger;

	if (adpcm_pos >= 0x10000 || adpcm_idle)
	{
		//adpcm_idle = 1;
		msm5205_reset_w(device,1);
		trigger = 0;
	}
	else
	{
		UINT8 *ROM = memory_region(device->machine, "adpcm");

		adpcm_data = ((trigger ? (ROM[adpcm_pos] & 0x0f) : (ROM[adpcm_pos] & 0xf0)>>4) );
		msm5205_data_w(device,adpcm_data & 0xf);
		trigger^=1;
		if(trigger == 0)
		{
			adpcm_pos++;
			if((ROM[adpcm_pos] & 0xff) == 0x70)
				adpcm_idle = 1;
		}
	}
}

static const msm5205_interface msm5205_config =
{
	chin_adpcm_int,	/* interrupt function */
	MSM5205_S64_4B	/* 8kHz */
};

static PALETTE_INIT( chinsan )
{
	UINT8 *src = memory_region( machine, "color_proms" );
	int i;

	for (i=0;i<0x100;i++)
	{
		palette_set_color_rgb(machine,i,pal4bit(src[i+0x200]),pal4bit(src[i+0x100]),pal4bit(src[i+0x000]));
	}
}

static MACHINE_DRIVER_START( chinsan )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,10000000/2)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(chinsan_map)
	MDRV_CPU_IO_MAP(chinsan_io)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_MACHINE_RESET( chinsan )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_VISIBLE_AREA(24, 512-24-1, 16, 256-16-1)

	MDRV_GFXDECODE(chinsan)
	MDRV_PALETTE_LENGTH(0x100)
	MDRV_PALETTE_INIT(chinsan)

	MDRV_VIDEO_START(chinsan)
	MDRV_VIDEO_UPDATE(chinsan)

	// sound hardware
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2203, 1500000) /* ? Mhz */
	MDRV_SOUND_CONFIG(ym2203_config)
	MDRV_SOUND_ROUTE(0, "mono", 0.15)
	MDRV_SOUND_ROUTE(1, "mono", 0.15)
	MDRV_SOUND_ROUTE(2, "mono", 0.15)
	MDRV_SOUND_ROUTE(3, "mono", 0.10)

	MDRV_SOUND_ADD("adpcm", MSM5205, 384000)
	MDRV_SOUND_CONFIG(msm5205_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END



ROM_START( chinsan )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* encrypted code / data */
	ROM_LOAD( "mm00.7d", 0x00000, 0x08000, CRC(f7a4414f) SHA1(f65223b2928f610ab97fda2f2c008806cf2420e5) )
	ROM_CONTINUE(        0x00000, 0x08000 )	// first half is blank
	ROM_LOAD( "mm01.8d", 0x10000, 0x10000, CRC(c69ddbf5) SHA1(9533365c1761b113174d53a2e23ce6a7baca7dfe) )

	ROM_REGION( 0x2000, "user1", 0 ) /* MC8123 key */
	ROM_LOAD( "317-5012.key",  0x0000, 0x2000, CRC(2ecfb132) SHA1(3110ef82080dd7d908cc6bf34c6643f187f90b29) )

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "mm20.7k", 0x00000, 0x10000, CRC(54efb409) SHA1(333adadd7f3dc3393dbe334303bae544b3d26c00) )
	ROM_LOAD( "mm21.8k", 0x10000, 0x10000, CRC(25f6c827) SHA1(add72a3cfa2f24105e36d0464c2db6a6bedd4139) )
	ROM_LOAD( "mm22.9k", 0x20000, 0x10000, CRC(6092f6e1) SHA1(32f53027dc954e314d7c5d04ff53f17358bbcf77) )

	ROM_REGION( 0x10000, "adpcm", 0 ) /* M5205 samples */
	ROM_LOAD( "mm40.13d", 0x00000, 0x10000, CRC(a408b8f7) SHA1(60a2644922cb60c0a1a3409761c7e50924360313) )

	ROM_REGION( 0x20, "user2", 0 )
	ROM_LOAD( "mm60.2c", 0x000, 0x020, CRC(88477178) SHA1(03c1c9e3e88a5ae9970cb4b872ad4b6e4d77a6da) )

	ROM_REGION( 0x300, "color_proms", 0 )
	ROM_LOAD( "mm61.9m",  0x000, 0x100, CRC(57024262) SHA1(e084e6baa3c529217f6f8e37c9dd5f0687ba2fc4) ) // b
	ROM_LOAD( "mm62.9n",  0x100, 0x100, CRC(b5a1dbe5) SHA1(770a791c061ce422f860bb8d32f82bbbf9b4d12a) ) // g
	ROM_LOAD( "mm63.10n", 0x200, 0x100, CRC(b65e3567) SHA1(f146af51dfaa5b4bf44c4e27f1a0292f8fd07ce9) ) // r
ROM_END


static DRIVER_INIT( chinsan )
{
	mc8123_decrypt_rom(machine, "maincpu", "user1", "bank1", 4);
}


GAME( 1987, chinsan,  0,    chinsan, chinsan, chinsan, ROT0, "Sanritsu", "Ganbare Chinsan Ooshoubu (MC-8123A, 317-5012)", 0 )
