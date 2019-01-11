// license:BSD-3-Clause
// copyright-holders:
/**********************************************************************************************************************

2017-11-05 Skeleton PLACEHOLDER ONLY.

Sony PlayStation 2.

Related to: Konami Bemani/Python, Namco Systems 147, 246, 256 (info from system16.com). See namecops2.cpp for details.

Technical specs: https://en.wikipedia.org/wiki/PlayStation_2_technical_specifications

If development proceeds, it may be prudent to make this and namcops2.cpp derived classes of a common class.

**********************************************************************************************************************/
// the skeleton memory map and config was copied from namcops2.cpp
/*********************************************************************************************************************
Contents of INF files:

SCPH-75008_BIOS_V14_RUS_220.INF
[EE]
CPUrev=0x2E42
FPUrev=0x2E40
BOARDid=0xFFFF
VU0MEMsize=4KB;4KB (*)
VU1MEMsize=16KB;16KB (*)
GSrev=0x551C
GSMEMsize=4MB (*)
ICache=16KB
Dcache=8KB
MEMsize=32MB

[IOP]
CPUrev=0x0030
ICache=4KB (*)
Dcache=1KB (*)
MEMsize=2MB
BOARDinf=0xFF
SPU2rev=0x08
SPU2MEMsize=2M (*)
CDVDmecha=0x05060600
CDVDmeka=0x00
DEV9rev=0x0031
SPDrev=0x13 (unknown)
SPDcaps=0x03 (eth+ata)
SPDmac=00:15:C1:6B:6D:AB
USBhcrev=0x00000010
iLinkports=2
iLinkspeed=400
iLinkvendor=0x------
iLinkdevice=0x------
iLinkSGUID=0x--------

SCPH-77004_BIOS_V15_EUR_220.INF
[EE]
CPUrev=0x2E42
FPUrev=0x2E40
BOARDid=0xFFFF
VU0MEMsize=4KB;4KB (*)
VU1MEMsize=16KB;16KB (*)
GSrev=0x551D
GSMEMsize=4MB (*)
ICache=16KB
Dcache=8KB
MEMsize=32MB

[IOP]
CPUrev=0x0030
ICache=4KB (*)
Dcache=1KB (*)
MEMsize=2MB
BOARDinf=0xFF
SPU2rev=0x08
SPU2MEMsize=2M (*)
CDVDmecha=0x02060A00
CDVDmeka=0x00
DEV9rev=0x0031
SPDrev=0x13 (unknown)
SPDcaps=0x03 (eth+ata)
SPDmac=00:15:C1:BB:55:F3
USBhcrev=0x00000010
iLinkports=2
iLinkspeed=400
iLinkvendor=0x------
iLinkdevice=0x------
iLinkSGUID=0x--------

SCPH-77008_BIOS_V15_RUS_220.INF
[EE]
CPUrev=0x2E42
FPUrev=0x2E40
BOARDid=0xFFFF
VU0MEMsize=4KB;4KB (*)
VU1MEMsize=16KB;16KB (*)
GSrev=0x551C
GSMEMsize=4MB (*)
ICache=16KB
Dcache=8KB
MEMsize=32MB

[IOP]
CPUrev=0x0030
ICache=4KB (*)
Dcache=1KB (*)
MEMsize=2MB
BOARDinf=0xFF
SPU2rev=0x08
SPU2MEMsize=2M (*)
CDVDmecha=0x05060A00
CDVDmeka=0x00
DEV9rev=0x0031
SPDrev=0x13 (unknown)
SPDcaps=0x03 (eth+ata)
SPDmac=00:1D:0D:4D:EF:AB
USBhcrev=0x00000010
iLinkports=2
iLinkspeed=400
iLinkvendor=0x------
iLinkdevice=0x------
iLinkSGUID=0x--------

SCPH-90001_BIOS_V18_USA_230.INF
[EE]
CPUrev=0x2E43
FPUrev=0x2E40
BOARDid=0xFFFF
VU0MEMsize=4KB;4KB (*)
VU1MEMsize=16KB;16KB (*)
GSrev=0x551F
GSMEMsize=4MB (*)
ICache=16KB
Dcache=8KB
MEMsize=32MB

[IOP]
CPUrev=0x0030
ICache=4KB (*)
Dcache=1KB (*)
MEMsize=2MB
BOARDinf=0xFF
SPU2rev=0x08
SPU2MEMsize=2M (*)
CDVDmecha=0x01060C00
CDVDmeka=0x00
DEV9rev=0x0031
SPDrev=0x13 (unknown)
SPDcaps=0x03 (eth+ata)
SPDmac=00:1F:A7:BF:4D:AB
USBhcrev=0x00000010
iLinkports=2
iLinkspeed=400
iLinkvendor=0x------
iLinkdevice=0x------
iLinkSGUID=0x--------

(*) values are not detected
************************************************************************************************************************/
#include "emu.h"
#include "cpu/mips/mips3.h"
#include "cpu/mips/r3000.h"
#include "screen.h"


class ps2sony_state : public driver_device
{
public:
	ps2sony_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void ps2sony(machine_config &config);
	void mem_map(address_map &map);
private:
	virtual void video_start() override;
	required_device<cpu_device> m_maincpu;
};


void ps2sony_state::video_start()
{
}

uint32_t ps2sony_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void ps2sony_state::mem_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).ram(); // 32 MB RAM
	map(0x1fc00000, 0x1fdfffff).rom().region("bios", 0);
}

static INPUT_PORTS_START( ps2sony )
INPUT_PORTS_END

MACHINE_CONFIG_START(ps2sony_state::ps2sony)
	MCFG_DEVICE_ADD("maincpu", R5000LE, 294'912'000) // actually R5900
	MCFG_MIPS3_ICACHE_SIZE(16384)
	MCFG_MIPS3_DCACHE_SIZE(16384)
	MCFG_DEVICE_PROGRAM_MAP(mem_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DRIVER(ps2sony_state, screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)

	MCFG_PALETTE_ADD("palette", 65536)
MACHINE_CONFIG_END


ROM_START( ps2 )
	// These came from the redump.org "Sony - PlayStation 2 - BIOS Datfile" (version 2017-10-26)
	ROM_REGION(0x400000, "bios", 0)
	ROM_DEFAULT_BIOS( "scph90002_e" )
	ROM_SYSTEM_BIOS( 0, "scph10000_t", "SCPH-10000 (Version 5.0 01/17/00 T)" )
	ROMX_LOAD( "ps2-0100j-20000117.bin", 0x0000, 0x400000, CRC(b7ef81a9) SHA1(aea061e6e263fdcc1c4fdbd68553ef78dae74263), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "dtlh10000_t_old", "DTL-H10000 (Version 5.0 02/17/00 T)" )
	ROMX_LOAD( "ps2-0101jd-20000217.bin", 0x0000, 0x400000, CRC(4f8b4205) SHA1(16f4a284d0e760ee13a2aff2f7dda928255e3080), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "scph1000015000_t", "SCPH-10000/SCPH-15000 (Version 5.0 02/17/00 T)" )
	ROMX_LOAD( "ps2-0101j-20000217.bin", 0x0000, 0x400000, CRC(211dfb6a) SHA1(916e02431bcd73140504da3355c9598143b77e11), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "dtlh10000_t", "DTL-H10000 (Version 5.0 02/24/00 T)" )
	ROMX_LOAD( "ps2-0101xd-20000224.bin", 0x0000, 0x400000, CRC(2fef9faf) SHA1(4440b246bfde7bb31002c584a76c6ef384908e84), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "dtlh30001_a", "DTL-H30001 (Version 5.0 07/27/00 A)" )
	ROMX_LOAD( "ps2-0110ad-20000727.bin", 0x0000, 0x400000, CRC(795578c1) SHA1(339c646cf0699268552df5b05f18f0a03a9f55ff), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "scph30001_a_old", "SCPH-30001 (Version 5.0 07/27/00 A)" )
	ROMX_LOAD( "ps2-0110a-20000727.bin", 0x0000, 0x400000, CRC(9678ad6a) SHA1(20f6ce6693cf97e9494f8f0227f2b7988ffaf961), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "scph30001_a", "SCPH-30001 (Version 5.0 09/02/00 A)" )
	ROMX_LOAD( "ps2-0120a-20000902.bin", 0x0000, 0x400000, CRC(1ae71e5d) SHA1(dbc2318a1029347b5af3a0c74b0bdf88d19efee6), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 7, "dtlh30002_e", "DTL-H30002 (Version 5.0 09/02/00 E)" )
	ROMX_LOAD( "ps2-0120ed-20000902.bin", 0x0000, 0x400000, CRC(25495aa7) SHA1(3bb1eecd618ab5c973c7bc53671a4475a02e1d5b), ROM_BIOS(7) )
	ROM_SYSTEM_BIOS( 8, "scph30002_e_old", "SCPH-30002/SCPH-30003/SCPH-30004 (Version 5.0 09/02/00 E)" )
	ROMX_LOAD( "ps2-0120e-20000902.bin", 0x0000, 0x400000, CRC(7b08c33b) SHA1(274c05fec654913a3f698d4b0d592085866a2cbd), ROM_BIOS(8) )
	ROM_SYSTEM_BIOS( 9, "scph18000_3_j", "SCPH-18000 (GH-003) (Version 5.0 10/27/00 J)" )
	ROMX_LOAD( "ps2-0120j-20001027-185015.bin", 0x0000, 0x400000, CRC(9b096622) SHA1(e481079eca752225555f0c26d14c9d0f94d9a8e9), ROM_BIOS(9) )
	ROM_SYSTEM_BIOS( 10, "scph18000_8_j", "SCPH-18000 (GH-008) (Version 5.0 10/27/00 J)" )
	ROMX_LOAD( "ps2-0120j-20001027-191435.bin", 0x0000, 0x400000, CRC(c1ccf3f6) SHA1(a9f5d8ed56cfff18add1b599010493461fa02448), ROM_BIOS(10) )
	ROM_SYSTEM_BIOS( 11, "dtlh30101_a", "DTL-H30101 (Version 5.0 12/28/00 A)" )
	ROMX_LOAD( "ps2-0150ad-20001228.bin", 0x0000, 0x400000, CRC(0380c2ce) SHA1(7284b9d16df9935afc384318e024c87ef0574fe5), ROM_BIOS(11) )
	ROM_SYSTEM_BIOS( 12, "scph30_a", "SCPH-30001/SCPH-35001 (Version 5.0 12/28/00 A)" )
	ROMX_LOAD( "ps2-0150a-20001228.bin", 0x0000, 0x400000, CRC(bce74746) SHA1(5af5b5077d84a9c037ebe12bfab8a38b31d8a543), ROM_BIOS(12) )
	ROM_SYSTEM_BIOS( 13, "scph30002_e", "SCPH-30002/SCPH-30003/SCPH-30004/SCHP-35002/SCPH-35003/SCPH-35004 (Version 5.0 12/28/00 E)" )
	ROMX_LOAD( "ps2-0150e-20001228.bin", 0x0000, 0x400000, CRC(1559fd43) SHA1(e22ef231faf3661edd92f2ee449a71297c82a092), ROM_BIOS(13) )
	ROM_SYSTEM_BIOS( 14, "dtlh30000_j", "DTL-H30000 (Version 5.0 01/18/01 J)" )
	ROMX_LOAD( "ps2-0150jd-20010118.bin", 0x0000, 0x400000, CRC(0b3ec2bc) SHA1(334e029fc7fd50222a399c50384ff42732652259), ROM_BIOS(14) )
	ROM_SYSTEM_BIOS( 15, "scph30000_j_old", "SCPH-30000 (Version 5.0 01/18/01 J)" )
	ROMX_LOAD( "ps2-0150j-20010118.bin", 0x0000, 0x400000, CRC(4fc3b495) SHA1(d6f365a0f07cd04ed28108e6ec5076e2f81e5f72), ROM_BIOS(15) )
	ROM_SYSTEM_BIOS( 16, "scph30001r_a_b", "SCPH-30001R (Version 5.0 04/27/01 A)" )
	ROMX_LOAD( "ps2-0160a-20010427.bin", 0x0000, 0x400000, CRC(4008ac18) SHA1(7331a40b4b4feb1b3f0f77b013b6d38483577baa), ROM_BIOS(16) )
	ROM_SYSTEM_BIOS( 17, "scph30000_j", "SCPH-30000 (Version 5.0 04/27/01 J)" )
	ROMX_LOAD( "ps2-0160j-20010427.bin", 0x0000, 0x400000, CRC(c268ef47) SHA1(e525a0c900e37acf0ae5a655d82a0abcb07c6f1f), ROM_BIOS(17) )
	ROM_SYSTEM_BIOS( 18, "scph30001r_a_a", "SCPH-30001R (Version 5.0 07/04/01 A)" )
	ROMX_LOAD( "ps2-0160a-20010704.bin", 0x0000, 0x400000, CRC(c506c693) SHA1(ce92e8e8c88665f2f645a9522e337823d47a914a), ROM_BIOS(18) )
	ROM_SYSTEM_BIOS( 19, "scph30002r_e_old", "SCPH-30002R/SCPH-30003R/SCPH-30004R (Version 5.0 07/04/01 E)" )
	ROMX_LOAD( "ps2-0160e-20010704.bin", 0x0000, 0x400000, CRC(f1ac735f) SHA1(3cbd048e437c785b5a05a0feced00117a8a42545), ROM_BIOS(19) )
	ROM_SYSTEM_BIOS( 20, "scph30001r_a", "SCPH-30001R (Version 5.0 10/04/01 A)" )
	ROMX_LOAD( "ps2-0160a-20011004.bin", 0x0000, 0x400000, CRC(a01ec625) SHA1(d257bce6ecaf3bafb704c75a1b4741b910bd2d49), ROM_BIOS(20) )
	ROM_SYSTEM_BIOS( 21, "scph30002r_e", "SCPH-30002R/SCPH-30003R/SCPH-30004R (Version 5.0 10/04/01 E)" )
	ROMX_LOAD( "ps2-0160e-20011004.bin", 0x0000, 0x400000, CRC(82aa5055) SHA1(ee34c3a87c53c75ca2a37d77b0042ca24d07831f), ROM_BIOS(21) )
	ROM_SYSTEM_BIOS( 22, "scph30006r_j", "SCPH-30006R (Version 5.0 07/30/01 J)" )
	ROMX_LOAD( "ps2-0160h-20010730.bin", 0x0000, 0x400000, CRC(75f83c67) SHA1(ba15dcf7aac13864c08222037e9321d7468c87d1), ROM_BIOS(22) )
	ROM_SYSTEM_BIOS( 23, "scph39001_a", "SCPH-39001 (Version 5.0 02/07/02 A)" )
	ROMX_LOAD( "ps2-0160a-20020207.bin", 0x0000, 0x400000, CRC(a19e0bf5) SHA1(f9a5d629a036b99128f7cb530c6e3ca016e9c8b7), ROM_BIOS(23) )
	ROM_SYSTEM_BIOS( 24, "scph39002_e", "SCPH-39002/SCPH-39003/SCPH-39004 (Version 5.0 03/19/02 E)" )
	ROMX_LOAD( "ps2-0160e-20020319.bin", 0x0000, 0x400000, CRC(2fe21e4d) SHA1(bff2902bd0ce9729a060581132541e9fd1a9fab6), ROM_BIOS(24) )
	ROM_SYSTEM_BIOS( 25, "scph39006_j", "SCPH-39006 (Version 5.0 04/26/02 J)" )
	ROMX_LOAD( "ps2-0160h-20020426.bin", 0x0000, 0x400000, CRC(3355623e) SHA1(e3a74125c426bcacabca00b513fab928665c8846), ROM_BIOS(25) )
	ROM_SYSTEM_BIOS( 26, "scph50000_j", "SCPH-50000 (Version 5.0 02/06/03 J)" )
	ROMX_LOAD( "ps2-0170j-20030206.bin", 0x0000, 0x400000, CRC(9457f64e) SHA1(d812ac65c357d392396ca9edee812dc41bed8bde), ROM_BIOS(26) )
	ROM_SYSTEM_BIOS( 27, "dtlh50002_e", "DTL-H50002 (Version 5.0 02/27/03 E)" )
	ROMX_LOAD( "ps2-0170ed-20030227.bin", 0x0000, 0x400000, CRC(970a9c56) SHA1(e220bb282378c1f48ea1b585b3675e51a6dca572), ROM_BIOS(27) )
	ROM_SYSTEM_BIOS( 28, "scph50002_e_old", "SCPH-50002/SCPH-50003/SCPH-50004 (Version 5.0 02/27/03 E)" )
	ROMX_LOAD( "ps2-0170e-20030227.bin", 0x0000, 0x400000, CRC(51b5fb8b) SHA1(ad15bd7eabd5bd81ba011516a5be44947d6641aa), ROM_BIOS(28) )
	ROM_SYSTEM_BIOS( 29, "dtlh50001_a", "DTL-H50001 (Version 5.0 03/25/03 A)" )
	ROMX_LOAD( "ps2-0170ad-20030325.bin", 0x0000, 0x400000, CRC(0e1ece79) SHA1(c5bc6e893b4c43d528142e56c96073024de64157), ROM_BIOS(29) )
	ROM_SYSTEM_BIOS( 30, "scph50001_a_old", "SCPH-50001 (Version 5.0 03/25/03 A)" )
	ROMX_LOAD( "ps2-0170a-20030325.bin", 0x0000, 0x400000, CRC(9a99e3f4) SHA1(d269d1ed513227f3ef7133c76cf1b3a64f97b15d), ROM_BIOS(30) )
	ROM_SYSTEM_BIOS( 31, "dtl50009_j", "DTL-50009 (Version 5.0 02/24/03 J)" )
	ROMX_LOAD( "ps2-0180cd-20030224.bin", 0x0000, 0x400000, CRC(8c1a04cf) SHA1(2de87767008fc4a303af64a46251156e965d9065), ROM_BIOS(31) )
	ROM_SYSTEM_BIOS( 32, "desr5000_j", "DESR-5000/DESR-5100/DESR-7000/DESR-7100 (Version 5.0 10/28/03 J)" )
	ROMX_LOAD( "ps2-0180j-20031028.bin", 0x0000, 0x400000, CRC(585fd27c) SHA1(aa4a35c14ee342cf7a03b1dde294ca10e64889e1), ROM_BIOS(32) )
	ROM_SYSTEM_BIOS( 33, "scph50001_a", "SCPH-50001 (Version 5.0 06/23/03 A)" )
	ROMX_LOAD( "ps2-0190a-20030623.bin", 0x0000, 0x400000, CRC(b3e87709) SHA1(c74d92a2952a2912b6698cbcf7742adac8f784d3), ROM_BIOS(33) )
	ROM_SYSTEM_BIOS( 34, "scph50002_e", "SCPH-50002/SCPH-50003/SCPH-50004 (Version 5.0 06/23/03 E)" )
	ROMX_LOAD( "ps2-0190e-20030623.bin", 0x0000, 0x400000, CRC(1752a52e) SHA1(18b9ba833c469c4683676cc20da5124080d980bb), ROM_BIOS(34) )
	ROM_SYSTEM_BIOS( 35, "scph50006_j", "SCPH-50006 (Version 5.0 06/23/03 J)" )
	ROMX_LOAD( "ps2-0190h-20030623.bin", 0x0000, 0x400000, CRC(36b0580c) SHA1(3e18fefb5b5a59046baf25a3f13da1e3a40b92be), ROM_BIOS(35) )
	ROM_SYSTEM_BIOS( 36, "scph50008_e", "SCPH-50008 (Version 5.0 06/23/03 E)" )
	ROMX_LOAD( "ps2-0190r-20030623.bin", 0x0000, 0x400000, CRC(25f6212a) SHA1(34a81db03ab617fbfdd7f9b861692dd2ecd57b82), ROM_BIOS(36) )
	ROM_SYSTEM_BIOS( 37, "scph50009_j", "SCPH-50009 (Version 5.0 06/23/03 J)" )
	ROMX_LOAD( "ps2-0190c-20030623.bin", 0x0000, 0x400000, CRC(d2347ee7) SHA1(92d9eb4b11cef97bb69a275b2851b72f7b0023d6), ROM_BIOS(37) )
	ROM_SYSTEM_BIOS( 38, "scph70000_j", "SCPH-70000 (Version 5.0 06/14/04 J)" )
	ROMX_LOAD( "ps2-0200j-20040614.bin", 0x0000, 0x400000, CRC(2f314730) SHA1(224ab5704ab719edeb05ca1d835812252c97c1b3), ROM_BIOS(38) )
	ROM_SYSTEM_BIOS( 39, "scph70001_a", "SCPH-70001/SCPH-70011/SCPH-70012 (Version 5.0 06/14/04 A)" )
	ROMX_LOAD( "ps2-0200a-20040614.bin", 0x0000, 0x400000, CRC(7ebd68de) SHA1(7a62e5f48603582707e9898eb055ea3eaee50d4c), ROM_BIOS(39) )
	ROM_SYSTEM_BIOS( 40, "scph70002_e", "SCPH-70002/SCPH-70003/SCPH-70004/SCPH-70008 (Version 5.0 06/14/04 E)" )
	ROMX_LOAD( "ps2-0200e-20040614.bin", 0x0000, 0x400000, CRC(6f8e3c29) SHA1(434bc0b4eb4827da0773ec0795aadc5162569a07), ROM_BIOS(40) )
	ROM_SYSTEM_BIOS( 41, "scph70006_j", "SCPH-70006 (Version 5.0 06/14/04 J)" )
	ROMX_LOAD( "ps2-0200h-20040614.bin", 0x0000, 0x400000, CRC(b57201bf) SHA1(7f8e812cab7c7393c85eac6c42661e1fd0a642df), ROM_BIOS(41) )
	ROM_SYSTEM_BIOS( 42, "desr5500_j", "DESR-5500/DESR-7500 (Version 5.0 09/17/04 J)" )
	ROMX_LOAD( "ps2-0210j-20040917.bin", 0x0000, 0x400000, CRC(55710d11) SHA1(bbb1af3085e77599691ec430d147810157da934f), ROM_BIOS(42) )
	ROM_SYSTEM_BIOS( 43, "scph75001_a", "SCPH-75001 (Version 5.0 06/20/05 A)" )
	ROMX_LOAD( "ps2-0220a-20050620.bin", 0x0000, 0x400000, CRC(d305a97a) SHA1(48d0445dffd1e879c7ae752c5166ec3101921555), ROM_BIOS(43) )
	ROM_SYSTEM_BIOS( 44, "scph75002_e", "SCPH-75002/SCPH-75003/SCPH-75004/SCPH-75008 (Version 5.0 06/20/05 E)" )
	ROMX_LOAD( "ps2-0220e-20050620.bin", 0x0000, 0x400000, CRC(e2862e39) SHA1(929a85e974faf4b40d0a7785023b758402c43bd9), ROM_BIOS(44) )
	ROM_SYSTEM_BIOS( 45, "scph77001_a", "SCPH-77001 (Version 5.0 02/10/06 A)" )
	ROMX_LOAD( "ps2-0220a-20060210.bin", 0x0000, 0x400000, CRC(1279fce9) SHA1(92e488d5b2705e4cca83d4d1efbc421012faf83e), ROM_BIOS(45) )
	ROM_SYSTEM_BIOS( 46, "scph77002_e", "SCPH-77002/SCPH-77003/SCPH-77004/SCPH-77008 (Version 5.0 02/10/06 E)" )
	ROMX_LOAD( "ps2-0220e-20060210.bin", 0x0000, 0x400000, CRC(23fa7baa) SHA1(28ad756d0cfd1e7b2e2de3de5d9e14207ee89761), ROM_BIOS(46) )
	ROM_SYSTEM_BIOS( 47, "scph77006_j", "SCPH-77006 (Version 5.0 02/10/06 J)" )
	ROMX_LOAD( "ps2-0220h-20060210.bin", 0x0000, 0x400000, CRC(23001fff) SHA1(fce2a24e5e0400cc6d98c08f426405d19173813e), ROM_BIOS(47) )
	ROM_SYSTEM_BIOS( 48, "scph79001_a", "SCPH-79001/SCPH-90001 (Version 5.0 09/05/06 A)" )
	ROMX_LOAD( "ps2-0220a-20060905.bin", 0x0000, 0x400000, CRC(1c17eafc) SHA1(8361d615cc895962e0f0838489337574dbdc9173), ROM_BIOS(48) )
	ROM_SYSTEM_BIOS( 49, "scph79002_e", "SCPH-79002/SCPH-79003/SCPH-79004/SCPH-79008/SCPH-90002/SCPH-90003/SCPH-90004 (Version 5.0 09/05/06 E)" )
	ROMX_LOAD( "ps2-0220e-20060905.bin", 0x0000, 0x400000, CRC(2d946dbf) SHA1(da5aacead2fb55807d6d4e70b1f10f4fdcfd3281), ROM_BIOS(49) )
	ROM_SYSTEM_BIOS( 50, "scph79006_j", "SCPH-79006/SCPH-90006 (Version 5.0 09/05/06 J)" )
	ROMX_LOAD( "ps2-0220h-20060905.bin", 0x0000, 0x400000, CRC(2d6e09ea) SHA1(a5a2ee0dd9a86ca35b94e97ca92476a584f755bf), ROM_BIOS(50) )
	ROM_SYSTEM_BIOS( 51, "scph90001_a", "SCPH-90001/SCPH-90010 (Version 5.0 02/20/08 A)" )
	ROMX_LOAD( "ps2-0230a-20080220.bin", 0x0000, 0x400000, CRC(286897c2) SHA1(f9229fe159d0353b9f0632f3fdc66819c9030458), ROM_BIOS(51) )
	ROM_SYSTEM_BIOS( 52, "scph90002_e", "SCPH-90002/SCPH-90003/SCPH-90004 (Version 5.0 02/20/08 E)" )
	ROMX_LOAD( "ps2-0230e-20080220.bin", 0x0000, 0x400000, CRC(19eb1081) SHA1(9915b5ba56798f4027ac1bd8d10abe0c1c9c326a), ROM_BIOS(52) )
	ROM_SYSTEM_BIOS( 53, "kdl22px300_j", "KDL-22PX300 (Version 5.0 04/15/10 J)" )
	ROMX_LOAD( "ps2-0250j-20100415.bin", 0x0000, 0x400000, CRC(4e8c160c) SHA1(4b5ef16b67e3b523d28ed2406106cb80470a06d0), ROM_BIOS(53) )

	// These came from unknown sources and are of unknown quality
	ROM_SYSTEM_BIOS( 54, "unknown", "Unknown" )
	ROM_LOAD( "scph-30004r_bios_v6_eur_160.bin", 0x000000, 0x400000, CRC(9386a740) SHA1(8fa040852d4b8688f0c84bcfffc65eb208f2b432) )
	ROM_LOAD( "scph39004.bin", 0x000000, 0x400000, CRC(1f2a283c) SHA1(004cc467439f053d5a1fcf4d1b7c13338ce63403) )
	ROM_LOAD( "scph50003.bin", 0x000000, 0x400000, CRC(a5860b09) SHA1(003f9bdae45a04c5eb28689813e818566a8c4610) )
	ROM_LOAD( "scph_bios_v9_eur_190.bin", 0x000000, 0x400000, CRC(bfe41270) SHA1(dbd7f4d41d54e4f0bf3c4042bb42a42d5d4ef95e) )
	ROM_LOAD( "scph_bios_v10_chn_190.bin", 0x000000, 0x400000, CRC(40d6c676) SHA1(b7548a92bd2caa9d60cbc7f79573a5d510f88012) )
	ROM_LOAD( "scph_bios_v12_rus_200.bin", 0x000000, 0x400000, CRC(92aa71a2) SHA1(cce6fac0f7e682ad167e1e828b2d53192c3d5051) )
	ROM_LOAD( "scph_bios_v15_jap_220.bin", 0x000000, 0x400000, CRC(493c1e58) SHA1(d9a7537fa463fcdd3e270af14a93731736cafc4a) )
ROM_END

CONS( 2000, ps2, 0, 0, ps2sony, ps2sony, ps2sony_state, empty_init, "Sony", "PlayStation 2", MACHINE_IS_SKELETON )
