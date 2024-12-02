// license:BSD-3-Clause
// copyright-holders:

/*
Banpresto and Takara / Tomy joint medal game system

PCBs are marked BPSC-2005KG for Banpresto and TMSC-2004KG01 for Takara / Tomy.
There's also a newer BPSC-2006KG PCB with a BPROM-2006KG sub-board for bigger storage capability.

The main components are:
- H8S/2324 (for Banpresto) or H8S/2329 (for Tomy) CPU. At a first glance they don't seem to make use of
  the internal ROM of the 2329.
- 24.576 MHz XTAL (near H8S)
- LC35V256ET-70W 256K SRAM (near CPU)
- Axell AG-2 AX51201 GFX chip
- IC42S32400-7TG 128 MBit SDRAM (near GFX chip)
- Oki M9810B (for Banpresto) or M9811 (for Tomy) ADPCM chip
- 4.08KSS2FT XTAL (near ADPCM chip)
- 2x bank of 8 DIP switches

The BPSC-2006KG PCB adds a TPA3004D2 audio amplifier.


** means not dumped
*** means not dumped nor identified

Banpresto "Chara Medal Island" series:
    BP-01 - Crayon Shin-chan - Arashi o Yobu! Sushitori Daigassen - クレヨンしんちゃん 嵐を呼ぶ！すし取り大合戦
**  BP-02 - Pocket Monster - Sonans ga Koronda! - ポケットモンスター ソーナンスがころんだ！
    BP-03 - Dragon Ball Z - Bucchigiri no Chou Kessen - ドラゴンボールZ ぶっちぎりの超決戦
*** BP-04 - ??? - ??? (possibly Eye Shield 21 - Nerae!! Gyakuten Touchdown - アイシールド21 狙え!!逆転タッチダウン)
    BP-05 - Chibi Maruko-chan - Wanage de Asobo!! - ちびまる子ちゃん輪投げで遊ぼ!!
**  BP-06 - Doraemon - SOS! Itazura Nezumi wo Yattsukero!! - ドラえもん SOS!いたずらネズミをやっつけろ!!
**  BP-07 - GeGeGe no Kitarou - Taose! Seiyou Youkai Gundan - ゲゲゲの鬼太郎 倒せ！西洋妖怪軍団
**  BP-08 - Keroro Gunsou - Hacha Mecha Yakyu Taikai de Arimasu!! - ケロロ軍曹 ハチャメチャ野球大会であります!!
almost surely more

Takara / Tomy "VS medal" series (games show produced by Amodio and Tomy copyright on title screen):
**  TM-01 - Kurohige Kiki Ippatsu - 黒ひげ危機一発
**  TM-02 - Tomica - トミカ
    TM-03 - Zoids - ゾイド
**  TM-04 - Keitei-oh - 海帝王
    TM-05 - Youkai VS Uchuujin - 妖怪ＶＳ宇宙人
    TM-06 - Satoyama Grand Prix - 里山グランプリ
    TM-07 - Ginga Yakyu - 銀河野球
    TM-08 - Yujo Gattai Shishigami-o - 友情合体獣神王
almost surely more

Banpresto ??? series:
    BPCT1P - ??? - ???
*/


#include "emu.h"

#include "cpu/h8/h8s2329.h"
#include "sound/okim9810.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class banpresto_tomy_h8s_state : public driver_device
{
public:
	banpresto_tomy_h8s_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	void base(machine_config &config);

private:
	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
};


uint32_t banpresto_tomy_h8s_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	return 0;
}


static INPUT_PORTS_START( base )
INPUT_PORTS_END


void banpresto_tomy_h8s_state::program_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
}


void banpresto_tomy_h8s_state::base(machine_config &config)
{
	H8S2320(config, m_maincpu, 24.576_MHz_XTAL); // should be H8S234 or H8S/2329. TODO: fix model once bug when creating a >16K unaligned view is fixed
	m_maincpu->set_addrmap(AS_PROGRAM, &banpresto_tomy_h8s_state::program_map);

	// Axell AX51201

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(60);
	screen.set_size(640, 480);
	screen.set_visarea(0, 639, 0, 479);
	screen.set_screen_update(FUNC(banpresto_tomy_h8s_state::screen_update));

	PALETTE(config, "palette").set_entries(65536); // TODO

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	okim9810_device &oki(OKIM9810(config, "oki", 4.096_MHz_XTAL)); // M9810B or M9811
	oki.add_route(0, "lspeaker", 0.80);
	oki.add_route(1, "rspeaker", 0.80);
}


/***************************************************************************

  ROM definition(s)

***************************************************************************/

ROM_START( cscaoysd )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "bp01-p_ver1.1.ic5", 0x000000, 0x080000, CRC(daa443b1) SHA1(f2a133a9a8cc9678f81a4e7e10f4d15af99ab530) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bp01-g_ver1.0.ic6", 0x000000, 0x400000, CRC(716ae57c) SHA1(e1bc89e0dd126574742496cd32d3586f6e584307) ) // x1xxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "bp01-s_ver1.0.ic4", 0x000000, 0x100000, CRC(40e957a5) SHA1(444d141b54d5f08520521bd89edc1d0bc10486c0) )
ROM_END

ROM_START( dbzbgck )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "bp03-p_ver1.1.ic5", 0x000000, 0x400000, CRC(6f9c1658) SHA1(c175781d6090468c20b7fb4ab118a41a675873c2) ) // 111xxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bp03-g_ver1.0.ic6", 0x000000, 0x400000, CRC(2784fa03) SHA1(15a6da46b604239a7ec829603527c6fed9da10e8) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "bp03-s_ver1.0.ic4", 0x000000, 0x100000, CRC(0cbf97b4) SHA1(c39106e4414cf55bd3b9aaf7d3c37de7fc14b48f) )
ROM_END

ROM_START( cmcwa)
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "bp05-p_ver1.1.ic5", 0x000000, 0x400000, CRC(f5efd28c) SHA1(a565de1bbb88d949af20150c93db850523ac3c85) ) // 111xxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "bp05-g_ver1.0.ic6", 0x000000, 0x400000, CRC(cef0dab1) SHA1(57e4e62801dcc77e4dbb3ace9788f62fed67cb0a) ) // 11xxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "bp05-s_ver1.0.ic4", 0x000000, 0x100000, CRC(5ee334cc) SHA1(6cbc237980d2aadb28970bd5eecec3c2cf9f873f) )
ROM_END

ROM_START( zoids )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "tm03-p_ver1.0.ic5", 0x000000, 0x080000, CRC(becd0060) SHA1(68b72a3e7270f3416935db018b6feeaaa8e4aeae) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "tm03-g_ver1.0.ic6", 0x000000, 0x400000, CRC(1dc85b79) SHA1(f144d085c77efa8b4ff23b75f01b71a3d2c9a3b0) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "tm03-s_ver1.0.ic4", 0x000000, 0x040000, CRC(d0a24b61) SHA1(1e12a20caefcaa220fa83bb60453ea970ee51ac1) )
ROM_END

ROM_START( youkai )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "tm05-p_ver1.0.ic5", 0x000000, 0x100000, CRC(dfec7575) SHA1(f707be1818103834f36772be93b162b2b32fd0e2) ) // 11xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "tm05-g_ver1.0.ic6", 0x000000, 0x400000, CRC(4c999232) SHA1(ca2498753247fe27c7285680df7b23bf65aa648c) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "tm05-s_ver1.0.ic4", 0x000000, 0x100000, CRC(f2534577) SHA1(9ef308c3fc6c04b04a3282417b2660545d8f154e) ) // 11xxxxxxxxxxxxxxxxxx = 0xFF
ROM_END

ROM_START( satoyama )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "tm06-p_ver1.0.ic5", 0x000000, 0x100000, CRC(f02cddba) SHA1(27162dbfdf364e16a490b7dfb3db0b19bcc54d41) ) // 11xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "tm06-g_ver1.0.ic6", 0x000000, 0x400000, CRC(b36fd1ee) SHA1(b576219273d25f6bfcc2e2f6f81e7c4a208fc70b) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "tm06-s_ver1.0.ic4", 0x000000, 0x100000, CRC(133c4dc8) SHA1(9af3ec284212268302b15f89831816a42ba3f353) )
ROM_END

ROM_START( gyakyu )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "tm07-p_ver1.0.ic5", 0x000000, 0x100000, CRC(ca3f2d4b) SHA1(36df59092af114b419d147d78eb4672fe1200ae4) ) // 11xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "tm07-g_ver1.0.ic6", 0x000000, 0x400000, CRC(9ff53c1e) SHA1(3edc38496a54a94df42b0b5ba5c7a888e9c2527c) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "tm07-s_ver1.0.ic4", 0x000000, 0x100000, CRC(05276f20) SHA1(501ce4fc14593e3f5e8ed7e08b8b7006d65212c6) )
ROM_END

ROM_START( yujogs )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "tm08-p_ver1.0.ic5", 0x000000, 0x100000, CRC(8c393893) SHA1(f8fad4f831c08bdf49400dc790a6693718408e15) ) // 1xxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "tm08-g_ver1.0.ic6", 0x000000, 0x400000, CRC(6b08c87c) SHA1(afa4cc64722fadac49fc62bb071c4b86311239f9) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "tm08-s_ver1.0.ic4", 0x000000, 0x100000, CRC(51632db2) SHA1(acc85dfc5250384775f58c66619ec0b701d0b218) )
ROM_END

ROM_START( unkbpmed )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "bpct1p_ver1.0.rom1", 0x000000, 0x400000, CRC(2a0326cf) SHA1(623617d1ffb686673caee3c345b6052b4c5d4af7) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROM_LOAD( "bp-cmtc1-g_ver1.0.rom3", 0x0000000, 0x2000000, CRC(7e482971) SHA1(1e5b0801515292731ac639b6e0d53b406af1138d) )

	ROM_REGION( 0x2000000, "oki", 0 )
	ROM_LOAD( "bp-cmtc1-s_ver1.0.rom2", 0x0000000, 0x2000000, CRC(337d688a) SHA1(276466924e6a23f531a7100d25390fdaae4fecfb) )
ROM_END

} // anonymous namespace


GAME( 2006, cscaoysd, 0, base, base, banpresto_tomy_h8s_state, empty_init, ROT0, "Banpresto",     "Crayon Shin-chan - Arashi o Yobu! Sushitori Daigassen", MACHINE_IS_SKELETON )
GAME( 2006, dbzbgck,  0, base, base, banpresto_tomy_h8s_state, empty_init, ROT0, "Banpresto",     "Dragon Ball Z - Bucchigiri no Chou Kessen",             MACHINE_IS_SKELETON )
GAME( 2007, cmcwa,    0, base, base, banpresto_tomy_h8s_state, empty_init, ROT0, "Banpresto",     "Chibi Maruko-chan - Wanage de Asobo!!",                 MACHINE_IS_SKELETON )
GAME( 2005, zoids,    0, base, base, banpresto_tomy_h8s_state, empty_init, ROT0, "Amodio / Tomy", "Zoids",                                                 MACHINE_IS_SKELETON ) // title to be verified
GAME( 2007, youkai,   0, base, base, banpresto_tomy_h8s_state, empty_init, ROT0, "Amodio / Tomy", "Youkai VS Uchuujin",                                    MACHINE_IS_SKELETON )
GAME( 2007, satoyama, 0, base, base, banpresto_tomy_h8s_state, empty_init, ROT0, "Amodio / Tomy", "Satoyama Grand Prix",                                   MACHINE_IS_SKELETON )
GAME( 2007, gyakyu,   0, base, base, banpresto_tomy_h8s_state, empty_init, ROT0, "Amodio / Tomy", "Ginga Yakyu",                                           MACHINE_IS_SKELETON )
GAME( 2007, yujogs,   0, base, base, banpresto_tomy_h8s_state, empty_init, ROT0, "Amodio / Tomy", "Yujo Gattai Shishigami-o",                              MACHINE_IS_SKELETON ) // title to be verified, inferred from some strings related to characters
GAME( 200?, unkbpmed, 0, base, base, banpresto_tomy_h8s_state, empty_init, ROT0, "Banpresto",     "unknown Banpresto medal game",                          MACHINE_IS_SKELETON )
