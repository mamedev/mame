// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/*
King Of Football (c)1995 BMC

preliminary driver by Tomasz Slanina

--

MC68000P10
M28 (OKI 6295, next to rom C9)
BMC ADB40817(80 Pin PQFP - google hits, but no datasheet or description)
RAMDAC TRC1710-80PCA (Monolithic 256-word by 18bit Look-up Table & Triple Video DAC with 6-bit DACs)
File 89C67 (MCU?? Next to 3.57954MHz OSC)
OSC: 21.47727MHz & 3.57954MHz
2 8-way dipswitchs
part # scratched 64 pin PLCC (soccer ball sticker over this chip ;-)

ft5_v16_c5.u14 \
ft5_v16_c6.u15 | 68000 program code

ft5_v6_c9.u21 - Sound samples

ft5_v6_c1.u59 \
ft5_v6_c2.u60 | Graphics
ft5_v6_c3.u61 |
ft5_v6_c4.u58 /

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "sound/ym2413.h"
#include "video/ramdac.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


#define NVRAM_HACK 1

class koftball_state : public driver_device
{
public:
	koftball_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_main_ram(*this, "main_ram"),
		m_bmc_1_videoram(*this, "bmc_1_videoram"),
		m_bmc_2_videoram(*this, "bmc_2_videoram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	void koftball(machine_config &config);

	void init_koftball();

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint16_t> m_main_ram;
	required_shared_ptr<uint16_t> m_bmc_1_videoram;
	required_shared_ptr<uint16_t> m_bmc_2_videoram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	tilemap_t *m_tilemap_1;
	tilemap_t *m_tilemap_2;
	uint16_t m_prot_data;

	DECLARE_READ16_MEMBER(random_number_r);
	DECLARE_READ16_MEMBER(prot_r);
	DECLARE_WRITE16_MEMBER(prot_w);
	DECLARE_WRITE16_MEMBER(bmc_1_videoram_w);
	DECLARE_WRITE16_MEMBER(bmc_2_videoram_w);
	TILE_GET_INFO_MEMBER(get_t1_tile_info);
	TILE_GET_INFO_MEMBER(get_t2_tile_info);
	virtual void video_start() override;
	uint32_t screen_update_koftball(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(bmc_interrupt);
	void koftball_mem(address_map &map);
	void ramdac_map(address_map &map);
};


TILE_GET_INFO_MEMBER(koftball_state::get_t1_tile_info)
{
	int data = m_bmc_1_videoram[tile_index];
	SET_TILE_INFO_MEMBER(0,
			data,
			0,
			0);
}

TILE_GET_INFO_MEMBER(koftball_state::get_t2_tile_info)
{
	int data = m_bmc_2_videoram[tile_index];
	SET_TILE_INFO_MEMBER(0,
			data,
			0,
			0);
}

void koftball_state::video_start()
{
	m_tilemap_1 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(koftball_state::get_t1_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,32);
	m_tilemap_2 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(koftball_state::get_t2_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,32);

	m_tilemap_1->set_transparent_pen(0);
}

uint32_t koftball_state::screen_update_koftball(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap_2->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap_1->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

READ16_MEMBER(koftball_state::random_number_r)
{
	return machine().rand();
}


READ16_MEMBER(koftball_state::prot_r)
{
	switch(m_prot_data)
	{
		case 0x0000: return 0x0d00;
		case 0xff00: return 0x8d00;

		case 0x8000: return 0x0f0f;
	}

	logerror("unk prot r %x %x\n",m_prot_data,  m_maincpu->pcbase());
	return machine().rand();
}

WRITE16_MEMBER(koftball_state::prot_w)
{
	COMBINE_DATA(&m_prot_data);
}

WRITE16_MEMBER(koftball_state::bmc_1_videoram_w)
{
	COMBINE_DATA(&m_bmc_1_videoram[offset]);
	m_tilemap_1->mark_tile_dirty(offset);
}

WRITE16_MEMBER(koftball_state::bmc_2_videoram_w)
{
	COMBINE_DATA(&m_bmc_2_videoram[offset]);
	m_tilemap_2->mark_tile_dirty(offset);
}

void koftball_state::koftball_mem(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x220000, 0x22ffff).ram().share("main_ram");

	map(0x260000, 0x260fff).w(FUNC(koftball_state::bmc_1_videoram_w)).share("bmc_1_videoram");
	map(0x261000, 0x261fff).w(FUNC(koftball_state::bmc_2_videoram_w)).share("bmc_2_videoram");
	map(0x262000, 0x26ffff).ram();

	map(0x280000, 0x28ffff).ram(); /* unused ? */
	map(0x2a0000, 0x2a001f).nopw();
	map(0x2a0000, 0x2a001f).r(FUNC(koftball_state::random_number_r));
	map(0x2b0000, 0x2b0003).r(FUNC(koftball_state::random_number_r));
	map(0x2d8000, 0x2d8001).r(FUNC(koftball_state::random_number_r));
	map(0x2da000, 0x2da003).w("ymsnd", FUNC(ym2413_device::write)).umask16(0xff00);

	map(0x2db001, 0x2db001).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x2db003, 0x2db003).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x2db005, 0x2db005).w("ramdac", FUNC(ramdac_device::mask_w));

	map(0x2dc000, 0x2dc000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x2f0000, 0x2f0003).portr("INPUTS");
	map(0x300000, 0x300001).nopw();
	map(0x320000, 0x320001).nopw();
	map(0x340000, 0x340001).r(FUNC(koftball_state::prot_r));
	map(0x360000, 0x360001).w(FUNC(koftball_state::prot_w));
}

void koftball_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

static INPUT_PORTS_START( koftball )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER )    PORT_NAME("info") PORT_CODE(KEYCODE_Z)//info page
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER )    PORT_NAME("test2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER )    PORT_NAME("dec") PORT_CODE(KEYCODE_C)//dec sound test
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER )    PORT_NAME("inc") PORT_CODE(KEYCODE_V)//inc sound test
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER )    PORT_NAME("test5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER )    PORT_NAME("test6") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER )    PORT_NAME("test7") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER )    PORT_NAME("test8") PORT_CODE(KEYCODE_A) //test mode exit

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER )    PORT_NAME("BET") PORT_CODE(KEYCODE_S) //bet ?
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER )    PORT_NAME("test12") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER )    PORT_NAME("test13") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER )    PORT_NAME("test14") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER )    PORT_NAME("sound test") PORT_CODE(KEYCODE_H) //test mdoe enter
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER )    PORT_NAME("test16") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OTHER )    PORT_NAME("Select") PORT_CODE(KEYCODE_K)//test mode select
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_OTHER )    PORT_NAME("test18") PORT_CODE(KEYCODE_L)
INPUT_PORTS_END


TIMER_DEVICE_CALLBACK_MEMBER(koftball_state::bmc_interrupt)
{
	int scanline = param;

	if(scanline == 240)
		m_maincpu->set_input_line(2, HOLD_LINE);

	if(scanline == 128)
		m_maincpu->set_input_line(3, HOLD_LINE);

	if(scanline == 64)
		m_maincpu->set_input_line(6, HOLD_LINE);
}

static const gfx_layout tilelayout =
{
	8,8,
	RGN_FRAC(1,2),
	8,
	{0,1,2,3, RGN_FRAC(1,2)+0,RGN_FRAC(1,2)+1,RGN_FRAC(1,2)+2,RGN_FRAC(1,2)+3 },
	{ 0, 4, 8, 12, 16,20,  24, 28 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static GFXDECODE_START( gfx_koftball )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,  0, 1 )
GFXDECODE_END


MACHINE_CONFIG_START(koftball_state::koftball)
	MCFG_DEVICE_ADD("maincpu", M68000, XTAL(21'477'272) / 2)
	MCFG_DEVICE_PROGRAM_MAP(koftball_mem)
	TIMER(config, "scantimer").configure_scanline(FUNC(koftball_state::bmc_interrupt), "screen", 0, 1);

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_UPDATE_DRIVER(koftball_state, screen_update_koftball)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 256)
	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, m_palette));
	ramdac.set_addrmap(0, &koftball_state::ramdac_map);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_koftball);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MCFG_DEVICE_ADD("ymsnd", YM2413, XTAL(3'579'545))  // guessed chip type, clock not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MCFG_DEVICE_ADD("oki", OKIM6295, 1122000, okim6295_device::PIN7_LOW) /* clock frequency & pin 7 not verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_CONFIG_END

ROM_START( koftball )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "ft5_v16_c5.u14", 0x000001, 0x10000, CRC(45c856e3) SHA1(0a25cfc2b09f1bf996f9149ee2a7d0a7e51794b7) )
	ROM_LOAD16_BYTE( "ft5_v16_c6.u15", 0x000000, 0x10000, CRC(5e1784a5) SHA1(5690d315500fb533b12b598cb0a51bd1eadd0505) )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* tiles */
	ROM_LOAD16_BYTE(    "ft5_v6_c3.u61", 0x00000, 0x20000, CRC(f3f747f3) SHA1(6e376d42099733e52779c089303391eeddf4fa87) )
	ROM_LOAD16_BYTE(    "ft5_v6_c4.u58", 0x00001, 0x20000, CRC(8b774574) SHA1(a79c1cf90d7b5ef0aba17770700b2fe18846f7b7) )
	ROM_LOAD16_BYTE(    "ft5_v6_c1.u59", 0x40000, 0x20000, CRC(b33a008f) SHA1(c4fd40883fa1c1cbc58f7b342fed753c52f0cf59) )
	ROM_LOAD16_BYTE(    "ft5_v6_c2.u60", 0x40001, 0x20000, CRC(3dc22223) SHA1(dc74800c51de3b6a7fbf7214a1da1d2f3d2aea84) )


	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ft5_v6_c9.u21", 0x00000, 0x10000,  CRC(f6216740) SHA1(3d1c795da2f8093e937107e3848cb96338536faf) )

ROM_END

#if NVRAM_HACK

static const uint16_t nvram[]=
{
	0x0000,0x5555,0x0000,0x5555,0x0000,0x5555,0x0000,0x5555,
	0x0000,0x5555,0x0000,0x0000,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x5555,0x5555,
	0x0000,0x0000,0x5555,0x5555,0x0000,0x0000,0x0467,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0xffff
};

#endif
void koftball_state::init_koftball()
{
	save_item(NAME(m_prot_data));

#if NVRAM_HACK
	{
		int offset = 0;
		while(nvram[offset] != 0xffff)
		{
			m_main_ram[offset]=nvram[offset];
			++offset;
		}
	}
#endif
}

GAME( 1995, koftball, 0, koftball, koftball, koftball_state, init_koftball, ROT0, "BMC", "King of Football", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
