// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/******************************************************************************
*       BPMMicro (formerly BPMicro) universal device programmers
*       Models supported in this driver: (BP=1148) BP-1200
*
*       All models:
*       Legacy:
*           EP-1 - 28 pin eproms, has adapter for 32 pin?
*           PLD-1100 - ndip plds/pals only?
*           EP-1140 - 32 pin eproms only?
*       286 based:
*           BP-1148 - fixed 512k ram, uses a special BP-1148 socket instead of a tech adapter+socket module
*           BP-1200 - fixed 512k ram, uses a TP-48 or TP-84 tech adapter, otherwise identical to above, same firmware
*       286 based w/extra header for >84 (up to 240?) pin drivers, expandable ram:
*           BP-1400 - uses a 30? pin SIMM for up to 8MB? of ram
*       Unclear whether 286 or 486, all have extra button per programmer:
*           BP-2100/84x4 - four BP-1?00s ganged together [1400?]
*           BP-2200/240x2 - two BP-1?00s ganged together [1600?]
*           BP-2200/240x4 - four BP-1?00s ganged together
*           BP-2200/240x6 - six BP-1?00s ganged together
*           BP-2500/240x4 - four BP-1?00s ganged together
*           BP-2000, BP-2600M - ganged/bulk/autofeed programmers?
*           Silicon Sculptor - custom firmware locked to Actel fpga/pld devices, may have a custom MB
*           Silicon Sculptor 6X - as above but 6 programmers ganged together
*           Silicon Sculptor II
*       486 based:
*           BP-1600 - 486DX2 based, uses a 72 pin SIMM for up to 16MB of ram (does NOT support 32MB SIMMs!)
*       probably 'universal platform':
*           BP-2510
*       486+USB 2nd gen 'universal platform':
*           BP-1610 - 486DX2 based, uses a laptop SODIMM for up to 512MB? of ram?, has USB
*           BP-1410 - 486DX2 based, uses a laptop SODIMM for up to 512MB? of ram?, has USB
*           BP-1710 - same as BP1610, but two programmers ganged together in a single case
*           Silicon Sculptor III and IV?
*
******************************************************************************
*       TODO:
*       Everything!
*       status LEDs
*       8-pin pin driver boards
*       tech adapter relays
*       Parallel port interface
*       Serial EEPROM (93c46) at U26 on BP1200 mainboard
*       Serial EEPROM (93c46) at (U1? U7? U12?) on TA-84 tech adapter
*       Serial EEPROM (93c46a) at ? on SM48D socket adapter
*       other socket adapters other than sm48d
*       DONE:
*       hardware pictures, cpu documentation, crystal, ram size
******************************************************************************
*       Links:
*       http://www3.bpmmicro.com/web/helpandsupport.nsf/69f301ee4e15195486256fcf0062c2eb/8194a48179484c9f862573220065d38e!OpenDocument
*       ftp://ftp.bpmmicro.com/Dnload/
******************************************************************************/

/* Core includes */
#include "emu.h"
#include "cpu/i86/i286.h"

class bpmmicro_state : public driver_device
{
public:
	bpmmicro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	DECLARE_DRIVER_INIT(bp1200);
private:
	required_device<cpu_device> m_maincpu;
};

/******************************************************************************
 Driver Init
******************************************************************************/

DRIVER_INIT_MEMBER(bpmmicro_state,bp1200)
{
}

/******************************************************************************
 Read/Write handlers
******************************************************************************/


/* todo (v1.24 rom)
from boot (2 digit hex offsets have 0x84000 added to them):
0x0a00 -> 0x82200
0x0001 -> 18
0x0003 -> 1a
0x0001 -> 0e
{lots of data banged to 1c}
0x0001 -> 02
0x0000 -> 02
read <- 00

0x0001 -> 18
0x0002 -> 1a
{lots of data banged to 1c}
0x0001 -> 02
0x0000 -> 02

0x0000 -> 18
0x0003 -> 1a
{lots of data banged to 1c}
0x0001 -> 02
0x0000 -> 02
read <- 00
read <- 00

0x0000 -> 18
0x0001 -> 1a
{lots of data banged to 1c}
0x0001 -> 02
0x0000 -> 02

0x0000 -> 0e
0x0001 -> 0e
0x0000 -> 0e
0x0001 -> 0e
0x0000 -> 0e
0x0001 -> 0e
read <- 00

0x0a00 -> 0x82200
0x0112 -> 0x82200

0x0000 -> 06
0x0000 -> 04
0x0000 -> 00

*/


/******************************************************************************
 Address Maps
******************************************************************************/

static ADDRESS_MAP_START(i286_mem, AS_PROGRAM, 16, bpmmicro_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x07ffff) AM_RAM
	// 82200 w
	// 84000 r
	// 8400e w
	// 84018 w
	// 8401a w - gets written with 3, then 2, then..
	// 8401c w - bits 0 and 1 are serial data and clock respectively, either for serial over the parallel port, or they connect to one of the 93c46s (probably the one on the mainboard @ U26?)
	AM_RANGE(0x0e0000, 0x0fffff) AM_ROM AM_REGION("bios", 0)
	AM_RANGE(0xfe0000, 0xffffff) AM_ROM AM_REGION("bios", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(i286_io, AS_IO, 16, bpmmicro_state)
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END


/******************************************************************************
 Input Ports
******************************************************************************/
static INPUT_PORTS_START( bpmmicro )
INPUT_PORTS_END

/******************************************************************************
 Machine Drivers
******************************************************************************/

static MACHINE_CONFIG_START( bpmmicro, bpmmicro_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80286, XTAL_32MHz/4) /* divider is guessed, cpu is an AMD N80L286-16/S part */
	MCFG_CPU_PROGRAM_MAP(i286_mem)
	MCFG_CPU_IO_MAP(i286_io)
MACHINE_CONFIG_END



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(bp1200)
	ROM_REGION(0x20000, "bios", 0)
	// note about roms: the BP-1200 has two jumpers controlling what type of rom is installed;
	// it needs 120ns or faster roms
	// the "W1" and "W2" labels are next to pins A on the pcb
	// for 2764 roms:
	// W1 [A B]C
	// W2 [A B]C
	// for 27256 roms:
	// W1 [A B]C
	// W2  A[B C]
	ROM_DEFAULT_BIOS("v124")
	ROM_SYSTEM_BIOS( 0, "v124", "BP-1200 V1.24")
	ROMX_LOAD("version_1.24_u8.st_m27c256b-12f1l.u8", 0x10001, 0x8000, CRC(86d46d76) SHA1(4733b03a28689a3d2c58278495fbf31d0c74ac01), ROM_SKIP(1) | ROM_BIOS(1)) // "bios1200.v124a.u8" on bpmmicro site
	ROMX_LOAD("version_1.24_u9.st_m27c256b-12f1l.u9", 0x10000, 0x8000, CRC(3bcc5c72) SHA1(3b281f2b464d8a4e366f8e2f0a8fa6dfd0a8f28c), ROM_SKIP(1) | ROM_BIOS(1)) // "bios1200.v124a.u9" on bpmmicro site
	ROM_SYSTEM_BIOS( 1, "v118", "BP-1200 V1.18")
	ROMX_LOAD("version_1.18_u8_2000.am27c256.u8", 0x10001, 0x8000, CRC(f8afa614) SHA1(a372bc35aea30595ab8f05c5e641021b45043ed3), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("version_1.18_u9_2000.am27c256.u9", 0x10000, 0x8000, CRC(049b2ad1) SHA1(c9405ff805f3814493ad007bae7a8cb6a12aeb32), ROM_SKIP(1) | ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "v113", "BP-1200 V1.13")
	ROMX_LOAD("bp-1200_v1.13_u8_1992_1997.at27c256r-12pc.u8", 0x10001, 0x8000, CRC(ec61dcad) SHA1(dbfee285456d24b93c1fa6e8557b13ab80c3c877), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD("bp-1200_v1.13_u9_1992_1995.at27c256r-12pc.u9", 0x10000, 0x8000, CRC(91ca5e70) SHA1(4a8c1894a67dfd5e0db088519a3ee4edaafdef58), ROM_SKIP(1) | ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 3, "v111", "BP-1200 V1.11")
	ROMX_LOAD("bp-1200_version_1.11_u8_1992_1995.at27c256r-12pc.u8", 0x10001, 0x8000, CRC(d1c051e4) SHA1(b27007a931b0662b3dc7e2d41c6ec5ed0cd49308), ROM_SKIP(1) | ROM_BIOS(4))
	ROMX_LOAD("bp-1200_version_1.11_u9_1992_1995.at27c256r-12pc.u9", 0x10000, 0x8000, CRC(99d46ba1) SHA1(144dbe6ed989ea88cfc1f6d1142508bb92519f87), ROM_SKIP(1) | ROM_BIOS(4))
	ROM_SYSTEM_BIOS( 4, "v105", "BP-1200 V1.05")
	ROMX_LOAD("bp-1200_1.05_u8__(c)_1993_rev_c__bp_microsystems.27c64-12pc.u8", 0x1c001, 0x2000, CRC(2c13a43c) SHA1(5dd67a09f72f693c085160b9beedd2454ba4ec37), ROM_SKIP(1) | ROM_BIOS(5)) // "BP-1200 1.05 U8 // (C) 1993 REV C // BP Microsystems" 27C64-12PC @U8
	ROMX_LOAD("bp-1200_1.05_u9__(c)_1993_rev_c__bp_microsystems.27c64-12pc.u9", 0x1c000, 0x2000, CRC(b88a311c) SHA1(fb5e0543c811cbbf8f24d1de204b4c0c1bd2f485), ROM_SKIP(1) | ROM_BIOS(5)) // "BP-1200 1.05 U9 // (C) 1993 REV C // BP Microsystems" 27C64-12PC @U9
	ROM_SYSTEM_BIOS( 5, "v104", "BP-1200 V1.04")
	ROMX_LOAD("bp-1200_1.04_u8__(c)_1992_rev_c__bp_microsystems.27c64.u8", 0x1c001, 0x2000, CRC(2ab47324) SHA1(052e578dae5db023f94b35d686a5352ffceec414), ROM_SKIP(1) | ROM_BIOS(6)) // "BP-1200 1.04 U8 // (C) 1992 REV C // BP Microsystems" OTP 27C64 @ U8
	ROMX_LOAD("bp-1200_1.04_u9__(c)_1993_rev_c__bp_microsystems.27c64.u9", 0x1c000, 0x2000, CRC(17b94d7a) SHA1(7ceed660dbdc638ac86ca8ba7fa456c297d88766), ROM_SKIP(1) | ROM_BIOS(6)) // "BP-1200 1.04 U9 // (C) 1993 REV C // BP Microsystems" OTP 27C64 @ U9
	ROM_SYSTEM_BIOS( 6, "v103", "BP-1200 V1.03")
	ROMX_LOAD("bp-1200_1.03_u8__(c)_1992_rev_c__bp_microsystems.27c64a-12.u8", 0x1c001, 0x2000, CRC(b01968b6) SHA1(d0c6aa0f0fe47b0915658e8c27286ab6ea90972e), ROM_SKIP(1) | ROM_BIOS(7)) // "BP-1200 1.03 U8 // (C) 1992 REV C // BP Microsystems" 27C64A-12 @ U8
	ROMX_LOAD("bp-1200_1.03_u9__(c)_1992_rev_c__bp_microsystems.27c64a-12.u9", 0x1c000, 0x2000, CRC(f58ffebb) SHA1(700d3ffed269fff6dc1cf2190bde8b989715c22a), ROM_SKIP(1) | ROM_BIOS(7)) // "BP-1200 1.03 U9 // (C) 1992 REV C // BP Microsystems" 27C64A-12 @ U9
ROM_END



/******************************************************************************
 Drivers
******************************************************************************/

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT     STATE           INIT           COMPANY              FULLNAME      FLAGS */
COMP( 1991, bp1200,     0,          0,      bpmmicro,   bpmmicro, bpmmicro_state, bp1200,      "BP Microsystems",   "BP-1200",    MACHINE_IS_SKELETON | MACHINE_NO_SOUND_HW )
