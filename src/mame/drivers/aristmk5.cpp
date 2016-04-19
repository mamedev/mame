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

  U35: PHILIPS 74HC273.
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
  U85: ARM250: Computer system on a chip. ARM 32bit RISC processor with memory, video, and I/O controllers.
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

  The 96-pin female connector at the bottom of the ROM banks is intended for a sub board
  with two ROM sockets, that once plugged switch the ROM bank 0 with the sub board bank.
  Just to place the clear chips without removing the U7 & U11 EPROMS.

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
	virtual void machine_start() override;
	virtual void machine_reset() override;
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

		if(PRG!=nullptr)

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

// MV4033 - 10 Credit Multiplier / 9 Line Multiline.
// Enchanted Forest - Export B - 10/02/97.
// Marked as 94.97%
// All devices are 27c4002 instead of 27c4096.
ROM_START( enchfore )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "jhg041503_enchanted_forest.u7",  0x000000, 0x80000, CRC(cae1fb55) SHA1(386913ddf9be406f46aab06cf3e27c3c38a4d52d) )  // 94.97%
	ROM_LOAD32_WORD( "jhg041503_enchanted_forest.u11", 0x000002, 0x80000, CRC(a71b7b3c) SHA1(26c3438398b6a3cc9946a1cd1c92d317a8e2738e) )  // 94.97%
	ROM_LOAD32_WORD( "jhg041503_enchanted_forest.u8",  0x100000, 0x80000, CRC(002dec6c) SHA1(fb3f4ce9cd8cd9e0e3133376ed014db83db041c5) )  // base
	ROM_LOAD32_WORD( "jhg041503_enchanted_forest.u12", 0x100002, 0x80000, CRC(c968471f) SHA1(9d54a5c396e6f83690db2fcb7ddcc8a47a7dd777) )  // base

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

// MV4033 - 10 Credit Multiplier / 9 Line Multiline.
// Magic Garden - Export B - 10/02/97.
// Marked as AHG1211 and 88.26%
ROM_START( mgarden )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "ahg1211-99_magic_garden.u7",  0x000000, 0x80000, CRC(4fe50505) SHA1(6cde87a8a6748af792a1fb101829491367bd4487) )
	ROM_LOAD32_WORD( "ahg1211-99_magic_garden.u11", 0x000002, 0x80000, CRC(723ffeee) SHA1(9eab33c9dbf656489914e539a28da5ae289e8df7) )
	ROM_LOAD32_WORD( "ahg1211-99_magic_garden.u8",  0x100000, 0x80000, CRC(a315ca28) SHA1(0309789362a945d592ee2eda912e4fc2e6ea5be6) )
	ROM_LOAD32_WORD( "ahg1211-99_magic_garden.u12", 0x100002, 0x80000, CRC(4b252c2c) SHA1(8be41fb2b8f8d2829c18ea123a02f3e61c136206) )

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

// 602/2 - 10 Credit Multiplier / 20 Line Multiline.
// QUEEN OF THE NILE - NSW/ACT - B - 13/05/97.
// Marked as AHG1206-99, Golden Pyramids, and 87.928%
// Queen of The Nile and Golden Pyramids are
// both the same game with different title.
ROM_START( goldpyra )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "ahg1206-99_golden_pyramids.u7",  0x000000, 0x80000, CRC(e6c80f67) SHA1(901cf8f8fd46c1c4a70e1954d2d2d88e7acd07a8) )
	ROM_LOAD32_WORD( "ahg1206-99_golden_pyramids.u11", 0x000002, 0x80000, CRC(3cc221ea) SHA1(a71d16b818110f5b632e996e9f2fcb8be17b2aee) )
	ROM_LOAD32_WORD( "ahg1206-99_golden_pyramids.u8",  0x100000, 0x80000, CRC(df1ffb31) SHA1(1cf9d008b1f8fdb06ba050c97dae79f272c8063c) )
	ROM_LOAD32_WORD( "ahg1206-99_golden_pyramids.u12", 0x100002, 0x80000, CRC(d2c8f786) SHA1(a9efa35c8f2833a2b77f092398ca959d5fe6194e) )

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
	ROM_LOAD32_WORD( "bumble_bugs_export.u7",  0x000000, 0x80000, CRC(ec605a36) SHA1(114e0840cfbd0c64645a5a33065db85462a0ba2d) )  // 92.691%
	ROM_LOAD32_WORD( "bumble_bugs_export.u11", 0x000002, 0x80000, CRC(17b154bd) SHA1(efdf307670a3d74f7980fec2d2197d837d4c26e2) )  // 92.691%
	ROM_LOAD32_WORD( "bumble_bugs_export.u8",  0x100000, 0x80000, CRC(e0c01d01) SHA1(9153129fd348a97da7cccf002e5d03e4b4db9264) )  // base
	ROM_LOAD32_WORD( "bumble_bugs_export.u12", 0x100002, 0x80000, CRC(28700d5d) SHA1(87a583cd487da6cb4c2da5f62297f0e577269fae) )  // base

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

// 577/3 - 10 Credit Multiplier / 9 Line Multiline.
// Tropical Delight -  Export  D - 24/09/97.
// Marked as PHG0625-02 and 92.25%.
// All devices are 27c4002 instead of 27c4096.
ROM_START( trpdlght )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "phg0625-02_tropical_delight.u7",  0x000000, 0x80000, CRC(3d06765f) SHA1(737d714e4ec48eb6283489f745dd305e7d70dad2) )  // 92.25%
	ROM_LOAD32_WORD( "phg0625-02_tropical_delight.u11", 0x000002, 0x80000, CRC(3963a3de) SHA1(fc2b06af3d1eba87407425dc4296a8b602952775) )  // 92.25%
	ROM_LOAD32_WORD( "phg0625-02_tropical_delight.u8",  0x100000, 0x80000, CRC(d4858407) SHA1(acf6776f19448648a26aaf53fcb4bc227c546033) )  // base
	ROM_LOAD32_WORD( "phg0625-02_tropical_delight.u12", 0x100002, 0x80000, CRC(852e433e) SHA1(17ec568edbabe3ee8649b26f4c5d0f501494f823) )  // base

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

// 596 - 10 Credit Multiplier / 9 Line Multiline. (touch)
// Chicken - Export C - 23/02/98.
// Marked as RHG0730, 92.588% and 'touch'
// All devices are 27c4002 instead of 27c4096.
ROM_START( chickena )
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "rhg0730_chicken.u7",  0x000000, 0x80000, CRC(ca196b37) SHA1(6b204204c1574439ccea1b6145d867a73bad304f) )  // 92.588%
	ROM_LOAD32_WORD( "rhg0730_chicken.u11", 0x000002, 0x80000, CRC(b0d7be28) SHA1(6998dce808bf7970500b9e1ce6efed3940ee2d63) )  // 92.588%
	ROM_LOAD32_WORD( "rhg0730_chicken.u8",  0x100000, 0x80000, CRC(80e3e34c) SHA1(3ad73c5fc21c4d9647ea514bf367073bbeb981a9) )  // base
	ROM_LOAD32_WORD( "rhg0730_chicken.u12", 0x100002, 0x80000, CRC(63d5ec8e) SHA1(dca76342ecee6843e6fc656aafc8ee2e4d19fd65) )  // base
	ROM_LOAD32_WORD( "rhg0730_chicken.u9",  0x200000, 0x80000, CRC(662ff210) SHA1(bbd2410fa2cd67e327981c3b2e16342fb9393401) )  // base
	ROM_LOAD32_WORD( "rhg0730_chicken.u13", 0x200002, 0x80000, CRC(c3cef8ae) SHA1(4e65787d61387b511972e514047528495e1de11c) )  // base
	ROM_LOAD32_WORD( "rhg0730_chicken.u10", 0x300000, 0x80000, CRC(8b3f7d6b) SHA1(7f1a04556c448976145652b05b690142376764d4) )  // base
	ROM_LOAD32_WORD( "rhg0730_chicken.u14", 0x300002, 0x80000, CRC(240f7759) SHA1(1fa5ba0185b027101dae207ec5d28b07d3d73fc2) )  // base

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

// MV4061 - 5 Credit Multiplier / 5 Line Multiline.
// Sweethearts II - Export - A - 29/06/98.
// Marked as PHG0742 and 92.252%
// All devices are 27c4002 instead of 27c4096.
ROM_START( swheart2 )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "phg0742_sweet_heart_2.u7",  0x000000, 0x80000, CRC(d6f83014) SHA1(7c6902d67157a04bdbbfc7c7d8ae1e22befd840f) )
	ROM_LOAD32_WORD( "phg0742_sweet_heart_2.u11", 0x000002, 0x80000, CRC(3fa6e538) SHA1(958461a54e57c4622151bbcde3de8f4ff3f9ec0a) )
	ROM_LOAD32_WORD( "phg0742_sweet_heart_2.u8",  0x100000, 0x80000, CRC(916409f7) SHA1(d5c3cb7afac14a27f4722528a3dac4b4f2d41580) )
	ROM_LOAD32_WORD( "phg0742_sweet_heart_2.u12", 0x100002, 0x80000, CRC(92f92875) SHA1(bdb24974c2bf7bfb772c34a02a20e97df9293c0c) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

// MV4084/1 - 10 Credit Multiplier / 9 Line Multiline.
// THE GAMBLER - Export  A - 30/10/98.
// Marked as EHG0916 and 92.268%.
// All devices are 27c4002 instead of 27c4096.
ROM_START( thgamblr )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "ehg0916_the_gambler.u7",  0x000000, 0x80000, CRC(7524c954) SHA1(0a895d1e2d09a2c873bbbbeb37bc59c25f3c577c) )  // 92.268%
	ROM_LOAD32_WORD( "ehg0916_the_gambler.u11", 0x000002, 0x80000, CRC(f29a6932) SHA1(17761218a04d36a599c987b4e13c0e3f46b7793f) )  // 92.268%
	ROM_LOAD32_WORD( "ehg0916_the_gambler.u8",  0x100000, 0x80000, CRC(e2221fdf) SHA1(8a7b2d5de68ae66fe1915a6faac6277249e3fb53) )  // base
	ROM_LOAD32_WORD( "ehg0916_the_gambler.u12", 0x100002, 0x80000, CRC(ebe957f9) SHA1(539945ec9beafe2c83051208370588fce2334f16) )  // base

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
	ROM_LOAD32_WORD( "mv4104_cuckoo.u7",  0x000000, 0x80000, CRC(0bd17338) SHA1(b8f467bdf8d76533a2b7d44fe93be414f25a3c31) )
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

// US003 - Multi credit / Multi line.
// Margarita Magic [Reel Game] - NSW/ACT - A - 07/07/2000.
// EHG1559 - This is a twenty-line game.
// The playlines are 1, 5, 10, 15 and 20.
// For 20 credit per line the max bet is 400
ROM_START( marmagic )
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "ehg1559_margarita_magic.u7",  0x000000, 0x80000, CRC(eab62e8f) SHA1(b125f9a9dc1c82886490d3807e883a7b4e1453a5) )
	ROM_LOAD32_WORD( "ehg1559_margarita_magic.u11", 0x000002, 0x80000, CRC(0b3c6a11) SHA1(05be4a4d070358600273d5dd4f6b4b37fee47105) )
	ROM_LOAD32_WORD( "ehg1559_margarita_magic.u8",  0x100000, 0x80000, CRC(db05591e) SHA1(8af241bbd4f744c66fb78fdaf739d9c8bc2580c0) )
	ROM_LOAD32_WORD( "ehg1559_margarita_magic.u12", 0x100002, 0x80000, CRC(b4458167) SHA1(d1e2040910ad748e58eaccd18ab0569b794b4d97) )
	ROM_LOAD32_WORD( "ehg1559_margarita_magic.u9",  0x200000, 0x80000, CRC(fc69523a) SHA1(c01b3c905b01671307bc5439d00f4454d0286b20) )
	ROM_LOAD32_WORD( "ehg1559_margarita_magic.u13", 0x200002, 0x80000, CRC(0cd174df) SHA1(707168fc3bef6c200ae6455c170b7c3e73502965) )

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

// MV4137 - 5,10,25,50 Credit Multiplier / 20 Line Multiline.
// Koala Mint [Reel Game] - Export A - 12/09/01.
// Marked as CHG1573.
ROM_START( koalamnt )
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "chg1573_koala_mint.u7",  0x000000, 0x80000, CRC(fa690af0) SHA1(9e1e5171e9da602c025bfb2aefad397a537794cb) )
	ROM_LOAD32_WORD( "chg1573_koala_mint.u11", 0x000002, 0x80000, CRC(c33bed43) SHA1(2c8f35ca08b4d6ac56de5ab7c2515f34e04cf6c8) )
	ROM_LOAD32_WORD( "chg1573_koala_mint.u8",  0x100000, 0x80000, CRC(4aeb2e54) SHA1(74002cd12d93352310a864a2ed434c7f43d26534) )  // base
	ROM_LOAD32_WORD( "chg1573_koala_mint.u12", 0x100002, 0x80000, CRC(2bf5786f) SHA1(f0693bbd2e6d2e110535205a1ad0b73a0ebd2f53) )  // base
	ROM_LOAD32_WORD( "chg1573_koala_mint.u9",  0x200000, 0x80000, CRC(1a2650e7) SHA1(55a8604ef19836880f53d44a035a49b009acbb5a) )  // base
	ROM_LOAD32_WORD( "chg1573_koala_mint.u13", 0x200002, 0x80000, CRC(51c78f63) SHA1(ef51e45d67a5684c35150747c186493258cb4549) )  // base
	ROM_LOAD32_WORD( "chg1573_koala_mint.u10", 0x300000, 0x80000, CRC(a0fb61fe) SHA1(2a77ed082bc6829905f83a3cb3c4c120fa4ba0f9) )  // base
	ROM_LOAD32_WORD( "chg1573_koala_mint.u14", 0x300002, 0x80000, CRC(5e4776e9) SHA1(d44851cbfaa054cd5675a841a3089a8f4fdc8421) )  // base

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

// MV4115/3 - 20 Line Multiline / 3,5,10,20,25,50 Credit Multiplier.
// Party Gras - Export  B - 06/02/2001.
// Marked as BHG1284 and 'touch'.
ROM_START( prtygras )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "bhg1284_party_grass.u7",  0x000000, 0x80000, CRC(02ed0631) SHA1(ae2c89c876a030d325ec94490d293deba772630e) )
	ROM_LOAD32_WORD( "bhg1284_party_grass.u11", 0x000002, 0x80000, CRC(7ac80cd9) SHA1(70e910784a1e1ea8820005082e76223a85a3c346) )
	ROM_LOAD32_WORD( "bhg1284_party_grass.u8",  0x100000, 0x80000, CRC(28774b9a) SHA1(ebdd738a73ffa7c5238640f4d7956751f7bb6243) )
	ROM_LOAD32_WORD( "bhg1284_party_grass.u12", 0x100002, 0x80000, CRC(942835c1) SHA1(fefc509311716559ac6b836a56b2c981907d499b) )

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


#define ROM_END_REGIONS \
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */ \
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 ) \
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 ) \
ROM_END


ROM_START( badbog )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Bad_Bog_1of6.U07.BIN", 0x0000000, 0x0080000, CRC(25aa8109) SHA1(cf4521b3d447812d2d9dbfdab9fe0cec71cdeb2e) )
	ROM_LOAD32_WORD("Bad_Bog_2of6.U11.BIN", 0x0000002, 0x0080000, CRC(774ff977) SHA1(5ce1aa8b7598b4bc8e5fa44de1c03b5f2851f5de) )
	ROM_LOAD32_WORD("Bad_Bog_3of6.U08.BIN", 0x0100000, 0x0080000, CRC(e52a279a) SHA1(4a3a080d840d8a894ec0ba0250a566831377f0f8) )
	ROM_LOAD32_WORD("Bad_Bog_4of6.U12.BIN", 0x0100002, 0x0080000, CRC(562aa123) SHA1(825a2d23321b636a3ff2565b2b72df3b97bd0ec8) )
	ROM_LOAD32_WORD("Bad_Bog_5of6.U09.BIN", 0x0200000, 0x0080000, CRC(66d5a7f7) SHA1(1a1f845a97677c43ff1090231434ae9d3d36ab4c) )
	ROM_LOAD32_WORD("Bad_Bog_6of6.U13.BIN", 0x0200002, 0x0080000, CRC(883b2ec3) SHA1(5b431d8c9c8eabca65ab22dcf2bdb22d49445bb1) )
ROM_END_REGIONS





ROM_START( blackpnt )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Black Panther NSW U11.BIN", 0x0000002, 0x0080000, CRC(de3358d3) SHA1(4f290940d8af9fe8d404802d5cecfd2d098eff12) )
	ROM_LOAD32_WORD("Black Panther NSW U12.BIN", 0x0100002, 0x0080000, CRC(bb2bf7bb) SHA1(f88208238a69fc79e33af17f39e25cd2857d7172) )
	ROM_LOAD32_WORD("Black Panther NSW U7.bin", 0x0000000, 0x0080000, CRC(eed76145) SHA1(6a40a6ba2ce320a37b086dc4916c92c8e38c065e) )
	ROM_LOAD32_WORD("Black Panther NSW U8.bin", 0x0100000, 0x0080000, CRC(58ddfb50) SHA1(c2152e65fa119136b7944b69e650310db78e62a8) )
ROM_END_REGIONS



ROM_START( bootscot )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Boot Scootin' U10.bin", 0x0300000, 0x0080000, CRC(b574c12d) SHA1(3b1d1d00ef3eae23493e2b0381ab80490af510d4) )
	ROM_LOAD32_WORD("Boot Scootin' U11.BIN", 0x0000002, 0x0080000, CRC(df066d27) SHA1(310422c78e93ce9f1f58b4a58a59bc2eba5c502a) )
	ROM_LOAD32_WORD("Boot Scootin' U12.BIN", 0x0100002, 0x0080000, CRC(87ddc7ef) SHA1(91473d8fd266a909fa8d4ec3df3a61861c6e9f4c) )
	ROM_LOAD32_WORD("Boot Scootin' U13.BIN", 0x0200002, 0x0080000, CRC(fca82ee7) SHA1(bb70f2e04047a58b697dca536b95f9bbcc295a8a) )
	ROM_LOAD32_WORD("Boot Scootin' U14.bin", 0x0300002, 0x0080000, CRC(75b9b89e) SHA1(08d487b3722f2ea5d2d18c78f571a44c78616dbe) )
	ROM_LOAD32_WORD("Boot Scootin' U7.BIN", 0x0000000, 0x0080000, CRC(f8e12462) SHA1(82a25757b2146204b86e557b8f1c45280e0668a8) )
	ROM_LOAD32_WORD("Boot Scootin' U8.bin", 0x0100000, 0x0080000, CRC(08e8de8d) SHA1(913d3e51821d8885affd2750c18d1000629b79d9) )
	ROM_LOAD32_WORD("Boot Scootin' U9.bin", 0x0200000, 0x0080000, CRC(a1ca5f2b) SHA1(c8fc6aff0c3819370339143966ec76910e40c671) )
ROM_END_REGIONS




ROM_START( bumbugs )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Bumble Bugs U11.BIN", 0x0000002, 0x0080000, CRC(5d888245) SHA1(bbbe61e09bebd5fcb79f060d5caee15100c9a685) )
	ROM_LOAD32_WORD("Bumble Bugs U7.bin", 0x0000000, 0x0080000, CRC(d4cfce73) SHA1(735c385779afe55e521dbfe9ebfdc55fe3346349) )
ROM_END_REGIONS




ROM_START( butdeli )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Butterfly Delight  U11.BIN", 0x0000002, 0x0080000, CRC(1ddf8732) SHA1(dc09db14c251699fdd46068f18ad6214e8752939) )
	ROM_LOAD32_WORD("Butterfly Delight  U12.BIN", 0x0100002, 0x0080000, CRC(0d58cf28) SHA1(aa65b7ee88b5bc872008a46e60bd49d9e5eda153) )
	ROM_LOAD32_WORD("Butterfly Delight  U7.bin", 0x0000000, 0x0080000, CRC(7f69cdfc) SHA1(1241741d21334df10d60080555824a87eae93db3) )
	ROM_LOAD32_WORD("Butterfly Delight  U8.bin", 0x0100000, 0x0080000, CRC(24d8135e) SHA1(1bc69e9927afe0300d15a49ca06ae527774b295a) )
ROM_END_REGIONS



ROM_START( cashchm )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("CASHCHAMELEON_U11.BIN", 0x0000002, 0x0080000, CRC(64921874) SHA1(5aa6a0d6e29f5e400e275f27b6adfbef595fe83a) )
	ROM_LOAD32_WORD("CASHCHAMELEON_U12.BIN", 0x0100002, 0x0080000, CRC(7ae3b5db) SHA1(238698b72f529ac4fb292d08267069d1da01b43b) )
	ROM_LOAD32_WORD("CASHCHAMELEON_U7.bin", 0x0000000, 0x0080000, CRC(c942ef22) SHA1(4f56674f749602ae928832f98a641e680af8989b) )
	ROM_LOAD32_WORD("CASHCHAMELEON_U8.bin", 0x0100000, 0x0080000, CRC(a8868277) SHA1(e199448a0a920219dc15443813061653b94d6d3a) )
ROM_END_REGIONS



ROM_START( cashcroa )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("cash_crop_1of4.U07.BIN", 0x0000000, 0x0080000, CRC(b0ff2aae) SHA1(b05667ffe952cae7a6581398552db6e47921090e) )
	ROM_LOAD32_WORD("cash_crop_2of4.U11.BIN", 0x0000002, 0x0080000, CRC(25a18efa) SHA1(0ee4f6cc66322397dbde53af2149f5fb35d788df) )
	ROM_LOAD32_WORD("cash_crop_3of4.U08.BIN", 0x0100000, 0x0080000, CRC(d4e7b4ba) SHA1(147a1ed5cdcbb84466a8024ad7e0778f85374489) )
	ROM_LOAD32_WORD("cash_crop_4of4.U12.BIN", 0x0100002, 0x0080000, CRC(570c7f8a) SHA1(7c9527e0b37970b7960c723727c3c650a48e8125) )
ROM_END_REGIONS





ROM_START( chkrun )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Chicken Run U11.BIN", 0x0000002, 0x0080000, CRC(65423867) SHA1(992bb4f717f79233d1300d248b145f95a627cff2) )
	ROM_LOAD32_WORD("Chicken Run U12.BIN", 0x0100002, 0x0080000, CRC(77b5d777) SHA1(f03afeaff08c9216e714f1e4bcc50292ba87ace4) )
	ROM_LOAD32_WORD("Chicken Run U13.BIN", 0x0200002, 0x0080000, CRC(88a1ccae) SHA1(e242f48f99044b4fdf1bf36d8e105df09f94aa50) )
	ROM_LOAD32_WORD("Chicken Run u7.bin", 0x0000000, 0x0080000, CRC(be69c21c) SHA1(8b546727b5972f33d077db0a64aa41a7fde6d417) )
	ROM_LOAD32_WORD("Chicken Run U8.bin", 0x0100000, 0x0080000, CRC(3161c16f) SHA1(8f2b14ec8ba5c9da80a226d2ce5a7e5256c8cbb4) )
	ROM_LOAD32_WORD("Chicken Run U9.bin", 0x0200000, 0x0080000, CRC(5506777b) SHA1(42512577056e1caefbea0e74879780c56787af13) )
ROM_END_REGIONS






ROM_START( coralr2v )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Coral Riches 2 U11.BIN", 0x0000002, 0x0080000, CRC(8cd17e90) SHA1(c6d6a29e62ca6e1b278a2e1d1b358e10ca2de4ed) )
	ROM_LOAD32_WORD("Coral Riches 2 U12.BIN", 0x0100002, 0x0080000, CRC(9ea140b5) SHA1(11f6b9ab60117f236b464c9dbc939dfb8f240359) )
	ROM_LOAD32_WORD("Coral Riches 2 U7.bin", 0x0000000, 0x0080000, CRC(02c430c3) SHA1(f4bae1aa5437af1df2a04f700da044bc4fb652b7) )
	ROM_LOAD32_WORD("Coral Riches 2 U8.bin", 0x0100000, 0x0080000, CRC(1ee9557c) SHA1(3bee295509d4b0c11ce41a7a20ba91230b7cb4ca) )
ROM_END_REGIONS



ROM_START( cuckooa )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Cuckoo_1of4_U07.BIN", 0x0000000, 0x0080000, CRC(5c7ef84a) SHA1(59563a076ecf391ac1779e0dcd530a1ea158a4e3) )
	ROM_LOAD32_WORD("Cuckoo_2of4_U11.BIN", 0x0000002, 0x0080000, CRC(a69c1416) SHA1(7fe57a194bf29346c039dfac1326f3ee5080e630) )
	ROM_LOAD32_WORD("Cuckoo_3of4_U08.BIN", 0x0100000, 0x0080000, CRC(a7b4242c) SHA1(4e6961e9b3267d17b93075c41a691a8033a34d90) )
	ROM_LOAD32_WORD("Cuckoo_4of4_U12.BIN", 0x0100002, 0x0080000, CRC(cb706eb7) SHA1(cbd6235ca7a29c78ef2cb659d9c21466ed39b360) )
ROM_END_REGIONS




ROM_START( dstblom )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Desert Bloom  U11.BIN", 0x0000002, 0x0080000, CRC(ed4e8dca) SHA1(1953033e570634cbcf8cd11194c14c57ffc6be53) )
	ROM_LOAD32_WORD("Desert Bloom  U12.BIN", 0x0100002, 0x0080000, CRC(0ad41815) SHA1(131efc6ed45d8f44a667bd30380c9e37c64f2c42) )
	ROM_LOAD32_WORD("Desert Bloom  U7.bin", 0x0000000, 0x0080000, CRC(fbfaa3fe) SHA1(3f915261503fc97eb556422e9ccdac81372c04cc) )
	ROM_LOAD32_WORD("Desert Bloom  U8.bin", 0x0100000, 0x0080000, CRC(cc0d567c) SHA1(c4da3d0c0c4420a9f8fbb6403db983b3e27d4b50) )
ROM_END_REGIONS



ROM_START( dmddove )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Diamond Dove  U11.BIN", 0x0000002, 0x0080000, CRC(ff4c684a) SHA1(6598c24a8717b8e624e387f000c584ec3b10a8cd) )
	ROM_LOAD32_WORD("Diamond Dove  U12.BIN", 0x0100002, 0x0080000, CRC(62209e81) SHA1(68383068de2e030467c3f3ac16459ae2f3b2cce6) )
	ROM_LOAD32_WORD("Diamond Dove  U13.BIN", 0x0200002, 0x0080000, CRC(952a850f) SHA1(66da391af532f9ef531d10995c96a90eb71cd09a) )
	ROM_LOAD32_WORD("Diamond Dove  U7.bin", 0x0000000, 0x0080000, CRC(2ebb3704) SHA1(42567d873d6ab9221d09e5449fa57b557677d2ab) )
	ROM_LOAD32_WORD("Diamond Dove  U8.bin", 0x0100000, 0x0080000, CRC(daa55b3b) SHA1(7aa96a51a3ea9f96c38d08e486eccc54ca4396a3) )
	ROM_LOAD32_WORD("Diamond Dove  U9.bin", 0x0200000, 0x0080000, CRC(2254f0e9) SHA1(5bccd65e7e616e1f6ed08a0c84862cb13f9f7098) )
ROM_END_REGIONS





ROM_START( drawpka )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("M Draw Free Poker Game  U11.BIN", 0x0000002, 0x0080000, CRC(ac8503fa) SHA1(30640a9c01239173c7430a46dcd2e2b28024c0cf) )
	ROM_LOAD32_WORD("M Draw Free Poker Game  U12.BIN", 0x0100002, 0x0080000, CRC(cd0dfdf5) SHA1(7bcf77c1bcd023b4ab08cef329dcf39dc2ca09d6) )
	ROM_LOAD32_WORD("M Draw Free Poker Game  U13.BIN", 0x0200002, 0x0080000, CRC(0d6f7ec5) SHA1(0a80257eb464e50292554f45583f3d7b85de2bc3) )
	ROM_LOAD32_WORD("M Draw Free Poker Game  U7.bin", 0x0000000, 0x0080000, CRC(7570eb03) SHA1(0fded55ee2d12cfae96e2910c68a131cd89147a0) )
	ROM_LOAD32_WORD("M Draw Free Poker Game  U8.bin", 0x0100000, 0x0080000, CRC(8c54bd65) SHA1(5870558f8b96fca2c355ccc6ffc09fc4684d141c) )
	ROM_LOAD32_WORD("M Draw Free Poker Game  U9.bin", 0x0200000, 0x0080000, CRC(10b96156) SHA1(1f89e0d8d210d2fd7e0b78b0205eb626d7c39542) )
ROM_END_REGIONS




ROM_START( dyjack )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Dynamite Jack_U11.BIN", 0x0000002, 0x0080000, CRC(5a0147ae) SHA1(f2135b2525eb50a03a8f6360e7edb92bf0b88740) )
	ROM_LOAD32_WORD("Dynamite Jack_U12.BIN", 0x0100002, 0x0080000, CRC(beee94ff) SHA1(fad0d3506d10330840d3e5fcdfd7f0aa20041969) )
	ROM_LOAD32_WORD("Dynamite Jack_U13.BIN", 0x0200002, 0x0080000, CRC(d204ff9c) SHA1(8ac5533928fb3ca247dc85cea67da45a6743f732) )
	ROM_LOAD32_WORD("Dynamite Jack_U7.bin", 0x0000000, 0x0080000, CRC(73783ecf) SHA1(280b4da540b405959f31c2eebbf87ab635d21c06) )
	ROM_LOAD32_WORD("Dynamite Jack_U8.bin", 0x0100000, 0x0080000, CRC(e686eab2) SHA1(6eb18adda82357ff84f77e9334733094430dfdc6) )
	ROM_LOAD32_WORD("Dynamite Jack_U9.bin", 0x0200000, 0x0080000, CRC(28a45170) SHA1(d7bb8e4dd24e3a3acf44e7fc40e49ebee5c15ec9) )
ROM_END_REGIONS



ROM_START( eldora )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("U11 El Dorado.bin", 0x0000002, 0x0080000, CRC(35233cf8) SHA1(e02477526f2f9e2663c1876f543d138b2caf28df) )
	ROM_LOAD32_WORD("U7 El Dorado.bin", 0x0000000, 0x0080000, CRC(d9afe87c) SHA1(577ea5da9c4e93a393711a0c7361365301f4241e) )
ROM_END_REGIONS







ROM_START( fortela )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Fortune_Teller_U10.bin", 0x0300000, 0x0080000, CRC(f2790419) SHA1(8720c37cc678e7c5666c67b9998fbb460a8aad90) )
	ROM_LOAD32_WORD("Fortune_Teller_U11.BIN", 0x0000002, 0x0080000, CRC(faab1283) SHA1(6200fc2047c4052e4fc3c2d28b26cd9ff67a08be) )
	ROM_LOAD32_WORD("Fortune_Teller_U12.BIN", 0x0100002, 0x0080000, CRC(fe5af3ac) SHA1(f08fe353c871ac4375f0fa25bf15f2638b33a370) )
	ROM_LOAD32_WORD("Fortune_Teller_U13.BIN", 0x0200002, 0x0080000, CRC(d0dd6627) SHA1(ea855da1759a27936615400993b381609071d66c) )
	ROM_LOAD32_WORD("Fortune_Teller_U14.bin", 0x0300002, 0x0080000, CRC(507bbe10) SHA1(01b1982c02a00b60aa39ee1b408d653365f728d4) )
	ROM_LOAD32_WORD("Fortune_Teller_U7.bin", 0x0000000, 0x0080000, CRC(78394106) SHA1(aedfb98d7aa515eebabf378edb9c43e01bcba010) )
	ROM_LOAD32_WORD("Fortune_Teller_U8.bin", 0x0100000, 0x0080000, CRC(7ce4ba38) SHA1(43b57e4dc96851f58d95e4f1b99d08f559e27f6a) )
	ROM_LOAD32_WORD("Fortune_Teller_U9.bin", 0x0200000, 0x0080000, CRC(a43cd994) SHA1(759fecc809ca1b038d782b173d5638d9be165f9a) )
ROM_END_REGIONS




ROM_START( genmagi )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("GENIEMAGIC_U10.bin", 0x0300000, 0x0080000, CRC(ea460e72) SHA1(4546e04cc04239528c93e22532db08fccebda8a8) )
	ROM_LOAD32_WORD("GENIEMAGIC_U11.BIN", 0x0000002, 0x0080000, CRC(88c304a3) SHA1(013d5d1d62b356ce5cdf0c9b036c4ca09f191668) )
	ROM_LOAD32_WORD("GENIEMAGIC_U12.BIN", 0x0100002, 0x0080000, CRC(44adc422) SHA1(81256ddebb29fbd69cab8e642faac39635dd1739) )
	ROM_LOAD32_WORD("GENIEMAGIC_U13.BIN", 0x0200002, 0x0080000, CRC(26f51647) SHA1(e980c021d8e2d295ba2d50446b36b85f42d3f318) )
	ROM_LOAD32_WORD("GENIEMAGIC_U14.bin", 0x0300002, 0x0080000, CRC(52092ffb) SHA1(6ed591a510e9186588470ec745caf8001712012e) )
	ROM_LOAD32_WORD("GENIEMAGIC_U7.bin", 0x0000000, 0x0080000, CRC(20ec3b50) SHA1(400ad7f86077184fee63690060fe2a51ba888e1b) )
	ROM_LOAD32_WORD("GENIEMAGIC_U8.bin", 0x0100000, 0x0080000, CRC(341bac7b) SHA1(67df39b8070f6d9afd183b04239d9e2844d588c5) )
	ROM_LOAD32_WORD("GENIEMAGIC_U9.bin", 0x0200000, 0x0080000, CRC(ce051dbd) SHA1(433717c5689dc865c1e42669a50e138eae017362) )
ROM_END_REGIONS




ROM_START( gnomatw )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Gnome The World U11.BIN", 0x0000002, 0x0080000, CRC(737d7178) SHA1(df788eea23b15415adc94543476b6ad982c4d79b) )
	ROM_LOAD32_WORD("Gnome The World U12.BIN", 0x0100002, 0x0080000, CRC(49eb3869) SHA1(d98fe385c667872f26d656a3240f557a70ba924f) )
	ROM_LOAD32_WORD("Gnome The World U7.bin", 0x0000000, 0x0080000, CRC(a5d3825e) SHA1(4ce7466eff770a2c6c3c5de620a14e05bb9fb406) )
	ROM_LOAD32_WORD("Gnome The World U8.bin", 0x0100000, 0x0080000, CRC(fe59ec8b) SHA1(b43778b51a0d695c179fa63ce45a47b9f550fb97) )
ROM_END_REGIONS





ROM_START( goldra )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Golden_Ra_1of6.U07.BIN", 0x0000000, 0x0080000, CRC(2f75d5f7) SHA1(d7f6ecff7cf759d80733b6d3f224caa5128be0b7) )
	ROM_LOAD32_WORD("Golden_Ra_2of6.U11.BIN", 0x0000002, 0x0080000, CRC(06a871c7) SHA1(95464d74c2295196e367e34efb816acedcd71265) )
	ROM_LOAD32_WORD("Golden_Ra_3of6.U08.BIN", 0x0100000, 0x0080000, CRC(940eabd7) SHA1(8d41b3fa27c827a7671b095618ac53750e6017f6) )
	ROM_LOAD32_WORD("Golden_Ra_4of6.U12.BIN", 0x0100002, 0x0080000, CRC(21c4a2d2) SHA1(77a24a5f98aad090223d301919645b5011667c28) )
	ROM_LOAD32_WORD("Golden_Ra_5of6.U09.BIN", 0x0200000, 0x0080000, CRC(b1cac0e7) SHA1(87f393a75c09e96a7fb893a767edcc81044e4fe3) )
	ROM_LOAD32_WORD("Golden_Ra_6of6.U13.BIN", 0x0200002, 0x0080000, CRC(8f62ccc5) SHA1(5105313192ab8dfd522b921c70b8b03a8a61ac63) )
ROM_END_REGIONS





ROM_START( hrttrhb )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Heart Throb U11.BIN", 0x0000002, 0x0080000, CRC(bde067d7) SHA1(cbf2cbd0644f1daeb5c3cd08d72f3d7aafe521ec) )
	ROM_LOAD32_WORD("Heart Throb U7.bin", 0x0000000, 0x0080000, CRC(de4d6d77) SHA1(959ffb7d06359870e07cb9d761f0bc0480c45e0c) )
ROM_END_REGIONS





ROM_START( incsun )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("INCASUN_NSW_U11.BIN", 0x0000002, 0x0080000, CRC(f51b411d) SHA1(fbbd587c90cd49bb36653cbd1948bc52f8396a41) )
	ROM_LOAD32_WORD("INCASUN_NSW_U12.BIN", 0x0100002, 0x0080000, CRC(0fa00c41) SHA1(79139834d5437b37346322bf632904c473e3463a) )
	ROM_LOAD32_WORD("INCASUN_NSW_U13.BIN", 0x0200002, 0x0080000, CRC(00407593) SHA1(4c759fe3267b1782ae84d8ed9134295dfaa0faaf) )
	ROM_LOAD32_WORD("INCASUN_NSW_U7.bin", 0x0000000, 0x0080000, CRC(180e098b) SHA1(48782c46a344dba0aaad407d0d4a432da091b0f5) )
	ROM_LOAD32_WORD("INCASUN_NSW_U8.bin", 0x0100000, 0x0080000, CRC(0c19f5ec) SHA1(95d7c9308b30b5193816e95c4276829612040298) )
	ROM_LOAD32_WORD("INCASUN_NSW_U9.bin", 0x0200000, 0x0080000, CRC(c82da820) SHA1(98a2710b1f793a7ee1070f89c66d49ce55e4156e) )
ROM_END_REGIONS







ROM_START( kookabk )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Kooka_Bucks_1of4.U07.BIN", 0x0000000, 0x0080000, CRC(b2fdf0e8) SHA1(0dd002cfad2fa4f217a0c67066d098f4cd3ba319) )
	ROM_LOAD32_WORD("Kooka_Bucks_2of4.U11.BIN", 0x0000002, 0x0080000, CRC(e8ab9afc) SHA1(4c3beefeafc6ac9d4538254bb5e01c12b35db922) )
	ROM_LOAD32_WORD("Kooka_Bucks_3of4.U08.BIN", 0x0100000, 0x0080000, CRC(f5a45c57) SHA1(a452a7359af6d5fde2c37946ee68807152f07d39) )
	ROM_LOAD32_WORD("Kooka_Bucks_4of4.U12.BIN", 0x0100002, 0x0080000, CRC(b2f2fd15) SHA1(9614f3ae6e82a40ecf44090d0b8d7bd8b6b1f830) )
ROM_END_REGIONS




ROM_START( locoloot )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Loco Loot U11.BIN", 0x0000002, 0x0080000, CRC(21332a1a) SHA1(76a4c30d1c9624984175e9bd117c68c9204f01df) )
	ROM_LOAD32_WORD("Loco Loot U7.bin", 0x0000000, 0x0080000, CRC(4f02763c) SHA1(302cea5fb157f65fc907f123ef42a0a38cc707ac) )
ROM_END_REGIONS


ROM_START( lonwolf )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Lone Wolf  U11.BIN", 0x0000002, 0x0080000, CRC(0ed6fb6b) SHA1(a2baa4154fe762e2c1b40a97b2d27265df8b5dab) )
	ROM_LOAD32_WORD("Lone Wolf  U7.bin", 0x0000000, 0x0080000, CRC(15024eae) SHA1(7101125aa8531c75f9d80fe357013d09dbb0fec9) )
ROM_END_REGIONS




ROM_START( mgctouc )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Magic Touch U11.BIN", 0x0000002, 0x0080000, CRC(614984e4) SHA1(e95d576993e8d9c0964899a7d5556c8e62d79242) )
	ROM_LOAD32_WORD("Magic Touch U12.BIN", 0x0100002, 0x0080000, CRC(f54c18db) SHA1(85bcc202f7425b3b7ef456c1c2db5a22648068a8) )
	ROM_LOAD32_WORD("Magic Touch U13.BIN", 0x0200002, 0x0080000, CRC(cfd2a86e) SHA1(66891a1b0e85ad7146b733f4b5d806db789d8821) )
	ROM_LOAD32_WORD("Magic Touch U7.bin", 0x0000000, 0x0080000, CRC(9fa3ee86) SHA1(ce7546b8d1dbf90eb8f4f8d3255dc1c215c966a7) )
	ROM_LOAD32_WORD("Magic Touch U8.bin", 0x0100000, 0x0080000, CRC(d7faf84d) SHA1(d2e49787d177767671fab64a723e1af619ce9ad2) )
	ROM_LOAD32_WORD("Magic Touch U9.bin", 0x0200000, 0x0080000, CRC(0e140453) SHA1(8b516fe598c7e754a471246effa1185845495640) )
ROM_END_REGIONS





ROM_START( monmous )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("MONEYMOUSE_U11.BIN", 0x0000002, 0x0080000, CRC(ac2243ea) SHA1(27c31e5102defa4f3982875b30a67e89af40d4ff) )
	ROM_LOAD32_WORD("MONEYMOUSE_U12.BIN", 0x0100002, 0x0080000, CRC(72d992ed) SHA1(94560305dacbe776ddc95114ad5e5ffaa234937c) )
	ROM_LOAD32_WORD("MONEYMOUSE_U7.bin", 0x0000000, 0x0080000, CRC(7f7972b6) SHA1(25991f476f55cd1eddc8e63af9c472c1d7e83481) )
	ROM_LOAD32_WORD("MONEYMOUSE_U8.bin", 0x0100000, 0x0080000, CRC(a10a4bff) SHA1(e6b36542dab8a3405579b333a125a6d3fd801b50) )
ROM_END_REGIONS





ROM_START( moutmon )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Mountain Money U11.BIN", 0x0000002, 0x0080000, CRC(4fb2a4dc) SHA1(23895b701387f7442a31969989d21cefe2a25efd) )
	ROM_LOAD32_WORD("Mountain Money U7.bin", 0x0000000, 0x0080000, CRC(b84342af) SHA1(e27e65730ddc897b01e8875a4da3ea2d6db2b858) )
ROM_END_REGIONS




ROM_START( mystgrd )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Mystic Garden U11.BIN", 0x0000002, 0x0080000, CRC(6e618fc5) SHA1(a02e7ca2433cf8128d74792833d9708a3ba5df4b) )
	ROM_LOAD32_WORD("Mystic Garden U7.bin", 0x0000000, 0x0080000, CRC(28d15442) SHA1(ee33017f3efcf688a43ea1d7f2b74b4b9a6d2cae) )
ROM_END_REGIONS





ROM_START( orchidms )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Orchid Mist  U11.BIN", 0x0000002, 0x0080000, CRC(fe79410b) SHA1(c91a0ce0cf87db518f910e9f47cabdcb91dc5496) )
	ROM_LOAD32_WORD("Orchid Mist  U12.BIN", 0x0100002, 0x0080000, CRC(165a762d) SHA1(8487d2e32bd2fab5a9114380ba2be6d34b097b11) )
	ROM_LOAD32_WORD("Orchid Mist  U7.bin", 0x0000000, 0x0080000, CRC(5d18ae22) SHA1(c10f7a83f51cfe75653ace8066b7dedf07e91b28) )
	ROM_LOAD32_WORD("Orchid Mist  U8.bin", 0x0100000, 0x0080000, CRC(09ec43e3) SHA1(947ed0982a148e6906666378e8c82315d40237d7) )
ROM_END_REGIONS





ROM_START( oscarar )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Oscar U11.BIN", 0x0000002, 0x0080000, CRC(11394e80) SHA1(1c6e7e954a6118e04da9d761fef8ec00c46d2af8) )
	ROM_LOAD32_WORD("Oscar U7.bin", 0x0000000, 0x0080000, CRC(930bdc00) SHA1(36b1a289abebc7cce64e767e201d8f8f7fe80cf2) )
ROM_END_REGIONS




ROM_START( pantmag )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Panther Magic 200 U11.BIN", 0x0000002, 0x0080000, CRC(0914594c) SHA1(b1bc1302847e3ea3c4ed96ae17047da031e5ca1a) )
	ROM_LOAD32_WORD("Panther Magic 200 U12.BIN", 0x0100002, 0x0080000, CRC(eae75fa9) SHA1(576c8cf98ad4032bbdde12162e2c1bdd10056762) )
	ROM_LOAD32_WORD("Panther Magic 200 U7.BIN", 0x0000000, 0x0080000, CRC(6383899d) SHA1(df96af7cb580565715da6e78b83e7ba6832028e7) )
	ROM_LOAD32_WORD("Panther Magic 200 U8.bin", 0x0100000, 0x0080000, CRC(db840d1b) SHA1(26ff790cd21f2005ae3a3e879ef07b87c8ae0020) )
ROM_END_REGIONS




ROM_START( peaflut )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("PEACOCKFLUTTER_U11.BIN", 0x0000002, 0x0080000, CRC(3134818c) SHA1(6fe158608b5da648fafd20cbcd213e6f2dc2104c) )
	ROM_LOAD32_WORD("PEACOCKFLUTTER_U12.BIN", 0x0100002, 0x0080000, CRC(2d96c449) SHA1(af98a864b9ed3f95227fd0d6edc6a38c0544c93f) )
	ROM_LOAD32_WORD("PEACOCKFLUTTER_U7.bin", 0x0000000, 0x0080000, CRC(e4497f35) SHA1(7030aba6c17fc391564385f5669e07edc94dca61) )
	ROM_LOAD32_WORD("PEACOCKFLUTTER_U8.bin", 0x0100000, 0x0080000, CRC(f239ca62) SHA1(53e3e2a4d62ceb9e921606e3670470c09e82118f) )
ROM_END_REGIONS




ROM_START( pengpay )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Penguin_Pays_U11.BIN", 0x0000002, 0x0080000, CRC(82fc4e23) SHA1(54e7698c4deed7202da8f178698ecdcf85f3f640) )
	ROM_LOAD32_WORD("Penguin_Pays_U12.BIN", 0x0100002, 0x0080000, CRC(90864742) SHA1(f6491e4fbce5d642b9d0224118923b56625338b1) )
	ROM_LOAD32_WORD("Penguin_Pays_U7.bin", 0x0000000, 0x0080000, CRC(47145744) SHA1(74a186a15537d8b05ce23f37c53f351e8058b0b2) )
	ROM_LOAD32_WORD("Penguin_Pays_U8.bin", 0x0100000, 0x0080000, CRC(8d37d7bf) SHA1(9c9b86cce9492f9de346e5a6944e2f0c5da6b9b1) )
ROM_END_REGIONS




ROM_START( przfigha )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("PRIZEFIGHT_U11.BIN", 0x0000002, 0x0080000, CRC(e1bf20d7) SHA1(bcc308b884433b3ebd890fafa667235a9fb7876c) )
	ROM_LOAD32_WORD("PRIZEFIGHT_U12.BIN", 0x0100002, 0x0080000, CRC(b4797555) SHA1(695aa6c40145fd9856821288680a24d316b7d4cd) )
	ROM_LOAD32_WORD("PRIZEFIGHT_U13.BIN", 0x0200002, 0x0080000, CRC(c16197d5) SHA1(716c4afdf2acde10ff09ad90b03bc5e689f0a737) )
	ROM_LOAD32_WORD("PRIZEFIGHT_U7.bin", 0x0000000, 0x0080000, CRC(2b1a9678) SHA1(c75de4c76cd934df746040d0515694d92e2fc145) )
	ROM_LOAD32_WORD("PRIZEFIGHT_U8.bin", 0x0100000, 0x0080000, CRC(92b68d43) SHA1(74ba55d6c7016de26692138d194f57f016feb938) )
	ROM_LOAD32_WORD("PRIZEFIGHT_U9.bin", 0x0200000, 0x0080000, CRC(b3163b0c) SHA1(e9aac4acb31a9af194626b25517aa7c169fe40bf) )
ROM_END_REGIONS





ROM_START( qncsh )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("queensofcash_U11.BIN", 0x0000002, 0x0080000, CRC(5001567e) SHA1(eadde9750856a7920e06955adc0db46082da655a) )
	ROM_LOAD32_WORD("queensofcash_U12.BIN", 0x0100002, 0x0080000, CRC(bfedb3fc) SHA1(e115db94b8ee7babb29e31e64b96d181f5c6491b) )
	ROM_LOAD32_WORD("queensofcash_U7.bin", 0x0000000, 0x0080000, CRC(591c96eb) SHA1(acd6f02206086d710a92401c618f715b3646d78a) )
	ROM_LOAD32_WORD("queensofcash_U8.bin", 0x0100000, 0x0080000, CRC(31ed5795) SHA1(8238da7c87195339d34cf24b3e0a7f3bf53d2b8a) )
ROM_END_REGIONS





ROM_START( sumospin )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Sumo Spins U11.BIN", 0x0000002, 0x0080000, CRC(919999fe) SHA1(3d800df5e0abed04c76928b04973ea7c7b02e5d1) )
	ROM_LOAD32_WORD("Sumo Spins U12.BIN", 0x0100002, 0x0080000, CRC(ba3eede2) SHA1(708a25af0908a1aa874b3ca4897816c65b0c9178) )
	ROM_LOAD32_WORD("Sumo Spins u7.bin", 0x0000000, 0x0080000, CRC(c3ec9f97) SHA1(62c886cc794de4b915533729c5ea5a71a4b59108) )
	ROM_LOAD32_WORD("Sumo Spins U8.bin", 0x0100000, 0x0080000, CRC(eb47f317) SHA1(43ead31e788cce1aa03011f634e939489d965144) )
ROM_END_REGIONS





ROM_START( sbucks3 )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("SUPERBUCKS3_U11.BIN", 0x0000002, 0x0080000, CRC(a810782c) SHA1(5d59b464c44ec32b2b977f8326c8bf3424a61e07) )
	ROM_LOAD32_WORD("SUPERBUCKS3_U12.BIN", 0x0100002, 0x0080000, CRC(a585172d) SHA1(3c74efb11285ff78ce76a7e8af2f936d3dc31290) )
	ROM_LOAD32_WORD("SUPERBUCKS3_U7.bin", 0x0000000, 0x0080000, CRC(e056c7db) SHA1(7a555583f750d8275b2ffd25a0efbe370a5ac43c) )
	ROM_LOAD32_WORD("SUPERBUCKS3_U8.bin", 0x0100000, 0x0080000, CRC(2ff83479) SHA1(2f0c6c12e115a5592c29e806a946817a4f1b89a3) )
ROM_END_REGIONS



ROM_START( tretrva )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("U11 Treasure Trove.bin", 0x0000002, 0x0080000, CRC(020a588d) SHA1(4759bef22017fb4c47c87adb6ca7253fdb6bca6b) )
	ROM_LOAD32_WORD("U12 Treasure Trove.bin", 0x0100002, 0x0080000, CRC(715f53cb) SHA1(364c35fc2d36180c13127c8004a8729126f68db1) )
	ROM_LOAD32_WORD("U7 Treasure Trove.bin", 0x0000000, 0x0080000, CRC(07a8b338) SHA1(7508d7d0e3494d355cb773165b240ba876a60eec) )
	ROM_LOAD32_WORD("U8 Treasure Trove.bin", 0x0100000, 0x0080000, CRC(89a042e7) SHA1(0f95cfd42ce7130176d42c6bbdf8ff22a6662894) )
ROM_END_REGIONS



ROM_START( triptrea )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("U11 Triple Treat.bin", 0x0000002, 0x0080000, CRC(fbc125b8) SHA1(55dbc3a236804f4a8d26be8e49c29fa5943c5bd6) )
	ROM_LOAD32_WORD("U12 Triple Treat.bin", 0x0100002, 0x0080000, CRC(5df3854a) SHA1(2b5175835c587caccafb73a1a5c8abf8f8463cf4) )
	ROM_LOAD32_WORD("U13 Triple Treat.bin", 0x0200002, 0x0080000, CRC(0a0b0ce1) SHA1(41a4d613cf1828df1832c087f0bc18d31076f056) )
	ROM_LOAD32_WORD("U7 Triple Treat.bin", 0x0000000, 0x0080000, CRC(7bc25bba) SHA1(d5f7c3a4bc3c652f57ee4cdbc883ec82069365d1) )
	ROM_LOAD32_WORD("U8 Triple Treat.bin", 0x0100000, 0x0080000, CRC(ef976f78) SHA1(d2c89e8d3bf6af112a99354133f308a5aabad46e) )
	ROM_LOAD32_WORD("U9 Triple Treat.bin", 0x0200000, 0x0080000, CRC(776fbfd2) SHA1(27820dbc6ee1424706aea9c4574da117636fef17) )
ROM_END_REGIONS



ROM_START( trjhrs )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("TROJANHORSE_U11.BIN", 0x0000002, 0x0080000, CRC(8c04ed89) SHA1(6727da3a457841e893e27bc8f10d4bb58a61f338) )
	ROM_LOAD32_WORD("TROJANHORSE_U12.BIN", 0x0100002, 0x0080000, CRC(1eb021a4) SHA1(3195eb5923da018b6c2dac10b70c47aef54dca35) )
	ROM_LOAD32_WORD("TROJANHORSE_U13.BIN", 0x0200002, 0x0080000, CRC(b6d1ceb6) SHA1(b41200620aaa905697ac73b4c86496a53f070ed3) )
	ROM_LOAD32_WORD("TROJANHORSE_U7.bin", 0x0000000, 0x0080000, CRC(7be0caf5) SHA1(b83fba7eb4624b3dc56f763b48b7c45fe31f3396) )
	ROM_LOAD32_WORD("TROJANHORSE_U8.bin", 0x0100000, 0x0080000, CRC(246d3693) SHA1(8c8b893c21e9a486fd36677d7157787bf5d6237b) )
	ROM_LOAD32_WORD("TROJANHORSE_U9.bin", 0x0200000, 0x0080000, CRC(15dee624) SHA1(d678ef7c25419342a1512fab84394e99309009ec) )
ROM_END_REGIONS








ROM_START( wildbill )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Wild Bill U11.BIN", 0x0000002, 0x0080000, CRC(57b3c340) SHA1(4f95ed7fed697cf2bfbde8215f6e35768cf20334) )
	ROM_LOAD32_WORD("Wild Bill U7.bin", 0x0000000, 0x0080000, CRC(e3117ab7) SHA1(c13912f524f1c1d373adb6382ceddd1bc18f7f02) )
ROM_END_REGIONS



ROM_START( wldcoug )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Wild Cougar  U11.BIN", 0x0000002, 0x0080000, CRC(6a5f2c41) SHA1(1365e083d44a373c2d4f17e8e61ec716ffb6d2d5) )
	ROM_LOAD32_WORD("Wild Cougar  U12.BIN", 0x0100002, 0x0080000, CRC(85bb41a7) SHA1(335f29f10f216e202b93b46a376958c3f5271461) )
	ROM_LOAD32_WORD("Wild Cougar  U7.bin", 0x0000000, 0x0080000, CRC(47154679) SHA1(21749fbaa60f9bf1db43bdd272e6628ae73bf759) )
	ROM_LOAD32_WORD("Wild Cougar  U8.bin", 0x0100000, 0x0080000, CRC(c262d098) SHA1(87940bd0aef6cb0f5ff21ccda4b209eef8e97eb1) )
ROM_END_REGIONS





ROM_START( chariotca )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("U11.BIN", 0x0000002, 0x0080000, CRC(bcbf9de9) SHA1(191ce749fe0d29b2783fb78d9338a00d65104daa) )
	ROM_LOAD32_WORD("U12.BIN", 0x0100002, 0x0080000, CRC(b44cf571) SHA1(04447820e015425493cade5611b3eb2f21e48c2e) )
	ROM_LOAD32_WORD("U7.bin", 0x0000000, 0x0080000, CRC(845f9913) SHA1(df6121290b30ff4a9c2d0e690cf8e7797e9a8612) )
	ROM_LOAD32_WORD("U8.bin", 0x0100000, 0x0080000, CRC(a3a74ecb) SHA1(52b3a41573a9fa1de05ce01a858e400f80e595b8) )
ROM_END_REGIONS



ROM_START( chkmatar )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("U11.BIN", 0x0000002, 0x0080000, CRC(5fb7bfb3) SHA1(2ad8b3c4753d19f9e3254ef3f4059951d7a111b4) )
	ROM_LOAD32_WORD("U12.BIN", 0x0100002, 0x0080000, CRC(b538bcbc) SHA1(cda404f9b16e7e76a33c208f62a5ac9c5e02aac4) )
	ROM_LOAD32_WORD("U13.BIN", 0x0200002, 0x0080000, CRC(ad12a718) SHA1(0c36729cb8da800668f533f65fcc870f5dfc0f6a) )
	ROM_LOAD32_WORD("U7.bin", 0x0000000, 0x0080000, CRC(059b940e) SHA1(f637508dafbd37169429c495a893addbc6d28834) )
	ROM_LOAD32_WORD("U8.bin", 0x0100000, 0x0080000, CRC(6912cc4a) SHA1(9469a6a0d2fd39d85655a8c7bc0668752f5f11fa) )
	ROM_LOAD32_WORD("U9.bin", 0x0200000, 0x0080000, CRC(53a573f0) SHA1(d51d698dcec273d157319200ad1c215e930b96ce) )
ROM_END_REGIONS




ROM_START( kgalah )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("U11.BIN", 0x0000002, 0x0080000, CRC(2b52a5e2) SHA1(0c852c6672a46f269f1407db0dd1825a51f242cc) )
	ROM_LOAD32_WORD("U12.BIN", 0x0100002, 0x0080000, CRC(15d5bfb4) SHA1(7c48dabfd83cc30fe2ffd0b4de63fbc9dc56ee2f) )
	ROM_LOAD32_WORD("U7.bin", 0x0000000, 0x0080000, CRC(9333543a) SHA1(dbbd59de046c35e70e71836b342eb5ecf4799575) )
	ROM_LOAD32_WORD("U8.bin", 0x0100000, 0x0080000, CRC(08bea3b7) SHA1(9a5d8cf60c9643061dede926a04006a9a674fd8f) )
ROM_END_REGIONS






ROM_START( qonilea )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("QUEENOFTHENILE_U11.BIN", 0x0000002, 0x0080000, CRC(ca4fe491) SHA1(2bd799f95c9a5afb7c96305bf56413ba864a26dd) )
	ROM_LOAD32_WORD("QUEENOFTHENILE_U12.BIN", 0x0100002, 0x0080000, CRC(bdcec4eb) SHA1(ef3658460263cd2e68e10015efdc016ad705213e) )
	ROM_LOAD32_WORD("QUEENOFTHENILE_U7.bin", 0x0000000, 0x0080000, CRC(f359afcf) SHA1(a8cbaea899f0108a179c58ec97241a57227afa79) )
	ROM_LOAD32_WORD("QUEENOFTHENILE_U8.bin", 0x0100000, 0x0080000, CRC(80efde3a) SHA1(1fac1b150c5c8c52a4caaa01c4571a0e7033278d) )
ROM_END_REGIONS







ROM_START( qonileb )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("U11.BIN", 0x0000002, 0x0080000, CRC(b5b76fb0) SHA1(40cb57e168f7884d64f6779e4e3b532c69df63b8) )
	ROM_LOAD32_WORD("U12.BIN", 0x0100002, 0x0080000, CRC(52bd3694) SHA1(bcfa3054c7577f7a1653b756828d048a5f1776e7) )
	ROM_LOAD32_WORD("U7.bin", 0x0000000, 0x0080000, CRC(0076da68) SHA1(ed301c102e88d5b637144ed32042da46780e5b34) )
	ROM_LOAD32_WORD("U8.bin", 0x0100000, 0x0080000, CRC(a6b856a2) SHA1(2a9ea01f64fa56dea86b0cd25e19dace34c17d0f) )
ROM_END_REGIONS



ROM_START( rwarhl )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("U10.bin", 0x0300000, 0x0080000, CRC(39f5861f) SHA1(c614ebe2c324d5c3fff32379300f2869fba49d39) )
	ROM_LOAD32_WORD("U11.BIN", 0x0000002, 0x0080000, CRC(4170c68d) SHA1(bc00af27bcc176f8d9c9fd0ec1a7139e28f85113) )
	ROM_LOAD32_WORD("U12.BIN", 0x0100002, 0x0080000, CRC(b8afd281) SHA1(2d73b5af667d36e8b29e9fc3cc62f220daeffbb9) )
	ROM_LOAD32_WORD("U13.BIN", 0x0200002, 0x0080000, CRC(36debb0e) SHA1(4aaa495f74dfb13aa1dc47f3a8af8e54496c1ab8) )
	ROM_LOAD32_WORD("U14.bin", 0x0300002, 0x0080000, CRC(92274626) SHA1(fae8d89efba9bf3d171bfe484015d009786ce40d) )
	ROM_LOAD32_WORD("U7.bin", 0x0000000, 0x0080000, CRC(68d9bf78) SHA1(6170ea26ebc732abbc26ba1da35a081c8aa8d154) )
	ROM_LOAD32_WORD("U8.bin", 0x0100000, 0x0080000, CRC(98ebea6f) SHA1(2d78cec777581a87bb4b84e7acd183b237c83e52) )
	ROM_LOAD32_WORD("U9.bin", 0x0200000, 0x0080000, CRC(eb7d7af6) SHA1(a11e8029b0d5ef9bb8c51fea4e9f0a051cdb2eaf) )
ROM_END_REGIONS




ROM_START( retrsam )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("U11.BIN", 0x0000002, 0x0080000, CRC(b91f5d4c) SHA1(8116166a759405b97797b4acb2cc3e139bd12de7) )
	ROM_LOAD32_WORD("U12.BIN", 0x0100002, 0x0080000, CRC(fdf22d5b) SHA1(664fa003a350c0a3b515b7c384d32176158c2d3e) )
	ROM_LOAD32_WORD("u7.bin", 0x0000000, 0x0080000, CRC(129be82c) SHA1(487639b7d42d6d35a9c48b44d26667c269b5b633) )
	ROM_LOAD32_WORD("U8.bin", 0x0100000, 0x0080000, CRC(8d0e61a8) SHA1(254b106e71a0888b0456afd8d63006d72c0ba292) )
ROM_END_REGIONS


ROM_START( unicornd )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("unicorndreaming_U11.BIN", 0x0000002, 0x0080000, CRC(b45885f1) SHA1(e32d4afce4e3e62a324173252f559909ea97fe3a) )
	ROM_LOAD32_WORD("unicorndreaming_U12.BIN", 0x0100002, 0x0080000, CRC(14afdeda) SHA1(1eb2a297e903dc1a0683425b37669e0af4ae4218) )
	ROM_LOAD32_WORD("unicorndreaming_u7.bin", 0x0000000, 0x0080000, CRC(d785d1b3) SHA1(4aa7c61036dd5fe1cdbc6c39a89881f88f3dd148) )
	ROM_LOAD32_WORD("unicorndreaming_U8.bin", 0x0100000, 0x0080000, CRC(6ba8f7eb) SHA1(bd5b15e22e713095f580b4c371d39af4af9e3289) )
ROM_END_REGIONS





/*************************
*      Game Drivers      *
*************************/

#define MACHINE_FLAGS MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND|MACHINE_IMPERFECT_GRAPHICS

//    YEAR  NAME       PARENT    MACHINE       INPUT     STATE           INIT      ROT     COMPANY       FULLNAME                                         FLAGS
GAME( 1995, aristmk5,  0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "MKV Set/Clear Chips (USA)",                      MACHINE_FLAGS|MACHINE_IS_BIOS_ROOT )

// Dates listed below are for the combination (reel layout), not release dates
GAME( 1995, enchfrst,  0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Enchanted Forest (0400122V, Local)",             MACHINE_FLAGS )  // 570/3,    E - 23/06/95
GAME( 1995, swthrt2v,  0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Sweet Hearts II (01J01986, Venezuela)",          MACHINE_FLAGS )  // 577/1,    C - 07/09/95
GAME( 1996, minemine,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Mine, Mine, Mine (Export)",                      MACHINE_FLAGS )  // 559/2,    E - 14/02/96
GAME( 1996, dolphntr,  0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Dolphin Treasure (0200424V, NSW/ACT)",           MACHINE_FLAGS )  // 602/1,    B - 06/12/96, Rev 3
GAME( 1996, dolphtra,  dolphntr, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Dolphin Treasure (0100424V, NSW/ACT)",           MACHINE_FLAGS )  // 602/1,    B - 06/12/96, Rev 1.24.4.0
GAME( 1996, dolphtre,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Dolphin Treasure (Export)",                      MACHINE_FLAGS )  // 602/1,    B - 06/12/96
GAME( 1996, cashcham,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Cash Chameleon (Export)",                        MACHINE_FLAGS )  // 603(a),   B - 06/12/96
GAME( 1997, enchfore,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Enchanted Forest (MV4033, Export, 94.97%)",      MACHINE_FLAGS )  // MV4033,   B - 10/02/97
GAME( 1997, mgarden,   aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Magic Garden (AHG1211, Export, 88.26%)",         MACHINE_FLAGS )  // MV4033,   B - 10/02/97
GAME( 1997, goldprmd,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Golden Pyramids (MV4091, USA)",                  MACHINE_FLAGS )  // MV4091,   B - 13/05/97
GAME( 1997, goldpyra,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Golden Pyramids (AHG1206-99, NSW/ACT, 87.928%)", MACHINE_FLAGS )  // 602/2,    B - 13/05/97
GAME( 1997, qotn,      0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Queen of the Nile (0200439V, NSW/ACT)",          MACHINE_FLAGS )  // 602/4,    B - 13/05/97
GAME( 1997, qotna,     aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Queen of the Nile (MV4091, NSW/ACT)",            MACHINE_FLAGS )  // MV4091,   B - 13/05/97 (US-Export HW?)
GAME( 1997, wldcougr,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Wild Cougar (Export)",                           MACHINE_FLAGS )  // 569/8,    D - 19/05/97
GAME( 1997, dmdtouch,  0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Diamond Touch (0400433V, Local)",                MACHINE_FLAGS )  // 604,      E - 30/06/97
GAME( 1997, bumblbug,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Bumble Bugs (Export, 92.691%)",                  MACHINE_FLAGS )  // 593,      D - 05/07/97
GAME( 1997, pengpays,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Penguin Pays (Export)",                          MACHINE_FLAGS )  // 586/7(b)  B - 14/07/97
GAME( 1997, trpdlght,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Tropical Delight (PHG0625-02, Export, 92.25%)",  MACHINE_FLAGS )  // 577/3,    D - 24/09/97
GAME( 1998, chickena,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Chicken (RHG0730, Export, 92.588%, touch)",      MACHINE_FLAGS )  // 596,      C - 23/02/98
GAME( 1998, adonis,    0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Adonis (0200751V, NSW/ACT)",                     MACHINE_FLAGS )  // 602/9,    A - 25/05/98, Rev 10
GAME( 1998, adonisa,   adonis,   aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Adonis (0100751V, NSW/ACT)",                     MACHINE_FLAGS )  // 602/9,    A - 25/05/98, Rev 9
GAME( 1998, swheart2,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Sweet Hearts II (PHG0742, Export, 92.252%)",     MACHINE_FLAGS )  // MV4061,   A - 29/06/98
GAME( 1998, reelrock,  0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Reelin-n-Rockin (0100779V, Local)",              MACHINE_FLAGS )  // 628,      A - 13/07/98
GAME( 1998, thgamblr,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "The Gambler (EHG0916, Export, 92.268%)",         MACHINE_FLAGS )  // MV4084/1, A - 30/10/98
GAME( 1998, indiandr,  0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Indian Dreaming (0100845V, Local)",              MACHINE_FLAGS )  // 628/1,    B - 15/12/98
GAME( 1998, chariotc,  0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "The Chariot Challenge (04J00714, NSW/ACT)",      MACHINE_FLAGS )  // 630,      A - 10/08/98, Rev 12
GAME( 1999, wtiger,    0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "White Tiger Classic (0200954V, NSW/ACT)",        MACHINE_FLAGS )  // 638/1,    B - 08/07/99
GAME( 1999, bootsctn,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Boot Scootin' (Export, 92.767%)",                MACHINE_FLAGS )  // MV4098,   A - 25/08/99
GAME( 2000, cuckoo,    aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Cuckoo (MV4104, Export)",                        MACHINE_FLAGS )  // MV4104,   C - 02/02/00
GAME( 2000, magicmsk,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Magic Mask (MV4115, Export, set 1)",             MACHINE_FLAGS )  // MV4115,   A - 09/05/00
GAME( 2000, magicmska, magicmsk, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Magic Mask (MV4115, Export, set 2)",             MACHINE_FLAGS )  // MV4115,   A - 09/05/00
GAME( 2000, margmgc,   0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Margarita Magic (01J00101, NSW/ACT)",            MACHINE_FLAGS )  // JB005,    A - 07/07/00
GAME( 2000, marmagic,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Margarita Magic (EHG1559, NSW/ACT)",             MACHINE_FLAGS )  // US003,    A - 07/07/00
GAME( 2001, prtygras,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Party Gras (MV4115/3, Export, touch)",           MACHINE_FLAGS )  // MV4115/3, B - 06/02/01
GAME( 2001, geishanz,  0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Geisha (0101408V, New Zealand)",                 MACHINE_FLAGS )  // MV4127,   A - 05/03/01
GAME( 2001, adonise,   aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Adonis (MV4124/1, Export)",                      MACHINE_FLAGS )  // MV4124/1, B - 31/07/01
GAME( 2001, koalamnt,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Koala Mint (MV4137, Export)",                    MACHINE_FLAGS )  // MV4137,   A - 12/09/01
GAME( 2001, partygrs,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0,  "Aristocrat", "Party Gras (MV4115/6, Export)",                  MACHINE_FLAGS )  // MV4115/6, A - 10/11/01

GAME( 199?, badbog,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Bad Bog Draw Poker",                 MACHINE_FLAGS )
GAME( 199?, blackpnt,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Black Panther (Aristocrat)",                 MACHINE_FLAGS )
GAME( 199?, bootscot,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Boot Scootin",                 MACHINE_FLAGS )
GAME( 199?, bumbugs,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Bumble Bugs",                 MACHINE_FLAGS )
GAME( 199?, butdeli,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Butterfly Delight",                 MACHINE_FLAGS )
GAME( 199?, cashchm,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Cash Chameleon",                 MACHINE_FLAGS )
GAME( 199?, cashcroa,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Cash Crop",                 MACHINE_FLAGS )
GAME( 199?, chkrun,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Chicken Run",                 MACHINE_FLAGS )
GAME( 199?, coralr2v, aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Coral Riches 2",                 MACHINE_FLAGS )
GAME( 199?, cuckooa,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Cuckoo",                 MACHINE_FLAGS )
GAME( 199?, dstblom,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Desert Bloom",                 MACHINE_FLAGS )
GAME( 199?, dmddove,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Diamond Dove",                 MACHINE_FLAGS )
GAME( 199?, drawpka,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Draw Poker",                 MACHINE_FLAGS )
GAME( 199?, dyjack,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Dynamite Jack",                 MACHINE_FLAGS )
GAME( 199?, eldora,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "El Dorado",                 MACHINE_FLAGS )
GAME( 199?, fortela,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Fortune Teller",                 MACHINE_FLAGS )
GAME( 199?, genmagi,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Genie Magic",                 MACHINE_FLAGS )
GAME( 199?, gnomatw,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Gnome Around The World",                 MACHINE_FLAGS )
GAME( 199?, goldra,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Golden Ra",                 MACHINE_FLAGS )
GAME( 199?, hrttrhb,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Heart Throb",                 MACHINE_FLAGS )
GAME( 199?, incsun,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Inca Sun",                 MACHINE_FLAGS )
GAME( 199?, kookabk,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Kooka Bucks",                 MACHINE_FLAGS )
GAME( 199?, locoloot,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Loco Loot",                 MACHINE_FLAGS )
GAME( 199?, lonwolf,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Lone Wolf",                 MACHINE_FLAGS )
GAME( 199?, mgctouc,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Magic Touch",                 MACHINE_FLAGS )
GAME( 199?, monmous,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Money Mouse",                 MACHINE_FLAGS )
GAME( 199?, moutmon,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Mountain Money",                 MACHINE_FLAGS )
GAME( 199?, mystgrd,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Mystic Garden",                 MACHINE_FLAGS )
GAME( 199?, orchidms,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Orchid Mist",                 MACHINE_FLAGS )
GAME( 199?, oscarar,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Oscar",                 MACHINE_FLAGS )
GAME( 199?, pantmag,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Panther Magic",                 MACHINE_FLAGS )
GAME( 199?, peaflut,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Peacock Flutter",                 MACHINE_FLAGS )
GAME( 199?, pengpay,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Penguin Pays",                 MACHINE_FLAGS )
GAME( 199?, przfigha,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Prize Fight",                 MACHINE_FLAGS )
GAME( 199?, qncsh,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Queens of Cash",                 MACHINE_FLAGS )
GAME( 199?, sumospin,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Sumo Spins",                 MACHINE_FLAGS )
GAME( 199?, sbucks3,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Superbucks 3",                 MACHINE_FLAGS )
GAME( 199?, tretrva,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Treasure Trove",                 MACHINE_FLAGS )
GAME( 199?, triptrea,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Triple Treat",                 MACHINE_FLAGS )
GAME( 199?, trjhrs,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Trojan Horse",                 MACHINE_FLAGS )
GAME( 199?, wildbill,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Wild Bill",                 MACHINE_FLAGS )
GAME( 199?, wldcoug,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Wild Cougar",                 MACHINE_FLAGS )
GAME( 199?, chariotca,  chariotc, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Chariot Challenge (0100787V)",                 MACHINE_FLAGS )
GAME( 199?, chkmatar,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Checkmate (01J00681)",                 MACHINE_FLAGS )
GAME( 199?, kgalah,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "King Galah (0200536V)",                 MACHINE_FLAGS )
GAME( 199?, qonilea,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Queen Of The Nile (0100439V)",                 MACHINE_FLAGS )
GAME( 199?, qonileb,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Queen Of The Nile (0300440V)",                 MACHINE_FLAGS )
GAME( 199?,rwarhl,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Rainbow Warrior Hyperlink (0101332V)",                 MACHINE_FLAGS )
GAME( 199?, retrsam,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Return Of The Samurai (0400549V)",                 MACHINE_FLAGS )
GAME( 199?, unicornd,  aristmk5, aristmk5_usa, aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Unicorn Dreaming",                 MACHINE_FLAGS )

