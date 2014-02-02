// licenses: MAME, BSD
// copyright-holders:R. Belmont
/***************************************************************************

	Fanuc System 15

	2014-01-04 Skeleton driver.

	This is a circa-1990 CNC machine with a very custom GUI, made of a
	bunch of boards plugged into a passive backplane.

	Possible boards include:
	A16B-2200-0020/04B : Main CPU Boa
	A16B-2200-0090/07A : Digital Servo 4-Axis Controller Board (no ROMs on this board)
	A16B-2200-0121/06C : Base 0 Board (1 ROM location but position is empty. -0120 has 6081 001A)

	A16B-2200-0131/11B : Base 1 Board (15 ROMs on this board, 4040, 9030, AB02)
		Fanuc FBI-A MB605111 (QFP100)
		41256-12 (x18)
		HD68HC000-12 (68000 CPU @ 12MHz, PLCC68)
		Fanuc SLC01 MB661128 (PLCC68)
		Fanuc BOC MB605117U (QFP100)
		HM53461-12 (x5)
		MB81C79A-35 (SOP28)
		24MHz OSC
		15 EPROMs

	A16B-2200-0150/03A : Conversational Board (4 ROMs on this board, 5A00, probably for Mill)
	or
	A16B-2200-0150/03A : Conversational Board (4 ROMs on this board, 5C30. 5C for 2 axis Lathe)
	or
	A16B-2200-0150/03A : Conversational Board (4 ROMs on this board, 5D30. 5D for 4 axis (twin Turret) lathe)
		Fanuc FBI-A MB605111 (QFP100)
		MB1422A (DIP42 - DRAM Controller)
		51256-10 (x18)
		MB89259A (SOP28)
		Intel 80286-8
		Intel 80287-8
		MBL82288-8 (DIP10)
		uPB8284 (DIP18)
		24MHz & 32MHz OSC's
		4 EPROMs

	A16B-2200-0160/07B : Graphic Board (3 ROMs on this board, 6082 001A, 600B 001D, 600B 002D. 600B is for Mill)
	or
	A16B-2200-0160/07B : Graphic Board (3 ROMs on this board, 6082 001A, 600A 001C, 600A 001D. 600A is for Lathe)
	Fanuc FBI-A MB605111 (QFP100)
		HD68HC000P10 (68000 @10MHz, PLCC68)
		HM62256-10 (x2)
		HM53461-12 (x3)
		HD63484 (PLCC68)
		HD6445 (PLCC44)
		Fanuc GBC MB652147 (PGA135)
		MB81464-12 (x12)
		20MHz OSC
		3 EPROMs. 6082 is common to both mills and lathes.
		600A is a known EPROM version required for lathes with FAPT conversational graphics.
		600B is unknown. Possibly for mills with FAPT.

	A20B-1003-0230/09C : Motherboard (dumb backplane, contains only slots)
	A20B-1003-0500/01A : Additional Graphic Board for 15TTF only (for TT, connects to A16B-2200-0160 Board. No info on this PCB)
	A20B-1003-0240/07B : Connector Assembly Unit / IO Board
	A20B-1003-0580/01A : PMC Cassette C (16000 Step + Pascal 128KB, small PCB in a yellow plastic box, contains just 2 EPROMs)

	Summary of CPUs:
	68020 (unknown speed) main CPU on "Base 2 board"
	68000-12 sub CPU on "Base 1 board"
	68000-10 graphics CPU on "Graphic board"
	80286-8 mill/lathe interface CPU
 
****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/i86/i286.h"

class fanucs15_state : public driver_device
{
public:
	fanucs15_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")		// main 68020
		, m_subcpu(*this, "subcpu")			// sub 68000-12
		, m_gfxcpu(*this, "gfxcpu")			// gfx 68000-10
		, m_convcpu(*this, "convcpu")		// conversational 80286-8
	{ }

	required_device<m68020_device> m_maincpu;
	required_device<m68000_device> m_subcpu;
	required_device<m68000_device> m_gfxcpu;
	required_device<i80286_cpu_device> m_convcpu;

private:
	virtual void machine_reset();
};

static ADDRESS_MAP_START(maincpu_mem, AS_PROGRAM, 32, fanucs15_state)
	AM_RANGE(0x00000000, 0x0017ffff) AM_ROM AM_REGION("base1b", 0)
	AM_RANGE(0x000f8000, 0x000fffff) AM_RAM	// filled with 0x96 on boot
	AM_RANGE(0xffff0000, 0xffffffff) AM_RAM	// initial stack
ADDRESS_MAP_END

static ADDRESS_MAP_START(subcpu_mem, AS_PROGRAM, 16, fanucs15_state)
	AM_RANGE(0x000000, 0x03ffff) AM_ROM AM_REGION("base1a", 0)
// some data ROM? (shared RAM?) goes at 80000 and prevents this from crashing.  not a program mirror, not the cassette.
// logically this would be the 9030_001e ROM since the 001e suffix matches this program, but that + 0x140 has an odd DWORD which causes
// an address error when used as a word pointer.  wheeeeee.
	AM_RANGE(0xfde000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(gfxcpu_mem, AS_PROGRAM, 16, fanucs15_state)
	AM_RANGE(0x000000, 0x01ffff) AM_ROM AM_REGION("gfxboard", 0)
	AM_RANGE(0xfe0000, 0xfeffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(convcpu_mem, AS_PROGRAM, 16, fanucs15_state)
	AM_RANGE(0x000000, 0x03ffff) AM_ROM AM_REGION("conversational", 0x40000)
	AM_RANGE(0x800000, 0x81ffff) AM_RAM
	AM_RANGE(0xf80000, 0xffffff) AM_ROM AM_REGION("conversational", 0)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( fanucs15 )
INPUT_PORTS_END

void fanucs15_state::machine_reset()
{
}

static MACHINE_CONFIG_START( fanucs15, fanucs15_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68020, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(maincpu_mem)
	MCFG_DEVICE_DISABLE()

	MCFG_CPU_ADD("subcpu", M68000, XTAL_12MHz)
	MCFG_CPU_PROGRAM_MAP(subcpu_mem)
	MCFG_DEVICE_DISABLE()

	MCFG_CPU_ADD("gfxcpu", M68000, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(gfxcpu_mem)

	MCFG_CPU_ADD("convcpu", I80286, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(convcpu_mem)
	MCFG_DEVICE_DISABLE()
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( fanucs15 )   
	ROM_REGION16_BE( 0x48000, "base1a", 0 )	// 68000 sub CPU code and data on base 1 board
	ROM_LOAD16_BYTE( "4040_001e_001.23m", 0x000001, 0x020000, CRC(2e12109f) SHA1(83ed846d3d59ab0d81b2e2e2231d1a444e462590) ) 
	ROM_LOAD16_BYTE( "4040_002e_002.27m", 0x000000, 0x020000, CRC(a5469692) SHA1(31c44edb36fb69d3d418a97e32e4a2769d1ec9e7) ) 
	ROM_LOAD( "9030_001e_381.18j", 0x040000, 0x008000, CRC(9f10a022) SHA1(dc4a242f7611143cc2d9564993fd5fa52f0ac13a) ) 

	ROM_REGION32_BE( 0x180000, "base1b", 0 )	// 68020 main CPU code and data on base 1 board
	ROM_LOAD16_BYTE( "ab02_081a_081.23e", 0x000001, 0x020000, CRC(5328b023) SHA1(661f2908f3287f7cd2b215cd29962f2789f7d99a) ) 
	ROM_LOAD16_BYTE( "ab02_082a_082.27e", 0x000000, 0x020000, CRC(ad37740f) SHA1(e65cc0a8b4e515fcf5fcefde99e95d100d310018) ) 
	ROM_LOAD16_BYTE( "ab02_0c1a_0c1.23f", 0x040001, 0x020000, CRC(62566569) SHA1(dd85b6e7875d996759b833552b00e1b3a0e3696b) ) 
	ROM_LOAD16_BYTE( "ab02_0c2a_0c2.27f", 0x040000, 0x020000, CRC(a4ade1fe) SHA1(44ef5358b34d3538fb061f235b8a14bac6b5faa8) ) 
	ROM_LOAD16_BYTE( "ab02_101a_101.23h", 0x080001, 0x020000, CRC(96f5d00e) SHA1(f5de3621df536435d27a0aac1c9d25e69601bd40) ) 
	ROM_LOAD16_BYTE( "ab02_102a_102.27h", 0x080000, 0x020000, CRC(e23a5414) SHA1(f6aff51dfd6d976b7cd33399c7aa3d06c7c06919) ) 
	ROM_LOAD16_BYTE( "ab02_141a_141.23k", 0x0c0001, 0x020000, CRC(3ceb6809) SHA1(7e37b18847b35f81c08b7b2ab62e99fa3a737c32) ) 
	ROM_LOAD16_BYTE( "ab02_142a_142.27k", 0x0c0000, 0x020000, CRC(1d8a4d7d) SHA1(580322e2927742bbcbf0bb2757730e2817b320e1) )
	// this appears to be a different version of the 081a/0c1a ROM program.  it's definitely for a 68020 (32-bit pointers everywhere)
	ROM_LOAD16_BYTE( "ab02_281a_281.8a",  0x100001, 0x020000, CRC(cec79742) SHA1(1233ff920d607206a80c8d187745e3d657a8635d) )
	ROM_LOAD16_BYTE( "ab02_282a_282.8d",  0x100000, 0x020000, CRC(63eacc0e) SHA1(1f25b99280112c720d778219b4610f556f33a7f1) ) 
	ROM_LOAD16_BYTE( "ab02_2c1a_2c1.10a", 0x140001, 0x020000, CRC(66eb74dd) SHA1(f256763cb15b4524c09bd09b88df46a1498846ef) ) 
	ROM_LOAD16_BYTE( "ab02_2c2a_2c2.10d", 0x140000, 0x020000, CRC(6edf4ff3) SHA1(9cbf7c6555cc27def3b580f5a7b0ff580984206d) ) 

	ROM_REGION16_LE( 0x80000, "conversational", 0 )	// 80286 ROMs, 5a00 for Mill
	ROM_LOAD16_BYTE( "5a00_041b.041", 0x000000, 0x020000, CRC(bf22c0a3) SHA1(56ab70bfd5794cb4db1d87c8becf7af522687564) ) 
	ROM_LOAD16_BYTE( "5a00_042b.042", 0x000001, 0x020000, CRC(7abc9d6b) SHA1(5cb6ad08ce93aa99391d1ce46ac8db8ba2e0f94a) ) 
	ROM_LOAD16_BYTE( "5a00_001b.001", 0x040000, 0x020000, CRC(cd2a2839) SHA1(bc20fd9ae9d071e1df835244aea85648d1bd1dbc) ) 
	ROM_LOAD16_BYTE( "5a00_002b.002", 0x040001, 0x020000, CRC(d9ebbb4a) SHA1(9c3d96e9b88848472210beacdf9d300ddd42d16e) ) 

	ROM_REGION16_BE( 0x50000, "gfxboard", 0 )	// graphics board 68000 code/data.  600a for lathe, 600b for mill, 6082 common to both
	ROM_LOAD16_BYTE( "600b_001d_low.3l",  0x000001, 0x010000, CRC(50566c5c) SHA1(966c8d90d09a9c50c5dedebe9c67f1755846b234) ) 
	ROM_LOAD16_BYTE( "600b_002d_high.3j", 0x000000, 0x010000, CRC(304e1ecb) SHA1(1e4b149b306550750fc03bd80bd399f239f68657) ) 
	ROM_LOAD16_BYTE( "600a_001c.bin",     0x020001, 0x010000, CRC(63d9fc2f) SHA1(280e825ba7b79e7c38282a4f4b762d2219fd873b) ) 
	ROM_LOAD16_BYTE( "600a_002c.bin",     0x020000, 0x010000, CRC(4d78e702) SHA1(a89bd07dc1ae030bdee5a541777825eaadbc2307) ) 
	// font or tilemap data?
	ROM_LOAD( "6082_001a_cg.12b", 0x040000, 0x010000, CRC(f3d10cf9) SHA1(bc5bc88dcb5f347e1442aa4a0897747958a53413) ) 

	ROM_REGION16_BE( 0x40000, "cassette", 0 )	// "PMC Cassette C", ROM cartridge that plugs into the Base 1 board
	ROM_LOAD16_BYTE( "pmc_high.a2",  0x000000, 0x020000, CRC(7b8f9a96) SHA1(08d828b612c45bb3f2f7a56df418cd8e34731bf4) ) 
	ROM_LOAD16_BYTE( "pmc_low.a1",   0x000001, 0x020000, CRC(3ab261f8) SHA1(20b7eef96deb91a3a867f9ac4165b0c188fbcff3) ) 
ROM_END

/* Driver */
/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT     CLASS          INIT COMPANY  FULLNAME       FLAGS */
COMP( 1990, fanucs15, 0,      0,     fanucs15,  fanucs15, driver_device, 0,   "Fanuc", "System 15", GAME_NOT_WORKING | GAME_NO_SOUND)
