/***************************************************************************************************
    DEC Rainbow 100

    Driver-in-progress by R. Belmont and Miodrag Milanovic.
    Portions (2013-2014) by Karl-Ludwig Deisenhofer (VT attributes, preliminary floppy, keyboard, DIP switches).

    STATE AS OF JUNE 2014
    ---------------------
    Driver is based entirely on the DEC-100 'B' variant (DEC-190 and DEC-100 A models are treated as clones).
    While this is OK for the compatible -190, it doesn't do justice to ancient '100 A' hardware.
RBCONVERT.ZIP has details on how model 'A' differs from version B.

Issues with this driver:
    (1) Keyboard emulation incomplete (inhibits the system from booting with ERROR 50).

(2) IRQ / DMA / arbitration logic should be verified
    DMA (needed for 'Extended communication option') or Z80-8088 arbitration is non-existent (E11/E13 dumps anyone?).

(3) Read errors when booting CPM 2 / DOS 2.x / DOS 3 (secondary boot loader finds errors on tracks >= 2).
Seek (+ verify) and a number of signals (TRACK > 43 TG43, INDEX etc.) appear to be incorrect (-> diag.disk aborts drive test).
UCSD systems (fort_sys, pas_sys) and diag disks boot!

    - NOT WORKING: serial (ERROR 60).
    - NOT WORKING: printer interface (ERROR 40). Like error 60 not mission-critical.

    - NON-CRITICAL: watchdog logic (MHFU - triggered after 108 ms without interrupts on original machine) does not work.
                    The timer is reset by TWO sources: the VERT INT L from the DC012, or the MHFU ENB L from the enable flip-flop.
                    MHFU gets active if the 8088 has not acknowledged a video processor interrupt within approx. 108 milliseconds

    - TO BE IMPLEMENTED AS SLOT DEVICES (for now, DIP settings affect 'system_parameter_r' only and are disabled):
            * Color graphics option (uses NEC upd7220 GDC).         REFERENCE: Programmer's Reference: AA-AE36A-TV.
            Either 384 x 240 x 16 or 800 x 240 x 4 colors (out of 4096). 8 ? 64 K video RAM. Pallette limited to 4 colors on 100-A.

            * Extended communication option (occupies BUNDLE_OPTION ports)  REFERENCE: AA-V172A-TV + Addendum AV-Y890A-TV.
            See also NEWCOM1.DOC in RBETECDOC.ZIP.   Board connected to the front rightmost expansion slot (1 of the expansion
            ports used by the hard disk controller). Thus can't be added to a system that includes the DEC RD50/51.
            => 2 ports, a high-speed RS-422 half-duplex interface (port A) + lower-speed RS-423 full/half-duplex interface
            with modem control (port B). A DMA mode allowed high-speed transfer of data into and out of the Rainbow's memory.

    - OTHER HARDWARE UPGRADES:
            * Suitable Solutions ClikClok (battery backed real time clock)
                    Plugs into the NVM chip socket on a 100-A and into the Boot ROM on the 100-B (there is a socket
                    on the ClikClok for the NVM / ROM chip). Came with software for 'all versions of MS-DOS'.

            * 8087  Numerical Data Coprocessor daughterboard.       REFERENCE: EK-PCNDP-IN-PRE
                    Daughterboard, to be plugged into the expansion port where the memory expansion card usually sits.
                    If a memory adapter board is present, it has to be plugged into a connector atop the 8087 copro board.
                    The 8088 is put into the CPU socket on the coprocessor board.
                    => see MATH test on 'Design Maturity Diagnostics' disk <=

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

             => the 'leaked' DOS 3.10 Beta -for Rainbow- 'should not be used' on rigs with NEC V20. It possibly wasn't tested, but boots and runs well.
             => on the NEC, auto detection (of option RAM) fails with the original V20 patch (above, left)
                Expect RAM related system crashes after swapping CPUs and altering physical RAM _afterwards_.
                Hard coded CPU loops are to blame. Try values from the alternate patch (right).
             => AAD/AAM - Intel 8088 honors the second byte (operand), NEC V20 ignores it and always uses base 0Ah (10).
             => UNDOCUMENTED: NEC V20 does not have "POP CS" (opcode 0F). There are more differences (opcode D6; the 2 byte POP: 8F Cx; FF Fx instructions)
             => NEW OPCODES: REPC, REPNC, CHKIND, PREPARE, DISPOSE; BCD string operations (ADD4S, CMP4S, SUB4S), bit-ops (NOT, SET, TEST, ROL4, ROR4)
                WARNING: undoc'd opcodes, INS, EXT and 8080 behaviour are unemulated yet! MESS' CPU source has up-to-date info.

    Meaning of Diagnostics LEDs (from PC100ESV1.PDF found, e.g.,
    on ftp://ftp.update.uu.se/pub/rainbow/doc/rainbow-docs/

    Internal Diagnostic Messages                               F
    Msg Message                               Lights Display   A
    No.                                       * = on o = off   T
                                              - = on or off    A
                                              1 2 3 4 5 6 7    L
    --------------------------------------------------------------
     1  Main Board (Video)                    o * * o * o *   Yes
     2  Main Board* (unsolicited interrupt)   * * * * o * o   Yes
     3  Drive A or B (index)                  o o * o o * *
     4  Drive A or B (motor)                  * * o o o * *
     5  Drive A or B (seek)                   o * o o o * *
     6  Drive A or B (read)                   * o o o o * *
     7  Drive A or B (restore)                o * * o o * *
     8  Drive A or B (step)                   * o * o o * *
     9  System Load incomplete+ (System Load) o o o o o o o
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
----------

DEC-100 model B
= part no.70-19974-02 according to document EK-RB100-TM_001

PCB # 5416206 / 5016205-01C1:

        7-6-5-4 |3-2-1
        DIAGNOSTIC-LEDs |J3   | |J2     | |J1    |
|------|----8088|Z80-|--|VIDEO|-|PRINTER|-|SERIAL|---|
|  2 x 64 K             |/KBD.|                 !!!!!|
|  R  A  M             NEC D7201C            |P|!W90!|
|                                            |O|!!!!!|
|   [W6]   ROM 1       INTEL 8088            |W|     |
|          (23-020e5-00)                     |E|     |
|                                            |R|     |
| ...J5..  BOOT ROM 0      ...J4...          =J8     |
| ...J6... (23-022e5-00)                             |
| [W5]                                               |
|                                                    |
|     INTEL 8251A   ZILOG Z 80A                      |
|                [W18]                               |
| A  4x                74 LS 244                     |
| M  S           [W15]                               |
| 9  -   DEC-DC011     74 LS 245                     |
| 1  R           [W14]                               |
| 2  A                  [W13]                        |
| 8  M   CHARGEN.-                                   |
|        ROM (4K)           ...J7...  | ...J9 = RX50 |
|------------PCB# 5416206 / 5016205-01C1-------------|
NOTES
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

// Define standard and maximum RAM sizes (A, then B model):
//#define BOARD_RAM 0x0ffff  // 64 K base RAM  (100-A)
//#define END_OF_RAM 0xcffff // very last byte (100-A) DO NOT CHANGE.

// DEC-100-B probes until a 'flaky' area is found (BOOT ROM around F400:0E04).
// It is no longer possible to key in the RAM size from within the 100-B BIOS.
#define BOARD_RAM 0x1ffff  // 128 K base RAM (100-B)
#define END_OF_RAM 0xdffff // very last byte (100-B) DO NOT CHANGE.

// TROUBLESHOOTING RAM
// Unexpected low RAM sizes are an indication of option RAM (at worst: 128 K on board) failure.
// While motherboard errors often render the system unbootable, bad option RAM (> 128 K)
// can be narrowed down with the Diagnostic Disk and codes from the 'Pocket Service Guide'
// EK-PC100-PS-002 (APPENDIX B.2.2); pc100ps2.pdf

// WORKAROUNDS:
// (1) FORCE LOGO: - not valid for 100-A ROM -
//#define FORCE_RAINBOW_B_LOGO

// ----------------------------------------------------------------------------------------------
#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/z80/z80.h"
#include "video/vtvideo.h"

#include "machine/wd_fdc.h"
#include "formats/rx50_dsk.h"
#include "imagedev/flopdrv.h"

#include "machine/i8251.h"
#include "machine/clock.h"
#include "machine/dec_lk201.h"
#include "machine/nvram.h"

#include "rainbow.lh" // BEZEL - LAYOUT with LEDs for diag 1-7, keyboard 8-11 and floppy 20-23

#define LK201_TAG   "lk201"
#define FD1793_TAG  "fd1793x"
#define INVALID_DRIVE 255

class rainbow_state : public driver_device
{
public:
	rainbow_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_inp1(*this, "W13"),
		m_inp2(*this, "W14"),
		m_inp3(*this, "W15"),
		m_inp4(*this, "W18"),
		m_inp5(*this, "BUNDLE OPTION"),
		m_inp6(*this, "FLOPPY CONTROLLER"),
		m_inp7(*this, "GRAPHICS OPTION"),
		m_inp8(*this, "MEMORY PRESENT"),
		m_inp9(*this, "MONITOR TYPE"),
		m_inp10(*this, "J17"),

		m_crtc(*this, "vt100_video"),
		m_i8088(*this, "maincpu"),
		m_z80(*this, "subcpu"),
		m_fdc(*this, FD1793_TAG),
		m_kbd8251(*this, "kbdser"),
		m_lk201(*this, LK201_TAG),
		m_p_ram(*this, "p_ram"),

		m_p_vol_ram(*this, "vol_ram"),
		m_p_nvram(*this, "nvram"),

		m_shared(*this, "sh_ram"),
		m_maincpu(*this, "maincpu")
	{
	}

	DECLARE_READ8_MEMBER(read_video_ram_r);
	DECLARE_WRITE_LINE_MEMBER(clear_video_interrupt);

	DECLARE_READ8_MEMBER(diagnostic_r);
	DECLARE_WRITE8_MEMBER(diagnostic_w);

	DECLARE_READ8_MEMBER(comm_control_r);
	DECLARE_WRITE8_MEMBER(comm_control_w);

	DECLARE_READ8_MEMBER(share_z80_r);
	DECLARE_WRITE8_MEMBER(share_z80_w);

	DECLARE_READ8_MEMBER(hd_status_68_r);

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

	UINT32 screen_update_rainbow(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	DECLARE_WRITE_LINE_MEMBER(write_keyboard_clock);
	TIMER_DEVICE_CALLBACK_MEMBER(motor_tick);

	DECLARE_FLOPPY_FORMATS( floppy_formats );
protected:
	virtual void machine_start();

private:
	enum
	{
		IRQ_8088_MAILBOX = 0,   // vector 0x27/a7   (lowest priority)
		IRQ_8088_KBD,           // vector 0x26/a6   KEYBOARD (8251A) Interrupt
//      IRQ_EXT_COMM,           // vector 0x25      [OPTION BOARD] Interrupt from external COMM.BOARD (non DMA)
//      IRQ_COMM_PRN_7201,      // vector 0x24      COMM./PRINTER (7201) IRQ
//      IRQ_EXT_DMAC,           // vector 0x23      [OPTION BOARD] : from external COMM.BOARD (DMA Control IRQ)
//      IRQ_GRF,                // vector 0x22      GRAPHICS IRQ
//      IRQ_SH_10_BDL,          // vector 0x21      BUNDLE IRQ (hard disc / COMM.BOARD) : Pin 23 of expansion connector J4
		IRQ_8088_VBL,           // vector 0x20/a0   (highest priority)
		IRQ_8088_MAX
	};

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
	required_device<rainbow_video_device> m_crtc;
	required_device<cpu_device> m_i8088;
	required_device<cpu_device> m_z80;
	required_device<fd1793_t> m_fdc;
	required_device<i8251_device> m_kbd8251;
	required_device<lk201_device> m_lk201;
	required_shared_ptr<UINT8> m_p_ram;
	required_shared_ptr<UINT8> m_p_vol_ram;
	required_shared_ptr<UINT8> m_p_nvram;
	required_shared_ptr<UINT8> m_shared;
	required_device<cpu_device> m_maincpu;

	void raise_8088_irq(int ref);
	void lower_8088_irq(int ref);
	void update_8088_irqs();

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
};

FLOPPY_FORMATS_MEMBER( rainbow_state::floppy_formats )
	FLOPPY_TD0_FORMAT,
	FLOPPY_RX50IMG_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( rainbow_floppies )
	SLOT_INTERFACE("525qd", FLOPPY_525_SSQD)
SLOT_INTERFACE_END


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

	rom[0xf4000 + 0x364a]= 2 + 8;  // 2 :set ; 4 : reset, 8 : set for 0xf4363 ( 0363 WAIT_FOR_BIT3__loc_35E )

	rom[0xf4000 + 0x0363]= 0x90;
	rom[0xf4000 + 0x0364]= 0x90;

	// If bit 2 = 1 (Efff9), then a keyboard powerup is necessary (=> will lock up in current state)
	rom[0xf4000 + 0x3638]= 0x80;  // OR instead of TEST
	rom[0xf4000 + 0x3639]= 0x0f;  // OR instead of TEST
	rom[0xf4000 + 0x363a]= 0x08;  // 04 => 08

	rom[0xf4000 + 0x363b]= 0xeb;  // COND => JMPS

	if (rom[0xf4174] == 0x75)
	{
		rom[0xf4174] = 0xeb; // jmps  RAINBOW100_LOGO__loc_33D
		rom[0xf4175] = 0x08;
	}

	if (rom[0xf4000 + 0x3ffc] == 0x31) // 100-B
		rom[0xf4384] = 0xeb; // JMPS  =>  BOOT80

	if (rom[0xf4000 + 0x3ffc] == 0x35) // v5.05
		rom[0xf437b] = 0xeb;
#endif

}

static ADDRESS_MAP_START( rainbow8088_map, AS_PROGRAM, 8, rainbow_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x0ffff) AM_RAM AM_SHARE("sh_ram")
	AM_RANGE(0x10000, END_OF_RAM) AM_RAM

	// Documentation claims there is a 256 x 4 bit NVRAM from 0xed000 to 0xed040 (*)
	//   shadowed at $ec000 - $ecfff and from $ed040 - $edfff.

	//  - PC-100 A might have had a smaller (NV-)RAM (*)
	//  - ED000 - ED0FF is the area the _DEC-100-B BIOS_ accesses - and checks.

	//  - Specs say that the CPU has direct access to volatile RAM only.
	//    So NVRAM is hidden now and loads & saves are triggered within the
	//    'diagnostic_w' handler (similar to real hardware).

	//  - Address bits 8-12 are ignored (-> AM_MIRROR).
	AM_RANGE(0xed000, 0xed0ff) AM_RAM AM_SHARE("vol_ram") AM_MIRROR(0x1f00)
	AM_RANGE(0xed100, 0xed1ff) AM_RAM AM_SHARE("nvram")

	AM_RANGE(0xee000, 0xeffff) AM_RAM AM_SHARE("p_ram")
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( rainbow8088_io , AS_IO, 8, rainbow_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE (0x00, 0x00) AM_READWRITE(i8088_latch_r, i8088_latch_w)

	// 0x02 Communication status / control register (8088)
	AM_RANGE (0x02, 0x02) AM_READWRITE(comm_control_r, comm_control_w)

	AM_RANGE (0x04, 0x04) AM_DEVWRITE("vt100_video", rainbow_video_device, dc011_w)

	// TODO: unmapped [06] : Communication bit rates (see page 21 of PC 100 SPEC)

	AM_RANGE (0x08, 0x08) AM_READ(system_parameter_r)

	AM_RANGE (0x0a, 0x0a) AM_READWRITE(diagnostic_r, diagnostic_w)

	AM_RANGE (0x0c, 0x0c) AM_DEVWRITE("vt100_video", rainbow_video_device, dc012_w)

	// TODO: unmapped [0e] : PRINTER BIT RATE REGISTER (WO)

	AM_RANGE(0x10, 0x10) AM_DEVREADWRITE("kbdser", i8251_device, data_r, data_w)
	AM_RANGE(0x11, 0x11) AM_DEVREADWRITE("kbdser", i8251_device, status_r, control_w)

	// UNMAPPED:
	// 0x20 -> 0x2f ***** EXTENDED COMM. OPTION / Option Select 1.
	// See boot rom @1EA6: 0x27 (<- RESET EXTENDED COMM OPTION  )
	// ===========================================================
	// 0x30 -> 0x3f ***** EXTENDED COMM. OPTION / Option Select 3.
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
	// TODO: hard disc emulation!
	// ------ Rainbow uses 'WD 1010 AL' (Western Digital 1983)
	//        Register compatible to WD2010 (present in MESS)
	// R/W REGISTERS 60 - 68 (?)
	AM_RANGE (0x68, 0x68) AM_READ(hd_status_68_r)
	// ===========================================================
	// HARD DISC SIZES AND LIMITS
	//   HARDWARE:
	//      Controller has a built-in limit of 8 heads / 1024 cylinders (67 MB). Standard geometry is 4 surfaces!
	//   BOOT LOADERS:
	//   - the DEC boot loader (and FDISK from DOS 3.10) initially allowed a maximum hard disc size of 20 MB.
	//   - the custom boot loader that comes with 'WUTIL 3.2' allows 117 MB and 8 surfaces.
	//   SOFTWARE:
	//   - MS-DOS 2 allows a maximum partition size of 16 MB (sizes > 15 MB are incompatible to DOS 3)
	//   - MS-DOS 3 has a global 1024 cylinder limit (32 MB).
	// ===========================================================
	// 0x70 -> 0x7f ***** EXTENDED COMM. OPTION / Option Select 4.
	// ===========================================================
ADDRESS_MAP_END

static ADDRESS_MAP_START(rainbowz80_mem, AS_PROGRAM, 8, rainbow_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0xffff ) AM_READWRITE(share_z80_r, share_z80_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( rainbowz80_io, AS_IO, 8, rainbow_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(z80_latch_r, z80_latch_w)
	AM_RANGE(0x20, 0x20) AM_READWRITE(z80_generalstat_r, z80_diskdiag_read_w) // read to port 0x20 used by MS-DOS 2.x diskette loader.
	AM_RANGE(0x21, 0x21) AM_READWRITE(z80_generalstat_r, z80_diskdiag_write_w)
	AM_RANGE(0x40, 0x40) AM_READWRITE(z80_diskstatus_r, z80_diskcontrol_w)
	AM_RANGE(0x60, 0x63)   AM_DEVREADWRITE(FD1793_TAG, fd1793_t, read, write)
ADDRESS_MAP_END

/* Input ports */

/* DIP switches */
static INPUT_PORTS_START( rainbow100b_in )

		PORT_START("MONITOR TYPE")
		PORT_DIPNAME( 0x03, 0x03, "MONOCHROME MONITOR")
PORT_DIPSETTING(0x01, "WHITE")
		PORT_DIPSETTING(    0x02, "GREEN" )
		PORT_DIPSETTING(    0x03, "AMBER" )

	// MEMORY, FLOPPY, BUNDLE, GRAPHICS affect 'system_parameter_r':
		PORT_START("MEMORY PRESENT")
		PORT_DIPNAME( 0xF0000, 0x20000, "MEMORY PRESENT")
		PORT_DIPSETTING(    0x10000, "64  K (MINIMUM ON 100-A)" ) // see BOARD_RAM
		PORT_DIPSETTING(    0x20000, "128 K (MINIMUM ON 100-B)" )
		PORT_DIPSETTING(    0x30000, "192 K (MEMORY OPTION)" )
		PORT_DIPSETTING(    0x40000, "256 K (MEMORY OPTION)" )
		PORT_DIPSETTING(    0x50000, "320 K (MEMORY OPTION)" )
		PORT_DIPSETTING(    0x60000, "384 K (MEMORY OPTION)" )
		PORT_DIPSETTING(    0x70000, "448 K (MEMORY OPTION)" )
		PORT_DIPSETTING(    0x80000, "512 K (MEMORY OPTION)" )
		PORT_DIPSETTING(    0x90000, "576 K (MEMORY OPTION)" )
		PORT_DIPSETTING(    0xA0000, "640 K (MEMORY OPTION)" )
		PORT_DIPSETTING(    0xB0000, "704 K (MEMORY OPTION)" )
		PORT_DIPSETTING(    0xC0000, "768 K (MEMORY OPTION)" )
		PORT_DIPSETTING(    0xD0000, "832 K (MAX.MEM.ON -A)" ) // see END_OF_RAM
		PORT_DIPSETTING(    0xE0000, "896 K (MAX.MEM.ON -B)" )

	// Floppy is always 'on', BUNDLE + GRAPHICS are not implemented yet:
		PORT_START("FLOPPY CONTROLLER")
		PORT_DIPNAME( 0x01, 0x01, "FLOPPY CONTROLLER") PORT_TOGGLE
		PORT_DIPSETTING(    0x01, DEF_STR( On ) )

	//  BUNDLE_OPTION: EXT.COMM.card -or- hard disk controller (marketed later).
	//         - hard disc and COMM.extension exclude each other!
	//         - connector J4 has 4 select lines (Option Select 1-4)
		PORT_START("BUNDLE OPTION")
		PORT_DIPNAME( 0x00, 0x00, "BUNDLE OPTION") PORT_TOGGLE
		PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
		PORT_DIPSETTING(    0x01, DEF_STR( On ) )

		PORT_START("GRAPHICS OPTION")
		PORT_DIPNAME( 0x00, 0x00, "GRAPHICS OPTION") PORT_TOGGLE
		PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
		PORT_DIPSETTING(    0x01, DEF_STR( On ) )

	// W13 - W18 are used for factory tests and affect the boot process -
	PORT_START("W13")
		PORT_DIPNAME( 0x02, 0x02, "W13 (FACTORY TEST A, LEAVE OFF)") PORT_TOGGLE
		PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
		PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("W14")
		PORT_DIPNAME( 0x04, 0x04, "W14 (FACTORY TEST B, LEAVE OFF)") PORT_TOGGLE
		PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
		PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("W15")
		PORT_DIPNAME( 0x08, 0x08, "W15 (FACTORY TEST C, LEAVE OFF)") PORT_TOGGLE
		PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
		PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	// DSR = 1 when switch is OFF - see i8251.c (status_r)
	PORT_START("W18")
		PORT_DIPNAME( 0x01, 0x00, "W18 (FACTORY TEST D, LEAVE OFF) (8251A: DSR)") PORT_TOGGLE
		PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
		PORT_DIPSETTING(    0x01, DEF_STR( On ) )
		PORT_WRITE_LINE_DEVICE_MEMBER("kbdser", i8251_device, write_dsr)

	// J17 jumper on FDC controller board shifts drive select (experimental) -
	PORT_START("J17")
		PORT_DIPNAME( 0x02, 0x00, "J17 DRIVE SELECT (A => C and B => D)") PORT_TOGGLE
		PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
		PORT_DIPSETTING(    0x02, DEF_STR( On ) )
INPUT_PORTS_END


void rainbow_state::machine_reset()
{
	m_unit = INVALID_DRIVE;

	m_fdc->reset();
	m_fdc->set_floppy(NULL);
	m_fdc->dden_w(0);
	//m_fdc->set_force_ready(false);

	/* configure RAM */
	address_space &program = m_maincpu->space(AS_PROGRAM);
	if (m_inp8->read() < END_OF_RAM)
	{
		program.unmap_readwrite(m_inp8->read(), END_OF_RAM);
	}

	// BIOS can't handle soft resets (=> triggers ERROR 16).
	if ( COLD_BOOT == 2 )
	{   // As a fallback, execute a hard reboot -
		device().machine().schedule_hard_reset();
	}

	if ( COLD_BOOT == 1 )
	{
		COLD_BOOT = 2;
		m_crtc->MHFU(-100); // reset MHFU counter
	}

	m_z80->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	INT88 = false;
	INTZ80 = false;

	m_zflip = true;
	m_z80_halted = true;
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


UINT32 rainbow_state::screen_update_rainbow(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_crtc->palette_select( m_inp9->read() );

	if ( m_SCREEN_BLANK )
			m_crtc->video_blanking(bitmap, cliprect);
	else
			m_crtc->video_update(bitmap, cliprect);
	return 0;
}

// Interrupt handling and arbitration.  See 3.1.3.8 OF PC-100 spec.
void rainbow_state::update_8088_irqs()
{
	static const int vectors[] = { 0x27, 0x26, 0x20 };

	if (m_irq_mask != 0)
	{
		for (int i = 0; i < IRQ_8088_MAX; i++)
		{
			if (m_irq_mask & (1<<i))
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
	m_irq_mask |= (1<<ref);
	update_8088_irqs();
}

void rainbow_state::lower_8088_irq(int ref)
{
	m_irq_mask &= ~(1<<ref);
	update_8088_irqs();
}

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
			return m_z80_private[offset & 0x7ff];
		}

		return m_shared[offset ^ 0x8000];
	}
	else
	{
		if (offset < 0x800)
		{
			return m_z80_private[offset];
		}

		return m_shared[offset];
	}

	// never executed
	//return 0xff;
}

WRITE8_MEMBER(rainbow_state::share_z80_w)
{
	if (m_zflip)
	{
		if (offset < 0x8000)
		{
			m_shared[offset + 0x8000] = data;
		}
		else if (offset < 0x8800)
		{
			m_z80_private[offset & 0x7ff] = data;
		}

		m_shared[offset ^ 0x8000] = data;
	}
	else
	{
		if (offset < 0x800)
		{
			m_z80_private[offset] = data;
		}
		else
		{
			m_shared[offset] = data;
		}
	}
}

// Until a full-blown hard-disc emulation evolves, deliver an error message:
READ8_MEMBER(rainbow_state::hd_status_68_r)
{
	// Top 3 bits = status / error code
	// SEE ->   W_INCHESTER__loc_80E

	// return 0xa0; // A0 : OK, DRIVE IS READY (!)

	return 0xe0; //  => 21 DRIVE NOT READY (BIOS; when W is pressed on boot screen)
}

READ8_MEMBER(rainbow_state::system_parameter_r)
{
/*  Info about option boards is in bits 0 - 3:
    SYSTEM PARAMETER INFORMATION: see AA-P308A-TV page 92 section 14.0
    Bundle card (1) | Floppy (2) | Graphics (4) | Memory option (8)
    0 1 2 3 4 5 6 7
    B F G M
    ( 1 means NOT present; 4-7 reserved )
*/
	return  ( ((m_inp5->read() == 1)        ? 0 : 1)  |
			((m_inp6->read() == 1)      ? 0 : 2)  |
			((m_inp7->read() == 1)      ? 0 : 4)  |
				((m_inp8->read() > BOARD_RAM)   ? 0 : 8)  |
				16 | 32 | 64 | 128 // to be verified.
		);
}

READ8_MEMBER(rainbow_state::comm_control_r)
{
/*  [02] COMMUNICATIONS STATUS REGISTER - PAGE 154 (**** READ **** )
    Used to read status of SERIAL port, IRQ line of each CPU, and MHFU logic enable signal.

//    What the specs says on how MHFU detection is disabled:
//    1.  by first disabling interrupts with CLI
//    2.  by writing 0x00 to port 0x10C (handled by 'dc012_w' in vtvideo)
//   (3.) MHFU is re-enabled by writing to 0x0c (or automatically after STI - when under BIOS control ?)
*/
	// During boot phase 2, do not consider MHFU ENABLE. Prevents ERROR 16.
	int data;
	if (COLD_BOOT == 2)
		data = 0;
	else
		data = m_crtc->MHFU(1);

	return (  ( (data > 0) ? 0x00 : 0x20) |// (L): status of MHFU flag => bit pos.5
					(   (INT88)    ? 0x00 : 0x40 ) |               // (L)
					(   (INTZ80)   ? 0x00 : 0x80 )                 // (L)
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

// 8088 reads port 0x00. See page 133 (4-34)
READ8_MEMBER(rainbow_state::i8088_latch_r)
{
//    printf("Read %02x from 8088 mailbox\n", m_8088_mailbox);
	lower_8088_irq(IRQ_8088_MAILBOX);

	INT88 = false; // BISLANG:  INTZ80 = false; //
	return m_8088_mailbox;
}

// 8088 writes port 0x00. See page 133 (4-34)
WRITE8_MEMBER(rainbow_state::i8088_latch_w)
{
//    printf("%02x to Z80 mailbox\n", data);
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

	INTZ80 = false; // BISLANG: INT88 = false;
	return m_z80_mailbox;
}

// Z80 writes to port 0x00
// See page 134 (4-35)
WRITE8_MEMBER(rainbow_state::z80_latch_w)
{
	//    printf("%02x to 8088 mailbox\n", data);
	raise_8088_irq(IRQ_8088_MAILBOX);
	m_8088_mailbox = data;

	INT88 = true;
}

// WRITE to 0x20
WRITE8_MEMBER(rainbow_state::z80_diskdiag_read_w)
{
	m_zflip = true;
}

// (Z80) : PORT 21H  _READ_
READ8_MEMBER(rainbow_state::z80_generalstat_r)
{
/*
General / diag.status register Z80 / see page 157 (table 4-18).

D7 : STEP L : reflects status of STEP signal _FROM FDC_
      (when this 2us output pulse is low, the stepper will move into DIR)
D6 : WRITE GATE L :reflects status of WRITE GATE signal _FROM FDC_
      (asserted low before data can be written on the diskette)
D5 : TR00: reflects status of TRACK 0 signal (= 1) * from the disk drive *
D4 : DIR L: reflects status of DIRECTION signal * FROM FDC * to disk
      (when low, the head will step towards the center)
D3 : READY L: reflects status of READY L signal * from the disk drive *
     (low active, asserts when disk is inserted and door is closed)
D2 : INT88 L: (bit reads the INT88 bit sent by Z80 to interrupt 8088)
D1 : INTZ80 L: (bit reads the INTZ80 bit sent by 8088 to interrupt Z80)
D0 : ZFLIP L: (read from the diagnostic control register of Z80A)
*/
	static int last_track;
	int track = m_fdc->track_r( space, 0);
	int fdc_step = 0;
	int fdc_ready = 0;
	int tk00 = 0;

	if(m_unit != INVALID_DRIVE)
	{
		if (track != last_track)
			fdc_step = 1;
		last_track = track;

		fdc_ready = m_floppy->ready_r();
		tk00 = ( m_floppy->trk00_r() != CLEAR_LINE );
	}

	int last_dir = 0;      // FAKE LAST_DIR + WRITE_GATE FOR NOW.
	int fdc_write_gate = 1;

		// ***** ALL LOW ACTIVE - EXCEPT tk00 :
	int data=(
				(  (fdc_step)             ? 0x00 : 0x80 )        |
				(  (fdc_write_gate == 1)  ? 0x00 : 0x40 )          |
				(  (tk00)                 ? 0x20 : 0x00 )        |
				(  (last_dir == 1)        ? 0x00 : 0x10 )        |
				(  (fdc_ready)            ? 0x00 : 0x08 )        |
				(   (INT88)    ? 0x00 : 0x04 )                     |
				(   (INTZ80)   ? 0x00 : 0x02 )                     |
				(  (m_zflip)  ? 0x00 : 0x01 )
			);

	return data;
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

// (Z80) : PORT 40H _READ_

// **********************************************************************
//  POLARITY OF _DRQ_ AND _IRQ_ (depends on controller type!)
// **********************************************************************
READ8_MEMBER(rainbow_state::z80_diskstatus_r)
{
	static int last_track;
	int track = m_fdc->track_r( space, 0);

	if (track != last_track)
		printf("\n%02d",track);
	last_track = track;

	// 40H diskette status Register **** READ ONLY *** ( 4-60 of TM100.pdf )

	// AND 00111011 - return what was WRITTEN to D5-D3, D1, D0 previously
	//                (except D7,D6,D2)
	int data = m_z80_diskcontrol & 0x3b;

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

	// D2: TG43 L :  0 = INDICATES TRACK > 43 SIGNAL FROM FDC TO DISK DRIVE.
	if ( track > 43)
		data = data & (255 - 4);
	else
		data = data | 4;

	// D1: DS1 H: reflect status of bits 0 and 1 form disk.control reg.
	// D0: DS0 H: "
	return data;
}

// (Z80) : PORT 40H  * WRITE *
// RX-50 has head A and head B (1 for each of the 2 disk slots in a RX-50).
// TODO: find out how head load and drive select really work.
WRITE8_MEMBER(rainbow_state::z80_diskcontrol_w)
{
	// FORCE_READY = 0 : assert DRIVE READY on FDC (diagnostic override; USED BY BIOS!)
	//bool force_ready = ( (data & 4) == 0 ) ? true : false;

	int drive;
	if ( m_inp10->read() && ((data & 3) < 2) )
		drive = (data & 1) + 2;
	else
		drive = data & 3;

	int selected_drive = INVALID_DRIVE;
	static const char *names[] = { FD1793_TAG ":0", FD1793_TAG ":1", FD1793_TAG ":2", FD1793_TAG ":3" };

	floppy_connector *con = machine().device<floppy_connector>(names[drive]);
	if (con)
	{
		m_floppy = con->get_device();
			if (m_floppy)
			{
				selected_drive = drive;

				m_fdc->set_floppy(m_floppy);  // Sets new  _image device_
				m_floppy->set_rpm(300.);

				// RX50 board has additional 'side select' - ignored by WD emulation?
				m_floppy->ss_w((data & 20) ? 1 : 0);
			}
	}

	output_set_value("driveled0",  (selected_drive == 0) ? 1 : 0 );
	output_set_value("driveled1",  (selected_drive == 1) ? 1 : 0 );

	output_set_value("driveled2",  (selected_drive == 2) ? 1 : 0 );
	output_set_value("driveled3",  (selected_drive == 3) ? 1 : 0 );

	if (selected_drive < 4)
	{
			m_unit = selected_drive;

			if (MOTOR_DISABLE_counter == 0) // "one shot"
				MOTOR_DISABLE_counter = 4800; // 2400 = 500 ms

//          m_fdc->set_force_ready(force_ready);
	}
	else
	{
//          m_fdc->set_force_ready(false);
	}

			for(int f_num=0; f_num <= 3; f_num++)
			{
				floppy_connector *con = machine().device<floppy_connector>(names[f_num]);
				floppy_image_device *tmp_floppy = con->get_device();
				tmp_floppy->mon_w( (f_num == m_unit) ? CLEAR_LINE : ASSERT_LINE );
	}

	if(m_unit == INVALID_DRIVE)
	{
		data = data & (255 -3);
		data = data | 8;  // MOTOR 0 OFF
		data = data | 16; // MOTOR 1 OFF
	}
	else
	{
		data = (data & (255 - 3)) | m_unit;

		if(m_unit < 2)
			data = data & (255 - 8); // MOTOR 0 (for A or B)

		if(m_unit > 1)
			data = data & (255 - 16); // MOTOR 1 (for C or D)
			}
	m_z80_diskcontrol = data;
}

READ8_MEMBER( rainbow_state::read_video_ram_r )
{
	return m_p_ram[offset];
}

INTERRUPT_GEN_MEMBER(rainbow_state::vblank_irq)
{
	raise_8088_irq(IRQ_8088_VBL);
	m_crtc->notify_vblank(true);
}

WRITE_LINE_MEMBER( rainbow_state::clear_video_interrupt )
{
	lower_8088_irq(IRQ_8088_VBL);
	m_crtc->notify_vblank(false);
}

READ8_MEMBER( rainbow_state::diagnostic_r ) // 8088 (port 0A READ). Fig.4-29 + table 4-15
{
//    printf("%02x DIP value ORed to diagnostic\n", ( m_inp1->read() | m_inp2->read() | m_inp3->read()   )  );

	return ( (m_diagnostic & (0xf1)) | (    m_inp1->read() |
											m_inp2->read() |
											m_inp3->read()
										)
			);
}

WRITE8_MEMBER( rainbow_state::diagnostic_w ) // 8088 (port 0A WRITTEN). Fig.4-28 + table 4-15
{
//    printf("%02x to diag port (PC=%x)\n", data, space.device().safe_pc());
	m_SCREEN_BLANK = (data & 2) ? false : true;


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

	if(m_unit != INVALID_DRIVE)
	{
		if ( (m_diagnostic & 1) && !(data & 1) )
		{
			m_fdc->soft_reset();
		}

		if ( !(m_diagnostic & 1) && (data & 1) )
		{
			m_fdc->soft_reset(); // See formatter description p.197 or 5-13
		}
	}
	// MISSING BITS (* not vital for normal operation, see diag.disk) -
	// BIT 2: GRF VID SEL (0 = system module; 1 = graphics option)
	// BIT 3: PARITY TEST (1 = enables parity test on memory option board)
	// * BIT 4: DIAG LOOPBACK (0 at power-up; 1 directs RX50 and DC12 output to printer port)
	// * BIT 5: PORT LOOPBACK (1 enables loopback for COMM, PRINTER, KEYBOARD ports)

	// BIT 6: Transfer data from volatile memory to NVM
	if ( !(data & 0x40)  && (m_diagnostic & 0x40) )
		memcpy( m_p_nvram, m_p_vol_ram, 256);

	// BIT 7: Transfer data from NVM to volatile memory
	if ( (data & 0x80)  && !(m_diagnostic & 0x80) )
		memcpy( m_p_vol_ram, m_p_nvram, 256);

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
	if (MOTOR_DISABLE_counter)
		MOTOR_DISABLE_counter--;

	if (MOTOR_DISABLE_counter == 1)
	{
			output_set_value("driveled0", 0); // DRIVE 0 (A)
			output_set_value("driveled1", 0); // DRIVE 1 (B)
			output_set_value("driveled2", 0); // DRIVE 2 (C)
			output_set_value("driveled3", 0); // DRIVE 3 (D)
	}

	if ( m_crtc->MHFU(1) ) // MHFU ENABLED ?
	{
/*              int data = m_crtc->MHFU(-1); // increment MHFU, return new value
                //  if (data >  480) ...
                //     m_crtc->MHFU(-100);
                //     machine().schedule_hard_reset(); // not exactly a proper watchdog reset
*/
	}

	if (m_beep_counter > 0)
			m_beep_counter--;
}

// on 100-B, DTR from the keyboard 8051 controls bit 7 of IRQ vectors
WRITE_LINE_MEMBER(rainbow_state::irq_hi_w)
{
	m_irq_high = (state == ASSERT_LINE) ? 0x80 : 0;
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
	{ 15*8, 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( rainbow )
	GFXDECODE_ENTRY( "chargen", 0x0000, rainbow_charlayout, 0, 1 )
GFXDECODE_END


static MACHINE_CONFIG_START( rainbow, rainbow_state )
	MCFG_DEFAULT_LAYOUT(layout_rainbow)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8088, XTAL_24_0734MHz / 5)
	MCFG_CPU_PROGRAM_MAP(rainbow8088_map)
	MCFG_CPU_IO_MAP(rainbow8088_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", rainbow_state,  vblank_irq)

	MCFG_CPU_ADD("subcpu",Z80, XTAL_24_0734MHz / 6)
	MCFG_CPU_PROGRAM_MAP(rainbowz80_mem)
	MCFG_CPU_IO_MAP(rainbowz80_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(132*10, 49*10)
	MCFG_SCREEN_VISIBLE_AREA(0, 80 * 10-1, 0, 24 * 10-1)
	MCFG_SCREEN_UPDATE_DRIVER(rainbow_state, screen_update_rainbow)
	MCFG_SCREEN_PALETTE("vt100_video:palette")
	MCFG_GFXDECODE_ADD("gfxdecode", "vt100_video:palette", rainbow)

	MCFG_DEVICE_ADD("vt100_video", RAINBOW_VIDEO, 0)
	MCFG_VT_SET_SCREEN("screen")
	MCFG_VT_CHARGEN("chargen")
	MCFG_VT_VIDEO_RAM_CALLBACK(READ8(rainbow_state, read_video_ram_r))
	MCFG_VT_VIDEO_CLEAR_VIDEO_INTERRUPT_CALLBACK(WRITELINE(rainbow_state, clear_video_interrupt))

	MCFG_FD1793x_ADD(FD1793_TAG, XTAL_24_0734MHz / 24) // no separate 1 Mhz quartz
	MCFG_WD_FDC_FORCE_READY
	MCFG_FLOPPY_DRIVE_ADD(FD1793_TAG ":0", rainbow_floppies, "525qd", rainbow_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FD1793_TAG ":1", rainbow_floppies, "525qd", rainbow_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FD1793_TAG ":2", rainbow_floppies, "525qd", rainbow_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FD1793_TAG ":3", rainbow_floppies, "525qd", rainbow_state::floppy_formats)
	MCFG_SOFTWARE_LIST_ADD("flop_list","rainbow")

	MCFG_DEVICE_ADD("kbdser", I8251, 0)
	MCFG_I8251_TXD_HANDLER(WRITELINE(rainbow_state, kbd_tx))
	MCFG_I8251_DTR_HANDLER(WRITELINE(rainbow_state, irq_hi_w))
	MCFG_I8251_RXRDY_HANDLER(WRITELINE(rainbow_state, kbd_rxready_w))
	MCFG_I8251_TXRDY_HANDLER(WRITELINE(rainbow_state, kbd_txready_w))

	MCFG_DEVICE_ADD(LK201_TAG, LK201, 0)
	MCFG_LK201_TX_HANDLER(DEVWRITELINE("kbdser", i8251_device, write_rxd))

	MCFG_DEVICE_ADD("keyboard_clock", CLOCK, 4800*16) // 8251 is set to /16 on the clock input
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(rainbow_state, write_keyboard_clock))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("keyboard", rainbow_state, motor_tick, attotime::from_hz(4800*16))

	MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END

// 'Rainbow 100-A' (system module 70-19974-00, PSU H7842-A)
// - first generation hardware (introduced May '82) with ROM 04.03.11
// - 64 K base RAM on board (instead of 128 K on 'B' model).  832 K RAM max.
// - inability to boot from hard disc (mind the inadequate PSU)
// - cannot control bit 7 of IRQ vector (prevents DOS >= 2.05 from booting on unmodified hardware)
// - limited palette with color graphics option (4 instead of 16 colors)
// - smaller ROMs (3 x 2764) with fewer routines (no documented way to beep...)

// FIXME: 12-19606-02 and 12-19678-04 are just PART NUMBERS from the 100-A field manual.
// Someone who knows the DEC naming conventions should correct them -
ROM_START( rainbow100a )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD( "12-19606-02a.bin", 0xFA000, 0x2000, NO_DUMP) // ROM (FA000-FBFFF) (E89) 8 K
	ROM_LOAD( "12-19606-02b.bin", 0xFC000, 0x2000, NO_DUMP) // ROM (FC000-FDFFF) (E90) 8 K
	ROM_LOAD( "12-19678-04.bin", 0xFE000, 0x2000, NO_DUMP)  // ROM (FE000-FFFFF) (E91) 8 K

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "chargen.bin", 0x0000, 0x1000, CRC(1685e452) SHA1(bc299ff1cb74afcededf1a7beb9001188fdcf02f))
ROM_END

// ROM definition for 100-B (system module 70-19974-02, PSU H7842-D)
// Built until ~ May 1986 (see MP-01491-00)
// - 32 K ROM (version 5.03)
// - 128 K base and 896 K max. mem.
ROM_START( rainbow )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD( "23-022e5-00.bin",  0xf0000, 0x4000, CRC(9d1332b4) SHA1(736306d2a36bd44f95a39b36ebbab211cc8fea6e))
	ROM_RELOAD(0xf4000,0x4000)
	ROM_LOAD( "23-020e5-00.bin", 0xf8000, 0x4000, CRC(8638712f) SHA1(8269b0d95dc6efbe67d500dac3999df4838625d8)) // German, French, English
	//ROM_LOAD( "23-015e5-00.bin", 0xf8000, 0x4000, NO_DUMP) // Dutch, French, English
	//ROM_LOAD( "23-016e5-00.bin", 0xf8000, 0x4000, NO_DUMP) // Finish, Swedish, English
	//ROM_LOAD( "23-017e5-00.bin", 0xf8000, 0x4000, NO_DUMP) // Danish, Norwegian, English
	//ROM_LOAD( "23-018e5-00.bin", 0xf8000, 0x4000, NO_DUMP) // Spanish, Italian, English
	ROM_RELOAD(0xfc000,0x4000)
	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "chargen.bin", 0x0000, 0x1000, CRC(1685e452) SHA1(bc299ff1cb74afcededf1a7beb9001188fdcf02f))
ROM_END

// 'Rainbow 190 B' (announced March 1985) is identical to 100-B, with alternate ROM v5.05.
// According to an article in Wall Street Journal it came with a 10 MB HD and 640 K RAM.

// CHANGES: the 'Boot 2.4' manual mentions 'recent ROM changes for MASS 11' in January 1985.
// Older ROMs like 04.03.11 (for PC-100-A) or 05.03 (100-B) obviously do not incorporate these.
// => jump tables (F4000-F40083 and FC000-FC004D) were not extended.
// => absolute addresses of some internal routines have changed.
// => programs that do not rely on specific ROM versions should be compatible.
// MASS was a VAX word processor, so changes likely affected terminal emulation.

// FIXME: ROM names are made up.
// Someone who knows the DEC naming conventions should correct them -
ROM_START( rainbow190 )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD( "dec190rom0.bin",  0xf0000, 0x4000, CRC(fac191d2) SHA1(4aff5b1e031d3b5eafc568b23e68235270bb34de) )
	ROM_RELOAD(0xf4000,0x4000)
	ROM_LOAD( "dec190rom1.bin", 0xf8000, 0x4000, CRC(5ce59632) SHA1(d29793f7014c57a4e7cb77bbf6e84f9113635ed2) )

	ROM_RELOAD(0xfc000,0x4000)
	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "chargen.bin", 0x0000, 0x1000, CRC(1685e452) SHA1(bc299ff1cb74afcededf1a7beb9001188fdcf02f))
ROM_END

/* Driver */

/*    YEAR  NAME         PARENT   COMPAT  MACHINE       INPUT      STATE          INIT COMPANY                         FULLNAME       FLAGS */
COMP( 1982, rainbow100a , rainbow,      0,  rainbow, rainbow100b_in, driver_device, 0,  "Digital Equipment Corporation", "Rainbow 100-A", GAME_IS_SKELETON                        )
COMP( 1983, rainbow   , 0      ,      0,  rainbow, rainbow100b_in, driver_device, 0,  "Digital Equipment Corporation", "Rainbow 100-B", GAME_NOT_WORKING | GAME_IMPERFECT_COLORS)
COMP( 1985, rainbow190 , rainbow,      0,  rainbow, rainbow100b_in, driver_device, 0,  "Digital Equipment Corporation", "Rainbow 190-B", GAME_NOT_WORKING | GAME_IMPERFECT_COLORS)
