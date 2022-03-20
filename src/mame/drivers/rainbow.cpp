// license:GPL-2.0+
// copyright-holders:Miodrag Milanovic,Karl-Ludwig Deisenhofer
/***************************************************************************************************
DEC Rainbow 100

Driver-in-progress by R. Belmont and Miodrag Milanovic.
Portions (2013 - 2018) by Karl-Ludwig Deisenhofer (Floppy, ClikClok RTC, NVRAM, DIPs, hard disk, Color Graphics).
Baud rate generator by AJR (2018) and Shattered (2016), keyboard & GDC fixes by Cracyc (June - Nov. 2016).

Model B implementation has a workaround prevents a side effect of ERROR 13 to unlock floppy drives A-D.

Native single sided 5.25" images with 80 tracks, 10 sectors are well tested (*.IMD / *.TD0=TeleDisk / *.IMG with 400 K).
VT180 images (184.320 Bytes) are very unreliable in CP/M - though a real machine can read them.
5.25 MFM PC style drives and 720 K (3.5 " DS-DD MFM PC formatted disks) (on slots 3 + 4) show regressions / bugs
  as of Dec.2018 (file content bad while dir is OK, seek errors, write fault errors when copying _to_ hard disk).

ALWAYS USE THE RIGHT SLOT AND SAVE YOUR DATA BEFORE MOUNTING FOREIGN DISK FORMATS!

You * should * also reassign SETUP (away from F3, where it sits on a LK201).
DATA LOSS POSSIBLE: when in partial emulation mode, F3 performs a hard reset!

STATE AS OF DECEMBER 2018
-------------------------
Driver is based entirely on the DEC-100 'B' variant (DEC-190 and DEC-100 A models are treated as clones).
While this is OK for the compatible -190, it doesn't do justice to ancient '100 A' hardware.
The public domain file RBCONVERT.ZIP documents how model 'A' differs from version B.
NVRAM files from -A and -B machines are not interchangeable. If problems arise, delete the NVRAM file.

Venix 86-R (BSW) is working, just follow https://github.com/bsdimp/venix/blob/master/doc/MESS-RB-INSTALL.md

CPM 2.1 / DOS2.11 / DOS 3.x / diag disks boot. UCSD systems (fort_sys, pas_sys) boot, but expect 4 QD drives
  loaded with disks (reassign slots, reset and mount three empty 400 K images before startup at #2, #3, #4).

It is possible to boot DOS 3.10 from floppy A: and later use a hard disk attached to E:.

NB.: a single hard disk (5 - 67 MB, 512 byte sectors) may be attached before startup. It should remain there
until shutdown. "Hot swapping" wasn't possible on the original system (our GUI just doesn't forbid it).

To create a DEC RD50/ST506 compatible image (153 cylinders, 4 heads, 16 sectors, standard 512 byte sectors) enter
>chdman createhd -c none -chs 153,4,16 -ss 512 -o RD50_ST506.chd
NOTE: use -c none parameter for no compression. No more than 8 heads or 1024 cylinders.

Some BUGS remain: BIOS autoboot doesnt work at all. It is not possible to boot from a properly formatted
 winchester with "W" (CPU crash). So there's an issue with the secondary boot loader (for hard disks)...

CTRL-SETUP (soft reboot) always triggers ERROR 19 (64 K RAM err.). One explanation is that ZFLIP/ZRESET is
handled wrongly, so shared mem. just below $8000 is tainted by Z80 stack data. A reentrance problem?

Occassionally, ERROR 13 -keyboard stuck- appears (for reasons yet unknown).


CORVUS HARD DISK
----------------
Up to 4 Corvus Disks with up to 20 MB each can be emulated (to be mounted as hard disks 2 - 5).
MS DOS 2.x and CP/M v2.x were once supported, but are untested (in part because no binary drivers have survived).

To get a Corvus 11 drive up and running under CP/M 1.x, you'll need drcdutil.td0 from Donald Maslin's Archive.

First, create a 11 MB hard disk:
>Chdman createhd -c none -chs 306,4,20 -ss 512 -o CORVUS11.chd
[ -chs 306,2,20 for the 6 MB model and  -chs 306,6,20 for the 20 MB type ]

Then make a copy of your CP/M 86-80 V1.x boot disk. This copy must be patched to make the Corvus hard drive usable!
With 'drcdutil.td0' mounted in A: and a write enabled (non TeleDisk) image of CPM 1.x in B: type:
b:>SUBMIT A:hinstall

This replaces the following CP/M files on B:
   B:Z80CCP.SYS  <- A:HZ80CCP.SYS
   B:Z80.SYS     <- A:HZ80.SYS
   B:PRMTVPV.SYS <- A:HPRMTVPV.SYS

Due to a missing drive specification in HINSTALL.SUB, the last PIP must be invoked manually:
b:>PIP B:PRMTVPVT.SYS=A:HPRMTVPV.SYS[V]

Finally, boot from the newly patched CP/M disk and type CLINK2TN (a step necessary after each cold boot).
CLINK2TN can only be used together with a Corvus 11 MB hard disk. It needs a patched CP/M 1.x disk and won't run on CP/M 2.x.
[ use CLINK2FV for the 6 MB model and CLINK2TW for the 20 MB type ]

Two steps are needed to initialize the new disk:
Step 1: invoke PUTGET, then press "f". Enter "Drive no: 1", "HEX BYTE? e5", "Starting disc address?  2320", "Number of Sectors? 64"
Step 2: invoke PUTGET, then press "f". Enter "Drive no: 1", "HEX BYTE? e5", "Starting disc address?  48592", "Number of Sectors? 64"
Done.

Required steps vary with 5 and 20 MB models (look into the *.DOC files in DRCDUTIL.TD0 / CLINK86.A86 / DRIVEL.COM).
Parameters for initialization can be taken from Chapter 2 of the Disk System Installion Guide for TRS-80 II (same type H drives).


COLOR EMULATION (NEC 7220 + extra hardware)
-------------------------------------------

-------------------- Differences to VT240: ---------------------------------------------------
   - Registers of graphics option not directly mapped (indirect access via mode register)
   - write mask is 16 bits wide (not only 8)
   - scroll register is 8 bits wide - not 16.
   - no "LINE ERASE MODE", 7220 DMA lines are unused. No ZOOM hardware (factor must always be 1)

   Two modes: highres and medres mode (different bank length..?)
   - MEDRES: palette of 16 colors out of 4096.   384 x 240
   - HIGRES: palette of  4 colors out of 4096.   800 x 240
   Palette takes 2 byte per palette entry. CLUT ("color map") is 32 byte long.
------------------------------------------------------------------------------------------------

DEC 'R-M-B' COLOR CABLE VS. THE UNOFFICIAL 'R-G-B' MODE (a bit of history)
   (1) the standard DEC "color cable" connected the green gun of a VR241 to the mono output of the Rainbow
   (2) an unofficial DIY cable enabled R-G-B graphics + separate text

EMULATION SPECIFIC
   (1) COLOR_MONITOR reflects DEC's recommendation (R-M-B with VR241 above)
   (2) DUAL MONITOR enables both screens, even if onboard graphics has been accidently shut off
       (also helps debugging semi broken programs, for example Doodle).
   (3) AUTODETECT (DIP setting) snoops the color palette and chooses the correct 'wiring'

SCREEN 1 vs. SCREEN 2 IN EMULATION
   All GDC 7220 output is displayed on the right. Be it color or monochrome, Option Graphics output is on screen 2.
   If you select MONO_MONITOR via DIP, output from GDC will appear on screen 2 in 16 shades of grey.
   The type of monochrome monitor (VR-210 A, B or C) is selectable via another DIP (coarsly simulates a phosphor color).

BUGS
- GDC diagnostic disk fails on 9 of 13 tests (tests 4 and 6 - 13).

Details
a. (Rainbow driver) : interaction between DEC's external hardware and the NEC 7220 isn't fully understood (see page 173 of AA-AE36A)
   It is also unclear what port $50 actually does when it 'synchronizes R-M-W cycles'.
   For now, we provide sane defaults for both vector and bitmap units without disturbing display mode(s) or the NEC 7220.
b. the Hblank / Vblank ratio is plainly wrong (quick test / subtest #6),
c. IRQs are flagged as 'erratic' (quick test / subtest #12).
d. (7220) : incorrect fifo stati are handed out (GDC reports FIFO_EMPTY instead of _FULL when quick test #4 floods the queue)
e. (7220) : RDAT with MOD 2 used extensively here, but unimplemented (modes other than 0 undocumented by NEC / Intel)

UNIMPLEMENTED:
- Rainbow 100 A palette quirks (2 bit palette... applies to certain modes only)

UNKNOWN IMPLEMENTATION DETAILS:
1. READBACK (hard copy programs like JOBSDUMP definitely use it. See also GDC diagnostics).  VRAM_R...?

2. UNVERIFIED DIVIDERS (31.188 Mhz / 32) is at least close to 1 Mhz (as on the VT240, which uses a very similar design)

3. UPD7220 / CORE oddities

To obtain pixel exact graphics use 'Graphics Only' in Video Options and cmd.line switches -nowindow -aspect1 auto -nokeepaspect
(Over-Under or Side-by-Side modes always distorted on my 1600 x 900 laptop)


CURRENTY UNEMULATED
-------------------
(a) the serial printer on port B prints garbage. It is worth to mention that port B relies on XON/XOFF,
    while DTR_L (CTS B) means 'printer ready'. There is also a ROM patch in place...

(b1) LOOPBACK circuit not emulated (used in startup tests).

(b2) system interaction tests HALT Z80 CPU at location $0211 (forever). Boot the RX50 diag.disk
 to see what happens (key 3 - individual tests, then 12 - system interaction). Uses LOOPBACK too?

(c) arbitration chip (E11; in 100-A schematics or E13 in -B) is dumped, but yet unemulated.
It is a 6308 OTP ROM (2048 bit, 256 x 8) used as a lookup table (LUT) with the address pins (A)
used as inputs and the data pins (D) as output.

Plays a role in DMA access to lower memory (limited to 64 K; Extended communication option only).
Arbiter is also involved in refresh and shared memory contention (affects Z80/8088 CPU cycles).

=> INPUTS on E13 (PC-100 B):

SH5 RF SH REQ H   -> Pin 19 (A7) shared memory request / refresh ?
     1K -> +5 V   -> Pin 18 (A6) < UNUSED >
SH 2 BDL ACK (L)  -> Pin 17 (A5) BUNDLE OPTION: IRQ acknowledged
SH 2 NONSHRCYC H  -> Pin 5 (A4) unshared memory cycle is in progress
SH 2 PRECHARGE H  -> Pin 4 (A3)
SH 2 SHMUX 88 ENB -> Pin 3 (A2) shared memory
SH2 DO REFRESH H  -> Pin 2 (A1) indicates that extended memory must be refreshed -> on J6 as (L)
SH10 BDL REQ (L)  -> Pin 1 (A0) BUNDLE OPTION wishes to use shared memory

HARDWARE UPGRADES WORTH EMULATING (should be implemented as SLOT DEVICES):
* Extended communication option (occupies BUNDLE_OPTION 1 + 2)  REFERENCE: AA-V172A-TV + Addendum AV-Y890A-TV.
Two ports, a high-speed RS-422 half-duplex interface (port A) + lower-speed RS-423 full/half-duplex interface
with modem control (port B). A 5 Mhz. 8237 DMA controller transfers data into and out of shared memory (not: optional RAM).

Uses SHRAM, SHMA, BDL SH WR L, NONSHARED CYCLE. Implementation requires DMA and arbitration logic (using dump of E11/E13 ?).
Can't be added if RD51 hard disk controller present (J4 + J5). For programming info see NEWCOM1.DOC (-> RBETECDOC.ZIP).

* ( NO DUMP YET ) PC CHARACTER SET (Suitable Solutions?). Supported by IBM PC software emulator named CodeBlue (see 3.1 patch)

* ( NO DUMP YET ) TECHNICAL CHARACTER SET (TCS; available for Rainbow 100, 100B, 100+; $95 from DEC)
Source: price list of a DEC reseller.
Contains 94 graphic characters from $A1 - $FE, including symbols and characters used in technical applications,
 see http://support.attachmate.com/techdocs/1184.html and http://vt100.net/charsets/technical.html

* 8087  Numerical Data Coprocessor daughterboard.       REFERENCE: EK-PCNDP-IN-PRE
Daughterboard, to be plugged into the expansion port where the memory expansion card usually sits (J6).
If a memory adapter board is present, it has to be plugged into a connector atop the 8087 copro board.
The 8088 is put into the CPU socket on the coprocessor board.
SOFTWARE: MATH test on 'Design Maturity Diagnostics'; AutoCad, TurboPascal and Fortran.

* Suitable Solutions TURBOW286: 12 Mhz, 68-pin, low power AMD N80L286-12 and WAYLAND/EDSUN EL286-88-10-B ( 80286 to 8088 Processor Signal Converter )
plus DC 7174 or DT 7174 (barely readable). Add-on card, replaces main 8088 cpu (via ribbon cable). Patched V5.03 BOOT ROM labeled 'TBSS1.3 - 3ED4'.

* NEC_V20 (requires modded BOOT ROM because of - at least 2 - hard coded timing loops):
100A:         100B/100+:                       100B+ ALTERNATE RECOMMENDATION (fixes RAM size auto-detection problems when V20 is in place.
Tested on a 30+ year old live machine. Your mileage may vary)

Location Data  Location Data                   Loc.|Data
....     ..    ....     ..  ------------------ 00C6 46 [ increases 'wait for Z80' from approx. 27,5 ms (old value 40) to 30,5 ms ]
....     ..    ....     ..  ------------------ 0303 00 [ disable CHECKSUM ]
043F     64    072F     64 <-----------------> 072F 73 [ increases minimum cycle time from 2600 (64) to 3000 ms (73) ]
067D     20    0B36     20 <-----------------> 0B36 20 [ USE A VALUE OF 20 FOR THE NEC - as in the initial patch! CHANGES CAUSE VFR-ERROR 10 ]
1FFE     2B    3FFE     1B  (BIOS CHECKSUM)
1FFF     70    3FFF     88  (BIOS CHECKSUM)

--------------------------------------------------------------
Meaning of Diagnostics LEDs (from PC100ESV1.PDF found, e.g.,
on ftp://ftp.update.uu.se/pub/rainbow/doc/rainbow-docs/

Internal Diagnostic Messages                               F
Msg Message                               Lights Display   A
No.                                       * = on o = off   T
..........................................- = on or off    A
..........................................1 2 3 4 5 6 7    L
--------------------------------------------------------------
.1  Main Board (Video)                    o * * o * o *   Yes
.2  Main Board* (unsolicited interrupt)   * * * * o * o   Yes
.3  Drive A or B (index)                  o o * o o * *
.4  Drive A or B (motor)                  * * o o o * *
.5  Drive A or B (seek)                   o * o o o * *
.6  Drive A or B (read)                   * o o o o * *
.7  Drive A or B (restore)                o * * o o * *
.8  Drive A or B (step)                   * o * o o * *
.9  System Load incomplete+ (System Load) o o o o o o o
10  Main Board (video, vfr)               * * * o * o *   Yes
11  System Load incomplete+ (Boot Load)   o o o o o o o
12  Drive A or B (not ready)              o o o o o * *
13  Keyboard                              * * o * o * o   Yes
14  Main Board (nvm data)                 * * * * o * *
15  (no msg. 15 in that table)
16  Interrupts off*                       * * * o o o o   Cond.
17  Main Board (video RAM)                * * * o * * o   Yes
18  Main Board (Z80 crc)                  * * * * o o *   Yes
19  Main Board RAM (0-64K)                - - - * * o *   Yes
20  Main Board (unsolicited int., Z80)    * * * o o o *   Yes
21  Drive Not Ready+                      o o o o o o o
22  Remove Card or Diskette               o * * o o o *
23  Non-System Diskette+                  o o o o o o o
24  new memory size = nnnK                o o o o o o o
25  Set Up Defaults stored                o o o o o o o
26  Main Board (RAM arbitration)          * * * o * o o   Yes
27  Main Board (RAM option)               - - - * * o o
28  RX50 controller board                 * * * o o * *
29  Main Board* (Z80 response)            * * * * o o o
30  Main Board (ROM crc, ROM 0)           * * * * * * *   Yes
31  Main Board (ROM crc, ROM 1)           * * * * * * o   Yes
-   Main Board (ROM crc, ROM 2)           * * * o * * *   Yes
33  Main Board (contention)               o o o o o * o   Yes
40  Main Board (printer port)             * o * * o * o
50  Main Board (keyboard port)            o o * * o * o   Yes
60  Main Board (comm port)                o * * * o * o
--------------------------------------------------------------
*   These errors can occur at any time because the circuits
are monitored constantly
+   These messages may occur during power-up if auto boot is
selected

PCB layout
==========

DEC-100 model B
= part no.70-19974-02 according to document EK-RB100-TM_001

PCB # 5416206 / 5016205-01C1:

7-6-5-4 |3-2-1
DIAGNOSTIC-LEDs |J3   | |J2     | |J1    |
|------|----8088|Z80-|--|VIDEO|-|PRINTER|-|SERIAL|----|
|  2 x 64 K             |/KBD.|                  !!!!!|
|  R  A  M              NEC D7201C            |P|!W90!|
|                                             |O|!!!!!|
|   [W6]    ROM 1       INTEL 8088            |W|     |
|           (23-020e5-00)                     |E|     |
|                                             |R|     |
| ...J5..   BOOT ROM 0      ...J4...          =J8     |
|           (23-022e5-00)                             |
| ...J6...                                            |
| [W5]                                                |
|                                                     |
|     INTEL 8251A   ZILOG Z 80A                       |
|                [W18]                                |
| A  4x                74 LS 244                      |
| M  S           [W15]                                |
| 9  -   DEC-DC011     74 LS 245                      |
| 1  R           [W14]                                |
| 2  A                  [W13]                         |
| 8  M   CHARGEN.-                                    |
|        ROM (4K)            ...J7...  | ...J9 = RX50 |
|                                                     |
|-------------PCB# 5416206 / 5016205-01C1-------------|

CONNECTORS ("J"):
    ...J5... ...J4... both: RD51 controller (hard disk)
    ...J5... ...J4... both: EXTENDED COMM. controller

    ...J6... is the MEMORY OPTION connector (52 pin)
    ...J7... is the GRAPHICS OPTION connector (40 pin)
    ...J9... RX50 FLOPPY CONTROLLER (40 pin; REQUIRED)

JUMPERS (labeled "W"):
  W5 + W6 are out when 16K x 8 EPROMS are used
/ W5 + W6 installed => 32 K x 8 EPROMs (pin 27 = A14)

W13, W14, W15, W18 = for manufacturing tests.
=> W13 - W15 affect diagnostic read register (port $0a)
=> W18 pulls DSR to ground and affects 8251A - port $11 (bit 7)

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! DO NOT SHORT JUMPER / CONNECTOR [W90] ON LIVE HARDWARE  !!
!!                                                         !!
!! WARNING:  CIRCUIT DAMAGE could occur if this jumper is  !!
!! set by end users.        See PDF document AA-V523A-TV.  !!
!!                                                         !!
!! W90 connects to pin 2 (Voltage Bias on PWR connector J8)!!
!! and is designed FOR ===> FACTORY TESTS OF THE PSU <===  !!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

WIRE CONNECTORS - SEEN ON SCHEMATICS - NOT PRESENT ON DEC-100 B (-A only?):
W16 pulls J2 printer port pin 1 to GND when set (chassis to logical GND).
W17 pulls J1 serial  port pin 1 to GND when set (chassis to logical GND).
****************************************************************************/
#include "emu.h"

#include "cpu/i86/i86.h"
#include "cpu/z80/z80.h"
#include "video/vtvideo.h"
#include "video/upd7220.h"

#include "machine/wd_fdc.h"
#include "formats/rx50_dsk.h"
#include "formats/pc_dsk.h" // PC Formats
#include "imagedev/floppy.h"

#include "imagedev/harddriv.h"
#include "machine/wd2010.h"
#include "machine/corvushd.h"

#include "machine/z80sio.h"
#include "bus/rs232/rs232.h"
#include "imagedev/bitbngr.h"
#include "machine/com8116.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/terminal.h"

#include "machine/i8251.h"
#include "machine/dec_lk201.h"
#include "machine/nvram.h"
#include "machine/ripple_counter.h"
#include "machine/timer.h"
#include "machine/ram.h"

#include "machine/ds1315.h"
#include "emupal.h"
#include "softlist.h"
#include "screen.h"

#include "rainbow.lh" // BEZEL - LAYOUT with LEDs for diag 1-7, keyboard 8-11 and floppy 20-23

#define RD51_MAX_HEAD 8
#define RD51_MAX_CYLINDER 1024
#define RD51_SECTORS_PER_TRACK 17

#define RTC_ENABLED
// Tested drivers (from Suitable Solutions distribution disk and Latrobe archive), preferred first -
// File.........Version / author ------------------- YY/YYYY ----- Read only RTC_BASE ---- Platform
// RBCLIK21.COM Author: Vincent Esser. With source.. 4 digits (Y2K)..Y.......$fc000/fe000..100-B (default cfg.)
// CLIKA.COM .. V1.03A (C) 1987 Suitable Solutions.. 2 digits........N (*)...$ed000........100-A
// CLIKCLOK.COM V1.01 (C) 1986,87 Suitable Solutions 2 digits........N (*)...$fc000/fe000..100-B (default   " )
// CLIKF4.COM . V1.0  (C) 1986 Suitable Solutions... 2 digits........N (*)...$f4000........100-B (alternate " )
// (*)   Time or date changes are not persistent in emulation. To prove the setter works, changes are logged.

// (Y2K) DS1315 unit only holds 2 digits, so Vincent Esser's freeware employs a windowing technique.
//       While Suitable's DOS 3.10 accepts dates > 2000, don't take that for granted with software from the 80s.
//  Model B can have an RTC mapped at 0xF4000 instead - ClikClok V1.0 / CLIKF4.COM

// ----------------------------------------------------------------------------------------------
// * MHFU disabled by writing a _sensible_ value to port 0x10C (instead of port 0x0c)
// Note: documentation incorrectly claims that zero must be written to 0x10C.

// * MHFU re-enabled by writing to 0x0c.
// DEC says that MHFU is also re-enabled 'automatically after STI' (when under BIOS control?)

// Schematics show "VERT FREQ INT" (= DC012 output, pin 2) and MHFU ENBL L are evaluated,
//  as well as the power good signal from the PSU (AC_OK). MS_TO_POWER_GOOD is a guess:
#define MS_TO_POWER_GOOD 350
// Reset duration of 108 ms from documentation -
#define RESET_DURATION_MS 108

// Driver uses an IRQ callback from the 8088 -and a counter- to determine if the CPU is alive.
// Counter is reset by writing to 0x10c, or by acknowledging (!) a VBL IRQ within 108 ms.
#define MHFU_IS_ENABLED 1
#define MHFU_COUNT -1
#define MHFU_VALUE -2
#define MHFU_RESET_and_ENABLE   -100
#define MHFU_RESET_and_DISABLE  -200
#define MHFU_RESET              -250

// ----------------------------------------------------------------------------------------------
// NEC 7220 GDC     *************************************

// Indirect Register, port $53, see page 181 of AA-AE36A (PDF):
// (actual values : see comments)
#define GDC_SELECT_WRITE_BUFFER  0x01 // 0xFE
#define GDC_SELECT_PATTERN_MULTIPLIER 0x02 // 0xFD
#define GDC_SELECT_PATTERN       0x04 // 0xFB
#define GDC_SELECT_FG_BG         0x08 // 0xF7
#define GDC_SELECT_ALU_PS        0x10 // 0xEF
#define GDC_SELECT_COLOR_MAP     0x20 // 0xDF
#define GDC_SELECT_MODE_REGISTER 0x40 // 0xBF
#define GDC_SELECT_SCROLL_MAP    0x80 // 0x7F

// MODE REGISTER
#define GDC_MODE_HIGHRES        0x01
#define GDC_MODE_VECTOR         0x02

// ( " ) READBACK OPERATION  (if ENABLE_WRITES = 0):
#define GDC_MODE_ENABLE_WRITES       0x10
#define GDC_MODE_READONLY_SCROLL_MAP 0x20

// ( " )  READBACK OPERATION  (plane select = bit mask in bits 2 + 3 of MODE register):
#define GDC_MODE_READBACK_PLANE_MASK 12
#define GDC_MODE_READBACK_PLANE_00  0x00
#define GDC_MODE_READBACK_PLANE_01  0x04
#define GDC_MODE_READBACK_PLANE_02  0x08
#define GDC_MODE_READBACK_PLANE_03  0x0c

#define GDC_MODE_ENABLE_VSYNC_IRQ 0x40
#define GDC_MODE_ENABLE_VIDEO   0x80

// ALU_PS REGISTER (bits 5 + 4):
#define ALU_PS_MODE_MASK 48
#define REPLACE_MODE    00
#define COMPLEMENT_MODE 16
#define OVERLAY_MODE    32

// ----------------------------------------------------------------------------------------------
#define LK201_TAG   "lk201"
#define FD1793_TAG  "fd1793x"

#define INVALID_DRIVE 255
#define MAX_FLOPPIES 4

// Monitor configurations -> see DIP switches. New: auto-detect color palette (last option).
static constexpr int MONO_MONITOR = 0x01;  // Tetris-M and Pacman-M need this setting (no auto-detection)
static constexpr int COLOR_MONITOR = 0x02; // DEC recommendation. GWBASIC and most old libraries. Superseded by later development
static constexpr int DUAL_MONITOR = 0x03;  // Debugging, AutoCad, 'newer' freeware. Green is missing with unpatched software (for technical reasons)
static constexpr int AUTODETECT_MONITOR = 0x04;  // Snoop palette, then choose best output.

class rainbow_base_state : public driver_device
{
public:
	rainbow_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),

		m_inp1(*this, "W13"),
		m_inp2(*this, "W14"),
		m_inp3(*this, "W15"),
		m_inp4(*this, "W18"),
		m_inp5(*this, "DEC_HARD_DISK"), // DO NOT CHANGE ORDER
		m_inp6(*this, "CORVUS_HARD_DISKS"), // DO NOT CHANGE ORDER
		m_inp7(*this, "GRAPHICS_OPTION"),   // DO NOT CHANGE ORDER
		m_inp9(*this, "MONO_MONITOR_TYPE"),
		m_inp10(*this, "J17"),
		m_inp11(*this, "CLIKCLOK"),
		m_inp12(*this, "WATCHDOG"),
		m_inp13(*this, "MONITOR_CONFIGURATION"),

		m_crtc(*this, "vt100_video"),

		m_i8088(*this, "maincpu"),
		m_z80(*this, "subcpu"),
		m_ram(*this, "ram"),

		m_fdc(*this, FD1793_TAG),
		m_floppies(*this, FD1793_TAG ":%u", 0U),
		m_hdc(*this, "hdc"),
		m_corvus_hdc(*this, "corvus"),

		m_mpsc(*this, "mpsc"),
		m_dbrg(*this, "dbrg"),
		m_comm_port(*this, "comm"),

		m_kbd8251(*this, "kbdser"),
		m_lk201(*this, LK201_TAG),

		m_p_ram(*this, "p_ram"),

		m_p_vol_ram(*this, "vol_ram"),
		m_p_nvram(*this, "nvram"),

		m_rtc(*this, "rtc"),
		m_hgdc(*this, "upd7220"), // GDC

		m_screen2(*this, "screen2"),
		m_palette2(*this, "palette2"), // GDC
		m_video_ram(*this, "vram"),

		m_digits(*this, "digit%u", 0U),
		m_leds(*this, "led%u", 1U),
		m_driveleds(*this, "driveled%u", 0U)
	{
	}

	void rainbow_base(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	void rainbow8088_base_map(address_map &map);
	void rainbow8088_base_io(address_map &map);

	uint8_t ext_ram_r(offs_t offset);

	void rtc_w(offs_t offset, uint8_t data);

	uint8_t read_video_ram_r(offs_t offset);
	DECLARE_WRITE_LINE_MEMBER(video_interrupt);

	uint8_t diagnostic_r();
	void diagnostic_w(uint8_t data);

	uint8_t comm_control_r();
	void comm_control_w(uint8_t data);

	uint8_t share_z80_r(offs_t offset);
	void share_z80_w(offs_t offset, uint8_t data);

	// 'RD51' MFM CONTROLLER (WD1010) *************************************
	uint8_t hd_status_60_r(); // TRI STATE DATA PORT (R/W)
	void hd_status_60_w(uint8_t data);

	uint8_t hd_status_68_r(); // EXTRA REGISTER 0x68 (R/W 8088)
	void hd_status_68_w(uint8_t data);

	uint8_t hd_status_69_r(); // EXTRA REGISTER 0x69 (R/- 8088)

	DECLARE_WRITE_LINE_MEMBER(bundle_irq);
	DECLARE_WRITE_LINE_MEMBER(hdc_bdrq);  // BUFFER DATA REQUEST (FROM WD)
	DECLARE_WRITE_LINE_MEMBER(hdc_bcr);   // BUFFER COUNTER RESET (FROM WD)

	DECLARE_WRITE_LINE_MEMBER(hdc_step);
	DECLARE_WRITE_LINE_MEMBER(hdc_direction);

	DECLARE_WRITE_LINE_MEMBER(hdc_read_sector);
	DECLARE_WRITE_LINE_MEMBER(hdc_write_sector);

	DECLARE_READ_LINE_MEMBER(hdc_drive_ready);
	DECLARE_READ_LINE_MEMBER(hdc_write_fault);

	uint8_t corvus_status_r();

	uint8_t i8088_latch_r();
	void i8088_latch_w(uint8_t data);
	uint8_t z80_latch_r();
	void z80_latch_w(uint8_t data);

	void z80_diskdiag_read_w(uint8_t data);
	void z80_diskdiag_write_w(uint8_t data);

	uint8_t z80_generalstat_r();

	uint8_t z80_diskstatus_r();
	void z80_diskcontrol_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(kbd_tx);
	DECLARE_WRITE_LINE_MEMBER(kbd_rxready_w);
	DECLARE_WRITE_LINE_MEMBER(kbd_txready_w);

	uint8_t rtc_reset();
	uint8_t rtc_enable();

	DECLARE_WRITE_LINE_MEMBER(mpsc_irq);
	void comm_bitrate_w(uint8_t data);
	void printer_bitrate_w(uint8_t data);
	void bitrate_counter_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(dbrg_fr_w);
	DECLARE_WRITE_LINE_MEMBER(dbrg_ft_w);

	void GDC_EXTRA_REGISTER_w(offs_t offset, uint8_t data);
	uint8_t GDC_EXTRA_REGISTER_r(offs_t offset);

	uint32_t screen_update_rainbow(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	IRQ_CALLBACK_MEMBER(irq_callback);

	TIMER_DEVICE_CALLBACK_MEMBER(hd_motor_tick);

	static void floppy_formats(format_registration &fr);

	UPD7220_DISPLAY_PIXELS_MEMBER( hgdc_display_pixels );
	uint16_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint16_t data);
	DECLARE_WRITE_LINE_MEMBER(GDC_vblank_irq);

	void rainbowz80_io(address_map &map);
	void rainbowz80_mem(address_map &map);
	void upd7220_map(address_map &map);
	enum
	{   // LOWEST PRIORITY
		// Mnemonic - - - - - -  TYPE  ADDRESS - Source
		//                      [1][0]  [1][0] <= Depends on DTR(L) output of keyboard PUSART (on Rainbow-100 B)
		IRQ_8088_MAILBOX = 0, // 27/A7  9C/29C  - [built-in] Interrupt from Z80A
		IRQ_8088_KBD,         // 26/A6  98/298  - [built-in] KEYBOARD Interrupt - 8251A
		IRQ_BDL_INTR_L,       // 25/A5  94/294  - [ext. BUNDLE OPTION] Hard disk or Extended communication IRQ (no DMA)
		IRQ_COMM_PTR_INTR_L,  // 24/A4  90/290  - [built-in 7201] Communication/Printer interrupt
		IRQ_DMAC_INTR_L,      // 23/A3  8C/28C  - [ext. COMM.BOARD only] - external DMA Controller (8237) interrupt
		IRQ_GRF_INTR_L,       // 22/A2  88/288  - [ext. COLOR GRAPHICS]
		IRQ_BDL_INTR_1L,      // 21/A1  84/284  - [ext. COMM.BOARD only]
		IRQ_8088_VBL,         // 20/A0  80/280  - [built-in DC012] - VERT INTR L (= schematics)
		IRQ_8088_NMI          // 02/02  08/08   - [external MEMORY EXTENSION] - PARITY ERROR L
	};  // HIGHEST PRIORITY

	required_ioport m_inp1;
	required_ioport m_inp2;
	required_ioport m_inp3;
	required_ioport m_inp4;
	required_ioport m_inp5;
	required_ioport m_inp6;
	required_ioport m_inp7;
	required_ioport m_inp9;
	required_ioport m_inp10;
	required_ioport m_inp11;
	required_ioport m_inp12;
	required_ioport m_inp13;
	required_device<rainbow_video_device> m_crtc;
	required_device<cpu_device> m_i8088;
	required_device<cpu_device> m_z80;
	required_device<ram_device> m_ram;

	required_device<fd1793_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppies;
	optional_device<wd2010_device> m_hdc;

	required_device<corvus_hdc_device> m_corvus_hdc;

	required_device<upd7201_device> m_mpsc;
	required_device<com8116_003_device> m_dbrg;
	required_device<rs232_port_device> m_comm_port;

	required_device<i8251_device> m_kbd8251;
	required_device<lk201_device> m_lk201;
	required_shared_ptr<uint8_t> m_p_ram;
	required_shared_ptr<uint8_t> m_p_vol_ram;
	required_shared_ptr<uint8_t> m_p_nvram;

	optional_device<ds1315_device> m_rtc;

	required_device<upd7220_device> m_hgdc;  // GDC
	required_device<screen_device> m_screen2;
	required_device<palette_device> m_palette2;
	required_shared_ptr<uint16_t> m_video_ram;

	output_finder<2> m_digits;
	output_finder<7> m_leds;
	output_finder<4> m_driveleds;

	void raise_8088_irq(int ref);
	void lower_8088_irq(int ref);

	void update_mpsc_irq();
	int m_mpsc_irq;
	void update_8088_irqs();

	void update_bundle_irq(); // RD51 or COMM.OPTION!
	int do_write_sector();
	void hdc_buffer_counter_reset();
	void hdc_reset();

	hard_disk_file *rainbow_hdc_file(int ref);

	uint8_t m_gdc_write_buffer[16]; // 16 x 8 bits for CPU, 8 x 16 for GDC
	uint8_t m_gdc_color_map[32];
	uint8_t m_gdc_scroll_buffer[256];

	uint8_t  m_gdc_indirect_register;
	uint8_t  m_gdc_mode_register;
	uint8_t  m_gdc_scroll_index;
	uint8_t  m_gdc_color_map_index;
	uint8_t  m_gdc_write_buffer_index;
	uint8_t  m_gdc_alu_ps_register;
	uint8_t  m_gdc_fg_bg;
	uint8_t  m_vpat, m_patmult, m_patcnt, m_patidx;

	uint16_t m_gdc_write_mask;

	bool m_onboard_video_selected;   // (internal switch, on board video to mono out)
	bool m_screen_blank;

	uint8_t m_monitor_suggested;

	int m_int88;
	int m_intz80;

	bool m_zflip;                   // Z80 alternate memory map with A15 inverted
	bool m_z80_halted;
	int  m_z80_diskcontrol;         // retains values needed for status register

	uint8_t m_printer_bitrate;

	bool m_kbd_tx_ready, m_kbd_rx_ready;
	int m_KBD;

	uint8_t m_diagnostic;

	uint8_t m_z80_private[0x800];     // Z80 private 2K
	uint8_t m_z80_mailbox, m_8088_mailbox;

	void update_kbd_irq();

	int m_present_drive;
	floppy_image_device *m_floppy;

	int m_irq_high;
	uint32_t m_irq_mask;

	int m_bdl_irq;
	int m_hdc_buf_offset;

	bool m_hdc_index_latch;
	bool m_hdc_step_latch;
	int m_hdc_direction;
	bool m_hdc_write_gate;

	bool m_hdc_drive_ready;
	bool m_hdc_write_fault;

	uint8_t m_hdc_buffer[2048];

	bool m_power_good;
	emu_timer   *cmd_timer;
	emu_timer   *switch_off_timer;

	const int vectors[9] = { 0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21, 0x20, 0x02 };

	// VIDEO LEVELS:  0 is 100 % output; F is 0 % output. Range of 0...255.
	// LIMITED RANGE levels for 100-A model (valid only for all mono + green out on COLOR MONITOR):
	//const uint8_t A_MONO_GREEN_video_levels[16] = { 255 , 185,  166, 21, 255 , 185,  166, 21, 255 , 185,  166, 21, 255 , 185,  166, 21};

	// FULL RANGE video levels for 100-B model, taken from page 46 of PDF
	const uint8_t video_levels[16] = { 255, 217,  201,186, 171, 156, 140, 125, 110, 97, 79, 66, 54, 31, 18, 0 };

	const int comm_rates[16] = { 50,75,110,134,150,200,300,600,1200,1800,2000,2400,3600,4800,9600,19200 };
};

class rainbow_modela_state : public rainbow_base_state
{
public:
	rainbow_modela_state(const machine_config &mconfig, device_type type, const char *tag) :
		rainbow_base_state(mconfig, type, tag)
	{
	}

	void rainbow_modela(machine_config &config);

private:
	virtual void machine_reset() override;

	void rainbow8088_map(address_map &map);
	void rainbow8088_io(address_map &map);

	void ext_ram_w(offs_t offset, uint8_t data);
	uint8_t rtc_r(offs_t offset);
	DECLARE_WRITE_LINE_MEMBER(irq_hi_w);
	uint8_t system_parameter_r();
};

class rainbow_modelb_state : public rainbow_base_state
{
public:
	rainbow_modelb_state(const machine_config &mconfig, device_type type, const char *tag) :
		rainbow_base_state(mconfig, type, tag)
	{
	}

	void rainbow_modelb(machine_config &config);

private:
	virtual void machine_reset() override;

	void rainbow8088_map(address_map &map);
	void rainbow8088_io(address_map &map);

	void ext_ram_w(offs_t offset, uint8_t data);
	uint8_t rtc_r(offs_t offset);
	DECLARE_WRITE_LINE_MEMBER(irq_hi_w);
	uint8_t system_parameter_r();
};

// It * should be * OK to RESET the SCROLL_BUFFER and the COLOR_MAP (at least with WELL WRITTEN programs)

// Situation less clear for vector mode (some programs work extensively * before * OPTION_GRFX_RESET

// THIS MACRO * RESETS *  the PATTERN TO DEFAULT.
// NOTE 2: m_patmult  MUST BE LOADED BEFORE !!
#define OPTION_RESET_PATTERNS \
	m_vpat = 0xff;                              \
	if (m_patmult == 0)  m_patmult = 0x01;      \
	if (m_patcnt == 0)   m_patcnt = m_patmult;  \
	if (m_patidx == 0)   m_patidx = 7;


// GDC RESET MACRO - used in  "machine_reset"  & GDC_EXTRA_REGISTER_w   !
#define OPTION_GRFX_RESET                                   \
	lower_8088_irq(IRQ_GRF_INTR_L);                         \
	m_monitor_suggested = m_inp13->read();                  \
	m_gdc_indirect_register = 0;                            \
	m_gdc_color_map_index = 0;                              \
	for (int i = 0; i < 256; i++)                           \
		m_gdc_scroll_buffer[i] = i;                         \
	m_gdc_scroll_index = 0;                                 \
	m_gdc_write_buffer_index = 0;                           \
	m_gdc_write_mask = 0x00;                                \
	m_gdc_alu_ps_register = 0x0F;                           \
	m_gdc_fg_bg = 0xF0;                                     \
	m_gdc_mode_register &= GDC_MODE_VECTOR | GDC_MODE_HIGHRES | GDC_MODE_ENABLE_WRITES | GDC_MODE_READONLY_SCROLL_MAP;\
	m_gdc_mode_register |= GDC_MODE_ENABLE_VIDEO;           \
	logerror("\n** OPTION GRFX. RESET **\n");

UPD7220_DISPLAY_PIXELS_MEMBER( rainbow_base_state::hgdc_display_pixels )
{
	if(m_inp7->read() == 0)
		return;

	const rgb_t *paletteX = m_palette2->palette()->entry_list_raw();

	uint16_t plane0, plane1, plane2, plane3;
	uint8_t pen;

	if (m_onboard_video_selected && (m_inp13->read() != DUAL_MONITOR))
	{
		for (int xi = 0; xi < 16; xi++) // blank screen when VT102 output active (..)
		{
			if (bitmap.cliprect().contains(x + xi, y))
				bitmap.pix(y, x + xi) = 0;
		}
		return; // no output from graphics option
	}

	// ********************* GET BITMAP DATA FOR 4 PLANES ***************************************
	// _READ_ BIT MAP  from 2 or 4 planes (plane 0 is least, plane 3 most significant). See page 42 / 43
	if (m_gdc_mode_register & GDC_MODE_HIGHRES)
	{
		address = ( m_gdc_scroll_buffer[ ((address & 0x7FC0) >> 7) & 0xff ] << 7) |  (address & 0x7F);
		plane0 = m_video_ram[((address & 0x3fff) + 0x0000)];
		plane1 = m_video_ram[((address & 0x3fff) + 0x8000)];
		plane2 = plane3 = 0;
	}
	else
	{
		address = ( m_gdc_scroll_buffer[ ((address & 0x3FC0) >> 7) & 0xff ] << 7) |  (address & 0x7F);
		// MED.RESOLUTION (4 planes, 4 color bits, 16 color map entries / 16 -or 4- MONOCHROME SHADES)
		plane0 = m_video_ram[((address & 0x1fff) + 0x00000)];
		plane1 = m_video_ram[((address & 0x1fff) + 0x08000)];
		plane2 = m_video_ram[((address & 0x1fff) + 0x10000)];
		plane3 = m_video_ram[((address & 0x1fff) + 0x18000)];
	}

	bool mono = (m_monitor_suggested == MONO_MONITOR) ? true : false; // 1 = MONO, 2 = COLOR, 3 = DUAL MONITOR; 4 = AUTO

	for (int xi = 0; xi < 16; xi++)
	{
		pen = BIT(plane0 ,xi) |
			 (BIT(plane1 ,xi) << 1) |
			 (BIT(plane2 ,xi) << 2) |
			 (BIT(plane3 ,xi) << 3);

		if (bitmap.cliprect().contains(x + xi, y))
			bitmap.pix(y, x + xi) = paletteX[mono ? (pen + 16) : pen];
	}
}

void rainbow_base_state::floppy_formats(format_registration &fr)
{
	// this order is important as there is a DS 40track 400KB pc format that is the same size as the SS 80track rx50 format
	fr.add(FLOPPY_RX50IMG_FORMAT);
	fr.add_pc_formats();
}

static void rainbow_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD); // QD means 80 tracks with DD data rate (single or double sided).
	device.option_add("525dd", FLOPPY_525_DD); // mimic a 5.25" PC (40 track) drive. Requires IDrive5.SYS.
	device.option_add("35dd", FLOPPY_35_DD); // mimic 3.5" PC drive (720K, double density). Use Impdrv3.SYS.
	device.option_add("525ssdd", FLOPPY_525_SSDD); // to read a single sided, (160K) PC-DOS 1 disk with MediaMaster
}

void rainbow_base_state::machine_start()
{
	m_power_good = false; // Simulate AC_OK signal from power supply.
	cmd_timer = timer_alloc(0);
	cmd_timer->adjust(attotime::from_msec(MS_TO_POWER_GOOD));

	switch_off_timer = timer_alloc(1);
	switch_off_timer->adjust(attotime::from_msec(10));

	m_digits.resolve();
	m_leds.resolve();
	m_driveleds.resolve();

	m_screen_blank = false;

	auto *printer_port = subdevice<rs232_port_device>("printer");
	printer_port->write_dtr(0);
	printer_port->write_rts(0);

	save_item(NAME(m_z80_private));
	save_item(NAME(m_z80_mailbox));
	save_item(NAME(m_8088_mailbox));
	save_item(NAME(m_zflip));
	save_item(NAME(m_printer_bitrate));
	save_item(NAME(m_kbd_tx_ready));
	save_item(NAME(m_kbd_rx_ready));
	save_item(NAME(m_irq_high));
	save_item(NAME(m_irq_mask));
}



void rainbow_base_state::rainbow8088_base_map(address_map &map)
{
	map.unmap_value_high();

	// There is a 2212 (256 x 4 bit) NVRAM from 0xed000 to 0xed0ff (*)
	// shadowed at $ec000 - $ecfff and from $ed100 - $edfff.

	// (*) ED000 - ED0FF is the area the DEC-100-B Bios accesses and checks

	//  - Specs say that the CPU has direct access to volatile RAM only.
	//    So NVRAM is hidden and loads & saves are triggered within the
	//    'diagnostic_w' handler (similar to real hardware).

	//  - Address bits 8-12 are ignored (-> mirror()).
	map(0xed000, 0xed0ff).ram().share("vol_ram"); //.mirror(0x1f00);
	map(0xed100, 0xed1ff).ram().share("nvram");

	map(0xee000, 0xeffff).ram().share("p_ram");
	map(0xf0000, 0xfffff).rom();
}

void rainbow_modela_state::rainbow8088_map(address_map &map)
{
	rainbow8088_base_map(map);
	map(0x00000, 0xcffff).rw(FUNC(rainbow_modela_state::ext_ram_r), FUNC(rainbow_modela_state::ext_ram_w));

#ifdef RTC_ENABLED
	// *********************************** / DS1315 'PHANTOM CLOCK' IMPLEMENTATION FOR 'DEC-100-A' ***************************************
	map(0xed000, 0xed000).r(FUNC(rainbow_modela_state::rtc_r));
	map(0xed0fe, 0xed0ff).w(FUNC(rainbow_modela_state::rtc_w));
	// *********************************** / DS1315 'PHANTOM CLOCK' IMPLEMENTATION FOR 'DEC-100-A' ***************************************
#endif
}

// DEC-100-B probes until a 'flaky' area is found (BOOT ROM around F400:0E04).
// It is no longer possible to key in the RAM size from within the 100-B BIOS.
void rainbow_modelb_state::rainbow8088_map(address_map &map)
{
	rainbow8088_base_map(map);
	map(0x00000, 0xdffff).rw(FUNC(rainbow_modelb_state::ext_ram_r), FUNC(rainbow_modelb_state::ext_ram_w));

#ifdef RTC_ENABLED
	// *********************************** / DS1315 'PHANTOM CLOCK' IMPLEMENTATION FOR 'DEC-100-B' ***************************************
	// No address space needed ( -> IRQs must be disabled to block ROM accesses during reads ).
	map(0xfc000, 0xfe104).r(FUNC(rainbow_modelb_state::rtc_r));
	// *********************************** / DS1315 'PHANTOM CLOCK' IMPLEMENTATION FOR 'DEC-100-B' ***************************************
#endif
}

void rainbow_base_state::rainbow8088_base_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x1ff);
	map(0x00, 0x00).rw(FUNC(rainbow_base_state::i8088_latch_r), FUNC(rainbow_base_state::i8088_latch_w));
	map(0x02, 0x02).rw(FUNC(rainbow_base_state::comm_control_r), FUNC(rainbow_base_state::comm_control_w)); // Communication status / control register (8088)
	map(0x04, 0x04).w(m_crtc, FUNC(rainbow_video_device::dc011_w));

	map(0x06, 0x06).w(FUNC(rainbow_base_state::comm_bitrate_w));

	map(0x0a, 0x0a).rw(FUNC(rainbow_base_state::diagnostic_r), FUNC(rainbow_base_state::diagnostic_w));
	map(0x0c, 0x0c).select(0x100).w(m_crtc, FUNC(rainbow_video_device::dc012_w));

	map(0x0e, 0x0e).w(FUNC(rainbow_base_state::printer_bitrate_w));

	map(0x10, 0x11).rw(m_kbd8251, FUNC(i8251_device::read), FUNC(i8251_device::write));

	// ===========================================================
	// There are 4 select lines for Option Select 1 to 4
	// Option Select ------------------- Bundle Option Present
	// 1 2 3 4:                          BDL PRES (L):
	// X X o o Communication Option----- X
	// o X o o RD51 hard disk controller X --------- (X = SELECT)
	// ===========================================================
	// 0x20 -> 0x2f ***** EXTENDED COMM. OPTION / Option Select 1.
	// See boot rom @1EA6: 0x27 (<- RESET EXTENDED COMM OPTION  )

	// Corvus B/H harddisk controller (incompatible with EXT.COMM OPTION):
	map(0x20, 0x20).rw(m_corvus_hdc, FUNC(corvus_hdc_device::read), FUNC(corvus_hdc_device::write));
	map(0x21, 0x21).r(FUNC(rainbow_base_state::corvus_status_r));

	// ===========================================================
	// 0x30 -> 0x3f ***** Option Select 3
	// ===========================================================
	// 0x40  COMMUNICATIONS DATA REGISTER (MPSC)
	// 0x41  PRINTER DATA REGISTER (MPSC)
	// 0x42  COMMUNICATIONS CONTROL / STATUS REGISTER (MPSC)
	// 0x43  PRINTER CONTROL / STATUS REGISTER (MPSC)
	// ===========================================================
	// 0x50 - 0x57 ***** COLOR GRAPHICS OPTION:

	// * Color graphics option (NEC upd7220 GDC plus external hw.). See Programmer's Reference AA-AE36A-TV.
	// Either 384 x 240 x 16 or 800 x 240 x 4 colors (out of 4096). 8 x 64 K video RAM.
	// (Write Buffer, Pattern Register/Multiplier, ALU/PS, Color Map, readback and offset/scroll hardware):
	map(0x50, 0x57).rw(FUNC(rainbow_base_state::GDC_EXTRA_REGISTER_r), FUNC(rainbow_base_state::GDC_EXTRA_REGISTER_w));

	// ===========================================================
	// 0x60 -> 0x6f ***** EXTENDED COMM. OPTION / Option Select 2.
	// ===========================================================
	// 0x60 -> 0x6f ***** RD51 HD. CONTROLLER   / Option Select 2.
	map(0x60, 0x67).rw(m_hdc, FUNC(wd2010_device::read), FUNC(wd2010_device::write)).mirror(0x100);
	map(0x68, 0x68).rw(FUNC(rainbow_base_state::hd_status_68_r), FUNC(rainbow_base_state::hd_status_68_w));
	map(0x69, 0x69).r(FUNC(rainbow_base_state::hd_status_69_r));
	// ===========================================================
	// THE RD51 CONTROLLER: WD1010AL - 00 (WDC '83)
	// + 2 K x 8 SRAM (SY2128-4 or Japan 8328) 21-17872-01
	// + 74(L)Sxxx glue logic (drive/head select, buffers etc.)
	// + 10 Mhz Quartz (/2)
	// SERVICE JUMPERS (not to be removed for normal operation):
	//   JUMPER "W1" : bridge between 10 Mhz master clock and board
	//   JUMPER "W2" : bridges SYNC within Read Data Circuit
	//   JUMPER "W3" : bridges 'drive read data' (from hard disk)
	// Later RD51 boards (> '83 week 28 ?) have no jumpers at all.
	// ===========================================================
	// DEC RD TYPE (MByte) CYL ---- HEADS ---- MODEL (typical)
	// DEC RD50 (5 Mbyte): 153 cyl. 4 heads -- ST506
	// DEC RD51(10 Mbyte); 306 cyl. 4 heads -- ST412
	// DEC RD31(20 Mbyte); 615 cyl. 4 heads -- ST225
	// DEC RD52(32 Mbyte); 512 cyl. 8 heads -- Q540  [!]
	// DEC RD32(40 Mbyte); 820 cyl. 6 heads -- ST251 [!]
	// DEC RD53(67 Mbyte); 1024 cyl.8 heads -- 1325  [!]
	// [!] More than 4 heads. Prepare with WUTIL and / or DSKPREP.

	// SIZE RESTRICTIONS
	// * HARDWARE:
	//      WD1010 controller has a built-in limit of 8 heads / 1024 cylinders.
	// * BOOT LOADERS:
	//   - the DEC boot loader (and FDISK from DOS 3.10) initially allowed a maximum hard disc size of 20 MB.
	//   - the custom boot loader that comes with 'WUTIL 3.2' allows 117 MB and 8 surfaces.
	// * SOFTWARE:
	//   - MS-DOS 2 allows a maximum partition size of 16 MB (sizes > 15 MB are incompatible to DOS 3)
	//     [ no more than 4 partitions of 8 MB size on one hard disk possible ]
	//   - MS-DOS 3 - and Concurrent CPM - have a global 32 MB (1024 cylinder) limit
	//   - a CP/M-86-80 partition can have up to 8 MB (all CP/M partitions together must not exceed 10 MB)
	// ===========================================================
	// 0x70 -> 0x7f ***** Option Select 4
	// ===========================================================
	// 0x10c -> (MHFU disable register handled by 0x0c + select())
}

void rainbow_modela_state::rainbow8088_io(address_map &map)
{
	rainbow_base_state::rainbow8088_base_io(map);
	map(0x08, 0x08).r(FUNC(rainbow_modela_state::system_parameter_r));
}

void rainbow_modelb_state::rainbow8088_io(address_map &map)
{
	rainbow_base_state::rainbow8088_base_io(map);
	map(0x08, 0x08).r(FUNC(rainbow_modelb_state::system_parameter_r));
}

void rainbow_base_state::rainbowz80_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(rainbow_base_state::share_z80_r), FUNC(rainbow_base_state::share_z80_w));
}

void rainbow_base_state::rainbowz80_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).rw(FUNC(rainbow_base_state::z80_latch_r), FUNC(rainbow_base_state::z80_latch_w));
	map(0x20, 0x20).rw(FUNC(rainbow_base_state::z80_generalstat_r), FUNC(rainbow_base_state::z80_diskdiag_read_w)); // read to port 0x20 used by MS-DOS 2.x diskette loader.
	map(0x21, 0x21).rw(FUNC(rainbow_base_state::z80_generalstat_r), FUNC(rainbow_base_state::z80_diskdiag_write_w));
	map(0x40, 0x40).rw(FUNC(rainbow_base_state::z80_diskstatus_r), FUNC(rainbow_base_state::z80_diskcontrol_w));
	map(0x60, 0x63).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));

	// Z80 I/O shadow area > $80
	map(0x80, 0x80).rw(FUNC(rainbow_base_state::z80_latch_r), FUNC(rainbow_base_state::z80_latch_w));
	map(0xA0, 0xA0).rw(FUNC(rainbow_base_state::z80_generalstat_r), FUNC(rainbow_base_state::z80_diskdiag_read_w)); // read to port 0x20 used by MS-DOS 2.x diskette loader.
	map(0xA1, 0xA1).rw(FUNC(rainbow_base_state::z80_generalstat_r), FUNC(rainbow_base_state::z80_diskdiag_write_w));
	map(0xC0, 0xC0).rw(FUNC(rainbow_base_state::z80_diskstatus_r), FUNC(rainbow_base_state::z80_diskcontrol_w));
	map(0xE0, 0xE3).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
}

/* Input ports */

/* DIP switches */
static INPUT_PORTS_START(rainbow100b_in)

	PORT_START("MONO_MONITOR_TYPE")
	PORT_DIPNAME(0x03, 0x03, "MONO MONITOR TYPE")
	PORT_DIPSETTING(0x01, "WHITE (VR201-A)")
	PORT_DIPSETTING(0x02, "GREEN (VR201-B)")
	PORT_DIPSETTING(0x03, "AMBER (VR201-C)")

	// FLOPPY, BUNDLE, GRAPHICS affect 'system_parameter_r':

	// EXT.COMM.card -or- RD51 HD. controller (marketed later).
	PORT_START("DEC_HARD_DISK") // BUNDLE_OPTION
	PORT_DIPNAME(0x01, 0x00, "DEC HARD DISK (#1)") PORT_TOGGLE
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))

	PORT_START("CORVUS_HARD_DISKS")
	PORT_DIPNAME(0x01, 0x00, "CORVUS HARD DISKS (#2 to #5)") PORT_TOGGLE
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))

	PORT_START("CLIKCLOK") // DS1315 RTC
	PORT_DIPNAME(0x01, 0x00, "REAL TIME CLOCK (CLIKCLOK)") PORT_TOGGLE
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))

	PORT_START("GRAPHICS_OPTION") // GDC
	PORT_DIPNAME(0x01, 0x00, "GRAPHICS OPTION") PORT_TOGGLE
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))

	// W13 - W18 are used for factory tests and affect the boot process -
	PORT_START("W13")
	PORT_DIPNAME(0x02, 0x02, "W13 (FACTORY TEST A, LEAVE OFF)") PORT_TOGGLE
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_START("W14")
	PORT_DIPNAME(0x04, 0x04, "W14 (FACTORY TEST B, LEAVE OFF)") PORT_TOGGLE
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_START("W15")
	PORT_DIPNAME(0x08, 0x08, "W15 (FACTORY TEST C, LEAVE OFF)") PORT_TOGGLE
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_START("W18") // DSR = 1 when switch is OFF - see i8251.c
	PORT_DIPNAME(0x01, 0x00, "W18 (FACTORY TEST D, LEAVE OFF) (8251A: DSR)") PORT_TOGGLE
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))
	PORT_WRITE_LINE_DEVICE_MEMBER("kbdser", i8251_device, write_dsr)

	// J17 jumper on FDC controller board shifts drive select (experimental) -
	PORT_START("J17")
	PORT_DIPNAME(0x02, 0x00, "J17 DRIVE SELECT (A => C and B => D)") PORT_TOGGLE
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x02, DEF_STR(On))

	PORT_START("WATCHDOG")
	PORT_DIPNAME(0x01, 0x00, "WATCHDOG ENABLED (MHFU)") PORT_TOGGLE
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))

	PORT_START("MONITOR_CONFIGURATION") // GDC
	PORT_DIPNAME(0x0F, 0x04, "MONITOR CONFIGURATION")
	PORT_DIPSETTING(0x04, "AUTODETECT")
	PORT_DIPSETTING(0x01, "MONO ONLY / 4 to 16 monochrome shades (single VR-201)")
	PORT_DIPSETTING(0x02, "COLOR ONLY (single VR-241 with BCC-17 cable)")
	PORT_DIPSETTING(0x03, "DUAL MONITOR (SCREEN 1: TEXT;  SCREEN 2: R-G-B)")
INPUT_PORTS_END

void rainbow_base_state::machine_reset()
{
	// 'F3' (in partial emulation) here replaces 'CTRL-SETUP' (soft reboot on an original Rainbow)
	// FIXME: BIOS reports error 19 when CTRL-SETUP is pressed (Z80 or flags aren't fully reset then?)
	popmessage("Reset");

	m_crtc->MHFU(MHFU_RESET_and_DISABLE);

	m_rtc->chip_reset();     // * Reset RTC to a defined state *

	//  *********** HARD DISK CONTROLLERS...
	address_space &io = m_i8088->space(AS_IO);
	if (m_inp5->read() == 0x01) // ...PRESENT?
	{
		// Install 8088 read / write handler
		io.unmap_readwrite(0x60, 0x60);
		io.install_read_handler(0x60, 0x60, read8smo_delegate(*this, FUNC(rainbow_base_state::hd_status_60_r)));
		io.install_write_handler(0x60, 0x60, write8smo_delegate(*this, FUNC(rainbow_base_state::hd_status_60_w)));

		hdc_reset();
		m_hdc_drive_ready = true;
		m_hdc_write_fault = false;

		hard_disk_file *local_hard_disk;
		local_hard_disk = rainbow_hdc_file(0); // one hard disk for now.

		m_leds[0] = 0;
		switch_off_timer->adjust(attotime::from_msec(500));

		if (local_hard_disk)
		{
			hard_disk_info *info;
			if ((info = hard_disk_get_info(local_hard_disk)))
			{
				m_leds[0] = 1;

				uint32_t max_sector = (info->cylinders) * (info->heads) * (info->sectors);
				popmessage("DEC %u (%3.2f) MB HARD DISK MOUNTED.\nGEOMETRY: %d HEADS (1..%d ARE OK).\n%d CYLINDERS (151 to %d ARE OK).\n%d SECTORS / TRACK (up to %d ARE OK). \n%d BYTES / SECTOR (128 to 1024 ARE OK).\n",
					max_sector * info->sectorbytes / 1000000,
					(float)max_sector * (float)info->sectorbytes / 1048576.0f,
					info->heads, RD51_MAX_HEAD,
					info->cylinders, RD51_MAX_CYLINDER,
					info->sectors, RD51_SECTORS_PER_TRACK,
					info->sectorbytes);
			}
		}
	}

	if (m_inp6->read() == 0x00) // Unmap port if Corvus not present
			io.unmap_readwrite(0x20, 0x20);

	// *********** FLOPPY DISK CONTROLLER [ NOT OPTIONAL ]
	m_present_drive = INVALID_DRIVE;
	m_fdc->reset();
	m_fdc->set_floppy(nullptr);
	m_fdc->dden_w(0);

	// *********** NEC 7220 DISPLAY CONTROLLER [ OPTIONAL ]
	OPTION_GRFX_RESET
	OPTION_RESET_PATTERNS

	for (int i = 0; i < 32; i++)
		m_gdc_color_map[i] = 0x00;
	m_gdc_color_map_index = 0;
	// *********** Z80

	m_z80->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_z80_halted = true;

	m_zflip = true; // ZRESET high on startup
	m_diagnostic = 0;   // DIAGNOSTIC_R/W registers (shouldn't it be 1?)

	m_intz80 = false;
	m_int88 = false;

	// *********** SERIAL COMM. (7201)
	m_mpsc->reset();
	m_mpsc_irq = 0;
	m_printer_bitrate = 0;

	// *********** KEYBOARD + IRQ
	m_kbd_tx_ready = m_kbd_rx_ready = false;
	m_kbd8251->write_cts(0);
	m_KBD = 0;

	m_irq_high = 0;
	m_irq_mask = 0;

	// RESET RED LEDs
	m_leds[0] = 1;
	m_leds[1] = 1;
	m_leds[2] = 1;
	m_leds[3] = 1;
	m_leds[4] = 1;
	m_leds[5] = 1;
	m_leds[6] = 1;

	// GREEN KEYBOARD LEDs (see machine/dec_lk201.cpp)
}

void rainbow_modela_state::machine_reset()
{
	rainbow_base_state::machine_reset();
	logerror("*** RAINBOW A MODEL ASSUMED (64 - 832 K RAM).\n");
}

void rainbow_modelb_state::machine_reset()
{
	rainbow_base_state::machine_reset();
	logerror("*** RAINBOW B MODEL ASSUMED (128 - 896 K RAM)\n");
}

// Simulate AC_OK signal (power good) and RESET after ~ 108 ms.
void rainbow_base_state::device_timer(emu_timer &timer, device_timer_id tid, int param)
{
	switch (tid)
	{
	case 0:
		cmd_timer->adjust(attotime::never);

		if (m_power_good == false)
		{
			m_power_good = true;
			logerror("**** POWER GOOD ****\n");
		}
		else
		{
			logerror("**** WATCHDOG: CPU RESET ****\n");
			m_i8088->reset(); // gives 'ERROR_16 - INTERRUPTS OFF' (indicates hardware failure or software bug).
		}
		break; // case 0

	case 1:

		switch_off_timer->adjust(attotime::never);

		m_driveleds[0] = 0; // DRIVE 0 (A)
		m_driveleds[1] = 0; // DRIVE 1 (B)
		m_driveleds[2] = 0; // DRIVE 2 (C)
		m_driveleds[3] = 0; // DRIVE 3 (D)

		m_leds[0] = 1;  // 1 = OFF (One of the CPU LEDs as drive LED for DEC hard disk)
		m_leds[1] = 1;  // 1 = OFF (One of the CPU LEDs as drive LED for Corvus HD)

		break; // case 1

	} // switch (timer ID)
}

uint32_t rainbow_base_state::screen_update_rainbow(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static int old_monitor;

	if ((m_monitor_suggested < 1) || (m_monitor_suggested > 3))
			m_monitor_suggested = COLOR_MONITOR;

	if (m_monitor_suggested != old_monitor)
	{
		old_monitor = m_monitor_suggested;
		if (m_monitor_suggested == 1)
			popmessage("MONOCHROME MONITOR");
		if (m_monitor_suggested == 2)
			popmessage("COLOR MONITOR");
		if (m_monitor_suggested == 3)
			popmessage("DUAL MONITOR");
	}

	int palette_selected;
	if (m_onboard_video_selected && (m_monitor_suggested == COLOR_MONITOR))
		 palette_selected = 2; // Color monitor; green text
	else
		 palette_selected = m_inp9->read();

	m_crtc->palette_select(palette_selected);
	m_crtc->video_update(bitmap, cliprect);

	if (m_screen_blank || ((!m_onboard_video_selected) && (m_inp13->read() != DUAL_MONITOR))) // dual monitor: never blank all
		m_crtc->video_blanking(bitmap, cliprect);
	else
		m_crtc->video_update(bitmap, cliprect);
	return 0;
}

// Interrupt handling and arbitration.  See 3.1.3.8 OF PC-100 spec.
void rainbow_base_state::update_8088_irqs()
{
	if (m_irq_mask != 0)
	{
		for (int i = IRQ_8088_VBL; i >= 0; i--)
		{
			if (m_irq_mask & (1 << i))
			{
				m_i8088->set_input_line_and_vector(INPUT_LINE_INT0, ASSERT_LINE, vectors[i] | m_irq_high); // I8088
				break;
			}
		}
	}
	else
	{
		m_i8088->set_input_line(INPUT_LINE_INT0, CLEAR_LINE);
	}
}


void rainbow_base_state::raise_8088_irq(int ref)
{
	m_irq_mask |= (1 << ref);
	update_8088_irqs();
}

void rainbow_base_state::lower_8088_irq(int ref)
{
	m_irq_mask &= ~(1 << ref);
	update_8088_irqs();
}


// IRQ service for 7201 (commm / printer)
void rainbow_base_state::update_mpsc_irq()
{
	if (m_mpsc_irq == 0)
		lower_8088_irq(IRQ_COMM_PTR_INTR_L);
	else
		raise_8088_irq(IRQ_COMM_PTR_INTR_L);
}

WRITE_LINE_MEMBER(rainbow_base_state::mpsc_irq)
{
	m_mpsc_irq = state;
	update_mpsc_irq();
}

// PORT 0x06 : Communication bit rates (see page 21 of PC 100 SPEC)
void rainbow_base_state::comm_bitrate_w(uint8_t data)
{
	m_dbrg->str_w(data & 0x0f);  // PDF is wrong, low nibble is RECEIVE clock (verified in SETUP).
	logerror("\n(COMM.) receive bitrate = %d ($%02x)\n", comm_rates[data & 0x0f] , data & 0x0f);

	m_dbrg->stt_w( ((data & 0xf0) >> 4) );
	logerror("(COMM.) transmit bitrate = %d ($%02x)\n", comm_rates[((data & 0xf0) >> 4)] ,(data & 0xf0) >> 4);
}

// PORT 0x0e : Printer bit rates
void rainbow_base_state::printer_bitrate_w(uint8_t data)
{
	m_printer_bitrate = data & 7;
	// bits 0 - 2 = 0: nominally 75 bps, actually 75.35 bps
	// bits 0 - 2 = 1: nominally 150 bps, actually 150.7 bps
	// bits 0 - 2 = 2: nominally 300 bps, actually 301.4 bps
	// bits 0 - 2 = 3: nominally 600 bps, actually 602.8 bps
	// bits 0 - 2 = 4: nominally 1200 bps, actually 1205.6 bps
	// bits 0 - 2 = 5: nominally 2400 bps, actually 2411.2 bps
	// bits 0 - 2 = 6: nominally 4800 bps, actually 4822.4 bps (keyboard is tied to this rate)
	// bits 0 - 2 = 7: nominally 9600 bps, actually 9644.8 bps
	// TX and RX rate cannot be programmed independently.
	logerror("\n(PRINTER) receive = transmit bitrate: %d ($%02x)", 9600 / ( 1 << (7 - (data & 7))) , data & 7);

	// "bit 3 controls the communications port clock (RxC,TxC). External clock when 1, internal when 0"
	logerror(" - CLOCK (0 = internal): %02x", data & 8);
}

WRITE_LINE_MEMBER(rainbow_base_state::dbrg_fr_w)
{
	m_mpsc->rxca_w(state);
}

WRITE_LINE_MEMBER(rainbow_base_state::dbrg_ft_w)
{
	m_mpsc->txca_w(state);
}

void rainbow_base_state::bitrate_counter_w(uint8_t data)
{
	bool prt_rxtxc = BIT(data, 7 - m_printer_bitrate);
	bool kbd_rxtxc = BIT(data, 1);

	m_mpsc->rxcb_w(prt_rxtxc);
	m_mpsc->txcb_w(prt_rxtxc);

	m_kbd8251->write_rxc(kbd_rxtxc);
	m_kbd8251->write_txc(kbd_rxtxc);
}

// Only Z80 * private SRAM * is wait state free
// (= fast enough to allow proper I/O to the floppy)

// Shared memory is contended by refresh, concurrent
//    8088 accesses and arbitration logic (DMA).
uint8_t rainbow_base_state::share_z80_r(offs_t offset)
{
	if (m_zflip)
	{
		if (offset < 0x8000)
		{
			return m_ram->read(offset + 0x8000);
		}
		else if (offset < 0x8800)
		{
			return m_z80_private[offset & 0x7ff]; // SRAM
		}

		return m_ram->read(offset ^ 0x8000);
	}
	else
	{
		if (offset < 0x800)
		{
			return m_z80_private[offset]; // SRAM
		}

		return m_ram->read(offset);
	}
}

void rainbow_base_state::share_z80_w(offs_t offset, uint8_t data)
{
	if (m_zflip)
	{
		if (offset < 0x8000)
		{
			m_ram->write(offset + 0x8000, data);
			return; // [!]
		}
		else if (offset < 0x8800)
		{
			m_z80_private[offset & 0x7ff] = data; // SRAM
			return; // [!]
		}

		m_ram->write(offset ^ 0x8000, data);
	}
	else
	{
		if (offset < 0x800)
			m_z80_private[offset] = data; // SRAM
		else
			m_ram->write(offset, data);
	}
	return;
}

uint8_t rainbow_base_state::ext_ram_r(offs_t offset)
{
	if (offset < m_ram->size())
		return m_ram->read(offset);
	return 0;
}

void rainbow_modela_state::ext_ram_w(offs_t offset, uint8_t data)
{
	if (offset < m_ram->size())
		m_ram->write(offset, data);
}

// NMI logic (parity test)
void rainbow_modelb_state::ext_ram_w(offs_t offset, uint8_t data)
{
	if (offset < m_ram->size())
		m_ram->write(offset, data);

	if (m_diagnostic & 0x08)
		if (offset >= 0x10000)
			m_i8088->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

// ------------------------ClikClok (for 100-A; DS1315) ------------------------------------------
// Version for 100-A plugs into NVRAM chip socket. There is a socket on the ClikClok for the NVRAM

// Requires a short program from the Suitable Solutions ClikClok distribution disk (CLIKA.COM)
// - also needed to set time/date (*).                   Reads $ed000, writes ed0fe/ed0ff.
void rainbow_base_state::rtc_w(offs_t offset, uint8_t data)
{
	if (m_inp11->read() == 0x01) // if enabled...
	{
		switch (offset)
		{
		case 0x00: // Write to 0xED0FE
			if (m_rtc->chip_enable())
				m_rtc->write_data(offset & 0x01); // Transfer data to DS1315 (data = offset):
			else
				m_rtc->read_0(); // (RTC ACTIVATION) read magic pattern 0
			break;

		case 0x01: // Write to 0xED0FF
			if (m_rtc->chip_enable())
				m_rtc->write_data(offset & 0x01); // Transfer data to DS1315 (data = offset):
			else
				m_rtc->read_1(); // (RTC ACTIVATION) read magic pattern 1
			break;
		}
	}
	m_p_vol_ram[offset] = data;  // Poke value into VOL_RAM.
}

// ------------------------ClikClok (for 100-B; DS1315)  ------------------------------------------------
// Add-on hardware, occupies one of the EPROM sockets of the 100-B. TODO: check address decoders on board
// Requires CLIKCLOK.COM or RBCLIK21.COM (freeware from Latrobe).                       Uses FC000/FE000.
uint8_t rainbow_modela_state::rtc_r(offs_t offset)
{
	if (m_inp11->read() == 0x01) // if enabled...
	{
		if (offset == 0x00) // read time/date from 0xED000 (ClikClok for 100-A)
		{
			if (m_rtc->chip_enable())
				return m_rtc->read_data() & 0x01;
			 else
				m_rtc->chip_reset();
		}
	}

	return m_p_vol_ram[offset];  // return volatile RAM
}

uint8_t rainbow_modelb_state::rtc_r(offs_t offset)
{
	if (m_inp11->read() == 0x01) // if enabled...
	{
		switch (offset)
		{
		// Transfer data to DS1315 (data = offset):
		case 0x0000:  // RTC_WRITE_DATA_0 0xFC000
		case 0x2000:  // RTC_WRITE_DATA_0 0xFE000 (MIRROR)

		case 0x0001:  // RTC_WRITE_DATA_1 0xFC001
		case 0x2001:  // RTC_WRITE_DATA_1 0xFE001 (MIRROR)
			m_rtc->write_data(offset & 0x01);
			break;

		// Read actual time/date from ClikClok:
		case 0x0004:  // 0xFC004
		case 0x2004:  // 0xFE004 (MIRROR)
			if (m_rtc->chip_enable())
				return (m_rtc->read_data() & 0x01);
			[[fallthrough]]; // FIXME: really?
		// (RTC ACTIVATION) read magic pattern 0
		case 0x0100:  // 0xFC100
		case 0x2100:  // 0xFE100 (MIRROR)
			m_rtc->read_0();
			break;

		// (RTC ACTIVATION) read magic pattern 1
		case 0x0101:  // 0xFC101
		case 0x2101:  // 0xFE101 (MIRROR)
			m_rtc->read_1();
			break;

		// RESET
		case 0x0104:  // 0xFC104
		case 0x2104:  // 0xFE104 (MIRROR)
			m_rtc->chip_reset();
			break;
		}
	}

	uint8_t *rom = memregion("maincpu")->base();
	return rom[0xfc000 + offset];  // return ROM
}
// ------------------------/ ClikClok (for model B; DS1315)  -------------------------------


// --------------------------------- Corvus (B/H)  -----------------------------------------
// PORT 0x21 : Corvus status register (ready / direction)
uint8_t rainbow_base_state::corvus_status_r()
{
	if (m_inp6->read() == 0) // Corvus controller
	{
		popmessage("Corvus controller invoked - but switched OFF.\nCheck DIP and perform a reset.\n\nIncompatible software also triggers this warning (illegal access to port $21)");
		return 0;
	}
	else
	{
		m_leds[1] = 0;
		switch_off_timer->adjust(attotime::from_msec(500));

		uint8_t status = m_corvus_hdc->status_r();
		uint8_t data = BIT(status, 7); // 0x80 BUSY (Set = Busy, Clear = Ready)
		data |= BIT(status, 6) << 1; // 0x40 DIR. (Controller -> Host, or Host->Controller)
		return data;
	}
}
// ---------------------------------/ Corvus (B/H)  ----------------------------------------


// ---------------------------- RD51 HARD DISK CONTROLLER ----------------------------------
static const int SECTOR_SIZES[4] = { 256, 512, 1024, 128 };

void rainbow_base_state::hdc_reset()
{
//  logerror(">> HARD DISC CONTROLLER RESET <<\n");
	m_hdc->reset();

	m_bdl_irq = 0;
	update_bundle_irq(); // reset INTRQ

	m_hdc_buf_offset = 0;
	m_hdc_direction = 0;

	m_hdc->buffer_ready(false);
	m_hdc_write_gate = false;

	m_hdc_step_latch = false;
	m_hdc_index_latch = false;
}

// Return 'hard_disk_file' object for harddisk 1 (fixed).
// < nullptr if geometry is insane or other errors occured >
hard_disk_file *rainbow_base_state::rainbow_hdc_file(int drv)
{
	m_hdc_drive_ready = false;

	if (m_inp5->read() != 0x01) // ...PRESENT?
		return nullptr;

	if (drv != 0)
		return nullptr;

	harddisk_image_device *img = dynamic_cast<harddisk_image_device *>(subdevice("decharddisk1"));

	if (!img)
		return nullptr;

	if (!img->exists())
		return nullptr;

	hard_disk_file *file = img->get_hard_disk_file();
	hard_disk_info *info = hard_disk_get_info(file);

	// MFM ALLOWS UP TO 17 SECTORS / TRACK.
	// CYLINDERS: 151 (~ 5 MB) to 1024 (max. cylinders on WD1010 controller)
	if (((info->sectors <= RD51_SECTORS_PER_TRACK)) &&
		((info->heads >= 1) && (info->heads <= RD51_MAX_HEAD)) &&            // HEADS WITHIN 1...8
		((info->cylinders > 150) && (info->cylinders <= RD51_MAX_CYLINDER)))
	{
		m_hdc_drive_ready = true;
		return file;  // HAS SANE GEOMETRY
	}
	else
	{
		uint32_t max_sector = info->cylinders * info->heads * info->sectors;
		popmessage("DEC %u (%3.2f) MB HARD DISK REJECTED.\nGEOMETRY: %d HEADS (1..%d ARE OK).\n%d CYLINDERS (151 to %d ARE OK).\n%d SECTORS / TRACK (up to %d ARE OK). \n%d BYTES / SECTOR (128 to 1024 ARE OK).\n",
					max_sector * info->sectorbytes / 1000000,
					(float)max_sector * (float)info->sectorbytes / 1048576.0f,
					info->heads, RD51_MAX_HEAD,
					info->cylinders, RD51_MAX_CYLINDER,
					info->sectors, RD51_SECTORS_PER_TRACK,
					info->sectorbytes);
		logerror("<<< === HARD DISK IMAGE REJECTED = (invalid geometry) === >>>\n");
		return nullptr;
	}
}

// LBA sector from CHS
static uint32_t get_and_print_lbasector(device_t *device, hard_disk_info *info, uint16_t cylinder, uint8_t head, uint8_t sector_number)
{
	if (info == nullptr)
		return 0;

	// LBA_ADDRESS = (C * HEADS + H) * NUMBER_SECTORS + (S - 1)
	uint32_t lbasector = (double)cylinder * info->heads; // LBA : ( x 4 )
	lbasector += head;
	lbasector *= info->sectors;   // LBA : ( x 16 )
	lbasector += (sector_number - 1); // + (sector number - 1)

//  device->logerror(" CYLINDER %u - HEAD %u - SECTOR NUMBER %u (LBA-SECTOR %u) ", cylinder, head, sector_number, lbasector);
	return lbasector;
}

// READ SECTOR (on BCS 1 -> 0 transition)
WRITE_LINE_MEMBER(rainbow_base_state::hdc_read_sector)
{
	static int last_state;
	int read_status = 1;

	if (!m_hdc_write_gate) // do not read when WRITE GATE is on
	{
		uint8_t sdh = (m_hdc->read(0x06));
		int drv = (sdh & (8 + 16)) >> 3; // get DRIVE from SDH register

		if ((state == 0) && (last_state == 1) && (drv == 0))
		{
			read_status = 2; //          logerror("\nTRYING TO READ");
			m_leds[0] = 0;
			switch_off_timer->adjust(attotime::from_msec(500));

			int hi = (m_hdc->read(0x05)) & 0x07;
			uint16_t cylinder = (m_hdc->read(0x04)) | (hi << 8);
			uint8_t sector_number = m_hdc->read(0x03);

			hard_disk_file *local_hard_disk;
			local_hard_disk = rainbow_hdc_file(0); // one hard disk for now.

			if (local_hard_disk)
			{
				read_status = 3;

				hard_disk_info *info;
				if ((info = hard_disk_get_info(local_hard_disk)))
				{
					read_status = 4;
					m_leds[0] = 1;

					// Pointer to info + C + H + S
					uint32_t lbasector = get_and_print_lbasector(this, info, cylinder, sdh & 0x07, sector_number);

					if ((cylinder <= info->cylinders) &&                          // filter invalid ranges
						(SECTOR_SIZES[(sdh >> 5) & 0x03] == info->sectorbytes)    // may not vary in image!
						)
					{
						read_status = 5;
						if (hard_disk_read(local_hard_disk, lbasector, m_hdc_buffer)) // accepts LBA sector (uint32_t) !
							read_status = 0; //  logerror("...success!\n");
					}
				}
				m_hdc_buf_offset = 0;
				m_hdc->buffer_ready(true);
			} // if valid  (..)

			if (read_status != 0)
			{
				logerror("...** READ FAILED WITH STATUS %u ** (CYLINDER %u - HEAD %u - SECTOR # %u - SECTOR_SIZE %u ) ***\n",
					read_status, cylinder, sdh & 0x07, sector_number, SECTOR_SIZES[(sdh >> 5) & 0x03]
				);
			}

		} //   (on BCS 1 -> 0)

	} // do not read when WRITE GATE is on

	last_state = state;
}


// WRITE SECTOR
// ...IF WRITE_GATE (WG) TRANSITS FROM 1 -> 0

// NO PROVISIONS for  sector sizes != 512 or MULTIPLE DRIVES (> 0) !!!
WRITE_LINE_MEMBER(rainbow_base_state::hdc_write_sector)
{
	int success = 0;
	static int wg_last;

	if (state == 0)
		m_hdc_write_gate = false;
	else
		m_hdc_write_gate = true;

	int drv = ((m_hdc->read(0x06)) & (8 + 16)) >> 3; // get DRIVE from SDH register

	if (state == 0 && wg_last == 1 && drv == 0)  // Check correct state transition and DRIVE 0 ....
	{
		m_leds[0] = 0;  // (1 = OFF ) =HARD DISK ACTIVITY =
		switch_off_timer->adjust(attotime::from_msec(500));

		if (rainbow_hdc_file(0) != nullptr)
		{
			success = do_write_sector();
			if (success < 88)
				logerror("! SECTOR WRITE (or FORMAT) FAULT !  ERROR CODE %i.\n", success);

			m_hdc_buf_offset = 0;
			m_hdc->buffer_ready(false);
		}

		// CHD WRITE FAILURES  or  UNMOUNTED HARDDSIK TRIGGER A PERMANENT ERROR.
		if (success < 50)
			m_hdc_write_fault = true; // reset only by HDC RESET!
	}

	wg_last = state;  // remember state
}


// Initiated by 'hdc_write_sector' (below)
// - in turn invoked by a WG: 1 -> 0 transit.
// STATUS CODES:
//   0 = DEFAULT ERROR (no HARD DISK FILE ?)
//   10 = CHD WRITE FAILURE (?)

//  50 = SANITY CHECK FAILED (cylinder limit / <> 512 sectors?)

//  88 = (LOW LEVEL) WRITE/FORMAT (sector_count != 1 IGNORED)
//  99 = SUCCESS : SECTOR WRITTEN

// * RELIES * ON THE FACT THAT THERE WILL BE NO MULTI SECTOR TRANSFERS (!)
int rainbow_base_state::do_write_sector()
{
	int feedback = 0; // no error
	m_leds[0] = 0; // ON
	switch_off_timer->adjust(attotime::from_msec(500));

	hard_disk_file *local_hard_disk = rainbow_hdc_file(0); // one hard disk for now.

	if (local_hard_disk)
	{
		hard_disk_info *info = hard_disk_get_info(local_hard_disk);
		if (info)
		{
			feedback = 10;
			m_leds[0] = 1; // OFF

			uint8_t sdh = (m_hdc->read(0x06));

			int hi = (m_hdc->read(0x05)) & 0x07;
			uint16_t cylinder = (m_hdc->read(0x04)) | (hi << 8);

			int sector_number = m_hdc->read(0x03);
			int sector_count = m_hdc->read(0x02); // (1 = single sector)

			if (!(cylinder <= info->cylinders &&                     // filter invalid cylinders
				SECTOR_SIZES[(sdh >> 5) & 0x03] == info->sectorbytes // 512, may not vary
				))
			{
				logerror("...*** SANITY CHECK FAILED (CYLINDER %u vs. info->cylinders %u - - SECTOR_SIZE %u vs. info->sectorbytes %u) ***\n",
					cylinder, info->cylinders, SECTOR_SIZES[(sdh >> 5) & 0x03], info->sectorbytes);
				return 50;
			}
			// Pointer to info + C + H + S
			uint32_t lbasector = get_and_print_lbasector(this, info, cylinder, sdh & 0x07, sector_number);

			if (sector_count != 1) // ignore all SECTOR_COUNTS != 1
				return 88; // logerror(" - ** IGNORED (SECTOR_COUNT !=1) **\n");

			if (hard_disk_write(local_hard_disk, lbasector, m_hdc_buffer))  // accepts LBA sector (uint32_t) !
				feedback = 99; // success
			else
				logerror("...FAILURE **** \n");

		} // IF 'info' not nullptr
	} // IF hard disk present
	return feedback;
}


uint8_t rainbow_base_state::hd_status_60_r()
{
	int data = m_hdc_buffer[m_hdc_buf_offset];
	//logerror("HARD DISK DISK BUFFER: READ offset %04x | data = %02x\n", m_hdc_buf_offset, data); // ! DO NOT CHANGE ORDER !

	m_hdc_buf_offset += 1;
	if (m_hdc_buf_offset >= 1024) // 1 K enforced by controller
	{
		m_hdc_buf_offset = 0;
		m_hdc->buffer_ready(true);
	}
	return data;
}

void rainbow_base_state::hd_status_60_w(uint8_t data)
{
	//logerror("HARD DISK BUFFER: WRITE offset %04x | data = %02x\n", m_hdc_buf_offset, data);

	m_hdc_buffer[m_hdc_buf_offset] = data;
	m_hdc_buf_offset += 1;

	if (m_hdc_buf_offset >= 1024) // 1 K enforced by controller
	{
		m_hdc_buf_offset = 0;
		m_hdc->buffer_ready(true);
	}
}


// Secondary Command / Status Registers(68H) is...
//   (A) a write - only register for commands
//   (B) a read - only register for status signals
// Holds the status of the following signals:
// - 3 hard-wired controller module identification bits.
// - signals from the WD1010 chip,
// - disk drive(latched status signals)
uint8_t rainbow_base_state::hd_status_68_r()
{
	// (*) Bits 5-7 : HARD WIRED IDENTIFICATION BITS, bits 5+7 = 1 and bit 6 = 0  (= 101 f?r RD51 module)
	int data = 0xe0; // 111 gives DRIVE NOT READY (when W is pressed on boot screen)
	if (m_inp5->read() == 0x01 && rainbow_hdc_file(0) != nullptr)
		data = 0xa0; // A0 : OK, DRIVE IS READY (!)

	int my_offset = 0x07;
	int stat = m_hdc->read(my_offset);
//  logerror("(x68) WD1010 register %04x (STATUS) read, result : %04x\n", my_offset, stat);

	// NOTE: SEEK COMPLETE IS CURRENTLY HARD WIRED / NOT FULLY EMULATED -
	// Bit 4 : SEEK COMPLETE: This status bit indicates that the disk drive positioned the R/W heads over the desired track on the disk surface.

	// (ALT.TEXT): "Seek Complete - When this signal from the disk drive goes low(0), it indicates that the R /W heads settled on the correct track.
	// Writing is inhibited until this signal goes low(0).  Seek complete is high(1) during normal seek operation.
	if (stat & 16) // SEEK COMPLETE (bit 4)?
		data |= 16;

	// Bit 3 : DIRECTION : This bit indicates the direction the read/write heads in the disk
	//                     drive will move when the WD1010 chip issues step pulse(s). When high(1), the R / W heads will move toward the spindle.
	//                     When low (0), the heads will move away from the spindle, towards track O.
	if (m_hdc_direction)
		data |= 8;

	// Bit 2 :  LATCHED STEP PULSE: This status bit from the step pulse latch indicates if the WD1010
	//              chip issued a step pulse since the last time the 8088 processor cleared the step pulse latch.
	if (m_hdc_step_latch)
		data |= 4;

	// Bit 1 :  LATCHED INDEX : This status bit from the index latch indicates if the disk drive
	//                  encountered an index mark since the last time the 8088 processor cleared the index latch.
	if (m_hdc_index_latch)
		data |= 2;

	// Bit 0 :  CTRL BUSY : indicates that the WD 1010 chip is accessing the sector buffer. When this bit is set,
	//          the 8088 cannot access the WD 1010 registers.
	if (stat & 128) // BUSY (bit 7)?
		data |= 1;

	return data;
}


// 68 (WRITE): Secondary Command Registers (68H) - -  "write-only register for commands"
// - see TABLE 4.8 (4-24)
void rainbow_base_state::hd_status_68_w(uint8_t data)
{
	// Bit 4-7 : --- not used / reserved

	// Bit 3 :  CLEAR STEP LATCH : This bit BAD<3>H clears out the step pulse latch. The step pulse
	//latch is set every time the WD1010 chip issues a step pulse.The output of the step pulse latch is sent to the secondary status register.
	if (data & 0x08)
		m_hdc_step_latch = false;

	// Bit 2 :  CLEAR INDEX LATCH : This bit BAD<2>H clears out the index latch. The index latch is
	//set when the disk drive senses the index position on the disk.The index latch output is sent to the secondary status register.
	if (data & 0x04)
		m_hdc_index_latch = false;

	// * Bit 1 :  SOFTWARE INITIALIZE: The BAD<I>H bit sets this bit. This bit, when set, initializes the
	// controller. The controller cannot be accessed for 7 microseconds(us) after the 8088 issues the software initialize.
	if (data & 0x02)
		hdc_reset();

	// * Bit 0 :  SET BUFFER READY : READ SECTOR command: this bit, when set, tells the WDI010 chip that the sector buffer was emptied which would then end the
	//          command. WRITE SECTOR / FORMAT CMD: bit tells the WD1010 that the sector buffer now contains valid data for transfer to the disk drive.

	// * SET BY BIOS:  2 : (WD1010 IRQ based transfer operation?)  @ 0810
	//                 1 : see  @ 088D after 'READ_SECTOR_OK'
	if (data & 0x01)
	{
		m_leds[0] = 0;  // 1 = OFF (One of the CPU LEDs as DRIVE LED) = HARD DISK ACTIVITY =
		switch_off_timer->adjust(attotime::from_msec(500));

		m_hdc->buffer_ready(true);
	}
}


/*
/ READ ONLY REGISTER (HEX 69)

The drive status register at I/O address 69H is a read-only register
that monitors the status of control and error signals to/from the disk drive.

0 Drive Select - high (1) indicates that the controller module is selecting the drive.

1-3 Head Select - These 3 bits are the binary head address of the R/W head
selected for the current read/write operation. The RD51 drive has 4 heads.

4 Write Gate - The WDlOI0 chip asserts this bit high (1) to inform the 8088 of
data being written on the disk. Signal also enables write current in disk drive.

5 Drive Write Fault - The disk drive asserts this bit high (1) to indicate that a condition
exists at the drive that may cause improper writing on the disk.
Inhibits further writing until the error is corrected (.. until RESET?) [Bavarese]

6 Drive Ready - When the disk drive together with SEEK COMPLETE asserts this
bit high (1), it indicates that the drive is ready to read, write, or
seek. When this bit is low (0), all reading, writing, and seeking are
inhibited.

7 Track 0 - The disk drive sets this bit high (1) when the R/W heads are
positioned over cylinder 0 (the data track furthest away from the spindle).
*/
uint8_t rainbow_base_state::hd_status_69_r()
{
	int hs = m_hdc->read(0x06) & (1 + 2 + 4); // SDH bits 0-2 = HEAD #
//  logerror("(x69 READ) %i = HEAD SELECT WD1010\n", hs);

	uint8_t data = (hs << 1);

	// DRIVE SELECT: 2 bits in SDH register of WDx010 could address 4 drives.
	// External circuit supports 1 drive here (DRIVE 0 selected or deselected)
	int drv = ((m_hdc->read(0x06) >> 3) & 0x01);  // 0x03 gives error R6 with DIAG.DISK
	if (drv == 0)
		data |= 1; //      logerror("(x69 READ) %i = _DRIVE # 0_ SELECT! \n", drv);

	if (m_hdc_write_gate) // WRITE GATE (cached here)
		data |= 16;

	if (m_hdc_write_fault)
		data |= 32;

	if (m_hdc_drive_ready)
		data |= 64;

	// Fake TRACK 0 signal  (normally FROM DRIVE)
	if ((m_hdc->read(0x04) == 0) && (m_hdc->read(0x05) == 0)) // CYL.LO - CYL.HI
		data |= 128; //      logerror("(x69 READ) TRACK 00 detected\n");

	return data;
}

// TREAT SIGNALS FROM / TO CONTROLLER
WRITE_LINE_MEMBER(rainbow_base_state::hdc_step)
{
	m_hdc_step_latch = true;

	m_leds[0] = 0;  // 1 = OFF (One of the CPU LEDs as DRIVE LED)  = HARD DISK ACTIVITY =
	switch_off_timer->adjust(attotime::from_msec(500));
}

WRITE_LINE_MEMBER(rainbow_base_state::hdc_direction)
{
	m_hdc_direction = state; // (0 = OUT)
}

READ_LINE_MEMBER(rainbow_base_state::hdc_drive_ready)
{
	return m_hdc_drive_ready;
}

READ_LINE_MEMBER(rainbow_base_state::hdc_write_fault)
{
	return m_hdc_write_fault;
}

// Buffer counter reset when BCR goes from 0 -> 1
WRITE_LINE_MEMBER(rainbow_base_state::hdc_bcr)
{
	static int bcr_state;
	if (bcr_state == 0 && state == 1)
		hdc_buffer_counter_reset();
	bcr_state = state;
}

void rainbow_base_state::hdc_buffer_counter_reset()
{
	m_hdc->buffer_ready(false);
	m_hdc_buf_offset = 0;
}

// DATA REQUEST - When high (..) initiates data transfers
// to or from the sector buffer. On a READ, this signal
// goes high AFTER the sector buffer is filled.

// On a WRITE / FORMAT command, signal goes high when the WD1010
// chip is READY TO ACCESS the information in the sector buffer.
WRITE_LINE_MEMBER(rainbow_base_state::hdc_bdrq)
{
	static int old_state;
//  logerror("BDRQ - BUFFER DATA REQUEST OBTAINED: %u\n", state);

	if (state == 1 && old_state == 0)
	{
		hdc_buffer_counter_reset();

		m_bdl_irq = state;
		update_bundle_irq(); // TRIGGER AN INTERRUPT
	}
	old_state = state;
}
// ---------------------------- / RD51 HARD DISK CONTROLLER ----------------------------------


// IRQ service for both RD51 and COMM. OPTION
void rainbow_base_state::update_bundle_irq()
{
	if (m_bdl_irq == 0)
	{
		lower_8088_irq(IRQ_BDL_INTR_L);

		if (m_inp5->read() == 0x01)
			hdc_buffer_counter_reset();
	}
	else
	{
		raise_8088_irq(IRQ_BDL_INTR_L);
	}
}

WRITE_LINE_MEMBER(rainbow_base_state::bundle_irq)
{
	m_bdl_irq = state;
	update_bundle_irq();
}


uint8_t rainbow_modela_state::system_parameter_r()
{
	/*  Info about option boards is in bits 0 - 3:
	SYSTEM PARAMETER INFORMATION: see AA-P308A-TV page 92 section 14.0
	Bundle card (1) | Floppy (2) | Graphics (4) | Memory option (8)
	0 1 2 3 4 5 6 7
	B F G M
	(bit SET means NOT present; 4-7 reserved )

	B : no separation between the 2 available 'bundle cards' (HD controller / COMM.OPTION) ?

	M : old RAM extension (128 / 192 K ?) detected with OPTION_PRESENT bit, newer models 'by presence'.
	BIOS uses a separate IRQ vector for RAM board detection (at least on a 100-B).
	*/
	return ((m_inp5->read() == 1 ? 0 : 1) |
			(m_inp7->read() == 1 ? 0 : 4) | // Floppy is always present (bit 1 zero)
			(m_ram->size() > 0x10000 ? 0 : 8) |
			0xf0); // unverified
}

uint8_t rainbow_modelb_state::system_parameter_r()
{
	/*  Info about option boards is in bits 0 - 3:
	SYSTEM PARAMETER INFORMATION: see AA-P308A-TV page 92 section 14.0
	Bundle card (1) | Floppy (2) | Graphics (4) | Memory option (8)
	0 1 2 3 4 5 6 7
	B F G M
	(bit SET means NOT present; 4-7 reserved )

	B : no separation between the 2 available 'bundle cards' (HD controller / COMM.OPTION) ?

	M : old RAM extension (128 / 192 K ?) detected with OPTION_PRESENT bit, newer models 'by presence'.
	BIOS uses a separate IRQ vector for RAM board detection (at least on a 100-B).
	*/
	return ((m_inp5->read() == 1 ? 0 : 1) |
			(m_inp7->read() == 1 ? 0 : 4) | // Floppy is always present (bit 1 zero)
			0xf8); // unverified
}

//  [02] COMMUNICATIONS STATUS REGISTER - PAGE 154 (**** READ **** )
//  Used to read status of SERIAL port, IRQ line of each CPU, and MHFU logic enable signal.

// 0 COMM RI   (reflects status of RI line at COMM port)
// 1 COMM SI / SCF(reflects status of speed indicator line or
//                 the secondary receive line signal detect at COMM port)
// 2 COMM DSR  (reflects status of DSR at COMM)
// 3 COMM CTS  (reflects status of CTS at COMM)
// 4 COMM RLSD (receive line signal detect at COMM; also connected to DCDA on MPSC)
uint8_t rainbow_base_state::comm_control_r()
{
	bool is_mhfu_enabled = false;
	if (m_power_good)
		is_mhfu_enabled = m_crtc->MHFU(MHFU_IS_ENABLED);

	return (m_comm_port->ri_r() ? 0x01 : 0x00) |
		   (m_comm_port->si_r() ? 0x02 : 0x00) |
		   (m_comm_port->dsr_r() ? 0x04 : 0x00) |
		   (m_comm_port->cts_r() ? 0x08 : 0x00) |
		   (m_comm_port->dcd_r() ? 0x10 : 0x00) |
		   (is_mhfu_enabled ? 0x00 : 0x20) |   // (L) status of MHFU flag => bit pos.5
		   (m_int88 ? 0x00 : 0x40) |           // (L)
		   (m_intz80 ? 0x00 : 0x80);           // (L)

}

//  Communication control register of -COMM- port (when written):
// (these 4 bits talk DIRECTLY to the COMM port according to schematics):
// 0 COMM SPD SEL H (controls speed select line of COMM port)
// 1 COMM SRTS H     (controls secondary request to send line of COMM)
// 2 COMM DTR L      (controls terminal ready line of COMM)
// 3 COMM RTS        (controls request to send line of COMM)
void rainbow_base_state::comm_control_w(uint8_t data)
{
	logerror("%02x to COMM.CONTROL REGISTER ", data);

	m_comm_port->write_spds(BIT(data, 0));
	// SRTS not currently emulated
	m_comm_port->write_dtr(BIT(data, 2));
	m_comm_port->write_rts(BIT(data, 3));

	/* 8088 LEDs:
	5  7  6  4    <- BIT POSITION
	D6 -D5-D4-D3  <- INTERNAL LED NUMBER (DEC PDF)
	-4--5--6--7-  <- NUMBERS EMBOSSED ON BACK OF PLASTIC HOUSING (see error chart)
	*/
	m_leds[3] = BIT(data, 5); // LED "D6"
	m_leds[4] = BIT(data, 7); // LED "D5"
	m_leds[5] = BIT(data, 6); // LED "D4"
	m_leds[6] = BIT(data, 4); // LED "D3"
}

// 8088 writes to port 0x00 (interrupts Z80)
// See page 133 (4-34)
void rainbow_base_state::i8088_latch_w(uint8_t data)
{
	// logerror("%02x to Z80 mailbox\n", data);

	// The interrupt vector address(F7H) placed on the bus is hardwired into the Z80A interrupt vector encoder.
	// The F7H interrupt vector address causes the Z80A processor to perform an RST 30 instruction in
	// interrupt mode 0
	m_z80->set_input_line_and_vector(0, ASSERT_LINE, 0xf7); // Z80
	m_z80_mailbox = data;

	m_intz80 = true;
}

// Z80 reads port 0x00
// See page 134 (4-35)
uint8_t rainbow_base_state::z80_latch_r()
{
	// logerror("Read %02x from Z80 mailbox\n", m_z80_mailbox);
	m_z80->set_input_line(0, CLEAR_LINE);

	m_intz80 = false;
	return m_z80_mailbox;
}

// Z80 writes to port 0x00 (interrupts 8088)
// See page 134 (4-35)
void rainbow_base_state::z80_latch_w(uint8_t data)
{
	// logerror("%02x to 8088 mailbox\n", data);
	raise_8088_irq(IRQ_8088_MAILBOX);
	m_8088_mailbox = data;

	m_int88 = true;
}

// 8088 reads port 0x00. See page 133 (4-34)
uint8_t rainbow_base_state::i8088_latch_r()
{
	// logerror("Read %02x from 8088 mailbox\n", m_8088_mailbox);
	lower_8088_irq(IRQ_8088_MAILBOX);

	m_int88 = false;
	return m_8088_mailbox;
}

// (Z80) : WRITE to 0x20
void rainbow_base_state::z80_diskdiag_read_w(uint8_t data)
{
	m_zflip = true; //  "a write to 20H will _SET_ ZFLIP"
}

// (Z80) : PORT 21H * WRITE *
void rainbow_base_state::z80_diskdiag_write_w(uint8_t data)
{
	/*   Z80 LEDs:
	4   5   6  <- bit #
	D11 D10 -D9 <- INTERNAL LED NUMBER (see PDF)
	-1 --2-- 3  <- NUMBERS EMBOSSED ON BACK OF PLASTIC HOUSING (see error chart)
	*/
	m_leds[0] = BIT(data, 4); // LED "D11"
	m_leds[1] = BIT(data, 5); // LED "D10"
	m_leds[2] = BIT(data, 6); // LED "D9"

	m_zflip = false; // "a write to 21H will reset ZFLIP"
}

// (Z80) : PORT 20H / 21H  _READ_
uint8_t rainbow_base_state::z80_generalstat_r()
{
	/*
	General / diag.status register Z80 / see page 157 (table 4-18).
	---- BITS FROM RX50 CONTROLLER CARD:
	D7 : STEP L : reflects status of STEP signal _FROM FDC_
	(when this 2us output pulse is low, the stepper will move into DIR)
	D6 : WRITE GATE L :reflects status of WRITE GATE signal _FROM FDC_
	(asserted low before data can be written on the diskette)
	D5 : TR00: reflects status of TRACK 0 signal (= 1) * from the disk drive *
	D4 : DIR L: reflects status of DIRECTION signal * FROM FDC * to disk
	(when low, the head will step towards the center)
	D3 : READY L: reflects status of READY L signal * from the disk drive *
	(low active, asserts when disk is inserted and door is closed)
	---- BITS BELOW FROM MAINBOARD:
	D2 : INT88 L: (bit reads the INT88 bit sent by Z80 to interrupt 8088)
	D1 : INTZ80 L: (bit reads the INTZ80 bit sent by 8088 to interrupt Z80)
	D0 : ZFLIP L: (read from the diagnostic control register of Z80A)
	*/
	static int last_track;
	int track = 0;

	int fdc_step = 0;
	int fdc_ready = 0;
	int tk00 = 0;
	int fdc_write_gate = 0;
	int last_dir = 0;

	uint8_t fdc_status;

	if (m_fdc)
	{
		track = m_fdc->track_r();
		if (track == 0)
			tk00 = 1;

		if (track != last_track)
			fdc_step = 1;  // calculate STEP (sic)

		last_dir = track > last_track ? 0 : 1; // see WD_FDC
		last_track = track;

		fdc_status = m_fdc->status_r();

		if ((fdc_status & 0x80) == 0) // (see WD_FDC: S_WP = 0x40, S_NRDY = 0x80, S_TR00 = 0x04)
			fdc_ready = 1;

		if (fdc_ready && ((fdc_status & 0x40) == 0) && m_power_good)
			fdc_write_gate = 1; // "valid only when drive is selected" !
	}
	// logerror(" RDY:%x  WG:%d ",fdc_ready,fdc_write_gate);
	int data = (fdc_step ? 0x00 : 0x80) |
			   (fdc_write_gate ? 0x00 : 0x40) |
			   (tk00 ? 0x20 : 0x00) |  // ***** ALL LOW ACTIVE - EXCEPT tk00 :
			   (last_dir ? 0x00 : 0x10) |
			   (fdc_ready ? 0x00 : 0x08) |
			   (m_int88 ? 0x00 : 0x04) |
			   (m_intz80 ? 0x00 : 0x02) |
			   (m_zflip ? 0x00 : 0x01);

	return data;
}


// (Z80) : PORT 40H _READ_
// 40H diskette status Register **** READ ONLY *** ( 4-60 of TM100.pdf )
uint8_t rainbow_base_state::z80_diskstatus_r()
{
	int track = 0xEE;
	int data = m_z80_diskcontrol & (255 - 0x80 - 0x40 - 0x20 - 0x04); // 00011011

	// D7: DRQ: reflects status of DATA REQUEST signal from FDC.
	// '1' indicates that FDC has read data OR requires new write data.

	// D6: IRQ: indicates INTERRUPT REQUEST signal from FDC. Indicates that a
	//          status bit has changed. Set to 1 at the completion of any
	//          command (.. see page 207 or 5-25).
	if (m_fdc)
	{
		data |= m_fdc->drq_r()   ? 0x80 : 0x00;
		data |= m_fdc->intrq_r() ? 0x40 : 0x00;
		track = m_fdc->track_r();

		// D2: TG43 * LOW ACTIVE * :  0 = INDICATES TRACK > 43 SIGNAL FROM FDC TO DISK DRIVE.
		// (asserted when writing data to tracks 44 through 79)
		data |= (track > 43) ? 0x00 : 0x04;  // ! LOW ACTIVE !
	}

	// D5: SIDE 0 * HIGH ACTIVE *: status of side select signal at J2 + J3 of RX50 controller.
	//              For 1 sided drives, this bit will always read low (0).
	if (m_floppy != nullptr)
		data |= m_floppy->ss_r() ? 0x20 : 0x00;

	// *LOW ACTIVE *
	// D4: MOTOR 1 ON L: 0 = indicates MOTOR 1 ON bit is set in drive control reg.
	// D3: MOTOR 0 ON L: 0 = indicates MOTOR 0 ON bit is set in drive  "

	// Print HEX track number
	static uint8_t bcd2hex[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71 };
	// 0...9 ,A (0x77), b (0x7c), C (0x39) , d (0x5e), E (0x79), F (0x71)
	m_digits[0] = bcd2hex[(track >> 4) & 0x0f];
	m_digits[1] = bcd2hex[track & 0x0f];

	// D1: DS1 H: reflect status of bits 0 and 1 from disk.control reg.
	// D0: DS0 H: "
	return data;
}


// (Z80) : PORT 40H  * WRITE *
// NOTE: routine will accept invalid drive letters...

// ALL SIGNALS ARE HIGH ACTIVE (H), EXCEPT:
// BIT 5 : SIDE 0 L : For single sided drives, this bit is always set to 0 for side O.
void rainbow_base_state::z80_diskcontrol_w(uint8_t data)
{
	int enable_start;
	int disable_start; // set defaults

	int selected_drive = INVALID_DRIVE;

	int drive = 0;
	if (m_inp10->read() && ((data & 3) < 2))
		drive = (data & 1) + 2;
	else
		drive = data & 3;

	if (m_floppies[drive])
	{
		m_floppy = m_floppies[drive]->get_device();
		if (m_floppy)
			selected_drive = drive;
	}

	if (selected_drive == INVALID_DRIVE)
	{
		logerror("(m_present_drive = %i)   ** SELECTED DRIVE ** INVALID. (selected drive = %i)\n", m_present_drive, selected_drive);

		m_present_drive = INVALID_DRIVE;
		m_floppy = nullptr;
	}

	for (int i = 0; i < 4; i++)
		m_driveleds[i] = (selected_drive == i) ? 1 : 0;
	switch_off_timer->adjust(attotime::from_msec(500));

	if (m_floppy != nullptr)
	{
		m_fdc->set_floppy(m_floppy);  // Sets new  _image device_
		m_fdc->dden_w(0); // 0 = MFM
		m_floppy->ss_w((data & 0x20) ? 1 : 0); // RX50 board in Rainbow has 'side select'
		m_floppy->set_rpm(300.);

		if ( !m_floppy->exists() && (selected_drive > 1) )
			popmessage("NO IMAGE ATTACHED TO %c\n", 65 + selected_drive );
	}

	if (selected_drive < MAX_FLOPPIES)
	{
		m_present_drive = selected_drive;

		bool force_ready = ((data & 4) == 0) ? true : false;
		m_fdc->set_force_ready(force_ready); // 1 : assert DRIVE READY on FDC (diagnostic override)

		if (selected_drive < 2)
		{
			data |= 8;
			enable_start = 0;
			disable_start = 2;
		}
		else
		{
			data |= 16;
			enable_start = 2;
			disable_start = 4;
		}

		// RX-50 has head A and head B (1 for each of the 2 disk slots in a RX-50).
		// Assume the other one is switched off -
		for (int f_num = 0; f_num < MAX_FLOPPIES; f_num++)
		{
		floppy_image_device *tmp_floppy = m_floppies[f_num]->get_device();

		if (!tmp_floppy)
			continue;
		tmp_floppy->mon_w(ASSERT_LINE);
		if ((f_num >= enable_start) && (f_num < disable_start))
			tmp_floppy->mon_w(CLEAR_LINE); // enable
		}
	}

	data = (data & (255 - 3)); // invalid drive = DRIVE 0 ?!

	if (m_present_drive == INVALID_DRIVE)
		logerror("**** INVALID DRIVE ****\n");
	else
		data = data | m_present_drive;

	m_z80_diskcontrol = data;
}
// --------- END OF Z80 --------------------

uint8_t rainbow_base_state::read_video_ram_r(offs_t offset)
{
	return m_p_ram[offset];
}




// **************************************************
// VIDEO INTERRUPT HANDLING
// **************************************************

// CPU acknowledge of VBL IRQ resets counter
IRQ_CALLBACK_MEMBER(rainbow_base_state::irq_callback)
{
	int intnum = -1;
	for (int i = IRQ_8088_VBL; i >= 0; i--)
	{
			if (m_irq_mask & (1 << i))
			{
				if (i == IRQ_8088_VBL)  // If VBL IRQ acknowledged...
					m_crtc->MHFU(MHFU_RESET); // ...reset counter (also: DC012_W)

				intnum = vectors[i] | m_irq_high;
				break;
			}
	}
	return intnum;
}

// NEC7220 Vsync IRQ ***************************************** GDC

// VERIFY: SCROLL_MAP & COLOR_MAP are updated at the next VSYNC (not immediately)... Are there more registers?
WRITE_LINE_MEMBER(rainbow_base_state::GDC_vblank_irq)
{
	// VERIFICATION NEEDED: IRQ raised before or after new palette loaded...?
	if (m_gdc_mode_register & GDC_MODE_ENABLE_VSYNC_IRQ) // 0x40
		raise_8088_irq(IRQ_GRF_INTR_L);
	else
		lower_8088_irq(IRQ_GRF_INTR_L);

	m_monitor_suggested = m_inp13->read();
	if (m_monitor_suggested < 1 || m_monitor_suggested > 3)
		m_monitor_suggested = COLOR_MONITOR;

	int mono_sum = 0;
	int green_sum = 0;
	for (uint8_t xi = 0; xi < 16; xi++) // DELAYED LOAD OF PALETTE ...
	{
		uint8_t colordata1  = m_gdc_color_map[xi];
		uint8_t colordata2 = m_gdc_color_map[xi + 16];      // Does it matter if the palette is incomplete...?

		//              Color map:  32 x 8
		//              2nd 16 Byte     1st 16 Bytes (colordata1)
		//              -----------     ------------
		//              7..4  3..0      7..4  3..0
		//              Mono  Blue      Red   Green
		// NOTE: 2nd 16 BYTES ARE MONO PALETTE, 1st 16 ARE COLOR PALETTE * HERE * (on the VT240 driver, it is the other way round)

		uint8_t mono = (colordata2 & 0xF0) >> 4;  // FIXME: limit palette in appropriate modes on 100-A
		mono_sum += mono;

		uint8_t blue = (colordata2 & 0x0F);

		uint8_t red  = (colordata1 & 0xF0) >> 4;
		uint8_t green =(colordata1 & 0x0F);
		green_sum += green;

		switch (m_monitor_suggested)
		{
		case MONO_MONITOR:
			switch (m_inp9->read()) //  - monochrome monitor (phosphor) type  (1,2,3)
			{
			case 1: // BLACK & WHITE
				m_palette2->set_pen_color(xi + 16, pal4bit(mono), pal4bit(mono), pal4bit(mono) );
				break;

			case 2: // GREEN SHADES. Hand picked value from vtvideo coarsly transformed into a RGB value:
				red   = uint8_t( ( 35.0f / 100.0f) *  ( video_levels[ mono ] / 2.55f) );
				green = uint8_t( (145.0f / 100.0f) *  ( video_levels[ mono ] / 2.55f) );
				blue  = uint8_t( ( 75.0f / 100.0f) *  ( video_levels[ mono ] / 2.55f) );
				m_palette2->set_pen_color(xi + 16, rgb_t( red, green, blue) );
				break;

			case 3: // AMBER. Assumption: "normal" value at 80 % is 213, 146, 82 (decimal)
				red   = uint8_t( (213.0f / 100.0f) *  ( video_levels[ mono ] / 2.55f) );
				green = uint8_t( (146.0f / 100.0f) *  ( video_levels[ mono ] / 2.55f) );
				blue  = uint8_t( ( 82.0f / 100.0f) *  ( video_levels[ mono ] / 2.55f) );
				m_palette2->set_pen_color(xi + 16, rgb_t( red, green, blue) );
				break;
			}
			break;

		case COLOR_MONITOR:
			if (!(m_gdc_mode_register & GDC_MODE_ENABLE_VIDEO))
				red = blue = 0; // Page 21 of AA-AE36A (PDF) explains why

			m_palette2->set_pen_color(xi, pal4bit(red) , pal4bit(mono) , pal4bit(blue));
			break;

		case DUAL_MONITOR:
			m_palette2->set_pen_color(xi, pal4bit(red), pal4bit(green), pal4bit(blue));
			break;
		}
	} // palette (loop)

	if (green_sum > 0 && green_sum == mono_sum) // (R-G-B + M) palette (split cable). Examples: PACMAN, AutoCad
	{
		if (m_monitor_suggested == MONO_MONITOR)
			logerror("\n[HINT: COLOR PALETTE DETECTED - SUITABLE FOR DUAL MONITOR] ");
		if (m_inp13->read() == AUTODETECT_MONITOR)
			m_monitor_suggested = DUAL_MONITOR;
	}

	if (green_sum == 0 && mono_sum > 0)  // No green = original DEC spec. Example: NLANDER. All older libaries use R-M-B.
	{
		if (m_inp13->read() == AUTODETECT_MONITOR)
			m_monitor_suggested = COLOR_MONITOR;
		if (m_monitor_suggested == DUAL_MONITOR)
			logerror("\n[HINT: SINGLE COLOR MONITOR ONLY!  GREEN MISSING => NO SPLIT CABLE]");
	}

} // 7220 vblank IRQ


WRITE_LINE_MEMBER(rainbow_base_state::video_interrupt)
{
	if (state == ASSERT_LINE)
		raise_8088_irq(IRQ_8088_VBL);
	else
		lower_8088_irq(IRQ_8088_VBL);

	if (state == ASSERT_LINE && m_power_good && m_crtc->MHFU(MHFU_IS_ENABLED)) // If enabled...
	{
		if (m_crtc->MHFU(MHFU_VALUE) > 10) // + more than (10 * 16.666) msecs gone (108 ms would be by the book)
		{
			m_crtc->MHFU(MHFU_RESET_and_DISABLE);
			popmessage("**** WATCHDOG TRIPPED:nVBL IRQ not acknowledged within (at least) 108 milliseconds. ****");

			if (m_inp12->read() == 0x01) // (DIP) for watchdog active?
				cmd_timer->adjust(attotime::from_msec(RESET_DURATION_MS));
		}
	}
}

// Reflects bits from 'diagnostic_w' (1:1), except test jumpers
uint8_t rainbow_base_state::diagnostic_r() // 8088 (port 0A READ). Fig.4-29 + table 4-15
{
	return ((m_diagnostic & 0xf1) | m_inp1->read() | m_inp2->read() | m_inp3->read());
}

void rainbow_base_state::diagnostic_w(uint8_t data) // 8088 (port 0A WRITTEN). Fig.4-28 + table 4-15
{
	//    logerror("%02x to diag port (PC=%x)\n", data, m_i8088->pc());

	// ZRESET from 8088 to Z80 - - HIGH at powerup!
	if (!(data & 1))
	{
		m_z80->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_z80_halted = true;
	}

	if ((data & 1) && (m_z80_halted))
	{
		m_zflip = true;
		m_z80_halted = false;

		m_z80->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_z80->reset();
	}

	if ((m_diagnostic & 1) && !(data & 1)) // ZRESET goes LOW...
	{
		logerror("FDC ** RESET **\n");
		m_fdc->reset();
	}

	if (!(m_diagnostic & 1) && (data & 1)) // ZRESET goes HIGH...
	{
		logerror("FDC RESTORE\n");
		m_fdc->reset(); // See formatter description p.197 or 5-13
	}

	m_screen_blank = BIT(data, 1)? false : true; // inverse logic

	// Switch determines how the monochrome output pin is taken from:
	//    0  = M(ono) out from system module (DC011/DC012). Default, also used to setup dual monitors.
	//    1  = M(ono) output from GRAPHICS OPTION. (G)reen remains unused with a single COLOR monitor.
	m_onboard_video_selected = (data & 0x04) ? false : true;
	if (!m_onboard_video_selected)
	{
		if (m_inp7->read() == 1)
		{
			logerror("HINT: GRAPHICS OPTION ON. TEXT ONLY (DC011/DC012) OUTPUT NOW DISABLED.\n");
		}
		else
		{       logerror("ALARM: GRAPHICS OPTION * SWITCHED OFF * VIA DIP. TEXT OUTPUT STILL ENABLED!\n");
			m_onboard_video_selected = true;
		}
		logerror("DATA: %x (PC=%x)\n", data, m_i8088->pc());
	}

	// BIT 3: PARITY (1 enables parity test on memory board. Usually 64K per bank). -> ext_ram_w.
	if (data & 0x08)
		logerror("*** PARITY TEST [on RAM EXTENSION] - (bit 3 - diagnostic_w)\n");

	// MISSING BITS (* not vital for normal operation, see diag.disk) -
	// * BIT 4: DIAG LOOPBACK (0 at power-up; 1 directs RX50 and DC12 output to printer port)
	// * BIT 5: PORT LOOPBACK (1 enables loopback for COMM, PRINTER, KEYBOARD ports)

	/* 2.1.7.3 DIAGNOSTIC LOOPBACK Maintenance Bit - The DIAGNOSTIC LOOPBACK bit is a
	maintenance bit that is cleared on power - up.This bit, when set to 1,
	allows the floppy data separator and the serial video output to be tested
	through the use of the printer port. The following table shows how signals are routed.

	DIAGNOSTIC LOOPBACK = 0     DIAGNOSTIC LOOPBACK = 1     SIGNAL INPUT
	SIGNAL SOURCE               SIGNAL SOURCE               TO
	FROM                        FROM
	PRT RDATA(J2)               VIDEO OUT                   PRT RXD(7201)
	PRT RXTXC                   500 KHZ                     PRT RXTXC(7201)
	MASTER CLK                  250 KHZ                     VIDEO CLK(DCO11)
	FLOPPY RAW DATA             PRT TXD(7201)               FLOPPY DATA SEPARATOR

	During Diagnostic Loopback, the - TEST input of the 8088 is connected to the
	interrupt output of the MPSC.Thus, using the 8088's WAIT instruction in a
	polled I / O loop, the diagnostic firmware will be able to keep up with the
	500 Kb data rate on the MPSC.
	*/
	if (data & 16)
	{
		logerror("WARNING: UNEMULATED DIAG LOOPBACK (directs RX50 and DC12 output to printer port) ****\n");
	}

	address_space &io = m_i8088->space(AS_IO);
	if (data & 32)
	{
		/* BIT 5: PORT LOOPBACK (1 enables loopback for COMM, PRINTER, KEYBOARD ports)
		2.1.7.2. of AA-V523A-TV (PDF Mar83) says how the signals are routed:
		port_loopback_0  |  port_loopback_1   SIGNAL INPUT TO
		COMM RCV DATA.......COMM TXD..........COMM_RXD
		PRT  RCV DATA.......KBD TXD...........PRT RDATA
		KBD  RCV DATA.......PRT TXD...........KBD RXD
		*/
		logerror("WARNING: UNEMULATED PORT LOOPBACK (COMM, PRINTER, KEYBOARD ports) ****\n");

		io.unmap_readwrite(0x40, 0x43);  // unmap MPSC handlers to prevent CPU crashes ("INTERRUPTS OFF")
	}

	// Install 8088 read / write handler once loopback test is over
	if ( !(data & 32) && (m_diagnostic & 32) )
	{
			io.install_readwrite_handler(0x40, 0x43, read8sm_delegate(*m_mpsc, FUNC(upd7201_device::cd_ba_r)), write8sm_delegate(*m_mpsc, FUNC(upd7201_device::cd_ba_w)));
			logerror("\n **** COMM HANDLER INSTALLED **** ");
			//popmessage("Autoboot from drive %c", m_p_nvram[0xab] ? (64 + m_p_nvram[0xab]) : 0x3F );
	}

	// BIT 6: Transfer data from volatile memory to NVM  (PROGRAM: 1 => 0   BIT 6)
	if (!(data & 0x40) && (m_diagnostic & 0x40))
		memcpy(m_p_nvram, m_p_vol_ram, 256);

	// BIT 7: Transfer data from NVM to volatile memory (RECALL 0 => 1     BIT 7)
	if ((data & 0x80) && !(m_diagnostic & 0x80))
		memcpy(m_p_vol_ram, m_p_nvram, 256);

	m_diagnostic = data;
}

// KEYBOARD
void rainbow_base_state::update_kbd_irq()
{
	if ((m_kbd_rx_ready) || (m_kbd_tx_ready))
		raise_8088_irq(IRQ_8088_KBD);
	else
		lower_8088_irq(IRQ_8088_KBD);
}

WRITE_LINE_MEMBER(rainbow_base_state::kbd_tx)
{
	m_lk201->rx_w(state);
}

WRITE_LINE_MEMBER(rainbow_base_state::kbd_rxready_w)
{
	m_kbd_rx_ready = (state == 1) ? true : false;
	update_kbd_irq();
}

WRITE_LINE_MEMBER(rainbow_base_state::kbd_txready_w)
{
	m_kbd_tx_ready = (state == 1) ? true : false;
	update_kbd_irq();
}

TIMER_DEVICE_CALLBACK_MEMBER(rainbow_base_state::hd_motor_tick)
{
	if (m_power_good)
		m_crtc->MHFU(MHFU_COUNT); // // Increment IF ENABLED and POWER_GOOD, return count

	m_hdc_index_latch = true; // HDC drive index signal (not working ?)
}

WRITE_LINE_MEMBER(rainbow_modela_state::irq_hi_w)
{
	m_irq_high = 0;
}

// on 100-B, DTR from the keyboard 8051 controls bit 7 of IRQ vectors
WRITE_LINE_MEMBER(rainbow_modelb_state::irq_hi_w)
{
	m_irq_high = (state == ASSERT_LINE) ? 0x80 : 0;
}


// ********************************* NEC UPD7220 ***********************************************
// Readback mode: correct place?  Not for vector mode (really)...?

// NOTE: "More than one plane at a time can be enabled for a write operation; however,
//        only one plane can be enabled for a read operation at anyone time."

uint16_t rainbow_base_state::vram_r(offs_t offset)
{
	if ((!(m_gdc_mode_register & GDC_MODE_VECTOR)) || machine().side_effects_disabled())  // (NOT VECTOR MODE)
	{
		// SCROLL_MAP IN BITMAP MODE ONLY...?
		if (m_gdc_mode_register & GDC_MODE_HIGHRES)
			offset = (m_gdc_scroll_buffer[(offset & 0x3FC0) >> 6] << 6) | (offset & 0x3F);
		else
			offset = (m_gdc_scroll_buffer[(offset & 0x1FC0) >> 6] << 6) | (offset & 0x3F);

		int readback_plane = 0;

		if (!(m_gdc_mode_register & GDC_MODE_ENABLE_WRITES)) // 0x10           // READBACK OPERATION - if ENABLE_WRITES NOT SET
		   readback_plane = (m_gdc_mode_register & GDC_MODE_READBACK_PLANE_MASK) >> 2; // READBACK PLANE 00..02, mask in bits 2+3

		return m_video_ram[ (offset & 0x7fff)  + (0x8000 * readback_plane)];
	}
	return 0xffff;
}

// NOTE: Rainbow has separate registers for fore and background.
void rainbow_base_state::vram_w(offs_t offset, uint16_t data)
{
	if (m_gdc_mode_register & GDC_MODE_HIGHRES)
		offset = (m_gdc_scroll_buffer[(offset & 0x3FC0) >> 6] << 6) | (offset & 0x3F);
	else
		offset = (m_gdc_scroll_buffer[(offset & 0x1FC0) >> 6] << 6) | (offset & 0x3F);

	offset &= 0xffff; // same as in VT240?
	uint16_t chr = data; // VT240 : uint8_t

	if (m_gdc_mode_register & GDC_MODE_VECTOR) // VT240 : if(SELECT_VECTOR_PATTERN_REGISTER)
	{
		chr = bitswap<8>(m_vpat, m_patidx, m_patidx, m_patidx, m_patidx, m_patidx, m_patidx, m_patidx, m_patidx);
		chr |= (chr << 8);
		if (m_patcnt-- == 0)
		{
			m_patcnt = m_patmult;
			if (m_patidx-- == 0)
				m_patidx = 7;
		}
	}
	else
	{
		chr = m_gdc_write_buffer[ m_gdc_write_buffer_index++ ];
		m_gdc_write_buffer_index &= 0xf;

		chr |= (m_gdc_write_buffer[m_gdc_write_buffer_index++] << 8);
		m_gdc_write_buffer_index &= 0xf;
	}

	if (m_gdc_mode_register & GDC_MODE_ENABLE_WRITES) // 0x10
	{
		// ALU_PS register: controls logic used in writing to the bitmap / inhibiting of writing to specified planes.
		//     plane select and logic operations on write buffer... (and more)  **** SEE  PAGE 36 ****
		int ps = m_gdc_alu_ps_register & 0x0F; // PLANE SELECT 0..3    // VT 240 : ~m_gdc_alu_ps_register & 3;
		uint8_t fore = ((m_gdc_fg_bg & 0xf0)) >> 4;
		uint8_t back =  (m_gdc_fg_bg & 0x0f);      // background : 0..3 confirmed, see p.39 AA-AE36A (PDF)

		for (int i = 0; i <= 3; i++)
		{
			if (BIT(ps, i)) // 1 means don't touch (bits already inverted)
			{
				uint16_t mem = m_video_ram[(offset & 0xffff) + (0x8000 * i)];

				uint16_t out = 0; // VT240 : uint8_t
				for (int j = 0; j <= 15; j++)  // REPLACE MODE : one replaced by FG, zero by BG ( 16 instead of 8 bit on VT240 )
					out |= BIT(chr, j) ? ((fore & 1) << j) : ((back & 1) << j);

				switch ((m_gdc_alu_ps_register) & ALU_PS_MODE_MASK)
				{
				case OVERLAY_MODE: // (OR)
					out |= mem;
					break;

				case COMPLEMENT_MODE: // (XOR)
					out ^= ~mem;
					break;

				default: // ALL ELSE
					break;
				}

				if (!(m_gdc_mode_register & GDC_MODE_VECTOR)) // 0 : Text Mode and Write Mask Batch
					out = (out & ~m_gdc_write_mask) | (mem & m_gdc_write_mask);
				else
					out = (out & ~data) | (mem & data);

				if (m_gdc_mode_register & GDC_MODE_ENABLE_WRITES) // 0x10
					m_video_ram[(offset & 0xffff) + (0x8000 * i)] = out;
		   } // if plane selected

			fore >>= 1;
			back >>= 1;

		} // plane select (LOOP)
		return;
	} // if enable_writes
}

// (READ)
// Read  scroll buffer (see GDC Diagnostic Disk, SCROLL BUFFER test)
uint8_t rainbow_base_state::GDC_EXTRA_REGISTER_r(offs_t offset)
{
	uint8_t data = 0;
	switch (offset)
	{
	case 0:
		data = m_gdc_mode_register; // ?
		break;

	case 1:
		if (m_gdc_indirect_register & GDC_SELECT_SCROLL_MAP ) // 0x80
		{
			// Documentation says it is always incremented (read and write):
			data = m_gdc_scroll_buffer[m_gdc_scroll_index++]; // // * READ * SCROLL_MAP ( 256 x 8 )
			m_gdc_scroll_index &= 0xFF; // 0...255  (CPU accesses 256 bytes)
			break;
		}
		else
			logerror("\n * UNEXPECTED CASE: READ REGISTER 50..55 with INDIRECT_REGISTER $%02x and OFFSET $%02x *", m_gdc_indirect_register, offset);
		break;

	case 6:
	case 7:
		data = m_hgdc->read(offset & 0x01);
		break;

	default:
		logerror("\n * UNHANDLED CASE: READ REGISTER 50..55 with INDIRECT_REGISTER $%02x and OFFSET $%02x *", m_gdc_indirect_register, offset);
		break;
	} // switch
	return data;
}

void rainbow_base_state::GDC_EXTRA_REGISTER_w(offs_t offset, uint8_t data)
{
	static int last_message, last_mode, last_readback, last_scroll_index;

	if (offset > 0) // Port $50 reset done @ boot ROM 1EB4/8 regardless if option present.
	{
		if (m_inp7->read() != 1)
		{
			if (last_message != 1)
			{
				popmessage("\nCOLOR GRAPHICS ADAPTER INVOKED.  PLEASE TURN ON THE APPROPRIATE DIP SWITCH, THEN REBOOT.\n");
				logerror("OFFSET: %x (PC=%x)\n", 0x50 +offset , m_i8088->pc());
				last_message = 1;
			}
			return;
		}
	}

	switch (offset)
	{
	case 0: // Mode register must be reloaded following any write to port 50 (software reset).
		// FIXME: "Any write to this port also resynchronizes the
		//        read/modify/write memory cycles of the Graphics Option to those of the GDC." (?)

		if (data & 1) // PDF QV069 suggests 1 -> 0 -> 1. Most programs just set bit 0 (PACMAN).
		{
			// Graphics option software reset (separate from GDC reset...)
			OPTION_GRFX_RESET
			OPTION_RESET_PATTERNS
		}
		break;

	case 1: //  51h = DATA loaded into (a register previously addressed by a write to 53h)
		if (m_gdc_indirect_register & GDC_SELECT_WRITE_BUFFER) // 0x01
		{
			m_gdc_write_buffer_index = 0;                   // (writing to 51h  CLEARS  the index counter)
			break;
		}

		if (m_gdc_indirect_register & GDC_SELECT_COLOR_MAP) // 0x20
		{
			m_gdc_color_map[m_gdc_color_map_index++] = ~data; // tilde data verified by DIAGNOSTIC!
			if (m_gdc_color_map_index == 32)
			{
				m_gdc_color_map_index = 0; // 0...31  (CPU accesses 32 bytes

				logerror("* COLOR MAP FULLY LOADED *\n");
				for (int zi = 0; zi < 16; zi++)
				{
					int g =  m_gdc_color_map[zi] & 0x0F;
					int r = (m_gdc_color_map[zi] & 0xF0) >> 4;

					int b =  m_gdc_color_map[zi + 16] & 0x0F;
					int m =  (m_gdc_color_map[zi + 16] & 0xF0) >> 4;
					logerror("[%d] %1x %1x %1x  %1x (1:1)\n", zi, r, g, b, m);
				}
				logerror("------------------------------\n");
			} // if all colors present
			break;
		}

		if (m_gdc_indirect_register & GDC_SELECT_SCROLL_MAP) // 0x80
		{
			if (!(m_gdc_mode_register & GDC_MODE_READONLY_SCROLL_MAP)) // ? READONLY / WRITE logic  correct...?
			{
				m_gdc_scroll_buffer[m_gdc_scroll_index] = data; // // WRITE TO SCROLL_MAP ( 256 x 8 )

				if (m_gdc_scroll_index == 255)
					logerror("---- SCROLL MAP FULLY LOADED ---*\n");
				m_gdc_scroll_index++;
				m_gdc_scroll_index &= 0xFF; // 0...255  (CPU accesses 256 bytes)
			}
			break;
		}

		// -----------------PATTERN + MULTIPLIER USED IN VECTOR MODE ONLY!
		// SEE PAGE 37 OF AA-AE36A (PDF).
		if (m_gdc_indirect_register & GDC_SELECT_PATTERN_MULTIPLIER) // 0x02
		{
			// On a Rainbow, 12 indicates a multiplier of 16-12 = 4 (example)
			m_patmult = 16 - (data & 15); // 4 bit register  // VT240: "patmult_w"
			break;
		}

		if (m_gdc_indirect_register & GDC_SELECT_PATTERN) // 0x04
		{
			// NOTE : Pattern Multiplier MUST BE LOADED before (!)
			m_vpat = data;
			break;
		}

		if (m_gdc_indirect_register & GDC_SELECT_FG_BG) // 0x08
		{
			m_gdc_fg_bg = data;  // Neither bitswap nor negated (and also not both)...
			break; //  Next: prepare FG / BG (4 bits each) + plane  in  ALU - PLANE_SELECT register.
		}

		if (m_gdc_indirect_register & GDC_SELECT_ALU_PS) // 0x10
		{
			m_gdc_alu_ps_register = ~data;  // Negated...
			break;
		}

		if (m_gdc_indirect_register & GDC_SELECT_MODE_REGISTER) // 0x40
		{
			m_gdc_mode_register =  data; // Neither bitswap nor negated (and also not both)...

			if (data & GDC_MODE_HIGHRES) // 0x01
			{
				if (last_message != 2)
				{
					last_message = 2;
					logerror("* HIGH RESOLUTION *\n");
				}
			}
			else
			{
				if (last_message != 3)
				{
					last_message = 3;
					logerror("MEDIUM RESOLUTION\n");
				}
			}

			if (last_mode != (data & GDC_MODE_VECTOR)) // 0x02
			{
				last_mode = data & GDC_MODE_VECTOR;
				if (data & GDC_MODE_VECTOR)
					logerror(" VECTOR MODE ");
				else
					logerror(" WORD MODE ");
			}

			if (last_readback != (data & GDC_MODE_ENABLE_WRITES)) // 0x10
			{
				last_readback = data & GDC_MODE_ENABLE_WRITES;
				if (data & GDC_MODE_ENABLE_WRITES) // 0x10
					logerror(" READBACK: OFF - ENABLE_WRITES ");
				else    // READBACK PLANE 00..02 - mask in bits 2+3:
					logerror(" READBACK MODE; plane = %02x ", m_gdc_mode_register & GDC_MODE_READBACK_PLANE_MASK); // unsure if PLANE is set... already?!
			}

			if (last_scroll_index != m_gdc_scroll_index)
			{
				last_scroll_index = m_gdc_scroll_index;
				if (data & GDC_MODE_READONLY_SCROLL_MAP) // 0x20
				   logerror(" SCROLL MAP READ_ONLY. Index : %02x ", m_gdc_scroll_index);
				 else
				   logerror(" SCROLL MAP IS WRITABLE. Index : %02x ", m_gdc_scroll_index);
			}

			if (!(data & GDC_MODE_ENABLE_VSYNC_IRQ)) // 0x40
				lower_8088_irq(IRQ_GRF_INTR_L); // also clears the interrupt

			// case 0x80 :  If this bit is a 1 red and blue outputs are enabled. If this bit is a 0 red and blue outputs are disabled (page 20 of AA-AE36A)
			break;
		} // GDC_SELECT_MODE_REGISTER

		logerror("\n* UNIMPLEMENTED CASE. MODE = %02x / m_gdc_indirect_register = %02x\n",m_gdc_mode_register, m_gdc_indirect_register);
		break;

	case 2:
		//  52h   Data written to this port is loaded into the Write Buffer
		//        While the CPU accesses the Write Buffer as sixteen 8-bit bytes,
		//        the GDC accesses the buffer as eight 16-bit words.
		//        A 16-bit Write Mask gives the GDC control over individual bits of a word.
		// --------------------  WRITE BUFFER USED IN WORD MODE ONLY !
		// "OUTPUT WRITE BUFFER IS THE INVERSE OF THE INPUT" (quote from 4-3 of the PDF)
		//  BITSWAP SEEMS NECESSARY (see digits in DOODLE)... !
		m_gdc_write_buffer[m_gdc_write_buffer_index++] = ~bitswap<8>(data, 0, 1, 2, 3, 4, 5, 6, 7);
		m_gdc_write_buffer_index &= 0xf; // write up to 16 bytes to port 52h.
		break;

	case 3: //  53h   Indirect Register; address selection for indirect addressing. See 51h.
		m_gdc_indirect_register = data ^ 0xff;

		// Index to WRITE_BUFFER is reset via dummy write to port 51h (not here!).

		if (m_gdc_indirect_register & GDC_SELECT_COLOR_MAP) // 0x20
			m_gdc_color_map_index = 0;                      // (also clears the index counter)
		// NEXT: 32 BYTE COLOR MAP, LOADED TO $51

		//if (m_gdc_indirect_register & GDC_SELECT_MODE_REGISTER) // 0x40
		//      logerror(" *** SELECT MODE REGISTER");

		if (m_gdc_indirect_register & GDC_SELECT_SCROLL_MAP) // 0x80
		{
			if (last_scroll_index != m_gdc_scroll_index)
			{
				last_scroll_index =  m_gdc_scroll_index;
				logerror(" *** SCROLL INDEX COUNTER RESET, old value = %d", m_gdc_scroll_index);
			}
			m_gdc_scroll_index = 0;                         // (also clears the index counter)
		}  // NEXT: LOAD 256 BYTE SCROLL MAP INTO $51
		break;

	// --------- WRITE MASK (2 x 8 = 16 bits) USED IN WORD MODE ONLY !
	// There is no specific order for the WRITE_MASK (according to txt/code samples in DEC's PDF).
	// NOTE: LOW <-> HI JUXTAPOSITION!
	case 4: // 54h   Write Mask LOW
		m_gdc_write_mask = ( bitswap<8>(data, 0, 1, 2, 3, 4, 5, 6, 7) << 8 )  | ( m_gdc_write_mask & 0x00FF );
		break;
	case 5: // 55h   Write Mask HIGH
		m_gdc_write_mask = ( m_gdc_write_mask & 0xFF00 ) | bitswap<8>(data, 0, 1, 2, 3, 4, 5, 6, 7);
		break;

	case 6:
	case 7:
		m_hgdc->write(offset & 0x01, data);
		break;
	} // switch

}


/* F4 Character Displayer */
static const gfx_layout rainbow_charlayout =
{
	8, 10,          /* 8 x 16 characters */
	256,            /* 256 characters */
	1,              /* 1 bits per pixel */
	{ 0 },          /* no bitplanes */
			/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 15 * 8, 0 * 8, 1 * 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8, 8 * 8 },
	8 * 16      /* every char takes 16 bytes */
};

static GFXDECODE_START(gfx_rainbow)
	GFXDECODE_ENTRY("chargen", 0x0000, rainbow_charlayout, 0, 1)
GFXDECODE_END

// Allocate 512 K (4 x 64 K x 16 bit) of memory (GDC):
void rainbow_base_state::upd7220_map(address_map &map)
{
	map(0x00000, 0x3ffff).rw(FUNC(rainbow_base_state::vram_r), FUNC(rainbow_base_state::vram_w)).share("vram");
}

void rainbow_base_state::rainbow_base(machine_config &config)
{
	config.set_default_layout(layout_rainbow);

	/* basic machine hardware */
	I8088(config, m_i8088, 24.0734_MHz_XTAL / 5); // approximately 4.815 MHz
	m_i8088->set_irq_acknowledge_callback(FUNC(rainbow_base_state::irq_callback));

	Z80(config, m_z80, 24.0734_MHz_XTAL / 6);
	m_z80->set_addrmap(AS_PROGRAM, &rainbow_base_state::rainbowz80_mem);
	m_z80->set_addrmap(AS_IO, &rainbow_base_state::rainbowz80_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(24.0734_MHz_XTAL / 6, 442, 0, 400, 264, 0, 240); // ~NTSC compatible video timing (?)
	screen.set_screen_update(FUNC(rainbow_base_state::screen_update_rainbow));
	screen.set_palette("vt100_video:palette");
	GFXDECODE(config, "gfxdecode", "vt100_video:palette", gfx_rainbow);

	RAINBOW_VIDEO(config, m_crtc, 24.0734_MHz_XTAL);
	m_crtc->set_screen("screen");
	m_crtc->set_chargen("chargen");
	m_crtc->ram_rd_callback().set(FUNC(rainbow_base_state::read_video_ram_r));
	m_crtc->vert_freq_intr_wr_callback().set(FUNC(rainbow_base_state::video_interrupt));

	// *************************** COLOR GRAPHICS (OPTION) **************************************
	// While the OSC frequency is confirmed, the divider is not (refresh rate is ~60 Hz with 32).
	UPD7220(config, m_hgdc, 31188000 / 32); // Duell schematics shows a 31.188 Mhz oscillator (confirmed by RFKA).
	m_hgdc->vsync_wr_callback().set(FUNC(rainbow_base_state::GDC_vblank_irq)); // "The vsync callback line needs to be below the 7220 DEVICE_ADD line."

	m_hgdc->set_addrmap(0, &rainbow_base_state::upd7220_map);
	m_hgdc->set_display_pixels(FUNC(rainbow_base_state::hgdc_display_pixels));
	m_hgdc->set_screen(m_screen2); // set_screen needs to be added after 7720 device in the machine config, not after the screen.

	PALETTE(config, m_palette2).set_entries(32);

	SCREEN(config, m_screen2, SCREEN_TYPE_RASTER);
	m_screen2->set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK | VIDEO_ALWAYS_UPDATE);

	// VR241 color monitor is specified for 20 MHz bandwidth ( 60 Hz / 15.72 kHz horizontal rate )
	// - sufficient for 800 x 240 non-interlaced at 60 Hz (non interlaced).
	//m_screen2->set_raw(31188000 / 2 , 992, 0, 800, 262, 0, 240);

	// Alternate configuration:
	m_screen2->set_raw(31188000 / 4 , 496, 0, 400, 262, 0, 240);

	m_screen2->set_screen_update("upd7220", FUNC(upd7220_device::screen_update));

	FD1793(config, m_fdc, 24.0734_MHz_XTAL / 24); // no separate 1 Mhz quartz
	FLOPPY_CONNECTOR(config, FD1793_TAG ":0", rainbow_floppies, "525qd", rainbow_base_state::floppy_formats);
	FLOPPY_CONNECTOR(config, FD1793_TAG ":1", rainbow_floppies, "525qd", rainbow_base_state::floppy_formats);
	FLOPPY_CONNECTOR(config, FD1793_TAG ":2", rainbow_floppies, "525qd", rainbow_base_state::floppy_formats);
	FLOPPY_CONNECTOR(config, FD1793_TAG ":3", rainbow_floppies, "525qd", rainbow_base_state::floppy_formats);
	//FLOPPY_CONNECTOR(config, FD1793_TAG ":2", rainbow_floppies, "525dd", rainbow_base_state::floppy_formats);
	//FLOPPY_CONNECTOR(config, FD1793_TAG ":3", rainbow_floppies, "35dd", rainbow_base_state::floppy_formats);
	SOFTWARE_LIST(config, "flop_list").set_original("rainbow");

	/// ********************************* HARD DISK CONTROLLER *****************************************
	WD2010(config, m_hdc, 5000000); // 10 Mhz quartz on controller (divided by 2 for WCLK)
	m_hdc->out_intrq_callback().set(FUNC(rainbow_base_state::bundle_irq)); // FIRST IRQ SOURCE (OR'ed with DRQ)
	m_hdc->out_bdrq_callback().set(FUNC(rainbow_base_state::hdc_bdrq));  // BUFFER DATA REQUEST

	// SIGNALS -FROM- WD CONTROLLER:
	m_hdc->out_bcs_callback().set(FUNC(rainbow_base_state::hdc_read_sector)); // Problem: OUT_BCS_CB = WRITE8 ... (!)
	m_hdc->out_bcr_callback().set(FUNC(rainbow_base_state::hdc_bcr));         // BUFFER COUNTER RESET (pulses)

	m_hdc->out_wg_callback().set(FUNC(rainbow_base_state::hdc_write_sector));   // WRITE GATE
	m_hdc->out_step_callback().set(FUNC(rainbow_base_state::hdc_step));         // STEP PULSE
	m_hdc->out_dirin_callback().set(FUNC(rainbow_base_state::hdc_direction));

	// WF + DRDY are actually wired to a routine here:
	m_hdc->in_wf_callback().set(FUNC(rainbow_base_state::hdc_write_fault));   // WRITE FAULT (fatal until next reset)
	m_hdc->in_drdy_callback().set(FUNC(rainbow_base_state::hdc_drive_ready)); // DRIVE_READY (VCC = ready)

	// Always set seek complete and track 00 signal (not super clean, but does not affect operation):
	m_hdc->in_sc_callback().set_constant(1);                             // SEEK COMPLETE (VCC = complete)
	m_hdc->in_tk000_callback().set_constant(1);                  // TRACK 00 signal (= from drive)

	HARDDISK(config, "decharddisk1");
	/// ******************************** / HARD DISK CONTROLLER ****************************************

	CORVUS_HDC(config, m_corvus_hdc, 0);
	HARDDISK(config, "harddisk1", "corvus_hdd");
	HARDDISK(config, "harddisk2", "corvus_hdd");
	HARDDISK(config, "harddisk3", "corvus_hdd");
	HARDDISK(config, "harddisk4", "corvus_hdd");

	DS1315(config, m_rtc, 0); // DS1315 (ClikClok for DEC-100 B)   * OPTIONAL *

	COM8116_003(config, m_dbrg, 24.0734_MHz_XTAL / 4); // 6.01835 MHz (nominally 6 MHz)
	m_dbrg->fr_handler().set(FUNC(rainbow_base_state::dbrg_fr_w));
	m_dbrg->ft_handler().set(FUNC(rainbow_base_state::dbrg_ft_w));

	UPD7201(config, m_mpsc, 24.0734_MHz_XTAL / 5 / 2); // 2.4073 MHz (nominally 2.5 MHz)
	m_mpsc->out_int_callback().set(FUNC(rainbow_base_state::mpsc_irq));
	m_mpsc->out_txda_callback().set(m_comm_port, FUNC(rs232_port_device::write_txd));
	m_mpsc->out_txdb_callback().set("printer", FUNC(rs232_port_device::write_txd));
	// RTS and DTR outputs are not connected

	RS232_PORT(config, m_comm_port, default_rs232_devices, nullptr);
	m_comm_port->rxd_handler().set(m_mpsc, FUNC(upd7201_device::rxa_w));
	m_comm_port->cts_handler().set(m_mpsc, FUNC(upd7201_device::ctsa_w));
	m_comm_port->dcd_handler().set(m_mpsc, FUNC(upd7201_device::dcda_w));

	rs232_port_device &printer(RS232_PORT(config, "printer", default_rs232_devices, nullptr));
	printer.rxd_handler().set(m_mpsc, FUNC(upd7201_device::rxb_w));
	printer.dcd_handler().set(m_mpsc, FUNC(upd7201_device::ctsb_w)); // actually DTR

	m_comm_port->option_add("microsoft_mouse", MSFT_HLE_SERIAL_MOUSE);
	m_comm_port->option_add("logitech_mouse", LOGITECH_HLE_SERIAL_MOUSE);
	m_comm_port->option_add("msystems_mouse", MSYSTEMS_HLE_SERIAL_MOUSE);
	m_comm_port->set_default_option("logitech_mouse");

	printer.set_default_option("printer");

	I8251(config, m_kbd8251, 24.0734_MHz_XTAL / 5 / 2);
	m_kbd8251->txd_handler().set(FUNC(rainbow_base_state::kbd_tx));
	m_kbd8251->rxrdy_handler().set(FUNC(rainbow_base_state::kbd_rxready_w));
	m_kbd8251->txrdy_handler().set(FUNC(rainbow_base_state::kbd_txready_w));

	LK201(config, m_lk201, 0);
	m_lk201->tx_handler().set(m_kbd8251, FUNC(i8251_device::write_rxd));

	ripple_counter_device &prtbrg(RIPPLE_COUNTER(config, "prtbrg", 24.0734_MHz_XTAL / 6 / 13)); // 74LS393 at E17 (both halves)
	// divided clock should ideally be 307.2 kHz, but is actually approximately 308.6333 kHz
	prtbrg.set_stages(8);
	prtbrg.count_out_cb().set(FUNC(rainbow_base_state::bitrate_counter_w));

	TIMER(config, "motor").configure_periodic(FUNC(rainbow_base_state::hd_motor_tick), attotime::from_hz(60));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

void rainbow_modela_state::rainbow_modela(machine_config &config)
{
	rainbow_base(config);
	m_i8088->set_addrmap(AS_PROGRAM, &rainbow_modela_state::rainbow8088_map);
	m_i8088->set_addrmap(AS_IO, &rainbow_modela_state::rainbow8088_io);
	RAM(config, m_ram).set_default_size("64K").set_extra_options("64K,128K,192K,256K,320K,384K,448K,512K,576K,640K,704K,768K");
	m_kbd8251->dtr_handler().set(FUNC(rainbow_modela_state::irq_hi_w));
}

void rainbow_modelb_state::rainbow_modelb(machine_config &config)
{
	rainbow_base(config);
	m_i8088->set_addrmap(AS_PROGRAM, &rainbow_modelb_state::rainbow8088_map);
	m_i8088->set_addrmap(AS_IO, &rainbow_modelb_state::rainbow8088_io);
	RAM(config, m_ram).set_default_size("128K").set_extra_options("128K,192K,256K,320K,384K,448K,512K,576K,640K,704K,768K,832K,896K");
	m_kbd8251->dtr_handler().set(FUNC(rainbow_modelb_state::irq_hi_w));
}

//----------------------------------------------------------------------------------------
// 'Rainbow 100-A' (system module 70-19974-00, PSU H7842-A)
// - first generation hardware (introduced May '82) with ROM 04.03.11
// - inability to boot from hard disc (mind the inadequate PSU)
//----------------------------------------------------------------------------------------
// AVAILABLE RAM: 64 K on board (versus 128 K on model 'B').

// Two compatible memory expansions were sold by DEC:
// (PCIXX-AA) : 64 K (usable on either Rainbow 100-A or 100-B) *
// (PCIXX-AB) : 192 K ( " )  *
// Totals to 256 K on a 100-A, while the RAM limit appears to be 832 K.

// * DEC changed the way signals are handled on J6 (memory connector) later:
//  "Whether a PC100-A or PC100-B memory module is installed on the PC100-B system module
//   affects the functions the signals on 5 pins (29, 30, 32, 43, and 47) of the J6 connector
//   will perform." (from 'EK-RB100_TM_001 Addendum for PC100-A_PC100-B Dec.84' page 120).
//----------------------------------------------------------------------------------------
// KNOWN DIFFERENCES TO 100-B:
// - cannot control bit 7 of IRQ vector (prevents DOS > 2.01 from booting on unmodified hardware)
// - 4 color palette with graphics option (instead of 16 colors on later models)
// - smaller ROMs (3 x 2764) with fewer routines (no documented way to beep...)
// - socketed NVRAM chip: X2212D 8238AES
ROM_START(rainbow100a)
	ROM_REGION(0x100000, "maincpu", 0)

	ROM_DEFAULT_BIOS("040311a")
	// printer and comm. port error messages, keyboard works, machine boots
	ROM_SYSTEM_BIOS(0, "040311a", "Version 04.03.11A")
	ROMX_LOAD("23-176e4-00.e89", 0xfa000, 0x2000, CRC(405e9619) SHA1(86604dccea84b46e05d705abeda25b12f7cc8c59), ROM_BIOS(0)) // ROM (FA000-FBFFF) (E89) 8 K
	ROMX_LOAD("23-177e4-00.e90", 0xfc000, 0x2000, CRC(1ec72a66) SHA1(ed19944ae711e97d6bec34c885be04c4c3c95852), ROM_BIOS(0)) // ROM (FC000-FDFFF) (E90) 8 K
	ROM_FILL(0xfa26d, 1, 0x00) // [0xFA000 + 0x026d] disable CRC check   [100-A ROM]
	ROM_FILL(0xfadea, 1, 0x00) // [0xFA000 + 0x0dea] Floppy workaround: in case of Z80 RESPONSE FAILURE ($80 bit set in AL), don't block floppy access

	// printer, comm. and a third error message, no keyboard, doesn't boot
	ROM_SYSTEM_BIOS(1, "010111a", "Version 01.01.11A")
	ROMX_LOAD("23-090e4-0_11-24.bin", 0xfa000, 0x2000, CRC(08d83c93) SHA1(ee26c3162071ccc3f7dce1c2afb9933329a287de), ROM_BIOS(1)) // ROM (FA000-FBFFF) (E89) 8 K
	ROMX_LOAD("23-091e4-0_11-24.bin", 0xfc000, 0x2000, CRC(387c50f2) SHA1(598f04ccbdf0e34826785992a56891d6674ac47e), ROM_BIOS(1)) // ROM (FC000-FDFFF) (E90) 8 K


	// SOCKETED LANGUAGE ROM (E91) with 1 single localization per ROM -
	ROM_LOAD("23-092e4-00.e91", 0xfe000, 0x2000, CRC(c269175a) SHA1(e82cf69b811f1e376621277f81db28e299fe06f0))  // ROM (FE000-FFFFF) (E91) 8 K - English (?)
	// See also MP-01491-00 - PC100A FIELD MAINTENANCE SET. Appendix A of EK-RB100 Rainbow
	// Technical Manual Addendum f.100A and 100B (Dec.84) lists 15 localizations / part numbers

	ROM_REGION(0x1000, "chargen", 0) // [E98] 2732 (4 K) EPROM
	ROM_LOAD("23-020e3-00.e98", 0x0000, 0x1000, CRC(b5ee2824) SHA1(8e940e32f39ec5c51cae0351ddd59ab06416d5c6))

	// Z80 ARBITRATION PROM
	ROM_REGION(0x100, "prom", 0)
	ROM_LOAD("23-090b1.mmi6308-ij.e11", 0x0000, 0x0100, CRC(cac3a7e3) SHA1(2d0468cda36fa287f705364c56dbf62f548d2e4c) ) // MMI 6308-IJ; Silkscreen stamp: "LM8413 // 090B1"; 256x8 Open Collector prom @E11, same prom is @E13 on 100-B
ROM_END


//----------------------------------------------------------------------------------------
// ROM definition for 100-B (system module 70-19974-02, PSU H7842-D)
// Built until ~ May 1986 (from MP-01491-00)
// - 32 K ROM (version 5.03)
// - 128 K base and 896 K max. mem.
ROM_START(rainbow)
	ROM_REGION(0x100000, "maincpu", 0)

	// Note that the 'Field Maintenance Print Set 1984' also lists alternate revision 'A1' with
	//              23-063e3-00 (for chargen) and '23-074e5-00' / '23-073e5-00' for E5-01 / E5-02.

	// Part numbers 22E5, 20E5 and 37E3 verified to match revision "B" (FCC ID : A0994Q - PC100 - B).

	// BOOT ROM
	ROM_LOAD("23-022e5-00.bin", 0xf0000, 0x4000, CRC(9d1332b4) SHA1(736306d2a36bd44f95a39b36ebbab211cc8fea6e))
	ROM_RELOAD(0xf4000, 0x4000)
	ROM_FILL(0xf4303, 1, 0x00) // [0xf4000 + 0x0303] disable CRC check   [100-B ROM]
	ROM_FILL(0xf535e, 1, 0x00) // [0xf4000 + 0x135e] Floppy workaround: in case of Z80 RESPONSE FAILURE ($80 bit set in AL), don't block floppy access


	// LANGUAGE ROM
	ROM_LOAD("23-020e5-00.bin", 0xf8000, 0x4000, CRC(8638712f) SHA1(8269b0d95dc6efbe67d500dac3999df4838625d8)) // German, French, English
	//ROM_LOAD( "23-015e5-00.bin", 0xf8000, 0x4000, NO_DUMP) // Dutch, French, English
	//ROM_LOAD( "23-016e5-00.bin", 0xf8000, 0x4000, NO_DUMP) // Finish, Swedish, English
	//ROM_LOAD( "23-017e5-00.bin", 0xf8000, 0x4000, NO_DUMP) // Danish, Norwegian, English
	//ROM_LOAD( "23-018e5-00.bin", 0xf8000, 0x4000, NO_DUMP) // Spanish, Italian, English
	ROM_RELOAD(0xfc000, 0x4000)

	// CHARACTER GENERATOR (E3-03)
	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("23-037e3.bin", 0x0000, 0x1000, CRC(1685e452) SHA1(bc299ff1cb74afcededf1a7beb9001188fdcf02f)) // the 'invalid character' symbol and the yen symbol were changed vs 23-020e3 from 100a

	// Z80 ARBITRATION PROM
	ROM_REGION(0x100, "prom", 0)
	ROM_LOAD("23-090b1.mmi6308-ij.e13", 0x0000, 0x0100, CRC(cac3a7e3) SHA1(2d0468cda36fa287f705364c56dbf62f548d2e4c) ) // MMI 6308-IJ; Silkscreen stamp: "LM8413 // 090B1"; 256x8 Open Collector prom @E13, same prom is @E11 on 100-A
ROM_END

//----------------------------------------------------------------------------------------
// 'Rainbow 190 B' (announced March 1985) is identical to 100-B, with alternate ROM v5.05.
// According to an article in Wall Street Journal it came with a 10 MB HD and 640 K RAM.

// All programs not dependent on specific ROM addresses should work. A first glance:
// - jump tables (F4000-F40083 and FC000-FC004D) were not extended
// - absolute addresses of some internal routines have changed (affects BOOT 2.x / 3.x dual boot)

// A Readme from January 1985 mentions 'recent ROM changes for MASS 11' (a VAX word processor).
// It is *likely* that the sole differences between 5.05 and 5.03 affect terminal emulation.

ROM_START(rainbow190)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("dec190rom0.bin", 0xf0000, 0x4000, CRC(fac191d2) SHA1(4aff5b1e031d3b5eafc568b23e68235270bb34de)) //FIXME: need correct rom name
	ROM_RELOAD(0xf4000, 0x4000)
	ROM_LOAD("dec190rom1.bin", 0xf8000, 0x4000, CRC(5ce59632) SHA1(d29793f7014c57a4e7cb77bbf6e84f9113635ed2)) //FIXME: need correct rom name

	ROM_RELOAD(0xfc000, 0x4000)
	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("23-037e3.bin", 0x0000, 0x1000, CRC(1685e452) SHA1(bc299ff1cb74afcededf1a7beb9001188fdcf02f))

	// Z80 ARBITRATION PROM
	ROM_REGION(0x100, "prom", 0)
	ROM_LOAD("23-090b1.mmi6308-ij.e13", 0x0000, 0x0100, CRC(cac3a7e3) SHA1(2d0468cda36fa287f705364c56dbf62f548d2e4c) ) // MMI 6308-IJ; Silkscreen stamp: "LM8413 // 090B1"; 256x8 Open Collector prom @E13, same prom is @E11 on 100-A
ROM_END
//----------------------------------------------------------------------------------------

/* Driver */

/*   YEAR  NAME         PARENT   COMPAT  MACHINE         INPUT           STATE                 INIT        COMPANY                          FULLNAME         FLAGS */
COMP(1982, rainbow100a, rainbow, 0,      rainbow_modela, rainbow100b_in, rainbow_modela_state, empty_init, "Digital Equipment Corporation", "Rainbow 100-A", MACHINE_IS_SKELETON)
COMP(1983, rainbow,     0,       0,      rainbow_modelb, rainbow100b_in, rainbow_modelb_state, empty_init, "Digital Equipment Corporation", "Rainbow 100-B", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS)
COMP(1985, rainbow190,  rainbow, 0,      rainbow_modelb, rainbow100b_in, rainbow_modelb_state, empty_init, "Digital Equipment Corporation", "Rainbow 190-B", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_COLORS)
