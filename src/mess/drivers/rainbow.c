// license:GPL-2.0+
// copyright-holders:Miodrag Milanovic,Karl-Ludwig Deisenhofer
/***************************************************************************************************
DEC Rainbow 100

Driver-in-progress by R. Belmont and Miodrag Milanovic.
Portions (2013 - 2015) by Karl-Ludwig Deisenhofer (Floppy, ClikClok RTC, NVRAM, DIPs, hard disk).

STATE AS OF APRIL 2015
----------------------
Driver is based entirely on the DEC-100 'B' variant (DEC-190 and DEC-100 A models are treated as clones).
While this is OK for the compatible -190, it doesn't do justice to ancient '100 A' hardware.
RBCONVERT.ZIP has details on how model 'A' differs from version B.

Issues with this driver:

(1) Keyboard emulation incomplete (fatal; inhibits the system from booting with ERROR 50).

Serial ports do not work, so serial communication failure (ERROR 60) and ERROR 40 (serial
printer interface) result. Unfortunately the BIOS tests all three serial interfaces in line.

(2) while DOS 3 and UCSD systems (fort_sys, pas_sys) + diag disks boot, CPM 2.x and DOS 2.x die
in secondary boot loader with a RESTORE (seek track 0) when track 2 sector 1 should be loaded.

Writing files to floppy is next to impossible on both CPM 1.x and DOS 3 (these two OS boot
with keyboard workarounds enabled). File deletion works, so few bytes pass.

(3) heavy system interaction stalls the driver. Start one of the torture tests on the diag.disk
and see what happens (system interaction, Z80, about any test except the video / CRT tests).

(4) arbitration chip (E11; in 100-A schematics or E13 in -B) should be dumped.
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


UPGRADES WORTH EMULATING:
- SHOULD BE IMPLEMENTED AS SLOT DEVICES (for now, DIP settings affect 'system_parameter_r' only and are disabled):

* Color graphics option (uses NEC upd7220 GDC).         REFERENCE: Programmer's Reference: AA-AE36A-TV.
Either 384 x 240 x 16 or 800 x 240 x 4 colors (out of 4096). 8 ? 64 K video RAM. Pallette limited to 4 colors on 100-A.
Graphics output is independent from monochrome output.

* Extended communication option (occupies BUNDLE_OPTION 1 + 2)  REFERENCE: AA-V172A-TV + Addendum AV-Y890A-TV.
Two ports, a high-speed RS-422 half-duplex interface (port A) + lower-speed RS-423 full/half-duplex interface
with modem control (port B). A 5 Mhz. 8237 DMA controller transfers data into and out of shared memory (not: optional RAM).

Uses SHRAM, SHMA, BDL SH WR L, NONSHARED CYCLE. Implementation requires DMA and arbitration logic (dump of E11/E13 ?).
Can't be added if RD51 hard disk controller present (J4 + J5). For programming info see NEWCOM1.DOC (-> RBETECDOC.ZIP).


* ( NO DUMP YET )  PC character set. Enhances Code Blue emulation. Simple CHARACTER ROM replacement?

* ( NO DUMP YET )  TCS / Technical Character Set ('$95 from DEC, for Rainbow 100, 100B, 100+ ; separate docs available')
  Source: price list of a DEC reseller. Possibly identical to http://vt100.net/charsets/technical.html

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

// ---------------------------------------------------------------------------
// WORKAROUNDS:
// (1) FORCE LOGO: - not valid for 100-A ROM -
//#define FORCE_RAINBOW_B_LOGO

// (2) KEYBOARD_WORKAROUND : also requires FORCE...LOGO (and preliminary headers)
//#define KEYBOARD_WORKAROUND
//#define KBD_DELAY 8    // (debounce delay)
// ---------------------------------------------------------------------------


// Define standard and maximum RAM sizes (A, then B model):
//#define BOARD_RAM 0x0ffff  // 64 K base RAM  (100-A)
//#define END_OF_RAM 0xcffff // very last byte (100-A) DO NOT CHANGE.

// DEC-100-B probes until a 'flaky' area is found (BOOT ROM around F400:0E04).
// It is no longer possible to key in the RAM size from within the 100-B BIOS.
#define BOARD_RAM 0x1ffff  // 128 K base RAM (100-B)
#define END_OF_RAM 0xdffff // very last byte (100-B) DO NOT CHANGE.

// ----------------------------------------------------------------------------------------------
#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/z80/z80.h"
#include "video/vtvideo.h"

#include "machine/wd_fdc.h"
#include "formats/rx50_dsk.h"
#include "formats/pc_dsk.h" // PC Formats (TESTING)
#include "imagedev/flopdrv.h"

#include "imagedev/harddriv.h"
#include "machine/wd2010.h"

#include "machine/i8251.h"
#include "machine/clock.h"
#include "machine/dec_lk201.h"
#include "machine/nvram.h"

#include "machine/ds1315.h"

#include "rainbow.lh" // BEZEL - LAYOUT with LEDs for diag 1-7, keyboard 8-11 and floppy 20-23

#define LK201_TAG   "lk201"
#define FD1793_TAG  "fd1793x"

#define INVALID_DRIVE 255
#define MAX_FLOPPIES 4

class rainbow_state : public driver_device
{
public:
	rainbow_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),

#ifdef KEYBOARD_WORKAROUND
#include "./m_kbd1.c"
#endif


		m_inp1(*this, "W13"),
		m_inp2(*this, "W14"),
		m_inp3(*this, "W15"),
		m_inp4(*this, "W18"),
		m_inp5(*this, "HARD DISK PRESENT"), // DO NOT CHANGE ORDER (also: COMMUNICATION EXTENSION)
		m_inp6(*this, "FLOPPY CONTROLLER"), // DO NOT CHANGE ORDER
		m_inp7(*this, "GRAPHICS OPTION"),   // DO NOT CHANGE ORDER
		m_inp8(*this, "MEMORY PRESENT"),    // DO NOT CHANGE ORDER
		m_inp9(*this, "MONITOR TYPE"),
		m_inp10(*this, "J17"),
		m_inp11(*this, "CLIKCLOK"),
		m_inp12(*this, "WATCHDOG"),

		m_crtc(*this, "vt100_video"),
		m_i8088(*this, "maincpu"),
		m_z80(*this, "subcpu"),
		m_fdc(*this, FD1793_TAG),

		m_hdc(*this, "hdc"),

		m_kbd8251(*this, "kbdser"),
		m_lk201(*this, LK201_TAG),
		m_p_ram(*this, "p_ram"),

		m_p_vol_ram(*this, "vol_ram"),
		m_p_nvram(*this, "nvram"),

		m_shared(*this, "sh_ram"),

		m_rtc(*this, "rtc")
	{
	}

	DECLARE_READ8_MEMBER(read_video_ram_r);
	DECLARE_WRITE_LINE_MEMBER(clear_video_interrupt);
	//DECLARE_WRITE8_MEMBER(hd_status_10C_w); // MHFU DISABLE REGISTER (W)

	DECLARE_READ8_MEMBER(diagnostic_r);
	DECLARE_WRITE8_MEMBER(diagnostic_w);

	DECLARE_READ8_MEMBER(comm_control_r);
	DECLARE_WRITE8_MEMBER(comm_control_w);

	DECLARE_READ8_MEMBER(share_z80_r);
	DECLARE_WRITE8_MEMBER(share_z80_w);

	// 'RD51' MFM CONTROLLER (WD1010) *************************************
	DECLARE_READ8_MEMBER(hd_status_60_r); // TRI STATE DATA PORT (R/W)
	DECLARE_WRITE8_MEMBER(hd_status_60_w);

	DECLARE_READ8_MEMBER(hd_status_68_r); // EXTRA REGISTER 0x68 (R/W 8088)
	DECLARE_WRITE8_MEMBER(hd_status_68_w);

	DECLARE_READ8_MEMBER(hd_status_69_r); // EXTRA REGISTER 0x69 (R/- 8088)

	DECLARE_WRITE_LINE_MEMBER(bundle_irq);
	DECLARE_WRITE_LINE_MEMBER(hdc_bdrq);  // BUFFER DATA REQUEST (FROM WD)
	DECLARE_WRITE_LINE_MEMBER(hdc_bcr);   // BUFFER COUNTER RESET (FROM WD)

	DECLARE_WRITE_LINE_MEMBER(hdc_step);
	DECLARE_WRITE_LINE_MEMBER(hdc_direction);

	DECLARE_WRITE_LINE_MEMBER(hdc_read_sector);
	DECLARE_WRITE_LINE_MEMBER(hdc_write_sector);

	DECLARE_READ_LINE_MEMBER(hdc_drive_ready);
	DECLARE_READ_LINE_MEMBER(hdc_write_fault);

	DECLARE_READ8_MEMBER(i8088_latch_r);
	DECLARE_WRITE8_MEMBER(i8088_latch_w);
	DECLARE_READ8_MEMBER(z80_latch_r);
	DECLARE_WRITE8_MEMBER(z80_latch_w);

	DECLARE_WRITE8_MEMBER(z80_diskdiag_read_w);
	DECLARE_WRITE8_MEMBER(z80_diskdiag_write_w);

	DECLARE_READ8_MEMBER(z80_generalstat_r);

	DECLARE_READ8_MEMBER(z80_diskstatus_r);
	DECLARE_WRITE8_MEMBER(z80_diskcontrol_w);

	DECLARE_READ8_MEMBER(system_parameter_r);

	DECLARE_WRITE_LINE_MEMBER(kbd_tx);
	DECLARE_WRITE_LINE_MEMBER(kbd_rxready_w);
	DECLARE_WRITE_LINE_MEMBER(kbd_txready_w);

	DECLARE_WRITE_LINE_MEMBER(irq_hi_w);

	DECLARE_READ8_MEMBER(rtc_reset);
	DECLARE_READ8_MEMBER(rtc_enable);
	DECLARE_READ8_MEMBER(rtc_r);

	DECLARE_READ8_MEMBER(rtc_reset2);
	DECLARE_READ8_MEMBER(rtc_enable2);
	DECLARE_READ8_MEMBER(rtc_r2);

	DECLARE_READ8_MEMBER(rtc_w);


#ifdef KEYBOARD_WORKAROUND
#include "./port9x_Ax.c"
#endif
	UINT32 screen_update_rainbow(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);

	DECLARE_WRITE_LINE_MEMBER(write_keyboard_clock);
	TIMER_DEVICE_CALLBACK_MEMBER(motor_tick);

	DECLARE_FLOPPY_FORMATS(floppy_formats);
protected:
	virtual void machine_start();

private:
	enum
	{   // LOWEST PRIORITY
		// Mnemonic - - - - - -  TYPE  ADDRESS - Source
		//                      [1][0]  [1][0] <= Depends on DTR(L) output of keyboard PUSART (on Rainbow-100 B)
		IRQ_8088_MAILBOX = 0, // 27/A7  9C/29C  - [built-in] Interrupt from Z80A
		IRQ_8088_KBD,         // 26/A6  98/298  - [built-in] KEYBOARD Interrupt - 8251A
		IRQ_BDL_INTR_L,       // 25/A5  94/294  - [ext. BUNDLE OPTION] Hard disk or Extended communication IRQ (no DMA)
		// IRQ_COMM_PTR_INTR_L,  // 24/A4  90/290  - [built-in 7201] Communication/Printer interrupt
		// IRQ_DMAC_INTR_L,      // 23/A3  8C/28C  - [ext. COMM.BOARD only] - external DMA Controller (8237) interrupt
		// IRQ_GRF_INTR_L,       // 22/A2  88/288  - [ext. COLOR GRAPHICS]
		// IRQ_BDL_INTR_1L,      // 21/A1  84/284  - [ext. COMM.BOARD only]
		IRQ_8088_VBL,         // 20/A0  80/280  - [built-in DC012] - VERT INTR L (= schematics)
		IRQ_8088_NMI          // 02/02  08/08   - [external MEMORY EXTENSION] - PARITY ERROR L
	};  // HIGHEST PRIORITY


#ifdef KEYBOARD_WORKAROUND
#include "./m_kbd2.c"
#endif

	required_ioport m_inp1;
	required_ioport m_inp2;
	required_ioport m_inp3;
	required_ioport m_inp4;
	required_ioport m_inp5;
	required_ioport m_inp6;
	required_ioport m_inp7;
	required_ioport m_inp8;
	required_ioport m_inp9;
	required_ioport m_inp10;
	required_ioport m_inp11;
	required_ioport m_inp12;
	required_device<rainbow_video_device> m_crtc;
	required_device<cpu_device> m_i8088;
	required_device<cpu_device> m_z80;

	required_device<fd1793_t> m_fdc;
	optional_device<wd2010_device> m_hdc;

	required_device<i8251_device> m_kbd8251;
	required_device<lk201_device> m_lk201;
	required_shared_ptr<UINT8> m_p_ram;
	required_shared_ptr<UINT8> m_p_vol_ram;
	required_shared_ptr<UINT8> m_p_nvram;
	required_shared_ptr<UINT8> m_shared;

	optional_device<ds1315_device> m_rtc;

	void raise_8088_irq(int ref);
	void lower_8088_irq(int ref);

	void update_8088_irqs();

	void update_bundle_irq(); // RD51 or COMM.OPTION!
	int do_write_sector();
	void hdc_buffer_counter_reset();
	void hdc_reset();

	hard_disk_file *rainbow_hdc_file(int ref);

	bool m_SCREEN_BLANK;

	int INT88, INTZ80;

	bool m_zflip;                   // Z80 alternate memory map with A15 inverted
	bool m_z80_halted;
	int  m_z80_diskcontrol;         // retains values needed for status register

	bool m_kbd_tx_ready, m_kbd_rx_ready;
	int m_KBD;

	int m_beep_counter;
	int MOTOR_DISABLE_counter;

	int COLD_BOOT;
	UINT8 m_diagnostic;

	UINT8 m_z80_private[0x800];     // Z80 private 2K
	UINT8 m_z80_mailbox, m_8088_mailbox;

	void update_kbd_irq();
	virtual void machine_reset();

	int m_unit;
	floppy_image_device *m_floppy;

	int m_irq_high;
	UINT32 m_irq_mask;

	int m_bdl_irq;
	int m_hdc_buf_offset;

	bool m_hdc_index_latch;
	bool m_hdc_step_latch;
	int m_hdc_direction;
	bool m_hdc_track0;
	bool m_hdc_write_gate;

	bool m_hdc_drive_ready;
	bool m_hdc_write_fault;

	UINT8 m_hdc_buffer[2048];
};

// initially only RX50IMG_FORMAT,  FLOPPY_TD0_FORMAT, (IMD + PC : TESTING !!)
FLOPPY_FORMATS_MEMBER(rainbow_state::floppy_formats)
FLOPPY_RX50IMG_FORMAT,
FLOPPY_TD0_FORMAT,
FLOPPY_IMD_FORMAT,
FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

// initially only : SLOT_INTERFACE("525qd", FLOPPY_525_SSQD)
static SLOT_INTERFACE_START(rainbow_floppies)
SLOT_INTERFACE("525qd0", FLOPPY_525_SSQD)
SLOT_INTERFACE("525qd1", FLOPPY_525_SSQD)
SLOT_INTERFACE("525qd2", FLOPPY_525_SSQD)
SLOT_INTERFACE("525qd3", FLOPPY_525_SSQD)
SLOT_INTERFACE_END
// testing: SLOT_INTERFACE("35ssdd", FLOPPY_35_SSDD)

void rainbow_state::machine_start()
{
	MOTOR_DISABLE_counter = 2; // soon resets drv.LEDs
	COLD_BOOT = 1;

	m_SCREEN_BLANK = false;

	save_item(NAME(m_z80_private));
	save_item(NAME(m_z80_mailbox));
	save_item(NAME(m_8088_mailbox));
	save_item(NAME(m_zflip));
	save_item(NAME(m_kbd_tx_ready));
	save_item(NAME(m_kbd_rx_ready));
	save_item(NAME(m_irq_high));
	save_item(NAME(m_irq_mask));

#ifdef FORCE_RAINBOW_B_LOGO
	UINT8 *rom = memregion("maincpu")->base();

	rom[0xf4000 + 0x364a] = 2 + 8;  // 2 :set ; 4 : reset, 8 : set for 0xf4363 ( 0363 WAIT_FOR_BIT3__loc_35E )

	rom[0xf4000 + 0x0363] = 0x90;
	rom[0xf4000 + 0x0364] = 0x90;

	// If bit 2 = 1 (Efff9), then a keyboard powerup is necessary (=> will lock up in current state)
	rom[0xf4000 + 0x3638] = 0x80;  // OR instead of TEST
	rom[0xf4000 + 0x3639] = 0x0f;  // OR instead of TEST
	rom[0xf4000 + 0x363a] = 0x08;  // 04 => 08

	rom[0xf4000 + 0x363b] = 0xeb;  // COND => JMPS

	if (rom[0xf4174] == 0x75)
	{
		rom[0xf4174] = 0xeb; // jmps  RAINBOW100_LOGO__loc_33D
		rom[0xf4175] = 0x08;
	}

	if (rom[0xf4000 + 0x3ffc] == 0x31) // 100-B
		rom[0xf4384] = 0xeb; // JMPS  =>  BOOT80

	if (rom[0xf4000 + 0x3ffc] == 0x35) // v5.05
		rom[0xf437b] = 0xeb;

	//TEST-DEBUG: always reset NVM to defaults (!)
	//rom[0x3d6c] = 0x90;
	//rom[0x3d6d] = 0x90;
#endif

#ifdef KEYBOARD_WORKAROUND
#include "./rainbow_keyboard0.c"
#endif
}

static ADDRESS_MAP_START(rainbow8088_map, AS_PROGRAM, 8, rainbow_state)
ADDRESS_MAP_UNMAP_HIGH
AM_RANGE(0x00000, 0x0ffff) AM_RAM AM_SHARE("sh_ram")
AM_RANGE(0x10000, END_OF_RAM) AM_RAM

// There is a 2212 (256 x 4 bit) NVRAM from 0xed000 to 0xed0ff (*)
// shadowed at $ec000 - $ecfff and from $ed100 - $edfff.

// (*) ED000 - ED0FF is the area the DEC-100-B BIOS accesses and checks

//  - Specs say that the CPU has direct access to volatile RAM only.
//    So NVRAM is hidden and loads & saves are triggered within the
//    'diagnostic_w' handler (similar to real hardware).

//  - Address bits 8-12 are ignored (-> AM_MIRROR).
AM_RANGE(0xed000, 0xed0ff) AM_RAM AM_SHARE("vol_ram") AM_MIRROR(0x1f00)
AM_RANGE(0xed100, 0xed1ff) AM_RAM AM_SHARE("nvram")

AM_RANGE(0xee000, 0xeffff) AM_RAM AM_SHARE("p_ram")
AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(rainbow8088_io, AS_IO, 8, rainbow_state)
ADDRESS_MAP_UNMAP_HIGH
//ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_GLOBAL_MASK(0x1ff)
AM_RANGE(0x00, 0x00) AM_READWRITE(i8088_latch_r, i8088_latch_w)

// 0x02 Communication status / control register (8088)
AM_RANGE(0x02, 0x02) AM_READWRITE(comm_control_r, comm_control_w)

AM_RANGE(0x04, 0x04) AM_DEVWRITE("vt100_video", rainbow_video_device, dc011_w)

// TODO: unmapped [06] : Communication bit rates (see page 21 of PC 100 SPEC)

AM_RANGE(0x08, 0x08) AM_READ(system_parameter_r)

AM_RANGE(0x0a, 0x0a) AM_READWRITE(diagnostic_r, diagnostic_w)

AM_RANGE(0x0c, 0x0c) AM_DEVWRITE("vt100_video", rainbow_video_device, dc012_w)

// TODO: unmapped [0e] : PRINTER BIT RATE REGISTER (WO)

AM_RANGE(0x10, 0x10) AM_DEVREADWRITE("kbdser", i8251_device, data_r, data_w)
AM_RANGE(0x11, 0x11) AM_DEVREADWRITE("kbdser", i8251_device, status_r, control_w)

// ===========================================================
// There are 4 select lines for Option Select 1 to 4
// Option Select ------------------- Bundle Option Present
// 1 2 3 4:                          BDL PRES (L):
// X X o o Communication Option----- X
// o X o o RD51 hard disk controller X --------- (X = SELECT)
// ===========================================================
// 0x20 -> 0x2f ***** EXTENDED COMM. OPTION / Option Select 1.
// See boot rom @1EA6: 0x27 (<- RESET EXTENDED COMM OPTION  )
// ===========================================================
// 0x30 -> 0x3f ***** Option Select 3
// ===========================================================
// 0x40  COMMUNICATIONS DATA REGISTER (MPSC)
// 0x41  PRINTER DATA REGISTER (MPSC)
// 0x42  COMMUNICATIONS CONTROL / STATUS REGISTER (MPSC)
// 0x43  PRINTER CONTROL / STATUS REGISTER (MPSC)
// ===========================================================
// 0x50 - 0x57 ***** COLOR GRAPHICS OPTION:
//  50h   Graphics option software reset.  Any write to this
//        port also resynchronizes the read/modify/write memory
//        cycles of the Graphics Option to those of the GDC (*)
//                                       * see boot ROM @1EB4/8
//  51h   Data written to this port is loaded into the area
//        selected by the previous write to port 53h.

//  52h   Data written to this port is loaded into the Write Buffer.

//  53h   Data written to this port provides address selection
//        for indirect addressing (see Indirect Register).

//  54h   Data written to this port is loaded into the low-order
//        byte of the Write Mask.

//  55h   Data written to this port is loaded into the high-order
//        byte of the Write Mask.

//  56h   Data written to this port is loaded into the GDC's FIFO
//        Buffer and flagged as a parameter.
// ===========================================================
// 0x60 -> 0x6f ***** EXTENDED COMM. OPTION / Option Select 2.
// ===========================================================
// 0x60 -> 0x6f ***** RD51 HD. CONTROLLER   / Option Select 2.
AM_RANGE(0x60, 0x67) AM_DEVREADWRITE("hdc", wd2010_device, read, write)
AM_RANGE(0x68, 0x68) AM_READWRITE(hd_status_68_r, hd_status_68_w)
AM_RANGE(0x69, 0x69) AM_READ(hd_status_69_r)
// ===========================================================
// THE RD51 CONTROLLER:
// - WD1010AL - 00 (WDC '83)
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
// DEC RD52(32 Mbyte); 512 cyl. 8 heads -- Q540 [!]
// DEC RD32(40 Mbyte); 820 cyl. 6 heads -- ST251 [!]
// DEC RD53(67 Mbyte); 1024 cyl.8 heads -- 1325 [!]
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
// 0x10c
AM_RANGE(0x10c, 0x10c) AM_DEVWRITE("vt100_video", rainbow_video_device, dc012_w)


#ifdef KEYBOARD_WORKAROUND
#include "./am_range_9x_Ax.c"
#endif

ADDRESS_MAP_END

static ADDRESS_MAP_START(rainbowz80_mem, AS_PROGRAM, 8, rainbow_state)
ADDRESS_MAP_UNMAP_HIGH
AM_RANGE(0x0000, 0xffff) AM_READWRITE(share_z80_r, share_z80_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(rainbowz80_io, AS_IO, 8, rainbow_state)
ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_GLOBAL_MASK(0xff)
AM_RANGE(0x00, 0x00) AM_READWRITE(z80_latch_r, z80_latch_w)
AM_RANGE(0x20, 0x20) AM_READWRITE(z80_generalstat_r, z80_diskdiag_read_w) // read to port 0x20 used by MS-DOS 2.x diskette loader.
AM_RANGE(0x21, 0x21) AM_READWRITE(z80_generalstat_r, z80_diskdiag_write_w)
AM_RANGE(0x40, 0x40) AM_READWRITE(z80_diskstatus_r, z80_diskcontrol_w)
AM_RANGE(0x60, 0x63) AM_DEVREADWRITE(FD1793_TAG, fd1793_t, read, write)

// Z80 I/O shadow area > $80
AM_RANGE(0x80, 0x80) AM_READWRITE(z80_latch_r, z80_latch_w)
AM_RANGE(0xA0, 0xA0) AM_READWRITE(z80_generalstat_r, z80_diskdiag_read_w) // read to port 0x20 used by MS-DOS 2.x diskette loader.
AM_RANGE(0xA1, 0xA1) AM_READWRITE(z80_generalstat_r, z80_diskdiag_write_w)
AM_RANGE(0xC0, 0xC0) AM_READWRITE(z80_diskstatus_r, z80_diskcontrol_w)
AM_RANGE(0xE0, 0xE3) AM_DEVREADWRITE(FD1793_TAG, fd1793_t, read, write)
ADDRESS_MAP_END

/* Input ports */

/* DIP switches */
static INPUT_PORTS_START(rainbow100b_in)

#ifdef KEYBOARD_WORKAROUND
#include "./rainbow_ipt.c"
#endif
PORT_START("MONITOR TYPE")
PORT_DIPNAME(0x03, 0x03, "MONOCHROME MONITOR")
PORT_DIPSETTING(0x01, "WHITE")
PORT_DIPSETTING(0x02, "GREEN")
PORT_DIPSETTING(0x03, "AMBER")

// MEMORY, FLOPPY, BUNDLE, GRAPHICS affect 'system_parameter_r':
PORT_START("MEMORY PRESENT")
PORT_DIPNAME(0xF0000, 0x20000, "MEMORY PRESENT")
PORT_DIPSETTING(0x10000, "64  K (MINIMUM ON 100-A)") // see BOARD_RAM
PORT_DIPSETTING(0x20000, "128 K (MINIMUM ON 100-B)")
PORT_DIPSETTING(0x30000, "192 K (w. MEMORY OPTION)")
PORT_DIPSETTING(0x40000, "256 K (w. MEMORY OPTION)")
PORT_DIPSETTING(0x50000, "320 K (100-B MEMORY OPTION)")
PORT_DIPSETTING(0x60000, "384 K (100-B MEMORY OPTION)")
PORT_DIPSETTING(0x70000, "448 K (100-B MEMORY OPTION)")
PORT_DIPSETTING(0x80000, "512 K (100-B MEMORY OPTION)")
PORT_DIPSETTING(0x90000, "576 K (100-B MEMORY OPTION)")
PORT_DIPSETTING(0xA0000, "640 K (100-B MEMORY OPTION)")
PORT_DIPSETTING(0xB0000, "704 K (100-B MEMORY OPTION)")
PORT_DIPSETTING(0xC0000, "768 K (100-B MEMORY OPTION)")
PORT_DIPSETTING(0xD0000, "832 K (100-B MEMORY OPTION)") // see END_OF_RAM
PORT_DIPSETTING(0xE0000, "896 K (100-B MAX.   MEMORY)")

PORT_START("FLOPPY CONTROLLER") // floppy controller is not optional
PORT_DIPNAME(0x01, 0x01, "FLOPPY CONTROLLER") PORT_TOGGLE
PORT_DIPSETTING(0x01, DEF_STR(On))

// EXT.COMM.card -or- RD51 HD. controller (marketed later).
PORT_START("HARD DISK PRESENT") // BUNDLE_OPTION
PORT_DIPNAME(0x01, 0x00, "HARD DISK PRESENT") PORT_TOGGLE
PORT_DIPSETTING(0x00, DEF_STR(Off))
PORT_DIPSETTING(0x01, DEF_STR(On))

PORT_START("CLIKCLOK") // DS1315 RTC
PORT_DIPNAME(0x01, 0x00, "REAL TIME CLOCK (CLIKCLOK)") PORT_TOGGLE
PORT_DIPSETTING(0x00, DEF_STR(Off))
PORT_DIPSETTING(0x01, DEF_STR(On))

PORT_START("GRAPHICS OPTION") // NEC COLOR GRAPHICS not implemented yet
PORT_DIPNAME(0x00, 0x00, "GRAPHICS OPTION") PORT_TOGGLE
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
PORT_DIPNAME(0x01, 0x01, "WATCHDOG ENABLED (MHFU)") PORT_TOGGLE
PORT_DIPSETTING(0x00, DEF_STR(Off))
PORT_DIPSETTING(0x01, DEF_STR(On))

INPUT_PORTS_END

void rainbow_state::machine_reset()
{
	/* configure RAM */
	address_space &program = machine().device<cpu_device>("maincpu")->space(AS_PROGRAM);
	if (m_inp8->read() < END_OF_RAM)
	{
		program.unmap_readwrite(m_inp8->read(), END_OF_RAM);
	}
	// BIOS can't handle soft resets (would trigger ERROR 16).
	// As a fallback, execute a hard reboot!
	if (COLD_BOOT == 2)
	{       // FIXME: ask for confirmation (via UI ?)
			device().machine().schedule_hard_reset();
	}

	/* *****************************************************************************************************************
	    Suitable Solutions ClikClok (one of the more compatible battery backed real time clocks)

	    DESCRIPTION: plugs into NVRAM chip socket on a 100-A and into one of the (EP)ROM sockets on the 100-B
	    ............ (there is a socket on the ClikClok for the NVRAM / EPROM chip).

	    DS1315 phantom clock. No address space needed (-> IRQs must be disabled to block ROM accesses during reads).

	    DRIVERS: 'rbclik.zip' DOS and CP/M binaries plus source from DEC employee; Reads & displays times. Y2K READY.
	    + 'newclk.arc' (Suitable Solutions; sets time and date; uses FE000 and up). 2 digit year here.

	    TODO: obtain hardware / check address decoders. Access logic here is derived from Vincent Esser's source.
	*****************************************************************************************************************/

	// * Reset RTC to a defined state *
	#define RTC_RESET 0xFC104 // read $FC104 or mirror $FE104
	program.install_read_handler(RTC_RESET, RTC_RESET, 0, 0, read8_delegate(FUNC(rainbow_state::rtc_reset), this));
	program.install_read_handler(RTC_RESET + 0x2000, RTC_RESET + 0x2000, 0, 0, read8_delegate(FUNC(rainbow_state::rtc_reset2), this));

	// A magic pattern enables reads or writes (-> RTC_WRITE_DATA_0 / RTC_WRITE_DATA_1)
	// 64 bits read from two alternating addresses (see DS1315.C)
	#define RTC_PATTERN_0 0xFC100 // MIRROR: FE100
	#define RTC_PATTERN_1 0xFC101 // MIRROR: FE101
	program.install_read_handler(RTC_PATTERN_0, RTC_PATTERN_1, 0, 0, read8_delegate(FUNC(rainbow_state::rtc_enable), this));
	program.install_read_handler(RTC_PATTERN_0 + 0x2000, RTC_PATTERN_1 + 0x2000, 0, 0, read8_delegate(FUNC(rainbow_state::rtc_enable2), this));

	// * Read actual time/date from ClikClok *
	#define RTC_READ_DATA 0xFC004 // Single byte - delivers one bit (if RTC enabled). MIRROR: FE004
	program.install_read_handler(RTC_READ_DATA, RTC_READ_DATA, 0, 0, read8_delegate(FUNC(rainbow_state::rtc_r), this));
	program.install_read_handler(RTC_READ_DATA + 0x2000, RTC_READ_DATA + 0x2000, 0, 0, read8_delegate(FUNC(rainbow_state::rtc_r2), this));

	// * Secretly transmit data to RTC (set time / date) *  Works only if magic pattern enabled RTC. Look ma, no writes!
	#define RTC_WRITE_DATA_0 0xFE000
	#define RTC_WRITE_DATA_1 0xFE001
	program.install_read_handler(RTC_WRITE_DATA_0, RTC_WRITE_DATA_1, 0, 0, read8_delegate(FUNC(rainbow_state::rtc_w), this));

	m_rtc->chip_reset();
	// *********************************** / DS1315 'PHANTOM CLOCK' IMPLEMENTATION FOR 'DEC-100-B' ***************************************


	if (COLD_BOOT == 1)
	{
		COLD_BOOT = 2;
		m_crtc->MHFU(-100); // reset MHFU counter
	}

	//  *********** HARD DISK CONTROLLER...
	if (m_inp5->read() == 0x01) // ...PRESENT?
	{
		// Install 8088 read / write handler
		address_space &io = machine().device<cpu_device>("maincpu")->space(AS_IO);
		io.unmap_readwrite(0x60, 0x60);
		io.install_read_handler(0x60, 0x60, read8_delegate(FUNC(rainbow_state::hd_status_60_r), this));
		io.install_write_handler(0x60, 0x60, write8_delegate(FUNC(rainbow_state::hd_status_60_w), this));

		hdc_reset();
		m_hdc_drive_ready = true;
		m_hdc_write_fault = false;

		hard_disk_file *local_hard_disk;
		local_hard_disk = rainbow_hdc_file(0); // one hard disk for now.

		output_set_value("led1", 0);
		if (local_hard_disk)
		{
			hard_disk_info *info;
			if ( (info = hard_disk_get_info(local_hard_disk)) )
			{
				output_set_value("led1", 1);

				UINT32 max_sector = (info->cylinders) * (info->heads) * (info->sectors);
				printf("\n%u MB HARD DISK: HEADS (1..8 OK) = %d / CYL. (151..1024 OK) = %d / SPT. (16 OK) = %d / SECTOR_BYTES (128..1024 OK) = %d\n", max_sector * 512 / 1000000,
					info->heads, info->cylinders, info->sectors, info->sectorbytes);
			}
		}
	}

	//  *********** FLOPPY DISK CONTROLLER
	m_unit = INVALID_DRIVE;
	m_fdc->reset();
	m_fdc->set_floppy(NULL);
	m_fdc->dden_w(0);

	m_z80->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_z80_halted = true;
	m_zflip = true;

	INTZ80 = false;
	INT88 = false;

	m_kbd_tx_ready = m_kbd_rx_ready = false;
	m_kbd8251->write_cts(1);
	m_KBD = 0;

	m_irq_high = 0;

	// RESET ALL LEDs
	output_set_value("led1", 1);
	output_set_value("led2", 1);
	output_set_value("led3", 1);
	output_set_value("led4", 1);
	output_set_value("led5", 1);
	output_set_value("led6", 1);
	output_set_value("led7", 1);

	// GREEN KEYBOARD LEDs (1 = on, 0 = off):
	output_set_value("led_wait", 0);    // led8
	output_set_value("led_compose", 0); // led9
	output_set_value("led_lock", 0);    // led10
	output_set_value("led_hold", 0);    // led11

	m_irq_mask = 0;
}

#ifdef KEYBOARD_WORKAROUND
#include "./rainbow_keyboard2.c"
#endif

UINT32 rainbow_state::screen_update_rainbow(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
/*
    // Suppress display when accessing floppy (switch to 'smooth scroll' when working with DOS, please)!
    if (MOTOR_DISABLE_counter) // IF motor running...
    {
        if (m_p_vol_ram[0x84] == 0x00) // IF jump scroll
            return 0;
    }
*/
	m_crtc->palette_select(m_inp9->read());

	if (m_SCREEN_BLANK)
		m_crtc->video_blanking(bitmap, cliprect);
	else
		m_crtc->video_update(bitmap, cliprect);
	return 0;
}

// Interrupt handling and arbitration.  See 3.1.3.8 OF PC-100 spec.
void rainbow_state::update_8088_irqs()
{
	static const int vectors[] = { 0x27, 0x26, 0x25, 0x20 };

	if (m_irq_mask != 0)
	{
		for (int i = 0; i < IRQ_8088_NMI; i++)
		{
			if (m_irq_mask & (1 << i))
			{
				m_i8088->set_input_line_and_vector(INPUT_LINE_INT0, ASSERT_LINE, vectors[i] | m_irq_high);
				break;
			}
		}
	}
	else
	{
		m_i8088->set_input_line(INPUT_LINE_INT0, CLEAR_LINE);
	}
}

void rainbow_state::raise_8088_irq(int ref)
{
	m_irq_mask |= (1 << ref);
	update_8088_irqs();
}

void rainbow_state::lower_8088_irq(int ref)
{
	m_irq_mask &= ~(1 << ref);
	update_8088_irqs();
}

// Only Z80 * private SRAM * is wait state free
// (= fast enough to allow proper I/O to the floppy)

// Shared memory is contended by refresh, concurrent
//    8088 accesses and arbitration logic (DMA).
READ8_MEMBER(rainbow_state::share_z80_r)
{
	if (m_zflip)
	{
		if (offset < 0x8000)
		{
			return m_shared[offset + 0x8000];
		}
		else if (offset < 0x8800)
		{
			return m_z80_private[offset & 0x7ff]; // SRAM
		}

		return m_shared[offset ^ 0x8000];
	}
	else
	{
		if (offset < 0x800)
		{
			return m_z80_private[offset]; // SRAM
		}

		return m_shared[offset];
	}
}

WRITE8_MEMBER(rainbow_state::share_z80_w)
{
	if (m_zflip)
	{
		if (offset < 0x8000)
		{
			m_shared[offset + 0x8000] = data;
			return; // [!]
		}
		else if (offset < 0x8800)
		{
			m_z80_private[offset & 0x7ff] = data; // SRAM
			return; // [!]
		}

		m_shared[offset ^ 0x8000] = data;
	}
	else
	{
		if (offset < 0x800)
			m_z80_private[offset] = data; // SRAM
		else
			m_shared[offset] = data;
	}
	return;
}


// ------------------------ClikClok (for model B; DS1315)  ---------------------------------
#define RTC_RESET_MACRO  m_rtc->chip_reset(); \
							UINT8 *rom = memregion("maincpu")->base();

#define RTC_ENABLE_MACRO \
	if (m_inp11->read() == 0x01) \
	{                                            \
	if (offset & 1)                          \
		m_rtc->read_1(space, 0);             \
	else                                     \
		m_rtc->read_0(space, 0);              \
	}                                          \
	UINT8 *rom = memregion("maincpu")->base();

#define RTC_READ_MACRO \
	if (m_rtc->chip_enable() && (m_inp11->read() == 0x01))  \
		return (m_rtc->read_data(space, 0) & 0x01);         \
	UINT8 *rom = memregion("maincpu")->base();

// *********** RTC RESET **************
READ8_MEMBER(rainbow_state::rtc_reset)
{
	RTC_RESET_MACRO
	return rom[RTC_RESET];
}

READ8_MEMBER(rainbow_state::rtc_reset2)
{
	RTC_RESET_MACRO
	return rom[RTC_RESET + 0x2000];
}

// *********** RTC ENABLE **************
READ8_MEMBER(rainbow_state::rtc_enable)
{
	RTC_ENABLE_MACRO
	return rom[RTC_PATTERN_0 + offset];
}

READ8_MEMBER(rainbow_state::rtc_enable2)
{
	RTC_ENABLE_MACRO
	return rom[RTC_PATTERN_0 + offset + 0x2000];
}

// ** READ single bit from RTC (or ROM) **
READ8_MEMBER(rainbow_state::rtc_r)
{
	RTC_READ_MACRO
	return rom[RTC_READ_DATA];
}

READ8_MEMBER(rainbow_state::rtc_r2)
{
	RTC_READ_MACRO
	return rom[RTC_READ_DATA + 0x2000];
}

// Transfer data to DS1315 (read operation; data in offset):
// (always return ROM char to prevent potential IRQ issues)
READ8_MEMBER(rainbow_state::rtc_w)
{
	if (m_rtc->chip_enable() && (m_inp11->read() == 0x01))
		m_rtc->write_data(space, offset & 0x01);

	UINT8 *rom = memregion("maincpu")->base();
	return rom[RTC_WRITE_DATA_0 + offset];
}
// ------------------------/ ClikClok (for model B; DS1315)  ---------------------------------


// ---------------------------- RD51 HARD DISK CONTROLLER ----------------------------------
static const int SECTOR_SIZES[4] = { 256, 512, 1024, 128 };

#define RD51_MAX_HEAD 8
#define RD51_MAX_CYLINDER 1024
#define RD51_SECTORS_PER_TRACK 16

void rainbow_state::hdc_reset()
{
//  printf(">> HARD DISC CONTROLLER RESET <<\n");
	logerror(">> HARD DISC CONTROLLER RESET <<\n");

	m_hdc->reset(); // NEW HDC

	m_bdl_irq = 0;
	update_bundle_irq(); // reset INTRQ

	m_hdc_buf_offset = 0;
	m_hdc_direction = 0;

	m_hdc->buffer_ready(false);
	m_hdc_write_gate = false;

	m_hdc_step_latch = false;
	m_hdc_index_latch = false;

	m_hdc_track0 = true; // set true to avoid stepping
}

// Return 'hard_disk_file' object for harddisk 1 (fixed).
// < NULL if geometry is insane or other errors occured >
hard_disk_file *(rainbow_state::rainbow_hdc_file(int drv))
{
	m_hdc_drive_ready = false;

	if (m_inp5->read() != 0x01) // ...PRESENT?
		return NULL;

	if (drv != 0)
		return NULL;

	harddisk_image_device *img = NULL;
	img = dynamic_cast<harddisk_image_device *>(machine().device(subtag("harddisk1").c_str()));

	if (!img)
		return NULL;

	if (!img->exists())
		return NULL;

	hard_disk_file *file = img->get_hard_disk_file();
	hard_disk_info *info = hard_disk_get_info(file);

	// ALWAYS 16 SECTORS / TRACK.
	// CYLINDERS: 151 (~ 5 MB) to 1024 (max. cylinders on WD1010 controller)
	if (((info->sectors == RD51_SECTORS_PER_TRACK)) &&
		((info->heads >= 1) && (info->heads <= RD51_MAX_HEAD)) &&            // HEADS WITHIN 1...8
		((info->cylinders > 150) && (info->cylinders <= RD51_MAX_CYLINDER))
		)
	{
		m_hdc_drive_ready = true;
		return file;  // HAS SANE GEOMETRY
	}
	else
	{
		UINT32 max_sector = info->cylinders * info->heads * info->sectors;
		printf("%u MB HARD DISK: HEADS (1..8 OK) = %d / CYL. (151..1024 OK) = %d / SPT. (16 OK) = %d / SECTOR_BYTES (128..1024 OK) = %d\n", max_sector * 512 / 1000000,
			info->heads, info->cylinders, info->sectors, info->sectorbytes);

		printf("\n <<< === REJECTED = (SANITY CHECK FAILED) === >>> \n");
		return NULL;
	}
}

// LBA sector from CHS
static UINT32 get_and_print_lbasector(hard_disk_info *info, UINT16 cylinder, UINT8 head, UINT8 sector_number)
{
	if (info == NULL)
		return 0;

	// LBA_ADDRESS = (C * HEADS + H) * NUMBER_SECTORS + (S - 1)
	UINT32 lbasector = (double)cylinder * info->heads; // LBA : ( x 4 )
	lbasector += head;
	lbasector *= info->sectors;   // LBA : ( x 16 )
	lbasector += (sector_number - 1); // + (sector number - 1)

	logerror(" CYLINDER %u - HEAD %u - SECTOR NUMBER %u (LBA-SECTOR %u) ", cylinder, head, sector_number, lbasector);
	return lbasector;
}

// READ SECTOR (on BCS 1 -> 0 transition)
WRITE_LINE_MEMBER(rainbow_state::hdc_read_sector)
{
	static int last_state;
	int read_status = 1;

	if (!m_hdc_write_gate) // do not read when WRITE GATE is on
	{
		UINT8 SDH = (m_hdc->read(space(AS_PROGRAM), 0x06));
		int drv = (SDH & (8 + 16)) >> 3; // get DRIVE from SDH register

		if ( (state == 0) && (last_state == 1) && (drv == 0) )
		{
			read_status = 2;

			logerror("\nTRYING TO READ");
			output_set_value("led1", 0);

			int hi = (m_hdc->read(space(AS_PROGRAM), 0x05)) & 0x07;
			UINT16 cylinder = (m_hdc->read(space(AS_PROGRAM), 0x04)) | (hi << 8);
			UINT8 sector_number = m_hdc->read(space(AS_PROGRAM), 0x03);

			hard_disk_file *local_hard_disk;
			local_hard_disk = rainbow_hdc_file(0); // one hard disk for now.

			if (local_hard_disk)
			{
				read_status = 3;

				hard_disk_info *info;
				if ( (info = hard_disk_get_info(local_hard_disk)) )
				{
					read_status = 4;
					output_set_value("led1", 1);

					// Pointer to info + C + H + S
					UINT32 lbasector = get_and_print_lbasector(info, cylinder, SDH & 0x07, sector_number);

					if ((cylinder <= info->cylinders) &&                          // filter invalid ranges
						(SECTOR_SIZES[(SDH >> 5) & 0x03] == info->sectorbytes)    // may not vary in image!
						)
					{
						read_status = 5;
						if( hard_disk_read(local_hard_disk, lbasector, m_hdc_buffer) ) // accepts LBA sector (UINT32) !
						{
							read_status = 0;
							logerror("...success!\n");
						}
					}
				}
				m_hdc_buf_offset = 0;
				m_hdc->buffer_ready(true);
			} // if valid  (..)

			if (read_status != 0)
			{
				logerror("...** READ FAILED WITH STATUS %u ** (CYLINDER %u - HEAD %u - SECTOR # %u - SECTOR_SIZE %u ) ***\n",
												read_status, cylinder, SDH & 0x07, sector_number, SECTOR_SIZES[(SDH >> 5) & 0x03]
						) ;
			}

		} //   (on BCS 1 -> 0)

	} // do not read when WRITE GATE is on

	last_state = state;
}


// WRITE SECTOR
// ...IF WRITE_GATE (WG) TRANSITS FROM 1 -> 0

// NO PROVISIONS for  sector sizes != 512 or MULTIPLE DRIVES (> 0) !!!
WRITE_LINE_MEMBER(rainbow_state::hdc_write_sector)
{
	int success = 0;
	static int wg_last;

	if (state == 0)
		m_hdc_write_gate = false;
	else
		m_hdc_write_gate = true;

	int drv = ((m_hdc->read(space(AS_PROGRAM), 0x06)) & (8 + 16)) >> 3; // get DRIVE from SDH register

	if (((state == 0) && (wg_last == 1))  // Check correct state transition and DRIVE 0 ....
		&& (drv == 0)
		)
	{
		output_set_value("led1", 0);  // (1 = OFF ) =HARD DISK ACTIVITY =
		MOTOR_DISABLE_counter = 20;

		if (rainbow_hdc_file(0) != NULL)
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
int rainbow_state::do_write_sector()
{
	int feedback = 0; // no error
	output_set_value("led1", 0); // ON

	hard_disk_file *local_hard_disk;
	local_hard_disk = rainbow_hdc_file(0); // one hard disk for now.

	if (local_hard_disk)
	{
		hard_disk_info *info;
		if ( (info = hard_disk_get_info(local_hard_disk)) )
		{
			feedback = 10;
			logerror("\n* TRYING TO WRITE * ");
			output_set_value("led1", 1); // OFF

			UINT8 SDH = (m_hdc->read(space(AS_PROGRAM), 0x06));

			int hi = (m_hdc->read(space(AS_PROGRAM), 0x05)) & 0x07;
			UINT16 cylinder = (m_hdc->read(space(AS_PROGRAM), 0x04)) | (hi << 8);

			int sector_number = m_hdc->read(space(AS_PROGRAM), 0x03);
			int sector_count = m_hdc->read(space(AS_PROGRAM), 0x02); // (1 = single sector)

			if (!((cylinder <= info->cylinders) &&                     // filter invalid cylinders
				(SECTOR_SIZES[(SDH >> 5) & 0x03] == info->sectorbytes) // 512, may not vary
				))
			{
				logerror("...*** SANITY CHECK FAILED (CYLINDER %u vs. info->cylinders %u - - SECTOR_SIZE %u vs. info->sectorbytes %u) ***\n",
					cylinder, info->cylinders, SECTOR_SIZES[(SDH >> 5) & 0x03], info->sectorbytes
					);
				return 50;
			}
			// Pointer to info + C + H + S
			UINT32 lbasector = get_and_print_lbasector(info, cylinder, SDH & 0x07, sector_number);

			if (sector_count != 1) // ignore all SECTOR_COUNTS != 1
			{
				logerror(" - ** IGNORED (SECTOR_COUNT !=1) **\n");
				return 88; // BAIL OUT
			}

			if (hard_disk_write(local_hard_disk, lbasector, m_hdc_buffer))  // accepts LBA sector (UINT32) !
			{
				logerror("...success! ****\n");
				feedback = 99;
			}
			else
			{
				logerror("...FAILURE **** \n");
			}

		} // IF 'info' not NULL
	} // IF hard disk present

	return feedback;
}



READ8_MEMBER(rainbow_state::hd_status_60_r)
{
	int data = m_hdc_buffer[m_hdc_buf_offset];
	//logerror("HARD DISK DISK BUFFER: READ offset %04x | data = %02x\n", m_hdc_buf_offset, data); // ! DO NOT CHANGE ORDER !

	m_hdc_buf_offset = (m_hdc_buf_offset + 1);
	if (m_hdc_buf_offset >= 1024) // 1 K enforced by controller
	{
		m_hdc_buf_offset = 0;
		m_hdc->buffer_ready(true);
	}
	return data;
}

WRITE8_MEMBER(rainbow_state::hd_status_60_w)
{
	//logerror("HARD DISK BUFFER: WRITE offset %04x | data = %02x\n", m_hdc_buf_offset, data);

	m_hdc_buffer[m_hdc_buf_offset] = data;
	m_hdc_buf_offset = (m_hdc_buf_offset + 1);

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
READ8_MEMBER(rainbow_state::hd_status_68_r)
{
	// (*) Bits 5-7 : HARD WIRED IDENTIFICATION BITS, bits 5+7 = 1 and bit 6 = 0  (= 101 f?r RD51 module)
	int data = 0xe0; // 111 gives DRIVE NOT READY (when W is pressed on boot screen)
	if ((m_inp5->read() == 0x01) && (rainbow_hdc_file(0) != NULL))
		data = 0xa0; // A0 : OK, DRIVE IS READY (!)

	int my_offset = 0x07;
	int stat = m_hdc->read(space, my_offset);
	logerror("(x68) WD1010 register %04x (STATUS) read, result : %04x\n", my_offset, stat);

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


// 68 (WRITE): Secondary Command Registers (68H) - - ERKL?RUNG: "write-only register for commands"
// - see TABLE 4.8 (4-24)
WRITE8_MEMBER(rainbow_state::hd_status_68_w)
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
		logerror(">> HARD DISC * SET BUFFER READY * <<\n");

		output_set_value("led1", 0);  // 1 = OFF (One of the CPU LEDs as DRIVE LED) = HARD DISK ACTIVITY =
		MOTOR_DISABLE_counter = 20;

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
READ8_MEMBER(rainbow_state::hd_status_69_r)
{
	int HS = m_hdc->read(space, 0x06) & (1 + 2 + 4); // SDH bits 0-2 = HEAD #
	logerror("(x69 READ) %i = HEAD SELECT WD1010\n", HS);

	UINT8 data = (HS << 1);

	// DRIVE SELECT: 2 bits in SDH register of WDx010 could address 4 drives.
	// External circuit supports 1 drive here (DRIVE 0 selected or deselected)
	int DRV = ((m_hdc->read(space, 0x06) >> 3) & 0x01);  // 0x03 gives error R6 with DIAG.DISK
	if (DRV == 0)
	{
		logerror("(x69 READ) %i = _DRIVE # 0_ SELECT! \n", DRV);
		data |= 1;
	}

	if (m_hdc_write_gate) // WRITE GATE (cached here)
		data |= 16;

	if (m_hdc_write_fault)
		data |= 32;

	if (m_hdc_drive_ready)
		data |= 64;

	// Fake TRACK 0 signal  (normally FROM DRIVE)
	m_hdc_track0 = false; // Set a default

	int stat1 = m_hdc->read(space, 0x04); // CYL LO
	int stat2 = m_hdc->read(space, 0x05); // CYL HI
	if ((stat1 == 0) && (stat2 == 0))
			m_hdc_track0 = true;

	if (m_hdc_track0)
	{
		data |= 128;
		logerror("(x69 READ) TRACK 00 detected\n");
	}

	return data;
}

// TREAT SIGNALS FROM / TO CONTROLLER
WRITE_LINE_MEMBER(rainbow_state::hdc_step)
{
	m_hdc_step_latch = true;

	output_set_value("led1", 0);  // 1 = OFF (One of the CPU LEDs as DRIVE LED)  = HARD DISK ACTIVITY =
	MOTOR_DISABLE_counter = 20;
}

WRITE_LINE_MEMBER(rainbow_state::hdc_direction)
{
	m_hdc_direction = state; // (0 = OUT)
}

READ_LINE_MEMBER(rainbow_state::hdc_drive_ready)
{
	return m_hdc_drive_ready;
}

READ_LINE_MEMBER(rainbow_state::hdc_write_fault)
{
	return m_hdc_write_fault;
}

// Buffer counter reset when BCR goes from 0 -> 1
WRITE_LINE_MEMBER(rainbow_state::hdc_bcr)
{
	static int bcr_state;
	if ((bcr_state == 0) && (state == 1))
		hdc_buffer_counter_reset();
	bcr_state = state;
}

void rainbow_state::hdc_buffer_counter_reset()
{
	m_hdc->buffer_ready(false);
	m_hdc_buf_offset = 0;
}

// DATA REQUEST - When high (..) initiates data transfers
// to or from the sector buffer. On a READ, this signal
// goes high AFTER the sector buffer is filled.

// On a WRITE / FORMAT command, signal goes high when the WD1010
// chip is READY TO ACCESS the information in the sector buffer.
WRITE_LINE_MEMBER(rainbow_state::hdc_bdrq)
{
	static int old_state;

	logerror("BDRQ - BUFFER DATA REQUEST OBTAINED: %u\n", state);
	if ((state == 1) && (old_state == 0))
	{
		hdc_buffer_counter_reset();

		m_bdl_irq = state;
		update_bundle_irq(); // TRIGGER AN INTERRUPT
	}
	old_state = state;
}
// ---------------------------- / RD51 HARD DISK CONTROLLER ----------------------------------


// IRQ service for both RD51 and COMM. OPTION
void rainbow_state::update_bundle_irq()
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

WRITE_LINE_MEMBER(rainbow_state::bundle_irq)
{
	m_bdl_irq = state;
	update_bundle_irq();
}



READ8_MEMBER(rainbow_state::system_parameter_r)
{
#ifdef FORCE_RAINBOW_B_LOGO

	// WORKAROUND: Initialize NVRAM + VOLATILE RAM with DEFAULTS + correct RAM SIZE !
	int test = (m_inp8->read() >> 16) - 2; // 0 = 128 K (!)
	if (test >= 0)
	{
		m_p_vol_ram[0xdb] = test;
		m_p_nvram[0xdb] = test;
	}

	// NVRAM @ 0x89 : AUTO RPT(0 = OFF 1 = ON)
	m_p_nvram[0x89] = 0;  //  AUTO REPEAT ON (1 would be DETRIMENTAL TO "SET-UP" (80/132 + 50/60 Hz) with KEYBOARD_WORKAROUND (!)
#endif


	/*  Info about option boards is in bits 0 - 3:
	SYSTEM PARAMETER INFORMATION: see AA-P308A-TV page 92 section 14.0
	Bundle card (1) | Floppy (2) | Graphics (4) | Memory option (8)
	0 1 2 3 4 5 6 7
	B F G M
	(bit SET means NOT present; 4-7 reserved )

	B : no separation between the 2 available 'bundle cards' (HD controller / COMM.OPTION) ?

	M : old RAM extension (128 / 192 K ?) detected with OPTION_PRESENT bit, newer models 'by presence'.
	    BIOS uses a seperate IRQ vector for RAM board detection (at least on a 100-B).
	*/
	return  (((m_inp5->read() == 1) ? 0 : 1) |
		((m_inp6->read() == 1) ? 0 : 2) |
		((m_inp7->read() == 1) ? 0 : 4) |
		((m_inp8->read() > BOARD_RAM) ? 0 : 8)
		//              16 | 32 | 64 | 128 // to be verified.
		);
}

READ8_MEMBER(rainbow_state::comm_control_r)
{
	/*  [02] COMMUNICATIONS STATUS REGISTER - PAGE 154 (**** READ **** )
	Used to read status of SERIAL port, IRQ line of each CPU, and MHFU logic enable signal.

	//    What the specs says on how MHFU detection is disabled:
	//    1.  by first disabling interrupts with CLI
	//    2.  by writing 0x00 to port 0x10C (handled by a 8088 write handler)
	//    MHFU is re-enabled by writing to 0x0c (or 'automatically after STI') <-- when under BIOS control ?
	*/
	int data = 0;
	if (COLD_BOOT == 2)
		data = 0;   // During boot phase 2, never enable MHFU (prevents errors).
	else
	{
		data = m_crtc->MHFU(1);
	}
	return (((data) ? 0x00 : 0x20) | //  // (L) status of MHFU flag => bit pos.5
		((INT88) ? 0x00 : 0x40)        | // (L)
		((INTZ80) ? 0x00 : 0x80)         // (L)
		);
}

WRITE8_MEMBER(rainbow_state::comm_control_w)
{
	/* Communication control register of -COMM- port (when written):

	8088 LEDs:
	5  7  6  4    <- BIT POSITION
	D6 -D5-D4-D3  <- INTERNAL LED NUMBER (DEC PDF)
	-4--5--6--7-  <- NUMBERS EMBOSSED ON BACK OF PLASTIC HOUSING (see error chart)
	*/
	output_set_value("led4", BIT(data, 5)); // LED "D6"
	output_set_value("led5", BIT(data, 7)); // LED "D5"
	output_set_value("led6", BIT(data, 6)); // LED "D4"
	output_set_value("led7", BIT(data, 4)); // LED "D3"

	//  printf("%02x to COMM.CONTROL REGISTER\n", data);
}



// 8088 writes to port 0x00 (interrupts Z80)
// See page 133 (4-34)
WRITE8_MEMBER(rainbow_state::i8088_latch_w)
{
	//    printf("%02x to Z80 mailbox\n", data);

	// The interrupt vector address(F7H) placed on the bus is hardwired into the Z80A interrupt vector encoder.
	// The F7H interrupt vector address causes the Z80A processor to perform an RST 30 instruction in
	// interrupt mode 0
	m_z80->set_input_line_and_vector(0, ASSERT_LINE, 0xf7);
	m_z80_mailbox = data;

	INTZ80 = true; //
}

// Z80 reads port 0x00
// See page 134 (4-35)
READ8_MEMBER(rainbow_state::z80_latch_r)
{
	//    printf("Read %02x from Z80 mailbox\n", m_z80_mailbox);
	m_z80->set_input_line(0, CLEAR_LINE);

	INTZ80 = false;
	return m_z80_mailbox;
}

// Z80 writes to port 0x00 (interrupts 8088)
// See page 134 (4-35)
WRITE8_MEMBER(rainbow_state::z80_latch_w)
{
	//    printf("%02x to 8088 mailbox\n", data);
	raise_8088_irq(IRQ_8088_MAILBOX);
	m_8088_mailbox = data;

	INT88 = true;
}

// 8088 reads port 0x00. See page 133 (4-34)
READ8_MEMBER(rainbow_state::i8088_latch_r)
{
	//    printf("Read %02x from 8088 mailbox\n", m_8088_mailbox);
	lower_8088_irq(IRQ_8088_MAILBOX);

	INT88 = false;
	return m_8088_mailbox;
}

// (Z80) : WRITE to 0x20
WRITE8_MEMBER(rainbow_state::z80_diskdiag_read_w)
{
	m_zflip = true;
}

// (Z80) : PORT 21H * WRITE *
WRITE8_MEMBER(rainbow_state::z80_diskdiag_write_w)
{
	/*   Z80 LEDs:
	4   5   6  <- bit #
	D11 D10 -D9 <- INTERNAL LED NUMBER (see PDF)
	-1 --2-- 3  <- NUMBERS EMBOSSED ON BACK OF PLASTIC HOUSING (see error chart)
	*/
	output_set_value("led1", BIT(data, 4)); // LED "D11"
	output_set_value("led2", BIT(data, 5)); // LED "D10"
	output_set_value("led3", BIT(data, 6)); // LED "D9"

	m_zflip = false;
}

// (Z80) : PORT 21H  _READ_
READ8_MEMBER(rainbow_state::z80_generalstat_r)
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

	printf("\nFLOPPY %02d - ", m_unit); // TEST-DEBUG

	// Attempt to block invalid accesses
	if (m_unit != INVALID_DRIVE)
	{
		track = m_fdc->track_r(space, 0);
		if (track != last_track)
			fdc_step = 1;  // calculate STEP (sic)
		last_track = track;

		if (!m_floppy->ready_r()) // weird (see wd_fdc)
			fdc_ready = 1;

		if (fdc_ready)
			fdc_write_gate = 1; // * FAKE * WRITE GATE !

		// "valid only when drive is selected" !
		if (!m_floppy->trk00_r()) // weird (see wd_fdc)
			tk00 = 1;

		if (last_track > track)
			last_dir = 1;      // correct?
		else
			last_dir = 0;

		if (fdc_ready == 1)
		printf(" RDY:1 "); // TEST-DEBUG
		else
		printf(" RDY:0 "); // TEST-DEBUG

		if (fdc_step == 1)
		printf(" STP:1 "); // TEST-DEBUG
		else
		printf(" STP:0 "); // TEST-DEBUG

		if (tk00 == 0)
		printf(" TK00=0 "); // TEST-DEBUG
		else
		printf(" TK00=1 "); // TEST-DEBUG

	}

	int data = (
		((fdc_step) ? 0x00 : 0x80) |
		((fdc_write_gate) ? 0x00 : 0x40) |
		((tk00) ? 0x20 : 0x00) |  // ***** ALL LOW ACTIVE - EXCEPT tk00 :
		((last_dir) ? 0x00 : 0x10) |
		((fdc_ready) ? 0x00 : 0x08) |
		((INT88) ? 0x00 : 0x04) |
		((INTZ80) ? 0x00 : 0x02) |
		((m_zflip) ? 0x00 : 0x01)
		);

	return data;
}



// (Z80) : PORT 40H _READ_
// 40H diskette status Register **** READ ONLY *** ( 4-60 of TM100.pdf )
READ8_MEMBER(rainbow_state::z80_diskstatus_r)
{
	int track = 0;

	// AND 00111011 - return what was WRITTEN to D5-D3, D1, D0 previously
	//                (except D7,D6,D2)
	int data = m_z80_diskcontrol & 0x3b;

	// -DOES NOT WORK HERE- Attempt to block invalid accesses
	if (m_unit != INVALID_DRIVE)
	{
		// D7: DRQ: reflects status of DATA REQUEST signal from FDC.
		// '1' indicates that FDC has read data OR requires new write data.
		data |= m_fdc->drq_r() ? 0x80 : 0x00;

		// D6: IRQ: indicates INTERRUPT REQUEST signal from FDC. Indicates that a
		//          status bit has changed. Set to 1 at the completion of any
		//          command (.. see page 207 or 5-25).
		data |= m_fdc->intrq_r() ? 0x40 : 0x00;

		// D5: SIDE 0H: status of side select signal at J2 + J3 of RX50 controller.
		//              For 1 sided drives, this bit will always read low (0).

		// D4: MOTOR 1 ON L: 0 = indicates MOTOR 1 ON bit is set in drive control reg.
		// D3: MOTOR 0 ON L: 0 = indicates MOTOR 0 ON bit is set in drive  "

		track = m_fdc->track_r(space, 0);

		// Print HEX track number
		static UINT8 bcd2hex[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71 };
		// 0...9 ,A (0x77), b (0x7c), C (0x39) , d (0x5e), E (0x79), F (0x71)
		output_set_digit_value(0, bcd2hex[(track >> 4) & 0x0f]);
		output_set_digit_value(1, bcd2hex[(track - ((track >> 4) << 4)) & 0x0f]);
	}

	// D2: TG43 L :  0 = INDICATES TRACK > 43 SIGNAL FROM FDC TO DISK DRIVE.
	if (track > 43)
		data = data & (255 - 4);
	else
		data = data | 4;

	// D1: DS1 H: reflect status of bits 0 and 1 form disk.control reg.
	// D0: DS0 H: "
	return data;
}

// (Z80) : PORT 40H  * WRITE *
// NOTE: routine will accept invalid drive letters...
WRITE8_MEMBER(rainbow_state::z80_diskcontrol_w)
{
	int selected_drive = INVALID_DRIVE;
	static const char *names[] = { FD1793_TAG ":0", FD1793_TAG ":1", FD1793_TAG ":2", FD1793_TAG ":3" };

	int drive = 0;
	if (m_inp10->read() && ((data & 3) < 2))
		drive = (data & 1) + 2;
	else
		drive = data & 3;

	floppy_connector *con = NULL;
	if (drive < MAX_FLOPPIES)
		con = machine().device<floppy_connector>(names[drive]);

	if (con)
	{
		m_floppy = con->get_device();
		if (m_floppy)
			selected_drive = drive;
		//printf("%i <- SELECTED DRIVE...\n", m_unit);
	}

	if (selected_drive == INVALID_DRIVE)
	{
		printf("(m_unit = %i)   ** SELECTED DRIVE ** INVALID. (selected drive = %i)\n", m_unit, selected_drive);
		m_unit = INVALID_DRIVE;
		m_floppy = NULL;
	}

	if (m_floppy != NULL)
	{
		m_fdc->set_floppy(m_floppy);  // Sets new  _image device_
		if (!m_floppy->exists())
		{
			m_floppy = NULL;
			printf("(m_unit = %i) SELECTED IMAGE *** DOES NOT EXIST *** (selected drive = %i)\n", m_unit, selected_drive);
			selected_drive = INVALID_DRIVE;
			//m_unit = INVALID_DRIVE;
		}
		else
		{
			m_floppy->ss_w((data & 20) ? 1 : 0); // RX50 board in Rainbow has 'side select'
			m_floppy->set_rpm(300.);
		}
	}

	output_set_value("driveled0", (selected_drive == 0) ? 1 : 0);
	output_set_value("driveled1", (selected_drive == 1) ? 1 : 0);

	output_set_value("driveled2", (selected_drive == 2) ? 1 : 0);
	output_set_value("driveled3", (selected_drive == 3) ? 1 : 0);

	if (selected_drive < 4)
	{
		m_unit = selected_drive;

		if (MOTOR_DISABLE_counter == 0) // "one shot"
			MOTOR_DISABLE_counter = 20;

		// FORCE_READY = 0 : assert DRIVE READY on FDC (diagnostic override; USED BY BIOS!)
		bool force_ready = ((data & 4) == 0) ? true : false;
		m_fdc->set_force_ready(force_ready);
	}

	int enable_start = 0;
	int disable_start = 2; // set defaults

	if (m_unit == INVALID_DRIVE)
	{
		printf("\n**** INVALID DRIVE ****");
		data = (data & (255 - 3)) | m_unit;

		data = data & (255 - 8); // MOTOR 0 OFF
		data = data & (255 - 16); // MOTOR 1 OFF
	}
	else
	{
		data = (data & (255 - 3)) | m_unit;

		if (m_unit < 2)
		{
			data = data | 8; // MOTOR 0 (for A or B)
		}
		else
		{
			data = data | 16; // MOTOR 1 (for C or D)
			enable_start = 2;
			disable_start = 4;
		}

		// RX-50 has head A and head B (1 for each of the 2 disk slots in a RX-50).
		// Assume the other one is switched off -
		for (int f_num = 0; f_num < MAX_FLOPPIES; f_num++)
		{
			floppy_connector *con = machine().device<floppy_connector>(names[f_num]);
			floppy_image_device *tmp_floppy = con->get_device();

			tmp_floppy->mon_w(ASSERT_LINE);
			if ((f_num >= enable_start) && (f_num < disable_start))
				tmp_floppy->mon_w(CLEAR_LINE); // enable
		}

	}

	m_z80_diskcontrol = data;
}
// --------- END OF Z80 --------------------

READ8_MEMBER(rainbow_state::read_video_ram_r)
{
	return m_p_ram[offset];
}


// **************************************************
// VIDEO INTERRUPT HANDLING
// **************************************************

INTERRUPT_GEN_MEMBER(rainbow_state::vblank_irq)
{
	raise_8088_irq(IRQ_8088_VBL);
	m_crtc->notify_vblank(true);
}


WRITE_LINE_MEMBER(rainbow_state::clear_video_interrupt)
{
	lower_8088_irq(IRQ_8088_VBL);
	m_crtc->notify_vblank(false);
}

// Reflects bits from 'diagnostic_w', except test jumpers
READ8_MEMBER(rainbow_state::diagnostic_r) // 8088 (port 0A READ). Fig.4-29 + table 4-15
{
	return ((m_diagnostic & (0xf1)) |   // MASK 0xf1 = 11110001
		(m_inp1->read() |
		m_inp2->read() |
		m_inp3->read()
		)
		);
}


WRITE8_MEMBER(rainbow_state::diagnostic_w) // 8088 (port 0A WRITTEN). Fig.4-28 + table 4-15
{
	//    printf("%02x to diag port (PC=%x)\n", data, space.device().safe_pc());
	m_SCREEN_BLANK = (data & 2) ? false : true;

	// ZRESET (high at powerup)
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

	if ((m_diagnostic & 1) && !(data & 1))
	{
		printf("\nFDC ** RESET **");
		m_fdc->reset();
	}

	if (!(m_diagnostic & 1) && (data & 1))
	{
		printf("\nFDC RESTORE");
		m_fdc->soft_reset(); // See formatter description p.197 or 5-13
	}

	// MISSING BITS (* not vital for normal operation, see diag.disk) -
	// BIT 2: GRF VID SEL (0 = system module; 1 = graphics option)
	// BIT 3: PARITY TEST (1 = enables parity test on memory option board)
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
		printf("\nWARNING: UNEMULATED DIAG LOOPBACK (directs RX50 and DC12 output to printer port) ****");
	}

	if (data & 32)
	{
		/* BIT 5: PORT LOOPBACK (1 enables loopback for COMM, PRINTER, KEYBOARD ports)
		2.1.7.2. of AA-V523A-TV (PDF Mar83) says how the signals are routed:
		port_loopback_0  |  port_loopback_1   SIGNAL INPUT TO
		COMM RCV DATA.......COMM TXD..........COMM_RXD
		PRT  RCV DATA.......KBD TXD...........PRT RDATA
		KBD  RCV DATA.......PRT TXD...........KBD RXD
		*/
		printf("\nWARNING: UNEMULATED PORT LOOPBACK (COMM, PRINTER, KEYBOARD ports) ****");
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
void rainbow_state::update_kbd_irq()
{
#ifndef KEYBOARD_WORKAROUND
	if ((m_kbd_rx_ready) || (m_kbd_tx_ready))
	{
		raise_8088_irq(IRQ_8088_KBD);
	}
	else
	{
		lower_8088_irq(IRQ_8088_KBD);
	}
#endif
}

WRITE_LINE_MEMBER(rainbow_state::kbd_tx)
{
	//    printf("%02x to keyboard\n", state);
	m_lk201->rx_w(state);
}

WRITE_LINE_MEMBER(rainbow_state::kbd_rxready_w)
{
	//    printf("rxready %d\n", state);
	m_kbd_rx_ready = (state == 1) ? true : false;
	update_kbd_irq();
}

WRITE_LINE_MEMBER(rainbow_state::kbd_txready_w)
{
	//    printf("8251 txready %d\n", state);
	m_kbd_tx_ready = (state == 1) ? true : false;
	update_kbd_irq();
}

WRITE_LINE_MEMBER(rainbow_state::write_keyboard_clock)
{
	m_kbd8251->write_txc(state);
	m_kbd8251->write_rxc(state);
}

TIMER_DEVICE_CALLBACK_MEMBER(rainbow_state::motor_tick)
{
	m_hdc_index_latch = true; // HDC drive index signal (not working ?)

	if (MOTOR_DISABLE_counter)
		MOTOR_DISABLE_counter--;

	if (MOTOR_DISABLE_counter < 2)
	{
		output_set_value("driveled0", 0); // DRIVE 0 (A)
		output_set_value("driveled1", 0); // DRIVE 1 (B)
		output_set_value("driveled2", 0); // DRIVE 2 (C)
		output_set_value("driveled3", 0); // DRIVE 3 (D)

		output_set_value("led1", 1);  // 1 = OFF (One of the CPU LEDs as DRIVE LED)
	}

	if (m_crtc->MHFU(1)) // MHFU * flag * enabled ?
	{
		int data = m_crtc->MHFU(-1); // increment MHFU, return new value

		// MHFU gets active if the 8088 has not acknowledged a video processor interrupt within approx. 108 milliseconds.
		// Timer reset by 2 sources : the VERT INT L from the DC012, or the MHFU ENB L from the enable flip - flop.

		if ( data > 480 ) // longer than needed. Better than an instant "INTERRUPT 16"
		{
			for (int i = 0; i < 9; i++)
				printf("\nWATCHDOG TRIPPED *** NOW RESET MACHINE ***\n");

			m_crtc->MHFU(-100);  // -100 : Enable MHFU flag

			if (m_inp12->read() == 0x01) // DIP for watchdog set?
			{
				COLD_BOOT = 0;  // massive foulup (no recovery)
				device().machine().schedule_soft_reset(); // FFFF:0
			}
		}
	}

	if (m_beep_counter > 0)
		m_beep_counter--;
}

// on 100-B, DTR from the keyboard 8051 controls bit 7 of IRQ vectors
WRITE_LINE_MEMBER(rainbow_state::irq_hi_w)
{
	m_irq_high = (state == ASSERT_LINE) ? 0x80 : 0;
	//logerror("%i = m_irq_high\n", m_irq_high);
}

/* F4 Character Displayer */
static const gfx_layout rainbow_charlayout =
{
	8, 10,                  /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 15 * 8, 0 * 8, 1 * 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8, 8 * 8 },
	8 * 16                    /* every char takes 16 bytes */
};

static GFXDECODE_START(rainbow)
GFXDECODE_ENTRY("chargen", 0x0000, rainbow_charlayout, 0, 1)
GFXDECODE_END


static MACHINE_CONFIG_START(rainbow, rainbow_state)
MCFG_DEFAULT_LAYOUT(layout_rainbow)

/* basic machine hardware */
MCFG_CPU_ADD("maincpu", I8088, XTAL_24_0734MHz / 5)
MCFG_CPU_PROGRAM_MAP(rainbow8088_map)
MCFG_CPU_IO_MAP(rainbow8088_io)
MCFG_CPU_VBLANK_INT_DRIVER("screen", rainbow_state, vblank_irq)

MCFG_CPU_ADD("subcpu", Z80, XTAL_24_0734MHz / 6)
MCFG_CPU_PROGRAM_MAP(rainbowz80_mem)
MCFG_CPU_IO_MAP(rainbowz80_io)

/* video hardware */
MCFG_SCREEN_ADD("screen", RASTER)
MCFG_SCREEN_REFRESH_RATE(60)
MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
MCFG_SCREEN_SIZE(132 * 10, 49 * 10)
MCFG_SCREEN_VISIBLE_AREA(0, 80 * 10 - 1, 0, 48 * 10 - 1)
MCFG_SCREEN_UPDATE_DRIVER(rainbow_state, screen_update_rainbow)
MCFG_SCREEN_PALETTE("vt100_video:palette")
MCFG_GFXDECODE_ADD("gfxdecode", "vt100_video:palette", rainbow)

MCFG_DEVICE_ADD("vt100_video", RAINBOW_VIDEO, 0)
MCFG_VT_SET_SCREEN("screen")
MCFG_VT_CHARGEN("chargen")
MCFG_VT_VIDEO_RAM_CALLBACK(READ8(rainbow_state, read_video_ram_r))
MCFG_VT_VIDEO_CLEAR_VIDEO_INTERRUPT_CALLBACK(WRITELINE(rainbow_state, clear_video_interrupt))

MCFG_FD1793_ADD(FD1793_TAG, XTAL_24_0734MHz / 24) // no separate 1 Mhz quartz
//MCFG_WD_FDC_FORCE_READY
MCFG_FLOPPY_DRIVE_ADD(FD1793_TAG ":0", rainbow_floppies, "525qd0", rainbow_state::floppy_formats)
MCFG_FLOPPY_DRIVE_ADD(FD1793_TAG ":1", rainbow_floppies, "525qd1", rainbow_state::floppy_formats)
MCFG_FLOPPY_DRIVE_ADD(FD1793_TAG ":2", rainbow_floppies, "525qd2", rainbow_state::floppy_formats)
MCFG_FLOPPY_DRIVE_ADD(FD1793_TAG ":3", rainbow_floppies, "525qd3", rainbow_state::floppy_formats)
MCFG_SOFTWARE_LIST_ADD("flop_list", "rainbow")


/// ********************************* HARD DISK CONTROLLER *****************************************
MCFG_DEVICE_ADD("hdc", WD2010, 5000000) // 10 Mhz quartz on controller (divided by 2 for WCLK)
MCFG_WD2010_OUT_INTRQ_CB(WRITELINE(rainbow_state, bundle_irq)) // FIRST IRQ SOURCE (OR'ed with DRQ)
MCFG_WD2010_OUT_BDRQ_CB(WRITELINE(rainbow_state, hdc_bdrq))  // BUFFER DATA REQUEST

// SIGNALS -FROM- WD CONTROLLER:
MCFG_WD2010_OUT_BCS_CB(WRITELINE(rainbow_state, hdc_read_sector)) // Problem: OUT_BCS_CB = WRITE8 ... (!)
MCFG_WD2010_OUT_BCR_CB(WRITELINE(rainbow_state, hdc_bcr))         // BUFFER COUNTER RESET (pulses)

MCFG_WD2010_OUT_WG_CB(WRITELINE(rainbow_state, hdc_write_sector))   // WRITE GATE
MCFG_WD2010_OUT_STEP_CB(WRITELINE(rainbow_state, hdc_step))         // STEP PULSE
MCFG_WD2010_OUT_DIRIN_CB(WRITELINE(rainbow_state, hdc_direction))

MCFG_WD2010_IN_WF_CB(READLINE(rainbow_state, hdc_write_fault))   // WRITE FAULT  (set to GND if not serviced)

MCFG_WD2010_IN_DRDY_CB(READLINE(rainbow_state, hdc_drive_ready)) // DRIVE_READY  (set to VCC if not serviced)
MCFG_WD2010_IN_SC_CB(VCC)                                        // SEEK COMPLETE (set to VCC if not serviced)

// CURRENTLY NOT EVALUATED WITHIN 'WD2010':
MCFG_WD2010_IN_TK000_CB(VCC)
MCFG_WD2010_IN_INDEX_CB(VCC)

MCFG_HARDDISK_ADD("harddisk1")
/// ******************************** / HARD DISK CONTROLLER ****************************************

MCFG_DS1315_ADD("rtc") // DS1315 (ClikClok for DEC-100 B)   * OPTIONAL *

MCFG_DEVICE_ADD("kbdser", I8251, 0)
MCFG_I8251_TXD_HANDLER(WRITELINE(rainbow_state, kbd_tx))
MCFG_I8251_DTR_HANDLER(WRITELINE(rainbow_state, irq_hi_w))
MCFG_I8251_RXRDY_HANDLER(WRITELINE(rainbow_state, kbd_rxready_w))
MCFG_I8251_TXRDY_HANDLER(WRITELINE(rainbow_state, kbd_txready_w))

MCFG_DEVICE_ADD(LK201_TAG, LK201, 0)
MCFG_LK201_TX_HANDLER(DEVWRITELINE("kbdser", i8251_device, write_rxd))

MCFG_DEVICE_ADD("keyboard_clock", CLOCK, 4800 * 16) // 8251 is set to /16 on the clock input
MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(rainbow_state, write_keyboard_clock))

MCFG_TIMER_DRIVER_ADD_PERIODIC("motor", rainbow_state, motor_tick, attotime::from_hz(60))

MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END

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

ROM_LOAD("23-176e4-00.bin", 0xFA000, 0x2000, NO_DUMP) // ROM (FA000-FBFFF) (E89) 8 K
ROM_LOAD("23-177e4-00.bin", 0xFC000, 0x2000, NO_DUMP) // ROM (FC000-FDFFF) (E90) 8 K

// SOCKETED LANGUAGE ROM (E91) with 1 single localization per ROM -
ROM_LOAD("23-092e4-00.bin", 0xFE000, 0x2000, NO_DUMP)  // ROM (FE000-FFFFF) (E91) 8 K - English (?)
// See also MP-01491-00 - PC100A FIELD MAINTENANCE SET. Appendix A of EK-RB100 Rainbow
// Technical Manual Addendum f.100A and 100B (Dec.84) lists 15 localizations / part numbers

ROM_REGION(0x1000, "chargen", 0) // [E98] 2732 (4 K) EPROM
ROM_LOAD("23-020e3-00.bin", 0x0000, 0x1000, CRC(1685e452) SHA1(bc299ff1cb74afcededf1a7beb9001188fdcf02f))
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

// LANGUAGE ROM
ROM_LOAD("23-020e5-00.bin", 0xf8000, 0x4000, CRC(8638712f) SHA1(8269b0d95dc6efbe67d500dac3999df4838625d8)) // German, French, English
//ROM_LOAD( "23-015e5-00.bin", 0xf8000, 0x4000, NO_DUMP) // Dutch, French, English
//ROM_LOAD( "23-016e5-00.bin", 0xf8000, 0x4000, NO_DUMP) // Finish, Swedish, English
//ROM_LOAD( "23-017e5-00.bin", 0xf8000, 0x4000, NO_DUMP) // Danish, Norwegian, English
//ROM_LOAD( "23-018e5-00.bin", 0xf8000, 0x4000, NO_DUMP) // Spanish, Italian, English
ROM_RELOAD(0xfc000, 0x4000)

// CHARACTER GENERATOR (E3-03)
ROM_REGION(0x1000, "chargen", 0)
ROM_LOAD("23-037e3.bin", 0x0000, 0x1000, CRC(1685e452) SHA1(bc299ff1cb74afcededf1a7beb9001188fdcf02f))
ROM_END

//----------------------------------------------------------------------------------------
// 'Rainbow 190 B' (announced March 1985) is identical to 100-B, with alternate ROM v5.05.
// According to an article in Wall Street Journal it came with a 10 MB HD and 640 K RAM.

// All programs not dependent on specific ROM addresses should work. A first glance:
// - jump tables (F4000-F40083 and FC000-FC004D) were not extended
// - absolute addresses of some internal routines have changed (affects BOOT 2.x / 3.x dual boot)

// A Readme from January 1985 mentions 'recent ROM changes for MASS 11' (a VAX word processor).
// It is *likely* that the sole differences between 5.05 and 5.03 affect terminal emulation.

// FIXME: ROM names are * made up *.
// Someone who knows the DEC naming conventions should correct them -
ROM_START(rainbow190)
ROM_REGION(0x100000, "maincpu", 0)
ROM_LOAD("dec190rom0.bin", 0xf0000, 0x4000, CRC(fac191d2) SHA1(4aff5b1e031d3b5eafc568b23e68235270bb34de))
ROM_RELOAD(0xf4000, 0x4000)
ROM_LOAD("dec190rom1.bin", 0xf8000, 0x4000, CRC(5ce59632) SHA1(d29793f7014c57a4e7cb77bbf6e84f9113635ed2))

ROM_RELOAD(0xfc000, 0x4000)
ROM_REGION(0x1000, "chargen", 0)
ROM_LOAD("chargen.bin", 0x0000, 0x1000, CRC(1685e452) SHA1(bc299ff1cb74afcededf1a7beb9001188fdcf02f))
ROM_END
//----------------------------------------------------------------------------------------

/* Driver */

/*    YEAR  NAME          PARENT   COMPAT   MACHINE       INPUT      STATE          INIT COMPANY                         FULLNAME       FLAGS */
COMP(1982, rainbow100a, rainbow, 0, rainbow, rainbow100b_in, driver_device, 0, "Digital Equipment Corporation", "Rainbow 100-A", MACHINE_IS_SKELETON)
COMP(1983, rainbow, 0, 0, rainbow, rainbow100b_in, driver_device, 0, "Digital Equipment Corporation", "Rainbow 100-B", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_COLORS)
COMP(1985, rainbow190, rainbow, 0, rainbow, rainbow100b_in, driver_device, 0, "Digital Equipment Corporation", "Rainbow 190-B", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_COLORS)
