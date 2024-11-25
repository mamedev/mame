// license:BSD-3-Clause
// copyright-holders:Yochizo
// thanks-to:Richard Bush
/***************************************************************************

Exzisus
-------------------------------------
driver by Yochizo

This driver is heavily dependent on the Raine source.
Very thanks to Richard Bush and the Raine team.


Supported games :
==================
   Exzisus (2 sets)    (C) 1987 Taito


System specs :
===============
   CPU       : Z80 x 4
   Sound     : YM2151
   Chips     : TC0010VCU x 2 + PC060HA

   There are two types of Exzisus PCB:

   * The first (K1100256A) has separate RGB outputs for the background and sprites.
   Exactly how they are combined to form the final image is unknown.
   * The second, later PCB has a single video output and is JAMMA compliant.

TODO:
- There must be a way for CPU a to stop CPU c, otherwise the RAM check in test
  mode cannot work. However, the only way I found to do that is making writes
  to F404 pulse the reset line, which isn't a common way to handle these things.

****************************************************************************/

#include "emu.h"

#include "taitosnd.h"
#include "taitoipt.h"

#include "cpu/z80/z80.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class exzisus_state : public driver_device
{
public:
	exzisus_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_cpuc(*this, "cpuc"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_objectram(*this, "objectram%u", 0U),
		m_videoram(*this, "videoram%u", 0U),
		m_sharedram_ac(*this, "sharedram_ac"),
		m_sharedram_ab(*this, "sharedram_ab"),
		m_cpubank(*this, "cpubank%u", 0U)
	{ }

	void exzisus(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_cpuc;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr_array<uint8_t, 2> m_objectram;
	required_shared_ptr_array<uint8_t, 2> m_videoram;
	required_shared_ptr<uint8_t> m_sharedram_ac;
	required_shared_ptr<uint8_t> m_sharedram_ab;
	required_memory_bank_array<2> m_cpubank;

	template <uint8_t Which> void cpu_bankswitch_w(uint8_t data);
	void coincounter_w(uint8_t data);
	void cpuc_reset_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cpua_map(address_map &map) ATTR_COLD;
	void cpub_map(address_map &map) ATTR_COLD;
	void cpuc_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

 Video hardware of this hardware is almost similar with "mexico86". So,
 most routines are derived from mexico86 driver.

***************************************************************************/


/***************************************************************************
  Screen refresh
***************************************************************************/

uint32_t exzisus_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// Is this correct ?
	bitmap.fill(1023, cliprect);

	// 2 TC0010VCU. TODO: devicify if more drivers use this chip
	for (int j = 0; j < 2; j++)
	{
		int sx = 0;

		for (int offs = 0 ; offs < m_objectram[j].bytes() ; offs += 4)
		{
			int height;

			// Skip empty sprites.
			if ( !(*(uint32_t *)(&m_objectram[j][offs])) )
			{
				continue;
			}

			int gfx_num = m_objectram[j][offs + 1];
			int gfx_attr = m_objectram[j][offs + 3];

			int gfx_offs;
			if ((gfx_num & 0x80) == 0)  // 16x16 sprites
			{
				gfx_offs = ((gfx_num & 0x7f) << 3);
				height = 2;

				sx = m_objectram[j][offs + 2];
				sx |= (gfx_attr & 0x40) << 2;
			}
			else    // tilemaps (each sprite is a 16x256 column)
			{
				gfx_offs = ((gfx_num & 0x3f) << 7) + 0x0400;
				height = 32;

				if (gfx_num & 0x40)         // Next column
				{
					sx += 16;
				}
				else
				{
					sx = m_objectram[j][offs + 2];
					sx |= (gfx_attr & 0x40) << 2;
				}
			}

			int sy = 256 - (height << 3) - (m_objectram[j][offs]);

			for (int xc = 0 ; xc < 2 ; xc++)
			{
				int goffs = gfx_offs;
				for (int yc = 0 ; yc < height ; yc++)
				{
					int code  = (m_videoram[j][goffs + 1] << 8) | m_videoram[j][goffs];
					int color = (m_videoram[j][goffs + 1] >> 6) | (gfx_attr & 0x0f);
					int x = (sx + (xc << 3)) & 0xff;
					int y = (sy + (yc << 3)) & 0xff;

					if (flip_screen())
					{
						x = 248 - x;
						y = 248 - y;
					}

					m_gfxdecode->gfx(j)->transpen(bitmap, cliprect,
							code & 0x3fff,
							color,
							flip_screen(), flip_screen(),
							x, y, 15);
					goffs += 2;
				}
				gfx_offs += height << 1;
			}
		}
	}
	return 0;
}


/***************************************************************************

  Memory Handler(s)

***************************************************************************/

template <uint8_t Which>
void exzisus_state::cpu_bankswitch_w(uint8_t data)
{
	m_cpubank[Which]->set_entry(data & 0x0f);
	flip_screen_set(data & 0x40);
}

void exzisus_state::coincounter_w(uint8_t data)
{
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x01);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x02);
	machine().bookkeeping().coin_counter_w(0, data & 0x04);
	machine().bookkeeping().coin_counter_w(1, data & 0x08);
}

void exzisus_state::cpuc_reset_w(uint8_t data)
{
	m_cpuc->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

#if 0
// without cpuc_reset_w, the following patch would be needed for the RAM check to work
void exzisus_state::init_exzisus()
{
	uint8_t *RAM = memregion("cpua")->base();

	// Fix WORK RAM error
	RAM[0x67fd] = 0x18;

	// Fix ROM 1 error
	RAM[0x6829] = 0x18;
}
#endif


/**************************************************************************

  Memory Map(s)

**************************************************************************/

void exzisus_state::cpua_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_cpubank[0]);
	map(0xc000, 0xc5ff).ram().share(m_objectram[1]);
	map(0xc600, 0xdfff).ram().share(m_videoram[1]);
	map(0xe000, 0xefff).ram().share(m_sharedram_ac);
	map(0xf400, 0xf400).w(FUNC(exzisus_state::cpu_bankswitch_w<0>));
	map(0xf404, 0xf404).w(FUNC(exzisus_state::cpuc_reset_w)); // ??
	map(0xf800, 0xffff).ram().share(m_sharedram_ab);
}

void exzisus_state::cpub_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_cpubank[1]);
	map(0xc000, 0xc5ff).ram().share(m_objectram[0]);
	map(0xc600, 0xdfff).ram().share(m_videoram[0]);
	map(0xe000, 0xefff).ram();
	map(0xf000, 0xf000).nopr().w("ciu", FUNC(pc060ha_device::master_port_w));
	map(0xf001, 0xf001).rw("ciu", FUNC(pc060ha_device::master_comm_r), FUNC(pc060ha_device::master_comm_w));
	map(0xf400, 0xf400).portr("P1");
	map(0xf400, 0xf400).w(FUNC(exzisus_state::cpu_bankswitch_w<1>));
	map(0xf401, 0xf401).portr("P2");
	map(0xf402, 0xf402).portr("SYSTEM");
	map(0xf402, 0xf402).w(FUNC(exzisus_state::coincounter_w));
	map(0xf404, 0xf404).portr("DSWA");
	map(0xf404, 0xf404).nopw(); // ??
	map(0xf405, 0xf405).portr("DSWB");
	map(0xf800, 0xffff).ram().share(m_sharedram_ab);
}

void exzisus_state::cpuc_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x85ff).ram().share(m_objectram[1]);
	map(0x8600, 0x9fff).ram().share(m_videoram[1]);
	map(0xa000, 0xafff).ram().share(m_sharedram_ac);
	map(0xb000, 0xbfff).ram();
}

void exzisus_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8fff).ram();
	map(0x9000, 0x9001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xa000, 0xa000).nopr().w("ciu", FUNC(pc060ha_device::slave_port_w));
	map(0xa001, 0xa001).rw("ciu", FUNC(pc060ha_device::slave_comm_r), FUNC(pc060ha_device::slave_comm_w));
}


/***************************************************************************

  Input Port(s)

***************************************************************************/

static INPUT_PORTS_START( exzisus )
	PORT_START("P1")
	TAITO_JOY_UDRL_2_BUTTONS( 1 )

	PORT_START("P2")
	TAITO_JOY_UDRL_2_BUTTONS( 2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL
	TAITO_COINAGE_JAPAN_OLD

	PORT_START("DSWB")
	TAITO_DIFFICULTY
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "100k and every 150k" )
	PORT_DIPSETTING(    0x0c, "150k and every 200k" )
	PORT_DIPSETTING(    0x04, "150k" )
	PORT_DIPSETTING(    0x00, "200k" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Service Mode (buggy)" )      // buggy: all other switches in DSW2 must be on
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/***************************************************************************

  Machine Driver(s)

***************************************************************************/

void exzisus_state::machine_start()
{
	m_cpubank[0]->configure_entries(0, 16, memregion("cpua")->base(), 0x4000);
	m_cpubank[1]->configure_entries(0, 16, memregion("cpub")->base(), 0x4000);
}

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2), RGN_FRAC(1,2)+4, 0, 4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static GFXDECODE_START( gfx_exzisus )
	GFXDECODE_ENTRY( "bg0", 0, charlayout,   0, 256 )
	GFXDECODE_ENTRY( "bg1", 0, charlayout, 256, 256 )
GFXDECODE_END



// All clocks are unconfirmed
void exzisus_state::exzisus(machine_config &config)
{
	// basic machine hardware
	z80_device &cpua(Z80(config, "cpua", 6000000));
	cpua.set_addrmap(AS_PROGRAM, &exzisus_state::cpua_map);
	cpua.set_vblank_int("screen", FUNC(exzisus_state::irq0_line_hold));

	z80_device &cpub(Z80(config, "cpub", 6000000));
	cpub.set_addrmap(AS_PROGRAM, &exzisus_state::cpub_map);
	cpub.set_vblank_int("screen", FUNC(exzisus_state::irq0_line_hold));

	Z80(config, m_cpuc, 6000000);
	m_cpuc->set_addrmap(AS_PROGRAM, &exzisus_state::cpuc_map);
	m_cpuc->set_vblank_int("screen", FUNC(exzisus_state::irq0_line_hold));

	z80_device &audiocpu(Z80(config, "audiocpu", 4000000));
	audiocpu.set_addrmap(AS_PROGRAM, &exzisus_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));   // 10 CPU slices per frame - enough for the sound CPU to read all commands

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(exzisus_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_exzisus);
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 1024);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 4000000));
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(0, "mono", 0.50);
	ymsnd.add_route(1, "mono", 0.50);

	pc060ha_device &ciu(PC060HA(config, "ciu", 0));
	ciu.nmi_callback().set_inputline("audiocpu", INPUT_LINE_NMI);
	ciu.reset_callback().set_inputline("audiocpu", INPUT_LINE_RESET);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( exzisus )
	ROM_REGION( 0x40000, "cpua", 0 )
	ROM_LOAD( "b12-09.7d",  0x00000, 0x10000, CRC(e80f49a9) SHA1(3995d52195cdadfa82ff992ec0456fce09e75132) )
	ROM_LOAD( "b12-11.9d",  0x10000, 0x10000, CRC(11fcda2c) SHA1(4f8d1dff339d96ffadde2cc7eec23cfeb42481f2) )

	ROM_REGION( 0x40000, "cpub", 0 )
	ROM_LOAD( "b12-10.7f",  0x00000, 0x10000, CRC(a60227f1) SHA1(1e0d09f6b77794095092316fe8bf823d4c7775bb) )
	ROM_LOAD( "b12-12.8f",  0x10000, 0x10000, CRC(a662be67) SHA1(0643480d56d8ac020288db800a705dd5d0d3ad9f) )
	ROM_LOAD( "b12-13.10f", 0x20000, 0x10000, CRC(04a29633) SHA1(39476365241718f01f9630c12467cb24791a67e1) )

	ROM_REGION( 0x10000, "cpuc", 0 )
	ROM_LOAD( "b12-14.12c", 0x00000, 0x08000, CRC(b5ce5e75) SHA1(6d5ec788684e1be4c727ac02b9fa313a42985b40) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b12-21.19f", 0x00000, 0x08000, CRC(b7e0f00e) SHA1(f79ef0dee6bd29c09b8e5c586514200e3fbaa87e) )

	ROM_REGION( 0x80000, "bg0", ROMREGION_INVERT )
	ROM_LOAD( "b12-16.17d", 0x00000, 0x10000, CRC(6fec6acb) SHA1(2289c116d3f6093988a088d011f192dd4a99aa77) )
	ROM_LOAD( "b12-18.19d", 0x10000, 0x10000, CRC(64e358aa) SHA1(cd1a23458b1a2f9c8c8aea8086dc04e0f6cc6908) )
	ROM_LOAD( "b12-20.20d", 0x20000, 0x10000, CRC(87f52e89) SHA1(3f8530aca087fa2a32dc6dfbcfe2f86604ee3ca1) )
	ROM_LOAD( "b12-15.17c", 0x40000, 0x10000, CRC(d81107c8) SHA1(c024c9b7956de493687e1373318d4cd74b3555b2) )
	ROM_LOAD( "b12-17.19c", 0x50000, 0x10000, CRC(db1d5a6c) SHA1(c2e1b8d92c2b3b2ce775ed50ca4a37e84ed35a93) )
	ROM_LOAD( "b12-19.20c", 0x60000, 0x10000, CRC(772b2641) SHA1(35cc6d5a725f1817791e710afde992e64d14104f) )

	ROM_REGION( 0x80000, "bg1", ROMREGION_INVERT )
	ROM_LOAD( "b12-05.1c",  0x00000, 0x10000, CRC(be5c5cc1) SHA1(af50c1ee0ce134871ea636c0e939f1a007a1cc13) )
	ROM_LOAD( "b12-07.3c",  0x10000, 0x10000, CRC(9353e39f) SHA1(576620818eb7a50a86aac0376a58ac22a29bb16d) )
	ROM_LOAD( "b12-06.1d",  0x40000, 0x10000, CRC(8571e6ed) SHA1(0a3228408f4d2afe3744172d24ae41d6400c30b6) )
	ROM_LOAD( "b12-08.3d",  0x50000, 0x10000, CRC(55ea5cca) SHA1(5717652ca028ff7a55c40573d52755985ed77ef7) )

	ROM_REGION( 0x00c00, "proms", 0 )
	ROM_LOAD( "b12-27.13l", 0x00000, 0x00100, CRC(524c9a01) SHA1(1894de29ba15a26043706ca4c5ca33aa8373447a) )
	ROM_LOAD( "b12-24.6m",  0x00100, 0x00100, CRC(1aa5bde9) SHA1(1bb6d5614183ff98600c5555ec8f5c545648e55c) )
	ROM_LOAD( "b12-26.12l", 0x00400, 0x00100, CRC(65f42c61) SHA1(7dc493d918f16661e3524c4189e785edfd345dbb) )
	ROM_LOAD( "b12-23.4m",  0x00500, 0x00100, CRC(fad4db5f) SHA1(bb1169ed6147fb9ac413f0d63e428dd190e7641d) )
	ROM_LOAD( "b12-25.11l", 0x00800, 0x00100, CRC(3e30f45b) SHA1(aa1f5278975c101f03feb16cac0bd074a6a41d2c) )
	ROM_LOAD( "b12-22.2m",  0x00900, 0x00100, CRC(936855d2) SHA1(57bb7cb40462f37e49c4ff0d9c833bbd2fb78428) )
ROM_END

ROM_START( exzisusa )
	ROM_REGION( 0x40000, "cpua", 0 )
	ROM_LOAD( "b23-10.7d",  0x00000, 0x10000, CRC(c80216fc) SHA1(7b952779c420be08573768f09bd65d0a188df024) )
	ROM_LOAD( "b23-12.9d",  0x10000, 0x10000, CRC(13637f54) SHA1(c175bc60120e32eec6ccca822fa497a42dd59823) )

	ROM_REGION( 0x40000, "cpub", 0 )
	ROM_LOAD( "b23-11.7f",  0x00000, 0x10000, CRC(d6a79cef) SHA1(e2b56aa38c017b24b50f304b9fe49ee14006f9a4) )
	ROM_LOAD( "b12-12.8f",  0x10000, 0x10000, CRC(a662be67) SHA1(0643480d56d8ac020288db800a705dd5d0d3ad9f) )
	ROM_LOAD( "b12-13.10f", 0x20000, 0x10000, CRC(04a29633) SHA1(39476365241718f01f9630c12467cb24791a67e1) )

	ROM_REGION( 0x10000, "cpuc", 0 )
	ROM_LOAD( "b23-13.12c", 0x00000, 0x08000, CRC(51110aa1) SHA1(34c2701625eb1987affad1efd19ff8c9971456ae) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b23-14.19f", 0x00000, 0x08000, CRC(f7ca7df2) SHA1(6048d9341f0303546e447a76439e1927d14cdd57) )

	ROM_REGION( 0x80000, "bg0", ROMREGION_INVERT )
	ROM_LOAD( "b12-16.17d", 0x00000, 0x10000, CRC(6fec6acb) SHA1(2289c116d3f6093988a088d011f192dd4a99aa77) )
	ROM_LOAD( "b12-18.19d", 0x10000, 0x10000, CRC(64e358aa) SHA1(cd1a23458b1a2f9c8c8aea8086dc04e0f6cc6908) )
	ROM_LOAD( "b12-20.20d", 0x20000, 0x10000, CRC(87f52e89) SHA1(3f8530aca087fa2a32dc6dfbcfe2f86604ee3ca1) )
	ROM_LOAD( "b12-15.17c", 0x40000, 0x10000, CRC(d81107c8) SHA1(c024c9b7956de493687e1373318d4cd74b3555b2) )
	ROM_LOAD( "b12-17.19c", 0x50000, 0x10000, CRC(db1d5a6c) SHA1(c2e1b8d92c2b3b2ce775ed50ca4a37e84ed35a93) )
	ROM_LOAD( "b12-19.20c", 0x60000, 0x10000, CRC(772b2641) SHA1(35cc6d5a725f1817791e710afde992e64d14104f) )

	ROM_REGION( 0x80000, "bg1", ROMREGION_INVERT )
	ROM_LOAD( "b23-06.1c",  0x00000, 0x10000, CRC(44f8f661) SHA1(d77160a89e45556cd9ce211d89c398e1086d8d92) )
	ROM_LOAD( "b23-08.3c",  0x10000, 0x10000, CRC(1ce498c1) SHA1(a9ce3de997089bd40c99bd89919b459c9f215fc8) )
	ROM_LOAD( "b23-07.1d",  0x40000, 0x10000, CRC(d7f6ec89) SHA1(e8da207ddaf46ceff870b45ecec0e89c499291b4) )
	ROM_LOAD( "b23-09.3d",  0x50000, 0x10000, CRC(6651617f) SHA1(6351a0b01589cb181b896285ade70e9dfcd799ec) )

	ROM_REGION( 0x00c00, "proms", 0 )
	// These appear to be twice the correct size
	ROM_LOAD( "b23-04.15l", 0x00000, 0x00400, CRC(5042cffa) SHA1(c969748866a12681cf2dbf25a46da2c4e4f92313) )
	ROM_LOAD( "b23-03.14l", 0x00400, 0x00400, BAD_DUMP CRC(9458fd45) SHA1(7f7cdacf37bb6f15de1109fa73ba3c5fc88893d0) ) // D0 is fixed
	ROM_LOAD( "b23-05.16l", 0x00800, 0x00400, CRC(87f0f69a) SHA1(37df6fd56245fab9beaabfd86fd8f95d7c42c2a5) )
ROM_END

ROM_START( exzisust )
	ROM_REGION( 0x40000, "cpua", 0 )
	ROM_LOAD( "b23-10.7d",  0x00000, 0x10000, CRC(c80216fc) SHA1(7b952779c420be08573768f09bd65d0a188df024) )
	ROM_LOAD( "b23-12.9d",  0x10000, 0x10000, CRC(13637f54) SHA1(c175bc60120e32eec6ccca822fa497a42dd59823) )

	ROM_REGION( 0x40000, "cpub", 0 )
	ROM_LOAD( "b23-15.7f",  0x00000, 0x10000, CRC(2f8b3752) SHA1(acfbb8aa20e6b031b9543e1e56268f3f5c7f7f07) )
	ROM_LOAD( "b12-12.8f",  0x10000, 0x10000, CRC(a662be67) SHA1(0643480d56d8ac020288db800a705dd5d0d3ad9f) )
	ROM_LOAD( "b12-13.10f", 0x20000, 0x10000, CRC(04a29633) SHA1(39476365241718f01f9630c12467cb24791a67e1) )

	ROM_REGION( 0x10000, "cpuc", 0 )
	ROM_LOAD( "b23-13.12c", 0x00000, 0x08000, CRC(51110aa1) SHA1(34c2701625eb1987affad1efd19ff8c9971456ae) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b23-14.19f", 0x00000, 0x08000, CRC(f7ca7df2) SHA1(6048d9341f0303546e447a76439e1927d14cdd57) )

	ROM_REGION( 0x80000, "bg0", ROMREGION_INVERT )
	ROM_LOAD( "b12-16.17d", 0x00000, 0x10000, CRC(6fec6acb) SHA1(2289c116d3f6093988a088d011f192dd4a99aa77) )
	ROM_LOAD( "b12-18.19d", 0x10000, 0x10000, CRC(64e358aa) SHA1(cd1a23458b1a2f9c8c8aea8086dc04e0f6cc6908) )
	ROM_LOAD( "b12-20.20d", 0x20000, 0x10000, CRC(87f52e89) SHA1(3f8530aca087fa2a32dc6dfbcfe2f86604ee3ca1) )
	ROM_LOAD( "b12-15.17c", 0x40000, 0x10000, CRC(d81107c8) SHA1(c024c9b7956de493687e1373318d4cd74b3555b2) )
	ROM_LOAD( "b12-17.19c", 0x50000, 0x10000, CRC(db1d5a6c) SHA1(c2e1b8d92c2b3b2ce775ed50ca4a37e84ed35a93) )
	ROM_LOAD( "b12-19.20c", 0x60000, 0x10000, CRC(772b2641) SHA1(35cc6d5a725f1817791e710afde992e64d14104f) )

	ROM_REGION( 0x80000, "bg1", ROMREGION_INVERT )
	ROM_LOAD( "b23-06.1c",  0x00000, 0x10000, CRC(44f8f661) SHA1(d77160a89e45556cd9ce211d89c398e1086d8d92) )
	ROM_LOAD( "b23-08.3c",  0x10000, 0x10000, CRC(1ce498c1) SHA1(a9ce3de997089bd40c99bd89919b459c9f215fc8) )
	ROM_LOAD( "b23-07.1d",  0x40000, 0x10000, CRC(d7f6ec89) SHA1(e8da207ddaf46ceff870b45ecec0e89c499291b4) )
	ROM_LOAD( "b23-09.3d",  0x50000, 0x10000, CRC(6651617f) SHA1(6351a0b01589cb181b896285ade70e9dfcd799ec) )

	ROM_REGION( 0x00c00, "proms", 0 )
	// These appear to be twice the correct size
	ROM_LOAD( "b23-04.15l", 0x00000, 0x00400, CRC(5042cffa) SHA1(c969748866a12681cf2dbf25a46da2c4e4f92313) )
	ROM_LOAD( "b23-03.14l", 0x00400, 0x00400, BAD_DUMP CRC(9458fd45) SHA1(7f7cdacf37bb6f15de1109fa73ba3c5fc88893d0) ) // D0 is fixed
	ROM_LOAD( "b23-05.16l", 0x00800, 0x00400, CRC(87f0f69a) SHA1(37df6fd56245fab9beaabfd86fd8f95d7c42c2a5) )
ROM_END

} // anonymous namespace


GAME( 1987, exzisus,  0,       exzisus, exzisus, exzisus_state, empty_init, ROT0, "Taito Corporation",               "Exzisus (Japan, dedicated)",  MACHINE_SUPPORTS_SAVE )
GAME( 1987, exzisusa, exzisus, exzisus, exzisus, exzisus_state, empty_init, ROT0, "Taito Corporation",               "Exzisus (Japan, conversion)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, exzisust, exzisus, exzisus, exzisus, exzisus_state, empty_init, ROT0, "Taito Corporation (TAD license)", "Exzisus (TAD license)",       MACHINE_SUPPORTS_SAVE )
