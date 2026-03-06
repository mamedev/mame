// license:BSD-3-Clause
// copyright-holders:Darren Olafson, Quench, Stephane Humbert
/***************************************************************************

        Toaplan Rally Bike/Dash Yarou hardware
        ------------------------------------
        MAME Driver by: Darren Olafson
        Technical info: Carl-Henrik Skarstedt  &  Magnus Danielsson
        Driver updates: Quench
        Video updates : SUZ
        Driven from toaplan/toaplan1.cpp


Supported games:

    ROM set     Toaplan
    name        board No        Game name
    --------------------------------------------------
    rallybik    TP-O12      Rally Bike/Dash Yarou


Stephh's and AWJ's notes (based on the games M68000 and Z80 code and some tests) :

1) 'rallybik'

  - Region read from DSWB (port 0x50 in CPU1) then stored at 0x8004 (CPU1 shared RAM) =
    0x180008.w (CPU0 shared RAM) then stored at 0x0804f4.w .
  - Coinage relies on bits 4 and 5 of the region (code at 0x0bda in CPU1) :
      * ..10.... : TOAPLAN_COINAGE_WORLD (tables at 0x0c35 (COIN1) and 0x0c3d (COIN2) in CPU1)
      *  else    : TOAPLAN_COINAGE_JAPAN (table at 0x0c25 (COIN1 AND COIN2) in CPU1)
  - Title screen relies on bits 4 and 5 of the region (code at 0x00220e) :
      * ..00.... : "Dash Yarou"
      *  else    : "Rally Bike"
  - Notice screen relies on bits 4 and 5 of the region (code at 0x001ac0) :
      * ..00.... : "FOR USE IN JAPAN ONLY"
      *  else    : no notice screen
  - Copyright relies on bits 4 and 5 of the region (code at 0x001e68) :
      * ..00.... : "TAITO CORPORATION" / "ALL RIGHTS RESERVED"
      * ..01.... : "TAITO AMERICA CORP." / "ALL RIGHTS RESERVED"
      * ..10.... : "TAITO CORP. JAPAN" / "ALL RIGHTS RESERVED"
      * ..11.... : "TAITO AMERICA CORP." / "LICENCED TO ROMSTAR"
  - Number of letters for initials relies on bits 4 and 5 of the region
    (code at 0x0008fe = init - code at 0x0022e8 = enter) :
      * ..00.... : 6 letters
      *  else    : 3 letters
  - To enter the "test mode", press START1 when the grid is displayed.
  - When "TEST" Switch is ON, you can do the following with the STARTn buttons :
      * press START2 to pause game
      * press START1 to unpause game
      * when START1 and START2 are pressed, the game enters in "slow motion" mode
  - When "TEST" Switch is ON, collision and fuel consuption routines are not called.
    Don't forget to turn the "TEST" Switch OFF when time is over on bonus stage,
    or the level will never end !
  - When cabinet is set to "Upright", you can use joystick and buttons from both players
    (code at 0x001c44).

***************************************************************************/

#include "emu.h"

#include "toaplan_bcu.h"
#include "toaplan_scu.h"
#include "toaplan_video_controller.h"
#include "toaplipt.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "sound/ymopl.h"
#include "video/bufsprite.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class rallybik_state : public driver_device
{
public:
	rallybik_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_ymsnd(*this, "ymsnd"),
		m_spritegen(*this, "scu"),
		m_bcu(*this, "bcu"),
		m_vctrl(*this, "vctrl"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette_%u", 0U),
		m_paletteram(*this, "paletteram_%u", 0U, 0x800U, ENDIANNESS_BIG),
		m_sharedram(*this, "sharedram"),
		m_spriteram(*this, "spriteram")
	{ }

	void rallybik(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<m68000_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<ym3812_device> m_ymsnd;
	required_device<toaplan_scu_device> m_spritegen;
	required_device<toaplan_bcu_device> m_bcu;
	required_device<toaplan_video_controller_device> m_vctrl;
	required_device<screen_device> m_screen;
	required_device_array<palette_device, 2> m_palette;

	memory_share_array_creator<u16, 2> m_paletteram;

	required_shared_ptr<u8> m_sharedram;
	required_device<buffered_spriteram16_device> m_spriteram;

	u8 shared_r(offs_t offset);
	void shared_w(offs_t offset, u8 data);
	void reset_sound_w(u8 data);

	u16 tileram_r(offs_t offset);

	template <unsigned Which> void coin_counter_w(int state);
	template <unsigned Which> void coin_lockout_w(int state);

	void pri_cb(u8 priority, u32 &pri_mask);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	void main_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

/***************************** Video handlers *****************************/

u16 rallybik_state::tileram_r(offs_t offset)
{
	u16 data = m_bcu->tileram_r(offset);

	if (offset == 0)    /* some bit lines may be stuck to others */
	{
		data |= ((data & 0xf000) >> 4);
		data |= ((data & 0x0030) << 2);
	}
	return data;
}

void rallybik_state::pri_cb(u8 priority, u32 &pri_mask)
{
	switch (priority)
	{
		case 0: pri_mask = GFX_PMASK_1|GFX_PMASK_2|GFX_PMASK_4|GFX_PMASK_8; break; // disable?
		case 1: pri_mask = GFX_PMASK_2|GFX_PMASK_4|GFX_PMASK_8;             break; // over tilemap priority ~0x4, under tilemap priority 0x5~
		case 2: pri_mask = GFX_PMASK_4|GFX_PMASK_8;                         break; // over tilemap priority ~0x8, under tilemap priority 0x9~
		case 3: pri_mask = GFX_PMASK_8;                                     break; // over tilemap priority ~0xc, under tilemap priority 0xd~
	}
}

u32 rallybik_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	// first draw everything, including "disabled" tiles and priority 0
	m_bcu->draw_background(screen, bitmap, cliprect, 0, 1);

	// then draw the higher priority layers in order
	for (int priority = 1; priority < 16; priority++)
	{
		// get priority mask
		// value: 0x1 (tilemap priority 0x0~0x4), 0x2 (tilemap priority 0x5~0x8), 0x4 (tilemap priority 0x9~0xc), 0x8 (tilemap priority 0xd~)
		const u32 primask = 1 << (((priority - 1) >> 2));
		m_bcu->draw_tilemap(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(priority), primask);
	}

	m_spritegen->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), m_spriteram->bytes());
	return 0;
}

void rallybik_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		if (m_vctrl->intenable())
			m_maincpu->set_input_line(4, HOLD_LINE);
	}
}

/***************************** Read/Write handlers *****************************/

u8 rallybik_state::shared_r(offs_t offset)
{
	return m_sharedram[offset];
}

void rallybik_state::shared_w(offs_t offset, u8 data)
{
	m_sharedram[offset] = data;
}


void rallybik_state::reset_sound_w(u8 data)
{
	if (data == 0)
	{
		m_ymsnd->reset();
		m_audiocpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	}
}

template <unsigned Which>
void rallybik_state::coin_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(Which, state);
}

template <unsigned Which>
void rallybik_state::coin_lockout_w(int state)
{
	machine().bookkeeping().coin_lockout_w(Which, !state);
}

/***************************** 68000 Memory Map *****************************/

void rallybik_state::main_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x040000, 0x07ffff).rom();
	map(0x080000, 0x083fff).ram();
	map(0x0c0000, 0x0c0fff).ram().share("spriteram");
	map(0x100000, 0x10001f).m(m_bcu, FUNC(toaplan_bcu_device::host_map));
	map(0x100004, 0x100007).r(FUNC(rallybik_state::tileram_r));
	map(0x140000, 0x147fff).m(m_vctrl, FUNC(toaplan_video_controller_device::host_map));
	map(0x180000, 0x180fff).rw(FUNC(rallybik_state::shared_r), FUNC(rallybik_state::shared_w)).umask16(0x00ff);
	map(0x1c0000, 0x1c0003).w(m_bcu, FUNC(toaplan_bcu_device::tile_offsets_w));
	map(0x1c8001, 0x1c8001).w(FUNC(rallybik_state::reset_sound_w));
}

/***************************** Z80 Memory Map *******************************/

void rallybik_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram().share(m_sharedram);
}

void rallybik_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("P1");
	map(0x10, 0x10).portr("P2");
	map(0x20, 0x20).portr("SYSTEM");
	map(0x30, 0x30).w("coinlatch", FUNC(ls259_device::write_nibble_d0));  /* Coin counter/lockout */
	map(0x40, 0x40).portr("DSWA");
	map(0x50, 0x50).portr("DSWB");
	map(0x60, 0x61).rw(m_ymsnd, FUNC(ym3812_device::read), FUNC(ym3812_device::write));
}

/*****************************************************************************
    Input Port definitions
*****************************************************************************/

/* verified from M68000 and Z80 code */
static INPUT_PORTS_START( rallybik )
	PORT_START("P1")
	TOAPLAN_JOY_UDLR_2_BUTTONS( 1 )

	PORT_START("P2")
	TOAPLAN_JOY_UDLR_2_BUTTONS( 2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_TILT )
	TOAPLAN_TEST_SWITCH( 0x04, IP_ACTIVE_HIGH ) /* see notes */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("VBLANK")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0xfffe, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	/* in 0x40 (CPU1) -> 0x8003 (CPU1 shared RAM) = 0x180006.w (CPU0 shared RAM) -> 0x0804f2.w */
	PORT_START("DSWA")
	TOAPLAN_MACHINE_COCKTAIL_LOC(SW1)
	TOAPLAN_COINAGE_DUAL_LOC(DSWB, 0x30, 0x20, SW1)                  /* see notes */

	/* in 0x50 (CPU1) -> 0x8004 (CPU1 shared RAM) = 0x180008.w (CPU0 shared RAM) -> 0x0804f4.w */
	PORT_START("DSWB")
	TOAPLAN_DIFFICULTY_LOC(SW2)
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_HIGH )                PORT_DIPLOCATION("SW2:!3")
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_HIGH )                PORT_DIPLOCATION("SW2:!4")
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Region ) )         PORT_DIPLOCATION("SW2:!5,!6") /* bits 4 and 5 listed as unused in the Dip Switches screen */
	PORT_DIPSETTING(    0x20, DEF_STR( Europe ) )                                       /* Taito Corp. Japan */
	PORT_DIPSETTING(    0x10, DEF_STR( USA ) )                                          /* Taito America Corp. */
	PORT_DIPSETTING(    0x30, "USA (Romstar license)" )                                 /* Taito America Corp. */
	PORT_DIPSETTING(    0x00, DEF_STR( Japan ) )                                        /* Taito Corporation */
	PORT_DIPNAME( 0x40, 0x00, "Dip Switch Display" )      PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:!8")     /* not on race 1 */
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	/* P1 : in 0x00 (CPU1) -> 0x8006 (CPU1 shared RAM) = 0x18000c.w (CPU0 shared RAM) */
	/* P2 : in 0x10 (CPU1) -> 0x8007 (CPU1 shared RAM) = 0x18000e.w (CPU0 shared RAM) */
	/* SYSTEM : in 0x20 (CPU1) -> 0x8005 (CPU1 shared RAM) = 0x18000a.w (CPU0 shared RAM) -> 0x0804f6.w */
	/* VBLANK : 0x140000.w */
INPUT_PORTS_END


static const gfx_layout tilelayout =
{
	8,8,            /* 8x8 */
	RGN_FRAC(1,2),  /* 16384/32768 tiles */
	4,              /* 4 bits per pixel */
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	16*8            /* every tile takes 16 consecutive bytes */
};

static GFXDECODE_START( gfx_bcu )
	GFXDECODE_ENTRY( "tiles", 0, tilelayout, 0, 64 )
GFXDECODE_END

void rallybik_state::machine_reset()
{
	machine().bookkeeping().coin_lockout_global_w(0);
}


void rallybik_state::rallybik(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(10'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &rallybik_state::main_map);

	Z80(config, m_audiocpu, XTAL(28'000'000) / 8);
	m_audiocpu->set_addrmap(AS_PROGRAM, &rallybik_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &rallybik_state::sound_io_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	ls259_device &coinlatch(LS259(config, "coinlatch")); // 19L
	coinlatch.q_out_cb<4>().set(FUNC(rallybik_state::coin_counter_w<0>));
	coinlatch.q_out_cb<5>().set(FUNC(rallybik_state::coin_counter_w<1>));
	coinlatch.q_out_cb<6>().set(FUNC(rallybik_state::coin_lockout_w<0>));
	coinlatch.q_out_cb<7>().set(FUNC(rallybik_state::coin_lockout_w<1>));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	// Parameter uploaded to CRTC
	m_screen->set_raw(XTAL(28'000'000) / 4, (224+1)*2, 0, 320, (140+1)*2, 0, 240);
	m_screen->set_screen_update(FUNC(rallybik_state::screen_update));
	m_screen->screen_vblank().set(m_spriteram, FUNC(buffered_spriteram16_device::vblank_copy_rising));
	m_screen->screen_vblank().append(FUNC(rallybik_state::screen_vblank));

	BUFFERED_SPRITERAM16(config, m_spriteram);

	TOAPLAN_BCU(config, m_bcu, XTAL(28'000'000), m_palette[0], gfx_bcu);
	m_bcu->set_offset(-0x00d, -0x111, -0x80, 0x8);

	TOAPLAN_SCU(config, m_spritegen, XTAL(28'000'000));
	m_spritegen->set_screen(m_screen);
	m_spritegen->set_palette(m_palette[1]);
	m_spritegen->set_xoffsets(31, 15);
	m_spritegen->set_pri_callback(FUNC(rallybik_state::pri_cb));

	TOAPLAN_VIDEO_CONTROLLER(config, m_vctrl, XTAL(28'000'000));
	m_vctrl->set_screen(m_screen);
	m_vctrl->set_palette_tag(0, m_palette[0]);
	m_vctrl->set_palette_tag(1, m_palette[1]);
	m_vctrl->set_paletteram_tag(0, m_paletteram[0]);
	m_vctrl->set_paletteram_tag(1, m_paletteram[1]);
	m_vctrl->set_byte_per_color(0, 2);
	m_vctrl->set_byte_per_color(1, 2);
	m_vctrl->vblank_callback().set_ioport("VBLANK");

	PALETTE(config, m_palette[0]).set_format(palette_device::xBGR_555, 0x400);
	PALETTE(config, m_palette[1]).set_format(palette_device::xBGR_555, 0x400);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM3812(config, m_ymsnd, XTAL(28'000'000) / 8);
	m_ymsnd->irq_handler().set_inputline(m_audiocpu, 0);
	m_ymsnd->add_route(ALL_OUTPUTS, "mono", 1.0);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( rallybik )
	ROM_REGION( 0x080000, "maincpu", 0 ) // Main 68K code
	ROM_LOAD16_BYTE( "b45-02.rom",  0x000000, 0x08000, CRC(383386d7) SHA1(fc420b6adc79a408a68f0661d0c62ed7dbe8b6d7) )
	ROM_LOAD16_BYTE( "b45-01.rom",  0x000001, 0x08000, CRC(7602f6a7) SHA1(2939c261a4bc63586681080f5643916c85e81c7d) )
	ROM_LOAD16_BYTE( "b45-04.rom",  0x040000, 0x20000, CRC(e9b005b1) SHA1(19b5acfd5fb2683a56a701400b11ee6f64a9bdf1) )
	ROM_LOAD16_BYTE( "b45-03.rom",  0x040001, 0x20000, CRC(555344ce) SHA1(398963f488fe6f19c0b8518d80c946c242d0fc45) )

	ROM_REGION( 0x8000, "audiocpu", 0 ) // Sound Z80 code
	ROM_LOAD( "b45-05.rom",  0x0000, 0x4000, CRC(10814601) SHA1(bad7a834d8849752a7f3000bb5154ec0fa50d695) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD16_BYTE( "b45-09.bin",  0x00000, 0x20000, CRC(1dc7b010) SHA1(67e8633bd787ffcae0e7867e7e591c492c4f2d63) )
	ROM_LOAD16_BYTE( "b45-08.bin",  0x00001, 0x20000, CRC(fab661ba) SHA1(acc43cd6d979b1c6a348727f315643d7b8f1496a) )
	ROM_LOAD16_BYTE( "b45-07.bin",  0x40000, 0x20000, CRC(cd3748b4) SHA1(a20eb19a0f813112b4e5d9cd91db29de9b37af17) )
	ROM_LOAD16_BYTE( "b45-06.bin",  0x40001, 0x20000, CRC(144b085c) SHA1(84b7412d58fe9c5e9915896db92e80a621571b74) )

	ROM_REGION( 0x40000, "scu", 0 )
	ROM_LOAD( "b45-11.rom",  0x00000, 0x10000, CRC(0d56e8bb) SHA1(c29cb53f846c73b7cf9936051fb0f9dd3805f53f) )
	ROM_LOAD( "b45-10.rom",  0x10000, 0x10000, CRC(dbb7c57e) SHA1(268132965cd65b5e972ca9d0258c30b8a86f3703) )
	ROM_LOAD( "b45-12.rom",  0x20000, 0x10000, CRC(cf5aae4e) SHA1(5832c52d2e9b86414d8ee2926fa190abe9e41da4) )
	ROM_LOAD( "b45-13.rom",  0x30000, 0x10000, CRC(1683b07c) SHA1(54356893357cd1297f24f1d85b7289d80740262d) )

	ROM_REGION( 0x240, "proms", 0 ) // nibble BPROMs, lo/hi order to be determined
	ROM_LOAD( "b45-15.bpr",  0x000, 0x100, CRC(24e7d62f) SHA1(1c06a1ef1b6a722794ca1d5ee2c476ecaa5178a3) ) // sprite priority control ??
	ROM_LOAD( "b45-16.bpr",  0x100, 0x100, CRC(a50cef09) SHA1(55cafb5b2551b80ae708e9b966cf37c70a16d310) ) // sprite priority control ??
	ROM_LOAD( "b45-14.bpr",  0x200, 0x020, CRC(f72482db) SHA1(b0cb911f9c81f6088a5aa8760916ddae1f8534d7) ) // sprite control ??
	ROM_LOAD( "b45-17.bpr",  0x220, 0x020, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) ) // sprite attribute (flip/position) ??
ROM_END

} // Anonymous namespace

//    YEAR  NAME        PARENT    MACHINE   INPUT      CLASS           INIT        ROT     COMPANY                        FULLNAME                                        FLAGS
GAME( 1988, rallybik,   0,        rallybik, rallybik,  rallybik_state, empty_init, ROT270, "Toaplan / Taito Corporation", "Rally Bike (Europe, US) / Dash Yarou (Japan)", 0 )
