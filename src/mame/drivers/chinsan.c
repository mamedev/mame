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

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/mc8123.h"
#include "sound/2203intf.h"
#include "sound/msm5205.h"

class chinsan_state : public driver_device
{
public:
	chinsan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_video(*this, "video"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_video;

	/* misc */
	UINT8    m_port_select;
	UINT32   m_adpcm_pos;
	UINT8    m_adpcm_idle;
	UINT8	 m_adpcm_data;
	UINT8    m_trigger;
	DECLARE_WRITE8_MEMBER(ctrl_w);
	DECLARE_WRITE8_MEMBER(chinsan_port00_w);
	DECLARE_READ8_MEMBER(chinsan_input_port_0_r);
	DECLARE_READ8_MEMBER(chinsan_input_port_1_r);
};


/*************************************
 *
 *  Video emulation
 *
 *************************************/

static PALETTE_INIT( chinsan )
{
	UINT8 *src = machine.root_device().memregion( "color_proms" )->base();
	int i;

	for (i = 0; i < 0x100; i++)
		palette_set_color_rgb(machine, i, pal4bit(src[i + 0x200]), pal4bit(src[i + 0x100]), pal4bit(src[i + 0x000]));
}

static VIDEO_START( chinsan )
{
}

static SCREEN_UPDATE_IND16( chinsan )
{
	chinsan_state *state = screen.machine().driver_data<chinsan_state>();
	int y, x, count;
	count = 0;
	for (y = 0; y < 32; y++)
	{
		for (x = 0; x < 64; x++)
		{
			int tileno, colour;
			tileno = state->m_video[count] | (state->m_video[count + 0x800] << 8);
			colour = state->m_video[count + 0x1000] >> 3;
			drawgfx_opaque(bitmap,cliprect,screen.machine().gfx[0],tileno,colour,0,0,x*8,y*8);
			count++;
		}
	}

	return 0;
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_MEMBER(chinsan_state::ctrl_w)
{
	membank("bank1")->set_entry(data >> 6);
}

static WRITE8_DEVICE_HANDLER( ym_port_w1 )
{
	logerror("ym_write port 1 %02x\n", data);
}


static WRITE8_DEVICE_HANDLER( ym_port_w2 )
{
	logerror("ym_write port 2 %02x\n", data);
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

WRITE8_MEMBER(chinsan_state::chinsan_port00_w)
{

	m_port_select = data;

	if (
	   (data != 0x40) &&
	   (data != 0x4f) &&
	   (data != 0x53) &&
	   (data != 0x57) &&
	   (data != 0x5b) &&
	   (data != 0x5d) &&
	   (data != 0x5e))
		logerror("write port 00 %02x\n", data);

}

READ8_MEMBER(chinsan_state::chinsan_input_port_0_r)
{

	//return 0xff; // the inputs don't seem to work, so just return ff for now

	switch (m_port_select)
	{
		/* i doubt these are both really the same.. */
		case 0x40:
		case 0x4f:
			return ioport("MAHJONG_P2_1")->read();

		case 0x53:
			return ioport("MAHJONG_P2_2")->read();

		case 0x57:
			return ioport("MAHJONG_P2_3")->read();

		case 0x5b:
			return ioport("MAHJONG_P2_4")->read();

		case 0x5d:
			return ioport("MAHJONG_P2_5")->read();

		case 0x5e:
			return ioport("MAHJONG_P2_6")->read();
	}

	printf("chinsan_input_port_0_r unk_r %02x\n", m_port_select);
	return machine().rand();
}

READ8_MEMBER(chinsan_state::chinsan_input_port_1_r)
{

	switch (m_port_select)
	{
		/* i doubt these are both really the same.. */
		case 0x40:
		case 0x4f:
			return ioport("MAHJONG_P1_1")->read();

		case 0x53:
			return ioport("MAHJONG_P1_2")->read();

		case 0x57:
			return ioport("MAHJONG_P1_3")->read();

		case 0x5b:
			return ioport("MAHJONG_P1_4")->read();

		case 0x5d:
			return ioport("MAHJONG_P1_5")->read();

		case 0x5e:
			return ioport("MAHJONG_P1_6")->read();
	}

	printf("chinsan_input_port_1_r unk_r %02x\n", m_port_select);
	return machine().rand();
}

static WRITE8_DEVICE_HANDLER( chin_adpcm_w )
{
	chinsan_state *state = device->machine().driver_data<chinsan_state>();
	state->m_adpcm_pos = (data & 0xff) * 0x100;
	state->m_adpcm_idle = 0;
	msm5205_reset_w(device, 0);
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( chinsan_map, AS_PROGRAM, 8, chinsan_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xf7ff) AM_RAM AM_SHARE("video")
ADDRESS_MAP_END

static ADDRESS_MAP_START( chinsan_io, AS_IO, 8, chinsan_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(chinsan_port00_w)
	AM_RANGE(0x01, 0x01) AM_READ(chinsan_input_port_0_r)
	AM_RANGE(0x02, 0x02) AM_READ(chinsan_input_port_1_r)
	AM_RANGE(0x10, 0x11) AM_DEVREADWRITE_LEGACY("ymsnd", ym2203_r, ym2203_w)
	AM_RANGE(0x20, 0x20) AM_DEVWRITE_LEGACY("adpcm", chin_adpcm_w)
	AM_RANGE(0x30, 0x30) AM_WRITE(ctrl_w)	// ROM bank + unknown stuff (input mutliplex?)
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

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



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

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

/*************************************
 *
 *  Sound interface
 *
 *************************************/

static void chin_adpcm_int( device_t *device )
{
	chinsan_state *state = device->machine().driver_data<chinsan_state>();

	if (state->m_adpcm_pos >= 0x10000 || state->m_adpcm_idle)
	{
		//state->m_adpcm_idle = 1;
		msm5205_reset_w(device, 1);
		state->m_trigger = 0;
	}
	else
	{
		UINT8 *ROM = device->machine().root_device().memregion("adpcm")->base();

		state->m_adpcm_data = ((state->m_trigger ? (ROM[state->m_adpcm_pos] & 0x0f) : (ROM[state->m_adpcm_pos] & 0xf0) >> 4));
		msm5205_data_w(device, state->m_adpcm_data & 0xf);
		state->m_trigger ^= 1;
		if(state->m_trigger == 0)
		{
			state->m_adpcm_pos++;
			if ((ROM[state->m_adpcm_pos] & 0xff) == 0x70)
				state->m_adpcm_idle = 1;
		}
	}
}

static const msm5205_interface msm5205_config =
{
	chin_adpcm_int,	/* interrupt function */
	MSM5205_S64_4B	/* 8kHz */
};

/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( chinsan )
{
	chinsan_state *state = machine.driver_data<chinsan_state>();

	state->membank("bank1")->configure_entries(0, 4, state->memregion("maincpu")->base() + 0x10000, 0x4000);

	state->save_item(NAME(state->m_adpcm_idle));
	state->save_item(NAME(state->m_port_select));
	state->save_item(NAME(state->m_adpcm_pos));
	state->save_item(NAME(state->m_adpcm_data));
	state->save_item(NAME(state->m_trigger));
}

static MACHINE_RESET( chinsan )
{
	chinsan_state *state = machine.driver_data<chinsan_state>();

	state->m_adpcm_idle = 1;
	state->m_port_select = 0;
	state->m_adpcm_pos = 0;
	state->m_adpcm_data = 0;
	state->m_trigger = 0;
}


static MACHINE_CONFIG_START( chinsan, chinsan_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,10000000/2)		 /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(chinsan_map)
	MCFG_CPU_IO_MAP(chinsan_io)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	MCFG_MACHINE_START( chinsan )
	MCFG_MACHINE_RESET( chinsan )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_VISIBLE_AREA(24, 512-24-1, 16, 256-16-1)
	MCFG_SCREEN_UPDATE_STATIC(chinsan)

	MCFG_GFXDECODE(chinsan)
	MCFG_PALETTE_LENGTH(0x100)
	MCFG_PALETTE_INIT(chinsan)

	MCFG_VIDEO_START(chinsan)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, 1500000) /* ? Mhz */
	MCFG_SOUND_CONFIG(ym2203_config)
	MCFG_SOUND_ROUTE(0, "mono", 0.15)
	MCFG_SOUND_ROUTE(1, "mono", 0.15)
	MCFG_SOUND_ROUTE(2, "mono", 0.15)
	MCFG_SOUND_ROUTE(3, "mono", 0.10)

	MCFG_SOUND_ADD("adpcm", MSM5205, 384000)
	MCFG_SOUND_CONFIG(msm5205_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

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


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static DRIVER_INIT( chinsan )
{
	mc8123_decrypt_rom(machine, "maincpu", "user1", "bank1", 4);
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1987, chinsan,  0,    chinsan, chinsan, chinsan, ROT0, "Sanritsu", "Ganbare Chinsan Ooshoubu (MC-8123A, 317-5012)", GAME_SUPPORTS_SAVE )
