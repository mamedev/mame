// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

 quickpick5.cpp: Konami "Quick Pick 5" medal game

 Quick Pick 5
 (c) 199? Konami

 Driver by R. Belmont

 Rundown of PCB:
  Main CPU:  Z80

 Konami Custom chips:
  051649 (SCC1 sound)
  053252 (timing/interrupt controller)
  053244 (sprites)
  053245 (sprites)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "sound/k051649.h"
#include "sound/okim6295.h"
#include "video/k053244_k053245.h"
#include "video/konami_helper.h"
#include "machine/k053252.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class quickpick5_state : public driver_device
{
public:
	quickpick5_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_k053245(*this, "k053245"),
		m_k051649(*this, "k051649"),
		m_k053252(*this, "k053252"),
		m_gfxdecode(*this, "gfxdecode"),
		m_oki(*this, "oki"),
		m_ttlrom_offset(0)
	{ }

	void quickpick5(machine_config &config);

private:
	uint32_t screen_update_quickpick5(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	K05324X_CB_MEMBER(sprite_callback);
	TILE_GET_INFO_MEMBER(ttl_get_tile_info);
	DECLARE_WRITE8_MEMBER(ccu_int_time_w);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	READ8_MEMBER(control_r) { return m_control; }

	WRITE_LINE_MEMBER(vbl_ack_w) { m_maincpu->set_input_line(0, CLEAR_LINE); }
	WRITE_LINE_MEMBER(nmi_ack_w) { m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE); }

	// A0 is inverted to match the Z80's endianness.  Typical Konami.
	READ8_MEMBER(k244_r) { return m_k053245->k053244_r(offset^1);  }
	WRITE8_MEMBER(k244_w) { m_k053245->k053244_w(offset^1, data); }
	READ8_MEMBER(k245_r) { return m_k053245->k053245_r(offset^1);  }
	WRITE8_MEMBER(k245_w) { m_k053245->k053245_w(offset^1, data); }

	WRITE8_MEMBER(control_w)
	{
		membank("bank1")->set_entry(data&0x1);
		if (((m_control & 0x60) != 0x60) && ((data & 0x60) == 0x60))
		{
			m_ttlrom_offset = 0;
		}
		m_control = data;
	}

	DECLARE_READ8_MEMBER(vram_r);
	DECLARE_WRITE8_MEMBER(vram_w);

	void quickpick5_main(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<k05324x_device> m_k053245;
	required_device<k051649_device> m_k051649;
	required_device<k053252_device> m_k053252;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<okim6295_device> m_oki;

	int         m_ttl_gfx_index;
	tilemap_t   *m_ttl_tilemap;
	uint8_t     m_control;
	int         m_ttlrom_offset;
	uint8_t     m_vram[0x1000];
	bool        m_title_hack;
	int         m_ccu_int_time, m_ccu_int_time_count;
};

WRITE8_MEMBER(quickpick5_state::ccu_int_time_w)
{
	logerror("ccu_int_time rewritten with value of %02x\n", data);
	m_ccu_int_time = data;
}

READ8_MEMBER(quickpick5_state::vram_r)
{
	if ((m_control & 0x10) == 0x10)
	{
		offset |= 0x800;
		if ((offset >= 0x800) && (offset <= 0x880))
		{
			return m_k051649->k051649_waveform_r(offset & 0x7f);
		}
		else if ((offset >= 0x8e0) && (offset <= 0x8ff))
		{
			return m_k051649->k051649_test_r();
		}
	}

	if ((m_control & 0x60) == 0x60)
	{
		uint8_t *ROM = memregion("ttl")->base();
		return ROM[m_ttlrom_offset++];
	}

	return m_vram[offset];
}

WRITE8_MEMBER(quickpick5_state::vram_w)
{
	if ((m_control & 0x10) == 0x10)
	{
		offset |= 0x800;
		if ((offset >= 0x800) && (offset < 0x880))
		{
			m_k051649->k051649_waveform_w(offset-0x800, data);
			return;
		}
		else if (offset < 0x88a)
		{
			m_k051649->k051649_frequency_w(offset-0x880, data);
			return;
		}
		else if (offset < 0x88f)
		{
			m_k051649->k051649_volume_w(offset-0x88a, data);
			return;
		}
		else if (offset < 0x890)
		{
			m_k051649->k051649_keyonoff_w(data);
			return;
		}

		m_k051649->k051649_test_w(data);
		return;
	}

	m_vram[offset] = data;
	m_ttl_tilemap->mark_tile_dirty(offset>>1);
}

void quickpick5_state::video_start()
{
	static const gfx_layout charlayout =
	{
		8, 8,   // 8x8
		4096,   // # of tiles
		4,      // 4bpp
		{ 0, 1, 2, 3 }, // plane offsets
		{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 }, // X offsets
		{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 }, // Y offsets
		8*8*4
	};

	int gfx_index;

	/* find first empty slot to decode gfx */
	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (m_gfxdecode->gfx(gfx_index) == nullptr)
			break;

	assert(gfx_index != MAX_GFX_ELEMENTS);

	// decode the ttl layer's gfx
	m_gfxdecode->set_gfx(gfx_index, std::make_unique<gfx_element>(m_palette, charlayout, memregion("ttl")->base(), 0, m_palette->entries() / 16, 0));
	m_ttl_gfx_index = gfx_index;

	m_ttl_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(quickpick5_state::ttl_get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_ttl_tilemap->set_transparent_pen(0);
	m_ttl_tilemap->set_scrollx(80);
	m_ttl_tilemap->set_scrolly(28);

	m_ttlrom_offset = 0;
	m_ccu_int_time_count = 0;
	m_ccu_int_time = 20;
}

TILE_GET_INFO_MEMBER(quickpick5_state::ttl_get_tile_info)
{
	uint8_t *lvram = &m_vram[0];
	int attr, code;

	attr = lvram[BYTE_XOR_LE((tile_index<<1)+1)];
	code = lvram[BYTE_XOR_LE((tile_index<<1))] | ((attr & 0xf) << 8);
	attr >>= 3;
	attr &= ~1;

	SET_TILE_INFO_MEMBER(m_ttl_gfx_index, code, attr, 0);
}

uint32_t quickpick5_state::screen_update_quickpick5(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	screen.priority().fill(0, cliprect);
	m_title_hack = false;
	m_k053245->sprites_draw(bitmap, cliprect, screen.priority());

	if (!m_title_hack)
	{
		m_ttl_tilemap->draw(screen, bitmap, cliprect, 0, 0xff);
	}

	return 0;
}

K05324X_CB_MEMBER(quickpick5_state::sprite_callback)
{
	*code = (*code & 0x7ff);
	*color = (*color & 0x003f);

	if (*code == 0x520)
	{
		m_title_hack = true;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(quickpick5_state::scanline)
{
	int scanline = param;

	// z80 /IRQ is connected to the IRQ1(vblank) pin of k053252 CCU
	if(scanline == 255)
	{
		m_maincpu->set_input_line(0, ASSERT_LINE);
	}

	// z80 /NMI is connected to the IRQ2 pin of k053252 CCU
	// the following code is emulating INT_TIME of the k053252, this code will go away
	// when the new konami branch is merged.
	m_ccu_int_time_count--;
	if (m_ccu_int_time_count <= 0)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		m_ccu_int_time_count = m_ccu_int_time;
	}
}

void quickpick5_state::quickpick5_main(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc000, 0xdbff).ram().share("nvram");
	map(0xdc00, 0xdc0f).rw(m_k053252, FUNC(k053252_device::read), FUNC(k053252_device::write));
	map(0xdc40, 0xdc4f).rw(FUNC(quickpick5_state::k244_r), FUNC(quickpick5_state::k244_w));
	map(0xdc80, 0xdc80).portr("DSW3");
	map(0xdc81, 0xdc81).portr("DSW4");
	map(0xdcc0, 0xdcc0).portr("DSW1");
	map(0xdcc1, 0xdcc1).portr("DSW2");
	map(0xdd00, 0xdd00).nopw();
	map(0xdd40, 0xdd40).noprw();
	map(0xdd80, 0xdd80).rw(FUNC(quickpick5_state::control_r), FUNC(quickpick5_state::control_w));
	map(0xddc0, 0xddc0).nopw();
	map(0xde00, 0xde00).nopw();
	map(0xde40, 0xde40).w(m_oki, FUNC(okim6295_device::write));
	map(0xe000, 0xefff).rw(FUNC(quickpick5_state::vram_r), FUNC(quickpick5_state::vram_w));
	map(0xf000, 0xf7ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xf800, 0xffff).rw(FUNC(quickpick5_state::k245_r), FUNC(quickpick5_state::k245_w));
}

static INPUT_PORTS_START( quickpick5 )
	PORT_START("DSW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "Reset Switch" )   PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x02, 0x00, "Global Stats" )   PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x04, 0x00, "Last Game Stats" )   PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )   PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
INPUT_PORTS_END

void quickpick5_state::machine_start()
{
	membank("bank1")->configure_entries(0, 0x2, memregion("maincpu")->base()+0x8000, 0x4000);
	membank("bank1")->set_entry(0);
}

void quickpick5_state::machine_reset()
{
}

void quickpick5_state::quickpick5(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(32'000'000)/4); // z84c0008pec 8mhz part, 32Mhz xtal verified on PCB, divisor unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &quickpick5_state::quickpick5_main);
	TIMER(config, "scantimer").configure_scanline(FUNC(quickpick5_state::scanline), "screen", 0, 1);

	K053252(config, m_k053252, XTAL(32'000'000)/4); /* K053252, xtal verified, divider not verified */
	m_k053252->int1_ack().set(FUNC(quickpick5_state::vbl_ack_w));
	m_k053252->int2_ack().set(FUNC(quickpick5_state::nmi_ack_w));
	m_k053252->int_time().set(FUNC(quickpick5_state::ccu_int_time_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.62);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(20));
	screen.set_size(64*8, 33*8);
	screen.set_visarea(88, 456-1, 28, 256-1);
	screen.set_screen_update(FUNC(quickpick5_state::screen_update_quickpick5));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);
	m_palette->enable_shadows();

	K053245(config, m_k053245, 0);
	m_k053245->set_palette(m_palette);
	m_k053245->set_offsets(-(44+80), 20);
	m_k053245->set_sprite_callback(FUNC(quickpick5_state::sprite_callback), this);

	GFXDECODE(config, m_gfxdecode, m_palette, gfxdecode_device::empty);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	K051649(config, m_k051649, XTAL(32'000'000)/18);  // xtal is verified, divider is not
	m_k051649->add_route(ALL_OUTPUTS, "mono", 0.45);

	OKIM6295(config, m_oki, XTAL(32'000'000)/18, okim6295_device::PIN7_HIGH);
	m_oki->add_route(0, "mono", 1.0);
	m_oki->add_route(1, "mono", 1.0);
}

ROM_START( quickp5 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "117.10e.bin",  0x000000, 0x010000, CRC(3645e1a5) SHA1(7d0d98772f3732510e7a58f50a622fcec74087c3) )

	ROM_REGION( 0x40000, "k053245", 0 )   /* sprites */
	ROM_LOAD32_BYTE( "117-a02-7k.bin", 0x000003, 0x010000, CRC(745a1dc9) SHA1(33d876fb70cb802d62f87ad3721740e0961c7bec) )
	ROM_LOAD32_BYTE( "117-a03-7l.bin", 0x000002, 0x010000, CRC(07ec6db7) SHA1(7a94efc5f313fee6b9b63b7d2b6ba1cbf4158900) )
	ROM_LOAD32_BYTE( "117-a04-3l.bin", 0x000001, 0x010000, CRC(08dba5df) SHA1(2174be21c5a7db31ccc20ca0b88e4a94145776a5) )
	ROM_LOAD32_BYTE( "117-a05-3k.bin", 0x000000, 0x010000, CRC(9b2d0501) SHA1(3f1c69ef101153da5ac3335585541006c42e954d) )

	ROM_REGION( 0x80000, "ttl", 0 ) /* TTL text tilemap characters? */
	ROM_LOAD( "117-18e.bin",  0x000000, 0x020000, CRC(10e0d1e2) SHA1(f4ba190814d5e3f3e910c9da24845b6ddb259bff) )

	ROM_REGION( 0x20000, "oki", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "117-a01-2e.bin", 0x000000, 0x020000, CRC(3d8fbd01) SHA1(f350da2a4e7bfff9975188a39acf73415bd85b3d) )

	ROM_REGION( 0x80000, "pals", 0 )
	ROM_LOAD( "054590.11g",   0x000000, 0x040000, CRC(0442621c) SHA1(2e79bea4e37028a3c1223fb4e3b3e12ccad2b39b) )
	ROM_LOAD( "054591.12g",   0x040000, 0x040000, CRC(eaa92d8f) SHA1(7a430f11127148f0c035973ce21cfec4cb60ce9d) )

ROM_END

GAME( 1995, quickp5, 0, quickpick5, quickpick5,  quickpick5_state, empty_init, ROT0, "Konami", "Quick Pick 5", MACHINE_NOT_WORKING)

