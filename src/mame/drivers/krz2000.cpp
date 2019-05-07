// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    krz2000.cpp - Kurzweil K2000 series

    Skeleton driver by R. Belmont

    Hardware in brief:
        TMP68301 CPU @ 16 MHz
        uPD72064 FDC
        85C30 SCSI
        M37450 on I/O board to handle panel/display/keyboard scanning
        HD6303 on I/O board to manage reverb DSP program loading
        VLSI ASIC "Calvin" - Unknown purpose (speculation: handles maincpu and PCM ROM/RAM addressing and DRAM refresh?)
         (on later K2000 and all later K-series units this is replaced by a more advanced VLSI ASIC "Janis")
        2x VLSI ASIC "Hobbes" - Unknown purpose (speculation: 'reverb' DSP, interpolation, accumulation, and shifting and dithering down a wider accumulator to a 16 bit output sample?)

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/tmp68301.h"
#include "machine/bankdev.h"
#include "machine/upd765.h"
#include "machine/ncr5380n.h"
#include "machine/nscsi_cd.h"
#include "machine/nscsi_hd.h"
#include "video/t6963c.h"
#include "screen.h"
#include "emupal.h"
#include "softlist.h"
#include "speaker.h"

class k2000_state : public driver_device
{
public:
	k2000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_1m_bank(*this, "bank1m")
		, m_mainram(*this, "mainram")
	{ }

	void k2000(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void k2000_map(address_map &map);
	void bank_map_1m(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_1m_bank;
	required_shared_ptr<uint16_t> m_mainram;

	DECLARE_WRITE16_MEMBER(ctrl_w);

	void k2000_palette(palette_device &palette) const;
};

WRITE16_MEMBER(k2000_state::ctrl_w)
{
	data &= 0xff;
	logerror("%02x to ctrl_w\n", data);
	// bit 4: 0=program ROM at 0, work RAM at 100000, 1=work RAM at 0, program ROM at 100000.
	m_1m_bank->set_bank((data >> 4) & 1);
}

void k2000_state::machine_start()
{
}

void k2000_state::machine_reset()
{
	m_1m_bank->set_bank(0);
}

void k2000_state::k2000_map(address_map &map)
{
	// word writes to 000000 region - unknown
	// word writes to 000090 region - unknown
	// word writes to 000180 region - unknown
	// word writes to 000240 region - unknown
	map(0x000000, 0x1fffff).m(m_1m_bank, FUNC(address_map_bank_device::amap16));
	map(0x700000, 0x700003).rw("lcd", FUNC(lm24014h_device::read), FUNC(lm24014h_device::write)).umask16(0x00ff);
	map(0x7e0000, 0x7e0001).w(FUNC(k2000_state::ctrl_w));
}

void k2000_state::bank_map_1m(address_map &map)
{
	map(0x000000, 0x0fffff).rom().region("maincpu", 0);
	map(0x100000, 0x1fffff).ram().share("mainram");
	map(0x200000, 0x2fffff).ram().share("mainram");
	map(0x300000, 0x3fffff).rom().region("maincpu", 0);
}

void k2000_state::k2000(machine_config &config)
{
	TMP68301(config, m_maincpu, XTAL(12'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &k2000_state::k2000_map);

	ADDRESS_MAP_BANK(config, "bank1m").set_map(&k2000_state::bank_map_1m).set_options(ENDIANNESS_BIG, 16, 24, 0x200000);

	LM24014H(config, "lcd");

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}



static INPUT_PORTS_START( k2000 )
INPUT_PORTS_END

ROM_START( k2000 )
	ROM_REGION(0x140000, "maincpu", 0)
	// note that since this firmware revision ends with a J, it is intended for a K2000 with the "Janis" Sample/Addressing ASIC (the later mainboard)
	// the earlier board has firmware revisions which end with a C, for the "Calvin" Sample/Addressing ASIC, on the older K2000 mainboard
	ROM_LOAD16_BYTE( "k2j-k2rj_eng_hi__v2.0j_3b69__=c=_1993_yca.tms27c040.u6", 0x000000, 0x080000, CRC(35c17fc3) SHA1(b91deec0127669b46af05a2acaa212e29e49abfb) ) // TMS27C040 EPROM with sticker: "K2J-K2RJ ENG HI // V2.0J 3B69 // (C) 1993 YCA" @ U6
	ROM_LOAD16_BYTE( "k2j-k2rj_eng_lo__v2.0j_0db0__=c=_1993_yca.tms27c040.u3", 0x000001, 0x080000, CRC(11c7f436) SHA1(c2afe84b58d71932f223097ea01812eb513bd740) ) // TMS27C040 EPROM with sticker: "K2J-K2RJ ENG LO // V2.0J 0DB0 // (C) 1993 YCA" @ U3
	ROM_LOAD16_BYTE( "k2j-k2rj_su_hi__v12ts_5e89__=c=_1993_yca.m27c1001.u5", 0x100000, 0x020000, CRC(16e0bdb7) SHA1(962fa10896f6a95210d752be28f02640869893a4) ) // MC27C1001 EPROM with sticker: "K2J-K2RJ SU HI // V12TS 5E89 // (C) 1993 YCA" @ U5
	ROM_LOAD16_BYTE( "k2j-k2rj_su_lo__v12ts_2f52__=c=_1993_yca.m27c1001.u2", 0x100001, 0x020000, CRC(cb11e837) SHA1(bcdf3d5abe8c53727a142008acb2755ed0ecc6ea) ) // MC27C1001 EPROM with sticker: "K2J-K2RJ SU LO // V12TS 2F52 // (C) 1993 YCA" @ U2

	ROM_REGION(0x2000000, "pcm", 0) // 23c080(?) 8MBit mask roms, byte-mode, in pairs. These are common on at least several members of the K2000 series; each has a unique number stored in the JEDEC id field.
	ROM_LOAD16_BYTE( "k2m1h_25da__830106-01__=c=1991_yca__japan_9322_d.ide6ed.u38", 0x000001, 0x100000, CRC(f110b0e7) SHA1(d8731b74b1ca6761f8fd3f6360bfe2f1cc3077bc) ) // Silkscreen: "K2M1H 25DA // 830106-01 // (C)1991 YCA // JAPAN 9322 D" @ U38; chip JEDEC id = e6 ed
	ROM_LOAD16_BYTE( "k2m1l_20fe__830107-01__=c=1991_yca__japan_9324_d.idb9a4.u43", 0x000000, 0x100000, CRC(00715fbe) SHA1(99f661096031b794de216c74ce9b780e9889d344) ) // Silkscreen: "K2M1L 20FE // 830107-01 // (C)1991 YCA // JAPAN 9324 D" @ U43; chip JEDEC id = b9 a4
	ROM_LOAD16_BYTE( "k2m2h_de34__830108-01__=c=1991_yca__japan_9324_d.id0000.u39", 0x200001, 0x100000, CRC(99aae00e) SHA1(7045fb6b19b046f3f068a3581b6498ee62603fb4) ) // Silkscreen: "K2M2H DE34 // 830108-01 // (C)1991 YCA // JAPAN 9324 D" @ U39; chip JEDEC id = 00 00
	ROM_LOAD16_BYTE( "k2m2l_1bdf__830109-01__=c=1991_yca__japan_9324_d.id836c.u44", 0x200000, 0x100000, CRC(b2acd497) SHA1(24d3e84016fa08a990ce4c39294ad47fb0cab3d0) ) // Silkscreen: "K2M2L 1BDF // 830109-01 // (C)1991 YCA // JAPAN 9324 D" @ U44; chip JEDEC id = 83 6c
	ROM_LOAD16_BYTE( "k2m3h_0e87__830110-01__=c=1991_yca__japan_9324_d.iddbfa.u40", 0x400001, 0x100000, CRC(f448694f) SHA1(484593d072c43fe442cd8cc6cc40cd24677b35cc) ) // Silkscreen: "K2M3H 0E87 // 830110-01 // (C)1991 YCA // JAPAN 9324 D" @ U40; chip JEDEC id = db fa
	ROM_LOAD16_BYTE( "k2m3l_3cde__830111-01__=c=1991_yca__japan_9329_d.ide8b5.u46", 0x400000, 0x100000, CRC(be8408f9) SHA1(fbeab2d690532d055d424be52d937e2729b3daac) ) // Silkscreen: "K2M3L 3CDE // 830111-01 // (C)1991 YCA // JAPAN 9329 D" @ U46; chip JEDEC id = e8 b5
	ROM_LOAD16_BYTE( "k2m4h_3f2d__830112-01__=c=1991_yca__japan_9324_d.iddbdb.u42", 0x600001, 0x100000, CRC(da8666f5) SHA1(4d0f306cad9a3a96cf1232b9b8df12fae044a1d6) ) // Silkscreen: "K2M4H 3F2D // 830112-01 // (C)1991 YCA // JAPAN 9324 D" @ U42; chip JEDEC id = db db
	ROM_LOAD16_BYTE( "k2m4l_2e6d__830113-01__=c=1991_yca__japan_9327_d.id3199.u47", 0x600000, 0x100000, CRC(6eb73185) SHA1(fe48fe44be90a856251974750b1eac7f5291e1e6) ) // Silkscreen: "K2M4L 2E6D // 830113-01 // (C)1991 YCA // JAPAN 9327 D" @ U47; chip JEDEC id = 31 99

	/* The K2000 can be expanded by installing up to two
	 *  "K2000 Sound Rom Babybd" P/N 331022-01 sound modules, onto a
	 *  "K2000 SOUND ROM DAUGHTERBOARD" RMB-k P/N 331021-03 daughterboard,
	 * as well as upgrading the board firmware to turn the unit into a
	 * K2000R. The shipping 'K2000R' model just needs this daughterboard
	 * to be added. (Did the K2000R come with the daughterboard populated?)
	 * http://tk386.com/k2000_innards/DSC_8139.jpg
	 *
	 * The 'babyboards' have two mezzanine connectors on them and plug into
	 * the daughterboard, which itself connects to the K2000 mainboard via
	 * connector J12 (a 3-row eurocard connector with 16 pins per row).
	 * The babyboard ROMs are 23c1610 16MBit mask ROMs, used in word mode.
	 *
	 * These same 4-ROM expansion modules can be used on the K2000, K2500,
	 * K2600, and is included in the K2661 from factory.
	 *
	 * Note that the K2600 uses a similar part called RMB-26 which has an
	 * additional 2 ROM slots intended for more dense single-chip ROMs.
	 * see http://kurzweil.com/accessory/rmb-26_soundblock_daughterboard/
	 */


	/* 'Orchestral' babyboard AKA "ROM 1" or 'Kurzweil RM-126':
	 * Note: There are several variations on the ROM markings for the ROMs on
	 * this babyboard, depending on what year it was produced.
	 * The contents are presumed (but not proven) to be the same.
	 * The older board variant presumably has a part number of K91B10162.
	 */
	// older board variant:
	// https://images.reverb.com/image/upload/s--DdCT5Kan--/a_180/f_auto,t_large/v1511800096/wiysqmlb9udd4hgxpppz.jpg
	// https://thumbs.worthpoint.com/zoom/images1/1/0716/15/kurzweil-rm1-orchestral-rom-block_1_3dca4ab8eb35682075d5e5cb77e4876c.jpg
	// Silkscreen: "KZK ROM 1-1 // 830114-01 N // (C) 1993 YCA" @ U1
	// Silkscreen: "KZK ROM 1-2 // 830115-01 N // (C) 1993 YCA" @ U2
	// Silkscreen: "KZK ROM 1-3 // 830116-01 N // (C) 1993 YCA" @ U3
	// Silkscreen: "KZK ROM 1-4 // 830117-01 N // (C) 1993 YCA" @ U4
	// newer board variant:
	// https://media.sweetwater.com/api/i/q-82__ha-8bb2b6f5cf9d25a9__hmac-757ef5825b7f607b540327a23b1e1ace1fe15df5/images/items/750/RM126-large.jpg
	ROM_LOAD( "k2xxrom1-1__830114-01n__2003kurzweil__0402.u1", 0x800000, 0x200000, CRC(39b55c26) SHA1(5937365835a9c1038dfd73d8624f4d7ab0ec48fa) ) // Silkscreen: "K2xxROM1-1 // 830114-01N // <square symbol>2003KURZWEIL // 0402" @ U1
	ROM_LOAD( "k2xxrom1-2__830115-01n__2003kurzweil__0402.u2", 0xa00000, 0x200000, CRC(fba51423) SHA1(4cfdb08c297dca15fddd13963bed0ef501b67c55) ) // Silkscreen: "K2xxROM1-2 // 830115-01N // <square symbol>2003KURZWEIL // 0402" @ U2
	ROM_LOAD( "k2xxrom1-3__830116-01n__2003kurzweil__0402.u3", 0xc00000, 0x200000, CRC(4efedf7f) SHA1(b6888d533a6338aae4d51d3b70f964eb96266069) ) // Silkscreen: "K2xxROM1-3 // 830116-01N // <square symbol>2003KURZWEIL // 0402" @ U3
	ROM_LOAD( "k2xxrom1-4__830117-01n__2003kurzweil__0402.u4", 0xe00000, 0x200000, CRC(4b83d510) SHA1(061a0e954779cb7214ca17bb3a879bb7bc36eddd) ) // Silkscreen: "K2xxROM1-4 // 830117-01N // <square symbol>2003KURZWEIL // 0402" @ U4


	// 'Contemporary' babyboard AKA "ROM 2" or 'Kurzweil RM-226':
	// see https://images.reverb.com/image/upload/s--t9U6Ex2u--/a_180/f_auto,t_large/v1511749743/r0fglivtamtoqrevhatt.jpg
	// and https://images.reverb.com/image/upload/s--6eU1RGQ4--/a_exif,c_limit,e_unsharp_mask:80,f_auto,fl_progressive,g_south,h_620,q_90,w_620/v1524633114/gxnbseznct6cptmspizt.jpg
	// Note there is a older version of contemporary with roms 830131-01
	// thru 830134-01; this older version is a VERY MUCH WANTED dump.
	// It only seems to have been produced during a short period in 1994-1995?
	// The old version may share some roms with the Kurzweil SP76 and SP88
	// Stage Pianos.
	// https://www.mobikin.com/images/android/rom.jpg

	// newer contemporary is 830146-01 4463, 830147-01 C9BE, 830148-01 93B2, 830134-01 F265
	ROM_LOAD( "k2xxrom2-1__830146-01n__2003kurzweil__0402.u1", 0x1000000, 0x200000, CRC(03628775) SHA1(1db7dd514c2a7b810d0e5fef6f614498d695879e) ) // Silkscreen: "K2xxROM2-1 // 830146-01N // <square symbol>2003KURZWEIL // 0402" @ U1; from alt ROM labels, sum16 is 4463
	ROM_LOAD( "k2xxrom2-2__830147-01n__2003kurzweil__0402.u2", 0x1200000, 0x200000, CRC(7b81c227) SHA1(954d41e9fce54eb4a4ce81b5095227a1478a6828) ) // Silkscreen: "K2xxROM2-2 // 830147-01N // <square symbol>2003KURZWEIL // 0402" @ U2; from alt ROM labels, sum16 is c9be
	ROM_LOAD( "k2xxrom2-3__830148-01n__2003kurzweil__0403.u3", 0x1400000, 0x200000, CRC(62af1ba7) SHA1(8910abfd33a939d8a20cd69576c94342f194e23a) ) // Silkscreen: "K2xxROM2-3 // 830148-01N // <square symbol>2003KURZWEIL // 0403" @ U3; from alt ROM labels, sum16 is 93b2
	ROM_LOAD( "k2xxrom2-4__830134-01n__2003kurzweil__0402.u4", 0x1600000, 0x200000, CRC(11f3dfb6) SHA1(a74e040b316f7a6042368c6ae9c2b0cda8656614) ) // Silkscreen: "K2xxROM2-4 // 830134-01N // <square symbol>2003KURZWEIL // 0402" @ U4; from alt ROM labels, sum16 is f265

	// Note1: the roms 830122-01 thru 830127-01 are from the Kurzweil MK-10
	// see https://images.reverb.com/image/upload/s--sBo5xoMU--/a_exif,c_limit,e_unsharp_mask:80,f_auto,fl_progressive,g_south,h_1600,q_80,w_1600/v1504391767/v7iopgveuh6z42kxzobo.jpg
	// Note2: the rom 830129-01 is from the Kurzweil RG200

	ROM_REGION(0x3000, "pals", 0)
	ROM_LOAD( "pseudo_v4d.u11.gal16v8b.jed", 0x000000, 0x000bd0, CRC(43561132) SHA1(a0c567c81022bc7fb83023d89556ccd5aa1ab36d) )
	ROM_LOAD( "sndram_v1.u50.gal16v8b.jed", 0x001000, 0x000bd0, CRC(cabc9335) SHA1(968fa5baa43c7589c901f09b12085437834aeb37) )
	ROM_LOAD( "godot_v5.u10.gal20v8a.jed", 0x002000, 0x00066f, CRC(c6517456) SHA1(b82530d46afdca5f6460e77ac11710cad55a6b89) )
ROM_END

CONS( 1990, k2000, 0, 0, k2000, k2000, k2000_state, empty_init, "Kurzweil Music Systems", "K2000", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )

