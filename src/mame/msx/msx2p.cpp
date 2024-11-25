// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#include "emu.h"
#include "msx.h"
#include "msx_keyboard.h"
#include "msx_matsushita.h"
#include "msx_s1985.h"
#include "msx_systemflags.h"
#include "bus/msx/slot/disk.h"
#include "bus/msx/slot/music.h"
#include "bus/msx/slot/panasonic08.h"
#include "bus/msx/slot/ram_mm.h"
#include "bus/msx/slot/sony08.h"
#include "softlist_dev.h"

#include "msx_jp.lh"
#include "msx_jp_1fdd.lh"
#include "msx_jp_2fdd.lh"
#include "msx_nocode_1fdd.lh"

using namespace msx_keyboard;


/***************************************************************************

  MSX2+ machines

***************************************************************************/

namespace {

class msx2p_state : public msx2p_base_state
{
public:
	msx2p_state(const machine_config &mconfig, device_type type, const char *tag)
		: msx2p_base_state(mconfig, type, tag, 21.477272_MHz_XTAL, 6)
	{
	}

	// MSX2+ machines
	// Did the expert machines have the f4 boot flags i/o port? (not referenced from system bios)
	void expert3i(machine_config &config);
	void expert3t(machine_config &config);
	void expertac(machine_config &config);
	void expertdx(machine_config &config);
	void fsa1fx(machine_config &config);
	void fsa1wsx(machine_config &config);
	void fsa1wx(machine_config &config);
	void fsa1wxa(machine_config &config);
	void phc70fd(machine_config &config);
	void phc70fd2(machine_config &config);
	void phc35j(machine_config &config);
	void hbf1xdj(machine_config &config);
	void hbf1xv(machine_config &config);
};

/********************************  MSX 2+ **********************************/

/* MSX2+ - Ciel Expert 3 IDE */

ROM_START(expert3i)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("exp30bios.rom", 0x0000,  0x8000, CRC(a10bb1ce) SHA1(5029cf47031b22bd5d1f68ebfd3be6d6da56dfe9))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("exp30ext.rom", 0x0000, 0x4000, CRC(6bcf4100) SHA1(cc1744c6c513d6409a142b4fb42fbe70a95d9b7f))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("cieldisk.rom", 0x0000, 0x4000, CRC(bb550b09) SHA1(0274dd9b5096065a7f4ed019101124c9bd1d56b8))

	ROM_REGION(0x4000, "music", 0)
	ROM_LOAD("exp30mus.rom", 0x0000, 0x4000, CRC(9881b3fd) SHA1(befebc916bfdb5e8057040f0ae82b5517a7750db))

	ROM_REGION(0x10000, "ide", 0)
	ROM_LOAD("ide240a.rom", 0x00000, 0x10000, CRC(7adf857f) SHA1(8a919dbeed92db8c06a611279efaed8552810239))
ROM_END

void msx2p_state::expert3i(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: wd2793, 1 or 2? drives
	// 2 Cartridge slots?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 1, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_MUSIC, "music", 1, 1, 1, 1, "music").set_ym2413_tag(m_ym2413);
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793, "disk", 1, 2, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_ROM, "ide", 1, 3, 0, 4, "ide");         /* IDE hardware needs to be emulated */
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 2, 0, 4).set_total_size(0x40000);       // 256KB?? Mapper RAM
	add_cartridge_slot<2>(config, 3);

	msx_ym2413(config);

	msx2plus(SND_AY8910, config, layout_msx_nocode_1fdd);
}

/* MSX2+ - Ciel Expert 3 Turbo
This one is a full motherboard by CIEL (not an upgrade kit), created to replace the motherboard of a Gradiente Expert (which means that only the case, the analog boards and the keyboard remains Gradiente). This new motherboard has the following built-in features:

1) MSX2+
2) Support either 3.57MHz or 7.14MHz natively, switched either by software (*1) or by a hardware-switch on the front panel. Turbo-led included.
3) Up to 4MB of Memory Mapper (1MB is the most common configuration)
4) MSX-Music
5) 4 expansion slots (two external on the front panel, two internal)
6) Stereo sound (YM2413 channels 0-6 on right, PSG+YM2413 channels 7-9 on left)
7) Support the V9938 instead of the V9958 by switching some jumpers
8) The main-ram can be placed on slot 2 or slot 3, using jumpers (slot 2 is the default)


*1: A routine hidden inside the BIOS frame-0 is used to switch the turbo.
 */

/* Uses a Z84C0010 - CMOS processor working at 7 MHz */
ROM_START(expert3t)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("exp30bios.rom", 0x0000, 0x8000, CRC(a10bb1ce) SHA1(5029cf47031b22bd5d1f68ebfd3be6d6da56dfe9))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("exp30ext.rom", 0x0000, 0x4000, CRC(6bcf4100) SHA1(cc1744c6c513d6409a142b4fb42fbe70a95d9b7f))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("cieldisk.rom", 0x0000, 0x4000, CRC(bb550b09) SHA1(0274dd9b5096065a7f4ed019101124c9bd1d56b8))

	ROM_REGION(0x4000, "music", 0)
	ROM_LOAD("exp30mus.rom", 0x0000, 0x4000, CRC(9881b3fd) SHA1(befebc916bfdb5e8057040f0ae82b5517a7750db))

	ROM_REGION(0x4000, "turbo", 0)
	ROM_LOAD("turbo.rom", 0x0000, 0x4000, CRC(ab528416) SHA1(d468604269ae7664ac739ea9f922a05e14ffa3d1))
ROM_END

void msx2p_state::expert3t(machine_config &config)
{
	// AY8910
	// FDC: wd2793?, 1 or 2? drives
	// 4 Cartridge/Expansion slots?
	// FM/YM2413 built-in

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1, 0);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 1, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_MUSIC, "music", 1, 1, 1, 1, "music").set_ym2413_tag(m_ym2413);
	add_internal_slot(config, MSX_SLOT_ROM, "turbo", 1, 2, 1, 1, "turbo");          /* Turbo hardware needs to be emulated */
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793, "disk", 1, 3, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 2, 0, 4).set_total_size(0x40000);       // 256KB Mapper RAM
	add_cartridge_slot<2>(config, 3);

	msx_ym2413(config);

	msx2plus(SND_AY8910, config, layout_msx_nocode_1fdd);
}

/* MSX2+ - Gradiente Expert AC88+ */

ROM_START(expertac)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ac88bios.rom", 0x0000, 0x8000, CRC(9ce0da44) SHA1(1fc2306911ab6e1ebdf7cb8c3c34a7f116414e88))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("ac88ext.rom", 0x0000, 0x4000, CRC(c74c005c) SHA1(d5528825c7eea2cfeadd64db1dbdbe1344478fc6))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("panadisk.rom", 0x0000, 0x4000, CRC(17fa392b) SHA1(7ed7c55e0359737ac5e68d38cb6903f9e5d7c2b6))

	ROM_REGION(0x4000, "asm", 0)
	ROM_LOAD("ac88asm.rom", 0x0000, 0x4000, CRC(a8a955ae) SHA1(91e522473a8470511584df3ee5b325ea5e2b81ef))

	ROM_REGION(0x4000, "xbasic", 0)
	ROM_LOAD("xbasic2.rom", 0x0000, 0x4000, CRC(2825b1a0) SHA1(47370bec7ca1f0615a54eda548b07fbc0c7ef398))
ROM_END

void msx2p_state::expertac(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: wd2793?, 1 or 2? drives
	// 2 Cartridge slots?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000);   // 64KB Mapper RAM??
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "asm", 3, 1, 1, 1, "asm");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793, "disk", 3, 2, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_ROM, "xbasic", 3, 3, 1, 1, "xbasic");

	msx2plus(SND_AY8910, config, layout_msx_nocode_1fdd);
}

/* MSX2+ - Gradiente Expert DDX+ */

ROM_START(expertdx)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ddxbios.rom", 0x0000, 0x8000, CRC(e00af3dc) SHA1(5c463dd990582e677c8206f61035a7c54d8c67f0))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("ddxext.rom", 0x0000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("panadisk.rom", 0x0000, 0x4000, CRC(17fa392b) SHA1(7ed7c55e0359737ac5e68d38cb6903f9e5d7c2b6))

	ROM_REGION(0x4000, "xbasic", 0)
	ROM_LOAD("xbasic2.rom", 0x0000, 0x4000, CRC(2825b1a0) SHA1(47370bec7ca1f0615a54eda548b07fbc0c7ef398))

	ROM_REGION(0x8000, "kanjirom", 0)
	ROM_LOAD("kanji.rom", 0x0000, 0x8000, CRC(b4fc574d) SHA1(dcc3a67732aa01c4f2ee8d1ad886444a4dbafe06))
ROM_END

void msx2p_state::expertdx(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: tc8566af, 1 3.5" DSDD drive?
	// 2 Cartridge slots?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1, 0);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 1, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "xbasic", 1, 2, 1, 1, "xbasic");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK3_TC8566, "disk", 1, 3, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 2, 0, 4).set_total_size(0x10000);   // 64KB Mapper RAM??
	add_cartridge_slot<2>(config, 3);
	/* Kanji? */

	msx2plus(SND_AY8910, config, layout_msx_nocode_1fdd);
}

/* MSX2+ - Panasonic FS-A1FX */

ROM_START(fsa1fx)
	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD("a1fx.ic16", 0, 0x40000, CRC(c0b2d882) SHA1(623cbca109b6410df08ee7062150a6bda4b5d5d4))

	// Kanji rom contents are the first half of the single rom
//  ROM_REGION(0x20000, "kanji", 0)
	ROM_REGION(0x40000, "kanji", 0)
	ROM_LOAD("a1fx.ic16", 0, 0x40000, CRC(c0b2d882) SHA1(623cbca109b6410df08ee7062150a6bda4b5d5d4))
ROM_END

void msx2p_state::fsa1fx(machine_config &config)
{
	// AY8910 (in T9769)
	// FDC: tc8566af, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// T9769(B)
	// ren-sha turbo slider
	// pause button
	// firmware switch

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "maincpu", 0x30000);
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000).set_unused_bits(0x80);   // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "maincpu", 0x38000);
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "maincpu", 0x28000);
	add_internal_disk_mirrored(config, MSX_SLOT_DISK3_TC8566, "disk", 3, 2, 1, 2, "maincpu", 0x3c000);
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 3, 1, 2, "maincpu", 0x20000);

	msx_matsushita_device &matsushita(MSX_MATSUSHITA(config, "matsushita", 0));
	matsushita.turbo_callback().set([this] (int state) {
		// 0 - 5.369317 MHz
		// 1 - 3.579545 MHz
		m_maincpu->set_unscaled_clock(m_main_xtal / (state ? 6 : 4));
	});

	MSX_SYSTEMFLAGS(config, "sysflags", m_maincpu, 0xff);
	set_cold_boot_flags(0xff);

	m_kanji_fsa1fx = true;
	msx2plus(SND_AY8910, config, layout_msx_jp_1fdd);
}

/* MSX2+ - Panasonic FS-A1WSX */

ROM_START(fsa1wsx)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("a1wsbios.rom", 0x0000, 0x8000, CRC(358ec547) SHA1(f4433752d3bf876bfefb363c749d4d2e08a218b6))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("a1wsext.rom", 0x0000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("a1wsdisk.rom", 0x0000, 0x4000, CRC(17fa392b) SHA1(7ed7c55e0359737ac5e68d38cb6903f9e5d7c2b6))

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("a1wskdr.rom", 0x0000, 0x8000, CRC(b4fc574d) SHA1(dcc3a67732aa01c4f2ee8d1ad886444a4dbafe06))

	ROM_REGION(0x4000, "msxmusic", 0)
	ROM_LOAD("a1wsmusp.rom", 0x0000, 0x4000, CRC(5c32eb29) SHA1(aad42ba4289b33d8eed225d42cea930b7fc5c228))

	ROM_REGION(0x200000, "firmware", 0)
	ROM_LOAD("a1wsfirm.rom", 0x000000, 0x200000, CRC(e363595d) SHA1(3330d9b6b76e3c4ccb7cf252496ed15d08b95d3f))

	ROM_REGION(0x40000, "kanji", 0)
	ROM_LOAD("a1wskfn.rom", 0, 0x40000, CRC(1f6406fb) SHA1(5aff2d9b6efc723bc395b0f96f0adfa83cc54a49))
ROM_END

void msx2p_state::fsa1wsx(machine_config &config)
{
	// AY8910 (in T9769)
	// FDC: tc8566af, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// FM built-in
	// T9769(C)
	// ren-sha turbo slider
	// firmware switch
	// pause button

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_MUSIC, "msxmusic", 0, 2, 1, 1, "msxmusic").set_ym2413_tag(m_ym2413);
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000).set_unused_bits(0x80);   // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "kdr");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK3_TC8566, "disk", 3, 2, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_PANASONIC08, "firmware", 3, 3, 0, 4, "firmware");

	msx_matsushita_device &matsushita(MSX_MATSUSHITA(config, "matsushita", 0));
	matsushita.turbo_callback().set([this] (int state) {
		// 0 - 5.369317 MHz
		// 1 - 3.579545 MHz
		m_maincpu->set_unscaled_clock(m_main_xtal / (state ? 6 : 4));
	});

	MSX_SYSTEMFLAGS(config, "sysflags", m_maincpu, 0xff);
	set_cold_boot_flags(0xff);

	msx_ym2413(config);

	msx2plus(SND_AY8910, config, layout_msx_jp_1fdd);
}

/* MSX2+ - Panasonic FS-A1WX */

ROM_START(fsa1wx)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("a1wxbios.rom", 0x0000, 0x8000, CRC(19771608) SHA1(e90f80a61d94c617850c415e12ad70ac41e66bb7))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("a1wxext.rom", 0x0000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("a1wxdisk.rom", 0x0000, 0x4000, CRC(905daa1b) SHA1(bb59c849898d46a23fdbd0cc04ab35088e74a18d))

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("a1wxkdr.rom", 0x0000, 0x8000, CRC(a068cba9) SHA1(1ef3956f7f918873fb9b031339bba45d1e5e5878))

	ROM_REGION(0x4000, "msxmusic", 0)
	ROM_LOAD("a1wxmusp.rom", 0x0000, 0x4000, CRC(456e494e) SHA1(6354ccc5c100b1c558c9395fa8c00784d2e9b0a3))

	ROM_REGION(0x200000, "firmware", 0)
	ROM_LOAD("a1wxfirm.rom", 0x000000, 0x200000, CRC(283f3250) SHA1(d37ab4bd2bfddd8c97476cbe7347ae581a6f2972))

	ROM_REGION(0x40000, "kanji", 0)
	ROM_LOAD("a1wxkfn.rom", 0, 0x40000, CRC(1f6406fb) SHA1(5aff2d9b6efc723bc395b0f96f0adfa83cc54a49))
ROM_END

void msx2p_state::fsa1wx(machine_config &config)
{
	// AY8910 (in T9769)
	// FDC: tc8566af, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// FM built-in
	// MSX Engine T9769A/B
	// ren-sha turbo slider
	// firmware switch
	// pause button

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_MUSIC, "msxmusic", 0, 2, 1, 1, "msxmusic").set_ym2413_tag(m_ym2413);
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000).set_unused_bits(0x80);   // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "kdr");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK3_TC8566, "disk", 3, 2, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_PANASONIC08, "firmware", 3, 3, 0, 4, "firmware");

	msx_matsushita_device &matsushita(MSX_MATSUSHITA(config, "matsushita", 0));
	matsushita.turbo_callback().set([this] (int state) {
		// 0 - 5.369317 MHz
		// 1 - 3.579545 MHz
		m_maincpu->set_unscaled_clock(m_main_xtal / (state ? 6 : 4));
	});

	MSX_SYSTEMFLAGS(config, "sysflags", m_maincpu, 0xff);
	set_cold_boot_flags(0xff);

	msx_ym2413(config);

	msx2plus(SND_AY8910, config, layout_msx_jp_1fdd);
}

/* MSX2+ - Panasonic FS-A1WX (a) */

ROM_START(fsa1wxa)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("a1wxbios.rom", 0x0000, 0x8000, CRC(19771608) SHA1(e90f80a61d94c617850c415e12ad70ac41e66bb7))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("a1wxext.rom", 0x0000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("a1wxdisk.rom", 0x0000, 0x4000, CRC(2bda0184) SHA1(2a0d228afde36ac7c5d3c2aac9c7c664dd071a8c))

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("a1wxkdr.rom", 0x0000, 0x8000, CRC(a068cba9) SHA1(1ef3956f7f918873fb9b031339bba45d1e5e5878))

	ROM_REGION(0x4000, "msxmusic", 0)
	ROM_LOAD("a1wxmusp.rom", 0x0000, 0x4000, CRC(456e494e) SHA1(6354ccc5c100b1c558c9395fa8c00784d2e9b0a3))

	ROM_REGION(0x200000, "firmware", 0)
	ROM_LOAD("a1wxfira.rom", 0x000000, 0x200000, CRC(58440a8e) SHA1(8e0d4a77e7d5736e8225c2df4701509363eb230f))

	ROM_REGION(0x40000, "kanji", 0)
	ROM_LOAD("a1wxkfn.rom", 0, 0x40000, CRC(1f6406fb) SHA1(5aff2d9b6efc723bc395b0f96f0adfa83cc54a49))
ROM_END

/* MSX2+ - Sanyo Wavy PHC-35J */

ROM_START(phc35j)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("35jbios.rom", 0x0000, 0x8000, CRC(358ec547) SHA1(f4433752d3bf876bfefb363c749d4d2e08a218b6))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("35jext.rom", 0x0000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("35jkdr.rom", 0x0000, 0x8000, CRC(b4fc574d) SHA1(dcc3a67732aa01c4f2ee8d1ad886444a4dbafe06))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("35jkfn.rom", 0, 0x20000, CRC(c9651b32) SHA1(84a645becec0a25d3ab7a909cde1b242699a8662))
ROM_END

void msx2p_state::phc35j(machine_config &config)
{
	// AY8910 (in T9769)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// T9769A
	// ren-sha turbo slider
	// pause button

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000).set_unused_bits(0x80);   // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "kdr");

	MSX_SYSTEMFLAGS(config, "sysflags", m_maincpu, 0xff);
	set_cold_boot_flags(0xff);
	msx2plus(SND_AY8910, config, layout_msx_jp);
}

/* MSX2+ - Sanyo Wavy PHC-70FD */

ROM_START(phc70fd)
	ROM_REGION(0x20000, "mainrom", 0)
	ROM_LOAD("phc-70fd.rom", 0x0000, 0x20000, CRC(d2307ddf) SHA1(b6f2ca2e8a18d6c7cd326cb8d1a1d7d747f23059))
//  ROM_LOAD("70fdbios.rom", 0x0000, 0x8000, CRC(19771608) SHA1(e90f80a61d94c617850c415e12ad70ac41e66bb7))
//  ROM_LOAD("70fdext.rom",  0x8000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))
//  ROM_LOAD("70fddisk.rom", 0xc000, 0x4000, CRC(db7f1125) SHA1(9efa744be8355675e7bfdd3976bbbfaf85d62e1d))
//  ROM_LOAD("70fdkdr.rom", 0x10000, 0x8000, CRC(a068cba9) SHA1(1ef3956f7f918873fb9b031339bba45d1e5e5878))
//  ROM_LOAD("70fdmus.rom", 0x18000, 0x4000, CRC(5c32eb29) SHA1(aad42ba4289b33d8eed225d42cea930b7fc5c228))
//  ROM_LOAD("70fdbas.rom", 0x1c000, 0x4000, CRC(da7be246) SHA1(22b3191d865010264001b9d896186a9818478a6b))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("70fdkfn.rom", 0, 0x20000, CRC(c9651b32) SHA1(84a645becec0a25d3ab7a909cde1b242699a8662))
ROM_END

void msx2p_state::phc70fd(machine_config &config)
{
	// AY8910 (in T9769)
	// FDC: tc8566af, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// T9769
	// FM built-in
	// ren-sha turbo slider
	// pause button

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom", 0x10000);
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000).set_unused_bits(0x80);   // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "mainrom", 0x18000);
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "mainrom", 0x8000);
	add_internal_disk_mirrored(config, MSX_SLOT_DISK3_TC8566, "disk", 3, 2, 1, 2, "mainrom", 0x1c000);
	add_internal_slot(config, MSX_SLOT_MUSIC, "msxmusic", 3, 3, 1, 1, "mainrom", 0x00000).set_ym2413_tag(m_ym2413);
	add_internal_slot(config, MSX_SLOT_ROM, "basickun", 3, 3, 2, 1, "mainrom", 0x04000);

	MSX_SYSTEMFLAGS(config, "sysflags", m_maincpu, 0xff);
	set_cold_boot_flags(0xff);

	msx_ym2413(config);

	msx2plus(SND_AY8910, config, layout_msx_jp_1fdd);
}

/* MSX2+ - Sanyo Wavy PHC-70FD2 */

ROM_START(phc70fd2)
	ROM_REGION(0x20000, "mainrom", 0)
	ROM_LOAD("70f2bios.rom", 0x0000, 0x8000, CRC(19771608) SHA1(e90f80a61d94c617850c415e12ad70ac41e66bb7))

	ROM_REGION(0x4000, "subrom", 0)
	ROM_LOAD("70f2ext.rom", 0x0000, 0x4000, CRC(b8ba44d3) SHA1(fe0254cbfc11405b79e7c86c7769bd6322b04995))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("70f2disk.rom", 0x0000, 0x4000, CRC(db7f1125) SHA1(9efa744be8355675e7bfdd3976bbbfaf85d62e1d))

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("70f2kdr.rom", 0x0000, 0x8000, CRC(a068cba9) SHA1(1ef3956f7f918873fb9b031339bba45d1e5e5878))

	ROM_REGION(0x4000, "msxmusic", 0)
	ROM_LOAD("70f2mus.rom", 0x0000, 0x4000, CRC(5c32eb29) SHA1(aad42ba4289b33d8eed225d42cea930b7fc5c228))

	ROM_REGION(0x4000, "basickun", 0)
	ROM_LOAD("70f2bas.rom", 0x0000, 0x4000, CRC(da7be246) SHA1(22b3191d865010264001b9d896186a9818478a6b))

	ROM_REGION(0x40000, "kanji", 0)
	ROM_LOAD("70f2kfn.rom", 0, 0x40000, CRC(9a850db9) SHA1(bcdb4dae303dfe5234f372d70a5e0271d3202c36))
ROM_END

void msx2p_state::phc70fd2(machine_config &config)
{
	// AY8910
	// FDC: tc8566af, 2 3.5" DSDD drives
	// 2 Cartridge slots
	// FM built-in
	// T9769

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000).set_unused_bits(0x80);   // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "subrom");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "kdr");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK3_TC8566_2_DRIVES, "disk", 3, 2, 1, 2, "diskrom");
	add_internal_slot(config, MSX_SLOT_MUSIC, "msxmusic", 3, 3, 1, 1, "msxmusic").set_ym2413_tag(m_ym2413);
	add_internal_slot(config, MSX_SLOT_ROM, "basickun", 3, 3, 2, 1, "basickun");

	MSX_SYSTEMFLAGS(config, "sysflags", m_maincpu, 0xff);
	set_cold_boot_flags(0xff);

	msx_ym2413(config);

	msx2plus(SND_AY8910, config, layout_msx_jp_2fdd);
}

/* MSX2+ - Sony HB-F1XDJ */

ROM_START(hbf1xdj)
	ROM_REGION(0x20000, "mainrom", 0)
	ROM_LOAD("hb-f1xdj_main.rom", 0x0000, 0x20000, CRC(d89bab74) SHA1(f2a1d326d72d4c70ea214d7883838de8847a82b7))

	ROM_REGION(0x100000, "firmware", 0)
	ROM_LOAD("f1xjfirm.rom", 0x000000, 0x100000, CRC(77be583f) SHA1(ade0c5ba5574f8114d7079050317099b4519e88f))

	ROM_REGION(0x40000, "kanji", 0)
	ROM_LOAD("f1xjkfn.rom", 0, 0x40000, CRC(7016dfd0) SHA1(218d91eb6df2823c924d3774a9f455492a10aecb))
ROM_END

void msx2p_state::hbf1xdj(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: MB89311, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// FM built-in
	// S-1985 MSX Engine
	// speed controller
	// pause button
	// ren-sha turbo

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_SONY08, "firmware", 0, 3, 0, 4, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000).set_unused_bits(0x80);   // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "mainrom", 0x8000);
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "mainrom", 0x10000);
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_N, "disk", 3, 2, 1, 2, "mainrom", 0xc000);
	add_internal_slot(config, MSX_SLOT_MUSIC, "msxmusic", 3, 3, 1, 1, "mainrom", 0x18000).set_ym2413_tag(m_ym2413);

	MSX_SYSTEMFLAGS(config, "sysflags", m_maincpu, 0x00);

	MSX_S1985(config, "s1985", 0);

	msx_ym2413(config);

	msx2plus(SND_YM2149, config, layout_msx_jp_1fdd);
}

/* MSX2+ - Sony HB-F1XV */

ROM_START(hbf1xv)
	ROM_REGION(0x20000, "mainrom", 0)
	ROM_LOAD("hb-f1xdj_main.rom", 0x0000, 0x20000, CRC(d89bab74) SHA1(f2a1d326d72d4c70ea214d7883838de8847a82b7))

	ROM_REGION(0x100000, "firmware", 0)
	ROM_LOAD("f1xvfirm.rom", 0x0, 0x100000, CRC(77be583f) SHA1(ade0c5ba5574f8114d7079050317099b4519e88f))

	ROM_REGION(0x40000, "kanji", 0)
	ROM_LOAD("f1xvkfn.rom", 0, 0x40000, CRC(7016dfd0) SHA1(218d91eb6df2823c924d3774a9f455492a10aecb))
ROM_END

void msx2p_state::hbf1xv(machine_config &config)
{
	// YM2149 (in S-1985 MSX Engine)
	// FDC: mb89311, 1 3.5" DSDD drives
	// 2 Cartridge slots
	// FM built-in
	// S-1985 MSX Engine
	// speed controller
	// pause button
	// ren-sha turbo

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_SONY08, "firmware", 0, 3, 0, 4, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x10000).set_unused_bits(0x80);   // 64KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 1, 0, 1, "mainrom", 0x8000);
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "mainrom", 0x10000);
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_N, "disk", 3, 2, 1, 2, "mainrom", 0xc000);
	add_internal_slot(config, MSX_SLOT_MUSIC, "msxmusic", 3, 3, 1, 1, "mainrom", 0x18000).set_ym2413_tag(m_ym2413);

	MSX_SYSTEMFLAGS(config, "sysflags", m_maincpu, 0x00);

	MSX_S1985(config, "s1985", 0);

	msx_ym2413(config);

	msx2plus(SND_YM2149, config, layout_msx_jp_1fdd);
}

} // anonymous namespace

COMP(19??, expert3i,   0,        0,     expert3i,   msx2,     msx2p_state, empty_init, "Ciel", "Expert 3 IDE (MSX2+, Brazil)", MACHINE_NOT_WORKING) // Some hardware not emulated
COMP(1996, expert3t,   0,        0,     expert3t,   msx2,     msx2p_state, empty_init, "Ciel", "Expert 3 Turbo (MSX2+, Brazil)", MACHINE_NOT_WORKING) // Some hardware not emulated
COMP(19??, expertac,   0,        0,     expertac,   msx2,     msx2p_state, empty_init, "Gradiente", "Expert AC88+ (MSX2+, Brazil)", MACHINE_NOT_WORKING) // Some hardware not emulated
COMP(19??, expertdx,   0,        0,     expertdx,   msx2,     msx2p_state, empty_init, "Gradiente", "Expert DDX+ (MSX2+, Brazil)", MACHINE_NOT_WORKING) // Some hardware not emulated
COMP(1988, fsa1fx,     0,        0,     fsa1fx,     msx2jp,   msx2p_state, empty_init, "Panasonic", "FS-A1FX (MSX2+, Japan)", 0)
COMP(1989, fsa1wsx,    0,        0,     fsa1wsx,    msx2jp,   msx2p_state, empty_init, "Panasonic", "FS-A1WSX (MSX2+, Japan)", 0)
COMP(1988, fsa1wx,     fsa1wxa,  0,     fsa1wx,     msx2jp,   msx2p_state, empty_init, "Panasonic", "FS-A1WX / 1st released version (MSX2+, Japan)", 0)
COMP(1988, fsa1wxa,    0,        0,     fsa1wx,     msx2jp,   msx2p_state, empty_init, "Panasonic", "FS-A1WX / 2nd released version (MSX2+, Japan)", 0)
COMP(1988, phc70fd,    phc70fd2, 0,     phc70fd,    msx2jp,   msx2p_state, empty_init, "Sanyo", "PHC-70FD / Wavy70FD (MSX2+, Japan)", 0)
COMP(1989, phc70fd2,   0,        0,     phc70fd2,   msx2jp,   msx2p_state, empty_init, "Sanyo", "PHC-70FD2 / Wavy70FD2 (MSX2+, Japan)", 0)
COMP(1989, phc35j,     0,        0,     phc35j,     msx2jp,   msx2p_state, empty_init, "Sanyo", "PHC-35J / Wavy35 (MSX2+, Japan)", 0)
COMP(1988, hbf1xdj,    0,        0,     hbf1xdj,    msx2jp,   msx2p_state, empty_init, "Sony", "HB-F1XDJ (MSX2+, Japan)", 0)
COMP(1989, hbf1xv,     0,        0,     hbf1xv,     msx2jp,   msx2p_state, empty_init, "Sony", "HB-F1XV (MSX2+, Japan)", 0)
