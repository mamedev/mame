// licenses: MAME, BSD
// copyright-holders:R. Belmont
/***************************************************************************

Fanuc System P Model G
Fanuc 1983

2014-03-22 Skeleton driver.
This is a machine from 1983 in a single case
with a lot of ports and a unique keyboard.

Also known as Fanuc P-G System, this is a dedicated 8085+8086+8087-based computer
system running software for CNC Programming.
The system boots up from on-board EPROM and shows a big ASCII-art boot screen
FANUC SYSTEM P MODEL G and the ROM software version in the lower right corner.
To initiate booting from the floppy drive hold down the LOAD key for 3-5 seconds.
The system checks for a long LOAD key press so that it doesn't load software
if the LOAD key is accidentally pressed quickly while using the system, which would
erase everything in memory and all data up to that point and re-load the software from
scratch. When loading is activated application software is read from floppies.

The softwares are not point and click auto-generation type conversational CAD/CAM
applications. The earlier 'non-Symbolic' software requires knowledge of programming in APT
and other languages of the era. The 'Symbolic' software has menus and asks questions and
the blanks must be filled in correctly. The graphics are mostly made of lines but are
sufficient to complete the task easily. Efficient and effective use of this system
requires deep knowledge of CNC Machining techniques (Turning/Milling etc) and a
good understanding of machining processes and procedures. With correct usage this system
can be used to create CNC G-Code programs for any part that can be manufactured on a
CNC Machine. Because the system is made in Japan in the early 80's and the manuals are
very technical it also requires some skill in deciphering Japanese-English translated
technical texts to understand how to use it properly.

The box housing everything is 20" wide by 20" deep by 12" high and weighs
approximately 40 pounds. Power input is 85VAC to 110VAC. For the non-US and
non-Japanese markets a separate dedicated power supply is provided and is 12"
wide by 8" deep by 10" high and weighs approximately 20 pounds.

A number of optional peripherals can connect to it including a Fanuc Printer,
Fanuc PPR Unit (Paper tape Puncher/Reader with built-in printer), Fanuc Program
File (containing a 20MB HDD, two 8" floppy drives and two RS232 ports), Fanuc Cassette
Adapter, XY Plotter (A3 or A1), Fanuc Digitizing Tablet (A3 or A0) and Fanuc I/O Selector Box.

The P-G System has an internal 12" color monitor and dual 5 1/4" floppy drives.
A later model was released in 1986 called the Mark II using dual 3 1/2" floppy
drives. The previous version was SYSTEM P MODEL D. It had a 12" green monochrome
monitor and booted from, and stored to, a cassette tape or floppy disk.

The screen resolution is 512 x 384 pixels.
It can display 64 characters x 24 lines.

The floppy format is custom. Floppies are double sided double density and
regular PC DSDD 360k floppies can be used after they are formatted using the
P-G System.
The floppy geometry is 40 tracks, 16 sectors per track, 256 bytes per sector
and 2 sides for a total storage capacity of 327680 bytes.
The floppy drives are typical PC-type 5 1/4" 360k drives and were manufactured
by Y-E DATA, model YD-580.

The floppy disks can be backed-up and imaged using a DOS program called ImageDisk
which is available here.....
http://www.classiccmp.org/dunfield/img/index.htm
With a 5 1/4" HD floppy drive, in the GUI in settings change the number of
cylinders to 40, translate speed 300 -> 250 (to read a DD disk on a HD drive).
On the main menu press R to Read, type a file-name and press enter, press enter
again to skip the comment. Press enter again and it will read the disk and save
it to the HDD.

The following is a complete list of software titles available.
The info is taken from a glossy sales brochure printed in July 1985.
Other versions did exist so this list is not final.
* denotes it is dumped. All other titles are not dumped and are needed.

Language Input -

                 Title           Part Number
                 --------------------------------
                 FAPT TURN       A08B-0033-J600#E
                 FAPT CUT        A08B-0033-J620#E
                 FAPT MILL       A08B-0033-J640#E
                 FAPT DIE-II     A08B-0033-J660#E
                 FAPT PUNCH-I    A08B-0033-J520#E
                 FAPT PUNCH-II   A08B-0033-J700#E
                 FAPT HELICAL    A08B-0033-J642#E
                 FAPT POST       A08B-0033-H642#E
                *FAPT POST       A08B-0031-H630   Edition C 85/1/31


Graphic Input -

                 Title                 Part Number
                 --------------------------------------
                *Symbolic FAPT TURN    A08B-0033-J800#E +English
                 Symbolic FAPT MILL    A08B-0033-J840#E
                 Symbolic FAPT DRILL   A08B-0033-J860#E
                 Symbolic FAPT CUT     A08B-0033-J820#E
                 FAPT DIGITIZER        A08B-0033-J510#E

+ Symbolic FAPT TURN was available in English, German, French, Dutch, Finnish,
and Swedish versions.


Support System -

                 Title          Part Number
                 -------------------------------
                *FAPT TRACER    A08B-0033-H620#E  Edition B 85/1/16
                *FAPT TEACHER   A08B-0033-J610#E  Edition B 85/1/12
                *FAPT DOCTOR    A08B-0033-J600#E  Edition B 84/12/21


Note: To initiate booting from the floppy drive hold down the LOAD key for 3-5 seconds.

The software for the Fanuc System P Model G is extremely rare now and very
difficult to find. If you do have any of these wanted software titles or any manuals
listed below and want to help please contact me (Guru) via http://mamedev.org/contact.html

The following is a complete list of manuals available for the first edition of the
Fanuc System P Model G released in 1983. The info is taken from a glossy sales brochure
printed in July 1985. There were other manuals released later for the Mark II and
updated manuals (each with a different part number).
The manuals were available in Japanese and English. The part numbers listed here
are English versions, denoted by the E at the end of the part number.
* denotes these manuals are secured and available in PDF format.

Description -

                 Title                        Part Number
                 ---------------------------------------
                 FAPT TURN/MILL Description   B-54102E
                 FAPT CUT Description         B-54103E
                 FAPT PUNCH-I Description     B-54104E
                 FAPT TRACER Description      B-54106E
                 FAPT DIGITIZER Description   B-54107E
                 FAPT DIE-II Description      B-54121E
                 Symbolic FAPT Description    B-54131E


Operator's Manual -

                 Title                                          Part Number
                 ----------------------------------------------------------
                *System P-Model G Operator's Manual             B-54111E/03
                 System P-Model G Mark II Operator's Manual     B-66014E
                *System P-Model G Operator's Manual Supplement  B-54112E/03-1
                 FAPT TURN/MILL Operator's Manual               B-54112E
                 FAPT CUT Operator's Manual                     B-54113E
                 FAPT PUNCH-I Operator's Manual                 B-54114E
                 FAPT PUNCH-II Operator's Manual                B-54115E
                *FAPT TRACER Operator's Manual                  B-54116E/03
                 FAPT DIGITIZER Operator's Manual               B-54117E
                *FAPT Universal POST Operator's Manual          B-54118E/02
                 FAPT DIE-II Operator's Manual (Volume 1)       B-54122E
                 FAPT DIE-II Operator's Manual (Volume 2)       B-54122E-1
                *FAPT TEACHER Operator's Manual                 B-54126E/01
                 220S FAPT MILL Operator's Manual               B-54127E
                *Symbolic FAPT TURN Operator's Manual           B-54132E/01
                 Symbolic FAPT MILL Operator's Manual           B-54134E
                 Symbolic FAPT CUT Operator's Manual            B-54136E
                 Symbolic FAPT DRILL Operator's Manual          B-54138E
                *Symbolic FAPT TURN Operator's Manual           B-66025E/01 (for System P Mark II)


Others -

                 Title                                     Part Number
                 -----------------------------------------------------
                *Symbolic FAPT TURN Operator's Handbook    B-53034E (for System P Model D)
                 FANUC CASSETTE Operator's Manual          B-53484E
                 FAPT DIE-II Part program examples         B-54123E
                 FAPT TURN/MILL/CUT Part program examples  B-54128E
                 Symbolic FAPT TURN Operator's Handbook    B-54133E
                 System P-Model G Operator's Handbook      B-54158E
                *System P-Model G Maintenance Manual       B-54159E/01
                *FANUC PPR Operator's Manual               B-54584E/01

Note the handbooks are pocket-sized 8" long by 3 1/2" wide and approximately 50 pages.


The unit has it's own dedicated keyboard with many special keys.
The keyboard layout is shown below.

|------------------------------------------------------------------------------|
|                                                                              |
| LOAD F0 F1 F2 F3 F4 F5 F6 F7 F8 F9 F10 F11 F12 F13 F14 F15      R0 R1 R2 R3  |
|                                                                              |
|                                                                              |
|          !  "  #  $  %  &  '  (  )     =                                     |
|  K0      1  2  3  4  5  6  7  8  9  0  -   ^   Y   DEL          7  8  9  +   | (Y is Japanese Yen sign)
|                                                                              |
|                                                                              |
|  K1   CAN Q  W  E  R  T  Y  U  I  O  P   @   [   NL   BS        4  5  6  -   | (NL means NEXT LINE, BS is backspace)
|                                                                              | (NL is equivalent to return or enter and)
|                                        +   *                                 | (forces the cursor to move to the next data entry point)
|  K2     UC  A  S  D  F  G  H  J  K  L  ;   :   ]   UC           1  2  3  x   | (UC is uppercase)
|                                                                              |
|                                   <  >   ?                                   |
|  K3      LC  Z  X  C  V  B  N  M  ,  .   /   -   LC             0  ,  .  /   | (LC is lowercase)
|                                                                              |
|                                                                              |
|                  _S__P__A__C__E__B__A__R_                         _N__L_     |
|                                                                              |
|------------------------------------------------------------------------------|

On the numeric keypad there are directional arrows on numbers 1 2 3 4 6 7 8 9
1 3 7 9 have arrows pointing South West, South East, North West, North East.
2 4 6 8 have arrows pointing down, left, right and up.
5 is the center and has no additional markings on it.
Number 0 has an anti-clockwise 180 degrees arc with an arrow at the end and . has
a clockwise 180 degrees arc with an arrow at the end.
These keys are the 'Symbolic' keys.

The F-keys and R-keys are programmed by the software that is running on the system.
The F-keys act like SPDT switches and can be toggled either off or on.
When they are on, a LED in the center of the key lights.
For Symbolic FAPT TURN these keys are pre-programmed as follows.....

F0 - ON:  Sets the backwards direction when using the R1 key.
     OFF: Sets the forwards direction when using the R1 key. Default is OFF.
F1 - ON:  Makes the whole screen the graphic area.
     OFF: Auto-calc the graphic area so the graphic does not overlap the text. Default is OFF.
F2 - ON:  Shows the parts figure (graphics). Default is ON.
     OFF: Does not show the parts figure.
F3 - ON:  Display the NC G-Code data on screen. Default is ON.
     OFF: Does not display the NC G-Code data on screen.
F4 - ON:  Printer ON.
     OFF: Printer OFF. The printer can be switched on or off any time. When enabled
          everything displayed on the screen will also print on the printer. Default is OFF.
F5 - ON:  Stops execution of the NC G-Code data before each process begins. Keyboard input additions
          can also be done at this time. To continue press NL.
     OFF: Program execution continues to the end. Default is OFF.
F6 - ON:  Outputs the NC G-Code data to a separately selected medium (floppy/cassette or paper tape)
     OFF: No output to additional medium. Default is OFF.
F7 - ON:  Stops each time a line of NC G-Code data is output. This is equivalent to Single Block on a CNC Machine.
          To continue press NL.
     OFF: Program execution continues to the end. Default is OFF.
F8 -
F9 -
F10- ON:  Sends the part figure graphic and NC G-Code data to the XY plotter
     OFF: No output to XY plotter
F11-
F12-
F13-
F14-
F15-

The function of the R-keys changes depending on the application and the menu shown on the screen.
The R-keys are used for tasks within the current screen so the function of the R-keys is always
displayed on screen at all times.
The initial Symbolic FAPT TURN settings for the R-keys are....
R0 - FAPT Execution
R1 - Family Program
R2 - Setting
R3 - Auxiliary Work


Box Layout (top view)
----------

A08B-0033-B001
|--------------------------------------------|
| ------------MAIN PCB---------------------- |
|   -----------SUB-PCB-----------         |  |
|                                         |  |
| |-----------------------|               |  |
| |                       |               |  |
| |                       |               P  |
| |       CRT UNIT        |               O  |
| |                       |               W  |
| |                       |               E  |
| |      12" COLOR        |  |---------|  R  |
| |                       |  |FDD UNIT |  |  |
| |        SCREEN         |  |A87L-0001|  P  |
| |                       |  |-0026    |  C  |
| |                       |  |         |  B  |
| |                       |  |         |  |  |
| |                       |  |5 1/4"   |  |  |
| |                       |  |FLOPPY   |     |
| |                       |  |DRIVES   |     |
| |                       |  |x2       |     |
| |-----------------------|  |---------|     |
|--------------------------------------------|


Main PCB Layout
---------------

A20B-1000-0710/03B
|-------------------------------------------|
| CNF CNE     CND  CNC        CNB      CNA  |
|   VR1     ^                               |
|                JUMPERS           XXXXXXXXX|
|       % MB15541         XXXXXXXXXXXXXXXXXX|
|                         XXXXXXXXXXXXXXXXXX|
| 8087-3           D8253  XXXXXXXXXXXXXXXXXX|
|   8086-2   D765  D8253           XXXXXXXXX|
|             D8257                         |
|15MHz   D8259 D8259     D8251 D8251        |
|D8284  040_001A.13A     D8251 D8251        |
|       040_002A.15A  VR2         CN2   CN1 |
|       CN7     CN6      CN5      CN4   CN3 |
|-------------------------------------------|
Notes:
      D8086   - Intel 8086 CPU. Clock input 5.000MHz [15/3]
      D8087   - Intel 8087 x87 Floating-Point Co-Processor. Clock input 5.000MHz [15/3]
      XXXXXXX - Fujitsu MB8265-15 65536 x1-bit DRAM (72 chips total)
      MB15541 - Fujitsu MB15541 Custom Chip
      D765    - NEC D765 Single/Double Density Floppy-Disk Controller. Clock input 4.000MHz [16/4]
      D8251   - Intel D8251 Programmable Communications Interface (USART)
      D8253   - NEC D8253 Programmable Interval Timer. Clock input 1.25MHz [15/12]
      D8257   - NEC D8257 Programmable DMA Controller. Clock input 3.000MHz [15/5]
      D8259   - NEC D8259 Programmable Interrupt Controller
      D8284   - Intel D8284 Clock Generator and Driver for 8086/8088 Processors
      A40_00* - Fujitsu MBM2764 8k x8-bit EPROM
      VR1/VR2 - Potentiometer
      ^       - 3 chips marked Y-E Data Fujitsu
                MB4393
                MB14324
                MB14323
      %       - Unknown 20-pin Ceramic DIP chip with heat-sink
      CNA     - 50-pin flat cable joining to Sub PCB
      CNB     - 50-pin flat cable joining to Sub PCB
      CNC     - 6-pin power cable joining to Sub PCB
      CND     - 34-pin flat cable joining to FDD Unit
      CNE     - Fanuc Honda MR-50 50-pin female connector for expansion (not used)
      CNF     - Power input connector
      CN1     - 25-pin Female D-type connector. (for RS232 external peripherals \  CNC Machine,
      CN2     - 25-pin Female D-type connector. (for RS232 external peripherals  | PPR Unit, X-Y Plotter,
      CN3     - 25-pin Female D-type connector. (for RS232 external peripherals  | Tablet,
      CN4     - 25-pin Female D-type connector. (for RS232 external peripherals /  Cassette Adapter etc (connections in any order)
      CN5     - Fanuc Honda MR-50 50-pin female connector (probably for external connection of the Fanuc Program File Unit)
      CN6     - Fanuc Honda MR-20 20-pin female connector for the keyboard
      CN7     - Fanuc Honda MR-20 20-pin male. Specification says 'not used' but this appears to be a
                Facit 4070 Parallel Reader/Puncher connector
                Pinout: (pin 1 is top left, location key is on the opposite side)
                       |---------------------------------------------------|
                       |                                                   |
                       | 1_PR   2_TE  3_ERR  4_TTY3  5_+6V  6_TTY2  7_TTY1 |
                       |                                                   |
                       |    8_SG   9_SD   10_0V  11_CH1  12_CH2  13_CH3    O
                       |                                                   |
                       | 14_CH4 15_CH5 16_CH6 17_CH7  18_CH8 19_CH9  20_PI |
                       |                                                   |
                       |---------------------------------------------------|
      JUMPERS - 15 2-pin jumpers labelled S1 to S15. S2, S3 & S4 are not shorted. All others are shorted.


Sub PCB Layout
--------------

A20B-1000-0720/02B
|--------------------------------|
| CNA     CNB      CNC   CND     |
|                                |
|                  MB15542    CNE|
|                                |
|                  HD6845S  D8085|
|            16MHz               |
|                                |
|   X                            |
| XXXXXXXX                       |
| XXXXXXXX                       |
| XXXXXXXX      6264 A41_010B.28B|
|               6264 A41_020A.30B|
|--------------------------------|
Notes:
      D8085   - NEC D8085A-2 CPU. Clock input 8.000MHz [16/2].
                Note 8085 has internal /2 divider so actual clock speed is 4.000MHz
      HD6845S - Hitachi HD6845S / HD46505S CRT Controller. Clock input 2.000MHz [8/2]
      6264    - Hitachi HM6264P-15 8k x 8-bit SRAM
      XXXXXXX - Fujitsu MB8265-15 65536 x1-bit DRAM (25 chips total)
      MB15542 - Fujitsu MB15542 Custom Chip
      A41_010B- Intel D27128 16k x8-bit EPROM
      A42_020A- Hitachi 27256G 32k x8-bit EPROM
      CNA     - 50-pin flat cable joining to Main PCB
      CNB     - 50-pin flat cable joining to Main PCB
      CNC     - 6-pin power cable joining to Main PCB
      CND     - 20-pin flat cable joining to CRT Unit (video output)
      CNE     - Fanuc Honda MR-50 50-pin male connector for expansion (not used)
      HSync   - 22.7273kHz
      VSync   - 54.6330Hz


Block Diagram
-------------
Below is the block diagram shown in the Maintenance Manual.
The arrows denote direction of data flow.

          |-------|                                             |--------|      |----------------|
          |Sub CPU|                                             |Main CPU|<---->|Math Coprocessor|
          |-------|                                             |--------|      |----------------|
              /\                                                     /\                 /\
              |                                                      |                  |
              |                                                      \/                 \/
  |-----|     |                                                      |------------------|
  |EPROM|<--->|                                                               /\
  |-----|     |          |---------------|                                    |     |----------------|
              |          | Common memory |<---------------------------------->|<--->|RS232C interface|---CN1
              |<-------->|===============|                                    |     |----------------|
              |      /-->| Graphic memory|------|                             |
              |      |   |---------------|      |                             |     |----------------|
              |      |                          |         |--------|          |<--->|RS232C interface|---CN2
              |      |                            |         | BOOT   |<-------->|     |----------------|
              |      |                          |         | EPROM  |          |
              |      |   |----------------|     |         |--------|          |     |----------------|
              |<-----|-->|Character memory|--|  |                             |<--->|RS232C interface|---CN3
              |      |-->|----------------|  |  |         |--------|          |     |----------------|
              |      |                       |  |         |Main RAM|<-------->|
              |      |                       |  |         |--------|          |     |----------------|
              |      |                       |  |                             |<--->|RS232C interface|---CN4
              |      |                       |  |                             |     |----------------|
|---------|   |      |                       \/ \/                            |
|Keyboard |   \/     \---|---------------------------|                        |     |-----------------|  CN9  |--------|
|interface|<->|<-------->|    CRT control circuit    |                        |<--->|Floppy controller|---O---|FDD UNIT|
|----|----|              |-------------|-------------|                        |     |-----------------|       |--------|
     |                                 |                                      |
     |                                 |                                      \/
     O CN6                             O CN8                                  O CN5
     |                                 |
     |                                 |
 |---|----|                       |----|----|
 |Keyboard|                       | Screen  |
 |--------|                       |---------|


  TODO:
    - Is the VRAM hookup anything like correct?
    - Hookup enough keyboard to get it to boot a floppy, the FAPT DOCTOR
      program will be invaluable to answering many questions.
    - Shared RAM is 8k, but there are 2 6264s on the sub board.  Is shared RAM
       banked?
    - I/O is at F00xx:
        ':maincpu' (FC15A): unmapped program memory write to F0012 = 00CE & 00FF
        ':maincpu' (FC15D): unmapped program memory write to F0016 = 00CE & 00FF
        ':maincpu' (FC160): unmapped program memory write to F001A = 00CE & 00FF
        ':maincpu' (FC163): unmapped program memory write to F001E = 00CE & 00FF
        ':maincpu' (FC16D): unmapped program memory write to F000E = 0034 & 00FF
        ':maincpu' (FC172): unmapped program memory write to F0008 = 00D4 & 00FF
        ':maincpu' (FC177): unmapped program memory write to F0008 = 0030 & 00FF
        ':maincpu' (FC17C): unmapped program memory write to F000E = 0056 & 00FF
        ':maincpu' (FC181): unmapped program memory write to F000A = 0010 & 00FF
        ':maincpu' (FC186): unmapped program memory write to F000E = 0096 & 00FF
        ':maincpu' (FC18B): unmapped program memory write to F000C = 0010 & 00FF
        ':maincpu' (FC190): unmapped program memory write to F004E = 0034 & 00FF
        ':maincpu' (FC195): unmapped program memory write to F0048 = 0020 & 00FF
        ':maincpu' (FC19A): unmapped program memory write to F0048 = 004E & 00FF
        ':maincpu' (FC19F): unmapped program memory write to F004E = 0056 & 00FF
        ':maincpu' (FC1A4): unmapped program memory write to F004A = 0010 & 00FF
        ':maincpu' (FC1A9): unmapped program memory write to F004E = 0096 & 00FF
        ':maincpu' (FC1AE): unmapped program memory write to F004C = 0010 & 00FF

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/i8085/i8085.h"
#include "machine/ram.h"
#include "machine/i8251.h"
#include "machine/8257dma.h"
#include "machine/upd765.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/upd765.h"
#include "video/mc6845.h"
#include "formats/imd_dsk.h"

#define MAINCPU_TAG "maincpu"
#define SUBCPU_TAG  "subcpu"
#define USART0_TAG  "usart0"
#define USART1_TAG  "usart1"
#define USART2_TAG  "usart2"
#define USART3_TAG  "usart3"
#define PIT0_TAG    "pit0"
#define PIT1_TAG    "pit1"
#define PIC0_TAG    "pic0"
#define PIC1_TAG    "pic1"
#define DMAC_TAG    "dmac"
#define CRTC_TAG    "crtc"
#define FDC_TAG     "fdc"
#define SCREEN_TAG  "screen"
#define SHARED_TAG  "shared"
#define CHARGEN_TAG "chargen"

class fanucspmg_state : public driver_device
{
public:
	fanucspmg_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, MAINCPU_TAG)
		, m_subcpu(*this, SUBCPU_TAG)
		, m_usart0(*this, USART0_TAG)
		, m_usart1(*this, USART1_TAG)
		, m_usart2(*this, USART2_TAG)
		, m_usart3(*this, USART3_TAG)
		, m_pit0(*this, PIT0_TAG)
		, m_pit1(*this, PIT1_TAG)
		, m_pic0(*this, PIC0_TAG)
		, m_pic1(*this, PIC1_TAG)
		, m_dmac(*this, DMAC_TAG)
		, m_crtc(*this, CRTC_TAG)
		, m_fdc(*this, FDC_TAG)
		, m_shared(*this, SHARED_TAG)
		, m_chargen(*this, CHARGEN_TAG)
	{ }

	required_device<i8086_cpu_device> m_maincpu;
	required_device<i8085a_cpu_device> m_subcpu;
	required_device<i8251_device> m_usart0;
	required_device<i8251_device> m_usart1;
	required_device<i8251_device> m_usart2;
	required_device<i8251_device> m_usart3;
	required_device<pit8253_device> m_pit0;
	required_device<pit8253_device> m_pit1;
	required_device<pic8259_device> m_pic0;
	required_device<pic8259_device> m_pic1;
	required_device<i8257_device> m_dmac;
	required_device<mc6845_device> m_crtc;
	required_device<upd765a_device> m_fdc;
	required_shared_ptr<UINT8> m_shared;
	required_memory_region m_chargen;

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_READ8_MEMBER(shared_r);
	DECLARE_WRITE8_MEMBER(shared_w);
	DECLARE_READ8_MEMBER(vram1_r);
	DECLARE_WRITE8_MEMBER(vram1_w);
	DECLARE_READ8_MEMBER(vram2_r);
	DECLARE_WRITE8_MEMBER(vram2_w);
	DECLARE_WRITE8_MEMBER(vram_bank_w);
	DECLARE_READ8_MEMBER(vblank_ack_r);
	DECLARE_WRITE8_MEMBER(vbl_ctrl_w);
	DECLARE_WRITE8_MEMBER(keyboard_row_w);
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_WRITE8_MEMBER(video_ctrl_w);

	DECLARE_READ8_MEMBER(test_r);
	DECLARE_READ8_MEMBER(vbl_r);

	DECLARE_WRITE_LINE_MEMBER(vsync_w);

	DECLARE_DRIVER_INIT(fanucspmg);

	UINT8 m_vram[24576];
	UINT8 m_video_ctrl;

private:
	virtual void machine_reset();
	INT32 m_vram_bank;
	UINT8 m_vbl_ctrl;
	UINT8 m_keyboard_row;
	UINT8 m_vbl_stat;
};

DRIVER_INIT_MEMBER(fanucspmg_state, fanucspmg)
{
	memset(m_vram, 0, sizeof(m_vram));

	save_item(NAME(m_vram));
	save_item(NAME(m_vram_bank));
	save_item(NAME(m_vbl_ctrl));
	save_item(NAME(m_keyboard_row));
	save_item(NAME(m_video_ctrl));
}

READ8_MEMBER(fanucspmg_state::shared_r)
{
	return m_shared[offset];
}

WRITE8_MEMBER(fanucspmg_state::shared_w)
{
	m_shared[offset] = data;
}

READ8_MEMBER(fanucspmg_state::test_r)
{
	return 0x00;    // 0x80 to start weird not-sure-what process which may be FDC related
}

READ8_MEMBER(fanucspmg_state::vbl_r)
{
	return m_vbl_stat;
}

static ADDRESS_MAP_START(maincpu_mem, AS_PROGRAM, 16, fanucspmg_state)
	AM_RANGE(0x00000, 0x7ffff) AM_RAM   // main RAM

	AM_RANGE(0x88000, 0x88001) AM_READ8(vbl_r, 0xffff)

	AM_RANGE(0xf0004, 0xf0005) AM_READ8(test_r, 0xffff)

	AM_RANGE(0xf8000, 0xf9fff) AM_READWRITE8(shared_r, shared_w, 0xffff)
	AM_RANGE(0xfc000, 0xfffff) AM_ROM AM_REGION(MAINCPU_TAG, 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(maincpu_io, AS_IO, 16, fanucspmg_state)
ADDRESS_MAP_END

WRITE_LINE_MEMBER(fanucspmg_state::vsync_w)
{
	if ((m_vbl_ctrl & 0x08) == 0x08)
	{
		if (state == ASSERT_LINE)
		{
			m_subcpu->set_input_line(I8085_RST75_LINE, ASSERT_LINE);
		}
	}

	m_vbl_stat = (state == ASSERT_LINE) ? 1 : 0;
}

READ8_MEMBER(fanucspmg_state::vram1_r)
{
	return m_vram[m_vram_bank + offset];
}

WRITE8_MEMBER(fanucspmg_state::vram1_w)
{
	m_vram[m_vram_bank + offset] = data;
}

READ8_MEMBER(fanucspmg_state::vram2_r)
{
	return m_vram[m_vram_bank + offset + 0x600];
}

WRITE8_MEMBER(fanucspmg_state::vram2_w)
{
	m_vram[m_vram_bank + offset + 0x600] = data;
}

WRITE8_MEMBER(fanucspmg_state::vram_bank_w)
{
	m_vram_bank = (data & 7) * 0xc00;
}

READ8_MEMBER(fanucspmg_state::vblank_ack_r)
{
	m_subcpu->set_input_line(I8085_RST75_LINE, CLEAR_LINE);

	return 0xff;
}

// bit 1 seems to route to bit 7 of f0004 on the 8086 (signals the "LOAD" key pressed?)
// bit 3 appears to enable vblank IRQs
WRITE8_MEMBER(fanucspmg_state::vbl_ctrl_w)
{
	m_vbl_ctrl = data;
}

WRITE8_MEMBER(fanucspmg_state::keyboard_row_w)
{
	m_keyboard_row = data;
}

READ8_MEMBER(fanucspmg_state::keyboard_r)
{
	return 0;
}

// bit 0 is set when clearing VRAM
// bit 1 is display enable
WRITE8_MEMBER(fanucspmg_state::video_ctrl_w)
{
	m_video_ctrl = data;
}

static ADDRESS_MAP_START(subcpu_mem, AS_PROGRAM, 8, fanucspmg_state)
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_REGION(SUBCPU_TAG, 0)

	AM_RANGE(0x4000, 0x45ff) AM_READWRITE(vram1_r, vram1_w)
	AM_RANGE(0x4800, 0x4dff) AM_READWRITE(vram2_r, vram2_w)

	AM_RANGE(0x5000, 0x5000) AM_DEVREADWRITE(CRTC_TAG, mc6845_device, status_r, address_w)
	AM_RANGE(0x5001, 0x5001) AM_DEVREADWRITE(CRTC_TAG, mc6845_device, register_r, register_w)
	AM_RANGE(0x5008, 0x5008) AM_WRITE(keyboard_row_w)
	AM_RANGE(0x5009, 0x5009) AM_READ(keyboard_r)
	AM_RANGE(0x500a, 0x500b) AM_WRITENOP    // probably keyboard related, not sure how though
	AM_RANGE(0x500c, 0x500c) AM_WRITE(vbl_ctrl_w)
	AM_RANGE(0x500d, 0x500d) AM_WRITE(vram_bank_w)
	AM_RANGE(0x500e, 0x500e) AM_READ(vblank_ack_r)
	AM_RANGE(0x5018, 0x5018) AM_WRITE(video_ctrl_w)

	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE(SHARED_TAG) // shared RAM
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( fanucspmg )
INPUT_PORTS_END

void fanucspmg_state::machine_reset()
{
	m_vbl_ctrl = 0;
	m_vram_bank = 0;
	m_video_ctrl = 0;
}

READ8_MEMBER(fanucspmg_state::memory_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(fanucspmg_state::memory_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.write_byte(offset, data);
}

I8257_INTERFACE( fanucspmg_dma )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(fanucspmg_state, memory_read_byte),
	DEVCB_DRIVER_MEMBER(fanucspmg_state, memory_write_byte),
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL }
};

static MC6845_UPDATE_ROW( fanuc_update_row )
{
	fanucspmg_state *state = downcast<fanucspmg_state *>(device->owner());
	UINT32  *p = &bitmap.pix32(y);
	int i;
	UINT8 *chargen = state->m_chargen->base();

	for ( i = 0; i < x_count; i++ )
	{
		UINT16 offset = ( ma + i );

		if (state->m_video_ctrl & 0x02)
		{
			if (offset <= 0x5ff)
			{
				UINT8 chr = state->m_vram[offset + 0x600];
				UINT8 attr = state->m_vram[offset];
				UINT8 data = chargen[ chr + (ra * 256) ];
				UINT32 fg = 0xffffff;
				UINT32 bg = 0;

				// colors are black, red, green, yellow, blue, purple, light blue, and white
				// attr & 0x70 = 0x60 is green, need more software running to know the others
				if (((attr>>4) & 7) == 6)
				{
					fg = 0x008800;
				}

				*p++ = ( data & 0x01 ) ? fg : bg;
				*p++ = ( data & 0x02 ) ? fg : bg;
				*p++ = ( data & 0x04 ) ? fg : bg;
				*p++ = ( data & 0x08 ) ? fg : bg;
				*p++ = ( data & 0x10 ) ? fg : bg;
				*p++ = ( data & 0x20 ) ? fg : bg;
				*p++ = ( data & 0x40 ) ? fg : bg;
				*p++ = ( data & 0x80 ) ? fg : bg;
			}
		}
		else
		{
			*p++ = 0;
			*p++ = 0;
			*p++ = 0;
			*p++ = 0;
			*p++ = 0;
			*p++ = 0;
			*p++ = 0;
			*p++ = 0;
		}
	}
}

static MC6845_INTERFACE( mc6845_fanuc_intf )
{
	false,              /* show border area */
	0,0,0,0,            /* visarea adjustment */
	8,                  /* number of pixels per video memory address */
	NULL,               /* begin_update */
	fanuc_update_row,   /* update_row */
	NULL,               /* end_update */
	DEVCB_NULL,         /* on_de_changed */
	DEVCB_NULL,         /* on_cur_changed */
	DEVCB_NULL,         /* on hsync changed */
	DEVCB_DRIVER_LINE_MEMBER(fanucspmg_state, vsync_w), /* on vsync changed */
	NULL
};

static SLOT_INTERFACE_START( fanuc_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

FLOPPY_FORMATS_MEMBER( fanucspmg_state::floppy_formats )
	FLOPPY_IMD_FORMAT
FLOPPY_FORMATS_END

static MACHINE_CONFIG_START( fanucspmg, fanucspmg_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(MAINCPU_TAG, I8086, XTAL_15MHz/3)
	MCFG_CPU_PROGRAM_MAP(maincpu_mem)
	MCFG_CPU_IO_MAP(maincpu_io)

	MCFG_CPU_ADD(SUBCPU_TAG, I8085A, XTAL_16MHz/2/2)
	MCFG_CPU_PROGRAM_MAP(subcpu_mem)

	MCFG_DEVICE_ADD(USART0_TAG, I8251, 0)
	MCFG_DEVICE_ADD(USART1_TAG, I8251, 0)
	MCFG_DEVICE_ADD(USART2_TAG, I8251, 0)
	MCFG_DEVICE_ADD(USART3_TAG, I8251, 0)

	MCFG_DEVICE_ADD(PIT0_TAG, PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_15MHz/12)
	MCFG_PIT8253_CLK1(XTAL_15MHz/12)
	MCFG_PIT8253_CLK2(XTAL_15MHz/12)
	MCFG_DEVICE_ADD(PIT1_TAG, PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_15MHz/12)
	MCFG_PIT8253_CLK1(XTAL_15MHz/12)
	MCFG_PIT8253_CLK2(XTAL_15MHz/12)

	MCFG_I8257_ADD(DMAC_TAG, XTAL_15MHz / 5, fanucspmg_dma)

	MCFG_PIC8259_ADD(PIC0_TAG, INPUTLINE("maincpu", 0), VCC, NULL)
	MCFG_PIC8259_ADD(PIC1_TAG, INPUTLINE("maincpu", 0), VCC, NULL)

	MCFG_UPD765A_ADD(FDC_TAG, true, true)
	MCFG_FLOPPY_DRIVE_ADD(FDC_TAG":0", fanuc_floppies, "525dd", fanucspmg_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FDC_TAG":1", fanuc_floppies, "525dd", fanucspmg_state::floppy_formats)

	MCFG_SCREEN_ADD( SCREEN_TAG, RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_15MHz, 640, 0, 512, 390, 0, 384 )
	MCFG_SCREEN_UPDATE_DEVICE( CRTC_TAG, mc6845_device, screen_update )

	MCFG_MC6845_ADD( CRTC_TAG, HD6845, SCREEN_TAG, XTAL_8MHz/2, mc6845_fanuc_intf)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( fanucspg )
	ROM_REGION(0x4000, MAINCPU_TAG, 0)
	ROM_LOAD16_BYTE( "a40_001a.13a", 0x000000, 0x002000, CRC(1b8ac8ef) SHA1(309c081d25270e082ebf846b4f73cef76b52d991) )
	ROM_LOAD16_BYTE( "a40_002a.15a", 0x000001, 0x002000, CRC(587ae652) SHA1(ebc5a4c3d64ab9d6dd4d5355f85bc894e7294e17) )

	ROM_REGION(0x4000, SUBCPU_TAG, 0)
	ROM_LOAD( "a41_010b.28b", 0x000000, 0x004000, CRC(35a9714f) SHA1(5697b6c4db5adb5702dc1290ecc98758d5fab221) )

	ROM_REGION(0x8000, CHARGEN_TAG, 0)
	ROM_LOAD( "a42_020a.30b", 0x000000, 0x008000, CRC(33eb5962) SHA1(1157a72089ff77e8db9a9a8fcd0f6c32a1374f56) )
ROM_END

/* Driver */
/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT      CLASS           INIT       COMPANY  FULLNAME            FLAGS */
COMP( 1983, fanucspg, 0,      0,    fanucspmg, fanucspmg, fanucspmg_state, fanucspmg, "Fanuc", "System P Model G", GAME_NOT_WORKING | GAME_NO_SOUND)
