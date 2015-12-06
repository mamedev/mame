// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
/*
 'Swinging Singles' US distribution by Ent. Ent. Ltd
 Original Japan release is 'Utamaro' by 'Yachiyo' (undumped!)
 driver by Tomasz Slanina


 Crap XXX game.
 Three roms contains text "BY YACHIYO"

 Upper half of 7.bin = upper half of 8.bin = intentional or bad dump ?

 TODO:
 - atamanot: needs a trojan, in order to understand how the protection really works.
 - colors (missing prom(s) ?)
 - samples (at least two of unused roms contains samples (unkn. format , adpcm ?)
 - dips (one is tested in game (difficulty related?), another 2 are tested at start)

 Unknown reads/writes:
 - AY i/o ports (writes)
 - mem $c000, $c001 = protection device ? if tests fails, game crashes (problems with stack - skipped code with "pop af")
 - i/o port $8 = data read used for  $e command arg for one of AY chips (volume? - could be a sample player (based on volume changes?)
 - i/o port $1a = 1 or 0, rarely accessed, related to crt  writes

==================================================================

Computer Quiz Atama no Taisou
(c)1983 Yachiyo Denki / Uni Enterprize

-----------------------------------------
TZU-093
CPU: Z80(IC9) surface scrached Z80?(IC28)
Sound: AY-3-8910 x2
OSC: 14.000MHz
RAMs:
IC5 (probably 6116)
IC4 (probably 6116)
IC45 (probably 6116)
IC44 (probably 6116)

M5L5101LP-1(IC19,20,21,22)
-----------------------------------------
ROMs:
TT1.2        [da9e270d] (2764)
TT2.3        [7595ade8] /

CA.49        [28d20b52] (2764)
CC.48        [209cab0d]  |
CB.47        [8bc85c0c]  |
CD.46        [22e8d103] /

IC36.BIN     [643e3077] (surface scrached, 27C256)
IC35.BIN     [fe0302a0]  |
IC34.BIN     [06e7c7da]  |
IC33.BIN     [323a70e7] /

color PROMs:
1.52         [13f5762b] (82S129)
2.53         [4142f525]  |
3.54         [88acb21e] /


-----------------------------------------
Sub board
CQM-082-M
RAM:
HM6116LP-3
Other:
Battery
8-position DIPSW
-----------------------------------------
ROMs:
TA.BIN       [5c61edaf] (2764)
TB.BIN       [07bd2e6f]  |
TC.BIN       [1e09ac09]  |
TD.BIN       [bd514d51]  |
TE.BIN       [825ed49f]  |
TF.BIN       [d92b5eb9]  |
TG.BIN       [eb25aa72]  |
TH.BIN       [13396cfb] /

TI.BIN       [60193df3] (2764)
J.BIN        [cd16ddbf]  |
K.BIN        [c75c7a1e]  |
L.BIN        [dbb4ed60] /


/////////////////////////////////////////
DIPSW
1: off
2: off
3: coinage
    on: 1coin 2credits
   off: 1coin 1credit
4: Note
    on: show notes
   off: game
5: Test mode
    on: test
   off: game
6: Attract sound
    on: no
   off: yes
7-8: Timer setting
   off-off: quickest
    on-off: quick
   off- on: slow
    on- on: slowest
(default settings: all off)


wiring
GND(sw)   A01|B01 GND(speaker)
GND(power)A02|B02 GND(power)
+5V       A03|B03 +5V
--------  A04|B04 +5V(coin counter)
+12V      A05|B05 +12V
--------  A06|B06 speaker

    (A7-A15, B7-B15: NC)

2P genre1 A16|B16 1P genre1
2P genre2 A17|B17 1P genre2
2P genre3 A18|B18 1P genre3
2P genre4 A19|B19 1P genre4
2P push A A20|B20 1P push A
2P push B A21|B21 1P push B
2P push C A22|B22 1P push C
Flip/flop A23|B23 Flip/flop
--------  A24|B24 --------
GREEN     A25|B25 BLUE
RED       A26|B26 SYNC
GND       A27|B27 GND
GND(video)A28|B28 coin sw


/////////////////////////////////////////



--- Team Japump!!! ---
Dumped by Chack'n
04/Nov/2009


*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

#define NUM_PENS (4*8)
#define VMEM_SIZE 0x100

class ssingles_state : public driver_device
{
public:
	ssingles_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	UINT8 m_videoram[VMEM_SIZE];
	UINT8 m_colorram[VMEM_SIZE];
	UINT8 m_prot_data;
	pen_t m_pens[NUM_PENS];

	UINT8 m_atamanot_prot_state;
	DECLARE_WRITE8_MEMBER(ssingles_videoram_w);
	DECLARE_WRITE8_MEMBER(ssingles_colorram_w);
	DECLARE_READ8_MEMBER(c000_r);
	DECLARE_READ8_MEMBER(c001_r);
	DECLARE_WRITE8_MEMBER(c001_w);
	DECLARE_READ8_MEMBER(atamanot_prot_r);
	DECLARE_WRITE8_MEMBER(atamanot_prot_w);
	DECLARE_CUSTOM_INPUT_MEMBER(controls_r);
	DECLARE_DRIVER_INIT(ssingles);
	virtual void video_start() override;
	INTERRUPT_GEN_MEMBER(atamanot_irq);
	MC6845_UPDATE_ROW(ssingles_update_row);
	MC6845_UPDATE_ROW(atamanot_update_row);
	required_device<cpu_device> m_maincpu;
};

//fake palette
static const UINT8 ssingles_colors[NUM_PENS*3]=
{
	0x00,0x00,0x00, 0xff,0xff,0xff, 0xff,0x00,0x00, 0x80,0x00,0x00,
	0x00,0x00,0x00, 0xf0,0xf0,0xf0, 0xff,0xff,0x00, 0x40,0x40,0x40,
	0x00,0x00,0x00, 0xff,0xff,0xff, 0xff,0x00,0x00, 0xff,0xff,0x00,
	0x00,0x00,0x00, 0xff,0xff,0x00, 0xd0,0x00,0x00, 0x80,0x00,0x00,
	0x00,0x00,0x00, 0xff,0x00,0x00, 0xff,0xff,0x00, 0x80,0x80,0x00,
	0x00,0x00,0x00, 0xff,0x00,0x00, 0x40,0x40,0x40, 0xd0,0xd0,0xd0,
	0x00,0x00,0x00, 0x00,0x00,0xff, 0x60,0x40,0x30, 0xff,0xff,0x00,
	0x00,0x00,0x00, 0xff,0x00,0xff, 0x80,0x00,0x80, 0x40,0x00,0x40
};

MC6845_UPDATE_ROW( ssingles_state::ssingles_update_row )
{
	UINT32 tile_address;
	UINT16 cell, palette;
	UINT8 b0, b1;
	const UINT8 *gfx = memregion("gfx1")->base();

	for (int cx = 0; cx < x_count; ++cx)
	{
		int address = ((ma >> 1) + (cx >> 1)) & 0xff;

		cell = m_videoram[address] + (m_colorram[address] << 8);

		tile_address = ((cell & 0x3ff) << 4) + ra;
		palette = (cell >> 10) & 0x1c;

		if (cx & 1)
		{
			b0 = gfx[tile_address + 0x0000]; /*  9.bin */
			b1 = gfx[tile_address + 0x8000]; /* 11.bin */
		}
		else
		{
			b0 = gfx[tile_address + 0x4000]; /* 10.bin */
			b1 = gfx[tile_address + 0xc000]; /* 12.bin */
		}

		for (int x = 7; x >= 0; --x)
		{
			bitmap.pix32(y, (cx << 3) | x) = m_pens[palette + ((b1 & 1) | ((b0 & 1) << 1))];
			b0 >>= 1;
			b1 >>= 1;
		}
	}
}

MC6845_UPDATE_ROW( ssingles_state::atamanot_update_row )
{
	UINT32 tile_address;
	UINT16 cell, palette;
	UINT8 b0, b1;
	const UINT8 *gfx = memregion("gfx1")->base();

	for (int cx = 0; cx < x_count; ++cx)
	{
		int address = ((ma >> 1) + (cx >> 1)) & 0xff;

		cell = m_videoram[address] + (m_colorram[address] << 8);

		tile_address = ((cell & 0x1ff) << 4) + ra;
		palette = (cell >> 10) & 0x1c;

		if (cx & 1)
		{
			b0 = gfx[tile_address + 0x0000]; /*  9.bin */
			b1 = gfx[tile_address + 0x4000]; /* 11.bin */
		}
		else
		{
			b0 = gfx[tile_address + 0x2000]; /* 10.bin */
			b1 = gfx[tile_address + 0x6000]; /* 12.bin */
		}

		for (int x = 7; x >= 0; --x)
		{
			bitmap.pix32(y, (cx << 3) | x) = m_pens[palette + ((b1 & 1) | ((b0 & 1) << 1))];
			b0 >>= 1;
			b1 >>= 1;
		}
	}
}


WRITE8_MEMBER(ssingles_state::ssingles_videoram_w)
{
	UINT8 *vram = memregion("vram")->base();
	vram[offset] = data;
	m_videoram[offset]=data;
}

WRITE8_MEMBER(ssingles_state::ssingles_colorram_w)
{
	UINT8 *cram = memregion("cram")->base();
	cram[offset] = data;
	m_colorram[offset]=data;
}


void ssingles_state::video_start()
{
	{
		int i;
		for(i=0;i<NUM_PENS;++i)
		{
			m_pens[i]=rgb_t(ssingles_colors[3*i], ssingles_colors[3*i+1], ssingles_colors[3*i+2]);
		}
	}
}


READ8_MEMBER(ssingles_state::c000_r)
{
	return m_prot_data;
}

READ8_MEMBER(ssingles_state::c001_r)
{
	m_prot_data=0xc4;
	return 0;
}

WRITE8_MEMBER(ssingles_state::c001_w)
{
	m_prot_data^=data^0x11;
}

CUSTOM_INPUT_MEMBER(ssingles_state::controls_r)
{
	int data = 7;
	switch(ioport("EXTRA")->read())     //multiplexed
	{
		case 0x01: data = 1; break;
		case 0x02: data = 2; break;
		case 0x04: data = 3; break;
		case 0x08: data = 4; break;
		case 0x10: data = 5; break;
		case 0x20: data = 6; break;
		case 0x40: data = 0; break;
	}

	return data;
}

static ADDRESS_MAP_START( ssingles_map, AS_PROGRAM, 8, ssingles_state )
	AM_RANGE(0x0000, 0x00ff) AM_WRITE(ssingles_videoram_w)
	AM_RANGE(0x0800, 0x08ff) AM_WRITE(ssingles_colorram_w)
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READ(c000_r )
	AM_RANGE(0xc001, 0xc001) AM_READWRITE(c001_r, c001_w )
	AM_RANGE(0x6000, 0xbfff) AM_ROM
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END


READ8_MEMBER(ssingles_state::atamanot_prot_r)
{
	static const char prot_id[] = { "PROGRAM BY KOYAMA" };

	logerror("%04x %02x\n",offset,m_atamanot_prot_state);

	switch(m_atamanot_prot_state)
	{
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
			return prot_id[offset % 0x11];

		case 0xc0:
			return 2; // 1 goes to service mode?
	}

	return 0;
}

WRITE8_MEMBER(ssingles_state::atamanot_prot_w)
{
	m_atamanot_prot_state = data;
}


static ADDRESS_MAP_START( atamanot_map, AS_PROGRAM, 8, ssingles_state )
	AM_RANGE(0x0000, 0x00ff) AM_WRITE(ssingles_videoram_w)
	AM_RANGE(0x0800, 0x08ff) AM_WRITE(ssingles_colorram_w)
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x6000, 0x60ff) AM_RAM //kanji tilemap?
//  AM_RANGE(0x6000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_READ(atamanot_prot_r)
//  AM_RANGE(0x8000, 0x9fff) AM_ROM AM_REGION("question",0x10000)
//  AM_RANGE(0xc000, 0xc000) AM_READ(c000_r )
//  AM_RANGE(0xc001, 0xc001) AM_READWRITE(c001_r, c001_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( ssingles_io_map, AS_IO, 8, ssingles_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("ay1", ay8910_device, address_w)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("ay1", ay8910_device, data_w)
	AM_RANGE(0x06, 0x06) AM_DEVWRITE("ay2", ay8910_device, address_w)
	AM_RANGE(0x08, 0x08) AM_READNOP
	AM_RANGE(0x0a, 0x0a) AM_DEVWRITE("ay2", ay8910_device, data_w)
	AM_RANGE(0x16, 0x16) AM_READ_PORT("DSW0")
	AM_RANGE(0x18, 0x18) AM_READ_PORT("DSW1")
	AM_RANGE(0x1c, 0x1c) AM_READ_PORT("INPUTS")
//  AM_RANGE(0x1a, 0x1a) AM_WRITENOP //video/crt related
	AM_RANGE(0xfe, 0xfe) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xff, 0xff) AM_DEVWRITE("crtc", mc6845_device, register_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( atamanot_io_map, AS_IO, 8, ssingles_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("ay1", ay8910_device, address_w)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("ay1", ay8910_device, data_w)
	AM_RANGE(0x06, 0x06) AM_DEVWRITE("ay2", ay8910_device, address_w)
	AM_RANGE(0x08, 0x08) AM_READNOP
	AM_RANGE(0x0a, 0x0a) AM_DEVWRITE("ay2", ay8910_device, data_w)
	AM_RANGE(0x16, 0x16) AM_READ_PORT("DSW0")
	AM_RANGE(0x18, 0x18) AM_READ_PORT("DSW1") AM_WRITE(atamanot_prot_w)
	AM_RANGE(0x1c, 0x1c) AM_READ_PORT("INPUTS")
//  AM_RANGE(0x1a, 0x1a) AM_WRITENOP //video/crt related
	AM_RANGE(0xfe, 0xfe) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xff, 0xff) AM_DEVWRITE("crtc", mc6845_device, register_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( ssingles )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) //must be LOW
	PORT_BIT( 0x1c, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, ssingles_state,controls_r, NULL)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 )

	PORT_START("EXTRA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x04, 0x04, "Unk1" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Unk2" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Unk3" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Unk4" ) //tested in game, every frame, could be difficulty related
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Unk5" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Unk 6" )
	PORT_DIPSETTING(    0x01, "Pos 1" )
	PORT_DIPSETTING(    0x03, "Pos 2" )
	PORT_DIPSETTING(    0x00, "Pos 3" )
	PORT_DIPSETTING(    0x02, "Pos 4" )
	PORT_DIPNAME( 0x04, 0x04, "Unk7" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Unk8" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Unk9" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "UnkA" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "UnkB" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "UnkC" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
INPUT_PORTS_END

/*
atamanot kanji gfx decoding:

It looks "stolen" from an unknown Japanese computer?
*/

static const gfx_layout layout_8x8 =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};


static const gfx_layout layout_16x16 =
{
	16,16,
	RGN_FRAC(1,4),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		RGN_FRAC(1,4)+0, RGN_FRAC(1,4)+1, RGN_FRAC(1,4)+2, RGN_FRAC(1,4)+3, RGN_FRAC(1,4)+4, RGN_FRAC(1,4)+5, RGN_FRAC(1,4)+6, RGN_FRAC(1,4)+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		RGN_FRAC(2,4)+0*8, RGN_FRAC(2,4)+1*8, RGN_FRAC(2,4)+2*8, RGN_FRAC(2,4)+3*8, RGN_FRAC(2,4)+4*8, RGN_FRAC(2,4)+5*8, RGN_FRAC(2,4)+6*8, RGN_FRAC(2,4)+7*8 },
	8*8
};


static const gfx_layout layout_8x16 =
{
	8,16,
	RGN_FRAC(1,2),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		RGN_FRAC(2,4)+0*8, RGN_FRAC(2,4)+1*8, RGN_FRAC(2,4)+2*8, RGN_FRAC(2,4)+3*8, RGN_FRAC(2,4)+4*8, RGN_FRAC(2,4)+5*8, RGN_FRAC(2,4)+6*8, RGN_FRAC(2,4)+7*8 },
	8*8
};

static GFXDECODE_START( ssingles )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8, 0, 8 )
GFXDECODE_END

static GFXDECODE_START( atamanot )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8, 0, 8 )
	GFXDECODE_ENTRY( "kanji", 0, layout_16x16,     0, 8 )
	GFXDECODE_ENTRY( "kanji_uc", 0, layout_8x16,     0, 8 )
	GFXDECODE_ENTRY( "kanji_lc", 0, layout_8x16,     0, 8 )
GFXDECODE_END

static MACHINE_CONFIG_START( ssingles, ssingles_state )

	MCFG_CPU_ADD("maincpu", Z80,4000000)         /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(ssingles_map)
	MCFG_CPU_IO_MAP(ssingles_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", ssingles_state,  nmi_line_pulse)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(4000000, 256, 0, 256, 256, 0, 256)   /* temporary, CRTC will configure screen */
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)

	MCFG_PALETTE_ADD("palette", 4) //guess

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ssingles)

	MCFG_MC6845_ADD("crtc", MC6845, "screen", 1000000 /* ? MHz */)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(ssingles_state, ssingles_update_row)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 1500000) /* ? MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_SOUND_ADD("ay2", AY8910, 1500000) /* ? MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

MACHINE_CONFIG_END

INTERRUPT_GEN_MEMBER(ssingles_state::atamanot_irq)
{
	// ...
}

static MACHINE_CONFIG_DERIVED( atamanot, ssingles )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(atamanot_map)
	MCFG_CPU_IO_MAP(atamanot_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", ssingles_state,  atamanot_irq)

	MCFG_DEVICE_REMOVE("crtc")

	MCFG_MC6845_ADD("crtc", MC6845, "screen", 1000000 /* ? MHz */)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(ssingles_state, atamanot_update_row)

	MCFG_GFXDECODE_MODIFY("gfxdecode", atamanot)
MACHINE_CONFIG_END

ROM_START( ssingles )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 main CPU  */
	ROM_LOAD( "1.bin", 0x00000, 0x2000, CRC(43f02215) SHA1(9f04a7d4671ff39fd2bd8ec7afced4981ee7be05) )
	ROM_LOAD( "2.bin", 0x06000, 0x2000, CRC(281f27e4) SHA1(cef28717ab2ed991a5709464c01490f0ab1dc17c) )
	ROM_LOAD( "3.bin", 0x08000, 0x2000, CRC(14fdcb65) SHA1(70f7fcb46e74937de0e4037c9fe79349a30d0d07) )
	ROM_LOAD( "4.bin", 0x0a000, 0x2000, CRC(acb44685) SHA1(d68aab8b7e68d842a350d3fb76985ac857b1d972) )

	ROM_REGION( 0x100, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x100, "cram", ROMREGION_ERASE00 )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "9.bin",  0x0000, 0x4000, CRC(57fac6f9) SHA1(12f6695c9831399e599a95008ebf9db943725437) )
	ROM_LOAD( "10.bin", 0x4000, 0x4000, CRC(cd3ba260) SHA1(2499ad9982cc6356e2eb3a0f10d77886872a0c9f) )
	ROM_LOAD( "11.bin", 0x8000, 0x4000, CRC(f7107b29) SHA1(a405926fd3cb4b3d2a1c705dcde25d961dba5884) )
	ROM_LOAD( "12.bin", 0xc000, 0x4000, CRC(e5585a93) SHA1(04d55699b56d869066f2be2c6ac48042aa6c3108) )

	ROM_REGION( 0x08000, "user1", 0) /* samples ? data ?*/
	ROM_LOAD( "5.bin", 0x00000, 0x2000, CRC(242a8dda) SHA1(e140893cc05fb8cee75904d98b02626f2565ed1b) )
	ROM_LOAD( "6.bin", 0x02000, 0x2000, CRC(85ab8aab) SHA1(566f034e1ba23382442f27457447133a0e0f1cfc) )
	ROM_LOAD( "7.bin", 0x04000, 0x2000, CRC(57cc112d) SHA1(fc861c58ae39503497f04d302a9f16fca19b37fb) )
	ROM_LOAD( "8.bin", 0x06000, 0x2000, CRC(52de717a) SHA1(e60399355165fb46fac862fb7fcdff16ff351631) )

ROM_END


ROM_START( atamanot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tt1.2",   0x0000, 0x2000, CRC(da9e270d) SHA1(b7408be913dad8abf022c6153f2493204dd74952) )
	ROM_LOAD( "tt2.3",   0x2000, 0x2000, CRC(7595ade8) SHA1(71f9d6d987407f88cdd3b28bd1e35e00ac17e1f5) )

	ROM_REGION( 0x100, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x100, "cram", ROMREGION_ERASE00 )

	ROM_REGION( 0x18000, "question", 0 ) //question roms?
	ROM_LOAD( "ta.bin",  0x00000, 0x2000, CRC(5c61edaf) SHA1(ea56df6b320aa7e52828aaccbb5838cd0c756f24) )
	ROM_LOAD( "tb.bin",  0x02000, 0x2000, CRC(07bd2e6f) SHA1(bf245d8208db447572e484057b9daa6276f03683) )
	ROM_LOAD( "tc.bin",  0x04000, 0x2000, CRC(1e09ac09) SHA1(91ec1b2c5767b5bad8915f7c9984f423fcb399c9) )
	ROM_LOAD( "td.bin",  0x06000, 0x2000, CRC(bd514d51) SHA1(1a1e95558b2608f0103ca1b42fe9e59ccb90487f) )
	ROM_LOAD( "te.bin",  0x08000, 0x2000, CRC(825ed49f) SHA1(775044f6d53ecbfa0ab604947a21e368bad85ce0) )
	ROM_LOAD( "tf.bin",  0x0a000, 0x2000, CRC(d92b5eb9) SHA1(311fdefdc1f1026cb7f7cc1e1adaffbdbe7a70d9) )
	ROM_LOAD( "tg.bin",  0x0c000, 0x2000, CRC(eb25aa72) SHA1(de3a3d87a2eb540b96947f776c321dc9a7c21e78) )
	ROM_LOAD( "th.bin",  0x0e000, 0x2000, CRC(13396cfb) SHA1(d98ea4ff2e2175aa7003e37001664b3fa898c071) )
	ROM_LOAD( "ti.bin",  0x10000, 0x2000, CRC(60193df3) SHA1(58840bde303a760a0458224983af0c0bbe939a2f) )
	ROM_LOAD( "j.bin",   0x12000, 0x2000, CRC(cd16ddbf) SHA1(b418b5d73d3699697ebd42a6f4df598dcdcaf264) )
	ROM_LOAD( "k.bin",   0x14000, 0x2000, CRC(c75c7a1e) SHA1(59b136626267fa3ba5a2e1709acb632142e1560e) )
	ROM_LOAD( "l.bin",   0x16000, 0x2000, CRC(dbb4ed60) SHA1(b5054ba3ccd268594d22e1e67f70bb227095ca4c) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "ca.49",   0x0000, 0x2000, CRC(28d20b52) SHA1(a104ef1cd103f31803b88bd2d4804eab5a26e7fa) )
	ROM_LOAD( "cc.48",   0x2000, 0x2000, CRC(209cab0d) SHA1(9a89af1f7186e4845e43f9cdafd273e69d280bfb) )
	ROM_LOAD( "cb.47",   0x4000, 0x2000, CRC(8bc85c0c) SHA1(64701bc910c28666d15ee22f59f32888cc2302ae) )
	ROM_LOAD( "cd.46",   0x6000, 0x2000, CRC(22e8d103) SHA1(f0146f7e192eef8d03404a9c5b8a9f9c9577d936) )

	ROM_REGION( 0x20000, "kanji", 0 )
	ROM_LOAD( "ic36.bin",   0x18000, 0x8000, CRC(643e3077) SHA1(fa81a3a3eebd59c6dc9c9b7eeb4a480bb1440c17) )
	ROM_LOAD( "ic35.bin",   0x10000, 0x8000, CRC(fe0302a0) SHA1(f8d3a58da4e8dd09db240039f5216e7ebe9cc384) )
	ROM_LOAD( "ic34.bin",   0x08000, 0x8000, CRC(06e7c7da) SHA1(a222c0b0eccfda613f916320e6afbb33385921ba) )
	ROM_LOAD( "ic33.bin",   0x00000, 0x8000, CRC(323a70e7) SHA1(55e570f039c97d15b06bfcb1ebf03562cbcf8324) )

	ROM_REGION( 0x10000, "kanji_uc", 0 ) //upper case
	ROM_COPY( "kanji", 0x10000, 0x08000, 0x08000 )
	ROM_COPY( "kanji", nullptr, 0x00000, 0x08000 )

	ROM_REGION( 0x10000, "kanji_lc", 0 ) //lower case
	ROM_COPY( "kanji", 0x18000, 0x08000, 0x08000 )
	ROM_COPY( "kanji", 0x08000, 0x00000, 0x08000 )

	ROM_REGION( 0x0300,  "proms", 0 ) //NOT color proms
	ROM_LOAD( "1.52",   0x00000, 0x0100, CRC(13f5762b) SHA1(da9cc51eda0681b0d3c17b212d23ab89af2813ff) )
	ROM_LOAD( "2.53",   0x00100, 0x0100, CRC(4142f525) SHA1(2e2e896ba7b49df9cf3fddff6becc07a3d50d926) )
	ROM_LOAD( "3.54",   0x00200, 0x0100, CRC(88acb21e) SHA1(18fe5280dad6687daf6bf42d37dde45157fab5e3) )
ROM_END

DRIVER_INIT_MEMBER(ssingles_state,ssingles)
{
	save_item(NAME(m_videoram));
	save_item(NAME(m_colorram));
}

GAME( 1983, ssingles, 0, ssingles, ssingles, ssingles_state, ssingles, ROT90, "Yachiyo Denki (Entertainment Enterprises, Ltd. license)", "Swinging Singles (US)", MACHINE_SUPPORTS_SAVE | MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_SOUND )
GAME( 1983, atamanot, 0, atamanot, ssingles, ssingles_state, ssingles, ROT90, "Yachiyo Denki / Uni Enterprize", "Computer Quiz Atama no Taisou (Japan)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
