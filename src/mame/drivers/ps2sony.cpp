// license:BSD-3-Clause
// copyright-holders:
/**********************************************************************************************************************

2017-11-05 Skeleton PLACEHOLDER ONLY.

Sony Playstation 2.

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

static ADDRESS_MAP_START(mem_map, AS_PROGRAM, 32, ps2sony_state)
	AM_RANGE(0x00000000, 0x01ffffff) AM_RAM // 32 MB RAM
	AM_RANGE(0x1fc00000, 0x1fdfffff) AM_ROM AM_REGION("bios", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START( ps2sony )
INPUT_PORTS_END

static MACHINE_CONFIG_START( ps2sony )
	MCFG_CPU_ADD("maincpu", R5000LE, 294'912'000) // actually R5900
	MCFG_MIPS3_ICACHE_SIZE(16384)
	MCFG_MIPS3_DCACHE_SIZE(16384)
	MCFG_CPU_PROGRAM_MAP(mem_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DRIVER(ps2sony_state, screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)

	MCFG_PALETTE_ADD("palette", 65536)
MACHINE_CONFIG_END


ROM_START( ps2 )
	// hopefully an expert can sort the good from the bad here
	// A quick search revealed many different names with the same CRC. These have been pruned to one per CRC.
	ROM_REGION(0x400000, "bios", 0)
	ROM_LOAD( "scph-10000_bios_v1_jap_100.bin", 0x000000, 0x400000, CRC(b7ef81a9) SHA1(aea061e6e263fdcc1c4fdbd68553ef78dae74263) )
	ROM_LOAD( "scph-10000_bios_v1_jap_100.mec", 0x000000, 0x000004, CRC(054a6ec2) SHA1(3f8affbe07024ccd2f5b8374072d0928c583a862) )
	ROM_LOAD( "scph-10000_bios_v1_jap_100.nvm", 0x000000, 0x000400, CRC(8ccf5c07) SHA1(d2213680f9c5ea1718b241b6fa49518e8881906e) )
	ROM_LOAD( "scph18000.bin", 0x000000, 0x080000, CRC(0dcce9d7) SHA1(d7d6be084f51354bc951d8fa2d8d912aa70abc5e) )
	ROM_LOAD( "scph10000.nvm", 0x000000, 0x000400, CRC(cab11486) SHA1(06bbfe8ce3b15bd9c61bfb32943acf3a9cf5f185) )
	ROM_LOAD( "scph-30000_bios_v4_jap_150.erom", 0x000000, 0x1c0000, CRC(dbaf1f8b) SHA1(9b06efa8eadcb7370f45b6dacd201a0f933e1d3e) )
	ROM_LOAD( "scph-30000_bios_v4_jap_150.nvm", 0x000000, 0x000400, CRC(66845ab8) SHA1(ed05a503f746c3c521c5fcee2119b1e44d008833) )
	ROM_LOAD( "scph-30000_bios_v4_jap_150.rom1", 0x000000, 0x040000, CRC(cc009eea) SHA1(59136fba07fff0b6aa3ba3fa92677904e8eaf0b9) )
	ROM_LOAD( "scph-30000_bios_v4_jap_150.bin", 0x000000, 0x400000, CRC(4fc3b495) SHA1(d6f365a0f07cd04ed28108e6ec5076e2f81e5f72) )
	ROM_LOAD( "scph-30003_bios_v3_uk_120.bin", 0x000000, 0x400000, CRC(7b08c33b) SHA1(274c05fec654913a3f698d4b0d592085866a2cbd) )
	ROM_LOAD( "scph-30004r_bios_v6_eur_160.nvm", 0x000000, 0x000400, CRC(3493bb06) SHA1(6d70031a3a931fb6770eefef0f306b9771677077) )
	ROM_LOAD( "scph-30004r_bios_v6_eur_160.bin", 0x000000, 0x400000, CRC(9386a740) SHA1(8fa040852d4b8688f0c84bcfffc65eb208f2b432) )
	ROM_LOAD( "scph-39001_bios_v7_usa_160.nvm", 0x000000, 0x000400, CRC(e14a6cd6) SHA1(219647cab29e4f702c36ec08d183404161c26265) )
	ROM_LOAD( "scph-39001_bios_v7_usa_160.bin", 0x000000, 0x400000, CRC(a19e0bf5) SHA1(f9a5d629a036b99128f7cb530c6e3ca016e9c8b7) )
	ROM_LOAD( "scph-39004_bios_v7_eur_160.nvm", 0x000000, 0x000400, CRC(482d8e77) SHA1(ef5e53b321955cd02d23c327ff8385f50c6eb5ec) )
	ROM_LOAD( "scph-39004_bios_v7_eur_160.rom1", 0x000000, 0x040000, CRC(e33543d4) SHA1(2a82bad5b7b42524b957317a65cae2f0bade4a40) )
	ROM_LOAD( "scph39004.bin", 0x000000, 0x400000, CRC(1f2a283c) SHA1(004cc467439f053d5a1fcf4d1b7c13338ce63403) )
	ROM_LOAD( "scph-39004_bios_v7_eur_160.erom", 0x000000, 0x1c0000, CRC(da81998e) SHA1(bd4879890d2d34fbf06a7ddcd206ca60e6c89c28) )
	ROM_LOAD( "scph-39004_bios_v7_eur_160.bin", 0x000000, 0x400000, CRC(2fe21e4d) SHA1(bff2902bd0ce9729a060581132541e9fd1a9fab6) )
	ROM_LOAD( "scph-50000_bios_vx_jap_170.bin", 0x000000, 0x400000, CRC(9457f64e) SHA1(d812ac65c357d392396ca9edee812dc41bed8bde) )
	ROM_LOAD( "scph50003.bin", 0x000000, 0x400000, CRC(a5860b09) SHA1(003f9bdae45a04c5eb28689813e818566a8c4610) )
	ROM_LOAD( "scph-50004_bios_v9_eur_190.bin", 0x000000, 0x400000, CRC(bfe41270) SHA1(dbd7f4d41d54e4f0bf3c4042bb42a42d5d4ef95e) )
	ROM_LOAD( "scph-50004_bios_v9_eur_190.rom1", 0x000000, 0x080000, CRC(f70813cb) SHA1(883798fac48efdb80a3a848e67b8a3ed26fc8338) )
	ROM_LOAD( "scph-50004_bios_v9_eur_190.nvm", 0x000000, 0x000400, CRC(efb5af2e) SHA1(60cacbf3d72e1e7834203da608037b1bf83b40e8) )
	ROM_LOAD( "scph-50004_bios_v9_eur_190.erom", 0x000000, 0x300000, CRC(6a37e17d) SHA1(7fb0136d70cd5151c67be1e8cf086f8b343c87e7) )
	ROM_LOAD( "scph-50009_bios_v10_chn_190.bin", 0x000000, 0x400000, CRC(40d6c676) SHA1(b7548a92bd2caa9d60cbc7f79573a5d510f88012) )
	ROM_LOAD( "scph-50009_bios_v10_chn_190.erom", 0x000000, 0x300000, CRC(ff480141) SHA1(2ed11fd9333e62e36659565a744330a5902ed5e3) )
	ROM_LOAD( "scph-50009_bios_v10_chn_190.nvm", 0x000000, 0x000400, CRC(d116bf8a) SHA1(3ffa2e74c8c8ce0e10e2069956f12d2407b91c65) )
	ROM_LOAD( "scph-50009_bios_v10_chn_190.rom1", 0x000000, 0x080000, CRC(929b97f6) SHA1(b284280341f16b1581c28ff81934bfdf403da1f0) )
	ROM_LOAD( "scph-50009_bios_v10_chn_190.rom2", 0x000000, 0x080000, CRC(d374379b) SHA1(7aef1dd6de4b4f64f31daad615bbccdb3e62e79c) )
	ROM_LOAD( "scph-70000_bios_v12_jap_200.rom1", 0x000000, 0x080000, CRC(0e8797c0) SHA1(cc44355ded232a2ba5309bb24d4cd55af08696b2) )
	ROM_LOAD( "scph-70000_bios_v12_jap_200.rom2", 0x000000, 0x07fffc, CRC(70d1fd12) SHA1(14a36eeac08796be39626539279ddb9e3744e687) )
	ROM_LOAD( "scph-70000_bios_v12_jap_200.nvm", 0x000000, 0x000400, CRC(62579c93) SHA1(480083ad18b029fd859131fd21aefc26742ca836) )
	ROM_LOAD( "scph-70000_bios_v12_jap_200.bin", 0x000000, 0x400000, CRC(2f314730) SHA1(224ab5704ab719edeb05ca1d835812252c97c1b3) )
	ROM_LOAD( "scph-70004_bios_v12_eur_200.bin", 0x000000, 0x400000, CRC(6f8e3c29) SHA1(434bc0b4eb4827da0773ec0795aadc5162569a07) )
	ROM_LOAD( "scph-70004_bios_v12_eur_200.nvm", 0x000000, 0x000400, CRC(a8d51ead) SHA1(d3cb3b94c73e7a8afedc8294b108630a9df8164c) )
	ROM_LOAD( "scph-70004_bios_v12_eur_200.erom", 0x000000, 0x300000, CRC(bf3cbc65) SHA1(ec0121bb8e2a209044309780d2efc2559c9a0ef1) )
	ROM_LOAD( "scph-70006_bios_v12_hk_200.nvm", 0x000000, 0x000400, CRC(df95b3e3) SHA1(06ccc54cdbe8b017a629155ccb66310cba9c4720) )
	ROM_LOAD( "scph-70006_bios_v12_hk_200.bin", 0x000000, 0x400000, CRC(b57201bf) SHA1(7f8e812cab7c7393c85eac6c42661e1fd0a642df) )
	ROM_LOAD( "scph-70008_bios_v12_rus_200.bin", 0x000000, 0x400000, CRC(92aa71a2) SHA1(cce6fac0f7e682ad167e1e828b2d53192c3d5051) )
	ROM_LOAD( "scph-70008_bios_v12_rus_200.nvm", 0x000000, 0x000400, CRC(c5dd92d4) SHA1(6611151e8f599230cdf19a17318e7e5ddc781961) )
	ROM_LOAD( "scph-70012_bios_v12_usa_200.nvm", 0x000000, 0x000400, CRC(bbeeab7f) SHA1(6aa84081567beba4c1cf6dad4018a293c9846382) )
	ROM_LOAD( "scph-70012_bios_v12_usa_200.bin", 0x000000, 0x400000, CRC(7ebd68de) SHA1(7a62e5f48603582707e9898eb055ea3eaee50d4c) )
	ROM_LOAD( "scph-75008_bios_v14_rus_220.nvm", 0x000000, 0x000400, CRC(86547c14) SHA1(fff13c6e30c55df9d6241ee08e5d0026c6a5d618) )
	ROM_LOAD( "scph-75008_bios_v14_rus_220.rom0", 0x000000, 0x400000, CRC(e2862e39) SHA1(929a85e974faf4b40d0a7785023b758402c43bd9) )
	ROM_LOAD( "scph-77000_bios_v15_jap_220.rom1", 0x000000, 0x080000, CRC(0917d374) SHA1(b96cf94772f01b8434038285896467884a84b0c2) )
	ROM_LOAD( "scph-77000_bios_v15_jap_220.nvm", 0x000000, 0x000400, CRC(63e93a3c) SHA1(16d03c6ed2fa155a22caa88fcc29f3aad90ac689) )
	ROM_LOAD( "scph-77000_bios_v15_jap_220.erom", 0x000000, 0x300000, CRC(a96ce863) SHA1(6b357e8624095770952fc408cdc6647fa62e0d28) )
	ROM_LOAD( "scph-77000_bios_v15_jap_220.bin", 0x000000, 0x400000, CRC(493c1e58) SHA1(d9a7537fa463fcdd3e270af14a93731736cafc4a) )
	ROM_LOAD( "scph-77001_bios_v15_usa_220.nvm", 0x000000, 0x000400, CRC(990a8e27) SHA1(e8ed29188a06d47900e51d94630dc355cfb85a30) )
	ROM_LOAD( "scph-77001_bios_v15_usa_220.rom0", 0x000000, 0x400000, CRC(1279fce9) SHA1(92e488d5b2705e4cca83d4d1efbc421012faf83e) )
	ROM_LOAD( "scph-77004_bios_v15_eur_220.diff", 0x000000, 0x400000, CRC(30e3370b) SHA1(0dfbc7a4831a00f5e4dd887f305f9dfdcba3ba51) )
	ROM_LOAD( "scph-77004_bios_v15_eur_220.rom0", 0x000000, 0x400000, CRC(23fa7baa) SHA1(28ad756d0cfd1e7b2e2de3de5d9e14207ee89761) )
	ROM_LOAD( "scph-77004_bios_v15_eur_220.rom1", 0x000000, 0x400000, CRC(cecc5e2d) SHA1(747e36b086aceceb300c72c0d1bbab38e2920e63) )
	ROM_LOAD( "scph-77004_bios_v15_eur_220.nvm", 0x000000, 0x000400, CRC(fc979a77) SHA1(5a16f2ef10a936a37aee30b560015117527df47a) )
	ROM_LOAD( "scph-77008_bios_v15_rus_220.nvm", 0x000000, 0x000400, CRC(7ba25709) SHA1(84a9628af6fa8335247c002b1ce6c81a419ea9a5) )
	ROM_LOAD( "scph-77008_bios_v15_rus_220.diff", 0x000000, 0x400000, CRC(7c3ff671) SHA1(d618a77a69dc24f4c9f603fc0d566f004091811c) )
	ROM_LOAD( "scph-90001_bios_v18_usa_230.rom0", 0x000000, 0x400000, CRC(286897c2) SHA1(f9229fe159d0353b9f0632f3fdc66819c9030458) )
	ROM_LOAD( "scph-90001_bios_v18_usa_230.nvm", 0x000000, 0x000400, CRC(b794a878) SHA1(258ad0f26b0b269462bbd9acbb5506d5dfbcc3d5) )
	ROM_LOAD( "scph-90001_bios_v18_usa_230.diff", 0x000000, 0x400000, CRC(15d68051) SHA1(6472c74fff309e2de2f9a8b2ff7ca1beba351c06) )
	ROM_LOAD( "fps2bios.rom", 0x000000, 0x07eba0, CRC(ba45bd94) SHA1(a375c34adbcf0cd840049c20e672305c6fe173ab) ) // WIP_040527
	ROM_LOAD( "ps2-0220a-20050620.bin", 0x000000, 0x400000, CRC(d305a97a) SHA1(48d0445dffd1e879c7ae752c5166ec3101921555) )
	ROM_LOAD( "rom1.bin",     0x000000, 0x030c00, CRC(2c3bcd32) SHA1(47d2ec4b342649e4c391043ab915d4435f9d180d) )
ROM_END

CONS(2000, ps2, 0, 0, ps2sony, ps2sony, ps2sony_state, 0, "Sony", "Playstation 2", MACHINE_IS_SKELETON )
