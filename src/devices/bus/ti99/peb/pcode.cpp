// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 P-Code Card emulation.

    The P-Code card is part of the UCSD p-System support for the TI-99
    computer family. This system is a comprehensive development system for
    creating, running, and debugging programs written in UCSD Pascal.

    The complete system consists of
    - P-Code card, plugged into the Peripheral Expansion Box (PEB)
    - Software on disk:
      + PHD5063: UCSD p-System Compiler
      + PHD5064: UCSD p-System Assembler/Linker
      + PHD5065: UCSD p-System Editor/Filer (2 disks)

    The card has a switch on the circuit board extending outside the PEB
    which allows to turn off the card without removing it. Unlike other
    expansion cards for the TI system, the P-Code card immediately takes
    over control after the system is turned on.

    When the p-System is booted, the screen turns cyan and remains empty.
    There are two beeps, a pause for about 15 seconds, another three beeps,
    and then a welcome text is displayed with a one-line menu at the screen
    top. (Delay times seem unrealistically short; the manual says
    30-60 seconds. To be checked.)
    Many of the functions require one of the disks be inserted in one
    of the disk drives. You can leave the p-System by waiting for the menu
    to appear, and typing H (halt). This returns you to the Master Title
    Screen, and the card is inactive until the system is reset.

    The P-Code card contains the P-Code interpreter which is somewhat
    comparable to today's Java virtual machine. Programs written for the
    p-System are interchangeable between different platforms.

    On the P-Code card we find 12 KiB of ROM, visible in the DSR memory area
    (>4000 to >5FFF). The first 4 KiB (>4000->4FFF) are from the 4732 ROM,
    the second and third 4 KiB (>5000->5FFF) are from a 4764 ROM, switched
    by setting the CRU bit 4 to 1 on the CRU base >1F00.

    CRU base >1F00
        Bit 0: Activate card
        Bit 4: Select bank 2 of the 4764 ROM (0 = bank 1)
        Bit 7: May be connected to an indicator LED which is by default
               wired to bit 0 (on the PCB)

    The lines are used in a slightly uncommon way: the three bits of the
    CRU bit address are A8, A13, and A14 (A15=LSB). Hence, bit 4 is at
    address >1F80, and bit 7 is at address >1F86. These bits are purely
    write-only.

    Moreover, the card contains 48 KiB of GROM memory, occupying the address
    space from G>0000 to G>FFFF in portions of 6KiB at every 8KiB boundary.

    Another specialty of the card is that the GROM contents are accessed via
    another GROM base address than what is used in the console:
    - >5BFC = read GROM data
    - >5BFE = read GROM address
    - >5FFC = write GROM data
    - >5FFE = write GROM address

    This makes the GROM memory "private" to the card; together with the
    rest of the ROM space the ports become invisible when the card is
    deactivated.

    Michael Zapf

    July 2009: First version
    September 2010: Rewritten as device
    February 2012: Rewritten as class

*****************************************************************************/

#include "emu.h"
#include "pcode.h"

#define LOG_WARN        (1U<<1)   // Warnings
#define LOG_CONFIG      (1U<<2)   // Configuration
#define LOG_ROM         (1U<<3)
#define LOG_GROM        (1U<<4)
#define LOG_SWITCH      (1U<<5)
#define LOG_CRU         (1U<<6)

#define VERBOSE ( LOG_CONFIG | LOG_WARN )

#include "logmacro.h"

DEFINE_DEVICE_TYPE(TI99_P_CODE, bus::ti99::peb::ti_pcode_card_device, "ti99_pcode", "TI-99 P-Code Card")

namespace bus::ti99::peb {

#define PCODE_GROM_TAG "pcode_grom"
#define PCODE_ROM_TAG "pcode_rom"

#define ACTIVE_TAG "ACTIVE"

#define CRU_BASE 0x1f00

ti_pcode_card_device::ti_pcode_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TI99_P_CODE, tag, owner, clock),
	device_ti99_peribox_card_interface(mconfig, *this),
	m_groms(*this, "grom%u", 0U),
	m_crulatch(*this, "crulatch"),
	m_rom(nullptr),
	m_bank_select(0),
	m_active(false),
	m_clock_count(0),
	m_clockhigh(false),
	m_inDsrArea(false),
	m_isrom0(false),
	m_isrom12(false),
	m_isgrom(false),
	m_address(0)
{
}

void ti_pcode_card_device::setaddress_dbin(offs_t offset, int state)
{
	m_address = offset;
	m_inDsrArea = in_dsr_space(offset, true);

	line_state a14 = ((m_address & 2)!=0)? ASSERT_LINE : CLEAR_LINE;

	m_isrom0 = ((m_address & 0xf000)==0x4000);
	m_isrom12 = ((m_address & 0xf000)==0x5000);

	// Valid access (GROM write with DBIN=0 or read with DBIN=1)
	bool validaccess = (state==CLEAR_LINE || (m_address & 0x0400)==0);

	// GROM access  0101 1011 1111 1100
	m_isgrom = ((m_address & 0xfbfd)==0x5bfc) && validaccess;

	if (validaccess)
	{
		// always deliver to GROM so that the select line may be cleared
		line_state mline = (state!=0)? ASSERT_LINE : CLEAR_LINE;
		line_state gsq = m_isgrom? ASSERT_LINE : CLEAR_LINE;

		for (auto &grom : m_groms)
			grom->set_lines(mline, a14, gsq);
	}
}

void ti_pcode_card_device::debugger_read(offs_t offset, uint8_t& value)
{
	// The debuger does not call setaddress
	if (m_active && in_dsr_space(offset, true))
	{
		bool isrom0 = ((offset & 0xf000)==0x4000);
		bool isrom12 = ((offset & 0xf000)==0x5000);
		if (isrom0) value = m_rom[offset & 0x0fff];
		else
			if (isrom12) value = m_rom[(m_bank_select<<12) | (offset & 0x0fff)];
	}
}

void ti_pcode_card_device::readz(offs_t offset, uint8_t *value)
{
	// Care for debugger
	if (machine().side_effects_disabled())
	{
		debugger_read(offset, *value);
		return;
	}

	if (m_active && m_inDsrArea && m_selected)
	{
		if (m_isrom0)
		{
			*value = m_rom[m_address & 0x0fff];
			LOGMASKED(LOG_ROM, "Read from rom %04x: %02x\n", offset&0xffff, *value);
		}
		else
		{
			if (m_isgrom)
			{
				for (auto &grom : m_groms)
					grom->readz(value);
				LOGMASKED(LOG_GROM, "Read from grom %04x: %02x\n", m_address&0xffff, *value);
			}
			else
			{
				if (m_isrom12)
				{
					// Accesses ROM 4764 (2*4K)
					// We have two banks here which are activated according
					// to the setting of CRU bit 4
					// Bank 0 is the ROM above
					// 0001 xxxx xxxx xxxx   Bank 1
					// 0010 xxxx xxxx xxxx   Bank 2
					*value = m_rom[(m_bank_select<<12) | (m_address & 0x0fff)];
					LOGMASKED(LOG_ROM, "Read from rom %04x (%02x): %02x\n", m_address&0xffff, m_bank_select, *value);
				}
			}
		}
	}
}

/*
    Write a byte in P-Code ROM space. This is only used for setting the
    GROM address.
*/
void ti_pcode_card_device::write(offs_t offset, uint8_t data)
{
	if (machine().side_effects_disabled()) return;
	if (m_active && m_isgrom && m_selected)
	{
		for (auto &grom : m_groms)
			grom->write(data);
	}
}

/*
    Common READY* line from the GROMs.
*/
WRITE_LINE_MEMBER( ti_pcode_card_device::ready_line )
{
	m_slot->set_ready(state);
}

/*
    CLKOUT line from the CPU. This line is divided by 8 to generate a 375 Khz
    clock input for the GROMs, which are thus running at a lower rate than
    those in the console driven by the VDP (477 kHz).
*/
WRITE_LINE_MEMBER( ti_pcode_card_device::clock_in)
{
	m_clock_count = (m_clock_count+1) & 0x03;  // four pulses high, four pulses low
	if (m_clock_count==0)
	{
		// Toggle
		m_clockhigh = !m_clockhigh;
		for (auto &grom : m_groms)
			grom->gclock_in(m_clockhigh ? ASSERT_LINE : CLEAR_LINE);
	}
}

/*
    CRU read handler. The P-Code card does not offer CRU read lines, so
    we just ignore any request. (Note that CRU lines are not like memory; you
    may be able to write to them, but not necessarily read them again.)
*/
void ti_pcode_card_device::crureadz(offs_t offset, uint8_t *value)
{
}

/*
    The CRU write handler.
    Bit 0 = activate card
    Bit 4 = select second bank of high ROM.

    Somewhat uncommon, the CRU address is created from address lines
    A8, A13, and A14 so bit 0 is at 0x1f00, but bit 4 is at 0x1f80. Accordingly,
    bit 7 would be 0x1f86 but it is not used.
*/
void ti_pcode_card_device::cruwrite(offs_t offset, uint8_t data)
{
	if ((offset & 0xff00)==CRU_BASE)
		m_crulatch->write_bit((offset & 0x80) >> 5 | (offset & 0x06) >> 1, data);
}

WRITE_LINE_MEMBER(ti_pcode_card_device::pcpage_w)
{
	m_selected = state;
}

WRITE_LINE_MEMBER(ti_pcode_card_device::ekrpg_w)
{
	m_bank_select = state ? 2 : 1;   // we're calling this bank 1 and bank 2
	LOGMASKED(LOG_CRU, "Select rom bank %d\n", m_bank_select);
}

void ti_pcode_card_device::device_start()
{
	m_rom = memregion(PCODE_ROM_TAG)->base();

	save_item(NAME(m_bank_select));
	save_item(NAME(m_active));
	save_item(NAME(m_clock_count));
	save_item(NAME(m_clockhigh));
	save_item(NAME(m_inDsrArea));
	save_item(NAME(m_isrom0));
	save_item(NAME(m_isrom12));
	save_item(NAME(m_isgrom));
	save_item(NAME(m_address));
}

void ti_pcode_card_device::device_reset()
{
	m_bank_select = 1;
	m_selected = false;
	m_clock_count = 0;
	m_clockhigh = false;

	m_active = ioport(ACTIVE_TAG)->read();

	m_isrom0 = false;
	m_isrom12 = false;
	m_isgrom = false;
	m_address = 0;
}

void ti_pcode_card_device::device_config_complete()
{
}

INPUT_CHANGED_MEMBER( ti_pcode_card_device::switch_changed )
{
	LOGMASKED(LOG_SWITCH, "Switch changed to %d\n", newval);
	m_active = (newval != 0);
}


INPUT_PORTS_START( ti99_pcode )
	PORT_START( ACTIVE_TAG )
	PORT_DIPNAME( 0x01, 0x00, "P-Code activation switch" ) PORT_CHANGED_MEMBER(DEVICE_SELF, ti_pcode_card_device, switch_changed, 0)
		PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
		PORT_DIPSETTING( 0x01, DEF_STR( On ) )
INPUT_PORTS_END

ROM_START( ti99_pcode )
	ROM_REGION(0x10000, PCODE_GROM_TAG, 0)
	// The order of the GROMs with respect to the socket number is not guaranteed to be correct
	// as all GROMs are connected in parallel and dumped in-system
	ROM_LOAD("pcode_grom0.u11", 0x0000, 0x1800, CRC(505e5df0) SHA1(66911fba7599c64981180f8a673581f4b05941ff))
	ROM_LOAD("pcode_grom1.u13", 0x2000, 0x1800, CRC(63b546d5) SHA1(3d830c8bdac102275ec0702eff1ebf4b67484f52))
	ROM_LOAD("pcode_grom2.u14", 0x4000, 0x1800, CRC(28821e5c) SHA1(c147bd5d8d624caa690284bfc253c6699e3518d4))
	ROM_LOAD("pcode_grom3.u16", 0x6000, 0x1800, CRC(1db4a4a5) SHA1(f7a0ba8050f00ccc1ee328c66df5cc4269748ced))
	ROM_LOAD("pcode_grom4.u19", 0x8000, 0x1800, CRC(9618eb9b) SHA1(1f223f3febcb93e648cefe49c83bfeac802be9d6))
	ROM_LOAD("pcode_grom5.u20", 0xa000, 0x1800, CRC(c47efe6d) SHA1(f5b56c7de1cb1e7345a0716d35f00a3a9722febe))
	ROM_LOAD("pcode_grom6.u21", 0xc000, 0x1800, CRC(06a34c93) SHA1(56172c56afa3868f2098328f81881022230d949d))
	ROM_LOAD("pcode_grom7.u22", 0xe000, 0x1800, CRC(a09ca8d9) SHA1(2ea33d875f9c8e7c00df023a0d8d4461d50f0a87))

	ROM_REGION(0x3000, PCODE_ROM_TAG, 0)
	ROM_LOAD("pcode_rom0.u1", 0x0000, 0x1000, CRC(3881d5b0) SHA1(a60e0468bb15ff72f97cf6e80979ca8c11ed0426)) /* TI P-Code card rom4732 */
	ROM_LOAD("pcode_rom1.u18", 0x1000, 0x2000, CRC(46a06b8b) SHA1(24e2608179921aef312cdee6f455e3f46deb30d0)) /* TI P-Code card rom4764 */
ROM_END

void ti_pcode_card_device::device_add_mconfig(machine_config &config)
{
	for (unsigned i = 0; m_groms.size() > i; ++i)
		TMC0430(config, m_groms[i], PCODE_GROM_TAG, 0x2000 * i, i).ready_cb().set(FUNC(ti_pcode_card_device::ready_line));

	LS259(config, m_crulatch); // U12
	m_crulatch->q_out_cb<0>().set(FUNC(ti_pcode_card_device::pcpage_w));
	m_crulatch->q_out_cb<4>().set(FUNC(ti_pcode_card_device::ekrpg_w));
}

const tiny_rom_entry *ti_pcode_card_device::device_rom_region() const
{
	return ROM_NAME( ti99_pcode );
}

ioport_constructor ti_pcode_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ti99_pcode );
}

} // end namespace bus::ti99::peb
