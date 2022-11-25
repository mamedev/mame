// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#include "emu.h"
#include "msx.h"
#include "msx_keyboard.h"
#include "bus/msx/slot/ax230.h"
#include "bus/msx/slot/disk.h"
#include "bus/msx/slot/msx_rs232.h"
#include "bus/msx/slot/ram.h"
#include "bus/msx/slot/ram_mm.h"
#include "bus/msx/slot/rom.h"

using namespace msx_keyboard;


/***************************************************************************

  MSX1 Game drivers

Undumped and/or not emulated:
- Daewoo CPC-200
- Daewoo Zemmix CPC-50
- Daewoo Zemmix DTX-1493FW
- General PCT-50
- General PCT-55
- Goldstar FC-80
- Hitachi MB-H21
- Hitachi MB-H80 (unreleased)
- In Tensai DPC-200CD
- Jotan Holland Bingo
- Misawa-Van CX-5
- Mitsubishi ML-FX2
- Mitsubishi ML-TS1
- Network DPC-200
- Olympia DPC-200
- Panasonic FS-3900
- Philips NMS 800
- Philips VG-8020/29 - MSX1
- Philips VG-8020/40 - MSX1
- Phonola VG-8000 (Italian market, mostly likely same as Philips VG-8000)
- Phonola VG-8010 (Italian market, mostly likely same as Philips VG-8010)
- Phonola VG-8020 (Italian market, mostly likely same as Philips VG-8020)
- Pioneer PX-V7
- Radiola MK 180
- Sakhr AH-200
- Sakhr AX-100
- Sakhr AX-170F
- Sakhr AX-330
- Sakhr AX-660
- Sakhr AX-990
- Salora MSX (prototypes)
- Sanno PHC-SPC
- Sanno SPCmk-II
- Sanno SPCmk-III
- Sanyo MPC-1 / Wavy1
- Schneider MC 810
- Sincorp SBX (Argentina, homebrew)
- Sony HB-10B
- Sony HB-10D
- Sony HB-501F
- Sony HB-75AS
- Sony HB-75B
- Sony HB-701
- Spectravideo SVI-728 (Arabic)
- Spectravideo SVI-728 (Danish/Norwegian)
- Spectravideo SVI-728 (Swedish/Finnish)
- Talent DPS-201
- Toshiba HX-10AA
- Toshiba HX-10DPN
- Toshiba HX-10I
- Toshiba HX-10SF
- Toshiba HX-20AR
- Toshiba HX-22CH
- Toshiba HX-22GB
- Toshiba HX-30
- Tosbiba HX-31
- Toshiba HX-52
- Triton PC64
- Vestel FC-200
- Victor HC-30
- Victor HC-60
- Wandy DPC-200
- Yamaha CX5
- Yamaha CX5MA (Australia / New Zealand)
- Yamaha CX5MC (Canada)
- Yamaha CX5ME (UK)
- Yamaha CX5MF (France)
- Yamaha CX5MG (Germany)
- Yamaha CX5MS (Scandinavia)
- Yamaha YIS-603
- Yeno DPC-64
***************************************************************************/

namespace {

class msx1_state : public msx_state
{
public:
	msx1_state(const machine_config &mconfig, device_type type, const char *tag)
		: msx_state(mconfig, type, tag)
	{
	}

	void ax150(machine_config &config);
	void ax170(machine_config &config);
	void ax230(machine_config &config);
	void canonv8(machine_config &config);
	void canonv10(machine_config &config);
	void canonv20(machine_config &config);
	void canonv20e(machine_config &config);
	void canonv25(machine_config &config);
	void cf1200(machine_config &config);
	void cf2000(machine_config &config);
	void cf2700(machine_config &config);
	void cf2700g(machine_config &config);
	void cf2700uk(machine_config &config);
	void cf3000(machine_config &config);
	void cf3300(machine_config &config);
	void cpc50a(machine_config &config);
	void cpc50b(machine_config &config);
	void cpc51(machine_config &config);
	void cpc88(machine_config &config);
	void cx5f(machine_config &config);
	void cx5f1(machine_config &config);
	void cx5mu(machine_config &config);
	void dgnmsx(machine_config &config);
	void dpc100(machine_config &config);
	void dpc180(machine_config &config);
	void dpc200(machine_config &config);
	void dpc200e(machine_config &config);
	void expert10(machine_config &config);
	void expert11(machine_config &config);
	void expert13(machine_config &config);
	void expertdp(machine_config &config);
	void expertpl(machine_config &config);
	void fmx(machine_config &config);
	void fdpc200(machine_config &config);
	void fpc500(machine_config &config);
	void fs1300(machine_config &config);
	void fs4000(machine_config &config);
	void fs4000a(machine_config &config);
	void fspc800(machine_config &config);
	void gfc1080(machine_config &config);
	void gfc1080a(machine_config &config);
	void gsfc80u(machine_config &config);
	void gsfc200(machine_config &config);
	void hb10(machine_config &config);
	void hb10p(machine_config &config);
	void hb20p(machine_config &config);
	void hb55(machine_config &config);
	void hb55d(machine_config &config);
	void hb55p(machine_config &config);
	void hb75(machine_config &config);
	void hb75d(machine_config &config);
	void hb75p(machine_config &config);
	void hb101(machine_config &config);
	void hb101p(machine_config &config);
	void hb201(machine_config &config);
	void hb201p(machine_config &config);
	void hb501p(machine_config &config);
	void hb701fd(machine_config &config);
	void hb8000(machine_config &config);
	void hc5(machine_config &config);
	void hc6(machine_config &config);
	void hc7(machine_config &config);
	void hotbi13b(machine_config &config);
	void hotbi13p(machine_config &config);
	void hx10(machine_config &config);
	void hx10d(machine_config &config);
	void hx10dp(machine_config &config);
	void hx10e(machine_config &config);
	void hx10f(machine_config &config);
	void hx10s(machine_config &config);
	void hx10sa(machine_config &config);
	void hx20(machine_config &config);
	void hx20e(machine_config &config);
	void hx20i(machine_config &config);
	void hx21(machine_config &config);
	void hx21f(machine_config &config);
	void hx22(machine_config &config);
	void hx22i(machine_config &config);
	void hx32(machine_config &config);
	void hx51i(machine_config &config);
	void jvchc7gb(machine_config &config);
	void mbh1(machine_config &config);
	void mbh1e(machine_config &config);
	void mbh2(machine_config &config);
	void mbh25(machine_config &config);
	void mbh50(machine_config &config);
	void ml8000(machine_config &config);
	void mlf48(machine_config &config);
	void mlf80(machine_config &config);
	void mlf110(machine_config &config);
	void mlf120(machine_config &config);
	void mlfx1(machine_config &config);
	void mpc10(machine_config &config);
	void mpc64(machine_config &config);
	void mpc100(machine_config &config);
	void mpc200(machine_config &config);
	void mpc200sp(machine_config &config);
	void mx10(machine_config &config);
	void mx15(machine_config &config);
	void mx64(machine_config &config);
	void mx101(machine_config &config);
	void nms801(machine_config &config);
	void perfect1(machine_config &config);
	void phc2(machine_config &config);
	void phc28(machine_config &config);
	void phc28l(machine_config &config);
	void phc28s(machine_config &config);
	void piopx7(machine_config &config);
	void piopx7uk(machine_config &config);
	void piopxv60(machine_config &config);
	void pv7(machine_config &config);
	void pv16(machine_config &config);
	void spc800(machine_config &config);
	void svi728(machine_config &config);
	void sx100(machine_config &config);
	void tadpc200(machine_config &config);
	void vg8000(machine_config &config);
	void vg8010(machine_config &config);
	void vg8010f(machine_config &config);
	void vg802000(machine_config &config);
	void vg802020(machine_config &config);
	void vg8020f(machine_config &config);
	void yc64(machine_config &config);
	void yis303(machine_config &config);
	void yis503(machine_config &config);
	void yis503f(machine_config &config);
};

/* MSX - Al Fateh 100 - rebranded Sakhr / Al Alamiah AX-170, dump needed to verify */

/* MSX - Al Fateh 123 - rebranded Sakhr / Al Alamiah AX-230, dump needed to verify */

/* MSX - AVT DPC-200 - See Fenner DPC-200 (they are the same machine, same roms) */

/* MSX - AVT FC-200 - probably same as Goldstar FC-80, dump needed to verify */

/* MSX - Bawareth Perfect MSX1 (DPC-200CD) */

ROM_START(perfect1)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("perfect1bios.rom", 0x0000, 0x8000, CRC(a317e6b4) SHA1(e998f0c441f4f1800ef44e42cd1659150206cf79))

	ROM_REGION(0x8000, "firmware", 0)
	ROM_SYSTEM_BIOS(0, "v1990", "1990 Firmware")
	ROMX_LOAD("cpc-200bw_v1_0", 0x0000, 0x8000, CRC(d6373270) SHA1(29a9169b605b5881e4a15fcfd65209a4e8679285), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v1987", "1987 Firmware (v3.21)")
	ROMX_LOAD("perfect1arab.rom", 0x0000, 0x8000, CRC(6db04a4d) SHA1(01012a0e2738708861f66b6921b2e2108f2edb54), ROM_BIOS(1))
ROM_END

void msx1_state::perfect1(machine_config &config)
{
	// GSS Z8400A PS cpu
	// AY-3-8910
	// TMS9129
	// DW64MX1
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 1, 1, 2, "firmware");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 0, 4); // 64KB RAM
	add_cartridge_slot<1>(config, 1);
	// expansion slot in slot #2

	msx1(VDP_TMS9129, SND_AY8910, config);
}

/* MSX - Canon V-8 */

ROM_START(canonv8)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("v8bios.rom", 0x0000, 0x8000, CRC(e941b08e) SHA1(97f9a0b45ee4b34d87ca3f163df32e1f48b0f09c))
ROM_END

void msx1_state::canonv8(machine_config &config)
{
	// NEC D780C cpu
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 1 Cartridge slots
	// S3527
	// No printer port

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 3, 1);

	m_hw_def.has_printer_port(false);
	msx1(VDP_TMS9118, SND_YM2149, config);
}

/* MSX - Canon V-10 */

ROM_START(canonv10)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("v10bios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx1_state::canonv10(machine_config &config)
{
	// Zilog Z80A
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	msx1(VDP_TMS9918A, SND_YM2149, config);
}

/* MSX - Canon V-20 */

ROM_START(canonv20)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("v20bios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx1_state::canonv20(machine_config &config)
{
	// NEC D780C cpu
	// XTAL: 14.31818(Z80/PSG) + 10.6875(VDP)
	// YM2149
	// TMS9918ANL
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(VDP_TMS9918A, SND_YM2149, config);
}

/* MSX - Canon V-20E */

ROM_START(canonv20e)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("3256m67-5a3_z-2_uk.u20", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))
ROM_END

void msx1_state::canonv20e(machine_config &config)
{
	// Zilog Z8400A PS Z80A cpu
	// XTAL: 14.31818(Z80/PSG) + 10.6875(VDP)
	// YM2149
	// TMS9929ANL
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(VDP_TMS9929A, SND_YM2149, config);
}

/* MSX - Canon V-20F */

ROM_START(canonv20f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("v20fbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(e0e894b7) SHA1(d99eebded5db5fce1e072d08e642c0909bc7efdd)) // need verification
ROM_END

/* MSX - Canon V-20G */

ROM_START(canonv20g)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("v20gbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(d6e704ad) SHA1(d67be6d7d56d7229418f4e122f2ec27990db7d19)) // need verification
ROM_END

/* MSX - Canon V-20S */

ROM_START(canonv20s)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("v20sbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(c72b186e) SHA1(9fb289ea5c11d497ee00703f64e82575d1c59923)) // need verification
ROM_END

/* MSX - Casio MX-10 */

ROM_START(mx10)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("3256d19-5k3_z-1.g2", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx1_state::mx10(machine_config &config)
{
	// Z80: uPD780C-1
	// AY-3-8910
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot for KB-10 to add a printer port and 2 more cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1); // 16KB RAM
	add_cartridge_slot<1>(config, 1);

	m_hw_def.has_cassette(false).has_printer_port(false);
	msx1(VDP_TMS9118, SND_AY8910, config);
}

/* MSX - Casio MX-15 */

ROM_START(mx15)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mx15bios.rom", 0x0000, 0x8000, CRC(6481230f) SHA1(a7ed5fd940f4e3a33e676840c0a83ac7ee54d972))
ROM_END

void msx1_state::mx15(machine_config &config)
{
	// AY-3-8910
	// T6950
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot for KB-15 to add a printer port and 2 more cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1); // 16KB RAM
	add_cartridge_slot<1>(config, 1);

	m_hw_def.has_printer_port(false);
	msx1(VDP_TMS9928A, SND_AY8910, config);
}

/* MSX - Casio MX-101 */

ROM_START(mx101)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("3256d19-5k3_z-1", 0x0000, 0x8000, BAD_DUMP CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821)) // need verification
ROM_END

void msx1_state::mx101(machine_config &config)
{
	// Z80: uPD780C-1
	// AY-3-8910
	// FDC: None, 0 drives
	// 1 Cartridge slots
	// 1 Expansion slot for KB-10 to add a printer port and 2 more cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1); // 16KB RAM
	add_cartridge_slot<1>(config, 1);

	m_hw_def.has_cassette(false).has_printer_port(false);
	msx1(VDP_TMS9118, SND_AY8910, config);
}

/* MSX - Casio PV-7 */

ROM_START(pv7)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("pv7bios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx1_state::pv7(machine_config &config)
{
	// AY8910?
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// Non-standard cassette port
	// No printer port
	// 1 Expansion slot for KB-7 to add a printer port, 2 more cartridge slots, and 8KB RAM
	// Z80: uPD780C-1

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1).force_start_address(0xe000);   // 8KB RAM
	add_cartridge_slot<1>(config, 1);

	m_hw_def.has_printer_port(false);
	msx1(VDP_TMS9118, SND_AY8910, config);
}

/* MSX - Casio PV-16 */

ROM_START(pv16)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("3256d19-5k3_z-1", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx1_state::pv16(machine_config &config)
{
	// NEC UPD780C
	// AY-3-8910
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// No printer port
	// 1 Expansion slot for KB-7 to add a printer port, 2 more cartridge slots, and 8KB RAM

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);

	m_hw_def.has_printer_port(false);
	msx1(VDP_TMS9118, SND_AY8910, config);
}

/* MSX - CE-TEC MPC-80, German version of Daewoo DPC-200, dump needed to verify */

/* MSX - Daewoo CPC-200 */

/* MSX - Daewoo CPC-88 */

ROM_START(cpc88)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("88bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(3ab0cd3b) SHA1(171b587bd5a947a13f3114120b6e7baca3b57d78)) // need verification

	ROM_REGION(0x4000, "hangul", 0)
	ROM_LOAD("88han.rom",  0x0000, 0x2000, BAD_DUMP CRC(938db440) SHA1(d41676fde0a3047792f93c4a41509b8749e55e74)) // need verification
	ROM_RELOAD(0x2000, 0x2000) // Are the contents really mirrored?
ROM_END

void msx1_state::cpc88(machine_config &config)
{
	// AY-3-8910A
	// TMS9928A ?
	// FDC: None, 0 drives
	// 0 Cartridge slots
	// Expansion slot allows addition of cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 2, 1, "hangul");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);   // 64KB RAM

	m_hw_def.has_printer_port(false);
	msx1(VDP_TMS9928A, SND_AY8910, config);
}

/* MSX - Daewoo DPC-100 */

ROM_START(dpc100)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("100bios.rom", 0x0000, 0x8000, CRC(3ab0cd3b) SHA1(171b587bd5a947a13f3114120b6e7baca3b57d78))

	ROM_REGION(0x4000, "hangul", 0)
	// should be 0x2000?
	ROM_LOAD("100han.rom",  0x0000, 0x4000, CRC(97478efb) SHA1(4421fa2504cbce18f7c84b5ea97f04e017007f07))
ROM_END

void msx1_state::dpc100(machine_config &config)
{
	// GSS Z8400A PS
	// AY-3-8910A
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 2, 1, "hangul");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 3, 1);   // 16KB RAM
	// expansion slot is in slot #3

	msx1(VDP_TMS9918A, SND_AY8910, config);
}

/* MSX - Daewoo DPC-180 */

ROM_START(dpc180)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("180bios.rom", 0x0000, 0x8000, CRC(3ab0cd3b) SHA1(171b587bd5a947a13f3114120b6e7baca3b57d78))

	ROM_REGION(0x4000, "hangul", 0)
	// should be 0x2000?
	ROM_LOAD("180han.rom",  0x0000, 0x4000, CRC(97478efb) SHA1(4421fa2504cbce18f7c84b5ea97f04e017007f07))
ROM_END

void msx1_state::dpc180(machine_config &config)
{
	// AY-3-8910A
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 2, 1, "hangul");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);   // 64KB RAM
	// Expansion slot is in slot #3

	msx1(VDP_TMS9918A, SND_AY8910, config);
}

/* MSX - Daewoo DPC-200 */

ROM_START(dpc200)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("200bios.rom", 0x0000, 0x8000, CRC(3ab0cd3b) SHA1(171b587bd5a947a13f3114120b6e7baca3b57d78))

	ROM_REGION(0x4000, "hangul", 0)
	// should be 0x2000?
	ROM_LOAD("200han.rom",  0x0000, 0x4000, CRC(97478efb) SHA1(4421fa2504cbce18f7c84b5ea97f04e017007f07))
ROM_END

void msx1_state::dpc200(machine_config &config)
{
	// GSS Z8400A PS cpu
	// AY-3-8910A
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 2, 1, "hangul");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	// Expansion slot is in slot #3

	msx1(VDP_TMS9918A, SND_AY8910, config);
}

/* MSX - Daewoo DPC-200E (France) */

ROM_START(dpc200e)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("dpc200ebios.rom", 0x0000, 0x8000, CRC(d2110d66) SHA1(d3af963e2529662eae63f04a2530454685a1989f))
ROM_END

void msx1_state::dpc200e(machine_config &config)
{
	// AY-3-8910
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);  // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	// Expansion slot is in slot #3

	msx1(VDP_TMS9129, SND_AY8910, config);
}

/* MSX - Daewoo Zemmix CPC-50 */

/* MSX - Daewoo Zemmix CPC-50A */

ROM_START(cpc50a)
	ROM_REGION(0x8000, "mainrom", 0)
	// HM6264LP-15 / U0422880 (ic4)
	// GMCE? VER1.01 (ic5)
	ROM_LOAD("50abios.rom", 0x0000, 0x8000, BAD_DUMP CRC(c3a868ef) SHA1(a08a940aa87313509e00bc5ac7494d53d8e03492)) // need verification
ROM_END

void msx1_state::cpc50a(machine_config &config)
{
	// NEC uPD780C cpu
	// AY-3-8910A
	// DW64MX1
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// No keyboard
	// No cassette port
	// No printer port

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 3, 1).force_start_address(0xe000);  // 8KB RAM

	m_hw_def.has_cassette(false)
		.has_printer_port(false);
	msx1(VDP_TMS9118, SND_AY8910, config);
}

/* MSX - Daewoo Zemmix CPC-50B */

ROM_START(cpc50b)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("50bbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(c3a868ef) SHA1(a08a940aa87313509e00bc5ac7494d53d8e03492)) // need verification
ROM_END

void msx1_state::cpc50b(machine_config &config)
{
	// AY-3-8910A
	// DW64MX1
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// No keyboard
	// No cassette port
	// No printer port

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 3, 1);  // 16KB RAM

	m_hw_def.has_cassette(false)
		.has_printer_port(false);
	msx1(VDP_TMS9118, SND_AY8910, config);
}

/* MSX - Daewoo Zemmix CPC-51 */

ROM_START(cpc51)
	ROM_REGION(0x8000, "mainrom", 0)
	// Sticker: CPC-51 V 1.01 (ic05)
	ROM_LOAD("cpc-51_v_1_01.ic05", 0x0000, 0x8000, CRC(c3a868ef) SHA1(a08a940aa87313509e00bc5ac7494d53d8e03492))
ROM_END

void msx1_state::cpc51(machine_config &config)
{
	// GSS Z8400A PS cpu
	// AY-3-8910A
	// FDC: None, 0 drives
	// DW64MX1
	// 1 Cartridge slot
	// No keyboard, just a keyboard connector
	// No cassette port
	// No printer port

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM

	m_hw_def.has_cassette(false)
		.has_printer_port(false);
	msx1(VDP_TMS9118, SND_AY8910, config);
}

/* MSX - Daewoo Zemmix DTX-1493FW */

/* MSX - Dragon MSX-64 */

ROM_START(dgnmsx)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("uk1msx048.ic37", 0x0000, 0x4000, BAD_DUMP CRC(24c198be) SHA1(7f8c94cb8913db32a696dec80ffc78e46693f1b7)) // need verification
	ROM_LOAD("uk2msx058.ic6",  0x4000, 0x4000, BAD_DUMP CRC(e516e7e5) SHA1(05fedd4b9bfcf4949020c79d32c4c3f03a54fb62)) // need verification
ROM_END

void msx1_state::dgnmsx(machine_config &config)
{
	// Sharp LH0080A cpu
	// AY-3-8910
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	add_cartridge_slot<2>(config, 3);

	msx1(VDP_TMS9129, SND_AY8910, config);
}

/* MSX - Dynadata DPC-200 */
// GSS Z8400A PS
// AY-3-8910A
// 1 Cartridge slot
// 1 Expansion slot
// TMS9929A

/* MSX - Fenner DPC-200 */

ROM_START(fdpc200)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("dpc200bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db)) // need verification
ROM_END

void msx1_state::fdpc200(machine_config &config)
{
	// AY-3-8910
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);  // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	// Expansion slot

	msx1(VDP_TMS9929A, SND_AY8910, config);
}

/* MSX - Fenner FPC-500 */

ROM_START(fpc500)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("fpc500bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db)) // need verification
ROM_END

void msx1_state::fpc500(machine_config &config)
{
	// AY-3-8910
	// T6950 vdp
	// T7775 msx engine
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(VDP_TMS9929A, SND_AY8910, config);
}

/* MSX - Fenner SPC-800 */

ROM_START(fspc800)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("spc800bios.u7", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))
ROM_END

void msx1_state::fspc800(machine_config &config)
{
	// GSS Z8400A
	// AY-3-8910A
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(VDP_TMS9129, SND_AY8910, config);
}

/* MSX - Fujitsu FM-X */

ROM_START(fmx)
	ROM_REGION(0x8000, "mainrom", 0)
	// mb62h010 ?
	ROM_LOAD("fmxbios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx1_state::fmx(machine_config &config)
{
	// 21.4772Mhz
	// AY-3-8910A
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 "Fujitsu expansion" slot for MB22450 (to connect FM-X to FM7) or MB22451 (printer port + 16KB ram)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1); // 16KB RAM
	// Fujitsu expansion slot #1 in slot 1
	add_cartridge_slot<1>(config, 2);

	m_hw_def.has_printer_port(false);
	msx1(VDP_TMS9928A, SND_AY8910, config);
}

/* MSX - General PCT-50 */

/* MSX - General PCT-55 */

/* MSX - Goldstar FC-80 */

/* MSX - Goldstar FC-80U */

ROM_START(gsfc80u)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("fc80ubios.rom", 0x0000, 0x8000, CRC(3ab0cd3b) SHA1(171b587bd5a947a13f3114120b6e7baca3b57d78))

	ROM_REGION(0x4000, "hangul", 0)
	ROM_LOAD("fc80uhan.rom",  0x0000, 0x2000, CRC(0cdb8501) SHA1(58dbe73ae80c2c409e766c3ace730ecd7bec89d0))
	ROM_RELOAD(0x2000, 0x2000)
ROM_END

void msx1_state::gsfc80u(machine_config &config)
{
	// AY-3-8910A
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot
	// Hangul LED

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 2, 1, "hangul");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	// Expansion slot in slot #3

	msx1(VDP_TMS9918A, SND_AY8910, config);
}

/* MSX - Goldstar FC-200 */

ROM_START(gsfc200)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("fc200bios.rom.u5a", 0x0000, 0x4000, CRC(61f473fb) SHA1(c425750bbb2ae1d278216b45029d303e37d8df2f))
	ROM_LOAD("fc200bios.rom.u5b", 0x4000, 0x4000, CRC(1a99b1a1) SHA1(e18f72271b64693a2a2bc226e1b9ebd0448e07c0))
ROM_END

void msx1_state::gsfc200(machine_config &config)
{
	// AY-3-8910A
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	// Expansion slot in slot #3

	msx1(VDP_TMS9129, SND_AY8910, config);
}

/* MSX - Goldstar GFC-1080 */

ROM_START(gfc1080)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("gfc1080bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(d9cdd4a6) SHA1(6b0be712b9c95c1e912252ab5703e1c0bc457d9e)) // need verification

	ROM_REGION(0x4000, "hangul", 0)
	ROM_LOAD("gfc1080han.rom", 0x0000, 0x4000, BAD_DUMP CRC(f209448c) SHA1(141b44212ba28e7d03e0b54126fedd9e0807dc42)) // need verification

	ROM_REGION(0x4000, "pasocalc", 0)
	ROM_LOAD("gfc1080pasocalc.rom", 0x0000, 0x4000, BAD_DUMP CRC(4014f7ea) SHA1(a5581fa3ce10f90f15ba3dc53d57b02d6e4af172)) // need verification
ROM_END

void msx1_state::gfc1080(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 2, 1, "hangul");
	add_internal_slot(config, MSX_SLOT_ROM, "pasocalc", 0, 3, 1, "pasocalc");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4); // 64KB RAM
	// Expansion slot in slot #3

	msx1(VDP_TMS9928A, SND_AY8910, config);
}

/* MSX - Goldstar GFC-1080A */

ROM_START(gfc1080a)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("gfc1080abios.rom", 0x0000, 0x8000, BAD_DUMP CRC(3ab0cd3b) SHA1(171b587bd5a947a13f3114120b6e7baca3b57d78)) // need verification

	ROM_REGION(0x4000, "hangul", 0)
	ROM_LOAD("gfc1080ahan.rom", 0x0000, 0x2000, BAD_DUMP CRC(0cdb8501) SHA1(58dbe73ae80c2c409e766c3ace730ecd7bec89d0)) // need verification
	ROM_RELOAD(0x2000, 0x2000)
ROM_END

void msx1_state::gfc1080a(machine_config &config)
{
	// AY-3-8910A
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 2, 1, "hangul");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4); // 64KB RAM
	// Expansion slot in slot #3

	msx1(VDP_TMS9918A, SND_AY8910, config);
}

/* MSX - Gradiente Expert 1.3 - source? */

ROM_START(expert13)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("expbios13.rom", 0x0000, 0x8000, BAD_DUMP CRC(5638bc38) SHA1(605f5af3f358c6811f54e0173bad908614a198c0)) // need verification
ROM_END

void msx1_state::expert13(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2 Cartridge slots?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 2, 0, 4).set_total_size(0x10000);   // 64KB Mapper RAM
	add_cartridge_slot<2>(config, 3);

	msx1(VDP_TMS9928A, SND_AY8910, config);
}

/* MSX - Gradiente Expert DDPlus */
ROM_START(expertdp)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("eddpbios.rom", 0x0000, 0x8000, CRC(efb4b972) SHA1(d6720845928ee848cfa88a86accb067397685f02))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("eddpdisk.rom", 0x0000, 0x4000, CRC(549f1d90) SHA1(f1525de4e0b60a6687156c2a96f8a8b2044b6c56))
ROM_END

void msx1_state::expertdp(machine_config &config)
{
	// T7766A (AY-3-8910A compatible, integrated in T7937A)
	// FDC: mb8877a, 1 3.5" DSDD drive
	// non-standard cassette port
	// non-standard printer port
	// 2 Cartridge slots
	// T6950 (integrated in T7937A)
	// MSX Engine T7937A (also VDP)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 0, 4);  // 64KB RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK2_MB8877, "diskrom", 3, 3, 1, 2, "diskrom");

	msx1(VDP_TMS9928A, SND_AY8910, config);
}

/* MSX - Gradiente Expert Plus */

ROM_START(expertpl)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("exppbios.rom", 0x0000, 0x8000, CRC(efb4b972) SHA1(d6720845928ee848cfa88a86accb067397685f02))

	ROM_REGION(0x4000, "demo", 0)
	ROM_LOAD("exppdemo.rom", 0x0000, 0x4000, CRC(a9bbef64) SHA1(d4cea8c815f3eeabe0c6a1c845f902ec4318bf6b))
ROM_END

void msx1_state::expertpl(machine_config &config)
{
	// T7766A (AY-3-8910 compatible in T7937A)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// MSX Engine T7937A (with T6950 VDP and T7766A psg)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 0, 4);  // 64KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "demo", 3, 3, 2, 1, "demo");

	msx1(VDP_TMS9928A, SND_AY8910, config);
}

/* MSX - Gradiente Expert XP-800 (1.0) */

ROM_START(expert10)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("expbios.rom", 0x0000, 0x8000, CRC(07610d77) SHA1(ef3e010eb57e4476700a3bbff9d2119ab3acdf62))
ROM_END

void msx1_state::expert10(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// non-standard cassette port
	// non-standard printer port
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	// when no cartridge is inserted the expansion slot can be used in this slot
	add_cartridge_slot<2>(config, 3);

	msx1(VDP_TMS9128, SND_AY8910, config);
}

/* MSX - Gradiente Expert XP-800 (1.1) / GPC-1 */
ROM_START(expert11)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("expbios11.rom", 0x0000, 0x8000, CRC(efb4b972) SHA1(d6720845928ee848cfa88a86accb067397685f02))
ROM_END

void msx1_state::expert11(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	// when no cartridge is inserted the expansion slot can be used in this slot
	add_cartridge_slot<2>(config, 3);

	msx1(VDP_TMS9128, SND_AY8910, config);
}

/* MSX - Hitachi MB-H1 */

ROM_START(mbh1)
	ROM_REGION(0xc000, "mainrom", 0)
	ROM_LOAD("mbh1bios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("mbh1firm.rom", 0x0000, 0x2000, CRC(83f5662b) SHA1(3e005832138ffde8b1c36025754f81c2112b236d))
ROM_END

void msx1_state::mbh1(machine_config &config)
{
	// AY-3-8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// Speed controller (normal, slow 1, slow 2)
	// Firmware should be bypassed when a cartridge is inserted

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2); // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 1, 1, "firmware");

	msx1(VDP_TMS9918A, SND_AY8910, config);
}

/* MSX - Hitachi MB-H1E */

ROM_START(mbh1e)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mbh1bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821)) // need verification
ROM_END

void msx1_state::mbh1e(machine_config &config)
{
	// AY-3-8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// Speed controller (normal, slow 1, slow 2)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1); // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	msx1(VDP_TMS9918A, SND_AY8910, config);
}

/* MSX - Hitachi MB-H2 */

ROM_START(mbh2)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mbh2bios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("mbh2firm.rom", 0x0000, 0x4000, CRC(4f03c947) SHA1(e2140fa2e8e59090ecccf55b62323ea9dcc66d0b))
ROM_END

void msx1_state::mbh2(machine_config &config)
{
	// AY-3-8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// Builtin cassette player
	// Speed controller (normal, slow 1, slow 2)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 2, 1, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4); // 64KB RAM

	msx1(VDP_TMS9928A, SND_AY8910, config);
}

/* MSX - Hitachi MB-H21 */

/* MSX - Hitachi MB-H25 */

ROM_START(mbh25)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mbh25bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821)) // need verification
ROM_END

void msx1_state::mbh25(machine_config &config)
{
	// AY-3-8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// Speed controller (normal, slow 1, slow 2)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2); // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	msx1(VDP_TMS9118, SND_AY8910, config);
}

/* MSX - Hitachi MB-H50 */

ROM_START(mbh50)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mbh50bios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx1_state::mbh50(machine_config &config)
{
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// T6950A
	// MSX-Engine S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4); // 64KB RAM

	msx1(VDP_TMS9928A, SND_YM2149, config);
}

/* MSX - Hitachi MB-H80 (unreleased) */

/* MSX - In Tensai DPC-200CD */

/* MSX - JVC HC-7E / HC-7GB (different power supplies) */

ROM_START(jvchc7gb)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hc7gbbios.rom", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))
ROM_END

void msx1_state::jvchc7gb(machine_config &config)
{
	// AY-3-8910A
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	add_cartridge_slot<2>(config, 3);

	msx1(VDP_TMS9929A, SND_AY8910, config);
}

/* MSX - Jotan Holland Bingo */

/* MSX - Misawa-Van CX-5 */

/* MSX - Mitsubishi ML-8000 */

ROM_START(ml8000)
	ROM_REGION(0x8000, "mainrom", 0)
	// Same contents as standard 32kb bios with sha1 302afb5d8be26c758309ca3df611ae69cced2821
	// split across 4 eeproms
	ROM_LOAD("1.ic56", 0x0000, 0x2000, BAD_DUMP CRC(782e39fd) SHA1(ad20865df0d33ee5379b69be984302fb85d74c5a)) // need verification
	ROM_LOAD("2.ic32", 0x2000, 0x2000, BAD_DUMP CRC(d859cf14) SHA1(b6894ed3cd5be5a73a93fd02d275d6c1237310e6)) // need verification
	ROM_LOAD("3.ic26", 0x4000, 0x2000, BAD_DUMP CRC(b3211adf) SHA1(fad7be46d1137f636c23dc01e498cc38cff486e3)) // need verification
	ROM_LOAD("4.ic22", 0x6000, 0x2000, BAD_DUMP CRC(24e09092) SHA1(d6fdff0fa86a248ce319888275d1c634480b58c4)) // need verification
ROM_END

void msx1_state::ml8000(machine_config &config)
{
	// NEC uPD780C
	// AY-3-8910
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2); // 32KB RAM
	add_cartridge_slot<1>(config, 1);

	msx1(VDP_TMS9918A, SND_AY8910, config);
}

/* MSX - Mitsubishi ML-F48 */

ROM_START(mlf48)
	ROM_REGION(0x8000, "mainrom", 0)
	// 4j3 hn613256p m82 ?
	ROM_LOAD("mlf48bios.ic2d", 0x0000, 0x8000, BAD_DUMP CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e)) // needs verification
ROM_END

void msx1_state::mlf48(machine_config &config)
{
	// NEC uPD780C
	// AY-3-8910A
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 2, 2); // 32KB RAM
	add_cartridge_slot<1>(config, 2);
	add_cartridge_slot<2>(config, 3);

	msx1(VDP_TMS9929A, SND_AY8910, config); // videochip needs verification
}

/* MSX - Mitsubishi ML-F80 */

ROM_START(mlf80)
	ROM_REGION(0x8000, "mainrom", 0)
	// 4j1 hn613256p m82 ?
	ROM_LOAD("mlf80bios.ic2d", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))
ROM_END

void msx1_state::mlf80(machine_config &config)
{
	// 21.3750MHz
	// AY-3-8910A
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);  // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	add_cartridge_slot<2>(config, 3);

	msx1(VDP_TMS9929A, SND_AY8910, config); // videochip needs verification
}

/* MSX - Mitsubishi ML-F110 */

ROM_START(mlf110)
	ROM_REGION(0x8000, "mainrom", 0)
	// 4j1 mn613256p d35
	ROM_LOAD("hn613256p.ic6d", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx1_state::mlf110(machine_config &config)
{
	// NEC uPD780C
	// AY-3-8910A
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 3, 1); // 16KB RAM
	add_cartridge_slot<1>(config, 2);
	add_cartridge_slot<2>(config, 3);

	msx1(VDP_TMS9918A, SND_AY8910, config);
}

/* MSX - Mitsubishi ML-F120 / ML-F120D */

ROM_START(mlf120)
	ROM_REGION(0x8000, "mainrom", 0)
	// 4k3 hn613256p d35 ?
	// 904p874h11 ?
	ROM_LOAD("hn613256p.ic6d", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("t704p890h11.ic8d", 0x0000, 0x4000, CRC(4b5f3173) SHA1(21a9f60cb6370d0617ce54c42bb7d8e40a4ab560))
ROM_END

void msx1_state::mlf120(machine_config &config)
{
	// NEC uPD780C
	// AY-3-8910A
	// FDC: None, 0 drives
	// 2 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2); // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 1, 1, "firmware");

	msx1(VDP_TMS9918A, SND_AY8910, config);
}

/* MSX - Mitsubishi ML-F120D (functionality wise same as ML-F120 but with RGB out instead of composite)
Different PCB from ML-F120:
21.4772Mhz
TMS9928A
AY-3-8910
3 ROMs? - labels unreadable, locations ic5?, ic7f, ic10f
 */

/* MSX - Mitsubishi ML-FX1 */

ROM_START(mlfx1)
	ROM_REGION(0x8000, "mainrom", 0)
	// 5c1 hn613256p t21 t704p874h21-u
	ROM_LOAD("mlfx1bios.ic6c", 0x0000, 0x8000, CRC(62867dce) SHA1(0cbe0df4af45e8f531e9c761403ac9e71808f20c))
ROM_END

void msx1_state::mlfx1(machine_config &config)
{
	// NEC uPD780C
	// YM2149 (in S3527 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// MSX-Engine S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM

	msx1(VDP_TMS9929A, SND_YM2149, config);
}

/* MSX - Mitsubishi ML-FX2 */

/* MSX - Mitsubishi ML-TS1 */

/* MSX - National CF-1200 */

ROM_START(cf1200)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("1200bios.rom", 0x0000, 0x8000, CRC(5ad03407) SHA1(c7a2c5baee6a9f0e1c6ee7d76944c0ab1886796c))
ROM_END

void msx1_state::cf1200(machine_config &config)
{
	// AY8910, needs verification
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	msx1(VDP_TMS9918A, SND_AY8910, config); // soundchip needs verification
}

/* MSX - National CF-2000 */

ROM_START(cf2000)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("2000bios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx1_state::cf2000(machine_config &config)
{
	// AY-3-8910A, needs verification
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	msx1(VDP_TMS9928A, SND_AY8910, config); // soundchip needs verification
}

/* MSX - National CF-2700 */

ROM_START(cf2700)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("2700bios.rom.ic32", 0x0000, 0x8000, CRC(5ad03407) SHA1(c7a2c5baee6a9f0e1c6ee7d76944c0ab1886796c))
ROM_END

void msx1_state::cf2700(machine_config &config)
{
	// NEC upD780C
	// AY-3-8910A
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	msx1(VDP_TMS9918A, SND_AY8910, config);
}

/* MSX - National CF-3000 */

ROM_START(cf3000)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("3000bios.rom", 0x0000, 0x8000, CRC(5ad03407) SHA1(c7a2c5baee6a9f0e1c6ee7d76944c0ab1886796c))
ROM_END

void msx1_state::cf3000(machine_config &config)
{
	// AY-3-8910
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);  // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	add_cartridge_slot<2>(config, 3);

	msx1(VDP_TMS9928A, SND_AY8910, config);
}

/* MSX - National CF-3300 */

ROM_START(cf3300)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("3300bios.rom", 0x0000, 0x8000, CRC(5ad03407) SHA1(c7a2c5baee6a9f0e1c6ee7d76944c0ab1886796c))

	ROM_REGION(0x4000, "diskrom", 0)
	ROM_LOAD("3300disk.rom", 0x0000, 0x4000, CRC(549f1d90) SHA1(f1525de4e0b60a6687156c2a96f8a8b2044b6c56))
ROM_END

void msx1_state::cf3300(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: mb8877a, 1 3.5" SSDD drive
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 0, 4);  // 64KB RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK2_MB8877_SS, "diskrom", 3, 1, 1, 2, "diskrom");

	msx1(VDP_TMS9928A, SND_AY8910, config);
}

/* MSX - National FS-1300 */

ROM_START(fs1300)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("1300bios.rom", 0x0000, 0x8000, CRC(5ad03407) SHA1(c7a2c5baee6a9f0e1c6ee7d76944c0ab1886796c))
ROM_END

void msx1_state::fs1300(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(VDP_TMS9918A, SND_AY8910, config);
}

/* MSX - National FS-4000 */

ROM_START(fs4000)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("4000bios.rom",  0x0000, 0x8000, CRC(071135e0) SHA1(df48902f5f12af8867ae1a87f255145f0e5e0774))

	ROM_REGION(0x8000, "word", 0)
	ROM_LOAD("4000word.rom",  0x0000, 0x8000, CRC(950b6c87) SHA1(931d6318774bd495a32ec3dabf8d0edfc9913324))

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("4000kdr.rom",  0x0000, 0x8000, CRC(ebaa5a1e) SHA1(77bd67d5d10d459d343e79eafcd8e17eb0f209dd))

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("4000kfn.rom", 0, 0x20000, CRC(956dc96d) SHA1(9ed3ab6d893632b9246e91b412cd5db519e7586b))
ROM_END

void msx1_state::fs4000(machine_config &config)
{
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// MSX-Engine S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "word", 3, 0, 0, 2, "word");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "kdr");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM

	msx1(VDP_TMS9128, SND_YM2149, config);
}

/* MSX - National FS-4000 (Alt) */

ROM_START(fs4000a)
	ROM_REGION(0x8000 ,"mainrom", 0)
	ROM_LOAD("4000bios.rom",  0x0000, 0x8000, BAD_DUMP CRC(071135e0) SHA1(df48902f5f12af8867ae1a87f255145f0e5e0774)) // need verification

	ROM_REGION(0x8000, "word", 0)
	ROM_LOAD("4000wora.rom",  0x0000, 0x8000, BAD_DUMP CRC(52f4cdf7) SHA1(acbac3cb5b700254bed2cacc19fa54f1950f371d)) // need verification

	ROM_REGION(0x8000, "kdr", 0)
	ROM_LOAD("4000kdra.rom", 0x0000, 0x8000, BAD_DUMP CRC(b2db6bf5) SHA1(3a9a942ed888dd641cddf8deada1879c454df3c6)) // need verification

	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("4000kfn.rom", 0, 0x20000, BAD_DUMP CRC(956dc96d) SHA1(9ed3ab6d893632b9246e91b412cd5db519e7586b)) // need verification
ROM_END

void msx1_state::fs4000a(machine_config &config)
{
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// MSX-Engine S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "word", 3, 0, 0, 2, "word");
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "kdr");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 0, 4);  // 64KB RAM

	msx1(VDP_TMS9128, SND_YM2149, config);
}

/* MSX - Network DPC-200 */

/* MSX - Olympia DPC-200 */
// GSS Z8400A PS
// AY-3-8910A
// ic8 ROM 9256C-0047 R09256C-INT

/* MSX - Olympia PHC-2 */

ROM_START(phc2)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("r09256c-fra.ic8", 0x0000, 0x8000, CRC(d2110d66) SHA1(d3af963e2529662eae63f04a2530454685a1989f))
ROM_END

void msx1_state::phc2(machine_config &config)
{
	// AY-3-8910
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);  // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	// Expansion slot in slot #3

	msx1(VDP_TMS9129, SND_AY8910, config);
}

/* MSX - Olympia PHC-28 */

ROM_START(phc28)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("phc28bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(eceb2802) SHA1(195950173701abeb460a1a070d83466f3f53b337)) // need verification
ROM_END

void msx1_state::phc28(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	// Expansion slot in slot #3

	msx1(VDP_TMS9929A, SND_AY8910, config); // soundchip and videochip need verification
}

/* MSX - Panasonic CF-2700 (Germany) */

ROM_START(cf2700g)
	ROM_REGION(0x8000, "mainrom", 0)
	// mn23257rfa
	ROM_LOAD("cf2700g.ic32", 0x0000, 0x8000, CRC(4aa194f4) SHA1(69bf27b610e11437dad1f7a1c37a63179a293d12))
ROM_END

void msx1_state::cf2700g(machine_config &config)
{
	// NEC uPD780C
	// AY-3-8910A
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);  // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	add_cartridge_slot<2>(config, 3);

	msx1(VDP_TMS9129, SND_AY8910, config);
}

/* MSX - Panasonic CF-2700 (UK) */

ROM_START(cf2700uk)
	ROM_REGION(0x8000, "mainrom", 0)
	// mn23257rfa
	ROM_LOAD("cf2700uk.ic32", 0x0000, 0x8000, CRC(15e503de) SHA1(5e6b1306a30bbb46af61487d1a3cc1b0a69004c3))
ROM_END

void msx1_state::cf2700uk(machine_config &config)
{
	// NEC uPD780C
	// AY-3-8910A
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);  // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	add_cartridge_slot<2>(config, 3);

	msx1(VDP_TMS9129, SND_AY8910, config);
}

/* MSX - Panasonic FS-3900 */

/* MSX - Philips NMS 800 */
// SGS Z8400AB1
// AY-3-8910A
// TMS9129
// U3 - ST27256-25CP
// 0 Cartridge slots
// No printer port

/* MSX - Philips NMS 801 */

ROM_START(nms801)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("801bios.rom", 0x0000, 0x8000, CRC(fa089461) SHA1(21329398c0f350e330b353f45f21aa7ba338fc8d))
ROM_END

void msx1_state::nms801(machine_config &config)
{
	// SGS Z8400AB1
	// AY-3-8910A
	// FDC: None, 0 drives
	// 0 Cartridge slots
	// No printer port

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	m_hw_def.has_printer_port(false);
	msx1(VDP_TMS9129, SND_AY8910, config);
}

/* MSX - Philips VG-8000 */

ROM_START(vg8000)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("8000bios.rom", 0x0000, 0x8000, CRC(efd970b0) SHA1(42252cf87deeb58181a7bfec7c874190a1351779))
ROM_END

void msx1_state::vg8000(machine_config &config)
{
	// SGS Z8400AB1
	// AY-3-8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// No printer port

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	m_hw_def.has_printer_port(false);
	msx1(VDP_TMS9129, SND_AY8910, config);
}

/* MSX - Philips VG-8010 / VG-8010/00 */

ROM_START(vg8010)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("vg8000v1_0.663", 0x0000, 0x8000, CRC(efd970b0) SHA1(42252cf87deeb58181a7bfec7c874190a1351779))
ROM_END

void msx1_state::vg8010(machine_config &config)
{
	// SGS Z8400AB1
	// AY-3-8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// No printer port

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	m_hw_def.has_printer_port(false);
	msx1(VDP_TMS9129, SND_AY8910, config);
}

/* MSX - Philips VG-8010F / VG-8010/19 */

ROM_START(vg8010f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("8010fbios.663", 0x0000, 0x8000, CRC(df57c9ca) SHA1(898630ad1497dc9a329580c682ee55c4bcb9c30c))
ROM_END

void msx1_state::vg8010f(machine_config &config)
{
	// SGS Z8400AB1
	// AY-3-8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// No printer port

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	m_hw_def.has_printer_port(false);
	msx1(VDP_TMS9129, SND_AY8910, config);
}

/* MSX - Philips VG-8020-00 */

ROM_START(vg802000)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("8020-00bios.rom", 0x0000, 0x8000, CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db))
ROM_END

void msx1_state::vg802000(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(VDP_TMS9929A, SND_YM2149, config);
}

/* MSX - Philips VG-8020/19 / VG-8020F */

ROM_START(vg8020f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("8020-19bios.u11", 0x0000, 0x8000, CRC(70ce2d45) SHA1(ae4a6632d4456ef44603e72f5acd5bbcd6c0d124))
ROM_END

void msx1_state::vg8020f(machine_config &config)
{
	// YM2149 (in S-3527 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);   // 64KB RAM

	msx1(VDP_TMS9929A, SND_YM2149, config);
}

/* MSX - Philips VG-8020/20 */

ROM_START(vg802020)
	ROM_REGION(0x8000, "mainrom", 0)
	// m38256-k5 z-4 int rev1
	ROM_LOAD("8020-20bios.u11", 0x0000, 0x8000, CRC(a317e6b4) SHA1(e998f0c441f4f1800ef44e42cd1659150206cf79))
ROM_END

void msx1_state::vg802020(machine_config &config)
{
	// Zilog Z8400APS
	// YM2149 (in S-3527 MSX Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S-3527 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);   // 64KB RAM

	msx1(VDP_TMS9129, SND_YM2149, config);
}

/* MSX - Phonola VG-8000 (Italian market, mostly likely same as Philips VG-8000) */

/* MSX - Phonola VG-8010 (Italian market, mostly likely same as Philips VG-8010) */

/* MSX - Phonola VG-8020 (Italian market, mostly likely same as Philips VG-8020) */

/* MSX - Pioneer PX-7

|---------------------------------------|
|  CN1     CN2                          |
|                                       |
|                                       |
|  IC33                                 |---------------------------------|
|                                                      CN3                |
|   IC32   IC34            IC38  IC40                                     |
|                                                               IC20      |
|   IC15   IC18  IC43      IC8   IC35   IC6     |----IC3---|              |
|                                               |----------|    IC21      |
|   IC16   IC19  |---IC13---|    IC7    IC10                              |
|                |----------|                   IC36  IC29      ---       |
|   IC17   IC14                                     X2          | |       |
|                |--IC12---|     |----IC1-----|       IC37      |I|       |
|   IC28   IC11  |---------|     |------------|   X1            |C|       |
|                                                               |2|       |
|  |----IC4----| |----IC5----|   IC39  IC9      IC42  IC44      | |       |
|  |-----------| |-----------|                                  ---       |
|                                                                         |
|       IC45   IC31    IC30      IC41                                     |
|                                                                         |
|  CN4 CN5  CN6  CN7                                  CN8                 |
|-------------------------------------------------------------------------|

Notes:
  X1 - 3.579MHz
  X2 - 500kHz
  IC1 - Sharp LH0080A Z80A-CPU-D
  IC2 - TMS91289NL
  IC3 - MB111S112  Z10 (500kHz)
  IC4  - M5L8255AP-5
  IC5  - YM2149F
  IC6,IC7,IC8,IC10,IC45 - SN74LS367AN
  IC9 - SN74LS245N
  IC11,IC34 - SN74LS139N
  IC12 - YM2301-23908 / 53 18 85 A (might indicate a version)
  IC13 - Pioneer PD5031 2364-213 514100 (M5L2764-213)
  IC14,IC17,IC30,IC31 - SN74LS157N
  IC15-IC19 - MB81416-12
  IC20,IC21 - TMS4416-I5NL
  IC28 - SN74LS153N
  IC29 - SN74LS02N
  IC32 - SN74LS374N
  IC33 - M5218P
  IC35 - SN74LS74AN
  IC36 - SN74LS30N
  IC37-IC39 - SN74LS04N
  IC40,IC41 - SN74LS05N
  IC42 - SN74LS08N
  IC43,IC44 - SN74LS32N
  CN1 - Printer
  CN2 - Cassette recorder
  CN3 - Expansion slot
  CN4 - Keyboard
  CN5 - Keyboard
  CN6 - Controller #1
  CN7 - Controller #2
  CN8 - Expansion slot

 */

// BIOS is for an international keyboard while the machine has a Japanese layout
ROM_START(piopx7)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ym2301.ic12", 0x0000, 0x8000, BAD_DUMP CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))

	ROM_REGION(0x4000, "pbasic", 0)
	ROM_LOAD("pd5031.ic13", 0x0000, 0x2000, CRC(a5d0c0b9) SHA1(665d805f96616e1037f1823050657b7849899283)) // v1.0
	ROM_FILL(0x2000, 0x2000, 0x6e)
ROM_END

void msx1_state::piopx7(machine_config &config)
{
	// TMS9928ANL VDP with sync/overlay interface
	// AY-3-8910 PSG
	// Pioneer System Remote (SR) system control interface
	// FDC: None, 0 drives
	// 2 Cartridge slots

	// Line-level stereo audio input can be mixed with sound output, balance controlled with slider on front panel
	// Front-panel switch allows audio input to be passed through bypassing the mixing circuit
	// Line input can be muted under software control, e.g. when loading data from Laserdisc
	// Right channel of line input is additionally routed via some signal processing to the cassette input for loading data from Laserdisc

	// PSG port B bits 0-5 can be used to drive controller pins 1-6, 1-7, 2-6, 2-7, 1-8 and 2-8 low if 0 is written

	// Slot #2 7FFE is the SR control register LCON
	// Bit 7 R = /ACK (significant with acknowledge 1->0 with respect to remote control signal transmission)
	// Bit 0 R = RMCLK (clock produced by dividing CLK1/CLK2 frequency by 128)
	// Bit 0 W = /REM (high output with bit serial data output generated in synchronisation with RMCLK)

	// Slot #2 7FFF is the video overlay control register VCON
	// Bit 7 R = /EXTV (low when external video input available; high when not available)
	// Bit 7 W = Mute (line input signal muting)
	// Bit 0 R = INTEXV (interrupt available when external video signal OFF, reset on read)
	// Bit 0 W = /OVERLAY (0 = superimpose, 1 = non-superimpose)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_ROM, "pbasic", 2, 1, 1, "pbasic");
	add_cartridge_slot<2>(config, 3);

	msx1(VDP_TMS9928A, SND_AY8910, config);
}

/* MSX - Pioneer PX-7UK */

ROM_START(piopx7uk)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("px7ukbios.rom",   0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))

	ROM_REGION(0x4000, "pbasic", 0)
	ROM_LOAD("px7ukpbasic.rom", 0x0000, 0x2000, CRC(91e0df72) SHA1(4f0102cdc27216fd9bcdb9663db728d2ccd8ca6d)) // v1,1
	ROM_FILL(0x2000, 0x2000, 0x6e)

// Is this a cartridge that came with the machine?
// ROM_REGION(0x8000, "videoart", 0)
// ROM_LOAD("videoart.rom", 0x0000, 0x8000, CRC(0ba148dc) SHA1(b7b4e4cd40a856bb071976e6cf0f5e546fc86a78))
ROM_END

void msx1_state::piopx7uk(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_ROM, "pbasic", 2, 1, 1, "pbasic");
	add_cartridge_slot<2>(config, 3);

	msx1(VDP_TMS9129, SND_YM2149, config);
}

/* MSX - Pioneer PX-V60 */

ROM_START(piopxv60)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("pxv60bios.rom",   0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))

	ROM_REGION(0x4000, "pbasic", 0)
	ROM_LOAD("pd5031.rom", 0x0000, 0x2000, CRC(91e0df72) SHA1(4f0102cdc27216fd9bcdb9663db728d2ccd8ca6d)) // v1.1
	ROM_FILL(0x2000, 0x2000, 0x6E)
ROM_END

void msx1_state::piopxv60(machine_config &config)
{
	// Sharp LH0080A
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_ROM, "pbasic", 2, 1, 1, "pbasic");
	add_cartridge_slot<2>(config, 3);

	msx1(VDP_TMS9128, SND_YM2149, config);
}

/* MSX - Pioneer PX-V7 */

/* MSX - Radiola MK 180 */

/* MSX - Sakhr AH-200 */

/* MSX - Sakhr AX-100 */

/* MSX - Sakhr AX-150 */

ROM_START(ax150)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ax150bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(bd95c436) SHA1(5e094fca95ab8e91873ee372a3f1239b9a48a48d)) // need verification

	ROM_REGION(0x8000, "arabic", 0)
	ROM_LOAD("ax150arab.rom", 0x0000, 0x8000, BAD_DUMP CRC(339cd1aa) SHA1(0287b2ec897b9196788cd9f10c99e1487d7adbbb)) // need verification
ROM_END

void msx1_state::ax150(machine_config &config)
{
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// YM2220 (compatible with TMS9918)
	// MSX-Engine S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 0, 2, 2); // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "arabic", 0, 1, 1, 2, "arabic");
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 0, 2, 0, 2); // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	msx1(VDP_TMS9918, SND_YM2149, config);
}

/* MSX - Sakhr AX-170 */

ROM_START (ax170)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ax170bios.rom", 0x0000, 0x8000, CRC(bd95c436) SHA1(5e094fca95ab8e91873ee372a3f1239b9a48a48d))

	ROM_REGION(0x8000, "arabic", 0)
	ROM_LOAD("ax170arab.rom", 0x0000, 0x8000, CRC(339cd1aa) SHA1(0287b2ec897b9196788cd9f10c99e1487d7adbbb))
ROM_END

void msx1_state::ax170(machine_config &config)
{
	// AY-3-8910 in T7937
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// T6950 in T7937
	// T7937 (in ax170mk2)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "arabic", 1, 1, 2, "arabic");
	add_cartridge_slot<1>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 0, 4); // 64KB RAM
	add_cartridge_slot<2>(config, 3, 3);

	msx1(VDP_TMS9929A, SND_AY8910, config);
}

/* MSX - Sakhr AX-170F */

/* MSX - Sakhr AX-230 */

ROM_START (ax230)
	ROM_REGION(0x20000, "mainrom", 0)
	ROM_LOAD("qxxca0259.ic125", 0x0000, 0x20000, CRC(f1a3e650) SHA1(0340707c5de2310dcf5e569b7db4c6a6a5590cb7))

	ROM_REGION(0x100000, "games", 0)
	ROM_LOAD("qxxca0270.ic127", 0x00000, 0x100000, CRC(103c11c4) SHA1(620a209bdfdb65a22380031fce654bd1df61def2))
ROM_END

void msx1_state::ax230(machine_config &config)
{
	// AY-3-8910 in T7937
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// T6950 in T7937
	// T7937

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	// TODO: is and if so, how is the rest of ic125 accessed?
	add_internal_slot(config, MSX_SLOT_ROM, "arabic1", 1, 1, 1, "mainrom", 0xc000);
	add_internal_slot(config, MSX_SLOT_ROM, "arabic2", 1, 2, 1, "mainrom", 0x8000);
	add_cartridge_slot<1>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 0, 4); // 64KB RAM
	add_internal_slot(config, MSX_SLOT_AX230, "games", 3, 3, 1, 2, "games");

	msx1(VDP_TMS9918, SND_AY8910, config);
}

/* MSX - Sakhr AX-330 */

/* MSX - Sakhr AX-660 */

/* MSX - Sakhr AX-990 */

/* MSX - Salora MSX (prototypes) */

/* MSX - Samsung SPC-800 */

ROM_START(spc800)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("spc800bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(3ab0cd3b) SHA1(171b587bd5a947a13f3114120b6e7baca3b57d78)) // need verification

	ROM_REGION(0x4000, "hangul", 0)
	ROM_LOAD("spc800han.rom",  0x0000, 0x4000, BAD_DUMP CRC(5ae2b013) SHA1(1e7616261a203580c1044205ad8766d104f1d874)) // need verification
ROM_END

void msx1_state::spc800(machine_config &config)
{
	// AY-3-8910
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "hangul", 0, 2, 1, "hangul");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB?? RAM
	// Expansion slot in slot #3

	msx1(VDP_TMS9118, SND_AY8910, config);
}

/* MSX - Sanno PHC-SPC */

/* MSX - Sanno SPCmk-II */

/* MSX - Sanno SPCmk-III */

/* MSX - Sanyo MPC-1 / Wavy1 */

/* MSX - Sanyo MPC-10 / Wavy10 */

/* MSX - Sanyo MPC-64 */

ROM_START(mpc64)
	ROM_REGION(0x8000, "mainrom", 0)
	// hn613256p t22 5c1 ?
	ROM_LOAD("mpc64bios.ic111", 0x0000, 0x8000, BAD_DUMP CRC(d6e704ad) SHA1(d67be6d7d56d7229418f4e122f2ec27990db7d19)) // needs verification
ROM_END

void msx1_state::mpc64(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(VDP_TMS9928A, SND_AY8910, config);
}

/* MSX - Sanyo MPC-100 */

ROM_START(mpc100)
	ROM_REGION(0x8000, "mainrom", 0)
	// hn613256p 4j1 ?
	ROM_LOAD("mpc100bios.ic122", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))
ROM_END

void msx1_state::mpc100(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(VDP_TMS9929A, SND_AY8910, config);
}

/* MSX - Sanyo MPC-200 */

ROM_START(mpc200)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mpc200bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e)) // need verification
ROM_END

void msx1_state::mpc200(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// T6950
	// T7775 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "bios", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4); // 64KB RAM

	msx1(VDP_TMS9929A, SND_AY8910, config);
}

/* MSX - Sanyo MPC-200SP (same as Sanyo MPC-200 ?) */

ROM_START(mpc200sp)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("mpcsp200bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(bcd79900) SHA1(fc8c2b69351e60dc902add232032c2d69f00e41e)) // need verification
ROM_END

void msx1_state::mpc200sp(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2? Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4); // 64KB RAM

	msx1(VDP_TMS9929A, SND_AY8910, config);
}

/* MSX - Sanyo PHC-28L */

ROM_START(phc28l)
	ROM_REGION(0x8000, "mainrom", 0)
	// 5a1 hn613256p g42
	ROM_LOAD("28lbios.ic20", 0x0000, 0x8000, CRC(d2110d66) SHA1(d3af963e2529662eae63f04a2530454685a1989f))
ROM_END

void msx1_state::phc28l(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(VDP_TMS9929A, SND_YM2149, config);
}

/* MSX - Sanyo PHC-28S */

ROM_START(phc28s)
	ROM_REGION(0x8000, "mainrom", 0)
	// 4l1 hn613256p m74
	ROM_LOAD("28sbios.ic2", 0x0000, 0x8000, CRC(e5cf6b3c) SHA1(b1cce60ef61c058f5e42ef7ac635018d1a431168))
ROM_END

void msx1_state::phc28s(machine_config &config)
{
	// 10.738MHz and 3.574545MHz
	// AY8910
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 2);   // 32KB RAM

	msx1(VDP_TMS9929A, SND_AY8910, config);
}

/* MSX - Sanyo Wavy MPC-10 */

ROM_START(mpc10)
	ROM_REGION(0x8000, "mainrom", 0)
	// Split across 2 roms? (image hard to read)
	// ic14? hn613256p d24 ?
	// ic15? hn613256p d25 ?
	ROM_LOAD("mpc10.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx1_state::mpc10(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #2
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 2, 2);   // 32KB RAM

	msx1(VDP_TMS9918, SND_AY8910, config);
}

/* MSX - Schneider MC 810 */

/* MSX - Sharp Epcom HB-8000 (HotBit) */

ROM_START(hb8000)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_SYSTEM_BIOS(0, "v12", "v1.2")
	ROMX_LOAD("hotbit12.ic28", 0x0000, 0x8000, CRC(f59a4a0c) SHA1(9425815446d468058705bae545ffa13646744a87), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v11", "v1.1")
	ROMX_LOAD("hotbit11.ic28", 0x0000, 0x8000, CRC(b6942694) SHA1(663f8c512d04d213fa616b0db5eefe3774012a4b), ROM_BIOS(1))
	// v1.0 missing
ROM_END

void msx1_state::hb8000(machine_config &config)
{
	// AY8910 and YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(VDP_TMS9128, SND_AY8910, config);
}

/* MSX - Sharp Epcom HB-8000 (HotBit 1.3b) */

ROM_START(hotbi13b)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hotbit13b.rom", 0x0000, 0x8000, BAD_DUMP CRC(7a19820e) SHA1(e0c2bfb078562d15acabc5831020a2370ea87052)) // need verification
ROM_END

void msx1_state::hotbi13b(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2 Cartridge slots?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 4).set_total_size(0x10000);   // 64KB Mapper RAM

	msx1(VDP_TMS9928A, SND_AY8910, config);
}

/* MSX - Sharp Epcom HB-8000 (HotBit 1.3p) */

ROM_START(hotbi13p)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hotbit13p.rom", 0x0000, 0x8000, BAD_DUMP CRC(150e239c) SHA1(942f9507d206cd8156f15601fe8032fcf0e3875b)) // need verification
ROM_END

void msx1_state::hotbi13p(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2 Cartridge slots?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 4).set_total_size(0x10000);   // 64KB Mapper RAM

	msx1(VDP_TMS9928A, SND_AY8910, config);
}

/* MSX - Sincorp SBX (Argentina, homebrew) */

/* MSX - Sony HB-10 */

ROM_START(hb10)
	ROM_REGION(0x8000, "mainrom", 0)
	// 5h3 hn613256p c78
	// 3l1 hn613256p c78 ?
	ROM_LOAD("hb10bios.ic12", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx1_state::hb10(machine_config &config)
{
	// YM2149 (in S-S3527 MSX-Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// S3527 MSX-Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);  // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);

	msx1(VDP_TMS9118, SND_YM2149, config);
}

/* MSX - Sony HB-10B */

/* MSX - Sony HB-10D */
// ic12 - tmm23256p

/* MSX - Sony HB-10P */

ROM_START(hb10p)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("10pbios.ic12", 0x0000, 0x8000, CRC(0f488dd8) SHA1(5e7c8eab238712d1e18b0219c0f4d4dae180420d))
ROM_END

void msx1_state::hb10p(machine_config &config)
{
	// XTAL: 3.579545 + 22.168(VDP)
	// YM2149 (in S3527 MSX-Engine)
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// T6950

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	// A mirror of RAM appears in slot #0, page #3
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(VDP_TMS9929A, SND_YM2149, config);
}

/* MSX - Sony HB-20P */

ROM_START(hb20p)
	ROM_REGION(0x8000, "mainrom", 0)
	// lh2359z3
	ROM_LOAD("20pbios.ic12", 0x0000, 0x8000, CRC(15ddeb5c) SHA1(63050d2d21214a721cc55f152c22b7be8061ac33))
ROM_END

void msx1_state::hb20p(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// T6950A

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	// A mirror of RAM appears in slot #0, page #3
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(VDP_TMS9929A, SND_YM2149, config);
}

/* MSX - Sony HB-201 */

ROM_START(hb201)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("201bios.ic9", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("201note.ic8", 0x0000, 0x4000, CRC(74567244) SHA1(0f4f09f1a6ef7535b243afabfb44a3a0eb0498d9))
ROM_END

void msx1_state::hb201(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 2, 1, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(VDP_TMS9118, SND_YM2149, config);
}

/* MSX - Sony HB-201P */

ROM_START(hb201p)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("201pbios.rom.ic9", 0x0000, 0x8000, CRC(0f488dd8) SHA1(5e7c8eab238712d1e18b0219c0f4d4dae180420d))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("201pnote.rom.ic8", 0x0000, 0x4000, CRC(1ff9b6ec) SHA1(e84d3ec7a595ee36b50e979683c84105c1871857))
ROM_END

void msx1_state::hb201p(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 2, 1, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(VDP_TMS9129, SND_YM2149, config);
}

/* MSX - Sony HB-501F */
// ic1 - tmm23256p
// YM2149
// TMS9129
// S3527

/* MSX - Sony HB-501P */

ROM_START(hb501p)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("501pbios.rom", 0x0000, 0x8000, CRC(0f488dd8) SHA1(5e7c8eab238712d1e18b0219c0f4d4dae180420d))
ROM_END

void msx1_state::hb501p(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// T6950
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(VDP_TMS9929A, SND_YM2149, config);
}

/* MSX - Sony HB-55 (Version 1) */

ROM_START(hb55)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hb55bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821)) // need verification

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("hb55note.rom", 0x0000, 0x2000, BAD_DUMP CRC(5743ab55) SHA1(b9179db93608c4da649532e704f072e0a3ea1b22)) // need verification
ROM_END

void msx1_state::hb55(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 2, 1, "firmware");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #3

	msx1(VDP_TMS9918A, SND_AY8910, config);
}

/* MSX - Sony HB-55D, is this HB-55 2nd version? */

ROM_START(hb55d)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("55dbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(7e2b32dd) SHA1(38a645febd0e0fe86d594f27c2d14be995acc730)) // need verification

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("55dnote.rom", 0x0000, 0x4000, BAD_DUMP CRC(8aae0494) SHA1(97ce59892573cac3c440efff6d74c8a1c29a5ad3)) // need verification
ROM_END

void msx1_state::hb55d(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2 Cartridge slots?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 2, 1, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 3, 1);   // 16KB RAM
	add_cartridge_slot<2>(config, 3);

	msx1(VDP_TMS9929A, SND_AY8910, config);
}

/* MSX - Sony HB-55P */

ROM_START(hb55p)
	ROM_REGION(0x8000, "mainrom", 0)
	// Are there machines with ic42 and ic43 populated like this?
	// Image on msx.org shows only ic42 and ic44 populated (for hb-55)
//  ROM_LOAD("55pbios.ic42", 0x0000, 0x4000, CRC(24c198be) SHA1(7f8c94cb8913db32a696dec80ffc78e46693f1b7))
//  ROM_LOAD("55pbios.ic43", 0x4000, 0x4000, CRC(e516e7e5) SHA1(05fedd4b9bfcf4949020c79d32c4c3f03a54fb62))
	ROM_LOAD("55pbios.ic42", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("55pnote.ic44", 0x0000, 0x4000, CRC(492b12f8) SHA1(b262aedc71b445303f84efe5e865cbb71fd7d952))
ROM_END

void msx1_state::hb55p(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 2, 1, "firmware");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 3);

	msx1(VDP_TMS9929A, SND_AY8910, config);
}

/* MSX - Sony HB-75 */

ROM_START(hb75)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("75bios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("75note.rom", 0x0000, 0x4000, CRC(2433dd0b) SHA1(5f26319aec3354a94e2a98e07b2c70046bc45417))
ROM_END

void msx1_state::hb75(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 2, 1, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	// Expansion slot in slot #3

	msx1(VDP_TMS9918A, SND_AY8910, config);
}

/* MSX - Sony HB-75AS */

/* MSX - Sony HB-75B */

/* MSX - Sony HB-75D */

ROM_START(hb75d)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("75dbios.rom", 0x0000, 0x8000, CRC(7e2b32dd) SHA1(38a645febd0e0fe86d594f27c2d14be995acc730))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("75dnote.rom", 0x0000, 0x4000, CRC(8aae0494) SHA1(97ce59892573cac3c440efff6d74c8a1c29a5ad3))
ROM_END

void msx1_state::hb75d(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 2, 1, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	add_cartridge_slot<2>(config, 3);

	msx1(VDP_TMS9929A, SND_AY8910, config);
}

/* MSX - Sony HB-75F */

/* MSX - Sony HB-75P */

ROM_START(hb75p)
	ROM_REGION(0x8000, "mainrom", 0)
	// Are there machines with ic42 and ic43 populated like this?
	// HB-75P internal image on msx.org only has 2 roms populated (ic42 and ic44)
//  ROM_LOAD("75pbios.ic42", 0x0000, 0x4000, CRC(24c198be) SHA1(7f8c94cb8913db32a696dec80ffc78e46693f1b7))
//  ROM_LOAD("75pbios.ic43", 0x4000, 0x4000, CRC(e516e7e5) SHA1(05fedd4b9bfcf4949020c79d32c4c3f03a54fb62))
	ROM_LOAD("75pbios.ic42", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("75pnote.ic44", 0x0000, 0x4000, CRC(492b12f8) SHA1(b262aedc71b445303f84efe5e865cbb71fd7d952))
ROM_END

void msx1_state::hb75p(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 2, 1, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	add_cartridge_slot<2>(config, 3);

	msx1(VDP_TMS9929A, SND_AY8910, config);
}

/* MSX - Sony HB-101 */

ROM_START(hb101)
	ROM_REGION(0x8000, "mainrom", 0)
	// 4l1 hn613256p d78
	ROM_LOAD("101pbios.ic108", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))

	ROM_REGION(0x4000, "firmware", 0)
	// m38128a-91 4202
	ROM_LOAD("101pnote.ic111", 0x0000, 0x4000, CRC(f62e75f6) SHA1(64adb7fcf9b86f59d8658badb02f58e61bb15712))
ROM_END

void msx1_state::hb101(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 1, 1, "firmware");

	msx1(VDP_TMS9118, SND_YM2149, config);
}

/* MSX - Sony HB-101P */

ROM_START(hb101p)
	ROM_REGION(0x8000, "mainrom", 0)
	// m38256-7b 5411
	ROM_LOAD("101pbios.ic9", 0x0000, 0x8000, CRC(0f488dd8) SHA1(5e7c8eab238712d1e18b0219c0f4d4dae180420d))

	ROM_REGION(0x4000, "firmware", 0)
	// m38128a-f6 5501
	ROM_LOAD("101pnote.ic8", 0x0000, 0x4000, CRC(525017c2) SHA1(8ffc24677fd9d2606a79718764261cdf02434f0a))
ROM_END

void msx1_state::hb101p(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 1, 1, "firmware");

	msx1(VDP_TMS9929A, SND_YM2149, config);
}

/* MSX - Sony HB-701 */

/* MSX - Sony HB-701FD */

ROM_START(hb701fd)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hb701fdbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821)) // need verification

	ROM_REGION(0x4000, "diskrom", 0)
	// FDC register contents at 3ff8-3fff
	ROM_LOAD("hb701fddisk.rom", 0x0000, 0x4000, BAD_DUMP CRC(71961d9d) SHA1(2144036d6573d666143e890e5413956bfe8f66c5)) // need verification
ROM_END

void msx1_state::hb701fd(machine_config &config)
{
	// YM2149
	// FDC: WD2793, 1 3.5" SSDD drive
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 0, 4);  // 64KB RAM
	add_internal_disk_mirrored(config, MSX_SLOT_DISK1_WD2793_SS, "disk", 3, 1, 1, 2, "diskrom");

	msx1(VDP_TMS9928A, SND_YM2149, config);
}

/* MSX - Spectravideo SVI-728 */

ROM_START(svi728)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("728bios.rom", 0x0000, 0x8000, CRC(1ce9246c) SHA1(ea6a82cf8c6e65eb30b98755c8577cde8d9186c0))
ROM_END

void msx1_state::svi728(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 1 Cartridge slots, 1 Expansion slot (eg for SVI-707)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "mainmirror", 0, 2, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);  // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	// Expansion slot (for eg SVI-707) in slot #3

	msx1(VDP_TMS9129, SND_AY8910, config);
}

/* MSX - Spectravideo SVI-728 (Arabic) */

/* MSX - Spectravideo SVI-728 (Danish/Norwegian) */

/* MSX - Spectravideo SVI-728 (Spanish) */

ROM_START(svi728es)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("728esbios.rom", 0x0000, 0x8000, CRC(76c5e381) SHA1(82415ee031721d1954bfa42e1c6dd79d71c692d6))
ROM_END

/* MSX - Spectravideo SVI-728 (Swedish/Finnish) */

/* MSX - Talent DPC-200 */

// Spanish keyboard
ROM_START(tadpc200)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("dpc200bios.rom", 0x0000, 0x8000, CRC(01ad4fab) SHA1(d5bf4814ea694481c8badbb8de8d56a08ee03cc0))
ROM_END

// International keyboard
ROM_START(tadpc200b)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("dpc200altbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db)) // need verification
ROM_END

void msx1_state::tadpc200(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// DW64MX1

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 1, 0, 4);  // 64KB RAM
	add_cartridge_slot<1>(config, 2);
	// Expansion slot in slot #3

	msx1(VDP_TMS9129, SND_AY8910, config);
}

/* MSX - Talent DPS-201 */

/* MSX - Toshiba HX-10 / HX-10P
Code on PCB: MSX TUK
        |---------------------------|-------------------|-------------|
        |   CN1  CN2  CN3  CN4               CN5                      |
        |                        |---------------------------|        |
        |                        |---------------------------|        |
        |                                    CN6                      |
        |                        IC40                                 |
        |                                                         CN7 |
        |                      IC38     IC32     IC33     IC37        |
        |                                                             |
        |                      Q2       IC31     IC34     IC35        |
        |    Q1                                                   CN8 |
        |                                                 IC39        |
        |   |--IC15------| |--IC2----|   |----IC1-----|               |
        |   |------------| |---------|   |------------|               |
        |                                                 IC30        |
        |                 IC3    IC4                              CN9 |
        |                               |-----IC15-------|            |
        |  IC17   IC18    IC7    IC8    |----------------|            |
        |                                                 IC27        |
        |  IC19   IC20    IC9    IC10   |----IC25------|              |
|----|  |                               |--------------|  IC26        |
| Q  |  |  IC21   IC22    IC11   IC12                                 |
|    |  |                                                             |
| S  |  |  IC23   IC24    IC13   IC14    IC29             IC28        |
|  L |  |                                                             |
|    |  |                                 CN11   CN10                 |
|----|  |-------------------------------------------------------------|

Notes:
  Mainboard components:
   IC1               - Sharp LH0080A Z80A-CPU-D
   IC2               - MB83256
   IC3,IC4,IC27,IC28 - Texas Instruments SN74LS157N
   IC7-IC14          - HM4864AP
   IC15              - Toshiba TCX-1007 (64pin custom chip)
   IC16              - 40pin chip covered with some kind of heatsink(?), probably TMS9929A
   IC17-IC24         - 4116-3
   IC25              - AY-3-8910A
   IC26              - SN74LS09N
   IC29              - HD74LS145P
   IC30-IC34         - M74LS367AP
   IC35              - MB74LS74A
   IC37              - HD74LS373P
   IC38              - Toshiba TC74HCU04P
   IC39              - HD74LS08P
   IC40              - TA75559P
   Q1                - 10687.5
   Q2                - 3579545
   CN1               - Cassette connector
   CN2               - RF connector
   CN3               - Audio connector
   CN4               - Video connector
   CN5               - Expansion connector
   CN6               - Cartridge connector
   CN7               - Printer connector
   CN8               - Joystick 2 connector
   CN9               - Joystick 1 connector
   CN10              - Keyboard connector 1
   CN11              - Keyboard connector 2

  Extra pcb (video related?) components::
   Q - 4.433619
   S - 74LS04
   L - LVA510
 */

ROM_START(hx10)
	ROM_REGION(0x8000, "mainrom", 0)
	// tmm23256p
	ROM_LOAD("hx10bios.ic2", 0x0000, 0x8000, CRC(5486b711) SHA1(4dad9de7c28b452351cc12910849b51bd9a37ab3))
ROM_END

void msx1_state::hx10(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 1 Cartridge slot, 1 Toshiba Expension slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	// Expansion slot in slot #3

	msx1(VDP_TMS9929A, SND_AY8910, config);
}

/* MSX - Toshiba HX-10AA */

/* MSX - Toshiba HX-10D */

ROM_START(hx10d)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx10dbios.ic5", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx1_state::hx10d(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);   // 64KB RAM
	// Expansion slot in slot #3

	m_hw_def.has_printer_port(false);
	msx1(VDP_TMS9918A, SND_AY8910, config);
}

/* MSX - Toshiba HX-10DP */

ROM_START(hx10dp)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx10dpbios.ic2", 0x0000, 0x8000, BAD_DUMP CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821)) // need verification
ROM_END

void msx1_state::hx10dp(machine_config &config)
{
	// AY8910?
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);   // 64KB RAM
	// Expansion slot in slot #3

	msx1(VDP_TMS9918A, SND_AY8910, config);
}

/* MSX - Toshiba HX-10DPN */

/* MSX - Toshiba HX-10E */

ROM_START(hx10e)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx10ebios.rom", 0x0000, 0x8000, BAD_DUMP CRC(5486b711) SHA1(4dad9de7c28b452351cc12910849b51bd9a37ab3)) // need verification
ROM_END

void msx1_state::hx10e(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);   // 64KB RAM
	// Expansion slot in slot #3

	msx1(VDP_TMS9929A, SND_AY8910, config);
}

/* MSX - Toshiba HX-10F */

ROM_START(hx10f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx10fbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(e0e894b7) SHA1(d99eebded5db5fce1e072d08e642c0909bc7efdd)) // need verification
ROM_END

void msx1_state::hx10f(machine_config &config)
{
	// AY8910?
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);   // 64KB RAM
	// Expansion slot in slot #3

	msx1(VDP_TMS9929A, SND_AY8910, config);
}

/* MSX - Toshiba HX-10I */

/* MSX - Toshiba HX-10S */

// BIOS is for an international keyboard but the machine had a japanese keyboard layout so that cannot be correct
ROM_START(hx10s)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx10sbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(5486b711) SHA1(4dad9de7c28b452351cc12910849b51bd9a37ab3)) // need verification
ROM_END

void msx1_state::hx10s(machine_config &config)
{
	// AY8910?
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #3

	m_hw_def.has_printer_port(false);
	msx1(VDP_TMS9918A, SND_AY8910, config);
}

/* MSX - Toshiba HX-10SA */

ROM_START(hx10sa)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx10sabios.rom", 0x0000, 0x8000, BAD_DUMP CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821)) // need verification
ROM_END

void msx1_state::hx10sa(machine_config &config)
{
	// AY8910?
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #3

	m_hw_def.has_printer_port(false);
	msx1(VDP_TMS9918A, SND_AY8910, config);
}

/* MSX - Toshiba HX-10SF */

/* MSX - Toshiba HX-20 */

// BIOS is for an internation keyboard but the machine had japanese keyboard layout
// Firmware is in English
ROM_START(hx20)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx20bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db)) // need verification

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("hx20word.rom", 0x0000, 0x8000, BAD_DUMP CRC(39b3e1c0) SHA1(9f7cfa932bd7dfd0d9ecaadc51655fb557c2e125)) // need verification
ROM_END

void msx1_state::hx20(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// HX-R701 RS-232 optional
	// T6950

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 3, 0, 0, 2);   // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 3, 1, 2, "firmware");

	msx1(VDP_TMS9129, SND_YM2149, config);
}

/* MSX - Toshiba HX-20AR */
// TMS9128

/* MSX - Toahiba HX-20E */

ROM_START(hx20e)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("tmm23256p_2023.ic2", 0x0000, 0x8000, CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db))

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("tc53257p_2047.ic3", 0x0000, 0x8000, CRC(39b3e1c0) SHA1(9f7cfa932bd7dfd0d9ecaadc51655fb557c2e125))
ROM_END

void msx1_state::hx20e(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// T6950

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 3, 0, 0, 2);   // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 3, 1, 2, "firmware");

	msx1(VDP_TMS9129, SND_YM2149, config);
}

/* MSX - Toshiba HX-20I */

ROM_START(hx20i)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx20ibios.rom", 0x0000, 0x8000, BAD_DUMP CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db)) // need verification

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("hx20iword.rom", 0x0000, 0x8000, BAD_DUMP CRC(39b3e1c0) SHA1(9f7cfa932bd7dfd0d9ecaadc51655fb557c2e125)) // need verification
ROM_END

void msx1_state::hx20i(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// T6950

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 3, 0, 0, 2);   // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 3, 1, 2, "firmware");

	msx1(VDP_TMS9129, SND_YM2149, config);
}

/* MSX - Toshiba HX-21 */

ROM_START(hx21)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("tmm23256p_2011.ic2", 0x0000, 0x8000, CRC(83ba6fde) SHA1(01600d06d83072d4e757b29728555efde2c79705))

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("tmm23256p_2014.ic3", 0x0000, 0x8000, CRC(87508e78) SHA1(4e2ec9c0294a18a3ab463f182f9333d2af68cdca))
ROM_END

void msx1_state::hx21(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// HX-R701 RS-232 optional
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 3, 0, 0, 4);   // 64KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 3, 1, 2, "firmware");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware_mirror1", 3, 3, 0, 1, "firmware");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware_mirror2", 3, 3, 3, 1, "firmware", 0x4000);

	msx1(VDP_TMS9928A, SND_AY8910, config);
}

/* MSX - Toshiba HX-21F */

ROM_START(hx21f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx21fbios.rom", 0x0000, 0x8000, BAD_DUMP CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db)) // need verification

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("hx21fword.rom", 0x0000, 0x8000, BAD_DUMP CRC(f9e29c66) SHA1(3289336b2c12161fd926a7e5ce865770ae7038af)) // need verification
ROM_END

void msx1_state::hx21f(machine_config &config)
{
	// AY8910/YM2149?
	// FDC: None, 0 drives
	// 2 Cartridge slots

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 3, 0, 2, 2);   // 32KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 3, 1, 2, "firmware");

	msx1(VDP_TMS9929A, SND_AY8910, config);
}

/* MSX - Toshiba HX-22 */

ROM_START(hx22)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("tms23256p_2011.ic2", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("tms23256p_2014.ic3", 0x0000, 0x8000, CRC(87508e78) SHA1(4e2ec9c0294a18a3ab463f182f9333d2af68cdca))
ROM_END

void msx1_state::hx22(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// RS232C builtin

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 3, 0, 0, 4);   // 64KB RAM
	add_internal_slot_irq<3>(config, MSX_SLOT_RS232_TOSHIBA, "firmware", 3, 3, 1, 2, "firmware");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware_mirror1", 3, 3, 0, 1, "firmware");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware_mirror2", 3, 3, 3, 1, "firmware", 0x4000);

	msx1(VDP_TMS9928A, SND_AY8910, config);
}

/* MSX - Toshiba HX-22CH */

/* MSX - Toshibe HX-22GB */

/* MSX - Toshiba HX-22I */

ROM_START(hx22i)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("tmm23256p_2023.ic2", 0x0000, 0x8000, CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db))

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("tmm23256p_2046.ic3", 0x0000, 0x8000, CRC(f9e29c66) SHA1(3289336b2c12161fd926a7e5ce865770ae7038af))
ROM_END

void msx1_state::hx22i(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// RS232C builtin
	// Z80: LH0080A

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 3, 0, 2, 2);   // 32KB RAM
	add_internal_slot_irq<3>(config, MSX_SLOT_RS232_TOSHIBA, "firmware", 3, 3, 1, 2, "firmware");

	msx1(VDP_TMS9929A, SND_AY8910, config);
}

/* MSX - Toshiba HX-30 */

/* MSX - Tosbiba HX-31 */

/* MSX - Toshiba HX-32 */

ROM_START(hx32)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hx32bios.ic3", 0x0000, 0x8000, CRC(83ba6fde) SHA1(01600d06d83072d4e757b29728555efde2c79705))

	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("hx32firm.ic2", 0x0000, 0x8000, CRC(efc3aca7) SHA1(ed589da7f359a4e139a23cd82d9a6a6fa3d70db0))

	// Same as HX-M200 Kanji cartridge
	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("hx32kfn.rom", 0x0000, 0x20000, CRC(d23d4d2d) SHA1(db03211b7db46899df41db2b1dfbec972109a967))
ROM_END

void msx1_state::hx32(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// HX-R701 RS-232 optional
	// T6950

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 3, 0, 0, 4);   // 64KB RAM
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 3, 3, 1, 2, "firmware");

	msx1(VDP_TMS9928A, SND_YM2149, config);
}

/* MSX - Toshiba HX-51I */

ROM_START(hx51i)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("tmm23256ad_2023.ic8", 0x0000, 0x8000, CRC(8205795e) SHA1(829c00c3114f25b3dae5157c0a238b52a3ac37db))
ROM_END

void msx1_state::hx51i(machine_config &config)
{
	// AY8910 in T7937
	// FDC: None, 0 drives
	// 2 Cartridge slots
	// T6950 in T7937
	// T7937

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram1", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM, "ram2", 3, 0, 2);   // 32KB RAM

	msx1(VDP_TMS9918, SND_AY8910, config);
}

/* MSX - Toshiba HX-52 */

/* MSX - Triton PC64 */

/* MSX - Vestel FC-200 */

/* MSX - Victor HC-5 */

ROM_START(hc5)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hc5bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821)) // need verification
ROM_END

void msx1_state::hc5(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives,
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1); // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #2
	// Module slot in slot #3

	msx1(VDP_TMS9928A, SND_YM2149, config);
}

/* MSX - Victor HC-6 */

ROM_START(hc6)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hc6bios.rom", 0x0000, 0x8000, BAD_DUMP CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821)) // need verification
ROM_END

void msx1_state::hc6(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives,
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2); // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #2
	// Module slot in slot #3

	msx1(VDP_TMS9928A, SND_YM2149, config);
}

/* MSX - Victor HC-7 */

ROM_START(hc7)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("hc7bios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))

	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("hc7firm.rom", 0x0000, 0x4000, CRC(7d62951d) SHA1(e211534064ea6f436f2e7458ded35c398f17b761))
ROM_END

void msx1_state::hc7(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives,
	// 2 Cartridge slots?

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_ROM, "firmware", 0, 3, 1, "firmware");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4); // 64KB RAM
	add_cartridge_slot<2>(config, 3);

	msx1(VDP_TMS9928A, SND_AY8910, config);
}

/* MSX - Victor HC-30 */

/* MSX - Victor HC-60 */

/* MSX - Wandy DPC-200 */

/* MSX - Yamaha CX5 */

/* MSX - Yamaha CX5F (with SFG01) */

ROM_START(cx5f1)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("cx5fbios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
//  ROM_LOAD("cx5fbios.rom", 0x0000, 0x8000, CRC(dc662057) SHA1(36d77d357a5fd15af2ab266ee66e5091ba4770a3))
ROM_END

void msx1_state::cx5f1(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Yamaha expansion slot (50 pins)
	// 1 Yamaha module slot (60 pins)
	// S-5327 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2); // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	// Expansion bus in slot #2
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, msx_yamaha_60pin, "sfg01");

	msx1(VDP_TMS9928A, SND_YM2149, config);
}

/* MSX - Yamaha CX5F (with SFG05) */

ROM_START(cx5f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("cx5fbios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
//  ROM_LOAD("cx5fbios.rom", 0x0000, 0x8000, CRC(dc662057) SHA1(36d77d357a5fd15af2ab266ee66e5091ba4770a3))
ROM_END

void msx1_state::cx5f(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Yamaha expansion slot (50 pins)
	// 1 Yamaha module slot (60 pins)
	// S-5327 MSX Engine

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2); // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	// Expansion bus in slot #2
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, msx_yamaha_60pin, "sfg05");

	msx1(VDP_TMS9928A, SND_YM2149, config);
}

/* MSX - Yamaha CX5MA (Australia / New Zealand */

/* MSX - Yamaha CX5MC (Canada) */

/* MSX - Yamaha CX5ME (UK) */

/* MSX - Yamaha CX5MF (France) */

/* MSX - Yamaha CX5MG (Germany) */

/* MSX - Yamaha CX5MS (Scandinavia) */

/* MSX - Yamaha CX5MU (USA) */

ROM_START(cx5mu)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ym2301-23907.tmm23256p-2011", 0x0000, 0x8000, CRC(e2242b53) SHA1(706dd67036baeec7127e4ccd8c8db8f6ce7d0e4c))
ROM_END

void msx1_state::cx5mu(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Expansion slot (50 pins)
	// 1 Module slot (60 pins interface instead of regular 50 pin cartridge interface)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #2
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, msx_yamaha_60pin, "sfg01");

	msx1(VDP_TMS9918A, SND_YM2149, config);
}

/* MSX - Yamaha SX-100 */

ROM_START(sx100)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("sx100bios.rom", 0x0000, 0x8000, CRC(3b08dc03) SHA1(4d0c37ad722366ac7aa3d64291c3db72884deb2d))
ROM_END

void msx1_state::sx100(machine_config &config)
{
	// YM2149 in S3527
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// YM2220, TMS9918 compatible
	// S3527

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);

	msx1(VDP_TMS9918, SND_YM2149, config);
}

/* MSX - Yamaha YIS-303 */

// BIOS is for an international keyboard while the machine has a Japanese layout
ROM_START(yis303)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("ym2301-23907.tmm23256p-2011", 0x0000, 0x8000, BAD_DUMP CRC(e2242b53) SHA1(706dd67036baeec7127e4ccd8c8db8f6ce7d0e4c))
ROM_END

void msx1_state::yis303(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Yamaha expansion slot (50 pin)
	// 1 Yamaha module slot (60 pin)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 3, 1);   // 16KB RAM
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #2
	// mounting sfg01 works, sfg05 does not
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, 0, msx_yamaha_60pin, nullptr);

	m_hw_def.has_printer_port(false);
	msx1(VDP_TMS9918A, SND_YM2149, config);
}

/* MSX - Yamaha YIS-503 */

ROM_START(yis503)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yis503bios.rom", 0x0000, 0x8000, CRC(ee229390) SHA1(302afb5d8be26c758309ca3df611ae69cced2821))
ROM_END

void msx1_state::yis503(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Yamaha expansion slot
	// 1 Yamaha module slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2);   // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #2
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, msx_yamaha_60pin, nullptr);

	msx1(VDP_TMS9928A, SND_YM2149, config);
}

/* MSX - Yamaha YIS-503F */

ROM_START(yis503f)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yis503f.rom", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))
ROM_END

void msx1_state::yis503f(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 1 Cartridge slot
	// 1 Yamaha expansion slot (50 pin)
	// 1 Yamaha module slot (60 pin)

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 0, 2, 2);  // 32KB RAM
	add_cartridge_slot<1>(config, 1);
	// Expansion slot in slot #2
	add_cartridge_slot<3>(config, MSX_SLOT_YAMAHA_EXPANSION, "module", 3, msx_yamaha_60pin, nullptr);

	msx1(VDP_TMS9929A, SND_YM2149, config);
}

/* MSX - Yamaha YIS-603 */

/* MSX - Yashica YC-64 */

ROM_START(yc64)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("yc64bios.u20", 0x0000, 0x8000, CRC(e9ccd789) SHA1(8963fc041975f31dc2ab1019cfdd4967999de53e))
ROM_END

void msx1_state::yc64(machine_config &config)
{
	// YM2149
	// FDC: None, 0 drives
	// 1 Cartridge slot

	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 3, 0, 4);  // 64KB RAM

	msx1(VDP_TMS9929A, SND_YM2149, config);
}

/* MSX - Yeno DPC-64 */

/* MSX - Yeno MX64 */

ROM_START(mx64)
	ROM_REGION(0x8000, "mainrom", 0)
	ROM_LOAD("tmm23256p_7953.ic2d", 0x0000, 0x8000, BAD_DUMP CRC(e0e894b7) SHA1(d99eebded5db5fce1e072d08e642c0909bc7efdd)) // need verification
ROM_END

void msx1_state::mx64(machine_config &config)
{
	// AY8910
	// FDC: None, 0 drives
	// 2 Cartridge slots

	// slot layout needs verification
	add_internal_slot(config, MSX_SLOT_ROM, "mainrom", 0, 0, 2, "mainrom");
	add_cartridge_slot<1>(config, 1);
	add_internal_slot(config, MSX_SLOT_RAM, "ram", 2, 0, 4);  // 64KB RAM
	add_cartridge_slot<2>(config, 3);

	msx1(VDP_TMS9928A, SND_AY8910, config);
}

} // anonymous namespace


/*   YEAR  NAME        PARENT    COMPAT MACHINE     INPUT     CLASS      INIT        COMPANY       FULLNAME */
/* MSX1 */
COMP(1986, perfect1,   0,        0,     perfect1,   msx,      msx1_state, empty_init, "Bawareth", "Perfect MSX1 (MSX1, Middle East)", 0)
COMP(1985, canonv8,    0,        0,     canonv8,    msxjp,    msx1_state, empty_init, "Canon", "V-8 (MSX1, Japan)", 0)
COMP(1984, canonv10,   canonv20, 0,     canonv10,   msxjp,    msx1_state, empty_init, "Canon", "V-10 (MSX1, Japan)", 0)
COMP(1984, canonv20,   0,        0,     canonv20,   msxjp,    msx1_state, empty_init, "Canon", "V-20 (MSX1, Japan)", 0)
COMP(1985, canonv20e,  canonv20, 0,     canonv20e,  msxuk,    msx1_state, empty_init, "Canon", "V-20E (MSX1, UK)", 0)
COMP(1985, canonv20f,  canonv20, 0,     canonv20e,  msxfr,    msx1_state, empty_init, "Canon", "V-20F (MSX1, France)", 0)
COMP(198?, canonv20g,  canonv20, 0,     canonv20e,  msxde,    msx1_state, empty_init, "Canon", "V-20G (MSX1, Germany)", 0)
COMP(198?, canonv20s,  canonv20, 0,     canonv20e,  msxsp,    msx1_state, empty_init, "Canon", "V-20S (MSX1, Spain)", 0)
COMP(1985, mx10,       mx15,     0,     mx10,       msxjp,    msx1_state, empty_init, "Casio", "MX-10 (MSX1, Japan)", 0)
COMP(1987, mx101,      mx15,     0,     mx101,      msxjp,    msx1_state, empty_init, "Casio", "MX-101 (MSX1, Japan)", 0)
COMP(1986, mx15,       0,        0,     mx15,       msx,      msx1_state, empty_init, "Casio", "MX-15 (MSX1, International)", 0)
COMP(1984, pv7,        pv16,     0,     pv7,        msxjp,    msx1_state, empty_init, "Casio", "PV-7 (MSX1, Japan)", 0)
COMP(1985, pv16,       0,        0,     pv16,       msxjp,    msx1_state, empty_init, "Casio", "PV-16 (MSX1, Japan)", 0)
COMP(1984, cpc88,      0,        0,     cpc88,      msxkr,    msx1_state, empty_init, "Daewoo", "CPC-88 (MSX1, Korea)", 0)
COMP(1984, dpc100,     dpc200,   0,     dpc100,     msxkr,    msx1_state, empty_init, "Daewoo", "IQ-1000 DPC-100 (MSX1, Korea)", 0)
COMP(1984, dpc180,     dpc200,   0,     dpc180,     msxkr,    msx1_state, empty_init, "Daewoo", "IQ-1000 DPC-180 (MSX1, Korea)", 0)
COMP(1984, dpc200,     0,        0,     dpc200,     msxkr,    msx1_state, empty_init, "Daewoo", "IQ-1000 DPC-200 (MSX1, Korea)", 0)
COMP(1985, dpc200e,    0,        0,     dpc200e,    msxfr,    msx1_state, empty_init, "Daewoo", "DPC-200E (MSX1, French)", 0)
COMP(1986, cpc50a,     cpc51,    0,     cpc50a,     msxkr,    msx1_state, empty_init, "Daewoo", "Zemmix CPC-50A (MSX1, Korea)", 0)
COMP(1987, cpc50b,     cpc51,    0,     cpc50b,     msxkr,    msx1_state, empty_init, "Daewoo", "Zemmix CPC-50B (MSX1, Korea)", 0)
COMP(1988, cpc51,      0,        0,     cpc51,      msxkr,    msx1_state, empty_init, "Daewoo", "Zemmix CPC-51 (MSX1, Korea)", 0)
COMP(1985, dgnmsx,     0,        0,     dgnmsx,     msxuk,    msx1_state, empty_init, "Eurohard S.A.", "Dragon MSX-64 (MSX1, Spain)", 0)
COMP(1985, fdpc200,    0,        0,     fdpc200,    msx,      msx1_state, empty_init, "Fenner", "DPC-200 (MSX1, Italy)", 0)
COMP(1985, fpc500,     0,        0,     fpc500,     msx,      msx1_state, empty_init, "Fenner", "FPC-500 (MSX1, Italy)", 0)
COMP(1984, fspc800,    0,        0,     fspc800,    msxuk,    msx1_state, empty_init, "Fenner", "SPC-800 (MSX1, Italy)", 0)
COMP(1983, fmx,        0,        0,     fmx,        msxjp,    msx1_state, empty_init, "Fujitsu", "FM-X (MSX1, Japan)", 0)
COMP(1984, gsfc80u,    0,        0,     gsfc80u,    msxkr,    msx1_state, empty_init, "Goldstar", "FC-80U (MSX1, Korea)", 0)
COMP(1984, gsfc200,    0,        0,     gsfc200,    msx,      msx1_state, empty_init, "Goldstar", "FC-200 (MSX1, Europe)", 0)
COMP(1985, gfc1080,    0,        0,     gfc1080,    msxkr,    msx1_state, empty_init, "Goldstar", "GFC-1080 (MSX1, Korea)", 0)
COMP(1985, gfc1080a,   0,        0,     gfc1080a,   msxkr,    msx1_state, empty_init, "Goldstar", "GFC-1080A (MSX1, Korea)", 0)
COMP(1985, expert10,   expert13, 0,     expert10,   expert10, msx1_state, empty_init, "Gradiente", "Expert XP-800 (1.0) (MSX1, Brazil)", 0)
COMP(198?, expert11,   expert13, 0,     expert11,   expert11, msx1_state, empty_init, "Gradiente", "Expert XP-800 (1.1) / Expert GPC-1 (MSX1, Brazil)", 0)
COMP(198?, expert13,   0,        0,     expert13,   expert11, msx1_state, empty_init, "Gradiente", "Expert 1.3 (MSX1, Brazil)", 0)
COMP(1989, expertdp,   0,        0,     expertdp,   expert11, msx1_state, empty_init, "Gradiente", "Expert DDPlus (MSX1, Brazil)", 0)
COMP(1989, expertpl,   0,        0,     expertpl,   expert11, msx1_state, empty_init, "Gradiente", "Expert Plus (MSX1, Brazil)", 0)
COMP(1983, mbh1,       0,        0,     mbh1,       msxjp,    msx1_state, empty_init, "Hitachi", "MB-H1 (MSX1, Japan)", 0)
COMP(1984, mbh1e,      mbh1,     0,     mbh1e,      msxjp,    msx1_state, empty_init, "Hitachi", "MB-H1E (MSX1, Japan)", 0)
COMP(1984, mbh2,       0,        0,     mbh2,       msxjp,    msx1_state, empty_init, "Hitachi", "MB-H2 (MSX1, Japan)", 0)
COMP(1988, mbh25,      0,        0,     mbh25,      msxjp,    msx1_state, empty_init, "Hitachi", "MB-H25 (MSX1, Japan)", 0)
COMP(1986, mbh50,      0,        0,     mbh50,      msxjp,    msx1_state, empty_init, "Hitachi", "MB-H50 (MSX1, Japan)", 0)
COMP(1985, jvchc7gb,   0,        0,     jvchc7gb,   msxuk,    msx1_state, empty_init, "JVC", "HC-7E / HC-7GB (MSX1, Europe)", 0)
COMP(1983, ml8000,     0,        0,     ml8000,     msxjp,    msx1_state, empty_init, "Mitsubishi", "ML-8000 (MSX1, Japan)", 0)
COMP(1984, mlf48,      0,        0,     mlf48,      msxuk,    msx1_state, empty_init, "Mitsubishi", "ML-F48 (MSX1, UK)", 0)
COMP(1984, mlf80,      0,        0,     mlf80,      msxuk,    msx1_state, empty_init, "Mitsubishi", "ML-F80 (MSX1, UK)", 0)
COMP(1984, mlf110,     0,        0,     mlf110,     msxjp,    msx1_state, empty_init, "Mitsubishi", "ML-F110 (MSX1, Japan)", 0)
COMP(1984, mlf120,     0,        0,     mlf120,     msxjp,    msx1_state, empty_init, "Mitsubishi", "ML-F120 (MSX1, Japan)", 0)
COMP(1986, mlfx1,      0,        0,     mlfx1,      mlfx1,    msx1_state, empty_init, "Mitsubishi", "ML-FX1 (MSX1, Spain)", 0)
COMP(1985, cf1200,     0,        0,     cf1200,     msxjp,    msx1_state, empty_init, "National", "CF-1200 (MSX1, Japan)", 0)
COMP(1983, cf2000,     0,        0,     cf2000,     msxjp,    msx1_state, empty_init, "National", "CF-2000 (MSX1, Japan)", 0)
COMP(1984, cf2700,     0,        0,     cf2700,     msxjp,    msx1_state, empty_init, "National", "CF-2700 (MSX1, Japan)", 0)
COMP(1984, cf3000,     0,        0,     cf3000,     cf3000,   msx1_state, empty_init, "National", "CF-3000 (MSX1, Japan)", 0)
COMP(1985, cf3300,     0,        0,     cf3300,     cf3000,   msx1_state, empty_init, "National", "CF-3300 (MSX1, Japan)", 0)
COMP(1985, fs1300,     0,        0,     fs1300,     msxjp,    msx1_state, empty_init, "National", "FS-1300 (MSX1, Japan)", 0)
COMP(1985, fs4000,     0,        0,     fs4000,     fs4000,   msx1_state, empty_init, "National", "FS-4000 (MSX1, Japan)", 0)
COMP(1985, fs4000a,    fs4000,   0,     fs4000a,    fs4000,   msx1_state, empty_init, "National", "FS-4000 (alt) (MSX1, Japan)", 0)
COMP(1985, phc2,       0,        0,     phc2,       msxfr,    msx1_state, empty_init, "Olympia", "PHC-2 (MSX1, France)" , 0)
COMP(1984, phc28,      0,        0,     phc28,      msxfr,    msx1_state, empty_init, "Olympia", "PHC-28 (MSX1, France)", 0)
COMP(1985, cf2700g,    cf2700uk, 0,     cf2700g,    msxde,    msx1_state, empty_init, "Panasonic", "CF-2700 (MSX1, Germany)", 0)
COMP(1985, cf2700uk,   0,        0,     cf2700uk,   msxuk,    msx1_state, empty_init, "Panasonic", "CF-2700 (MSX1, UK)", 0)
COMP(1989, nms801,     0,        0,     nms801,     msx,      msx1_state, empty_init, "Philips", "NMS-801 (MSX1, Italy)", 0)
COMP(1984, vg8000,     vg8010,   0,     vg8000,     vg8010,   msx1_state, empty_init, "Philips", "VG-8000 (MSX1, Europe)", 0)
COMP(1985, vg8010,     0,        0,     vg8010,     vg8010,   msx1_state, empty_init, "Philips", "VG-8010 / VG-8010/00 (MSX1, Europe)", 0)
COMP(1985, vg8010f,    vg8010,   0,     vg8010f,    vg8010f,  msx1_state, empty_init, "Philips", "VG-8010F / VG-8010/19 (MSX1, French)" , 0)
COMP(1985, vg802000,   vg802020, 0,     vg802000,   msx,      msx1_state, empty_init, "Philips", "VG-8020/00 (MSX1, Europe)", 0)
COMP(1985, vg8020f,    vg802020, 0,     vg8020f,    msxfr,    msx1_state, empty_init, "Philips", "VG-8020/19 / VG-8020F (MSX1, French)", 0)
COMP(1985, vg802020,   0,        0,     vg802020,   msx,      msx1_state, empty_init, "Philips", "VG-8020/20 (MSX1, Europe)", 0)
COMP(1984, piopx7,     0,        0,     piopx7,     msxjp,    msx1_state, empty_init, "Pioneer", "PX-07 Palcom (MSX1, Japan)", 0)
COMP(1985, piopx7uk,   piopx7,   0,     piopx7uk,   msxuk,    msx1_state, empty_init, "Pioneer", "PX-07UK Palcom (MSX1, UK)", 0)
COMP(1986, piopxv60,   piopx7,   0,     piopxv60,   msxjp,    msx1_state, empty_init, "Pioneer", "PX-V60 (MSX1, Japan)", 0)
COMP(1986, ax150,      0,        0,     ax150,      msx,      msx1_state, empty_init, "Sakhr", "AX-150 (MSX1, Arabic)", 0)
COMP(1986, ax170,      0,        0,     ax170,      msx,      msx1_state, empty_init, "Sakhr", "AX-170 (MSX1, Arabic)", 0)
COMP(1986, ax230,      0,        0,     ax230,      msx,      msx1_state, empty_init, "Sakhr", "AX-230 (MSX1, Arabic)", MACHINE_IMPERFECT_GRAPHICS)
COMP(1984, spc800,     0,        0,     spc800,     msxkr,    msx1_state, empty_init, "Samsung", "SPC-800 (MSX1, Korea)", 0)
COMP(1983, mpc10,      0,        0,     mpc10,      msxjp,    msx1_state, empty_init, "Sanyo", "MPC-10 / Wavy10 (MSX1, Japan)", 0)
COMP(1985, mpc64,      0,        0,     mpc64,      msxde,    msx1_state, empty_init, "Sanyo", "MPC-64 (MSX1, Germany)", 0)
COMP(1985, mpc100,     0,        0,     mpc100,     msxuk,    msx1_state, empty_init, "Sanyo", "MPC-100 (MSX1, UK)", 0)
COMP(1985, mpc200,     0,        0,     mpc200,     msxuk,    msx1_state, empty_init, "Sanyo", "MPC-200 (MSX1, UK)", 0) // Is this the same as MPC-200SP, is it even real ?
COMP(1985, mpc200sp,   mpc200,   0,     mpc200sp,   msxsp,    msx1_state, empty_init, "Sanyo", "MPC-200SP (MSX1, Spain)", 0)
COMP(1985, phc28l,     0,        0,     phc28l,     msxfr,    msx1_state, empty_init, "Sanyo", "PHC-28L (MSX1, France)", 0)
COMP(1984, phc28s,     0,        0,     phc28s,     msxuk,    msx1_state, empty_init, "Sanyo", "PHC-28S (MSX1, France)", 0)
COMP(1985, hb8000,     0,        0,     hb8000,     hotbit,   msx1_state, empty_init, "Sharp / Epcom", "HB-8000 Hotbit (MSX1, Brazil)", 0)
COMP(198?, hotbi13b,   hotbi13p, 0,     hotbi13b,   hotbit,   msx1_state, empty_init, "Sharp / Epcom", "HB-8000 Hotbit 1.3b (MSX1, Brazil)", 0)
COMP(198?, hotbi13p,   0,        0,     hotbi13p,   hotbit,   msx1_state, empty_init, "Sharp / Epcom", "HB-8000 Hotbit 1.3p (MSX1, Brazil)", 0)
COMP(1985, hb10,       hb10p,    0,     hb10,       msxjp,    msx1_state, empty_init, "Sony", "HB-10 (MSX1, Japan)", 0)
COMP(1986, hb10p,      0,        0,     hb10p,      msxuk,    msx1_state, empty_init, "Sony", "HB-10P (MSX1, Netherlands)", 0)
COMP(1984, hb101,      hb101p,   0,     hb101,      msxjp,    msx1_state, empty_init, "Sony", "HB-101 (MSX1, Japan)", 0)
COMP(1984, hb101p,     0,        0,     hb101p,     msxuk,    msx1_state, empty_init, "Sony", "HB-101P (MSX1, Europe)", 0)
COMP(1986, hb20p,      0,        0,     hb20p,      msxsp,    msx1_state, empty_init, "Sony", "HB-20P (MSX1, Spain)", 0)
COMP(1985, hb201,      hb201p,   0,     hb201,      msxjp,    msx1_state, empty_init, "Sony", "HB-201 (MSX1, Japan)", 0)
COMP(1985, hb201p,     0,        0,     hb201p,     msxuk,    msx1_state, empty_init, "Sony", "HB-201P (MSX1, Europe)", 0)
COMP(1984, hb501p,     0,        0,     hb501p,     msxuk,    msx1_state, empty_init, "Sony", "HB-501P (MSX1, Europe)", 0)
COMP(1983, hb55,       hb55p,    0,     hb55,       msxjp,    msx1_state, empty_init, "Sony", "HB-55 (MSX1, Japan)", 0)
COMP(1984, hb55d,      hb55p,    0,     hb55d,      msxde,    msx1_state, empty_init, "Sony", "HB-55D (MSX1, Germany)", 0)
COMP(1984, hb55p,      0,        0,     hb55p,      msxuk,    msx1_state, empty_init, "Sony", "HB-55P (MSX1, Europe)", 0)
COMP(1984, hb701fd,    0,        0,     hb701fd,    msxjp,    msx1_state, empty_init, "Sony", "HB-701FD (MSX1, Japan)", 0)
COMP(1984, hb75,       hb75p,    0,     hb75,       msxjp,    msx1_state, empty_init, "Sony", "HB-75 (MSX1, Japan)", 0)
COMP(1984, hb75d,      hb75p,    0,     hb75d,      msxde,    msx1_state, empty_init, "Sony", "HB-75D (MSX1, Germany)", 0)
COMP(1984, hb75p,      0,        0,     hb75p,      msxuk,    msx1_state, empty_init, "Sony", "HB-75P (MSX1, Europe)", 0)
COMP(1984, svi728,     0,        0,     svi728,     svi728,   msx1_state, empty_init, "Spectravideo", "SVI-728 (MSX1, International)", 0)
COMP(1984, svi728es,   svi728,   0,     svi728,     svi728sp, msx1_state, empty_init, "Spectravideo", "SVI-728 (MSX1, Spanish)", 0)
COMP(1986, tadpc200,   dpc200,   0,     tadpc200,   msxsp,    msx1_state, empty_init, "Talent", "DPC-200 (MSX1, Argentina, Spanish keyboard)", 0)
COMP(1986, tadpc200b,  dpc200,   0,     tadpc200,   msx,      msx1_state, empty_init, "Talent", "DPC-200 (MSX1, Argentina, international keyboard)", 0)
COMP(1984, hx10,       0,        0,     hx10,       msx,      msx1_state, empty_init, "Toshiba", "HX-10AA (MSX1, Europe)", 0)
COMP(1983, hx10d,      hx10,     0,     hx10d,      msxjp,    msx1_state, empty_init, "Toshiba", "HX-10D (MSX1, Japan)", 0)
COMP(1984, hx10dp,     hx10,     0,     hx10dp,     msxjp,    msx1_state, empty_init, "Toshiba", "HX-10DP (MSX1, Japan)", 0)
COMP(1984, hx10e,      hx10,     0,     hx10e,      msx,      msx1_state, empty_init, "Toshiba", "HX-10E (MSX1, Spain)", 0)
COMP(1984, hx10f,      hx10,     0,     hx10f,      msx,      msx1_state, empty_init, "Toshiba", "HX-10F (MSX1, France)", 0)
COMP(1983, hx10s,      hx10,     0,     hx10s,      msxjp,    msx1_state, empty_init, "Toshiba", "HX-10S (MSX1, Japan)", 0)
COMP(1984, hx10sa,     hx10,     0,     hx10sa,     msxjp,    msx1_state, empty_init, "Toshiba", "HX-10SA (MSX1, Japan)", 0)
COMP(1984, hx20,       0,        0,     hx20,       msxjp,    msx1_state, empty_init, "Toshiba", "HX-20 (MSX1, Japan)", 0)
COMP(1985, hx20e,      hx20,     0,     hx20e,      msx,      msx1_state, empty_init, "Toshiba", "HX-20E (MSX1, Spain)", 0)
COMP(1985, hx20i,      hx20,     0,     hx20i,      msx,      msx1_state, empty_init, "Toshiba", "HX-20I (MSX1, Italy)", 0)
COMP(1984, hx21,       0,        0,     hx21,       msxjp,    msx1_state, empty_init, "Toshiba", "HX-21 (MSX1, Japan)", 0)
COMP(1985, hx21f,      hx21,     0,     hx21f,      msx,      msx1_state, empty_init, "Toshiba", "HX-21F (MSX1, France)", 0)
COMP(1984, hx22,       0,        0,     hx22,       msxjp,    msx1_state, empty_init, "Toshiba", "HX-22 (MSX1, Japan)", 0)
COMP(1985, hx22i,      hx22,     0,     hx22i,      msx,      msx1_state, empty_init, "Toshiba", "HX-22I (MSX1, Italy)", 0)
COMP(1985, hx32,       0,        0,     hx32,       msxjp,    msx1_state, empty_init, "Toshiba", "HX-32 (MSX1, Japan)", 0)
COMP(1985, hx51i,      0,        0,     hx51i,      msx,      msx1_state, empty_init, "Toshiba", "HX-51I (MSX1, Italy, Spain)", 0)
COMP(1983, hc5,        hc7,      0,     hc5,        msxjp,    msx1_state, empty_init, "Victor", "HC-5 (MSX1, Japan)", 0)
COMP(1984, hc6,        hc7,      0,     hc6,        msxjp,    msx1_state, empty_init, "Victor", "HC-6 (MSX1, Japan)", 0)
COMP(1985, hc7,        0,        0,     hc7,        msxjp,    msx1_state, empty_init, "Victor", "HC-7 (MSX1, Japan)", 0)
COMP(1984, cx5f1,      cx5f,     0,     cx5f1,      msxjp,    msx1_state, empty_init, "Yamaha", "CX5F w/SFG01 (MSX1, Japan)", 0)
COMP(1984, cx5f,       0,        0,     cx5f,       msxjp,    msx1_state, empty_init, "Yamaha", "CX5F w/SFG05 (MSX1, Japan)", 0)
COMP(1984, cx5mu,      0,        0,     cx5mu,      msx,      msx1_state, empty_init, "Yamaha", "CX5MU (MSX1, USA)", 0)
COMP(1985, sx100,      0,        0,     sx100,      msxjp,    msx1_state, empty_init, "Yamaha", "SX-100 (MSX1, Japan)", 0)
COMP(1984, yis303,     0,        0,     yis303,     msxjp,    msx1_state, empty_init, "Yamaha", "YIS303 (MSX1, Japan)", 0)
COMP(1984, yis503,     0,        0,     yis503,     msxjp,    msx1_state, empty_init, "Yamaha", "YIS503 (MSX1, Japan)", 0)
COMP(1984, yis503f,    yis503,   0,     yis503f,    msxuk,    msx1_state, empty_init, "Yamaha", "YIS503F (MSX1, French)", 0)
COMP(1984, yc64,       0,        0,     yc64,       msxuk,    msx1_state, empty_init, "Yashica", "YC-64 (MSX1, Europe)", 0)
COMP(1985, mx64,       0,        0,     mx64,       msxfr,    msx1_state, empty_init, "Yeno", "MX64 (MSX1, France)", 0)
