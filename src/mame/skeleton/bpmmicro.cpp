// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/******************************************************************************
*       BPM Microsystems (formerly BP Microsystems, before 20060828)
          universal device programmers
*       Models supported in this driver so far: BP-1148/BP-1200

*       1000-series (non-ganged, manual feed) models:
*       EP-series:
*           EP-1 - 28 pin, eproms only? has adapter for 32 pin?
*           EP-1132 - 32 pin, eproms only?
*           EP-1140 - 40 pin, eproms only?
*       PLD-series:
*           PLD-1100 - ndip plds/pals only?
*           PLD-1128 - ndip plds/pals only?
*       286 based w/512k ram, fixed: (1992ish)
*           BP-1148 - fixed 512k ram, uses a special 'low cost' BP-1148 socket
              instead of a tech adapter+socket module
*           BP-1200 - fixed 512k ram, uses a TA-84 or TA-240 tech adapter plus
              a socket module, otherwise identical to above, same firmware
*       286 based w/240 pin tech adapter integrated as a "mezzanine board", expandable ram:
*           BP-1400/240 - uses a 30 OR 72 pin SIMM (some programmers may have
              the 30 pin SIMM socket populated) for up to 8MB? of ram
*           Silicon Sculptor - custom firmware locked to Actel fpga/pld [1400?]
              devices, may have a custom MB; drives 84 pins (no third connector)
              so probably "BP1400/84" based, i.e. a neutered BP-1400/240.
*           Silicon Sculptor 6X - as above but 6 programmers ganged together
*       486 based:
*           BP-1600 - 486DX4 100Mhz based, uses a 72 pin SIMM for up to 16MB of
              ram (does NOT support 32MB SIMMs!), supports 1.5vdd devices
*           Silicon Sculptor II - same as BP-1600 except it has the extra
              button and different firmware and a different mezzanine
              board/tech adapter; comes with a 72-pin SIMM installed
*       probably 'universal platform':
*           BP-2510
*       486+USB "6th Gen":
*           BP-1410 - 486DX4 based, uses a laptop SODIMM for up to 512MB? of ram?, has USB
*           BP-1610 - unclear what the difference to 1410 is
*           BP-1710 - same as BP1610, but two programmers ganged together in a single case
*           Silicon Sculptor III - 486DX4 100Mhz
*       There exist "7th" "8th" and "9th" gen programmers as well.

*       2000-series (ganged, manual programmers)
*       Unclear whether 286 or 486, all have extra button per programmer:
*           BP-2100/84x4 - four BP-1?00s ganged together [1400/84 based?]
*           BP-2200/240x2 - two BP-1?00s ganged together [1400/240 based?]
*           BP-2200/240x4 - four BP-1?00s ganged together
*           BP-2200/240x6 - six BP-1?00s ganged together
*           BP-2500/240x4 - four BP-1?00s ganged together [1600 based?]
*           BP-2000, BP-2600M - ganged programmers?

*
******************************************************************************
*       TODO:
*       Everything!
*       status LEDs
*       8-pin pin driver boards
*       tech adapter relays
*       Parallel port interface
*       Serial EEPROM (93c46) at U38 on BP1200 mainboard
*       Serial EEPROM (93c46) at (U1? U7? U12?) on TA-84 tech adapter
*       Serial EEPROM (93c46a) at ? on SM48D socket adapter
*       other socket adapters other than sm48d
*       DONE:
*       hardware pictures, cpu documentation, crystal, ram size
******************************************************************************
*       Links:
*       http://www3.bpmmicro.com/web/helpandsupport.nsf/69f301ee4e15195486256fcf0062c2eb/8194a48179484c9f862573220065d38e!OpenDocument
*       ftp://ftp.bpmmicro.com/Dnload/
******************************************************************************
*       Analog driver cards:
        The BP-1200, 1400 and 1600 have up to 6 of these cards in them.
        Each card can drive exactly 8 pins with analog (pwm-controlled?)
        voltages. The BP-1200 is probably usable with as few as one of these
        cards installed, but can only be used with 8-pin devices in that case!
******************************************************************************
*       Tech adapters for BP-1148 and BP-1200:
*       Note: Regardless of tech adapter, only up to 48 pins are drivable with
         analog (pseudo-dac-per-pin) voltages, the remainder are pulled high or
         low by the tech adapter.
*       TA-84: 84 pin tech adapter
         Rev C: Small board which doesn't cover the whole front of the BP-1200.
          no screen printing on the case, only identifiable by the pcb marking
          this PCB can be populated with either 48 or 84 relays; if the former,
          it is known as an STD48 pcb; the latter is presumably STD84 and may
          have an otherwise unpopulated PGA FPGA or ASIC on it as well.
         Rev E: Marked "CPCBTA84V", a larger board which covers the entire
          front of the BP-1200 including the LEDs, but has its own 3 LEDs on
          it (why not plastic light pipes?) controlled probably through the
          93c46 bus. This board again has either 48 relays on it, or 84 relays
          and an FPGA or ASIC on it.
*        TA-240: 240 pin tech adapter, this is a full sized shield which
          like the CPCBTA84V covers the entire front of the BP1200.
          It likely has even more relays in it, and it provides the same
          "three" connectors that the bp1400 and 1600 do natively, to allow
          for 240 pins to be driven. It almost certainly has an FPGA or ASIC
          on it as well, possibly several.
******************************************************************************
*       SM48D socket module:
*       The SM48D socket module has two DIN 41612/IEC 60603-2 sockets on the
          bottom, each of which has two rows of pins, with the middle "B" row
          unpopulated. It contains six 74HCT164 Serial-in Parallel-out shift
          registers, for 48 bits of serially drivable state, which is used to
          drive the gates of transistors which act as a pull-down to GND for
          each of the 48 pins.
          There is also a relay, of unknown purpose.
          There are several versions of the SM48D pcb which existed:
          Rev B uses all through-hole components, and extra passive resistor arrays for every pin (1992)
          Rev C uses all through-hole components (1992)
          Rev E "CPCBS48D" uses all surface mount/soic components (1998)
          For revision E, assuming IC1 is the upper leftmost IC, and IC7 is
          the lower rightmost ic, from left to right then top to bottom
          and ic4 being the 93c46;
          The 74hct164s are chained in this order and drive the following pins:
          IC3: TODO: ADD ME
          IC2: "
          IC7: "
          IC5: "
          IC6: "
          IC1: "

*       Looking "through" the pcb from the top, the connectors are arranged
          as such:
(note: J3 may be the wrong label, it could be J5)

           ___J3___                                             ___J4___
   GND -- |A32  C32|                                           |A32  C32| -- GND
   VCC -- |A31  C31|                                           |A31  C31| <> J3 A03
J4 C30 <> |A30  C30|                           ?Relay control? |A30  C30| <> J3 A30
   GND -- |A29  C29|                                           |A29  C29|
Pin 01 <> |A28  C28|                                           |A28  C28| <> Pin 48
Pin 02 <> |A27  C27|                                           |A27  C27| <> Pin 47
Pin 03 <> |A26  C26|                                           |A26  C26| <> Pin 46
Pin 04 <> |A25  C25|                                           |A25  C25| <> Pin 45
Pin 05 <> |A24  C24|                                           |A24  C24| <> Pin 44
Pin 06 <> |A23  C23| -- GND                             GND -- |A23  C23| <> Pin 43
Pin 07 <> |A22  C22|                                           |A22  C22| <> Pin 42
Pin 08 <> |A21  C21|                                           |A21  C21| <> Pin 41
Pin 09 <> |A20  C20|                                           |A20  C20| <> Pin 40
Pin 10 <> |A19  C19|                                           |A19  C19| <> Pin 39
Pin 11 <> |A18  C18|                                           |A18  C18| <> Pin 38
Pin 12 <> |A17  C17| -- GND                             GND -- |A17  C17| <> Pin 37
Pin 13 <> |A16  C16|                                           |A16  C16| <> Pin 36
Pin 14 <> |A15  C15|                                           |A15  C15| <> Pin 35
Pin 15 <> |A14  C14|                                           |A14  C14| <> Pin 34
Pin 16 <> |A13  C13|                                           |A13  C13| <> Pin 33
Pin 17 <> |A12  C12|                                           |A12  C12| <> Pin 32
Pin 18 <> |A11  C11| -- GND                             GND -- |A11  C11| <> Pin 31
Pin 19 <> |A10  C10|                                           |A10  C10| <> Pin 30
Pin 20 <> |A09  C09|                                           |A09  C09| <> Pin 29
Pin 21 <> |A08  C08|                                           |A08  C08| <> Pin 28
Pin 22 <> |A07  C07|                                     ?? <> |A07  C07| <> Pin 27
Pin 23 <> |A06  C06|                                           |A06  C06| <> Pin 26
Pin 24 <> |A05  C05|                                           |A05  C05| <> Pin 25
          |A04  C04|                               93c46 CS -> |A04  C04| -- VCC
J4 C31 <> |A03  C03|                              93c46 CLK -> |A03  C03|
J4 C02 <> |A02  C02| <- '164 CP   93c46 DI AND '164 IC3 DSB -> |A02  C02| <> J3 A02
   GND -- |A01  C01| <- '164 /MR                   93c46 DO <- |A01  C01| -- GND
          ----------                                           ----------

******************************************************************************/

/* Core includes */
#include "emu.h"
#include "cpu/i86/i286.h"
#include "machine/eepromser.h"


namespace {

class bpmmicro_state : public driver_device
{
public:
	bpmmicro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_eeprom_u38(*this, "eeprom_u38")
	{
	}

	void init_bp1200();
	void unknown_82200_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t latch_84000_r(offs_t offset, uint16_t mem_mask = ~0);
	void latch_84002_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void unknown_8400e_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void unknown_84018_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void unknown_8401a_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void eeprom_8401c_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	virtual void machine_start() override ATTR_COLD;
	void bpmmicro(machine_config &config);
	void i286_io(address_map &map) ATTR_COLD;
	void i286_mem(address_map &map) ATTR_COLD;
private:
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom_u38;
	uint16_t m_shifter = 0;
	uint16_t m_latch = 0;
};

/******************************************************************************
 Driver Init
******************************************************************************/

void bpmmicro_state::init_bp1200()
{
	m_shifter = 0;
	m_latch = 0;
}

/******************************************************************************
 Machine Start/Reset
******************************************************************************/

void bpmmicro_state::machine_start()
{
	save_item(NAME(m_shifter));
	save_item(NAME(m_latch));
}

/******************************************************************************
 Read/Write handlers
******************************************************************************/

void bpmmicro_state::unknown_82200_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: unknown write to 0x82200 offset %08x mask (%08x) data %08x\n",machine().describe_context(),offset<<1,mem_mask,data);
}

uint16_t bpmmicro_state::latch_84000_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t returnval = m_latch; // not sure this is correct, it could be that the 93c48 DO pin is connected directly to bit 7 here...
	logerror("%08x:Read 0x84000 octal latch %08x (%08x), got %08x\n", machine().describe_context(), offset << 1, mem_mask, returnval);
	return returnval;
}

void bpmmicro_state::latch_84002_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: write to 0x84002 octal latch clock? %08x mask (%08x) data %08x\n",machine().describe_context(),offset<<1,mem_mask,data);
	if (data) m_latch = m_shifter;
}

void bpmmicro_state::unknown_8400e_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: unknown write to 0x8400e offset %08x mask (%08x) data %08x\n",machine().describe_context(),offset<<1,mem_mask,data);
}

void bpmmicro_state::unknown_84018_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: unknown write to 0x84018 offset %08x mask (%08x) data %08x\n",machine().describe_context(),offset<<1,mem_mask,data);
	if (data&1) // HACK
	{
		m_eeprom_u38->cs_write(CLEAR_LINE);
		m_eeprom_u38->cs_write(ASSERT_LINE);
	}
}

void bpmmicro_state::unknown_8401a_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: unknown write to 0x8401a offset %08x mask (%08x) data %08x\n",machine().describe_context(),offset<<1,mem_mask,data);
}

void bpmmicro_state::eeprom_8401c_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s:write eeprom %08x (%08x) %08x\n",machine().describe_context(),offset<<1,mem_mask,data);
	if (ACCESSING_BITS_0_7)
	{
		m_eeprom_u38->di_write(BIT(data, 0));
		m_eeprom_u38->clk_write(BIT(data, 1) ? ASSERT_LINE : CLEAR_LINE);
		if (BIT(data,1)) // is this correct at all?
		{
			m_shifter <<= 1;
			m_shifter |= m_eeprom_u38->do_read();
		}
		/* bits 2 thru 7 here also do something;
		There is a 74HCT14 hex inverter schmitt trigger at u26
		and it is possible these 6 bits feed the inputs of that chip.
		These MIGHT be the CS lines for the 6 pin driver cards!
		 */
	}
}


/* todo (v1.24 rom)
from boot (2 digit hex offsets have 0x84000 added to them):
0x0a00 -> 0x82200
0x0001 -> 18
0x0003 -> 1a
0x0001 -> 0e
{lots of data banged to 1c}
0x0001 -> 02
0x0000 -> 02
read <- 00

0x0001 -> 18
0x0002 -> 1a
{lots of data banged to 1c}
0x0001 -> 02
0x0000 -> 02

0x0000 -> 18
0x0003 -> 1a
{lots of data banged to 1c}
0x0001 -> 02
0x0000 -> 02
read <- 00
read <- 00

0x0000 -> 18
0x0001 -> 1a
{lots of data banged to 1c}
0x0001 -> 02
0x0000 -> 02

0x0000 -> 0e
0x0001 -> 0e
0x0000 -> 0e
0x0001 -> 0e
0x0000 -> 0e
0x0001 -> 0e
read <- 00

0x0a00 -> 0x82200
0x0112 -> 0x82200

0x0000 -> 06
0x0000 -> 04
0x0000 -> 00

*/


/******************************************************************************
 Address Maps
******************************************************************************/

void bpmmicro_state::i286_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x07ffff).ram(); // 512k ram
	map(0x082200, 0x82201).w(FUNC(bpmmicro_state::unknown_82200_w));
	map(0x084000, 0x84001).r(FUNC(bpmmicro_state::latch_84000_r)); // GUESS: this is reading the octal latch
	map(0x084002, 0x84003).w(FUNC(bpmmicro_state::latch_84002_w)); // GUESS: this is clocking the CK pin on the octal latch from bit 0, dumping the contents of a serial in parallel out shifter into said latch
	map(0x08400e, 0x8400f).w(FUNC(bpmmicro_state::unknown_8400e_w));
	map(0x084018, 0x84019).w(FUNC(bpmmicro_state::unknown_84018_w));
	map(0x08401a, 0x8401b).w(FUNC(bpmmicro_state::unknown_8401a_w));
	map(0x08401c, 0x8401d).w(FUNC(bpmmicro_state::eeprom_8401c_w));
	map(0x0f0000, 0x0fffff).rom().region("bios", 0x10000);
	//map(0xfe0000, 0xffffff).rom().region("bios", 0); //?
	map(0xfffff0, 0xffffff).rom().region("bios", 0x1fff0); //?
}

void bpmmicro_state::i286_io(address_map &map)
{
	map.unmap_value_high();
}


/******************************************************************************
 Input Ports
******************************************************************************/
static INPUT_PORTS_START( bpmmicro )
INPUT_PORTS_END

/******************************************************************************
 Machine Drivers
******************************************************************************/

void bpmmicro_state::bpmmicro(machine_config &config)
{
	/* basic machine hardware */
	I80286(config, m_maincpu, XTAL(32'000'000)/4); /* divider is guessed, cpu is an AMD N80L286-16/S part */
	m_maincpu->set_addrmap(AS_PROGRAM, &bpmmicro_state::i286_mem);
	m_maincpu->set_addrmap(AS_IO, &bpmmicro_state::i286_io);

	EEPROM_93C46_16BIT(config, "eeprom_u38");
}



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(bp1200)
	ROM_REGION16_LE(0x20000, "bios", 0)
	// note about roms: the BP-1200 has two jumpers controlling what type of rom is installed;
	// it needs 120ns or faster roms
	// the "W1" and "W2" labels are next to pins A on the pcb
	// for 2764 roms:
	// W1 [A B]C
	// W2 [A B]C
	// for 27256 roms:
	// W1 [A B]C
	// W2  A[B C]
	ROM_DEFAULT_BIOS("v124")
	ROM_SYSTEM_BIOS( 0, "v124", "BP-1200 V1.24")
	ROMX_LOAD("version_1.24_u8.st_m27c256b-12f1l.u8", 0x10001, 0x8000, CRC(86d46d76) SHA1(4733b03a28689a3d2c58278495fbf31d0c74ac01), ROM_SKIP(1) | ROM_BIOS(0)) // "bios1200.v124a.u8" on bpmmicro site
	ROMX_LOAD("version_1.24_u9.st_m27c256b-12f1l.u9", 0x10000, 0x8000, CRC(3bcc5c72) SHA1(3b281f2b464d8a4e366f8e2f0a8fa6dfd0a8f28c), ROM_SKIP(1) | ROM_BIOS(0)) // "bios1200.v124a.u9" on bpmmicro site
	ROM_SYSTEM_BIOS( 1, "v118", "BP-1200 V1.18")
	ROMX_LOAD("version_1.18_u8_2000.am27c256.u8", 0x10001, 0x8000, CRC(f8afa614) SHA1(a372bc35aea30595ab8f05c5e641021b45043ed3), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("version_1.18_u9_2000.am27c256.u9", 0x10000, 0x8000, CRC(049b2ad1) SHA1(c9405ff805f3814493ad007bae7a8cb6a12aeb32), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "v113", "BP-1200 V1.13")
	ROMX_LOAD("bp-1200_v1.13_u8_1992_1997.at27c256r-12pc.u8", 0x10001, 0x8000, CRC(ec61dcad) SHA1(dbfee285456d24b93c1fa6e8557b13ab80c3c877), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("bp-1200_v1.13_u9_1992_1995.at27c256r-12pc.u9", 0x10000, 0x8000, CRC(91ca5e70) SHA1(4a8c1894a67dfd5e0db088519a3ee4edaafdef58), ROM_SKIP(1) | ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 3, "v111", "BP-1200 V1.11")
	ROMX_LOAD("bp-1200_version_1.11_u8_1992_1995.at27c256r-12pc.u8", 0x10001, 0x8000, CRC(d1c051e4) SHA1(b27007a931b0662b3dc7e2d41c6ec5ed0cd49308), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD("bp-1200_version_1.11_u9_1992_1995.at27c256r-12pc.u9", 0x10000, 0x8000, CRC(99d46ba1) SHA1(144dbe6ed989ea88cfc1f6d1142508bb92519f87), ROM_SKIP(1) | ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 4, "v105", "BP-1200 V1.05")
	ROMX_LOAD("bp-1200_1.05_u8__=c=_1993_rev_c__bp_microsystems.27c64-12pc.u8", 0x1c001, 0x2000, CRC(2c13a43c) SHA1(5dd67a09f72f693c085160b9beedd2454ba4ec37), ROM_SKIP(1) | ROM_BIOS(4)) // "BP-1200 1.05 U8 // (C) 1993 REV C // BP Microsystems" 27C64-12PC @U8
	ROMX_LOAD("bp-1200_1.05_u9__=c=_1993_rev_c__bp_microsystems.27c64-12pc.u9", 0x1c000, 0x2000, CRC(b88a311c) SHA1(fb5e0543c811cbbf8f24d1de204b4c0c1bd2f485), ROM_SKIP(1) | ROM_BIOS(4)) // "BP-1200 1.05 U9 // (C) 1993 REV C // BP Microsystems" 27C64-12PC @U9
	ROM_SYSTEM_BIOS( 5, "v104", "BP-1200 V1.04")
	ROMX_LOAD("bp-1200_1.04_u8__=c=_1992_rev_c__bp_microsystems.27c64.u8", 0x1c001, 0x2000, CRC(2ab47324) SHA1(052e578dae5db023f94b35d686a5352ffceec414), ROM_SKIP(1) | ROM_BIOS(5)) // "BP-1200 1.04 U8 // (C) 1992 REV C // BP Microsystems" OTP 27C64 @ U8
	ROMX_LOAD("bp-1200_1.04_u9__=c=_1993_rev_c__bp_microsystems.27c64.u9", 0x1c000, 0x2000, CRC(17b94d7a) SHA1(7ceed660dbdc638ac86ca8ba7fa456c297d88766), ROM_SKIP(1) | ROM_BIOS(5)) // "BP-1200 1.04 U9 // (C) 1993 REV C // BP Microsystems" OTP 27C64 @ U9
	ROM_SYSTEM_BIOS( 6, "v103", "BP-1200 V1.03")
	ROMX_LOAD("bp-1200_1.03_u8__=c=_1992_rev_c__bp_microsystems.27c64a-12.u8", 0x1c001, 0x2000, CRC(b01968b6) SHA1(d0c6aa0f0fe47b0915658e8c27286ab6ea90972e), ROM_SKIP(1) | ROM_BIOS(6)) // "BP-1200 1.03 U8 // (C) 1992 REV C // BP Microsystems" 27C64A-12 @ U8
	ROMX_LOAD("bp-1200_1.03_u9__=c=_1992_rev_c__bp_microsystems.27c64a-12.u9", 0x1c000, 0x2000, CRC(f58ffebb) SHA1(700d3ffed269fff6dc1cf2190bde8b989715c22a), ROM_SKIP(1) | ROM_BIOS(6)) // "BP-1200 1.03 U9 // (C) 1992 REV C // BP Microsystems" 27C64A-12 @ U9
ROM_END

} // anonymous namespace


/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT         COMPANY            FULLNAME   FLAGS
COMP( 1992, bp1200, 0,      0,      bpmmicro, bpmmicro, bpmmicro_state, init_bp1200, "BP Microsystems", "BP-1200", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
