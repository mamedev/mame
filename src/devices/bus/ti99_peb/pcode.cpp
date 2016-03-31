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

#include "pcode.h"

#define PCODE_GROM_TAG "pcode_grom"
#define PCODE_ROM_TAG "pcode_rom"

#define PGROM0_TAG "grom0"
#define PGROM1_TAG "grom1"
#define PGROM2_TAG "grom2"
#define PGROM3_TAG "grom3"
#define PGROM4_TAG "grom4"
#define PGROM5_TAG "grom5"
#define PGROM6_TAG "grom6"
#define PGROM7_TAG "grom7"

#define GROMMASK 0x1ffd
#define GROMREAD 0x1bfc
#define GROMWRITE 0x1ffc

#define ACTIVE_TAG "ACTIVE"

#define TRACE_ROM 0
#define TRACE_GROM 0
#define TRACE_CRU 0
#define TRACE_SWITCH 0

ti_pcode_card_device::ti_pcode_card_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: ti_expansion_card_device(mconfig, TI99_P_CODE, "TI-99 P-Code Card", tag, owner, clock, "ti99_pcode", __FILE__),
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

SETADDRESS_DBIN_MEMBER( ti_pcode_card_device::setaddress_dbin )
{
	// Do not allow setaddress for the debugger. It will mess up the
	// setaddress/memory access pairs when the CPU enters wait states.
	if (space.debugger_access()) return;

	m_address = offset;
	m_inDsrArea = ((m_address & m_select_mask)==m_select_value);

	line_state a14 = ((m_address & 2)!=0)? ASSERT_LINE : CLEAR_LINE;

	m_isrom0 = ((m_address & 0xf000)==0x4000);
	m_isrom12 = ((m_address & 0xf000)==0x5000);

	// Valid access (GROM write with DBIN=0 or read with DBIN=1)
	bool validaccess = (state==CLEAR_LINE || (m_address & 0x0400)==0);

	// GROM access  0101 1011 1111 1100
	m_isgrom = ((m_address & 0xfbfd)==0x5bfc) && validaccess;

	if (validaccess)
	{
		int lines = (state==ASSERT_LINE)? 1 : 0;
		if (a14==ASSERT_LINE) lines |= 2;
		line_state select = m_isgrom? ASSERT_LINE : CLEAR_LINE;

		// always deliver to GROM so that the select line may be cleared
		for (int i=0; i < 8; i++)
			m_grom[i]->set_lines(space, lines, select);
	}
}

READ8Z_MEMBER( ti_pcode_card_device::readz )
{
	if (m_active && m_inDsrArea && m_selected)
	{
		if (m_isrom0)
		{
			*value = m_rom[m_address & 0x0fff];
			if (TRACE_ROM) logerror("Read from rom %04x: %02x\n", offset&0xffff, *value);
		}
		else
		{
			if (m_isgrom)
			{
				for (auto& elem : m_grom) elem->readz(space, m_address, value);
				if (TRACE_GROM) logerror("Read from grom %04x: %02x\n", m_address&0xffff, *value);
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
					if (TRACE_ROM) logerror("Read from rom %04x (%02x): %02x\n", m_address&0xffff, m_bank_select, *value);
				}
			}
		}
	}
}

/*
    Write a byte in P-Code ROM space. This is only used for setting the
    GROM address.
*/
WRITE8_MEMBER( ti_pcode_card_device::write )
{
	if (m_active && m_isgrom && m_selected)
	{
		for (auto & elem : m_grom) elem->write(space, m_address, data);
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
		for (auto & elem : m_grom) elem->gclock_in(m_clockhigh? ASSERT_LINE : CLEAR_LINE);
	}
}

/*
    CRU read handler. The P-Code card does not offer CRU read lines, so
    we just ignore any request. (Note that CRU lines are not like memory; you
    may be able to write to them, but not necessarily read them again.)
*/
READ8Z_MEMBER(ti_pcode_card_device::crureadz)
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
WRITE8_MEMBER(ti_pcode_card_device::cruwrite)
{
	if ((offset & 0xff00)==m_cru_base)
	{
		int addr = offset & 0x00ff;

		if (addr==0)
			m_selected = (data != 0);

		if (addr==0x80) // Bit 4 is on address line 8
		{
			m_bank_select = (data+1);   // we're calling this bank 1 and bank 2
			if (TRACE_CRU) logerror("Select rom bank %d\n", m_bank_select);
		}
	}
}

void ti_pcode_card_device::device_start()
{
	m_cru_base = 0x1f00;
	m_grom[0] = downcast<tmc0430_device*>(subdevice(PGROM0_TAG));
	m_grom[1] = downcast<tmc0430_device*>(subdevice(PGROM1_TAG));
	m_grom[2] = downcast<tmc0430_device*>(subdevice(PGROM2_TAG));
	m_grom[3] = downcast<tmc0430_device*>(subdevice(PGROM3_TAG));
	m_grom[4] = downcast<tmc0430_device*>(subdevice(PGROM4_TAG));
	m_grom[5] = downcast<tmc0430_device*>(subdevice(PGROM5_TAG));
	m_grom[6] = downcast<tmc0430_device*>(subdevice(PGROM6_TAG));
	m_grom[7] = downcast<tmc0430_device*>(subdevice(PGROM7_TAG));
	m_rom = memregion(PCODE_ROM_TAG)->base();
}

void ti_pcode_card_device::device_reset()
{
	if (m_genmod)
	{
		m_select_mask = 0x1fe000;
		m_select_value = 0x174000;
	}
	else
	{
		m_select_mask = 0x7e000;
		m_select_value = 0x74000;
	}
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
	if (TRACE_SWITCH) logerror("Switch changed to %d\n", newval);
	m_active = (newval != 0);
}


MACHINE_CONFIG_FRAGMENT( ti99_pcode )
	MCFG_GROM_ADD( PGROM0_TAG, 0, PCODE_GROM_TAG, 0x0000, WRITELINE(ti_pcode_card_device, ready_line))
	MCFG_GROM_ADD( PGROM1_TAG, 1, PCODE_GROM_TAG, 0x2000, WRITELINE(ti_pcode_card_device, ready_line))
	MCFG_GROM_ADD( PGROM2_TAG, 2, PCODE_GROM_TAG, 0x4000, WRITELINE(ti_pcode_card_device, ready_line))
	MCFG_GROM_ADD( PGROM3_TAG, 3, PCODE_GROM_TAG, 0x6000, WRITELINE(ti_pcode_card_device, ready_line))
	MCFG_GROM_ADD( PGROM4_TAG, 4, PCODE_GROM_TAG, 0x8000, WRITELINE(ti_pcode_card_device, ready_line))
	MCFG_GROM_ADD( PGROM5_TAG, 5, PCODE_GROM_TAG, 0xa000, WRITELINE(ti_pcode_card_device, ready_line))
	MCFG_GROM_ADD( PGROM6_TAG, 6, PCODE_GROM_TAG, 0xc000, WRITELINE(ti_pcode_card_device, ready_line))
	MCFG_GROM_ADD( PGROM7_TAG, 7, PCODE_GROM_TAG, 0xe000, WRITELINE(ti_pcode_card_device, ready_line))
MACHINE_CONFIG_END

INPUT_PORTS_START( ti99_pcode )
	PORT_START( ACTIVE_TAG )
	PORT_DIPNAME( 0x01, 0x00, "P-Code activation switch" ) PORT_CHANGED_MEMBER(DEVICE_SELF, ti_pcode_card_device, switch_changed, nullptr)
		PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
		PORT_DIPSETTING( 0x01, DEF_STR( On ) )
INPUT_PORTS_END

ROM_START( ti99_pcode )
	ROM_REGION(0x10000, PCODE_GROM_TAG, 0)
	ROM_LOAD("pcode_g0.bin", 0x0000, 0x10000, CRC(541b3860) SHA1(7be77c216737334ae997753a6a85136f117affb7)) /* TI P-Code card groms */
	ROM_REGION(0x3000, PCODE_ROM_TAG, 0)
	ROM_LOAD("pcode_r0.bin", 0x0000, 0x1000, CRC(3881d5b0) SHA1(a60e0468bb15ff72f97cf6e80979ca8c11ed0426)) /* TI P-Code card rom4732 */
	ROM_LOAD("pcode_r1.bin", 0x1000, 0x2000, CRC(46a06b8b) SHA1(24e2608179921aef312cdee6f455e3f46deb30d0)) /* TI P-Code card rom4764 */
ROM_END

machine_config_constructor ti_pcode_card_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ti99_pcode );
}

const rom_entry *ti_pcode_card_device::device_rom_region() const
{
	return ROM_NAME( ti99_pcode );
}

ioport_constructor ti_pcode_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ti99_pcode );
}

const device_type TI99_P_CODE = &device_creator<ti_pcode_card_device>;
