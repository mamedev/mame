// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#include "emu.h"
#include "msx.h"
#include "msx_keyboard.h"
#include "bus/msx/slot/disk.h"
#include "bus/msx/slot/msx_rs232.h"
#include "bus/msx/slot/ram.h"
#include "bus/msx/slot/ram_mm.h"
#include "bus/msx/slot/rom.h"
#include "softlist_dev.h"

#include "msx_ar.lh"
#include "msx_ar_1fdd.lh"
#include "msx_en.lh"
#include "msx_ru.lh"
#include "msx_nocode.lh"
#include "msx_nocode_1fdd.lh"

using namespace msx_keyboard;

/***************************************************************************

  MSX1 with v9938 Game drivers

Undumped and/or not emulated:
- Sakhr AX-200 (Arabic/French)
- Spectravideo SVI-738 (German)
- Yamaha CX5MII-128A (Australia, New Zealand)
- Yamaha CX5MII-128 C (Canada)
- Yamaha CX5MII-128 E (UK)
- Yamaha CX5MII-128 F (France)
- Yamaha CX5MII-128 G (Germany)
- Yamaha CX5MII-128 P (Spain)
- Yamaha CX5MII-128 S (Scandinavia)
- Yamaha CX5MII-128 U (USA)
- Yamaha CX5MIIA (Australia, New Zealand)
- Yamaha CX5MIIC (Canada)
- Yamaha CX5MIIE (UK)
- Yamaha CX5MIIF (France)
- Yamaha CX5MIIG (Germany)
- Yamaha CX5MIIP (Spain)
- Yamaha CX5MIIS (Scandinavia)
- Yamaha CX5MIIU (USA)
***************************************************************************/

namespace {

class msx1_v9938_state : public msx_state
{
public:
	msx1_v9938_state(const machine_config &mconfig, device_type type, const char *tag);

	void ax200(machine_config &mconfig);
	void ax200m(machine_config &mconfig);
	void cx5m128(machine_config &config);
	void cx5miib(machine_config &config);
	void svi738(machine_config &config);
	void svi738ar(machine_config &config);
	void tadpc200a(machine_config &config);
	void y503iir(machine_config &config);
	void y503iir2(machine_config &config);
	void yis503ii(machine_config &config);

protected:
	void msx1_v9938(ay8910_type ay8910_type, machine_config &config, const internal_layout &layout);
	void msx1_v9938_pal(ay8910_type ay8910_type, machine_config &config, const internal_layout &layout);
	void svi738_base(machine_config &config, const internal_layout &layout);

	void io_map(address_map &map) ATTR_COLD;

	required_device<v9938_device> m_v9938;
};

msx1_v9938_state::msx1_v9938_state(const machine_config &mconfig, device_type type, const char *tag)
	: msx_state(mconfig, type, tag, 21.477272_MHz_XTAL, 6)
	, m_v9938(*this, "v9938")
{
}

void msx1_v9938_state::io_map(address_map &map)
{
	msx_base_io_map(map);
	map(0x98, 0x9b).rw(m_v9938, FUNC(v9938_device::read), FUNC(v9938_device::write));
}

void msx1_v9938_state::msx1_v9938(ay8910_type ay8910_type, machine_config &config, const internal_layout &layout)
{
	msx_base(ay8910_type, config, layout);

	m_maincpu->set_addrmap(AS_IO, &msx1_v9938_state::io_map);

	// video hardware
	V9938(config, m_v9938, 21.477272_MHz_XTAL);
	m_v9938->set_screen_ntsc(m_screen);
	m_v9938->set_vram_size(0x4000);
	m_v9938->int_cb().set(m_mainirq, FUNC(input_merger_device::in_w<0>));

	// Software lists
	msx1_add_softlists(config);
}

void msx1_v9938_state::msx1_v9938_pal(ay8910_type ay8910_type, machine_config &config, const internal_layout &layout)
{
	msx1_v9938(ay8910_type, config, layout);
	m_v9938->set_screen_pal(m_screen);
}

/* MSX - Sakhr AX-200 (Arabic/English) */

ROM_START (ax200)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ax200bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(cae98b30) SHA1(079c018739c37485f3d64ef2145a0267fce6e20e)) // need verification

	ROM_REGION(0x8000, "arabic", 0)
	ROM_LOAD("ax200arab.rom", 0x0000, 0x8000, BAD_DUMP CRC(b041e610) SHA1(7574cc5655805ea316011a8123b064917f06f83c)) // need verification
ROM_END

void msx1_v9938_state::ax200(machine_config &config)
{
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// V9938
	// MSX Engine S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 0, 2, 2); // 32KB RAM
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 0, 2, 0, 2); // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "arabic", 0, 3, 1, 2, "arabic");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, msx_yamaha_60pin, nullptr);

	msx1_v9938_pal(SND_YM2149, config, layout_msx_ar);
}

/* MSX - Sakhr AX-200 (Arabic/French) */

/* MSX - Sakhr AX-200M (Arabic/English) */

ROM_START (ax200m)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ax200bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(cae98b30) SHA1(079c018739c37485f3d64ef2145a0267fce6e20e)) // need verification

	ROM_REGION(0x8000, "arabic", 0)
	ROM_LOAD("ax200arab.rom", 0x0000, 0x8000, BAD_DUMP CRC(b041e610) SHA1(7574cc5655805ea316011a8123b064917f06f83c)) // need verification
ROM_END

void msx1_v9938_state::ax200m(machine_config &config)
{
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// V9938
	// MSX Engine S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 0, 2, 2); // 32KB RAM
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 0, 2, 0, 2); // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "arabic", 0, 3, 1, 2, "arabic");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	// Dumped unit had a SFG05 with version M5.00.011 rom
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, msx_yamaha_60pin, "sfg05");

	msx1_v9938_pal(SND_YM2149, config, layout_msx_ar);
}

/* MSX - Spectravideo SVI-738 */

ROM_START(svi738)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("738bios.rom", 0x0000, 0x8000, CRC(ad007d62) SHA1(c53b3f2c00f31683914f7452f3f4d94ae2929c0d))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("738disk.rom", 0x0000, 0x4000, CRC(acd27a36) SHA1(99a40266bc296cef1d432cb0caa8df1a7e570be4))

	ROM_REGION(0x4000, "rs232rom", ROMREGION_ERASEFF)
	ROM_LOAD("738232c.rom", 0x0000, 0x2000, CRC(3353dcc6) SHA1(4e9384c9d137f0ab65ffc5a78f04cd8c9df6c8b7))
ROM_END

void msx1_v9938_state::svi738_base(machine_config &config, const internal_layout &layout)
{
	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);  // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	add_internal_slot_irq<2>(config, MSX_SLOT_RS232_SVI738, "rs232", 3, 0, 1, 1, "rs232rom");
	add_internal_disk_mirrored(config, MSX_SLOT_DISK2_FD1793_SS, "disk", 3, 1, 1, 2, "diskrom").use_motor_for_led();

	msx1_v9938_pal(SND_AY8910, config, layout);
}

void msx1_v9938_state::svi738(machine_config &config)
{
	// AY8910
	// FDC: fd1793, 1 3.5" SSDD drive
	// 1 Cartridge slot
	// builtin 80 columns card (V9938)
	// RS-232C interface

	svi738_base(config, layout_msx_nocode_1fdd);
}

/* MSX - Spectravideo SVI-738 Arabic */

ROM_START(svi738ar)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("738arbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(ad007d62) SHA1(c53b3f2c00f31683914f7452f3f4d94ae2929c0d)) // need verification

	ROM_REGION(0x8000, "arab", 0)
	ROM_LOAD("738arab.rom",  0x0000, 0x8000, BAD_DUMP CRC(339cd1aa) SHA1(0287b2ec897b9196788cd9f10c99e1487d7adbbb)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("738ardisk.rom", 0x0000, 0x4000, BAD_DUMP CRC(acd27a36) SHA1(99a40266bc296cef1d432cb0caa8df1a7e570be4)) // meed verification

	ROM_REGION(0x4000, "rs232rom", ROMREGION_ERASEFF)
	ROM_LOAD("738ar232c.rom", 0x0000, 0x2000, BAD_DUMP CRC(3353dcc6) SHA1(4e9384c9d137f0ab65ffc5a78f04cd8c9df6c8b7)) // need verification
ROM_END

void msx1_v9938_state::svi738ar(machine_config &config)
{
	svi738_base(config, layout_msx_ar_1fdd);
	add_internal_slot(config, MSX_SLOT_ROM, "arab", 3, 3, 1, 2, "arab");
}

/* MSX - Spectravideo SVI-738 Danish/Norwegian */

ROM_START(svi738dk)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("738dkbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(88720320) SHA1(1bda5af20cb86565bdc1ebd1e59a691fed7f9256)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("738dkdisk.rom", 0x0000, 0x4000, BAD_DUMP CRC(fb884df4) SHA1(6d3a530ae822ec91f6444c681c9b08b9efadc7e7)) // need verification

	ROM_REGION(0x4000, "rs232rom", ROMREGION_ERASEFF)
	ROM_LOAD("738dk232c.rom", 0x0000, 0x2000, BAD_DUMP CRC(3353dcc6) SHA1(4e9384c9d137f0ab65ffc5a78f04cd8c9df6c8b7)) // need verification
ROM_END

/* MSX - Spectravideo SVI-738 German */

/* MSX - Spectravideo SVI-738 Polish */

ROM_START(svi738pl)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("738plbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(431b8bf5) SHA1(c90077ed84133a947841e07856e71133ba779da6)) // IC51 on board, need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("738disk.rom",   0x0000, 0x4000, BAD_DUMP CRC(acd27a36) SHA1(99a40266bc296cef1d432cb0caa8df1a7e570be4)) // need verification

	ROM_REGION(0x4000, "rs232rom", ROMREGION_ERASEFF)
	ROM_LOAD("738232c.rom",   0x0000, 0x2000, BAD_DUMP CRC(3353dcc6) SHA1(4e9384c9d137f0ab65ffc5a78f04cd8c9df6c8b7)) // need verification
ROM_END

/* MSX - Spectravideo SVI-738 Spanish */

ROM_START(svi738sp)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("738spbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(f0c0cbb9) SHA1(5f04d5799ed72ea4993e7c4302a1dd55ac1ea8cd)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("738spdisk.rom", 0x0000, 0x4000, BAD_DUMP CRC(fb884df4) SHA1(6d3a530ae822ec91f6444c681c9b08b9efadc7e7)) // need verification

	ROM_REGION(0x4000, "rs232rom", ROMREGION_ERASEFF)
	ROM_LOAD("738sp232c.rom", 0x0000, 0x2000, BAD_DUMP CRC(3353dcc6) SHA1(4e9384c9d137f0ab65ffc5a78f04cd8c9df6c8b7)) // need verification
ROM_END

/* MSX - Spectravideo SVI-738 Swedish/Finnish */

ROM_START(svi738sw)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("738sebios.rom", 0x0000, 0x8000, CRC(c8ccdaa0) SHA1(87f4d0fa58cfe9cef818a3185df2735e6da6168c))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("738sedisk.rom", 0x0000, 0x4000, CRC(fb884df4) SHA1(6d3a530ae822ec91f6444c681c9b08b9efadc7e7))

	ROM_REGION(0x4000, "rs232rom", ROMREGION_ERASEFF)
	ROM_LOAD("738se232c.rom", 0x0000, 0x2000, CRC(3353dcc6) SHA1(4e9384c9d137f0ab65ffc5a78f04cd8c9df6c8b7))
ROM_END

/* MSX - Talent DPC-200A */

ROM_START(tadpc200a)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("dpc200abios.rom", 0x0000, 0x8000, BAD_DUMP CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db)) // need verification
ROM_END

void msx1_v9938_state::tadpc200a(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// S1985

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);  // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	// Expansion slot

	msx1_v9938_pal(SND_YM2149, config, layout_msx_nocode);
}

/* MSX - Yamaha CX5MII-128A (Australia, New Zealand) */

/* MSX - Yamaha CX5MII-128B (Italy) */

// Exact region unknown
ROM_START(cx5m128)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("cx5m128bios.rom", 0x0000, 0x8000, CRC(7973e080) SHA1(ea4a723cf098be7d7b40f23a7ab831cf5e2190d7))

	ROM_REGION(0x4000, "subrom", ROMREGION_ERASEFF)
	ROM_LOAD("cx5m128sub.rom",  0x0000, 0x2000, CRC(b17a776d) SHA1(c2340313bfda751181e8a5287d60f77bc6a2f3e6))
ROM_END

void msx1_v9938_state::cx5m128(machine_config &config)
{
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// 1 Mini cart slot (with YRM-502)
	// 1 Yamaha Module slot
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 1, 1, "subrom");
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_MINICART, "minicart", 3, 1, msx_yamaha_minicart, nullptr);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x20000);   // 128KB Mapper RAM
	add_cartridge_slot<4>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, 3, msx_yamaha_60pin, "sfg05");

	msx1_v9938_pal(SND_YM2149, config, layout_msx_nocode);
	SOFTWARE_LIST(config, "minicart_list").set_original("msx_yamaha_minicart");
}

/* MSX - Yamaha CX5MII-128 C (Canada) */

/* MSX - Yamaha CX5MII-128 E (UK) */

/* MSX - Yamaha CX5MII-128 F (France) */

/* MSX - Yamaha CX5MII-128 G (Germany) */

/* MSX - Yamaha CX5MII-128 P (Spain) */

/* MSX - Yamaha CX5MII-128 S (Scandinavia) */

/* MSX - Yamaha CX5MII-128 U (USA) */

/* MSX - Yamaha CX5MIIA (Australia, New Zealand) */

/* MSX - Yamaha CX5MIIB (Italy) */

ROM_START(cx5miib)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("cx5mii_basic-bios1.rom", 0x0000, 0x8000, CRC(507b2caa) SHA1(0dde59e8d98fa524961cd37b0e100dbfb42cf576))

	ROM_REGION(0x4000, "subrom", 0)
	// overdump?
	ROM_LOAD("cx5mii_sub.rom",  0x0000, 0x4000, BAD_DUMP CRC(317f9bb5) SHA1(0ce800666c0d66bc2aa0b73a16f228289b9198be))
ROM_END

void msx1_v9938_state::cx5miib(machine_config &config)
{
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// 1 Mini cartridge slot (with YRM-502)
	// 1 Module slot
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "subrom", 3, 0, 1, 1, "subrom");
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_MINICART, "minicart", 3, 1, msx_yamaha_minicart, nullptr);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 2, 0, 4).set_total_size(0x10000);   // 64KB Mapper RAM
	add_cartridge_slot<4>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, 3, msx_yamaha_60pin, "sfg05");

	msx1_v9938_pal(SND_YM2149, config, layout_msx_nocode);
	SOFTWARE_LIST(config, "minicart_list").set_original("msx_yamaha_minicart");
}

/* MSX - Yamaha CX5MIIC (Canada) */

/* MSX - Yamaha CX5MIIE (UK) */

/* MSX - Yamaha CX5MIIF (France) */

/* MSX - Yamaha CX5MIIG (Germany) */

/* MSX - Yamaha CX5MIIP (Spain) */

/* MSX - Yamaha CX5MIIS (Scandinavia) */

/* MSX - Yamaha CX5MIIU (USA) */

/* MSX - Yamaha YIS-503II */

ROM_START(yis503ii)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yis503iibios.rom", 0x0000, 0x8000, CRC(3b08dc03) SHA1(4d0c37ad722366ac7aa3d64291c3db72884deb2d))
ROM_END

void msx1_v9938_state::yis503ii(machine_config &config)
{
	// YM2149 in S3527
	// FDC: None, 0 drives
	// S3527
	// 2 Cartridge slots
	// 1 Yamaha module slot (60 pin)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, 3, msx_yamaha_60pin, nullptr);

	msx1_v9938(SND_YM2149, config, layout_msx_nocode);
}

/* MSX - Yamaha YIS503-IIR Russian */

ROM_START(y503iir)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yis503iirbios.rom", 0x0000, 0x8000, CRC(e751d55c) SHA1(807a823d4cac527c9f3758ed412aa2584c7f6d37))
// This is in the module slot by default
// ROM_LOAD("yis503iirnet.rom",  0xc000, 0x2000, CRC(0731db3f) SHA1(264fbb2de69fdb03f87dc5413428f6aa19511a7f))
ROM_END

void msx1_v9938_state::y503iir(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// 1 Mini cartridge slot
	// 1 Yamaha module slot
	// S-3527 MSX Engine
	// V9938 VDP

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_MINICART, "minicart", 3, 1, msx_yamaha_minicart, nullptr);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM
	// This should have a serial network interface by default
	add_cartridge_slot<4>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, 3, msx_yamaha_60pin, nullptr);

	msx1_v9938_pal(SND_YM2149, config, layout_msx_ru);
	SOFTWARE_LIST(config, "minicart_list").set_original("msx_yamaha_minicart");
}

/* MSX - Yamaha YIS503-IIR Estonian */

ROM_START(y503iir2)
	ROM_REGION(0x10000, "mainrom", 0)
	ROM_LOAD("yis503ii2bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(1548cee3) SHA1(42c7fff25b1bd90776ac0aea971241aedce8947d)) // need verification
// This is in the module slot by default
// ROM_LOAD("yis503iirnet.rom",  0xc000, 0x2000, CRC(0731db3f) SHA1(264fbb2de69fdb03f87dc5413428f6aa19511a7f))
ROM_END

void msx1_v9938_state::y503iir2(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// 1 Mini cartridge slot
	// 1 Yamaha module slot
	// S-3527 MSX Engine
	// V9938 VDP

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_MINICART, "minicart", 3, 1, msx_yamaha_minicart, nullptr);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM
	// This should have a serial network interface by default
	add_cartridge_slot<4>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, 3, msx_yamaha_60pin, nullptr);

	msx1_v9938_pal(SND_YM2149, config, layout_msx_ru);
	SOFTWARE_LIST(config, "minicart_list").set_original("msx_yamaha_minicart");
}

} // anonymous namespace

COMP(1986, ax200,      0,        0,     ax200,      msx,      msx1_v9938_state, empty_init, "Sakhr", "AX-200 (MSX1, Arabic/English)", 0)
COMP(1986, ax200m,     ax200,    0,     ax200m,     msx,      msx1_v9938_state, empty_init, "Sakhr", "AX-200M (MSX1, Arabic/English)", 0)
COMP(1985, svi738,     0,        0,     svi738,     msx,      msx1_v9938_state, empty_init, "Spectravideo", "SVI-738 (MSX1, International)", 0)
COMP(1987, svi738ar,   svi738,   0,     svi738ar,   msx,      msx1_v9938_state, empty_init, "Spectravideo", "SVI-738 (MSX1, Arabic)", 0)
COMP(1985, svi738dk,   svi738,   0,     svi738,     svi738dk, msx1_v9938_state, empty_init, "Spectravideo", "SVI-738 (MSX1, Denmark, Norway)", 0)
COMP(1986, svi738pl,   svi738,   0,     svi738,     msx,      msx1_v9938_state, empty_init, "Spectravideo", "SVI-738 (MSX1, Poland)", 0)
COMP(1985, svi738sp,   svi738,   0,     svi738,     msxsp,    msx1_v9938_state, empty_init, "Spectravideo", "SVI-738 (MSX1, Spain)", 0)
COMP(1985, svi738sw,   svi738,   0,     svi738,     svi738sw, msx1_v9938_state, empty_init, "Spectravideo", "SVI-738 (MSX1, Finland, Sweden)", 0)
COMP(1988, tadpc200a,  dpc200,   0,     tadpc200a,  msx,      msx1_v9938_state, empty_init, "Talent", "DPC-200A (MSX1, Argentina)", 0) // Should have a Spanish keyboard layout?
COMP(1984, cx5m128,    0,        0,     cx5m128,    msx,      msx1_v9938_state, empty_init, "Yamaha", "CX5M-128 (MSX1)", 0)
COMP(1984, cx5miib,    cx5m128,  0,     cx5miib,    msx,      msx1_v9938_state, empty_init, "Yamaha", "CX5MIIB (MSX1, Italy)", 0)
COMP(1985, yis503ii,   yis503,   0,     yis503ii,   msxjp,    msx1_v9938_state, empty_init, "Yamaha", "YIS503II (MSX1, Japan)", 0)
COMP(1985, y503iir,    yis503,   0,     y503iir,    msxru,    msx1_v9938_state, empty_init, "Yamaha", "YIS503IIR (MSX1, USSR)", 0)
COMP(1986, y503iir2,   yis503,   0,     y503iir2,   y503iir2, msx1_v9938_state, empty_init, "Yamaha", "YIS503IIR (MSX1, Estonian)", 0)
