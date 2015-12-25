// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Bega's Battle (c) 1983 Data East Corporation

preliminary driver by Angelo Salese

TODO:
- laserdisc hook-up and 6850 comms;
- "RAM TEST ERROR 5J" in Bega's Battle and Cobra Command
- "SOUND TEST READ ERROR"
- color offset number origin is unknown;
- Bega's Battle VBLANK hack (ld/framework fault most likely)
- CPU clocks
- dip-switches
- clean-ups

***************************************************************************

There are three hardware versions of Cobra Command (LD):

Data East single PCB, Pioneer LD-V1000
Data East 3-boardset, same hardware as Bega's Battle & Road Blaster, Sony
LDP-1000
MACH3 conversion kit (ROMs, disc, decals), Pioneer PR-8210

There are four versions of the laserdisc.

Pioneer (08359)
Data East (Japan), LDS-301 with an orange label
Data East (USA), LDS-301A with a green label
Sony (a1090731704132a)

The Data East labelled discs were released with the DE 3-boardset version
and MACH3 conversion.
The Pioneer Labelled disc was released with the DE single PCB version.
Not sure what version the Sony disc came from. It was given to me by the
copyright owner of Road Blaster, who also gave me a Road Blaster disc/kit
which has a similar Sony label.

I peeled the Data East labels off an orange and a green labelled disc and
the labels underneath were identical to the Sony labelled disc (Sony Japan,
disc No.a1090731704132a).

Physical appearances aside, the Sony and Pioneer pressed discs have
identical content.

===========================================================================


---------------------------------
Bega's Battle by DATA EAST (1983)
---------------------------------
malcor




Location   Device    File ID    Checksum
----------------------------------------
TB 14F      2764      AN00        E929   [ main program ] [ Rev.1 ]
TB 12F      2764      AN01        7B4D   [ main program ] [ Rev.1 ]
TB 11F      2764      AN02        3390   [ main program ] [ Rev.1 ]
TB 9F       2764      AN03        A9E5   [ main program ] [ Rev.1 ]
TB 8F       2764      AN04        303E   [ main program ] [ Rev.1 ]
TB 6F       2764      AN05        3A89   [ main program ] [ Rev.1 ]

TB 14F      2764      AN00-3      E983   [ main program ] [ Rev.3 ]
TB 11F      2764      AN02-3      46DA   [ main program ] [ Rev.3 ]
TB 9F       2764      AN03-3      B99B   [ main program ] [ Rev.3 ]
TB 8F       2764      AN04-3      3A57   [ main program ] [ Rev.3 ]
TB 6F       2764      AN05-3      3A9D   [ main program ] [ Rev.3 ]

TB 15C      2764      AN06        916B   [ snd  program ]
TB 3A       2764      AN07        944B
TB 4A       2764      AN08        798F
TB 6A       2764      AN09        DF57
TB 12A      2764      AN0A        5B95
TB 14A      2764      AN0B        F2C7
TB 15A      2764      AN0C        1605
LB 2F     82S123     AF-8.bpr     00FC   [ DSP select  ]
LB 14K   PAL10L8     LP1-1.pld    6A1A
LB 7C    PAL10L8     LP1-2.pld    6A16
LB 8C    PAL12L6     LP1-3.pld    76B3
LB 11E   PAL12L6     LP1-4.pld    769F
LB 6C    PAL10L8     LP1-5.pld    6A99
LB 12C   PAL10L8     LP1-5.pld    6A99
TB 10H   PAL10L8     LP2-1.pld    6A36
TB C10   PAL10L8     LP2-4.pld    6A05


Note: TB - Top board      VDO-2 DE-0139-1
      LB - Lower board    VDO-1 DE-0138-1

           Laserdisc video game


Brief hardware overview
-----------------------

Main processor  - 6502
                - EF68B50P   communications interface
                - AM2950DC   I/O port to sound processor

Sound processor - 6502
             2x - AY-3-8910

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/ay8910.h"
#include "machine/ldv1000.h"


class deco_ld_state : public driver_device
{
public:
	deco_ld_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_audiocpu(*this, "audiocpu"),
			m_laserdisc(*this, "laserdisc"),
			m_vram0(*this, "vram0"),
			m_attr0(*this, "attr0"),
			m_vram1(*this, "vram1"),
			m_attr1(*this, "attr1"),
			m_gfxdecode(*this, "gfxdecode"),
			m_screen(*this, "screen"),
			m_palette(*this, "palette")
			{ }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<pioneer_ldv1000_device> m_laserdisc;
	required_shared_ptr<UINT8> m_vram0;
	required_shared_ptr<UINT8> m_attr0;
	required_shared_ptr<UINT8> m_vram1;
	required_shared_ptr<UINT8> m_attr1;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	UINT8 m_laserdisc_data;
	int m_nmimask;
	DECLARE_WRITE8_MEMBER(rblaster_sound_w);
	DECLARE_READ8_MEMBER(laserdisc_r);
	DECLARE_WRITE8_MEMBER(laserdisc_w);
	DECLARE_READ8_MEMBER(sound_status_r);
	DECLARE_WRITE8_MEMBER(nmimask_w);
	DECLARE_WRITE8_MEMBER(decold_sound_cmd_w);
	DECLARE_WRITE8_MEMBER(decold_palette_w);
	DECLARE_CUSTOM_INPUT_MEMBER(begas_vblank_r);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	virtual void machine_start() override;
	UINT32 screen_update_rblaster(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(sound_interrupt);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT8 *spriteram, UINT16 tile_bank );
};

void deco_ld_state::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT8 *spriteram, UINT16 tile_bank )
{
	gfx_element *gfx = m_gfxdecode->gfx(1);
	int i,spr_offs,x,y,col,fx,fy;

	/*
	[+0] ---- -x-- flip X
	[+0] ---- --x- flip Y
	[+0] ---- ---x enable this sprite
	[+1] tile number
	[+2] y coord
	[+3] x coord
	*/

	for(i=0;i<0x20;i+=4)
	{
		if(~spriteram[i+0] & 1)
			continue;

		spr_offs = spriteram[i+1]|tile_bank;
		x = spriteram[i+3];
		y = spriteram[i+2];
		col = 6; /* TODO */
		fx = (spriteram[i+0] & 0x04) ? 1 : 0;
		fy = (spriteram[i+0] & 0x02) ? 1 : 0;

		gfx->transpen(bitmap,cliprect,spr_offs,col,fx,fy,x,y,0);
	}

	for(i=0x3e0;i<0x400;i+=4)
	{
		if(~spriteram[i+0] & 1)
			continue;

		spr_offs = spriteram[i+1]|tile_bank;
		x = spriteram[i+3];
		y = spriteram[i+2];
		col = 6; /* TODO */
		fx = (spriteram[i+0] & 0x04) ? 1 : 0;
		fy = (spriteram[i+0] & 0x02) ? 1 : 0;

		gfx->transpen(bitmap,cliprect,spr_offs,col,fx,fy,x,y,0);
	}
}

UINT32 deco_ld_state::screen_update_rblaster(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int y,x;

	bitmap.fill(0, cliprect);

	draw_sprites(bitmap,cliprect,m_vram1,0x000);
	draw_sprites(bitmap,cliprect,m_vram0,0x100);

	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			int attr = m_attr0[x+y*32];
			int tile = m_vram0[x+y*32] | ((attr & 3) << 8);
			int colour = (6 & 0x7); /* TODO */

			gfx->transpen(bitmap,cliprect,tile|0x400,colour,0,0,x*8,y*8,0);
		}
	}

	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			int attr = m_attr1[x+y*32];
			int tile = m_vram1[x+y*32] | ((attr & 3) << 8);
			int colour = (6 & 0x7); /* TODO */

			gfx->transpen(bitmap,cliprect,tile,colour,0,0,x*8,y*8,0);
		}
	}

	return 0;
}



READ8_MEMBER(deco_ld_state::laserdisc_r)
{
	UINT8 result = m_laserdisc->status_r();
//  osd_printf_debug("laserdisc_r = %02X\n", result);
	return result;
}


WRITE8_MEMBER(deco_ld_state::laserdisc_w)
{
	m_laserdisc_data = data;
}


WRITE8_MEMBER(deco_ld_state::decold_sound_cmd_w)
{
	soundlatch_byte_w(space, 0, data);
	m_audiocpu->set_input_line(0, HOLD_LINE);
}

/* same as Burger Time HW */
WRITE8_MEMBER(deco_ld_state::decold_palette_w)
{
	m_palette->write(space, offset, UINT8(~data));
}

/* unknown, but certainly related to audiocpu somehow */
READ8_MEMBER(deco_ld_state::sound_status_r)
{
	return 0xff ^ 0x40;
}

static ADDRESS_MAP_START( rblaster_map, AS_PROGRAM, 8, deco_ld_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x1000) AM_READ_PORT("IN0") AM_WRITENOP // (w) coin lockout
	AM_RANGE(0x1001, 0x1001) AM_READ_PORT("DSW1")
	AM_RANGE(0x1002, 0x1002) AM_READ_PORT("DSW2")
	AM_RANGE(0x1003, 0x1003) AM_READ_PORT("IN1")
	AM_RANGE(0x1004, 0x1004) AM_READ(soundlatch2_byte_r) AM_WRITE(decold_sound_cmd_w)
	AM_RANGE(0x1005, 0x1005) AM_READ(sound_status_r)
	AM_RANGE(0x1006, 0x1006) AM_NOP // 6850 status
	AM_RANGE(0x1007, 0x1007) AM_READWRITE(laserdisc_r,laserdisc_w) // 6850 data
	AM_RANGE(0x1800, 0x1fff) AM_RAM_WRITE(decold_palette_w) AM_SHARE("palette")
	AM_RANGE(0x2000, 0x27ff) AM_RAM
	AM_RANGE(0x2800, 0x2bff) AM_RAM AM_SHARE("vram0")
	AM_RANGE(0x2c00, 0x2fff) AM_RAM AM_SHARE("attr0")
	AM_RANGE(0x3000, 0x37ff) AM_RAM
	AM_RANGE(0x3800, 0x3bff) AM_RAM AM_SHARE("vram1")
	AM_RANGE(0x3c00, 0x3fff) AM_RAM AM_SHARE("attr1")
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END


/* sound arrangement is pratically identical to Zero Target. */

#ifdef UNUSED_FUNCTION
WRITE8_MEMBER(deco_ld_state::nmimask_w)
{
	m_nmimask = data & 0x80;
}
#endif

INTERRUPT_GEN_MEMBER(deco_ld_state::sound_interrupt)
{
	if (!m_nmimask) device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


static ADDRESS_MAP_START( rblaster_sound_map, AS_PROGRAM, 8, deco_ld_state )
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	AM_RANGE(0x2000, 0x2000) AM_DEVWRITE("ay1", ay8910_device, data_w)
	AM_RANGE(0x4000, 0x4000) AM_DEVWRITE("ay1", ay8910_device, address_w)
	AM_RANGE(0x6000, 0x6000) AM_DEVWRITE("ay2", ay8910_device, data_w)
	AM_RANGE(0x8000, 0x8000) AM_DEVWRITE("ay2", ay8910_device, address_w)
	AM_RANGE(0xa000, 0xa000) AM_READWRITE(soundlatch_byte_r,soundlatch2_byte_w)
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END

CUSTOM_INPUT_MEMBER( deco_ld_state::begas_vblank_r )
{
	return m_screen->vpos() >= 240*2;
}

INPUT_CHANGED_MEMBER(deco_ld_state::coin_inserted)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( begas )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_TILT )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
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
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )  PORT_CUSTOM_MEMBER(DEVICE_SELF,deco_ld_state,begas_vblank_r, NULL) // TODO: IPT_VBLANK doesn't seem to work fine?

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
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
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, deco_ld_state,coin_inserted, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, deco_ld_state,coin_inserted, 0)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( cobra )
	PORT_INCLUDE( begas )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	/* TODO: dips */
INPUT_PORTS_END

static INPUT_PORTS_START( rblaster )
	PORT_INCLUDE( begas )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	/* TODO: dips */
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3),RGN_FRAC(1,3),RGN_FRAC(0,3) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	8*8
};


static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3),RGN_FRAC(1,3),RGN_FRAC(0,3) },
	{ 7, 6, 5, 4, 3, 2, 1, 0, 16*8+7, 16*8+6, 16*8+5, 16*8+4, 16*8+3, 16*8+2, 16*8+1, 16*8+0 },
	{ 15*8, 14*8, 13*8, 12*8, 11*8, 10*8, 9*8, 8*8,
			7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	16*16
};

static GFXDECODE_START( rblaster )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 8 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,     0, 8 )
GFXDECODE_END

void deco_ld_state::machine_start()
{
}

static MACHINE_CONFIG_START( rblaster, deco_ld_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6502,8000000/2)
	MCFG_CPU_PROGRAM_MAP(rblaster_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", deco_ld_state, irq0_line_hold)

	MCFG_CPU_ADD("audiocpu",M6502,8000000/2)
	MCFG_CPU_PROGRAM_MAP(rblaster_sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(deco_ld_state, sound_interrupt,  640)

//  MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_LASERDISC_LDV1000_ADD("laserdisc") //Sony LDP-1000A, is it truly compatible with the Pioneer?
	MCFG_LASERDISC_OVERLAY_DRIVER(256, 256, deco_ld_state, screen_update_rblaster)
	MCFG_LASERDISC_OVERLAY_CLIP(0, 256-1, 8, 240-1)
	MCFG_LASERDISC_OVERLAY_PALETTE("palette")

	/* video hardware */
	MCFG_LASERDISC_SCREEN_ADD_NTSC("screen", "laserdisc")
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", rblaster)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_FORMAT(BBGGGRRR)

	/* sound hardware */
	/* TODO: mixing */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ay1", AY8910, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.25)

	MCFG_SOUND_MODIFY("laserdisc")
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( begas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "an05-3",   0x4000, 0x2000, CRC(c917a283) SHA1(b91f8cd18b8cc1189e4b69d6932d6f01d4ccfb81) )
	ROM_LOAD( "an04-3",   0x6000, 0x2000, CRC(935b2b0a) SHA1(e7c09960607569bd88e9af396aa70661f4352efb) )
	ROM_LOAD( "an03-3",   0x8000, 0x2000, CRC(79438d80) SHA1(e641336f23c6b84d84313ef3e94871ac9aa8b612) )
	ROM_LOAD( "an02-3",   0xa000, 0x2000, CRC(98ce4ca0) SHA1(e7db66b1f0f06b0a21e7450962ba70f460a24847) )
	ROM_LOAD( "an01",     0xc000, 0x2000, CRC(15f8921d) SHA1(32f945bee8f30e5896da38ac6184a11c0a8194bb) )
	ROM_LOAD( "an00-3",   0xe000, 0x2000, CRC(124a3a36) SHA1(e2f7110196cb46fcda429c613388285b46ec1a9e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "an06",   0xe000, 0x2000, CRC(cbbcd730) SHA1(2f2e78fcf2eba71044bec60d27d8756d9b5af551) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "an0a",   0x0000, 0x2000, CRC(e429305d) SHA1(9a05ab7916235d028b6b05270703516581825660) )
	ROM_LOAD( "an0b",   0x4000, 0x2000, CRC(09e4b780) SHA1(0735420b8529017e507feecf8f74fecd80fbf7d5) )
	ROM_LOAD( "an0c",   0x8000, 0x2000, CRC(0c127207) SHA1(b8372b2fa20ffe5ac278f558c07fd761c86e514b) )

	ROM_LOAD( "an07",   0x2000, 0x2000, CRC(6b8ad735) SHA1(a703523202d40e409e2345a6626b9e29b7a59cd3) )
	ROM_LOAD( "an08",   0x6000, 0x2000, CRC(b5518391) SHA1(57f6407491cff075f76a8b459cc33e8b9a91e7de) )
	ROM_LOAD( "an09",   0xa000, 0x2000, CRC(b7375fd7) SHA1(93a59e99e375bdba77199a705b5e304ece221617) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "af-8.bpr",    0x00, 0x20, CRC(20006a72) SHA1(6d0e1c6de45079f9e128186478a7e0ed3fd471d0) )

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "lp1-1.pld",   0x0000, 44, CRC(cc84cb09) SHA1(61620ef30dfd6c81cc517f95ab6358c619ca3298) )
	ROM_LOAD( "lp1-2.pld",   0x0100, 44, CRC(60e16fc4) SHA1(1df735f393ed0fcf1272fceada9764084ff11e07) )
	ROM_LOAD( "lp1-3.pld",   0x0200, 52, CRC(976a7c57) SHA1(202c55a236799fb44a977c074c231ed54c71a872) )
	ROM_LOAD( "lp1-4.pld",   0x0300, 52, CRC(cc9a442f) SHA1(5d08873b204b15f888d02d79e049119e05e41b45) )
	ROM_LOAD( "lp1-5.pld",   0x0400, 44, CRC(2d9f3118) SHA1(02e40a99f131bb47562d5b90fdfb11ca8cd90da6) )
	ROM_LOAD( "lp2-1.pld",   0x0500, 44, CRC(dbb05313) SHA1(fc37db24f12c4f5170945c9ec9a333e4583c1712) )
	ROM_LOAD( "lp2-4.pld",   0x0600, 44, CRC(4c72736c) SHA1(6f7521284a5d960ff05c4361095c3e89a79f7475) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "begas", 0, NO_DUMP )
ROM_END

ROM_START( begas1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "an05",   0x4000, 0x2000, CRC(91a05549) SHA1(425668ee0dcf44bc011ee3649aa82df6ad3180eb) )
	ROM_LOAD( "an04",   0x6000, 0x2000, CRC(670966fe) SHA1(c179e3045ed0e46c5829fce5297ada475141e662) )
	ROM_LOAD( "an03",   0x8000, 0x2000, CRC(d2d85cdf) SHA1(da557ce5c3252297d2c073a0242e1989b0b7388b) )
	ROM_LOAD( "an02",   0xa000, 0x2000, CRC(84d13c20) SHA1(6474d90b84bca88c35cdb1d4c117ce431d6addf7) )
	ROM_LOAD( "an01",   0xc000, 0x2000, CRC(15f8921d) SHA1(32f945bee8f30e5896da38ac6184a11c0a8194bb) )
	ROM_LOAD( "an00",   0xe000, 0x2000, CRC(184297f3) SHA1(6813f076fde3eb583929506b2e65d9cd988b1b75) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "an06",   0xe000, 0x2000, CRC(cbbcd730) SHA1(2f2e78fcf2eba71044bec60d27d8756d9b5af551) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "an0a",   0x0000, 0x2000, CRC(e429305d) SHA1(9a05ab7916235d028b6b05270703516581825660) )
	ROM_LOAD( "an0b",   0x4000, 0x2000, CRC(09e4b780) SHA1(0735420b8529017e507feecf8f74fecd80fbf7d5) )
	ROM_LOAD( "an0c",   0x8000, 0x2000, CRC(0c127207) SHA1(b8372b2fa20ffe5ac278f558c07fd761c86e514b) )

	ROM_LOAD( "an07",   0x2000, 0x2000, CRC(6b8ad735) SHA1(a703523202d40e409e2345a6626b9e29b7a59cd3) )
	ROM_LOAD( "an08",   0x6000, 0x2000, CRC(b5518391) SHA1(57f6407491cff075f76a8b459cc33e8b9a91e7de) )
	ROM_LOAD( "an09",   0xa000, 0x2000, CRC(b7375fd7) SHA1(93a59e99e375bdba77199a705b5e304ece221617) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "af-8.bpr",    0x00, 0x20, CRC(20006a72) SHA1(6d0e1c6de45079f9e128186478a7e0ed3fd471d0) )

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "lp1-1.pld",   0x0000, 44, CRC(cc84cb09) SHA1(61620ef30dfd6c81cc517f95ab6358c619ca3298) )
	ROM_LOAD( "lp1-2.pld",   0x0100, 44, CRC(60e16fc4) SHA1(1df735f393ed0fcf1272fceada9764084ff11e07) )
	ROM_LOAD( "lp1-3.pld",   0x0200, 52, CRC(976a7c57) SHA1(202c55a236799fb44a977c074c231ed54c71a872) )
	ROM_LOAD( "lp1-4.pld",   0x0300, 52, CRC(cc9a442f) SHA1(5d08873b204b15f888d02d79e049119e05e41b45) )
	ROM_LOAD( "lp1-5.pld",   0x0400, 44, CRC(2d9f3118) SHA1(02e40a99f131bb47562d5b90fdfb11ca8cd90da6) )
	ROM_LOAD( "lp2-1.pld",   0x0500, 44, CRC(dbb05313) SHA1(fc37db24f12c4f5170945c9ec9a333e4583c1712) )
	ROM_LOAD( "lp2-4.pld",   0x0600, 44, CRC(4c72736c) SHA1(6f7521284a5d960ff05c4361095c3e89a79f7475) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "begas", 0, NO_DUMP )
ROM_END

ROM_START( rblaster )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "01.bin",   0xc000, 0x2000, CRC(e4733c49) SHA1(357f46a80273f8a365d16cddf5e2caaeeacaf4ad) )
	ROM_LOAD( "00.bin",   0xe000, 0x2000, CRC(084d6ae2) SHA1(f49eb2d53bad5af88a12535ba628c9decce690ff) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "02.bin",   0xe000, 0x2000, CRC(6c20335d) SHA1(b28e80f112553af8e3fba9ebbfc10d1f56396ac1) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "03.bin",   0x0000, 0x2000, CRC(d1ff5ffb) SHA1(29df207e225e3b0477d5566d256198310d6ae526) )
	ROM_LOAD( "06.bin",   0x2000, 0x2000, CRC(d1ff5ffb) SHA1(29df207e225e3b0477d5566d256198310d6ae526) )
	ROM_LOAD( "04.bin",   0x4000, 0x2000, CRC(da2c84d9) SHA1(3452b0e2a45fa771e226c3a3668afbf3ceb0ec11) )
	ROM_LOAD( "07.bin",   0x6000, 0x2000, CRC(da2c84d9) SHA1(3452b0e2a45fa771e226c3a3668afbf3ceb0ec11) )
	ROM_LOAD( "05.bin",   0x8000, 0x2000, CRC(4608b516) SHA1(44af4be84a0b807ea0813ce86376a4b6fd927e5a) )
	ROM_LOAD( "08.bin",   0xa000, 0x2000, CRC(4608b516) SHA1(44af4be84a0b807ea0813ce86376a4b6fd927e5a) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "rblaster", 0, NO_DUMP )
ROM_END

ROM_START( cobra )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "au03-2",   0x8000, 0x2000, CRC(8f0a8fba) SHA1(8e11d2bd665a5ca6b3bb11aa2b707458c1534327) )
	ROM_LOAD( "au02-2",   0xa000, 0x2000, CRC(7db11acf) SHA1(1eebae0741f5735bc8966f3c31a9c07dac2e3916) )
	ROM_LOAD( "au01-2",   0xc000, 0x2000, CRC(523dd8f6) SHA1(47bd4c9b2272e9a710e6e97f2505075df68101ed) )
	ROM_LOAD( "au00-2",   0xe000, 0x2000, CRC(6c0f1f16) SHA1(ed05d3eaa24e84b1dfb4e1eb5f69b23e4a1494ba) )
	ROM_COPY( "maincpu",  0x8000, 0x4000, 0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "au06",   0xe000, 0x2000, CRC(ccc94eb0) SHA1(354a933ddc6a1e1118c2bf176faaab5d01fc92d3) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "au0a",   0x0000, 0x2000, CRC(6aaedcf3) SHA1(52dc913eecf8a159784d500217cffd7a6d8eb45c) )
	ROM_LOAD( "au0b",   0x4000, 0x2000, CRC(92247877) SHA1(f9bb0c20212ab13caabfb5beb9b6afc807bc9555) )
	ROM_LOAD( "au0c",   0x8000, 0x2000, CRC(d00a2762) SHA1(84d4329b39b9fd30682b7efa5cb2744934c5ee5c) )
	ROM_LOAD( "au07",   0x2000, 0x2000, CRC(d4bf12a5) SHA1(e172f69ae02ac2670b70af0cfcf3887dd99c2761) )
	ROM_LOAD( "au08",   0x6000, 0x2000, CRC(63158274) SHA1(c728e8ba0a11ea67cf508877ad74a3aab9ef26fc) )
	ROM_LOAD( "au09",   0xa000, 0x2000, CRC(74e93394) SHA1(7a1470cf2008b1bef8d950939b758707297b3655) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "cobra", 0, SHA1(8390498294aca97a5d1769032e7b115d1a42f5d3) )
ROM_END

ROM_START( cobraa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bd03-a4.bin",      0x8000, 0x2000, CRC(f1b9df77) SHA1(7fcc613463441f69e4336c12252a616508c296d1) )
	ROM_LOAD( "bd02-a3.bin",      0xa000, 0x2000, CRC(3d802707) SHA1(89033b2f9b295b74b6b5034c837509377f5aba62) )
	ROM_LOAD( "bd01-a2.bin",      0xc000, 0x2000, CRC(1b4db507) SHA1(c58021cb7dcbcb159d9bb24d231755b3d07aa74a) )
	ROM_LOAD( "bd000-1-a1.bin",   0xe000, 0x2000, CRC(8d9ad777) SHA1(10914251350988a82c0cfa1dc6e22587b885cc71) )
	ROM_COPY( "maincpu",          0x8000, 0x4000, 0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bd07-b11.bin",   0xe000, 0x2000, CRC(584d714a) SHA1(e3df3c42367d879c5f12db78d2797049ba18e6f8) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "bd04-.a10",     0x0000, 0x2000, CRC(33013cc2) SHA1(380ca35a891805e2aa1d35000a3770483aaedbfc) )
	ROM_LOAD( "bd06-a8.bin",   0x4000, 0x2000, CRC(b1340125) SHA1(eef325c56d3a441432c7d31ea1dadfaa0c871c10) )
	ROM_LOAD( "bd05-a9.bin",   0x8000, 0x2000, CRC(98412178) SHA1(20e93f377a0d572a6752d7435aa4d0a3feec0e92) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "cobra", 0, SHA1(8390498294aca97a5d1769032e7b115d1a42f5d3) ) // might be wrong for this set

	ROM_REGION( 0x40000, "misc", 0 )
	ROM_LOAD( "lp4-1.pal16l8cn.bin",        0x0000, 0x40000, CRC(4aeb2c7e) SHA1(3c962656cffc8d927047c64a15afccab767d776f) ) // dumped with cgfm's tool
	ROM_LOAD( "lp4-1.pal16l8cn.pld",        0x0000, 0x00f71, CRC(ac1f1177) SHA1(ab721a840207354916c96e0ae83220fed12c6352) )
//  ROM_LOAD( "lp4-2-pal10l8.d6.jed",       0x0000, 0x00249, CRC(309b3ce5) SHA1(04f185911d33730004c7cd44a693dd1b69b82032) )
	ROM_LOAD( "lp4-2-pal10l8.d6.bin",       0x0000, 0x0002c, CRC(e594fd13) SHA1(4bb8a9b7cf8f8eaa3c9f290b6e5085a10c927e20) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "vd0-c.h15",        0x0000, 0x00020, CRC(02c27aa0) SHA1(e7b814aabbfbcd992f78254b29b6ab6fa8115429) )
	ROM_LOAD( "vd0-t.f6",         0x0000, 0x00020, CRC(78449942) SHA1(584e25f7bffccd943c4db1edf05552f7989e08a4) )
ROM_END



GAME( 1983, begas,  0,       rblaster,  begas, driver_device,  0, ROT0, "Data East", "Bega's Battle (Revision 3)", MACHINE_NOT_WORKING )
GAME( 1983, begas1, begas,   rblaster,  begas, driver_device,  0, ROT0, "Data East", "Bega's Battle (Revision 1)", MACHINE_NOT_WORKING )
GAME( 1984, cobra,  0,       rblaster,  cobra, driver_device,  0, ROT0, "Data East", "Cobra Command (Data East LD, set 1)", MACHINE_NOT_WORKING )
GAME( 1984, cobraa, cobra,   rblaster,  cobra, driver_device,  0, ROT0, "Data East", "Cobra Command (Data East LD, set 2)", MACHINE_NOT_WORKING ) // might be a prototype
// Thunder Storm (Cobra Command Japanese version)
GAME( 1985, rblaster,  0,    rblaster,  rblaster, driver_device,  0, ROT0, "Data East", "Road Blaster (Data East LD)", MACHINE_NOT_WORKING )
