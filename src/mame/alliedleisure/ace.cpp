// license:GPL-2.0+
// copyright-holders:Jarek Burczynski
/****************************************************************************

Ace by Allied Leisure

Driver by Jarek Burczynski
2002.09.19



Allied Leisure 1976
"MAJOR MFG. INC. SUNNYVALE, CA" in PCB etch

18MHz
                                                          5MHz

8080


2101
2101


A5               3106          3106         3106
A4
A3                                                      3622.K4
A2                                   2101
A1                   2101            2101

                                                         [ RANGE ] [ TIME ]
                                                        (two 0-9 thumbwheel switches)


5x2101 - SRAM 256x4
3x3106 - SRAM 256x1
1x3622 - ROM 512x4


****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "emupal.h"
#include "screen.h"

#include "ace.lh"


namespace {

static constexpr XTAL MASTER_CLOCK = 18_MHz_XTAL;


class aceal_state : public driver_device
{
public:
	aceal_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_scoreram(*this, "scoreram"),
		m_ram2(*this, "ram2"),
		m_characterram(*this, "characterram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void ace(machine_config &config);

private:
	void ace_objpos_w(offs_t offset, uint8_t data);
	void ace_characterram_w(offs_t offset, uint8_t data);
	void ace_scoreram_w(offs_t offset, uint8_t data);
	uint8_t unk_r();
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_ace(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void ace_postload();
	void main_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;

	/* video-related */
	required_shared_ptr<uint8_t> m_scoreram;
	required_shared_ptr<uint8_t> m_ram2;
	required_shared_ptr<uint8_t> m_characterram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* input-related */
	int m_objpos[8];
};


void aceal_state::ace_objpos_w(offs_t offset, uint8_t data)
{
	m_objpos[offset] = data;
}

void aceal_state::video_start()
{
	m_gfxdecode->gfx(1)->set_source(m_characterram);
	m_gfxdecode->gfx(2)->set_source(m_characterram);
	m_gfxdecode->gfx(3)->set_source(m_characterram);
	m_gfxdecode->gfx(4)->set_source(m_scoreram);
}

uint32_t aceal_state::screen_update_ace(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	m_gfxdecode->gfx(1)->opaque(bitmap, cliprect, 0, 0, 0, 0, m_objpos[0], m_objpos[1]);
	m_gfxdecode->gfx(2)->opaque(bitmap, cliprect, 0, 0, 0, 0, m_objpos[2], m_objpos[3]);
	m_gfxdecode->gfx(3)->opaque(bitmap, cliprect, 0, 0, 0, 0, m_objpos[4], m_objpos[5]);

	for (int offs = 0; offs < 8; offs++)
		m_gfxdecode->gfx(4)->opaque(bitmap, cliprect, offs, 0, 0, 0, 10 * 8 + offs * 16, 256 - 16);

	return 0;
}


void aceal_state::ace_characterram_w(offs_t offset, uint8_t data)
{
	if (m_characterram[offset] != data)
	{
		if (data & ~0x07)
			logerror("write to %04x data = %02x\n", 0x8000 + offset, data);

		m_characterram[offset] = data;
		m_gfxdecode->gfx(1)->mark_dirty(0);
		m_gfxdecode->gfx(2)->mark_dirty(0);
		m_gfxdecode->gfx(3)->mark_dirty(0);
	}
}

void aceal_state::ace_scoreram_w(offs_t offset, uint8_t data)
{
	m_scoreram[offset] = data;
	m_gfxdecode->gfx(4)->mark_dirty(offset / 32);
}

uint8_t aceal_state::unk_r()
{
	return machine().rand() & 0xff;
}


/* 5x2101 - SRAM 256x4 */
/* 3x3106 - SRAM 256x1 */
/* 1x3622 - ROM 512x4  - doesn't seem to be used ????????????*/

void aceal_state::main_map(address_map &map)
{
	map(0x0000, 0x09ff).rom();

	map(0x2000, 0x20ff).ram().w(FUNC(aceal_state::ace_scoreram_w)).share("scoreram");  /* 2x2101 */
	map(0x8300, 0x83ff).ram().share("ram2");    /* 2x2101 */
	map(0x8000, 0x80ff).ram().w(FUNC(aceal_state::ace_characterram_w)).share("characterram");  /* 3x3101 (3bits: 0, 1, 2) */

	map(0xc000, 0xc005).w(FUNC(aceal_state::ace_objpos_w));

	/* players inputs */
	map(0xc008, 0xc008).portr("c008");
	map(0xc009, 0xc009).portr("c009");
	map(0xc00a, 0xc00a).portr("c00a");
	map(0xc00b, 0xc00b).portr("c00b");
	map(0xc00c, 0xc00c).portr("c00c");
	map(0xc00d, 0xc00d).portr("c00d");
	map(0xc00e, 0xc00e).portr("c00e");
	map(0xc00f, 0xc00f).portr("c00f");
	map(0xc010, 0xc010).portr("c010");
	map(0xc011, 0xc011).portr("c011");

	map(0xc012, 0xc012).r(FUNC(aceal_state::unk_r));

	/* vblank */
	map(0xc014, 0xc014).portr("c014");

	/* coin */
	map(0xc015, 0xc015).portr("c015");

	/* start (must read 1 at least once to make the game run) */
	map(0xc016, 0xc016).portr("c016");

	map(0xc017, 0xc017).r(FUNC(aceal_state::unk_r));
	map(0xc018, 0xc018).r(FUNC(aceal_state::unk_r));
	map(0xc019, 0xc019).r(FUNC(aceal_state::unk_r));

	map(0xc020, 0xc020).r(FUNC(aceal_state::unk_r));
	map(0xc021, 0xc021).r(FUNC(aceal_state::unk_r));
	map(0xc022, 0xc022).r(FUNC(aceal_state::unk_r));
	map(0xc023, 0xc023).r(FUNC(aceal_state::unk_r));
	map(0xc024, 0xc024).r(FUNC(aceal_state::unk_r));
	map(0xc025, 0xc025).r(FUNC(aceal_state::unk_r));
	map(0xc026, 0xc026).r(FUNC(aceal_state::unk_r));
}


static INPUT_PORTS_START( ace )
	PORT_START("c008")  /* player thrust */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_2WAY PORT_PLAYER(1) PORT_NAME("P1 Thrust")

	PORT_START("c009")  /* player slowdown */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_2WAY PORT_PLAYER(1) PORT_NAME("P1 Slowdown")

	PORT_START("c00a")  /* player left */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)

	PORT_START("c00b")  /* player right */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)

	PORT_START("c00c")  /* player fire */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Fire")

	PORT_START("c00d")  /* enemy thrust */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_2WAY PORT_PLAYER(2) PORT_NAME("P2 Thrust")

	PORT_START("c00e")  /* enemy slowdown */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_2WAY PORT_PLAYER(2) PORT_NAME("P2 Slowdown")

	PORT_START("c00f")  /* enemy left  */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)

	PORT_START("c010")  /* enemy right */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)

	PORT_START("c011")  /* enemy fire */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Fire")

	//c012

	PORT_START("c014")  /* VBLANK??? */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("c015")  /* coin input */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("c016")  /* game start */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	16,16,  /* 16*16 chars */
	8,  /* 8 characters */
	1,      /* 1 bit per pixel */
	{ 4 },  /* character rom is 512x4 bits (3622 type)*/
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3, 16+0, 16+1, 16+2, 16+3, 24+0, 24+1, 24+2, 24+3 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	64*8    /* every char takes 64 consecutive bytes */
};

static const gfx_layout charlayout0 =
{
	16,16,  /* 16*16 chars */
	1,  /* 1 characters */
	1,      /* 1 bit per pixel */
	{ 7 },  /* bit 0 in character ram */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128, 8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
	256*8   /* every char takes 256 consecutive bytes */
};

static const gfx_layout charlayout1 =
{
	16,16,  /* 16*16 chars */
	1,  /* 1 characters */
	1,      /* 1 bit per pixel */
	{ 6 },  /* bit 1 in character ram */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128, 8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
	256*8   /* every char takes 256 consecutive bytes */
};

static const gfx_layout charlayout2 =
{
	16,16,  /* 16*16 chars */
	1,  /* 1 characters */
	1,      /* 1 bit per pixel */
	{ 5 },  /* bit 2 in character ram */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128, 8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
	256*8   /* every char takes 256 consecutive bytes */
};

static const gfx_layout scorelayout =
{
	16,16,  /* 16*16 chars */
	8,  /* 8 characters */
	1,      /* 1 bit per pixel */
	{ 0 },  /*  */
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	32*8    /* every char takes 32 consecutive bytes */
};

static GFXDECODE_START( gfx_ace )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,  0, 2 )
	GFXDECODE_ENTRY( nullptr, 0x8000, charlayout0, 0, 2 ) /* the game dynamically modifies this */
	GFXDECODE_ENTRY( nullptr, 0x8000, charlayout1, 0, 2 ) /* the game dynamically modifies this */
	GFXDECODE_ENTRY( nullptr, 0x8000, charlayout2, 0, 2 ) /* the game dynamically modifies this */
	GFXDECODE_ENTRY( nullptr, 0x8000, scorelayout, 0, 2 ) /* the game dynamically modifies this */
GFXDECODE_END

void aceal_state::ace_postload()
{
	m_gfxdecode->gfx(1)->mark_dirty(0);
	m_gfxdecode->gfx(2)->mark_dirty(0);
	m_gfxdecode->gfx(3)->mark_dirty(0);
	m_gfxdecode->gfx(4)->mark_dirty(0);
}

void aceal_state::machine_start()
{
	save_item(NAME(m_objpos));
	machine().save().register_postload(save_prepost_delegate(FUNC(aceal_state::ace_postload), this));
}

void aceal_state::machine_reset()
{
	for (auto & elem : m_objpos)
		elem = 0;
}

void aceal_state::ace(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, MASTER_CLOCK/9); /* 2 MHz ? */
	m_maincpu->set_addrmap(AS_PROGRAM, &aceal_state::main_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(32*8, 32*8);
	screen.set_visarea(4*8, 32*8-1, 2*8, 32*8-1);
	screen.set_screen_update(FUNC(aceal_state::screen_update_ace));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ace);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* sound hardware */
	/* ???? */
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( ace )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ace.a1",     0x0000, 0x0200, CRC(16811834) SHA1(5502812dd161908eea3fa8851d7e5c1e22b0f8ff) )
	ROM_LOAD( "ace.a2",     0x0200, 0x0200, CRC(f9eae80e) SHA1(8865b86c7b5d57c76312c16f8a614bf35ffaf532) )
	ROM_LOAD( "ace.a3",     0x0400, 0x0200, CRC(c5c63b8c) SHA1(2079dd12ff0c4aafec19aeb9baa70fc9b6788356) )
	ROM_LOAD( "ace.a4",     0x0600, 0x0200, CRC(ea4503aa) SHA1(fea610124b9f7ea18d29b4e4599253ba1ee067e1) )
	ROM_LOAD( "ace.a5",     0x0800, 0x0200, CRC(623c58e7) SHA1(a92418bc323a1ae76eae8e094e4d6ebd1e8da14e) )

	/* not used - I couldn't guess when this should be displayed */
	ROM_REGION( 0x0200, "gfx1", 0 )
	ROM_LOAD( "ace.k4",     0x0000, 0x0200, CRC(daa05ec6) SHA1(8b71ffb802293dc93f6b492ff128a704e676a5fd) )
ROM_END

} // anonymous namespace


GAMEL(1976, ace, 0, ace, ace, aceal_state, empty_init, ROT0, "Allied Leisure", "Ace", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND, layout_ace )
