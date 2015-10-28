// license:BSD-3-Clause
// copyright-holders:David Haywood, Palindrome, Roberto Fresca
/****************************************************************************************************************

    Aristocrat MK5 / MKV hardware
    possibly 'Acorn Archimedes on a chip' hardware

    Note: ARM250 mapping is not identical to plain AA

    BIOS ROMs are actually nowhere to be found on a regular MK5 system. They can be used
    to change the system configurations on a PCB board by swapping them with the game ROMs
    u7/u11 locations.

    TODO (MK-5 specific):
    - Fix remaining errors
    - If all tests passes, this msg is printed on the keyboard serial port:
    "System Startup Code Entered \n Gos_create could not allocate stack for the new process \n
    Unrecoverable error occurred. System will now restart"
    Apparently it looks like some sort of protection device ...

    code DASMing of POST (adonis):
    - bp 0x3400224:
      checks work RAM [0x87000], if bit 0 active high then all tests are skipped (presumably for debugging), otherwise check stuff;
        - bp 0x3400230: EPROM checksum branch test
        - bp 0x3400258: DRAM Check branch test
        - bp 0x3400280: CPU Check branch test
            bp 0x340027c: checks IRQ status A and FIQ status bit 7 (force IRQ flag)
            - R0 == 0: CPU Check OK
            - R0 == 1: IRQ status A force IRQ flag check failed
            - R0 == 2: FIQ status force IRQ flag check failed
            - R0 == 3: Internal Latch check 0x3250050 == 0xf5
        - bp 0x34002a8: SRAM Check branch test (I2C)
            - basically writes to the I2C clock/data then read-backs it
        - bp 0x34002d0: 2KHz Timer branch test
            bp 0x34002cc: it does various test with GO command reads (that are undefined on plain AA) and
                          IRQA status bit 0, that's "printer busy" on original AA but here it have a completely
                          different meaning.
        - bp 0x34002f8: DRAM emulator branch tests
            bp 0x34002f4:
            - R0 == 0 "DRAM emulator found"
            - R0 == 1 "DRAM emulator found"
            - R0 == 3 "DRAM emulator not found - Error"
            - R0 == 4 "DRAM emulator found instead of DRAM - Error"
            - R0 == x "Undefined error in DRAM emulator area"
            It r/w RAM location 0 and it expects to NOT read-back value written.

    goldprmd: checks if a "keyboard IRQ" fires (IRQ status B bit 6), it seems a serial port with data on it,
              returns an External Video Crystal Error (bp 3400278)

    dmdtouch:
        bp 3400640: checks 2MByte DRAM
            - writes from 0x1000 to 0x100000, with 0x400 bytes index increment and 0xfb data increment
            - writes from 0x100000 to 0x200000, with 0x400 bytes index increment and 0xfb data increment
            - bp 3400720 checks if the aforementioned checks are ok (currently fails at the very first work RAM check
              at 0x1000, it returns the value that actually should be at 0x141000)
        bp 340064c: if R0 == 0 2MB DRAM is ok, otherwise there's an error

    set chip (BIOS):
        same as goldprmd (serial + ext video crystal check)
        bp 3400110: External Video Crystal test

*****************************************************************************************************************

  MKV S2 Mainboard, PCB Layout:

  +--------------------------------------------------------------------------------------------------------+
  |   |    96-pin male connector     |  |    96-pin male connector     |  |    96-pin male connector     | |
  |   +------------------------------+  +------------------------------+  +------------------------------+ |
  |            +---+       +--+                                                          +---+ +---+ +---+ |
  +-+          |VR1|       |  |U89              +------+        +------+                 |AA | |AB | |AC | |
  | |          |   |       |  |                 |AMP   |        |U35   |                 +---+ +---+ +---+ |
  |S|          |   |       |  |                 +------+        +------+                 +---+ +---+ +---+ |
  |I|          +---+       +--+ +--+                                                     |U46| |U21| |U66| |
  |M|                           |  |U52                  ARISTOCRAT                      +---+ +---+ +---+ |
  |M|                    +----+ |  |                     MKV S2 MAINBOARD                   +------------+ |
  | |    +---------+     |U47 | |  |                     PCB 0801-410091                    | 3V BATTERY | |
  |S|    |U72      |     +----+ +--+                     ASSY 2501-410389                   |            | |
  |O|    |         |       +-----+                       ISSUE A01                          +------------+ |
  |C|    |         |       |U23  |                                                                  +----+ |
  |K|    |         |       |     |                                                                  |U53 | |
  |E|    +---------+       +-----+       +------------+                                             +----+ |
  |T|                                    |U85         |    +----+ +----+ +--+ +--+ +----+ +----+ +--+ +--+ |
  | |    +---------+                     |            |    |U58 | |U54 | |U | |U | |U59 | |U61 | |U | |U | |
  | |    |U71      |                     |    CPU     |    +----+ +----+ |1 | |4 | +----+ +----+ |1 | |4 | |
  | |    |         |                     |            |           +----+ |4 | |8 |  +------+     |5 | |9 | |
  | |    |         |  +-----+            |            |           |U56 | |9 | |  |  |U36   |     |2 | |  | |
  | |    |         |  |U65  |            |            |           +----+ +--+ +--+  |      |     +--+ +--+ |
  | |    +---------+  |     |            +------------+                             +------+               |
  | |                 +-----+     +-----+ +---+                                                            |
  +-+    +---+                    |U73  | |X2 |                    +----------------+   +----------------+ |
  |      |U26|                    |     | +---+   +---+            |U14             |   |U10             | |
  |      +---+                    +-----+         |U50|            |                |   |                | |
  |      |U27|                       +-----+      +---+            +----------------+   +----------------+ |
  |      +---+                       |U5   |      |U40|            |U13             |   |U9              | |
  |                                  |     |      +---+            |                |   |                | |
  |                                  +-----+      |U41|            +----------------+   +----------------+ |
  |                                               +---+            |U12             |   |U8              | |
  |          +---+                                                 |                |   |                | |
  |          |VR2|                         +-----+         +-----+ +----------------+   +----------------+ |
  |          |   |                         |U24  |         |U22  | |U11             |   |U7              | |
  |          |   |                         |     |         |     | |                |   |                | |
  |          |   |                         +-----+         +-----+ +----------------+   +----------------+ |
  |          +---+                                            +----------------------------------+         |
  |                                                           |     96-pin female connector      |         |
  +--------------------------------------------------------------------------------------------------------+

  U5: 48 MHz crystal (unpopulated from factory).

  U7:  27C4096 ROM socket (bank 0).
  U8:  27C4096 ROM socket (bank 1).
  U9:  27C4096 ROM socket (bank 2).
  U10: 27C4096 ROM socket (bank 3).

  U11: 27C4096 ROM socket (bank 0).
  U12: 27C4096 ROM socket (bank 1).
  U13: 27C4096 ROM socket (bank 2).
  U14: 27C4096 ROM socket (bank 3).

  U21: NEC D43256BGU-70LL (32k x 8bit CMOS Static RAM).
  U22: LATTICE GAL20V8B-15LJ (High Performance E2CMOS PLD Generic Array Logic, 28-Lead PLCC).
  U23: LATTICE GAL16V8D-25LJ (High Performance E2CMOS PLD Generic Array Logic, 20-Lead PLCC).
  U24: LATTICE GAL16V8D-25LJ (High Performance E2CMOS PLD Generic Array Logic, 20-Lead PLCC).
  U26: SGS THOMSON ST93C46 (1K (64 x 16 or 128 x 8) Serial EEPROM).
  U27: SGS THOMSON ST93C46 (1K (64 x 16 or 128 x 8) Serial EEPROM).

  U35: PHILIPS 74HC2...
  U36: LATTICE GAL20V8B-15LJ (High Performance E2CMOS PLD Generic Array Logic, 28-Lead PLCC).
  U40: Dallas Semiconductor DS1202S (Serial Timekeeping Chip).
  U41: Maxim Integrated MAX705CSA (MPU Supervisory Circuits).
  U46: NEC D43256BGU-70LL (32k x 8bit CMOS Static RAM).
  U47: Maxim Integrated MAX202CWE (RS-232 Interface IC).
  U48: ISSI IS41C16257-60K (256K x 16bit (4-MBIT) Dynamic RAM With Fast Page Mode).
  U49: ISSI IS41C16257-60K (256K x 16bit (4-MBIT) Dynamic RAM With Fast Page Mode).
  U50: Dallas Semiconductor DS1620 (Digital Thermometer and Thermostat).
  U52: Allegro MicroSystems UDN2543B (Protected quad power driver).
  U53: SGS THOMSON 74HC244 (Octal buffer/line driver, 3-state).
  U54: Motorola AC244 (Octal Buffer/Line Driver with 3-State Outputs).
  U56: SGS THOMSON 74HC244 (Octal buffer/line driver, 3-state).
  U58: Motorola AC244 (Octal Buffer/Line Driver with 3-State Outputs).
  U59: Motorola AC244 (Octal Buffer/Line Driver with 3-State Outputs).
  U61: Motorola AC244 (Octal Buffer/Line Driver with 3-State Outputs).
  U65: LATTICE GAL20V8B-15LJ (High Performance E2CMOS PLD Generic Array Logic, 28-Lead PLCC).
  U66: NEC D43256BGU-70LL (32k x 8bit CMOS Static RAM).
  U71: Texas Instruments TL16C452FN (UART Interface IC Dual UART w/Prl Port & w/o FIFO).
  U72: Texas Instruments TL16C452FN (UART Interface IC Dual UART w/Prl Port & w/o FIFO).
  U73: CX0826 72 MHz crystal.
  U89: Allegro MicroSystems UDN2543B (Protected quad power driver).
  U149: ISSI IS41C16257-60K (256K x 16bit (4-MBIT) Dynamic RAM With Fast Page Mode).
  U152: ISSI IS41C16257-60K (256K x 16bit (4-MBIT) Dynamic RAM With Fast Page Mode).

  AA:  SGS THOMSON 74HC244 (Octal buffer/line driver, 3-state).
  AB:  SGS THOMSON 74HC244 (Octal buffer/line driver, 3-state).
  AC:  PHILIPS 74HC245D (Octal bus transceiver, 3-state).

  AMP: TDA 2006 (12W Audio Amplifier).

  VR1: Motorola 7805 (3-Terminal 1A Positive Voltage Regulator).
  VR2: SGS THOMSON L4975A (5A stepdown monolithic power switching regulator at 5.1V-40V).

  X2:  Unpopulated crystal (from factory).

*****************************************************************************************************************/

#define MASTER_CLOCK        XTAL_72MHz      /* confirmed */

#include "emu.h"
#include "cpu/arm/arm.h"
#include "sound/dac.h"
#include "includes/archimds.h"
//#include "machine/i2cmem.h"


class aristmk5_state : public archimedes_state
{
public:
	aristmk5_state(const machine_config &mconfig, device_type type, const char *tag)
		: archimedes_state(mconfig, type, tag) { }

	emu_timer *m_mk5_2KHz_timer;
	emu_timer *m_mk5_VSYNC_timer;
	UINT8 m_ext_latch;
	UINT8 m_flyback;
	DECLARE_WRITE32_MEMBER(Ns5w48);
	DECLARE_READ32_MEMBER(Ns5x58);
	DECLARE_READ32_MEMBER(mk5_ioc_r);
	DECLARE_WRITE32_MEMBER(mk5_ioc_w);
	DECLARE_READ32_MEMBER(Ns5r50);
	DECLARE_WRITE32_MEMBER(sram_banksel_w);
	DECLARE_DRIVER_INIT(aristmk5);
	virtual void machine_start();
	virtual void machine_reset();
	TIMER_CALLBACK_MEMBER(mk5_VSYNC_callback);
	TIMER_CALLBACK_MEMBER(mk5_2KHz_callback);
};


TIMER_CALLBACK_MEMBER(aristmk5_state::mk5_VSYNC_callback)
{
	m_ioc_regs[IRQ_STATUS_A] |= 0x08; //turn vsync bit on
	m_mk5_VSYNC_timer->adjust(attotime::never);
}

WRITE32_MEMBER(aristmk5_state::Ns5w48)
{
	/*
	There is one writeable register which is written with the Ns5w48 strobe. It contains four bits which are
	taken from bits 16 to 19 of the word being written. The register is cleared whenever the chip is reset. The
	register controls part of the video system. Bit 3(from data bus bit 19) controls the eorv output. If the bit is
	one, eorv outputs the NV/CSYNC signal from VIDC. If the bit is zero, eorv outputs inverted NV/CSYNC. Bit 2 of
	the register controls the eorh output. If the bit is zero, eorh is the NHSYNC output of VIDC. If the bit is one,
	eorh is inverted NHSYNC. Bits 1 and 0 control what is fed to the vidclk output as follows:

	     Bit1     Bit0     vidclk
	     0        0        24 Mhz clock
	     0        1        25 Mhz clock ;// external video crystal
	     1        0        36 Mhz clock
	     1        1        24 Mhz clock


	*/

	/*
	Golden Pyramids disassembly

	MOV     R0, #0x3200000
	ROM:03400948                 MOV     R1, #8
	ROM:0340094C                 STRB    R1, [R0,#0x14]  ; clear vsync
	ROM:03400950                 LDR     R2, =0xC350     ; 50000
	ROM:03400954
	ROM:03400954 loc_3400954                             ; CODE XREF: sub_3400944+18?j
	ROM:03400954                 NOP
	ROM:03400958                 SUBS    R2, R2, #1
	ROM:0340095C                 BNE     loc_3400954     ; does this 50000 times, presumably to wait for vsync
	ROM:03400960                 MOV     R0, #0x3200000
	ROM:03400964                 LDRB    R1, [R0,#0x10]  ; reads the irq status a
	ROM:03400968                 TST     R1, #8          ; test vsync
	*/


	m_ioc_regs[IRQ_STATUS_A] &= ~0x08;

	/*          bit 1              bit 0 */
	if((data &~(0x02)) && (data & (0x01))) // external video crystal is enabled. 25 mhz
	{
			m_mk5_VSYNC_timer->adjust(attotime::from_hz(50000)); // not sure but see above
	}
	if((data &~(0x02)) && (data &~(0x01))) // video clock is enabled. 24 mhz
	{
			m_mk5_VSYNC_timer->adjust(attotime::from_hz(50000)); // not sure
	}
	if((data & (0x02)) && (data &~(0x01))) // video clock is enabled. 36 mhz
	{
			m_mk5_VSYNC_timer->adjust(attotime::from_hz(50000)); // not sure
	}
	if((data &(0x02)) && (data &(0x01))) // video clock is enabled. 24 mhz
	{
			m_mk5_VSYNC_timer->adjust(attotime::from_hz(50000)); // not sure
	}
}

TIMER_CALLBACK_MEMBER(aristmk5_state::mk5_2KHz_callback)
{
	m_ioc_regs[IRQ_STATUS_A] |= 0x01;
	m_mk5_2KHz_timer->adjust(attotime::never);

}

READ32_MEMBER(aristmk5_state::Ns5x58)
{
	/*
	    1953.125 Hz for the operating system timer interrupt

	The pintr pin ( printer interrupt ) is connected to an interrupt latch in IOEB.
	A rising edge on pintr causes an interrupt to be latched in IOEB. The latch output
	is connected to the NIL[6] interrupt input on IOC and goes low when the rising edge is detected.
	The interrupt is cleared (NIL[6] is set high) by resetting the chip or by the NS5x58
	strobe.

	NIL[6] IOEB/1pintr - Interrupt Input ( OS Tick Interrput )

	Rising edge signal
	010101010101  .-------.   logic 0      .-------------.
	------------->|pint   |---1pintr------>|NIL[6]       |
	              | IOEB  |                |     IOC     |
	              `-------'                `-------------'
	*/


	// reset 2KHz timer
	m_mk5_2KHz_timer->adjust(attotime::from_hz(1953.125));
	m_ioc_regs[IRQ_STATUS_A] &= ~0x01;
	m_maincpu->set_input_line(ARM_IRQ_LINE, CLEAR_LINE);
	return 0xffffffff;
}

/* same as plain AA but with the I2C unconnected */
READ32_MEMBER(aristmk5_state::mk5_ioc_r)
{
	UINT32 ioc_addr;

	ioc_addr = offset*4;
	ioc_addr >>= 16;
	ioc_addr &= 0x37;

	if(((ioc_addr == 0x20) || (ioc_addr == 0x30)) && (offset & 0x1f) == 0)
	{
		int vert_pos;

		vert_pos = m_screen->vpos();
		m_flyback = (vert_pos <= m_vidc_regs[VIDC_VDSR] || vert_pos >= m_vidc_regs[VIDC_VDER]) ? 0x80 : 0x00;

		//i2c_data = (i2cmem_sda_read(machine().device("i2cmem")) & 1);

		return (m_flyback) | (m_ioc_regs[CONTROL] & 0x7c) | (1<<1) | 1;
	}

	return archimedes_ioc_r(space,offset,mem_mask);
}

WRITE32_MEMBER(aristmk5_state::mk5_ioc_w)
{
	UINT32 ioc_addr;

	ioc_addr = offset*4;
	ioc_addr >>= 16;
	ioc_addr &= 0x37;

	if(!m_ext_latch)
	{
		if(((ioc_addr == 0x20) || (ioc_addr == 0x30)) && (offset & 0x1f) == 0)
		{
			m_ioc_regs[CONTROL] = data & 0x7c;
			return;
		}
		else
			archimedes_ioc_w(space,offset,data,mem_mask);
	}
}

READ32_MEMBER(aristmk5_state::Ns5r50)
{
	return 0xf5; // checked inside the CPU check, unknown meaning
}

WRITE32_MEMBER(aristmk5_state::sram_banksel_w)
{
	/*

	The Main Board provides 32 kbytes of Static Random Access Memory (SRAM) with
	battery back-up for the electronic meters.
	The SRAM contains machine metering information, recording money in/out and
	game history etc. It is critical that this data is preserved reliably, and various
	jurisdictions require multiple backups of the data.
	Three standard low power SRAMs are fitted to the board. The data is usually
	replicated three times, so that each chip contains identical data. Each memory is
	checked against the other to verify that the stored data is correct.
	Each chip is mapped to the same address, and the chip selected depends on the bank
	select register. Access is mutually exclusive, increasing security with only one chip
	visible in the CPU address space at a time. If the CPU crashes and overwrites
	memory only one of the three devices can be corrupted. On reset the bank select
	register selects bank 0, which does not exist. The SRAMs are at banks 1,2,3.
	Each of the SRAM chips may be powered from a separate battery, further reducing
	the possibility of losing data. For the US Gaming Machine, a single battery provides
	power for all three SRAMs. This battery also powers the Real Time Clock


	CHIP SELECT & SRAM BANKING

	write: 03010420 40  select bank 1
	write: 3220000 01   store 0x01 @ 3220000
	write: 03010420 80  select bank 2
	write: 3220000 02   store 0x02 @ 3220000
	write: 03010420 C0  ...
	write: 3220000 03   ...
	write: 03010420 00  ...
	write: 3220000 00   ...
	write: 03010420 40  select the first SRAM chip
	read:  3220000 01   read the value 0x1 back hopefully
	write: 03010420 80  ...
	read:  3220000 02   ...
	write: 03010420 C0  ...
	read:  3220000 03   ...
	write: 03010420 00  select bank 0


	     Bit 0 - Page 1
	     Bit 1 - Page 2
	     Bit 2 - Page 3
	     NC
	     NC
	     NC
	     Bit 6 - SRAM 1
	     Bit 7 - SRAM 2

	     Bit 1 and 2 on select Page 4.
	     Bit 6 and 7 on select SRAM 3.

	     4 pages of 32k for each sram chip.
	*/
	membank("sram_bank")->set_entry((data & 0xc0) >> 6);
	membank("sram_bank_nz")->set_entry((data & 0xc0) >> 6);
}

/* U.S games have no dram emulator enabled */
static ADDRESS_MAP_START( aristmk5_map, AS_PROGRAM, 32, aristmk5_state )
	AM_RANGE(0x00000000, 0x01ffffff) AM_READWRITE(archimedes_memc_logical_r, archimedes_memc_logical_w)
	AM_RANGE(0x02000000, 0x02ffffff) AM_RAM AM_SHARE("physicalram") /* physical RAM - 16 MB for now, should be 512k for the A310 */

	/* MK-5 overrides */
	AM_RANGE(0x03010420, 0x03010423) AM_WRITE(sram_banksel_w) // SRAM bank select write

//  AM_RANGE(0x0301049c, 0x0301051f) AM_DEVREADWRITE_LEGACY("eeprom", eeprom_r, eeprom_w) // eeprom ???

	AM_RANGE(0x03010810, 0x03010813) AM_READWRITE(watchdog_reset32_r,watchdog_reset32_w) //MK-5 specific, watchdog
//  System Startup Code Enabled protection appears to be located at 0x3010400 - 0x30104ff
	AM_RANGE(0x03220000, 0x0323ffff) AM_RAMBANK("sram_bank") //AM_BASE_SIZE_GENERIC(nvram) // nvram 32kbytes x 3

	// bank5 slow
	AM_RANGE(0x03250048, 0x0325004b) AM_WRITE(Ns5w48) //IOEB control register
	AM_RANGE(0x03250050, 0x03250053) AM_READ(Ns5r50)  //IOEB ID register
	AM_RANGE(0x03250058, 0x0325005b) AM_READ(Ns5x58)  //IOEB interrupt Latch

	AM_RANGE(0x03000000, 0x0331ffff) AM_READWRITE(mk5_ioc_r, mk5_ioc_w)
	AM_RANGE(0x03320000, 0x0333ffff) AM_RAMBANK("sram_bank_nz") // AM_BASE_SIZE_GENERIC(nvram) // nvram 32kbytes x 3 NZ
	AM_RANGE(0x03400000, 0x035fffff) AM_ROM AM_REGION("maincpu", 0) AM_WRITE(archimedes_vidc_w)
	AM_RANGE(0x03600000, 0x037fffff) AM_READWRITE(archimedes_memc_r, archimedes_memc_w)
	AM_RANGE(0x03800000, 0x039fffff) AM_WRITE(archimedes_memc_page_w)
ADDRESS_MAP_END

/* with dram emulator enabled */
static ADDRESS_MAP_START( aristmk5_drame_map, AS_PROGRAM, 32, aristmk5_state )
	AM_RANGE(0x00000000, 0x01ffffff) AM_READWRITE(aristmk5_drame_memc_logical_r, archimedes_memc_logical_w)
	AM_RANGE(0x02000000, 0x02ffffff) AM_RAM AM_SHARE("physicalram") /* physical RAM - 16 MB for now, should be 512k for the A310 */

	/* MK-5 overrides */
	AM_RANGE(0x03010420, 0x03010423) AM_WRITE(sram_banksel_w) // SRAM bank select write

//  AM_RANGE(0x0301049c, 0x0301051f) AM_DEVREADWRITE_LEGACY("eeprom", eeprom_r, eeprom_w) // eeprom ???

	AM_RANGE(0x03010810, 0x03010813) AM_READWRITE(watchdog_reset32_r,watchdog_reset32_w) //MK-5 specific, watchdog
//  System Startup Code Enabled protection appears to be located at 0x3010400 - 0x30104ff
	AM_RANGE(0x03220000, 0x0323ffff) AM_RAMBANK("sram_bank") //AM_BASE_SIZE_GENERIC(nvram) // nvram 32kbytes x 3

	// bank5 slow
	AM_RANGE(0x03250048, 0x0325004b) AM_WRITE(Ns5w48) //IOEB control register
	AM_RANGE(0x03250050, 0x03250053) AM_READ(Ns5r50)  //IOEB ID register
	AM_RANGE(0x03250058, 0x0325005b) AM_READ(Ns5x58)  //IOEB interrupt Latch


	AM_RANGE(0x03000000, 0x0331ffff) AM_READWRITE(mk5_ioc_r, mk5_ioc_w)
	AM_RANGE(0x03320000, 0x0333ffff) AM_RAMBANK("sram_bank_nz") // AM_BASE_SIZE_GENERIC(nvram) // nvram 32kbytes x 3 NZ
	AM_RANGE(0x03400000, 0x035fffff) AM_ROM AM_REGION("maincpu", 0) AM_WRITE(archimedes_vidc_w)
	AM_RANGE(0x03600000, 0x037fffff) AM_READWRITE(archimedes_memc_r, archimedes_memc_w)
	AM_RANGE(0x03800000, 0x039fffff) AM_WRITE(archimedes_memc_page_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( aristmk5 )
	/* This simulates the ROM swap */
	PORT_START("ROM_LOAD")
	PORT_CONFNAME( 0x03, 0x03, "System Mode" )
	PORT_CONFSETTING(    0x00, "Set Chip v4.04 Mode" )
	PORT_CONFSETTING(    0x01, "Set Chip v4.4 Mode" )
	PORT_CONFSETTING(    0x02, "Clear Chip Mode" )
	PORT_CONFSETTING(    0x03, "Game Mode" )
INPUT_PORTS_END

DRIVER_INIT_MEMBER(aristmk5_state,aristmk5)
{
	UINT8 *SRAM    = memregion("sram")->base();
	UINT8 *SRAM_NZ = memregion("sram")->base();

	archimedes_driver_init();

	membank("sram_bank")->configure_entries(0, 4,    &SRAM[0],    0x20000);
	membank("sram_bank_nz")->configure_entries(0, 4, &SRAM_NZ[0], 0x20000);
}


void aristmk5_state::machine_start()
{
	archimedes_init();

	// reset the DAC to centerline
	//m_dac->write_signed8(0x80);

	m_mk5_2KHz_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(aristmk5_state::mk5_2KHz_callback),this));
	m_mk5_VSYNC_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(aristmk5_state::mk5_VSYNC_callback),this));
}

void aristmk5_state::machine_reset()
{
	archimedes_reset();
	m_mk5_2KHz_timer->adjust(attotime::from_hz(1953.125)); // 8MHz / 4096
	m_mk5_VSYNC_timer->adjust(attotime::from_hz(50000)); // default bit 1 & bit 2 == 0

	m_ioc_regs[IRQ_STATUS_B] |= 0x40; //hack, set keyboard irq empty to be ON

	/* load the roms according to what the operator wants */
	{
		UINT8 *ROM = memregion("maincpu")->base();
		UINT8 *PRG;// = memregion("prg_code")->base();
		int i;
		UINT8 op_mode;
		static const char *const rom_region[] = { "set_chip_4.04", "set_chip_4.4", "clear_chip", "game_prg" };

		op_mode = ioport("ROM_LOAD")->read();

		PRG = memregion(rom_region[op_mode & 3])->base();

		if(PRG!=NULL)

		for(i=0;i<0x400000;i++)
			ROM[i] = PRG[i];
	}
}

#if 0
#define NVRAM_SIZE 256
#define NVRAM_PAGE_SIZE 0   /* max size of one write request */
#endif


static MACHINE_CONFIG_START( aristmk5, aristmk5_state )
	MCFG_CPU_ADD("maincpu", ARM, MASTER_CLOCK/6)    // 12000000
	MCFG_CPU_PROGRAM_MAP(aristmk5_drame_map)
	MCFG_WATCHDOG_TIME_INIT(attotime::from_seconds(2))  /* 1.6 - 2 seconds */

//  MCFG_I2CMEM_ADD("i2cmem")
//  MCFG_I2CMEM_PAGE_SIZE(NVRAM_PAGE_SIZE)
//  MCFG_I2CMEM_DATA_SIZE(NVRAM_SIZE)
	/* TODO: this isn't supposed to access a keyboard ... */
	MCFG_DEVICE_ADD("kart", AAKART, 12000000/128) // TODO: frequency

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(640, 400)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 400-1)
	MCFG_SCREEN_UPDATE_DRIVER(archimedes_state, screen_update)

	MCFG_PALETTE_ADD("palette", 0x200)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DAC_ADD("dac0")
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_DAC_ADD("dac2")
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_DAC_ADD("dac3")
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_DAC_ADD("dac4")
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_DAC_ADD("dac5")
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_DAC_ADD("dac6")
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_DAC_ADD("dac7")
	MCFG_SOUND_ROUTE(0, "mono", 0.10)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( aristmk5_usa, aristmk5_state )
	MCFG_CPU_ADD("maincpu", ARM, MASTER_CLOCK/6)    // 12000000
	MCFG_CPU_PROGRAM_MAP(aristmk5_map)
	MCFG_WATCHDOG_TIME_INIT(attotime::from_seconds(2))  /* 1.6 - 2 seconds */

//  MCFG_I2CMEM_ADD("i2cmem")
//  MCFG_I2CMEM_PAGE_SIZE(NVRAM_PAGE_SIZE)
//  MCFG_I2CMEM_DATA_SIZE(NVRAM_SIZE)
	/* TODO: this isn't supposed to access a keyboard ... */
	MCFG_DEVICE_ADD("kart", AAKART, 12000000/128) // TODO: frequency

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(640, 400)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 400-1)
	MCFG_SCREEN_UPDATE_DRIVER(archimedes_state, screen_update)

	MCFG_PALETTE_ADD("palette", 0x200)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DAC_ADD("dac0")
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_DAC_ADD("dac2")
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_DAC_ADD("dac3")
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_DAC_ADD("dac4")
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_DAC_ADD("dac5")
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_DAC_ADD("dac6")
	MCFG_SOUND_ROUTE(0, "mono", 0.10)

	MCFG_DAC_ADD("dac7")
	MCFG_SOUND_ROUTE(0, "mono", 0.10)
MACHINE_CONFIG_END

#define ARISTOCRAT_MK5_BIOS \
	ROM_REGION( 0x400000, "set_chip_4.04", ROMREGION_ERASEFF ) \
	/* setchip v4.04.08 4meg */ \
	ROM_LOAD32_WORD( "setchip v4.04.08.u7",  0x000000, 0x80000, CRC(e8e8dc75) SHA1(201fe95256459ce34fdb6f7498135ab5016d07f3) ) \
	ROM_LOAD32_WORD( "setchip v4.04.08.u11", 0x000002, 0x80000, CRC(ff7a9035) SHA1(4352c4336e61947c555fdc80c61f944076f64b64) ) \
	ROM_REGION( 0x400000, "set_chip_4.4", ROMREGION_ERASEFF ) \
	/* setchip v4.4 4meg 42pin */ \
	ROM_LOAD32_WORD( "setchip v4.4.u7",  0x000000, 0x80000, CRC(2453137e) SHA1(b59998e75ae3924da16faf47b9cfe9afd60d810c) ) \
	ROM_LOAD32_WORD( "setchip v4.4.u11", 0x000002, 0x80000, CRC(82dfa12a) SHA1(86fd0f0ad8d5d1bc503392a40bbcdadb055b2765) ) \
	ROM_REGION( 0x400000, "clear_chip", ROMREGION_ERASEFF ) \
	/* clear chip */ \
	ROM_LOAD32_WORD( "clear.u7",  0x000000, 0x80000, CRC(5a254b22) SHA1(8444f237b392df2a3cb42ea349e7af32f47dd544) ) \
	ROM_LOAD32_WORD( "clear.u11", 0x000002, 0x80000, CRC(def36617) SHA1(c7ba5b08e884a8fb36c9fb51c08e243e32c81f89) ) \
	/* GALs */ \
	ROM_REGION( 0x600, "gals", 0 ) \
	ROM_LOAD( "a562837.u36",  0x000000, 0x000157, CRC(1f269234) SHA1(29940dd50fb55c632935f62ff44ca724379c7a43) ) \
	ROM_LOAD( "a562838.u65",  0x000200, 0x000157, CRC(f2f3c40a) SHA1(b795dfa5cc4e8127c3f3a0906664910d1325ec92) ) \
	ROM_LOAD( "a562840.u22",  0x000400, 0x000157, CRC(941d4cdb) SHA1(1ca091fba69e92f262dbb3d40f515703c8981793) )

ROM_START( aristmk5 )
	ARISTOCRAT_MK5_BIOS

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( reelrock )
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0100779v.u7",  0x000000, 0x80000, CRC(b60af34f) SHA1(1143380b765db234b3871c0fe04736472fde7de4) )
	ROM_LOAD32_WORD( "0100779v.u11", 0x000002, 0x80000, CRC(57e341d0) SHA1(9b0d50763bb74ca5fe404c9cd526633721cf6677) )
	ROM_LOAD32_WORD( "0100779v.u8",  0x100000, 0x80000, CRC(57eec667) SHA1(5f3888d75f48b6148f451d7ebb7f99e1a0939f3c) )
	ROM_LOAD32_WORD( "0100779v.u12", 0x100002, 0x80000, CRC(4ac20679) SHA1(0ac732ffe6a33806e4a06e87ec875a3e1314e06b) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( indiandr )
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0100845v.u7",  0x000000, 0x80000, CRC(0c924a3e) SHA1(499b4ae601e53173e3ba5f400a40e5ae7bbaa043) )
	ROM_LOAD32_WORD( "0100845v.u11", 0x000002, 0x80000, CRC(e371dc0f) SHA1(a01ab7fb63a19c144f2c465ecdfc042695124bdf) )
	ROM_LOAD32_WORD( "0100845v.u8",  0x100000, 0x80000, CRC(1c6bfb47) SHA1(7f751cb499a6185a0ab64eeec511583ceeee6ee8) )
	ROM_LOAD32_WORD( "0100845v.u12", 0x100002, 0x80000, CRC(4bbe67f6) SHA1(928f88387da66697f1de54f086531f600f80a15e) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( dolphntr )
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0200424v.u7",  0x000000, 0x80000, CRC(5dd88306) SHA1(ee8ec7d123d057e8df9be0e8dadecea7dab7aafd) )
	ROM_LOAD32_WORD( "0200424v.u11", 0x000002, 0x80000, CRC(bcb732ea) SHA1(838300914846c6e740780e5a24b9db7304a8a88d) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( dolphtra )
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0100424v.u7",  0x000000, 0x80000, CRC(657faef7) SHA1(09e1f9d461e855c10cf8b825ef83dd3e7db65b43) )
	ROM_LOAD32_WORD( "0100424v.u11", 0x000002, 0x80000, CRC(65aa46ec) SHA1(3ad4270efbc2e947097d94a3258a544d79a1d599) )
	ROM_LOAD32_WORD( "0100424v.u8",  0x100000, 0x80000, CRC(e77868ad) SHA1(3345da120075bc0da47bac0a4840790693382620) )
	ROM_LOAD32_WORD( "0100424v.u12", 0x100002, 0x80000, CRC(6abd9309) SHA1(c405a13f5bfe447c1ab20d92e140e4fb145920d4) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( qotn )
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0200439v.u7",  0x000000, 0x80000, CRC(d476a893) SHA1(186d6fb1830c33976f2d3c96e4f045ece885dc63) )
	ROM_LOAD32_WORD( "0200439v.u11", 0x000002, 0x80000, CRC(8b0d7205) SHA1(ffa03f1c9332a1a7443eb91b0ded56e7cd9e3cee) )
	ROM_LOAD32_WORD( "0200439v.u8",  0x100000, 0x80000, CRC(9b996ef1) SHA1(72489e9a0ee5c34f7cad3d121bcd08e09ef72360) )
	ROM_LOAD32_WORD( "0200439v.u12", 0x100002, 0x80000, CRC(2a0f7feb) SHA1(27c89dadf759e6c892121650758c44ec50990cb6) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

// MV4091 - 10 Credit Multiplier / 9 Line Multiline.
// QUEEN OF THE NILE - NSW/ACT  B - 13/05/97.
// Marked as GHG409102
// All devices are 27c4002 instead of 27c4096.
// Even when it's a NSW/ACT, the program seems to be for US-Export platforms...
ROM_START( qotna )
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "mv4091_qotn.u7",  0x000000, 0x80000, CRC(a00ab2cf) SHA1(eb3120fe4b1d0554c224c7646e727e86fd35975e) )
	ROM_LOAD32_WORD( "mv4091_qotn.u11", 0x000002, 0x80000, CRC(c4a35337) SHA1(d469ed154caed1f0a4cf89e67d852924c95172ed) )
	ROM_LOAD32_WORD( "mv4091_qotn.u8",  0x100000, 0x80000, CRC(16a629e1) SHA1(0dee11a2f1b2068a86b3e0b6c01d115555a657c9) )
	ROM_LOAD32_WORD( "mv4091_qotn.u12", 0x100002, 0x80000, CRC(7871a846) SHA1(ac1d741092afda842e1864f1a7a14137a9ee46d9) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( swthrt2v )
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "01j01986.u7",  0x000000, 0x80000, CRC(f51b2faa) SHA1(dbcfdbee92af5f89a8a2611bbc687ee0cc907642) )
	ROM_LOAD32_WORD( "01j01986.u11", 0x000002, 0x80000, CRC(bd7ead91) SHA1(9f775428a4aa0b0a8ee17aed9be620edc2020c5e) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( enchfrst )
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0400122v.u7",  0x000000, 0x80000, CRC(b5829b27) SHA1(f6f84c8dc524dcee95e37b93ead9090903bdca4f) )
	ROM_LOAD32_WORD( "0400122v.u11", 0x000002, 0x80000, CRC(7a97adc8) SHA1(b52f7fdc7edf9ad92351154c01b8003c0576ed94) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( margmgc )
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "01j00101.u7",  0x000000, 0x80000, CRC(eee7ebaf) SHA1(bad0c08578877f84325c07d51c6ed76c40b70720) )
	ROM_LOAD32_WORD( "01j00101.u11", 0x000002, 0x80000, CRC(4901a166) SHA1(8afe6f08b4ac5c17744dff73939c4bc93124fdf1) )
	ROM_LOAD32_WORD( "01j00101.u8",  0x100000, 0x80000, CRC(b0d78efe) SHA1(bc8b345290f4d31c6553f1e2700bc8324b4eeeac) )
	ROM_LOAD32_WORD( "01j00101.u12", 0x100002, 0x80000, CRC(90ff59a8) SHA1(c9e342db2b5e8c3f45efa8496bc369385046e920) )
	ROM_LOAD32_WORD( "01j00101.u9",  0x200000, 0x80000, CRC(1f0ca910) SHA1(be7a2f395eae09a29faf99ba34551fbc38f20fdb) )
	ROM_LOAD32_WORD( "01j00101.u13", 0x200002, 0x80000, CRC(3f702945) SHA1(a6c9a848d059c1e564fdc5a65bf8c9600853edfa) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

// 0200751V - 10 Credit Multiplier / 20 Line Multiline.
// ADONIS - NSW/ACT A - 25/05/98  Revision: 10  602/9.
ROM_START( adonis )
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0200751v.u7",  0x000000, 0x80000, CRC(ab386ab0) SHA1(56c5baea4272866a9fe18bdc371a49f155251f86) )
	ROM_LOAD32_WORD( "0200751v.u11", 0x000002, 0x80000, CRC(ce8c8449) SHA1(9894f0286f27147dcc437e4406870fe695a6f61a) )
	ROM_LOAD32_WORD( "0200751v.u8",  0x100000, 0x80000, CRC(99097a82) SHA1(a08214ab4781b06b46fc3be5c48288e373230ef4) )
	ROM_LOAD32_WORD( "0200751v.u12", 0x100002, 0x80000, CRC(443a7b6d) SHA1(c19a1c50fb8774826a1e12adacba8bbfce320891) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "eeproms", 0 )
	ROM_LOAD( "st93c46.u26", 0x0000, 0x0080, NO_DUMP )
	ROM_LOAD( "st93c46.u27", 0x0080, 0x0080, NO_DUMP )

	ROM_REGION( 0x0005, "plds", 0 )
	ROM_LOAD( "gal20v8b.u22", 0x0000, 0x0001, NO_DUMP ) /* 28-Lead PLCC package. Unable to read */
	ROM_LOAD( "gal16v8d.u23", 0x0000, 0x0001, NO_DUMP ) /* 20-Lead PLCC package. Unable to read */
	ROM_LOAD( "gal16v8d.u24", 0x0000, 0x0001, NO_DUMP ) /* 20-Lead PLCC package. Unable to read */
	ROM_LOAD( "gal20v8b.u36", 0x0000, 0x0001, NO_DUMP ) /* 28-Lead PLCC package. Unable to read */
	ROM_LOAD( "gal20v8b.u65", 0x0000, 0x0001, NO_DUMP ) /* 28-Lead PLCC package. Unable to read */
ROM_END

// 0100751V - 10 Credit Multiplier / 20 Line Multiline.
// ADONIS - NSW/ACT A - 25/05/98  Revision: 9  602/9.
ROM_START( adonisa )
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0100751v.u7",  0x000000, 0x80000, CRC(ca3e97db) SHA1(bd0a4402e57891899d92ea85a87fb8925a44f706) )
	ROM_LOAD32_WORD( "0100751v.u11", 0x000002, 0x80000, CRC(cfe3f792) SHA1(aa1bf77101404c2018a5e5b808f1d683e29ae942) )
	ROM_LOAD32_WORD( "0100751v.u8",  0x100000, 0x80000, CRC(d55204bd) SHA1(208c089d435ea4af25d0b9b3d5e79fea397bc885) )
	ROM_LOAD32_WORD( "0100751v.u12", 0x100002, 0x80000, CRC(77090858) SHA1(76ebc15b26f378ac95276f0aa26d077e3646a6f1) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "eeproms", 0 )
	ROM_LOAD( "st93c46.u27", 0x0000, 0x0080, CRC(115c305a) SHA1(684a70d74ec92564e17c4292cd357e603842c485) )
	ROM_LOAD( "st93c46.u26", 0x0080, 0x0080, CRC(652d544c) SHA1(cd5bd20e9a0f22d7367cc169e2844a02751c7c91) ) // blank... all 0xff's

	ROM_REGION( 0x0005, "plds", 0 )
	ROM_LOAD( "gal20v8b.u22", 0x0000, 0x0001, NO_DUMP ) /* 28-Lead PLCC package. Unable to read */
	ROM_LOAD( "gal16v8d.u23", 0x0000, 0x0001, NO_DUMP ) /* 20-Lead PLCC package. Unable to read */
	ROM_LOAD( "gal16v8d.u24", 0x0000, 0x0001, NO_DUMP ) /* 20-Lead PLCC package. Unable to read */
	ROM_LOAD( "gal20v8b.u36", 0x0000, 0x0001, NO_DUMP ) /* 28-Lead PLCC package. Unable to read */
	ROM_LOAD( "gal20v8b.u65", 0x0000, 0x0001, NO_DUMP ) /* 28-Lead PLCC package. Unable to read */
ROM_END

// 630 - 10 Credit Multiplier / 9 Line Multiline.
// The Chariot Challenge - NSW/ACT - A - 10/08/98.
// 04J00714
ROM_START( chariotc )
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "04j00714_chariot_challenge.u7",  0x000000, 0x80000, CRC(2f3a1af7) SHA1(e1448116a81687cb79dd380dfbc8decf1f83e649) )
	ROM_LOAD32_WORD( "04j00714_chariot_challenge.u11", 0x000002, 0x80000, CRC(ef4f49e8) SHA1(8ff21f679a55cdfebcf22c109dfd3b41773293bd) )
	ROM_LOAD32_WORD( "04j00714_chariot_challenge.u8",  0x100000, 0x80000, CRC(fa24cfde) SHA1(1725c38a8a15915d8aa8e59afef9ce1d6e8d01c5) )
	ROM_LOAD32_WORD( "04j00714_chariot_challenge.u12", 0x100002, 0x80000, CRC(b8d4a5ec) SHA1(097e44cdb30b9aafd7f5358c8f0cdd130ec0615e) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( wtiger )
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0200954v.u7",  0x000000, 0x80000, CRC(752e54c5) SHA1(9317544a7cf2d9bf29347d31fe72331fc3d018ef) )
	ROM_LOAD32_WORD( "0200954v.u11", 0x000002, 0x80000, CRC(38e888b1) SHA1(acc857eb2be19140bbb58d70583e08f24807b9f2) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

/****************** Touchscreen games and New Zealand games ******************/

ROM_START( dmdtouch )
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0400433v.u7",  0x000000, 0x80000, CRC(71b19365) SHA1(5a8ba1806af544d33e9acbcbbc0555805b4074e6) )
	ROM_LOAD32_WORD( "0400433v.u11", 0x000002, 0x80000, CRC(3d836342) SHA1(b015a4ba998b39ed86cdb6247c9c7f1365641b59) )
	ROM_LOAD32_WORD( "0400433v.u8",  0x100000, 0x80000, CRC(971bbf63) SHA1(082f81115209c7089c76fb207248da3c347a080b) )
	ROM_LOAD32_WORD( "0400433v.u12", 0x100002, 0x80000, CRC(9e0d08e2) SHA1(38b10f7c37f1cefe9271549073dc0a4fed409aec) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASEFF )
ROM_END

ROM_START( geishanz )
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0101408v.u7",  0x000000, 0x80000, CRC(ebdde248) SHA1(83f4f4deb5c6f5b33ae066d50e043a24cb0cbfe0) )
	ROM_LOAD32_WORD( "0101408v.u11", 0x000002, 0x80000, CRC(2f9e7cd4) SHA1(e9498879c9ca66740856c00fda0416f5d9f7c823) )
	ROM_LOAD32_WORD( "0101408v.u8",  0x100000, 0x80000, CRC(87e41b1b) SHA1(029687aeaed701e0f4b8da9d1d60a5a0a9445518) )
	ROM_LOAD32_WORD( "0101408v.u12", 0x100002, 0x80000, CRC(255f2368) SHA1(eb955452e1ed8d9d4f30f3372d7321f01d3654d3) )
	ROM_LOAD32_WORD( "0101408v.u9",  0x200000, 0x80000, CRC(5f161953) SHA1(d07353d006811813b94cb022857f49c4906fd87b) )
	ROM_LOAD32_WORD( "0101408v.u13", 0x200002, 0x80000, CRC(5ef6323e) SHA1(82a720d814ca06c6d286c59bbf325d9a1034375a) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

/*********************** US and Export games (requires set chips) ***********************/

// 559/2 - 10 Credit Multiplier / 9 Line Multiline.
// Mine, Mine, Mine - Export E - 14/02/96.
// All devices are 27c4002 instead of 27c4096.
ROM_START( minemine )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "mine_export.u7",  0x000000, 0x80000, CRC(41bc3714) SHA1(5a8f7d24a6a697524af7997dcedd214fcaf48768) )
	ROM_LOAD32_WORD( "mine_export.u11", 0x000002, 0x80000, CRC(75803b10) SHA1(2ff3d966da2992ddcc7e229d979cc1ee623b4900) )
	ROM_LOAD32_WORD( "mine_export.u8",  0x100000, 0x80000, CRC(0a3e2baf) SHA1(b9ab989cf383cd6ea0aa1ead137558a1a6f5901d) )
	ROM_LOAD32_WORD( "mine_export.u12", 0x100002, 0x80000, CRC(26c01532) SHA1(ec68ad44b703609c7bc27275f8d9250a16d9067c) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

// 602/1 - 10 Credit Multiplier / 9 Line Multiline.
// Dolphin Treasure - Export B - 06/12/96.
// All devices are 27c4002 instead of 27c4096.
ROM_START( dolphtre )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "dolphin_treasure_export.u7",  0x000000, 0x80000, CRC(97e3e4d0) SHA1(211b9b9e0f25dfaf9d1dfe1d3d88592522aa6f07) )
	ROM_LOAD32_WORD( "dolphin_treasure_export.u11", 0x000002, 0x80000, CRC(de221eb5) SHA1(0e550e90b7fd5670f3f3a8589239c342ed70dc3d) )
	ROM_LOAD32_WORD( "dolphin_treasure_export.u8",  0x100000, 0x80000, CRC(cb3ca8b6) SHA1(dba8bdaa406c07870f95241466359e39a012a70b) )
	ROM_LOAD32_WORD( "dolphin_treasure_export.u12", 0x100002, 0x80000, CRC(8ee1c2d3) SHA1(e6ecaaac0cb4518ecc0d36532ab532f46e3e628b) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

// 603(a) - 3,5,10,25,50 Credit Multiplier / 20 Line Multiline.
// Cash Chameleon 100cm - Export B - 06/12/96.
// Marked as DHG4078.
ROM_START( cashcham )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "dhg4078_cash_chameleon.u7",  0x000000, 0x80000, CRC(cb407a19) SHA1(d98421d6548e48b413f6dfcab4e240e98fcc9a69) )
	ROM_LOAD32_WORD( "dhg4078_cash_chameleon.u11", 0x000002, 0x80000, CRC(94d73843) SHA1(ab236750c67e7fff3af831f1d03f45c45f280fd1) )
	ROM_LOAD32_WORD( "dhg4078_cash_chameleon.u8",  0x100000, 0x80000, CRC(4cae8a5d) SHA1(3232461afd75ce71f8a2cb4ac7e9a3caeb8aabcd) )
	ROM_LOAD32_WORD( "dhg4078_cash_chameleon.u12", 0x100002, 0x80000, CRC(39e17f0b) SHA1(25a0364fa45e4e78d6c365b0739606e71597bd71) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( goldprmd )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "goldprmd.u7",  0x000000, 0x80000, CRC(2fbed80c) SHA1(fb0d97cb2be96da37c487fc3aef06c6120efdb46) )
	ROM_LOAD32_WORD( "goldprmd.u11", 0x000002, 0x80000, CRC(ec9c183c) SHA1(e405082ee779c4fee103fb7384469c9d6afbc95b) )
	ROM_LOAD32_WORD( "goldprmd.u8",  0x100000, 0x80000, CRC(3cd7d8e5) SHA1(ae83a7c335564c398330d43295997b8ca547c92d) )
	ROM_LOAD32_WORD( "goldprmd.u12", 0x100002, 0x80000, CRC(8bbf45d0) SHA1(f58f28e7cc4ac225197959566d81973b5aa0e836) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

// 569/8 - 10 Credit Multiplier / 9 Line Multiline.
// Wild Cougar - Export D - 19/05/97.
// All devices are 27c4002 instead of 27c4096.
ROM_START( wldcougr )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "wild_cougar_export.u7",  0x000000, 0x80000, CRC(7ada053f) SHA1(5102b0b9db505454624750a3fd6db455682538f3) )
	ROM_LOAD32_WORD( "wild_cougar_export.u11", 0x000002, 0x80000, CRC(69a78695) SHA1(1ed89cf38dc85f752449a858cd9558bed235af58) )
	ROM_LOAD32_WORD( "wild_cougar_export.u8",  0x100000, 0x80000, CRC(496b0295) SHA1(237183a192ad9b4bc133014cc83149d4a7062785) )
	ROM_LOAD32_WORD( "wild_cougar_export.u12", 0x100002, 0x80000, CRC(fe2bafdc) SHA1(e8b454db44a532d75b3aff323855340695688f0f) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

// 593 - 10 Credit Multiplier / 9 Line Multiline.
// Bumble Bugs - Export D - 05/07/97.
// All devices are 27c4002 instead of 27c4096.
// Marked as CHG029604 and 92.691%
ROM_START( bumblbug )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "bumble_bugs_export.u7",  0x000000, 0x80000, CRC(ec605a36) SHA1(114e0840cfbd0c64645a5a33065db85462a0ba2d) )    // 92.691%
	ROM_LOAD32_WORD( "bumble_bugs_export.u11", 0x000002, 0x80000, CRC(17b154bd) SHA1(efdf307670a3d74f7980fec2d2197d837d4c26e2) )    // 92.691%
	ROM_LOAD32_WORD( "bumble_bugs_export.u8",  0x100000, 0x80000, CRC(e0c01d01) SHA1(9153129fd348a97da7cccf002e5d03e4b4db9264) )    // base
	ROM_LOAD32_WORD( "bumble_bugs_export.u12", 0x100002, 0x80000, CRC(28700d5d) SHA1(87a583cd487da6cb4c2da5f62297f0e577269fae) )    // base

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

// 586/7(b) - 10 Credit Multiplier / 9 Line Multiline.
// Penguin Pays - Export B - 14/07/97.
// All devices are 27c4002 instead of 27c4096.
ROM_START( pengpays )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "penguin_pays_export.u7",  0x000000, 0x80000, CRC(19d75260) SHA1(798472b1b5d8f5ca99d8bfe57e99a76686f0aa3f) )
	ROM_LOAD32_WORD( "penguin_pays_export.u11", 0x000002, 0x80000, CRC(2b010813) SHA1(a383997308881a3ac35de56fe10e3852fa89fdf6) )
	ROM_LOAD32_WORD( "penguin_pays_export.u8",  0x100000, 0x80000, CRC(6aeaebc8) SHA1(6f70b14e9f4e9940512bd6e89bc9ccbfe1f4a81f) )
	ROM_LOAD32_WORD( "penguin_pays_export.u12", 0x100002, 0x80000, CRC(d959a048) SHA1(92f69090d599f95b48e79213e5b7d486e083d8f4) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

// MV4098 - 10 Credit Multiplier / 9 Line Multiline.
// BOOT SCOOTIN' - Export A - 25/08/99.
// All devices are 27c4002 instead of 27c4096.
// Marked as GHG1012 and 92.767%
ROM_START( bootsctn )
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "ghg1012_boot_scootin.u7",  0x000000, 0x80000, CRC(ca26f31e) SHA1(e8da31cc8d12bf8a28f1ca4d796259ae9010f8af) )  // 92.767%
	ROM_LOAD32_WORD( "ghg1012_boot_scootin.u11", 0x000002, 0x80000, CRC(61da1767) SHA1(83d4df1060975f03f291b9119c0d2b8debb0fb60) )  // 92.767%
	ROM_LOAD32_WORD( "ghg1012_boot_scootin.u8",  0x100000, 0x80000, CRC(9ae4d616) SHA1(60d4d36f75685dfe21f914fa4682cd6a64fcfa58) )  // base
	ROM_LOAD32_WORD( "ghg1012_boot_scootin.u12", 0x100002, 0x80000, CRC(2c50c083) SHA1(ae3b01200d152df9b2966b5308c71e9d1ad43d78) )  // base
	ROM_LOAD32_WORD( "ghg1012_boot_scootin.u9",  0x200000, 0x80000, CRC(c0a4920d) SHA1(d2c6d259d2e067b6e5ad72a6ef164aac7d72bc5a) )  // base
	ROM_LOAD32_WORD( "ghg1012_boot_scootin.u13", 0x200002, 0x80000, CRC(55716d82) SHA1(5b9982a49201842e9551a9c763a6babbb47a863e) )  // base
	ROM_LOAD32_WORD( "ghg1012_boot_scootin.u10", 0x300000, 0x80000, CRC(3ecdf7ee) SHA1(9d658a22da737daafdf6cb0d49009796036d04b1) )  // base
	ROM_LOAD32_WORD( "ghg1012_boot_scootin.u14", 0x300002, 0x80000, CRC(18934c51) SHA1(f7c9c95c687dbfe89747e7877157fde37bc1119e) )  // base

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

// MV4104  3,5,10,20,25,50 Credit Multiplier / 9-20 Line Multiline.
// CUCKOO - Export C - 02/02/00.
// All devices are 27c4002 instead of 27c4096.
ROM_START( cuckoo )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "mv4104_cuckoo.u7",  0x000000, 0x80000, CRC(0bd17338) SHA1(b8f467bdf8d76533a2b7d44fe93be414f25a3c31) ) //
	ROM_LOAD32_WORD( "mv4104_cuckoo.u11", 0x000002, 0x80000, CRC(4c407deb) SHA1(57589e61a376ddff99cd420eb47bf8c902c6a249) )
	ROM_LOAD32_WORD( "mv4104_cuckoo.u8",  0x100000, 0x80000, CRC(33f52052) SHA1(89cbfe588d91244adff4c520fa94962d69ff20bf) )
	ROM_LOAD32_WORD( "mv4104_cuckoo.u12", 0x100002, 0x80000, CRC(00bb7597) SHA1(f4d6b21091e320a82d59477469340633b001ed0d) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

// MV4115 - 5,10,20 Credit Multiplier / 9 Line Multiline.
// Magic Mask [Reel Game] - Export A - 09/05/2000.
ROM_START( magicmsk )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "magicmsk.u7",  0x000000, 0x80000, CRC(17317eb9) SHA1(3ddb8d61f23461c3194af534928164550208bbee) )
	ROM_LOAD32_WORD( "magicmsk.u11", 0x000002, 0x80000, CRC(42af4b3f) SHA1(5d88951f77782ff3861b6550ace076662a0b45aa) )
	ROM_LOAD32_WORD( "magicmsk.u8",  0x100000, 0x80000, CRC(23aefb5a) SHA1(ba4488754794f75f53b9c81b74b6ccd992c64acc) )
	ROM_LOAD32_WORD( "magicmsk.u12", 0x100002, 0x80000, CRC(6829a7bf) SHA1(97eed83763d0ec5e753d6ad194e906b1307c4940) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

// MV4115 - 5,10,20 Credit Multiplier / 9 Line Multiline.
// Magic Mask [Reel Game] - Export A - 09/05/2000.
// Alternate set with identical description, but way different
// than the parent. All devices are 27c4002 instead of 27c4096.
//
// romcmp magicmsk.zip magicmska.zip
// 4 and 4 files
// magicmsk.u12    mv4115_magic_mask.u12    21.547699%
// magicmsk.u8     mv4115_magic_mask.u8     21.138954%
// magicmsk.u11    mv4115_magic_mask.u11    17.786026%
// magicmsk.u7     mv4115_magic_mask.u7     16.893578%
ROM_START( magicmska )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "mv4115_magic_mask.u7",  0x000000, 0x80000, CRC(452a19c9) SHA1(aab1f4ccfc6cdb382f7a0e85491614cc58811a08) )
	ROM_LOAD32_WORD( "mv4115_magic_mask.u11", 0x000002, 0x80000, CRC(c57601f3) SHA1(1616a424b41ad6fea6383a08d5352e8240433374) )
	ROM_LOAD32_WORD( "mv4115_magic_mask.u8",  0x100000, 0x80000, CRC(607d7447) SHA1(064dbfe8b52eebe1be7a41735da3fa01eacd1686) )
	ROM_LOAD32_WORD( "mv4115_magic_mask.u12", 0x100002, 0x80000, CRC(cf4cd569) SHA1(408edcd746587d249c4286f7a99f33ad94214f7c) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

// MV4124/1 - 5,10,25,50 Credit Multiplier / 20 Line Multiline.
// Adonis [Reel Game] - Export B - 31/07/01.
// Marked as BHG1284.
ROM_START( adonise )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "bhg1284_mv4124_adonis.u7",  0x000000, 0x80000, CRC(ed6254d7) SHA1(d2b790fdd7f5fc7b78fcfc4c96d0fc272ccf8da6) )
	ROM_LOAD32_WORD( "bhg1284_mv4124_adonis.u11", 0x000002, 0x80000, CRC(1f629286) SHA1(bce380a6a76c77bc790436bd6f94474a1db0c231) )
	ROM_LOAD32_WORD( "bhg1284_mv4124_adonis.u8",  0x100000, 0x80000, CRC(b756c96d) SHA1(494df20090d415e83d599023203c13273e7925ad) )
	ROM_LOAD32_WORD( "bhg1284_mv4124_adonis.u12", 0x100002, 0x80000, CRC(1d3b6b8f) SHA1(1ddcfd7323cc7e79d3e39d913fdb5cf5cd53d56d) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

// MV4115/6 - 9/20 Line Multiline Multiplier.
// Party Gras [Reel Game] - Export A - 10/11/2001.
// All devices are 27c4002 instead of 27c4096.
ROM_START( partygrs )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "mv4115-6_party_gras.u7",  0x000000, 0x80000, CRC(53047385) SHA1(efe50e8785047986513f2de63d2425ba80417481) )
	ROM_LOAD32_WORD( "mv4115-6_party_gras.u11", 0x000002, 0x80000, CRC(f8bd9f7f) SHA1(a8c67a644f9090890e8f33e620fe0bb4633bd6e8) )
	ROM_LOAD32_WORD( "mv4115-6_party_gras.u8",  0x100000, 0x80000, CRC(0b98a0fa) SHA1(c9ada21e39472f28cd9b8ec19be7235410ad3e7a) )
	ROM_LOAD32_WORD( "mv4115-6_party_gras.u12", 0x100002, 0x80000, CRC(00d1395c) SHA1(d9a66d6cdb5aa4f583d8c23306b1416646cbde93) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


/*************************
*      Game Drivers      *
*************************/

//    YEAR  NAME       PARENT    MACHINE       INPUT     STATE           INIT      ROT     COMPANY       FULLNAME                                       FLAGS
GAME( 1995, aristmk5,  0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "MKV Set/Clear Chips (USA)",                    MACHINE_NOT_WORKING|MACHINE_IS_BIOS_ROOT )

// Dates listed below are for the combination (reel layout), not release dates
GAME( 1995, enchfrst,  0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Enchanted Forest (0400122V, Local)",           MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // 570/3,    E - 23/06/95
GAME( 1995, swthrt2v,  0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Sweet Hearts II (01J01986, Venezuela)",        MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // 577/1,    C - 07/09/95
GAME( 1996, minemine,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Mine, Mine, Mine (Export)",                    MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // 559/2,    E - 14/02/96
GAME( 1996, dolphntr,  0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Dolphin Treasure (0200424V, NSW/ACT)",         MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // 602/1,    B - 06/12/96, Rev 3
GAME( 1996, dolphtra,  dolphntr, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Dolphin Treasure (0100424V, NSW/ACT)",         MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // 602/1,    B - 06/12/96, Rev 1.24.4.0
GAME( 1996, dolphtre,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Dolphin Treasure (Export)",                    MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // 602/1,    B - 06/12/96
GAME( 1996, cashcham,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Cash Chameleon (Export)",                      MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // 603(a),   B - 06/12/96
GAME( 1997, goldprmd,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Golden Pyramids (MV4091, USA)",                MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // MV4091,   B - 13/05/97
GAME( 1997, qotn,      0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Queen of the Nile (0200439V, NSW/ACT)",        MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // 602/4,    B - 13/05/97
GAME( 1997, qotna,     aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Queen of the Nile (MV4091, NSW/ACT)",          MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // MV4091,   B - 13/05/97 (US-Export HW?)
GAME( 1997, wldcougr,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Wild Cougar (Export)",                         MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // 569/8,    D - 19/05/97
GAME( 1997, dmdtouch,  0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Diamond Touch (0400433V, Local)",              MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // 604,      E - 30/06/97
GAME( 1997, bumblbug,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Bumble Bugs (Export, 92.691%)",                MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // 593,      D - 05/07/97
GAME( 1997, pengpays,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Penguin Pays (Export)",                        MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // 586/7(b)  B - 14/07/97
GAME( 1998, adonis,    0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Adonis (0200751V, NSW/ACT)",                   MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // 602/9,    A - 25/05/98, Rev 10
GAME( 1998, adonisa,   adonis,   aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Adonis (0100751V, NSW/ACT)",                   MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // 602/9,    A - 25/05/98, Rev 9
GAME( 1998, reelrock,  0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Reelin-n-Rockin (0100779V, Local)",            MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // 628,      A - 13/07/98
GAME( 1998, indiandr,  0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Indian Dreaming (0100845V, Local)",            MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // 628/1,    B - 15/12/98
GAME( 1998, chariotc,  0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "The Chariot Challenge (04J00714, NSW/ACT)",    MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // 630,      A - 10/08/98, Rev 12
GAME( 1999, wtiger,    0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "White Tiger Classic (0200954V, NSW/ACT)",      MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // 638/1,    B - 08/07/99
GAME( 1999, bootsctn,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Boot Scootin' (Export, 92.767%)",              MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // MV4098,   A - 25/08/99
GAME( 2000, cuckoo,    aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Cuckoo (MV4104, Export)",                      MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // MV4104,   C - 02/02/00
GAME( 2000, magicmsk,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Magic Mask (MV4115, Export, set 1)",           MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // MV4115,   A - 09/05/00
GAME( 2000, magicmska, magicmsk, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Magic Mask (MV4115, Export, set 2)",           MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // MV4115,   A - 09/05/00
GAME( 2000, margmgc,   0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Margarita Magic (01J00101, NSW/ACT)",          MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // JB005,    A - 07/07/00
GAME( 2001, geishanz,  0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Geisha (0101408V, New Zealand)",               MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // MV4127,   A - 05/03/01
GAME( 2001, adonise,   aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Adonis (MV4124/1, Export)",                    MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // MV4124/1, B - 31/07/01
GAME( 2001, partygrs,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Party Gras (MV4115/6, Export)",                MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND )  // MV4115/6, A - 10/11/01
