// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Poker Spirit (c) 1992 Taito

TODO:
- accesses unmapped range $e80 in TMP68303F for a sprite DMA $300ff2 -> $b10000
- Palette, $a04000 looks the most likely candidate except it spawns 4-bits per gun in dword?
- Opto coin chutes are non-canonical, need to press H then L in quick succession
  (game will presumably draw a coin error if this isn't done right, needs Reset SW to fix)
- EEPROM
- shun: uses more GFX chipset features (i.e. scrolling); needs correct inputs; needs soft reset after
  boot to display something; has unemulated 7-seg display

===================================================================================================

2 PCBs: Base PCB (stickered K11J0730A Poker Spirit) + video PCB (stickered K11X0731A Poker Spirit)

Main components:

Base PCB:
1 x TMP68303F-16
2 x 84256A-10L CMOS 256K-bit low power SRAM
1 x TE7750 I/O expander
1 x M66011 serial bus controller
1 x 93C46 EEPROM
1 x 32.000MHz Osc (near the TMP)
2 x 8-dip banks

Video PCB:
1 x TC0470LIN video custom
1 x TC0600OBT video custom
1 x TC0650FDA video custom
1 x Z8400BB1
1 x OKI M6295
1 x YM2203C
1 x PC060HA CIU
1 x 36.000MHz Osc (near the Z80)

**************************************************************************************************/

#include "emu.h"

#include "taitoio_opto.h"
#include "taitosnd.h"

#include "cpu/m68000/tmp68301.h"
#include "cpu/z80/z80.h"
#include "machine/te7750.h"
#include "sound/okim6295.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class pkspirit_state : public driver_device
{
public:
	pkspirit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mainio(*this, "mainio")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_vidram(*this, "vidram")
		, m_sprram(*this, "sprram")
		, m_mainram(*this, "mainram")
		, m_audiobank(*this, "audiobank")
		, m_opto(*this, "opto%u", 1U)
	{ }

	void pkspirit(machine_config &config);
	void shun(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<tmp68301_device> m_maincpu;
	required_device<te7750_device> m_mainio;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<u16> m_vidram;
	required_shared_ptr<u16> m_sprram;
	required_shared_ptr<u16> m_mainram;

	required_memory_bank m_audiobank;

	required_device_array<taitoio_opto_device, 2> m_opto;

	void vidram_w(offs_t offset, u16 data, u16 mem_mask);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void shun_main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


void pkspirit_state::video_start()
{
}

uint32_t pkspirit_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	uint16_t *spriteram_start = m_mainram + 0xff2 / 2;
	//uint16_t* spriteram_start = m_sprram; // once DMA is handled in the TMP68303 this should be used instead

	for (int i = 0; i < 0x800 / 2; i += 4)
	{
		// somewhat illogical, but the full tilemap sprites need to be drawn first, in reverse order
		// followed by the regular sprites, again in reverse order.
		// (or there is some priority control)
		int reali = ((0x3fc - i) + 0x8) & 0x3ff;

		uint16_t sp0 = spriteram_start[reali + 0];
		uint16_t sp1 = spriteram_start[reali + 1];
		//uint16_t sp2 = spriteram_start[preali + 2];
		uint16_t sp3 = spriteram_start[reali + 3];

		int xpos = sp1 & 0x3ff;
		int ypos = sp0 & 0x1ff;

		ypos = 0x1ff - ypos;

		int sizex;
		int sizey;
		int xshift;

		int base = (sp3 & 0x7ffe);

		int pal = (sp1 & 0xfc00) >> 10;

		if (sp0 & 0x8000) // large tilemap sprites
		{
			sizex = 0x40;
			sizey = 0x40;
			xshift = 0x40;
		}
		else
		{
			xshift = 0;
			sizex = 1;
			sizey = 1;

			// TODO work out which bits are controlling this
			// 0x2000 might be priority?
			switch (sp0 & 0x7e00)
			{
			case 0x0200: // sprites that aren't in use?
			{
				sizex = 1;
				sizey = 1;
				break;
			}
			case 0x0600: // Values on Double Up screen
			{
				sizex = 1;
				sizey = 2;
				break;
			}
			case 0x0e00: // HOLD text
			{
				sizex = 2;
				sizey = 2;
				break;
			}
			case 0x1600: // Game Over
			{
				sizex = 4;
				sizey = 2;
				break;
			}
			case 0x1a00: // TAITO logo on title
			{
				sizex = 4;
				sizey = 4;
				break;
			}

			case 0x3800: // Reel on bonus type 2(?) (not sure this is visible)
			{
				sizex = 4;
				sizey = 2;
				break;
			}
			case 0x3a00: // Reel on bonus type 2
			{
				sizex = 4;
				sizey = 4; // there is still a slight glitch with this reel if 4 is used, maybe priority?
				break;
			}
			}
		}

		//if (sp0 != 0x0000)
		//  printf("sp%03x : %04x %04x %04x %04x\n", i, sp0, sp1, sp2, sp3);

		if (sp0 != 0x0000)
		{
			int count = 0;
			for (int y = 0; y < sizey; y++)
			{
				int ydraw = y * 16 + ypos - 31;

				for (int x = 0; x < sizex; x++)
				{
					int xdraw = x * 16 + xpos - xshift;

					uint16_t tile = m_vidram[base + count];
					m_gfxdecode->gfx(0)->transpen(bitmap, cliprect, tile, pal, 0, 0, xdraw, ydraw, 0);

					count++;
				}
			}
		}

	}

	return 0;
}

void pkspirit_state::machine_start()
{
	m_audiobank->configure_entries(0, 4, memregion("audiocpu")->base(), 0x4000);
}

void pkspirit_state::vidram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vidram[offset]);
	m_gfxdecode->gfx(1)->mark_dirty(offset / 16);
}

void pkspirit_state::main_map(address_map &map) // TODO: verify everything
{
	map(0x000000, 0x01ffff).rom().region("maincpu", 0);
	map(0x100000, 0x10001f).rw(m_mainio, FUNC(te7750_device::read), FUNC(te7750_device::write)).umask16(0x00ff);
	map(0x200000, 0x200001).portr("DSW");
	map(0x300000, 0x30ffff).ram().share("mainram");
	map(0x800001, 0x800001).w("ciu", FUNC(pc060ha_device::master_port_w));
	map(0x800003, 0x800003).rw("ciu", FUNC(pc060ha_device::master_comm_r), FUNC(pc060ha_device::master_comm_w));
	//map(0x900000, 0x900001).w // ?

	map(0xa00000, 0xa0003f).ram(); // is this still palette? (for the uploaded tiles?)
	map(0xa04000, 0xa057ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xb00000, 0xb0ffff).ram().w(FUNC(pkspirit_state::vidram_w)).share("vidram"); // it uploads a 2bpp tileset at c000-cfff, why? it's just another copy of the basic font

	map(0xb10000, 0xb107ff).ram().share("sprram"); // spritelist should be copied here

	map(0xb10800, 0xb1087f).ram(); // control registers for one of the custom GFX chips?
	map(0xb20000, 0xb2001f).ram(); // control registers for one of the custom GFX chips?
}

void pkspirit_state::shun_main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().region("maincpu", 0);
	map(0x100000, 0x10001f).rw(m_mainio, FUNC(te7750_device::read), FUNC(te7750_device::write)).umask16(0x00ff);
	map(0x200000, 0x20ffff).ram().share("mainram");
	map(0x300000, 0x300001).portr("DSW");
	// map(0x310000, 0x310007).rw() // ??
	map(0x800001, 0x800001).w("ciu", FUNC(pc060ha_device::master_port_w));
	map(0x800003, 0x800003).rw("ciu", FUNC(pc060ha_device::master_comm_r), FUNC(pc060ha_device::master_comm_w));
	//map(0x900000, 0x900001).nopw(); // ?

	map(0xa00000, 0xa001ff).ram(); // is this still palette? (for the uploaded tiles?)
	map(0xa04000, 0xa057ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette"); // up to 0xa05fff for this game?
	map(0xb00000, 0xb0ffff).ram().w(FUNC(pkspirit_state::vidram_w)).share("vidram"); // it uploads a 2bpp tileset at c000-cfff, why? it's just another copy of the basic font

	map(0xb10000, 0xb107ff).ram().share("sprram"); // spritelist should be copied here

	map(0xb10800, 0xb1087f).ram(); // control registers for one of the custom GFX chips?
	map(0xb20000, 0xb2001f).ram(); // control registers for one of the custom GFX chips?
}

void pkspirit_state::sound_map(address_map &map) // TODO: verify everything
{
	map(0x0000, 0x3fff).rom().region("audiocpu", 0);
	map(0x4000, 0x7fff).bankr("audiobank");
	map(0x8000, 0x8fff).ram();
	map(0xa000, 0xa000).w("ciu", FUNC(pc060ha_device::slave_port_w));
	map(0xa001, 0xa001).rw("ciu", FUNC(pc060ha_device::slave_comm_r), FUNC(pc060ha_device::slave_comm_w));
	map(0xc000, 0xc001).rw("opn", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xe000, 0xe000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


// TODO: verify some of these labels means anything in normal English, remap accordingly
static INPUT_PORTS_START( pkspirit )
	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_WRITE_LINE_DEVICE_MEMBER("opto1", taitoio_opto_device, coin_sense_w)
	// FIXME: check error triggered if pressed for too long once video emulation is better
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_WRITE_LINE_DEVICE_MEMBER("opto2", taitoio_opto_device, coin_sense_w)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("1 Bet SW")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Duabet SW")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal/Draw SW") // "DE/DR"
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Double SW")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Payout SW")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("opto2", taitoio_opto_device, opto_h_r)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("opto2", taitoio_opto_device, opto_l_r)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("opto1", taitoio_opto_device, opto_h_r)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("opto1", taitoio_opto_device, opto_l_r)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Reset Key")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Last Key")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Meter Key")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE4 ) PORT_NAME("All Clear SW")
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT (0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Hopper Rot") // "Hop Rot"?
	PORT_BIT (0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Pay Out") // Hopper related?
	PORT_BIT (0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Hopper Over") // "Hop Over"?
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	// DSW settings from manual, machine translated. Default is all off.
	// TODO: Lots of settings are defined only as 'A' or 'B', so actual effects on game should be observed when the driver is more complete
	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Credit Type" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, "A" )
	PORT_DIPSETTING(      0x0000, "B" )
	PORT_DIPNAME( 0x0002, 0x0002, "Background Style" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, "Green" )
	PORT_DIPSETTING(      0x0000, "Blue" )
	PORT_DIPNAME( 0x0004, 0x0004, "Card Speed" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, "Fast" )
	PORT_DIPSETTING(      0x0000, "Slow" )
	PORT_DIPNAME( 0x0008, 0x0008, "Card Deal Type" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, "Slide" )
	PORT_DIPSETTING(      0x0000, "Appear" )
	PORT_DIPNAME( 0x0010, 0x0010, "Double Up Type" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, "Without Reel" )
	PORT_DIPSETTING(      0x0000, "With Reel" )
	PORT_DIPNAME( 0x0020, 0x0020, "Max Bet" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPSETTING(      0x0020, "10" )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7") // default off according to dip sheet, but left on for testing convenience
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Card Type" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, "Standard Deck" )
	PORT_DIPSETTING(      0x0000, "Taito Deck" )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Up / Down Credit" ) PORT_DIPLOCATION("SW2:3,4") // actually spelt 'doun' in the dip sheet
	PORT_DIPSETTING(      0x0c00, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, "10" )
	PORT_DIPSETTING(      0x0400, "50" )
	PORT_DIPSETTING(      0x0000, "100" )
	PORT_DIPNAME( 0x1000, 0x1000, "Hopper" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Double Up Reveal Order" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, "Show Chosen Card First" )
	PORT_DIPSETTING(      0x0000, "Show Chosen Card Last" )
	PORT_DIPNAME( 0x4000, 0x4000, "Bell" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	// debug/cheat option
	PORT_DIPNAME( 0x8000, 0x8000, "Credit Pool 500" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


const gfx_layout gfx_16x16x5_planar =
{
	16,16,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(0,5), RGN_FRAC(1,5), RGN_FRAC(2,5), RGN_FRAC(3,5), RGN_FRAC(4,5) },
	{ STEP8(7,-1), STEP8(15,-1) },
	{ STEP16(0,16) },
	16*16
};

const gfx_layout gfx_ram =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0,1 },
	{ STEP8(0,2), STEP8(128,2) },
	{ STEP8(0,16), STEP8(256,16) },
	32*16
};

static GFXDECODE_START( gfx_pkspirit ) // TODO: wrong, needs adjustments
	GFXDECODE_ENTRY( "tiles", 0, gfx_16x16x5_planar, 0, 256)
	GFXDECODE_RAM( "vidram", 0, gfx_ram, 0, 256 )
GFXDECODE_END



void pkspirit_state::pkspirit(machine_config &config)
{
	// basic machine hardware
	TMP68303(config, m_maincpu, 32_MHz_XTAL / 2); // divider not verified, TMP68303F-16
	m_maincpu->set_addrmap(AS_PROGRAM, &pkspirit_state::main_map);
	m_maincpu->parallel_r_cb().set([this]() { logerror("%s par_r\n", machine().describe_context()); return 0xffff; });

	z80_device &audiocpu(Z80(config, "audiocpu", 36_MHz_XTAL / 9)); // divider not verified, but marked as 4MHz on PCB
	audiocpu.set_addrmap(AS_PROGRAM, &pkspirit_state::sound_map);

	TE7750(config, m_mainio);
	// TODO: check me
//  m_mainio->ios_cb().set_constant(7);
	m_mainio->in_port1_cb().set_ioport("IN1");
	m_mainio->in_port2_cb().set_ioport("IN2");
	m_mainio->in_port3_cb().set_ioport("IN3");
	m_mainio->in_port4_cb().set_ioport("IN4");

	for (auto &opto : m_opto)
		TAITOIO_OPTO(config, opto, 0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: wrong
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0*8, 64*8-1, 0*8, 52*8-1); // uncertain height
	screen.set_palette("palette");
	screen.set_screen_update(FUNC(pkspirit_state::screen_update));
	screen.screen_vblank().set_inputline(m_maincpu, 1);

	GFXDECODE(config, "gfxdecode", "palette", gfx_pkspirit);
	PALETTE(config, "palette").set_format(palette_device::xRGB_888, 0x1800/4);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	pc060ha_device &ciu(PC060HA(config, "ciu", 0));
	ciu.nmi_callback().set_inputline("audiocpu", INPUT_LINE_NMI);
	ciu.reset_callback().set_inputline("audiocpu", INPUT_LINE_RESET);

	ym2203_device &opn(YM2203(config, "opn", 36_MHz_XTAL / 9)); // divider not verified
	opn.irq_handler().set_inputline("audiocpu", 0);
	opn.port_a_write_callback().set_membank(m_audiobank).mask(0x03);
	opn.add_route(ALL_OUTPUTS, "mono", 0.30);

	OKIM6295(config, "oki", 1.056_MHz_XTAL, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.5); // all verified
}

void pkspirit_state::shun(machine_config &config)
{
	pkspirit(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &pkspirit_state::shun_main_map);
}


ROM_START( pkspirit )
	ROM_REGION( 0x40000, "maincpu", 0 ) // on base PCB
	ROM_LOAD16_BYTE( "d41_18.ic26", 0x00000, 0x20000, CRC(7bf56618) SHA1(e619325856bf39c2fbc359cf71bde73fbee54d79) ) // 1xxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "d41_17.ic25", 0x00001, 0x20000, CRC(913f9b21) SHA1(f0a29aa07d48f62d2b394874e78ca5c3d26e36a4) ) // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x10000, "audiocpu", 0 ) // on video PCB
	ROM_LOAD( "d41_06.ic21", 0x00000, 0x10000, CRC(64103680) SHA1(81701348691562e296527fb8e1731de2f02d71d1) )

	ROM_REGION( 0xa0000, "tiles", 0 )
	ROM_LOAD16_BYTE( "d41_14.ic12", 0x00000, 0x10000, CRC(3ec2fe4b) SHA1(74877d1623e2336770ff0606d6ca7d7c89a51004) )
	ROM_LOAD16_BYTE( "d41_16.ic16", 0x00001, 0x10000, CRC(6c9d169d) SHA1(e6cd2ddd6b6242e2fadbbcb4b3170dd54391b25e) )
	ROM_LOAD16_BYTE( "d41_12.ic9",  0x20000, 0x10000, CRC(7c521521) SHA1(10ac517921a08ce5387656f33bf3e45879fa614a) )
	ROM_LOAD16_BYTE( "d41_11.ic8",  0x20001, 0x10000, CRC(4913401d) SHA1(d7c92405b7c5505a6b4ac738b6d502cf99f995c6) )
	ROM_LOAD16_BYTE( "d41_10.ic6",  0x40000, 0x10000, CRC(0af0488c) SHA1(36a77b980038731b703f935a6a813bc9295b7889) )
	ROM_LOAD16_BYTE( "d41_09.ic5",  0x40001, 0x10000, CRC(199820a2) SHA1(ae3af3aa424b535fc03ef6bb99fca146bdcd88b1) )
	ROM_LOAD16_BYTE( "d41_07.ic1",  0x60000, 0x10000, CRC(c57b18f8) SHA1(e25f2ff8d0bf312d23b8ff09f07f63e61c3094c6) )
	ROM_LOAD16_BYTE( "d41_08.ic2",  0x60001, 0x10000, CRC(67b941dd) SHA1(cc9abb5d7f3cb90a91921097d23261d80f418287) )
	ROM_LOAD16_BYTE( "d41_15.ic15", 0x80000, 0x10000, CRC(df078e72) SHA1(b781b1ce3b87755513eefc295f00159f495359d8) )
	ROM_LOAD16_BYTE( "d41_13.ic11", 0x80001, 0x10000, CRC(78e1fc0b) SHA1(8475a97cc574971ec201840a71dbf823762f2970) )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 ) // on video PCB
	// empty socket at IC38. Confirmed on 2 different PCBs

	ROM_REGION( 0x80, "eeprom", 0 ) // on base PCB
	ROM_LOAD( "93c46.ic37", 0x00, 0x80, NO_DUMP )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "d41_05.ic13", 0x000, 0x144, NO_DUMP ) // PAL20L8B, on base PCB
	ROM_LOAD( "d41_04.ic19", 0x200, 0x144, NO_DUMP ) // PAL20L8B, on video PCB
ROM_END

ROM_START( shun ) // “駿”
	ROM_REGION( 0x40000, "maincpu", 0 ) // on base PCB
	ROM_LOAD16_BYTE( "d43_13.ic26", 0x00000, 0x20000, CRC(ffaefe8d) SHA1(4851495f53ef6c68ef3274efa2e5d046d62e0ffc) )
	ROM_LOAD16_BYTE( "d43_12.ic25", 0x00001, 0x20000, CRC(a35000c1) SHA1(315006e709f46e65dec5682318d2986b98acdaf2) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // on video PCB
	ROM_LOAD( "d43_01.ic21", 0x00000, 0x10000, CRC(42b85482) SHA1(c121daed24f82b6b55a2475c1c1defb8f59e6b97) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x140000, "tiles", 0 )
	ROM_LOAD16_BYTE( "d43_11.ic16", 0x000000, 0x20000, CRC(5a660455) SHA1(b1ceab84361b3064176939c7afaef6f77378b63d) )
	ROM_LOAD16_BYTE( "d41_16.ic16", 0x000001, 0x20000, CRC(531167e9) SHA1(10ca3a71280989bac74001d386182b7db362c8e1) )
	ROM_LOAD16_BYTE( "d43_07.ic9",  0x040000, 0x20000, CRC(ff94ba37) SHA1(44e40ddc47d10072d90ab29d7ec5c98a7f2837a2) )
	ROM_LOAD16_BYTE( "d43_06.ic8",  0x040001, 0x20000, CRC(be7e8795) SHA1(db6da068674e66c01a90d6fe7f4500ada58282cc) )
	ROM_LOAD16_BYTE( "d43_05.ic6",  0x080000, 0x20000, CRC(dfcf5018) SHA1(f7e263990e08c689a39ff3c49bb118abf2eebf22) )
	ROM_LOAD16_BYTE( "d43_04.ic5",  0x080001, 0x20000, CRC(a35c6917) SHA1(13c42d0bc34198599dc255e1982bb458df30e1e6) )
	ROM_LOAD16_BYTE( "d43_02.ic1",  0x0c0000, 0x20000, CRC(9009759d) SHA1(b0047017b50f4e9437194a2b10b70a27b994f150) )
	ROM_LOAD16_BYTE( "d43_03.ic2",  0x0c0001, 0x20000, CRC(66cd4a76) SHA1(da9b08b5767d7881f0becddd04f2362bdba40a52) )
	ROM_LOAD16_BYTE( "d43_10.ic15", 0x100000, 0x20000, CRC(ab43e4ff) SHA1(c7361cae6a0a8fdc01d37402c5eef3c57d06f1d4) )
	ROM_LOAD16_BYTE( "d43_08.ic11", 0x100001, 0x20000, CRC(41a4723e) SHA1(7273be1b68d8552e4c25cccf455eb1589dc07eda) )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 ) // on video PCB

	ROM_REGION( 0x80, "eeprom", 0 ) // on base PCB
	ROM_LOAD( "93c46.ic37", 0x00, 0x80, NO_DUMP )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "d41_05.ic13", 0x000, 0x144, NO_DUMP ) // PAL20L8B, on base PCB
	ROM_LOAD( "d41_04.ic19", 0x200, 0x144, NO_DUMP ) // PAL20L8B, on video PCB
ROM_END

} // anonymous namespace


GAME( 1992, pkspirit, 0, pkspirit, pkspirit, pkspirit_state, empty_init, ROT0, "Taito Corporation", "Poker Spirit", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, shun,     0, shun,     pkspirit, pkspirit_state, empty_init, ROT0, "Taito Corporation", "Shun",         MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
