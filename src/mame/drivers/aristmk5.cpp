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
    - Layouts for various configurations
    - Bill acceptor
    - Serial printer
    - Default NVRAM

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

    dimtouch:
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


#include "emu.h"
#include "includes/archimds.h"

#include "cpu/arm/arm.h"
#include "machine/ds1302.h"
#include "machine/watchdog.h"
#include "machine/eepromser.h"
#include "machine/microtch.h"
#include "machine/input_merger.h"
#include "machine/nvram.h"
#include "machine/ins8250.h"
#include "machine/ticket.h"
#include "sound/volt_reg.h"
#include "speaker.h"

#include "aristmk5.lh"
#include "aristmk5_us.lh"

#define MASTER_CLOCK        XTAL_72MHz      /* confirmed */

class aristmk5_state : public archimedes_state
{
public:
	aristmk5_state(const machine_config &mconfig, device_type type, const char *tag)
		: archimedes_state(mconfig, type, tag)
		, m_eeprom(*this, "eeprom%d", 0)
		, m_rtc(*this, "rtc")
		, m_nvram(*this, "nvram")
		, m_hopper(*this, "hopper")
		, m_sram(*this, "sram")
		, m_p1(*this, "P1")
		, m_p2(*this, "P2")
		, m_extra_ports(*this, "EXTRA")
		 { }

	DECLARE_WRITE32_MEMBER(Ns5w48);
	DECLARE_READ32_MEMBER(Ns5x58);
	DECLARE_READ32_MEMBER(Ns5r50);
	DECLARE_WRITE8_MEMBER(sram_banksel_w);
	DECLARE_WRITE8_MEMBER(eeprom_w);
	DECLARE_WRITE8_MEMBER(eeprom_usa_w);
	DECLARE_WRITE8_MEMBER(rtc_w);
	DECLARE_WRITE8_MEMBER(rtc_usa_w);
	DECLARE_WRITE8_MEMBER(hopper_w);
	DECLARE_READ8_MEMBER(eeprom_r);
	DECLARE_READ8_MEMBER(ldor_r);
	DECLARE_WRITE8_MEMBER(ldor_clk_w);
	DECLARE_WRITE8_MEMBER(buttons_lamps_w);
	DECLARE_WRITE8_MEMBER(other_lamps_w);
	DECLARE_WRITE8_MEMBER(bill_acceptor_lamps_w);
	DECLARE_READ8_MEMBER(sram_r);
	DECLARE_WRITE8_MEMBER(sram_w);
	DECLARE_WRITE8_MEMBER(spi_mux_w);
	DECLARE_WRITE8_MEMBER(spi_data_w);
	DECLARE_READ8_MEMBER(spi_int_ack_r);
	DECLARE_WRITE8_MEMBER(spi_int_ack_w);
	DECLARE_READ8_MEMBER(spi_data_r);
	DECLARE_WRITE_LINE_MEMBER(uart_irq_callback);

	DECLARE_DRIVER_INIT(aristmk5);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	TIMER_CALLBACK_MEMBER(mk5_VSYNC_callback);
	TIMER_CALLBACK_MEMBER(mk5_2KHz_callback);
	TIMER_CALLBACK_MEMBER(spi_timer);

	INPUT_CHANGED_MEMBER(coin_start);
	CUSTOM_INPUT_MEMBER(coin_r);
	CUSTOM_INPUT_MEMBER(coin_usa_r);
	CUSTOM_INPUT_MEMBER(hopper_r);

private:
	required_device_array<eeprom_serial_93cxx_device, 2> m_eeprom;
	required_device<ds1302_device> m_rtc;
	required_device<nvram_device> m_nvram;
	required_device<ticket_dispenser_device> m_hopper;
	required_memory_region m_sram;
	required_ioport m_p1;
	required_ioport m_p2;
	required_ioport m_extra_ports;

	emu_timer *     m_mk5_2KHz_timer;
	emu_timer *     m_mk5_VSYNC_timer;
	emu_timer *     m_spi_timer;
	uint8_t         m_sram_bank;
	uint8_t         m_ldor_shift_reg;
	uint8_t         m_hopper_test;
	uint64_t        m_coin_start_cycles;
	uint8_t         m_coin_div;
	uint8_t         m_spi_mux;
	uint8_t         m_spi_latch;
	uint8_t         m_spi_bits;
	uint32_t        m_spi_data[8];
};


WRITE8_MEMBER(aristmk5_state::spi_mux_w)
{
	uint8_t spi_mux = (data >> 4) & 7;

	if (spi_mux == m_spi_mux)
		return;

	m_spi_mux = spi_mux;

	switch (m_spi_mux)
	{
	case 0: // Test
	case 3: // not used
		break;

	case 1: // Top box lamps
		break;

	case 2: // Mechanical meters
		for(int i=0; i<4; i++)
			output().set_lamp_value(32 + i, BIT(m_spi_data[m_spi_mux], 1 + i));     // Tower Lamps
		break;

	case 4: // Door inputs
		m_spi_data[m_spi_mux] = m_p1->read();
		break;

	case 5: // Door outputs
		for(int i=0; i<32; i++)
			output().set_lamp_value(i, BIT(m_spi_data[m_spi_mux], i));
		break;

	case 6: // Main board slow I/O
		m_spi_data[m_spi_mux] = m_p2->read() & ~((data  & 0x80) ? 0 : 0x100);
		break;

	case 7: // Main board security registers
		break;
	}
}

WRITE8_MEMBER(aristmk5_state::spi_data_w)
{
	m_spi_latch = data;
	m_spi_bits = 0;

	// start the SPI clock
	m_spi_timer->adjust(attotime::from_hz((double)MASTER_CLOCK / 9 / 512 / 2), 0, attotime::from_hz((double)MASTER_CLOCK / 9 / 512 / 2));
}

READ8_MEMBER(aristmk5_state::spi_data_r)
{
	return m_spi_latch;
}

READ8_MEMBER(aristmk5_state::spi_int_ack_r)
{
	archimedes_clear_irq_b(0x08);
	return 0;
}

WRITE8_MEMBER(aristmk5_state::spi_int_ack_w)
{
	archimedes_clear_irq_b(0x08);
}

TIMER_CALLBACK_MEMBER(aristmk5_state::spi_timer)
{
	if (m_spi_mux == 0 || m_spi_mux == 3)
	{
		m_spi_latch = (((m_spi_latch & 1) << 7) ^ 0x80) | ((m_spi_latch >> 1) & 0x7f);
	}
	else
	{
		static int mux_bits[8] = { 0, 16, 16, 0, 24, 32, 24, 8 };

		uint32_t mux_mask = ((uint32_t)1 << (mux_bits[m_spi_mux] - 1)) - 1;
		uint32_t spi_in_bit = m_spi_data[m_spi_mux] & 1;
		uint32_t spi_out_bit = m_spi_latch & 1;

		m_spi_data[m_spi_mux] = (spi_out_bit << (mux_bits[m_spi_mux] - 1)) | ((m_spi_data[m_spi_mux] >> 1) & mux_mask);
		m_spi_latch = (spi_in_bit << 7) | ((m_spi_latch >> 1) & 0x7f);
	}

	// SPI interrupt
	if (++m_spi_bits == 8)
	{
		m_spi_timer->adjust(attotime::never);
		archimedes_request_irq_b(0x08);
	}
}

WRITE_LINE_MEMBER(aristmk5_state::uart_irq_callback)
{
	if (state)
		archimedes_request_irq_b(0x20);
	else
		archimedes_clear_irq_b(0x20);
}

TIMER_CALLBACK_MEMBER(aristmk5_state::mk5_VSYNC_callback)
{
	archimedes_request_irq_a(0x08); //turn vsync bit on
	m_mk5_VSYNC_timer->adjust(attotime::never);
}

READ8_MEMBER(aristmk5_state::sram_r)
{
	return m_sram->base()[(m_sram_bank << 14) | (offset & 0x3fff)];
}

WRITE8_MEMBER(aristmk5_state::sram_w)
{
	m_sram->base()[(m_sram_bank << 14) | (offset & 0x3fff)] = data;
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


	archimedes_clear_irq_a(0x08);

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
	archimedes_request_irq_a(0x01);
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
	m_mk5_2KHz_timer->adjust(attotime::from_hz((double)MASTER_CLOCK / 9 / 4096));
	archimedes_clear_irq_a(0x01);
	return 0xffffffff;
}

READ32_MEMBER(aristmk5_state::Ns5r50)
{
	return 0xf5; // checked inside the CPU check, unknown meaning
}

READ8_MEMBER(aristmk5_state::eeprom_r)
{
	uint8_t data = 0x00;
	if (m_eeprom[0]->do_read() && m_eeprom[1]->do_read())
		data |= 0x04;

	if (m_rtc->io_r())
		data |= 0x02;

	return data;
}

WRITE8_MEMBER(aristmk5_state::hopper_w)
{
	m_hopper->write(space, 0, (data & 0x02) ? 0x80 : 0);
	m_hopper_test = BIT(data, 2);
}

WRITE8_MEMBER(aristmk5_state::rtc_w)
{
	m_rtc->ce_w(BIT(data, 5));

	if (BIT(data, 6))
		m_rtc->io_w(BIT(data, 3));

	m_rtc->sclk_w(BIT(data, 4));
}

WRITE8_MEMBER(aristmk5_state::rtc_usa_w)
{
	rtc_w(space, offset, data, mem_mask);
	m_hopper_test = BIT(data, 2);
}

WRITE8_MEMBER(aristmk5_state::eeprom_w)
{
	m_coin_div = data & 1;

	m_eeprom[0]->cs_write(BIT(data, 5));
	m_eeprom[1]->cs_write(BIT(data, 6));
	m_eeprom[0]->di_write(BIT(data, 3));
	m_eeprom[1]->di_write(BIT(data, 3));
	m_eeprom[0]->clk_write(BIT(data, 4));
	m_eeprom[1]->clk_write(BIT(data, 4));
}

WRITE8_MEMBER(aristmk5_state::eeprom_usa_w)
{
	eeprom_w(space, offset, data, mem_mask);
	m_hopper->write(space, 0, (data & 0x04) ? 0x80 : 0);
}

READ8_MEMBER(aristmk5_state::ldor_r)
{
	if (m_extra_ports->read() & 0x01)
		m_ldor_shift_reg = 0;   // open the Logic door clears the shift register

	return (m_ldor_shift_reg & 0x80) | 0x60 | ((m_hopper_test && m_hopper->line_r()) ? 0x10 : 0x00);
}

WRITE8_MEMBER(aristmk5_state::ldor_clk_w)
{
	m_ldor_shift_reg = (m_ldor_shift_reg << 1) | BIT(data, 0);
}

WRITE8_MEMBER(aristmk5_state::sram_banksel_w)
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

	m_sram_bank = ((data & 0xc0) >> 3) | (data & 0x07);
}

WRITE8_MEMBER(aristmk5_state::buttons_lamps_w)
{
	for(int i=0; i<8; i++)
		output().set_lamp_value((offset >> 2) * 8 + i, BIT(data, i));
}

WRITE8_MEMBER(aristmk5_state::other_lamps_w)
{
	for(int i=0; i<8; i++)
		output().set_lamp_value(16 + i, BIT(data, i));
}

WRITE8_MEMBER(aristmk5_state::bill_acceptor_lamps_w)
{
	for(int i=0; i<8; i++)
		output().set_lamp_value(24 + i, BIT(data, i));
}

static ADDRESS_MAP_START( aristmk5_map, AS_PROGRAM, 32, aristmk5_state )
	AM_RANGE(0x02000000, 0x02ffffff) AM_RAM AM_SHARE("physicalram") /* physical RAM - 16 MB for now, should be 512k for the A310 */

	/* MK-5 overrides */
	AM_RANGE(0x03010420, 0x03010423) AM_WRITE8(sram_banksel_w, 0x000000ff) // SRAM bank select write

	AM_RANGE(0x03010480, 0x0301049f) AM_DEVREADWRITE8("uart_0a", ins8250_uart_device, ins8250_r, ins8250_w, 0x000000ff)
	AM_RANGE(0x03010500, 0x0301051f) AM_DEVREADWRITE8("uart_0b", ins8250_uart_device, ins8250_r, ins8250_w, 0x000000ff)
	AM_RANGE(0x03010580, 0x03010583) AM_READ_PORT("P3")
	AM_RANGE(0x03010600, 0x0301061f) AM_DEVREADWRITE8("uart_1a", ins8250_uart_device, ins8250_r, ins8250_w, 0x000000ff)
	AM_RANGE(0x03010680, 0x0301069f) AM_DEVREADWRITE8("uart_1b", ins8250_uart_device, ins8250_r, ins8250_w, 0x000000ff)

	AM_RANGE(0x03010700, 0x03010703) AM_READ_PORT("P6")
	AM_RANGE(0x03010800, 0x03010803) AM_READ8(eeprom_r, 0x000000ff)
	AM_RANGE(0x03010810, 0x03010813) AM_DEVREADWRITE("watchdog", watchdog_timer_device, reset32_r, reset32_w) //MK-5 specific, watchdog
	AM_RANGE(0x03220000, 0x0323ffff) AM_READWRITE8(sram_r, sram_w, 0x000000ff)

	// bank5 slow
	AM_RANGE(0x03250048, 0x0325004b) AM_WRITE(Ns5w48) //IOEB control register
	AM_RANGE(0x03250050, 0x03250053) AM_READ(Ns5r50)  //IOEB ID register
	AM_RANGE(0x03250058, 0x0325005b) AM_READ(Ns5x58)  //IOEB interrupt Latch

	AM_RANGE(0x03000000, 0x0331ffff) AM_READWRITE(archimedes_ioc_r, archimedes_ioc_w)
	AM_RANGE(0x03320000, 0x0333ffff) AM_READWRITE8(sram_r, sram_w, 0x000000ff)

	AM_RANGE(0x03400000, 0x035fffff) AM_WRITE(archimedes_vidc_w)
	AM_RANGE(0x03600000, 0x037fffff) AM_WRITE(archimedes_memc_w)
	AM_RANGE(0x03800000, 0x039fffff) AM_WRITE(archimedes_memc_page_w)

	AM_RANGE(0x03400000, 0x03bfffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

/* U.S games have no dram emulator enabled */
static ADDRESS_MAP_START( aristmk5_usa_map, AS_PROGRAM, 32, aristmk5_state )
	AM_RANGE(0x00000000, 0x01ffffff) AM_READWRITE(archimedes_memc_logical_r, archimedes_memc_logical_w)

	AM_RANGE(0x03010440, 0x03010443) AM_WRITE8(rtc_usa_w, 0x000000ff)
	AM_RANGE(0x03010450, 0x03010453) AM_WRITE8(eeprom_usa_w, 0x000000ff)

	AM_RANGE(0x03012000, 0x03012003) AM_READ_PORT("P1")
	AM_RANGE(0x03012010, 0x03012013) AM_READ_PORT("P2")
	AM_RANGE(0x03012200, 0x03012203) AM_READ_PORT("DSW1")
	AM_RANGE(0x03012210, 0x03012213) AM_READ_PORT("DSW2")
	AM_RANGE(0x03010584, 0x03010587) AM_READ_PORT("P4")

	AM_RANGE(0x03012020, 0x03012023) AM_READ8(ldor_r, 0x000000ff)
	AM_RANGE(0x03012070, 0x03012073) AM_WRITE8(ldor_clk_w, 0x000000ff)
	AM_RANGE(0x03012184, 0x03012187) AM_READ_PORT("P5")

	AM_RANGE(0x03012000, 0x0301201f) AM_WRITE8(buttons_lamps_w, 0x000000ff)
	AM_RANGE(0x03012030, 0x0301203f) AM_WRITE8(other_lamps_w, 0x000000ff)
	AM_RANGE(0x03012380, 0x0301238f) AM_WRITE8(bill_acceptor_lamps_w, 0x000000ff)

	AM_RANGE(0x03012100, 0x0301211f) AM_DEVREADWRITE8("uart_2a", ins8250_uart_device, ins8250_r, ins8250_w, 0x000000ff)
	AM_RANGE(0x03012140, 0x0301215f) AM_DEVREADWRITE8("uart_2b", ins8250_uart_device, ins8250_r, ins8250_w, 0x000000ff)
	AM_RANGE(0x03012300, 0x0301231f) AM_DEVREADWRITE8("uart_3a", ins8250_uart_device, ins8250_r, ins8250_w, 0x000000ff)
	AM_RANGE(0x03012340, 0x0301235f) AM_DEVREADWRITE8("uart_3b", ins8250_uart_device, ins8250_r, ins8250_w, 0x000000ff)

	AM_IMPORT_FROM(aristmk5_map)
ADDRESS_MAP_END

/* with dram emulator enabled */
static ADDRESS_MAP_START( aristmk5_drame_map, AS_PROGRAM, 32, aristmk5_state )
	AM_RANGE(0x00000000, 0x01ffffff) AM_READWRITE(aristmk5_drame_memc_logical_r, archimedes_memc_logical_w)

	AM_RANGE(0x03010430, 0x03010433) AM_WRITE8(hopper_w, 0x000000ff)
	AM_RANGE(0x03010440, 0x03010443) AM_WRITE8(rtc_w, 0x000000ff)
	AM_RANGE(0x03010450, 0x03010453) AM_WRITE8(eeprom_w, 0x000000ff)

	AM_RANGE(0x03010400, 0x03010403) AM_WRITE8(spi_mux_w, 0x000000ff)
	AM_RANGE(0x03010470, 0x03010473) AM_WRITE8(spi_data_w, 0x000000ff)
	AM_RANGE(0x03010850, 0x03010853) AM_READWRITE8(spi_int_ack_r, spi_int_ack_w, 0x000000ff)
	AM_RANGE(0x03010870, 0x03010873) AM_READ8(spi_data_r, 0x000000ff)

	AM_RANGE(0x03014000, 0x0301401f) AM_DEVREADWRITE8("uart_2a", ins8250_uart_device, ins8250_r, ins8250_w, 0x000000ff)
	AM_RANGE(0x03014020, 0x0301403f) AM_DEVREADWRITE8("uart_2b", ins8250_uart_device, ins8250_r, ins8250_w, 0x000000ff)

	AM_IMPORT_FROM(aristmk5_map)
ADDRESS_MAP_END


CUSTOM_INPUT_MEMBER(aristmk5_state::hopper_r)
{
	return (m_hopper_test && m_hopper->line_r()) ? 0 : 1;
}

CUSTOM_INPUT_MEMBER(aristmk5_state::coin_usa_r)
{
	//  ---x  Coin Acceptor
	//  --x-  Credit Sense
	//  -x--  Error Signal
	//  x---  Diverter Optic

	uint8_t data = 0x07;

	if (!m_coin_div)
		data |= 0x08;

	if (m_coin_start_cycles)
	{
		attotime diff = m_maincpu->cycles_to_attotime(m_maincpu->total_cycles() - m_coin_start_cycles);

		if (diff > attotime::from_msec(5) && diff < attotime::from_msec(10))
			data &= ~0x01;
		if (diff > attotime::from_msec(15) && diff < attotime::from_msec(20))
			data &= ~0x02;
		if (diff <= attotime::from_msec(3))
			data |= 0x08;

		if (diff > attotime::from_msec(30))
			m_coin_start_cycles = 0;
	}

	return data;
}

CUSTOM_INPUT_MEMBER(aristmk5_state::coin_r)
{
	uint8_t data = 0x01;

	if (m_coin_start_cycles)
	{
		attotime diff = m_maincpu->cycles_to_attotime(m_maincpu->total_cycles() - m_coin_start_cycles);

		if (diff > attotime::from_msec(10) && diff < attotime::from_msec(15))
			data &= ~0x01;
		if (diff > attotime::from_msec(0) && diff < attotime::from_msec(20))
			data |= 0x10;
		if (diff > attotime::from_msec(15) && diff < attotime::from_msec(30))
			data |= 0x08;
		if (diff > attotime::from_msec(25) && !m_coin_div)
			data |= 0x02;

		if (diff > attotime::from_msec(30))
			m_coin_start_cycles = 0;
	}

	return data;
}

INPUT_CHANGED_MEMBER(aristmk5_state::coin_start)
{
	if (newval && !m_coin_start_cycles)
		m_coin_start_cycles = m_maincpu->total_cycles();
}

static INPUT_PORTS_START( aristmk5_usa )
	/* This simulates the ROM swap */
	PORT_START("ROM_LOAD")
	PORT_CONFNAME( 0x03, 0x03, "System Mode" )
	PORT_CONFSETTING(    0x00, "USA Set Chip v4.04.09 Mode" )
	PORT_CONFSETTING(    0x01, "USA Set Chip v4.04.00 Mode" )
	PORT_CONFSETTING(    0x02, "USA Set Chip v4.02.04 Mode" )
	PORT_CONFSETTING(    0x03, "Game Mode" )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW2:8")

	PORT_START("P1")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_CODE(KEYCODE_J) PORT_NAME("Gamble")
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_CODE(KEYCODE_K) PORT_NAME("Take Win")
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_CODE(KEYCODE_H) PORT_NAME("Bet 10 Credits / Black")
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_CODE(KEYCODE_G) PORT_NAME("Bet 5 Credits")
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_CODE(KEYCODE_F) PORT_NAME("Bet 3 Credits")
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_CODE(KEYCODE_D) PORT_NAME("Bet 2 Credits")
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_CODE(KEYCODE_S) PORT_NAME("Bet 1 Credit / Red")
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_CODE(KEYCODE_A) PORT_NAME("Service")

	PORT_START("P2")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_CODE(KEYCODE_Q) PORT_NAME("Cashout")
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_CODE(KEYCODE_W) PORT_NAME("1 Line")
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_CODE(KEYCODE_E) PORT_NAME("3 Lines")
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_CODE(KEYCODE_R) PORT_NAME("5 Lines")
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_CODE(KEYCODE_T) PORT_NAME("7 Lines")
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_CODE(KEYCODE_Y) PORT_NAME("9 Lines")
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_CODE(KEYCODE_U) // unused
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_CODE(KEYCODE_I) // unused

	PORT_START("P3")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_CODE(KEYCODE_X)
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_CODE(KEYCODE_N)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_SERVICE)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_CODE(KEYCODE_V) PORT_NAME("Reset Key")
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_CODE(KEYCODE_B) PORT_TOGGLE PORT_NAME("Bill acceptor door")
	PORT_BIT(0x00000040, IP_ACTIVE_LOW , IPT_KEYPAD)  PORT_CODE(KEYCODE_M) PORT_TOGGLE PORT_NAME("Main door")
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_CODE(KEYCODE_C) PORT_TOGGLE PORT_NAME("Cashbox door")

	PORT_START("P4")
	PORT_BIT(0x00000078, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, aristmk5_state, coin_usa_r, nullptr)

	PORT_START("P5")
	PORT_BIT(0x00000008, IP_ACTIVE_LOW,  IPT_OTHER)   // Meters

	PORT_START("P6")
	PORT_BIT(0x00000002, IP_ACTIVE_LOW, IPT_OTHER)    // Battery

	PORT_START("EXTRA")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_OTHER)   PORT_TOGGLE PORT_CODE(KEYCODE_L)   PORT_NAME("Logic door")
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_COIN1)   PORT_CHANGED_MEMBER(DEVICE_SELF, aristmk5_state, coin_start, nullptr)
INPUT_PORTS_END

static INPUT_PORTS_START( aristmk5 )
	/* This simulates the ROM swap */
	PORT_START("ROM_LOAD")
	PORT_CONFNAME( 0x03, 0x03, "System Mode" )
	PORT_CONFSETTING(    0x00, "USA Set Chip v4.04.09 Mode" )
	PORT_CONFSETTING(    0x01, "USA Set Chip v4.04.00 Mode" )
	PORT_CONFSETTING(    0x02, "USA Set Chip v4.02.04 Mode" )
	PORT_CONFSETTING(    0x03, "Game Mode" )

	PORT_START("P1")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_CODE(KEYCODE_J) PORT_NAME("Gamble")
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_CODE(KEYCODE_K) PORT_NAME("Take Win")
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_CODE(KEYCODE_H) PORT_NAME("Play 20 Lines / Black")
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_CODE(KEYCODE_G) PORT_NAME("Play 15 Lines")
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_CODE(KEYCODE_F) PORT_NAME("Play 10 Lines")
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_CODE(KEYCODE_D) PORT_NAME("Play 5 Lines")
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_CODE(KEYCODE_S) PORT_NAME("Play 1 Line / Red")
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_CODE(KEYCODE_A) PORT_NAME("Reserve")
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_CODE(KEYCODE_Q) PORT_NAME("Collect")
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_CODE(KEYCODE_W) PORT_NAME("Bet 1 Credit / Heart")
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_CODE(KEYCODE_E) PORT_NAME("Bet 2 Credits / Diamond")
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_CODE(KEYCODE_R) PORT_NAME("Bet 3 Credits")
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_CODE(KEYCODE_T) PORT_NAME("Bet 5 Credits / Club")
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_CODE(KEYCODE_Y) PORT_NAME("Bet 10 Credits / Spade")
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_CODE(KEYCODE_U) // unused
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_CODE(KEYCODE_I) // unused
	PORT_BIT(0x00ff0000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("P2")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_CODE(KEYCODE_V) PORT_NAME("Reset Key")
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_SERVICE)
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_OTHER)     // Hopper full
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_OTHER)     // Hopper empty
	PORT_BIT(0x00000100, IP_ACTIVE_LOW,  IPT_KEYPAD)    PORT_CODE(KEYCODE_M) PORT_TOGGLE PORT_NAME("Main door optical sensor")
	PORT_BIT(0x0000fe00, IP_ACTIVE_HIGH, IPT_UNUSED)    // Unused optical security sensors
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_CODE(KEYCODE_L) PORT_TOGGLE PORT_NAME("Logic door")
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_CODE(KEYCODE_Z) PORT_TOGGLE PORT_NAME("Topbox door")
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_CODE(KEYCODE_X) PORT_TOGGLE PORT_NAME("Meter cage")
	PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_CODE(KEYCODE_C) PORT_TOGGLE PORT_NAME("Cashbox door")
	PORT_BIT(0x00100000, IP_ACTIVE_LOW,  IPT_KEYPAD)    PORT_CODE(KEYCODE_M) PORT_TOGGLE PORT_NAME("Main door")
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_CODE(KEYCODE_B) PORT_TOGGLE PORT_NAME("Bill acceptor door")
	PORT_BIT(0x00c00000, IP_ACTIVE_HIGH, IPT_UNUSED)    // Unused mechanical security switch

	PORT_START("P3")
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_CUSTOM_MEMBER(DEVICE_SELF, aristmk5_state, hopper_r, nullptr)
	PORT_BIT(0x000000f8, IP_ACTIVE_HIGH, IPT_SPECIAL)   PORT_CUSTOM_MEMBER(DEVICE_SELF, aristmk5_state, coin_r, nullptr)

	PORT_START("P6")
	PORT_BIT(0x00000002, IP_ACTIVE_LOW, IPT_OTHER)          // Battery

	PORT_START("EXTRA")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_COIN1)         PORT_CHANGED_MEMBER(DEVICE_SELF, aristmk5_state, coin_start, nullptr)
INPUT_PORTS_END

DRIVER_INIT_MEMBER(aristmk5_state,aristmk5)
{
	archimedes_driver_init();

	int do_debug = 0;

	if (do_debug)
	{
		// DEBUG code for showing the range of the ROMs that get checksummed (for adding to rom loading comments)
		// unfortunately the checksum only covers the code part of the ROM, leaving the data without any kind of
		// verification.  Given that the existing bad-dumps would be non-obvious if it the checksums weren't incorrect
		// this is potentially worrying.  The actual checksum scheme is weak too, a simple 32-bit add.

		uint32_t *ROM = (uint32_t*)memregion("game_prg")->base();
		int size = memregion("game_prg")->bytes();
		int found = 0;

		for (int i = 0;i < (size / 4) - 4;i++)
		{
			if (((ROM[i + 0] & 0xfffff000) == 0xe59f1000) &&
				(ROM[i + 1] == 0xe3a03000) &&
				((ROM[i + 2] & 0xfffff000) == 0xe59f4000) &&
				(ROM[i + 3] == 0xe0444001))
			{

				printf("Checksum code found at 0x%06x\n", i * 4);
				found = 1;

				int baseoff = ROM[i + 0] & 0x00000fff;
				int baseoff2 = ROM[i + 2] & 0x00000fff;
				int baseoff3 = ROM[i + 5] & 0x00000fff;
				int baseoff4 = ROM[i + 8] & 0x00000fff;

				//printf("values offset %08x %08x %08x %08x\n", baseoff, baseoff2, baseoff3, baseoff4);

				int actual = ROM[i + (baseoff / 4) + 2]; // base
				int actual2 = ROM[i + 2 + (baseoff2 / 4) + 2]; // end
				int actual3 = ROM[i + 5 + (baseoff3 / 4) + 2]; // skip end (where checksum is)
				int actual4 = ROM[i + 8 + (baseoff4 / 4) + 2]; // skip start (^)

				//printf("values %08x %08x %08x %08x\n", actual, actual2, actual3, actual4);

				actual2 = actual2 - actual;
				actual3 = actual3 - actual;
				actual4 = actual4 - actual;
				actual = 0;

				if ((actual4 - actual3) != 4)
				{
					printf("UNUSUAL SKIP RANGE %06x - %06x\n", actual3, actual4 + 3);
				}
				else
				{
					int expectedchecksum = ROM[actual3 / 4];
					printf("0x%06x-0x%06x is the Checksummed Range (excluding 0x%06x-0x%06x where Checksum is stored)\n    Expected Checksum   0x%08x\n", actual, actual2 - 1, actual3, actual4 + 3, expectedchecksum);

					// the checksum is a simple 32-bit sum with the dword containing the checksum skipped (and the dword after it for no obvious reason!)
					uint32_t calculatedchecksum = 0;
					for (int i = actual / 4; i < actual2 / 4;i++)
					{
						if ((i < (actual3 / 4)) ||
							(i > (actual4 / 4)))
						{
							calculatedchecksum += ROM[i];

							//  printf("Using address %08x, value %08x, Calculated Checksum %08x\n", i*4,  ROM[i], calculatedchecksum);
						}
						else
						{
							//  printf("SKIPPING address %08x, value %08x, Calculated Checksum %08x\n", i*4,  ROM[i], calculatedchecksum);
						}
					}

					printf("    Calculated Checksum 0x%08x ", calculatedchecksum);
					if (calculatedchecksum == expectedchecksum)
					{
						printf(" (OK)\n");
					}
					else
					{
						printf(" (BAD)\n");
					}

					size = size / 4;

					// almost always just the end of the roms
					int realend = 0;
					for (int i = size - 1; i >= actual2 / 4;i--)
					{
						if ((ROM[i] != 0xffffffff) && (ROM[i] != 0x00000000))
						{
							//printf("real data end at %08x\n", i * 4);
							realend = i;
							i = actual2 / 4 - 1;
						}
					}


					int realend2 = 0;
					for (int i = realend - 4; i >= actual2 / 4;i--)
					{
						if ((ROM[i] != 0xffffffff) && (ROM[i] != 0x00000000))
						{
							//printf("real data end at %08x (value is %08x)\n", i * 4, ROM[i]);
							realend2 = (i * 4) + 3;
							i = actual2 / 4 - 1;
						}
					}

					realend = (realend * 4) + 3;

					if ((realend & 0xffff) == 0xffff)
					{
						printf("0x%06x-0x%06x is the non-Checksummed range still containing data but NOT covered by Checksum\n", actual2, realend2);
						printf("0x%06x-0x%06x is the non-Checksummed range if the additional vectors? at the end are included\n", actual2, realend);
					}
					else
					{
						printf("0x%06x-0x%06x is the non-Checksummed range (unusual endpoint)\n", actual2, realend);
					}
				}
			}
		}

		if (found == 0)
		{
			printf("checksum code not found\n");
		}
	}
}


void aristmk5_state::machine_start()
{
	m_nvram->set_base(m_sram->base(), m_sram->bytes());

	archimedes_init();

	m_mk5_2KHz_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(aristmk5_state::mk5_2KHz_callback),this));
	m_mk5_VSYNC_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(aristmk5_state::mk5_VSYNC_callback),this));
	m_spi_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(aristmk5_state::spi_timer), this));
}

void aristmk5_state::machine_reset()
{
	archimedes_reset();
	m_mk5_2KHz_timer->adjust(attotime::from_hz((double)MASTER_CLOCK / 9 / 4096)); // 8MHz / 4096
	m_mk5_VSYNC_timer->adjust(attotime::from_hz(50000)); // default bit 1 & bit 2 == 0

	m_ioc_regs[IRQ_STATUS_B] |= 0x40; //hack, set keyboard irq empty to be ON

	/* load the roms according to what the operator wants */
	{
		uint8_t *ROM = memregion("maincpu")->base();
		uint8_t *PRG;// = memregion("prg_code")->base();
		int i;
		uint8_t op_mode;
		static const char *const rom_region[] = { "set_4.04.09", "set_4.04.00", "set_4.02.04", "game_prg" };

		op_mode = ioport("ROM_LOAD")->read();

		PRG = memregion(rom_region[op_mode & 3])->base();

		if(PRG!=nullptr)

		for(i=0;i<0x400000;i++)
			ROM[i] = PRG[i];
	}

	m_ldor_shift_reg = 0x55;
	m_coin_start_cycles = 0;
	m_sram_bank = 0;
	m_hopper_test = 1;
	m_coin_div = 0;
	m_spi_mux = 0;
	m_spi_latch = 0;
	m_spi_bits = 0;
	memset(m_spi_data, 0, sizeof(m_spi_data));
}


static MACHINE_CONFIG_START( aristmk5, aristmk5_state )
	MCFG_CPU_ADD("maincpu", ARM, MASTER_CLOCK/6)    // 12000000
	MCFG_CPU_PROGRAM_MAP(aristmk5_drame_map)

	MCFG_WATCHDOG_ADD("watchdog")
	MCFG_WATCHDOG_TIME_INIT(attotime::from_seconds(2))  /* 1.6 - 2 seconds */

	/* TODO: this isn't supposed to access a keyboard ... */
	MCFG_DEVICE_ADD("kart", AAKART, 12000000/128) // TODO: frequency

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(640, 400)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 400-1)
	MCFG_SCREEN_UPDATE_DRIVER(archimedes_state, screen_update)

	MCFG_PALETTE_ADD("palette", 0x200)

	MCFG_EEPROM_SERIAL_93C56_ADD("eeprom0")
	MCFG_EEPROM_SERIAL_93C56_ADD("eeprom1")

	MCFG_NVRAM_ADD_NO_FILL("nvram")

	// TL16C452FN U71
	MCFG_DEVICE_ADD("uart_0a", NS16450, MASTER_CLOCK / 9)
	MCFG_INS8250_OUT_INT_CB(DEVWRITELINE("uart_irq", input_merger_device, in0_w))
	MCFG_DEVICE_ADD("uart_0b", NS16450, MASTER_CLOCK / 9)
	MCFG_INS8250_OUT_INT_CB(DEVWRITELINE("uart_irq", input_merger_device, in1_w))

	// TL16C452FN U72
	MCFG_DEVICE_ADD("uart_1a", NS16450, MASTER_CLOCK / 9)
	MCFG_INS8250_OUT_INT_CB(DEVWRITELINE("uart_irq", input_merger_device, in2_w))
	MCFG_DEVICE_ADD("uart_1b", NS16450, MASTER_CLOCK / 9)
	MCFG_INS8250_OUT_INT_CB(DEVWRITELINE("uart_irq", input_merger_device, in3_w))

	// COMM port 4 - 5
	MCFG_DEVICE_ADD("uart_2a", NS16450, MASTER_CLOCK / 9)
//  MCFG_INS8250_OUT_INT_CB(WRITELINE(aristmk5_state, uart_irq_callback))
	MCFG_DEVICE_ADD("uart_2b", NS16450, MASTER_CLOCK / 9)
//  MCFG_INS8250_OUT_INT_CB(WRITELINE(aristmk5_state, uart_irq_callback))

	// COMM port 6 - 7
	MCFG_DEVICE_ADD("uart_3a", NS16450, MASTER_CLOCK / 9)
//  MCFG_INS8250_OUT_INT_CB(WRITELINE(aristmk5_state, uart_irq_callback))
	MCFG_DEVICE_ADD("uart_3b", NS16450, MASTER_CLOCK / 9)
//  MCFG_INS8250_OUT_INT_CB(WRITELINE(aristmk5_state, uart_irq_callback))

	MCFG_INPUT_MERGER_ACTIVE_HIGH("uart_irq")
	MCFG_INPUT_MERGER_OUTPUT_HANDLER(WRITELINE(aristmk5_state, uart_irq_callback))

	MCFG_DS1302_ADD("rtc", XTAL_32_768kHz)

	MCFG_TICKET_DISPENSER_ADD("hopper", attotime::from_msec(100), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_LOW)

	MCFG_SPEAKER_STANDARD_MONO("speaker")
	MCFG_SOUND_ADD("dac0", DAC_16BIT_R2R_TWOS_COMPLEMENT, 0) MCFG_SOUND_ROUTE(0, "speaker", 0.1) // unknown DAC
	MCFG_SOUND_ADD("dac1", DAC_16BIT_R2R_TWOS_COMPLEMENT, 0) MCFG_SOUND_ROUTE(0, "speaker", 0.1) // unknown DAC
	MCFG_SOUND_ADD("dac2", DAC_16BIT_R2R_TWOS_COMPLEMENT, 0) MCFG_SOUND_ROUTE(0, "speaker", 0.1) // unknown DAC
	MCFG_SOUND_ADD("dac3", DAC_16BIT_R2R_TWOS_COMPLEMENT, 0) MCFG_SOUND_ROUTE(0, "speaker", 0.1) // unknown DAC
	MCFG_SOUND_ADD("dac4", DAC_16BIT_R2R_TWOS_COMPLEMENT, 0) MCFG_SOUND_ROUTE(0, "speaker", 0.1) // unknown DAC
	MCFG_SOUND_ADD("dac5", DAC_16BIT_R2R_TWOS_COMPLEMENT, 0) MCFG_SOUND_ROUTE(0, "speaker", 0.1) // unknown DAC
	MCFG_SOUND_ADD("dac6", DAC_16BIT_R2R_TWOS_COMPLEMENT, 0) MCFG_SOUND_ROUTE(0, "speaker", 0.1) // unknown DAC
	MCFG_SOUND_ADD("dac7", DAC_16BIT_R2R_TWOS_COMPLEMENT, 0) MCFG_SOUND_ROUTE(0, "speaker", 0.1) // unknown DAC
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dac0", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac0", -1.0, DAC_VREF_NEG_INPUT)
	MCFG_SOUND_ROUTE_EX(0, "dac1", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac1", -1.0, DAC_VREF_NEG_INPUT)
	MCFG_SOUND_ROUTE_EX(0, "dac2", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac2", -1.0, DAC_VREF_NEG_INPUT)
	MCFG_SOUND_ROUTE_EX(0, "dac3", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac3", -1.0, DAC_VREF_NEG_INPUT)
	MCFG_SOUND_ROUTE_EX(0, "dac4", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac4", -1.0, DAC_VREF_NEG_INPUT)
	MCFG_SOUND_ROUTE_EX(0, "dac5", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac5", -1.0, DAC_VREF_NEG_INPUT)
	MCFG_SOUND_ROUTE_EX(0, "dac6", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac6", -1.0, DAC_VREF_NEG_INPUT)
	MCFG_SOUND_ROUTE_EX(0, "dac7", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac7", -1.0, DAC_VREF_NEG_INPUT)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( aristmk5_touch, aristmk5 )
	MCFG_DEVICE_MODIFY("uart_0a")
	MCFG_INS8250_OUT_TX_CB(DEVWRITELINE("microtouch", microtouch_device, rx))

	MCFG_MICROTOUCH_ADD("microtouch", 2400, DEVWRITELINE("uart_0a", ins8250_uart_device, rx_w))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( aristmk5_usa, aristmk5 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(aristmk5_usa_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( aristmk5_usa_touch, aristmk5_usa )
	MCFG_DEVICE_MODIFY("uart_0a")
	MCFG_INS8250_OUT_TX_CB(DEVWRITELINE("microtouch", microtouch_device, rx))

	MCFG_MICROTOUCH_ADD("microtouch", 2400, DEVWRITELINE("uart_0a", ins8250_uart_device, rx_w))
MACHINE_CONFIG_END

#define ARISTOCRAT_MK5_BIOS \
	ROM_REGION( 0x400000, "set_4.04.09", ROMREGION_ERASEFF ) /* setchip v4.04.08 4meg */ \
	ROM_LOAD32_WORD( "setchip v4.04.09.u7",  0x000000, 0x80000, CRC(e8e8dc75) SHA1(201fe95256459ce34fdb6f7498135ab5016d07f3) ) \
	ROM_LOAD32_WORD( "setchip v4.04.09.u11", 0x000002, 0x80000, CRC(ff7a9035) SHA1(4352c4336e61947c555fdc80c61f944076f64b64) ) \
	ROM_REGION( 0x400000, "set_4.04.00", ROMREGION_ERASEFF ) /* setchip v4.04.00 4meg 42pin */ \
	ROM_LOAD32_WORD( "setchip v4.04.00.u7",  0x000000, 0x80000, CRC(2453137e) SHA1(b59998e75ae3924da16faf47b9cfe9afd60d810c) ) \
	ROM_LOAD32_WORD( "setchip v4.04.00.u11", 0x000002, 0x80000, CRC(82dfa12a) SHA1(86fd0f0ad8d5d1bc503392a40bbcdadb055b2765) ) \
	ROM_REGION( 0x400000, "set_4.02.04", ROMREGION_ERASEFF ) /* setchip v4.02.04 */ \
	ROM_LOAD32_WORD( "setchip v4.02.04.u7",  0x000000, 0x80000, CRC(5a254b22) SHA1(8444f237b392df2a3cb42ea349e7af32f47dd544) ) \
	ROM_LOAD32_WORD( "setchip v4.02.04.u11", 0x000002, 0x80000, CRC(def36617) SHA1(c7ba5b08e884a8fb36c9fb51c08e243e32c81f89) ) \
	/* GALs */ \
	ROM_REGION( 0x600, "gals", 0 ) \
	ROM_LOAD( "a562837.u36",  0x000000, 0x000157, CRC(1f269234) SHA1(29940dd50fb55c632935f62ff44ca724379c7a43) ) \
	ROM_LOAD( "a562838.u65",  0x000200, 0x000157, CRC(f2f3c40a) SHA1(b795dfa5cc4e8127c3f3a0906664910d1325ec92) ) \
	ROM_LOAD( "a562840.u22",  0x000400, 0x000157, CRC(941d4cdb) SHA1(1ca091fba69e92f262dbb3d40f515703c8981793) ) \
	ROM_REGION16_BE( 0x100, "eeprom0", ROMREGION_ERASEFF ) \
	ROM_REGION16_BE( 0x100, "eeprom1", ROMREGION_ERASEFF ) \

#define ARISTOCRAT_MK5_BIOS_HAVE_EEPROMS \
	ROM_REGION( 0x400000, "set_4.04.09", ROMREGION_ERASEFF ) /* setchip v4.04.08 4meg */ \
	ROM_LOAD32_WORD( "setchip v4.04.09.u7",  0x000000, 0x80000, CRC(e8e8dc75) SHA1(201fe95256459ce34fdb6f7498135ab5016d07f3) ) \
	ROM_LOAD32_WORD( "setchip v4.04.09.u11", 0x000002, 0x80000, CRC(ff7a9035) SHA1(4352c4336e61947c555fdc80c61f944076f64b64) ) \
	ROM_REGION( 0x400000, "set_4.04.00", ROMREGION_ERASEFF ) /* setchip v4.04.00 4meg 42pin */ \
	ROM_LOAD32_WORD( "setchip v4.04.00.u7",  0x000000, 0x80000, CRC(2453137e) SHA1(b59998e75ae3924da16faf47b9cfe9afd60d810c) ) \
	ROM_LOAD32_WORD( "setchip v4.04.00.u11", 0x000002, 0x80000, CRC(82dfa12a) SHA1(86fd0f0ad8d5d1bc503392a40bbcdadb055b2765) ) \
	ROM_REGION( 0x400000, "set_4.02.04", ROMREGION_ERASEFF ) /* setchip v4.02.04 */ \
	ROM_LOAD32_WORD( "setchip v4.02.04.u7",  0x000000, 0x80000, CRC(5a254b22) SHA1(8444f237b392df2a3cb42ea349e7af32f47dd544) ) \
	ROM_LOAD32_WORD( "setchip v4.02.04.u11", 0x000002, 0x80000, CRC(def36617) SHA1(c7ba5b08e884a8fb36c9fb51c08e243e32c81f89) ) \
	/* GALs */ \
	ROM_REGION( 0x600, "gals", 0 ) \
	ROM_LOAD( "a562837.u36",  0x000000, 0x000157, CRC(1f269234) SHA1(29940dd50fb55c632935f62ff44ca724379c7a43) ) \
	ROM_LOAD( "a562838.u65",  0x000200, 0x000157, CRC(f2f3c40a) SHA1(b795dfa5cc4e8127c3f3a0906664910d1325ec92) ) \
	ROM_LOAD( "a562840.u22",  0x000400, 0x000157, CRC(941d4cdb) SHA1(1ca091fba69e92f262dbb3d40f515703c8981793) ) \


ROM_START( aristmk5 )
	ARISTOCRAT_MK5_BIOS

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END



// 0200751V - 10 Credit Multiplier / 20 Line Multiline.
// ADONIS - NSW/ACT A - 25/05/98  Revision: 10  602/9.
ROM_START( adonis )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000bf8
	    0x000000-0x05eb1b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xfc98a056
	        Calculated Checksum 0xfc98a056  (OK)
	    0x05eb1c-0x10fa8b is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x05eb1c-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
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
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000bf8
	    0x000000-0x05cdc3 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x91f374c7
	        Calculated Checksum 0x91f374c7  (OK)
	    0x05cdc4-0x11000b is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x05cdc4-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
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


// MV4124/1 - 5,10,25,50 Credit Multiplier / 20 Line Multiline.
// Adonis [Reel Game] - Export B - 31/07/01.
// Marked as BHG1508.
ROM_START( adonisu )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000d18
	    0x000000-0x0e8a7b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xe6715f98
	        Calculated Checksum 0xc80cd95e  (BAD)
	    0x0e8a7c-0x1c5f47 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x0e8a7c-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	// the checksum only covers part of the first 2 roms, marked all as BAD_DUMP because it can't be trusted without a full redump.
	ROM_LOAD32_WORD( "bhg1508.u7",  0x000000, 0x80000, BAD_DUMP CRC(ed6254d7) SHA1(d2b790fdd7f5fc7b78fcfc4c96d0fc272ccf8da6) )
	ROM_LOAD32_WORD( "bhg1508.u11", 0x000002, 0x80000, BAD_DUMP CRC(1f629286) SHA1(bce380a6a76c77bc790436bd6f94474a1db0c231) )
	ROM_LOAD32_WORD( "bhg1508.u8",  0x100000, 0x80000, BAD_DUMP CRC(b756c96d) SHA1(494df20090d415e83d599023203c13273e7925ad) )
	ROM_LOAD32_WORD( "bhg1508.u12", 0x100002, 0x80000, BAD_DUMP CRC(1d3b6b8f) SHA1(1ddcfd7323cc7e79d3e39d913fdb5cf5cd53d56d) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( adonisce )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	/*
	    Checksum code found at 0x000c44
	    0x000000-0x06ddab is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x07c97aad
	        Calculated Checksum 0x07c97aad  (OK)
	    0x06ddac-0x2a41cb is the non-Checksummed range
	*/
	ROM_LOAD32_WORD("0201005v.u7",  0x0000000, 0x0080000, CRC(32149323) SHA1(abfc6a8518a39528db3700c2cb558e925ceeda6d) )
	ROM_LOAD32_WORD("0201005v.u11", 0x0000002, 0x0080000, CRC(309b0b55) SHA1(669568031d305b29395345a26a5d004d83881433) )
	ROM_LOAD32_WORD("0201005v.u8",  0x0100000, 0x0080000, CRC(e9185e3c) SHA1(99609a152a55246d0f5377f943deec47e68fb9fc) )
	ROM_LOAD32_WORD("0201005v.u12", 0x0100002, 0x0080000, CRC(8b675dff) SHA1(275579d21be51e6ca01be7c57018e142d1d40875) )
	ROM_LOAD32_WORD("0201005v.u9",  0x0200000, 0x0080000, CRC(c2e973e7) SHA1(e89bdaa56b0c3c7bd77c8141421f76be9ff2e71b) )
	ROM_LOAD32_WORD("0201005v.u13", 0x0200002, 0x0080000, CRC(e005a7e8) SHA1(cbb313f5d1d04c5a441b3f92b7a90a281ddb4885) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( baddog )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000ae4
	    0x000000-0x056f3f is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x15ac4012
	        Calculated Checksum 0x15ac4012  (OK)
	    0x056f40-0x2fb607 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x056f40-0x2fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0200428v.u7",  0x0000000, 0x0080000, CRC(25aa8109) SHA1(cf4521b3d447812d2d9dbfdab9fe0cec71cdeb2e) )
	ROM_LOAD32_WORD("0200428v.u11", 0x0000002, 0x0080000, CRC(774ff977) SHA1(5ce1aa8b7598b4bc8e5fa44de1c03b5f2851f5de) )
	ROM_LOAD32_WORD("0200428v.u8",  0x0100000, 0x0080000, CRC(e52a279a) SHA1(4a3a080d840d8a894ec0ba0250a566831377f0f8) )
	ROM_LOAD32_WORD("0200428v.u12", 0x0100002, 0x0080000, CRC(562aa123) SHA1(825a2d23321b636a3ff2565b2b72df3b97bd0ec8) )
	ROM_LOAD32_WORD("0200428v.u9",  0x0200000, 0x0080000, CRC(66d5a7f7) SHA1(1a1f845a97677c43ff1090231434ae9d3d36ab4c) )
	ROM_LOAD32_WORD("0200428v.u13", 0x0200002, 0x0080000, CRC(883b2ec3) SHA1(5b431d8c9c8eabca65ab22dcf2bdb22d49445bb1) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( blackpnt )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000bb0
	    0x000000-0x056d8b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xeebac434
	        Calculated Checksum 0xeebac434  (OK)
	    0x056d8c-0x138557 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x056d8c-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0200818v.u7",  0x0000000, 0x0080000, CRC(eed76145) SHA1(6a40a6ba2ce320a37b086dc4916c92c8e38c065e) )
	ROM_LOAD32_WORD("0200818v.u11", 0x0000002, 0x0080000, CRC(de3358d3) SHA1(4f290940d8af9fe8d404802d5cecfd2d098eff12) )
	ROM_LOAD32_WORD("0200818v.u8",  0x0100000, 0x0080000, CRC(58ddfb50) SHA1(c2152e65fa119136b7944b69e650310db78e62a8) )
	ROM_LOAD32_WORD("0200818v.u12", 0x0100002, 0x0080000, CRC(bb2bf7bb) SHA1(f88208238a69fc79e33af17f39e25cd2857d7172) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( bootsctn )
	ARISTOCRAT_MK5_BIOS
	/*
	    0x000000-0x06c177 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xb0980753
	        Calculated Checksum 0xb0980753  (OK)
	    0x06c178-0x384a9b is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x06c178-0x3fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100812v.u7",  0x0000000, 0x0080000, CRC(f8e12462) SHA1(82a25757b2146204b86e557b8f1c45280e0668a8) )
	ROM_LOAD32_WORD("0100812v.u11", 0x0000002, 0x0080000, CRC(df066d27) SHA1(310422c78e93ce9f1f58b4a58a59bc2eba5c502a) )
	ROM_LOAD32_WORD("0100812v.u8",  0x0100000, 0x0080000, CRC(08e8de8d) SHA1(913d3e51821d8885affd2750c18d1000629b79d9) )
	ROM_LOAD32_WORD("0100812v.u12", 0x0100002, 0x0080000, CRC(87ddc7ef) SHA1(91473d8fd266a909fa8d4ec3df3a61861c6e9f4c) )
	ROM_LOAD32_WORD("0100812v.u9",  0x0200000, 0x0080000, CRC(a1ca5f2b) SHA1(c8fc6aff0c3819370339143966ec76910e40c671) )
	ROM_LOAD32_WORD("0100812v.u13", 0x0200002, 0x0080000, CRC(fca82ee7) SHA1(bb70f2e04047a58b697dca536b95f9bbcc295a8a) )
	ROM_LOAD32_WORD("0100812v.u10", 0x0300000, 0x0080000, CRC(b574c12d) SHA1(3b1d1d00ef3eae23493e2b0381ab80490af510d4) )
	ROM_LOAD32_WORD("0100812v.u14", 0x0300002, 0x0080000, CRC(75b9b89e) SHA1(08d487b3722f2ea5d2d18c78f571a44c78616dbe) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


// MV4098 - 10 Credit Multiplier / 9 Line Multiline.
// BOOT SCOOTIN' - Export A - 25/08/99.
// All devices are 27c4002 instead of 27c4096.
// Marked as GHG101202 and 92.767%
ROM_START( bootsctnu )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000d08
	    0x000000-0x0941ab is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xdf68cecf
	        Calculated Checksum 0xed145d01  (BAD)
	    0x0941ac-0x328187 is the non-Checksummed range (suspicious endpoint)
	*/

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	// the checksum only covers part of the first 2 roms, marked all as BAD_DUMP because it can't be trusted without a full redump.
	ROM_LOAD32_WORD( "ghg101202.u7",  0x000000, 0x80000, BAD_DUMP CRC(ca26f31e) SHA1(e8da31cc8d12bf8a28f1ca4d796259ae9010f8af) )  // 92.767%
	ROM_LOAD32_WORD( "ghg101202.u11", 0x000002, 0x80000, BAD_DUMP CRC(61da1767) SHA1(83d4df1060975f03f291b9119c0d2b8debb0fb60) )  // 92.767%
	ROM_LOAD32_WORD( "ghg101202.u8",  0x100000, 0x80000, BAD_DUMP CRC(9ae4d616) SHA1(60d4d36f75685dfe21f914fa4682cd6a64fcfa58) )  // base
	ROM_LOAD32_WORD( "ghg101202.u12", 0x100002, 0x80000, BAD_DUMP CRC(2c50c083) SHA1(ae3b01200d152df9b2966b5308c71e9d1ad43d78) )  // base
	ROM_LOAD32_WORD( "ghg101202.u9",  0x200000, 0x80000, BAD_DUMP CRC(c0a4920d) SHA1(d2c6d259d2e067b6e5ad72a6ef164aac7d72bc5a) )  // base
	ROM_LOAD32_WORD( "ghg101202.u13", 0x200002, 0x80000, BAD_DUMP CRC(55716d82) SHA1(5b9982a49201842e9551a9c763a6babbb47a863e) )  // base
	ROM_LOAD32_WORD( "ghg101202.u10", 0x300000, 0x80000, BAD_DUMP CRC(3ecdf7ee) SHA1(9d658a22da737daafdf6cb0d49009796036d04b1) )  // base
	ROM_LOAD32_WORD( "ghg101202.u14", 0x300002, 0x80000, BAD_DUMP CRC(18934c51) SHA1(f7c9c95c687dbfe89747e7877157fde37bc1119e) )  // base

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( bumblbug )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b68
	    0x000000-0x05b94b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xf5d418fe
	        Calculated Checksum 0xf5d418fe  (OK)
	    0x05b94c-0x0fc69f is the non-Checksummed range (unusual endpoint)
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0200510v.u7",  0x0000000, 0x0080000, CRC(d4cfce73) SHA1(735c385779afe55e521dbfe9ebfdc55fe3346349) )
	ROM_LOAD32_WORD("0200510v.u11", 0x0000002, 0x0080000, CRC(5d888245) SHA1(bbbe61e09bebd5fcb79f060d5caee15100c9a685) )
	// missing 8+12?

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( bumblbugql )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000ac8
	    0x000000-0x05554b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x66b20ae6
	        Calculated Checksum 0x66b20ae6  (OK)
	    0x05554c-0x1c4e2b is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x05554c-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0200456v.u7",  0x000000, 0x80000, CRC(f04dd78b) SHA1(443057fc3e02406d46cf68f95e85e5a0fd8e7b1e) )
	ROM_LOAD32_WORD( "0200456v.u11", 0x000002, 0x80000, CRC(3b50b21b) SHA1(7c20d1bfb82cdd19c046a545ae251e3560b8f00d) )
	ROM_LOAD32_WORD( "0200456v.u8",  0x100000, 0x80000, CRC(da86d682) SHA1(b1aa739215f1f0967d6a03060d9a2f10115c0b03) )
	ROM_LOAD32_WORD( "0200456v.u12", 0x100002, 0x80000, CRC(9f0d7615) SHA1(1453b1476510e1dd68bc14feba72dc59b9dfe676) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


// 593 - 10 Credit Multiplier / 9 Line Multiline.
// Bumble Bugs - Export D - 05/07/97.
// All devices are 27c4002 instead of 27c4096.
// Marked as CHG047903 and 92.691%
ROM_START( bumblbugu )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000d08
	    0x000000-0x0b1f47 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x9f3936f9
	        Calculated Checksum 0x16f5c058  (BAD)
	    0x0b1f48-0x183c1f is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x0b1f48-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	// the checksum only covers part of the first 2 roms, marked all as BAD_DUMP because it can't be trusted without a full redump.
	ROM_LOAD32_WORD( "chg047903.u7",  0x000000, 0x80000, BAD_DUMP CRC(ec605a36) SHA1(114e0840cfbd0c64645a5a33065db85462a0ba2d) )  // 92.691%
	ROM_LOAD32_WORD( "chg047903.u11", 0x000002, 0x80000, BAD_DUMP CRC(17b154bd) SHA1(efdf307670a3d74f7980fec2d2197d837d4c26e2) )  // 92.691%
	ROM_LOAD32_WORD( "chg047903.u8",  0x100000, 0x80000, BAD_DUMP CRC(e0c01d01) SHA1(9153129fd348a97da7cccf002e5d03e4b4db9264) )  // base
	ROM_LOAD32_WORD( "chg047903.u12", 0x100002, 0x80000, BAD_DUMP CRC(28700d5d) SHA1(87a583cd487da6cb4c2da5f62297f0e577269fae) )  // base

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( buttdeli )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000adc
	    0x000000-0x04477f is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x19143954
	        Calculated Checksum 0x19143954  (OK)
	    0x044780-0x1c713b is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x044780-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0200143v.u7",  0x0000000, 0x0080000, CRC(7f69cdfc) SHA1(1241741d21334df10d60080555824a87eae93db3) )
	ROM_LOAD32_WORD("0200143v.u11", 0x0000002, 0x0080000, CRC(1ddf8732) SHA1(dc09db14c251699fdd46068f18ad6214e8752939) )
	ROM_LOAD32_WORD("0200143v.u8",  0x0100000, 0x0080000, CRC(24d8135e) SHA1(1bc69e9927afe0300d15a49ca06ae527774b295a) )
	ROM_LOAD32_WORD("0200143v.u12", 0x0100002, 0x0080000, CRC(0d58cf28) SHA1(aa65b7ee88b5bc872008a46e60bd49d9e5eda153) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( cashcat )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	/*
	    Checksum code found at 0x000adc
	    0x000000-0x04477f is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x19143954
	        Calculated Checksum 0x19143954  (OK)
	    0x044780-0x1c713b is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x044780-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_LOAD32_WORD("0300863v.u7",  0x0000000, 0x0080000, CRC(de0f0202) SHA1(994f6c47b1e2e0e133853dc69b189752104486e4) )
	ROM_LOAD32_WORD("0300863v.u11", 0x0000002, 0x0080000, CRC(e60e8bd1) SHA1(ffaa7be8968047b9ee54a117d273a14cbca41028) )
	ROM_LOAD32_WORD("0300863v.u8",  0x0100000, 0x0080000, CRC(37d41d35) SHA1(c959b787383d6f91d20e18f37a38a965407a9ff0) )
	ROM_LOAD32_WORD("0300863v.u12", 0x0100002, 0x0080000, CRC(f930fc07) SHA1(cb3fdbd5b87af7b14067f7999740470d3cf434df) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( cashcham )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000ae0
	    0x000000-0x055f83 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x159a2aa3
	        Calculated Checksum 0x159a2aa3  (OK)
	    0x055f84-0x1dbdd7 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x055f84-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100438v.u7",  0x0000000, 0x0080000, CRC(c942ef22) SHA1(4f56674f749602ae928832f98a641e680af8989b) )
	ROM_LOAD32_WORD("0100438v.u11", 0x0000002, 0x0080000, CRC(64921874) SHA1(5aa6a0d6e29f5e400e275f27b6adfbef595fe83a) )
	ROM_LOAD32_WORD("0100438v.u8",  0x0100000, 0x0080000, CRC(a8868277) SHA1(e199448a0a920219dc15443813061653b94d6d3a) )
	ROM_LOAD32_WORD("0100438v.u12", 0x0100002, 0x0080000, CRC(7ae3b5db) SHA1(238698b72f529ac4fb292d08267069d1da01b43b) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( cashchama )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b00
	    0x000000-0x05ca1b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xa32ccd1b
	        Calculated Checksum 0xa32ccd1b  (OK)
	    0x05ca1c-0x1dbdd7 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x05ca1c-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0200437v.u7",  0x0000000, 0x0080000, CRC(a287fd5a) SHA1(7d06f679e5ff38e0989819410856361962c93e42) )
	ROM_LOAD32_WORD("0200437v.u11", 0x0000002, 0x0080000, CRC(1875532b) SHA1(e410524b94b1c7860c1ef81ce5e0b4bf992f12ad) )
	ROM_LOAD32_WORD("0200437v.u8",  0x0100000, 0x0080000, CRC(edbfc684) SHA1(8849374e5df34359d228a4b447c409b76fe36b35) )
	ROM_LOAD32_WORD("0200437v.u12", 0x0100002, 0x0080000, CRC(571aab82) SHA1(03895d1a08d2dd868fd594db1aaeb29b295f0d98) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( cashchamnz )
	ARISTOCRAT_MK5_BIOS
	// checksum code not found (uses different startup sequence)
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0300781v.u7",  0x0000000, 0x0080000, CRC(009e109e) SHA1(b912b474a226af17bef554f4db6fade7cd2e558f) )
	ROM_LOAD32_WORD("0300781v.u11", 0x0000002, 0x0080000, CRC(826da4ac) SHA1(6bf852b438f5257474c265ace2826b7bd0d9b087) )
	ROM_LOAD32_WORD("0300781v.u8",  0x0100000, 0x0080000, CRC(f798ab06) SHA1(0f51ffd0e7abee6af0c5a29ab9ad1c8bfcd567a0) )
	ROM_LOAD32_WORD("0300781v.u12", 0x0100002, 0x0080000, CRC(2aeb0265) SHA1(50e526ecccfdd35f7e156e1873cf4c81fb117069) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

// 603(a) - 3,5,10,25,50 Credit Multiplier / 20 Line Multiline.
// Cash Chameleon 100cm - Export B - 06/12/96.
// Marked as DHG4078.
ROM_START( cashchamu )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000d08
	    0x000000-0x09b413 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x741cd9a0
	        Calculated Checksum 0x740e5ad7  (BAD)
	    0x09b414-0x1b550b is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x09b414-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	// the checksum only covers part of the first 2 roms, marked all as BAD_DUMP because it can't be trusted without a full redump.
	ROM_LOAD32_WORD( "dhg4078-99.u7",  0x000000, 0x80000, BAD_DUMP CRC(cb407a19) SHA1(d98421d6548e48b413f6dfcab4e240e98fcc9a69) )
	ROM_LOAD32_WORD( "dhg4078-99.u11", 0x000002, 0x80000, BAD_DUMP CRC(94d73843) SHA1(ab236750c67e7fff3af831f1d03f45c45f280fd1) )
	ROM_LOAD32_WORD( "dhg4078-99.u8",  0x100000, 0x80000, BAD_DUMP CRC(4cae8a5d) SHA1(3232461afd75ce71f8a2cb4ac7e9a3caeb8aabcd) )
	ROM_LOAD32_WORD( "dhg4078-99.u12", 0x100002, 0x80000, BAD_DUMP CRC(39e17f0b) SHA1(25a0364fa45e4e78d6c365b0739606e71597bd71) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( cashcra5 )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b80
	    0x000000-0x06076b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x2c872d3e
	        Calculated Checksum 0x2c872d3e  (OK)
	    0x06076c-0x1a2ecf is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x06076c-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0300467v.u7",  0x0000000, 0x0080000, CRC(b0ff2aae) SHA1(b05667ffe952cae7a6581398552db6e47921090e) )
	ROM_LOAD32_WORD("0300467v.u11", 0x0000002, 0x0080000, CRC(25a18efa) SHA1(0ee4f6cc66322397dbde53af2149f5fb35d788df) )
	ROM_LOAD32_WORD("0300467v.u8",  0x0100000, 0x0080000, CRC(d4e7b4ba) SHA1(147a1ed5cdcbb84466a8024ad7e0778f85374489) )
	ROM_LOAD32_WORD("0300467v.u12", 0x0100002, 0x0080000, CRC(570c7f8a) SHA1(7c9527e0b37970b7960c723727c3c650a48e8125) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


// 630 - 10 Credit Multiplier / 9 Line Multiline.
// The Chariot Challenge - Venezuela - A - 10/08/98.
// 04J00714
ROM_START( chariotc )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000ba8
	    0x000000-0x07dbb7 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x203ac6e8
	        Calculated Checksum 0x203ac6e8  (OK)
	    0x07dbb8-0x1b3787 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x07dbb8-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "04j00714.u7",  0x000000, 0x80000, CRC(2f3a1af7) SHA1(e1448116a81687cb79dd380dfbc8decf1f83e649) )
	ROM_LOAD32_WORD( "04j00714.u11", 0x000002, 0x80000, CRC(ef4f49e8) SHA1(8ff21f679a55cdfebcf22c109dfd3b41773293bd) )
	ROM_LOAD32_WORD( "04j00714.u8",  0x100000, 0x80000, CRC(fa24cfde) SHA1(1725c38a8a15915d8aa8e59afef9ce1d6e8d01c5) )
	ROM_LOAD32_WORD( "04j00714.u12", 0x100002, 0x80000, CRC(b8d4a5ec) SHA1(097e44cdb30b9aafd7f5358c8f0cdd130ec0615e) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( chariotca )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000ba8
	    0x000000-0x0603fb is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xbe63efe6
	        Calculated Checksum 0xbe63efe6  (OK)
	    0x0603fc-0x17a75b is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x0603fc-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100787v.u7",  0x0000000, 0x0080000, CRC(845f9913) SHA1(df6121290b30ff4a9c2d0e690cf8e7797e9a8612) )
	ROM_LOAD32_WORD("0100787v.u11", 0x0000002, 0x0080000, CRC(bcbf9de9) SHA1(191ce749fe0d29b2783fb78d9338a00d65104daa) )
	ROM_LOAD32_WORD("0100787v.u8",  0x0100000, 0x0080000, CRC(a3a74ecb) SHA1(52b3a41573a9fa1de05ce01a858e400f80e595b8) )
	ROM_LOAD32_WORD("0100787v.u12", 0x0100002, 0x0080000, CRC(b44cf571) SHA1(04447820e015425493cade5611b3eb2f21e48c2e) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( checkma5 )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000c38
	    0x000000-0x071847 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x0de9b6ca
	        Calculated Checksum 0x0de9b6ca  (OK)
	    0x071848-0x25ff4b is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x071848-0x2fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("01j00681.u7",  0x0000000, 0x0080000, CRC(059b940e) SHA1(f637508dafbd37169429c495a893addbc6d28834) )
	ROM_LOAD32_WORD("01j00681.u11", 0x0000002, 0x0080000, CRC(5fb7bfb3) SHA1(2ad8b3c4753d19f9e3254ef3f4059951d7a111b4) )
	ROM_LOAD32_WORD("01j00681.u8",  0x0100000, 0x0080000, CRC(6912cc4a) SHA1(9469a6a0d2fd39d85655a8c7bc0668752f5f11fa) )
	ROM_LOAD32_WORD("01j00681.u12", 0x0100002, 0x0080000, CRC(b538bcbc) SHA1(cda404f9b16e7e76a33c208f62a5ac9c5e02aac4) )
	ROM_LOAD32_WORD("01j00681.u9",  0x0200000, 0x0080000, CRC(53a573f0) SHA1(d51d698dcec273d157319200ad1c215e930b96ce) )
	ROM_LOAD32_WORD("01j00681.u13", 0x0200002, 0x0080000, CRC(ad12a718) SHA1(0c36729cb8da800668f533f65fcc870f5dfc0f6a) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( chickna5 )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b80
	    0x000000-0x053fb7 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x8afbaabc
	        Calculated Checksum 0x8afbaabc  (OK)
	    0x053fb8-0x2fda37 is the non-Checksummed range (unusual endpoint)
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100351v.u7",  0x0000000, 0x0080000, CRC(be69c21c) SHA1(8b546727b5972f33d077db0a64aa41a7fde6d417) )
	ROM_LOAD32_WORD("0100351v.u11", 0x0000002, 0x0080000, CRC(65423867) SHA1(992bb4f717f79233d1300d248b145f95a627cff2) )
	ROM_LOAD32_WORD("0100351v.u8",  0x0100000, 0x0080000, CRC(3161c16f) SHA1(8f2b14ec8ba5c9da80a226d2ce5a7e5256c8cbb4) )
	ROM_LOAD32_WORD("0100351v.u12", 0x0100002, 0x0080000, CRC(77b5d777) SHA1(f03afeaff08c9216e714f1e4bcc50292ba87ace4) )
	ROM_LOAD32_WORD("0100351v.u9",  0x0200000, 0x0080000, CRC(5506777b) SHA1(42512577056e1caefbea0e74879780c56787af13) )
	ROM_LOAD32_WORD("0100351v.u13", 0x0200002, 0x0080000, CRC(88a1ccae) SHA1(e242f48f99044b4fdf1bf36d8e105df09f94aa50) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


// 596 - 10 Credit Multiplier / 9 Line Multiline. (touch)
// Chicken - Export C - 23/02/98.
// Marked as RHG0730, 92.588% and 'touch'
// All devices are 27c4002 instead of 27c4096.
ROM_START( chickna5u )
	ARISTOCRAT_MK5_BIOS

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "rhg073003.u7",  0x000000, 0x080000, CRC(06558129) SHA1(be726c0d35776faf1ecd20eb0a193e68a1fb1a84) )
	ROM_LOAD32_WORD( "rhg073003.u11", 0x000002, 0x080000, CRC(0eadf5d4) SHA1(b783f6e1911fc098d1b4d1d8c75862e031078e5b) )
	ROM_LOAD32_WORD( "rhg073003.u8",  0x100000, 0x080000, CRC(683e96bc) SHA1(bca8e87bea9f7044fa29dc4518e2ac5b429e3313) )
	ROM_LOAD32_WORD( "rhg073003.u12", 0x100002, 0x080000, CRC(8313b03b) SHA1(d2a91bae8063d89ec9a1edab6df3e6711719d2c2) )
	ROM_LOAD32_WORD( "rhg073003.u9",  0x200000, 0x080000, CRC(9c08aefa) SHA1(fe3ffa8eb308ab216cc08dd2ce51113b4ef74c4a) )
	ROM_LOAD32_WORD( "rhg073003.u13", 0x200002, 0x080000, CRC(69fd4f89) SHA1(4e0469caecf9293197a4a5de960eb9dcfee39ca3) )
	ROM_LOAD32_WORD( "rhg073003.u10", 0x300000, 0x080000, CRC(9aae49d7) SHA1(5cf87b747ea7561766fe0ffc15967fea657b252b) )
	ROM_LOAD32_WORD( "rhg073003.u14", 0x300002, 0x080000, CRC(240f7759) SHA1(1fa5ba0185b027101dae207ec5d28b07d3d73fc2) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( chickna5qld )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000ac8
	    0x000000-0x05f193 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xeff4424a
	        Calculated Checksum 0xeff4424a  (OK)
	    0x05f194-0x3a9a7f is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x05f194-0x3fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0200530v.u7",  0x000000, 0x80000, CRC(2d53de96) SHA1(6f2ed8f68d0474021a302d7e06ba869c0f1f7262) )
	ROM_LOAD32_WORD( "0200530v.u11", 0x000002, 0x80000, CRC(ed80acab) SHA1(d7ec3a063c45180e0b32935db9b8a01bcdaaa9a7) )
	ROM_LOAD32_WORD( "0200530v.u8",  0x100000, 0x80000, CRC(fbe704d3) SHA1(fe06489ba9628307f54ab60ab6909b45491116ae) )
	ROM_LOAD32_WORD( "0200530v.u12", 0x100002, 0x80000, CRC(c78215c0) SHA1(52b372df4a0f78cc557a874d8d40819aed191cdd) )
	ROM_LOAD32_WORD( "0200530v.u9",  0x200000, 0x80000, CRC(835903f5) SHA1(0a3bbc4e81629265d873716e9120eb95ea28b42c) )
	ROM_LOAD32_WORD( "0200530v.u13", 0x200002, 0x80000, CRC(7a5c1ca3) SHA1(d56103142392234298117d6b0d9163e0d3e52a7e) )
	ROM_LOAD32_WORD( "0200530v.u10", 0x300000, 0x80000, CRC(6c9399c1) SHA1(066afebc8ffcdf9e6a176e18997db242aa84269b) )
	ROM_LOAD32_WORD( "0200530v.u14", 0x300002, 0x80000, CRC(e87cf6c9) SHA1(a330644658da6100d7231b1c47260dc7f2e88448) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( coralrc2 )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000be8
	    0x000000-0x05ba63 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x12fce303
	        Calculated Checksum 0x12fce303  (OK)
	    0x05ba64-0x12b3e3 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x05ba64-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100919v.u7",  0x0000000, 0x0080000, CRC(02c430c3) SHA1(f4bae1aa5437af1df2a04f700da044bc4fb652b7) )
	ROM_LOAD32_WORD("0100919v.u11", 0x0000002, 0x0080000, CRC(8cd17e90) SHA1(c6d6a29e62ca6e1b278a2e1d1b358e10ca2de4ed) )
	ROM_LOAD32_WORD("0100919v.u8",  0x0100000, 0x0080000, CRC(1ee9557c) SHA1(3bee295509d4b0c11ce41a7a20ba91230b7cb4ca) )
	ROM_LOAD32_WORD("0100919v.u12", 0x0100002, 0x0080000, CRC(9ea140b5) SHA1(11f6b9ab60117f236b464c9dbc939dfb8f240359) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( cuckoo )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b10
	    0x000000-0x05f63f is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x6aa5ad46
	        Calculated Checksum 0x6aa5ad46  (OK)
	    0x05f640-0x1b1deb is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x05f640-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0200753v.u7",  0x0000000, 0x0080000, CRC(5c7ef84a) SHA1(59563a076ecf391ac1779e0dcd530a1ea158a4e3) )
	ROM_LOAD32_WORD("0200753v.u11", 0x0000002, 0x0080000, CRC(a69c1416) SHA1(7fe57a194bf29346c039dfac1326f3ee5080e630) )
	ROM_LOAD32_WORD("0200753v.u8",  0x0100000, 0x0080000, CRC(a7b4242c) SHA1(4e6961e9b3267d17b93075c41a691a8033a34d90) )
	ROM_LOAD32_WORD("0200753v.u12", 0x0100002, 0x0080000, CRC(cb706eb7) SHA1(cbd6235ca7a29c78ef2cb659d9c21466ed39b360) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


// MV4104  3,5,10,20,25,50 Credit Multiplier / 9-20 Line Multiline.
// CUCKOO - Export C - 02/02/00.
// All devices are 27c4002 instead of 27c4096.
ROM_START( cuckoou )
	ARISTOCRAT_MK5_BIOS_HAVE_EEPROMS
	/*
	    Checksum code found at 0x000d18
	    0x000000-0x0a588b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x9e544942
	        Calculated Checksum 0x9e544942  (OK)
	    0x0a588c-0x184b17 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x0a588c-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "chg1195.u7",  0x000000, 0x80000, CRC(0bd17338) SHA1(b8f467bdf8d76533a2b7d44fe93be414f25a3c31) )
	ROM_LOAD32_WORD( "chg1195.u11", 0x000002, 0x80000, CRC(4c407deb) SHA1(57589e61a376ddff99cd420eb47bf8c902c6a249) )
	ROM_LOAD32_WORD( "chg1195.u8",  0x100000, 0x80000, CRC(33f52052) SHA1(89cbfe588d91244adff4c520fa94962d69ff20bf) )
	ROM_LOAD32_WORD( "chg1195.u12", 0x100002, 0x80000, CRC(00bb7597) SHA1(f4d6b21091e320a82d59477469340633b001ed0d) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )

	ROM_REGION16_BE( 0x100, "eeprom0", 0 )
	ROM_LOAD16_WORD_SWAP( "eeprom0",      0x000000, 0x000100, CRC(fea8a821) SHA1(c744cac6af7621524fc3a2b0a9a135a32b33c81b) )

	ROM_REGION16_BE( 0x100, "eeprom1", 0 )
	ROM_LOAD16_WORD_SWAP( "eeprom1",      0x000000, 0x000100, CRC(415b9c77) SHA1(86a3b3aabd81f5fcf767dd53f7034f7d58f2020e) )

	ROM_REGION( 0x80000, "nvram", 0 )
	ROM_LOAD( "nvram",        0x000000, 0x080000, CRC(64c895fe) SHA1(12c75338dd1b2260d0581744cef1b705c718727f) )

	ROM_REGION( 0x20, "rtc", 0 )
	ROM_LOAD( "rtc",          0x000000, 0x00001f, CRC(6909acb0) SHA1(6a4589599cd1c477e916474e7b029e9a4e92019b) )

ROM_END


ROM_START( dstbloom )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000adc
	    0x000000-0x0431d3 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x3a2c9103
	        Calculated Checksum 0x3a2c9103  (OK)
	    0x0431d4-0x1cb32b is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x0431d4-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0200111v.u7",  0x0000000, 0x0080000, CRC(fbfaa3fe) SHA1(3f915261503fc97eb556422e9ccdac81372c04cc) )
	ROM_LOAD32_WORD("0200111v.u11", 0x0000002, 0x0080000, CRC(ed4e8dca) SHA1(1953033e570634cbcf8cd11194c14c57ffc6be53) )
	ROM_LOAD32_WORD("0200111v.u8",  0x0100000, 0x0080000, CRC(cc0d567c) SHA1(c4da3d0c0c4420a9f8fbb6403db983b3e27d4b50) )
	ROM_LOAD32_WORD("0200111v.u12", 0x0100002, 0x0080000, CRC(0ad41815) SHA1(131efc6ed45d8f44a667bd30380c9e37c64f2c42) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( dmdfever )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	/*
	    Checksum code found at 0x000ad8
	    0x000000-0x054f3f is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x87d3b331
	        Calculated Checksum 0x87d3b331  (OK)
	    0x054f40-0x0ef137 is the non-Checksummed range (unusual endpoint)
	*/
	ROM_LOAD32_WORD( "0200302v.u7",  0x000000, 0x80000, CRC(d90032f9) SHA1(9c34e626168bdfa3ff2722d9ff1970d826135cf7) )
	ROM_LOAD32_WORD( "0200302v.u11", 0x000002, 0x80000, CRC(29620f05) SHA1(172b6226c443931f0c4ddc44a63c8fc0e6be3824) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASEFF )
ROM_END

ROM_START( diamdove )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b78
	    0x000000-0x063a9f is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x2dfce931
	        Calculated Checksum 0x2dfce931  (OK)
	    0x063aa0-0x273ea3 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x063aa0-0x2fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0101018v.u7",  0x0000000, 0x0080000, CRC(2ebb3704) SHA1(42567d873d6ab9221d09e5449fa57b557677d2ab) )
	ROM_LOAD32_WORD("0101018v.u11", 0x0000002, 0x0080000, CRC(ff4c684a) SHA1(6598c24a8717b8e624e387f000c584ec3b10a8cd) )
	ROM_LOAD32_WORD("0101018v.u8",  0x0100000, 0x0080000, CRC(daa55b3b) SHA1(7aa96a51a3ea9f96c38d08e486eccc54ca4396a3) )
	ROM_LOAD32_WORD("0101018v.u12", 0x0100002, 0x0080000, CRC(62209e81) SHA1(68383068de2e030467c3f3ac16459ae2f3b2cce6) )
	ROM_LOAD32_WORD("0101018v.u9",  0x0200000, 0x0080000, CRC(2254f0e9) SHA1(5bccd65e7e616e1f6ed08a0c84862cb13f9f7098) )
	ROM_LOAD32_WORD("0101018v.u13", 0x0200002, 0x0080000, CRC(952a850f) SHA1(66da391af532f9ef531d10995c96a90eb71cd09a) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( dimtouch )
	ARISTOCRAT_MK5_BIOS
	// checksum code not found (uses different startup sequence)
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0400433v.u7",  0x000000, 0x80000, CRC(71b19365) SHA1(5a8ba1806af544d33e9acbcbbc0555805b4074e6) )
	ROM_LOAD32_WORD( "0400433v.u11", 0x000002, 0x80000, CRC(3d836342) SHA1(b015a4ba998b39ed86cdb6247c9c7f1365641b59) )
	ROM_LOAD32_WORD( "0400433v.u8",  0x100000, 0x80000, CRC(971bbf63) SHA1(082f81115209c7089c76fb207248da3c347a080b) )
	ROM_LOAD32_WORD( "0400433v.u12", 0x100002, 0x80000, CRC(9e0d08e2) SHA1(38b10f7c37f1cefe9271549073dc0a4fed409aec) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASEFF )
ROM_END


ROM_START( dolphntr )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b08
	    0x000000-0x05c367 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x14ccd8a1
	        Calculated Checksum 0x14ccd8a1  (OK)
	    0x05c368-0x0fe787 is the non-Checksummed range (unusual endpoint)
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0200424v.u7",  0x000000, 0x80000, CRC(5dd88306) SHA1(ee8ec7d123d057e8df9be0e8dadecea7dab7aafd) )
	ROM_LOAD32_WORD( "0200424v.u11", 0x000002, 0x80000, CRC(bcb732ea) SHA1(838300914846c6e740780e5a24b9db7304a8a88d) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( dolphntra )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b08
	    0x000000-0x053897 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x81967fa4
	        Calculated Checksum 0x81967fa4  (OK)
	    0x053898-0x1cac2f is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x053898-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0100424v.u7",  0x000000, 0x80000, CRC(657faef7) SHA1(09e1f9d461e855c10cf8b825ef83dd3e7db65b43) )
	ROM_LOAD32_WORD( "0100424v.u11", 0x000002, 0x80000, CRC(65aa46ec) SHA1(3ad4270efbc2e947097d94a3258a544d79a1d599) )
	ROM_LOAD32_WORD( "0100424v.u8",  0x100000, 0x80000, CRC(e77868ad) SHA1(3345da120075bc0da47bac0a4840790693382620) )
	ROM_LOAD32_WORD( "0100424v.u12", 0x100002, 0x80000, CRC(6abd9309) SHA1(c405a13f5bfe447c1ab20d92e140e4fb145920d4) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( dolphntrb )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b20
	    0x000000-0x0536c3 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xeee6e6fc
	        Calculated Checksum 0xeee6e6fc  (OK)
	    0x0536c4-0x1ce293 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x0536c4-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100388v.u7",  0x0000000, 0x0080000, CRC(7463b5f6) SHA1(89e5cf8143d0b4ed54aa2c9bd8840f0aba19322e) )
	ROM_LOAD32_WORD("0100388v.u11", 0x0000002, 0x0080000, CRC(8e391b67) SHA1(4b7a7295d3a96e26bf1958eb30af0b6582a5e5a6) )
	ROM_LOAD32_WORD("0100388v.u8",  0x0100000, 0x0080000, CRC(195bec0f) SHA1(86bdc53e682476c2d90c5e51d4bccdc048d22e7f) )
	ROM_LOAD32_WORD("0100388v.u12", 0x0100002, 0x0080000, CRC(1a1fbbcf) SHA1(6e3772dcccd9b5958bec3bfac9af22b2eabca32e) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( dolphntrce )
	ARISTOCRAT_MK5_BIOS

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("ahg1606.u7",  0x0000000, 0x0080000, CRC(d468edf7) SHA1(100672d09184e06130ce253749bd9e20ee0a06d4) )
	ROM_LOAD32_WORD("ahg1606.u11", 0x0000002, 0x0080000, CRC(0fe64635) SHA1(b504216e59984951b46701019f87cad759ab60f2) )
	ROM_LOAD32_WORD("ahg1606.u8",  0x0100000, 0x0080000, CRC(a53a2de4) SHA1(1741af795f88e867021f3c08d8990611d893a8e8) )
	ROM_LOAD32_WORD("ahg1606.u12", 0x0100002, 0x0080000, CRC(c2e268a2) SHA1(6aeb27ae844dbf495c64be210bcac97f4c7a6969) )
	ROM_LOAD32_WORD("ahg1606.u9",  0x0200000, 0x0080000, CRC(fe9146f7) SHA1(95bef8910d213d588d45ec2639828a31aab6603c) )
	ROM_LOAD32_WORD("ahg1606.u13", 0x0200002, 0x0080000, CRC(d6aa89fe) SHA1(eccb49d49f533aeed9fefb14018bcc06d3fdaf23) )
	ROM_LOAD32_WORD("ahg1606.u10", 0x0300000, 0x0080000, CRC(0be76189) SHA1(a458f620f48b9f4a73f59d31ba98864c5a64e1d7) )
	ROM_LOAD32_WORD("ahg1606.u14", 0x0300002, 0x0080000, CRC(c6c59ed6) SHA1(0ce8e5824c5937ffe2eeb34320db9dc568bca7cb) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


// 602/1 - 10 Credit Multiplier / 9 Line Multiline.
// Dolphin Treasure - Export B - 06/12/96.
// All devices are 27c4002 instead of 27c4096.
ROM_START( dolphntru )
	ARISTOCRAT_MK5_BIOS_HAVE_EEPROMS
	/*
	    Checksum code found at 0x000d08
	    0x000000-0x08ec8b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x9caf255e
	        Calculated Checksum 0x9caf255e  (OK)
	    0x08ec8c-0x13d99f is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x08ec8c-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "fhg407702.u7",  0x000000, 0x80000, CRC(97e3e4d0) SHA1(211b9b9e0f25dfaf9d1dfe1d3d88592522aa6f07) )
	ROM_LOAD32_WORD( "fhg407702.u11", 0x000002, 0x80000, CRC(de221eb5) SHA1(0e550e90b7fd5670f3f3a8589239c342ed70dc3d) )
	ROM_LOAD32_WORD( "fhg407702.u8",  0x100000, 0x80000, CRC(cb3ca8b6) SHA1(dba8bdaa406c07870f95241466359e39a012a70b) )
	ROM_LOAD32_WORD( "fhg407702.u12", 0x100002, 0x80000, CRC(8ee1c2d3) SHA1(e6ecaaac0cb4518ecc0d36532ab532f46e3e628b) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )

	ROM_REGION16_BE( 0x100, "eeprom0", 0 )
	ROM_LOAD16_WORD_SWAP( "eeprom0",      0x000000, 0x000100, CRC(fea8a821) SHA1(c744cac6af7621524fc3a2b0a9a135a32b33c81b) )

	ROM_REGION16_BE( 0x100, "eeprom1", 0 )
	ROM_LOAD16_WORD_SWAP( "eeprom1",      0x000000, 0x000100, CRC(1fc27753) SHA1(7e5008faaf115dc506481430272285117c989d8e) )

	ROM_REGION( 0x80000, "nvram", 0 )
	ROM_LOAD( "nvram",        0x000000, 0x080000, CRC(0063e5ca) SHA1(a3d7b636bc9d792e93d11cb2babf24fbdd6d7776) )

	ROM_REGION( 0x20, "rtc", 0 )
	ROM_LOAD( "rtc",          0x000000, 0x00001f, CRC(6909acb0) SHA1(6a4589599cd1c477e916474e7b029e9a4e92019b) )
ROM_END

ROM_START( drgneye )
	ARISTOCRAT_MK5_BIOS

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100521v.u7",  0x0000000, 0x0080000, CRC(db9c952d) SHA1(4cbe3ffe6cf0bb112cb9a2d7a4ff0b28154d32c1) )
	ROM_LOAD32_WORD("0100521v.u11", 0x0000002, 0x0080000, CRC(2bb47749) SHA1(796f610e5202b5eb26a6e901d43ee5d9e3f95332) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( dynajack )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b78
	    0x000000-0x07031b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xd8815d1c
	        Calculated Checksum 0xd8815d1c  (OK)
	    0x07031c-0x227a4b is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x07031c-0x2fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("01j00081.u7",  0x0000000, 0x0080000, CRC(73783ecf) SHA1(280b4da540b405959f31c2eebbf87ab635d21c06) )
	ROM_LOAD32_WORD("01j00081.u11", 0x0000002, 0x0080000, CRC(5a0147ae) SHA1(f2135b2525eb50a03a8f6360e7edb92bf0b88740) )
	ROM_LOAD32_WORD("01j00081.u8",  0x0100000, 0x0080000, CRC(e686eab2) SHA1(6eb18adda82357ff84f77e9334733094430dfdc6) )
	ROM_LOAD32_WORD("01j00081.u12", 0x0100002, 0x0080000, CRC(beee94ff) SHA1(fad0d3506d10330840d3e5fcdfd7f0aa20041969) )
	ROM_LOAD32_WORD("01j00081.u9",  0x0200000, 0x0080000, CRC(28a45170) SHA1(d7bb8e4dd24e3a3acf44e7fc40e49ebee5c15ec9) )
	ROM_LOAD32_WORD("01j00081.u13", 0x0200002, 0x0080000, CRC(d204ff9c) SHA1(8ac5533928fb3ca247dc85cea67da45a6743f732) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( eldorda5 )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b88
	    0x000000-0x06328b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xed424efa
	        Calculated Checksum 0xed424efa  (OK)
	    0x06328c-0x0d4b57 is the non-Checksummed range (unusual endpoint)
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100652v.u7",  0x0000000, 0x0080000, CRC(d9afe87c) SHA1(577ea5da9c4e93a393711a0c7361365301f4241e) )
	ROM_LOAD32_WORD("0100652v.u11", 0x0000002, 0x0080000, CRC(35233cf8) SHA1(e02477526f2f9e2663c1876f543d138b2caf28df) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( eforsta5 )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000ae4
	    0x000000-0x045da3 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x2c99855f
	        Calculated Checksum 0x2c99855f  (OK)
	    0x045da4-0x0ebd43 is the non-Checksummed range (unusual endpoint)
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0400122v.u7",  0x000000, 0x80000, CRC(b5829b27) SHA1(f6f84c8dc524dcee95e37b93ead9090903bdca4f) )
	ROM_LOAD32_WORD( "0400122v.u11", 0x000002, 0x80000, CRC(7a97adc8) SHA1(b52f7fdc7edf9ad92351154c01b8003c0576ed94) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


// MV4033 - 10 Credit Multiplier / 9 Line Multiline.
// Enchanted Forest - Export B - 10/02/97.
// Marked as 94.97%
// All devices are 27c4002 instead of 27c4096.
ROM_START( eforsta5u )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000d08
	    0x000000-0x0a5233 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x5de71535
	        Calculated Checksum 0x5de71535  (OK)
	    0x0a5234-0x15dbdf is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x0a5234-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/

	// if you enable the additional debug output you get 'Error in graphics EPROMs' so these ROMs are also bad even if the above passes
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "jhg041503.u7",  0x000000, 0x80000, BAD_DUMP CRC(cae1fb55) SHA1(386913ddf9be406f46aab06cf3e27c3c38a4d52d) )  // 94.97%
	ROM_LOAD32_WORD( "jhg041503.u11", 0x000002, 0x80000, BAD_DUMP CRC(a71b7b3c) SHA1(26c3438398b6a3cc9946a1cd1c92d317a8e2738e) )  // 94.97%
	ROM_LOAD32_WORD( "jhg041503.u8",  0x100000, 0x80000, BAD_DUMP CRC(002dec6c) SHA1(fb3f4ce9cd8cd9e0e3133376ed014db83db041c5) )  // base
	ROM_LOAD32_WORD( "jhg041503.u12", 0x100002, 0x80000, BAD_DUMP CRC(c968471f) SHA1(9d54a5c396e6f83690db2fcb7ddcc8a47a7dd777) )  // base

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( fortellr )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b78
	    0x000000-0x07038b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x49e7e64e
	        Calculated Checksum 0x49e7e64e  (OK)
	    0x07038c-0x3616a7 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x07038c-0x3fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("01j00131.u7",  0x0000000, 0x0080000, CRC(78394106) SHA1(aedfb98d7aa515eebabf378edb9c43e01bcba010) )
	ROM_LOAD32_WORD("01j00131.u11", 0x0000002, 0x0080000, CRC(faab1283) SHA1(6200fc2047c4052e4fc3c2d28b26cd9ff67a08be) )
	ROM_LOAD32_WORD("01j00131.u8",  0x0100000, 0x0080000, CRC(7ce4ba38) SHA1(43b57e4dc96851f58d95e4f1b99d08f559e27f6a) )
	ROM_LOAD32_WORD("01j00131.u12", 0x0100002, 0x0080000, CRC(fe5af3ac) SHA1(f08fe353c871ac4375f0fa25bf15f2638b33a370) )
	ROM_LOAD32_WORD("01j00131.u9",  0x0200000, 0x0080000, CRC(a43cd994) SHA1(759fecc809ca1b038d782b173d5638d9be165f9a) )
	ROM_LOAD32_WORD("01j00131.u13", 0x0200002, 0x0080000, CRC(d0dd6627) SHA1(ea855da1759a27936615400993b381609071d66c) )
	ROM_LOAD32_WORD("01j00131.u10", 0x0300000, 0x0080000, CRC(f2790419) SHA1(8720c37cc678e7c5666c67b9998fbb460a8aad90) )
	ROM_LOAD32_WORD("01j00131.u14", 0x0300002, 0x0080000, CRC(507bbe10) SHA1(01b1982c02a00b60aa39ee1b408d653365f728d4) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


// MV4084/1 - 10 Credit Multiplier / 9 Line Multiline.
// THE GAMBLER - Export  A - 30/10/98.
// Marked as EHG0916 and 92.268%.
// All devices are 27c4002 instead of 27c4096.
ROM_START( gambler )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000d08
	    0x000000-0x08f46b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x9eb3c0ef
	        Calculated Checksum 0x9eb3c0ef  (OK)
	    0x08f46c-0x1354cb is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x08f46c-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	// if you enable the additional debug output you get 'Error2 in graphics EPROMs' so these ROMs are also bad even if the above passes
	ROM_LOAD32_WORD( "ehg091602.u7",  0x000000, 0x80000, BAD_DUMP CRC(7524c954) SHA1(0a895d1e2d09a2c873bbbbeb37bc59c25f3c577c) )  // 92.268%
	ROM_LOAD32_WORD( "ehg091602.u11", 0x000002, 0x80000, BAD_DUMP CRC(f29a6932) SHA1(17761218a04d36a599c987b4e13c0e3f46b7793f) )  // 92.268%
	ROM_LOAD32_WORD( "ehg091602.u8",  0x100000, 0x80000, BAD_DUMP CRC(e2221fdf) SHA1(8a7b2d5de68ae66fe1915a6faac6277249e3fb53) )  // base
	ROM_LOAD32_WORD( "ehg091602.u12", 0x100002, 0x80000, BAD_DUMP CRC(ebe957f9) SHA1(539945ec9beafe2c83051208370588fce2334f16) )  // base

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( geisha )
	ARISTOCRAT_MK5_BIOS
	// checksum code not found (uses different startup sequence)
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


ROM_START( genmagi )
	ARISTOCRAT_MK5_BIOS
	// checksum code not found (uses different startup sequence)
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0200894v.u7",  0x0000000, 0x0080000, CRC(20ec3b50) SHA1(400ad7f86077184fee63690060fe2a51ba888e1b) )
	ROM_LOAD32_WORD("0200894v.u11", 0x0000002, 0x0080000, CRC(88c304a3) SHA1(013d5d1d62b356ce5cdf0c9b036c4ca09f191668) )
	ROM_LOAD32_WORD("0200894v.u8",  0x0100000, 0x0080000, CRC(341bac7b) SHA1(67df39b8070f6d9afd183b04239d9e2844d588c5) )
	ROM_LOAD32_WORD("0200894v.u12", 0x0100002, 0x0080000, CRC(44adc422) SHA1(81256ddebb29fbd69cab8e642faac39635dd1739) )
	ROM_LOAD32_WORD("0200894v.u9",  0x0200000, 0x0080000, CRC(ce051dbd) SHA1(433717c5689dc865c1e42669a50e138eae017362) )
	ROM_LOAD32_WORD("0200894v.u13", 0x0200002, 0x0080000, CRC(26f51647) SHA1(e980c021d8e2d295ba2d50446b36b85f42d3f318) )
	ROM_LOAD32_WORD("0200894v.u10", 0x0300000, 0x0080000, CRC(ea460e72) SHA1(4546e04cc04239528c93e22532db08fccebda8a8) )
	ROM_LOAD32_WORD("0200894v.u14", 0x0300002, 0x0080000, CRC(52092ffb) SHA1(6ed591a510e9186588470ec745caf8001712012e) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( gnomeatw )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b68
	    0x000000-0x05ebcb is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xd396114d
	        Calculated Checksum 0xd396114d  (OK)
	    0x05ebcc-0x1bf9db is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x05ebcc-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100767v.u7",  0x0000000, 0x0080000, CRC(a5d3825e) SHA1(4ce7466eff770a2c6c3c5de620a14e05bb9fb406) )
	ROM_LOAD32_WORD("0100767v.u11", 0x0000002, 0x0080000, CRC(737d7178) SHA1(df788eea23b15415adc94543476b6ad982c4d79b) )
	ROM_LOAD32_WORD("0100767v.u8",  0x0100000, 0x0080000, CRC(fe59ec8b) SHA1(b43778b51a0d695c179fa63ce45a47b9f550fb97) )
	ROM_LOAD32_WORD("0100767v.u12", 0x0100002, 0x0080000, CRC(49eb3869) SHA1(d98fe385c667872f26d656a3240f557a70ba924f) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( goldpyr )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000d08
	    0x000000-0x08ec83 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x7c8c2fbf
	        Calculated Checksum 0x7c8c2fbf  (OK)
	    0x08ec84-0x1aca63 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x08ec84-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "ahg120503.u7",  0x000000, 0x80000, CRC(2fbed80c) SHA1(fb0d97cb2be96da37c487fc3aef06c6120efdb46) )
	ROM_LOAD32_WORD( "ahg120503.u11", 0x000002, 0x80000, CRC(ec9c183c) SHA1(e405082ee779c4fee103fb7384469c9d6afbc95b) )
	ROM_LOAD32_WORD( "ahg120503.u8",  0x100000, 0x80000, CRC(3cd7d8e5) SHA1(ae83a7c335564c398330d43295997b8ca547c92d) )
	ROM_LOAD32_WORD( "ahg120503.u12", 0x100002, 0x80000, CRC(8bbf45d0) SHA1(f58f28e7cc4ac225197959566d81973b5aa0e836) )

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
	/*
	    Checksum code found at 0x000d08
	    0x000000-0x08ef13 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xd3126f08
	        Calculated Checksum 0x26ee6f08  (BAD)
	    0x08ef14-0x1aca3b is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x08ef14-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	// the checksum only covers part of the first 2 roms, marked all as BAD_DUMP because it can't be trusted without a full redump.
	ROM_LOAD32_WORD( "ahg1206-99.u7",  0x000000, 0x80000, BAD_DUMP CRC(e6c80f67) SHA1(901cf8f8fd46c1c4a70e1954d2d2d88e7acd07a8) )
	ROM_LOAD32_WORD( "ahg1206-99.u11", 0x000002, 0x80000, BAD_DUMP CRC(3cc221ea) SHA1(a71d16b818110f5b632e996e9f2fcb8be17b2aee) )
	ROM_LOAD32_WORD( "ahg1206-99.u8",  0x100000, 0x80000, BAD_DUMP CRC(df1ffb31) SHA1(1cf9d008b1f8fdb06ba050c97dae79f272c8063c) )
	ROM_LOAD32_WORD( "ahg1206-99.u12", 0x100002, 0x80000, BAD_DUMP CRC(d2c8f786) SHA1(a9efa35c8f2833a2b77f092398ca959d5fe6194e) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( goldpyrb )
	ARISTOCRAT_MK5_BIOS

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	// these are the 'bios' for Casino games (could be moved to a different base set)
	ROM_LOAD32_WORD( "0700474v.u7",  0x000000, 0x80000, CRC(04b7dcbf) SHA1(eded1223336181bb08f9593247f1f79d96278b75) )
	ROM_LOAD32_WORD( "0700474v.u11", 0x000002, 0x80000, CRC(a89ce1b5) SHA1(411b474a111f23ebd834bea5af0bf0cf3926d590) )

	ROM_LOAD32_WORD( "0100878v.u8",  0x100000, 0x80000, CRC(c3184f1c) SHA1(3f808b465175108d48ca5b2560e4546b30a7fd72) )
	ROM_LOAD32_WORD( "0100878v.u12", 0x100002, 0x80000, CRC(acb3de77) SHA1(e0e337d6efbd6ee8e0c0ec2653c3dc0bd5741ff4) )
	ROM_LOAD32_WORD( "0100878v.u9",  0x200000, 0x80000, CRC(0a2f6903) SHA1(11fd913f8c3a677ae07c7ec50548a82c1eaf63ee) )
	ROM_LOAD32_WORD( "0100878v.u13", 0x200002, 0x80000, CRC(0df660be) SHA1(73d370d90655dada34f2b5b2209652632c34a22e) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( goldenra )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b98
	    0x000000-0x068297 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x1cc81433
	        Calculated Checksum 0x1cc81433  (OK)
	    0x068298-0x285abf is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x068298-0x2fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0101164v.u7",  0x0000000, 0x0080000, CRC(2f75d5f7) SHA1(d7f6ecff7cf759d80733b6d3f224caa5128be0b7) )
	ROM_LOAD32_WORD("0101164v.u11", 0x0000002, 0x0080000, CRC(06a871c7) SHA1(95464d74c2295196e367e34efb816acedcd71265) )
	ROM_LOAD32_WORD("0101164v.u8",  0x0100000, 0x0080000, CRC(940eabd7) SHA1(8d41b3fa27c827a7671b095618ac53750e6017f6) )
	ROM_LOAD32_WORD("0101164v.u12", 0x0100002, 0x0080000, CRC(21c4a2d2) SHA1(77a24a5f98aad090223d301919645b5011667c28) )
	ROM_LOAD32_WORD("0101164v.u9",  0x0200000, 0x0080000, CRC(b1cac0e7) SHA1(87f393a75c09e96a7fb893a767edcc81044e4fe3) )
	ROM_LOAD32_WORD("0101164v.u13", 0x0200002, 0x0080000, CRC(8f62ccc5) SHA1(5105313192ab8dfd522b921c70b8b03a8a61ac63) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( incasun )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000bf8
	    0x000000-0x05f56b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x86b74381
	        Calculated Checksum 0x86b74381  (OK)
	    0x05f56c-0x23586f is the non-Checksummed range (unusual endpoint)
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100872v.u7",  0x0000000, 0x0080000, CRC(180e098b) SHA1(48782c46a344dba0aaad407d0d4a432da091b0f5) )
	ROM_LOAD32_WORD("0100872v.u11", 0x0000002, 0x0080000, CRC(f51b411d) SHA1(fbbd587c90cd49bb36653cbd1948bc52f8396a41) )
	ROM_LOAD32_WORD("0100872v.u8",  0x0100000, 0x0080000, CRC(0c19f5ec) SHA1(95d7c9308b30b5193816e95c4276829612040298) )
	ROM_LOAD32_WORD("0100872v.u12", 0x0100002, 0x0080000, CRC(0fa00c41) SHA1(79139834d5437b37346322bf632904c473e3463a) )
	ROM_LOAD32_WORD("0100872v.u9",  0x0200000, 0x0080000, CRC(c82da820) SHA1(98a2710b1f793a7ee1070f89c66d49ce55e4156e) )
	ROM_LOAD32_WORD("0100872v.u13", 0x0200002, 0x0080000, CRC(00407593) SHA1(4c759fe3267b1782ae84d8ed9134295dfaa0faaf) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( incasunu )
	ARISTOCRAT_MK5_BIOS_HAVE_EEPROMS

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("chg1458.u7",  0x0000000, 0x0080000, CRC(20c78b79) SHA1(d7402ff89160f25c9f4f67bbf688621d4ce22205) )
	ROM_LOAD32_WORD("chg1458.u11", 0x0000002, 0x0080000, CRC(12304203) SHA1(eea44382a2711ceb6661949692e5b5a742dd0761) )
	ROM_LOAD32_WORD("chg1458.u8",  0x0100000, 0x0080000, CRC(4618ecd4) SHA1(0ac6bfd6ec2bda5f4d474769f35bc81431f25c2a) )
	ROM_LOAD32_WORD("chg1458.u12", 0x0100002, 0x0080000, CRC(b07d450c) SHA1(432fb4728480b76018b22e971027efb23deb7ff3) )
	ROM_LOAD32_WORD("chg1458.u9",  0x0200000, 0x0080000, CRC(2f909651) SHA1(b4beaebbb20e879a1e23683a9001cbbd2ebf70c4) )
	ROM_LOAD32_WORD("chg1458.u13", 0x0200002, 0x0080000, CRC(2e573a8d) SHA1(aa8ac4f4a427829f0a5929273c618edb4ecf7b36) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )

	ROM_REGION16_BE( 0x100, "eeprom0", 0 )
	ROM_LOAD16_WORD_SWAP( "eeprom0",      0x000000, 0x000100, CRC(fea8a821) SHA1(c744cac6af7621524fc3a2b0a9a135a32b33c81b) )

	ROM_REGION16_BE( 0x100, "eeprom1", 0 )
	ROM_LOAD16_WORD_SWAP( "eeprom1",      0x000000, 0x000100, CRC(b3efdb60) SHA1(f219175019b7237f1e2d132f36803097f2a1d174) )

	ROM_REGION( 0x80000, "nvram", 0 )
	ROM_LOAD( "nvram",        0x000000, 0x080000, CRC(a68e890e) SHA1(8ab087a09cfee8d3e2d84b1003b6798c7223be03) )

	ROM_REGION( 0x20, "rtc", 0 )
	ROM_LOAD( "rtc",          0x000000, 0x00001f, CRC(6909acb0) SHA1(6a4589599cd1c477e916474e7b029e9a4e92019b) )
ROM_END

ROM_START( incasunsp )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	/*
	    Checksum code found at 0x000bf8
	    0x000000-0x05f70f is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x1de6e2c7
	        Calculated Checksum 0x1de6e2c7  (OK)
	    0x05f710-0x235a13 is the non-Checksummed range (unusual endpoint)
	*/
	ROM_LOAD32_WORD("sp__0100872v.u7",  0x0000000, 0x0080000, CRC(62919753) SHA1(0f0d186260a64b8b45671f68abf497586264793e) )
	ROM_LOAD32_WORD("sp__0100872v.u11", 0x0000002, 0x0080000, CRC(f221ac71) SHA1(c2c1f8703e9a41e5c4d5ebfeac57e220a64e9657) )
	ROM_LOAD32_WORD("sp__0100872v.u8",  0x0100000, 0x0080000, CRC(6610599f) SHA1(6d787ae58e2de2b3379a25f394c15434d4e2a8c1) )
	ROM_LOAD32_WORD("sp__0100872v.u12", 0x0100002, 0x0080000, CRC(6633e701) SHA1(02e691c7d18901e70bf8c4e4aa6f856e153f05d4) )
	ROM_LOAD32_WORD("sp__0100872v.u9",  0x0200000, 0x0080000, CRC(b6035aa7) SHA1(e96e802cda6f20caf523203f2032a88488bdfb65) )
	ROM_LOAD32_WORD("sp__0100872v.u13", 0x0200002, 0x0080000, CRC(6d66c6b4) SHA1(2106f2ede58bd4d09334e32a1553f02a154bb767) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( incasunnz )
	ARISTOCRAT_MK5_BIOS
	// checksum code not found (uses different startup sequence)
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0101108v.u7",  0x0000000, 0x0080000, CRC(1e7be5ca) SHA1(333b7665fab8f60fb60e9d3b44de96725763ca17) )
	ROM_LOAD32_WORD("0101108v.u11", 0x0000002, 0x0080000, CRC(2ff86b76) SHA1(c491ca19320bd3e15199b3ca1fcf36a70e386daa) )
	ROM_LOAD32_WORD("0101108v.u8",  0x0100000, 0x0080000, CRC(3eb64fc9) SHA1(31f7d56443091da211c45dddb97375305c3cfeae) )
	ROM_LOAD32_WORD("0101108v.u12", 0x0100002, 0x0080000, CRC(d91114c5) SHA1(fa88c70d81ff5e4df539b873803376e79eb6a479) )
	ROM_LOAD32_WORD("0101108v.u9",  0x0200000, 0x0080000, CRC(6da340db) SHA1(4d7528aa27561185a7d53a0c44a4e95e40aadc26) )
	ROM_LOAD32_WORD("0101108v.u13", 0x0200002, 0x0080000, CRC(472f4097) SHA1(5ebe72b138cdc67989db17c82979eeddc60a081e) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( indrema5 )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000ba8
	    0x000000-0x06323f is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x965e92e4
	        Calculated Checksum 0x965e92e4  (OK)
	    0x063240-0x1cd2d3 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x063240-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0100845v.u7",  0x000000, 0x80000, CRC(0c924a3e) SHA1(499b4ae601e53173e3ba5f400a40e5ae7bbaa043) )
	ROM_LOAD32_WORD( "0100845v.u11", 0x000002, 0x80000, CRC(e371dc0f) SHA1(a01ab7fb63a19c144f2c465ecdfc042695124bdf) )
	ROM_LOAD32_WORD( "0100845v.u8",  0x100000, 0x80000, CRC(1c6bfb47) SHA1(7f751cb499a6185a0ab64eeec511583ceeee6ee8) )
	ROM_LOAD32_WORD( "0100845v.u12", 0x100002, 0x80000, CRC(4bbe67f6) SHA1(928f88387da66697f1de54f086531f600f80a15e) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( jungjuic )
	ARISTOCRAT_MK5_BIOS
	/*
	    note, this actually contains a 2nd checksum for the game, this is likely the base/bios check only.

	    Checksum code found at 0x001b74
	    0x000000-0x089a2f is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x5ad8a58b
	        Calculated Checksum 0x5ad8a58b  (OK)
	    0x089a30-0x1b4043 is the non-Checksummed range (unusual endpoint)

	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	// these are the 'bios' for Casino games (could be moved to a different base set)
	ROM_LOAD32_WORD( "0700474v.u7",  0x000000, 0x80000, CRC(04b7dcbf) SHA1(eded1223336181bb08f9593247f1f79d96278b75) )
	ROM_LOAD32_WORD( "0700474v.u11", 0x000002, 0x80000, CRC(a89ce1b5) SHA1(411b474a111f23ebd834bea5af0bf0cf3926d590) )

	ROM_LOAD32_WORD( "0200240v.u8",  0x100000, 0x80000, CRC(10c61ff7) SHA1(86d17cf2492612c3a6284a1c8e41a67a5199c0eb) )
	ROM_LOAD32_WORD( "0200240v.u12", 0x100002, 0x80000, CRC(ffa3d0ba) SHA1(e60e01d4d425aea483387fa2f9ae5bb69b80f829) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( kgalah )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b28
	    0x000000-0x05af27 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xa4ff4d2a
	        Calculated Checksum 0xa4ff4d2a  (OK)
	    0x05af28-0x1b3e9f is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x05af28-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0200536v.u7",  0x0000000, 0x0080000, CRC(9333543a) SHA1(dbbd59de046c35e70e71836b342eb5ecf4799575) )
	ROM_LOAD32_WORD("0200536v.u11", 0x0000002, 0x0080000, CRC(2b52a5e2) SHA1(0c852c6672a46f269f1407db0dd1825a51f242cc) )
	ROM_LOAD32_WORD("0200536v.u8",  0x0100000, 0x0080000, CRC(08bea3b7) SHA1(9a5d8cf60c9643061dede926a04006a9a674fd8f) )
	ROM_LOAD32_WORD("0200536v.u12", 0x0100002, 0x0080000, CRC(15d5bfb4) SHA1(7c48dabfd83cc30fe2ffd0b4de63fbc9dc56ee2f) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


// MV4137 - 5,10,25,50 Credit Multiplier / 20 Line Multiline.
// Koala Mint [Reel Game] - Export A - 12/09/01.
// Marked as CHG1573.
ROM_START( koalamnt )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000d18
	    0x000000-0x0ec32b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x5e570341
	        Calculated Checksum 0x17df3e7d  (BAD)
	    0x0ec32c-0x34ebdf is the non-Checksummed range (unusual endpoint)
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	// the checksum only covers part of the first 2 roms, marked all as BAD_DUMP because it can't be trusted without a full redump.
	ROM_LOAD32_WORD( "chg1573_koala_mint.u7",  0x000000, 0x80000, BAD_DUMP CRC(fa690af0) SHA1(9e1e5171e9da602c025bfb2aefad397a537794cb) )
	ROM_LOAD32_WORD( "chg1573_koala_mint.u11", 0x000002, 0x80000, BAD_DUMP CRC(c33bed43) SHA1(2c8f35ca08b4d6ac56de5ab7c2515f34e04cf6c8) )
	ROM_LOAD32_WORD( "chg1573_koala_mint.u8",  0x100000, 0x80000, BAD_DUMP CRC(4aeb2e54) SHA1(74002cd12d93352310a864a2ed434c7f43d26534) )  // base
	ROM_LOAD32_WORD( "chg1573_koala_mint.u12", 0x100002, 0x80000, BAD_DUMP CRC(2bf5786f) SHA1(f0693bbd2e6d2e110535205a1ad0b73a0ebd2f53) )  // base
	ROM_LOAD32_WORD( "chg1573_koala_mint.u9",  0x200000, 0x80000, BAD_DUMP CRC(1a2650e7) SHA1(55a8604ef19836880f53d44a035a49b009acbb5a) )  // base
	ROM_LOAD32_WORD( "chg1573_koala_mint.u13", 0x200002, 0x80000, BAD_DUMP CRC(51c78f63) SHA1(ef51e45d67a5684c35150747c186493258cb4549) )  // base
	ROM_LOAD32_WORD( "chg1573_koala_mint.u10", 0x300000, 0x80000, BAD_DUMP CRC(a0fb61fe) SHA1(2a77ed082bc6829905f83a3cb3c4c120fa4ba0f9) )  // base
	ROM_LOAD32_WORD( "chg1573_koala_mint.u14", 0x300002, 0x80000, BAD_DUMP CRC(5e4776e9) SHA1(d44851cbfaa054cd5675a841a3089a8f4fdc8421) )  // base

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( kookabuk )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b68
	    0x000000-0x061857 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xf03ce7cb
	        Calculated Checksum 0xf03ce7cb  (OK)
	    0x061858-0x1a2757 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x061858-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100677v.u7",  0x0000000, 0x0080000, CRC(b2fdf0e8) SHA1(0dd002cfad2fa4f217a0c67066d098f4cd3ba319) )
	ROM_LOAD32_WORD("0100677v.u11", 0x0000002, 0x0080000, CRC(e8ab9afc) SHA1(4c3beefeafc6ac9d4538254bb5e01c12b35db922) )
	ROM_LOAD32_WORD("0100677v.u8",  0x0100000, 0x0080000, CRC(f5a45c57) SHA1(a452a7359af6d5fde2c37946ee68807152f07d39) )
	ROM_LOAD32_WORD("0100677v.u12", 0x0100002, 0x0080000, CRC(b2f2fd15) SHA1(9614f3ae6e82a40ecf44090d0b8d7bd8b6b1f830) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( locoloot )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b20
	    0x000000-0x055e93 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xafd2e94d
	        Calculated Checksum 0xafd2e94d  (OK)
	    0x055e94-0x0bbf23 is the non-Checksummed range (unusual endpoint)
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100472v.u7",  0x0000000, 0x0080000, CRC(4f02763c) SHA1(302cea5fb157f65fc907f123ef42a0a38cc707ac) )
	ROM_LOAD32_WORD("0100472v.u11", 0x0000002, 0x0080000, CRC(21332a1a) SHA1(76a4c30d1c9624984175e9bd117c68c9204f01df) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( locolootnz )
	ARISTOCRAT_MK5_BIOS
	// checksum code not found (uses different startup sequence)
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0600725v.u7",  0x0000000, 0x0080000, CRC(164dd049) SHA1(c99c56af72cb1eb69591cb8f7bacbd06bdb6494d) )
	ROM_LOAD32_WORD("0600725v.u11", 0x0000002, 0x0080000, CRC(93b0bde3) SHA1(06cb79482f8a94e1a504eead9cdf6da41cba1fb9) )
	ROM_LOAD32_WORD("0600725v.u8",  0x0100000, 0x0080000, CRC(8cb449ce) SHA1(2372cf126c2c95d9637b0a761dfc7ea223f0aa54) )
	ROM_LOAD32_WORD("0600725v.u12", 0x0100002, 0x0080000, CRC(29f03505) SHA1(c173167f43cc2eef0e063118e03bc37a87188391) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( lonewolf )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b48
	    0x000000-0x0580f3 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x424e42b6
	        Calculated Checksum 0x424e42b6  (OK)
	    0x0580f4-0x0df6b7 is the non-Checksummed range (unusual endpoint)
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100587v.u7",  0x0000000, 0x0080000, CRC(15024eae) SHA1(7101125aa8531c75f9d80fe357013d09dbb0fec9) )
	ROM_LOAD32_WORD("0100587v.u11", 0x0000002, 0x0080000, CRC(0ed6fb6b) SHA1(a2baa4154fe762e2c1b40a97b2d27265df8b5dab) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


// MV4033 - 10 Credit Multiplier / 9 Line Multiline.
// Magic Garden - Export B - 10/02/97.
// Marked as AHG1211 and 88.26%
ROM_START( mgarden )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000d08
	    0x000000-0x0a522b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x8b0f5dae
	        Calculated Checksum 0x8afcb91f  (BAD)
	    0x0a522c-0x15dbd7 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x0a522c-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	// the checksum only covers part of the first 2 roms, marked all as BAD_DUMP because it can't be trusted without a full redump.
	ROM_LOAD32_WORD( "ahg1211-99.u7",  0x000000, 0x80000, BAD_DUMP CRC(4fe50505) SHA1(6cde87a8a6748af792a1fb101829491367bd4487) )
	ROM_LOAD32_WORD( "ahg1211-99.u11", 0x000002, 0x80000, BAD_DUMP CRC(723ffeee) SHA1(9eab33c9dbf656489914e539a28da5ae289e8df7) )
	ROM_LOAD32_WORD( "ahg1211-99.u8",  0x100000, 0x80000, BAD_DUMP CRC(a315ca28) SHA1(0309789362a945d592ee2eda912e4fc2e6ea5be6) )
	ROM_LOAD32_WORD( "ahg1211-99.u12", 0x100002, 0x80000, BAD_DUMP CRC(4b252c2c) SHA1(8be41fb2b8f8d2829c18ea123a02f3e61c136206) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


// MV4115 - 5,10,20 Credit Multiplier / 9 Line Multiline.
// Magic Mask [Reel Game] - Export A - 09/05/2000.
ROM_START( magimask )
	ARISTOCRAT_MK5_BIOS_HAVE_EEPROMS
	/*
	    Checksum code found at 0x000d18
	    0x000000-0x0e8527 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x1d86deee
	        Calculated Checksum 0x1d86deee  (OK)
	    0x0e8528-0x1e4887 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x0e8528-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "dhg1309.u7",  0x000000, 0x80000, CRC(17317eb9) SHA1(3ddb8d61f23461c3194af534928164550208bbee) )
	ROM_LOAD32_WORD( "dhg1309.u11", 0x000002, 0x80000, CRC(42af4b3f) SHA1(5d88951f77782ff3861b6550ace076662a0b45aa) )
	ROM_LOAD32_WORD( "dhg1309.u8",  0x100000, 0x80000, CRC(23aefb5a) SHA1(ba4488754794f75f53b9c81b74b6ccd992c64acc) )
	ROM_LOAD32_WORD( "dhg1309.u12", 0x100002, 0x80000, CRC(6829a7bf) SHA1(97eed83763d0ec5e753d6ad194e906b1307c4940) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )

	ROM_REGION16_BE( 0x100, "eeprom0", 0 )
	ROM_LOAD16_WORD_SWAP( "eeprom0",      0x000000, 0x000100, CRC(fea8a821) SHA1(c744cac6af7621524fc3a2b0a9a135a32b33c81b) )

	ROM_REGION16_BE( 0x100, "eeprom1", 0 )
	ROM_LOAD16_WORD_SWAP( "eeprom1",      0x000000, 0x000100, CRC(6e485bbc) SHA1(3d6c8d120c69ed2804f267c50681974f73e1ee51) )

	ROM_REGION( 0x80000, "nvram", 0 )
	ROM_LOAD( "nvram",        0x000000, 0x080000, CRC(538c7523) SHA1(1e6516b77daf855e397c1ec590e73637ce3b8406) )

	ROM_REGION( 0x20, "rtc", 0 )
	ROM_LOAD( "rtc",          0x000000, 0x00001f, CRC(6909acb0) SHA1(6a4589599cd1c477e916474e7b029e9a4e92019b) )
ROM_END


// MV4115 - 5,10,20 Credit Multiplier / 9 Line Multiline.
// Magic Mask [Reel Game] - Export A - 09/05/2000.
// Alternate set with identical description, but way different
// than the parent. All devices are 27c4002 instead of 27c4096.
//
// romcmp magicmsk.zip magicmska.zip
// 4 and 4 files
// dhg1309.u12    ahg1548.u12    21.547699%
// dhg1309.u8     ahg1548.u8     21.138954%
// dhg1309.u11    ahg1548.u11    17.786026%
// dhg1309.u7     ahg1548.u7     16.893578%
ROM_START( magimaska )
	ARISTOCRAT_MK5_BIOS_HAVE_EEPROMS
	/*
	    Checksum code found at 0x000d18
	    0x000000-0x0e9597 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x6610851f
	        Calculated Checksum 0x6610851f  (OK)
	    0x0e9598-0x1e591f is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x0e9598-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "ahg1548.u7",  0x000000, 0x80000, CRC(452a19c9) SHA1(aab1f4ccfc6cdb382f7a0e85491614cc58811a08) )
	ROM_LOAD32_WORD( "ahg1548.u11", 0x000002, 0x80000, CRC(c57601f3) SHA1(1616a424b41ad6fea6383a08d5352e8240433374) )
	ROM_LOAD32_WORD( "ahg1548.u8",  0x100000, 0x80000, CRC(607d7447) SHA1(064dbfe8b52eebe1be7a41735da3fa01eacd1686) )
	ROM_LOAD32_WORD( "ahg1548.u12", 0x100002, 0x80000, CRC(cf4cd569) SHA1(408edcd746587d249c4286f7a99f33ad94214f7c) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )

	ROM_REGION16_BE( 0x100, "eeprom0", 0 )
	ROM_LOAD16_WORD_SWAP( "eeprom0",      0x000000, 0x000100, CRC(fea8a821) SHA1(c744cac6af7621524fc3a2b0a9a135a32b33c81b) )

	ROM_REGION16_BE( 0x100, "eeprom1", 0 )
	ROM_LOAD16_WORD_SWAP( "eeprom1",      0x000000, 0x000100, CRC(a10501f9) SHA1(34fdcd16bd7dc474baadc0836e2083abaf589549) )

	ROM_REGION( 0x80000, "nvram", 0 )
	ROM_LOAD( "nvram",       0x000000, 0x080000, CRC(5365446b) SHA1(9ae7a72d0ed3e7f7523a2e0a8f0dc014c6490438) )

	ROM_REGION( 0x20, "rtc", 0 )
	ROM_LOAD( "rtc",          0x000000, 0x00001f, CRC(6909acb0) SHA1(6a4589599cd1c477e916474e7b029e9a4e92019b) )
ROM_END


ROM_START( magtcha5 )
	ARISTOCRAT_MK5_BIOS
	// checksum code not found (uses different startup sequence)
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0200455v.u7",  0x0000000, 0x0080000, CRC(9fa3ee86) SHA1(ce7546b8d1dbf90eb8f4f8d3255dc1c215c966a7) )
	ROM_LOAD32_WORD("0200455v.u11", 0x0000002, 0x0080000, CRC(614984e4) SHA1(e95d576993e8d9c0964899a7d5556c8e62d79242) )
	ROM_LOAD32_WORD("0200455v.u8",  0x0100000, 0x0080000, CRC(d7faf84d) SHA1(d2e49787d177767671fab64a723e1af619ce9ad2) )
	ROM_LOAD32_WORD("0200455v.u12", 0x0100002, 0x0080000, CRC(f54c18db) SHA1(85bcc202f7425b3b7ef456c1c2db5a22648068a8) )
	ROM_LOAD32_WORD("0200455v.u9",  0x0200000, 0x0080000, CRC(0e140453) SHA1(8b516fe598c7e754a471246effa1185845495640) )
	ROM_LOAD32_WORD("0200455v.u13", 0x0200002, 0x0080000, CRC(cfd2a86e) SHA1(66891a1b0e85ad7146b733f4b5d806db789d8821) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( marmagic )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b78
	    0x000000-0x06d93b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x59531d0a
	        Calculated Checksum 0x59531d0a  (OK)
	    0x06d93c-0x2deae3 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x06d93c-0x2fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
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

ROM_START( marmagicua )
	ARISTOCRAT_MK5_BIOS

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "ehg1558.u7",  0x000000, 0x80000, CRC(7c2602ae) SHA1(f5a13154448041575e4bea1399ed0a5a0888d493) )
	ROM_LOAD32_WORD( "ehg1558.u11", 0x000002, 0x80000, CRC(4fe3b18a) SHA1(cc36eddd264de1a088c16e1b741168130d895bd7) )
	ROM_LOAD32_WORD( "ehg1558.u8",  0x100000, 0x80000, CRC(c9ff4bd8) SHA1(5766d7c39e753fb2a2412a41338dcfd4e31b642b) )
	ROM_LOAD32_WORD( "ehg1558.u12", 0x100002, 0x80000, CRC(d00cd217) SHA1(2be7c1ef6c0a39d1c7ed391feeaf0f42a6471bae) )
	ROM_LOAD32_WORD( "ehg1558.u9",  0x200000, 0x80000, CRC(879e4b14) SHA1(52d9540dd3ed51d42a31d1a7effe75e995e95330) )
	ROM_LOAD32_WORD( "ehg1558.u13", 0x200002, 0x80000, CRC(770872a1) SHA1(cd430385a3dfc080decfa2402d7dbad0db912e15) )
	ROM_LOAD32_WORD( "ehg1558.u10", 0x300000, 0x80000, CRC(4301eb40) SHA1(1a32c61df2ab04c365135378d97d18bbb0b50179) )
	ROM_LOAD32_WORD( "ehg1558.u14", 0x300002, 0x80000, CRC(a2096cb3) SHA1(c2bba35396f9c83b17d692a384b6406f3441c8f5) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END



// US003 - Multi credit / Multi line.
// Margarita Magic [Reel Game] - NSW/ACT - A - 07/07/2000.
// EHG1559 - This is a twenty-line game.
// The playlines are 1, 5, 10, 15 and 20.
// For 20 credit per line the max bet is 400
ROM_START( marmagicu )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000d18
	    0x000000-0x0eda53 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xac229593
	        Calculated Checksum 0x67abc369  (BAD)
	    0x0eda54-0x2fffef is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x0eda54-0x2fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	// the checksum only covers part of the first 2 roms, marked all as BAD_DUMP because it can't be trusted without a full redump.
	ROM_LOAD32_WORD( "ehg1559.u7",  0x000000, 0x80000, BAD_DUMP CRC(eab62e8f) SHA1(b125f9a9dc1c82886490d3807e883a7b4e1453a5) )
	ROM_LOAD32_WORD( "ehg1559.u11", 0x000002, 0x80000, BAD_DUMP CRC(0b3c6a11) SHA1(05be4a4d070358600273d5dd4f6b4b37fee47105) )
	ROM_LOAD32_WORD( "ehg1559.u8",  0x100000, 0x80000, BAD_DUMP CRC(db05591e) SHA1(8af241bbd4f744c66fb78fdaf739d9c8bc2580c0) )
	ROM_LOAD32_WORD( "ehg1559.u12", 0x100002, 0x80000, BAD_DUMP CRC(b4458167) SHA1(d1e2040910ad748e58eaccd18ab0569b794b4d97) )
	ROM_LOAD32_WORD( "ehg1559.u9",  0x200000, 0x80000, BAD_DUMP CRC(fc69523a) SHA1(c01b3c905b01671307bc5439d00f4454d0286b20) )
	ROM_LOAD32_WORD( "ehg1559.u13", 0x200002, 0x80000, BAD_DUMP CRC(0cd174df) SHA1(707168fc3bef6c200ae6455c170b7c3e73502965) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


// 559/2 - 10 Credit Multiplier / 9 Line Multiline.
// Mine, Mine, Mine - Export E - 14/02/96.
// All devices are 27c4002 instead of 27c4096.
ROM_START( minemine )
	ARISTOCRAT_MK5_BIOS_HAVE_EEPROMS
	/*
	    Checksum code found at 0x000d10
	    0x000000-0x0a7203 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x75c908a7
	        Calculated Checksum 0x75c908a7  (OK)
	    0x0a7204-0x1a0edf is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x0a7204-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "vhg0416-99.u7",  0x000000, 0x80000, CRC(41bc3714) SHA1(5a8f7d24a6a697524af7997dcedd214fcaf48768) )
	ROM_LOAD32_WORD( "vhg0416-99.u11", 0x000002, 0x80000, CRC(75803b10) SHA1(2ff3d966da2992ddcc7e229d979cc1ee623b4900) )
	ROM_LOAD32_WORD( "vhg0416-99.u8",  0x100000, 0x80000, CRC(0a3e2baf) SHA1(b9ab989cf383cd6ea0aa1ead137558a1a6f5901d) )
	ROM_LOAD32_WORD( "vhg0416-99.u12", 0x100002, 0x80000, CRC(26c01532) SHA1(ec68ad44b703609c7bc27275f8d9250a16d9067c) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )

	ROM_REGION16_BE( 0x100, "eeprom0", 0 )
	ROM_LOAD16_WORD_SWAP( "eeprom0",      0x000000, 0x000100, CRC(fea8a821) SHA1(c744cac6af7621524fc3a2b0a9a135a32b33c81b) )

	ROM_REGION16_BE( 0x100, "eeprom1", 0 )
	ROM_LOAD16_WORD_SWAP( "eeprom1",      0x000000, 0x000100, CRC(8421e7c2) SHA1(fc1b07d5b7aadafc4a0f2e4dfa698e7c72340717) )

	ROM_REGION( 0x80000, "nvram", 0 )
	ROM_LOAD( "nvram",        0x000000, 0x080000, CRC(883f5023) SHA1(e526e337b5b0fc77091b4946b503b56307c390e9) )

	ROM_REGION( 0x20, "rtc", 0 )
	ROM_LOAD( "rtc",          0x000000, 0x00001f, CRC(6909acb0) SHA1(6a4589599cd1c477e916474e7b029e9a4e92019b) )
ROM_END


ROM_START( monmouse )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b80
	    0x000000-0x066077 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x569023a4
	        Calculated Checksum 0x569023a4  (OK)
	    0x066078-0x1faf7b is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x066078-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0400469v.u7",  0x0000000, 0x0080000, CRC(7f7972b6) SHA1(25991f476f55cd1eddc8e63af9c472c1d7e83481) )
	ROM_LOAD32_WORD("0400469v.u11", 0x0000002, 0x0080000, CRC(ac2243ea) SHA1(27c31e5102defa4f3982875b30a67e89af40d4ff) )
	ROM_LOAD32_WORD("0400469v.u8",  0x0100000, 0x0080000, CRC(a10a4bff) SHA1(e6b36542dab8a3405579b333a125a6d3fd801b50) )
	ROM_LOAD32_WORD("0400469v.u12", 0x0100002, 0x0080000, CRC(72d992ed) SHA1(94560305dacbe776ddc95114ad5e5ffaa234937c) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( montree )
	ARISTOCRAT_MK5_BIOS
	// checksum code not found (uses different startup sequence)
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0201397v.u7",  0x0000000, 0x0080000, CRC(982a0078) SHA1(163c15aebd2be623c0f2c7641360336399bc1f4f) )
	ROM_LOAD32_WORD("0201397v.u11", 0x0000002, 0x0080000, CRC(7a03e436) SHA1(016e6e36f7ca6f4c3b427cf98a1415ef6aa57225) )
	ROM_LOAD32_WORD("0201397v.u8",  0x0100000, 0x0080000, CRC(878b6419) SHA1(572d6a10cbf2b96e9afee4b3f32e9ad1ce7eabbb) )
	ROM_LOAD32_WORD("0201397v.u12", 0x0100002, 0x0080000, CRC(b11f51d0) SHA1(1579a24d470f418713334259368c7a4e35e8b5d3) )
	ROM_LOAD32_WORD("0201397v.u9",  0x0200000, 0x0080000, CRC(816fda3f) SHA1(d5eb7572a93e2a4681a2b1db7f302c5528f1c1c3) )
	ROM_LOAD32_WORD("0201397v.u13", 0x0200002, 0x0080000, CRC(e1f23c3f) SHA1(fe74c219c738625257fb62806e271a60075aaa07) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( mountmon )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000ae4
	    0x000000-0x04ee9b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x4bb1139e
	        Calculated Checksum 0x4bb1139e  (OK)
	    0x04ee9c-0x0e3a1f is the non-Checksummed range (unusual endpoint)
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100294v.u7",  0x0000000, 0x0080000, CRC(b84342af) SHA1(e27e65730ddc897b01e8875a4da3ea2d6db2b858) )
	ROM_LOAD32_WORD("0100294v.u11", 0x0000002, 0x0080000, CRC(4fb2a4dc) SHA1(23895b701387f7442a31969989d21cefe2a25efd) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( multidrw )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b98
	    0x000000-0x07477f is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xe2d3d401
	        Calculated Checksum 0xe2d3d401  (OK)
	    0x074780-0x2c5abb is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x074780-0x2fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0200956v.u7",  0x0000000, 0x0080000, CRC(7570eb03) SHA1(0fded55ee2d12cfae96e2910c68a131cd89147a0) )
	ROM_LOAD32_WORD("0200956v.u11", 0x0000002, 0x0080000, CRC(ac8503fa) SHA1(30640a9c01239173c7430a46dcd2e2b28024c0cf) )
	ROM_LOAD32_WORD("0200956v.u8",  0x0100000, 0x0080000, CRC(8c54bd65) SHA1(5870558f8b96fca2c355ccc6ffc09fc4684d141c) )
	ROM_LOAD32_WORD("0200956v.u12", 0x0100002, 0x0080000, CRC(cd0dfdf5) SHA1(7bcf77c1bcd023b4ab08cef329dcf39dc2ca09d6) )
	ROM_LOAD32_WORD("0200956v.u9",  0x0200000, 0x0080000, CRC(10b96156) SHA1(1f89e0d8d210d2fd7e0b78b0205eb626d7c39542) )
	ROM_LOAD32_WORD("0200956v.u13", 0x0200002, 0x0080000, CRC(0d6f7ec5) SHA1(0a80257eb464e50292554f45583f3d7b85de2bc3) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( mystgard )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000ae4
	    0x000000-0x04eea7 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x37310f71
	        Calculated Checksum 0x37310f71  (OK)
	    0x04eea8-0x0dce17 is the non-Checksummed range (unusual endpoint)
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("Mystic Garden.u7",  0x0000000, 0x0080000, CRC(28d15442) SHA1(ee33017f3efcf688a43ea1d7f2b74b4b9a6d2cae) )
	ROM_LOAD32_WORD("Mystic Garden.u11", 0x0000002, 0x0080000, CRC(6e618fc5) SHA1(a02e7ca2433cf8128d74792833d9708a3ba5df4b) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( orchidms )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b20
	    0x000000-0x0677c7 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x522d283f
	        Calculated Checksum 0x522d283f  (OK)
	    0x0677c8-0x13adcb is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x0677c8-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100275v.u7",  0x0000000, 0x0080000, CRC(5d18ae22) SHA1(c10f7a83f51cfe75653ace8066b7dedf07e91b28) )
	ROM_LOAD32_WORD("0100275v.u11", 0x0000002, 0x0080000, CRC(fe79410b) SHA1(c91a0ce0cf87db518f910e9f47cabdcb91dc5496) )
	ROM_LOAD32_WORD("0100275v.u8",  0x0100000, 0x0080000, CRC(09ec43e3) SHA1(947ed0982a148e6906666378e8c82315d40237d7) )
	ROM_LOAD32_WORD("0100275v.u12", 0x0100002, 0x0080000, CRC(165a762d) SHA1(8487d2e32bd2fab5a9114380ba2be6d34b097b11) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( oscara5 )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b80
	    0x000000-0x05d187 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xd11b30fc
	        Calculated Checksum 0xd11b30fc  (OK)
	    0x05d188-0x0e1d73 is the non-Checksummed range (unusual endpoint)
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0200348v.u7",  0x0000000, 0x0080000, CRC(930bdc00) SHA1(36b1a289abebc7cce64e767e201d8f8f7fe80cf2) )
	ROM_LOAD32_WORD("0200348v.u11", 0x0000002, 0x0080000, CRC(11394e80) SHA1(1c6e7e954a6118e04da9d761fef8ec00c46d2af8) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( pantmag )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000d18
	    0x000000-0x06d1ff is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x50122492
	        Calculated Checksum 0x50122492  (OK)
	    0x06d200-0x195d7b is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x06d200-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0101046v.u7",  0x0000000, 0x0080000, CRC(6383899d) SHA1(df96af7cb580565715da6e78b83e7ba6832028e7) )
	ROM_LOAD32_WORD("0101046v.u11", 0x0000002, 0x0080000, CRC(0914594c) SHA1(b1bc1302847e3ea3c4ed96ae17047da031e5ca1a) )
	ROM_LOAD32_WORD("0101046v.u8",  0x0100000, 0x0080000, CRC(db840d1b) SHA1(26ff790cd21f2005ae3a3e879ef07b87c8ae0020) )
	ROM_LOAD32_WORD("0101046v.u12", 0x0100002, 0x0080000, CRC(eae75fa9) SHA1(576c8cf98ad4032bbdde12162e2c1bdd10056762) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( pantmaga )
	ARISTOCRAT_MK5_BIOS

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100716v.u7",  0x0000000, 0x0080000, CRC(8646a4a6) SHA1(1449ba497e4ccebf5de9630bfaf31ad3c583fc44) )
	ROM_LOAD32_WORD("0100716v.u11", 0x0000002, 0x0080000, CRC(61ae2a5a) SHA1(3b11c0f1c6e1464211e6dea958f08d7212ee5756) )
	ROM_LOAD32_WORD("0100716v.u8",  0x0100000, 0x0080000, CRC(91997f98) SHA1(2353abdc0b160cf087230f0c290f37f045a4d07b) )
	ROM_LOAD32_WORD("0100716v.u12", 0x0100002, 0x0080000, CRC(6adfd0ab) SHA1(3b6479bcd95812f5678a27adc7decbc881cd6caa) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END



// MV4115/6 - 9/20 Line Multiline Multiplier.
// Party Gras [Reel Game] - Export A - 10/11/2001.
// All devices are 27c4002 instead of 27c4096.
ROM_START( partygrs )
	ARISTOCRAT_MK5_BIOS_HAVE_EEPROMS
	/*
	    Checksum code found at 0x000d18
	    0x000000-0x0e9b47 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x673ffb0f
	        Calculated Checksum 0x673ffb0f  (OK)
	    0x0e9b48-0x1fd2ab is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x0e9b48-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "ahg1567.u7",  0x000000, 0x80000, CRC(53047385) SHA1(efe50e8785047986513f2de63d2425ba80417481) )
	ROM_LOAD32_WORD( "ahg1567.u11", 0x000002, 0x80000, CRC(f8bd9f7f) SHA1(a8c67a644f9090890e8f33e620fe0bb4633bd6e8) )
	ROM_LOAD32_WORD( "ahg1567.u8",  0x100000, 0x80000, CRC(0b98a0fa) SHA1(c9ada21e39472f28cd9b8ec19be7235410ad3e7a) )
	ROM_LOAD32_WORD( "ahg1567.u12", 0x100002, 0x80000, CRC(00d1395c) SHA1(d9a66d6cdb5aa4f583d8c23306b1416646cbde93) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )

	ROM_REGION16_BE( 0x100, "eeprom0", 0 )
	ROM_LOAD16_WORD_SWAP( "eeprom0",      0x000000, 0x000100, CRC(fea8a821) SHA1(c744cac6af7621524fc3a2b0a9a135a32b33c81b) )

	ROM_REGION16_BE( 0x100, "eeprom1", 0 )
	ROM_LOAD16_WORD_SWAP( "eeprom1",      0x000000, 0x000100, CRC(a10501f9) SHA1(34fdcd16bd7dc474baadc0836e2083abaf589549) )

	ROM_REGION( 0x80000, "nvram", 0 )
	ROM_LOAD( "nvram",        0x000000, 0x080000, CRC(fec1b1df) SHA1(5981e2961692d4c8633afea4ecb4828eabba65bd) )

	ROM_REGION( 0x20, "rtc", 0 )
	ROM_LOAD( "rtc",          0x000000, 0x00001f, CRC(6909acb0) SHA1(6a4589599cd1c477e916474e7b029e9a4e92019b) )
ROM_END


// MV4115/3 - 20 Line Multiline / 3,5,10,20,25,50 Credit Multiplier.
// Party Gras - Export  B - 06/02/2001.
// Marked as BHG1284 and 'touch'.
ROM_START( partygrsa )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000d18
	    0x000000-0x0a69d3 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xf4a004d3
	        Calculated Checksum 0x221d04d3  (BAD)
	    0x0a69d4-0x1b953f is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x0a69d4-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	// the checksum only covers part of the first 2 roms, marked all as BAD_DUMP because it can't be trusted without a full redump.
	ROM_LOAD32_WORD( "bhg1284.u7",  0x000000, 0x80000, BAD_DUMP CRC(02ed0631) SHA1(ae2c89c876a030d325ec94490d293deba772630e) )
	ROM_LOAD32_WORD( "bhg1284.u11", 0x000002, 0x80000, BAD_DUMP CRC(7ac80cd9) SHA1(70e910784a1e1ea8820005082e76223a85a3c346) )
	ROM_LOAD32_WORD( "bhg1284.u8",  0x100000, 0x80000, BAD_DUMP CRC(28774b9a) SHA1(ebdd738a73ffa7c5238640f4d7956751f7bb6243) )
	ROM_LOAD32_WORD( "bhg1284.u12", 0x100002, 0x80000, BAD_DUMP CRC(942835c1) SHA1(fefc509311716559ac6b836a56b2c981907d499b) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( peaflut )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b98
	    0x000000-0x0638d3 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x2ce2619f
	        Calculated Checksum 0x2ce2619f  (OK)
	    0x0638d4-0x1dbf8b is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x0638d4-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("02j00011.u7",  0x0000000, 0x0080000, CRC(e4497f35) SHA1(7030aba6c17fc391564385f5669e07edc94dca61) )
	ROM_LOAD32_WORD("02j00011.u11", 0x0000002, 0x0080000, CRC(3134818c) SHA1(6fe158608b5da648fafd20cbcd213e6f2dc2104c) )
	ROM_LOAD32_WORD("02j00011.u8",  0x0100000, 0x0080000, CRC(f239ca62) SHA1(53e3e2a4d62ceb9e921606e3670470c09e82118f) )
	ROM_LOAD32_WORD("02j00011.u12", 0x0100002, 0x0080000, CRC(2d96c449) SHA1(af98a864b9ed3f95227fd0d6edc6a38c0544c93f) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( pengpay )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b68
	    0x000000-0x05c71f is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x68980cb3
	        Calculated Checksum 0x68980cb3  (OK)
	    0x05c720-0x1aefcf is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x05c720-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0200460v.u7",  0x0000000, 0x0080000, CRC(47145744) SHA1(74a186a15537d8b05ce23f37c53f351e8058b0b2) )
	ROM_LOAD32_WORD("0200460v.u11", 0x0000002, 0x0080000, CRC(82fc4e23) SHA1(54e7698c4deed7202da8f178698ecdcf85f3f640) )
	ROM_LOAD32_WORD("0200460v.u8",  0x0100000, 0x0080000, CRC(8d37d7bf) SHA1(9c9b86cce9492f9de346e5a6944e2f0c5da6b9b1) )
	ROM_LOAD32_WORD("0200460v.u12", 0x0100002, 0x0080000, CRC(90864742) SHA1(f6491e4fbce5d642b9d0224118923b56625338b1) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( pengpaya )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b60
	    0x000000-0x05644f is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x5bc8a3d6
	        Calculated Checksum 0x5bc8a3d6  (OK)
	    0x056450-0x1c19f3 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x056450-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0200357v.u7",  0x0000000, 0x0080000, CRC(cb21de26) SHA1(5a730f08db4d91b18f0b5a1f489f1d982b08edcc) )
	ROM_LOAD32_WORD("0200357v.u11", 0x0000002, 0x0080000, CRC(7dd73770) SHA1(14a2edf8cd33280464f979976486e1a9ae73cef5) )
	ROM_LOAD32_WORD("0200357v.u8",  0x0100000, 0x0080000, CRC(aa95406b) SHA1(fbec024dd210757a79a5ea4def79f88fed971e71) )
	ROM_LOAD32_WORD("0200357v.u12", 0x0100002, 0x0080000, CRC(123cbe90) SHA1(b51b84f79e0822a1be71485bbad514ab9fa55622) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( pengpayb )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b68
	    0x000000-0x05d7b7 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xd4f9ba59
	        Calculated Checksum 0xd4f9ba59  (OK)
	    0x05d7b8-0x1c9acf is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x05d7b8-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0200359v.u7",  0x0000000, 0x0080000, CRC(f51c4e02) SHA1(fca30b3ce0d063966df1e878338596d050664695) )
	ROM_LOAD32_WORD("0200359v.u11", 0x0000002, 0x0080000, CRC(c0f20ef7) SHA1(4df3aa337a2d4dd8ef29f4839b003c96fe1df526) )
	ROM_LOAD32_WORD("0200359v.u8" , 0x0100000, 0x0080000, CRC(23ea514d) SHA1(bcb83a8d768b078a03260a00fa09a4e2350c568c) )
	ROM_LOAD32_WORD("0200359v.u12", 0x0100002, 0x0080000, CRC(d2882682) SHA1(dd42edca8ef9d28dd5b16fe8132f8e0fb3c85979) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


// 586/7(b) - 10 Credit Multiplier / 9 Line Multiline.
// Penguin Pays - Export B - 14/07/97.
// All devices are 27c4002 instead of 27c4096.
ROM_START( pengpayu )
	ARISTOCRAT_MK5_BIOS_HAVE_EEPROMS
	/*
	    Checksum code found at 0x000d08
	    0x000000-0x0cd21b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x7dc52ffa
	        Calculated Checksum 0x7dc52ffa  (OK)
	    0x0cd21c-0x192ed7 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x0cd21c-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "bhi041703.u7",  0x000000, 0x80000, CRC(19d75260) SHA1(798472b1b5d8f5ca99d8bfe57e99a76686f0aa3f) )
	ROM_LOAD32_WORD( "bhi041703.u11", 0x000002, 0x80000, CRC(2b010813) SHA1(a383997308881a3ac35de56fe10e3852fa89fdf6) )
	ROM_LOAD32_WORD( "bhi041703.u8",  0x100000, 0x80000, CRC(6aeaebc8) SHA1(6f70b14e9f4e9940512bd6e89bc9ccbfe1f4a81f) )
	ROM_LOAD32_WORD( "bhi041703.u12", 0x100002, 0x80000, CRC(d959a048) SHA1(92f69090d599f95b48e79213e5b7d486e083d8f4) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )

	ROM_REGION16_BE( 0x100, "eeprom0", 0 )
	ROM_LOAD16_WORD_SWAP( "eeprom0",      0x000000, 0x000100, CRC(fea8a821) SHA1(c744cac6af7621524fc3a2b0a9a135a32b33c81b) )

	ROM_REGION16_BE( 0x100, "eeprom1", 0 )
	ROM_LOAD16_WORD_SWAP( "eeprom1",      0x000000, 0x000100, CRC(8421e7c2) SHA1(fc1b07d5b7aadafc4a0f2e4dfa698e7c72340717) )

	ROM_REGION( 0x80000, "nvram", 0 )
	ROM_LOAD( "nvram",        0x000000, 0x080000, CRC(4e5b9702) SHA1(b2b645db80c4ece24fae8ce6fb660e77ac8e5810) )

	ROM_REGION( 0x20, "rtc", 0 )
	ROM_LOAD( "rtc",          0x000000, 0x00001f, CRC(6909acb0) SHA1(6a4589599cd1c477e916474e7b029e9a4e92019b) )
ROM_END


ROM_START( petshopa )
	ARISTOCRAT_MK5_BIOS
	// checksum code not found (due to bad rom)
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100679v.u7",  0x0000000, 0x0080000, CRC(cf4a24fa) SHA1(b510de9199d16ba7319e1b692d7c6c09fcb735dc) )
	ROM_LOAD32_WORD("0100679v.u11", 0x0000002, 0x007fffd, BAD_DUMP CRC(bfaa9216) SHA1(19f1c7de05ff7f5f9f370be00cf8f0635e966809) ) // wrong size!
	ROM_LOAD32_WORD("0100679v.u8",  0x0100000, 0x0080000, CRC(bb9f7519) SHA1(fa311f1ec74c3b52e2feed36d7b7dc6a12336abe) )
	ROM_LOAD32_WORD("0100679v.u12", 0x0100002, 0x0080000, CRC(2cd12986) SHA1(b6b0bd6dd8c964498edc3763cb5c450795042a8d) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( petshop )
	ARISTOCRAT_MK5_BIOS

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100731v.u7",  0x0000000, 0x0080000, CRC(01cffccc) SHA1(a39d943e700fff34d82bcff8c61f2586ee65e673) )
	ROM_LOAD32_WORD("0100731v.u11", 0x0000002, 0x0080000, CRC(a8e906c5) SHA1(f6dd7bcf5fa90933c9741699f0c1e07b685ccb40) )
	ROM_LOAD32_WORD("0100731v.u8",  0x0100000, 0x0080000, CRC(757e1296) SHA1(e14508bbaa3439a93c8b716267a2198ed3c54728) )
	ROM_LOAD32_WORD("0100731v.u12", 0x0100002, 0x0080000, CRC(6e74cd57) SHA1(9092e656cbd8627b208b81ca0d737483a779bce1) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( phantpay )
	ARISTOCRAT_MK5_BIOS

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0500005v.u7",  0x0000000, 0x0080000, CRC(2cfc44a7) SHA1(a2a93047311d7a1f45e2915478ba2a11d5179194) )
	ROM_LOAD32_WORD("0500005v.u11", 0x0000002, 0x0080000, CRC(3e91ed2a) SHA1(92d49bd78d329ad53cb2063af2d324eada3f53d1) )
	ROM_LOAD32_WORD("0500005v.u8",  0x0100000, 0x0080000, CRC(ab1e77e9) SHA1(5a8da1210214ccc89dfde2e28f5142036a743172) )
	ROM_LOAD32_WORD("0500005v.u12", 0x0100002, 0x0080000, CRC(d43a092a) SHA1(5f851bd179b14ef3983b460ed932810f3713d3e5) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( penpir2 )
	ARISTOCRAT_MK5_BIOS

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	// these are the 'bios' for Casino games (could be moved to a different base set)
	ROM_LOAD32_WORD( "0700474v.u7",  0x000000, 0x80000, CRC(04b7dcbf) SHA1(eded1223336181bb08f9593247f1f79d96278b75) )
	ROM_LOAD32_WORD( "0700474v.u11", 0x000002, 0x80000, CRC(a89ce1b5) SHA1(411b474a111f23ebd834bea5af0bf0cf3926d590) )

	ROM_LOAD32_WORD( "0100869v.u8",  0x100000, 0x80000, CRC(5a87f637) SHA1(0fee8140637e9f923727e8c358c9f59b8319855d) )
	ROM_LOAD32_WORD( "0100869v.u12", 0x100002, 0x80000, CRC(2aef04c1) SHA1(7415f436960c7b4a43634161ca317b2ae34ee745) )
	ROM_LOAD32_WORD( "0100869v.u9",  0x200000, 0x80000, CRC(05de2653) SHA1(7d3f9d50013d8137cef285940b04209cfdae4a1d) )
	ROM_LOAD32_WORD( "0100869v.u13", 0x200002, 0x80000, CRC(e1dbfd58) SHA1(687b7254279734e1835e1713d032b5aa2cf70812) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( przfight )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b48
	    0x000000-0x053def is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x97c4e600
	        Calculated Checksum 0x97c4e600  (OK)
	    0x053df0-0x2a9f7f is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x053df0-0x2fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100299v.u7",  0x0000000, 0x0080000, CRC(2b1a9678) SHA1(c75de4c76cd934df746040d0515694d92e2fc145) )
	ROM_LOAD32_WORD("0100299v.u11", 0x0000002, 0x0080000, CRC(e1bf20d7) SHA1(bcc308b884433b3ebd890fafa667235a9fb7876c) )
	ROM_LOAD32_WORD("0100299v.u8",  0x0100000, 0x0080000, CRC(92b68d43) SHA1(74ba55d6c7016de26692138d194f57f016feb938) )
	ROM_LOAD32_WORD("0100299v.u12", 0x0100002, 0x0080000, CRC(b4797555) SHA1(695aa6c40145fd9856821288680a24d316b7d4cd) )
	ROM_LOAD32_WORD("0100299v.u9",  0x0200000, 0x0080000, CRC(b3163b0c) SHA1(e9aac4acb31a9af194626b25517aa7c169fe40bf) )
	ROM_LOAD32_WORD("0100299v.u13", 0x0200002, 0x0080000, CRC(c16197d5) SHA1(716c4afdf2acde10ff09ad90b03bc5e689f0a737) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( qcash )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000af4
	    0x000000-0x05d55b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x10b06e83
	        Calculated Checksum 0x10b06e83  (OK)
	    0x05d55c-0x1a669f is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x05d55c-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100706v.u7",  0x0000000, 0x0080000, CRC(591c96eb) SHA1(acd6f02206086d710a92401c618f715b3646d78a) )
	ROM_LOAD32_WORD("0100706v.u11", 0x0000002, 0x0080000, CRC(5001567e) SHA1(eadde9750856a7920e06955adc0db46082da655a) )
	ROM_LOAD32_WORD("0100706v.u8",  0x0100000, 0x0080000, CRC(31ed5795) SHA1(8238da7c87195339d34cf24b3e0a7f3bf53d2b8a) )
	ROM_LOAD32_WORD("0100706v.u12", 0x0100002, 0x0080000, CRC(bfedb3fc) SHA1(e115db94b8ee7babb29e31e64b96d181f5c6491b) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( qnile )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b80
	    0x000000-0x055c83 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x53fa5304
	        Calculated Checksum 0x53fa5304  (OK)
	    0x055c84-0x16745b is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x055c84-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100439v.u7",  0x0000000, 0x0080000, CRC(f359afcf) SHA1(a8cbaea899f0108a179c58ec97241a57227afa79) )
	ROM_LOAD32_WORD("0100439v.u11", 0x0000002, 0x0080000, CRC(ca4fe491) SHA1(2bd799f95c9a5afb7c96305bf56413ba864a26dd) )
	ROM_LOAD32_WORD("0100439v.u8",  0x0100000, 0x0080000, CRC(80efde3a) SHA1(1fac1b150c5c8c52a4caaa01c4571a0e7033278d) )
	ROM_LOAD32_WORD("0100439v.u12", 0x0100002, 0x0080000, CRC(bdcec4eb) SHA1(ef3658460263cd2e68e10015efdc016ad705213e) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( qnilea )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b80
	    0x000000-0x064c4b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xa60cbcfa
	        Calculated Checksum 0xa60cbcfa  (OK)
	    0x064c4c-0x172a17 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x064c4c-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0300440v.u7",  0x0000000, 0x0080000, CRC(0076da68) SHA1(ed301c102e88d5b637144ed32042da46780e5b34) )
	ROM_LOAD32_WORD("0300440v.u11", 0x0000002, 0x0080000, CRC(b5b76fb0) SHA1(40cb57e168f7884d64f6779e4e3b532c69df63b8) )
	ROM_LOAD32_WORD("0300440v.u8",  0x0100000, 0x0080000, CRC(a6b856a2) SHA1(2a9ea01f64fa56dea86b0cd25e19dace34c17d0f) )
	ROM_LOAD32_WORD("0300440v.u12", 0x0100002, 0x0080000, CRC(52bd3694) SHA1(bcfa3054c7577f7a1653b756828d048a5f1776e7) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( qnileb )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b80
	    0x000000-0x059dff is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xa63a9b3e
	        Calculated Checksum 0xa63a9b3e  (OK)
	    0x059e00-0x16b5d7 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x059e00-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0200439v.u7",  0x000000, 0x80000, CRC(d476a893) SHA1(186d6fb1830c33976f2d3c96e4f045ece885dc63) )
	ROM_LOAD32_WORD( "0200439v.u11", 0x000002, 0x80000, CRC(8b0d7205) SHA1(ffa03f1c9332a1a7443eb91b0ded56e7cd9e3cee) )
	ROM_LOAD32_WORD( "0200439v.u8",  0x100000, 0x80000, CRC(9b996ef1) SHA1(72489e9a0ee5c34f7cad3d121bcd08e09ef72360) )
	ROM_LOAD32_WORD( "0200439v.u12", 0x100002, 0x80000, CRC(2a0f7feb) SHA1(27c89dadf759e6c892121650758c44ec50990cb6) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( qnilec )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b80
	    0x000000-0x062913 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x2d52d80f
	        Calculated Checksum 0x2d52d80f  (OK)
	    0x062914-0x1740eb is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x062914-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0300439v.u7",  0x000000, 0x80000, CRC(63f9129e) SHA1(a513fd47d3ca4fe007730a06e5f6ffc2891dc74f) )
	ROM_LOAD32_WORD( "0300439v.u11", 0x000002, 0x80000, CRC(7217c3af) SHA1(518c3d79758e3253f937cf73da9398fa812bf4bc) )
	ROM_LOAD32_WORD( "0300439v.u8",  0x100000, 0x80000, CRC(90c92bf8) SHA1(bbc558ffb5a883c9f4ff9dc3362c4081990c970d) )
	ROM_LOAD32_WORD( "0300439v.u12", 0x100002, 0x80000, CRC(eec01bb4) SHA1(146fdce6b32a21659dc775e4a5f3bb027bd09825) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


// MV4091 - 10 Credit Multiplier / 9 Line Multiline.
// QUEEN OF THE NILE - NSW/ACT  B - 13/05/97.
// Marked as GHG409102
// All devices are 27c4002 instead of 27c4096.
// Even when it's a NSW/ACT, the program seems to be for US-Export platforms...
ROM_START( qnileu )
	ARISTOCRAT_MK5_BIOS_HAVE_EEPROMS
	/*
	    Checksum code found at 0x000d08
	    0x000000-0x08ec87 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xb2ac33b8
	        Calculated Checksum 0xb2ac33b8  (OK)
	    0x08ec88-0x1aca67 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x08ec88-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "ghg409102.u7",  0x000000, 0x80000, CRC(a00ab2cf) SHA1(eb3120fe4b1d0554c224c7646e727e86fd35975e) )
	ROM_LOAD32_WORD( "ghg409102.u11", 0x000002, 0x80000, CRC(c4a35337) SHA1(d469ed154caed1f0a4cf89e67d852924c95172ed) )
	ROM_LOAD32_WORD( "ghg409102.u8",  0x100000, 0x80000, CRC(16a629e1) SHA1(0dee11a2f1b2068a86b3e0b6c01d115555a657c9) )
	ROM_LOAD32_WORD( "ghg409102.u12", 0x100002, 0x80000, CRC(7871a846) SHA1(ac1d741092afda842e1864f1a7a14137a9ee46d9) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )

	ROM_REGION16_BE( 0x100, "eeprom0", 0 )
	ROM_LOAD16_WORD_SWAP( "eeprom0",      0x000000, 0x000100, CRC(fea8a821) SHA1(c744cac6af7621524fc3a2b0a9a135a32b33c81b) )

	ROM_REGION16_BE( 0x100, "eeprom1", 0 )
	ROM_LOAD16_WORD_SWAP( "eeprom1",      0x000000, 0x000100, CRC(1fc27753) SHA1(7e5008faaf115dc506481430272285117c989d8e) )

	ROM_REGION( 0x80000, "nvram", 0 )
	ROM_LOAD( "nvram",        0x000000, 0x080000, CRC(5a7bb53a) SHA1(cdac900925d0ee8f98209a377b9f8760de0c2883) )

	ROM_REGION( 0x20, "rtc", 0 )
	ROM_LOAD( "rtc",          0x000000, 0x00001f, CRC(6909acb0) SHA1(6a4589599cd1c477e916474e7b029e9a4e92019b) )
ROM_END


ROM_START( qnilemax )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000bb8
	    0x000000-0x06fd6f is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xcd901653
	        Calculated Checksum 0xcd901653  (OK)
	    0x06fd70-0x3864c7 is the non-Checksummed range (unusual endpoint)
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0401072v.u7",  0x0000000, 0x0080000, CRC(4ac2a82e) SHA1(3fc50e97ad48c57e21a37fbb6142152c72055ad4) )
	ROM_LOAD32_WORD("0401072v.u11", 0x0000002, 0x0080000, CRC(6ae3872f) SHA1(f8c1b31e4ebd4833dcc2b7cfff25f6473ad78f4e) )
	ROM_LOAD32_WORD("0401072v.u8",  0x0100000, 0x0080000, CRC(abbbf1de) SHA1(5efd88213180846ad8347e017e5ccee5b80b95d0) )
	ROM_LOAD32_WORD("0401072v.u12", 0x0100002, 0x0080000, CRC(65cea496) SHA1(4b827e2707c259717bf759e76dca1c96efada926) )
	ROM_LOAD32_WORD("0401072v.u9",  0x0200000, 0x0080000, CRC(750150eb) SHA1(a9ffe0b0bb2ef83a696fa568b0264d27bc650120) )
	ROM_LOAD32_WORD("0401072v.u13", 0x0200002, 0x0080000, CRC(1b77bbd0) SHA1(c93d2f844032631d9594d02fa6ac41e21025a8ea) )
	ROM_LOAD32_WORD("0401072v.u10", 0x0300000, 0x0080000, CRC(8491dbc4) SHA1(f0d4e470f0774a6aac168334390c116fd3d1075e) )
	ROM_LOAD32_WORD("0401072v.u14", 0x0300002, 0x0080000, CRC(4fbccf72) SHA1(97d6bb400caf78cb673a324d48d2580f1bbb1acd) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( qtbird )
	ARISTOCRAT_MK5_BIOS

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0500009v.u7",  0x0000000, 0x0080000, CRC(f294fc0a) SHA1(f3d60ca6008445f535fce027f5ec3fe82ae552c3) )
	ROM_LOAD32_WORD("0500009v.u11", 0x0000002, 0x0080000, CRC(328b7e04) SHA1(5c49f60b7c88d6e94e7ab464fad4eee6806f327a) )
	ROM_LOAD32_WORD("0500009v.u8",  0x0100000, 0x0080000, CRC(764b5568) SHA1(a097992499044b7ca017a8c85387dc1ea94ff27a) )
	ROM_LOAD32_WORD("0500009v.u12", 0x0100002, 0x0080000, CRC(bb8344a9) SHA1(8b0e904b937c7f34470ad946076240b0c54bf434) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( kgbirdmk5 )
	ARISTOCRAT_MK5_BIOS

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0200024v.u7",  0x000000, 0x080000, CRC(90aefddc) SHA1(610b850c1d3e882c4df9e0a09a056b0c97341a19) )
	ROM_LOAD32_WORD( "0200024v.u11", 0x000002, 0x080000, CRC(52791ad8) SHA1(6e4cf553b355f03ef69ef3c4e2816bbd0cbe6599) )
	ROM_LOAD32_WORD( "0200024v.u8",  0x100000, 0x080000, CRC(c0477ae3) SHA1(5005944b8b28553dd959192d614be7f1b6228a30) )
	ROM_LOAD32_WORD( "0200024v.u12", 0x100002, 0x080000, CRC(df176c5a) SHA1(dcaecdefb7c880b9425a6445dbed969968fe3d1c) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( rainwrce )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000bf8
	    0x000000-0x06bb13 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x6c1aaee7
	        Calculated Checksum 0x6c1aaee7  (OK)
	    0x06bb14-0x367863 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x06bb14-0x3fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0101332v.u7",  0x0000000, 0x0080000, CRC(68d9bf78) SHA1(6170ea26ebc732abbc26ba1da35a081c8aa8d154) )
	ROM_LOAD32_WORD("0101332v.u11", 0x0000002, 0x0080000, CRC(4170c68d) SHA1(bc00af27bcc176f8d9c9fd0ec1a7139e28f85113) )
	ROM_LOAD32_WORD("0101332v.u8",  0x0100000, 0x0080000, CRC(98ebea6f) SHA1(2d78cec777581a87bb4b84e7acd183b237c83e52) )
	ROM_LOAD32_WORD("0101332v.u12", 0x0100002, 0x0080000, CRC(b8afd281) SHA1(2d73b5af667d36e8b29e9fc3cc62f220daeffbb9) )
	ROM_LOAD32_WORD("0101332v.u9",  0x0200000, 0x0080000, CRC(eb7d7af6) SHA1(a11e8029b0d5ef9bb8c51fea4e9f0a051cdb2eaf) )
	ROM_LOAD32_WORD("0101332v.u13", 0x0200002, 0x0080000, CRC(36debb0e) SHA1(4aaa495f74dfb13aa1dc47f3a8af8e54496c1ab8) )
	ROM_LOAD32_WORD("0101332v.u10", 0x0300000, 0x0080000, CRC(39f5861f) SHA1(c614ebe2c324d5c3fff32379300f2869fba49d39) )
	ROM_LOAD32_WORD("0101332v.u14", 0x0300002, 0x0080000, CRC(92274626) SHA1(fae8d89efba9bf3d171bfe484015d009786ce40d) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( toutango )
	ARISTOCRAT_MK5_BIOS

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0100782v.u7",  0x000000, 0x080000, CRC(4c70120f) SHA1(e43b39c31c14d16ebf962d8dd201a882df74f595) )
	ROM_LOAD32_WORD( "0100782v.u11", 0x000002, 0x080000, CRC(18519789) SHA1(95385207be6e44746b5e78aa5622afb5258419b2) )
	ROM_LOAD32_WORD( "0100782v.u8",  0x100000, 0x080000, CRC(bf358a6f) SHA1(3ae3bcd486f9c6f5f2a799ed3e4f7b177a59465b) )
	ROM_LOAD32_WORD( "0100782v.u12", 0x100002, 0x080000, CRC(fd366efa) SHA1(22a372f5efe43b9320199b7534e9b3a39b582e4a) )
	ROM_LOAD32_WORD( "0100782v.u9",  0x200000, 0x080000, CRC(bc35aed0) SHA1(7ab25c3207c2be43cfefabe4d4bb0a98bc8e5aea) )
	ROM_LOAD32_WORD( "0100782v.u13", 0x200002, 0x080000, CRC(f8a67a69) SHA1(b1a28047cb4572ae15359c30f71cafa4bd70658c) )
	ROM_LOAD32_WORD( "0100782v.u10", 0x300000, 0x080000, CRC(e6528de7) SHA1(b3aa1937f0b673ba2cfa68acc7cb540ebefc66d4) )
	ROM_LOAD32_WORD( "0100782v.u14", 0x300002, 0x080000, CRC(69f2acde) SHA1(cda52548e675a06677a2d9fee89b33f9abb96f64) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( reelrock )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000ba8
	    0x000000-0x062f6f is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x67b49a57
	        Calculated Checksum 0x67b49a57  (OK)
	    0x062f70-0x1a752b is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x062f70-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0100779v.u7",  0x000000, 0x80000, CRC(b60af34f) SHA1(1143380b765db234b3871c0fe04736472fde7de4) )
	ROM_LOAD32_WORD( "0100779v.u11", 0x000002, 0x80000, CRC(57e341d0) SHA1(9b0d50763bb74ca5fe404c9cd526633721cf6677) )
	ROM_LOAD32_WORD( "0100779v.u8",  0x100000, 0x80000, CRC(57eec667) SHA1(5f3888d75f48b6148f451d7ebb7f99e1a0939f3c) )
	ROM_LOAD32_WORD( "0100779v.u12", 0x100002, 0x80000, CRC(4ac20679) SHA1(0ac732ffe6a33806e4a06e87ec875a3e1314e06b) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( retrsam )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b88
	    0x000000-0x06445b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xb6820a81
	        Calculated Checksum 0xb6820a81  (OK)
	    0x06445c-0x10203b is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x06445c-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("v0400549v.u7", 0x0000000, 0x0080000, CRC(129be82c) SHA1(487639b7d42d6d35a9c48b44d26667c269b5b633) )
	ROM_LOAD32_WORD("0400549v.u11", 0x0000002, 0x0080000, CRC(b91f5d4c) SHA1(8116166a759405b97797b4acb2cc3e139bd12de7) )
	ROM_LOAD32_WORD("0400549v.u8",  0x0100000, 0x0080000, CRC(8d0e61a8) SHA1(254b106e71a0888b0456afd8d63006d72c0ba292) )
	ROM_LOAD32_WORD("0400549v.u12", 0x0100002, 0x0080000, CRC(fdf22d5b) SHA1(664fa003a350c0a3b515b7c384d32176158c2d3e) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( retrsama )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b88
	    0x000000-0x0590b7 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xa4b725ab
	        Calculated Checksum 0xa4b725ab  (OK)
	    0x0590b8-0x0ef623 is the non-Checksummed range (unusual endpoint)
	*/

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0200549v.u7",  0x0000000, 0x0080000, CRC(e60859a1) SHA1(0be0114a87a21b955dfe24d01547e2d93dcb4f2c) )
	ROM_LOAD32_WORD("0200549v.u11", 0x0000002, 0x0080000, CRC(e662404b) SHA1(f0da3384c81d01ec17d24b2191d3a0b0eaf48d12) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( retrsamb )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b88
	    0x000000-0x05889b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xd80cf106
	        Calculated Checksum 0xd80cf106  (OK)
	    0x05889c-0x0f313b is the non-Checksummed range (unusual endpoint)
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0200506v.u7",  0x0000000, 0x0080000, CRC(acb913c1) SHA1(eb008b2b3d06f769f1ea1c75b52334e468c5f13c) )
	ROM_LOAD32_WORD("0200506v.u11", 0x0000002, 0x0080000, CRC(99f61822) SHA1(88a726a5c9cae3a7d3120cb9013ca4d38ef8c560) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( sumospin )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b88
	    0x000000-0x05d92b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x94d3401c
	        Calculated Checksum 0x94d3401c  (OK)
	    0x05d92c-0x18f637 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x05d92c-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0200606v.u7",  0x0000000, 0x0080000, CRC(c3ec9f97) SHA1(62c886cc794de4b915533729c5ea5a71a4b59108) )
	ROM_LOAD32_WORD("0200606v.u11", 0x0000002, 0x0080000, CRC(919999fe) SHA1(3d800df5e0abed04c76928b04973ea7c7b02e5d1) )
	ROM_LOAD32_WORD("0200606v.u8",  0x0100000, 0x0080000, CRC(eb47f317) SHA1(43ead31e788cce1aa03011f634e939489d965144) )
	ROM_LOAD32_WORD("0200606v.u12", 0x0100002, 0x0080000, CRC(ba3eede2) SHA1(708a25af0908a1aa874b3ca4897816c65b0c9178) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( sbuk2 )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b98
	    0x000000-0x06ab7f is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x874caad2
	        Calculated Checksum 0x874caad2  (OK)
	    0x06ab80-0x1fffef is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x06ab80-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0400501v.u7",  0x0000000, 0x0080000, CRC(f025775d) SHA1(71a94f6f17fa7cdcd997b0117b8f4afe21606a69) )
	ROM_LOAD32_WORD("0400501v.u11", 0x0000002, 0x0080000, CRC(f1b51a61) SHA1(8e9fcb071f704122e13333094828a41974646792) )
	ROM_LOAD32_WORD("0400501v.u8",  0x0100000, 0x0080000, BAD_DUMP CRC(03912f4e) SHA1(48bdcd2160e05261b7d834c53e1d483acaad098f) ) // bit 0x20 is stuck on for most of the ROM
	ROM_LOAD32_WORD("0400501v.u12", 0x0100002, 0x0080000, CRC(f9b65d2b) SHA1(f519fc284aaa08d3619e4d88e92e690320cf5432) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( sbuk3 )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000ba8
	    0x000000-0x05ead3 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x23d4cb22
	        Calculated Checksum 0x23d4cb22  (OK)
	    0x05ead4-0x114e33 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x05ead4-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0200711v.u7",  0x0000000, 0x0080000, CRC(e056c7db) SHA1(7a555583f750d8275b2ffd25a0efbe370a5ac43c) )
	ROM_LOAD32_WORD("0200711v.u11", 0x0000002, 0x0080000, CRC(a810782c) SHA1(5d59b464c44ec32b2b977f8326c8bf3424a61e07) )
	ROM_LOAD32_WORD("0200711v.u8",  0x0100000, 0x0080000, CRC(2ff83479) SHA1(2f0c6c12e115a5592c29e806a946817a4f1b89a3) )
	ROM_LOAD32_WORD("0200711v.u12", 0x0100002, 0x0080000, CRC(a585172d) SHA1(3c74efb11285ff78ce76a7e8af2f936d3dc31290) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( swhr2 )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000ae0
	    0x000000-0x041803 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x4449ca76
	        Calculated Checksum 0x4449ca76  (OK)
	    0x041804-0x0ecbb3 is the non-Checksummed range (unusual endpoint)
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0200004v.u7",  0x0000000, 0x0080000, CRC(de4d6d77) SHA1(959ffb7d06359870e07cb9d761f0bc0480c45e0c) )
	ROM_LOAD32_WORD("0200004v.u11", 0x0000002, 0x0080000, CRC(bde067d7) SHA1(cbf2cbd0644f1daeb5c3cd08d72f3d7aafe521ec) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


// MV4061 - 5 Credit Multiplier / 5 Line Multiline.
// Sweethearts II - Export - A - 29/06/98.
// Marked as PHG0742 and 92.252%
// All devices are 27c4002 instead of 27c4096.
ROM_START( swhr2u )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000d08
	    0x000000-0x0b31cb is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x0720df2c
	        Calculated Checksum 0x3dad9905  (BAD)
	    0x0b31cc-0x155097 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x0b31cc-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	// the checksum only covers part of the first 2 roms, marked all as BAD_DUMP because it can't be trusted without a full redump.
	ROM_LOAD32_WORD( "phg074202.u7",  0x000000, 0x80000, BAD_DUMP CRC(d6f83014) SHA1(7c6902d67157a04bdbbfc7c7d8ae1e22befd840f) )
	ROM_LOAD32_WORD( "phg074202.u11", 0x000002, 0x80000, BAD_DUMP CRC(3fa6e538) SHA1(958461a54e57c4622151bbcde3de8f4ff3f9ec0a) )
	ROM_LOAD32_WORD( "phg074202.u8",  0x100000, 0x80000, BAD_DUMP CRC(916409f7) SHA1(d5c3cb7afac14a27f4722528a3dac4b4f2d41580) )
	ROM_LOAD32_WORD( "phg074202.u12", 0x100002, 0x80000, BAD_DUMP CRC(92f92875) SHA1(bdb24974c2bf7bfb772c34a02a20e97df9293c0c) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( swhr2v )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b68
	    0x000000-0x07a763 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x014df7a2
	        Calculated Checksum 0x014df7a2  (OK)
	    0x07a764-0x0e360b is the non-Checksummed range (unusual endpoint)
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "01j01986.u7",  0x000000, 0x80000, CRC(f51b2faa) SHA1(dbcfdbee92af5f89a8a2611bbc687ee0cc907642) )
	ROM_LOAD32_WORD( "01j01986.u11", 0x000002, 0x80000, CRC(bd7ead91) SHA1(9f775428a4aa0b0a8ee17aed9be620edc2020c5e) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( topbana )
	ARISTOCRAT_MK5_BIOS
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0100550v.u11", 0x000002, 0x080000, CRC(1c64b3b6) SHA1(80bbc6e3f47ab932e9c07e0c6063197a2d8e81f7) )
	ROM_LOAD32_WORD( "0100550v.u7",  0x000000, 0x080000, CRC(9c5e2d66) SHA1(658143706c0e1f3b43b3ec301da1052363fe5244) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( trstrove )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b98
	    0x000000-0x0638d7 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x7fa3a1a8
	        Calculated Checksum 0x7fa3a1a8  (OK)
	    0x0638d8-0x158933 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x0638d8-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("01j00161.u7",  0x0000000, 0x0080000, CRC(07a8b338) SHA1(7508d7d0e3494d355cb773165b240ba876a60eec) )
	ROM_LOAD32_WORD("01j00161.u11", 0x0000002, 0x0080000, CRC(020a588d) SHA1(4759bef22017fb4c47c87adb6ca7253fdb6bca6b) )
	ROM_LOAD32_WORD("01j00161.u8",  0x0100000, 0x0080000, CRC(89a042e7) SHA1(0f95cfd42ce7130176d42c6bbdf8ff22a6662894) )
	ROM_LOAD32_WORD("01j00161.u12", 0x0100002, 0x0080000, CRC(715f53cb) SHA1(364c35fc2d36180c13127c8004a8729126f68db1) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( tritreat )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000d18
	    0x000000-0x07089b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x56d2b752
	        Calculated Checksum 0x56d2b752  (OK)
	    0x07089c-0x2903cf is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x07089c-0x2fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0201692v.u7",  0x0000000, 0x0080000, CRC(7bc25bba) SHA1(d5f7c3a4bc3c652f57ee4cdbc883ec82069365d1) )
	ROM_LOAD32_WORD("0201692v.u11", 0x0000002, 0x0080000, CRC(fbc125b8) SHA1(55dbc3a236804f4a8d26be8e49c29fa5943c5bd6) )
	ROM_LOAD32_WORD("0201692v.u8",  0x0100000, 0x0080000, CRC(ef976f78) SHA1(d2c89e8d3bf6af112a99354133f308a5aabad46e) )
	ROM_LOAD32_WORD("0201692v.u12", 0x0100002, 0x0080000, CRC(5df3854a) SHA1(2b5175835c587caccafb73a1a5c8abf8f8463cf4) )
	ROM_LOAD32_WORD("0201692v.u9",  0x0200000, 0x0080000, CRC(776fbfd2) SHA1(27820dbc6ee1424706aea9c4574da117636fef17) )
	ROM_LOAD32_WORD("0201692v.u13", 0x0200002, 0x0080000, CRC(0a0b0ce1) SHA1(41a4d613cf1828df1832c087f0bc18d31076f056) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( trojhors )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000bb8
	    0x000000-0x06e9f7 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x071faa81
	        Calculated Checksum 0x071faa81  (OK)
	    0x06e9f8-0x2df4f7 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x06e9f8-0x2fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("01j00851.u7",  0x0000000, 0x0080000, CRC(7be0caf5) SHA1(b83fba7eb4624b3dc56f763b48b7c45fe31f3396) )
	ROM_LOAD32_WORD("01j00851.u11", 0x0000002, 0x0080000, CRC(8c04ed89) SHA1(6727da3a457841e893e27bc8f10d4bb58a61f338) )
	ROM_LOAD32_WORD("01j00851.u8",  0x0100000, 0x0080000, CRC(246d3693) SHA1(8c8b893c21e9a486fd36677d7157787bf5d6237b) )
	ROM_LOAD32_WORD("01j00851.u12", 0x0100002, 0x0080000, CRC(1eb021a4) SHA1(3195eb5923da018b6c2dac10b70c47aef54dca35) )
	ROM_LOAD32_WORD("01j00851.u9",  0x0200000, 0x0080000, CRC(15dee624) SHA1(d678ef7c25419342a1512fab84394e99309009ec) )
	ROM_LOAD32_WORD("01j00851.u13", 0x0200002, 0x0080000, CRC(b6d1ceb6) SHA1(b41200620aaa905697ac73b4c86496a53f070ed3) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


// 577/3 - 10 Credit Multiplier / 9 Line Multiline.
// Tropical Delight -  Export  D - 24/09/97.
// Marked as PHG062502 and 92.25%.
// All devices are 27c4002 instead of 27c4096.
ROM_START( trpdlght )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000d08
	    0x000000-0x0b2d1f is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x910fae87
	        Calculated Checksum 0x2485ae87  (BAD)
	    0x0b2d20-0x15384f is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x0b2d20-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	// the checksum only covers part of the first 2 roms, marked all as BAD_DUMP because it can't be trusted without a full redump.
	ROM_LOAD32_WORD( "phg062502.u7",  0x000000, 0x80000, BAD_DUMP CRC(3d06765f) SHA1(737d714e4ec48eb6283489f745dd305e7d70dad2) )  // 92.25%
	ROM_LOAD32_WORD( "phg062502.u11", 0x000002, 0x80000, BAD_DUMP CRC(3963a3de) SHA1(fc2b06af3d1eba87407425dc4296a8b602952775) )  // 92.25%
	ROM_LOAD32_WORD( "phg062502.u8",  0x100000, 0x80000, BAD_DUMP CRC(d4858407) SHA1(acf6776f19448648a26aaf53fcb4bc227c546033) )  // base
	ROM_LOAD32_WORD( "phg062502.u12", 0x100002, 0x80000, BAD_DUMP CRC(852e433e) SHA1(17ec568edbabe3ee8649b26f4c5d0f501494f823) )  // base

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( unicornd )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000bf8
	    0x000000-0x05f36f is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x2f8bff86
	        Calculated Checksum 0x2f8bff86  (OK)
	    0x05f370-0x1d0a3f is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x05f370-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100791v.u7",  0x0000000, 0x0080000, CRC(d785d1b3) SHA1(4aa7c61036dd5fe1cdbc6c39a89881f88f3dd148) )
	ROM_LOAD32_WORD("0100791v.u11", 0x0000002, 0x0080000, CRC(b45885f1) SHA1(e32d4afce4e3e62a324173252f559909ea97fe3a) )
	ROM_LOAD32_WORD("0100791v.u8",  0x0100000, 0x0080000, CRC(6ba8f7eb) SHA1(bd5b15e22e713095f580b4c371d39af4af9e3289) )
	ROM_LOAD32_WORD("0100791v.u12", 0x0100002, 0x0080000, CRC(14afdeda) SHA1(1eb2a297e903dc1a0683425b37669e0af4ae4218) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( unicorndnz )
	ARISTOCRAT_MK5_BIOS
	// checksum code not found (uses different startup sequence)
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0101228v.u7",  0x0000000, 0x0080000, CRC(54d55ecb) SHA1(0afb2d6489f01ae55563030e228e1d5443738af1) )
	ROM_LOAD32_WORD("0101228v.u11", 0x0000002, 0x0080000, CRC(2be7933c) SHA1(7691d755714bf0801f9ca8510f82a80a4c231178) )
	ROM_LOAD32_WORD("0101228v.u8",  0x0100000, 0x0080000, CRC(9c4a6e7f) SHA1(6ac470ec777c68521ef74c66263c8229f8d21176) )
	ROM_LOAD32_WORD("0101228v.u12", 0x0100002, 0x0080000, CRC(00d4cd6e) SHA1(eb941164cf421f22bcc9864f198348c4f30d904c) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( wamazona )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b68
	    0x000000-0x052b8b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xc07f695c
	        Calculated Checksum 0xc07f695c  (OK)
	    0x052b8c-0x1fffef is the non-Checksummed range still containing data but NOT covered by Checksum  (unusual)
	    0x052b8c-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0200285v.u7",  0x0000000, 0x0080000, CRC(bfa21358) SHA1(6b76656401b3dbbace8d4335951468b9885fc7f0) )
	ROM_LOAD32_WORD("0200285v.u11", 0x0000002, 0x0080000, CRC(54b2a375) SHA1(635fde5c678b908fa58c0e04ba9b7a84fac1f7fe) )
	ROM_LOAD32_WORD("0200285v.u8",  0x0100000, 0x0080000, CRC(4e39f128) SHA1(91d3c6a9e5c30275c3f8967dde55214df097f2ba) )
	ROM_LOAD32_WORD("0200285v.u12", 0x0100002, 0x0080000, BAD_DUMP CRC(2cd12986) SHA1(b6b0bd6dd8c964498edc3763cb5c450795042a8d) ) // actually a bad dump of u7 from oscara5!

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( wamazon )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b68
	    0x000000-0x05c043 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x2c7f1cbb
	        Calculated Checksum 0x2c7f1cbb  (OK)
	    0x05c044-0x0f60cb is the non-Checksummed range (unusual endpoint)
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0200507v.u7",  0x0000000, 0x0080000, CRC(44576def) SHA1(3396460444ceb394c9c88e5fc37ccedcfc4b179c) )
	ROM_LOAD32_WORD("0200507v.u11", 0x0000002, 0x0080000, CRC(2e24756a) SHA1(247db8316e7815be7524aefc43a5756fad84779a) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( wamazonv )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b68
	    0x000000-0x07b2f3 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x7b4d5882
	        Calculated Checksum 0x7b4d5882  (OK)
	    0x07b2f4-0x11537b is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x07b2f4-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("01j01996.u7",  0x0000000, 0x0080000, CRC(c14d5f8a) SHA1(66059549f94048fe55ec52a098548e04c2ccece0) )
	ROM_LOAD32_WORD("01j01996.u11", 0x0000002, 0x0080000, CRC(6dde9ae6) SHA1(ae18d3fc2269549e60893a3cb828c2993f7f0bfa) )
	ROM_LOAD32_WORD("01j01996.u8",  0x0100000, 0x0080000, CRC(00aebc93) SHA1(6829f4d0fc13cb731138c7c54fac90d75f56588b) )
	ROM_LOAD32_WORD("01j01996.u12", 0x0100002, 0x0080000, CRC(945e0a05) SHA1(db7580b39c537a7b50898ee99cfceb5eb71be19a) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( wildbill )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000ad8
	    0x000000-0x054e6b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xd6b20386
	        Calculated Checksum 0xd6b20386  (OK)
	    0x054e6c-0x0ec99f is the non-Checksummed range (unusual endpoint)
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100297v.u7",  0x0000000, 0x0080000, CRC(e3117ab7) SHA1(c13912f524f1c1d373adb6382ceddd1bc18f7f02) )
	ROM_LOAD32_WORD("0100297v.u11", 0x0000002, 0x0080000, CRC(57b3c340) SHA1(4f95ed7fed697cf2bfbde8215f6e35768cf20334) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( wcougar )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000adc
	    0x000000-0x043573 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x0a061a1a
	        Calculated Checksum 0x0a061a1a  (OK)
	    0x043574-0x1061fb is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x043574-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0100167v.u7",  0x0000000, 0x0080000, CRC(47154679) SHA1(21749fbaa60f9bf1db43bdd272e6628ae73bf759) )
	ROM_LOAD32_WORD("0100167v.u11", 0x0000002, 0x0080000, CRC(6a5f2c41) SHA1(1365e083d44a373c2d4f17e8e61ec716ffb6d2d5) )
	ROM_LOAD32_WORD("0100167v.u8",  0x0100000, 0x0080000, CRC(c262d098) SHA1(87940bd0aef6cb0f5ff21ccda4b209eef8e97eb1) )
	ROM_LOAD32_WORD("0100167v.u12", 0x0100002, 0x0080000, CRC(85bb41a7) SHA1(335f29f10f216e202b93b46a376958c3f5271461) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


// 569/8 - 10 Credit Multiplier / 9 Line Multiline.
// Wild Cougar - Export D - 19/05/97.
// All devices are 27c4002 instead of 27c4096.
ROM_START( wcougaru )
	ARISTOCRAT_MK5_BIOS_HAVE_EEPROMS
	/*
	    Checksum code found at 0x000d08
	    0x000000-0x0b0d5b is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0xdfe9eb92
	        Calculated Checksum 0xdfe9eb92  (OK)
	    0x0b0d5c-0x153803 is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x0b0d5c-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "nhg029604.u7",  0x000000, 0x80000, CRC(7ada053f) SHA1(5102b0b9db505454624750a3fd6db455682538f3) )
	ROM_LOAD32_WORD( "nhg029604.u11", 0x000002, 0x80000, CRC(69a78695) SHA1(1ed89cf38dc85f752449a858cd9558bed235af58) )
	ROM_LOAD32_WORD( "nhg029604.u8",  0x100000, 0x80000, CRC(496b0295) SHA1(237183a192ad9b4bc133014cc83149d4a7062785) )
	ROM_LOAD32_WORD( "nhg029604.u12", 0x100002, 0x80000, CRC(fe2bafdc) SHA1(e8b454db44a532d75b3aff323855340695688f0f) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )

	ROM_REGION16_BE( 0x100, "eeprom0", 0 )
	ROM_LOAD16_WORD_SWAP( "eeprom0",      0x000000, 0x000100, CRC(fea8a821) SHA1(c744cac6af7621524fc3a2b0a9a135a32b33c81b) )

	ROM_REGION16_BE( 0x100, "eeprom1", 0 )
	ROM_LOAD16_WORD_SWAP( "eeprom1",      0x000000, 0x000100, CRC(8421e7c2) SHA1(fc1b07d5b7aadafc4a0f2e4dfa698e7c72340717) )

	ROM_REGION( 0x80000, "nvram", 0 )
	ROM_LOAD( "nvram",        0x000000, 0x080000, CRC(dfe52286) SHA1(db31fb64e2fff8aa5ba0cc6d3d73860e8019406c) )

	ROM_REGION( 0x20, "rtc", 0 )
	ROM_LOAD( "rtc",          0x000000, 0x00001f, CRC(6909acb0) SHA1(6a4589599cd1c477e916474e7b029e9a4e92019b) )
ROM_END

ROM_START( wizways )
	ARISTOCRAT_MK5_BIOS

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0200396v.u7",  0x0000000, 0x0080000, CRC(85fca945) SHA1(ca48a907b4f6c1b665ae053e2992681f02166cb1) )
	ROM_LOAD32_WORD("0200396v.u11", 0x0000002, 0x0080000, CRC(677c855f) SHA1(a4ec8e6151271af292379ead28214ef9163bfdc3) )
	ROM_LOAD32_WORD("0200396v.u8",  0x0100000, 0x0080000, CRC(4b1192ae) SHA1(2537249ccfc8c507762ac2c46d05ef13fa3d0bf9) )
	ROM_LOAD32_WORD("0200396v.u12", 0x0100002, 0x0080000, CRC(934d7286) SHA1(205f72b62d83667e9068141346bda3bcb9742a83) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( wnpost )
	ARISTOCRAT_MK5_BIOS

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("rhg041804.u7",  0x0000000, 0x0080000, CRC(73274802) SHA1(9838a63d5f4e1bf31675ac15c34a17e709f2f647) )
	ROM_LOAD32_WORD("rhg041804.u11", 0x0000002, 0x0080000, CRC(4f076a94) SHA1(2962429c930ffc17e2f37a006215ee6f1c649953) )
	ROM_LOAD32_WORD("rhg041804.u8",  0x0100000, 0x0080000, CRC(b1830ffa) SHA1(c23a97bfdfe9e408f0ec8053646d6c8c8e06a263) )
	ROM_LOAD32_WORD("rhg041804.u12", 0x0100002, 0x0080000, CRC(ff70b305) SHA1(3c832ed20b1d00318fd3d2428bb469735f68965b) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END

ROM_START( wthing )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000b74
	    0x000000-0x0673cb is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x89dd307a
	        Calculated Checksum 0x89dd307a  (OK)
	    0x0673cc-0x1b367b is the non-Checksummed range still containing data but NOT covered by Checksum
	    0x0673cc-0x1fffff is the non-Checksummed range if the additional vectors? at the end are included
	*/

	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD("0101158v.u7",  0x0000000, 0x0080000, CRC(eb402ffb) SHA1(49ef6ca2503a6e785f62cb29e505e5c2ba019e37) )
	ROM_LOAD32_WORD("0101158v.u11", 0x0000002, 0x0080000, CRC(61d22f2e) SHA1(b836e5afbd5bb14ae68e100a6042f1953ed57a21) )
	ROM_LOAD32_WORD("0101158v.u8",  0x0100000, 0x0080000, CRC(f21153b8) SHA1(24830b3939a8568b0d5b59d4fdbd2d9e7b46a6d7) )
	ROM_LOAD32_WORD("0101158v.u12", 0x0100002, 0x0080000, CRC(450a4f4f) SHA1(e59fb3260755a125c47ff6c1a042a48b0eace72d) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END


ROM_START( wtiger )
	ARISTOCRAT_MK5_BIOS
	/*
	    Checksum code found at 0x000d30
	    0x000000-0x060227 is the Checksummed Range (excluding 0x000020-0x000027 where Checksum is stored)
	        Expected Checksum   0x61da8e77
	        Calculated Checksum 0x61da8e77  (OK)
	    0x060228-0x0d61cf is the non-Checksummed range (unusual endpoint)
	*/
	ROM_REGION( 0x400000, "game_prg", ROMREGION_ERASEFF )
	ROM_LOAD32_WORD( "0200954v.u7",  0x000000, 0x80000, CRC(752e54c5) SHA1(9317544a7cf2d9bf29347d31fe72331fc3d018ef) )
	ROM_LOAD32_WORD( "0200954v.u11", 0x000002, 0x80000, CRC(38e888b1) SHA1(acc857eb2be19140bbb58d70583e08f24807b9f2) )

	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 ) /* ARM Code */
	ROM_REGION( 0x200000, "vram", ROMREGION_ERASE00 )
	ROM_REGION( 0x20000*4, "sram", ROMREGION_ERASE00 )
ROM_END



/*************************
*      Game Drivers      *
*************************/

#define MACHINE_FLAGS MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND|MACHINE_IMPERFECT_GRAPHICS

//    YEAR  NAME       PARENT    MACHINE       INPUT     STATE           INIT      ROT     COMPANY       FULLNAME                                         FLAGS
GAME( 1995, aristmk5,  0,        aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "MKV Set/Clear Chips (USA)",                    MACHINE_FLAGS|MACHINE_IS_BIOS_ROOT )

// Dates listed below are for the combination (reel layout), not release dates
GAMEL( 1998, adonis,    aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Adonis (0200751V, NSW/ACT)",                   MACHINE_FLAGS, layout_aristmk5 )  // 602/9,    A - 25/05/98, Rev 10
GAMEL( 1998, adonisa,   adonis,   aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Adonis (0100751V, NSW/ACT)",                   MACHINE_FLAGS, layout_aristmk5 )  // 602/9,    A - 25/05/98, Rev 9
GAMEL( 1999, adonisce,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Adonis - Cash Express (0201005V, NSW/ACT)",    MACHINE_FLAGS, layout_aristmk5 )  // 602/9, C - 06/07/99
GAMEL( 1996, baddog,    aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Bad Dog Poker (0200428V, NSW/ACT)",            MACHINE_FLAGS, layout_aristmk5 )  // 386/56, A - 17/12/96
GAMEL( 1998, bootsctn,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Boot Scootin' (0100812V, NSW/ACT)",            MACHINE_FLAGS, layout_aristmk5 )  // 616/1, B - 11/12/98
GAMEL( 1996, bumblbug,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Bumble Bugs (0200510V, NSW/ACT)",              MACHINE_FLAGS, layout_aristmk5 )  // 593, D - 5/07/96
GAMEL( 1996, bumblbugql,bumblbug, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Bumble Bugs (0200456V, Queensland)",           MACHINE_FLAGS, layout_aristmk5 )  // 593,      D - 5/07/96
GAMEL( 1995, buttdeli,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Butterfly Delight (0200143V, NSW/ACT)",        MACHINE_FLAGS, layout_aristmk5 )  // 571/4, A - 19/12/95
GAMEL( 1999, cashcat,   aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Cash Cat (0300863V, New Zealand)",             MACHINE_FLAGS, layout_aristmk5 )  // MV4089, A - 4/1/99
GAMEL( 1997, cashcham,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Cash Chameleon (0100438V, NSW/ACT)",           MACHINE_FLAGS, layout_aristmk5 )  // 603/1, C  - 15/4/97
GAMEL( 1998, cashchama, cashcham, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Cash Chameleon (0200437V, NSW/ACT)",           MACHINE_FLAGS, layout_aristmk5 )  // 603(a), D - 18/02/98
GAMEL( 1998, cashchamnz,cashcham, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Cash Chameleon (0300781V, New Zealand)",       MACHINE_FLAGS, layout_aristmk5 )  // MV4067, A - 31/08/98
GAMEL( 1997, cashcra5,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Cash Crop (0300467V, NSW/ACT)",                MACHINE_FLAGS, layout_aristmk5 )  // 607, C - 14/07/97
GAMEL( 1998, chariotc,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "The Chariot Challenge (04J00714, Venezuela)",   MACHINE_FLAGS, layout_aristmk5 )  // 630,      A - 10/08/98, Rev 12
GAMEL( 1998, chariotca, chariotc, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "The Chariot Challenge (0100787V, NSW/ACT)",    MACHINE_FLAGS, layout_aristmk5 )  // 630/1, A - 10/08/98
GAMEL( 2001, checkma5,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Checkmate (01J00681, NSW/ACT)",                MACHINE_FLAGS, layout_aristmk5 )  // JB011, B - 06/07/01
GAMEL( 1996, chickna5,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Chicken (0100351V, NSW/ACT)",                  MACHINE_FLAGS, layout_aristmk5 )  // 596, A - 27/08/96
GAMEL( 1998, chickna5u, chickna5, aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Chicken (RHG0730-03, US)",                 MACHINE_FLAGS, layout_aristmk5_us )  // 596,      C - 23/02/98
GAMEL( 1998, chickna5qld,chickna5,aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Chicken (0200530V, Queensland)",               MACHINE_FLAGS, layout_aristmk5 )  // 596, C - 23/02/98
GAMEL( 1998, coralrc2,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Coral Riches II (0100919V, NSW/ACT)",          MACHINE_FLAGS, layout_aristmk5 )  // 577/7, A - 29/12/98
GAMEL( 1998, cuckoo,    aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Cuckoo (0200753V, NSW/ACT)",                   MACHINE_FLAGS, layout_aristmk5 )  // 615/1, D - 03/07/98
GAMEL( 2000, cuckoou,   cuckoo,   aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Cuckoo (CHG1195, US)",                     MACHINE_FLAGS, layout_aristmk5_us )  // MV4104,   C - 02/02/00
GAMEL( 1995, dstbloom,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Desert Bloom (0200111V, NSW/ACT)",             MACHINE_FLAGS, layout_aristmk5 )  // 577/2, A - 12/10/95
GAMEL( 1999, diamdove,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Diamond Dove (0101018V, NSW/ACT)",             MACHINE_FLAGS, layout_aristmk5 )  // 640, B - 19/05/99
GAMEL( 1996, dmdfever,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Diamond Fever (0200302V, NSW/ACT)",            MACHINE_FLAGS, layout_aristmk5 )  // 483/7, E - 05/09/96 - jumps to service mode
GAMEL( 1997, dimtouch,  aristmk5, aristmk5_touch,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Diamond Touch (0400433V, NSW/ACT)",      MACHINE_FLAGS, layout_aristmk5 )  // 604,      E - 30/06/97
GAMEL( 1996, dolphntr,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Dolphin Treasure (0200424V, NSW/ACT)",         MACHINE_FLAGS, layout_aristmk5 )  // 602/1,    B - 06/12/96, Rev 3
GAMEL( 1996, dolphntra, dolphntr, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Dolphin Treasure (0100424V, NSW/ACT)",         MACHINE_FLAGS, layout_aristmk5 )  // 602/1,    B - 06/12/96, Rev 1.24.4.0
GAMEL( 1996, dolphntrb, dolphntr, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Dolphin Treasure (0100388V, NSW/ACT)",         MACHINE_FLAGS, layout_aristmk5 )  // 602, B - 10/12/96
GAMEL( 1996, dolphntru, dolphntr, aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Dolphin Treasure (FHG4077-02, US)",        MACHINE_FLAGS, layout_aristmk5_us )  // 602/1,    B - 06/12/96
GAMEL( 1999, dolphntrce,dolphntr, aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Dolphin Treasure - Cash Express (AHG1606, US)",MACHINE_FLAGS, layout_aristmk5_us ) //  MV4090, D - 22/12/99
GAMEL( 1997, drgneye,   aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Dragon's Eye (0100521V, NSW/ACT)",             MACHINE_FLAGS, layout_aristmk5 )  // 610, A - 09/05/97
GAMEL( 2000, dynajack,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Dynamite Jack (01J00081, NSW/ACT)",            MACHINE_FLAGS, layout_aristmk5 )  // JB004, A - 12/07/2000
GAMEL( 1998, eldorda5,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "El Dorado (0100652V, NSW/ACT)",                MACHINE_FLAGS, layout_aristmk5 )  // 623, B - 24/03/98
GAMEL( 1995, eforsta5,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Enchanted Forest (0400122V, NSW/ACT)",         MACHINE_FLAGS, layout_aristmk5 )  // 570/3,    E - 23/06/95
GAMEL( 2000, fortellr,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Fortune Teller (01J00131, NSW/ACT)",           MACHINE_FLAGS, layout_aristmk5 )  // JB006, D - 24/11/2000
GAMEL( 2001, geisha,    aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Geisha (0101408V, New Zealand)",               MACHINE_FLAGS, layout_aristmk5 )  // MV4127,   A - 05/03/01
GAMEL( 1999, genmagi,   aristmk5, aristmk5_touch,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Genie Magic (0200894V, NSW/ACT)",        MACHINE_FLAGS, layout_aristmk5 )  // ???,   C - 15/02/99
GAMEL( 1998, gnomeatw,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Gnome Around The World (0100767V, NSW/ACT)",   MACHINE_FLAGS, layout_aristmk5 )  // 625, C - 18/12/98
GAMEL( 1997, goldpyr,   aristmk5, aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Golden Pyramids (AHG1205-03, US)",         MACHINE_FLAGS, layout_aristmk5_us )  // MV4091,   B - 13/05/97
GAMEL( 1998, goldpyrb,  goldpyr,  aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Golden Pyramids (0100878V, Victoria)",         MACHINE_FLAGS, layout_aristmk5 ) // 602/5, C - 19/06/98
GAMEL( 2000, goldenra,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Golden Ra (0101164V, NSW/ACT)",                MACHINE_FLAGS, layout_aristmk5 )  // 661, A - 10/04/00
GAMEL( 1999, incasun,   aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Inca Sun (0100872V, NSW/ACT)",                 MACHINE_FLAGS, layout_aristmk5 )  // 631/3 B, B - 03/05/99
GAMEL( 1999, incasunsp, incasun,  aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Inca Sun (0100872V, NSW/ACT, Show Program)",   MACHINE_FLAGS, layout_aristmk5 )  // 631/3 B, B - 03/05/99
GAMEL( 2000, incasunnz, incasun,  aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Inca Sun (0101108V, New Zealand)",             MACHINE_FLAGS, layout_aristmk5 )  // MV4113, A - 6/3/00
GAMEL( 2000, incasunu,  incasun,  aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Inca Sun (CHG1458, US)",                   MACHINE_FLAGS, layout_aristmk5_us )  // MV4130/3, A - 05/09/00
GAMEL( 1998, indrema5,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Indian Dreaming (0100845V, NSW/ACT)",          MACHINE_FLAGS, layout_aristmk5 )  // 628/1,    B - 15/12/98
GAMEL( 1996, jungjuic,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Jungle Juice (0200240V, New Zealand)",         MACHINE_FLAGS, layout_aristmk5 )  // 566/3, F - 06/03/96
GAMEL( 1995, kgalah,    aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "King Galah (0200536V, NSW/ACT)",               MACHINE_FLAGS, layout_aristmk5 )  // 613/6, A - 21/07/95
GAMEL( 1994, kgbirdmk5, aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "K.G. Bird (0200024V, NSW/ACT)",               MACHINE_FLAGS, layout_aristmk5 )  // 540/3, D - 10/10/94
GAMEL( 1998, kookabuk,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Kooka Bucks (0100677V, NSW/ACT)",              MACHINE_FLAGS, layout_aristmk5 )  // 661, A - 03/04/98
GAMEL( 1997, locoloot,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Loco Loot (0100472V, NSW/ACT)",                MACHINE_FLAGS, layout_aristmk5 )  // 599/2, C - 17/06/97
GAMEL( 1998, locolootnz,locoloot, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Loco Loot (0600725V, New Zealand)",            MACHINE_FLAGS, layout_aristmk5 )  // MV4064, A - 8/7/98
GAMEL( 1997, lonewolf,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Lone Wolf (0100587V, NSW/ACT)",                MACHINE_FLAGS, layout_aristmk5 )  // 621, A - 29/10/97
GAMEL( 2000, magimask,  aristmk5, aristmk5_usa_touch, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Magic Mask (DHG1309, US)",           MACHINE_FLAGS, layout_aristmk5_us )  // MV4115,   A - 09/05/00
GAMEL( 2000, magimaska, magimask, aristmk5_usa_touch, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Magic Mask (AHG1548, US)",           MACHINE_FLAGS, layout_aristmk5_us )  // MV4115,   A - 09/05/00
GAMEL( 1997, magtcha5,  aristmk5, aristmk5_touch,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Magic Touch (0200455V, NSW/ACT)",        MACHINE_FLAGS, layout_aristmk5 )  // 606, A - 06/03/97
GAMEL( 2000, marmagic,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Margarita Magic (01J00101, NSW/ACT)",          MACHINE_FLAGS, layout_aristmk5 )  // JB005,    A - 07/07/00
GAMEL( 2000, marmagicua,marmagic, aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Margarita Magic (EHG1558, US)",            MACHINE_FLAGS, layout_aristmk5_us )  // US003, 07/07/2000
GAMEL( 1996, minemine,  aristmk5, aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Mine, Mine, Mine (VHG0416-99, US)",        MACHINE_FLAGS, layout_aristmk5_us )  // 559/2,    E - 14/02/96
GAMEL( 1997, monmouse,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Money Mouse (0400469V, NSW/ACT)",              MACHINE_FLAGS, layout_aristmk5 )  // 607/1, B - 08/04/97
GAMEL( 2001, montree,   aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Money Tree (0201397V, New Zealand)",           MACHINE_FLAGS, layout_aristmk5 )  // MV4126, C - 12/04/01
GAMEL( 1996, mountmon,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Mountain Money (0100294V, NSW/ACT)",           MACHINE_FLAGS, layout_aristmk5 )  // 595/3, B - 11/06/96
GAMEL( 2000, multidrw,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Multidraw - Free Games (0200956V, NSW/ACT)",   MACHINE_FLAGS, layout_aristmk5 )  // 386/64, E - 08/05/00
GAMEL( 1996, mystgard,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Mystic Garden (0100275V, NSW/ACT)",            MACHINE_FLAGS, layout_aristmk5 )  // 595/1, B - 11/06/96
GAMEL( 1999, orchidms,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Orchid Mist (0200849V, NSW/ACT)",              MACHINE_FLAGS, layout_aristmk5 )  // 601/3, C - 03/02/99
GAMEL( 1996, oscara5,   aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Oscar (0200348V, NSW/ACT)",                    MACHINE_FLAGS, layout_aristmk5 )  // 593/2, C - 20/09/96
GAMEL( 1999, pantmag,   aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Panther Magic (0101046V, NSW/ACT)",            MACHINE_FLAGS, layout_aristmk5 )  // 594/7, A - 06/10/99
GAMEL( 1999, pantmaga,  pantmag,  aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Panther Magic (0100716V, NSW/ACT)",            MACHINE_FLAGS, layout_aristmk5 )  // 594/4, A - 13/05/98
GAMEL( 2001, partygrs,  aristmk5, aristmk5_usa_touch, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Party Gras (AHG1567, US)",           MACHINE_FLAGS, layout_aristmk5_us )  // MV4115/6, A - 10/11/01
GAMEL( 2000, peaflut,   aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Peacock Flutter (02J00011, NSW/ACT)",          MACHINE_FLAGS, layout_aristmk5 )  // JB001, A - 10/03/00
GAMEL( 1997, pengpay,   aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Penguin Pays (0200460V, NSW/ACT)",             MACHINE_FLAGS, layout_aristmk5 )  // 586/4(a), D - 03/06/97
GAMEL( 1996, pengpaya,  pengpay,  aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Penguin Pays (0200357V, NSW/ACT)",             MACHINE_FLAGS, layout_aristmk5 )  // 586/4, C - 12/11/96
GAMEL( 1997, pengpayb,  pengpay,  aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Penguin Pays (0200359V, NSW/ACT)",             MACHINE_FLAGS, layout_aristmk5 )  // 586/3(a), D - 03/06/97
GAMEL( 1997, pengpayu,  pengpay,  aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Penguin Pays (BHI0417-03, US)",            MACHINE_FLAGS, layout_aristmk5_us )  // 586/7(b)  B - 14/07/97
GAMEL( 1998, petshop,   aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Pet Shop (0100731V, NSW/ACT)",             MACHINE_FLAGS, layout_aristmk5 )  // 618/1, A - 17/04/98
GAMEL( 1995, phantpay,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Phantom Pays (0500005V, NSW/ACT)",             MACHINE_FLAGS, layout_aristmk5 )  // 570/1, E - 12/09/95
GAMEL( 1998, penpir2,   aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Penguin Pirate II (0100869V, Victoria)",             MACHINE_FLAGS, layout_aristmk5 )  // 619/3, A - 17/12/98
GAMEL( 1996, przfight,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Prize Fight (0100299V, NSW/ACT)",              MACHINE_FLAGS, layout_aristmk5 )  // 578/4, B - 08/08/96
GAMEL( 1998, qcash,     aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Queens of Cash (0100706V, NSW/ACT)",           MACHINE_FLAGS, layout_aristmk5 )  // 603/6, C  - 23/07/98
GAMEL( 1997, qnile,     aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Queen of the Nile (0100439V, NSW/ACT)",        MACHINE_FLAGS, layout_aristmk5 )  // 602/4, B - 13/05/97
GAMEL( 1997, qnilea,    qnile,    aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Queen of the Nile (0300440V, NSW/ACT)",        MACHINE_FLAGS, layout_aristmk5 )  // 602/3, B - 13/05/97
GAMEL( 1997, qnileb,    qnile,    aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Queen of the Nile (0200439V, NSW/ACT)",        MACHINE_FLAGS, layout_aristmk5 )  // 602/4,    B - 13/05/97
GAMEL( 1997, qnilec,    qnile,    aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Queen of the Nile (0300439V, NSW/ACT)",        MACHINE_FLAGS, layout_aristmk5 )  // 602/4,    B - 13/05/97
GAMEL( 1997, qnileu,    qnile,    aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Queen of the Nile (GHG4091-02, US)",       MACHINE_FLAGS, layout_aristmk5_us )  // MV4091,   B - 13/05/97
GAMEL( 1999, qnilemax,  aristmk5, aristmk5_touch,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Queen of the Nile - Maximillions (0401072V, NSW/ACT)", MACHINE_FLAGS, layout_aristmk5 )  // 602/4, D - 18/06/99
GAMEL( 1994, qtbird,    aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Q.T. Bird (0500009V, NSW/ACT)",             MACHINE_FLAGS, layout_aristmk5 )  //  581, A - 27/10/94
GAMEL( 2000, rainwrce,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Rainbow Warriors - Cash Express (0101332V, NSW/ACT)",  MACHINE_FLAGS, layout_aristmk5 )  // 655, B - 02/03/00
GAMEL( 1998, reelrock,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Reelin-n-Rockin (0100779V, NSW/ACT)",          MACHINE_FLAGS, layout_aristmk5 )  // 628,      A - 13/07/98
GAMEL( 1997, retrsam,   aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Return of the Samurai (0400549V, NSW/ACT)",    MACHINE_FLAGS, layout_aristmk5 )  // 608, A - 17/04/97
GAMEL( 1997, retrsama,  retrsam,  aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Return of the Samurai (0200506V, NSW/ACT)",    MACHINE_FLAGS, layout_aristmk5 )  // 608, A - 17/04/97
GAMEL( 1997, retrsamb,  retrsam,  aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Return of the Samurai (0200549V, NSW/ACT)",    MACHINE_FLAGS, layout_aristmk5 )  // 608, A - 17/04/97
GAMEL( 1997, sumospin,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Sumo Spins (0200606V, NSW/ACT)",               MACHINE_FLAGS, layout_aristmk5 )  // 622, A - 08/12/97
GAMEL( 1998, sbuk3,     aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Super Bucks III (0200711V, NSW/ACT)",          MACHINE_FLAGS, layout_aristmk5 )  // 626, A - 22/04/98
GAMEL( 1995, swhr2,     aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Sweethearts II (0200004V, NSW/ACT)",           MACHINE_FLAGS, layout_aristmk5 )  // 577/1, C - 07/09/95
GAMEL( 1995, swhr2v,    swhr2,    aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Sweet Hearts II (01J01986, Venezuela)",        MACHINE_FLAGS, layout_aristmk5 )  // 577/1,    C - 07/09/95
GAMEL( 1997, topbana,   aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Top Banana (0100550V, NSW/ACT)",               MACHINE_FLAGS, layout_aristmk5 )  // 594/3, A - 18/08/97
GAMEL( 1998, toutango,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Toucan Tango (0100782V, NSW/ACT)",             MACHINE_FLAGS, layout_aristmk5 )  // 616/1, A - 17/06/98
GAMEL( 2000, trstrove,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Treasure Trove (01J00161, NSW/ACT)",           MACHINE_FLAGS, layout_aristmk5 )  // JB001/3, A - 5/10/00
GAMEL( 2002, tritreat,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Triple Treat (0201692V, NSW/ACT)",             MACHINE_FLAGS, layout_aristmk5 )  // 692, A - 17/05/02
GAMEL( 2001, trojhors,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Trojan Horse (01J00851, NSW/ACT)",             MACHINE_FLAGS, layout_aristmk5 )  // JB001/5, A - 30/10/01
GAMEL( 1998, unicornd,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Unicorn Dreaming (0100791V, NSW/ACT)",         MACHINE_FLAGS, layout_aristmk5 )  // 631/1, A - 31/08/98
GAMEL( 2000, unicorndnz,unicornd, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Unicorn Dreaming (0101228V, New Zealand)",     MACHINE_FLAGS, layout_aristmk5 )  // MV4113/1, A - 05/04/2000
GAMEL( 1996, wamazon,   aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Wild Amazon (0200507V, NSW/ACT)",              MACHINE_FLAGS, layout_aristmk5 )  // 506/8, A - 10/10/96
GAMEL( 1996, wamazonv,  wamazon,  aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Wild Amazon (01J01996, Venezuela)",            MACHINE_FLAGS, layout_aristmk5 )  // 506/8, A - 10/10/96
GAMEL( 1996, wildbill,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Wild Bill (0100297V, NSW/ACT)",                MACHINE_FLAGS, layout_aristmk5 )  // 543/8, C - 15/08/96
GAMEL( 1996, wcougar,   aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Wild Cougar (0100167V, NSW/ACT)",              MACHINE_FLAGS, layout_aristmk5 )  // 569/9, B - 27/2/96
GAMEL( 1997, wcougaru,  wcougar,  aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Wild Cougar (NHG0296-04, US)",             MACHINE_FLAGS, layout_aristmk5_us )  // 569/8,    D - 19/05/97
GAMEL( 1996, wizways,   aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Wizard Ways (0200396V, NSW/ACT)",              MACHINE_FLAGS, layout_aristmk5 )  // 598/3, A - 04/11/96
GAMEL( 1997, wnpost,    aristmk5, aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Winning Post (RHG0418-04, US)",       MACHINE_FLAGS, layout_aristmk5_us )  // 541/2, G - 11/02/97
GAMEL( 1999, wthing,    aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Wild Thing (0101158V, NSW/ACT)",               MACHINE_FLAGS, layout_aristmk5 )  // 608/4, B - 14/12/99
GAMEL( 1999, wtiger,    aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "White Tiger Classic (0200954V, NSW/ACT)",      MACHINE_FLAGS, layout_aristmk5 )  // 638/1,    B - 08/07/99

// the following might be bad dumps or need different hardware (unconfirmed)
GAMEL( 1996, blackpnt,  aristmk5, aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Black Panther (0200818V, Victoria)",           MACHINE_FLAGS, layout_aristmk5 )  // 594/1, A - 30/07/96 - doesn't boot, 'needs VLC (Video Lottery Consultants) comms instead of QCOM' (or bad?)

// the following parent sets are known bad dumps, and do not boot (confirmed)
GAMEL( 1998, gambler,   aristmk5, aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "The Gambler (EHG0916-02, US)",             MACHINE_FLAGS, layout_aristmk5_us )  // MV4084/1, A - 30/10/98
GAMEL( 2001, koalamnt,  aristmk5, aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Koala Mint (CHG1573, US)",                 MACHINE_FLAGS, layout_aristmk5_us )  // MV4137,   A - 12/09/01
GAMEL( 1997, mgarden,   aristmk5, aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Magic Garden (AHG1211-99, US)",            MACHINE_FLAGS, layout_aristmk5_us )  // MV4033,   B - 10/02/97
GAMEL( 1997, trpdlght,  aristmk5, aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Tropical Delight (PHG0625-02, US)",        MACHINE_FLAGS, layout_aristmk5_us )  // 577/3,    D - 24/09/97
GAMEL( 1999, sbuk2,     aristmk5, aristmk5,     aristmk5,     aristmk5_state, aristmk5, ROT0, "Aristocrat", "Super Bucks II (0400501V, NSW/ACT)",       MACHINE_FLAGS, layout_aristmk5 )  // 578, G - 26/07/99
// the following clone sets are known bad dumps, and do not boot (confirmed)
GAMEL( 2001, adonisu,   adonis,   aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Adonis (BHG1508, US)",                     MACHINE_FLAGS, layout_aristmk5_us )  // MV4124/1, B - 31/07/01
GAMEL( 1999, bootsctnu, bootsctn, aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Boot Scootin' (GHG1012-02, US)",           MACHINE_FLAGS, layout_aristmk5_us )  // MV4098,   A - 25/08/99
GAMEL( 1997, bumblbugu, bumblbug, aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Bumble Bugs (CHG0479-03, US)",             MACHINE_FLAGS, layout_aristmk5_us )  // 593,      D - 05/07/97
GAMEL( 1996, cashchamu, cashcham, aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Cash Chameleon (DHG4078-99, US)",          MACHINE_FLAGS, layout_aristmk5_us )  // 603(a),   B - 06/12/96
GAMEL( 1997, eforsta5u, eforsta5, aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Enchanted Forest (JHG0415-03, US)",        MACHINE_FLAGS, layout_aristmk5_us )  // MV4033,   B - 10/02/97
GAMEL( 1997, goldpyra,  goldpyr,  aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Golden Pyramids (AHG1206-99, US)",         MACHINE_FLAGS, layout_aristmk5_us )  // 602/2,    B - 13/05/97
GAMEL( 2000, marmagicu, marmagic, aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Margarita Magic (EHG1559, US)",            MACHINE_FLAGS, layout_aristmk5_us )  // US003,    A - 07/07/00
GAMEL( 2001, partygrsa, partygrs, aristmk5_usa_touch, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Party Gras (BHG1284, US)",           MACHINE_FLAGS, layout_aristmk5_us )  // MV4115/3, B - 06/02/01
GAMEL( 1998, petshopa,  petshop,  aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Pet Shop (0100679V, NSW/ACT)",                 MACHINE_FLAGS, layout_aristmk5 )  // 618, A - 09/03/98
GAMEL( 1998, swhr2u,    swhr2,    aristmk5_usa, aristmk5_usa, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Sweethearts II (PHG0742-02, US)",          MACHINE_FLAGS, layout_aristmk5_us )  // MV4061,   A - 29/06/98
GAMEL( 1996, wamazona,  wamazon,  aristmk5,     aristmk5, aristmk5_state, aristmk5, ROT0, "Aristocrat", "Wild Amazon (0200285V, NSW/ACT)",              MACHINE_FLAGS, layout_aristmk5 )  // 506/6, A - 7/5/96
