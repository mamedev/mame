// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Fanuc System 15

    2014-01-04 Skeleton driver.

    This is a circa-1990 CNC machine with a very custom GUI, made of a
    bunch of boards plugged into a passive backplane.

    Possible boards include:
    A16B-2200-0020/04B : Base 2 Board (contains 12 MHz 68020 "CNC CPU" and 1 or 2 MB of DRAM)
    A16B-2200-0090/07A : Digital Servo 4-Axis Controller Board (no ROMs on this board)
    A16B-2200-0121/06C : Base 0 Board (1 ROM location but position is empty. -0120 has 6081 001A)

    A16B-2200-0131/11B : Base 1 Board (15 ROMs on this board, 4040, 9030, AB02)
        See detailed layout and description below.

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


    Fanuc System 15A Base 1 Board A16B-2200-013

    PCB Layout
    ----------

    A16B-2200-0131/03B (note 1 and 03B have been added/printed later)
           |--------------|     |---|
    |------|     CA34     |-----|CA4|-------------------------------------|
    |      |--------------|75463|---|    LL  LL                   MB81C79A|
    |                       (x6)         LL  LL              |--------|   |
    |                                             HM53461(x5)|MB605117|   |
    |                                   FA8191               |        |   |
    |4040002E.M27 4040001E.M23          FA8191               |        |   |
    |                                                        |--------|   |
    |                                                                     |
    |                                    MC-122-41256A9A-12  |--------|   |
    |AB02142A.K27 AB02141A.K23           MC-122-41256A9A-12  |MB661128|   |
    |                         9030001E.J18                   |        |   |
    |                                                        |        |   |
    |                                                        |--------|   |
    |AB02102A.H27 AB02101A.H23                                            |
    |                                                                     |
    |                                                                     |
    |                                                                     |
    |AB020C2A.F27 AB020C1A.F23              24MHz                         |
    |                                                                     |
    |                                                                     |
    |                                                        |--------|   |
    |AB02082A.E27 AB02081A.E23      |------|                 |MB605111|   |
    |                               |68000 |                 |        |   |
    |                               |      |                 |        |   |
    |   |---------|                 |      |                 |--------|   |
    |   |---------|                 |------|                              |
    |      CNM1                                |-----------------------|  |
    |------------------------------------------|          CNA          |--|
                                               |-----------------------|
    Notes:
          MC-122-41256- NEC MC-122-41256A9A-12 256k RAM SIP module (with parity) containing NEC D41256L-12 32k x8 SRAM
                        2 SIP modules populated, 9 chips per module, 18 total RAMs, total 512k
          68000       - Hitachi HD68HC000-12 68000 CPU. Clock 24/2 (PLCC68)
          MB661128    - Fujitsu Fanuc SLC01 MB661128 (Serial Data Link Controller, PLCC68)
          MB605117    - Fujitsu Fanuc BOC MB605117U (DRAM/SRAM Interface Controller, QFP100)
          MB605111    - Fujitsu Fanuc FBI-A MB605111 (Global/Local Interface Controller, QFP100)
          HM53461     - Hitachi HM53461-12 64k x4-bit multiport CMOS video RAM (x5, SOJ24)
          MB81C79A    - Fujitsu MB81C79A-35 8k x 9-bit (72k) CMOS Static RAM (SOP28)
          FA8191      - Fanuc FA8191 RV07 custom ceramic module (contains resistors and transistors, SIL16)
          75463       - Texas Instruments SN75463 Dual High-Voltage, High-Current Peripheral Driver (DIP8)
          L           - LED
          CA34        - Connector for PMC Cassette C A02B-0094-C103 (holds 2 EPROMs containing the PMC Ladder)
                        The Ladder is specific and unique to each CNC machine, depending on capability and options.
          CA4         - Connector for cable joined to Servo Control Board A16B-2200-0090 or 16B-2200-0091
          CNA         - Connector plugs into motherboard slot CNA2 (BASE 1)
          CNM1        - 40 pin flat cable joined to optional board A16B-1600-0280/02A

          Note about EPROMs:
                            Software Series is denoted by the first 4 digits
                            ROM # (possibly relating to the memory location in hex) is the next 3 digits
                            Software Revision is the last letter
                            Both IC locations and ROM # are printed on this board

          ROM         IC         Memory     EPROM     Used
          Label       Location   Location   Type      For...
          -----------------------------------------------------------------------------------------
          9030001E    J18        381        27256     Digital Servo Control Program Version 9030 Revision E

          4040001E    M23        001        27C1001   PMC Control Program Version 4040 Revision E
          4040002E    M27        002        27C1001

          AB02081A    E23        081        27C1001   Boot Software Version AB02 Revision A
          AB02082A    E27        082        27C1001
          AB020C1A    F23        0C1        27C1001
          AB020C2A    F27        0C2        27C1001
          AB02101A    H23        101        27C1001
          AB02102A    H27        102        27C1001
          AB02141A    K23        141        27C1001
          AB02142A    K27        142        27C1001


    Another identical board from a different machine contains the following EPROMs....
    9030001F (known revisions exist up to at least rev M)
    4040001D, 4040002D (known revisions exist up to at least rev I)
    A202081G, A202082G
    A2020C1G, A2020C2G
    A202101G, A202102G

    The complete boot software series is listed below:

    15TA SOFTWARE SERIES (Single Turret 2 Axis Lathe)
    |------+----+---------+---+---+---+---|
    |S/W   |STEP|   CRT   |I/O|SER|SER|DNC|
    |SERIES|    |         |   |   |   |   |
    |      |1  2| 9M 14 9C|LNK|SPN|FB |1 2|
    +------+----+---------+---+---+---+---+
    |A201  |X   | X       |   |   |   |   |
    |A202  |X   |    X    |   |   |   |   |
    |A211  |   X| X       | X | X |   |X  |
    |A212  |   X|    X    | X | X |   |X  |
    |A219  |   X| X       | X | X | X |   |
    |A220  |   X|    X    | X | X | X |   |
    |A215  |   X| X       | X | X |   |  X|
    |A216  |   X|    X    | X | X |   |  X|
    |A217  |   X| X       | X | X | X |  X|
    |A218  |   X|    X    | X | X | X |  X|
    |A221  |   X|       X | X | X | ? |  ?|
    |------+----+---------+---+---+---+---|
    Notes:
          9M      - 9" Monochrome
          9C      - 9" Color
          14      - 14" Color
          SER SPN - Serial Spindle
          SER FB  - Serial FB ?
          DNC 1/2 - DNC (Direct NC) type 1 or type 2 for Direct PC to CNC program transfer operation


    15TTA SOFTWARE SERIES (Twin Turret 4 Axis Lathe)
    |------+----+---------+---+---+---+---|
    |S/W   |STEP|   CRT   |I/O|SER|SER|DNC|
    |SERIES|    |         |   |   |   |   |
    |      |1  2| 9M 14 9C|LNK|SPN|FB |1 2|
    +------+----+---------+---+---+---+---+
    |A401  |X   | X       |   |   |   |   |
    |A402  |X   |    X    |   |   |   |   |
    |A411  |   X| X       | X | X |   |X  |
    |A412  |   X|    X    | X | X |   |X  |
    |A419  |   X| X       | X | X | X |   |
    |A420  |   X|    X    | X | X | X |   |
    |------+----+---------+---+---+---+---|


    15TTF SOFTWARE SERIES (Twin Turret 4 Axis Lathe with FAPT)
    |------+----+---------+---+---+---+---|
    |S/W   |STEP|   CRT   |I/O|SER|SER|DNC|
    |SERIES|    |         |   |   |   |   |
    |      |1  2| 9M 14 9C|LNK|SPN|FB |1 2|
    +------+----+---------+---+---+---+---+
    |A502  |X   |       X |   |   |   |   |
    |A512  |   X|       X | X | X |   |X  |
    |A520  |   X|       X | X | X | X |   |
    |------+----+---------+---+---+---+---|


    15MA SOFTWARE SERIES (Mill With 3 Axes minimum XYZ)
    |------+----+---------+---+---+---+---|---|
    |S/W   |STEP|   CRT   |I/O|SER|SER|DNC|SUB|
    |SERIES|    |         |   |   |   |   |   |
    |      |1  2| 9M 14 9C|LNK|SPN|FB |1 2|CPU|
    +------+----+---------+---+---+---+---+---|
    |A001  |X   | X       |   |   |   |   |   |
    |A002  |X   |    X    |   |   |   |   |   |
    |A00A  |X   | X       |   |   |   |   |   |
    |A00B  |X   |    X    |   |   |   |   |   |
    |AA01  |X   | X       |   |   |   |   | X |
    |AA02  |X   |    X    |   |   |   |   | X |
    |A011  |   X| X       | X | X |   |X  |   |
    |A012  |   X|    X    | X | X |   |X  |   |
    |AA11  |   X| X       | X | X |   |X  | X |
    |AA12  |   X|    X    | X | X |   |X  | X |
    |A017  |   X| X       | X | X | X |  X|   |
    |A018  |   X|    X    | X | X | X |  X|   |
    |A019  |   X| X       | X | X | X |   |   |
    |A01A  |   X| X       | X | X |   |X  |   |
    |A01B  |   X|    X    | X | X |   |X  |   |
    |A020  |   X|    X    | X | X | X |   |   |
    |A021  |   X|        X| X | X | X |  ?|   |
    |A027 (NEW STANDARD)  |   |   |   |   |   |
    |A028 (NEW STANDARD)  |   |   |   |   |   |
    |A041 (FOR OSI ONLY)  |   |   |   |   |   |
    |A042 (FOR OSI ONLY)  |   |   |   |   |   |
    |AA19  |   X| X       | X | X | X |   | X |
    |AA20  |   X|    X    | X | X | X |   | X |
    |AA13  |   X| X       | X | X | X |  X| X |
    |AA14  |   X|    X    | X | X | X |  X| X |
    |AA23  |   X| X       | X | X | X |   | X |
    |AA24  |   X|    X    | X | X | X |   | X |
    |AA26  |   X|    X    | X | X | X |  ?| X |
    |AA27 (NEW STD)       |   |   |   |   |   |
    |AA28 (NEW STD)       |   |   |   |   |   |
    |AA41 (FOR OSI ONLY)  |   |   |   |   |   |
    |AA42 (FOR OSI ONLY)  |   |   |   |   |   |
    |---------------------+---+---+---+---+---|


    Optional PCB A16B-1600-0280/02A
    -------------------------------

    |---------------------------|
    |    181          182       |
    |                           |
    |                           |
    |                           |
    |    1C1          1C2       |
    |                           |
    |                           |
    |                           |
    |    201          202       |
    |                           |
    |                           |
    |                           |
    |    241          242       |
    |                           |
    |                           |
    |                           |
    |    281          282       |
    |                           |
    |                           |
    |                           |
    |    2C1          2C2       |
    |                           |
    |                           |
    |   |---------|             |
    |   |---------|             |
    |      CNM1                 |
    |---------------------------|
    Notes:
          The A16B-1600-0280/02A board contains only 12 DIP32 sockets and some logic and
          bolts to the back of the BASE 1 board. The board extends the boot software memory
          storage space if additional EPROMS are required. Not all sockets are populated.
          Connector CNM1 joins with a 40 pin flat cable to the BASE 1 board connector CNM1
          IC locations are not printed on this board. Only the ROM # is printed at each
          socket location.

          ROM         IC         Memory     EPROM     Used For
          Label       Location   Location   Type
          ---------------------------------------------------------------------------------
          AB02281A    N/A        281        27C1001   Boot Software Version AB02 Revision A
          AB02282A    N/A        282        27C1001
          AB022C1A    N/A        2C1        27C1001
          AB022C2A    N/A        2C2        27C1001


    15A EPROM population and option type
    ----------------+---------+-------------------------------------------
    PCB NAME        |ROM #    | Option Type (Basic, Option A1/A2/A3 etc)
    ----------------|---------+-------------------------------------------
    BASE 1 main     |081, 082 | B
    BASE 1 main     |0C1, 0C2 | B
    BASE 1 main     |101, 102 | A1 if step1 software, B if step2 software
    BASE 1 main     |141, 142 | A2 (only used for foreign language option)
    BASE 1 daughter |181, 182 | A3 (if used)
    BASE 1 daughter |1C1, 1C2 | not used
    BASE 1 daughter |201, 202 | not used
    BASE 1 daughter |241, 242 | A6 (if used)
    BASE 1 daughter |281, 282 | B when subcpu used
    BASE 1 daughter |2C1, 2C2 | B when subcpu used
    --------------------------+-------------------------------------------
    Note: 2 MEG DRAM is required on BASE2 PCB when A3 or A6 is used

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m68000/m68020.h"
#include "cpu/i86/i286.h"


namespace {

class fanucs15_state : public driver_device
{
public:
	fanucs15_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")       // main 68020
		, m_pmccpu(*this, "pmccpu")         // sub 68000-12
		, m_gfxcpu(*this, "gfxcpu")         // gfx 68000-10
		, m_convcpu(*this, "convcpu")       // conversational 80286-8
	{ }

	void fanucs15(machine_config &config);

private:
	required_device<m68020_device> m_maincpu;
	required_device<m68000_device> m_pmccpu;
	required_device<m68000_device> m_gfxcpu;
	required_device<i80286_cpu_device> m_convcpu;

	void convcpu_mem(address_map &map) ATTR_COLD;
	void gfxcpu_mem(address_map &map) ATTR_COLD;
	void maincpu_mem(address_map &map) ATTR_COLD;
	void pmccpu_mem(address_map &map) ATTR_COLD;

	virtual void machine_reset() override ATTR_COLD;
};

void fanucs15_state::maincpu_mem(address_map &map)
{
	map(0x00000000, 0x0017ffff).rom().region("base1b", 0);
	map(0x000f8000, 0x000fffff).ram(); // filled with 0x96 on boot
	map(0xffff0000, 0xffffffff).ram(); // initial stack
}

void fanucs15_state::pmccpu_mem(address_map &map)
{
	map(0x000000, 0x03ffff).rom().region("base1a", 0);
	map(0xfde000, 0xffffff).ram();
}

void fanucs15_state::gfxcpu_mem(address_map &map)
{
	map(0x000000, 0x01ffff).rom().region("gfxboard", 0);
	map(0xfe0000, 0xfeffff).ram();
}

void fanucs15_state::convcpu_mem(address_map &map)
{
	map(0x000000, 0x03ffff).rom().region("conversational", 0x40000);
	map(0x040000, 0x07ffff).rom().region("conversational", 0);
	map(0x800000, 0x87ffff).ram();
	map(0xf80000, 0xffffff).rom().region("conversational", 0);
}

/* Input ports */
static INPUT_PORTS_START( fanucs15 )
INPUT_PORTS_END

void fanucs15_state::machine_reset()
{
}

void fanucs15_state::fanucs15(machine_config &config)
{
	/* basic machine hardware */
	M68020(config, m_maincpu, XTAL(12'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &fanucs15_state::maincpu_mem);
	m_maincpu->set_disable();

	M68000(config, m_pmccpu, XTAL(12'000'000));
	m_pmccpu->set_addrmap(AS_PROGRAM, &fanucs15_state::pmccpu_mem);
	m_pmccpu->set_disable();

	M68000(config, m_gfxcpu, XTAL(10'000'000));      // wants bit 15 of 70500 to be set
	m_gfxcpu->set_addrmap(AS_PROGRAM, &fanucs15_state::gfxcpu_mem);
	m_gfxcpu->set_disable();

	I80286(config, m_convcpu, XTAL(8'000'000));      // wants 70500 to return 0x8000 (same as what gfxcpu looks for, basically)
	m_convcpu->set_addrmap(AS_PROGRAM, &fanucs15_state::convcpu_mem);
}

/* ROM definition */
ROM_START( fanucs15 )
	ROM_REGION16_BE( 0x50000, "base1a", 0 ) // 68000 sub CPU code and data on base 1 board (verified)
	ROM_LOAD16_BYTE( "4040_001e_001.23m", 0x000001, 0x020000, CRC(2e12109f) SHA1(83ed846d3d59ab0d81b2e2e2231d1a444e462590) )
	ROM_LOAD16_BYTE( "4040_002e_002.27m", 0x000000, 0x020000, CRC(a5469692) SHA1(31c44edb36fb69d3d418a97e32e4a2769d1ec9e7) )
	ROM_LOAD16_BYTE( "9030_001e_381.18j", 0x040001, 0x008000, CRC(9f10a022) SHA1(dc4a242f7611143cc2d9564993fd5fa52f0ac13a) )

	ROM_REGION32_BE( 0x180000, "base1b", 0 )    // 68020 main CPU code and data on base 1 board (verified)
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

	ROM_REGION16_LE( 0x80000, "conversational", 0 ) // 80286 ROMs, 5a00 for Mill
	ROM_LOAD16_BYTE( "5a00_041b.041", 0x000000, 0x020000, CRC(bf22c0a3) SHA1(56ab70bfd5794cb4db1d87c8becf7af522687564) )
	ROM_LOAD16_BYTE( "5a00_042b.042", 0x000001, 0x020000, CRC(7abc9d6b) SHA1(5cb6ad08ce93aa99391d1ce46ac8db8ba2e0f94a) )
	ROM_LOAD16_BYTE( "5a00_001b.001", 0x040000, 0x020000, CRC(cd2a2839) SHA1(bc20fd9ae9d071e1df835244aea85648d1bd1dbc) )
	ROM_LOAD16_BYTE( "5a00_002b.002", 0x040001, 0x020000, CRC(d9ebbb4a) SHA1(9c3d96e9b88848472210beacdf9d300ddd42d16e) )

	ROM_REGION16_BE( 0x50000, "gfxboard", 0 )   // graphics board 68000 code/data.  600a for lathe, 600b for mill, 6082 common to both
	ROM_LOAD16_BYTE( "600b_001d_low.3l",  0x000001, 0x010000, CRC(50566c5c) SHA1(966c8d90d09a9c50c5dedebe9c67f1755846b234) )
	ROM_LOAD16_BYTE( "600b_002d_high.3j", 0x000000, 0x010000, CRC(304e1ecb) SHA1(1e4b149b306550750fc03bd80bd399f239f68657) )
	ROM_LOAD16_BYTE( "600a_001c.bin",     0x020001, 0x010000, CRC(63d9fc2f) SHA1(280e825ba7b79e7c38282a4f4b762d2219fd873b) )
	ROM_LOAD16_BYTE( "600a_002c.bin",     0x020000, 0x010000, CRC(4d78e702) SHA1(a89bd07dc1ae030bdee5a541777825eaadbc2307) )
	// font or tilemap data?
	ROM_LOAD( "6082_001a_cg.12b", 0x040000, 0x010000, CRC(f3d10cf9) SHA1(bc5bc88dcb5f347e1442aa4a0897747958a53413) )

	ROM_REGION16_BE( 0x40000, "cassette", 0 )   // "PMC Cassette C", ROM cartridge that plugs into the Base 1 board
	ROM_LOAD16_BYTE( "pmc_high.a2",  0x000000, 0x020000, CRC(7b8f9a96) SHA1(08d828b612c45bb3f2f7a56df418cd8e34731bf4) )
	ROM_LOAD16_BYTE( "pmc_low.a1",   0x000001, 0x020000, CRC(3ab261f8) SHA1(20b7eef96deb91a3a867f9ac4165b0c188fbcff3) )
ROM_END

} // anonymous namespace


/* Driver */
//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY  FULLNAME     FLAGS
COMP( 1990, fanucs15, 0,      0,      fanucs15, fanucs15, fanucs15_state, empty_init, "Fanuc", "System 15", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
