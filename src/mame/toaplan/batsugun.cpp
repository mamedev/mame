// license:BSD-3-Clause
// copyright-holders:Quench, Yochizo, David Haywood

#include "emu.h"

#include "gp9001.h"
#include "toaplan_coincounter.h"
#include "toaplipt.h"

#include "cpu/m68000/m68000.h"
#include "cpu/nec/v25.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

/*
Name        Board No      Maker         Game name
----------------------------------------------------------------------------
batsugun    TP-030        Toaplan       Batsugun
batsuguna   TP-030        Toaplan       Batsugun (older)
batsugunsp  TP-030        Toaplan       Batsugun (Special Version)

batsugun - The Special Version has many changes to make the game easier: it adds an autofire button,
            replaces the regular bomb with the more powerful double bomb (which in the original version
            required both players in a two player game to press their bomb buttons at once), gives you
            a shield that can absorb one hit each time your ship "levels up", etc. It also changes the
            colors of the title screen, ship select screen, stages, and enemies.
            batsugun compared to batsuguna has code that looks more like the Special Version, but it
            doesn't have any of the Special Version features. All the differences between batsugun
            and batsuguna look like bug fixes that were carried over into the Special Version.
*/

namespace {

class batsugun_state : public driver_device
{
public:
	batsugun_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_vdp(*this, "gp9001_%u", 0U)
		, m_oki(*this, "oki")
		, m_palette(*this, "palette")
		, m_shared_ram(*this, "shared_ram")
		, m_audiocpu(*this, "audiocpu")
		, m_screen(*this, "screen")
		, m_coincounter(*this, "coincounter")
	{ }

	void batsugun(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	required_device<m68000_base_device> m_maincpu;
	required_device_array<gp9001vdp_device, 2> m_vdp;
	required_device<okim6295_device> m_oki;
	required_device<palette_device> m_palette;

private:
	u32 screen_update_batsugun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void batsugun_68k_mem(address_map &map) ATTR_COLD;
	void v25_mem(address_map &map) ATTR_COLD;
	void coin_sound_reset_w(u8 data);

	u8 shared_ram_r(offs_t offset) { return m_shared_ram[offset]; }
	void shared_ram_w(offs_t offset, u8 data) { m_shared_ram[offset] = data; }

	void screen_vblank(int state);

	void reset_audiocpu(int state);

	optional_shared_ptr<u8> m_shared_ram;  // 8 bit RAM shared between 68K and sound CPU
	optional_device<cpu_device> m_audiocpu;
	required_device<screen_device> m_screen;
	required_device<toaplan_coincounter_device> m_coincounter;
	bitmap_ind8 m_custom_priority_bitmap;
	bitmap_ind16 m_secondary_render_bitmap;
};

class batsugun_bootleg_state : public batsugun_state
{
public:
	batsugun_bootleg_state(const machine_config &mconfig, device_type type, const char *tag)
		: batsugun_state(mconfig, type, tag)
		, m_okibank(*this, "okibank")
	{ }

	void batsugunbl(machine_config &config);

	void init_batsugunbl();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void batsugunbl_oki_bankswitch_w(u8 data);
	void batsugunbl_68k_mem(address_map &map) ATTR_COLD;
	void cpu_space_batsugunbl_map(address_map &map);
	void fixeightbl_oki(address_map &map) ATTR_COLD;

	required_memory_bank m_okibank;
};


void batsugun_bootleg_state::video_start()
{
	batsugun_state::video_start();

	// This bootleg has additional layer offsets. TODO: further refinement needed
	m_vdp[0]->set_tm_extra_offsets(0, 0, 0, 0, 0);
	m_vdp[0]->set_tm_extra_offsets(1, 0, 0, 0, 0);
	m_vdp[0]->set_tm_extra_offsets(2, 0, 0, 0, 0);
	m_vdp[0]->set_sp_extra_offsets(0x37, 0x07, 0, 0);

	m_vdp[1]->set_tm_extra_offsets(0, -0x05, 0x07, 0, 0);
	m_vdp[1]->set_tm_extra_offsets(1, -0x05, 0x07, 0, 0);
	m_vdp[1]->set_tm_extra_offsets(2, 0, 0, 0, 0);
	m_vdp[1]->set_sp_extra_offsets(0x39, 0x12, 0, 0);

	m_vdp[0]->init_scroll_regs();
	m_vdp[1]->init_scroll_regs();
}

// renders to 2 bitmaps, and mixes output
u32 batsugun_state::screen_update_batsugun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_custom_priority_bitmap.fill(0, cliprect);
	m_vdp[0]->render_vdp(bitmap, cliprect);

	m_secondary_render_bitmap.fill(0, cliprect);
	m_custom_priority_bitmap.fill(0, cliprect);
	m_vdp[1]->render_vdp(m_secondary_render_bitmap, cliprect);

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		u16 *const src_vdp0 = &bitmap.pix(y);
		u16 const *const src_vdp1 = &m_secondary_render_bitmap.pix(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			const u16 GPU0_LUTaddr = src_vdp0[x];
			const u16 GPU1_LUTaddr = src_vdp1[x];
			const bool COMPARISON = ((GPU0_LUTaddr & 0x0780) > (GPU1_LUTaddr & 0x0780));

			if (!(GPU1_LUTaddr & 0x000f))
			{
				src_vdp0[x] = GPU0_LUTaddr;
			}
			else
			{
				if (!(GPU0_LUTaddr & 0x000f))
				{
					src_vdp0[x] = GPU1_LUTaddr; // bg pen
				}
				else
				{
					if (COMPARISON)
					{
						src_vdp0[x] = GPU1_LUTaddr;
					}
					else
					{
						src_vdp0[x] = GPU0_LUTaddr;
					}

				}
			}
		}
	}

	return 0;
}

void batsugun_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		m_vdp[0]->screen_eof();
		m_vdp[1]->screen_eof();
	}
}


void batsugun_state::reset_audiocpu(int state)
{
	if (state)
		coin_sound_reset_w(0);
}

void batsugun_state::machine_reset()
{
	if (m_audiocpu)
		coin_sound_reset_w(0);
}

void batsugun_state::video_start()
{
	m_screen->register_screen_bitmap(m_custom_priority_bitmap);
	m_secondary_render_bitmap.reset();
	m_vdp[0]->custom_priority_bitmap = &m_custom_priority_bitmap;
	m_screen->register_screen_bitmap(m_secondary_render_bitmap);
	m_vdp[1]->custom_priority_bitmap = &m_custom_priority_bitmap;
}

void batsugun_bootleg_state::fixeightbl_oki(address_map &map)
{
	map(0x00000, 0x2ffff).rom();
	map(0x30000, 0x3ffff).bankr(m_okibank);
}


void batsugun_state::coin_sound_reset_w(u8 data)
{
	m_coincounter->coin_w(data & ~0x20);
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 0x20) ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( base )
	PORT_START("IN1")
	TOAPLAN_JOY_UDLR_3_BUTTONS( 1 )

	PORT_START("IN2")
	TOAPLAN_JOY_UDLR_3_BUTTONS( 2 )

	PORT_START("SYS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_TILT )
	TOAPLAN_TEST_SWITCH( 0x04, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSWA")
	TOAPLAN_MACHINE_NO_COCKTAIL_LOC(SW1)
	// Coinage on bit mask 0x00f0
	PORT_BIT( 0x00f0, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Modified below

	PORT_START("DSWB")
	TOAPLAN_DIFFICULTY_LOC(SW2)
	// Per-game features on bit mask 0x00fc
	PORT_BIT( 0x00fc, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Modified below
INPUT_PORTS_END


static INPUT_PORTS_START( batsugun )
	PORT_INCLUDE( base )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0001,   0x0000, DEF_STR( Continue_Price ) ) PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(        0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(        0x0001, "Discount" )
	// Various features on bit mask 0x000e - see above
	TOAPLAN_COINAGE_JAPAN_LOC(SW1)  // European coinage shown in Service Mode but not actually used

	PORT_MODIFY("DSWB")
	// Difficulty on bit mask 0x0003 - see above
	PORT_DIPNAME( 0x000c,   0x0000, DEF_STR( Bonus_Life ) )     PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(        0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(        0x0008, "1500k only" )
	PORT_DIPSETTING(        0x0000, "1000k only" )
	PORT_DIPSETTING(        0x0004, "500k and every 600k" )
	PORT_DIPNAME( 0x0030,   0x0000, DEF_STR( Lives ) )          PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(        0x0030, "1" )
	PORT_DIPSETTING(        0x0020, "2" )
	PORT_DIPSETTING(        0x0000, "3" )
	PORT_DIPSETTING(        0x0010, "5" )
	PORT_DIPNAME( 0x0040,   0x0000, "Invulnerability (Cheat)" )         PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( Yes ) )

	PORT_START("JMPR")
	PORT_CONFNAME( 0x00f0,  0x0090, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!4,!3,!2,!1")
	PORT_CONFSETTING(       0x0090, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x0080, "Europe (Taito Corp.)" )
	PORT_CONFSETTING(       0x00b0, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x00a0, "USA (Taito Corp.)" )
	PORT_CONFSETTING(       0x00f0, DEF_STR( Japan ) )
//  PORT_CONFSETTING(        0x00e0, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x00d0, "Japan (Taito Corp.)" )
//  PORT_CONFSETTING(        0x00c0, "Japan (Taito Corp.)" )
	PORT_CONFSETTING(       0x0070, DEF_STR( Southeast_Asia ) )
	PORT_CONFSETTING(       0x0060, "Southeast Asia (Taito Corp.)" )
	PORT_CONFSETTING(       0x0050, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x0040, "Taiwan (Taito Corp.)" )
	PORT_CONFSETTING(       0x0030, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x0020, "Hong Kong (Taito Corp.)" )
	PORT_CONFSETTING(       0x0010, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x0000, "Korea (Unite Trading)" )
INPUT_PORTS_END


static INPUT_PORTS_START( batsugunbl )
	PORT_INCLUDE( batsugun )

	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x000f,  0x0009, DEF_STR( Region ) )  // PORT_CONFLOCATION("JP:!4,!3,!2,!1")
	PORT_CONFSETTING(       0x0009, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x0008, "Europe (Taito Corp.)" )
	PORT_CONFSETTING(       0x000b, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x000a, "USA (Taito Corp.)" )
	PORT_CONFSETTING(       0x000f, DEF_STR( Japan ) )
//  PORT_CONFSETTING(       0x000e, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x000d, "Japan (Taito Corp.)" )
//  PORT_CONFSETTING(       0x000c, "Japan (Taito Corp.)" )
	PORT_CONFSETTING(       0x0007, DEF_STR( Southeast_Asia ) )
	PORT_CONFSETTING(       0x0006, "Southeast Asia (Taito Corp.)" )
	PORT_CONFSETTING(       0x0005, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x0004, "Taiwan (Taito Corp.)" )
	PORT_CONFSETTING(       0x0003, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x0002, "Hong Kong (Taito Corp.)" )
	PORT_CONFSETTING(       0x0001, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x0000, "Korea (Unite Trading)" )
	PORT_CONFNAME( 0x00f0,  0x00f0, "(null)" )
INPUT_PORTS_END



void batsugun_state::batsugun_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x200010, 0x200011).portr("IN1");
	map(0x200014, 0x200015).portr("IN2");
	map(0x200018, 0x200019).portr("SYS");
	map(0x20001d, 0x20001d).w(FUNC(batsugun_state::coin_sound_reset_w));  // Coin count/lock + v25 reset line
	map(0x210000, 0x21ffff).rw(FUNC(batsugun_state::shared_ram_r), FUNC(batsugun_state::shared_ram_w)).umask16(0x00ff);
	map(0x300000, 0x30000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500000, 0x50000d).rw(m_vdp[1], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x700000, 0x700001).r(m_vdp[0], FUNC(gp9001vdp_device::vdpcount_r));
}

void batsugun_bootleg_state::batsugunbl_oki_bankswitch_w(u8 data)
{
	data &= 7;
	if (data <= 4) m_okibank->set_entry(data);
}


void batsugun_bootleg_state::batsugunbl_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	// map(0x200004, 0x200005).r() // only cleared at boot?
	map(0x200005, 0x200005).w(FUNC(batsugun_bootleg_state::batsugunbl_oki_bankswitch_w));  // TODO: doesn't sound correct
	map(0x200009, 0x200009).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x200010, 0x200011).portr("IN1");
	map(0x200014, 0x200015).portr("IN2");
	map(0x200018, 0x200019).portr("SYS");
	map(0x20001c, 0x20001d).nopw();  // leftover code from the original?
	map(0x21f004, 0x21f005).portr("DSWA");
	map(0x21f006, 0x21f007).portr("DSWB");
	map(0x21f008, 0x21f009).portr("JMPR");
	map(0x300000, 0x30000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500000, 0x50000d).rw(m_vdp[1], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x700000, 0x700001).r(m_vdp[0], FUNC(gp9001vdp_device::vdpcount_r));
}

void batsugun_bootleg_state::cpu_space_batsugunbl_map(address_map &map)
{
	map(0xfffff0, 0xffffff).m(m_maincpu, FUNC(m68000_base_device::autovectors_map));
	map(0xfffff5, 0xfffff5).lr8(NAME([this] () { m_maincpu->set_input_line(M68K_IRQ_2, CLEAR_LINE); return m68000_device::autovector(2); }));
}

void batsugun_state::v25_mem(address_map &map)
{
	map(0x00000, 0x00001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x00004, 0x00004).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x80000, 0x87fff).mirror(0x78000).ram().share(m_shared_ram);
}

void batsugun_state::batsugun(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 32_MHz_XTAL/2);  // 16MHz, 32MHz Oscillator
	m_maincpu->set_addrmap(AS_PROGRAM, &batsugun_state::batsugun_68k_mem);
	m_maincpu->reset_cb().set(FUNC(batsugun_state::reset_audiocpu));

	v25_device &audiocpu(V25(config, m_audiocpu, 32_MHz_XTAL/2));  // NEC V25 type Toaplan marked CPU ???
	audiocpu.set_addrmap(AS_PROGRAM, &batsugun_state::v25_mem);
	audiocpu.pt_in_cb().set_ioport("DSWA").exor(0xff);
	audiocpu.p0_in_cb().set_ioport("DSWB").exor(0xff);
	audiocpu.p1_in_cb().set_ioport("JMPR").exor(0xff);
	audiocpu.p2_out_cb().set_nop();  // bit 0 is FAULT according to kbash schematic

	TOAPLAN_COINCOUNTER(config, m_coincounter, 0);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(batsugun_state::screen_update_batsugun));
	m_screen->screen_vblank().set(FUNC(batsugun_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, gp9001vdp_device::VDP_PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette);
	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	GP9001_VDP(config, m_vdp[1], 27_MHz_XTAL);
	m_vdp[1]->set_palette(m_palette);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM2151(config, "ymsnd", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 0.5);

	OKIM6295(config, m_oki, 32_MHz_XTAL/8, okim6295_device::PIN7_LOW);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void batsugun_bootleg_state::batsugunbl(machine_config &config)
{
	batsugun(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &batsugun_bootleg_state::batsugunbl_68k_mem);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &batsugun_bootleg_state::cpu_space_batsugunbl_map);
	m_maincpu->reset_cb().set_nop();

	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_2, ASSERT_LINE);

	config.device_remove("audiocpu");
	config.device_remove("ymsnd");

	m_oki->set_addrmap(0, &batsugun_bootleg_state::fixeightbl_oki);
}


ROM_START( batsugun )
	ROM_REGION( 0x080000, "maincpu", 0 )  // Main 68K code
	ROM_LOAD16_WORD_SWAP( "tp030_1a.bin", 0x000000, 0x080000,  CRC(cb1d4554) SHA1(ef31f24d77e1c13bdf5558a04a6253e2e3e6a790) )

	// Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN)
	// It's a NEC V25 (PLCC94) (program uploaded by main CPU)

	ROM_REGION( 0x400000, "gp9001_0", 0 )
	ROM_LOAD( "tp030_3l.bin", 0x000000, 0x100000, CRC(3024b793) SHA1(e161db940f069279356fca2c5bf2753f07773705) )
	ROM_LOAD( "tp030_3h.bin", 0x100000, 0x100000, CRC(ed75730b) SHA1(341f0f728144a049486d996c9bb14078578c6879) )
	ROM_LOAD( "tp030_4l.bin", 0x200000, 0x100000, CRC(fedb9861) SHA1(4b0917056bd359b21935358c6bcc729262be6417) )
	ROM_LOAD( "tp030_4h.bin", 0x300000, 0x100000, CRC(d482948b) SHA1(31be7dc5cff072403b783bf203b9805ffcad7284) )

	ROM_REGION( 0x200000, "gp9001_1", 0 )
	ROM_LOAD( "tp030_5.bin",  0x000000, 0x100000, CRC(bcf5ba05) SHA1(40f98888a29cdd30cda5dfb60fdc667c69b0fdb0) )
	ROM_LOAD( "tp030_6.bin",  0x100000, 0x100000, CRC(0666fecd) SHA1(aa8f921fc51590b5b05bbe0b0ad0cce5ff359c64) )

	ROM_REGION( 0x40000, "oki", 0 )  // ADPCM Samples
	ROM_LOAD( "tp030_2.bin", 0x00000, 0x40000, CRC(276146f5) SHA1(bf11d1f6782cefcad77d52af4f7e6054a8f93440) )

	ROM_REGION( 0x1000, "plds", 0 )  // Logic for mixing output of both GP9001 GFX controllers
	ROM_LOAD( "tp030_u19_gal16v8b-15.bin", 0x0000, 0x117, CRC(f71669e8) SHA1(ec1fbe04605fee864af4b01f001af227938c9f21) )
//  ROM_LOAD( "tp030_u19_gal16v8b-15.jed", 0x0000, 0x991, CRC(31be54a2) SHA1(06278942a9a2ea858c0352b2ef5a65bf329b7b82) )
ROM_END

ROM_START( batsuguna )
	ROM_REGION( 0x080000, "maincpu", 0 )  // Main 68K code
	ROM_LOAD16_WORD_SWAP( "tp030_01.bin", 0x000000, 0x080000, CRC(3873d7dd) SHA1(baf6187d7d554cfcf4a86b63f07fc30df7ef84c9) )

	// Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN)
	// It's a NEC V25 (PLCC94) (program uploaded by main CPU)

	ROM_REGION( 0x400000, "gp9001_0", 0 )
	ROM_LOAD( "tp030_3l.bin", 0x000000, 0x100000, CRC(3024b793) SHA1(e161db940f069279356fca2c5bf2753f07773705) )
	ROM_LOAD( "tp030_3h.bin", 0x100000, 0x100000, CRC(ed75730b) SHA1(341f0f728144a049486d996c9bb14078578c6879) )
	ROM_LOAD( "tp030_4l.bin", 0x200000, 0x100000, CRC(fedb9861) SHA1(4b0917056bd359b21935358c6bcc729262be6417) )
	ROM_LOAD( "tp030_4h.bin", 0x300000, 0x100000, CRC(d482948b) SHA1(31be7dc5cff072403b783bf203b9805ffcad7284) )

	ROM_REGION( 0x200000, "gp9001_1", 0 )
	ROM_LOAD( "tp030_5.bin",  0x000000, 0x100000, CRC(bcf5ba05) SHA1(40f98888a29cdd30cda5dfb60fdc667c69b0fdb0) )
	ROM_LOAD( "tp030_6.bin",  0x100000, 0x100000, CRC(0666fecd) SHA1(aa8f921fc51590b5b05bbe0b0ad0cce5ff359c64) )

	ROM_REGION( 0x40000, "oki", 0 )         /* ADPCM Samples */
	ROM_LOAD( "tp030_2.bin", 0x00000, 0x40000, CRC(276146f5) SHA1(bf11d1f6782cefcad77d52af4f7e6054a8f93440) )

	ROM_REGION( 0x1000, "plds", 0 )         /* Logic for mixing output of both GP9001 GFX controllers */
	ROM_LOAD( "tp030_u19_gal16v8b-15.bin", 0x0000, 0x117, CRC(f71669e8) SHA1(ec1fbe04605fee864af4b01f001af227938c9f21) )
ROM_END

ROM_START( batsugunb )
	ROM_REGION( 0x080000, "maincpu", 0 )  // Main 68K code
	ROM_LOAD16_WORD_SWAP( "large_rom1.bin", 0x000000, 0x080000,  CRC(c9de8ed8) SHA1(8de9acd26e83c8ea3388137da528704116aa7bdb) )

	// Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN)
	// It's a NEC V25 (PLCC94) (program uploaded by main CPU)

	ROM_REGION( 0x400000, "gp9001_0", 0 )
	ROM_LOAD16_BYTE( "rom12.bin", 0x000000, 0x080000, CRC(d25affc6) SHA1(00803ae5a2bc06edbfb9ea6e3df51f195bbee8cb) )
	ROM_LOAD16_BYTE( "rom6.bin",  0x000001, 0x080000, CRC(ddd6df60) SHA1(3b46945c51e7b10b473d98916f075e8def336ce7) )
	ROM_LOAD16_BYTE( "rom11.bin", 0x100000, 0x080000, CRC(ed72fe3e) SHA1(5c0f4d5cc84b45e1924dacfa4c0b602cc1600b2f) )
	ROM_LOAD16_BYTE( "rom5.bin",  0x100001, 0x080000, CRC(fd44b33b) SHA1(791cf6056a2dbafa5f41f1dcf686947ee990647d) )
	ROM_LOAD16_BYTE( "rom10.bin", 0x200000, 0x080000, CRC(86b2c6a9) SHA1(b3f39246012c6cd9df69a6797d56479523b33bcb) )
	ROM_LOAD16_BYTE( "rom4.bin",  0x200001, 0x080000, CRC(e7c1c623) SHA1(0d8922ce901b5f74f1bd397d5d9c6ab4e918b1d1) )
	ROM_LOAD16_BYTE( "rom9.bin",  0x300000, 0x080000, CRC(fda8ee00) SHA1(d5ea617a72b2721386eb2dfc15b76de2e30f069c) )
	ROM_LOAD16_BYTE( "rom3.bin",  0x300001, 0x080000, CRC(a7c4dee8) SHA1(94e2dda067612fac810157f8cf392b685b38798b) )

	ROM_REGION( 0x200000, "gp9001_1", 0 )
	ROM_LOAD16_BYTE( "rom8.bin",  0x000000, 0x080000, CRC(a2c6a170) SHA1(154048ddc8ca2b4e9617e142d904ad2698b0ad02) )
	ROM_LOAD16_BYTE( "rom2.bin",  0x000001, 0x080000, CRC(a457e202) SHA1(4a9f2f95c866fc9d40af1c57ce1940f0a6dc1b82) )
	ROM_LOAD16_BYTE( "rom7.bin",  0x100000, 0x080000, CRC(8644518f) SHA1(570141deeb796cfae57600d5a518d34bb6dc14d0) )
	ROM_LOAD16_BYTE( "rom1.bin",  0x100001, 0x080000, CRC(8e339897) SHA1(80e84c291f287c0783bddfcb1b7ebf78c154cadc) )

	ROM_REGION( 0x40000, "oki", 0 )  // ADPCM Samples
	ROM_LOAD( "rom13.bin", 0x00000, 0x40000, CRC(276146f5) SHA1(bf11d1f6782cefcad77d52af4f7e6054a8f93440) )

	ROM_REGION( 0x1000, "plds", 0 )  // Logic for mixing output of both GP9001 GFX controllers
	ROM_LOAD( "tp030_u19_gal16v8b-15.bin", 0x0000, 0x117, CRC(f71669e8) SHA1(ec1fbe04605fee864af4b01f001af227938c9f21) )
//  ROM_LOAD( "tp030_u19_gal16v8b-15.jed", 0x0000, 0x991, CRC(31be54a2) SHA1(06278942a9a2ea858c0352b2ef5a65bf329b7b82) )
ROM_END


// very similar to batsuguna, same main CPU label, seems to have just a tiny bit more code
ROM_START( batsugunc )
	ROM_REGION( 0x080000, "maincpu", 0 )  // Main 68K code
	ROM_LOAD16_WORD_SWAP( "tp-030_01.u69", 0x000000, 0x080000, CRC(545305c4) SHA1(9411ad7fe0be89a9f04b9116c9c709dc5e98c345) )

	// Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN)
	// It's a NEC V25 (PLCC94) (program uploaded by main CPU)

	ROM_REGION( 0x400000, "gp9001_0", 0 )
	ROM_LOAD( "tp030_rom3-l.u55", 0x000000, 0x100000, CRC(3024b793) SHA1(e161db940f069279356fca2c5bf2753f07773705) )
	ROM_LOAD( "tp030_rom3-h.u56", 0x100000, 0x100000, CRC(ed75730b) SHA1(341f0f728144a049486d996c9bb14078578c6879) )
	ROM_LOAD( "tp030_rom4-l.u54", 0x200000, 0x100000, CRC(fedb9861) SHA1(4b0917056bd359b21935358c6bcc729262be6417) )
	ROM_LOAD( "tp030_rom4-h.u57", 0x300000, 0x100000, CRC(d482948b) SHA1(31be7dc5cff072403b783bf203b9805ffcad7284) )

	ROM_REGION( 0x200000, "gp9001_1", 0 )
	ROM_LOAD( "tp030_rom5.u32",  0x000000, 0x100000, CRC(bcf5ba05) SHA1(40f98888a29cdd30cda5dfb60fdc667c69b0fdb0) )
	ROM_LOAD( "tp030_rom6.u31",  0x100000, 0x100000, CRC(0666fecd) SHA1(aa8f921fc51590b5b05bbe0b0ad0cce5ff359c64) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "tp030_rom2.u65", 0x00000, 0x40000, CRC(276146f5) SHA1(bf11d1f6782cefcad77d52af4f7e6054a8f93440) )

	ROM_REGION( 0x1000, "plds", 0 )  // Logic for mixing output of both GP9001 GFX controllers
	ROM_LOAD( "tp030_u19_gal16v8b-15.bin", 0x0000, 0x117, CRC(f71669e8) SHA1(ec1fbe04605fee864af4b01f001af227938c9f21) )
//  ROM_LOAD( "tp030_u19_gal16v8b-15.jed", 0x0000, 0x991, CRC(31be54a2) SHA1(06278942a9a2ea858c0352b2ef5a65bf329b7b82) )
ROM_END

ROM_START( batsugunsp )
	ROM_REGION( 0x080000, "maincpu", 0 )  // Main 68K code
	ROM_LOAD16_WORD_SWAP( "tp-030sp.u69", 0x000000, 0x080000, CRC(8072a0cd) SHA1(3a0a9cdf894926a16800c4882a2b00383d981367) )

	// Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN)
	// It's a NEC V25 (PLCC94) (program uploaded by main CPU)

	ROM_REGION( 0x400000, "gp9001_0", 0 )
	ROM_LOAD( "tp030_3l.bin", 0x000000, 0x100000, CRC(3024b793) SHA1(e161db940f069279356fca2c5bf2753f07773705) )
	ROM_LOAD( "tp030_3h.bin", 0x100000, 0x100000, CRC(ed75730b) SHA1(341f0f728144a049486d996c9bb14078578c6879) )
	ROM_LOAD( "tp030_4l.bin", 0x200000, 0x100000, CRC(fedb9861) SHA1(4b0917056bd359b21935358c6bcc729262be6417) )
	ROM_LOAD( "tp030_4h.bin", 0x300000, 0x100000, CRC(d482948b) SHA1(31be7dc5cff072403b783bf203b9805ffcad7284) )

	ROM_REGION( 0x200000, "gp9001_1", 0 )
	ROM_LOAD( "tp030_5.bin",  0x000000, 0x100000, CRC(bcf5ba05) SHA1(40f98888a29cdd30cda5dfb60fdc667c69b0fdb0) )
	ROM_LOAD( "tp030_6.bin",  0x100000, 0x100000, CRC(0666fecd) SHA1(aa8f921fc51590b5b05bbe0b0ad0cce5ff359c64) )

	ROM_REGION( 0x40000, "oki", 0 )  // ADPCM Samples
	ROM_LOAD( "tp030_2.bin", 0x00000, 0x40000, CRC(276146f5) SHA1(bf11d1f6782cefcad77d52af4f7e6054a8f93440) )

	ROM_REGION( 0x1000, "plds", 0 )  // Logic for mixing output of both GP9001 GFX controllers
	ROM_LOAD( "tp030_u19_gal16v8b-15.bin", 0x0000, 0x117, CRC(f71669e8) SHA1(ec1fbe04605fee864af4b01f001af227938c9f21) )
ROM_END


// a cost-cutting bootleg PCB with only M68000 + OKIM6295. A pair of TPC1020 seem to do the job of the GP9001s.
// according to the dumper 'audio is pretty garbage, and some sprites overlay the UI incorrectly'
ROM_START( batsugunbl )
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "27c240.bin", 0x000000, 0x080000, CRC(a34df8bb) SHA1(10d456f5437b21a95fd8018bdb19a08a110241c4) )

	ROM_REGION( 0x400000, "gp9001_0", 0 )  // same as original
	ROM_LOAD( "27c8100-j.bin", 0x000000, 0x100000, CRC(3024b793) SHA1(e161db940f069279356fca2c5bf2753f07773705) )
	ROM_LOAD( "27c8100-k.bin", 0x100000, 0x100000, CRC(ed75730b) SHA1(341f0f728144a049486d996c9bb14078578c6879) )
	ROM_LOAD( "27c8100-l.bin", 0x200000, 0x100000, CRC(fedb9861) SHA1(4b0917056bd359b21935358c6bcc729262be6417) )
	ROM_LOAD( "27c8100-m.bin", 0x300000, 0x100000, CRC(d482948b) SHA1(31be7dc5cff072403b783bf203b9805ffcad7284) )

	ROM_REGION( 0x200000, "gp9001_1", 0 )  // same as original
	ROM_LOAD( "27c8100-n.bin",  0x000000, 0x100000, CRC(bcf5ba05) SHA1(40f98888a29cdd30cda5dfb60fdc667c69b0fdb0) )
	ROM_LOAD( "27c8100-o.bin",  0x100000, 0x100000, CRC(0666fecd) SHA1(aa8f921fc51590b5b05bbe0b0ad0cce5ff359c64) )

	ROM_REGION( 0x80000, "oki", 0 )  // more samples to compensate for missing YM2151
	ROM_LOAD( "27c040.bin", 0x00000, 0x80000, CRC(1f8ec1b6) SHA1(28107a90d29613ceddc001df2556543b33c1294c) )
ROM_END

/**********************************************************

  Dogyuun bootleg.

  Bootleg board with M68000 & OKI M6295
  (replacing the original YM2151 fm sounds with samples)

  2x Actel A1020A (seems to replace the GP9001s).
  The PCB design looks like two side mirrored.

  TODO:
  - Fix gfx priorities/issues
  - Check oki banking
  - Fix DIP switches bank offset

**********************************************************/
ROM_START( dogyuunbl )
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1.bin", 0x000000, 0x040000, CRC(7407d908) SHA1(6fa78c833989322b9424be852b77c9c7df5bf3eb) )
	ROM_LOAD16_BYTE( "2.bin", 0x000001, 0x040000, CRC(013e9801) SHA1(d6f5a53e1b8f304d32fafe44f3a540f3c410e42d) )

	ROM_REGION( 0x10000, "unknown", 0 )
	ROM_LOAD( "3.bin",  0x0000, 0x8000, CRC(456dd16e) SHA1(84779ee64d3ea33ba1ba4dee39b504a81c6811a1) )
	ROM_LOAD( "4.bin",  0x8000, 0x8000, CRC(456dd16e) SHA1(84779ee64d3ea33ba1ba4dee39b504a81c6811a1) )

	ROM_REGION( 0x200000, "gp9001_0", 0 )
	ROM_LOAD( "201.bin", 0x000000, 0x100000, CRC(8bb2ff15) SHA1(4693454b4b4c6984025c34e53f028cfc1ff77f3c) )
	ROM_LOAD( "202.bin", 0x100000, 0x100000, CRC(c38e7673) SHA1(8bda42704dc2fa269416e49dbac4800bf414de0a) )

	ROM_REGION( 0x400000, "gp9001_1", 0 )  // roms 203 & 204 are damaged and all dumps have a bunch of different bytes
	ROM_LOAD( "203_bad.bin", 0x000000, 0x200000, BAD_DUMP CRC(d35af12e) SHA1(71f3d4b08d7192063b6720c95c2ab0fb28f6df78) )
	ROM_LOAD( "204_bad.bin", 0x200000, 0x200000, BAD_DUMP CRC(fd35e7e1) SHA1(a77f3a5327cc9fd17449aecb09319fc0fc0da31e) )

	ROM_REGION( 0x1000000, "altdumps", 0 )  // some alt dumps of roms 203 & 204. Included here in case someone can figure out the correct ones.
	ROM_LOAD( "203_1_bad.bin", 0x000000, 0x200000, BAD_DUMP CRC(85f0dafd) SHA1(5b6202ed3d1e35396d0dcb925e86b71a86f7943b) )
	ROM_LOAD( "203_2_bad.bin", 0x200000, 0x200000, BAD_DUMP CRC(6d9bb107) SHA1(5c600119a13b48919bd6a4dacc512b2f0c524020) )
	ROM_LOAD( "203_3_bad.bin", 0x400000, 0x200000, BAD_DUMP CRC(e6a3aeb7) SHA1(9df29f344e802193d62c201f2305b05fa756e005) )
	ROM_LOAD( "203_4_bad.bin", 0x600000, 0x200000, BAD_DUMP CRC(2d7595da) SHA1(e882cbb4c26f08970103297148f6dab41a591747) )
	ROM_LOAD( "204_1_bad.bin", 0x800000, 0x200000, BAD_DUMP CRC(08ab308b) SHA1(b99024475fc7b621becfa2bea8fbdfd7c0cc96ef) )
	ROM_LOAD( "204_2_bad.bin", 0xa00000, 0x200000, BAD_DUMP CRC(5fc9bafd) SHA1(490e39e24d57bacdc13ceecd5db25ac03c5289c0) )
	ROM_LOAD( "204_3_bad.bin", 0xc00000, 0x200000, BAD_DUMP CRC(61235e6c) SHA1(56c25c7586692a2e684e54185e1671e0ce6358ec) )
	ROM_LOAD( "204_4_bad.bin", 0xe00000, 0x200000, BAD_DUMP CRC(ebacdc6e) SHA1(e687164964b1cacdb433c65b8435fc06b9a6ffe3) )

	ROM_REGION( 0x80000, "oki", 0 )  // ADPCM Samples... Huge. Need to check the banking
	ROM_LOAD( "sound.bin", 0x00000, 0x80000, CRC(c963aa9c) SHA1(f02634771ee1343b85b1ebbb5c8819a42c97dc23) )
ROM_END


void batsugun_bootleg_state::init_batsugunbl()
{
	u8 *ROM = memregion("oki")->base();

	m_okibank->configure_entries(0, 5, &ROM[0x30000], 0x10000);
}

} // anonymous namespace


/*********************************************
*                Game Drivers                *
*********************************************/

//    YEAR  NAME         PARENT    MACHINE     INPUT       CLASS           INIT           ROT     COMPANY    FULLNAME & FLAGS
GAME( 1993, batsugun,    0,        batsugun,   batsugun,   batsugun_state, empty_init,    ROT270, "Toaplan", "Batsugun", MACHINE_SUPPORTS_SAVE )
GAME( 1993, batsuguna,   batsugun, batsugun,   batsugun,   batsugun_state, empty_init,    ROT270, "Toaplan", "Batsugun (older, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, batsugunc,   batsugun, batsugun,   batsugun,   batsugun_state, empty_init,    ROT270, "Toaplan", "Batsugun (older, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, batsugunb,   batsugun, batsugun,   batsugun,   batsugun_state, empty_init,    ROT270, "Toaplan", "Batsugun (Korean PCB)", MACHINE_SUPPORTS_SAVE ) // cheap looking PCB (same 'TP-030' numbering as original) but without Mask ROMs.  Still has original customs etc.  Jumpers were set to the Korea Unite Trading license, so likely made in Korea, not a bootleg tho.
GAME( 1993, batsugunsp,  batsugun, batsugun,   batsugun,   batsugun_state, empty_init,    ROT270, "Toaplan", "Batsugun - Special Version", MACHINE_SUPPORTS_SAVE )
GAME( 1993, batsugunbl,  batsugun, batsugunbl, batsugunbl, batsugun_bootleg_state, init_batsugunbl, ROT270, "Toaplan", "Batsugun (bootleg)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // needs correct GFX offsets and oki banking fix
GAME( 1993, dogyuunbl,   dogyuun,  batsugunbl, batsugunbl, batsugun_bootleg_state, init_batsugunbl, ROT270, "Toaplan", "Dogyuun (bootleg)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )  // needs fix gfx priorities/issues and check oki banking
