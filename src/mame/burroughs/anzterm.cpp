// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**************************************************************************

    Burroughs EF315-I220 bank teller terminal

    These 8-bit terminals were used by banks before they were displaced
    by cheap PCs.  There's an external customer PIN pad with its own
    ROMs, and a keyboard for the teller.  Unfortunately the teller
    keyboard was missing from the unit that was dumped.  These terminals
    contain a hardware DES chip which was a controlled item due to trade
    restrictions on encryption technology.

    The main system consists of three large boards in a stack: D8302
    (I/O board), A8301 (memory board), and C8209 (CPU board).

    The MICR/OCR unit appears to be an off-the-shelf unit built by
    Recognition Equipment and has a single board mounted in the top
    case.  Its main CPU is a MOSTEK MK3870/42 which has an undumped
    4032-byte internal mask ROM.

    Onboard peripherals:
    - Small "green-screen" CRT display mounted on an adjustable "stalk"
    - Thermal printer
    - Card swipe
    - MICR/OCR for reading cheque numbers (swiped the same way as cards)

    Rear panel connectors:
    - J1 (25-pin D-sub male)
    - J2 (25-pin D-sub male)
    - J3 (7-pin MIL-C-5015 connector)
    - J4 (5-pin 180-degree DIN female)
    - J5 (25-pin D-sub male)
    - J6 (15-pin D-sub female)

    TODO:
    - Check designation of MICR board.
    - Does U2 on the MICR board exist?  I can't find it on the overlay.
    - Finish parts list and ASCII art for CPU board.
    - Do IC50 and IC56 on the I/O board exist?  I can't find them on the overlay.
    - Work out how clocks are derived.
    - Hook everything up.
    - Emulate DES engine.
    - Photograph the guts of the PIN pad and check ROM labels.
    - Find way to dump MK3870/42 in MICR reader.

****************************************************************************/

/*

(Despite my ASCII art, the main boards are actually all the same size.)

C8209 CPU Board
==================

Components:
  IC1                 - SN75189N (quad EIA-232-F line receiver)
  IC3                 - unpopulated (16-pin DIP)
  IC7                 - SN75174NG (quad EIA-422-B differential line driver)
  IC8                 - SN75175N (quad EIA-422-B differential line receiver)
  IC9                 - MC6850P (Motorola ACIA)
  IC12                - SN75189N (quad EIA-232-F line receiver)
  IC13                - SN75188N (quad EIA-232-E line driver)
  IC14, IC15          - unpopulated (16-pin DIP)
  IC16                - MC6854P (Motorola ADLC)
  IC17                - MC6850P (Motorola ACIA)
  IC18                - HD46850P (Hitachi ACIA clone)
  IC19                - MC6854P (Motorola ADLC)
  IC20                - unpopulated (24-pin DIP)
  IC21                - SN74LS74AN
  IC22                - SN74LS51N
  IC23                - SN75189N (quad EIA-232-F line receiver)
  IC24                - unpopulated (16-pin DIP)
  IC25                - SN74LS08N
  IC26                - SN74LS00N
  IC27                - DM74LS04N
  IC28                - SN74LS367AN
  IC29                - SN74LS161AN
  IC30, IC31, IC32    - SN74LS367AN
  IC33                - SN74LS74AN
  IC34                - SN74LS393N
  IC36                - SN74LS367AN
  IC37                - SN74LS20N
  IC38                - SN74LS74AN
  IC39                - P8214 (Intel Priority Interrupt Control Unit)
  IC42                - SN74LS245N
  IC45                - SN74LS10N
  IC46                - SN74LS86N
  IC48                - 74LS05N
  IC50                - WD200IAH-20 (DES encryption engine)
  IC51                - DM74LS04N
  IC52                - CD4093BE
  IC53                - SN74LS393N
  IC54                - SN74LS74AN
  IC55                - SN74LS00AN
  IC56                - SN74LS367AN
  IC57                - SN74LS132AN
  IC58                - SN74LS32N
  IC60                - unpopulated (16-pin DIP)
  J7                  - System bus (50-pin DIL IDC)
  J8                  - Power (24-pin SIL)


A8301 Memory Board
==================

+------------------------------+------------J1------------+---+                    +---+
|                              +--------------------------+   +---------J2---------+   |
|         [=IC9==]  [=IC19=]                                  +--------------------+   |
|                                                                                      |
| +--J-IC8---+  +--A-IC18--+   [=IC25=] [=IC29=] [=IC34=] [=RP12=]                     |
| +----------+  +----------+                                        Q5 Q6       Q7     |
|                             +--IC23---+                                              |
| +--K-IC7---+  +--B-IC17--+  +---------+    [=IC28==]   [=IC42==]                     |
| +----------+  +----------+                                        [=IC55=] [=IC66=]  |
|                             [=IC22=]  [=RP10=] [=IC33=] [=RP11=]                     |
| +--L-IC6---+  +--C-IC16--+                                        [=IC54=] [=IC65=]  |
| +----------+  +----------+                                                           |
|                             [=HP3==]   [=IC27=]       [=IC41=]    [=IC53=] [=RP14=]  |
| +--M-IC5---+  +--D-IC15--+                                                           |
| +----------+  +----------+  [=HP2==]                             [=IC52=] [=IC64=]   |
|                                                       [=IC40=]                       |
| +--N-IC4--+ +--E-IC14-+  +---HP1---+   [=IC26=]                  [=IC51=] [=IC63=]   |
| +---------+ +---------+  +---------+                  [=IC39==]  [=IC50=] [=IC62=]   |
|                         +-----+                                  [=IC49=] [=IC61=]   |
| +--P-IC3--+ +--F-IC13-+ |     |   [=IC21=]   [=IC32=] [=IC38=]                       |
| +---------+ +---------+ | B1  |                                  [=IC48=] [=IC60=]   |
|                         | B2  |             [=IC31=]  [=IC37=]   [=IC47=] [=IC59=]   |
| +--Q-IC2--+ +--G-IC12-+ | B3  |       Q4                         [=IC46=] [=IC58=]   |
| +---------+ +---------+ | B4  |    Q2       [=IC30=]  [=IC36=]   [=IC45=] [=IC57=]   |
|                         | B5  |       Q3                                             |
| +--R-IC1--+ +--H-IC11-+ |     |    Q1                 [=IC35=]   [=IC44=] [=IC56=]   |
| +---------+ +---------+ +-----+                                                      |
|         [=RP2=]  [=IC10=]                                        [=IC43=]            |
+--------------------------------------------------------------------------------------+

Components:
  B1, B2, B3, B4, B5  - space provided for various NiCd batteries - B4 is populated (3.6V 100mAh)
  HP1                 - 24-pin DIP wired links - 1/2/16/22/24 3/4/5 6 7/8/9/10 11/12 13/14 15/17/18 19/20 21 23
  HP2                 - 16-pin DIP wired links - 1/2/3/16 4/6/7/8/14 5/9/10/12 11/13/15
  HP3                 - 16-pin DIP wired links - 1/4 2 3 5/12 6/11 7/10 8/9 13 14 15 16
  IC1, IC2            - HM6116LP-3 (2048*8 CMOS SRAM)
  IC3, IC4            - unpopulated (24-pin DIP socket)
  IC5, IC6, IC7, IC8  - HN482764G (8192*8 UV EPROM)
  IC9                 - HM6147LP-3 (4096*1 CMOS SRAM; 18-pin DIP in 20-pin socket; overlay says "CMOS PARITY 1")
  IC10                - DM74LS156N
  IC11, IC12          - unpopulated (24-pin DIP socket)
  IC13, IC14          - TMS2516JL-45 (2048*8 UV EPROM)
  IC15                - unpopulated (28-pin DIP socket)
  IC16, IC17, IC18    - HN482764G (8192*8 UV EPROM)
  IC19                - unpopulated (20-pin DIP socket; overlay says "CMOS PARITY 2")
  IC20                - location for 14-pin DIP integrated transistor pack replacing Q1/Q2/Q3/Q4
  IC21                - SN74LS03N
  IC22                - DM74LS156N
  IC23                - TBP28S166N (2048*8 BPROM)
  IC24                - location for 16-pin DIP under IC23 (for smaller address decoding ROM/PLD?)
  IC25                - SN74LS367AN
  IC26                - DM74LS156N
  IC27                - SN74LS157N
  IC28                - SN74LS273N
  IC29                - SN74LS367AN
  IC30                - SN74LS139N
  IC31                - SN74LS32N
  IC32                - SN74LS20N
  IC33                - SN74LS280N
  IC34                - SN74LS367AN
  IC35                - SN74LS32N
  IC36                - SN74LS13N
  IC37                - SN74LS163AN
  IC38                - SN74S74N
  IC39                - BELFUSE 8331 0447-0439-99 (DRAM timing device?)
  IC40                - SN74LS132N
  IC41                - SN74S02N
  IC42                - SN74LS245N
  IC43                - SN74S08N
  IC44 - IC52         - HM4864P-2 (65536*1 DRAM; DRAM BANK 1 BIT 0 - 8 and PARITY, respectively)
  IC53, IC54          - SN74LS158N
  IC55                - SN74LS393N
  IC56 - IC64         - HM4864P-2 (65536*1 DRAM; DRAM BANK 2 BIT 0 - 8 and PARITY, respectively)
  IC65, IC66          - SN74LS158N
  J1                  - System bus (50-pin DIL IDC)
  J2                  - Power (24-pin SIL)


D8302 I/O Board
==================

+----------------------------+--------------J5--------------+----+                        +----+
|                            +------------------------------+    +-----------J7-----------+    |
|      J4 |                                                      +------------------------+    |
|         |                                                                                    |
|  J1 |   |     [=IC18=]     [=RP2=]                                                           |
|     |                    [==IC26=]                                                           |
|      J3 |              +---IC25---+                                                          |
| [=IC7=] |    [=IC17=]  +----------+             [=IC45=]                                     |
|                                                 [=IC44=]                                     |
| +---IC6---+    +---IC16--+                                                                   |
| +---------+    +---------+            [==IC34=] [=IC43=]                                     |
|                                                                                              |
| [==IC5==]     +-------IC15-------+                                                          ++
|               +------------------+    [=IC33=] [=IC42=]  Q12  [=IC51=]  Q13                 ||
| [==IC4==]                                                                                   ||
|           [=IC11=]         [=IC24=]            [=IC41=]                                     ||
| [==IC3==]                            [==IC32=]                                              ||
|           [=IC10=] [=IC14=] [=IC23=]           [=IC40=]       [=IC62=]   [=IC61=]        J9 ||
| [==IC2==]                             [==IC31=]                                             ||
|                                                 +-------IC39-------+     [=IC55=]           ||
| [==IC1==]                    [=IC22=] [==IC30=] +------------------+                        ||
|          ++                            [=IC29=]  [==IC38=]    [=IC49=]                      ++
|          J2                                                              [=IC54=]  [=IC60=]  |
|          ||                   [=IC21=]     IC28 +---IC37--+                                  |
|          ++                                     +---------+   [=IC48=]   [=IC53=]  [=IC59=]  |
|              J6                        [=IC27=]   [=IC36=] [=IC63=] [=IC47=]       [=IC58=]  |
|     [=IC9==] |  [=IC13=]  [=IC20=]+-X1-+                                                     |
|     [=IC8==] |  [=IC12=]  [=IC19=]+----+ [J10]    [=IC35=] [=IC46=] [=IC52=] [=IC57=]        |
+----------------------------------------------------------------------------------------------+

Components:
  IC1, IC2            - SN74LS273N
  IC3, IC4            - SN74LS245N
  IC5                 - SN74LS273N
  IC6                 - D2716D (2048*8 UV EPROM)
  IC7                 - DM7404N
  IC8                 - SN74LS04N
  IC9                 - SN74LS74AN
  IC10                - SN74LS157N
  IC11                - SN74LS166AN
  IC12                - SN74LS21N
  IC13                - SN74LS93N
  IC14                - SN74LS157N
  IC15                - HD46505SP-1 HD68A45SP (Hitachi CRTC clone)
  IC16                - HD46850P HD6850P (Hitachi ACIA clone)
  IC17                - MC1488 75188N (quad EIA-232-E line driver)
  IC18                - SN74LS86N
  IC19                - SN74LS08N
  IC20                - SN74LS93N
  IC21                - SN74LS132N
  IC22                - SN74LS08N
  IC23                - MC1489A SN75189AN (quad EIA-232-F line receiver)
  IC24                - SN74LS157N
  IC25                - D8251AC (NEC Programmable Communications Interface)
  IC26                - SN74LS245N
  IC27                - SN7406N
  IC28                - NE555P
  IC29                - DM74LS00N
  IC30, IC31, IC32    - MCM2114P20 (1024*4 SRAM)
  IC33                - SN74LS157N
  IC34                - SN74LS273N
  IC35                - SN74LS27N
  IC36                - 74LS168
  IC37                - D2716D (2048*8 UV EPROM)
  IC38                - SN74LS273N
  IC39                - HD46821P HD6821P (Hitachi PIA clone)
  IC40                - SN74LS74AN
  IC41, IC42, IC43    - 74LS32
  IC44                - SN74LS04N
  IC45                - SN74LS367AN
  IC47                - SN74LS10N
  IC48                - SN74LS74AN
  IC49                - 74LS03
  IC51                - 74LS138N
  IC52                - 74LS02
  IC53                - 74LS00N
  IC54                - SN74121N
  IC55                - SN74LS09N
  IC57, IC58          - SN74LS74AN
  IC59                - SN74LS14N
  IC60                - 74LS132N
  IC61                - SN74LS09N
  IC62                - SN74LS08N
  IC63                - 74LS03
  J1                  - Card Swipe (6-pin SIL)
  J2                  - MICR Reader Board (10-pin DIL IDC)
  J3                  - not populated (6-pin SIL)
  J4                  - CRT (8-pin SIL)
  J5                  - System bus (50-pin DIL IDC)
  J6                  - External connector on left side of case (6-pin SIL)
  J7                  - Power (24-pin SIL)
  J9                  - Printer (30-pin double-sided female edge connector)
  J10                 - Possibly service mode - case contains a loose connector for shorting two pins (6-pin SIL)
  X1                  - CK1100AC 15974.40KHz (oscillator module)


MICR Board
==================

+-------------------------------------------------------------+
|                            +---K1---+                       |
|               +---U15----+ +--------+                       |
|  Q5   [=U23=] +----------+                       [=U1==]    |
|  Q6                                                         |
|               +---U16----+  Q8                              |
|  Q7   [=U24=] +----------+     Q10 Q3                       |
|                                                             |
|               +---U17----+ +---U11---+   +---U7----+        |
|       [=U25=] +----------+ +---------+   +---------+  U4    |
|                                                             |
|  ++   [=U26=] +---U18----+                                  |
|  ||           +----------+     [=U12=]   +-----U5------+    |
|  ||                           JP5        +-------------+ U3 |
|  ||   [=U27=] +---U19----+                            ++    |
|  ||           +----------+                            U6    |
|  ||                                      +---U8----+  ++    |
|  ++   [=U28=] +---U20----+               +---------+        |
|  Q9   [=JP==] +----------+                              ++  |
|                                                         ||  |
|       +---U29---+            +----+    Q2    Q1         ||  |
|       +---------+            +U13 |     U10             ||  |
|                       Q4     +----+                     ||  |
|        [=U31=]  [=U21=]                                 ||  |
|                         +-----U14-----+    [=U9==]      ++  |
|        [=U30=]  [=U22=] +-------------+                     |
+-------------------------------------------------------------+

Components:
  JP                  - Jumpers JP1, JP2, JP3, JP4
  K1                  - SPDT Relay
  U1                  - TL497ACN (switching voltage regulator)
  U3, U4              - DS0026CN (two-phase MOS clock driver)
  U5                  - CF11000A
  U6                  - CXO-043D 36.0000MHz 4E (oscillator module)
  U7                  - AMI 8336CK 5020056 CO4423
  U8                  - 5020884-002 853C
  U9                  - SN7414N
  U10                 - ICL8211CPA (programmable voltage detector)
  U11                 - ET2732Q-3 (4096*8 UV EPROM)
  U12                 - SN74LS00N
  U13                 - MB63303A (Fujitsu mask-programmed gate array)
  U14                 - MK3870/42 (Mostek integrated F8 - 4032 byte program mask ROM, 64 bytes program RAM, 64 byte scratchpad)
  U15                 - AM2716DC (2048*8 UV EPROM; 24-pin DIP in 28-pin socket)
  U16                 - ET2732Q-3 (4096*8 UV EPROM; 24-pin DIP in 28-pin socket)
  U17, U18, U19       - unpopulated (28-pin DIP socket)
  U20                 - ET2732Q-3 (4096*8 UV EPROM; 24-pin DIP in 28-pin socket)
  U21                 - SN74LS26N
  U22                 - SN74LS245N
  U23                 - SN7406N
  U24                 - unpopulated (14-pin DIP socket)
  U25                 - SN74LS08N
  U26, U27            - SN75189AN (quad EIA-232-F line receiver)
  U28                 - SN74LS367AN
  U29                 - TMS2732AJL-45 (4096*8 UV EPROM)
  U30                 - unpopulated (20-pin SDIP socket)
  U31                 - SN74LS374N

*/

#include "emu.h"

#include "cpu/m6809/m6809.h"
#include "machine/6850acia.h"
#include "machine/i8214.h"
#include "machine/mc6854.h"
#include "emupal.h"
#include "screen.h"


namespace {

class anzterm_state : public driver_device
{
public:
	anzterm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{
	}

	void anzterm(machine_config &config);

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
	{
		return 0;
	}
	void anzterm_mem(address_map &map) ATTR_COLD;
};


gfx_layout const screenfont =
{
	8, 16,                                              // 8x16
	RGN_FRAC(1, 1),                                     // whole region
	1,                                                  // 1bpp
	{ 0 },                                              // bitplane offset
	{ 0*1,  1*1,  2*1,  3*1,  4*1,  5*1,  6*1,  7*1 },  // x offsets
	{ 0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
	  8*8,  9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },  // y offsets
	128                                                 // stride
};

gfx_layout const printfont =
{
	8, 8,                                       // 7x8
	RGN_FRAC(1, 1),                             // whole region
	1,                                          // 1bpp
	{ 0 },                                      // bitplane offset
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 }, // x offsets
	{ 7*1, 6*1, 5*1, 4*1, 3*1, 2*1, 1*1, 0*1 }, // y offsets
	64                                          // stride
};

GFXDECODE_START( gfx_anzterm )
	GFXDECODE_ENTRY("crtc", 0x0000, screenfont, 0, 1)
	GFXDECODE_ENTRY("prnt", 0x0000, printfont,  0, 1)
GFXDECODE_END


void anzterm_state::anzterm_mem(address_map &map)
{
	// There are two battery-backed 2kB SRAM chips with a 4kb SRAM chip for parity
	// There are two 64kB DRAM banks (with parity)
	// There's also a whole lot of ROM
	map(0x0000, 0x3fff).ram();
	map(0xe000, 0xffff).rom();
}


void anzterm_state::anzterm(machine_config &config)
{
	m6809_device &maincpu(M6809(config, "maincpu", 15974400/4));
	maincpu.set_addrmap(AS_PROGRAM, &anzterm_state::anzterm_mem);

	I8214(config, "pic.ic39", 0);
	MC6854(config, "adlc.ic16", 0);
	MC6854(config, "adlc.1c19", 0);
	ACIA6850(config, "acia.ic17", 0);
	ACIA6850(config, "acia.ic18", 0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_screen_update(FUNC(anzterm_state::screen_update));
	screen.set_palette("palette");
	screen.set_raw(15974400/4, 1024, 0, 104*8, 260, 0, 24*10); // this is totally wrong, it just stops a validation error

	PALETTE(config, "palette", palette_device::MONOCHROME);

	GFXDECODE(config, "gfxdecode", "palette", gfx_anzterm);
}


INPUT_PORTS_START( anzterm )
INPUT_PORTS_END

} // anonymous namespace


ROM_START( anzterm )
	// Main program on memory board - loading is definitely wrong
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fj-a-25.ic18",                 0xe000, 0x2000, CRC(8c31a9dd) SHA1(864babf6c77813f17ce9082013098e8a677b0af6) )
	ROM_LOAD( "fj-b-25.ic17",                 0x2000, 0x2000, CRC(a893aeaf) SHA1(64e8a935fe37195533a0a19f00bdc4e6a2007728) )
	ROM_LOAD( "fj-c-25.ic16",                 0x4000, 0x2000, CRC(ae678bde) SHA1(b41da540b99a0c3ef9489d9cd25e5fa32e2d13f9) )
	ROM_LOAD( "fj-e-25.ic14",                 0x6000, 0x0800, CRC(786beceb) SHA1(d20b870528d12f8457e3c746b539fcfc3ded3b0b) )
	ROM_LOAD( "fj-f-25.ic13",                 0x6800, 0x0800, CRC(2890d808) SHA1(6871f4c5fd1bc7e2d1db2e663ffb342988de94b7) )
	ROM_LOAD( "fj-j-25.ic8",                  0x8000, 0x2000, CRC(23fa4b36) SHA1(b3676579b2ea4efb0bf867557b53a6ccba7cc60f) )
	ROM_LOAD( "fj-k-25.ic7",                  0xa000, 0x2000, CRC(cbd17462) SHA1(7e7460f99e7dd5c9ae113f69b67e2b6079a57c6d) )
	ROM_LOAD( "fj-l-25.ic6",                  0xc000, 0x2000, CRC(8989c2ed) SHA1(912d8152f8f67a964dcd360151d8c8438a652d58) )
	ROM_LOAD( "fj-m-25.ic5",                  0x0000, 0x2000, CRC(82762fee) SHA1(234a438abab91936e7073bd7cc62414dfae10373) )

	// BPROM on memory board - address decoding?
	ROM_REGION( 0x0800, "prom", 0 )
	ROM_LOAD( "08177-80002.ic23",             0x0000, 0x0800, CRC(3cec2386) SHA1(a1ae5e07756eac5abbb3e178e12b213770432b5f) )

	// CRTC character ROM on I/O board
	ROM_REGION( 0x0800, "crtc", 0 )
	ROM_LOAD( "crt-5080-2.ic6",               0x0000, 0x0800, CRC(cdea8532) SHA1(683743e477518695c2a1d9510bee25b7ef3f909b) )

	// Printer font ROM on I/O board
	ROM_REGION( 0x0800, "prnt", 0 )
	ROM_LOAD( "prt-04.ic37",                  0x0000, 0x0800, CRC(68870564) SHA1(06819a633dc545f103e8b843a2f553ac46a16a05) )

	// ROMs in PIN pad
	ROM_REGION( 0x1000, "pinpad", 0 )
	ROM_LOAD( "ck-a-pr01.ic4",                0x0000, 0x0800, CRC(d0981882) SHA1(b55fd313c9b3e00039501a53a53c820d98f2258a) )
	ROM_LOAD( "ck-b-pr01.ic3",                0x0000, 0x0800, CRC(96c9d90d) SHA1(400980c7a2c5306be28b74284c626ef2ed24c1a5) )

	// Undumped microcontroller ROM in MICR reader
	ROM_REGION( 0x0fc0, "micrmcu", 0 )
	ROM_LOAD( "mk3870.u14",                   0x0000, 0x0fc0, NO_DUMP )

	// MICR reader data table ROMS, no idea how this stuff is used but dumps should be preserved
	ROM_REGION( 0x5000, "micrdata", 0 )
	ROM_LOAD( "cdn1-ebb.u20",                 0x0000, 0x1000, CRC(0f9a9db3) SHA1(aedfe3ba7afb1d0a827fec5418369fca9348940f) )
	ROM_LOAD( "cdn2-ebb.u16",                 0x1000, 0x1000, CRC(648fff69) SHA1(59653d34067d9a3061857507868fd2147dadf537) )
	ROM_LOAD( "6047204005.u15",               0x2000, 0x0800, CRC(70bfac37) SHA1(84081249ead5b957d98b3bd06665ef52d0a0243c) )
	ROM_LOAD( "6048225001.u29",               0x3000, 0x1000, CRC(59c73999) SHA1(7dd12b500e13b177d19a24d148310541f7e660b4) )
	ROM_LOAD( "ebb-fea-v96-9-23-83-f43a.u11", 0x4000, 0x1000, CRC(0e572470) SHA1(966e5eeb0114589a7cab3c29a1db48cdd8634be5) )
ROM_END

COMP( 1986?, anzterm, 0, 0, anzterm, anzterm, anzterm_state, empty_init, "Burroughs", "EF315-I220 Teller Terminal (ANZ)", MACHINE_IS_SKELETON ) // year comes from sticker on bottom of case, it's more likely a 1983 revision
