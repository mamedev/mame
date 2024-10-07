// license:BSD-3-Clause
// copyright-holders:David Haywood, Nicola Salmoria

/*
    New Quiz (Gals Panic bootleg on Modular System)

    PCB stack was marked as 'New Quiz'

    this is a clone of the expro02.cpp version of Gals Panic

    the hardware changes make it messy enough to have its own driver tho

    The Modular System cage contains 6! main boards for this game.

    MOD-6M - 68k board (CPU + 6 ROMs + RAM)
    COMP MOD-A - RAM board for Framebuffer gfx, 24Mhz XTAL
    MOD1/5 - Sound board (Z80, 2xYM2203C) (various wire mods and small sub-boards) Dipswitches also on here  (same/similar to the Euro League Modular board?)
    MOD51/3 - Sprite board (ROM sockets used for plug in board below)
    COMP AEREO MOD/5-1 (MODULAR SYSTEM 2) - Sprite ROM board, plugs into above, 24 sprite ROMs
    MOD 4/3 - Tilemap board, has logic + 4 tilemap ROMs, long thin sub-board (CAR-0484/1 SOLD) with no chips, just routing along one edge


    -- does the sound board have a MSM5205 or not?
*/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"
#include "cpu/z80/z80.h"
#include "sound/ymopn.h"
#include "machine/gen_latch.h"
#include "machine/bankdev.h"
#include "sound/msm5205.h"


namespace {

class galspanic_ms_state : public driver_device
{
public:
	galspanic_ms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_ym1(*this, "ym1"),
		m_ym2(*this, "ym2"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_paletteram(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_fg_ind8_pixram(*this, "fg_ind8ram"),
		m_bg_rgb555_pixram(*this, "bg_rgb555ram"),
		m_videoram2(*this, "videoram2"),
		m_scrollram(*this, "scrollram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_msm(*this, "msm"),
		m_soundrom(*this, "soundrom"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void newquiz(machine_config &config);
	void init_galpanicms();

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<ym2203_device> m_ym1;
	required_device<ym2203_device> m_ym2;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	required_shared_ptr<uint16_t> m_paletteram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_fg_ind8_pixram;
	required_shared_ptr<uint16_t> m_bg_rgb555_pixram;
	required_shared_ptr<uint16_t> m_videoram2;
	required_shared_ptr<uint16_t> m_scrollram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<msm5205_device> m_msm;
	required_device<address_map_bank_device> m_soundrom;
	required_device<generic_latch_8_device> m_soundlatch;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void galspanic_ms_palette(palette_device &palette) const;

	uint32_t screen_update_backgrounds(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// comad
	uint16_t comad_timer_r();
	void newquiz_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void soundrom_map(address_map &map) ATTR_COLD;

	uint8_t unk_r()
	{
		return machine().rand();
	}

	uint16_t vram2_r(offs_t offset, uint16_t mem_mask);
	void vram2_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	TILE_GET_INFO_MEMBER(get_tile_info_tilemap2);

	tilemap_t *m_bg_tilemap2 = nullptr;

	void splash_msm5205_int(int state);
	[[maybe_unused]] void splash_adpcm_data_w(uint8_t data);
	[[maybe_unused]] void splash_adpcm_control_w(uint8_t data);
	int m_adpcm_data = 0;

	void descramble_16x16tiles(uint8_t* src, int len);
};

TILE_GET_INFO_MEMBER(galspanic_ms_state::get_tile_info_tilemap2)
{
	int tile = m_videoram2[tile_index*2];

	//int attr = m_videoram2[(tile_index*2)+1] & 0xff;
	//int fx = (m_videoram2[(tile_index*2)+1] & 0xc0)>>6;

	int col = (tile & 0xf000)>>12;
	tile &= 0x0fff;

	tileinfo.set(1,tile,col,0);
}

uint16_t galspanic_ms_state::vram2_r(offs_t offset, uint16_t mem_mask)
{
	return m_videoram2[offset];
}

void galspanic_ms_state::vram2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram2[offset]);
	m_bg_tilemap2->mark_tile_dirty(offset/2);
}


uint16_t galspanic_ms_state::comad_timer_r()
{
	return (m_screen->vpos() & 0x07) << 8;
}

// lots of bad / leftover reads/writes from the original code in here
void galspanic_ms_state::newquiz_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();

	map(0x200000, 0x2fffff).rom().region("userx", 0); // reads from here, but what, if anything, maps here?

	map(0x500000, 0x51ffff).ram().share("fg_ind8ram");
	map(0x520000, 0x53ffff).ram().share("bg_rgb555ram");

	map(0x581000, 0x581fff).ram();
	map(0x584000, 0x5847ff).rw(FUNC(galspanic_ms_state::vram2_r), FUNC(galspanic_ms_state::vram2_w)).share("videoram2"); // was view2 tilemaps (moved from 0x580000 on original) presumably still 'bgtile' tiles tho
	map(0x584800, 0x584fff).ram().share("scrollram");
	map(0x585000, 0x58ffff).ram();

	map(0x600000, 0x600fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");

	map(0x680000, 0x68001f).ram(); // was view2 tilemap regs

	map(0x700000, 0x700fff).ram().share("oldspriteram"); // original spriteram? - drawing from here results in corrupt / missing sprites

	map(0x704000, 0x7047ff).ram().share("spriteram"); // bootleg uses this instead?

	map(0x780000, 0x78001f).ram();

	map(0x800000, 0x800001).portr("DSW1");
	map(0x800002, 0x800003).portr("DSW2");
	map(0x800004, 0x800005).portr("SYSTEM");

	map(0x80000e, 0x80000f).r(FUNC(galspanic_ms_state::comad_timer_r)).w(m_soundlatch, FUNC(generic_latch_8_device::write));

	map(0x840000, 0x840001).ram();

	map(0x900000, 0x900001).ram();

	map(0xa00000, 0xa00001).ram();

	map(0xb00000, 0xbfffff).rom().region("user1", 0); // reads girl data here

	map(0xc80000, 0xc8ffff).ram();

	map(0xe00012, 0xe00013).ram();
}


void galspanic_ms_state::machine_start()
{
}


void galspanic_ms_state::machine_reset()
{
	m_soundrom->set_bank(2);
}

void galspanic_ms_state::splash_adpcm_data_w(uint8_t data)
{
	m_adpcm_data = data;
}

void galspanic_ms_state::splash_adpcm_control_w(uint8_t data)
{
	m_msm->reset_w(BIT(data, 7));

	int bank = data & 0x7f;

	if ((bank != 0x02) &&
		(bank != 0x04) && (bank != 0x05) && (bank != 0x06) && (bank != 0x07) &&
		(bank != 0x0c) && (bank != 0x0d) && (bank != 0x0e) && (bank != 0x0f))
	{
		logerror("splash_adpcm_control_w %02x\n", data);
	}

	m_soundrom->set_bank(bank & 0xf);
}

void galspanic_ms_state::splash_msm5205_int(int state)
{
	m_msm->data_w(m_adpcm_data >> 4);
	m_adpcm_data = (m_adpcm_data << 4) & 0xf0;
}
void galspanic_ms_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8fff).ram();
	map(0x9000, 0x97ff).ram().mirror(0x0800);


	map(0xc000, 0xc000).nopw();

	//map(0x8000, 0xbfff).m(m_soundrom, FUNC(address_map_bank_device::amap8));

//  map(0xe000, 0xe000).w(FUNC(galspanic_ms_state::splash_adpcm_control_w));
//  map(0xe400, 0xe400).w(FUNC(galspanic_ms_state::splash_adpcm_data_w));

	map(0xa000, 0xa001).rw(m_ym1, FUNC(ym2203_device::read), FUNC(ym2203_device::write)).mirror(0x0008);
	map(0xa002, 0xa003).rw(m_ym2, FUNC(ym2203_device::read), FUNC(ym2203_device::write)).mirror(0x0008);
}

void galspanic_ms_state::soundrom_map(address_map &map)
{
	map(0x00000, 0x3ffff).rom().region("soundcpu", 0x000000);
}


void galspanic_ms_state::video_start()
{
	m_bg_tilemap2 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(galspanic_ms_state::get_tile_info_tilemap2)), TILEMAP_SCAN_ROWS,  16,  16, 32, 16);
	m_bg_tilemap2->set_transparent_pen(15);
}

void galspanic_ms_state::galspanic_ms_palette(palette_device &palette) const
{
	// first 2048 colors are dynamic

	// initialize 555 RGB lookup
	for (int i = 0; i < 32768; i++)
		palette.set_pen_color(2048 + i, pal5bit(i >> 5), pal5bit(i >> 10), pal5bit(i >> 0));
}

uint32_t galspanic_ms_state::screen_update_backgrounds(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int count;

	count = 0;
	for (int y = 0; y < 256; y++)
	{
		uint16_t *const dest = &bitmap.pix(y);
		for (int x = 0; x < 256; x++)
		{
			uint16_t dat = (m_bg_rgb555_pixram[count] & 0xfffe)>>1;
			dat += 2048;
			dest[(x-1)&0xff] = dat;
			count++;
		}
	}

	count = 0;
	for (int y = 0; y < 256; y++)
	{
		uint16_t *const dest = &bitmap.pix(y);
		for (int x = 0; x < 256; x++)
		{
			uint16_t const dat = m_fg_ind8_pixram[count] & 0x7ff;
			if (!(m_paletteram[dat] & 0x0001))
				dest[x] = dat;

			count++;
		}
	}

	screen.priority().fill(0, cliprect);

	return 0;
}

uint32_t galspanic_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update_backgrounds(screen, bitmap, cliprect);

	m_bg_tilemap2->set_scrollx(0, 64+m_scrollram[0x400/2]);
	m_bg_tilemap2->set_scrolly(0, 48-m_scrollram[0x402/2]);

	m_bg_tilemap2->draw(screen, bitmap, cliprect, 0, 0);

	// TODO, convert to device, share between Modualar System games
	const int NUM_SPRITES = 0x200;
	const int X_EXTRA_OFFSET = 240;

	for (int i = NUM_SPRITES-2; i >= 0; i-=2)
	{
		gfx_element *gfx = m_gfxdecode->gfx(0);

		uint16_t attr0 = m_spriteram[i + 0];
		uint16_t attr1 = m_spriteram[i + 1];

		uint16_t attr2 = m_spriteram[i + NUM_SPRITES];
		//uint16_t attr3 = m_spriteram[i + NUM_SPRITES + 1]; // unused?

		int ypos = attr0 & 0x00ff;
		int xpos = (attr1 & 0xff00)>>8;
		xpos |= (attr2 & 0x8000) ? 0x100 : 0x000;

		ypos = (0xff - ypos);

		int tile = (attr0 & 0xff00) >> 8;
		tile |= (attr1 & 0x003f) << 8;

		int flipx = 0; // different
		int flipy = (attr2 & 0x4000);

		gfx->transpen(bitmap,cliprect,tile,(attr2&0x0f00)>>8,flipx,flipy,xpos-16-X_EXTRA_OFFSET,ypos-14,15);
	}


	return 0;
}



static INPUT_PORTS_START( newquiz ) // copied from Gals Panic, might not be correct
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWA:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000C, 0x000C, DEF_STR( Lives ) )    PORT_DIPLOCATION("SWA:3,4")
	PORT_DIPSETTING(      0x0004, "2" )
	PORT_DIPSETTING(      0x000C, "3" )
	PORT_DIPSETTING(      0x0008, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Use Button" )        PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Censored Girls" )    PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Show Ending Picture" )   PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)   /* "Shot2" in "test mode" */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0001, "SWB:1" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0004, IP_ACTIVE_LOW, "SWB:3" )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SWB:4,5,6")
	PORT_DIPSETTING(      0x0028, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(      0x0080, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )


	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)   /* "Shot2" in "test mode" */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout tiles16x16x4_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,8,16,24 },
	{ 0,1,2,3,4,5,6,7, 512,513,514,515,516,517,518,519 },
	{ STEP16(0,32) },
	16 * 16 * 4
};

static GFXDECODE_START( gfx_galspanic_ms )
	GFXDECODE_ENTRY( "kan_spr", 0, tiles16x16x4_layout, 0x100, 16 )
	GFXDECODE_ENTRY( "bgtile", 0, tiles16x16x4_layout, 0x400, 16 )
GFXDECODE_END




void galspanic_ms_state::newquiz(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &galspanic_ms_state::newquiz_map);
	m_maincpu->set_vblank_int("screen", FUNC(galspanic_ms_state::irq6_line_hold)); // changed from original

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0, 256-32-1);
	m_screen->set_screen_update(FUNC(galspanic_ms_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(galspanic_ms_state::galspanic_ms_palette)).set_format(palette_device::GRBx_555, 2048 + 32768);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_galspanic_ms);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	Z80(config, m_soundcpu, 16_MHz_XTAL/4);
	m_soundcpu->set_addrmap(AS_PROGRAM, &galspanic_ms_state::sound_map);
//  m_soundcpu->set_periodic_int(FUNC(galspanic_ms_state::nmi_line_pulse), attotime::from_hz(60*64)); // no NMI here, just retn

	ADDRESS_MAP_BANK(config, m_soundrom).set_map(&galspanic_ms_state::soundrom_map).set_options(ENDIANNESS_LITTLE, 8, 18, 0x4000);

	GENERIC_LATCH_8(config, m_soundlatch);
	//m_soundlatch->data_pending_callback().set_inputline(m_soundcpu, INPUT_LINE_IRQ0);

	YM2203(config, m_ym1, XTAL(20'000'000)/16).add_route(ALL_OUTPUTS, "mono", 0.40);
	m_ym1->port_a_read_callback().set(FUNC(galspanic_ms_state::unk_r));
	m_ym1->port_b_read_callback().set(FUNC(galspanic_ms_state::unk_r));

	YM2203(config, m_ym2, XTAL(20'000'000)/16).add_route(ALL_OUTPUTS, "mono", 0.40);
	m_ym2->port_a_read_callback().set(FUNC(galspanic_ms_state::unk_r));
	m_ym2->port_b_read_callback().set(FUNC(galspanic_ms_state::unk_r));

	MSM5205(config, m_msm, XTAL(384'000));
	m_msm->vck_legacy_callback().set(FUNC(galspanic_ms_state::splash_msm5205_int));
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.80);
}


ROM_START( galpanicms )
	ROM_REGION( 0x100000, "maincpu", 0 ) // yes, one ROM is 4x the size of the other, there is no data to interleave with the extra data, does it actually map anywhere?
	ROM_LOAD16_BYTE( "cpu_ic17.bin",  0x000000, 0x80000, CRC(c65104e1) SHA1(189dde2a34b949bd1763d8ee0d74c86fead549b9) ) // AM27C040
	ROM_LOAD16_BYTE( "cpu_ic8.bin",   0x000001, 0x20000, CRC(e2e201e5) SHA1(2a8257f66139178af951d701fd263144aacf2808) ) // AM27C010

	ROM_REGION( 0x40000, "soundcpu", 0 ) // Z80 code
	ROM_LOAD( "snd_1.ic12", 0x000000, 0x10000, CRC(409e9233) SHA1(f13de7ddc00857c889250621ebccdae1b494cfd0) ) // TMS27C512

	ROM_REGION16_BE( 0x100000, "userx", ROMREGION_ERASEFF ) // 68000 data @ 0x200000

	ROM_REGION16_BE( 0x100000, "user1", 0 ) // 68000 data @ 0xb00000  (there is less girl data here than on the original Gals Panic sets, but no more spaces on this PCB and the ROM sizes are correct)
	ROM_LOAD16_BYTE( "cpu_ic20.bin", 0x000000, 0x40000, CRC(8dc5595e) SHA1(408022c6f81fc5ab0c53647a8a16d909227c43a6) ) // AM27C020
	ROM_LOAD16_BYTE( "cpu_ic11.bin", 0x000001, 0x40000, CRC(10967497) SHA1(65d1e5554f47a3f7a88fa1354035dff640faac2f) ) // AM27C020
	ROM_LOAD16_BYTE( "cpu_ic26.bin", 0x080000, 0x40000, CRC(c7e2135b) SHA1(8cf0c21c9b64e48da458bc29cfb15e1a5189c551) ) //  D27C020
	ROM_LOAD16_BYTE( "cpu_ic25.bin", 0x080001, 0x40000, CRC(406c2e3e) SHA1(ae229f01bdf0dea72a89c63b60f513a338fd8061) ) // AM27C020

	ROM_REGION( 0x200000, "kan_spr", ROMREGION_ERASEFF | ROMREGION_INVERT ) // sprites (seems to be the same as Gals Panic but alt ROM arrangement / decoding)
	ROM_LOAD32_BYTE( "5_gp_501.ic1",         0x000003, 0x010000, CRC(55ce19e8) SHA1(6aee3a43c731f017427b9316a6d2a536d7d44d35) ) // all TMS27C512
	ROM_LOAD32_BYTE( "5_gp_510.ic10",        0x000002, 0x010000, CRC(95cfabc0) SHA1(36708945b4a7c153e67c5f8d8a8a71e7b6cb0ca3) )
	ROM_LOAD32_BYTE( "5_gp_516.ic16",        0x000001, 0x010000, CRC(93c82aa3) SHA1(70d9beda4e2a93f89d2f0589c9fdbdb2ba4c7d7a) )
	ROM_LOAD32_BYTE( "5_gp_522.ic22",        0x000000, 0x010000, CRC(4d6b6890) SHA1(c4971eca6c738898d0f2be91bcd5a6b540075cfe) )

	ROM_LOAD32_BYTE( "5_gp_502.ic2",         0x040003, 0x010000, CRC(6948efa4) SHA1(f99681505f25bc2dd0a300af7f3deacd4bc430e2) )
	ROM_LOAD32_BYTE( "5_gp_511.ic11",        0x040002, 0x010000, CRC(67d42a09) SHA1(b15974cd515bf0b2da0f1f5cd2c2d8224b153357) )
	ROM_LOAD32_BYTE( "5_gp_517.ic17",        0x040001, 0x010000, CRC(c55177cd) SHA1(70eb291c206b2ef65967d6d12bfc88458f3c4215) )
	ROM_LOAD32_BYTE( "5_gp_523.ic23",        0x040000, 0x010000, CRC(8d2ae394) SHA1(054ebaef5af7cb4c1cf8f5658ef877544e25bdef) )

	ROM_LOAD32_BYTE( "5_gp_503.ic3",         0x080003, 0x010000, CRC(3eeacad6) SHA1(fb200a6b5e0f317a2ff57298c51949cd68953d88) )
	ROM_LOAD32_BYTE( "5_gp_512.ic12",        0x080002, 0x010000, CRC(02a56ff3) SHA1(c6717e0db51d521e7ff56d5ab0df45bb4a83301f) )
	ROM_LOAD32_BYTE( "5_gp_518.ic18",        0x080001, 0x010000, CRC(150fc3f3) SHA1(08a753d40cb4c7131b0218e97c999d2f7b8da38a) )
	ROM_LOAD32_BYTE( "5_gp_524.ic24",        0x080000, 0x010000, CRC(743b4584) SHA1(56a81934cf2fff4ca299ffa5355b926b6cded7c4) )

	ROM_LOAD32_BYTE( "5_gp_504.ic4",         0x0c0003, 0x010000, CRC(25244463) SHA1(d40605e6a6e37d60877cc93874bd9bdf77b954d0) )
	ROM_LOAD32_BYTE( "5_gp_513.ic13",        0x0c0002, 0x010000, CRC(efe47ee5) SHA1(a4b93ae5299583439f7ca6160acd595153f1f9ea) )
	ROM_LOAD32_BYTE( "5_gp_519.ic19",        0x0c0001, 0x010000, CRC(5b7b8ffb) SHA1(04c23b46bbc4e3cbfca7bb826c97f9c19e2b52fa) )
	ROM_LOAD32_BYTE( "5_gp_525.ic25",        0x0c0000, 0x010000, CRC(8b88f4b6) SHA1(67ce8cbc13a305c6e19572a031cc293cd3771650) )

	ROM_LOAD32_BYTE( "5_gp_505.ic5",         0x100003, 0x010000, CRC(bc4ed242) SHA1(e23401e96ca22fc2afd7375e1d8ea645232b8bc9) ) // 5_gp_505.ic5 == 5_gp_526.ic26
	ROM_LOAD32_BYTE( "5_gp_514.ic14",        0x100002, 0x010000, CRC(4cad79b4) SHA1(e7e7866aa3829d45814f1d8e0698807ed38f130d) )
	ROM_LOAD32_BYTE( "5_gp_520.ic20",        0x100001, 0x010000, CRC(61bc8e6a) SHA1(7e0e519c763b3bb547d7d6a58dd58ef7d542b70e) ) // xxxxx1xxxxxxxxxx = 0x00
	ROM_LOAD32_BYTE( "5_gp_526.ic26",        0x100000, 0x010000, CRC(bc4ed242) SHA1(e23401e96ca22fc2afd7375e1d8ea645232b8bc9) ) // 5_gp_505.ic5 == 5_gp_526.ic26

	ROM_LOAD32_BYTE( "5_gp_506.ic6",         0x140003, 0x010000, CRC(82d65489) SHA1(518ffd73bea3d473aa2424a554831ad5b879645a) )
	ROM_LOAD32_BYTE( "5_gp_515.ic15",        0x140002, 0x010000, CRC(f6d37373) SHA1(f399b92eb8490ca696b47c5b30ae5cd22de11b8a) )
	ROM_LOAD32_BYTE( "5_gp_521.ic21",        0x140001, 0x010000, CRC(00ed84f5) SHA1(d3704a0a0e0b3f0f6509f3ab80a7203794b61cca) )
	ROM_LOAD32_BYTE( "5_gp_527.ic27",        0x140000, 0x010000, CRC(689a8fae) SHA1(b16c33f02966aff3796b9dc528d3c2ca08ae49b1) )

	ROM_REGION( 0x40000, "bgtile", ROMREGION_INVERT ) // tilemap (less data than the encrypted layer on Gals Panic, probably no animation?)
	ROM_LOAD32_BYTE( "4_gp_401.ic17",        0x000003, 0x010000, CRC(cd7060f6) SHA1(c548b0ccdff0ae33a2b6eef3bf49d18bd0935321) ) // MC27C512AQ
	ROM_LOAD32_BYTE( "4_gp_402.ic16",        0x000002, 0x010000, CRC(c4627916) SHA1(9d30251a3a7ac0198f89c797ae59285870d21033) ) // MC27C512AQ
	ROM_LOAD32_BYTE( "4_gp_403.ic15",        0x000001, 0x010000, CRC(35d2bce4) SHA1(050e7adad47ea10a59761e59d8d4aee11960b0db) ) // MC27C512AQ
	ROM_LOAD32_BYTE( "4_gp_404.ic14",        0x000000, 0x010000, CRC(ea8eda3a) SHA1(dc2535d77788935322d1fb20097c1918b659d2b9) ) // MC27C512AQ

	ROM_REGION( 0x100, "soundprom", 0 )
	ROM_LOAD( "snd_p0115_74s288.ic20", 0x0000, 0x20, CRC(50bba48d) SHA1(0d677a67d3c3faca49ea3fe372d2df6802daefb0) )

	ROM_REGION( 0x100, "prom", 0 )
	ROM_LOAD( "51_502_63s141n.ic10", 0x0000, 0x100, CRC(15085e44) SHA1(646e7100fcb112594023cf02be036bd3d42cc13c) ) // same as Euro League modular bootleg

	ROM_REGION( 0x117, "gal", 0 )
	ROM_LOAD( "a_a-147_gal16v8-20hb1.ic47", 0x0000, 0x117, CRC(39102e72) SHA1(227e25df1555c226e5ae8fc7bcb0f2e2996e24cb) )
	ROM_LOAD( "a_a-347_gal16v8-25lp.ic50", 0x0000, 0x117, CRC(f377f5c7) SHA1(0ae69fe49735fb45245892d40a0bc97b5d387021) )

	ROM_REGION( 0x100, "protgal", 0 ) // all read protected
	ROM_LOAD( "4_403_gal16v8-20hb1.ic29", 0, 1, NO_DUMP )
	ROM_LOAD( "5_5147_gal16v8-25hb1.ic9", 0, 1, NO_DUMP )
	ROM_LOAD( "5_5247_gal16v8-25hb1.ic8", 0, 1, NO_DUMP )
	ROM_LOAD( "51_503_gal16v8-20hb1.ic46", 0, 1, NO_DUMP )
	ROM_LOAD( "a_a-247_gal20v8-20hb1.ic8", 0, 1, NO_DUMP )
	ROM_LOAD( "a_a-447_gal16v8-25lnc.ic51", 0, 1, NO_DUMP )
	ROM_LOAD( "cpu_606_gal16v8.ic13", 0, 1, NO_DUMP )
	ROM_LOAD( "cpu_647_gal16v8.ic7", 0, 1, NO_DUMP )
ROM_END

// reorganize graphics into something we can decode with a single pass
void galspanic_ms_state::descramble_16x16tiles(uint8_t* src, int len)
{
	std::vector<uint8_t> buffer(len);
	{
		for (int i = 0; i < len; i++)
		{
			int j = bitswap<20>(i, 19,18,17,16,15,12,11,10,9,8,7,6,5,14,13,4,3,2,1,0);
			buffer[j] = src[i];
		}

		std::copy(buffer.begin(), buffer.end(), &src[0]);
	}
}

void galspanic_ms_state::init_galpanicms()
{
	descramble_16x16tiles(memregion("bgtile")->base(), memregion("bgtile")->bytes());
}

} // anonymous namespace


GAME( 1991, galpanicms,  galsnew,  newquiz,  newquiz,  galspanic_ms_state, init_galpanicms, ROT90, "bootleg (Kaenko)", "New Quiz (Modular System bootleg of Gals Panic)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
