// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#include "emu.h"
#include "msx.h"
#include "msx_kanji12.h"
#include "msx_keyboard.h"
#include "msx_matsushita.h"
#include "msx_s1985.h"
#include "msx_systemflags.h"
#include "bus/msx/slot/bunsetsu.h"
#include "bus/msx/slot/disk.h"
#include "bus/msx/slot/fs4600.h"
#include "bus/msx/slot/fsa1fm.h"
#include "bus/msx/slot/msx_rs232.h"
#include "bus/msx/slot/msx_write.h"
#include "bus/msx/slot/music.h"
#include "bus/msx/slot/panasonic08.h"
#include "bus/msx/slot/ram.h"
#include "bus/msx/slot/ram_mm.h"
#include "bus/msx/slot/rom.h"
#include "bus/msx/slot/sony08.h"
#include "softlist_dev.h"

#include "msx_ar.lh"
#include "msx_ar_1fdd.lh"
#include "msx_ar_2fdd.lh"
#include "msx_jp.lh"
#include "msx_jp_1fdd.lh"
#include "msx_jp_2fdd.lh"
#include "msx_kr.lh"
#include "msx_kr_1fdd.lh"
#include "msx_ru.lh"
#include "msx_ru_1fdd.lh"
#include "msx_ru_2fdd.lh"
#include "msx_nocode.lh"
#include "msx_nocode_1fdd.lh"
#include "msx_nocode_2fdd.lh"
#include "msx_nocode_nocaps.lh"

using namespace msx_keyboard;


/***************************************************************************

  MSX2 machine drivers

Undumped and/or not emulated:
- AVT CPC-300 (prototype)
- Bawareth Perfect MSX2
- Daisen Sangyo MX-2021
- Laser MSX2 (unreleased)
- ML-TS2
- Philips NMS 8245 Home Banking (Italy)
- Perfect Perfect2 - MSX2
- Phonola NMS 8245
- Phonola NMS 8280
- Phonola VG-8235
- Sanyo MPC-25F
- Sanyo MPC-25FK
- Sanyo PCT-100
- Sony HB-F750 (prototype)
- Sony HB-G900D
- Sony HB-G900F
- Sony HB-T600
- Sony HB-T7
- Talent DPC-300
- Victor HC-90(A)
- Victor HC-90(B)
- Victor HC-90(V)
- Victor HC-90(T)
- Wandy CPC-300
***************************************************************************/

namespace {

class msx2_state : public msx2_base_state
{
public:
	msx2_state(const machine_config &mconfig, device_type type, const char *tag)
		: msx2_base_state(mconfig, type, tag, 21.477272_MHz_XTAL, 6)
	{
	}

	void ax350(machine_config &config);
	void ax350ii(machine_config &config);
	void ax350iif(machine_config &config);
	void ax370(machine_config &config);
	void ax500(machine_config &config);
	void canonv25(machine_config &config);
	void canonv30(machine_config &config);
	void canonv30f(machine_config &config);
	void cpc300(machine_config &config);
	void cpc300e(machine_config &config);
	void cpc330k(machine_config &config);
	void cpc400(machine_config &config);
	void cpc400s(machine_config &config);
	void cpc61(machine_config &config);
	void cpg120(machine_config &config);
	void cx7128(machine_config &config);
	void cx7m128(machine_config &config);
	void expert20(machine_config &config);
	void fpc900(machine_config &config);
	void kmc5000(machine_config &config);
	void mbh70(machine_config &config);
	void mlg1(machine_config &config);
	void mlg3(machine_config &config);
	void mlg10(machine_config &config);
	void mlg30(machine_config &config);
	void mlg30_2(machine_config &config);
	void mpc2300(machine_config &config);
	void mpc2500f(machine_config &config);
	void mpc25fd(machine_config &config);
	void mpc25fs(machine_config &config);
	void mpc27(machine_config &config);
	void fs4500(machine_config &config);
	void fs4600f(machine_config &config);
	void fs4700f(machine_config &config);
	void fs5000f2(machine_config &config);
	void fs5500f1(machine_config &config);
	void fs5500f2(machine_config &config);
	void fsa1(machine_config &config);
	void fsa1a(machine_config &config);
	void fsa1f(machine_config &config);
	void fsa1fm(machine_config &config);
	void fsa1mk2(machine_config &config);
	void fstm1(machine_config &config);
	void hbf1(machine_config &config);
	void hbf1ii(machine_config &config);
	void hbf1xd(machine_config &config);
	void hbf5(machine_config &config);
	void hbf500(machine_config &config);
	void hbf500_2(machine_config &config);
	void hbf500f(machine_config &config);
	void hbf500p(machine_config &config);
	void hbf700d(machine_config &config);
	void hbf700f(machine_config &config);
	void hbf700p(machine_config &config);
	void hbf700s(machine_config &config);
	void hbf900(machine_config &config);
	void hbf900a(machine_config &config);
	void hbf9p(machine_config &config);
	void hbf9pr(machine_config &config);
	void hbf9s(machine_config &config);
	void hbg900ap(machine_config &config);
	void hbg900p(machine_config &config);
	void hotbit20(machine_config &config);
	void hx23(machine_config &config);
	void hx23f(machine_config &config);
	void hx33(machine_config &config);
	void hx34(machine_config &config);
	void mbh3(machine_config &config);
	void nms8220(machine_config &config);
	void nms8245(machine_config &config);
	void nms8245f(machine_config &config);
	void nms8250(machine_config &config);
	void nms8255(machine_config &config);
	void nms8255f(machine_config &config);
	void nms8260(machine_config &config);
	void nms8280(machine_config &config);
	void nms8280f(machine_config &config);
	void nms8280g(machine_config &config);
	void phc23(machine_config &config);
	void phc23jb(machine_config &config);
	void phc55fd2(machine_config &config);
	void phc77(machine_config &config);
	void tpc310(machine_config &config);
	void tpp311(machine_config &config);
	void tps312(machine_config &config);
	void ucv102(machine_config &config);
	void vg8230(machine_config &config);
	void vg8235(machine_config &config);
	void vg8235f(machine_config &config);
	void vg8240(machine_config &config);
	void victhc80(machine_config &config);
	void victhc90(machine_config &config);
	void victhc90a(machine_config &config);
	void victhc95(machine_config &config);
	void victhc95a(machine_config &config);
	void y503iiir(machine_config &config);
	void y503iiire(machine_config &config);
	void yis604(machine_config &config);
	void y805128(machine_config &config);
	void y805128r2(machine_config &config);
	void y805128r2e(machine_config &config);
	void y805256(machine_config &config);
};

/* MSX2 - AVT CPC-300 (prototype) */

/* MSX2 - Bawareth Perfect MSX2 */

/* MSX2 - Canon V-25 */

ROM_START(canonv25)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("v25bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("v25ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))
ROM_END

void msx2_state::canonv25(machine_config &config)
{
	// YM2149 (in S3527 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S3527
	// 64KB VRAM

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x10000); // 64KB Mapper RAM

	msx2(SND_YM2149, config, layout_msx_jp);
	msx2_64kb_vram(config);
}

/* MSX2 - Canon V-30F */

ROM_START(canonv30f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("v30fbios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("v30fext.rom",  0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("v30fdisk.rom", 0x0000, 0x4000, CRC(b499277c) SHA1(33117c47543a4eb00392cb9ff4e503004999a97a))
ROM_END

void msx2_state::canonv30f(machine_config &config)
{
	// YM2149 (in S3527 MSX Engine)
	// FDC: mb8877a, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK7_MB8877, "disk", 3, 1, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000); // 128KB Mapper RAM

	msx2(SND_YM2149, config, layout_msx_jp_1fdd);
}

/* MSX2 - Daewoo CPC-300 */

ROM_START(cpc300)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("300bios.rom", 0x0000, 0x8000, CRC(53850907) SHA1(affa3c5cd8db79a1450ad8a7f405a425b251653d))

	ROM_REGION(0x8000, "subrom", 0)
	ROM_LOAD("300ext.rom", 0x0000, 0x8000, CRC(d64da39c) SHA1(fb51c505adfbc174df94289fa894ef969f5357bc))

	ROM_REGION(0x8000, "hangul", 0)
	ROM_LOAD("300han.rom", 0x0000, 0x8000, CRC(e78cd87f) SHA1(47a9d9a24e4fc6f9467c6e7d61a02d45f5a753ef))
ROM_END

void msx2_state::cpc300(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot
	// S-1985 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 1, 1, 2, "hangul");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 0, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0x80);   // 128KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 3, 0, 2, "subrom");
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #3

	MSX_S1985(config, "s1985", 0);
	msx2(SND_YM2149, config, layout_msx_kr);
}

/* MSX2 - Daewoo CPC-300E */

ROM_START(cpc300e)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("300ebios.rom", 0x0000, 0x8000, CRC(53850907) SHA1(affa3c5cd8db79a1450ad8a7f405a425b251653d))

	ROM_REGION(0x8000, "subrom", 0)
	ROM_LOAD("300eext.rom",  0x0000, 0x8000, CRC(c52bddda) SHA1(09f7d788698a23aa7eec140237e907d4c37cbfe0))

	ROM_REGION(0x8000, "hangul", 0)
	ROM_LOAD("300ehan.rom", 0x0000, 0x8000, CRC(e78cd87f) SHA1(47a9d9a24e4fc6f9467c6e7d61a02d45f5a753ef))
ROM_END

void msx2_state::cpc300e(machine_config &config)
{
	// YM2149 in S1985
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot
	// No clockchip
	// No joystick ports
	// S1985

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 1, 1, 2, "hangul");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 0, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0x80);   // 128KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 3, 0, 2, "subrom");
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #3

	MSX_S1985(config, "s1985", 0);
	msx2(SND_YM2149, config, layout_msx_kr);
}

/* MSX2 - Daewoo CPC-330K */

ROM_START(cpc330k)
	ROM_REGION(0x18000, "mainrom", 0)
	ROM_LOAD("330kbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(53850907) SHA1(affa3c5cd8db79a1450ad8a7f405a425b251653d)) // need verification

	ROM_REGION(0x8000, "subrom", 0)
	ROM_LOAD("330kext.rom",  0x0000, 0x8000, BAD_DUMP CRC(5d685cca) SHA1(97afbadd8fe34ab658cce8222a27cdbe19bcef39)) // need verification

	ROM_REGION(0x8000, "hangul", 0)
	ROM_LOAD("330khan.rom", 0x0000, 0x4000, BAD_DUMP CRC(3d6dd335) SHA1(d2b058989a700ca772b9591f42c01ed0f45f74d6)) // need verification
ROM_END

void msx2_state::cpc330k(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot
	// DW64MX1
	// Ergonomic keyboard, 2 builtin game controllers

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 1, 1, 2, "hangul");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 0, 2, 0, 4).set_total_size(0x20000);   // 128KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 3, 0, 2, "subrom");
	add_cartridge_slot<1>(config, 1);

	msx2(SND_AY8910, config, layout_msx_kr);
}

/* MSX2 - Daewoo CPC-400 */

ROM_START(cpc400)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("400bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(53850907) SHA1(affa3c5cd8db79a1450ad8a7f405a425b251653d)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("400disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(5fa517df) SHA1(914f6ccb25d78621186001f2f5e2aaa2d628cd0c)) // need verification

	ROM_REGION(0x8000, "subrom", 0)
	ROM_LOAD("400ext.rom",  0x0000, 0x8000, BAD_DUMP CRC(2ba104a3) SHA1(b6d3649a6647fa9f6bd61efc317485a20901128f)) // need verification

	ROM_REGION(0x8000, "hangul", 0)
	ROM_LOAD("400han.rom", 0x0000, 0x8000, BAD_DUMP CRC(a8ead5e3) SHA1(87936f808423dddfd00629056d6807b4be1dc63e)) // need verification

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("400kfn.rom", 0, 0x20000, BAD_DUMP CRC(b663c605) SHA1(965f4982790f1817bcbabbb38c8777183b231a55)) // need verification
ROM_END

void msx2_state::cpc400(machine_config &config)
{
	// AY8910
	// FDC: mb8877a, 1 3.5" DS?DD drive
	// 1 Cartridge slot
	// 1 Expansion slot
	// The check for an external kanji ROM always fails; deliberately? Expects other type of kanji extension? Bad dump?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 1, 1, 2, "hangul");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 0, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0x80);   // 128KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 3, 0, 2, "subrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_disk_mirrored(config, MSX_SLOT_DISK2_MB8877, "disk", 2, 1, 2, "diskrom");
	// Expansion slot in slot #3

	msx2(SND_AY8910, config, layout_msx_kr_1fdd);
}

/* MSX2 - Daewoo CPC-400S */

ROM_START(cpc400s)
	ROM_REGION(0x10000, "mainrom", 0)
	// bios, disk, and sub
	ROM_LOAD("400sbios.u38", 0x0000, 0x10000, CRC(dbb0f26d) SHA1(75f5f0a5a2e8f0935f33bb3cf07b83dd3e5f3347))

	ROM_REGION(0x10000, "hangul", 0)
	// hangul and kanji driver
	ROM_LOAD("400shan.u44", 0x0000, 0x10000, CRC(5fdb493c) SHA1(6b640c1d8cbeda6ca7d6facd16a206b62e059eee))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("400skfn.rom", 0, 0x20000, CRC(fa85368c) SHA1(30fff22e3e3d464993707488442721a5e56a9707))
ROM_END

void msx2_state::cpc400s(machine_config &config)
{
	// AY8910
	// FDC: mb8877a, 1 3.5" DS?DD drive
	// 1 Cartridge slot
	// DW64MX1
	// The check for an external kanji rom always fails; deliberately? expects other type of kanji extension? bad dump?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul1", 0, 1, 1, 1, "hangul", 0x4000);
	add_internal_slot(config, MSX_SLOT_ROM, "kfn", 0, 1, 2, 1, "hangul", 0xc000);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 0, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0x80);   // 128KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 3, 0, 1, "mainrom", 0xc000);
	add_internal_slot(config, MSX_SLOT_ROM, "hangul2", 0, 3, 1, 1, "hangul");
	add_cartridge_slot<1>(config, 1);
	add_internal_disk_mirrored(config, MSX_SLOT_DISK2_MB8877, "disk", 2, 1, 2, "mainrom", 0x8000);
	// Expansion slot in slot #3

	msx2(SND_AY8910, config, layout_msx_kr_1fdd);
}

/* MSX2 - Daewoo Zemmix CPC-61 */

ROM_START(cpc61)
	ROM_REGION(0x10000, "mainrom", 0)
	ROM_LOAD("cpc64122.rom", 0x0000, 0x10000, CRC(914dcad9) SHA1(dd3c39c8cfa06ec69f54c95c3b2291e3da7bd4f2))
ROM_END

void msx2_state::cpc61(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// DW64MX1
	// No keyboard, but a keyboard connector
	// No printer port
	// No cassette port

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 1, 1, 1, "mainrom", 0xc000);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 0, 2, 0, 4).set_total_size(0x10000); // 64KB Mapper RAM?
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 3, 0, 1, "mainrom", 0x8000);
	add_cartridge_slot<1>(config, 1);

	m_hw_def.has_cassette(false).has_printer_port(false);
	msx2(SND_AY8910, config, layout_msx_nocode_nocaps);
}

/* MSX2 - Daewoo Zemmix CPG-120 Normal */

ROM_START(cpg120)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("cpg120bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(b80c8e45) SHA1(310a02a9746bc062834e0cf2fabf7f3e0f7e829e)) // need verification

	ROM_REGION(0x8000, "subrom", 0)
	ROM_LOAD("cpg120ext.rom", 0x0000, 0x8000, BAD_DUMP CRC(b3d740b4) SHA1(7121c3c5ee6e4931fceda14a06f4c0e3b8eda437)) // need verification

	ROM_REGION(0x4000, "music", 0)
	ROM_LOAD("cpg128music.rom", 0x0000, 0x4000, BAD_DUMP CRC(73491999) SHA1(b9ee4f30a36e283a2b1b9a28a70ab9b9831570c6)) // need verification
ROM_END

void msx2_state::cpg120(machine_config &config)
{
	// By pressing the turbo button the CPU can be switched between 3.579545 and 5.369317 MHz
	// YM2149 (in S1985)
	// FDC: None, 0 drives
	// 2 Cartridge slots?
	// S-1985 MSX Engine
	// V9958 VDP
	// FM built in
	// No keyboard, but a keyboard connector?
	// No clock chip?
	// No printer port

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 0, 2, 0, 4).set_total_size(0x20000); // 128KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "saubrom", 0, 3, 0, 2, "subrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_MUSIC, "mus", 2, 1, 1, "music").set_ym2413_tag(m_ym2413);
	add_cartridge_slot<2>(config, 3);

	MSX_S1985(config, "s1985", 0);

	msx_ym2413(config);
	m_hw_def.has_printer_port(false);
	msx2_v9958_base(SND_AY8910, config, layout_msx_nocode_nocaps);
	msx2_add_softlists(config);
}

/* MSX2 - Daisen Sangyo MX-2021 */

/* MSX2 - Fenner FPC-900 */

ROM_START(fpc900)
	ROM_REGION(0x10000, "mainrom", 0)
	ROM_LOAD("fpc900bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("fpc900ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(66237ecf) SHA1(5c1f9c7fb655e43d38e5dd1fcc6b942b2ff68b02)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("fpc900disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(ca3307d3) SHA1(c3efedda7ab947a06d9345f7b8261076fa7ceeef)) // need verification
ROM_END

void msx2_state::fpc900(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: WD2793?, 1 3.5" DSDD drive
	// 2? Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x40000); // 256KB? Mapper RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793, "disk", 3, 3, 1, 2, "diskrom");

	msx2_pal(SND_YM2149, config, layout_msx_nocode_1fdd);
}

/* MSX2 - GR8Bit (should probably be a separate driver) */

/* MSX2 - Gradiente Expert 2.0 */

ROM_START(expert20)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("exp20bios.rom", 0x0000, 0x8000, CRC(6bacdce4) SHA1(9c43106dba3ae2829e9a11dffa9d000ed6d6454c))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("exp20ext.rom", 0x0000, 0x4000, CRC(08ced880) SHA1(4f2a7e0172f0214f025f23845f6e053d0ffd28e8))

	ROM_REGION(0x4000, "xbasic", 0)
	ROM_LOAD("xbasic2.rom", 0x0000, 0x4000, CRC(2825b1a0) SHA1(47370bec7ca1f0615a54eda548b07fbc0c7ef398))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("microsoldisk.rom", 0x0000, 0x4000, CRC(6704ef81) SHA1(a3028515ed829e900cc8deb403e17b09a38bf9b0))
ROM_END

void msx2_state::expert20(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: microsol, 1? 3.5"? DS?DD drive
	// 2 Cartridge slots?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 1, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "xbasic", 1, 1, 1, 1, "xbasic");
	add_internal_disk(config, MSX_SLOT_DISK5_WD2793, "disk", 1, 3, 1, 1, "diskrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 2, 0, 0, 4).set_total_size(0x20000).set_unused_bits(0x80);   // 128KB Mapper RAM
	add_cartridge_slot<2>(config, 3);

	msx2_pal(SND_AY8910, config, layout_msx_nocode_1fdd);
}

/* MSX2 - Hitachi MB-H3 */

ROM_START(mbh3)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mbh3bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("mbh3sub.rom", 0x000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("mbh3firm.rom", 0x0000, 0x8000, CRC(8c844173) SHA1(74ee82cc09ffcf78f6e9a3f0d993f8f80d81444c))
ROM_END

void msx2_state::mbh3(machine_config &config)
{
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 64KB VRAM
	// Touchpad
	// 2 Cartrdige slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0 ,1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 0, 1, 2, "firmware");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000); // 128KB Mapper RAM

	msx2(SND_YM2149, config, layout_msx_jp);
	msx2_64kb_vram(config);
}

/* MSX2 - Hitachi MB-H70 */

ROM_START(mbh70)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mbh70bios.rom" , 0x0000, 0x8000, BAD_DUMP CRC(a27c563d) SHA1(c1e46c00f1e38fc9e0ab487bf0513bd93ce61f3f)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("mbh70ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("mbh70disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(05661a3f) SHA1(e695fc0c917577a3183901a08ca9e5f9c60b8317)) // need verification

	ROM_REGION(0x100000, "firmware", 0)
	ROM_LOAD("mbh70halnote.rom", 0x0000, 0x100000, BAD_DUMP CRC(40313fec) SHA1(1af617bfd11b10a71936c606174a80019762ea71)) // need verification

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("mbh70kfn.rom", 0x0000, 0x20000, BAD_DUMP CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967)) // need verification
ROM_END

void msx2_state::mbh70(machine_config &config)
{
	// YM2149 (in S-1985)
	// FDC: WD2793?, 2? 3.5" DSDD drives
	// S-1985 MSX Engine
	// 3 Cartridge slots?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_SONY08, "firmware", 0, 3, 0, 4, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_N_2_DRIVES, "disk", 3, 0, 1, 2, "diskrom").use_motor_for_led();
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000); // 128KB Mapper RAM

	MSX_S1985(config, "s1985", 0);

	msx2(SND_YM2149, config, layout_msx_jp_2fdd);
}

/* MSX2 - Kawai KMC-5000 */

ROM_START(kmc5000)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("kmc5000bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("kmc5000ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("kmc5000disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(e25cacca) SHA1(607cfca605eaf82e3efa33459d6583efb7ecc13b)) // need verification

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("kmc5000kdr.rom", 0x0000, 0x8000, BAD_DUMP CRC(2dbea5ec) SHA1(ea35cc2cad9cfdf56cae224d8ee41579de37f000)) // need verification

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("kmc5000kfn.rom", 0, 0x20000, BAD_DUMP CRC(c61ddc5d) SHA1(5e872d5853698731a0ed22fb72dbcdfd59cd19c3)) // need verification
ROM_END

void msx2_state::kmc5000(machine_config &config)
{
	// YM2149 (in S-1985)
	// FDC: TC8566AF?, 1? 3.5" DSDD drive
	// S-1985 MSX Engine
	// 2? Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "kdr");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK3_TC8566, "disk", 3, 2, 1, 2, "diskrom");

	MSX_S1985(config, "s1985", 0);

	msx2(SND_YM2149, config, layout_msx_jp_1fdd);
}

/* MSX2 - Laser MSX2 (unreleased) */

/* MSX2 - Mitsubishi ML-G1 */

ROM_START(mlg1)
	// ic8f - hn27256g-25 (paint?)
	// ic10f - hn613128pn33-sp2 - t704p874h52 (sub?)
	// ic11f - hn613256pdc1 - t704p874h50 (bios?)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mlg1bios.rom", 0x0000, 0x8000, CRC(0cc7f817) SHA1(e4fdf518a8b9c8ab4290c21b83be2c347965fc24))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("mlg1ext.rom", 0x0000, 0x4000, CRC(dc0951bd) SHA1(1e9a955943aeea9b1807ddf1250ba6436d8dd276))

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("mlg1paint.rom", 0x0000, 0x8000, CRC(64df1750) SHA1(5cf0abca6dbcf940bc33c433ecb4e4ada02fbfe6))
ROM_END

void msx2_state::mlg1(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: None, 0 drives
	// S3527 MSX Engine
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x10000); // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 3, 0, 2, "firmware");

	msx2_pal(SND_YM2149, config, layout_msx_nocode);
}

/* MSX2 - Mitsubishi ML-G3 */

ROM_START(mlg3)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mlg3bios.rom", 0x0000, 0x8000, CRC(0cc7f817) SHA1(e4fdf518a8b9c8ab4290c21b83be2c347965fc24))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("mlg3ext.rom", 0x0000, 0x4000, CRC(dc0951bd) SHA1(1e9a955943aeea9b1807ddf1250ba6436d8dd276))

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("mlg3disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(b94ebc7a) SHA1(30ba1144c872a0bb1c91768e75a2c28ab1f4e3c6))

	ROM_REGION(0x4000, "rs232", 0)
	ROM_LOAD("mlg3rs232c.rom", 0x0000, 0x2000, CRC(849b325e) SHA1(b1ac74c2550d553579c1176f5dfde814218ec311))
	ROM_RELOAD(0x2000, 0x2000)
ROM_END

void msx2_state::mlg3(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: wd2793?, 1 3.5" DSDD drive
	// S3527 MSX Engine
	// 4 Cartridge slots (3 internal, 1 taken by RS232 board)
	// RS232 with switch. What does the switch do?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_N, "disk", 3, 0, 1, 2, "diskrom").use_motor_for_led();
	add_cartridge_slot<3>(config, 3, 1);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x10000); // 64KB Mapper RAM
	add_internal_slot_irq<4>(config, MSX_SLOT_RS232_MITSUBISHI, "rs232", 3, 3, 1, 1, "rs232");

	msx2_pal(SND_YM2149, config, layout_msx_nocode_1fdd);
}

/* MSX2 - Mitsubishi ML-G10 */

ROM_START(mlg10)
	// ic8f - 00000210 - hn27256g-25
	// ic10f - 5j1 - hn613128p - m50 - t704p874h42-g
	// ic11f - 5j1 - hn613256p - cn9 - t704p874h40-g
	ROM_REGION(0xc000, "mainrom", 0)
	ROM_LOAD("mlg10bios.rom", 0x0000, 0x8000, CRC(a27c563d) SHA1(c1e46c00f1e38fc9e0ab487bf0513bd93ce61f3f))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("mlg10ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("g10firm.rom", 0x0000, 0x8000, CRC(dd4007f9) SHA1(789bb6cdb2d1ed0348f36336da11b149d74e4d9f))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("mlg10kfn.rom", 0, 0x20000, CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967))
ROM_END

void msx2_state::mlg10(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: None, 0 drives
	// S3527 MSX Engine
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x10000); // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 3, 0, 2, "firmware");

	msx2(SND_YM2149, config, layout_msx_jp);
}

/* MSX2 - Mitsubishi ML-G30 Model 1 */

ROM_START(mlg30)
	ROM_REGION(0x10000, "mainrom", 0)
	ROM_LOAD("g30bios.rom", 0x0000, 0x8000, CRC(a27c563d) SHA1(c1e46c00f1e38fc9e0ab487bf0513bd93ce61f3f))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("g30ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("g30disk.rom", 0x0000, 0x4000, CRC(a90be8d5) SHA1(f7c3ac138918a493eb91628ed88cf37999059579))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("g30kfn.rom", 0x0000, 0x20000, CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967))
ROM_END

void msx2_state::mlg30(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: wd2793/tc8566af?, 1 3.5" DSDD drives
	// 4 Cartridge slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_N, "disk", 3, 0, 1, 2, "diskrom").use_motor_for_led();
	add_cartridge_slot<3>(config, 3, 1);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000);   // 128KB Mapper RAM
	add_cartridge_slot<4>(config, 3, 3);

	msx2(SND_YM2149, config, layout_msx_jp_1fdd);
}

/* MSX2 - Mitsubishi ML-G30 Model 2 */

ROM_START(mlg30_2)
	ROM_REGION(0x10000, "mainrom", 0)
	ROM_LOAD("g30bios.rom", 0x0000, 0x8000, CRC(a27c563d) SHA1(c1e46c00f1e38fc9e0ab487bf0513bd93ce61f3f))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("g30ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("g30disk.rom", 0x0000, 0x4000, CRC(995e6bf6) SHA1(6069d63c68b03fa56de040fb5f52eeadbffe2a2c))

	ROM_REGION(0x4000, "rs232", 0)
	ROM_LOAD("g30rs232c.rom", 0x0000, 0x2000, CRC(15d6ba9e) SHA1(782e54cf88eb4a974631eaa707aad97d3eb1ea14))
	ROM_RELOAD(0x2000, 0x2000)

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("g30kfn.rom", 0x0000, 0x20000, CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967))
ROM_END

void msx2_state::mlg30_2(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: wd2793/tc8566af?, 2 3.5" DSDD drives
	// 4 Cartridge slots (1 taken by RS232 board)
	// S3527
	// RS232 with switch. What does the switch do?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_N_2_DRIVES, "disk", 3, 0, 1, 2, "diskrom").use_motor_for_led();
	add_cartridge_slot<3>(config, 3, 1);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000);   // 128KB Mapper RAM
	add_internal_slot_irq<4>(config, MSX_SLOT_RS232_MITSUBISHI, "rs232", 3, 3, 1, 1, "rs232");

	msx2(SND_YM2149, config, layout_msx_jp_2fdd);
}

/* MSX2 - ML-TS100 (should be a separate driver) */

/* MSX2 - ML-TS100M2 (should be a separate driver) */

/* MSX2 - ML-TS2 */

/* MSX2 - NTT Captain Multi-Station (should be a separate driver due to the added V99C37-F) */

/* MSX2 - National FS-4500 */

ROM_START(fs4500)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("4500bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("4500ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "font", 0)
	ROM_LOAD("4500font.rom", 0x0000, 0x4000, CRC(4bd54f95) SHA1(3ce8e35790eb4689b21e14c7ecdd4b63943ee158))

	ROM_REGION(0x8000, "jukugo", 0)
	ROM_LOAD("4500buns.rom", 0x0000, 0x8000, CRC(c9398e11) SHA1(e89ea1e8e583392e2dd9debb8a4b6a162f58ba91))

	ROM_REGION(0x8000, "jusho", 0)
	ROM_LOAD("4500jush.rom", 0x0000, 0x8000, CRC(4debfd2d) SHA1(6442c1c5cece64c6dae90cc6ae3675f070d93e06))

	ROM_REGION(0xc000, "wordpro1", 0)
	ROM_LOAD("4500wor1.rom", 0x0000, 0xc000, CRC(0c8b5cfb) SHA1(3f047469b62d93904005a0ea29092e892724ce0b))

	ROM_REGION(0xc000, "wordpro2", 0)
	ROM_LOAD("4500wor2.rom", 0x0000, 0xc000, CRC(d9909451) SHA1(4c8ea05c09b40c41888fa18db065575a317fda16))

	ROM_REGION(0x4000, "kdr1", 0)
	ROM_LOAD("4500kdr1.rom", 0x0000, 0x4000, CRC(f8c7f0db) SHA1(df07e89fa0b1c7874f9cdf184c136f964fea4ff4))

	ROM_REGION(0x4000, "kdr2", 0)
	ROM_LOAD("4500kdr2.rom", 0x0000, 0x4000, CRC(69e87c31) SHA1(c63db26660da96af56f8a7d3ea18544b9ae5a37c))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("4500kfn.rom", 0, 0x20000, CRC(956dc96d) SHA1(9ed3ab6d893632b9246e91b412cd5db519e7586b))

	ROM_REGION(0x20000, "bunsetsu", 0)
	ROM_LOAD("4500budi.rom", 0, 0x20000, CRC(f94590f8) SHA1(1ebb06062428fcdc66808a03761818db2bba3c73))
ROM_END

void msx2_state::fs4500(machine_config &config)
{
	// YM2149 (in S3527 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S3527 MSX Engine
	// Matsushita switched device
	// 8KB SRAM (NEC D4364C-15L)
	// Builtin thermal printer
	// Switch to switch between JP50on and JIS keyboard layout
	// Switch to switch between internal and external printer
	// Switch to bypass the firmware

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "font", 0, 2, 0, 1, "font");
	add_internal_slot(config, MSX_SLOT_BUNSETSU, "jukugo", 0, 2, 1, 2, "jukugo").set_bunsetsu_region_tag("bunsetsu");
	add_internal_slot(config, MSX_SLOT_ROM, "jusho", 0, 3, 1, 2, "jusho");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "wordpro1", 3, 0, 0, 3, "wordpro1");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr1", 3, 0, 3, 1, "kdr1");
	add_internal_slot(config, MSX_SLOT_ROM, "wordpro2", 3, 1, 0, 3, "wordpro2");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr2", 3, 1, 3, 1, "kdr2");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM

	MSX_MATSUSHITA(config, "matsushita", 0);
	msx2(SND_YM2149, config, layout_msx_jp);
}

/* MSX2 - National FS-4600F */

ROM_START(fs4600f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("4600bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("4600ext.rom", 0x0000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("4600disk.rom", 0x0000, 0x4000, CRC(ae4e65b7) SHA1(073feb8bb645d935e099afaf61e6f04f52adee42))

	ROM_REGION(0x4000, "font1", 0)
	ROM_LOAD("4600fon1.rom", 0x0000, 0x4000, CRC(7391389b) SHA1(31292b9ca9fe7d1d8833530f44c0a5671bfefe4e))

	ROM_REGION(0x4000, "font2", 0)
	ROM_LOAD("4600fon2.rom", 0x0000, 0x4000, CRC(c3a6b445) SHA1(02155fc25c9bd23e1654fe81c74486351e1ecc28))

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("4600kdr.rom", 0x0000, 0x8000, CRC(b2db6bf5) SHA1(3a9a942ed888dd641cddf8deada1879c454df3c6))

	ROM_REGION(0x100000, "firmware", 0)
	ROM_LOAD("4600firm.rom", 0x0000, 0x100000, CRC(1df57472) SHA1(005794c10a4237de3907ba4a44d436078d3c06c2))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("4600kfn.rom", 0, 0x20000, CRC(c61ddc5d) SHA1(5e872d5853698731a0ed22fb72dbcdfd59cd19c3))

	/* Matsushita 12 dots Kanji ROM must be emulated */
	ROM_REGION(0x20000, "kanji12", 0)
	ROM_LOAD("4600kf12.rom", 0, 0x20000, CRC(340d1ef7) SHA1(a7a23dc01314e88381eee88b4878b39931ab4818))
ROM_END

void msx2_state::fs4600f(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: mb8877a, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S-1985 MSX Engine
	// 8KB SRAM (NEC D4364C-15L)
	// Builtin thermal printer
	// Switch to switch between JP50on and JIS keyboard layout
	// Switch to switch between internal and external printer
	// Switch to bypass the firmware

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "font1", 0, 2, 0, 1, "font1");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 0, 2, 1, 2, "kdr");
	add_internal_slot(config, MSX_SLOT_ROM, "font2", 0, 3, 0, 1, "font2");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_FS4600, "firmware", 3, 1, 0, 4, "firmware");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0x80);   // 128KB Mapper RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK2_MB8877, "disk", 3, 3, 1, 2, "diskrom");

	msx_kanji12_device &kanji12(MSX_KANJI12(config, "kanji12", 0));
	kanji12.set_rom_start("kanji12");

	MSX_S1985(config, "s1985", 0);

	msx2(SND_YM2149, config, layout_msx_jp_1fdd);
}

/* MSX2 - National FS-4700 */

ROM_START(fs4700f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("4700bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("4700ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "diskrom", 0)
	// ic403 - disk basic - copyright mei - 1986 das4700j1
	ROM_LOAD("4700disk.rom",  0x0000, 0x4000, CRC(1e7d6512) SHA1(78cd7f847e77fd8cd51a647efb2725ba93f4c471))

	ROM_REGION(0x4000, "font", 0)
	ROM_LOAD("4700font.rom", 0x0000, 0x4000, CRC(4bd54f95) SHA1(3ce8e35790eb4689b21e14c7ecdd4b63943ee158))

	ROM_REGION(0x8000, "jukugo", 0)
	ROM_LOAD("4700buns.rom", 0x0000, 0x8000, CRC(c9398e11) SHA1(e89ea1e8e583392e2dd9debb8a4b6a162f58ba91))

	ROM_REGION(0x8000, "jusho", 0)
	ROM_LOAD("4700jush.rom", 0x0000, 0x8000, CRC(4debfd2d) SHA1(6442c1c5cece64c6dae90cc6ae3675f070d93e06))

	ROM_REGION(0xc000, "wordpro1", 0)
	ROM_LOAD("4700wor1.rom", 0x0000, 0xc000, CRC(5f39a727) SHA1(f5af1d2a8bcf247f78847e1a9d995e581df87e8e))

	ROM_REGION(0xc000, "wordpro2", 0)
	ROM_LOAD("4700wor2.rom", 0x0000, 0xc000, CRC(d9909451) SHA1(4c8ea05c09b40c41888fa18db065575a317fda16))

	ROM_REGION(0x4000, "kdr1", 0)
	ROM_LOAD("4700kdr1.rom", 0x0000, 0x4000, CRC(f8c7f0db) SHA1(df07e89fa0b1c7874f9cdf184c136f964fea4ff4))

	ROM_REGION(0x4000, "kdr2", 0)
	ROM_LOAD("4700kdr2.rom", 0x0000, 0x4000, CRC(69e87c31) SHA1(c63db26660da96af56f8a7d3ea18544b9ae5a37c))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("4700kfn.rom", 0, 0x20000, CRC(956dc96d) SHA1(9ed3ab6d893632b9246e91b412cd5db519e7586b))

	ROM_REGION(0x20000, "bunsetsu", 0)
	ROM_LOAD("4700budi.rom", 0, 0x20000, CRC(f94590f8) SHA1(1ebb06062428fcdc66808a03761818db2bba3c73))
ROM_END

void msx2_state::fs4700f(machine_config &config)
{
	// YM2149 (in S3527 MSX Engine)
	// FDC: mb8877a, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S3527 MSX Engine
	// Matsushita switched device
	// 8KB SRAM (NEC D4364C-15L)
	// Builtin thermal printer
	// Switch to switch between JP50on and JIS keyboard layout
	// Switch to switch between internal and external printer
	// Switch to bypass the firmware

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "font", 0, 2, 0, 1, "font");
	add_internal_slot(config, MSX_SLOT_BUNSETSU, "buns", 0, 2, 1, 2, "jukugo").set_bunsetsu_region_tag("bunsetsu");
	add_internal_slot(config, MSX_SLOT_ROM, "jusho", 0, 3, 1, 2, "jusho");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "wordpro1", 3, 0, 0, 3, "wordpro1");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr1", 3, 0, 3, 1, "kdr1");
	add_internal_slot(config, MSX_SLOT_ROM, "wordpro2", 3, 1, 0, 3, "wordpro2");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr2", 3, 1, 3, 1, "kdr2");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK2_MB8877, "disk", 3, 3, 1, 2, "diskrom");

	MSX_S1985(config, "s1985", 0);

	MSX_MATSUSHITA(config, "matsushita", 0);

	msx2(SND_YM2149, config, layout_msx_jp_1fdd);
}

/* MSX2 - National FS-5000F2 */

ROM_START(fs5000f2)
	ROM_REGION(0x18000, "mainrom", ROMREGION_ERASEFF)
	ROM_LOAD("5000bios.rom", 0x0000, 0x8000, CRC(a44ea707) SHA1(59967765d6e9328909dee4dac1cbe4cf9d47d315))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("5000ext.rom",  0x0000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("5000disk.rom", 0x0000, 0x4000, CRC(ae4e65b7) SHA1(073feb8bb645d935e099afaf61e6f04f52adee42))

	ROM_REGION(0x8000, "setup", 0)
	ROM_LOAD("5000rtc.rom", 0x0000, 0x8000, CRC(03351598) SHA1(98bbfa3ab07b7a5cad55d7ddf7cbd9440caa2a86))

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("5000kdr.rom", 0x0000, 0x8000, CRC(b2db6bf5) SHA1(3a9a942ed888dd641cddf8deada1879c454df3c6))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("5000kfn.rom", 0, 0x20000, CRC(c61ddc5d) SHA1(5e872d5853698731a0ed22fb72dbcdfd59cd19c3))
ROM_END

void msx2_state::fs5000f2(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: wd2793, 2 3.5" DSDD drives
	// 3 Expansion slots
	// 2 Cartridge slots
	// S-1985 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "cn904", 0, 1, 0, 4, "mainrom", 0x8000); // expansion slot
	add_internal_slot(config, MSX_SLOT_ROM, "cn906", 0, 2, 0, 4, "mainrom", 0x8000); // expansion slot
	add_internal_slot(config, MSX_SLOT_ROM, "cn907", 0, 3, 0, 4, "mainrom", 0x8000); // expansion slot
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 0, 1, 2, "kdr");
	add_internal_slot(config, MSX_SLOT_ROM, "rtcrom", 3, 1, 1, 2, "setup");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0x80);   // 128KB Mapper RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK2_WD2793_2_DRIVES, "disk", 3, 3, 1, 2, "diskrom");

	MSX_S1985(config, "s1985", 0);

	msx2(SND_YM2149, config, layout_msx_jp_2fdd);
}

/* MSX2 - National FS-5500F1 */

ROM_START(fs5500f1)
	// ic206
	// OS1 ROM
	// MATSUSHITA
	// 1985 DAS5500A1
	// DFQT9062ZA
	// MSL27256K
	//
	// ic208
	// HKN ROM
	// MASTUSHITA
	// 1985 DAS5500E1
	// DFQT9068ZA
	//
	// ic209
	// API ROM
	// MATSUSHITA
	// 1985 DAS500C1
	// DFQT9064ZA
	// MSL27256K
	//
	// ic210
	// OS2 ROM
	// MATSUSHITA
	// 1985 DAS5500B1
	// DFQT9063ZA
	// HN4827128G-25
	//
	// ic211
	// FDC ROM
	// MATSUSHITA
	// 1985 DAS5500F1
	// DFQT9067ZA
	// HN4827128G-25
	ROM_REGION(0x18000, "mainrom", ROMREGION_ERASEFF)
	ROM_LOAD("5500bios.rom", 0x0000, 0x8000, CRC(5bf38e13) SHA1(44e0dd215b2a9f0770dd76fb49187c05b083eed9))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("5500ext.rom", 0x0000, 0x4000, CRC(3c42c367) SHA1(4be8371f3b03e70ddaca495958345f3c4f8e2d36))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("5500disk.rom", 0x0000, 0x4000, CRC(1e7d6512) SHA1(78cd7f847e77fd8cd51a647efb2725ba93f4c471))

	ROM_REGION(0x8000, "impose", 0)
	ROM_LOAD("5500imp.rom", 0x0000, 0x8000, CRC(6173a88c) SHA1(b677a861b67e8763a11d5dcf52416b42493ade57))

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("5500kdr.rom", 0x0000, 0x8000, CRC(b2db6bf5) SHA1(3a9a942ed888dd641cddf8deada1879c454df3c6))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("tc531000p_6611.ic212", 0, 0x20000, CRC(956dc96d) SHA1(9ed3ab6d893632b9246e91b412cd5db519e7586b))
ROM_END

void msx2_state::fs5500f1(machine_config &config)
{
	// YM2149 in (S3527 MSX Engine)
	// FDC: mb8877a, 1 3.5" DSDD drive
	// 3 Expansion slots
	// 2 Cartridge slots
	// S3527 MSX Engine
	// Matsushita switched device
	// Switch to switch between JP50on and JIS keyboard layout
	// Switch to bypass the firmware

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "cn904", 0, 1, 0, 4, "mainrom", 0x8000); // expansion slot
	add_internal_slot(config, MSX_SLOT_ROM, "cn906", 0, 2, 0, 4, "mainrom", 0x8000); // expansion slot
	add_internal_slot(config, MSX_SLOT_ROM, "cn908", 0, 3, 0, 4, "mainrom", 0x8000); // expansion slot
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 0, 1, 2, "kdr");
	add_internal_slot(config, MSX_SLOT_ROM, "impose", 3, 1, 1, 2, "impose");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK2_MB8877, "disk", 3, 3, 1, 2, "diskrom");

	MSX_MATSUSHITA(config, "matsushita", 0);

	msx2(SND_YM2149, config, layout_msx_jp_1fdd);
}

/* MSX2 - National FS-5500F2 */

ROM_START(fs5500f2)
	ROM_REGION(0x18000, "mainrom", ROMREGION_ERASEFF)
	ROM_LOAD("5500bios.rom", 0x0000, 0x8000, CRC(5bf38e13) SHA1(44e0dd215b2a9f0770dd76fb49187c05b083eed9))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("5500ext.rom", 0x0000, 0x4000, CRC(3c42c367) SHA1(4be8371f3b03e70ddaca495958345f3c4f8e2d36))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("5500disk.rom", 0x0000, 0x4000, CRC(1e7d6512) SHA1(78cd7f847e77fd8cd51a647efb2725ba93f4c471))

	ROM_REGION(0x8000, "impose", 0)
	ROM_LOAD("5500imp.rom", 0x0000, 0x8000, CRC(6173a88c) SHA1(b677a861b67e8763a11d5dcf52416b42493ade57))

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("5500kdr.rom", 0x0000, 0x8000, CRC(b2db6bf5) SHA1(3a9a942ed888dd641cddf8deada1879c454df3c6))

	ROM_REGION(0x20000, "kanji", 0)
	// ic212 - tc531000p
	ROM_LOAD("5500kfn.rom", 0, 0x20000, CRC(956dc96d) SHA1(9ed3ab6d893632b9246e91b412cd5db519e7586b))
ROM_END

void msx2_state::fs5500f2(machine_config &config)
{
	// YM2149 in (S3527 MSX Engine)
	// FDC: mb8877a, 2 3.5" DSDD drives
	// 3 Expansion slots
	// 2 Cartridge slots
	// S3527 MSX Engine
	// Matsushita switched device
	// Switch to switch between JP50on and JIS keyboard layout
	// Switch to bypass the firmware

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "cn904", 0, 1, 0, 4, "mainrom", 0x8000); // expansion slot
	add_internal_slot(config, MSX_SLOT_ROM, "cn906", 0, 2, 0, 4, "mainrom", 0x8000); // expansion slot
	add_internal_slot(config, MSX_SLOT_ROM, "cn908", 0, 3, 0, 4, "mainrom", 0x8000); // expansion slot
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 0, 1, 2, "kdr");
	add_internal_slot(config, MSX_SLOT_ROM, "impose", 3, 1, 1, 2, "impose");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK2_MB8877_2_DRIVES, "disk", 3, 3, 1, 2, "diskrom");

	MSX_MATSUSHITA(config, "matsushita", 0);

	msx2(SND_YM2149, config, layout_msx_jp_2fdd);
}

/* MSX2 - Panasonic FS-A1 */

ROM_START(fsa1)
	ROM_REGION(0x20000, "mainrom", 0)
	ROM_LOAD("fsa1.ic3", 0x00000, 0x20000, CRC(4d6dae42) SHA1(7bbe3f355d3129592268ae87f40ea7e3ced88f98))
ROM_END

void msx2_state::fsa1(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: None, 0 drives
	// S1985
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 0, 4);  // 64 KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 2, "mainrom", 0x8000);
	add_internal_slot(config, MSX_SLOT_ROM, "deskpac1", 3, 2, 1, 2, "mainrom", 0x10000);
	add_internal_slot(config, MSX_SLOT_ROM, "deskpac2", 3, 3, 1, 2, "mainrom", 0x18000);

	MSX_S1985(config, "s1985", 0);

	msx2(SND_YM2149, config, layout_msx_nocode_nocaps);
}

/* MSX2 - Panasonic FS-A1 (a) */

ROM_START(fsa1a)
	ROM_REGION(0x1c000, "maincpu", 0)
	ROM_LOAD("a1bios.rom",   0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))
	ROM_LOAD("a1ext.rom",    0x8000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))
	ROM_LOAD("a1desk1a.rom", 0xc000, 0x8000, CRC(25b5b170) SHA1(d9307bfdaab1312d25e38af7c0d3a7671a9f716b))
	ROM_LOAD("a1desk2.rom", 0x14000, 0x8000, CRC(7f6f4aa1) SHA1(7f5b76605e3d898cc4b5aacf1d7682b82fe84353))
ROM_END

void msx2_state::fsa1a(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: None, 0 drives
	// S1985
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "bios", 0, 0, 2, "maincpu");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 0, 4);  // 64KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "ext", 3, 1, 0, 1, "maincpu", 0x8000);
	add_internal_slot(config, MSX_SLOT_ROM, "desk1", 3, 2, 1, 2, "maincpu", 0xc000);
	add_internal_slot(config, MSX_SLOT_ROM, "desk2", 3, 3, 1, 2, "maincpu", 0x14000);

	MSX_S1985(config, "s1985", 0);

	msx2(SND_YM2149, config, layout_msx_nocode_nocaps);
}

/* MSX2 - Panasonic FS-A1F */

ROM_START(fsa1f)
	ROM_REGION(0x20000, "mainrom", 0)
	ROM_LOAD("da1024d0365r.ic18", 0x00000, 0x20000, CRC(64a53ec8) SHA1(9a62d7a5ccda974261f7c0600476d85e10deb99b))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("da531p6616_0.ic17", 0, 0x20000, CRC(c61ddc5d) SHA1(5e872d5853698731a0ed22fb72dbcdfd59cd19c3)) // da531p6616-0 - tc531000ap-6616
ROM_END

void msx2_state::fsa1f(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: tc8566af, 1 3.5" DSDD drive
	// S1985
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000).set_unused_bits(0x80);   // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "mainrom", 0x8000);
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "mainrom", 0x10000);
	add_internal_disk_mirrored(config, MSX_SLOT_DISK3_TC8566, "disk", 3, 2, 1, 2, "mainrom", 0xc000);
	add_internal_slot(config, MSX_SLOT_ROM, "cockpit", 3, 3, 1, 2, "mainrom", 0x18000);

	MSX_S1985(config, "s1985", 0);

	msx2(SND_YM2149, config, layout_msx_jp_1fdd);
}

/* MSX2 - Panasonic FS-A1FM */

ROM_START(fsa1fm)
	// Everything should be in one rom??
	// da534p66220 - tc534000p-6620 - ic12 - 4mbit rom
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("a1fmbios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("a1fmext.rom", 0x0000, 0x4000, CRC(ad295b5d) SHA1(d552319a19814494e3016de4b8f010e8f7b97e02))

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("a1fmdisk.rom", 0x0000, 0x4000, BAD_DUMP CRC(e25cacca) SHA1(607cfca605eaf82e3efa33459d6583efb7ecc13b))

	ROM_REGION(0x100000, "firmware", 0)
	ROM_LOAD("a1fmfirm.rom", 0x000000, 0x100000, CRC(8ce0ece7) SHA1(f89e3d8f3b6855c29d71d3149cc762e0f6918ad5))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("a1fmkfn.rom", 0, 0x20000, CRC(c61ddc5d) SHA1(5e872d5853698731a0ed22fb72dbcdfd59cd19c3))

	ROM_REGION(0x20000, "kanji12", 0)
	ROM_LOAD("a1fmkf12.rom", 0, 0x20000, CRC(340d1ef7) SHA1(a7a23dc01314e88381eee88b4878b39931ab4818))
ROM_END

void msx2_state::fsa1fm(machine_config &config)
{
	// AY8910 (in T9769)
	// FDC: tc8566af, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// T9769
	// 8KB SRAM
	// Integrated 1200baud modem

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000).set_unused_bits(0x80);   // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_FSA1FM, "modem", 3, 1, 1, 1, "firmware");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK3_TC8566, "disk", 3, 2, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_FSA1FM2, "firmware", 3, 3, 0, 3, "firmware");

	msx_kanji12_device &kanji12(MSX_KANJI12(config, "kanji12", 0));
	kanji12.set_rom_start("kanji12");

	msx2(SND_AY8910, config, layout_msx_jp_1fdd);
}

/* MSX2 - Panasonic FS-A1MK2 */

ROM_START(fsa1mk2)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("a1mkbios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("a1mk2ext.rom", 0x0000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))

	ROM_REGION(0x8000, "deskpac1", 0)
	ROM_LOAD("a1mkcoc1.rom", 0x0000, 0x8000, CRC(0eda3f57) SHA1(2752cd89754c05abdf7c23cba132d38e3ef0f27d))

	ROM_REGION(0x4000, "deskpac2", 0)
	ROM_LOAD("a1mkcoc2.rom", 0x0000, 0x4000, CRC(756d7128) SHA1(e194d290ebfa4595ce0349ea2fc15442508485b0))

	ROM_REGION(0x8000, "deskpac3", 0)
	ROM_LOAD("a1mkcoc3.rom", 0x0000, 0x8000, CRC(c1945676) SHA1(a3f4e2e4934074925d775afe30ac72f150ede543))
ROM_END

void msx2_state::fsa1mk2(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S1985
	// pause button

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000).set_unused_bits(0x80);   // 64 KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "deskpac1", 3, 1, 1, 2, "deskpac1");
	add_internal_slot(config, MSX_SLOT_ROM, "deskpac2", 3, 2, 1, 1, "deskpac2");
	add_internal_slot(config, MSX_SLOT_ROM, "deskpac3", 3, 3, 1, 2, "deskpac3");

	MSX_S1985(config, "s1985", 0);

	msx2(SND_YM2149, config, layout_msx_jp);
}

/* MSX2 - Philips HCS 280 */

/* MSX2 - Philips NMS 8220 - 2 possible sets (/00 /16) */

ROM_START(nms8220)
	ROM_REGION(0x20000, "mainrom", 0)
	ROM_LOAD("m531000-52_68503.u14", 0x0000, 0x20000, CRC(f506d7ab) SHA1(e761e7081c613ad4893a664334ce105841d0e80e))

	ROM_REGION(0x4000, "designer", 0)
	ROM_SYSTEM_BIOS(0, "v1308", "13-08-1986 Designer")
	ROMX_LOAD("8220pena.u13", 0x0000, 0x4000, CRC(17817b5a) SHA1(5df95d033ae70b107697b69470126ce1b7ae9eb5), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v1206", "12-06-1986 Designer")
	ROMX_LOAD("8220pen.u13",  0x0000, 0x4000, CRC(3d38c53e) SHA1(cb754aed85b3e97a7d3c5894310df7ca18f89f41), ROM_BIOS(1))
ROM_END

void msx2_state::nms8220(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom", 0x10000);
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "mainrom", 0x18000);
	// Memory mapper blocks mirrored every 8 blocks: 4x ram, 4x empty, 4x ram, 4x empty, etc
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x10000).set_unused_bits(0xf8);   // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "designer", 3, 3, 1, 1, "designer");
	add_internal_slot(config, MSX_SLOT_ROM, "designer_mirror", 3, 3, 2, 1, "designer");

	msx2_pal(SND_YM2149, config, layout_msx_nocode);
}

/* MSX2 - Philips NMS 8245 - 2 possible sets (/00 /16) */
/* /00 - A16 = 0 */
/* /16 - A16 = 1 */
/* /19 - Azerty keyboard */

ROM_START(nms8245)
	ROM_REGION(0x10000, "mainrom", 0)
	ROM_SYSTEM_BIOS(0, "v1.08", "v1.08")
	ROMX_LOAD("v1_08.u7", 0x00000, 0x10000, CRC(69d5cbe6) SHA1(cc57c1dcd7249ea9f8e2547244592e7d97308ed0), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v1.06", "v1.06")
	ROMX_LOAD("v1_06.u7", 0x00000, 0x10000, CRC(be4ae17e) SHA1(b746192dc333eaf2a725a44777303808a3649d63), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v1.05", "v1.05")
	ROMX_LOAD("v1_05.u7", 0x00000, 0x10000, CRC(cef8895d) SHA1(816ceb21088b116ec34be0ff869d02222b525e58), ROM_BIOS(2))
ROM_END

void msx2_state::nms8245(machine_config &config)
{
	// XTAL: 21328.1 (different from default)
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "mainrom", 0x8000);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0xf8);   // 128KB Mapper RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793, "disk", 3, 3, 1, 2, "mainrom", 0xc000);

	msx2_pal(SND_YM2149, config, layout_msx_nocode_1fdd);
}

/* MSX2 - Philips NMS 8245/19 */

ROM_START(nms8245f)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("nms8245.u7", 0x0000, 0x20000, BAD_DUMP CRC(0c827d5f) SHA1(064e706cb1f12b99b329944ceeedc0efc3b2d9be))
ROM_END

void msx2_state::nms8245f(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "bios", 0, 0, 2, "maincpu", 0x10000);
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "ext", 3, 0, 0, 1, "maincpu", 0x18000);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0xf8);   // 128KB Mapper RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793, "disk", 3, 3, 1, 2, "maincpu", 0x1c000);

	msx2_pal(SND_YM2149, config, layout_msx_nocode_1fdd);
}

/* MSX2 - Philips NMS 8245 Home Banking (Italy) */

/* MSX2 - Philips NMS 8250/00 */

ROM_START(nms8250)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("d23c256eac.ic119", 0x0000, 0x8000, CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("d23128ec.ic118", 0x0000, 0x4000, CRC(66237ecf) SHA1(5c1f9c7fb655e43d38e5dd1fcc6b942b2ff68b02))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_SYSTEM_BIOS(0, "v1.08", "v1.08 diskrom")
	ROMX_LOAD("v1.08.ic117", 0x0000, 0x4000, CRC(61f6fcd3) SHA1(dab3e6f36843392665b71b04178aadd8762c6589), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "unknown", "Unknown version diskrom")
	ROMX_LOAD("jq00014.ic117", 0x0000, 0x4000, CRC(ca3307d3) SHA1(c3efedda7ab947a06d9345f7b8261076fa7ceeef), ROM_BIOS(1))
ROM_END

void msx2_state::nms8250(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	// ROM is mirrored in all 4 pages
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom1", 3, 0, 1, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom2", 3, 0, 2, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom3", 3, 0, 3, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0xf8);   // 128KB Mapper RAM
	// ROM is not mirrored but the FDC registers are in all pages
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793, "disk", 3, 3, 1, 2, "diskrom");

	msx2_pal(SND_YM2149, config, layout_msx_nocode_1fdd);
}

/* MSX2 - Philips NMS 8250/16 */

ROM_START(nms8250_16)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("bios.ic119", 0x0000, 0x8000, CRC(5e3caf18) SHA1(ee0d8ccfc247368078d27183c34b3a5c0f4ae0f1))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("sub.ic118", 0x0000, 0x4000, CRC(0a0aeb2f) SHA1(b83770cca8453a153d7e260070a3a3c059d64ed3))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("jq00014.ic117", 0x0000, 0x4000, CRC(ca3307d3) SHA1(c3efedda7ab947a06d9345f7b8261076fa7ceeef))
ROM_END

/* MSX2 - Philips NMS 8250/19 */

ROM_START(nms8250_19)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("nms8250fbios.ic119", 0x0000, 0x8000, CRC(bee21533) SHA1(d18694e9e7040b2851fe985cefb89766868a2fd3))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("nms8250fext.ic118",  0x0000, 0x4000, CRC(781ba055) SHA1(fd4bcc81a8160a1dea06036c5f79d200f948f4d6))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("nms8250fdisk.ic117", 0x0000, 0x4000, CRC(ca3307d3) SHA1(c3efedda7ab947a06d9345f7b8261076fa7ceeef))
ROM_END

/* MSX2 - Philips NMS 8255 */

ROM_START(nms8255)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("8255bios.rom.ic119", 0x0000, 0x8000, CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("8255ext.rom.ic118",  0x0000, 0x4000, CRC(66237ecf) SHA1(5c1f9c7fb655e43d38e5dd1fcc6b942b2ff68b02))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("8255disk.rom.ic117", 0x0000, 0x4000, CRC(ca3307d3) SHA1(c3efedda7ab947a06d9345f7b8261076fa7ceeef))
ROM_END

void msx2_state::nms8255(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793, 2 3.5" DSDD drives
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	// ROM is mirrored in all 4 pages
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom1", 3, 0, 1, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom2", 3, 0, 2, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom3", 3, 0, 3, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0xf8);   // 128KB Mapper RAM
	// ROM is not mirrored but the FDC registers are in all pages
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_2_DRIVES, "disk", 3, 3, 1, 2, "diskrom");

	msx2_pal(SND_YM2149, config, layout_msx_nocode_2fdd);
}

/* MSX2 - Philips NMS 8255/19 */

ROM_START(nms8255f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("nms8255fbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(5cd35ced) SHA1(b034764e6a8978db60b1d652917f5e24a66a7925)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("nms8255fext.rom",  0x0000, 0x4000, BAD_DUMP CRC(781ba055) SHA1(fd4bcc81a8160a1dea06036c5f79d200f948f4d6)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("nms8255fdisk.rom", 0x0000, 0x4000, BAD_DUMP CRC(13b60725) SHA1(58ba1887e8fd21c912b6859cae6514bd874ffcca)) // need verification
ROM_END

void msx2_state::nms8255f(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793, 2 3.5" DSDD drives
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0xf8);   // 128KB Mapper RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_2_DRIVES, "disk", 3, 3, 1, 2, "diskrom");

	msx2_pal(SND_YM2149, config, layout_msx_nocode_2fdd);
}

/* MSX2 - Philips NMS 8260 */
/* Prototype created by JVC for Philips. Based on an NMS-8250 with the floppy drive removed and replaced with a 20MB JVC harddisk */

ROM_START(nms8260)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("nms8260bios.rom", 0x0000, 0x8000, CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("nms8260ext.rom",  0x0000, 0x4000, CRC(66237ecf) SHA1(5c1f9c7fb655e43d38e5dd1fcc6b942b2ff68b02))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("nms8260disk.rom", 0x0000, 0x4000, CRC(ca3307d3) SHA1(c3efedda7ab947a06d9345f7b8261076fa7ceeef))

	ROM_REGION(0x4000, "hddrom", 0)
	ROM_LOAD("nms8260hdd.rom", 0x0000, 0x4000, CRC(0051afc3) SHA1(77f9fe964f6d8cb8c4af3b5fe63ce6591d5288e6))
ROM_END

void msx2_state::nms8260(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793, 0 3.5" DSDD drives
	// 2 Cartridge slots
	// S-3527 MSX Engine
	// HDD

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_ROM, "hddrom", 2, 1, 1, "hddrom");
	// ROM is mirrored in all 4 pages
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom1", 3, 0, 1, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom2", 3, 0, 2, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom3", 3, 0, 3, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0xf8);   // 128KB Mapper RAM
	// There is actually only an FDC inside with a floppy controller to attach an external floppy drive
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_0, "disk", 3, 3, 1, 2, "diskrom");

	// Not confirmed as there are no pictures of the keyboard
	msx2_pal(SND_YM2149, config, layout_msx_nocode);
}

/* MSX2 - Philips NMS 8280 - 5 possible sets (/00 /02 /09 /16 /19) */

ROM_START(nms8280)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("8280bios.rom.ic119", 0x0000, 0x8000, BAD_DUMP CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("8280ext.rom.ic118",  0x0000, 0x4000, BAD_DUMP CRC(66237ecf) SHA1(5c1f9c7fb655e43d38e5dd1fcc6b942b2ff68b02)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("8280disk.rom.ic117", 0x0000, 0x4000, BAD_DUMP CRC(ca3307d3) SHA1(c3efedda7ab947a06d9345f7b8261076fa7ceeef)) // need verification
ROM_END

void msx2_state::nms8280(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: wd2793, 2 3.5" DSDD drives
	// 2 Cartridge slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0xf8);   // 128KB Mapper RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_2_DRIVES, "disk", 3, 3, 1, 2, "diskrom");

	msx2_pal(SND_YM2149, config, layout_msx_nocode_2fdd);
}

/* MSX2 - Philips NMS 8280F */

ROM_START(nms8280f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("8280fbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(5cd35ced) SHA1(b034764e6a8978db60b1d652917f5e24a66a7925)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("8280fext.rom",  0x0000, 0x4000, BAD_DUMP CRC(781ba055) SHA1(fd4bcc81a8160a1dea06036c5f79d200f948f4d6)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("8280fdisk.rom", 0x0000, 0x4000, BAD_DUMP CRC(13b60725) SHA1(58ba1887e8fd21c912b6859cae6514bd874ffcca)) // need verification
ROM_END

void msx2_state::nms8280f(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: wd2793, 2 3.5" DSDD drives
	// 2 Cartridge slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0xf8);   // 128KB Mapper RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_2_DRIVES, "disk", 3, 3, 1, 2, "diskrom");

	msx2_pal(SND_YM2149, config, layout_msx_nocode_2fdd);
}

/* MSX2 - Philips NMS 8280G */

ROM_START(nms8280g)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("8280gbios.rom.ic119", 0x0000, 0x8000, BAD_DUMP CRC(8fa060e2) SHA1(b17d9bea0eb16a1aa2d0ccbd7c9488da9f57698e)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("8280gext.rom.ic118", 0x0000, 0x4000, BAD_DUMP CRC(41e36d03) SHA1(4ab7b2030d022f5486abaab22aaeaf8aa23e05f3)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("8280gdisk.rom.ic117", 0x0000, 0x4000, BAD_DUMP CRC(d0beebb8) SHA1(d1001f93c87ff7fb389e418e33bf7bc81bdbb65f)) // need verification
ROM_END

void msx2_state::nms8280g(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: wd2793, 2 3.5" DSDD drives
	// 2 Cartridge slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0xf8);   // 128KB Mapper RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_2_DRIVES, "disk", 3, 3, 1, 2, "diskrom");

	msx2_pal(SND_YM2149, config, layout_msx_nocode_2fdd);
}

/* MSX2 - Philips VG-8230 (u11 - exp, u12 - basic, u13 - disk */

ROM_START(vg8230)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("8230bios.rom.u12", 0x0000, 0x8000, CRC(b31c851d) SHA1(0de3c802057560560a03d7965fcc4cff69f8575c))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("8230ext.rom.u11",  0x0000, 0x4000, CRC(8f84f783) SHA1(3288894e1be6af705871499b23c85732dbc40993))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("8230disk.rom.u13", 0x0000, 0x4000, CRC(7639758a) SHA1(0f5798850d11b316a4254b222ca08cc4ad6d4da2))
ROM_END

void msx2_state::vg8230(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793, 1 3.5" SSDD drive
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);   // 64KB RAM
	add_internal_disk(config, MSX_SLOT_DISK1_WD2793, "disk", 3, 3, 1, 1, "diskrom");

	msx2_pal(SND_YM2149, config, layout_msx_nocode_1fdd);
}

/* MSX2 - Philips VG-8235 (/00 and /20) */
/* 9 versions:
 * /00 NL,BE QWERTY        2.0
 * /02 DE    QWERTZ        2.0
 * /16 ES    QWERTY with ñ 2.0
 * /19 FR,BE AZERTY        2.0
 * /20 NL,BE QWERTY        2.1
 * /22 DE    QWERTZ        2.1
 * /29 DE    QWERTZ        2.1
 * /36 ES    QWERTY with ñ 2.1
 * /39 FR,BE AZERTY        2.1
 */

ROM_START(vg8235)
	ROM_SYSTEM_BIOS(0, "r20", "VG8235/20")
	ROM_SYSTEM_BIOS(1, "r00", "VG8235/00")

	ROM_REGION(0x8000, "mainrom", 0)
	ROMX_LOAD("8235_20.u48", 0x0000, 0x8000, CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e), ROM_BIOS(0))
	ROMX_LOAD("8235_00.u48", 0x0000, 0x8000, CRC(f05ed518) SHA1(5e1a4bd6826b29302a1eb88c340477e7cbd0b50a), ROM_BIOS(1))

	ROM_REGION(0x4000, "subrom", 0)
	ROMX_LOAD("8235_20.u49", 0x0000, 0x4000, CRC(66237ecf) SHA1(5c1f9c7fb655e43d38e5dd1fcc6b942b2ff68b02), ROM_BIOS(0))
	ROMX_LOAD("8235_00.u49", 0x0000, 0x4000, CRC(474439d1) SHA1(c289dad246364e2dd716c457ca5eecf98e76c9ab), ROM_BIOS(1))

	ROM_REGION(0x4000, "diskrom", 0)
	// Different versions might exist and mixed between /00 and /20
	ROMX_LOAD("8235_20.u50", 0x0000, 0x4000, CRC(0efbea8a) SHA1(1ee2abc68a81ae7e39548985021b6532f31171b2), ROM_BIOS(0))
	ROMX_LOAD("8235_00.u50", 0x0000, 0x4000, CRC(f39342ce) SHA1(7ce255ab63ba79f81d8b83d66f1108062d0b61f1), ROM_BIOS(1))
ROM_END

void msx2_state::vg8235(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793, 1 3.5" SSDD drive
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0xf8);   // 128KB Mapper RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793, "disk", 3, 3, 1, 2, "diskrom");

	msx2_pal(SND_YM2149, config, layout_msx_nocode_1fdd);
}

/* MSX2 - Philips VG-8235F (/19 and /39) */

ROM_START(vg8235f)
	ROM_SYSTEM_BIOS(0, "r39", "VG8235/39")
	ROM_SYSTEM_BIOS(1, "r19", "VG8235/19")

	ROM_REGION(0x8000, "mainrom", 0)
	ROMX_LOAD("8235_39.u48", 0x0000, 0x8000, CRC(5cd35ced) SHA1(b034764e6a8978db60b1d652917f5e24a66a7925), ROM_BIOS(0))
	ROMX_LOAD("8235_19.u48", 0x0000, 0x8000, BAD_DUMP CRC(c0577a50) SHA1(3926cdd91fa89657a811463e48cfbdb350676e51), ROM_BIOS(1)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROMX_LOAD("8235_39.u49", 0x0000, 0x4000, CRC(781ba055) SHA1(fd4bcc81a8160a1dea06036c5f79d200f948f4d6), ROM_BIOS(0))
	ROMX_LOAD("8235_19.u49", 0x0000, 0x4000, BAD_DUMP CRC(e235d5c8) SHA1(792e6b2814ab783d06c7576c1e3ccd6a9bbac34a), ROM_BIOS(1)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROMX_LOAD("8235_39.u50", 0x0000, 0x4000, CRC(768549a9) SHA1(959060445e92eb980ddd9df3ef9cfefcae2de1d0), ROM_BIOS(0))
	ROMX_LOAD("8235_19.u50", 0x0000, 0x4000, BAD_DUMP CRC(768549a9) SHA1(959060445e92eb980ddd9df3ef9cfefcae2de1d0), ROM_BIOS(1)) // need verification
ROM_END

void msx2_state::vg8235f(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793, 1 3.5" SSDD drive
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0xf8);   // 128KB Mapper RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_SS, "disk", 3, 3, 1, 2, "diskrom");

	msx2_pal(SND_YM2149, config, layout_msx_nocode_1fdd);
}

/* MSX2 - Philips VG-8240 (unreleased) */

ROM_START(vg8240)
	ROM_REGION(0x10000, "mainrom", 0)
	ROM_LOAD("8240bios.rom", 0x0000, 0x8000, CRC(6cdaf3a5) SHA1(6103b39f1e38d1aa2d84b1c3219c44f1abb5436e))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("8240ext.rom",  0x0000, 0x4000, CRC(66237ecf) SHA1(5c1f9c7fb655e43d38e5dd1fcc6b942b2ff68b02))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("8240disk.rom", 0x0000, 0x4000, CRC(ca3307d3) SHA1(c3efedda7ab947a06d9345f7b8261076fa7ceeef))
ROM_END

void msx2_state::vg8240(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots?
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);   // 64KB RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793, "disk", 3, 3, 1, 2, "diskrom");

	msx2_pal(SND_YM2149, config, layout_msx_nocode_1fdd);
}

/* MSX2 - Phonola NMS 8245 */

/* MSX2 - Phonola NMS 8280 */

/* MSX2 - Phonola VG-8235 */

/* MSX2 - Pioneer UC-V102 */

ROM_START(ucv102)
	ROM_REGION(0x8000, "mainrom", 0)
	// Machine is not supposed to display the MSX logo on startup
	ROM_LOAD("uc-v102bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(a27c563d) SHA1(c1e46c00f1e38fc9e0ab487bf0513bd93ce61f3f))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("uc-v102sub.rom", 0x0000, 0x4000, BAD_DUMP CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))

	ROM_REGION(0x4000, "rs232", ROMREGION_ERASEFF)
	ROM_LOAD("uc-v102rs232.rom", 0x0000, 0x2000, CRC(7c6790fc) SHA1(a4f19371fd09b73f2776cb637b0e9cbd8415f8eb))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("uc-v102disk.rom", 0x0000, 0x4000, CRC(a90be8d5) SHA1(f7c3ac138918a493eb91628ed88cf37999059579))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("kanjifont.rom", 0x0000, 0x20000, CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967))
ROM_END

void msx2_state::ucv102(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: wd1793, 1 3.5" DSDD drives (could be upgraded to 2)
	// 1 Cartridge slots
	// S1985
	// RS232

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 0, 2, 0, 4).set_total_size(0x10000);   // 64KB Mapper RAM
	add_cartridge_slot<1>(config, 1);
	add_internal_slot_irq<2>(config, MSX_SLOT_RS232, "rs232", 2, 1, 1, "rs232");
	// Expansion slot 1 connects to slots 2-1 and 3-1 (2x 50 pin)
	// Expansion slot 2 connects to slots 2-2 and 3-2 (2x 50 pin)
	// Expansion slot 3 connects to slots 2-3 and 3-3 (2x 50 pin)
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_FD1793, "disk", 3, 1, 2, "diskrom").use_motor_for_led(); // Mitsubishi MSW1793

	MSX_S1985(config, "s1985", 0);

	m_hw_def.has_cassette(false);
	msx2(SND_YM2149, config, layout_msx_jp_1fdd);
}

/* MSX2 - Sakhr AX-350 */

ROM_START(ax350)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ax350bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(ea306155) SHA1(35195ab67c289a0b470883464df66bc6ea5b00d3)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("ax350ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(7c7540b7) SHA1(ebb76f9061e875365023523607db610f2eda1d26)) // need verification

	ROM_REGION(0x8000, "arabic", 0)
	ROM_LOAD("ax350arab.rom", 0x0000, 0x8000, BAD_DUMP CRC(c0d8fc85) SHA1(2c9600c6e0025fee10d249e97448ecaa37e38c42)) // need verification

	ROM_REGION(0x8000, "swp", 0)
	ROM_LOAD("ax350swp.rom", 0x0000, 0x8000, BAD_DUMP CRC(076f40fc) SHA1(4b4508131dca6d811694ae6379f41364c477de58)) // need verification

	ROM_REGION(0x10000, "painter", 0)
	ROM_LOAD("ax350paint.rom", 0x0000, 0x10000, BAD_DUMP CRC(18956e3a) SHA1(ace202e87337fbc54fea21e22c0b3af0abe6f4ae)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("ax350disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(1e7d6512) SHA1(78cd7f847e77fd8cd51a647efb2725ba93f4c471)) // need verification
ROM_END

void msx2_state::ax350(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: wd2793/tc8566af?, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S1985

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "arabic", 0, 1, 1, 2, "arabic");
	add_internal_slot(config, MSX_SLOT_ROM, "swp", 0, 2, 1, 2, "swp");
	add_internal_slot(config, MSX_SLOT_ROM, "painter", 0, 3, 0, 4, "painter");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_disk_mirrored(config, MSX_SLOT_DISK2_WD2793, "disk", 3, 1, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0xf8);   // 128KB Mapper RAM

	MSX_S1985(config, "s1985", 0);

	msx2_pal(SND_YM2149, config, layout_msx_ar_1fdd);
}

/* MSX2 - Sakhr AX-350 II */

ROM_START(ax350ii)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ax350iibios.rom", 0x0000, 0x8000, CRC(ea306155) SHA1(35195ab67c289a0b470883464df66bc6ea5b00d3))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("ax350iiext.rom", 0x0000, 0x4000, CRC(7c7540b7) SHA1(ebb76f9061e875365023523607db610f2eda1d26))

	ROM_REGION(0x8000, "arabic", 0)
	ROM_LOAD("ax350iiarab.rom", 0x0000, 0x8000, CRC(e62f9bc7) SHA1(f8cd4c05083decfc098cff077e055a4ae1e91a73))

	ROM_REGION(0x8000, "swp", 0)
	ROM_LOAD("ax350iiword.rom", 0x0000, 0x8000, CRC(307ae37c) SHA1(3a74e73b94d066b0187feb743c5eceddf0c61c2b))

	ROM_REGION(0x10000, "painter", 0)
	ROM_LOAD("ax350iipaint.rom", 0x0000, 0x10000, CRC(18956e3a) SHA1(ace202e87337fbc54fea21e22c0b3af0abe6f4ae))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("ax350iidisk.rom", 0x0000, 0x4000, CRC(d07782a6) SHA1(358e69f427390041b5aa28018550a88f996bddb6))
ROM_END

void msx2_state::ax350ii(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: wd2793/tc8566af?, 1 3.5" DSDD drive (mb8877a in pcb picture)
	// 2 Cartridge slots
	// S1985

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "arabic", 0, 1, 1, 2, "arabic");
	add_internal_slot(config, MSX_SLOT_ROM, "swp", 0, 2, 1, 2, "swp");
	add_internal_slot(config, MSX_SLOT_ROM, "painter", 0, 3, 0, 4, "painter");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	// mirroring not confirmed
	add_internal_disk_mirrored(config, MSX_SLOT_DISK8_MB8877, "disk", 3, 1, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0xf8);   // 128KB Mapper RAM

	MSX_S1985(config, "s1985", 0);

	msx2_pal(SND_YM2149, config, layout_msx_ar_1fdd);
}

/* MSX2 - Sakhr AX-350 II F */

ROM_START(ax350iif)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ax350iifbios.rom", 0x0000, 0x8000, CRC(5cd35ced) SHA1(b034764e6a8978db60b1d652917f5e24a66a7925))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("ax350iifext.rom", 0x0000, 0x4000, CRC(16afa2e9) SHA1(4cbceba8f37f08272b612b6fc212eeaf379da9c3))

	ROM_REGION(0x8000, "arabic", 0)
	ROM_LOAD("ax350iifarab.rom", 0x0000, 0x8000, CRC(a64c3192) SHA1(5077b9c86ce1dc0a22c71782dac7fb3ca2a467e0))

	ROM_REGION(0x8000, "swp", 0)
	ROM_LOAD("ax350iifword.rom", 0x0000, 0x8000, CRC(097fd8ca) SHA1(54ff13b58868018fcd43c916b8d7c7200ebdcabe))

	ROM_REGION(0x10000, "painter", 0)
	ROM_LOAD("ax350iifpaint.rom", 0x0000, 0x10000, CRC(18956e3a) SHA1(ace202e87337fbc54fea21e22c0b3af0abe6f4ae))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("ax350iifdisk.rom", 0x0000, 0x4000, CRC(5eb46cd5) SHA1(bd0ad648d728c691fcee08eaaaa95e15e29c0d0d))
ROM_END

void msx2_state::ax350iif(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: wd2793/tc8566af?, 1 3.5" DSDD drive (mb8877a in pcb picture)
	// 2 Cartridge slots
	// S1985

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "arabic", 0, 1, 1, 2, "arabic");
	add_internal_slot(config, MSX_SLOT_ROM, "swp", 0, 2, 1, 2, "swp");
	add_internal_slot(config, MSX_SLOT_ROM, "painter", 0, 3, 0, 4, "painter");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	// mirroring not confirmed
	add_internal_disk_mirrored(config, MSX_SLOT_DISK8_MB8877, "disk", 3, 1, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0xf8);   // 128KB Mapper RAM

	MSX_S1985(config, "s1985", 0);

	msx2_pal(SND_YM2149, config, layout_msx_ar_1fdd);
}

/* MSX2 - Sakhr AX-370 */

ROM_START(ax370)
	ROM_REGION(0x30000, "mainrom", 0)
	ROM_LOAD("ax370bios.rom", 0x0000, 0x8000, CRC(ea306155) SHA1(35195ab67c289a0b470883464df66bc6ea5b00d3))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("ax370ext.rom", 0x0000, 0x4000, CRC(3c011d12) SHA1(ee9c6a073766bef2220a57372f5c0dbfc6e55c8c))

	ROM_REGION(0x8000, "arabic", 0)
	ROM_LOAD("ax370arab.rom", 0x0000, 0x8000, CRC(66c2c71e) SHA1(0c08e799a7cf130ae2b9bc93f28bd4959cee6fdc))

	ROM_REGION(0x8000, "swp", 0)
	ROM_LOAD("ax370swp.rom", 0x0000, 0x8000, CRC(076f40fc) SHA1(4b4508131dca6d811694ae6379f41364c477de58))

	ROM_REGION(0x8000, "sakhr", 0)
	ROM_LOAD("ax370sakhr.rom", 0x0000, 0x8000, CRC(71a356ce) SHA1(8167117a003824220c36775682acbb36b3733c5e))

	ROM_REGION(0x10000, "painter", 0)
	ROM_LOAD("ax370paint.rom", 0x00000, 0x10000, CRC(0394cb41) SHA1(1c9a5867d39f6f02a0a4ef291904623e2521c2c5))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("ax370disk.rom", 0x0000, 0x4000, CRC(db7f1125) SHA1(9efa744be8355675e7bfdd3976bbbfaf85d62e1d))
ROM_END

void msx2_state::ax370(machine_config &config)
{
	// AY8910 (in T9769B)
	// FDC: tc8566af, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// V9958
	// T9769B

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "arabic", 0, 1, 1, 2, "arabic");
	add_internal_slot(config, MSX_SLOT_ROM, "swp", 0, 2, 1, 2, "swp");
	add_internal_slot(config, MSX_SLOT_ROM, "sakhr", 0, 3, 1, 2, "sakhr");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK3_TC8566, "disk", 3, 0, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_ROM, "painter", 3, 1, 0, 4, "painter");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 3, 0, 4).set_total_size(0x20000).set_unused_bits(0xf8);   // 128KB Mapper RAM

	msx2_v9958_base(SND_AY8910, config, layout_msx_ar_1fdd);
	m_v9958->set_screen_pal(m_screen);
	msx2_add_softlists(config);
}

/* MSX2 - Sakhr AX-500 */

ROM_START(ax500)
	ROM_REGION(0x30000, "mainrom", 0)
	ROM_LOAD("ax500bios.rom", 0x0000, 0x8000, CRC(0a6e2e13) SHA1(dd1b577ea3ea69de84a68d311261392881f9eac3))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("ax500ext.rom", 0x0000, 0x4000, CRC(c9186a21) SHA1(7f86af13e81259a0db8f70d8a7e026fb918ee652))

	ROM_REGION(0x8000, "arabic", 0)
	ROM_LOAD("ax500arab.rom", 0x0000, 0x8000, CRC(11830686) SHA1(92bac0b2995f54f0eebf167cd447361a6a4923eb))

	ROM_REGION(0x8000, "swp", 0)
	ROM_LOAD("ax500swp.rom", 0x0000, 0x8000, CRC(17ed0610) SHA1(8674d000a52ec01fd80c8cb7cbaa66d4c3ca5cf7))

	ROM_REGION(0xc000, "sakhr", 0)
	ROM_LOAD("ax500sakhr.rom", 0x0000, 0xc000, CRC(bee11490) SHA1(8e889999ecec302f05d3bd0a0f127b489fcf3739))

	ROM_REGION(0x10000, "painter", 0)
	ROM_LOAD("ax500paint.rom", 0x00000, 0x10000, CRC(c32ea7ec) SHA1(80872d997d18e1a633e70b9da35a0d28113073e5))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("ax500disk.rom", 0x0000, 0x4000, CRC(a7d7746e) SHA1(a953bef071b603d6280bdf7ab6249c2e6f1a4cd8))
ROM_END

void msx2_state::ax500(machine_config &config)
{
	// YM2149 (in S9185)
	// FDC: wd2793?, 2 3.5" DSDD drives
	// 2 Cartridge slots
	// S1985

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "arabic", 0, 1, 1, 2, "arabic");
	add_internal_slot(config, MSX_SLOT_ROM, "swp", 0, 2, 1, 2, "swp");
	add_internal_slot(config, MSX_SLOT_ROM, "sakhr", 0, 3, 1, 2, "sakhr");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK8_WD2793_2_DRIVES, "disk", 3, 0, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_ROM, "painter", 3, 1, 0, 4, "painter");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x40000).set_unused_bits(0xf0);   // 256KB Mapper RAM
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, 3, msx_yamaha_60pin, nullptr);

	MSX_S1985(config, "s1985", 0);

	msx2_pal(SND_YM2149, config, layout_msx_ar_2fdd);
}

/* MSX2 - Sanyo MPC-2300 */

ROM_START(mpc2300)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("2300bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(e7d08e29) SHA1(0f851ee7a1cf79819f61cc89e9948ee72a413802)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("2300ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(3d7dc718) SHA1(e1f834b28c3ee7c9f79fe6fbf2b23c8a0617892b)) // need verification
ROM_END

void msx2_state::mpc2300(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: None, 0 drives
	// 2 Cartridge slots?
	// S1985

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x20000);   // 128KB?? Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");

	MSX_S1985(config, "s1985", 0);

	msx2(SND_YM2149, config, layout_msx_ru);
}

/* MSX2 - Sanyo MPC-2500FD */

ROM_START(mpc2500f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mpc2500fdbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(e7d08e29) SHA1(0f851ee7a1cf79819f61cc89e9948ee72a413802)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("mpc2500fdext.rom",  0x0000, 0x4000, BAD_DUMP CRC(3d7dc718) SHA1(e1f834b28c3ee7c9f79fe6fbf2b23c8a0617892b)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("mpc2500fddisk.rom", 0x0000, 0x4000, BAD_DUMP CRC(38454059) SHA1(58ac78bba29a06645ca8d6a94ef2ac68b743ad32)) // need verification
ROM_END

void msx2_state::mpc2500f(machine_config &config)
{
	// YM2149
	// FDC: wd2793?, 1? 3.5" DSDD drive?
	// 2 Cartridge slots?
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 2, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793, "disk", 3, 1, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x10000);   // 64KB?? Mapper RAM

	msx2(SND_YM2149, config, layout_msx_ru_1fdd);
}

/* MSX2 - Sanyo MPC-25F */

/* MSX2 - Sanyo MPC-25FD */

ROM_START(mpc25fd)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("25fdbios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("25fdext.rom",  0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("25fddisk.rom", 0x0000, 0x4000, CRC(1a91f241) SHA1(bdbc75aacba4ea0f359694f304ae436914733460))
ROM_END

void msx2_state::mpc25fd(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793, 1 drive
	// 1 Cartridge slot (slot 1)
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 2, 3, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom_mirror1", 2, 3, 1, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom_mirror2", 2, 3, 2, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom_mirror3", 2, 3, 3, 1, "subrom");
	// Mirrored in all 4 pages (only rom or also registers?)
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793, "disk", 3, 1, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);   // 64KB RAM

	msx2(SND_YM2149, config, layout_msx_jp_1fdd);
}

/* MSX2 - Sanyo MPC-25FK */

/* MSX2 - Sanyo MPC-25FS */

ROM_START(mpc25fs)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("25fdbios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("25fdext.rom",  0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("25fddisk.rom", 0x0000, 0x4000, CRC(0fa7b10e) SHA1(50f4098a77e7af7093e29cc8683d2b34b2d07b13))
ROM_END

void msx2_state::mpc25fs(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793, 1 drive
	// 1 Cartridge slot (slot 1)
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 2, 3, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom_mirror1", 2, 3, 1, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom_mirror2", 2, 3, 2, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom_mirror3", 2, 3, 3, 1, "subrom");
	// Mirrored in all 4 pages (only rom or also registers?)
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_SS, "disk", 3, 1, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);   // 64KB RAM

	msx2(SND_YM2149, config, layout_msx_jp_1fdd);
}

/* MSX2 - Sanyo Wavy MPC-27 */

ROM_START(mpc27)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mpc27bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(ba81b3dd) SHA1(4ce41fcc1a603411ec4e99556409c442078f0ecf)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("mpc27ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(90ca25b5) SHA1(fd9fa78bac25aa3c0792425b21d14e364cf7eea4)) // need verificaiton

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("mpc27disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(38454059) SHA1(58ac78bba29a06645ca8d6a94ef2ac68b743ad32)) // need verification

	ROM_REGION(0x4000, "lpen", 0)
	ROM_LOAD("mlp27.rom", 0x0000, 0x2000, BAD_DUMP CRC(8f9e6ba0) SHA1(c3a47480c9dd2235f40f9a53dab68e3c48adca01)) // need verification
	ROM_RELOAD(0x2000, 0x2000)
ROM_END

void msx2_state::mpc27(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: wd2793?, 1 drive
	// 2 Cartridge slots?
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x20000);   // 128KB?? RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_SS, "disk", 3, 2, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_ROM, "lpen", 3, 3, 1, 1, "lpen");

	msx2(SND_YM2149, config, layout_msx_jp_1fdd);
}

/* MSX2 - Sanyo PCT-100 */

/* MSX2 - Sanyo PHC-23 - "Wavy23" (and PHC-23J with different keyboard layout) */

ROM_START(phc23)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("23bios.rom", 0x0000, 0x8000, CRC(ba81b3dd) SHA1(4ce41fcc1a603411ec4e99556409c442078f0ecf))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("23ext.rom", 0x0000, 0x4000, CRC(90ca25b5) SHA1(fd9fa78bac25aa3c0792425b21d14e364cf7eea4))
ROM_END

void msx2_state::phc23(machine_config &config)
{
	// YM2149 (in S3527 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM

	msx2(SND_YM2149, config, layout_msx_jp);
}

/* MSX2 - Sanyo PHC-23J(B) / PHC-23J(GR) - "Wavy23" */

ROM_START(phc23jb)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("23bios.rom", 0x0000, 0x8000, CRC(ba81b3dd) SHA1(4ce41fcc1a603411ec4e99556409c442078f0ecf))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("23ext.rom", 0x0000, 0x4000, CRC(90ca25b5) SHA1(fd9fa78bac25aa3c0792425b21d14e364cf7eea4))
ROM_END

void msx2_state::phc23jb(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S-1985 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM

	MSX_S1985(config, "s1985", 0);
	msx2(SND_YM2149, config, layout_msx_jp);
}

/* MSX2 - Sanyo Wavy PHC-55FD2 */

ROM_START(phc55fd2)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("phc55fd2bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(ba81b3dd) SHA1(4ce41fcc1a603411ec4e99556409c442078f0ecf)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("phc55fd2ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(90ca25b5) SHA1(fd9fa78bac25aa3c0792425b21d14e364cf7eea4)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("phc55fd2disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(38454059) SHA1(58ac78bba29a06645ca8d6a94ef2ac68b743ad32)) // need verification
ROM_END

void msx2_state::phc55fd2(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: wd2793?, 2 3.5" DSDD drives
	// 2 Cartridge slots
	// S-1985 MSX Engine
	// T9763

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x20000);   // 128KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_2_DRIVES, "disk", 3, 2, 1, 2, "diskrom");

	MSX_S1985(config, "s1985", 0);

	msx2(SND_YM2149, config, layout_msx_jp_2fdd);
}

/* MSX2 - Sanyo Wavy PHC-77 */

ROM_START(phc77)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("phc77bios.rom", 0x0000, 0x8000, CRC(ba81b3dd) SHA1(4ce41fcc1a603411ec4e99556409c442078f0ecf))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("phc77ext.rom", 0x0000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("phc77disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(7c79759a) SHA1(a427b0c9a344c87b587568ecca7fee0abbe72189)) // Floppy registers visible in dump

	ROM_REGION(0x80000, "msxwrite", 0)
	ROM_LOAD("phc77msxwrite.rom", 0x00000, 0x80000, CRC(ef02e4f3) SHA1(4180544158a57c99162269e33e4f2c77c9fce84e))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("phc77kfn.rom", 0x0000, 0x20000, CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967))
ROM_END

void msx2_state::phc77(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: wd2793?, 1 drive; looks like mb8877a
	// 1 Cartridge slot
	// S-1985 MSX Engine
	// SRAM?
	// Builtin printer
	// Switch to turn off firmware

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_MSX_WRITE, "msxwrite", 2, 1, 2, "msxwrite");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_disk(config, MSX_SLOT_DISK9_WD2793_N, "disk", 3, 0, 1, 1, "diskrom");
	add_cartridge_slot<2>(config, 3, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 3, 0, 4);   // 64KB RAM

	MSX_S1985(config, "s1985", 0);

	msx2(SND_YM2149, config, layout_msx_jp_1fdd);
}

/* MSX2 - Sharp Epcom HotBit 2.0 - is this an officially released machine? */

ROM_START(hotbit20)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hb2bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(0160e8c9) SHA1(d0cfc35f22b150a1cb10decae4841dfe63b78251)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("hb2ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(08ced880) SHA1(4f2a7e0172f0214f025f23845f6e053d0ffd28e8)) // need verification

	ROM_REGION(0x4000, "xbasic", 0)
	ROM_LOAD("xbasic2.rom", 0x0000, 0x4000, BAD_DUMP CRC(2825b1a0) SHA1(47370bec7ca1f0615a54eda548b07fbc0c7ef398)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("microsoldisk.rom", 0x0000, 0x4000, BAD_DUMP CRC(6704ef81) SHA1(a3028515ed829e900cc8deb403e17b09a38bf9b0)) // need verification
ROM_END

void msx2_state::hotbit20(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: microsol, 1 or 2 drives?
	// 2 Cartridge slots?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1, 0);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 1, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "xbasic", 1, 1, 1, 1, "xbasic");
	add_internal_disk(config, MSX_SLOT_DISK5_WD2793, "disk", 1, 3, 1, 1, "diskrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 2, 0, 4).set_total_size(0x20000).set_unused_bits(0x80);   // 128KB Mapper RAM
	add_cartridge_slot<2>(config, 3);

	msx2_pal(SND_AY8910, config, layout_msx_nocode_1fdd);
}

/* MSX2 - Sony HB-F1 */

ROM_START(hbf1)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("f1bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("f1ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "firmware1", 0)
	ROM_LOAD("f1note1.rom", 0x0000, 0x4000, CRC(84810ea8) SHA1(9db72bb78792595a12499c821048504dc96ef848))

	ROM_REGION(0x8000, "firmware2", 0)
	ROM_LOAD("f1note2.rom", 0x0000, 0x8000, CRC(e32e5ee0) SHA1(aa78fc9bcd2343f84cf790310a768ee47f90c841))

	ROM_REGION(0x8000, "firmware3", 0)
	ROM_LOAD("f1note3.rom", 0x0000, 0x8000, CRC(73eb9329) SHA1(58accf41a90693874b86ce98d8d43c27beb8b6dc))
ROM_END

void msx2_state::hbf1(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S1985
	// pause button

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware1", 3, 0, 1, 1, "firmware1");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware2", 3, 1, 1, 2, "firmware2");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware3", 3, 2, 1, 2, "firmware3");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 3, 0, 4);  // 64KB RAM

	MSX_S1985(config, "s1985", 0);

	msx2(SND_YM2149, config, layout_msx_nocode_nocaps);
}

/* MSX2 - Sony HB-F1II */

ROM_START(hbf1ii)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("f12bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("f12ext.rom", 0x0000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))

	ROM_REGION(0x4000, "firmware1", 0)
	ROM_LOAD("f12note1.rom", 0x0000, 0x4000, CRC(dcacf970) SHA1(30d914cda2180889a40a3328e0a0c1327f4eaa10))

	ROM_REGION(0x8000, "firmware2", 0)
	ROM_LOAD("f12note2.rom", 0x0000, 0x8000, CRC(b0241a61) SHA1(ed2fea5c2a3c2e58d4f69f9d636e08574486a2b1))

	ROM_REGION(0x8000, "firmware3", 0)
	ROM_LOAD("f12note3.rom", 0x0000, 0x8000, CRC(44a10e6a) SHA1(917d1c079e03c4a44de864f123d03c4e32c8daae))
ROM_END

void msx2_state::hbf1ii(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S1985
	// rensha-turbo slider

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware1", 3, 0, 1, 1, "firmware1");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware2", 3, 1, 1, 2, "firmware2");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware3", 3, 2, 1, 2, "firmware3");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 3, 0, 4);   // 64KB RAM

	MSX_S1985(config, "s1985", 0);

	msx2(SND_YM2149, config, layout_msx_jp);
}

/* MSX2 - Sony HB-F1XD  / HB-F1XDmk2 */
/* HB-F1XDmk2 is a cost-reduced version of HB-F1XD but identical in emulation */

ROM_START(hbf1xd)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("f1xdbios.rom.ic27", 0x0000, 0x8000, CRC(ba81b3dd) SHA1(4ce41fcc1a603411ec4e99556409c442078f0ecf))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("f1xdext.rom.ic27", 0x0000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("f1xddisk.rom.ic27", 0x0000, 0x4000, CRC(54c73ad6) SHA1(12f2cc79b3d09723840bae774be48c0d721ec1c6))
ROM_END

void msx2_state::hbf1xd(machine_config &config)
{
	// YM2149 (in S-1895 MSX Engine)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S-1985 MSX Engine
	// pause button
	// speed controller slider
	// rensha turbo slider

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_N, "disk", 3, 0, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 3, 0, 4);   // 64KB RAM

	MSX_S1985(config, "s1985", 0);

	msx2(SND_YM2149, config, layout_msx_jp_1fdd);
}

/* MSX2 - Sony HB-F5 */

ROM_START(hbf5)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hbf5bios.ic25", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("hbf5ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("hbf5note.rom", 0x0000, 0x4000, CRC(0cdc0777) SHA1(06ba91d6732ee8a2ecd5dcc38b0ce42403d86708))
ROM_END

void msx2_state::hbf5(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 1, 1, 1, "firmware");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 0, 4);   // 64KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	msx2_pal(SND_YM2149, config, layout_msx_jp);
}

/* MSX2 - Sony HB-F500 */

ROM_START(hbf500)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("f500bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("f500ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("f500disk.rom", 0x0000, 0x4000, CRC(f7f5b0ea) SHA1(e93b8da1e8dddbb3742292b0e5e58731b90e9313))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("f500kfn.rom", 0, 0x20000, CRC(5a59926e) SHA1(6acaf2eeb57f65f7408235d5e07b7563229de799))
ROM_END

void msx2_state::hbf500(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 0, 2, 2);   // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793, "disk", 0, 1, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 0, 2, 0, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	msx2(SND_YM2149, config, layout_msx_jp_1fdd);
}

/* MSX2 - Sony HB-F500 2nd version (slot layout is different) */

ROM_START(hbf500_2)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("f500bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("f500ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("f500disk.rom", 0x0000, 0x4000, CRC(f7f5b0ea) SHA1(e93b8da1e8dddbb3742292b0e5e58731b90e9313))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("f500kfn.rom", 0, 0x20000, CRC(5a59926e) SHA1(6acaf2eeb57f65f7408235d5e07b7563229de799))
ROM_END

void msx2_state::hbf500_2(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793, "disk", 3, 1, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);   // 64KB RAM

	msx2(SND_YM2149, config, layout_msx_jp_1fdd);
}

/* MSX2 - Sony HB-F500F */

ROM_START(hbf500f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hbf500fbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(440dae3c) SHA1(fedd9b682d056ddd1e9b3d281723e12f859b2e69)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("hbf500fext.rom", 0x0000, 0x4000, BAD_DUMP CRC(e235d5c8) SHA1(792e6b2814ab783d06c7576c1e3ccd6a9bbac34a)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("hbf500fdisk.rom", 0x0000, 0x4000, BAD_DUMP CRC(6e718f5c) SHA1(0e081572f84555dc13bdb0c7044a19d6c164d985)) // need verification
ROM_END

void msx2_state::hbf500f(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 3 Cartridge slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 0, 2, 2);   // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_N, "disk", 0, 1, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 0, 2, 0, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_cartridge_slot<3>(config, 3);

	msx2_pal(SND_YM2149, config, layout_msx_nocode_1fdd);
}

/* MSX2 - Sony HB-F500P */

ROM_START(hbf500p)
	ROM_REGION(0x10000, "mainrom", 0)
	ROM_LOAD("500pbios.rom.ic41", 0x0000, 0x8000, CRC(b31c851d) SHA1(0de3c802057560560a03d7965fcc4cff69f8575c))
	// FDC register contents at 7ff8-7fff
	ROM_LOAD("500pext.ic47",      0x8000, 0x8000, BAD_DUMP CRC(cdd4824a) SHA1(505031f1e8396a6e0cb11c1540e6e7f6999d1191))
ROM_END

void msx2_state::hbf500p(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 3 Cartridge slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 0, 2, 2);   // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "mainrom", 0x8000);
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_N, "disk", 0, 1, 1, 2, "mainrom", 0xc000);
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 0, 2, 0, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_cartridge_slot<3>(config, 3);

	msx2_pal(SND_YM2149, config, layout_msx_nocode_1fdd);
}

/* MSX2 - Sony HB-F700D */

ROM_START(hbf700d)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("700dbios.rom.ic5", 0x0000, 0x8000, CRC(e975aa79) SHA1(cef16eb95502ba6ab2265fcafcedde470a101541))

	ROM_REGION(0x8000, "extrom", 0)
	// dumps as listed in openMSX and blueMSX
	//  ROM_LOAD("700dsub.ic6", 0x0000, 0x4000, CRC(8f84f783) SHA1(3288894e1be6af705871499b23c85732dbc40993))
	//  ROM_LOAD("700ddisk.ic6", 0x4000, 0x4000, CRC(54c73ad6) SHA1(12f2cc79b3d09723840bae774be48c0d721ec1c6))
	//
	// however according to the service manual these should be in the same rom chip
	// concatenation of 3288894e1be6af705871499b23c85732dbc40993 and 12f2cc79b3d09723840bae774be48c0d721ec1c6
	ROM_LOAD("700dext.ic6", 0x0000, 0x8000, BAD_DUMP CRC(2aba42dc) SHA1(9dee68aab6c921b0b20862a3f2f4e38ff8d155c0)) // to be verified with direct dump
ROM_END

void msx2_state::hbf700d(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S-1985 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "extrom", 3, 0, 0, 1, "extrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_N, "disk", 3, 0, 1, 2, "extrom", 0x4000);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 3, 0, 4).set_total_size(0x40000).set_unused_bits(0x80);   // 256KB Mapper RAM

	MSX_S1985(config, "s1985", 0);

	msx2_pal(SND_YM2149, config, layout_msx_nocode_1fdd);
}

/* MSX2 - Sony HB-F700F */

ROM_START(hbf700f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("700fbios.ic5", 0x0000, 0x8000, CRC(440dae3c) SHA1(fedd9b682d056ddd1e9b3d281723e12f859b2e69))

	ROM_REGION(0x8000, "extrom", 0)
	// dumps as listed in openMSX and blueMSX
	//  ROM_LOAD("700fsub.ic6", 0x0000, 0x4000, CRC(e235d5c8) SHA1(792e6b2814ab783d06c7576c1e3ccd6a9bbac34a))
	//  ROM_LOAD("700fdisk.ic6", 0x4000, 0x4000, CRC(54c73ad6) SHA1(12f2cc79b3d09723840bae774be48c0d721ec1c6))
	//
	// however according to the service manual these should be in the same rom chip
	// concatenation of 792e6b2814ab783d06c7576c1e3ccd6a9bbac34a and 12f2cc79b3d09723840bae774be48c0d721ec1c6
	ROM_LOAD("700fext.ic6",  0x0000, 0x8000, BAD_DUMP CRC(463db23b) SHA1(2ab5be13b356692e75a5d76a23f8e4cfc094b3df)) // to be verified with direct dump
ROM_END

void msx2_state::hbf700f(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S1985

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "extrom", 3, 0, 0, 1, "extrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_N, "disk", 3, 0, 1, 2, "extrom", 0x4000);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 3, 0, 4).set_total_size(0x40000).set_unused_bits(0x80);   // 256KB Mapper RAM

	MSX_S1985(config, "s1985", 0);

	msx2_pal(SND_YM2149, config, layout_msx_nocode_1fdd);
}

/* MSX2 - Sony HB-F700P */

ROM_START(hbf700p)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("700pbios.rom.ic5", 0x0000, 0x8000, CRC(b31c851d) SHA1(0de3c802057560560a03d7965fcc4cff69f8575c))

	ROM_REGION(0x8000, "extrom", 0)
	// dumps as listed in openMSX / blueMSX
	// openMSX also lists 24624c5fa3a8069b1d865cdea8a029f15c1955ea for the subrom but the disk rom
	// part of that machine is 'certainly not original' so this may also not be original.
	//  ROM_LOAD("700psub.ic6", 0x0000, 0x4000, CRC(8f84f783) SHA1(3288894e1be6af705871499b23c85732dbc40993))
	//  ROM_LOAD("700pdisk.ic6", 0x4000, 0x4000, CRC(1d9cc7f6) SHA1(3376cf9dd2b1ac9b41bf6bf6598b33136e86f9d5))
	//
	// however according to the service manual these should be in the same rom chip
	// concatenation of 3288894e1be6af705871499b23c85732dbc40993 and 3376cf9dd2b1ac9b41bf6bf6598b33136e86f9d5
	ROM_LOAD("700pext.ic6", 0x0000, 0x8000, BAD_DUMP CRC(63e1bffc) SHA1(496698a60432490dc1306c8cc1d4a6ded275261a)) // to be verified with direct dump
ROM_END

void msx2_state::hbf700p(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S-1985 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "extrom", 3, 0, 0, 1, "extrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793, "disk", 3, 0, 1, 2, "extrom", 0x4000);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 3, 0, 4).set_total_size(0x40000).set_unused_bits(0x80);   // 256KB Mapper RAM

	MSX_S1985(config, "s1985", 0);

	msx2_pal(SND_YM2149, config, layout_msx_nocode_1fdd);
}

/* MSX2 - Sony HB-F700S */

ROM_START(hbf700s)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("700sbios.rom.ic5", 0x0000, 0x8000, CRC(c2b889a5) SHA1(4811956f878c3e03da46317f787cdc4bebc86f47))

	ROM_REGION(0x8000, "extrom", 0)
	// dumps as listed in openMSX / blueMSX
	//  ROM_LOAD("700ssub.ic6", 0x0000, 0x4000, CRC(dc0951bd) SHA1(1e9a955943aeea9b1807ddf1250ba6436d8dd276))
	//  ROM_LOAD("700sdisk.ic6", 0x4000, 0x4000, CRC(1d9cc7f6) SHA1(3376cf9dd2b1ac9b41bf6bf6598b33136e86f9d5))
	//
	// however according to the service manual these should be in the same rom chip
	// concatenation of 1e9a955943aeea9b1807ddf1250ba6436d8dd276 and 3376cf9dd2b1ac9b41bf6bf6598b33136e86f9d5
	ROM_LOAD("700sext.ic6", 0x0000, 0x8000, BAD_DUMP CRC(28d1badf) SHA1(ae3ed88a2d7034178e08f7bdf5409f462bf67fc9)) // to be verified with direct dump
ROM_END

void msx2_state::hbf700s(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S1985

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "extrom", 3, 0, 0, 1, "extrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793, "disk", 3, 0, 1, 2, "extrom", 0x4000);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 3, 0, 4).set_total_size(0x40000).set_unused_bits(0x80);   // 256KB Mapper RAM

	MSX_S1985(config, "s1985", 0);

	msx2_pal(SND_YM2149, config, layout_msx_nocode_1fdd);
}

/* MSX2 - Sony HB-F750 (prototype) */

/* MSX2 - Sony HB-F900 */

ROM_START(hbf900)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("f900bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("f900ext.rom", 0x0000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("f900disk.rom", 0x0000, 0x4000, CRC(f83d0ea6) SHA1(fc760d1d7b16370abc7eea39955f230b95b37df6))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("f900util.rom", 0x0000, 0x4000, CRC(bc6c7c66) SHA1(558b7383544542cf7333700ff90c3efbf93ba2a3))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("f900kfn.rom", 0, 0x20000, CRC(5a59926e) SHA1(6acaf2eeb57f65f7408235d5e07b7563229de799))
ROM_END

void msx2_state::hbf900(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: wd2793, 2 3.5" DSDD drives
	// 2 Cartridge slots
	// S1985

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 1, 0, 4).set_total_size(0x40000).set_unused_bits(0x80);   // 256KB Mapper RAM
	add_internal_disk(config, MSX_SLOT_DISK1_WD2793_2_DRIVES, "disk", 3, 2, 1, 1, "diskrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 3, 1, 1, "firmware");

	MSX_S1985(config, "s1985", 0);

	msx2(SND_YM2149, config, layout_msx_jp_2fdd);
}

/* MSX2 - Sony HB-F900 (a) */

ROM_START(hbf900a)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("f900bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("f900ext.rom", 0x0000, 0x4000, CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("f900disa.rom", 0x0000, 0x4000, CRC(54c73ad6) SHA1(12f2cc79b3d09723840bae774be48c0d721ec1c6))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("f900util.rom", 0x0000, 0x4000, CRC(bc6c7c66) SHA1(558b7383544542cf7333700ff90c3efbf93ba2a3))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("f900kfn.rom", 0, 0x20000, CRC(5a59926e) SHA1(6acaf2eeb57f65f7408235d5e07b7563229de799))
ROM_END

void msx2_state::hbf900a(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: wd2793, 2 3.5" DSDD drives
	// 2 Cartridge slots
	// S1985

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 1, 0, 4).set_total_size(0x40000).set_unused_bits(0x80);   // 256KB Mapper RAM
	add_internal_disk(config, MSX_SLOT_DISK1_WD2793_N_2_DRIVES, "disk", 3, 2, 1, 1, "diskrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 3, 1, 1, "firmware");

	MSX_S1985(config, "s1985", 0);

	msx2(SND_YM2149, config, layout_msx_jp_2fdd);
}

/* MSX2 - Sony HB-F9P */

ROM_START(hbf9p)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("f9pbios.rom.ic11", 0x0000, 0x8000, CRC(b31c851d) SHA1(0de3c802057560560a03d7965fcc4cff69f8575c))

	ROM_REGION(0x8000, "subfirm", 0)
	// dumps as listed in openMSX / blueMSX
	// ROM_LOAD("f9psub.rom", 0x0000, 0x4000, CRC(7c456c8b) SHA1(7b4a96402847decfc110ff9eda713bdcd218bd83))
	// ROM_LOAD("f9pfirm2.rom", 0x0000, 0x4000, CRC(dea2cb50) SHA1(8cc1f7ceeef745bb34e80253971e137213671486))
	// concatenation of 7b4a96402847decfc110ff9eda713bdcd218bd83 and 8cc1f7ceeef745bb34e80253971e137213671486
	ROM_LOAD("f9pfirm1.ic12", 0x0000, 0x8000, BAD_DUMP CRC(524f67aa) SHA1(41a186afced50ca6312cb5b6c4adb684faca6232))

	ROM_REGION(0x8000, "firmware", 0)
	// like in HB-F9S, the halves should be swapped?
	ROM_LOAD("f9pfirm2.rom.ic13", 0x0000, 0x8000, BAD_DUMP CRC(ea97069f) SHA1(2d1880d1f5a6944fcb1b198b997a3d90ecd1903d))
ROM_END

void msx2_state::hbf9p(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S-1985 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subfirm", 3, 0, 0, 2, "subfirm");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 1, 1, 2, "firmware");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0x80);   // 128KB Mapper RAM

	MSX_S1985(config, "s1985", 0);
	msx2_pal(SND_YM2149, config, layout_msx_nocode);
}

/* MSX2 - Sony HB-F9P Russian */

ROM_START(hbf9pr)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("f9prbios.rom", 0x0000, 0x8000, CRC(f465311b) SHA1(7f440ec7295d889b097e1b66bf9bc5ce086f59aa))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("f9prext.rom", 0x0000, 0x4000, CRC(d701adac) SHA1(a6d7b1fd4ee896ca7513d02c033fc9a8aa065235))
ROM_END

void msx2_state::hbf9pr(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S-1985 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "ext", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "ext_mirror", 3, 0, 1, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000);   // 128KB Mapper RAM

	MSX_S1985(config, "s1985", 0);
	msx2_pal(SND_YM2149, config, layout_msx_ru);
}

/* MSX2 - Sony HB-F9S */

ROM_START(hbf9s)
	ROM_REGION(0x18000, "mainrom", 0)
	ROM_LOAD("f9sbios.ic11", 0x0000, 0x8000, CRC(c2b889a5) SHA1(4811956f878c3e03da46317f787cdc4bebc86f47))

	ROM_REGION(0x8000, "subfirm", 0)
	ROM_LOAD("f9sfirm1.ic12", 0x0000, 0x8000, CRC(cf39620b) SHA1(1166a93d7185ba024bdf2bfa9a30e1c447fb6db1))

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("f9sfirm2.ic13", 0x0000, 0x8000, CRC(4a271395) SHA1(7efac54dd8f580f3b7809ab35db4ae58f0eb84d1))
ROM_END

void msx2_state::hbf9s(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S-1985 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subfirm", 3, 0, 0, 2, "subfirm");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware_lo", 3, 1, 1, 1, "firmware", 0x4000);
	add_internal_slot(config, MSX_SLOT_ROM, "firmware_hi", 3, 1, 2, 1, "firmware");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0x80);   // 128KB Mapper RAM

	MSX_S1985(config, "s1985", 0);
	msx2_pal(SND_YM2149, config, layout_msx_nocode);
}

/* MSX2 - Sony HB-G900AP */

/* IC109 - 32KB Basic ROM SLOT#00 0000-7FFF */
/* IC112 - 16KB Basic ROM SLOT#01 0000-3FFF */
/* IC117 - 16KB Disk ROM SLOT#01 4000-7FFF */
/* IC123 - 32KB ROM RS232C ROM SLOT#02 4000-7FFF / Video Utility ROM SLOT#03 4000-7FFF */

/* MSX2 - Sony HB-G900AP */
ROM_START(hbg900ap)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("g900bios.ic109",  0x0000, 0x8000, CRC(b31c851d) SHA1(0de3c802057560560a03d7965fcc4cff69f8575c))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("g900ext.ic112", 0x0000, 0x4000, CRC(8f84f783) SHA1(3288894e1be6af705871499b23c85732dbc40993))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("g900disk.ic117", 0x0000, 0x4000, CRC(54c73ad6) SHA1(12f2cc79b3d09723840bae774be48c0d721ec1c6))

	ROM_REGION(0x4000, "rs232", 0)
	// Contents very likely to be ok, but should be inside a single rom together with firmware
	ROM_LOAD("g900232c.rom", 0x0000, 0x2000, BAD_DUMP CRC(be88e5f7) SHA1(b2776159a7b92d74308b434a6b3e5feba161e2b7))

	ROM_REGION(0x4000, "firmware", 0)
	// Contents very likely to be ok, but should be inside a single rom together with rs232 code
	ROM_LOAD("g900util.rom", 0x0000, 0x4000, BAD_DUMP CRC(ecf6abcf) SHA1(6bb18cd2d69f124ad0c7c23a13eb0d2139037696))
ROM_END

void msx2_state::hbg900ap(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S1985
	// rs232 switch for terminal / modem operation
	// rs232 2kb ram

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_N, "disk", 0, 1, 1, 2, "diskrom");
	add_internal_slot_irq_mirrored<3>(config, MSX_SLOT_RS232_SONY, "rs232", 0, 2, 0, 4, "rs232");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 3, 1, 1, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	// slot #3 is expanded
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x80000).set_unused_bits(0x80);   // 512KB Mapper RAM

	msx2_pal(SND_YM2149, config, layout_msx_nocode_1fdd);
}

/* MSX2 - Sony HB-G900D */

/* MSX2 - Sony HB-G900F */

/* MSX2 - Sony HB-G900P - 3x 32KB ROMs */

ROM_START(hbg900p)
	ROM_REGION(0x18000, "mainrom", 0)
	ROM_LOAD("g900bios.rom", 0x0000, 0x8000, CRC(b31c851d) SHA1(0de3c802057560560a03d7965fcc4cff69f8575c))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("g900ext.rom", 0x0000, 0x4000, CRC(8f84f783) SHA1(3288894e1be6af705871499b23c85732dbc40993))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("g900disk.rom", 0x0000, 0x4000, CRC(54c73ad6) SHA1(12f2cc79b3d09723840bae774be48c0d721ec1c6))

	ROM_REGION(0x4000, "rs232", 0)
	ROM_LOAD("g900232c.rom", 0x0000, 0x2000, CRC(be88e5f7) SHA1(b2776159a7b92d74308b434a6b3e5feba161e2b7))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("g900util.rom", 0x0000, 0x4000, CRC(d0417c20) SHA1(8779b004e7605a3c419825f0373a5d8fa84e1d5b))
ROM_END

void msx2_state::hbg900p(machine_config &config)
{
	// AY8910
	// FDC: wd2793, 1 3.5" DSDD drive
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_N, "diskrom", 0, 1, 1, 2, "diskrom");
	add_internal_slot_irq_mirrored<3>(config, MSX_SLOT_RS232_SONY, "rs232", 0, 2, 0, 4, "rs232");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 3, 1, 1, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);   // 64KB RAM

	msx2_pal(SND_AY8910, config, layout_msx_nocode_1fdd);
}

/* MSX2 - Sony HB-T600 */

/* MSX2 - Sony HB-T7 */

/* MSX2 - Talent DPC-300 */

/* MSX2 - Talent TPC-310 */

ROM_START(tpc310)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("tpc310bios.rom", 0x0000, 0x8000, CRC(8cd3e845) SHA1(7bba23669b7abfb6a142f9e1735b847d6e4e8267))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("tpc310ext.rom", 0x0000, 0x4000, CRC(094a9e7a) SHA1(39dfc46260f99b670916b1e55f67a5d4136e6e54))

	ROM_REGION(0x4000, "turbo", 0)
	ROM_LOAD("tpc310turbo.rom", 0x0000, 0x4000, CRC(0ea62a4d) SHA1(181bf58da7184e128cd419da3109b93344a543cf))

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("tpc310acc.rom", 0x0000, 0x8000, CRC(4fb8fab3) SHA1(cdeb0ed8adecaaadb78d5a5364fd603238591685))
ROM_END

void msx2_state::tpc310(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: None, 0 drives
	// 1 Cartridge slot (slot 2)
	// S-1985 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 1, 0, 4).set_total_size(0x20000).set_unused_bits(0x80);   // 128KB Mapper RAM
	add_cartridge_slot<1>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "turbo", 3, 0, 1, 1, "turbo");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 1, 1, 2, "firmware");
	// Expansion slot in slot #3-2

	MSX_S1985(config, "s1985", 0);

	msx2_pal(SND_YM2149, config, layout_msx_nocode);
}

/* MSX2 - Talent TPP-311 */

ROM_START(tpp311)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("311bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(8cd3e845) SHA1(7bba23669b7abfb6a142f9e1735b847d6e4e8267)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("311ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(094a9e7a) SHA1(39dfc46260f99b670916b1e55f67a5d4136e6e54)) // need verification

	ROM_REGION(0x8000, "logo", 0)
	ROM_LOAD("311logo.rom", 0x0000, 0x8000, BAD_DUMP CRC(0e6ecb9f) SHA1(e45ddc5bf1a1e63756d11fb43fc50276ca35cab0)) // need verification
ROM_END

void msx2_state::tpp311(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: None, 0 drives
	// 0 Cartridge slots?
	// S1985
	// 64KB VRAM

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 1, 0, 4).set_total_size(0x10000);   // 64KB?? Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "logo", 2, 1, 2, "logo");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 1, "subrom");

	MSX_S1985(config, "s1985", 0);

	msx2_pal(SND_YM2149, config, layout_msx_nocode);
	msx2_64kb_vram(config);
}

/* MSX2 - Talent TPS-312 */

ROM_START(tps312)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("312bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(8cd3e845) SHA1(7bba23669b7abfb6a142f9e1735b847d6e4e8267)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("312ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(094a9e7a) SHA1(39dfc46260f99b670916b1e55f67a5d4136e6e54)) // need verification

	ROM_REGION(0x8000, "plan", 0)
	ROM_LOAD("312plan.rom", 0x0000, 0x8000, BAD_DUMP CRC(b3a6aaf6) SHA1(6de80e863cdd7856ab7aac4c238224a5352bda3b)) // need verification

	ROM_REGION(0x4000, "write", 0)
	ROM_LOAD("312write.rom", 0x0000, 0x4000, BAD_DUMP CRC(63c6992f) SHA1(93682f5baba7697c40088e26f99ee065c78e83b8)) // need verification
ROM_END

void msx2_state::tps312(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 64KB VRAM
	// S1985

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 1, 0, 4).set_total_size(0x10000);   // 64KB Mapper RAM
	add_cartridge_slot<1>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "write", 3, 0, 1, 1, "write");
	add_internal_slot(config, MSX_SLOT_ROM, "plan", 3, 1, 0, 2, "plan");
	add_internal_slot(config, MSX_SLOT_ROM, "plan_mirror", 3, 1, 2, 2, "plan");
	// Expansion slot in slot #3-2

	MSX_S1985(config, "s1985", 0);

	msx2_pal(SND_YM2149, config, layout_msx_nocode);
	msx2_64kb_vram(config);
}

/* MSX2 - Toshiba FS-TM1 */

ROM_START(fstm1)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("fstm1bios.rom", 0x0000, 0x8000, CRC(d1e11d52) SHA1(7a69e9b9595f3b0060155f4b419c915d4d9d8ca1))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("fstm1ext.rom", 0x0000, 0x4000, CRC(4eebe9b1) SHA1(a4bdbdb20bf9fd3c492a890fbf541bf092eaa8e1))

	ROM_REGION(0x8000, "deskpac1", 0)
	ROM_LOAD("fstm1desk1.rom", 0x0000, 0x8000, CRC(8b802086) SHA1(30737040d90c136d34dd409fe579bc4cca11c469))

	ROM_REGION(0x8000, "deskpac2", 0)
	ROM_LOAD("fstm1desk2.rom", 0x0000, 0x8000, CRC(304820ea) SHA1(ff6e07d3976b0874164fae680ae028d598752049))
ROM_END

void msx2_state::fstm1(machine_config &config)
{
	// YM2149 (in S-1985)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S-1985 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000); // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "deskpac1", 3, 1, 1, 2, "deskpac1");
	add_internal_slot(config, MSX_SLOT_ROM, "deskpac2", 3, 3, 1, 2, "deskpac2");

	MSX_S1985(config, "s1985", 0);
	// Hard to see on pictures whether the machine has a CAPS led
	msx2_pal(SND_YM2149, config, layout_msx_nocode);
}

/* MSX2 - Toshiba HX-23 */

ROM_START(hx23)
	// roms from hx23f, assumed to be the same for hx23 but need verification
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx23bios.ic2", 0x0000, 0x8000, BAD_DUMP CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd)) // need verification

	ROM_REGION(0x8000, "subjwp", 0)
	ROM_LOAD("hx23subjwp.ic52", 0x0000, 0x8000, BAD_DUMP CRC(478016bf) SHA1(6ecf73a1dd55b363c2e68cc6245ece979aec1fc5)) // need verification

	ROM_REGION(0x8000, "rs232jwp", 0)
	ROM_LOAD("hx23rs232jwp.ic3", 0x0000, 0x8000, BAD_DUMP CRC(60160d3b) SHA1(0958361ac9b19782cf7017b2e762b416e0203f37)) // need verification
ROM_END

void msx2_state::hx23(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// 64KB VRAM
	// HX-R701 RS-232 optional
	// TCX-1012

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 3, 0, 0, 2);   // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "sub", 3, 1, 0, 1, "subjwp");
	add_internal_slot(config, MSX_SLOT_ROM, "jwp", 3, 1, 2, 1, "subjwp", 0x4000);
	add_internal_slot(config, MSX_SLOT_ROM, "rs232jwp", 3, 3, 1, 2, "rs232jwp");

	msx2(SND_AY8910, config, layout_msx_jp);
	msx2_64kb_vram(config);
}

/* MSX2 - Toshiba HX-23F */

ROM_START(hx23f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx23bios.ic2", 0x0000, 0x8000, BAD_DUMP CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd)) // need verification

	ROM_REGION(0x8000, "subjwp", 0)
	ROM_LOAD("hx23subjwp.ic52", 0x0000, 0x8000, BAD_DUMP CRC(478016bf) SHA1(6ecf73a1dd55b363c2e68cc6245ece979aec1fc5)) // need verification

	ROM_REGION(0x8000, "rs232jwp", 0)
	ROM_LOAD("hx23rs232jwp.ic3", 0x0000, 0x8000, BAD_DUMP CRC(60160d3b) SHA1(0958361ac9b19782cf7017b2e762b416e0203f37)) // need verification
ROM_END

void msx2_state::hx23f(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// HX-R701 RS-232 optional
	// TCX-1012

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 3, 0, 0, 2);   // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "sub", 3, 1, 0, 1, "subjwp");
	add_internal_slot(config, MSX_SLOT_ROM, "jwp", 3, 1, 2, 1, "subjwp", 0x4000);
	add_internal_slot(config, MSX_SLOT_ROM, "rs232jwp", 3, 3, 1, 2, "rs232jwp");

	msx2(SND_AY8910, config, layout_msx_jp);
}

/* MSX2 - Toshiba HX-33 */

ROM_START(hx33)
	ROM_REGION(0x20000, "mainrom", 0)
	ROM_LOAD("hx33bios.ic7", 0x0000, 0x20000, CRC(8dd5502b) SHA1(5e057526fe39d79e88e7ff1ce02ed669bd38929e))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("hx33kfn.rom", 0x0000, 0x20000, CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967))
ROM_END

void msx2_state::hx33(machine_config &config)
{
	// YM2149
	// FDC: None, 0, drives
	// 2 Cartridge slots
	// TCX-2001 + TCX-2002
	// HX-R702 RS-232 optional
	// 2KB SRAM
	// copy button

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000); // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "mainrom", 0x8000);
	add_internal_slot(config, MSX_SLOT_RS232_TOSHIBA_HX3X, "firmware", 3, 3, 1, 2, "mainrom", 0xc000);

	msx2(SND_YM2149, config, layout_msx_jp);
	msx2_64kb_vram(config);
}

/* MSX@ - Toshiba HX-34 */

ROM_START(hx34)
	ROM_REGION(0x20000, "mainrom", 0)
	ROM_LOAD("hx33bios.ic7", 0x0000, 0x20000, CRC(8dd5502b) SHA1(5e057526fe39d79e88e7ff1ce02ed669bd38929e))

	ROM_REGION(0x4000, "diskrom", 0)
	// hx34disk.rom has contents of floppy registers at offset 3ff0-3ff7 and mirrored at 3ff8-3fff
	ROM_LOAD("hx34disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(626b719d) SHA1(c88ef953b21370cbaef5e82575d093d6f9047ec6))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("hx34kfn.rom", 0x0000, 0x20000, CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967))
ROM_END

void msx2_state::hx34(machine_config &config)
{
	// YM2149
	// FDC: wd2793??, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// TCX-2001 + TCX-2002
	// HX-R703 RS232 optional
	// 2KB SRAM
	// copy button

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000); // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "mainrom", 0x8000);
	add_internal_disk(config, MSX_SLOT_DISK6_WD2793_N, "disk", 3, 2, 1, 1, "diskrom");
	add_internal_slot(config, MSX_SLOT_RS232_TOSHIBA_HX3X, "firmware", 3, 3, 1, 2, "mainrom", 0xc000);

	msx2(SND_YM2149, config, layout_msx_jp_1fdd);
}

/* MSX2 - Victor HC-80 */

ROM_START(victhc80)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hc80bios.rom",  0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("hc80ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("hc80firm.rom", 0x0000, 0x4000, CRC(30e8c08d) SHA1(7f498db2f431b9c0b42dac1c7ca46a236b780228))
ROM_END

void msx2_state::victhc80(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 0, 2, 2); // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 0, 2, 0, 2); // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 2, 1, "firmware");

	msx2(SND_YM2149, config, layout_msx_jp);
}

/* MSX2 - Victor HC-90 */

ROM_START(victhc90)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hc90bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("hc90ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("hc90disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(11bca2ed) SHA1(a7a34671bddb48fa6c74182e2977f9129558ec32)) // need verification

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("hc90firm.rom", 0x0000, 0x4000, BAD_DUMP CRC(53791d91) SHA1(caeffdd654394726c8c0824b21af7ff51c0b1031)) // need verification

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("hc90kfn.rom", 0x0000, 0x20000, BAD_DUMP CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967)) // need verification
ROM_END

void msx2_state::victhc90(machine_config &config)
{
	// YM2149
	// FDC: mb8877a?, 1 3.5" DSDD drive
	// RS232C builtin
	// 2nd CPU HD-64B180 @ 6.144 MHz
	// 1 Cartridge slot (slot 1 or 2?)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot_irq<2>(config, MSX_SLOT_RS232, "firmware", 0, 1, 1, 1, "firmware");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 0, 2, 0, 4).set_total_size(0x10000); // 64KB Mapper RAM
	add_cartridge_slot<1>(config, 1);
	add_internal_disk(config, MSX_SLOT_DISK10_MB8877, "disk", 3, 1, 1, "diskrom");

	msx2(SND_YM2149, config, layout_msx_jp_1fdd);
}

/* MSX2 - Victor HC-90(A) */

ROM_START(victhc90a)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("msx2basic_tmm23256.ic023", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("msx2basicext_tmm23128p.ic034", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x8000, "rs232fdd", 0)
	ROM_LOAD("rs232c_fdd_jvc024c_27c256.ic052", 0x000, 0x8000, CRC(19cfc325) SHA1(c991440778d5dc9ba54cc0e0f8e032d2f451366f))
	// Patch to fake reads from the system control register
	ROM_FILL(0x3ffd, 1, 0x80)
	ROM_FILL(0x7ffd, 1, 0x80)

	ROM_REGION(0x8000, "turbo", 0)
	ROM_LOAD("turbo_jvc019e_27c256.ic040", 0x0000, 0x8000, CRC(7820ea1a) SHA1(ae81cc93e3992e253d42f48451adc4806074f494))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("hc90a_kanjifont.rom", 0x0000, 0x20000, CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967))
ROM_END

void msx2_state::victhc90a(machine_config &config)
{
	// YM2149
	// FDC: mb8877a?, 1 3.5" DSDD drive
	// RS232C builtin
	// 2nd CPU HD-64B180 @ 6.144 MHz
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot_irq<2>(config, MSX_SLOT_RS232, "rs232fdd", 0, 1, 1, 1, "rs232fdd");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 0, 2, 0, 4).set_total_size(0x40000); // 256KB Mapper RAM
	add_cartridge_slot<1>(config, 1);
	add_internal_disk(config, MSX_SLOT_DISK10_MB8877, "disk", 3, 1, 1, "rs232fdd", 0x4000);

	msx2(SND_YM2149, config, layout_msx_jp_1fdd);
}

/* MSX2 - Victor HC-90(B) */

/* MSX2 - Victor HC-90(V) */

/* MSX2 - Victor HC-90(T) */

/* MSX2 - Victor HC-95 */

ROM_START(victhc95)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hc95bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("hc95ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("hc95disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(11bca2ed) SHA1(a7a34671bddb48fa6c74182e2977f9129558ec32)) // need verification

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("hc95firm.rom", 0x0000, 0x4000, BAD_DUMP CRC(53791d91) SHA1(caeffdd654394726c8c0824b21af7ff51c0b1031)) // need verification

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("hc95kfn.rom", 0x0000, 0x20000, BAD_DUMP CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967)) // need verification
ROM_END

void msx2_state::victhc95(machine_config &config)
{
	// YM2149
	// FDC: mb8877a, 2 3.5" DSDD drive
	// RS232C builtin
	// 2nd CPU HD-64B180 @ 6.144 MHz
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 1, 1, 1, "firmware");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 0, 2, 0, 4).set_total_size(0x10000); // 64KB Mapper RAM
	// 96 pin expansion bus in slot #0-3
	add_cartridge_slot<1>(config, 1);
	// 96 pin expansion bus in slot #2
	add_internal_disk(config, MSX_SLOT_DISK10_MB8877_2_DRIVES, "disk", 3, 1, 1, "diskrom");

	msx2(SND_YM2149, config, layout_msx_jp_2fdd);
}

/* MSX2 - Victor HC-95A */

ROM_START(victhc95a)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hc95abios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("hc95aext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("hc95adisk.rom", 0x0000, 0x4000, BAD_DUMP CRC(11bca2ed) SHA1(a7a34671bddb48fa6c74182e2977f9129558ec32))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("hc95afirm.rom", 0x0000, 0x4000, CRC(53791d91) SHA1(caeffdd654394726c8c0824b21af7ff51c0b1031))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("hc95akfn.rom", 0x0000, 0x20000, CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967))
ROM_END

void msx2_state::victhc95a(machine_config &config)
{
	// YM2149
	// FDC: mb8877a, 2 3.5" DSDD drive
	// RS232C builtin
	// 2nd CPU HD-64B180 @ 6.144 MHz
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 1, 1, 1, "firmware");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 0, 2, 0, 4).set_total_size(0x10000); // 64KB Mapper RAM
	// 96 pin expansion bus in slot #0-3
	add_cartridge_slot<1>(config, 1);
	// 96 pin expansion bus in slot #2
	add_internal_disk(config, MSX_SLOT_DISK10_MB8877_2_DRIVES, "disk", 3, 1, 1, "diskrom");

	msx2(SND_YM2149, config, layout_msx_jp_2fdd);
}

/* MSX2 - Wandy CPC-300 */

/* MSX2 - Yamaha CX7/128 */

ROM_START(cx7128)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("cx7mbios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("cx7mext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))
ROM_END

void msx2_state::cx7128(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_MINICART, "minicart", 3, 1, msx_yamaha_minicart, nullptr);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0x80);   // 64KB Mapper RAM
	add_cartridge_slot<4>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, 3, msx_yamaha_60pin, nullptr);

	msx2(SND_YM2149, config, layout_msx_jp);
	SOFTWARE_LIST(config, "minicart_list").set_original("msx_yamaha_minicart");
}

/* MSX2 - Yamaha CX7M/128 */

ROM_START(cx7m128)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("cx7mbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd)) // needs verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("cx7mext.rom", 0x0000, 0x4000, BAD_DUMP CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33)) // needs verification
ROM_END

void msx2_state::cx7m128(machine_config &config)
{
	// YM2149 (in S3527)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_MINICART, "minicart", 3, 1, msx_yamaha_minicart, nullptr);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000).set_unused_bits(0x80);   // 64KB Mapper RAM
	add_cartridge_slot<4>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, 3, msx_yamaha_60pin, "sfg05");

	msx2(SND_YM2149, config, layout_msx_jp);
	SOFTWARE_LIST(config, "minicart_list").set_original("msx_yamaha_minicart");
}

/* MSX2 - Yamaha YIS-503 III R (student) */

ROM_START(y503iiir)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yis503iiirbios.rom", 0x0000, 0x8000, CRC(e7d08e29) SHA1(0f851ee7a1cf79819f61cc89e9948ee72a413802))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("yis503iiirext.rom", 0x0000, 0x4000, CRC(34d21778) SHA1(03bf6d2ac86f5c9ab618e155442787c700f99fed))

	ROM_REGION(0x4000, "cpm", 0)
	ROM_LOAD("yis503iiircpm.rom", 0x0000, 0x4000, CRC(417bf00e) SHA1(f4f7a54cdf5a9dd6c59f7cb219c2c5eb0a00fa8a))

	ROM_REGION(0x8000, "network", 0)
	// has 2 * 2KB RAM?
	ROM_LOAD("yis503iiirnet.rom", 0x0000, 0x8000, CRC(75331cac) SHA1(307a7be064442feb4ab2e1a2bc971b138c1a1169))
ROM_END

void msx2_state::y503iiir(machine_config &config)
{
	// YM2149 (in S-3527)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// Networking builtin
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "cpm", 3, 0, 1, 1, "cpm");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000); // 128KB Mapper RAM
	// Yamaha expansion slot in slot #3-3
	add_internal_slot(config, MSX_SLOT_ROM, "network", 3, 3, 1, 2, "network");

	msx2_pal(SND_YM2149, config, layout_msx_ru);
}

/* MSX2 - Yamaha YIS-503 III R Estonian */

ROM_START(y503iiire)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yis503iiirebios.rom", 0x0000, 0x8000, CRC(d0c20f54) SHA1(ebb7eb540a390509edfd36c84288ba85e63f2d1f))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("yis503iiireext.rom", 0x0000, 0x4000, CRC(34d21778) SHA1(03bf6d2ac86f5c9ab618e155442787c700f99fed))

	ROM_REGION(0x4000, "cpm", 0)
	ROM_LOAD("yis503iiirecpm.rom", 0x0000, 0x4000, CRC(417bf00e) SHA1(f4f7a54cdf5a9dd6c59f7cb219c2c5eb0a00fa8a))

	ROM_REGION(0x8000, "network", 0)
	ROM_LOAD("yis503iiirnet.rom", 0x0000, 0x8000, CRC(75331cac) SHA1(307a7be064442feb4ab2e1a2bc971b138c1a1169))
ROM_END

/* MSX2 - Yamaha YIS604/128 */

ROM_START(yis604)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yis604bios.rom", 0x0000, 0x8000, CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("yis604ext.rom", 0x0000, 0x4000, CRC(4a48779c) SHA1(b8e30d604d319d511cbfbc61e5d8c38fbb9c5a33))
ROM_END

void msx2_state::yis604(machine_config &config)
{
	// YM2149 (in S-3527)
	// FDC: None, 0 drives
	// 1 Minicart slot (with Beginnner's Lesson)
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_MINICART, "minicart", 3, 1, msx_yamaha_minicart, nullptr);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000); // 128KB Mapper RAM
	add_cartridge_slot<4>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, 3, msx_yamaha_60pin, nullptr);

	msx2(SND_YM2149, config, layout_msx_jp);
	SOFTWARE_LIST(config, "minicart_list").set_original("msx_yamaha_minicart");
}

/* MSX2 - Yamaha YIS-805/128 */

ROM_START(y805128)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yis805128bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("yis805128ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("yis805128disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(ab94a273) SHA1(4b08a057e5863ade179dcf8bc9377e90940e6d61)) // need verification

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("yis805128kfn.rom", 0x0000, 0x20000, BAD_DUMP CRC(5a59926e) SHA1(6acaf2eeb57f65f7408235d5e07b7563229de799)) // need verification
ROM_END

void msx2_state::y805128(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: wd2793?, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// S1985 MSX Engine
	// 2KB SRAM
	// no mini cartridge slot?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_disk_mirrored(config, MSX_SLOT_DISK11_WD2793, "disk", 3, 0, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000); // 128KB Mapper RAM
	// Default: SKW-05
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, 3, msx_yamaha_60pin, nullptr);

	MSX_S1985(config, "s1985", 0);
	msx2(SND_YM2149, config, layout_msx_jp_1fdd);
}

/* MSX2 - Yamaha YIS-805/256 */

ROM_START(y805256)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yis805256bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(9b3e7b97) SHA1(0081ea0d25bc5cd8d70b60ad8cfdc7307812c0fd)) // need verification

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("yis805256ext.rom", 0x0000, 0x4000, BAD_DUMP CRC(43e7a7fc) SHA1(0fbd45ef3dd7bb82d4c31f1947884f411f1ca344)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("yis805256disk.rom", 0x0000, 0x4000, BAD_DUMP CRC(ab94a273) SHA1(4b08a057e5863ade179dcf8bc9377e90940e6d61)) // need verification

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("yis805256kfn.rom", 0x0000, 0x20000, BAD_DUMP CRC(5a59926e) SHA1(6acaf2eeb57f65f7408235d5e07b7563229de799)) // need verification
ROM_END

void msx2_state::y805256(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: wd2793?, 2 3.5" DSDD drives
	// 2 Cartridge slots
	// S1985 MSX Engine
	// 2KB SRAM
	// RS232C
	// no mini cartridge slot?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 0, 1, 0, 1, "subrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_2_DRIVES, "disk", 3, 0, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x40000); // 256KB Mapper RAM
	// Default: SKW-05
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, 3, msx_yamaha_60pin, nullptr);

	MSX_S1985(config, "s1985", 0);
	msx2(SND_YM2149, config, layout_msx_jp_2fdd);
}

/* MSX2 - Yamaha YIS-805/128R2 (teacher) */

ROM_START(y805128r2)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yis805128r2bios.rom", 0x0000, 0x8000, CRC(e7d08e29) SHA1(0f851ee7a1cf79819f61cc89e9948ee72a413802))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("yis805128r2ext.rom", 0x0000, 0x4000, CRC(34d21778) SHA1(03bf6d2ac86f5c9ab618e155442787c700f99fed))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("yis805128r2disk.rom", 0x0000, 0x4000, CRC(9eb7e24d) SHA1(3a481c7b7e4f0406a55952bc5b9f8cf9d699376c))

	ROM_REGION(0x8000, "network", 0)
	// has 2 * 2KB RAM ?
	ROM_LOAD("yis805128r2net.rom", 0x0000, 0x8000, CRC(0e345b43) SHA1(e8fd2bbc1bdab12c73a0fec178a190f9063547bb))

	ROM_REGION(0x10000, "firmware", 0)
	ROM_LOAD("yis805128r2paint.rom", 0x00000, 0x10000, CRC(1bda68a3) SHA1(7fd2a28c4fdaeb140f3c8c8fb90271b1472c97b9))
ROM_END

void msx2_state::y805128r2(machine_config &config)
{
	// YM2149 (in S1985)
	// FDC: wd2793, 2 3.5" DSDD drives
	// 2 Cartridge slots
	// S1985 MSX Engine
	// Networking built in

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 0, 0, 4, "firmware");
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK8_WD2793_2_DRIVES, "disk", 3, 1, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000); // 128KB Mapper RAM
	// This is actually the module slot
	add_internal_slot(config, MSX_SLOT_ROM, "network", 3, 3, 0, 2, "network", 0x00000);

	MSX_S1985(config, "s1985", 0);
	msx2_pal(SND_YM2149, config, layout_msx_ru_2fdd);
}

/* MSX2 - Yamaha YIS-805/128R2 Estonian */

ROM_START(y805128r2e)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yis805128r2ebios.rom", 0x0000, 0x8000, CRC(d0c20f54) SHA1(ebb7eb540a390509edfd36c84288ba85e63f2d1f))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("yis805128r2eext.rom", 0x0000, 0x4000, CRC(34d21778) SHA1(03bf6d2ac86f5c9ab618e155442787c700f99fed))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("yis805128r2edisk.rom", 0x0000, 0x4000, CRC(9eb7e24d) SHA1(3a481c7b7e4f0406a55952bc5b9f8cf9d699376c))

	ROM_REGION(0x8000, "network", 0)
	ROM_LOAD("yis805128r2enet.rom", 0x0000, 0x8000, CRC(0e345b43) SHA1(e8fd2bbc1bdab12c73a0fec178a190f9063547bb))

	ROM_REGION(0x10000, "firmware", 0)
	ROM_LOAD("yis805128r2epaint.rom", 0x00000, 0x10000, CRC(1bda68a3) SHA1(7fd2a28c4fdaeb140f3c8c8fb90271b1472c97b9))
ROM_END

} // anonymous namespace

/* MSX2 */
COMP(1985, canonv25,   0,        0,     canonv25,   msxjp,    msx2_state, empty_init, "Canon", "V-25 (MSX2, Japan)", 0)
COMP(1985, canonv30f,  0,        0,     canonv30f,  msx2,     msx2_state, empty_init, "Canon", "V-30F (MSX2, Japan)", 0)
COMP(1986, cpc300,     0,        0,     cpc300,     msx2kr,   msx2_state, empty_init, "Daewoo", "IQ-2000 CPC-300 (MSX2, Korea)", 0)
COMP(1987, cpc300e,    0,        0,     cpc300e,    msx2kr,   msx2_state, empty_init, "Daewoo", "IQ-2000 CPC-300E (MSX2, Korea)", 0)
COMP(1988, cpc330k,    0,        0,     cpc330k,    msx2kr,   msx2_state, empty_init, "Daewoo", "CPC-330K KOBO (MSX2, Korea)", 0)
COMP(1987, cpc400,     0,        0,     cpc400,     msx2kr,   msx2_state, empty_init, "Daewoo", "X-II CPC-400 (MSX2, Korea)", 0)
COMP(1988, cpc400s,    0,        0,     cpc400s,    msx2kr,   msx2_state, empty_init, "Daewoo", "X-II CPC-400S (MSX2, Korea)", 0)
COMP(1990, cpc61,      0,        0,     cpc61,      msxkr,    msx2_state, empty_init, "Daewoo", "Zemmix CPC-61 (MSX2, Korea)", 0)
COMP(1991, cpg120,     0,        0,     cpg120,     msx2kr,   msx2_state, empty_init, "Daewoo", "Zemmix CPG-120 Normal (MSX2, Korea)", 0)
COMP(1986, fpc900,     0,        0,     fpc900,     msx2,     msx2_state, empty_init, "Fenner", "FPC-900 (MSX2, Italy)", 0)
COMP(1986, expert20,   0,        0,     expert20,   msx2,     msx2_state, empty_init, "Gradiente", "Expert 2.0 (MSX2, Brazil)", 0)
COMP(1985, mbh3,       0,        0,     mbh3,       msx2jp,   msx2_state, empty_init, "Hitachi", "MB-H3 (MSX2, Japan)", 0)
COMP(1986, mbh70,      0,        0,     mbh70,      msx2jp,   msx2_state, empty_init, "Hitachi", "MB-H70 (MSX2, Japan)", MACHINE_NOT_WORKING) // How to enter/use the firmware?
COMP(1987, kmc5000,    0,        0,     kmc5000,    msx2jp,   msx2_state, empty_init, "Kawai", "KMC-5000 (MSX2, Japan)", 0)
COMP(1986, mlg1,       0,        0,     mlg1,       msx2sp,   msx2_state, empty_init, "Mitsubishi", "ML-G1 (MSX2, Spain)", 0)
COMP(1986, mlg3,       0,        0,     mlg3,       msx2sp,   msx2_state, empty_init, "Mitsubishi", "ML-G3 (MSX2, Spain)", 0)
COMP(1985, mlg10,      0,        0,     mlg10,      msx2jp,   msx2_state, empty_init, "Mitsubishi", "ML-G10 (MSX2, Japan)", 0)
COMP(1985, mlg30,      0,        0,     mlg30,      msx2jp,   msx2_state, empty_init, "Mitsubishi", "ML-G30 Model 1 (MSX2, Japan)", 0)
COMP(1985, mlg30_2,    0,        0,     mlg30_2,    msx2jp,   msx2_state, empty_init, "Mitsubishi", "ML-G30 Model 2 (MSX2, Japan)", 0)
COMP(1986, fs4500,     0,        0,     fs4500,     msx2jp,   msx2_state, empty_init, "National", "FS-4500 (MSX2, Japan)", 0)
COMP(1986, fs4600f,    0,        0,     fs4600f,    msx2jp,   msx2_state, empty_init, "National", "FS-4600F (MSX2, Japan)", 0)
COMP(1986, fs4700f,    0,        0,     fs4700f,    msx2jp,   msx2_state, empty_init, "National", "FS-4700F (MSX2, Japan)", 0)
COMP(1986, fs5000f2,   0,        0,     fs5000f2,   msx2jp,   msx2_state, empty_init, "National", "FS-5000F2 (MSX2, Japan)", 0)
COMP(1985, fs5500f1,   fs5500f2, 0,     fs5500f1,   msx2jp,   msx2_state, empty_init, "National", "FS-5500F1 (MSX2, Japan)", 0)
COMP(1985, fs5500f2,   0,        0,     fs5500f2,   msx2jp,   msx2_state, empty_init, "National", "FS-5500F2 (MSX2, Japan)", 0)
COMP(1986, fsa1,       fsa1a,    0,     fsa1,       msxjp,    msx2_state, empty_init, "Panasonic", "FS-A1 / 1st released version (MSX2, Japan)", 0)
COMP(1986, fsa1a,      0,        0,     fsa1a,      msxjp,    msx2_state, empty_init, "Panasonic", "FS-A1 / 2nd released version (MSX2, Japan)", 0)
COMP(1987, fsa1mk2,    0,        0,     fsa1mk2,    msx2jp,   msx2_state, empty_init, "Panasonic", "FS-A1MK2 (MSX2, Japan)", 0)
COMP(1987, fsa1f,      0,        0,     fsa1f,      msx2jp,   msx2_state, empty_init, "Panasonic", "FS-A1F (MSX2, Japan)", 0)
COMP(1988, fsa1fm,     0,        0,     fsa1fm,     msx2jp,   msx2_state, empty_init, "Panasonic", "FS-A1FM (MSX2, Japan)", MACHINE_NOT_WORKING) // Modem not emulated, firmware partially working
COMP(1987, nms8220,    0,        0,     nms8220,    msx,      msx2_state, empty_init, "Philips", "NMS 8220 (MSX2, Europe)", 0)
COMP(1987, nms8245,    0,        0,     nms8245,    msx,      msx2_state, empty_init, "Philips", "NMS 8245 (MSX2, Europe)", 0)
COMP(1987, nms8245f,   nms8245,  0,     nms8245f,   msxfr,    msx2_state, empty_init, "Philips", "NMS 8245F (MSX2, France)", 0)
COMP(1987, nms8250,    nms8255,  0,     nms8250,    msx2,     msx2_state, empty_init, "Philips", "NMS 8250 (MSX2, Europe)", 0)
COMP(1987, nms8250_16, nms8255,  0,     nms8250,    msx2sp,   msx2_state, empty_init, "Philips", "NMS 8250/16 (MSX2, Spain)", 0)
COMP(1987, nms8250_19, nms8255,  0,     nms8250,    msx2fr,   msx2_state, empty_init, "Philips", "NMS 8250/19 (MSX2, France)", 0)
COMP(1987, nms8255,    0,        0,     nms8255,    msx2,     msx2_state, empty_init, "Philips", "NMS 8255 (MSX2, Europe)", 0)
COMP(1987, nms8255f,   nms8255,  0,     nms8255f,   msx2fr,   msx2_state, empty_init, "Philips", "NMS 8255F (MSX2, France)", 0)
COMP(1987, nms8260,    0,        0,     nms8260,    msx2,     msx2_state, empty_init, "Philips", "NMS 8260 (MSX2, Prototype)", MACHINE_NOT_WORKING)
COMP(1987, nms8280,    0,        0,     nms8280,    msx2,     msx2_state, empty_init, "Philips", "NMS 8280 (MSX2, Europe)", 0)
COMP(1986, nms8280f,   nms8280,  0,     nms8280f,   msx2fr,   msx2_state, empty_init, "Philips", "NMS 8280F (MSX2, France)", 0)
COMP(1986, nms8280g,   nms8280,  0,     nms8280g,   msx2de,   msx2_state, empty_init, "Philips", "NMS 8280G (MSX2, Germany)", 0)
COMP(1986, vg8230,     0,        0,     vg8230,     msx,      msx2_state, empty_init, "Philips", "VG-8230 (MSX2, Netherlands)", 0)
COMP(1986, vg8235,     0,        0,     vg8235,     msx,      msx2_state, empty_init, "Philips", "VG-8235 (MSX2, Europe)", 0)
COMP(1986, vg8235f,    vg8235,   0,     vg8235f,    msxfr,    msx2_state, empty_init, "Philips", "VG-8235F (MSX2, France)", 0)
COMP(1986, vg8240,     0,        0,     vg8240,     msx,      msx2_state, empty_init, "Philips", "VG-8240 (MSX2, Prototype)", 0)
COMP(1987, ucv102,     0,        0,     ucv102,     msx2jp,   msx2_state, empty_init, "Pioneer", "UC-V102 (MSX2, Japan)", 0)
COMP(1987, ax350,      ax350ii,  0,     ax350,      msx,      msx2_state, empty_init, "Sakhr", "AX-350 (MSX2, Arabic)", 0)
COMP(1987, ax350ii,    0,        0,     ax350ii,    msx,      msx2_state, empty_init, "Sakhr", "AX-350 II (MSX2, Arabic)", 0)
COMP(1987, ax350iif,   ax350ii,  0,     ax350iif,   msxfr,    msx2_state, empty_init, "Sakhr", "AX-350 II F (MSX2, Arabic)", 0)
COMP(1988, ax370,      0,        0,     ax370,      msx2,     msx2_state, empty_init, "Sakhr", "AX-370 (MSX2, Arabic)", 0)
COMP(1987, ax500,      0,        0,     ax500,      msx2,     msx2_state, empty_init, "Sakhr", "AX-500 (MSX2, Arabic)", 0)
COMP(1987, mpc2300,    0,        0,     mpc2300,    msxru,    msx2_state, empty_init, "Sanyo", "MPC-2300 (MSX2, USSR)", 0)
COMP(1987, mpc2500f,   0,        0,     mpc2500f,   msx2ru,   msx2_state, empty_init, "Sanyo", "MPC-2500FD (MSX2, USSR)", 0)
COMP(1985, mpc25fd,    0,        0,     mpc25fd,    msx2jp,   msx2_state, empty_init, "Sanyo", "MPC-25FD (MSX2, Japan)", 0)
COMP(1985, mpc25fs,    0,        0,     mpc25fs,    msx2jp,   msx2_state, empty_init, "Sanyo", "MPC-25FS (MSX2, Japan)", 0)
COMP(1985, mpc27,      0,        0,     mpc27,      msx2jp,   msx2_state, empty_init, "Sanyo", "MPC-27 (MSX2, Japan)", MACHINE_NOT_WORKING) // Light pen not emulated
COMP(1986, phc23,      0,        0,     phc23,      msx2jp,   msx2_state, empty_init, "Sanyo", "PHC-23 / Wavy23 (MSX2, Japan)", 0)
//COMP(1987, phc23j,     0,        0,     phc23,      msx2jp,   msx2_state, empty_init, "Sanyo", "PHC-23J / Wavy23 (MSX2, Japan)", 0) // different keyboard layout
COMP(1987, phc23jb,    0,        0,     phc23jb,    msx2jp,   msx2_state, empty_init, "Sanyo", "PHC-23JB / Wavy23 (MSX2, Japan)", 0)
COMP(1988, phc55fd2,   0,        0,     phc55fd2,   msx2jp,   msx2_state, empty_init, "Sanyo", "PHC-55FD2 / Wavy55FD2 (MSX2, Japan)", 0)
COMP(1987, phc77,      0,        0,     phc77,      msx2jp,   msx2_state, empty_init, "Sanyo", "PHC-77 / Wavy77 (MSX2, Japan)", 0)
COMP(1986, hotbit20,   0,        0,     hotbit20,   msx2,     msx2_state, empty_init, "Sharp / Epcom", "HB-8000 Hotbit 2.0 (MSX2)", 0)
COMP(1986, hbf1,       hbf1xd,   0,     hbf1,       msxjp,    msx2_state, empty_init, "Sony", "HB-F1 (MSX2, Japan)", 0)
COMP(1987, hbf1ii,     hbf1xd,   0,     hbf1ii,     msxjp,    msx2_state, empty_init, "Sony", "HB-F1II (MSX2, Japan)", 0)
COMP(1987, hbf1xd,     0,        0,     hbf1xd,     msx2jp,   msx2_state, empty_init, "Sony", "HB-F1XD (MSX2, Japan)", 0)
COMP(1985, hbf5,       0,        0,     hbf5,       msx2jp,   msx2_state, empty_init, "Sony", "HB-F5 (MSX2, Japan)", 0)
COMP(1986, hbf9p,      0,        0,     hbf9p,      msx2uk,   msx2_state, empty_init, "Sony", "HB-F9P (MSX2, Europe)", 0)
COMP(19??, hbf9pr,     hbf9p,    0,     hbf9pr,     msx2ru,   msx2_state, empty_init, "Sony", "HB-F9P (MSX2, Russian, prototype)", 0)
COMP(1986, hbf9s,      hbf9p,    0,     hbf9s,      msx2sp,   msx2_state, empty_init, "Sony", "HB-F9S (MSX2, Spain)", 0)
COMP(1986, hbf500,     hbf500p,  0,     hbf500,     msx2jp,   msx2_state, empty_init, "Sony", "HB-F500 (MSX2, Japan)", 0)
COMP(1986, hbf500_2,   hbf500p,  0,     hbf500_2,   msx2jp,   msx2_state, empty_init, "Sony", "HB-F500 2nd version (MSX2, Japan)", 0)
COMP(1986, hbf500f,    hbf500p,  0,     hbf500f,    msx2fr,   msx2_state, empty_init, "Sony", "HB-F500F (MSX2, France)", 0)
COMP(1986, hbf500p,    0,        0,     hbf500p,    msx2,     msx2_state, empty_init, "Sony", "HB-F500P (MSX2, Europe)", 0)
COMP(1986, hbf700d,    hbf700p,  0,     hbf700d,    msx2de,   msx2_state, empty_init, "Sony", "HB-F700D (MSX2, Germany)", 0)
COMP(1986, hbf700f,    hbf700p,  0,     hbf700f,    msx2fr,   msx2_state, empty_init, "Sony", "HB-F700F (MSX2, France)", 0)
COMP(1986, hbf700p,    0,        0,     hbf700p,    msx2uk,   msx2_state, empty_init, "Sony", "HB-F700P (MSX2, Europe)", 0)
COMP(1986, hbf700s,    hbf700p,  0,     hbf700s,    msx2sp,   msx2_state, empty_init, "Sony", "HB-F700S (MSX2, Spain)", 0)
COMP(1986, hbf900,     hbf900a,  0,     hbf900,     msx2jp,   msx2_state, empty_init, "Sony", "HB-F900 (MSX2, Japan)", 0)
COMP(1986, hbf900a,    0,        0,     hbf900a,    msx2jp,   msx2_state, empty_init, "Sony", "HB-F900 (alt) (MSX2, Japan)", 0)
COMP(1987, hbg900ap,   hbg900p,  0,     hbg900ap,   msx2uk,   msx2_state, empty_init, "Sony", "HB-G900AP (MSX2, Europe)", MACHINE_NOT_WORKING) // rs232 not communicating
COMP(1986, hbg900p,    0,        0,     hbg900p,    msx2uk,   msx2_state, empty_init, "Sony", "HB-G900P (MSX2, Europe)", MACHINE_NOT_WORKING) // rs232 not communicating
COMP(1987, tpc310,     0,        0,     tpc310,     msxsp,    msx2_state, empty_init, "Talent", "TPC-310 (MSX2, Argentina)", 0)
COMP(1987, tpp311,     0,        0,     tpp311,     msxsp,    msx2_state, empty_init, "Talent", "TPP-311 (MSX2, Argentina)", 0)
COMP(1987, tps312,     0,        0,     tps312,     msxsp,    msx2_state, empty_init, "Talent", "TPS-312 (MSX2, Argentina)", 0)
COMP(1985, hx23,       hx23f,    0,     hx23,       msxjp,    msx2_state, empty_init, "Toshiba", "HX-23 (MSX2, Japan)", MACHINE_NOT_WORKING) // firmware goes into an infinite loop on the title screen
COMP(1985, hx23f,      0,        0,     hx23f,      msxjp,    msx2_state, empty_init, "Toshiba", "HX-23F (MSX2, Japan)", MACHINE_NOT_WORKING) // firmware goes into an infinite loop on the title screen
COMP(1985, hx33,       hx34,     0,     hx33,       msxjp,    msx2_state, empty_init, "Toshiba", "HX-33 w/HX-R702 (MSX2, Japan)", MACHINE_NOT_WORKING) // half the pixels are missing in the firmware?
COMP(1985, hx34,       0,        0,     hx34,       msx2jp,   msx2_state, empty_init, "Toshiba", "HX-34 w/HX-R703 (MSX2, Japan)", 0)
COMP(1986, fstm1,      0,        0,     fstm1,      msx,      msx2_state, empty_init, "Toshiba", "FS-TM1 (MSX2, Italy)", 0)
COMP(1986, victhc80,   0,        0,     victhc80,   msxjp,    msx2_state, empty_init, "Victor", "HC-80 (MSX2, Japan)", 0)
COMP(1986, victhc90,   victhc95, 0,     victhc90,   msx2jp,   msx2_state, empty_init, "Victor", "HC-90 (MSX2, Japan)", MACHINE_NOT_WORKING) // 2nd cpu/turbo not emulated, firmware won't start
COMP(1986, victhc90a,  victhc95, 0,     victhc90a,  msx2jp,   msx2_state, empty_init, "Victor", "HC-90A (MSX2, Japan)", MACHINE_NOT_WORKING) // 2nd cpu/turbo not emulated
COMP(1986, victhc95,   0,        0,     victhc95,   msx2jp,   msx2_state, empty_init, "Victor", "HC-95 (MSX2, Japan)", MACHINE_NOT_WORKING) // 2nd cpu/turbo not emulated, firmware won't start
COMP(1986, victhc95a,  victhc95, 0,     victhc95a,  msx2jp,   msx2_state, empty_init, "Victor", "HC-95A (MSX2, Japan)", MACHINE_NOT_WORKING) // 2nd cpu/turbo not emulated, firmware won't start
COMP(1985, cx7128,     cx7m128,  0,     cx7128,     msxjp,    msx2_state, empty_init, "Yamaha", "CX7/128 (MSX2, Japan)", 0)
COMP(1985, cx7m128,    0,        0,     cx7m128,    msxjp,    msx2_state, empty_init, "Yamaha", "CX7M/128 (MSX2, Japan)", 0)
COMP(1985, y503iiir,   0,        0,     y503iiir,   msxru,    msx2_state, empty_init, "Yamaha", "YIS-503 III R (MSX2, USSR)", MACHINE_NOT_WORKING) // network not implemented
COMP(198?, y503iiire,  y503iiir, 0,     y503iiir,   msx2,     msx2_state, empty_init, "Yamaha", "YIS-503 III R (MSX2, Estonian)", MACHINE_NOT_WORKING) // network not implemented
COMP(1985, yis604,     0,        0,     yis604,     msx2jp,   msx2_state, empty_init, "Yamaha", "YIS604/128 (MSX2, Japan)", 0)
COMP(1986, y805128,    y805256,  0,     y805128,    msx2jp,   msx2_state, empty_init, "Yamaha", "YIS805/128 (MSX2, Japan)", MACHINE_NOT_WORKING) // Floppy support broken
COMP(1986, y805128r2,  y805256,  0,     y805128r2,  msx2,     msx2_state, empty_init, "Yamaha", "YIS805/128R2 (MSX2, USSR)", MACHINE_NOT_WORKING) // Network not implemented
COMP(198?, y805128r2e, y805256,  0,     y805128r2,  y503iir2, msx2_state, empty_init, "Yamaha", "YIS805/128R2 (MSX2, Estonian)", MACHINE_NOT_WORKING) // Network not implemented
COMP(198?, y805256,    0,        0,     y805256,    msx2jp,   msx2_state, empty_init, "Yamaha", "YIS805/256 (MSX2, Japan)", MACHINE_NOT_WORKING) // Floppy support broken?
