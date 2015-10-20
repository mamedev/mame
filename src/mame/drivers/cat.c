// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Jonathan Gevaryahu
/***************************************************************************

    Canon Cat, Model V777
    IAI Swyft Model P0001
    Copyright (C) 2009-2013 Miodrag Milanovic and Jonathan Gevaryahu AKA Lord Nightmare
    With information and help from John "Sandy" Bumgarner, Dwight Elvey,
    Charles Springer, Terry Holmes, Jonathan Sand, Aza Raskin and others.


    This driver is dedicated in memory of Jef Raskin and Dave Boulton

    12/06/2009 Skeleton driver.
    15/06/2009 Working driver

Pictures: http://www.regnirps.com/Apple6502stuff/apple_iie_cat.htm

Canon cat blue "Use Front Labels":
` (really degree, +-, paragraph-mark and section-mark): LEFT MARGIN
-_: INDENT
+=: RIGHT MARGIN
UNDO: SPELL CHECK LEAP (this is actually a red LEAP label)
PERM SPACE/TAB: SET/CLEAR TAB
Q: UNDER LINE
W: BOLD
E: CAPS
T: Paragraph STYLE
U: LINE SPACE
O: ADD SPELLING
[ (really 1/4, 1/2, double-underline and |): SETUP
Backspace (really ERASE): ANSWER
LOCK: DOCUMENT LOCK
A: COPY
D: SEND CONTROL
G: CALC
J: PRINT
L: DISK
Colon: FORMAT/WIPE DISK (only when wheel! mode is enabled, see below)
"': PHONE
Enter (really RETURN): SEND
X: LOCAL LEAP
V: LEARN
N: EXPLAIN
,: SORT
?/: KBI/II
Pgup (really DOCUMENT/PAGE): TITLES
Leap Left: LEAP AGAIN (left)
Leap Right: LEAP AGAIN (right)


How to enable the FORTH interpreter (http://canoncat.org/canoncat/enableforth.html)

The definitive instructions of going Forth on a Cat, tested on a living Cat.
The Canon Cat is programmed with FORTH, a remarkably different programming language in itself.
On the Canon Cat, you can access the FORTH interpreter with a special sequence of key presses.
Here are exact working instructions thanks to Sandy Bumgarner:
- Type Enable Forth Language exactly like that, capitals and all.
- Highlight from the En to the ge, exactly [i.e. Leap back to Enable by keeping the left Leap
  key down while typing E n a, release the leap key, then click both leap keys simultaneously],
  and press USE FRONT with ERASE (the ANSWER command).
- The Cat should beep and flash the ruler.
- Then press USE FRONT and the SHIFT keys together and tap the space bar. Note that the cursor
  stops blinking. Now a Pressing the RETURN key gets the Forth OK and you are 'in' as they say.

In MESS, to activate it as above:
* when the Cat boots, type (without quotes) "Enable Forth Language"
* hold left-alt(leap left) and type E n a, release left-alt (the left cursor is now at the first character)
* simultaneously press both alt keys for a moment and release both (the whole "Enable Forth Language" line will be selected)
* press control(use front) and press backspace(ERASE) (The cat will beep here)
* press control(use front), shift, and space (the cursor should stop blinking)
* press enter and the forth "ok" prompt should appear. you can type 'page' and enter to clear the screen
Optional further steps:
* type without quotes "-1 wheel! savesetup re" at the forth prompt to permanently
  enable shift + use front + space to dump to forth mode easily
* change the keyboard setting in the setup menu (use front + [ ) to ASCII so you can type < and >
* after doing the -1 wheel! thing, you can compile a selected forth program in the editor
  by selecting it and hitting ANSWER (use front + ERASE)

If ever in forth mode you can return to the editor with the forth word (without quotes) "re"

Canon cat gate array ASIC markings:
GA1 (prototype): D65013CW276 [same as final]
GA1 (final): NH4-5001 276 [schematic: upD65013CW-276]
GA2 (prototype): D65013CW208 [DIFFERENT FROM FINAL! larger asic used here, shrank for final?]
GA2 (final): NH4-5002 191 [schematic: upD65012CW-191]
GA3 (prototype): D65013CW141 [same as final? typo 65013 vs 65012?]
GA3 (final): NH4-5003 141 [schematic: upD65012CW-141]

Canon cat credits easter egg:
* hold either leap key, then simultaneously hold shift, then type Q W E R A S D F Z X C V and release all the keys
* hit EXPLAIN (use front + N) and the credits screen will be displayed

Canon Cat credits details: (WIP)
Scott Kim - responsible for fonts on swyft and cat
Ralph Voorhees - Model construction and mockups (swyft 'flat cat')

Cat HLSL stuff:
*scanlines:
the cat has somewhat visible and fairly close scanlines with very little fuzziness
try hlsl options:
hlsl_prescale_x           4
hlsl_prescale_y           4
scanline_alpha            0.3
scanline_size             1.0
scanline_height           0.7
scanline_bright_scale     1.0
scanline_bright_offset    0.6
*phosphor persistence of the original cat CRT is VERY LONG and fades to a greenish-yellow color, though the main color itself is white
try hlsl option:
phosphor_life             0.93,0.95,0.87
which is fairly close but may actually be too SHORT compared to the real thing.


Swyft versions:
There are at least 4 variants of machines called 'swyft':
* The earliest desktop units which use plexi or rubber-tooled case and an
  angled monitor; about a dozen were made and at least two of clear plexi.
  These are sometimes called "wrinkled" swyfts. 5.25" drive, they may be able
  to read Apple2 Swyftware/Swyftdisk and Swyftcard-created disks.
  It is possible no prototypes of this type got beyond the 'runs forth console only' stage.
  http://archive.computerhistory.org/resources/access/physical-object/2011/09/102746929.01.01.lg.JPG
  http://www.digibarn.com/collections/systems/swyft/Swyft-No2-05-1271.jpg
  http://www.digibarn.com/friends/jef-raskin/slides/iai/A%20-687%20SWYFTPRO.JPG
* The early "flat cat" or "roadkill" portable LCD screen units with a white
  case and just a keyboard. Model SP0001
  http://www.digibarn.com/collections/systems/swyft/Image82.jpg
* The later "ur-cat" desktop units which use a machine tooled case and look
  more or less like the canon cat. Around 100-200 were made. 3.5" drive.
  These have a fully functional EDDE editor as the cat does, and can even compile
  forth programs.
  (the 'swyft' driver is based on one of these)
* The very late portable LCD units with a dark grey case and a row of hotkey
  buttons below the screen. Not dumped yet. At least one functional prototype exists.
  At least one plastic mockup exists with no innards.
  http://www.digibarn.com/collections/systems/swyft/swyft.jpg

Canon Cat versions:
There is really only one version of the cat which saw wide release, the US version.
* It is possible a very small number of UK/European units were released as a test.
  If so, these will have slightly different keyboard key caps and different
  system and spellcheck roms.

As for prototypes/dev cat machines, a few minor variants exist:
* Prototype cat motherboards used 16k*4bit drams instead of 64k*4bit as the
  final system did and hence max out at 128k of dram instead of 512k.
  One of the gate arrays is also different, and the motherboard is arranged
  differently. The IC9 "buserr" PAL is not used, even on the prototype.
  The final system included 256k of dram and can be upgraded to 512k.
* At least some developer units were modified to have an external BNC
  connector, ostensibly to display the internal screen's video externally.
  http://www.digibarn.com/collections/systems/canon-cat/Image55.jpg


Canon Cat:
<insert guru-diagram here once drawn>
Crystals:
X1: 19.968Mhz, used by GA2 (plus a PLL to multiply by 2?), and divide by 4 for
    cpuclk, divide by 8 for 2.5mhz and divide by 5.5 for 3.63mhz (is this
    supposed to be divide by 6? there may not be a pll if it is...)
X2: 3.579545Mhz, used by the DTMF generator chip AMI S2579 at IC40
X3: 2.4576Mhz, used by the modem chip AMI S35213 at IC37

IAI Swyft:
Board name: 950-0001C
"INFORMATION APPLIANCE INC. COPYRIGHT 1985"
 _________________|||||||||___________________________________________________________________________________
|                     J8               [=======J3=======]  ____                              Trans-           |
==                                                        /    \  (E2)                       former           |
==Jx                   74LS107   J5                       |PB1 |        uA339     MC3403                 -----|
|                                          7407           \____/                                         J7   =
| Y1       "TIMING B" 74LS132    74LS175                                                                 -----|
|                                                  ____________                            4N37  Relay   -----|
| TMS4256   74LS161  "DECODE E" "DISK 3.5C"       |            |                                         J6   =
|                                                 |  MC6850P   |                                         -----|
| TMS4256   74LS166   74HCT259   74LS299          '------------'        MC3403    MC3403                 _____|
|                      ___________________     ___________________             ||                       |     =
| TMS4256   74LS373   |                   |   |                   |            J2                       | J1  =
|                     |   MC68008P8       |   |       R6522P      |            ||              I   P R  |     =
| TMS4256   74F153    '-------------------'   '-------------------'     MN4053 || MN4053       N   R E  | B   =
|                                    (E1)                                                      D   O S  | R   =
| TMS4256   74F153    74HCT08     __________   ___________________      MC14412   DS1489       U E T I  | E   =
|                                |          | |                   | ||                         C S E S  | A   =
| TMS4256   74F153    74HC4040E  | AM27256  | |       R6522P      | ||                         T D C T  | K   =
|                                '----------' '-------------------' ||  INFORMATION            O   T O  | O   =
| TMS4256   74F153    "VIDEO 2B" .----------.                       J4  APPLIANCE INC.         R   I R  | U   =
|                                | AM27256  |   74HC02     74HC374  ||  Copyright 1985         S   O S  | T   =
| TMS4256   74F153    74LS393  B1|__________|                       ||  UM95089  Y2                N    |     =
|_____________________________[________J9___]__________________________________________________D13______|_____=

*Devices of interest:
J1: breakout of joystick, serial/rs232, hex-keypad, parallel port, and forth switch (and maybe cassette?) pins
    DIL 60 pin 2-row right-angle rectangular connector with metal shield contact;
    not all 60 pins are populated in the connector, only pins 1-6, 8, 13-15, 17-18, 23-30, 35-60 are present
    (traced partly by dwight)
J2: unpopulated 8-pin sip header, serial/rs232-related?
    (vcc ? ? ? ? ? ? gnd) (random guess: txd, rxd, rts, cts, dsr, dtr, and one pin could be cd/ri though the modem circuit may do that separately?)
J3: Floppy Connector
    (standard DIL 34 pin 2-row rectangular connector for mini-shugart/pc floppy cable; pin 2 IS connected somewhere and ?probably? is used for /DISKCHANGE like on an Amiga, with pin 34 being /TRUEREADY?)
    (as opposed to normal ibm pc 3.5" drives where pin 2 is unconnected or is /DENSITY *input to drive*, and pin 34 is /DISKCHANGE)
J4: 18-pin sip header for keyboard ribbon cable; bottom edge of board is pin 1
    Pins:
    1: GND through 220k resistor r78
    2: ? phone hook related? anode of diode d7; one of the pins of relay k2; topmost (boardwise) pin of transistor Q10
    3: 74HCT34 pin

J5: locking-tab-type "CONN HEADER VERT 4POS .100 TIN" connector for supplying power
    through a small cable with a berg connector at the other end, to the floppy drive
    (5v gnd gnd 12v)
J6: Phone connector, rj11 jack
J7: Line connector, rj11 jack
J8: 9-pin Video out/power in connector "CONN RECEPT 6POS .156 R/A PCB" plus "CONN RECEPT 3POS .156 R/A PCB" acting as one 9-pin connector
    (NC ? ? ? NC NC ? 12v 5v) (video, vsync, hsync and case/video-gnd are likely here)
    (the video pinout of the cat is: (Video Vsync Hsync SyncGnd PwrGnd PwrGnd +5v(VCC) +12v(VDD) -12v(VEE)) which is not the same as the swyft.
J9: unpopulated DIL 40-pin straight connector for a ROM debug/expansion/RAM-shadow daughterboard
    the pins after pin 12 connect to that of the ROM-LO 27256 pinout counting pins 1,28,2,27,3,26,etc
    the ROM-HI rom has a different /HICE pin which is not connected to this connector
    /LOCE is a15
    /HICE is !a15
    /ROM_OE comes from pin 14 of DECODE_E pal, and is shorted to /ROM_OE' by the cuttable jumper B1 which is not cut
    /ROM_OE' goes to the two EPROMS
    DECODE_18 is DECODE_E pal pin 18
    pin 1 (GND) is in the lower left and the pins count low-high then to the right
    (gnd N/C   E_CLK     R/W    /ROM_OE a17 vcc a14 a13 a8 a9 a11 /ROM_OE' a10 a15 d7 d6 d5 d4 d3 )
    (GND /IPL1 DECODE_18 /RESET gnd     a16 vcc a12 a7  a6 a5 a4  a3       a2  a1  a0 d0 d1 d2 gnd)
Jx: 4 pin on top side, 6 pin on bottom side edge ?debug? connector (doesn't have a Jx number)
    (trace me!)
B1: a cuttable trace on the pcb. Not cut, affects one of the pins on the unpopulated J9 connector only.
E1: jumper, unknown purpose, not set
E2: jumper, unknown purpose, not set
D13: LED
R6522P (upper): parallel port via
R6522P (lower): keyboard via
UM95089: DTMF Tone generator chip (if you read the datasheet this is technically ROM based!)
Y1: 15.8976Mhz, main clock?
Y2: 3.579545Mhz, used by the DTMF generator chip UM95089 (connects to pins 7 and 8 of it)
TMS4256-15NL - 262144 x 1 DRAM
PB1 - piezo speaker

*Pals:
"TIMING B" - AMPAL16R4APC (marked on silkscreen "TIMING PAL")
"DECODE E" - AMPAL16L8PC (marked on silkscreen "DECODE PAL")
"VIDEO 2B" - AMPAL16R4PC (marked on silkscreen "VIDEO PAL")
"DISK 3.5C" - AMPAL16R4PC (marked on silkscreen "DISK PAL")

*Deviations from silkscreen:
4N37 (marked on silkscreen "4N35")
74F153 (marked on silkscreen "74ALS153")
74HCT259 is socketed, possibly intended that the rom expansion daughterboard will have a ribbon cable fit in its socket?


ToDo:
* Canon Cat
- Find the mirrors for the write-only video control register and figure out
  what the writes actually do; hook these up properly to screen timing etc
- Floppy drive (3.5", Single Sided Double Density MFM, ~400kb)
  * Cat has very low level control of data being read or written, much like
    the Amiga or AppleII does
  * first sector is id ZERO which is unusual since most MFM formats are base-1
    for sector numbering
  * sectors are 512 bytes, standard MFM with usual address and data marks and
    ccitt crc16
  * track 0 contains sector zero repeated identically 10 times; the cat SEEMS
    to use this as a disk/document ID
  * tracks 1-79 each contain ten sectors, id 0x0 thru 0x9
    ('normal' mfm parlance: sectors -1, 0, 1, ... 7, 8)
  * this means the whole disk effectively contains 512*10*80 = 409600 bytes of
    data, though track 0 is just a disk "unique" identifier for the cat
    meaning 404480 usable bytes
  * (Once the floppy is working I'd declare the system working)
- Centronics port finishing touches: verify where the paper out, slct/err, and
  IPP pins map in memory. The firmware doesn't actually use them, but they must
  map somewhere as they connect to the ASIC.
- RS232C port and Modem "port" connected to the DUART's two ports
  These are currently optionally debug-logged but don't connect anywhere
- DTMF generator chip (connected to DUART 'user output' pins OP4,5,6,7)
- WIP: Watchdog timer/powerfail at 0x85xxxx (watchdog NMI needs to actually
  fire if wdt goes above a certain number, possibly 3, 7 or F?)
- Canon Cat released versions known: 1.74 US (dumped), 2.40 US (dumped both
  original compile, and Dwight's recompile from the released source code),
  2.42 (NEED DUMP)
  It is possible a few prototype UK 1.74 or 2.40 units were produced; the code
  roms of these will differ (they contain different spellcheck "core" code) as
  well as the spellcheck roms, the keyboard id and the keycaps.
- Known Spellcheck roms: NH7-0684 (US, dumped); NH7-0724 (UK, NEED DUMP);
  NH7-0813/0814 (Quebec/France, NEED DUMP); NH7-1019/1020/1021 (Germany, NEED DUMP)
  It is possible the non-US roms were never officially released.
  Wordlist sources: American Heritage (US and UK), Librarie Larousse (FR),
  Langenscheidt (DE)
- (would-be-really-nice-but-totally-unnecessary feature): due to open bus, the
  svrom1 and svrom2 checksums in diagnostics read as 01A80000 and 01020000
  respectively on a real machine (and hence appear inverted/'fail'-state).
  This requires sub-cycle accurate 68k open bus emulation to pull off, as well
  as emulating the fact that UDS/LDS are ?not connected? (unclear because this
  happens inside an asic) for the SVROMS (or the svram or the code roms, for
  that matter!)
- Hook Battery Low input to a dipswitch.
- Hook pfail to a dipswitch.
- Hook the floppy control register readback up properly, things seem to get
  confused.


* Swyft
- Figure out the keyboard (interrupts are involved? or maybe an NMI on a
  timer/vblank? It is possible this uses a similar 'keyboard read int'
  to what the cat does)
- get the keyboard scanning actually working; the VIAs are going nuts right now.
- Beeper (on one of the vias?)
- vblank/hblank stuff
- Get the 6850 ACIA working for communications
- Floppy (probably similar to the Cat)
- Centronics port (attached to one of the VIAs)
- Joystick port (also likely on a via)
- Keypad? (also likely on a via done as a grid scan?)
- Forth button (on the port on the back; keep in mind shift-usefront-space ALWAYS enables forth on a swyft)
- Multple undumped firmware revisions exist (330 and 331 are dumped)

// 74ls107 @ u18 pin 1 is 68008 /BERR pin

// mc6850 @ u33 pin 2 (RX_DATA) is
// mc6850 @ u33 pin 3 (RX_CLK) is 6522 @ u35 pin 17 (PB7)
// mc6850 @ u33 pin 4 (TX_CLK) is 6522 @ u35 pin 17 (PB7)
// mc6850 @ u33 pin 5 (/RTS) is
// mc6850 @ u33 pin 6 (TX_DATA) is
// mc6850 @ u33 pin 7 (/IRQ) is 68008 /IPL1 pin 41
// mc6850 @ u33 pin 8 (CS0) is 68008 A12 pin 10
// mc6850 @ u33 pin 9 (CS2) is DECODE E pin 18
// mc6850 @ u33 pin 10 (CS1) is 68008 /BERR pin 40
// mc6850 @ u33 pin 11 (RS) is 68008 A0 pin 46
// mc6850 @ u33 pin 13 (R/W) is 68008 R/W pin 30
// mc6850 @ u33 pin 14 (E) is 68008 E pin 38
// mc6850 @ u33 pin 15-22 (D7-D0) are 68008 D7 to D0 pins 20-27
// mc6850 @ u33 pin 23 (/DCD) is 74hc02 @ u35 pin 1
// mc6850 @ u33 pin 24 (/CTS) is N/C?

// 6522 @ u34:
// pin 2 (PA0) :
// pin 3 (PA1) :
// pin 4 (PA2) :
// pin 5 (PA3) :
// pin 6 (PA4) :
// pin 7 (PA5) :
// pin 8 (PA6) :
// pin 9 (PA7) :
// pin 10 (PB0) :
// pin 11 (PB1) :
// pin 12 (PB2) :
// pin 13 (PB3) :
// pin 14 (PB4) :
// pin 15 (PB5) :
// pin 16 (PB6) :
// pin 17 (PB7) :
// pin 18 (CB1) : ?from/to? Floppy connector j3 pin 8
// pin 19 (CB2) : ?from/to? 6522 @ u35 pin 16 (PB6)
// pin 21 (/IRQ) : out to 68008 /IPL1 pin 41
// pin 22 (R/W) : in from 68008 R/W pin 30
// pin 23 (/CS2) : in from DECODE E pin 18
// pin 24 (CS1) : in from 68008 A13 pin 11
// pin 25 (Phi2) : in from 68008 E pin 38
// pins 26-33 : in/out from/to 68008 D7 to D0 pins 20-27
// pin 34 (/RES) : in from 68008 /RESET pin 37 AND 68008 /HALT pin 36
// pins 35-38 (RS3-RS0) are 68008 A9-A6 pins 7-4
// pin 39 (CA2) is through inductor L11 and resistor r128 to peripheral connector pin 35 <end minus 26>
// pin 40 (CA1) is through inductor L30 and resistor r138 to peripheral connector pin 53 <end minus 8>

// 6522 @ u35
// pin 2 (PA0) :
// pin 3 (PA1) :
// pin 4 (PA2) :
// pin 5 (PA3) :
// pin 6 (PA4) :
// pin 7 (PA5) :
// pin 8 (PA6) :
// pin 9 (PA7) :
// pin 10 (PB0) :
// pin 11 (PB1) : in from 74hc02 @ u36 pin 4
// pin 12 (PB2) :
// pin 13 (PB3) :
// pin 14 (PB4) :
// pin 15 (PB5) :
// pin 16 (PB6) : 6522 @ u34 pin 19 (CB2)
// pin 17 (PB7) : mc6850 @ u33 pins 3 and 4 (RX_CLK, TX_CLK)
// pin 18 (CB1) : ds1489an @ u45 pin 11
// pin 19 (CB2) : mn4053b @ u40 pin 11 and mc14412 @ u41 pin 10
// pin 21 (/IRQ) : out to 68008 /IPL1 pin 41
// pin 22 (R/W) : in from 68008 R/W pin 30
// pin 23 (/CS2) : in from DECODE E pin 18
// pin 24 (CS1) : in from 68008 A14 pin 12
// pin 25 (Phi2) : in from 68008 E pin 38
// pins 26-33 : in/out from/to 68008 D7 to D0 pins 20-27
// pin 34 (/RES) : in from 68008 /RESET pin 37 AND 68008 /HALT pin 36
// pins 35-38 (RS3-RS0) : in from 68008 A9-A6 pins 7-4
// pin 39 (CA2) : out to 74HCT34 pin 11 (CLK) (keyboard column latch)
// pin 40 (CA1) : out? to? ds1489an @ u45 pin 8

// 74hc02 @ u36:
// pin 1 (Y1) : out to mc6850 @ u33 pin 23 /DCD
// pin 2 (A1) : in from (2 places: resistor R58 to ua339 @ u38 pin 4 (In1-)) <where does this actually come from? modem offhook?>
// pin 3 (B1) : in from mn4053b @ u40  pin 10 <probably from rs232>
// pin 4 (Y2) : out to 6522 @u35 pin 11
// pin 5 (A2) : in from 4N37 @ u48 pin 5 (output side emitter pin) (tied via R189 to gnd) <ring indicator?>
// pin 6 (B2) : in from 4N37 @ u48 pin 5 (output side emitter pin) (tied via R189 to gnd) <ring indicator?>
// pin 8 (B3) :
// pin 9 (A3) :
// pin 10 (Y3) :
// pin 11 (B4) : in from 68008 A15
// pin 12 (A4) : in from 68008 A15
// pin 13 (Y4) : out to EPROM @ U31 /CE

****************************************************************************/

// Defines

#undef DEBUG_GA2OPR_W
#undef DEBUG_VIDEO_CONTROL_W

#undef DEBUG_FLOPPY_CONTROL_W
#undef DEBUG_FLOPPY_CONTROL_R
#undef DEBUG_FLOPPY_DATA_W
#undef DEBUG_FLOPPY_DATA_R
#undef DEBUG_FLOPPY_STATUS_R

#undef DEBUG_PRINTER_DATA_W
#undef DEBUG_PRINTER_CONTROL_W

#undef DEBUG_MODEM_R
#undef DEBUG_MODEM_W

#undef DEBUG_DUART_OUTPUT_LINES
// data sent to rs232 port
#undef DEBUG_DUART_TXA
// data sent to modem chip
#undef DEBUG_DUART_TXB
#undef DEBUG_DUART_IRQ_HANDLER
#undef DEBUG_PRN_FF

#undef DEBUG_TEST_W

#define DEBUG_SWYFT_VIA0 1
#define DEBUG_SWYFT_VIA1 1


// Includes
#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/clock.h"
#include "machine/mc68681.h"
#include "machine/6850acia.h"
#include "machine/6522via.h"
#include "machine/nvram.h"
#include "sound/speaker.h"
#include "bus/centronics/ctronics.h"

class cat_state : public driver_device
{
public:
	enum
	{
		TIMER_KEYBOARD,
		TIMER_COUNTER_6MS
	};

	cat_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		//m_nvram(*this, "nvram"), // merge with svram?
		m_duart(*this, "duartn68681"),
		m_ctx(*this, "ctx"),
		m_ctx_data_out(*this, "ctx_data_out"),
		m_acia6850(*this, "acia6850"),
		m_via0(*this, "via6522_0"),
		m_via1(*this, "via6522_1"),
		m_speaker(*this, "speaker"),
		m_svram(*this, "svram"), // nvram
		m_p_cat_videoram(*this, "p_cat_vram"),
		m_p_swyft_videoram(*this, "p_swyft_vram"),
		m_y0(*this, "Y0"),
		m_y1(*this, "Y1"),
		m_y2(*this, "Y2"),
		m_y3(*this, "Y3"),
		m_y4(*this, "Y4"),
		m_y5(*this, "Y5"),
		m_y6(*this, "Y6"),
		m_y7(*this, "Y7"),
		m_dipsw(*this, "DIPSW1")
		{ }

	required_device<cpu_device> m_maincpu;
	//optional_device<nvram_device> m_nvram;
	optional_device<mc68681_device> m_duart; // only cat uses this
	optional_device<centronics_device> m_ctx;
	optional_device<output_latch_device> m_ctx_data_out;
	optional_device<acia6850_device> m_acia6850; // only swyft uses this
	optional_device<via6522_device> m_via0; // only swyft uses this
	optional_device<via6522_device> m_via1; // only swyft uses this
	optional_device<speaker_sound_device> m_speaker;
	optional_shared_ptr<UINT16> m_svram;
	optional_shared_ptr<UINT16> m_p_cat_videoram;
	optional_shared_ptr<UINT8> m_p_swyft_videoram;
	optional_ioport m_y0;
	optional_ioport m_y1;
	optional_ioport m_y2;
	optional_ioport m_y3;
	optional_ioport m_y4;
	optional_ioport m_y5;
	optional_ioport m_y6;
	optional_ioport m_y7;
	optional_ioport m_dipsw;
	emu_timer *m_keyboard_timer;
	emu_timer *m_6ms_timer;

	DECLARE_MACHINE_START(cat);
	DECLARE_MACHINE_RESET(cat);
	DECLARE_VIDEO_START(cat);
	DECLARE_DRIVER_INIT(cat);
	DECLARE_MACHINE_START(swyft);
	DECLARE_MACHINE_RESET(swyft);
	DECLARE_VIDEO_START(swyft);

	UINT32 screen_update_cat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_swyft(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER(cat_duart_irq_handler);
	DECLARE_WRITE_LINE_MEMBER(cat_duart_txa);
	DECLARE_WRITE_LINE_MEMBER(cat_duart_txb);
	DECLARE_WRITE8_MEMBER(cat_duart_output);
	DECLARE_WRITE_LINE_MEMBER(prn_ack_ff);

	DECLARE_READ16_MEMBER(cat_floppy_control_r);
	DECLARE_WRITE16_MEMBER(cat_floppy_control_w);
	DECLARE_WRITE16_MEMBER(cat_printer_data_w);
	DECLARE_READ16_MEMBER(cat_floppy_data_r);
	DECLARE_WRITE16_MEMBER(cat_floppy_data_w);
	DECLARE_READ16_MEMBER(cat_keyboard_r);
	DECLARE_WRITE16_MEMBER(cat_keyboard_w);
	DECLARE_WRITE16_MEMBER(cat_video_control_w);
	DECLARE_READ16_MEMBER(cat_floppy_status_r);
	DECLARE_READ16_MEMBER(cat_battery_r);
	DECLARE_WRITE16_MEMBER(cat_printer_control_w);
	DECLARE_READ16_MEMBER(cat_modem_r);
	DECLARE_WRITE16_MEMBER(cat_modem_w);
	DECLARE_READ16_MEMBER(cat_6ms_counter_r);
	DECLARE_WRITE16_MEMBER(cat_opr_w);
	DECLARE_READ16_MEMBER(cat_wdt_r);
	DECLARE_WRITE16_MEMBER(cat_tcb_w);
	DECLARE_READ16_MEMBER(cat_2e80_r);
	DECLARE_READ16_MEMBER(cat_0080_r);
	DECLARE_READ16_MEMBER(cat_0000_r);

	DECLARE_READ8_MEMBER(swyft_d0000);

	DECLARE_READ8_MEMBER(swyft_via0_r);
	DECLARE_WRITE8_MEMBER(swyft_via0_w);
	DECLARE_READ8_MEMBER(via0_pa_r);
	DECLARE_WRITE8_MEMBER(via0_pa_w);
	DECLARE_WRITE_LINE_MEMBER(via0_ca2_w);
	DECLARE_READ8_MEMBER(via0_pb_r);
	DECLARE_WRITE8_MEMBER(via0_pb_w);
	DECLARE_WRITE_LINE_MEMBER(via0_cb1_w);
	DECLARE_WRITE_LINE_MEMBER(via0_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(via0_int_w);

	DECLARE_READ8_MEMBER(swyft_via1_r);
	DECLARE_WRITE8_MEMBER(swyft_via1_w);
	DECLARE_READ8_MEMBER(via1_pa_r);
	DECLARE_WRITE8_MEMBER(via1_pa_w);
	DECLARE_WRITE_LINE_MEMBER(via1_ca2_w);
	DECLARE_READ8_MEMBER(via1_pb_r);
	DECLARE_WRITE8_MEMBER(via1_pb_w);
	DECLARE_WRITE_LINE_MEMBER(via1_cb1_w);
	DECLARE_WRITE_LINE_MEMBER(via1_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(via1_int_w);

	DECLARE_WRITE_LINE_MEMBER(write_acia_clock);

	/* gate array 2 has a 16-bit counter inside which counts at 10mhz and
	   rolls over at FFFF->0000; on roll-over (or likely at FFFF terminal count)
	   it triggers the KTOBF output. It does this every 6.5535ms, which causes
	   a 74LS74 d-latch at IC100 to invert the state of the DUART IP2 line;
	   this causes the DUART to fire an interrupt, which makes the 68000 read
	   the keyboard.
	   The watchdog counter and the 6ms counter are both incremented
	   every time the KTOBF pulses.
	 */
	UINT16 m_6ms_counter;
	UINT8 m_wdt_counter;
	UINT8 m_duart_ktobf_ff;
	/* the /ACK line from the centronics printer port goes through a similar
	   flipflop to the ktobf line as well, so duart IP4 inverts on /ACK rising edge
	 */
	UINT8 m_duart_prn_ack_prev_state;
	UINT8 m_duart_prn_ack_ff;
	/* Gate array 2 is in charge of serializing the video for display to the screen;
	   Gate array 1 is in charge of vblank/hblank timing, and in charge of refreshing
	   dram and indicating to GA2, using the /LDPS signal, what times the address it is
	   writing to ram it is intending to be used by GA2 to read 16 bits of data to be
	   shifted out to the screen. (it is obviously not active during hblank and vblank)
	   GA2 then takes: ((output_bit XNOR video_invert) AND video enable), and serially
	   bangs the result to the analog display circuitry.
	 */
	UINT8 m_video_enable;
	UINT8 m_video_invert;
	UINT16 m_pr_cont;
	UINT8 m_keyboard_line;
	UINT8 m_floppy_control;

	//TIMER_CALLBACK_MEMBER(keyboard_callback);
	TIMER_CALLBACK_MEMBER(counter_6ms_callback);
	IRQ_CALLBACK_MEMBER(cat_int_ack);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};

// TODO: this init doesn't actually work yet! please fix me!
/*
DRIVER_INIT_MEMBER( cat_state,cat )
{
    UINT8 *svrom = memregion("svrom")->base();
    int i;
    // fill svrom with the correct 2e80 pattern except where svrom1 sits
    // first half
    for (i = 0; i < 0x20000; i+=2)
        svrom[i] = 0x2E;
    // second half
    for (i = 0x20000; i < 0x40000; i+=2)
    {
        svrom[i] = 0x2E;
        svrom[i+1] = 0x80;
    }
}*/

/* 0x600000-0x65ffff Write: Video Generator (AKA NH4-5001 AKA Gate Array #1 @ IC30)
 writing to the video generator is done by putting the register number in the high 3 bits
 and the data to write in the lower 12 (14?) bits.
 The actual data-bus data written here is completely ignored,
 in fact it isn't even connected to the gate array; The firmware writes 0x0000.
 hence:
 0 = must be 0
 s = register select
 d = data to write
 ? = unknown
 x = ignored
 ? 1 1 ?  ? s s s  s ? ? ?  ? ? ? d  d d d d  d d d .
 600xxx - VSE (End of Frame)
 608xxx - VST (End of VSync)
 610xxx - VSS (VSync Start)
 618xxx - VDE (Active Lines) = value written * 4
 620xxx - unknown
 628xxx - unknown
 630xxx - unknown
 638xxx - unknown
 640xxx - HSE (End H Line)
 648xxx - HST (End HSync)
 650xxx - HSS (HSync Start)
 658xxx - VOC (Video Control)
 */
WRITE16_MEMBER( cat_state::cat_video_control_w )
{
	/*
	 * 006500AE ,          ( HSS HSync Start    89 )
	 * 006480C2 ,          ( HST End HSync   96 )
	 * 006400CE ,          ( HSE End H Line    104 )
	 * 006180B0 ,          ( VDE Active Lines    344 )
	 * 006100D4 ,          ( VSS VSync Start   362 )
	 * 006080F4 ,          ( VST End of VSync    378 )
	 * 00600120 ,          ( VSE End of Frame    400 )
	 * 006581C0 ,          ( VOC Video Control Normal Syncs )
	 * based on diagrams from https://archive.org/details/DTCJefRaskinDoc062
	 * we can determine what at least some of these values do:
	 * HORIZONTAL:
	 *   0 is the first pixel output to the screen at the end of the frontporch
	 *   stuff is shifted to the screen here, and then zeroed for the backporch; manual claims backporch is 2 cycles, it is actually 7
	 *   HSS (89) is the horizontal count at which the HSYNC pin goes high
	 *   sync is active here for 7 cycles; manual claims 10 but is wrong
	 *   HST (96) is the horizontal count at which the HSYNC pin goes low
	 *   sync is inactive here for 7 cycles, manual claims 8 but is wrong?, this is the frontporch
	 *   HSE (104) is the horizontal count at which the horizontal counter is reset to 0 (so counts 0-103 then back to 0)
	 *
	 * VERTICAL:
	 *   0 is the first vertical line displayed to screen
	 *   VDE (344) affects the number of lines that /LDPS is active for display of
	 *   VSS (362) is the vertical line at which the VSYNC pin goes high
	 *   VST (378) is the vertical line at which the VSYNC pin goes low
	 *   VSE (400) is the vertical line at which the vertical line count is reset to 0
	 *   VOC (0x1c0) controls the polarity and enabling of the sync signals in some unknown way
	 *     Suffice to say, whatever bit combination 0b00011100000x does, it enables both horiz and vert sync and both are positive
	 */
#ifdef DEBUG_VIDEO_CONTROL_W
	static const char *const regDest[16] = { "VSE (End of frame)", "VST (End of VSync)", "VSS (Start of VSync)", "VDE (Active Lines)", "unknown 620xxx", "unknown 628xxx", "unknown 630xxx", "unknown 638xxx", "HSE (end of horizontal line)", "HST (end of HSync)", "HSS (HSync Start)", "VOC (Video Control)", "unknown 660xxx", "unknown 668xxx", "unknown 670xxx", "unknown 678xxx" };
	fprintf(stderr,"Write to video chip address %06X; %02X -> register %s with data %04X\n", 0x600000+(offset<<1), offset&0xFF, regDest[(offset&0x3C000)>>14], data);
#endif
}

// Floppy control register (called fd.cont in the cat source code)
	/* FEDCBA98 (76543210 is open bus)
	 * |||||||\-- unknown[1] (may be some sort of 'reset' or debug bit? the cat code explicitly clears this bit but never sets it)
	 * ||||||\--- WRITE GATE: 0 = write head disabled, 1 = write head enabled (verified from cat source code)
	 * |||||\---- unknown[2] (leftover debug bit? unused by cat code)
	 * ||||\----- /DIRECTION: 1 = in, 0 = out (verified from forth cmd)
	 * |||\------ /SIDESEL: 1 = side1, 0 = side0 (verified from forth cmd)
	 * ||\------- STEP: 1 = STEP active, 0 = STEP inactive (verified from cat source code)
	 * |\-------- MOTOR ON: 1 = on, 0 = off (verified)
	 * \--------- /DRIVESELECT: 1 = drive 0, 0 = drive 1 (verified from forth cmd)
	 * all 8 bits 'stick' on write and are readable at this register as well
	 * [1] writing this bit as high seems to 'freeze' floppy acquisition so
	 *   the value at the floppy_data_r register is held rather than updated
	 *   with new data from the shifter/mfm clock/data separator
	 * [2] this bit's function is unknown. it could possibly be an FM vs MFM selector bit, where high = MFM, low = FM ? or MFM vs GCR?
	 */
// 0x800000-0x800001 read
READ16_MEMBER( cat_state::cat_floppy_control_r )
{
#ifdef DEBUG_FLOPPY_CONTROL_R
	fprintf(stderr,"Read from Floppy Status address %06X\n", 0x800000+(offset<<1));
#endif
	return (m_floppy_control << 8)|0x80; // LOW 8 BITS ARE OPEN BUS
}
// 0x800000-0x800001 write
WRITE16_MEMBER( cat_state::cat_floppy_control_w )
{
#ifdef DEBUG_FLOPPY_CONTROL_W
	fprintf(stderr,"Write to Floppy Control address %06X, data %04X\n", 0x800000+(offset<<1), data);
#endif
	m_floppy_control = (data >> 8)&0xFF;
}

// 0x800002-0x800003 read = 0x0080, see open bus
// 0x800002-0x800003 write
WRITE16_MEMBER( cat_state::cat_keyboard_w )
{
	m_keyboard_line = data >> 8;
}

// 0x800004-0x800005 'pr.data' write
// /DSTB (centronics pin 1) is implied by the cat source code to be pulsed
// low (for some unknown period of time) upon any write to this port.
WRITE16_MEMBER( cat_state::cat_printer_data_w )
{
#ifdef DEBUG_PRINTER_DATA_W
	fprintf(stderr,"Write to Printer Data address %06X, data %04X\n", 0x800004+(offset<<1), data);
#endif
	m_ctx_data_out->write(data>>8);
	m_ctx->write_strobe(1);
	m_ctx->write_strobe(0);
	m_ctx->write_strobe(1);
}
// 0x800006-0x800007: Floppy data register (called fd.dwr in the cat source code)
READ16_MEMBER( cat_state::cat_floppy_data_r )
{
#ifdef DEBUG_FLOPPY_DATA_R
	fprintf(stderr,"Read from Floppy Data address %06X\n", 0x800006+(offset<<1));
#endif
	return 0x0080;
}
WRITE16_MEMBER( cat_state::cat_floppy_data_w )
{
#ifdef DEBUG_FLOPPY_DATA_W
	fprintf(stderr,"Write to Floppy Data address %06X, data %04X\n", 0x800006+(offset<<1), data);
#endif
}

// 0x800008-0x800009: Floppy status register (called fd.status in the cat source code)
	/* FEDCBA98 (76543210 is open bus)
	 * |||||||\-- ? always low
	 * ||||||\--- ? always low
	 * |||||\---- READY: 1 = ready, 0 = not ready (verified from cat source code)
	 * ||||\----- /WRITE PROTECT: 1 = writable, 0 = protected (verified)
	 * |||\------ /TRACK0: 0 = on track 0, 1 = not on track 0 (verified)
	 * ||\------- /INDEX: 0 = index sensor active, 1 = index sensor inactive (verified)
	 * |\-------- ? this bit may indicate which drive is selected, i.e. same as floppy control bit 7; low on drive 1, high on drive 0?
	 * \--------- ? this bit may indicate 'data separator overflow'; it is usually low but becomes high if you manually select the floppy drive
	 ALL of these bits except bit F seem to be reset when the selected drive in floppy control is switched
	 */
READ16_MEMBER( cat_state::cat_floppy_status_r )
{
#ifdef DEBUG_FLOPPY_STATUS_R
	fprintf(stderr,"Read from Floppy Status address %06X\n", 0x800008+(offset<<1));
#endif
	return 0x2480;
}

// 0x80000a-0x80000b
READ16_MEMBER( cat_state::cat_keyboard_r )
{
	UINT16 retVal = 0;
	// Read country code
	if ((m_pr_cont&0xFF00) == 0x0900)
		retVal = m_dipsw->read();

	// Regular keyboard read
	if ((m_pr_cont&0xFF00) == 0x0800 || (m_pr_cont&0xFF00) == 0x0a00)
	{
		retVal=0xff00;
		switch(m_keyboard_line)
		{
			case 0x01: retVal = m_y0->read() << 8; break;
			case 0x02: retVal = m_y1->read() << 8; break;
			case 0x04: retVal = m_y2->read() << 8; break;
			case 0x08: retVal = m_y3->read() << 8; break;
			case 0x10: retVal = m_y4->read() << 8; break;
			case 0x20: retVal = m_y5->read() << 8; break;
			case 0x40: retVal = m_y6->read() << 8; break;
			case 0x80: retVal = m_y7->read() << 8; break;
		}
	}
#if 0
	if (((m_pr_cont&0xFF00) != 0x0800) && ((m_pr_cont&0xFF00) != 0x0900) && ((m_pr_cont&0xFF00) != 0x0a00))
	{
		fprintf(stderr,"Read from keyboard in %06X with unexpected pr_cont %04X\n", 0x80000a+(offset<<1), m_pr_cont);
	}
#endif
	return retVal;
}

// 0x80000c-0x80000d (unused in cat source code; may have originally been a separate read only port where 800006 would have been write-only)

// 0x80000e-0x80000f 'pr.cont' read
READ16_MEMBER( cat_state::cat_battery_r )
{
	/*
	 * FEDCBA98 (76543210 is open bus)
	 * |||||||\-- ? possibly PE (pin 1) read ("PAPER OUT" pin 12 of centronics port)
	 * ||||||\--- ? possibly SLCT/ERR (pin 3) read ("not selected or error" NAND of pins 32 and 13 of centronics port)
	 * |||||\---- (always 0?)
	 * ||||\----- (always 0?)
	 * |||\------ (always 0?)
	 * ||\------- (always 0?)
	 * |\-------- (always 0?)
	 * \--------- Battery status (0 = good, 1 = bad)
	 */
	/* just return that battery is full, i.e. bit 15 is 0 */
	/* to make the cat think the battery is bad, return 0x8080 instead of 0x0080 */
	// TODO: hook this to a dipswitch
	return 0x0080;
}
// 0x80000e-0x80000f 'pr.cont' write
WRITE16_MEMBER( cat_state::cat_printer_control_w )
{
	/*
	 * FEDCBA98 (76543210 is ignored)
	 * |||||||\-- CC line enable (pin 34) (verified from cat source code)
	 * ||||||\--- LEDE line enable (pin 33) (verified from cat source code)
	 * |||||\---- ?
	 * ||||\----- ? may be IPP (pin 2) write (non-standard pin 34 of centronics port) or another watchdog reset bit; may also be /DSTB-enable-on-pr.data-write
	 * |||\------ ?
	 * ||\------- ?
	 * |\-------- ?
	 * \--------- ?
	 */
	// writes of 0x0A00 turn on the keyboard LED on the LOCK key
	// writes of 0x0800 turn off the keyboard LED on the LOCK key
#ifdef DEBUG_PRINTER_CONTROL_W
	fprintf(stderr,"Write to Printer Control address %06X, data %04X\n", (offset<<1)+0x80000e, data);
#endif
	m_pr_cont = data;
}

// 0x820000: AMI S35213 300/1200 Single Chip Modem (datasheet found at http://bitsavers.trailing-edge.com/pdf/ami/_dataBooks/1985_AMI_MOS_Products_Catalog.pdf on pdf page 243)
READ16_MEMBER( cat_state::cat_modem_r )
{
#ifdef DEBUG_MODEM_R
	fprintf(stderr,"Read from s35213 modem address %06X\n", 0x820000+(offset<<1));
#endif
// HACK: return default 'sane' modem state
	return 0x00;
}

WRITE16_MEMBER( cat_state::cat_modem_w )
{
#ifdef DEBUG_MODEM_W
	fprintf(stderr,"Write to s35213 modem address %06X, data %04X\n", 0x820000+(offset<<1), data);
#endif
}

// 0x830000: 6ms counter (counts KTOBF pulses and does not reset; 16 bits wide)
READ16_MEMBER( cat_state::cat_6ms_counter_r )
{
	return m_6ms_counter;
}

/* 0x840001: 'opr' or 'ga2opr' Output Port Register (within GA2)
 * writing anything with bit 3 set here resets the watchdog
 * if the watchdog expires /NMI (and maybe /RESET) are asserted to the cpu
 * watchdog counter (counts KTOBF pulses and is reset on any ga2opr write with bit 3 set; <9 bits wide)
 */
WRITE16_MEMBER( cat_state::cat_opr_w )
{
	/*
	 * 76543210 (FEDCBA98 are ignored)
	 * |||||||\-- OFFHOOK pin (pin 3) output control (1 = puts phone off hook by energizing relay K2)
	 * ||||||\--- PHONE pin (pin 4) output control (1 = connects phone and line together by energizing relay K1)
	 * |||||\---- Video enable (1 = video on, 0 = video off/screen black)
	 * ||||\----- Watchdog reset (the watchdog is reset whenever this bit is written with a 1)
	 * |||\------ Video invert (1 = black-on-white video; 0 = white-on-black)
	 * ||\------- (unused?)
	 * |\-------- (unused?)
	 * \--------- (unused?)
	 */
#ifdef DEBUG_GA2OPR_W
	if (data != 0x001C)
		fprintf(stderr, "GA2 OPR (video ena/inv, watchdog, and phone relay) reg write: offset %06X, data %04X\n", 0x840000+(offset<<1), data);
#endif
	if (data&0x08) m_wdt_counter = 0;
	m_video_enable = BIT( data, 2 );
	m_video_invert = 1-BIT( data, 4 );
}

// 0x850000: 'wdt' "watchdog timer and power fail", read only?
	/* NOTE: BELOW IS A GUESS based on seeing what the register reads as,
	 * the cat code barely touches this stuff at all, despite the fact
	 * that the service manual states that PFAIL is supposed to be checked
	 * before each write to SVRAM, the forth code does NOT actually do that!
	 *
	 * 76543210
	 * ??????\\-- Watchdog count? (counts upward? if this reaches <some unknown number greater than 3> the watchdog fires? writing bit 3 set to opr above resets this)
	 *
	 * FEDCBA98
	 * |||||||\-- PFAIL state (MB3771 comparator: 1: vcc = 5v; 0: vcc != 5v, hence do not write to svram!)
	 * ||||||\--- (always 0?)
	 * |||||\---- (always 0?)
	 * ||||\----- (always 0?)
	 * |||\------ (always 0?)
	 * ||\------- (always 0?)
	 * |\-------- (always 0?)
	 * \--------- (always 0?)
	 */
READ16_MEMBER( cat_state::cat_wdt_r )
{
	uint16 Retval = 0x0100; // set pfail to 1; should this be a dipswitch?
	return Retval | m_wdt_counter;
}

// 0x860000: 'tcb' "test control bits" test mode register; what the bits do is
// unknown. 0x0000 is written here to disable test mode, and that is the extent
// of the cat touching this register.
WRITE16_MEMBER( cat_state::cat_tcb_w )
{
#ifdef DEBUG_TEST_W
	fprintf(stderr, "Test reg write: offset %06X, data %04X\n", 0x860000+(offset<<1), data);
#endif
}

// open bus etc handlers
READ16_MEMBER( cat_state::cat_2e80_r )
{
	return 0x2e80;
}

READ16_MEMBER( cat_state::cat_0000_r )
{
	return 0x0000;
}

READ16_MEMBER( cat_state::cat_0080_r )
{
	return 0x0080;
}


/* Canon cat memory map, based on testing and a 16MB dump of the entire address space of a running unit using forth "1000000 0 do i c@ semit loop"
68k address map:
a23 a22 a21 a20 a19 a18 a17 a16 a15 a14 a13 a12 a11 a10 a9  a8  a7  a6  a5  a4  a3  a2  a1  (a0 via UDS/LDS)
*i  *i  *   x   x   *   *   *   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x       *GATE ARRAY 2 DECODES THESE LINES TO ENABLE THIS AREA* (a23 and a22 are indirectly decoded via the /RAMROMCS and /IOCS lines from gate array 1)
0   0   0   x   x   0   a   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   b       R   ROM (ab: 00=ic4 01=ic2 10=ic5 11=ic3) (EPROM 27C512 x 4) [controlled via GA2 /ROMCS]
0   0   0   x   x   1   0   0   x   x   *   *   *   *   *   *   *   *   *   *   *   *   *   0       RW  SVRAM ic11 d4364 (battery backed) [controlled via GA2 /RAMCS]
0   0   0   x   x   1   x   1   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   0       O   OPEN BUS (reads as 0x2e) [may be controlled via GA2 /RAMCS?]
0   0   0   x   x   1   1   0   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   0       O   OPEN BUS (reads as 0x2e) [may be controlled via GA2 /RAMCS?]
0   0   0   x   x   1   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   1       O   OPEN BUS (reads as 0x80) [may be controlled via GA2 /RAMCS?]
0   0   1   x   x   0   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   0       R   SVROM 2 ic7 (not present on cat as sold, open bus reads as 0x2e) [controlled via GA2 /SVCS0]
0   0   1   x   x   0   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   1       R   SVROM 0 ic6 (MASK ROM tc531000) [controlled via GA2 /SVCS0]
0   0   1   x   x   1   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   0       O   OPEN BUS (reads as 0x2e) [controlled via GA2 /SVCS1] *SEE BELOW*
0   0   1   x   x   1   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   1       R   SVROM 1 ic8 (not present on cat as sold, open bus reads as 0x80) [controlled via GA2 /SVCS1] *SEE BELOW*
                                                                                                    *NOTE: on Dwight E's user-made developer unit, two 128K SRAMS are mapped in place of the two entries immediately above!* (this involves some creative wiring+sockets); the official IAI 'shadow ram board' maps the ram to the A00000-A3FFFF area instead)
0   1   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *       *BOTH GATE ARRAYS 1 and 2 DECODE THIS AREA; 2 DEALS WITH ADDR AND 1 WITH DATA/CAS/RAS*
0   1   0   x   x   a   b   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *       RW  VIDEO/SYSTEM DRAM (ab: 00=row 0, ic26-29; 01=row 1, ic22-25; 10=row 2; ic18-21; 11=row 3; ic14-17)
                                                                                                    *NOTE: DRAM rows 2 and 3 above are only usually populated in cat developer units!*
0   1   1   ?   ?   *   *   *   ?   ?   ?   ?   ?   ?   ?   *   *   *   *   *   *   *   *   x       W   VIDEO CONTRL REGISTERS (reads as 0x2e80)
1   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   *   *   *   *   *       *GATE ARRAY 3 DECODES THIS AREA, GA3 IS ENABLED BY /IOCS1 FROM GA2*
1   0   0   Y   Y   0   0   0   x   x   x   x   x   x   x   x   x   x   x   *   *   *   *   0       {'ga3'} *IO AREA* Note byte reads in this area behave erratically if both Y bits are set while word reads work fine always
                                                                            x   x   x   x   1       O   OPEN BUS (reads as 0x80)
                                                                            0   0   0   0   0       RW  {'fd.cont'} Floppy control lines (drive select, motor on, direction, step, side select, ?write gate?)
                                                                            0   0   0   1   0       W   {'kb.wr'} Keyboard Row Select (reads as 0x00)
                                                                            0   0   1   0   0       W   {'pr.data'} Centronics Printer Data W (reads as 0x00)
                                                                            0   0   1   1   0       RW  {'fd.dwr' and 'fd.drd'} Floppy data read/write register; maybe the write gate value in fd.cont chooses which?
                                                                            0   1   0   0   0       R   {'fd.status'} Floppy status lines (write protect, ready, index, track0)
                                                                            0   1   0   1   0       R   Keyboard Column Read
                                                                            0   1   1   0   0       W?  Unknown (reads as 0x00)
                                                                            0   1   1   1   0       RW  {'pr.cont'} Read: Battery status (MSB bit, 0 = ok, 1 = dead, other bits read as 0)/Write: Centronics Printer and Keyboard LED/Country Code Related
                                                                            1   x   x   x   0       W?  Unknown (reads as 0x00)
1   0   0   x   x   0   0   1   x   x   x   x   x   x   x   x   x   x   x   *   *   *   *   1       RW  {'duart'} 68681 DUART at ic34 [controlled via GA2 /DUARTCS]
1   0   0   x   x   0   1   0   x   x   x   x   x   x   x   x   x   x   *   *   *   *   *   0       RW  {'modem'} Modem Chip AMI S35213 @ IC37 DATA BIT 7 ONLY [controlled via GA2 /SMCS]
1   0   0   x   x   0   1   1   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   *       R   {'timer'} Read: Fixed 16-bit counter from ga2. increments every 6.5535ms when another 16-bit counter clocked at 10mhz overflows
1   0   0   x   x   1   0   0   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   *       W   {'opr'} Output Port (Video/Sync enable and watchdog reset?) register (screen enable on bit 3?) (reads as 0x2e80)
1   0   0   x   x   1   0   1   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   *       R   {'wdt'} Watchdog timer reads as 0x0100 0x0101 or 0x0102, some sort of test register or video status register?
1   0   0   x   x   1   1   0   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   *       R?W {'tcb'} test control bits (reads as 0x0000)
1   0   0   x   x   1   1   1   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   *       ?   Unknown (reads as 0x2e80)

1   0   1   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x       O   OPEN BUS (reads as 0x2e80) [68k DTACK is asserted by gate array 1 when accessing this area, for testing?] On real IAI shadow rom board, at least 0x40000 of ram lives here.
1   1   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x       O   OPEN BUS (reads as 0x2e80) [68k VPA is asserted by gate array 1 when accessing this area, for testing?]
*/


static ADDRESS_MAP_START(cat_mem, AS_PROGRAM, 16, cat_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x03ffff) AM_ROM AM_MIRROR(0x180000) // 256 KB ROM
	AM_RANGE(0x040000, 0x043fff) AM_RAM AM_SHARE("svram") AM_MIRROR(0x18C000)// SRAM powered by battery
	AM_RANGE(0x200000, 0x27ffff) AM_ROM AM_REGION("svrom",0x0000) AM_MIRROR(0x180000) // SV ROM
	AM_RANGE(0x400000, 0x47ffff) AM_RAM AM_SHARE("p_cat_vram") AM_MIRROR(0x180000) // 512 KB RAM
	AM_RANGE(0x600000, 0x67ffff) AM_READWRITE(cat_2e80_r,cat_video_control_w) AM_MIRROR(0x180000) // Gate Array #1: Video Addressing and Timing, dram refresh timing, dram /cs and /wr (ga2 does the actual video invert/display and access to the dram data bus)
	AM_RANGE(0x800000, 0x800001) AM_READWRITE(cat_floppy_control_r, cat_floppy_control_w) AM_MIRROR(0x18FFE0) // floppy control lines and readback
	AM_RANGE(0x800002, 0x800003) AM_READWRITE(cat_0080_r, cat_keyboard_w) AM_MIRROR(0x18FFE0) // keyboard col write
	AM_RANGE(0x800004, 0x800005) AM_READWRITE(cat_0080_r, cat_printer_data_w) AM_MIRROR(0x18FFE0) // Centronics Printer Data
	AM_RANGE(0x800006, 0x800007) AM_READWRITE(cat_floppy_data_r,cat_floppy_data_w) AM_MIRROR(0x18FFE0) // floppy data read/write
	AM_RANGE(0x800008, 0x800009) AM_READ(cat_floppy_status_r) AM_MIRROR(0x18FFE0) // floppy status lines
	AM_RANGE(0x80000a, 0x80000b) AM_READ(cat_keyboard_r) AM_MIRROR(0x18FFE0) // keyboard row read
	AM_RANGE(0x80000c, 0x80000d) AM_READ(cat_0080_r) AM_MIRROR(0x18FFE0) // Open bus?
	AM_RANGE(0x80000e, 0x80000f) AM_READWRITE(cat_battery_r,cat_printer_control_w) AM_MIRROR(0x18FFE0) // Centronics Printer Control, keyboard led and country code enable
	AM_RANGE(0x800010, 0x80001f) AM_READ(cat_0080_r) AM_MIRROR(0x18FFE0) // Open bus?
	AM_RANGE(0x810000, 0x81001f) AM_DEVREADWRITE8("duartn68681", mc68681_device, read, write, 0xff ) AM_MIRROR(0x18FFE0)
	AM_RANGE(0x820000, 0x82003f) AM_READWRITE(cat_modem_r,cat_modem_w) AM_MIRROR(0x18FFC0) // AMI S35213 Modem Chip, all access is on bit 7
	AM_RANGE(0x830000, 0x830001) AM_READ(cat_6ms_counter_r) AM_MIRROR(0x18FFFE) // 16bit 6ms counter clocked by output of another 16bit counter clocked at 10mhz
	AM_RANGE(0x840000, 0x840001) AM_READWRITE(cat_2e80_r,cat_opr_w) AM_MIRROR(0x18FFFE) // GA2 Output port register (video enable, invert, watchdog reset, phone relays)
	AM_RANGE(0x850000, 0x850001) AM_READ(cat_wdt_r) AM_MIRROR(0x18FFFE) // watchdog and power fail state read
	AM_RANGE(0x860000, 0x860001) AM_READWRITE(cat_0000_r, cat_tcb_w) AM_MIRROR(0x18FFFE) // Test mode
	AM_RANGE(0x870000, 0x870001) AM_READ(cat_2e80_r) AM_MIRROR(0x18FFFE) // Open bus?
	AM_RANGE(0xA00000, 0xA00001) AM_READ(cat_2e80_r) AM_MIRROR(0x1FFFFE) // Open bus/dtack? The 0xA00000-0xA3ffff area is ram used for shadow rom storage on cat developer machines, which is either banked over top of, or jumped to instead of the normal rom
	AM_RANGE(0xC00000, 0xC00001) AM_READ(cat_2e80_r) AM_MIRROR(0x3FFFFE) // Open bus/vme?
ADDRESS_MAP_END

/* Input ports */

/* 2009-07 FP
   FIXME: Natural keyboard does not catch all the Shifted chars. No idea of the reason!  */
static INPUT_PORTS_START( cat )
	PORT_START("DIPSW1")
	PORT_DIPNAME( 0x8000, 0x8000, "Mode" )
	PORT_DIPSETTING(    0x8000, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x0000, "Diagnostic" )
	PORT_DIPNAME( 0x7f00,0x7f00, "Country code" )
	PORT_DIPSETTING(    0x7f00, "United States" )
	PORT_DIPSETTING(    0x7e00, "Canada" )
	PORT_DIPSETTING(    0x7d00, "United Kingdom" )
	PORT_DIPSETTING(    0x7c00, "Norway" )
	PORT_DIPSETTING(    0x7b00, "France" )
	PORT_DIPSETTING(    0x7a00, "Denmark" )
	PORT_DIPSETTING(    0x7900, "Sweden" )
	PORT_DIPSETTING(    0x7800, DEF_STR(Japan) )
	PORT_DIPSETTING(    0x7700, "West Germany" )
	PORT_DIPSETTING(    0x7600, "Netherlands" )
	PORT_DIPSETTING(    0x7500, "Spain" )
	PORT_DIPSETTING(    0x7400, "Italy" )
	PORT_DIPSETTING(    0x7300, "Latin America" )
	PORT_DIPSETTING(    0x7200, "South Africa" )
	PORT_DIPSETTING(    0x7100, "Switzerland" )
	PORT_DIPSETTING(    0x7000, "ASCII" )

	PORT_START("Y0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('\xa2')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')

	PORT_START("Y1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('n') PORT_CHAR('B')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')

	PORT_START("Y2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED) // totally unused
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')

	PORT_START("Y3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left USE FRONT") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED) // totally unused

	PORT_START("Y4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right USE FRONT") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_F2) // intl only: latin diaresis and latin !; norway, danish and finnish * and '; others
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')

	PORT_START("Y5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('\xbd') PORT_CHAR('\xbc') //PORT_CHAR('}') PORT_CHAR('{')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("Y6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_F1) // intl only: latin inv ? and inv !; norway and danish ! and |; finnish <>; others
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left LEAP") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('[')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED) // totally unused

	PORT_START("Y7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_RSHIFT) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Leap") PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Page") PORT_CODE(KEYCODE_PGUP) PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift Lock") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Erase") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED) // totally unused
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("UNDO") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('\xb1') PORT_CHAR('\xb0') // PORT_CHAR('\\') PORT_CHAR('~')
INPUT_PORTS_END

static INPUT_PORTS_START( swyft )
// insert dwight and sandy's swyft keyboard map here once we figure out the byte line order
INPUT_PORTS_END


void cat_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_COUNTER_6MS:
		counter_6ms_callback(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in cat_state::device_timer");
	}
}

TIMER_CALLBACK_MEMBER(cat_state::counter_6ms_callback)
{
	// This is effectively also the KTOBF (guessed: acronym for "Keyboard Timer Out Bit Flip")
	// line connected in such a way to invert the d-flipflop connected to the duart IP2
	m_duart_ktobf_ff ^= 1;
	m_duart->ip2_w(m_duart_ktobf_ff);
	m_wdt_counter++;
	m_6ms_counter++;
}

IRQ_CALLBACK_MEMBER(cat_state::cat_int_ack)
{
	m_maincpu->set_input_line(M68K_IRQ_1,CLEAR_LINE);
	return M68K_INT_ACK_AUTOVECTOR;
}

MACHINE_START_MEMBER(cat_state,cat)
{
	m_duart_ktobf_ff = 0; // reset doesn't touch this
	m_duart_prn_ack_prev_state = 1; // technically uninitialized
	m_duart_prn_ack_ff = 0; // reset doesn't touch this
	m_6ms_counter = 0;
	m_wdt_counter = 0;
	m_video_enable = 1;
	m_video_invert = 0;
	m_6ms_timer = timer_alloc(TIMER_COUNTER_6MS);
	machine().device<nvram_device>("nvram")->set_base(m_svram, 0x4000);
}

MACHINE_RESET_MEMBER(cat_state,cat)
{
	m_6ms_counter = 0;
	m_wdt_counter = 0;
	m_floppy_control = 0;
	m_6ms_timer->adjust(attotime::zero, 0, attotime::from_hz((XTAL_19_968MHz/2)/65536));
}

VIDEO_START_MEMBER(cat_state,cat)
{
}

UINT32 cat_state::screen_update_cat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 code;
	int y, x, b;

	int addr = 0;
	if (m_video_enable == 1)
	{
		for (y = 0; y < 344; y++)
		{
			int horpos = 0;
			for (x = 0; x < 42; x++)
			{
				code = m_p_cat_videoram[addr++];
				for (b = 15; b >= 0; b--)
				{
					bitmap.pix16(y, horpos++) = ((code >> b) & 0x01) ^ m_video_invert;
				}
			}
		}
	} else {
		const rectangle black_area(0, 672 - 1, 0, 344 - 1);
		bitmap.fill(0, black_area);
	}
	return 0;
}

/* The duart is the only thing actually connected to the cpu IRQ pin
 * The KTOBF output of the gate array 2 (itself the terminal count output
 * of a 16-bit counter clocked at ~10mhz, hence 6.5536ms period) goes to a
 * d-latch and inputs on ip2 of the duart, causing the duart to fire an irq;
 * this is used by the cat to read the keyboard.
 * The duart also will, if configured to do so, fire an int when the state
 * changes of the centronics /ACK pin; this is used while printing.
 */
WRITE_LINE_MEMBER(cat_state::cat_duart_irq_handler)
{
	int irqvector = m_duart->get_irq_vector();

#ifdef DEBUG_DUART_IRQ_HANDLER
	fprintf(stderr, "Duart IRQ handler called: state: %02X, vector: %06X\n", state, irqvector);
#endif
	m_maincpu->set_input_line_and_vector(M68K_IRQ_1, state, irqvector);
}

WRITE_LINE_MEMBER(cat_state::cat_duart_txa) // semit sends stuff here; connects to the serial port on the back
{
#ifdef DEBUG_DUART_TXA
	fprintf(stderr, "Duart TXA: data %02X\n", state);
#endif
}

WRITE_LINE_MEMBER(cat_state::cat_duart_txb) // memit sends stuff here; connects to the modem chip
{
#ifdef DEBUG_DUART_TXB
	fprintf(stderr, "Duart TXB: data %02X\n", state);
#endif
}

/* mc68681 DUART Input pins:
 * IP0: CTS [using the DUART builtin hardware-CTS feature?]
 * IP1: Centronics /ACK (pin 10) positive edge detect (IP1 changes state 0->1 or 1->0 on the rising edge of /ACK using a 74ls74a d-flipflop)
 * IP2: KTOBF (IP2 changes state 0->1 or 1->0 on the rising edge of KTOBF using a 74ls74a d-flipflop; KTOBF is a 6.5536ms-period squarewave generated by one of the gate arrays, I need to check with a scope to see whether it is a single spike/pulse every 6.5536ms or if from the gate array it inverts every 6.5536ms, documentation isn't 100% clear but I suspect the former) [uses the Delta IP2 state change detection feature to generate an interrupt; I'm not sure if IP2 is used as a counter clock source but given the beep frequency of the real unit I very much doubt it, 6.5536ms is too slow]
 * IP3: RG ("ring" input)
 * IP4: Centronics BUSY (pin 11), inverted
 * IP5: DSR
 */

/* mc68681 DUART Output pins:
 * OP0: RTS [using the DUART builtin hardware-RTS feature?]
 * OP1: DTR
 * OP2: /TDCS (select/enable the S2579 DTMF tone generator chip)
 * OP3: speaker out [using the 'channel b 1X tx or rx clock output' or more likely the 'timer output' feature to generate a squarewave]
 * OP4: TD03 (data bus for the S2579 DTMF tone generator chip)
 * OP5: TD02 "
 * OP6: TD01 "
 * OP7: TD00 "
 */
WRITE8_MEMBER(cat_state::cat_duart_output)
{
#ifdef DEBUG_DUART_OUTPUT_LINES
	fprintf(stderr,"Duart output io lines changed to: %02X\n", data);
#endif
	m_speaker->level_w((data >> 3) & 1);
}

WRITE_LINE_MEMBER(cat_state::prn_ack_ff) // switch the flipflop state on the rising edge of /ACK
{
	if ((m_duart_prn_ack_prev_state == 0) && (state == 1))
	{
		m_duart_prn_ack_ff ^= 1;
	}
	m_duart->ip1_w(m_duart_prn_ack_ff);
	m_duart_prn_ack_prev_state = state;
#ifdef DEBUG_PRN_FF
	fprintf(stderr, "Printer ACK: state %02X, flipflop is now %02x\n", state, m_duart_prn_ack_ff);
#endif
}

static MACHINE_CONFIG_START( cat, cat_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000, XTAL_19_968MHz/4)
	MCFG_CPU_PROGRAM_MAP(cat_mem)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(cat_state,cat_int_ack)

	MCFG_MACHINE_START_OVERRIDE(cat_state,cat)
	MCFG_MACHINE_RESET_OVERRIDE(cat_state,cat)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(672, 344)
	MCFG_SCREEN_VISIBLE_AREA(0, 672-1, 0, 344-1)
	MCFG_SCREEN_UPDATE_DRIVER(cat_state, screen_update_cat)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	MCFG_VIDEO_START_OVERRIDE(cat_state,cat)

	MCFG_MC68681_ADD( "duartn68681", (XTAL_19_968MHz*2)/11 ) // duart is normally clocked by 3.6864mhz xtal, but cat seemingly uses a divider from the main xtal instead which probably yields 3.63054545Mhz. There is a trace to cut and a mounting area to allow using an actual 3.6864mhz xtal if you so desire
	MCFG_MC68681_IRQ_CALLBACK(WRITELINE(cat_state, cat_duart_irq_handler))
	MCFG_MC68681_A_TX_CALLBACK(WRITELINE(cat_state, cat_duart_txa))
	MCFG_MC68681_B_TX_CALLBACK(WRITELINE(cat_state, cat_duart_txb))
	MCFG_MC68681_OUTPORT_CALLBACK(WRITE8(cat_state, cat_duart_output))

	MCFG_CENTRONICS_ADD("ctx", centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE(cat_state, prn_ack_ff))
	MCFG_CENTRONICS_BUSY_HANDLER(DEVWRITELINE("duartn68681", mc68681_device, ip4_w)) MCFG_DEVCB_XOR(1)
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("ctx_data_out", "ctx")

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END


/* Swyft Memory map, based on watching the infoapp roms do their thing:
68k address map:
(a23,a22,a21,a20 lines don't exist on the 68008 so are considered unconnected)
a23 a22 a21 a20 a19 a18 a17 a16 a15 a14 a13 a12 a11 a10 a9  a8  a7  a6  a5  a4  a3  a2  a1  a0
x   x   x   x   0   0   ?   ?   0   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *        R   ROM-LO (/LOCE is 0, /HICE is 1)
x   x   x   x   0   0   ?   ?   1   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *        R   ROM-HI (/LOCE is 1, /HICE is 0)
x   x   x   x   0   1   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   a        RW  RAM
x   x   x   x   1   1  ?0? ?1?  ?   ?   ?   ?   ?   ?   ?   ?   ?   ?   ?   ?   *   *   *   *        R   ? status of something? floppy?
x   x   x   x   1   1  ?1? ?0?  ?   0   0   1   x   x   x   x   x   x   x   x   x   x   x   *        RW  6850 acia @U33, gets 0x55 steadystate and 0x57 written to it to reset it
x   x   x   x   1   1  ?1? ?0?  ?   0   1   0   x   x   *   *   *   *   x   x   x   x   x   x        RW  Parallel VIA 0 @ U34
x   x   x   x   1   1  ?1? ?0?  ?   1   0   0   x   x   *   *   *   *   x   x   x   x   x   x        RW  Keyboard VIA 1 @ U35
              ^               ^               ^               ^               ^

*/

/* Swyft rom and ram notes:
rom:
**Vectors:
0x0000-0x0003: SP boot vector
0x0004-0x0007: PC boot vector
**unknown:
0x0009-0x00BF: ? table
0x00C0-0x01DF: ? table
0x01E0-0x02DF: ? table (may be part of next table)
0x02E0-0x03DF: ? table
0x03E0-0x0B3F: int16-packed jump table (expanded to int32s at ram at 0x46000-0x46EC0 on boot)
0x0B40-0x0E83: ? function index tables?
0x0E84-0x1544: binary code (purpose?)
0x1545-0x24CF: ?
**Fonts:
0x24D0-0x254F: ? (likely font 1 width lookup table)
0x2550-0x2BCF: Font 1 data
0x2BD0-0x2C4F: ? (likely font 2 width lookup table)
0x2C50-0x32CF: Font 2 data
**unknown?:
0x32D0-0x360F: String data (and control codes?)
0x3610-0x364F: ? fill (0x03 0xe8)
0x3650-0x369F: ? fill (0x03 0x20)
0x36A0-0x384d: ? forth code?
0x384e-0x385d: Lookup table for phone keypad
0x385e-...: ?
...-0xC951: ?
0xC952: boot vector
0xC952-0xCAAE: binary code (purpose?)
    0xCD26-0xCD3B: ?init forth bytecode?
0xCD3C-0xCEBA: 0xFF fill (unused?)
0xCEEB-0xFFFE: Forth dictionaries for compiling, with <word> then <3 bytes> afterward? (or before it? most likely afterward)

ram: (system dram ranges from 0x40000-0x7FFFF)
0x40000-0x425CF - the screen display ram
(?0x425D0-0x44BA0 - ?unknown (maybe screen ram page 2?))
0x44DC6 - SP vector
0x46000-0x46EC0 - jump tables to instructions for ? (each forth word?)


on boot:
copy/expand packed rom short words 0x3E0-0xB3F to long words at 0x46000-0x46EC0
copy 0x24f longwords of zero beyond that up to 0x47800
CD26->A5 <?pointer to init stream function?>
44DC6->A7 <reset SP... why it does this twice, once by the vector and once here, i'm gonna guess has to do with running the code in a debugger or on a development daughterboard like the cat had, where the 68008 wouldn't get explicitly reset>
44F2A->A6 <?pointer to work ram space?>
EA2->A4 <?function>
E94->A3 <?function>
EAE->A2 <?function>
41800->D7 <?forth? opcode index base; the '1800' portion gets the opcode type added to it then is multiplied by 4 to produce the jump table offset within the 0x46000-0x46EC0 range>
46e3c->D4 <?pointer to more work ram space?>
CD22->D5 <?pointer to another function?>
write 0xFFFF to d0004.l
jump to A4(EA2)

read first stream byte (which is 0x03) from address pointed to by A5 (which is CD26), inc A5, OR the opcode (0x03) to D7
 (Note: if the forth opcodes are in order in the dictionary, then 0x03 is "!char" which is used to read a char from an arbitrary address)
copy D7 to A0
Add A0 low word to itself
Add A0 low word to itself again
move the long word from address pointed to by A0 (i.e. the specific opcode's area at the 46xxx part of ram) to A1
Jump to A1(11A4)

11A4: move 41b00 to D0 (select an opcode "page" 1bxx)
jump to 118E

118E: read next stream byte (in this case, 0x8E) from address pointed to by A5 (which is CD27), inc A5, OR the opcode (0x8e) to D7
add to 41b00 in d0, for 41b8E
Add A0 low word to itself
Add A0 low word to itself again
move the long word from address pointed to by A0 (i.e. the specific opcode's area at the 46xxx part of ram) to A1
Jump to A1(CD06)

CD06: jump to A3 (E94)

E94: subtract D5 from A5 (cd28 - cd22 = 0x0006)
write 6 to address @A5(44f28) and decrement A5
write D4(46e3c) to address @a6(44f26) and decrement a5
lea ($2, A1), A5 - i.e. increment A1 by 2, and write that to A5, so write CD06+2=CD08 to A5
A1->D5
A0->D4
read next stream byte (in this case, 0x03) from address pointed to by A5 (which is CD08), inc A5, OR the opcode (0x03) to D7

*/

static ADDRESS_MAP_START(swyft_mem, AS_PROGRAM, 8, cat_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x00ffff) AM_ROM AM_MIRROR(0xF00000) // 64 KB ROM
	AM_RANGE(0x040000, 0x07ffff) AM_RAM AM_MIRROR(0xF00000) AM_SHARE("p_swyft_vram") // 256 KB RAM
	AM_RANGE(0x0d0000, 0x0d000f) AM_READ(swyft_d0000) AM_MIRROR(0xF00000) // status of something? reads from d0000, d0004, d0008, d000a, d000e
	AM_RANGE(0x0e1000, 0x0e1000) AM_DEVWRITE("acia6850", acia6850_device, control_w) AM_MIRROR(0xF00000) // 6850 ACIA lives here
	AM_RANGE(0x0e2000, 0x0e2fff) AM_READWRITE(swyft_via0_r, swyft_via0_w) AM_MIRROR(0xF00000)// io area with selector on a9 a8 a7 a6?
	AM_RANGE(0x0e4000, 0x0e4fff) AM_READWRITE(swyft_via1_r, swyft_via1_w) AM_MIRROR(0xF00000)
ADDRESS_MAP_END

MACHINE_START_MEMBER(cat_state,swyft)
{
	m_via0->write_ca1(1);
	m_via0->write_ca2(1);
	m_via0->write_cb1(1);
	m_via0->write_cb2(1);

	m_via1->write_ca1(1);
	m_via1->write_ca2(1);
	m_via1->write_cb1(1);
	m_via1->write_cb2(1);
}

MACHINE_RESET_MEMBER(cat_state,swyft)
{
}

VIDEO_START_MEMBER(cat_state,swyft)
{
}

UINT32 cat_state::screen_update_swyft(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 code;
	int y, x, b;

	int addr = 0;
	for (y = 0; y < 242; y++)
	{
		int horpos = 0;
		for (x = 0; x < 40; x++)
		{
			code = m_p_swyft_videoram[addr++];
			for (b = 7; b >= 0; b--)
			{
				bitmap.pix16(y, horpos++) = (code >> b) & 0x01;
			}
		}
	}
	return 0;
}

READ8_MEMBER( cat_state::swyft_d0000 )
{
	// wtf is this supposed to be?
	UINT8 byte = 0xFF; // ?
	logerror("mystery device: read from 0x%5X, returning %02X\n", offset+0xD0000, byte);
	return byte;
}


// if bit is 1 enable: (obviously don't set more than one bit or you get bus contention!)
//                                           acia
//                                       via0
//                                    via1
// x   x   x   x   1   1  ?1? ?0?  ?   ^   ^   ^   ?   ?   *   *   *   *  ?*?  ?   ?   ?   ?   ?
//                                                         ^   ^   ^   ^  <- these four bits address the VIA registers? is this correct?
static const char *const swyft_via_regnames[] = { "0: ORB/IRB", "1: ORA/IRA", "2: DDRB", "3: DDRA", "4: T1C-L", "5: T1C-H", "6: T1L-L", "7: T1L-H", "8: T2C-L" "9: T2C-H", "A: SR", "B: ACR", "C: PCR", "D: IFR", "E: IER", "F: ORA/IRA*" };

READ8_MEMBER( cat_state::swyft_via0_r )
{
	if (offset&0x000C3F) fprintf(stderr,"VIA0: read from invalid offset in 68k space: %06X!\n", offset);
	UINT8 data = m_via0->read(space, (offset>>6)&0xF);
#ifdef DEBUG_SWYFT_VIA0
	logerror("VIA0 register %s read by cpu: returning %02x\n", swyft_via_regnames[(offset>>5)&0xF], data);
#endif
	return data;
}

WRITE8_MEMBER( cat_state::swyft_via0_w )
{
#ifdef DEBUG_SWYFT_VIA0
	logerror("VIA0 register %s written by cpu with data %02x\n", swyft_via_regnames[(offset>>5)&0xF], data);
#endif
	if (offset&0x000C3F) fprintf(stderr,"VIA0: write to invalid offset in 68k space: %06X, data: %02X!\n", offset, data);
	m_via1->write(space, (offset>>6)&0xF, data);
}

READ8_MEMBER( cat_state::swyft_via1_r )
{
	if (offset&0x000C3F) fprintf(stderr," VIA1: read from invalid offset in 68k space: %06X!\n", offset);
	UINT8 data = m_via1->read(space, (offset>>6)&0xF);
#ifdef DEBUG_SWYFT_VIA1
	logerror(" VIA1 register %s read by cpu: returning %02x\n", swyft_via_regnames[(offset>>5)&0xF], data);
#endif
	return data;
}

WRITE8_MEMBER( cat_state::swyft_via1_w )
{
#ifdef DEBUG_SWYFT_VIA1
	logerror(" VIA1 register %s written by cpu with data %02x\n", swyft_via_regnames[(offset>>5)&0xF], data);
#endif
	if (offset&0x000C3F) fprintf(stderr," VIA1: write to invalid offset in 68k space: %06X, data: %02X!\n", offset, data);
	m_via0->write(space, (offset>>6)&0xF, data);
}

// first via
READ8_MEMBER( cat_state::via0_pa_r )
{
	logerror("VIA0: Port A read!\n");
	return 0xFF;
}

WRITE8_MEMBER( cat_state::via0_pa_w )
{
	logerror("VIA0: Port A written with data of 0x%02x!\n", data);
}

WRITE_LINE_MEMBER ( cat_state::via0_ca2_w )
{
	logerror("VIA0: CA2 written with %d!\n", state);
}

READ8_MEMBER( cat_state::via0_pb_r )
{
	logerror("VIA0: Port B read!\n");
	return 0xFF;
}

WRITE8_MEMBER( cat_state::via0_pb_w )
{
	logerror("VIA0: Port B written with data of 0x%02x!\n", data);
}

WRITE_LINE_MEMBER ( cat_state::via0_cb1_w )
{
	logerror("VIA0: CB1 written with %d!\n", state);
}

WRITE_LINE_MEMBER ( cat_state::via0_cb2_w )
{
	logerror("VIA0: CB2 written with %d!\n", state);
}

WRITE_LINE_MEMBER ( cat_state::via0_int_w )
{
	logerror("VIA0: INT output set to %d!\n", state);
}

// second via
READ8_MEMBER( cat_state::via1_pa_r )
{
	logerror(" VIA1: Port A read!\n");
	return 0xFF;
}

WRITE8_MEMBER( cat_state::via1_pa_w )
{
	logerror(" VIA1: Port A written with data of 0x%02x!\n", data);
}

WRITE_LINE_MEMBER ( cat_state::via1_ca2_w )
{
	logerror(" VIA1: CA2 written with %d!\n", state);
}

READ8_MEMBER( cat_state::via1_pb_r )
{
	logerror(" VIA1: Port B read!\n");
	return 0xFF;
}

WRITE8_MEMBER( cat_state::via1_pb_w )
{
	logerror(" VIA1: Port B written with data of 0x%02x!\n", data);
}

WRITE_LINE_MEMBER ( cat_state::via1_cb1_w )
{
	logerror(" VIA1: CB1 written with %d!\n", state);
}

WRITE_LINE_MEMBER ( cat_state::via1_cb2_w )
{
	logerror(" VIA1: CB2 written with %d!\n", state);
}

WRITE_LINE_MEMBER ( cat_state::via1_int_w )
{
	logerror(" VIA1: INT output set to %d!\n", state);
}

WRITE_LINE_MEMBER( cat_state::write_acia_clock )
{
	m_acia6850->write_txc(state);
	m_acia6850->write_rxc(state);
}

static MACHINE_CONFIG_START( swyft, cat_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68008, XTAL_15_8976MHz/2) //MC68008P8, Y1=15.8976Mhz, clock GUESSED at Y1 / 2
	MCFG_CPU_PROGRAM_MAP(swyft_mem)

	MCFG_MACHINE_START_OVERRIDE(cat_state,swyft)
	MCFG_MACHINE_RESET_OVERRIDE(cat_state,swyft)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(320, 242)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 242-1)
	MCFG_SCREEN_UPDATE_DRIVER(cat_state, screen_update_swyft)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	MCFG_VIDEO_START_OVERRIDE(cat_state,swyft)

	MCFG_DEVICE_ADD("acia6850", ACIA6850, 0)
	// acia rx and tx clocks come from one of the VIA pins and are tied together, fix this below? acia e clock comes from 68008
	MCFG_DEVICE_ADD("acia_clock", CLOCK, (XTAL_15_8976MHz/2)/5) // out e clock from 68008, ~ 10in clocks per out clock
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(cat_state, write_acia_clock))

	MCFG_DEVICE_ADD("via6522_0", VIA6522, (XTAL_15_8976MHz/2)/5) // out e clock from 68008
	MCFG_VIA6522_READPA_HANDLER(READ8(cat_state, via0_pa_r))
	MCFG_VIA6522_READPB_HANDLER(READ8(cat_state, via0_pb_r))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(cat_state, via0_pa_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(cat_state, via0_pb_w))
	MCFG_VIA6522_CB1_HANDLER(WRITELINE(cat_state, via0_cb1_w))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(cat_state, via0_ca2_w))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(cat_state, via0_cb2_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(cat_state, via0_int_w))

	MCFG_DEVICE_ADD("via6522_1", VIA6522, (XTAL_15_8976MHz/2)/5) // out e clock from 68008
	MCFG_VIA6522_READPA_HANDLER(READ8(cat_state, via1_pa_r))
	MCFG_VIA6522_READPB_HANDLER(READ8(cat_state, via1_pb_r))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(cat_state, via1_pa_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(cat_state, via1_pb_w))
	MCFG_VIA6522_CB1_HANDLER(WRITELINE(cat_state, via1_cb1_w))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(cat_state, via1_ca2_w))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(cat_state, via1_cb2_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(cat_state, via1_int_w))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( swyft )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v331", "IAI Swyft Version 331 Firmware")
	ROMX_LOAD( "331-lo.u30", 0x0000, 0x8000, CRC(d6cc2e2f) SHA1(39ff26c18b1cf589fc48793263f280ef3780cc61), ROM_BIOS(1))
	ROMX_LOAD( "331-hi.u31", 0x8000, 0x8000, CRC(4677630a) SHA1(8845d702fa8b8e1a08352f4c59d3076cc2e1307e), ROM_BIOS(1))
	/* this version of the swyft code identifies itself at 0x3FCB as version 330 */
	ROM_SYSTEM_BIOS( 1, "v330", "IAI Swyft Version 330 Firmware")
	ROMX_LOAD( "infoapp.lo.u30", 0x0000, 0x8000, CRC(52c1bd66) SHA1(b3266d72970f9d64d94d405965b694f5dcb23bca), ROM_BIOS(2))
	ROMX_LOAD( "infoapp.hi.u31", 0x8000, 0x8000, CRC(83505015) SHA1(693c914819dd171114a8c408f399b56b470f6be0), ROM_BIOS(2))
	ROM_REGION( 0x4000, "pals", ROMREGION_ERASEFF )
	/* Swyft PALs:
	 * The Swyft has four PALs, whose rough function can be derived from their names:
	 * TIMING - state machine for DRAM refresh/access; handles ras/cas and choosing whether the video out shifter or the 68k is accessing ram. also divides clock
	 * DECODE - address decoder for the 68008
	 * VIDEO - state machine for the video shifter (and vblank/hblank?)
	 * DISK 3.5 - state machine for the floppy drive interface
	 */
	/* U9: Timing AMPAL16R4
	 *
	 * pins:
	 * 111111111000000000
	 * 987654321987654321
	 * ??QQQQ??EIIIIIIIIC
	 * |||||||||||||||||\-< /CK input - 15.8976mhz crystal and transistor oscillator
	 * ||||||||||||||||\--< ?
	 * |||||||||||||||\---< ?
	 * ||||||||||||||\----< ?
	 * |||||||||||||\-----< ?<also input to decode pal pin 1, video pal pin 1, source is ?>
	 * ||||||||||||\------< ?
	 * |||||||||||\-------< ?
	 * ||||||||||\--------< ?
	 * |||||||||\---------< ?
	 * ||||||||\----------< /OE input - shorted to GND
	 * |||||||\-----------? ?
	 * ||||||\------------? ?
	 * |||||\------------Q> /ROM_OE (to both eproms through jumper b1 and optionally j9 connector)
	 * ||||\-------------Q? ?
	 * |||\--------------Q? ?
	 * ||\---------------Q> output to decode pal pin 2
	 * |\----------------->? output? to ram multiplexer 'A' pins
	 * \------------------< ?
	 */
	ROM_LOAD( "timing_b.ampal16r4a.u9.jed", 0x0000, 0xb08, CRC(643e6e83) SHA1(7db167883f9d6cf385ce496d08976dc16fc3e2c3))
	/* U20: Decode AMPAL16L8
	 *
	 * pins:
	 * 111111111000000000
	 * 987654321987654321
	 * O??????OIIIIIIIIII
	 * |||||||||||||||||\-< TIMING PAL pin 5
	 * ||||||||||||||||\--< TIMING PAL pin 17
	 * |||||||||||||||\---< 68008 R/W (pin 30)
	 * ||||||||||||||\----< 68008 /DS (pin 29)
	 * |||||||||||||\-----< 68008 E (pin 38)
	 * ||||||||||||\------< 68008 A19
	 * |||||||||||\-------< 68008 A18
	 * ||||||||||\--------< 68008 A17
	 * |||||||||\---------< 68008 A16
	 * ||||||||\----------< ?
	 * |||||||\-----------> ?
	 * ||||||\------------? 68008 /VPA (pin 39)
	 * |||||\-------------> /ROM_OE (to both eproms through jumper b1 and optionally j9 connector)
	 * ||||\--------------? ?
	 * |||\---------------? ?
	 * ||\----------------? ?
	 * |\-----------------? goes to j9 connector pin 5
	 * \------------------< ?
	 */
	ROM_LOAD( "decode_e.ampal16l8.u20.jed", 0x1000, 0xb08, CRC(0b1dbd76) SHA1(08c144ad7a7bbdd53eefd271b2f6813f8b3b1594))
	ROM_LOAD( "video_2b.ampal16r4.u25.jed", 0x2000, 0xb08, CRC(caf91148) SHA1(3f8ddcb512a1c05395c74ad9a6ba7b87027ce4ec))
	ROM_LOAD( "disk_3.5c.ampal16r4.u28.jed", 0x3000, 0xb08, CRC(fd994d02) SHA1(f910ab16587dd248d63017da1e5b37855e4c1a0c))
ROM_END

ROM_START( cat )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASEFF )
	// SYS ROM
	/* This 2.40 code came from two development cat machines owned or formerly
	 * owned by former IAI employees Sandy Bumgarner and Dave Boulton.
	 * Dave Boulton's machine is interesting in that it has a prototype cat
	 * motherboard in it, which it has less space for dram than a 'released'
	 * cat does: it uses 16k*4 dram chips instead of 64k*4 as in the final
	 * cat, and hence can only support 128k of ram with all 4 rows of drams
	 * populated, as opposed to 256k-standard (2 rows) and 512k-max with all
	 * 4 rows populated on a "released" cat.
	 */
	ROM_SYSTEM_BIOS( 0, "r240", "Canon Cat V2.40 US Firmware")
	ROMX_LOAD( "boultl0.ic2", 0x00001, 0x10000, CRC(77b66208) SHA1(9d718c0a521fefe4f86ef328805b7921bade9d89), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "boulth0.ic4", 0x00000, 0x10000, CRC(f1e1361a) SHA1(0a85385527e2cc55790de9f9919eb44ac32d7f62), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "boultl1.ic3", 0x20001, 0x10000, CRC(c61dafb0) SHA1(93216c26c2d5fc71412acc548c96046a996ea668), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "boulth1.ic5", 0x20000, 0x10000, CRC(bed1f761) SHA1(d177e1d3a39b005dd94a6bda186221d597129af4), ROM_SKIP(1) | ROM_BIOS(1))
	/* This 2.40 code was compiled by Dwight Elvey based on the v2.40 source
	 * code disks recovered around 2004. It does NOT exactly match the above
	 * set exactly but has a few small differences. One of the printer drivers
	 * may have been replaced by Dwight with an HP PCL4 driver.
	 * It is as of yet unknown whether it is earlier or later code than the
	 * set above.
	 */
	ROM_SYSTEM_BIOS( 1, "r240r", "Canon Cat V2.40 US Firmware compiled from recovered source code")
	ROMX_LOAD( "r240l0.ic2", 0x00001, 0x10000, CRC(1b89bdc4) SHA1(39c639587dc30f9d6636b46d0465f06272838432), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "r240h0.ic4", 0x00000, 0x10000, CRC(94f89b8c) SHA1(6c336bc30636a02c625d31f3057ec86bf4d155fc), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "r240l1.ic3", 0x20001, 0x10000, CRC(1a73be4f) SHA1(e2de2cb485f78963368fb8ceba8fb66ca56dba34), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "r240h1.ic5", 0x20000, 0x10000, CRC(898dd9f6) SHA1(93e791dd4ed7e4afa47a04df6fdde359e41c2075), ROM_SKIP(1) | ROM_BIOS(2))
	/* This v1.74 code comes from (probably) the 'main us release' of first-run
	 * Canon cats, and was dumped from machine serial number R12014979
	 * Canon cat v1.74 roms are labeled as r74; they only added the major number
	 * to the rom label after v2.0?
	 */
	ROM_SYSTEM_BIOS( 2, "r174", "Canon Cat V1.74 US Firmware")
	ROMX_LOAD( "r74__0l__c18c.blue.ic2", 0x00001, 0x10000, CRC(b19aa0c8) SHA1(85b3e549cfb91bd3dd32335e02eaaf9350e80900), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD( "r74__0h__75a6.yellow.ic4", 0x00000, 0x10000, CRC(75281f77) SHA1(ed8b5e37713892ee83413d23c839d09e2fd2c1a9), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD( "r74__1l__c8a3.green.ic3", 0x20001, 0x10000, CRC(93275558) SHA1(f690077a87076fd51ae385ac5a455804cbc43c8f), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD( "r74__1h__3c37.white.ic5", 0x20000, 0x10000, CRC(5d7c3962) SHA1(8335993583fdd30b894c01c1a7a6aca61cd81bb4), ROM_SKIP(1) | ROM_BIOS(3))
	// According to Sandy Bumgarner, there should be a 2.42 version which fixes some bugs in the calc command vs 2.40
	// According to the Cat Repair Manual page 4-20, there should be a version called B91U0x (maybe 1.91 or 0.91?) with sum16s of 9F1F, FF0A, 79BF and 03FF

	ROM_REGION( 0x80000, "svrom", ROMREGION_ERASE00 )
	// SPELLING VERIFICATION ROM (SVROM)
	/* Romspace here is a little strange: there are 3 rom sockets on the board:
	 * svrom-0 maps to 200000-21ffff every ODD byte (d8-d0)
	 * svrom-1 maps to 200000-21ffff every EVEN byte (d15-d7)
	 *  (since no rom is in the socket; it reads as open bus, sometimes 0x2E)
	 * svrom-2 maps to 240000-25ffff every ODD byte (d8-d0)
	 *  (since no rom is in the socket; it reads as open bus, sometimes 0x80)
	 * there is no svrom-3 socket; 240000-25ffff EVEN always reads as 0x2E
	 * since ROM_FILL16BE(0x0, 0x80000, 0x2e80) doesn't exist, the
	 * even bytes and latter chunk of the svrom space need to be filled in
	 * DRIVER_INIT or some other means needs to be found to declare them as
	 * 'open bus' once the mame/mess core supports that.
	 * NOTE: there are at least 6 more SVROMS which existed (possibly in
	 * limited form), and are not dumped:
	 * UK (1 rom, NH7-0724)
	 * French/Quebec (2 roms, NH7-0813/0814)
	 * German (3 roms, NH7-1019/1020/1021)
	 * Each of these will also have its own code romset as well.
	 */
	// NH7-0684 (US, dumped):
	ROMX_LOAD( "uv1__nh7-0684__hn62301apc11__7h1.ic6", 0x00000, 0x20000, CRC(229ca210) SHA1(564b57647a34acdd82159993a3990a412233da14), ROM_SKIP(1)) // this is a 28pin tc531000 mask rom, 128KB long; "US" SVROM

	/* There is an unpopulated PAL16L8 at IC9 whose original purpose (based
	 * on the schematics) was probably to cause a 68k bus error when
	 * memory in certain ranges when accessed (likely so 'forth gone insane'
	 * won't destroy the contents of ram and svram).
	 * Its connections are (where Ix = inp on pin x, Ox = out on pin x):
	 * I1 = A23, I2 = A22, I3 = A2, I4 = R/W, I5 = A5, I6 = FC2, I7 = gnd,
	 * I8 = A1, I9 = gnd, I11 = gnd, O16 = /BERR,
	 * I14 = REMAP (connects to emulator 'shadow rom' board or to gnd when unused)
	 * Based on the inputs and outputs of this pal, almost if not the entire
	 * open bus and mirrored areas of the cat address space could be made
	 * to cause bus errors. REMAP was probably used to 'open up' the A00000-A7ffff
	 * shadow rom/ram area and make it writeable without erroring.
	 */
ROM_END

/* Driver */

/*    YEAR  NAME  PARENT  COMPAT   MACHINE    INPUT    DEVICE         INIT     COMPANY   FULLNAME       FLAGS */
COMP( 1985, swyft,0,      0,       swyft,     swyft,   driver_device, 0,       "Information Applicance Inc", "Swyft", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1987, cat,  swyft,  0,       cat,       cat,     driver_device, 0,       "Canon",  "Cat", MACHINE_NOT_WORKING)
