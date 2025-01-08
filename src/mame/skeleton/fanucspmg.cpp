// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

Fanuc System P-Model G
Fanuc 1983

2014-03-22 Skeleton driver.
This is a machine from 1983 in a single case
with a lot of ports and a unique keyboard.

Also known as Fanuc P-G System, this is a dedicated 8085+8086+8087-based computer
system running software for CNC Programming.
The system boots up from on-board EPROM and shows a big ASCII-art boot screen
FANUC SYSTEM P and the ROM software version in the lower right corner.
To initiate booting from the floppy drive hold down the LOAD key for 3-5 seconds.
The system checks for a long LOAD key press so that it doesn't load software
if the LOAD key is accidentally pressed quickly while using the system, which would
erase everything in memory and all data up to that point and re-load the software from
scratch. When loading is activated application software is read from floppies.

The software are not point and click auto-generation type conversational CAD/CAM
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

The P-G System has an internal 12" monitor and dual 5 1/4" floppy drives.
The first model had a 12" monochrome green monitor. In 1985 a color version was released.
In 1986 another model was released called the Mark II using dual 3 1/2" floppy drives.

The screen resolution is 512 x 384 pixels. It can display 64 characters x 24 lines.

The floppy format is custom. Floppies are double sided double density and
regular PC DSDD 360k floppies can be used after they are formatted using the
P-G System.
The floppy geometry is 40 tracks, 16 sectors per track, 256 bytes per sector
and 2 sides for a total storage capacity of 327680 bytes.
The floppy drives are typical PC-type 5 1/4" 360k drives and were manufactured
by Y-E DATA, model YD-580.

The 5 1/4" floppy disks can be backed-up and imaged using a DOS program called ImageDisk
which is available here.....
http://www.classiccmp.org/dunfield/img/index.htm
With a 5 1/4" HD floppy drive, in the GUI in settings change the number of
cylinders to 40, translate speed 300 -> 250 (to read a DD disk on a HD drive).
On the main menu press R to Read, type a file-name and press enter, press enter
again to skip the comment. Press enter again and it will read the disk and save
it to the HDD.

The following is a complete list of software titles available.
The info is taken from a glossy sales brochure printed in July 1985.
Other versions probably exist so this list is not final.
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
                *Symbolic FAPT TURN    A08B-0033-J800#E Edition B V02 L03 841116
                 Symbolic FAPT MILL    A08B-0033-J840#E
                 Symbolic FAPT DRILL   A08B-0033-J860#E
                 Symbolic FAPT CUT     A08B-0033-J820#E
                 FAPT DIGITIZER        A08B-0033-J510#E

+ Symbolic FAPT TURN was available in English, German, French, Dutch, Finnish,
and Swedish versions. Currently only the English version is archived.


Support System -

                 Title          Part Number
                 -------------------------------
                *FAPT TRACER    A08B-0033-H620#E  Edition B V02 L02 841108
                *FAPT TEACHER   A08B-0033-J610#E  Edition B L02 V01 841101
                *FAPT DOCTOR    A08B-0033-J600#E  Edition B V01 L03 841108


The software listed above with (*) have been tested on both mono and color versions and works fine.
Note: To initiate booting from the floppy drive hold down the LOAD key for 3-5 seconds.

The software for the Fanuc System P-Model G is extremely rare now and very difficult to find.
If you do have any of these wanted software titles or any manuals listed below and want to help
please contact me (Guru) via http://members.iinet.net.au/~lantra9jp1_nbn/gurudumps/comments.html

The following is a complete list of manuals available for the first edition of the
Fanuc System P-Model G released in 1983. The info is taken from a glossy sales brochure
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
The keyboard clips to the main box and is the top cover when the main box is transported.
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

A08B-0033-B001 (Color Version from 1985)
A08B-0031-B001 (Mono Version from 1983)
A08B-0031-B002 (Mono Version from 1984)
|--------------------------------------------|
| ------------MAIN PCB---------------------- |
|   -----------SUB-PCB-----------         |  |
|                                         |  |
| |-----------------------|               |  |
| |                       |               |  |
| |   FANUC 12" COLOR     |               P  |
| |       CRT UNIT        |               O  |
| |    A61L-0001-0078     |               W  |
| |                       |               E  |
| |                       |  |---------|  R  |
| |                       |  |FANUC    |  |  |
| |          OR           |  |FDD UNIT |  P  |
| |                       |  |A87l-0001|  C  |
| |                       |  |-0026    |  B  |
| |   FANUC 12" MONO      |  |         |  |  |
| |       CRT UNIT        |  |5 1/4"   |  |  |
| |   A61L-0001-0073      |  |FLOPPY   |     |
| |                       |  |DRIVES   |     |
| |                       |  |x2       |     |
| |-----------------------|  |---------|     |
|--------------------------------------------|
Notes:
      The CRT tube in the color version is a Matsushita 320DHB22. Input voltage is 110V AC
      The CRT tube in the mono version is a Hitachi 310KEB31. Input voltage is 24V DC and B+ is 11.0V
      The mono version does not have a SUB PCB
      The power PCB is identical for both color and mono versions


Main PCB Layout (for color version)
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
      D8086   - Intel 8086-2 CPU. Clock input 5.000MHz [15/3]
      D8087   - Intel 8087-3 x87 Floating-Point Co-Processor. Clock input 5.000MHz [15/3]
      XXXXXXX - Fujitsu MB8265-15 65536 x1-bit DRAM (72 chips total)
      MB15541 - Fujitsu MB15541 Custom Chip
      D765    - NEC D765 Single/Double Density Floppy-Disk Controller. Clock input 4.000MHz [16/4]
      D8251   - Intel D8251 Programmable Communications Interface (USART)
      D8253   - NEC D8253 Programmable Interval Timer. Clock input 1.25MHz [15/12]
      D8257   - NEC D8257 Programmable DMA Controller. Clock input 3.000MHz [15/5]
      D8259   - NEC D8259 Programmable Interrupt Controller
      D8284   - Intel D8284 Clock Generator and Driver for 8086/8088 Processors
      A40_00* - Fujitsu MBM2764 8k x8-bit EPROM
      VR1     - Potentiometer to adjust pulse width of floppy disk control unit
      VR2     - Potentiometer to adjust screen brightness
      ^       - 3 chips marked Y-E Data Fujitsu
                MB4393
                MB14324
                MB14323
      %       - Unknown 20-pin Ceramic DIP chip with heat-sink (likely to be Intel 8288 Bus Controller)
      CNA     - 50-pin flat cable joining to Sub PCB
      CNB     - 50-pin flat cable joining to Sub PCB
      CNC     - 6-pin power cable joining to Sub PCB
      CND     - 34-pin flat cable joining to FDD Unit
      CNE     - Fanuc Honda MR-50 50-pin female connector for factory testing (not used)
      CNF     - Power input connector
      CN1     - 25-pin Female D-type connector. (for RS232 external peripherals \  CNC Machine,
      CN2     - 25-pin Female D-type connector. (for RS232 external peripherals  | PPR Unit, X-Y Plotter,
      CN3     - 25-pin Female D-type connector. (for RS232 external peripherals  | Tablet,
      CN4     - 25-pin Female D-type connector. (for RS232 external peripherals /  Cassette Adapter etc (connections in any order)
      CN5     - Fanuc Honda MR-50 50-pin female connector (probably for external connection of the Fanuc Program File Unit)
      CN6     - Fanuc Honda MR-20 20-pin female connector for the keyboard
      CN7     - Fanuc Honda MR-20 20-pin male connector. Specification says 'not used'. Video signals are present on
                the connector so it is probably used for an external monitor
      JUMPERS - 15 2-pin jumpers labelled S1 to S15. S2, S3 & S4 are open and the others are shorted


Sub PCB Layout (for color version)
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
      D8085   - NEC D8085A-2 CPU. Clock input 8.000MHz [16/2]
                Note 8085 has internal /2 divider so actual clock speed is 4.000MHz
      HD6845S - Hitachi HD6845S / HD46505S CRT Controller. Clock input 2.000MHz [16/8]
      6264    - Hitachi HM6264P-15 8k x 8-bit SRAM
      XXXXXXX - Fujitsu MB8265-15 65536 x1-bit DRAM (25 chips total)
      MB15542 - Fujitsu MB15542 Custom Chip
      A41_010B- Intel D27128 16k x8-bit EPROM
      A42_020A- Hitachi 27256G 32k x8-bit EPROM
      CNA     - 50-pin flat cable joining to Main PCB
      CNB     - 50-pin flat cable joining to Main PCB
      CNC     - 6-pin power cable joining to Main PCB
      CND     - 20-pin flat cable joining to CRT Unit (video output)
      CNE     - Fanuc Honda MR-50 50-pin male connector for factory testing (not used)
      HSync   - 22.7273kHz
      VSync   - 54.6330Hz


Main PCB Layout (for mono version)
---------------

A20B-1000-0140/09F
|-------------------------------------------|
| CN10          CN9  VR1   CN8              |
|  6116         S3 ^                        |
| A22_020B.5G             %  D765  XXXXXXXXX|
|   16MHz   MB15542 MB15541  D8257 XXXXXXXXX|
|             S4 15MHz D8284       XXXXXXXXX|
| 8085-2    HD6845S  8087 D8259    XXXXXXXXX|
| CN12  A21_010F.17D 8086 D8253 A25_001A.33E|
|              CN11  S2    A25_002A.35E     |
|                    S1 D8251 D8251   D39   |
|                       D8251 D8251 S5 S6 S7|
|    YYYYYYYYY       VR2   D8253  CN2   CN1 |
|       CN7     CN6      CN5      CN4   CN3 |
|-------------------------------------------|
Notes:
      D8086   - Intel 8086-2 CPU. Clock input 5.000MHz [15/3]
      D8087   - Intel 8087-3 x87 Floating-Point Co-Processor. Clock input 5.000MHz [15/3]
      D8085   - NEC D8085A-2 CPU. Clock input 8.000MHz [16/2]
                Note 8085 has internal /2 divider so actual clock speed is 4.000MHz
      HD6845S - Hitachi HD6845S / HD46505S CRT Controller. Clock input 2.000MHz [16/8]
      XXXXXXX - Fujitsu MB8265-15 65536 x1-bit DRAM (36 chips total)
      YYYYYYY - Fujitsu MB8265-15 65536 x1-bit DRAM (9 chips total)
      6116    - Hitachi HM6116P-3 2k x 8-bit SRAM
      MB15541 - Fujitsu MB15541 Custom Chip
      MB15542 - Fujitsu MB15542 Custom Chip
      D765    - NEC D765 Single/Double Density Floppy-Disk Controller. Clock input 4.000MHz [16/4]
      D8251   - Intel D8251 Programmable Communications Interface (USART)
      D8253   - NEC D8253 Programmable Interval Timer. Clock input 1.25MHz [15/12]
      D8257   - NEC D8257 Programmable DMA Controller. Clock input 3.000MHz [15/5]
      D8259   - NEC D8259 Programmable Interrupt Controller
      D8284   - Intel D8284 Clock Generator and Driver for 8086/8088 Processors
      A2*     - Hitachi HN482764G 8k x8-bit EPROM
      VR1     - Potentiometer to adjust pulse width of floppy disk control unit
      VR2     - Potentiometer to adjust screen brightness
      ^       - 3 chips marked Y-E Data Fujitsu
                MB4393
                MB14324
                MB14323
      %       - Unknown 20-pin Ceramic DIP chip with heat-sink (likely to be Intel 8288 Bus Controller)
      CN1     - 25-pin Female D-type connector. (for RS232 external peripherals \  CNC Machine,
      CN2     - 25-pin Female D-type connector. (for RS232 external peripherals  | PPR Unit, X-Y Plotter,
      CN3     - 25-pin Female D-type connector. (for RS232 external peripherals  | Tablet,
      CN4     - 25-pin Female D-type connector. (for RS232 external peripherals /  Cassette Adapter etc (connections in any order)
      CN5     - Fanuc Honda MR-50 50-pin female connector (probably for external connection of the Fanuc Program File Unit)
      CN6     - Fanuc Honda MR-20 20-pin female connector for the keyboard
      CN7     - Fanuc Honda MR-20 20-pin male. Specification says 'not used' and no signals are present on the connector
      CN8     - 20-pin flat cable joining to CRT Unit (video output)
      CN9     - 34-pin flat cable joining to FDD Unit
      CN10    - Power input connector
      CN11/12 - Fanuc Honda MR-50 50-pin female connector for factory testing (not used)
      Sx      - 7 2-pin jumpers (S1 to S7). S2, S3 & S4 are open and the others are shorted
      D39     - Bank of 8 2-pin jumpers vertically orientated. 2 and 7 are shorted and the others are open


Block Diagram
-------------
Below is the block diagram shown in the System P-Model G Maintenance Manual, relating to the mono version.
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
              |      |                          |         | BOOT   |<-------->|     |----------------|
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

    To boot a floppy put "bp fc5fa,1,{ip=c682;g}" and "bp fc6d7,1,{ip=c755;g}"
    into the debugger.

    At NMI: f8008 must have bit 7 clear and bit 6 set (e008 on 8085)
            f8009 must not equal 0x01 (e009 on 8085)

            8085 sets f8008 to keyboard row 0 AND 0xf3
             "     "  f8009 to keyboard row 1


Keyboard Matrix (preliminary)
---------------
Row select
|         Columns Key by bit
|         D0   D1   D2   D3   D4   D5   D6   D7
V         V    V    V    V    V    V    V    V
0x??      F0   F1   F2   F3   F4   F5   F6   F7
0x??      F8   F9   F10  F11  F12  F13  F14  F15
0x30      0    1    2    3    4    5    6    7
0x38      8    9    :    ;    <    -    >    ?
0x40      @    A    B    C    D    E    F    G
0x48      H    I    J    K    L    M    N    O
0x50      P    Q    R    S    T    U    V    W
0x58      X    Y    Z    [    Yen  ]    ^    _
0x??      KP0  KP1  KP2  KP3  KP4  KP5  KP6  KP7
0x??      KP8  KP9  KP.  KP,  N/A  N/A  N/A  SPACE
0x??      K+   K-   K*   K/   N/A  N/A  N/A  DEL
0x??      BS  (K)NL CAN  N/A  N/A  N/A  N/A  N/A
0x??      K0   K1   K2   K3   N/A  N/A  N/A  N/A

The following keys I have no idea where they map as they don't show a consistent column bit in the diagram:
LOAD UC LC R0 R1 R2 R3
Also any keys which are N/A may actually have something else mapped there.

In short: this keyboard seems to follow some sort of ASCII-derived row/column pattern
If this is a true ascii keyboard, then UC, LC probably do not connect to the matrix at all,
but instead make it so rows 0x40, 0x48 0x50 and 0x58 produce characters from ascii rows 0x60, 0x68, 0x70 and 0x78 (uppercase becomes lowercase)
likewise 0x30 and 0x38 will produce chars from 0x20 and 0x28 (numbers become symbols)
the keypad symbols seem to use a different matrix pattern from the rest?

****************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "cpu/i86/i86.h"
#include "imagedev/floppy.h"
#include "machine/i8251.h"
#include "machine/i8257.h"
#include "machine/i8087.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "machine/upd765.h"
#include "video/mc6845.h"

#include "screen.h"


namespace {

#define MAINCPU_TAG "maincpu"
#define SUBCPU_TAG  "subcpu"
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
		, m_usart(*this, "usart%u", 0U)
		, m_pit(*this, "pit%u", 0U)
		, m_pic(*this, "pic%u", 0U)
		, m_dmac(*this, DMAC_TAG)
		, m_crtc(*this, CRTC_TAG)
		, m_fdc(*this, FDC_TAG)
		, m_shared(*this, SHARED_TAG)
		, m_chargen(*this, CHARGEN_TAG)
	{ }

	void fanucspmgm(machine_config &config);
	void fanucspmg(machine_config &config);

	void init_fanucspmg();

private:
	required_device<i8086_cpu_device> m_maincpu;
	required_device<i8085a_cpu_device> m_subcpu;
	required_device_array<i8251_device, 4> m_usart;
	required_device_array<pit8253_device, 2> m_pit;
	required_device_array<pic8259_device, 2> m_pic;
	required_device<i8257_device> m_dmac;
	required_device<mc6845_device> m_crtc;
	required_device<upd765a_device> m_fdc;
	required_shared_ptr<uint8_t> m_shared;
	required_memory_region m_chargen;

	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);
	uint8_t shared_r(offs_t offset);
	void shared_w(offs_t offset, uint8_t data);
	uint8_t vram1_r(offs_t offset);
	void vram1_w(offs_t offset, uint8_t data);
	uint8_t vram2_r(offs_t offset);
	void vram2_w(offs_t offset, uint8_t data);
	void vram_bank_w(uint8_t data);
	uint8_t vblank_ack_r();
	void vbl_ctrl_w(uint8_t data);
	void keyboard_row_w(uint8_t data);
	uint8_t keyboard_r();
	void video_ctrl_w(uint8_t data);
	uint8_t fdcdma_r();
	void fdcdma_w(uint8_t data);
	uint8_t get_slave_ack(offs_t offset);
	void dma_page_w(uint8_t data);

	uint16_t magic_r();

	void vsync_w(int state);
	void tc_w(int state);
	void hrq_w(int state);

	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_UPDATE_ROW(crtc_update_row_mono);

	uint8_t m_vram[24576];
	uint8_t m_video_ctrl;

	void maincpu_io(address_map &map) ATTR_COLD;
	void maincpu_mem(address_map &map) ATTR_COLD;
	void subcpu_mem(address_map &map) ATTR_COLD;

	virtual void machine_reset() override ATTR_COLD;
	int32_t m_vram_bank;
	uint8_t m_vbl_ctrl;
	uint8_t m_keyboard_row;
	uint8_t m_vbl_stat;
	uint8_t m_dma_page;
};

void fanucspmg_state::init_fanucspmg()
{
	memset(m_vram, 0, sizeof(m_vram));

	save_item(NAME(m_vram));
	save_item(NAME(m_vram_bank));
	save_item(NAME(m_vbl_ctrl));
	save_item(NAME(m_keyboard_row));
	save_item(NAME(m_video_ctrl));
}

uint8_t fanucspmg_state::shared_r(offs_t offset)
{
	return m_shared[offset];
}

void fanucspmg_state::shared_w(offs_t offset, uint8_t data)
{
	m_shared[offset] = data;
}

uint8_t fanucspmg_state::get_slave_ack(offs_t offset)
{
	if(offset == 7)
		return m_pic[1]->acknowledge();

	return 0x00;
}

void fanucspmg_state::tc_w(int state)
{
	m_fdc->tc_w(state);
}

void fanucspmg_state::hrq_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);
	m_dmac->hlda_w(state);
}

uint8_t fanucspmg_state::fdcdma_r()
{
	return m_fdc->dma_r();
}

void fanucspmg_state::fdcdma_w(uint8_t data)
{
	m_fdc->dma_w(data);
}

void fanucspmg_state::dma_page_w(uint8_t data)
{
	floppy_image_device *floppy0 = m_fdc->subdevice<floppy_connector>("0")->get_device();
	floppy_image_device *floppy1 = m_fdc->subdevice<floppy_connector>("1")->get_device();
	// verify
	floppy0->mon_w(!(data & 2));
	floppy1->mon_w(!(data & 2));

	m_dma_page = (data >> 2) & 0xf;
}

uint16_t fanucspmg_state::magic_r()
{
	return 0x0041;  // 31 = memory error
}

void fanucspmg_state::maincpu_mem(address_map &map)
{
	map(0x00000, 0x7ffff).ram();   // main RAM

	map(0x80000, 0x81fff).ram();   // believed to be shared RAM with a CPU inside the Program File
	map(0x88000, 0x88001).noprw();   // Program File "ready" bit

	map(0xf0000, 0xf0003).rw(m_pic[0], FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0xf0004, 0xf0007).m(m_fdc, FUNC(upd765a_device::map)).umask16(0x00ff);
	map(0xf0008, 0xf000f).rw(m_pit[0], FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0xf0010, 0xf0013).rw(m_usart[0], FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0xf0014, 0xf0017).rw(m_usart[1], FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0xf0018, 0xf001b).rw(m_usart[2], FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0xf001c, 0xf001f).rw(m_usart[3], FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0xf0020, 0xf0029).rw(m_dmac, FUNC(i8257_device::read), FUNC(i8257_device::write));
	map(0xf0042, 0xf0043).r(FUNC(fanucspmg_state::magic_r));
	map(0xf0046, 0xf0046).w(FUNC(fanucspmg_state::dma_page_w));
	map(0xf0048, 0xf004f).rw(m_pit[1], FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0xf2000, 0xf2003).rw(m_pic[1], FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);

	map(0xf8000, 0xf9fff).rw(FUNC(fanucspmg_state::shared_r), FUNC(fanucspmg_state::shared_w));
	map(0xfc000, 0xfffff).rom().region(MAINCPU_TAG, 0);
}

void fanucspmg_state::maincpu_io(address_map &map)
{
}

void fanucspmg_state::vsync_w(int state)
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

uint8_t fanucspmg_state::vram1_r(offs_t offset)
{
	return m_vram[m_vram_bank + offset];
}

void fanucspmg_state::vram1_w(offs_t offset, uint8_t data)
{
	m_vram[m_vram_bank + offset] = data;
}

uint8_t fanucspmg_state::vram2_r(offs_t offset)
{
	return m_vram[m_vram_bank + offset + 0x600];
}

void fanucspmg_state::vram2_w(offs_t offset, uint8_t data)
{
	m_vram[m_vram_bank + offset + 0x600] = data;
}

void fanucspmg_state::vram_bank_w(uint8_t data)
{
	m_vram_bank = (data & 7) * 0xc00;
}

uint8_t fanucspmg_state::vblank_ack_r()
{
	m_subcpu->set_input_line(I8085_RST75_LINE, CLEAR_LINE);

	return 0xff;
}

// bit 1 is unknown
// bit 3 appears to enable vblank IRQs
void fanucspmg_state::vbl_ctrl_w(uint8_t data)
{
	m_vbl_ctrl = data;
}

// row 2: raising a bit toggles the corresponding bit at 500a
// row 3: raising a bit toggles the corresponding bit at 500b
void fanucspmg_state::keyboard_row_w(uint8_t data)
{
	m_keyboard_row = data;
}

uint8_t fanucspmg_state::keyboard_r()
{
	return 0;
}

// bit 0 is set when clearing VRAM
// bit 1 is display enable
void fanucspmg_state::video_ctrl_w(uint8_t data)
{
	m_video_ctrl = data;
}

void fanucspmg_state::subcpu_mem(address_map &map)
{
	map(0x0000, 0x3fff).rom().region(SUBCPU_TAG, 0);

	map(0x4000, 0x45ff).rw(FUNC(fanucspmg_state::vram1_r), FUNC(fanucspmg_state::vram1_w));
	map(0x4800, 0x4dff).rw(FUNC(fanucspmg_state::vram2_r), FUNC(fanucspmg_state::vram2_w));

	map(0x5000, 0x5000).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x5001, 0x5001).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x5008, 0x5008).w(FUNC(fanucspmg_state::keyboard_row_w));
	map(0x5009, 0x5009).r(FUNC(fanucspmg_state::keyboard_r));
	map(0x500a, 0x500b).nopw();    // keyboard rows 2 and 3 control what's written here.  dip switches?
	map(0x500c, 0x500c).w(FUNC(fanucspmg_state::vbl_ctrl_w));
	map(0x500d, 0x500d).w(FUNC(fanucspmg_state::vram_bank_w));
	map(0x500e, 0x500e).r(FUNC(fanucspmg_state::vblank_ack_r));
	map(0x5018, 0x5018).w(FUNC(fanucspmg_state::video_ctrl_w));

	map(0xe000, 0xffff).ram().share(SHARED_TAG); // shared RAM
}

/* Input ports */
static INPUT_PORTS_START( fanucspmg )
INPUT_PORTS_END

void fanucspmg_state::machine_reset()
{
	m_vbl_ctrl = 0;
	m_vram_bank = 0;
	m_video_ctrl = 0;
	m_dma_page = 0;
}

uint8_t fanucspmg_state::memory_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset | (m_dma_page << 16));
}

void fanucspmg_state::memory_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.write_byte(offset | (m_dma_page << 16), data);
}

MC6845_UPDATE_ROW( fanucspmg_state::crtc_update_row )
{
	uint32_t *p = &bitmap.pix(y);
	uint8_t const *const chargen = m_chargen->base();

	for ( int i = 0; i < x_count; i++ )
	{
		uint16_t offset = ( ma + i );

		if (m_video_ctrl & 0x02)
		{
			if (offset <= 0x5ff)
			{
				uint8_t chr = m_vram[offset + 0x600];
				uint8_t attr = m_vram[offset];
				uint8_t data = chargen[ chr + (ra * 256) ];
				uint32_t fg = 0;
				uint32_t bg = 0;

				if (attr & 0x20) fg |= 0xff0000;
				if (attr & 0x40) fg |= 0x00ff00;
				if (attr & 0x80) fg |= 0x0000ff;

				*p++ = BIT(data, 0) ? fg : bg;
				*p++ = BIT(data, 1) ? fg : bg;
				*p++ = BIT(data, 2) ? fg : bg;
				*p++ = BIT(data, 3) ? fg : bg;
				*p++ = BIT(data, 4) ? fg : bg;
				*p++ = BIT(data, 5) ? fg : bg;
				*p++ = BIT(data, 6) ? fg : bg;
				*p++ = BIT(data, 7) ? fg : bg;
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

MC6845_UPDATE_ROW( fanucspmg_state::crtc_update_row_mono )
{
	uint32_t *p = &bitmap.pix(y);
	uint8_t const *const chargen = m_chargen->base();

	for ( int i = 0; i < x_count; i++ )
	{
		uint16_t offset = ( ma + i );

		if (m_video_ctrl & 0x02)
		{
			if (offset <= 0x5ff)
			{
				uint8_t chr = m_vram[offset + 0x600];
//              uint8_t attr = m_vram[offset];
				uint8_t data = chargen[ chr + (ra * 256) ];
				uint32_t fg = 0xff00;
				uint32_t bg = 0;

				*p++ = BIT(data, 0) ? fg : bg;
				*p++ = BIT(data, 1) ? fg : bg;
				*p++ = BIT(data, 2) ? fg : bg;
				*p++ = BIT(data, 3) ? fg : bg;
				*p++ = BIT(data, 4) ? fg : bg;
				*p++ = BIT(data, 5) ? fg : bg;
				*p++ = BIT(data, 6) ? fg : bg;
				*p++ = BIT(data, 7) ? fg : bg;
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

static void fanuc_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

void fanucspmg_state::fanucspmg(machine_config &config)
{
	/* basic machine hardware */
	I8086(config, m_maincpu, XTAL(15'000'000)/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &fanucspmg_state::maincpu_mem);
	m_maincpu->set_addrmap(AS_IO, &fanucspmg_state::maincpu_io);
	m_maincpu->set_irq_acknowledge_callback("pic0", FUNC(pic8259_device::inta_cb));
	m_maincpu->esc_opcode_handler().set("i8087", FUNC(i8087_device::insn_w));
	m_maincpu->esc_data_handler().set("i8087", FUNC(i8087_device::addr_w));

	i8087_device &i8087(I8087(config, "i8087", XTAL(15'000'000)/3));
	i8087.set_space_86(m_maincpu, AS_PROGRAM);
	//i8087.irq().set_inputline("maincpu", INPUT_LINE_NMI);  // TODO: presumably this is connected to the pic
	i8087.busy().set_inputline("maincpu", INPUT_LINE_TEST);

	I8085A(config, m_subcpu, XTAL(16'000'000)/2/2);
	m_subcpu->set_addrmap(AS_PROGRAM, &fanucspmg_state::subcpu_mem);

	I8251(config, m_usart[0], 0);
	I8251(config, m_usart[1], 0);
	I8251(config, m_usart[2], 0);
	I8251(config, m_usart[3], 0);

	PIT8253(config, m_pit[0], 0);
	m_pit[0]->set_clk<0>(XTAL(15'000'000)/12);
	m_pit[0]->set_clk<1>(XTAL(15'000'000)/12);
	m_pit[0]->set_clk<2>(XTAL(15'000'000)/12);

	PIT8253(config, m_pit[1], 0);
	m_pit[1]->set_clk<0>(XTAL(15'000'000)/12);
	m_pit[1]->set_clk<1>(XTAL(15'000'000)/12);
	m_pit[1]->set_clk<2>(XTAL(15'000'000)/12);

	I8257(config, m_dmac, XTAL(15'000'000) / 5);
	m_dmac->out_hrq_cb().set(FUNC(fanucspmg_state::hrq_w));
	m_dmac->out_tc_cb().set(FUNC(fanucspmg_state::tc_w));
	m_dmac->in_memr_cb().set(FUNC(fanucspmg_state::memory_read_byte));
	m_dmac->out_memw_cb().set(FUNC(fanucspmg_state::memory_write_byte));
	m_dmac->in_ior_cb<0>().set(FUNC(fanucspmg_state::fdcdma_r));
	m_dmac->out_iow_cb<0>().set(FUNC(fanucspmg_state::fdcdma_w));

	PIC8259(config, m_pic[0], 0);
	m_pic[0]->out_int_callback().set_inputline(m_maincpu, 0);
	m_pic[0]->in_sp_callback().set_constant(1);
	m_pic[0]->read_slave_ack_callback().set(FUNC(fanucspmg_state::get_slave_ack));

	PIC8259(config, m_pic[1], 0);
	m_pic[1]->out_int_callback().set(m_pic[0], FUNC(pic8259_device::ir7_w));
	m_pic[1]->in_sp_callback().set_constant(0);

	UPD765A(config, m_fdc, 8'000'000, true, true);
	m_fdc->intrq_wr_callback().set(m_pic[0], FUNC(pic8259_device::ir3_w));
	m_fdc->drq_wr_callback().set(m_dmac, FUNC(i8257_device::dreq0_w));
	FLOPPY_CONNECTOR(config, FDC_TAG":0", fanuc_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, FDC_TAG":1", fanuc_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats);

	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(15'000'000), 640, 0, 512, 390, 0, 384);
	screen.set_screen_update(CRTC_TAG, FUNC(mc6845_device::screen_update));

	HD6845S(config, m_crtc, XTAL(8'000'000)/2);
	m_crtc->set_screen(SCREEN_TAG);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(fanucspmg_state::crtc_update_row));
	m_crtc->out_vsync_callback().set(FUNC(fanucspmg_state::vsync_w));
}

void fanucspmg_state::fanucspmgm(machine_config &config)
{
	fanucspmg(config);

	m_crtc->set_update_row_callback(FUNC(fanucspmg_state::crtc_update_row_mono));
}

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

ROM_START( fanucspgm )
	ROM_REGION(0x4000, MAINCPU_TAG, 0)
	ROM_LOAD16_BYTE( "a25_001a.33e", 0x000000, 0x002000, CRC(81159267) SHA1(f5d53cc6e929f57e8c3747f80fc74d4b1643222d) )
	ROM_LOAD16_BYTE( "a25_002a.35e", 0x000001, 0x002000, CRC(4fb82c4d) SHA1(eb75e9a2d3c8e4ad56a74624ee8c52c785bd0da6) )

	ROM_REGION(0x4000, SUBCPU_TAG, 0)
	ROM_LOAD( "a21_010f.17d", 0x000000, 0x002000, CRC(ef192717) SHA1(7fb3f7ca290d2437ae5956700f88c801018ce1cc) )

	ROM_REGION(0x8000, CHARGEN_TAG, 0)
	ROM_LOAD( "a22_020b.5g",  0x000000, 0x002000, CRC(7b5f8e20) SHA1(9de607e541d8aad2d1ea56321270bb8466b16e3d) )
ROM_END

} // anonymous namespace


/* Driver */
//    YEAR  NAME       PARENT    COMPAT  MACHINE     INPUT      CLASS            INIT            COMPANY  FULLNAME                         FLAGS
COMP( 1983, fanucspg,  0,        0,      fanucspmg,  fanucspmg, fanucspmg_state, init_fanucspmg, "Fanuc", "System P Model G",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1983, fanucspgm, fanucspg, 0,      fanucspmgm, fanucspmg, fanucspmg_state, init_fanucspmg, "Fanuc", "System P Model G (monochrome)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
