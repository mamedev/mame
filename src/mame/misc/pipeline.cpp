// license:BSD-3-Clause
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

#include "cpu/m6805/m68705.h"
#include "cpu/z80/z80.h"

#include "machine/i8255.h"
#include "machine/z80ctc.h"
#include "machine/z80daisy.h"

#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class pipeline_state : public driver_device
{
public:
	pipeline_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mcu(*this, "mcu")
		, m_ctc(*this, "ctc")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_vram(*this, "vram%u", 1U)
		, m_vidctrl(0)
		, m_palram(*this, "palram", 0x1000, ENDIANNESS_LITTLE)
		, m_from_mcu(0)
		, m_sound_data(0)
	{
	}

	void pipeline(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void vram2_w(offs_t offset, u8 data);
	void vram1_w(offs_t offset, u8 data);
	void mcu_porta_w(u8 data);
	void vidctrl_w(u8 data);
	u8 protection_r();
	void protection_w(u8 data);
	u8 sound_data_r();
	void sound_data_w(u8 data);

	TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(get_tile_info2);

	void palette(palette_device &palette) const;

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_CALLBACK_MEMBER(vidctrl_deferred_w);
	TIMER_CALLBACK_MEMBER(protection_deferred_w);

	void cpu0_mem(address_map &map) ATTR_COLD;
	void cpu1_mem(address_map &map) ATTR_COLD;
	void sound_port(address_map &map) ATTR_COLD;

	required_device<cpu_device>         m_maincpu;
	required_device<m68705r_device>     m_mcu;
	required_device<z80ctc_device>      m_ctc;
	required_device<gfxdecode_device>   m_gfxdecode;
	required_device<palette_device>     m_palette;

	required_shared_ptr_array<u8, 2> m_vram;

	tilemap_t *m_tilemap[2];

	u8                       m_vidctrl;
	memory_share_creator<u8> m_palram;
	u8                       m_from_mcu;
	u8                       m_sound_data;
};


void pipeline_state::machine_start()
{
	membank("soundbank")->configure_entries(0, 2, memregion("audiocpu")->base() + 0x8000, 0x4000);

	save_item(NAME(m_from_mcu));
	save_item(NAME(m_sound_data));
}

TILE_GET_INFO_MEMBER(pipeline_state::get_tile_info)
{
	int code = m_vram[1][tile_index] + m_vram[1][tile_index + 0x800] * 256;
	tileinfo.set(0, code, 0, 0);
}

TILE_GET_INFO_MEMBER(pipeline_state::get_tile_info2)
{
	int code = m_vram[0][tile_index] + ((m_vram[0][tile_index + 0x800] >> 4)) * 256;
	int color = ((m_vram[0][tile_index + 0x800]) & 0xf);
	tileinfo.set(1, code, color, 0);
}

void pipeline_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pipeline_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 64,32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pipeline_state::get_tile_info2)), TILEMAP_SCAN_ROWS, 8,8, 64,32);
	m_tilemap[1]->set_transparent_pen(0);

	save_item(NAME(m_vidctrl));
}

u32 pipeline_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void pipeline_state::vidctrl_w(u8 data)
{
	// synchronization is needed to avoid spurious CTC interrupts
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(pipeline_state::vidctrl_deferred_w),this), data);
}

TIMER_CALLBACK_MEMBER(pipeline_state::vidctrl_deferred_w)
{
	m_vidctrl = u8(param);
	m_ctc->trg2(BIT(param, 1));
}

void pipeline_state::vram2_w(offs_t offset, u8 data)
{
	if (!(m_vidctrl & 1))
	{
		m_tilemap[0]->mark_tile_dirty(offset & 0x7ff);
		m_vram[1][offset] = data;
	}
	else
	{
		m_palram[offset] = data;
		if (offset < 0x300)
		{
			offset &= 0xff;
			m_palette->set_pen_color(offset, pal6bit(m_palram[offset]), pal6bit(m_palram[offset+0x100]), pal6bit(m_palram[offset+0x200]));
		}
	}
}

void pipeline_state::vram1_w(offs_t offset, u8 data)
{
	m_tilemap[1]->mark_tile_dirty(offset & 0x7ff);
	m_vram[0][offset] = data;
}

u8 pipeline_state::protection_r()
{
	return m_from_mcu;
}

TIMER_CALLBACK_MEMBER(pipeline_state::protection_deferred_w)
{
	m_mcu->pa_w(param);
}

void pipeline_state::protection_w(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(pipeline_state::protection_deferred_w),this), data);
	machine().scheduler().perfect_quantum(attotime::from_usec(100));
}

u8 pipeline_state::sound_data_r()
{
	return m_sound_data;
}

void pipeline_state::sound_data_w(u8 data)
{
	m_sound_data = data;
}

void pipeline_state::cpu0_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x97ff).ram().w(FUNC(pipeline_state::vram1_w)).share(m_vram[0]);
	map(0x9800, 0xa7ff).ram().w(FUNC(pipeline_state::vram2_w)).share(m_vram[1]);
	map(0xb800, 0xb803).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xb810, 0xb813).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xb830, 0xb830).noprw();
	map(0xb840, 0xb840).noprw();
}

void pipeline_state::cpu1_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("soundbank");
	map(0xc000, 0xc7ff).ram();
	map(0xe000, 0xe001).mirror(2).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
}

void pipeline_state::sound_port(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x04, 0x07).rw("ppi8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void pipeline_state::mcu_porta_w(u8 data)
{
	m_from_mcu = data;
}


// verified from Z80 code
static INPUT_PORTS_START( pipeline )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("DSW1")
	// bits 0 to 6 are tested from less to most significant - code at 0x00dd
	PORT_DIPNAME( 0x7f, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x07, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x7f, DEF_STR( 1C_1C ) )            // duplicated setting
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x1f, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x3f, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x00, "2" )

	PORT_START("DSW2")
	// bits 0 to 2 are tested from less to most significant - code at 0x0181
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )             // table at 0x35eb
	PORT_DIPSETTING(    0x01, DEF_STR( Medium ) )           // table at 0x35c5
	PORT_DIPSETTING(    0x07, DEF_STR( Medium ) )           // duplicated setting
	PORT_DIPSETTING(    0x03, DEF_STR( Hard ) )             // table at 0x35a0
	PORT_DIPNAME( 0x18, 0x18, "Water Speed" )               // check code at 0x2619 - table at 0x5685
	PORT_DIPSETTING(    0x18, "Slowest" )                   // 0x12
	PORT_DIPSETTING(    0x10, "Slow" )                      // 0x0f
	PORT_DIPSETTING(    0x08, "Fast" )                      // 0x0d
	PORT_DIPSETTING(    0x00, "Fastest" )                   // 0x08
	PORT_DIPNAME( 0x20, 0x20, "Continue" )                  // check code at 0x0ffd - see notes
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Checkpoints" )
	PORT_DIPNAME( 0xc0, 0x00, "Sounds/Music" )              // check code at 0x1c0a
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

static GFXDECODE_START( gfx_pipeline )
	GFXDECODE_ENTRY( "8bpp_tiles", 0, layout_8x8x8, 0x000, 1 )
	GFXDECODE_ENTRY( "3bpp_tiles", 0, layout_8x8x3, 0x100, 32 )
GFXDECODE_END

static const z80_daisy_config daisy_chain_sound[] =
{
	{ "ctc" },
	{ nullptr }
};

void pipeline_state::palette(palette_device &palette) const
{
	u8 const *const prom1 = &memregion("proms")->base()[0x000];
	u8 const *const prom2 = &memregion("proms")->base()[0x100];

	for (int i = 0; i < 0x100; i++)
	{
		int const c = prom1[i] | (prom2[i] << 4);
		int const r = c & 7;
		int const g = (c >> 3) & 7;
		int const b = (c >> 6) & 3;
		palette.set_pen_color(0x100 + i, rgb_t(pal3bit(r), pal3bit(g), pal2bit(b)));
	}
}

void pipeline_state::pipeline(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 7.3728_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &pipeline_state::cpu0_mem);

	z80_device& audiocpu(Z80(config, "audiocpu", 7.3728_MHz_XTAL / 2));
	audiocpu.set_daisy_config(daisy_chain_sound);
	audiocpu.set_addrmap(AS_PROGRAM, &pipeline_state::cpu1_mem);
	audiocpu.set_addrmap(AS_IO, &pipeline_state::sound_port);

	M68705R3(config, m_mcu, 7.3728_MHz_XTAL / 2);
	m_mcu->porta_w().set(FUNC(pipeline_state::mcu_porta_w));

	Z80CTC(config, m_ctc, 7.3728_MHz_XTAL / 2); // same as "audiocpu"
	// TODO: external clock needed for channel 1 (DAC-related)?
	m_ctc->intr_callback().set_inputline("audiocpu", INPUT_LINE_IRQ0);

	i8255_device &ppi0(I8255A(config, "ppi8255_0"));
	ppi0.in_pa_callback().set_ioport("P1");
	ppi0.out_pb_callback().set(FUNC(pipeline_state::sound_data_w)); // related to sound/music : check code at 0x1c0a
	ppi0.out_pc_callback().set(FUNC(pipeline_state::vidctrl_w));

	i8255_device &ppi1(I8255A(config, "ppi8255_1"));
	ppi1.in_pa_callback().set_ioport("DSW1");
	ppi1.in_pb_callback().set_ioport("DSW2");
	ppi1.in_pc_callback().set(FUNC(pipeline_state::protection_r));
	ppi1.out_pc_callback().set(FUNC(pipeline_state::protection_w));

	i8255_device &ppi2(I8255A(config, "ppi8255_2"));
	ppi2.in_pb_callback().set(FUNC(pipeline_state::sound_data_r));
	ppi2.out_pc_callback().set_membank("soundbank").bit(7);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 512);
	screen.set_visarea(0, 319, 16, 239);
	screen.set_screen_update(FUNC(pipeline_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline("maincpu", INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pipeline);
	PALETTE(config, m_palette, FUNC(pipeline_state::palette), 0x100 + 0x100);

	// audio hardware
	SPEAKER(config, "mono").front_center();
	YM2203(config, "ymsnd", 7.3728_MHz_XTAL / 4).add_route(ALL_OUTPUTS, "mono", 0.30);
}


ROM_START( pipeline )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "rom1.u77", 0x00000, 0x08000, CRC(6e928290) SHA1(e2c8c35c04fd8ce3ddd6ecec04b0193a248e4362) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "rom2.u84", 0x00000, 0x08000, CRC(e77c43b7) SHA1(8b04005bc448083a429ace3319fc7e168a61f2f9) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "68705r3.u74", 0x00000, 0x01000, CRC(9bef427e) SHA1(d8e9b144190ac1c837e379e4be69d1e258a6c666) )

	ROM_REGION( 0x18000, "3bpp_tiles",0  )
	ROM_LOAD( "rom3.u32", 0x00000, 0x08000, CRC(d065ca46) SHA1(9fd8bb66735195d1cd20420096438abb5cb3fd54) )
	ROM_LOAD( "rom4.u31", 0x08000, 0x08000, CRC(6dc86355) SHA1(4b73e95726f7f244977634a5a152c90acb4ba89f) )
	ROM_LOAD( "rom5.u30", 0x10000, 0x08000, CRC(93f3f82a) SHA1(bc018370efc67a614ef6efa82526225f0db008ac) )

	ROM_REGION( 0x100000, "8bpp_tiles",0 )
	ROM_LOAD( "rom13.u1", 0x00000, 0x20000, CRC(611d7e01) SHA1(77e83c51b059f6d64009740d2e7e7ac1c8a6c7ec) )
	ROM_LOAD( "rom12.u2", 0x20000, 0x20000, CRC(8933c908) SHA1(9f04e454aacda479b6d6dd84dedbf56855f07fca) )
	ROM_LOAD( "rom11.u3", 0x40000, 0x20000, CRC(4e20e82d) SHA1(507b996ab5c8b8fb88f826d808f4b74dfc770db3) )
	ROM_LOAD( "rom10.u4", 0x60000, 0x20000, CRC(9892e465) SHA1(8789d169128bfc8de449bd617601a0f7fe1a19fb) )
	ROM_LOAD( "rom9.u5",  0x80000, 0x20000, CRC(07d16ca1) SHA1(caecf98284236af0e1f77566a9c6950491d0902a) )
	ROM_LOAD( "rom8.u6",  0xa0000, 0x20000, CRC(4e244c8a) SHA1(6808b2b195601e8041a12fff4b77e487efba015e) )
	ROM_LOAD( "rom7.u7",  0xc0000, 0x20000, CRC(23eb84dd) SHA1(ad1d359ba59087a5e786d262194cfe7db9bb0000) )
	ROM_LOAD( "rom6.u8",  0xe0000, 0x20000, CRC(c34bee64) SHA1(2da2dc6e6615ccc2e3e7ca0ceb735a347923a728) )

	ROM_REGION( 0x00220, "proms", 0 )
	ROM_LOAD( "82s129.u10", 0x00000, 0x00100, CRC(e91a1f9e) SHA1(a293023ebe96a5438e89457a98d94beb6dad5418) )
	ROM_LOAD( "82s129.u9",  0x00100, 0x00100, CRC(1cc09f6f) SHA1(35c857e0f3df0dcceec963459978e779e94f76f6) )
	ROM_LOAD( "82s123.u79", 0x00200, 0x00020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
ROM_END

} // Anonymous namespace


GAME( 1990, pipeline, 0, pipeline, pipeline, pipeline_state, empty_init, ROT0, "Daehyun Electronics", "Pipeline", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
