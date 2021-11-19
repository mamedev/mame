// license:BSD-3-Clause
// copyright-holders:
/*******************************************************************************

    Skeleton driver for TAB Austria "Silverball" PC-based touch games


    Hardware overview for "Silverball (8.01)":
    Motherboard: Soyo 5EAS / Soyo SY-7IZB+ / Acorp 694XT-694XT1 / others
    32768K RAM (4 x 8MB)
    Intel Pentium-S CPU 133 MHz (A80502133)

    MicroTouch (external)
    ESS AudioDrive ES1869FC ISA sound card
    S3 Trio64V2/DX PCI VGA (86C775, 512KB RAM)
    I/O Controller Silverball 3 (ISA card, Actel A40MX04-F PL84, DB50 external connector)

    HDD Samsung Maxtor G1021U2 or other IDE HDD with similar capacity.

    The manual points to a HardLock parallel port security dongle, but it was missing
    from the dumped machine.

*******************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "speaker.h"

namespace {

class silverball_state : public driver_device
{
public:
	silverball_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void silverball(machine_config &config);

private:
	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<cpu_device> m_maincpu;
};

void silverball_state::mem_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0xfffc0000, 0xffffffff).rom().region("mb_bios", 0);
}

void silverball_state::io_map(address_map &map)
{
}

static INPUT_PORTS_START(silverball)
INPUT_PORTS_END

void silverball_state::silverball(machine_config &config)
{
	PENTIUM(config, m_maincpu, 133'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &silverball_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &silverball_state::io_map);
}


// Silverball machines used different motherboards. By now, we're adding all the known BIOSes here
#define SILVERBALL_BIOS \
	ROM_REGION32_LE(0x40000, "mb_bios", 0) \
	/* Acorp 694XT/694XT1 (all of them from the Silverball software update files) */ \
	ROM_SYSTEM_BIOS(0, "bios47", "BIOS47 (Acorp 694XT/694XT1)") \
	ROMX_LOAD("bios47.bin", 0x00000, 0x40000, CRC(248910ed) SHA1(2901c94fb003e5eaea7e91127ad1f7953826a248), ROM_BIOS(0)) /* 09/03/2001-694X-686A-6A6LJX3AC-00 */ \
	ROM_SYSTEM_BIOS(1, "bios46", "BIOS46 (Acorp 694XT/694XT1)") \
	ROMX_LOAD("bios46.bin", 0x00000, 0x40000, CRC(90074f2b) SHA1(08fdabad6114ede03a15ce51dec775080585b650), ROM_BIOS(1)) /* 09/03/2001-694X-686A-6A6LJX3AC-00 */ \
	ROM_SYSTEM_BIOS(2, "bios45", "BIOS45 (Acorp 694XT/694XT1)") \
	ROMX_LOAD("bios45.bin", 0x00000, 0x40000, CRC(790d5db0) SHA1(6bbfb35275eb0c7bb740ff9b1e299042d4c5952a), ROM_BIOS(2)) /* 09/03/2001-694X-686A-6A6LJX3AC-00 */ \
	ROM_SYSTEM_BIOS(3, "bios44", "BIOS44 (Acorp 694XT/694XT1)") \
	ROMX_LOAD("bios44.bin", 0x00000, 0x40000, CRC(a4b2703e) SHA1(9baa5e3aaaf120054979039e42bf1bca2a50616e), ROM_BIOS(3)) /* 09/03/2001-694X-686A-6A6LJX3AC-00 */ \
	/* No BIOS43? */ \
	ROM_SYSTEM_BIOS(4, "bios42", "BIOS42 (Acorp 694XT/694XT1)") \
	ROMX_LOAD("bios42.bin", 0x00000, 0x40000, CRC(820c057e) SHA1(b553961c1bbb6b4afa366eb0dba7ed29ca148f5e), ROM_BIOS(4)) /* 09/03/2001-694X-686A-6A6LJX3AC-00 */ \
	ROM_SYSTEM_BIOS(5, "bios41", "BIOS41 (Acorp 694XT/694XT1)") \
	ROMX_LOAD("bios41.bin", 0x00000, 0x40000, CRC(0940aebd) SHA1(38c8645eed9e17ef8e2cd0856f929697ce13df74), ROM_BIOS(5)) /* 09/03/2001-694X-686A-6A6LJX3AC-00 */ \
	ROM_SYSTEM_BIOS(6, "bios40", "BIOS40 (Acorp 694XT/694XT1)") \
	ROMX_LOAD("bios40.bin", 0x00000, 0x40000, CRC(c5ed7d4f) SHA1(9fcffb385b5d68bf8cf87615ffee781f38f5fb74), ROM_BIOS(6)) /* 694XT TAB-Bios40 03/09/2001 */ \
	/* Unknown PCB with the BIOS string 'SiS-5600-ITE8661' (all of them from the Silverball software update files) */ \
	ROM_SYSTEM_BIOS(7, "bios39", "BIOS39 (unknown motherboard)") \
	ROMX_LOAD("bios39.bin", 0x00000, 0x40000, CRC(e7541a28) SHA1(dad1aff2cb142d65e7c8736064dce8004451ea4b), ROM_BIOS(7)) /* SiS-5600-ITE8661 */ \
	ROM_SYSTEM_BIOS(8, "bios38", "BIOS38 (unknown motherboard)") \
	ROMX_LOAD("bios38.bin", 0x00000, 0x40000, CRC(cb49085b) SHA1(867b95ebef8fd53dad5b71cd4430ede08cf03540), ROM_BIOS(8)) /* SiS-5600-ITE8661 */ \
	ROM_SYSTEM_BIOS(9, "bios37", "BIOS37 (unknown motherboard)") \
	ROMX_LOAD("bios37.bin", 0x00000, 0x40000, CRC(9f35e2d2) SHA1(83026fc9fc72f795bc3229af98099b409c9bdc81), ROM_BIOS(9)) /* SiS-5600-ITE8661 */ \
	ROM_SYSTEM_BIOS(10, "bios36", "BIOS36 (unknown motherboard)") \
	ROMX_LOAD("bios36.bin", 0x00000, 0x40000, CRC(83ca05e9) SHA1(9d06677907c88481a167e52cc149995cfef0ff40), ROM_BIOS(10)) /* SiS-5600-ITE8661 */ \
	ROM_SYSTEM_BIOS(11, "bios35", "BIOS35 (unknown motherboard)") \
	ROMX_LOAD("bios35.bin", 0x00000, 0x40000, CRC(d5a2f966) SHA1(4f4c9688501814bc00b4cde58ac590b6e961816e), ROM_BIOS(11)) /* SiS-5600-ITE8661 */ \
	ROM_SYSTEM_BIOS(12, "bios34", "BIOS34 (unknown motherboard)") \
	ROMX_LOAD("bios34.bin", 0x00000, 0x40000, CRC(400702fe) SHA1(63a9e04d2ea3d4ace3e5c9f11cc62d0131bc4af0), ROM_BIOS(12)) /* SiS-5600-ITE8661 */ \
	/* Soyo SY-7IZB+ (all of them from the Silverball software update files) */ \
	ROM_SYSTEM_BIOS(13, "bios33", "BIOS33 (Soyo SY-7IZB+)") \
	ROMX_LOAD("bios33.bin", 0x00000, 0x40000, CRC(96658274) SHA1(1a5fc83cf84a4c774abc055688fa0c32c3784086), ROM_BIOS(13)) /* Soyo Computer Inc. SY-7IZB+ */ \
	ROM_SYSTEM_BIOS(14, "bios32", "BIOS32 (Soyo SY-7IZB+)") \
	ROMX_LOAD("bios32.bin", 0x00000, 0x40000, CRC(63318d8a) SHA1(1194a30a65186f2d0a9e11742d51232598c16011), ROM_BIOS(14)) /* Soyo Computer Inc. SY-7IZB+ */ \
	ROM_SYSTEM_BIOS(15, "bios31", "BIOS31 (Soyo SY-7IZB+)") \
	ROMX_LOAD("bios31.bin", 0x00000, 0x40000, CRC(5121b267) SHA1(97e51ed4d53d19ed69db1faba5395b46a1edb6b0), ROM_BIOS(15)) /* Soyo Computer Inc. SY-7IZB+ */ \
	ROM_SYSTEM_BIOS(16, "bios30", "BIOS30 (Soyo SY-7IZB+)") \
	ROMX_LOAD("bios30.bin", 0x00000, 0x40000, CRC(c6e30fb1) SHA1(1d9a900a5383bf58b66a2e9f7fb34c61397dd327), ROM_BIOS(16)) /* Soyo Computer Inc. SY-7IZB+ */ \
	/* Soyo SY-5EAS */ \
	ROM_SYSTEM_BIOS(17, "bios29", "BIOS29 (Soyo 5EAS)") /* Dumped from the actual Silverball 8.06 machine */ \
	ROMX_LOAD("bios29.bin", 0x00000, 0x20000, CRC(ddbd94f4) SHA1(60ad74e56265a7936cf19e8480c657223d11f2d0), ROM_BIOS(17)) /* 06/18/1998-EQ82C6618A-ET-2A5LDS2FC-29 */ \
	ROM_SYSTEM_BIOS(18, "test", "TEST (Soyo SY-5EAS)") /* BIOS update labeled as "TEST", from the Silverball software update files */ \
	ROMX_LOAD("test.bin",   0x00000, 0x20000, CRC(ddbd94f4) SHA1(60ad74e56265a7936cf19e8480c657223d11f2d0), ROM_BIOS(18)) /* 06/18/1998-EQ82C6618A-ET-2A5LDS2FC-29 */

ROM_START(slvrball806)
	SILVERBALL_BIOS
	ROM_DEFAULT_BIOS("bios29") // The one dumped from the actual machine

	DISK_REGION( "ide:0:hdd:image" ) // 16383 cylinders, 16 heads, 63 sectors
	DISK_IMAGE("silverball_8.06", 0, BAD_DUMP SHA1(4bd03240229a2f59d457e95e04837422c423111b)) // May contain operator data
ROM_END

ROM_START(slvrball720)
	SILVERBALL_BIOS
	ROM_DEFAULT_BIOS("bios29") // Not sure what PCB this HD was dumped from

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE("silverball_7.20", 0, BAD_DUMP SHA1(008d0146b579793f9ba2aa2e43ffa7ec1401f752)) // May contain operator data
ROM_END

ROM_START(slvrball632)
	SILVERBALL_BIOS
	ROM_DEFAULT_BIOS("bios29") // Not sure what PCB this HD was dumped from

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE("silverball_6.32", 0, BAD_DUMP SHA1(0193fbc3b27e0b3ad6139830dfec04172eb3089a)) // May contain operator data
ROM_END

// SilverBall V4.09 BULOVA: Windows 3.1, HardLock parallel dongle and two VGA drivers loaded, S3 Trio and Chips and Technologies 6555x (mm55x16) Accelerator
ROM_START(slvrballbu409)
	SILVERBALL_BIOS
	ROM_DEFAULT_BIOS("bios29") // Not sure what PCB this HD was dumped from

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE("silverball_bulova_4.09_1", 0, BAD_DUMP SHA1(da838ddccf285fb4e06d7f752949e745e6b4e2e7)) // May contain operator data
ROM_END

// SilverBall V4.09 BULOVA: Windows 3.1, HardLock parallel dongle and two VGA drivers loaded, S3 Trio and Chips and Technologies 6555x (mm55x16) Accelerator
// Probably the same as set 1, just with different operator data / configuration
ROM_START(slvrballbu409b)
	SILVERBALL_BIOS
	ROM_DEFAULT_BIOS("bios29") // Not sure what PCB this HD was dumped from

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE("silverball_bulova_4.09_2", 0, BAD_DUMP SHA1(86bf947b39cabcd207f79b7d6132199819e1fed7)) // May contain operator data
ROM_END


} // Anonymous namespace

GAME(1997?, slvrball806,    0,           silverball, silverball, silverball_state, empty_init, ROT0, "TAB Austria", "Silverball (8.01)",               MACHINE_IS_SKELETON)
GAME(1997?, slvrball720,    slvrball806, silverball, silverball, silverball_state, empty_init, ROT0, "TAB Austria", "Silverball (7.20)",               MACHINE_IS_SKELETON)
GAME(1997?, slvrball632,    slvrball806, silverball, silverball, silverball_state, empty_init, ROT0, "TAB Austria", "Silverball (6.32)",               MACHINE_IS_SKELETON)

GAME(199?,  slvrballbu409,  slvrball806, silverball, silverball, silverball_state, empty_init, ROT0, "TAB Austria", "Silverball Bulova (4.09, set 1)", MACHINE_IS_SKELETON)
GAME(199?,  slvrballbu409b, slvrball806, silverball, silverball, silverball_state, empty_init, ROT0, "TAB Austria", "Silverball Bulova (4.09, set 2)", MACHINE_IS_SKELETON) // Probably the same as set 1
