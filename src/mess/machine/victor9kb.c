// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Victor 9000 keyboard emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

/*

Keyboard PCB Layout
----------

Marking on PCB back: A65-02307-201D 007

|------------------------------------------------------------------------------------|
| 22-908-03 22-950-3B     XTAL 8021  74LS14     [804x]           [EPROM]  [???] L CN1=___
|         X       X       X       X       X       X        X      X   X      X      X    |
| X    X   X   X   X   X   X   X   X   X   X   X   X   X    X     X   X    X   X   X   X |
| X     X   X   X   X   X   X   X   X   X   X   X   X   X    X    X   X    X   X   X   X |
| X     X    X   X   X   X   X   X   X   X   X   X   X   X    X   X   X    X   X   X   X |
| X    X   X  X   X   X   X   X   X   X   X   X   X       X       X   X    X   X   X   X |
| X     X    marking             X                 X              X   X    X   X   X   X |
|----------------------------------------------------------------------------------------|


Notes:
    All IC's shown.
    XTAL        - 3.579545Mhz Crystal, marked "48-300-010" (front of xtal) and "3.579545Mhz" (back of xtal)
    74LS14      - Z4 - 74LS14 Hex inverter with Schmitt-trigger inputs (0.8v hysteresis)
    8021        - Z3 - Intel 8021 MCU, marked: "P8021 2137 // 8227 // 20-8021-139 // (C) INTEL 77"
    22-908-03   - Z2 - Exar Semiconductor XR22-008-03 keyboard matrix capacitive readout latch
    22-950-3B   - Z1 - Exar Semiconductor XR22-050-3B keyboard matrix row driver with 4 to 12 decoder/demultiplexer
    CN1 or J1   - J1 - keyboard data connector (SIP, 7 pins, right angle)
    L           - L1 & L2 - mil-spec 22uH 10% inductors (double wide silver band(mil spec), red(2) red(2) black(x1uH) silver(10%))

    [804x]      - unpopulated space for a 40 pin 804x or 803x MCU
    [EPROM]     - unpopulated space for an EPROM, if a ROMless 803x MCU was used
    [???]       - unpopulated space for an unknown NDIP10 IC or DIP-switch array
    X           - capacitive sensor pad for one key
    marking     - PCB trace marking: "KTC // A65-02307-007 // PCB 201 D"

74LS14 (Hex inverter with Schmitt-trigger inputs (0.8v hysteresis))
------------------
         __   __
   1A 1 |* \_/  | 14 VCC
  /1Y 2 |       | 13 6A
   2A 3 |       | 12 /6Y
  /2Y 4 |       | 11 5A
   3A 5 | 74LS14| 10 /5Y
  /3Y 6 |       | 9  4A
  GND 7 |_______| 8  /4Y


P8021 Pinout
------------------
            _____   _____
   P22   1 |*    \_/     | 28  VCC
   P23   2 |             | 27  P21
  PROG   3 |             | 26  P20
   P00   4 |             | 25  P17
   P01   5 |    P8021    | 24  P16
   P02   6 |             | 23  P15
   P03   7 |             | 22  P14
   P04   8 |             | 21  P13
   P05   9 |             | 20  P12
   P06  10 |             | 19  P11
   P07  11 |             | 18  P10
   ALE  12 |             | 17  RESET
    T1  13 |             | 16  XTAL 2
   VSS  14 |_____________| 15  XTAL 1


XR22-008-03 Pinout (AKA XR22-908-03)
------------------
            _____   _____
    D0   1 |*    \_/     | 20  Vcc
    D1   2 |             | 19  _CLR
    D2   3 |             | 18  Q0
    D3   4 |             | 17  Q1
   HYS   5 |  22-008-03  | 16  Q2
    D4   6 |             | 15  Q3
    D5   7 |             | 14  Q4
    D6   8 |             | 13  Q5
    D7   9 |             | 12  Q6
   GND  10 |_____________| 11  Q7


XR22-050-3B Pinout (AKA XR22-950-3B)
------------------
            _____   _____
    Y8   1 |*    \_/     | 20  Vcc
    Y9   2 |             | 19  Y7
   Y10   3 |             | 18  Y6
   Y11   4 |             | 17  Y5
  _STB   5 |  22-050-3B  | 16  Y4
    A0   6 |             | 15  Y3
    A1   7 |             | 14  Y2
    A2   8 |             | 13  Y1
    A3   9 |             | 12  Y0
   GND  10 |_____________| 11  OE?

i8021 to elsewhere connections
------------------------------
8021 pin - 22-xxx pin
  P22  1 - 74LS14 pin 5 (3A)                            AND (804x chip pin 37, P26)
  P23  2 - 74LS14 pin 11 (5A)                           AND (804x chip pin 38, P27)
 PROG  3 -                                                  (804x chip pin 25, PROG)
  P00  4 - N/C
  P01  5 - N/C
  P02  6 - N/C
  P03  7 - N/C
  P04  8 - N/C
  P05  9 - N/C
  P06 10 - N/C
  P07 11 - N/C
  ALE 12 - N/C
   T1 13 - 74LS14 pin 2 (/1Y)                           AND (804x chip pin 39, T1)
  VSS 14 - GND
XTAL1 15 - XTAL
XTAL2 16 - XTAL
RESET 17 - 10uf capacitor - VCC
  P10 18 - 22-908 pin 18 (Q0) AND 22-950 pin 6 (A0)     AND (804x chip pin 27, P10)
  P11 19 - 22-908 pin 17 (Q1) AND 22-950 pin 7 (A1)     AND (804x chip pin 28, P11)
  P12 20 - 22-908 pin 16 (Q2) AND 22-950 pin 8 (A2)     AND (804x chip pin 29, P12)
  P13 21 - 22-908 pin 15 (Q3) AND 22-950 pin 9 (A3)     AND (804x chip pin 30, P13)
  P14 22 - 22-908 pin 14 (Q4)                           AND (804x chip pin 31, P14)
  P15 23 - 22-908 pin 13 (Q5)                           AND (804x chip pin 32, P15)
  P16 24 - 22-908 pin 12 (Q6)                           AND (804x chip pin 33, P16)
  P17 25 - 22-908 pin 11 (Q7)                           AND (804x chip pin 34, P17)
  P20 26 - 22-908 pin 19 (/CLR) AND 22-950 pin 5 (/STB) AND (804x chip pin 35, P24)
  P21 27 - 74LS14 pin 9 (4A)                            AND (804x chip pin 36, P25)
  VCC 28 - VCC

74LS14 to elsewhere connections
-------------------------------
  1A  1 - 100 Ohm resistor - J1 pin 4
 /1Y  2 - 8021 pin 13 (T1)
  2A  3 - 74LS14 pin 8
 /2Y  4 - 22uH 10% inductor 'L1' - J1 pin 5
  3A  5 - 8021 pin 1 (P22)
 /3Y  6 - many keyboard contact pads (!)
 GND  7 - GND
 /4Y  8 - 74LS14 pin 3
  4A  9 - 8021 pin 27 (P21)
 /5Y 10 - 74LS14 pin 13
  5A 11 - 8021 pin 2 (P23)
 /6Y 12 - 22uH 10% inductor 'L2' - J1 pin 6
  6A 13 - 74LS14 pin 10
 VCC 14 - VCC


J1 (aka CN1) connections:
-------------------------
1 - VCC
2 - GND
3 - GND
4 - 100 Ohm 5% resistor -> 74LS14 pin 1 -> inverted 74LS14 pin 2 -> inverted 8021 pin 13 (T1)
5 - 22uH 10% inductor 'L1' <- 74LS14 pin 4 <- inverted 74LS14 pin 3 <- inverted 74LS14 pin 8 <- 74LS14 pin 9 <- 8021 pin 27 (P21)
6 - 22uH 10% inductor 'L2' <- 74LS14 pin 12 <- inverted 74LS14 pin 13 <- inverted 74LS14 pin 10 <- 74LS14 pin 11 <- 8021 pin 2 (P23)
7 - VCC

Pins 5,and 6 also have a .0047uF capacitor to GND (forming some sort of LC filter)
There is another .0047uF capacitor to ground from the connection between 74LS14 pin 1 and the 100 Ohm 5% resistor which connects to pin 4 (forming some sort of RC filter)

Exar Custom connections between each other:
-------------------------------------------
22-950 pin 11 (OE?) - 68KOhm 5% resistor - 22-908 pin 5 (HYS)

Cable connecting J1/CN1 to the Victor:
--------------------------------------
RJ45 when looking at end of plug (not socket!)
.||||||||.
|87654321|
|__----__|
   |__|

RJ45 pins   Wire Color   Connection
1           White        J1 pin 1 (VCC)
2           Yellow       J1 pin 2 (GND)
3           Green        J1 pin 3 (GND)
4           Orange       J1 pin 4 (?KBACK?)
5           Blue         J1 pin 5 (?KBRDY?)
6           Red          J1 pin 6 (?KBDATA?)
7           Black        J1 pin 7 (VCC)
8           shield/bare  Keyboard Frame Ground


Key Layout (USA Variant): (the S0x markings appear on the back of the PCB)
|------------------------------------------------------------------------------------|
| 22-908-03 22-950-3B     XTAL 8021  74LS14     [804x]           [EPROM]  [???] L CN1=___
|         01      02      03      04      05      06       07     08  09     10     11   |
| 12   13  14  15  16  17  18  19  20  21  22  23  24  25   26    27  28   29  30  31 32 |
| 33    34  35  36  37  38  39  40  41  42  43  44  45  46   47   48  49   50  51  52 53 |
| 54    55   56  57  58  59  60  61  62  63  64  65  66  67   68  69  70   71  72  73 74 |
| 75   76  77 78  79  80  81  82  83  84  85  86  87      88      89  90   91  92  93 94 |
| 95    96   marking             97                98             99 100  101 102 103 104|
|----------------------------------------------------------------------------------------|

   key - Shifted(top)/Unshifted(bottom)/Alt(front) (if no slashes in description assume key has just one symbol on it)
   S01 - [1]
   S02 - [2]
   S03 - [3]
   S04 - [4]
   S05 - [5]
   S06 - [6]
   S07 - [7]
   S08 - [8]
   S09 - UNUSED (under the [8] key, no metal contact on key)
   S10 - [9]
   S11 - [10]

   S12 - CLR/HOME
   S13 - (Degree symbol U+00B0)/(+- symbol U+00B1)/(Pi symbol U+03C0)
   S14 - !/1/|
   S15 - @/2/<
   S16 - #/3/>
   S17 - $/4/(centered closed dot U+00B7)
   S18 - %/5/(up arrow symbol U+2191)
   S19 - (cent symbol U+00A2)/6/(logical not symbol U+00AC)
   S20 - &/7/^
   S21 - * /8/`
   S22 - (/9/{
   S23 - )/0/}
   S24 - _/-/~
   S25 - +/=/\
   S26 - BACKSPACE
   S27 - INS
   S28 - DEL
   S29 - MODE CALC/= (white keypad key)
   S30 - % (white keypad key)
   S31 - (division symbol U+00F7) (white keypad key)
   S32 - (multiplication symbol U+00D7) (white keypad key)

   S33 - (up arrow, SCRL, down arrow)//VTAB
   S34 - TAB//BACK
   S35 - Q
   S36 - W
   S37 - E
   S38 - R
   S39 - T
   S40 - Y
   S41 - U
   S42 - I
   S43 - O
   S44 - P
   S45 - (1/4 symbol U+00BC)/(1/2 symbol U+00BD)
   S46 - [/]
   S47 - UNUSED (under the RETURN key, no metal contact on key)
   S48 - ERASE/EOL
   S49 - REQ/CAN
   S50 - 7 (white keypad key)
   S51 - 8 (white keypad key)
   S52 - 9 (white keypad key)
   S53 - - (white keypad key)

   S54 - (OFF,RVS,ON)//ESC
   S55 - LOCK//CAPS LOCK
   S56 - A
   S57 - S
   S58 - D
   S59 - F
   S60 - G
   S61 - H
   S62 - J
   S63 - K
   S64 - L
   S65 - :/;
   S66 - "/'
   S67 - UNUSED (under the RETURN key, no metal contact on key)
   S68 - RETURN
   S69 - WORD/(left arrow U+2190)/(volume up U+1F508 plus U+25B4) (i.e. 'Previous Word')
   S70 - WORD/(right arrow U+2192)/(volume down U+1F508 plus U+25BE) (i.e. 'Next Word')
   S71 - 4 (white keypad key)
   S72 - 5 (white keypad key)
   S73 - 6 (white keypad key)
   S74 - + (white keypad key)

   S75 - (OFF,UNDL,ON)
   S76 - SHIFT (left shift)
   S77 - UNUSED (under the left SHIFT key, no metal contact on key)
   S78 - Z
   S79 - X
   S80 - C
   S81 - V
   S82 - B
   S83 - N
   S84 - M
   S85 - ,/, (yes, both are comma)
   S86 - ./. (yes, both are period/fullstop)
   S87 - ?// (this is the actual / key)
   S88 - SHIFT (right shift)
   S89 - (up arrow U+2191)//(brightness up U+263C plus U+25B4)
   S90 - (down arrow U+2193)//(brightness down U+263C plus U+25BE)
   S91 - 1 (white keypad key)
   S92 - 2 (white keypad key)
   S93 - 3 (white keypad key)
   S94 - ENTER (white keypad key)

   S95 - RPT
   S96 - ALT
   S97 - (spacebar)
   S98 - PAUSE/CONT
   S99 - (left arrow U+2190)//(contrast up U+25D0 plus U+25B4) (U+1F313 can be used in place of U+25D0)
   S100 - (right arrow U+2192)//(contrast down U+25D0 plus U+25BE) ''
   S101 - 0 (white keypad key)
   S102 - 00 (white keypad key) ('double zero')
   S103 - . (white keypad key)
   S104 - UNUSED (under the ENTER (keypad) key, no metal contact on key)

   Note that the five unused key contacts:
   S09, S47, S67, S77 and S104
   may be used on international variants of the Victor 9000/Sirius 1 Keyboard.

Keyboard Matrix
---------------
Row select
|         Columns Sxxx by bit                 Columns Key by bit
|         D0  D1  D2  D3  D4  D5  D6  D7      D0   D1   D2   D3   D4   D5   D6   D7
V         V   V   V   V   V   V   V   V       V    V    V    V    V    V    V    V
Y0        12  13  33  34  54  55  75  76      CLRH DEGR SCRL TAB  RVS  LOCK UNDL LSHFT
Y1        14  15  35  36  56  57  77  78      1    2    Q    W    A    S    N/A  Z
Y2        16  17  37  38  58  59  79  80      3    4    E    R    D    F    X    C
Y3        18  19  39  40  60  61  81  82      5    6    T    Y    G    H    V    B
Y4        20  21  41  42  62  63  83  84      7    8    U    I    J    K    N    M
Y5        22  23  43  44  64  65  85  86      9    0    O    P    L    ;    ,    .
Y6        24  25  45  46  66  67  87  88      -    =    1/2  []   '    N/A  ?    RSHFT
Y7        26  27  47  48  68  69  89  90      BKSP INS  N/A  ERSE RETN WRDL UP   DOWN
Y8        28  29  49  50  70  71  91  92      DEL  MODE REQ  k7   WRDR k4   k1   k2
Y9        30  31  51  52  72  73  93  94      k%   kDIV k8   k9   k5   k6   k3   kENTR
Y10       32  11  53  10  74  09  07  06      kMUL [10] k-   [9]  k+   N/A  [7]  [6]
Y11       104 103 102 101 100 99  98  97      kN/A k.   k00  k0   RGHT LEFT PAUS SPACE
/3Y       05  04  03  02  01  08  95  96      [5]  [4]  [3]  [2]  [1]  [8]  RPT  ALT

*/

#include "victor9kb.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8021_TAG   "z3"

#define LOG 		0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VICTOR9K_KEYBOARD = &device_creator<victor_9000_keyboard_t>;


//-------------------------------------------------
//  ROM( victor9k_keyboard )
//-------------------------------------------------

ROM_START( victor9k_keyboard )
	ROM_REGION( 0x400, I8021_TAG, 0)
	ROM_LOAD( "20-8021-139.z3", 0x000, 0x400, CRC(0fe9d53d) SHA1(61d92ba90f98f8978bbd9303c1ac3134cde8cdcb) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *victor_9000_keyboard_t::device_rom_region() const
{
	return ROM_NAME( victor9k_keyboard );
}


//-------------------------------------------------
//  ADDRESS_MAP( kb_io )
//-------------------------------------------------

static ADDRESS_MAP_START( victor9k_keyboard_io, AS_IO, 8, victor_9000_keyboard_t )
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READWRITE(kb_p1_r, kb_p1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(kb_p2_w)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(kb_t1_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( victor9k_keyboard )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( victor9k_keyboard )
	MCFG_CPU_ADD(I8021_TAG, I8021, XTAL_3_579545MHz)
	MCFG_CPU_IO_MAP(victor9k_keyboard_io)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor victor_9000_keyboard_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( victor9k_keyboard );
}


//-------------------------------------------------
//  INPUT_PORTS( victor9k_keyboard )
//-------------------------------------------------

INPUT_PORTS_START( victor9k_keyboard )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("YA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("YB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("YC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor victor_9000_keyboard_t::device_input_ports() const
{
	return INPUT_PORTS_NAME( victor9k_keyboard );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  victor_9000_keyboard_t - constructor
//-------------------------------------------------

victor_9000_keyboard_t::victor_9000_keyboard_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, VICTOR9K_KEYBOARD, "Victor 9000 Keyboard", tag, owner, clock, "victor9kb", __FILE__),
	m_maincpu(*this, I8021_TAG),
	m_y0(*this, "Y0"),
	m_y1(*this, "Y1"),
	m_y2(*this, "Y2"),
	m_y3(*this, "Y3"),
	m_y4(*this, "Y4"),
	m_y5(*this, "Y5"),
	m_y6(*this, "Y6"),
	m_y7(*this, "Y7"),
	m_y8(*this, "Y8"),
	m_y9(*this, "Y9"),
	m_ya(*this, "YA"),
	m_yb(*this, "YB"),
	m_yc(*this, "YC"),
	m_kbrdy_cb(*this),
	m_kbdata_cb(*this),
	m_y(0),
	m_kbrdy(-1),
	m_kbdata(-1),
	m_kback(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void victor_9000_keyboard_t::device_start()
{
	// resolve callbacks
	m_kbrdy_cb.resolve_safe();
	m_kbdata_cb.resolve_safe();

	// state saving
	save_item(NAME(m_p1));
	save_item(NAME(m_y));
	save_item(NAME(m_stb));
	save_item(NAME(m_y12));
	save_item(NAME(m_kbrdy));
	save_item(NAME(m_kbdata));
	save_item(NAME(m_kback));
}


//-------------------------------------------------
//  kback_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( victor_9000_keyboard_t::kback_w )
{
	if (LOG) logerror("KBACK %u\n", state);

	m_kback = !state;
}


//-------------------------------------------------
//  kb_p1_r -
//-------------------------------------------------

READ8_MEMBER( victor_9000_keyboard_t::kb_p1_r )
{
	UINT8 data = 0xff;

	switch (m_y)
	{
		case 0: data &= m_y0->read(); break;
		case 1: data &= m_y1->read(); break;
		case 2: data &= m_y2->read(); break;
		case 3: data &= m_y3->read(); break;
		case 4: data &= m_y4->read(); break;
		case 5: data &= m_y5->read(); break;
		case 6: data &= m_y6->read(); break;
		case 7: data &= m_y7->read(); break;
		case 8: data &= m_y8->read(); break;
		case 9: data &= m_y9->read(); break;
		case 0xa: data &= m_ya->read(); break;
		case 0xb: data &= m_yb->read(); break;
	}

	if (!m_y12)
	{
		data &= m_yc->read();
	}

	return data;
}


//-------------------------------------------------
//  kb_p1_w -
//-------------------------------------------------

WRITE8_MEMBER( victor_9000_keyboard_t::kb_p1_w )
{
	m_p1 = data;
}


//-------------------------------------------------
//  kb_p2_w -
//-------------------------------------------------

WRITE8_MEMBER( victor_9000_keyboard_t::kb_p2_w )
{
	/*

	    bit     description

	    P20     22-908 CLR, 22-950 STB
	    P21     KBRDY
	    P22     Y12
	    P23     KBDATA

	*/

	// keyboard rows 0-11
	if (!BIT(data, 0))
	{
		m_y = m_p1 & 0x0f;
	}

	// keyboard row 12
	m_y12 = BIT(data, 2);

	// keyboard ready
	m_kbrdy_cb(BIT(data, 1));

	// keyboard data
	m_kbdata_cb(BIT(data, 3));
}


//-------------------------------------------------
//  kb_t1_r -
//-------------------------------------------------

READ8_MEMBER( victor_9000_keyboard_t::kb_t1_r )
{
	return m_kback;
}
