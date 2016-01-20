// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
/*
PCB Layout
----------

|----------------------------------------------------------|
|Z8430   ROM2.U84                          ROM3.U32        |
|                                                          |
|                                          ROM4.U31        |
|YM2203  Z80                                               |
|                                          ROM5.U30        |
|        6116                                              |
|                                          6116            |
| 8255                                                     |
|                                          6116   7.3728MHz|
|       Y3014                                              |
|                                                82S129.U10|
|J       82S123.U79                                        |
|A                                               82S129.U9 |
|M                                                         |
|M             6116                                        |
|A                                               ROM6.U8   |
|          ROM1.U77                                        |
| VOL                                            ROM7.U7   |
|               Z80                                        |
| UPC1241H                                       ROM8.U6   |
|              8255                                        |
|                                  6116          ROM9.U5   |
|                                                          |
|           68705R3                6116          ROM10.U4  |
|                                                          |
|DSW1                              6116          ROM11.U3  |
|                                                          |
|    8255                          6116          ROM12.U2  |
|                                                          |
|DSW2                              6116          ROM13.U1  |
|----------------------------------------------------------|
Notes:
      Z80 clocks (both) - 3.6864MHz [7.3728/2]
      Z8430             - zILOG Z8430 z80 CTC Counter Timer Circuit, clock 3.6864MHz [7.3728/2] (DIP28)
      68705R3           - Motorola MC68705R3 Microcontroller, clock 3.6864MHz [7.3728/2] (DIP40)
      YM2203 clock      - 1.8432MHz [7.3728/4]
      6116              - 2K x8 SRAM (DIP24)
      8255              - NEC D8255AC-2 Programmable Peripheral Interface (DIP40)
      VSync             - 60.0Hz
      HSync             - 15.40kHz


Stephh's notes (based on the games Z80 code and some tests) :

  - The "Continue" Dip Switch acts as follows :
      * when set to "Normal", you can continue a game from where you just lost (table at 0x2edd)
      * when set to "Checkpoints", your continue round is determined from where you just lost (table at 0x2f01) :
          . if you lose from round 01 to 05, you'll continue from round 01
          . if you lose from round 06 to 10, you'll continue from round 06
          . if you lose from round 11 to 15, you'll continue from round 11
          . if you lose from round 16 to 20, you'll continue from round 16
          . if you lose from round 21 to 25, you'll continue from round 21
          . if you lose from round 26 to 30, you'll continue from round 26
          . if you lose from round 31 to 35, you'll continue from round 31
          . if you lose from round 36, the game will become bogus because table is only 35 bytes wide
            and [0x2f01 + 0x23] = 0x6c which is not a valid round
    Round is stored at 0x800c (range 0x00-0x23).

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "sound/2203intf.h"
#include "machine/i8255.h"
#include "cpu/z80/z80daisy.h"
#include "cpu/m6805/m6805.h"


class pipeline_state : public driver_device
{
public:
	pipeline_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_vram1(*this, "vram1"),
		m_vram2(*this, "vram2") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_vram1;
	required_shared_ptr<UINT8> m_vram2;

	tilemap_t *m_tilemap1;
	tilemap_t *m_tilemap2;

	UINT8 m_vidctrl;
	std::unique_ptr<UINT8[]> m_palram;
	UINT8 m_toMCU;
	UINT8 m_fromMCU;
	UINT8 m_ddrA;

	DECLARE_WRITE8_MEMBER(vram2_w);
	DECLARE_WRITE8_MEMBER(vram1_w);
	DECLARE_WRITE8_MEMBER(mcu_portA_w);
	DECLARE_READ8_MEMBER(mcu_portA_r);
	DECLARE_WRITE8_MEMBER(mcu_ddrA_w);
	DECLARE_WRITE8_MEMBER(vidctrl_w);
	DECLARE_READ8_MEMBER(protection_r);
	DECLARE_WRITE8_MEMBER(protection_w);

	TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(get_tile_info2);

	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(pipeline);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_CALLBACK_MEMBER(protection_deferred_w);
};


void pipeline_state::machine_start()
{
	save_item(NAME(m_toMCU));
	save_item(NAME(m_fromMCU));
	save_item(NAME(m_ddrA));
}

TILE_GET_INFO_MEMBER(pipeline_state::get_tile_info)
{
	int code = m_vram2[tile_index]+m_vram2[tile_index+0x800]*256;
	SET_TILE_INFO_MEMBER(0,
		code,
		0,
		0);
}

TILE_GET_INFO_MEMBER(pipeline_state::get_tile_info2)
{
	int code =m_vram1[tile_index]+((m_vram1[tile_index+0x800]>>4))*256;
	int color=((m_vram1[tile_index+0x800])&0xf);
	SET_TILE_INFO_MEMBER(1,
		code,
		color,
		0);
}

void pipeline_state::video_start()
{
	m_palram=std::make_unique<UINT8[]>(0x1000);
	m_tilemap1 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pipeline_state::get_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,32 );
	m_tilemap2 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pipeline_state::get_tile_info2),this),TILEMAP_SCAN_ROWS,8,8,64,32 );
	m_tilemap2->set_transparent_pen(0);

	save_item(NAME(m_vidctrl));
	save_pointer(NAME(m_palram.get()), 0x1000);
}

UINT32 pipeline_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap1->draw(screen, bitmap, cliprect, 0,0);
	m_tilemap2->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}


WRITE8_MEMBER(pipeline_state::vidctrl_w)
{
	m_vidctrl=data;
}

WRITE8_MEMBER(pipeline_state::vram2_w)
{
	if(!(m_vidctrl&1))
	{
		m_tilemap1->mark_tile_dirty(offset&0x7ff);
		m_vram2[offset]=data;
	}
	else
	{
			m_palram[offset]=data;
			if(offset<0x300)
			{
			offset&=0xff;
			m_palette->set_pen_color(offset, pal6bit(m_palram[offset]), pal6bit(m_palram[offset+0x100]), pal6bit(m_palram[offset+0x200]));
			}
	}
}

WRITE8_MEMBER(pipeline_state::vram1_w)
{
	m_tilemap2->mark_tile_dirty(offset&0x7ff);
	m_vram1[offset]=data;
}

READ8_MEMBER(pipeline_state::protection_r)
{
	return m_fromMCU;
}

TIMER_CALLBACK_MEMBER(pipeline_state::protection_deferred_w)
{
	m_toMCU = param;
}

WRITE8_MEMBER(pipeline_state::protection_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(pipeline_state::protection_deferred_w),this), data);
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
}

static ADDRESS_MAP_START( cpu0_mem, AS_PROGRAM, 8, pipeline_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x97ff) AM_RAM_WRITE(vram1_w) AM_SHARE("vram1")
	AM_RANGE(0x9800, 0xa7ff) AM_RAM_WRITE(vram2_w) AM_SHARE("vram2")
	AM_RANGE(0xb800, 0xb803) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0xb810, 0xb813) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0xb830, 0xb830) AM_NOP
	AM_RANGE(0xb840, 0xb840) AM_NOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu1_mem, AS_PROGRAM, 8, pipeline_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xe000, 0xe003) AM_DEVREADWRITE("ppi8255_2", i8255_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_port, AS_IO, 8, pipeline_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ctc", z80ctc_device, read, write)
	AM_RANGE(0x06, 0x07) AM_NOP
ADDRESS_MAP_END

WRITE8_MEMBER(pipeline_state::mcu_portA_w)
{
	m_fromMCU=data;
}

READ8_MEMBER(pipeline_state::mcu_portA_r)
{
	return (m_fromMCU&m_ddrA)|(m_toMCU& ~m_ddrA);
}

WRITE8_MEMBER(pipeline_state::mcu_ddrA_w)
{
	m_ddrA=data;
}

static ADDRESS_MAP_START( mcu_mem, AS_PROGRAM, 8, pipeline_state )
	AM_RANGE(0x0000, 0x0000) AM_READ(mcu_portA_r) AM_WRITE(mcu_portA_w)
	AM_RANGE(0x0004, 0x0004) AM_WRITE(mcu_ddrA_w)
	AM_RANGE(0x0010, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x0fff) AM_ROM
ADDRESS_MAP_END


/* verified from Z80 code */
static INPUT_PORTS_START( pipeline )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("DSW1")
	/* bits 0 to 6 are tested from less to most significant - code at 0x00dd */
	PORT_DIPNAME( 0x7f, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x07, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x7f, DEF_STR( 1C_1C ) )            /* duplicated setting */
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x1f, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x3f, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x00, "2" )

	PORT_START("DSW2")
	/* bits 0 to 2 are tested from less to most significant - code at 0x0181 */
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )             /* table at 0x35eb */
	PORT_DIPSETTING(    0x01, DEF_STR( Medium ) )           /* table at 0x35c5 */
//  PORT_DIPSETTING(    0x07, DEF_STR( Medium ) )           /* duplicated setting */
	PORT_DIPSETTING(    0x03, DEF_STR( Hard ) )             /* table at 0x35a0 */
	PORT_DIPNAME( 0x18, 0x18, "Water Speed" )               /* check code at 0x2619 - table at 0x5685 */
	PORT_DIPSETTING(    0x18, "Slowest" )                   /* 0x12 */
	PORT_DIPSETTING(    0x10, "Slow" )                      /* 0x0f */
	PORT_DIPSETTING(    0x08, "Fast" )                      /* 0x0d */
	PORT_DIPSETTING(    0x00, "Fastest" )                   /* 0x08 */
	PORT_DIPNAME( 0x20, 0x20, "Continue" )                  /* check code at 0x0ffd - see notes */
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Checkpoints" )
	PORT_DIPNAME( 0xc0, 0x00, "Sounds/Music" )              /* check code at 0x1c0a - determine if it really affects music once it is supported */
	PORT_DIPSETTING(    0xc0, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, "Attract Mode" )
	PORT_DIPSETTING(    0x80, "Normal Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout layout_8x8x8 =
{
	8,8,
	RGN_FRAC(1,8),
	8,
	{RGN_FRAC(0,8),RGN_FRAC(1,8),RGN_FRAC(2,8),RGN_FRAC(3,8),RGN_FRAC(4,8),RGN_FRAC(5,8),RGN_FRAC(6,8),RGN_FRAC(7,8)},
	{ 0, 1, 2,  3,  4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout layout_8x8x3 =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3)},
	{ 0, 1, 2,  3,  4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( pipeline )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x8, 0x000, 1 ) // 8bpp tiles
	GFXDECODE_ENTRY( "gfx2", 0, layout_8x8x3, 0x100, 32 ) // 3bpp tiles
GFXDECODE_END

static const z80_daisy_config daisy_chain_sound[] =
{
	{ "ctc" },
	{ nullptr }
};

PALETTE_INIT_MEMBER(pipeline_state, pipeline)
{
	int r,g,b,i,c;
	UINT8 *prom1 = &memregion("proms")->base()[0x000];
	UINT8 *prom2 = &memregion("proms")->base()[0x100];

	for(i=0;i<0x100;i++)
	{
		c=prom1[i]|(prom2[i]<<4);
		r=c&7;
		g=(c>>3)&7;
		b=(c>>6)&3;
		r*=36;
		g*=36;
		b*=85;
		palette.set_pen_color(0x100+i, rgb_t(r, g, b));
	}
}

static MACHINE_CONFIG_START( pipeline, pipeline_state )
	/* basic machine hardware */

	MCFG_CPU_ADD("maincpu", Z80, 7372800/2)
	MCFG_CPU_PROGRAM_MAP(cpu0_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pipeline_state,  nmi_line_pulse)

	MCFG_CPU_ADD("audiocpu", Z80, 7372800/2)
	MCFG_CPU_CONFIG(daisy_chain_sound)
	MCFG_CPU_PROGRAM_MAP(cpu1_mem)
	MCFG_CPU_IO_MAP(sound_port)

	MCFG_CPU_ADD("mcu", M68705, 7372800/2)
	MCFG_CPU_PROGRAM_MAP(mcu_mem)

	MCFG_DEVICE_ADD("ctc", Z80CTC, 7372800/2 /* same as "audiocpu" */)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("audiocpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("P1"))
	// PORT B Write - related to sound/music : check code at 0x1c0a
	MCFG_I8255_OUT_PORTC_CB(WRITE8(pipeline_state, vidctrl_w))

	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("DSW1"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("DSW2"))
	MCFG_I8255_IN_PORTC_CB(READ8(pipeline_state, protection_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(pipeline_state, protection_w))

	MCFG_DEVICE_ADD("ppi8255_2", I8255A, 0)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 512)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 16, 239)
	MCFG_SCREEN_UPDATE_DRIVER(pipeline_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pipeline)

	MCFG_PALETTE_ADD("palette", 0x100+0x100)
	MCFG_PALETTE_INIT_OWNER(pipeline_state, pipeline)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, 7372800/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)

MACHINE_CONFIG_END


ROM_START( pipeline )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom1.u77", 0x00000, 0x08000, CRC(6e928290) SHA1(e2c8c35c04fd8ce3ddd6ecec04b0193a248e4362) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rom2.u84", 0x00000, 0x08000, CRC(e77c43b7) SHA1(8b04005bc448083a429ace3319fc7e168a61f2f9) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "68705r3.u74", 0x00000, 0x01000, CRC(9bef427e) SHA1(d8e9b144190ac1c837e379e4be69d1e258a6c666) )

	ROM_REGION( 0x18000, "gfx2",0  )
	ROM_LOAD( "rom3.u32", 0x00000, 0x08000, CRC(d065ca46) SHA1(9fd8bb66735195d1cd20420096438abb5cb3fd54) )
	ROM_LOAD( "rom4.u31", 0x08000, 0x08000, CRC(6dc86355) SHA1(4b73e95726f7f244977634a5a152c90acb4ba89f) )
	ROM_LOAD( "rom5.u30", 0x10000, 0x08000, CRC(93f3f82a) SHA1(bc018370efc67a614ef6efa82526225f0db008ac) )

	ROM_REGION( 0x100000, "gfx1",0 )
	ROM_LOAD( "rom13.u1", 0x00000, 0x20000,CRC(611d7e01) SHA1(77e83c51b059f6d64009740d2e7e7ac1c8a6c7ec) )
	ROM_LOAD( "rom12.u2", 0x20000, 0x20000,CRC(8933c908) SHA1(9f04e454aacda479b6d6dd84dedbf56855f07fca) )
	ROM_LOAD( "rom11.u3", 0x40000, 0x20000,CRC(4e20e82d) SHA1(507b996ab5c8b8fb88f826d808f4b74dfc770db3) )
	ROM_LOAD( "rom10.u4", 0x60000, 0x20000,CRC(9892e465) SHA1(8789d169128bfc8de449bd617601a0f7fe1a19fb) )
	ROM_LOAD( "rom9.u5",  0x80000, 0x20000,CRC(07d16ca1) SHA1(caecf98284236af0e1f77566a9c6950491d0902a) )
	ROM_LOAD( "rom8.u6",  0xa0000, 0x20000,CRC(4e244c8a) SHA1(6808b2b195601e8041a12fff4b77e487efba015e) )
	ROM_LOAD( "rom7.u7",  0xc0000, 0x20000,CRC(23eb84dd) SHA1(ad1d359ba59087a5e786d262194cfe7db9bb0000) )
	ROM_LOAD( "rom6.u8",  0xe0000, 0x20000,CRC(c34bee64) SHA1(2da2dc6e6615ccc2e3e7ca0ceb735a347923a728) )

	ROM_REGION( 0x00220, "proms", 0 )
	ROM_LOAD( "82s129.u10", 0x00000, 0x00100,CRC(e91a1f9e) SHA1(a293023ebe96a5438e89457a98d94beb6dad5418) )
	ROM_LOAD( "82s129.u9",  0x00100, 0x00100,CRC(1cc09f6f) SHA1(35c857e0f3df0dcceec963459978e779e94f76f6) )
	ROM_LOAD( "82s123.u79", 0x00200, 0x00020,CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
ROM_END

GAME( 1990, pipeline, 0, pipeline, pipeline, driver_device, 0, ROT0, "Daehyun Electronics", "Pipeline", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
