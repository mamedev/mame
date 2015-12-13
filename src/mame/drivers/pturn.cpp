// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina,Tatsuyuki Satoh
/*
Parallel Turn
(c) Jaleco, 1984
driver by Tomasz Slanina and Tatsuyuki Satoh

Custom Jaleco chip is some kind of state machine
used for calculate jump offsets.

Top PCB
-------

PCB No. PT-8418
CPU:  Z80A
SND:  AY-3-8010 x 2, Z80A
DIPS: 8 position x 2
RAM:  2114 x 2, MSM2128 x 1 (equivalent to 6116)

Other: Reset Switch near edge connector

       Custom JALECO chip (24 pin DIP) near RAM MSM2128, verified to be NOT 2128 ram.

       Pinouts are :
       Pin 1 hooked to pin 3 of ROM 7
       Pin 2 hooked to pin 4 of ROM 7
       Pin 3 hooked to pin 5 of ROM 7
       Pin 4 hooked to pin 6 of ROM 7
       Pin 5 hooked to pin 7 of ROM 7
       Pin 6 hooked to pin 8 of ROM 7
       Pin 7 hooked to pin 9 of ROM 7
       Pin 8 hooked to pin 10 of ROM 7
       Pin 9 hooked to pin 11 of ROM 7
       Pin 10 hooked to pin 12 of ROM 7
       Pin 11 hooked to pin 13 of ROM 7
       Pin 12 GND
       Pin 13 hooked to pin 13 of 2128 and pin 15 of ROM 7
       Pin 14 hooked to pin 14 of 2128 and pin 16 of ROM 7
       Pin 15 hooked to pin 17 of ROM 7
       Pin 16 hooked to pin 18 of ROM 7
       Pin 17 hooked to pin 19 of ROM 7
       Pin 18 NC
       Pin 19 NC
       Pin 20 hooked to pin 11 of 74LS32 at 4F
       Pin 21 hooked to pin 8 of 74LS32 at 4F
       Pin 22 hooked to pin 24 of ROM 7
       Pin 23 hooked to pin 25 of ROM 7
       Pin 24 +5

NOTE: The archive contains a different ROM7 that was in another archive.
      (I merged all archives since they are identical other than ROM 7 in one archive named 7.BIN)
      Perhaps this is from a bootleg PCB with a workaround for the custom JALECO chip?


ROMS: All ROM labels say only "PROM" and a number.
      1, 2, 3 and 9 near Z80 at 5K, all 2732's
      4, 5, 6 & 7 near custom JALECO chip and Z80 at 3D, all 2764's
      prom_red.3p type TBP24S10
      prom_grn.4p type N82S129
      prom_blu.4r type TBP24S10


LOWER PCB
---------

PCB:  PT-8419
XTAL: ? Stamped KSS5, connected to pins 8 & 13 of 74LS37 at 9P
      Also connected to 2 x 330 Ohm resistors which are in turn connected to pins
      9 & 11 of 9P. Pins 9 & 11 of 9P are joined with a 101 ceramic capacitor.
      UPDATE! When I substitute various speed xtals, a 8.000MHz xtal allows the PCB to work fine.

RAM:  AM93422DC x 2, MB7063 x 1, 2114 x 4

ROMS: All ROM labels say only "PROM" and a number.
      10, 14, 15 & 16 type 2764
      11, 12, 13 type 2732

*/
#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"


class pturn_state : public driver_device
{
public:
	pturn_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;

	tilemap_t *m_fgmap;
	tilemap_t *m_bgmap;
	int m_bgbank;
	int m_fgbank;
	int m_bgpalette;
	int m_fgpalette;
	int m_bgcolor;
	int m_nmi_main;
	int m_nmi_sub;

	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(nmi_main_enable_w);
	DECLARE_WRITE8_MEMBER(nmi_sub_enable_w);
	DECLARE_WRITE8_MEMBER(bgcolor_w);
	DECLARE_WRITE8_MEMBER(bg_scrollx_w);
	DECLARE_WRITE8_MEMBER(fgpalette_w);
	DECLARE_WRITE8_MEMBER(bg_scrolly_w);
	DECLARE_WRITE8_MEMBER(fgbank_w);
	DECLARE_WRITE8_MEMBER(bgbank_w);
	DECLARE_WRITE8_MEMBER(flip_w);
	DECLARE_READ8_MEMBER(custom_r);
	DECLARE_READ8_MEMBER(protection_r);
	DECLARE_READ8_MEMBER(protection2_r);

	TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	DECLARE_DRIVER_INIT(pturn);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(sub_intgen);
	INTERRUPT_GEN_MEMBER(main_intgen);
};




static const UINT8 tile_lookup[0x10]=
{
	0x00, 0x10, 0x40, 0x50,
	0x20, 0x30, 0x60, 0x70,
	0x80, 0x90, 0xc0, 0xd0,
	0xa0, 0xb0, 0xe0, 0xf0
};

TILE_GET_INFO_MEMBER(pturn_state::get_tile_info)
{
	int tileno = m_videoram[tile_index];

	tileno=tile_lookup[tileno>>4]|(tileno&0xf)|(m_fgbank<<8);

	SET_TILE_INFO_MEMBER(0,tileno,m_fgpalette,0);
}



TILE_GET_INFO_MEMBER(pturn_state::get_bg_tile_info)
{
	int tileno = memregion("user1")->base()[tile_index];
	int palno=m_bgpalette;
	if(palno==1)
	{
		palno=25;
	}
	SET_TILE_INFO_MEMBER(1,tileno+m_bgbank*256,palno,0);
}

void pturn_state::video_start()
{
	m_fgmap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pturn_state::get_tile_info),this),TILEMAP_SCAN_ROWS,8, 8,32,32);
	m_fgmap->set_transparent_pen(0);
	m_bgmap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pturn_state::get_bg_tile_info),this),TILEMAP_SCAN_ROWS,8, 8,32,32*8);
	m_bgmap->set_transparent_pen(0);

	save_item(NAME(m_bgbank));
	save_item(NAME(m_fgbank));
	save_item(NAME(m_bgpalette));
	save_item(NAME(m_fgpalette));
	save_item(NAME(m_bgcolor));
}

UINT32 pturn_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_bgcolor, cliprect);
	m_bgmap->draw(screen, bitmap, cliprect, 0,0);
	for (int offs = 0x80-4 ; offs >=0 ; offs -= 4)
	{
		int sy=256-m_spriteram[offs]-16 ;
		int sx=m_spriteram[offs+3]-16 ;

		int flipx=m_spriteram[offs+1]&0x40;
		int flipy=m_spriteram[offs+1]&0x80;


		if (flip_screen_x())
		{
			sx = 224 - sx;
			flipx ^= 0x40;
		}

		if (flip_screen_y())
		{
			flipy ^= 0x80;
			sy = 224 - sy;
		}

		if(sx|sy)
		{
			m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
			m_spriteram[offs+1] & 0x3f ,
			(m_spriteram[offs+2] & 0x1f),
			flipx, flipy,
			sx,sy,0);
		}
	}
	m_fgmap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

#ifdef UNUSED_FUNCTION
READ8_MEMBER(pturn_state::protection_r)
{
	return 0x66;
}

READ8_MEMBER(pturn_state::protection2_r)
{
	return 0xfe;
}
#endif

WRITE8_MEMBER(pturn_state::videoram_w)
{
	m_videoram[offset]=data;
	m_fgmap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(pturn_state::nmi_main_enable_w)
{
	m_nmi_main = data;
}

WRITE8_MEMBER(pturn_state::nmi_sub_enable_w)
{
	m_nmi_sub = data;
}

WRITE8_MEMBER(pturn_state::bgcolor_w)
{
	m_bgcolor=data;
}

WRITE8_MEMBER(pturn_state::bg_scrollx_w)
{
	m_bgmap->set_scrolly(0, (data>>5)*32*8);
	m_bgpalette=data&0x1f;
	m_bgmap->mark_all_dirty();
}

WRITE8_MEMBER(pturn_state::fgpalette_w)
{
	m_fgpalette=data&0x1f;
	m_fgmap->mark_all_dirty();
}

WRITE8_MEMBER(pturn_state::bg_scrolly_w)
{
	m_bgmap->set_scrollx(0, data);
}

WRITE8_MEMBER(pturn_state::fgbank_w)
{
	m_fgbank=data&1;
	m_fgmap->mark_all_dirty();
}

WRITE8_MEMBER(pturn_state::bgbank_w)
{
	m_bgbank=data&1;
	m_bgmap->mark_all_dirty();
}

WRITE8_MEMBER(pturn_state::flip_w)
{
	flip_screen_set(data);
}


READ8_MEMBER(pturn_state::custom_r)
{
	int addr = (int)offset + 0xc800;

	switch(addr)
	{
		case 0xc803:
			// pc=4a4,4a7 : dummy read?
			return 0x00;

		case 0xCA73:
			// pc=0x0123 , bit6 must be 0
			// pc=0x0545 , +40 must be 0xfe (check at 0577)
			return 0xbe;

		//case 0xca00:
		//  return 0x00; // pc=0x0131 for protect reset?

		case 0xca74:
			// pc=0x04db ,must be 66 (check at 016A)
			return 0x66;
	}
	return 0x00;
}


static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, pturn_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xc800, 0xcfff) AM_WRITENOP AM_READ(custom_r)

	AM_RANGE(0xdfe0, 0xdfe0) AM_NOP

	AM_RANGE(0xe000, 0xe3ff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xe400, 0xe400) AM_WRITE(fgpalette_w)
	AM_RANGE(0xe800, 0xe800) AM_WRITE(soundlatch_byte_w)

	AM_RANGE(0xf000, 0xf0ff) AM_RAM AM_SHARE("spriteram")

	AM_RANGE(0xf400, 0xf400) AM_WRITE(bg_scrollx_w)

	AM_RANGE(0xf800, 0xf800) AM_READ_PORT("P1") AM_WRITENOP
	AM_RANGE(0xf801, 0xf801) AM_READ_PORT("P2") AM_WRITE(bgcolor_w)
	AM_RANGE(0xf802, 0xf802) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xf803, 0xf803) AM_WRITE(bg_scrolly_w)
	AM_RANGE(0xf804, 0xf804) AM_READ_PORT("DSW2")
	AM_RANGE(0xf805, 0xf805) AM_READ_PORT("DSW1")
	AM_RANGE(0xf806, 0xf806) AM_READNOP /* Protection related, ((val&3)==2) -> jump to 0 */

	AM_RANGE(0xfc00, 0xfc00) AM_WRITE(flip_w)
	AM_RANGE(0xfc01, 0xfc01) AM_WRITE(nmi_main_enable_w)
	AM_RANGE(0xfc02, 0xfc02) AM_WRITENOP /* Unknown */
	AM_RANGE(0xfc03, 0xfc03) AM_WRITENOP /* Unknown */
	AM_RANGE(0xfc04, 0xfc04) AM_WRITE(bgbank_w)
	AM_RANGE(0xfc05, 0xfc05) AM_WRITE(fgbank_w)
	AM_RANGE(0xfc06, 0xfc06) AM_WRITENOP /* Unknown */
	AM_RANGE(0xfc07, 0xfc07) AM_WRITENOP /* Unknown */

ADDRESS_MAP_END

static ADDRESS_MAP_START( sub_map, AS_PROGRAM, 8, pturn_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x3000, 0x3000) AM_READ(soundlatch_byte_r) AM_WRITE(nmi_sub_enable_w)
	AM_RANGE(0x4000, 0x4000) AM_RAM
	AM_RANGE(0x5000, 0x5001) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x6000, 0x6001) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
ADDRESS_MAP_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0,1,2,3, 4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	32,32,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7,
	16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
	24*8+0, 24*8+1, 24*8+2, 24*8+3, 24*8+4, 24*8+5, 24*8+6, 24*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8,
	64*8, 65*8, 66*8, 67*8, 68*8, 69*8, 70*8, 71*8,
	96*8, 97*8, 98*8, 99*8, 100*8, 101*8, 102*8, 103*8 },
	128*8
};

static GFXDECODE_START( pturn )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0x000, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,   0x000, 32 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 0x000, 32 )
GFXDECODE_END

static INPUT_PORTS_START( pturn )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 ) /* service coin */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0xc8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x02, "7" )
	PORT_DIPSETTING(    0x03, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0c, "100000" )
	PORT_DIPSETTING(    0x08, "50000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0xb0, IP_ACTIVE_HIGH, IPT_UNUSED ) /* marked as "NOT USED" in doc */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x00, "Freeze" )
	PORT_DIPSETTING(    0x00, "Normal Display" )
	PORT_DIPSETTING(    0x40, "Stop Motion" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Language ) ) /* marked as "NOT USED" in doc */
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )
INPUT_PORTS_END

INTERRUPT_GEN_MEMBER(pturn_state::sub_intgen)
{
	if(m_nmi_sub)
	{
		device.execute().set_input_line(INPUT_LINE_NMI,PULSE_LINE);
	}
}

INTERRUPT_GEN_MEMBER(pturn_state::main_intgen)
{
	if (m_nmi_main)
	{
		device.execute().set_input_line(INPUT_LINE_NMI,PULSE_LINE);
	}
}

void pturn_state::machine_start()
{
	save_item(NAME(m_nmi_main));
	save_item(NAME(m_nmi_sub));
}

void pturn_state::machine_reset()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	soundlatch_clear_byte_w(space,0,0);
}

static MACHINE_CONFIG_START( pturn, pturn_state )
	MCFG_CPU_ADD("maincpu", Z80, 12000000/3)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pturn_state,  main_intgen)

	MCFG_CPU_ADD("audiocpu", Z80, 12000000/3)
	MCFG_CPU_PROGRAM_MAP(sub_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(pturn_state, sub_intgen, 3*60)


	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(pturn_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS("palette", 0x100)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pturn)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 2000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, 2000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


ROM_START( pturn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "prom4.8d", 0x00000,0x2000, CRC(d3ae0840) SHA1(5ac5f2626de7865cdf379cf15ae3872e798b7e25))
	ROM_LOAD( "prom6.8b", 0x02000,0x2000, CRC(65f09c56) SHA1(c0a7a1bfaacfc4af14d8485e2b5f2c604937a1e4))
	ROM_LOAD( "prom5.7d", 0x04000,0x2000, CRC(de48afb4) SHA1(9412288b63cf3ae8c9522b1fcacc4aa36ac7a23c))
	ROM_LOAD( "prom7.7b", 0x06000,0x2000, CRC(bfaeff9f) SHA1(63972c311f28971e121fbccd4c0d78edbdb6bdb4))

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "prom9.5n", 0x00000,0x1000, CRC(8b4d944e) SHA1(6f956d972c2c2ef875378910b80ca59701710957))

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "prom1.8k", 0x00000,0x1000, CRC(10aba36d) SHA1(5f9ce00365b3be91f0942b282b3cfc0c791baf98))
	ROM_LOAD( "prom2.7k", 0x01000,0x1000, CRC(b8a4d94e) SHA1(78f9db58ceb4a87ab2744529b0e7ad3eb826e627))
	ROM_LOAD( "prom3.6k", 0x02000,0x1000, CRC(9f51185b) SHA1(84690556da013567133b7d8fcda25b9fb831e4b0))

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "prom11.16f", 0x000000, 0x01000, CRC(129122c6) SHA1(feb6d9abddb4d888b49861a32a063d009ca994aa) )
	ROM_LOAD( "prom12.16h", 0x001000, 0x01000, CRC(69b09323) SHA1(726749b625052984e1d8c71eb69511c35ca75f9c) )
	ROM_LOAD( "prom13.16k", 0x002000, 0x01000, CRC(e9f67599) SHA1(b2eb144c8ce9ff57bd66ba57705d5e242115ef41) )

	ROM_REGION( 0x6000, "gfx3", 0 )
	ROM_LOAD( "prom14.16l", 0x000000, 0x02000, CRC(ffaa0b8a) SHA1(20b1acc2562e493539fe34d056e6254e4b2458be) )
	ROM_LOAD( "prom15.16m", 0x002000, 0x02000, CRC(41445155) SHA1(36d81b411729447ca7ff712ac27d8a0f2015bcac) )
	ROM_LOAD( "prom16.16p", 0x004000, 0x02000, CRC(94814c5d) SHA1(e4ab6c0ae94184d5270cadb887f56e3550b6d9f2) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "prom_red.3p", 0x0000, 0x0100, CRC(505fd8c2) SHA1(f2660fe512c76412a7b9f4be21fe549dd59fbda0) )
	ROM_LOAD( "prom_grn.4p", 0x0100, 0x0100, CRC(6a00199d) SHA1(ff0ac7ae83d970778a756f7445afed3785fc1150) )
	ROM_LOAD( "prom_blu.4r", 0x0200, 0x0100, CRC(7b4c5788) SHA1(ca02b12c19be7981daa070533455bd4d227d56cd) )

	ROM_REGION( 0x2000, "user1", 0 )
	ROM_LOAD( "prom10.16d", 0x0000,0x2000, CRC(a96e3c95) SHA1(a3b1c1723fcda80c11d9858819659e5e9dfe5dd3))

ROM_END


DRIVER_INIT_MEMBER(pturn_state,pturn)
{
	/*
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc0dd, 0xc0dd, read8_delegate(FUNC(pturn_state::protection_r), this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc0db, 0xc0db, read8_delegate(FUNC(pturn_state::protection2_r), this));
	*/
}

GAME( 1984, pturn,  0, pturn,  pturn, pturn_state,  pturn, ROT90,   "Jaleco", "Parallel Turn",  MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
