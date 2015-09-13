// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************

    TI-99/8 main board logic

    This component implements the address decoder and mapper logic from the
    TI-99/8 console.

    The TI-99/8 defines a "logical address map" with 64 KiB (according to the
    16 address bits) and a "physical address map" with 16 MiB (according to the
    24 address bits of the mapper). Note that the mapper only uses 16 outgoing
    address lines and multiplexes the address bytes.

    Note: The TI-99/8's internal codename was "Armadillo"

==============================
    Mapper (codename "Amigo")
==============================

    Initial setting of mapper (as defined in the power-up routine, TI-99/4A mode)

    0   00ff0000 -> Unmapped; logical address 0000...0fff = ROM0
    1   00ff0000 -> Unmapped; logical address 1000...1fff = ROM0
    2   00000800 -> DRAM; 2000 = 000800, 2fff = 0017ff
    3   00001800 -> DRAM; 3000 = 001800, 3fff = 0027ff
    4   00ff4000 -> DSR space (internal / ioport)
    5   00ff5000 -> DSR space (internal / ioport)
    6   00ff6000 -> Cartridge space (6000..6fff)
    7   00ff7000 -> Cartridge space (7000..7fff)
    8   00ff0000 -> Unmapped; device ports (VDP) and SRAM
    9   00ff0000 -> Unmapped; device ports (Speech, GROM)
    A   00002800 -> DRAM; a000 = 002800, afff = 0037ff
    B   00003800 -> DRAM; b000 = 003800, bfff = 0047ff
    C   00004800 -> DRAM; c000 = 004800, cfff = 0057ff
    D   00005800 -> DRAM; d000 = 005800, dfff = 0067ff
    E   00006800 -> DRAM; e000 = 006800, efff = 0077ff
    F   00007800 -> DRAM; f000 = 007800, ffff = 0087ff

    Format of map table entry (not emulated)

    +------+------+------+------+---+---+---+---------+----------+---------+
    | WProt| XProt| RProt|  *   | 0 | 0 | 0 |  Upper  |   High   |   Low   |
    +------+------+------+------+---+---+---+---------+----------+---------+

    WProt: Write protection if set to 1
    XProt: Execute protection if set to 1
    RProt: Read protection if set to 1

    When a protection violation occurs, the tms9901 INT1* pin is pulled low
    (active).  The pin remains low until the mapper status register is read.

    Address handling
    ----------------
    Physical address is (Upper * 2^16) + (High * 2^8) + Low

    The mapper calculates the actual physical address by looking up the
    table entry from the first four bits of the logical address and then
    *adding* the remaining 12 bits of the logical address on the map value.

    The value 0xff0000 is used to indicate a non-mapped area.

    Mapper control register
    -----------------------
    The mapper control register is used to initiate a map load/save operation.

    +---+---+---+---+---+---+---+---+
    | 0 | 0 | 0 | 0 | Map File  | RW|
    +---+---+---+---+---+---+---+---+

    The map file is a number from 0-7 indicating the set of map values for the
    operation, which means the location in SRAM where the next 64 values are
    loaded from or stored into.

    RW = 1: load from SRAM into mapper
    RW = 0: store from mapper into SRAM

    When read, the mapper register returns the violation flags:
    +------+------+------+---+---+---+---+---+
    | WProt| XProt| RProt| 0 | 0 | 0 | 0 | 0 |
    +------+------+------+---+---+---+---+---+

    Logical address space (LAS)
    ===========================
    The LAS is the address space as seen by the TMS 9995 CPU. It is 64 KiB large.
    The LAS can be configured in two ways:
    - the native (99/8) mode
    - and the compatibility mode (99/4A)

    Both modes are selected by CRU bit 20 on base 0000 (named "CRUS").

    The console starts up in compatibility mode.

    The compatibility mode organizes the LAS in a similar way as the TI-99/4A.
    This means that machine language programs should run with no or only minor
    changes. In particular, game cartridges work without problems.

    The native mode rearranges the address space and puts memory-mapped devices
    to other positions.

    TI-99/4A compatibility mode (CRUS=1)
    ------------------------------------
    0000-1fff: 2 KiB ROM0
    2000-7fff: Free area
    8000-87ff: 2 KiB SRAM
      8000-81ff: mapper files (8 files with 16*4 bytes each)
      8200-82ff: Free RAM
      8300-83ff: Scratch-pad RAM as in the 99/4A
      8400-840f: Sound chip
    8800-880f: VDP read port (data, status)
    8810-881f: Mapper access port
    8820-8bff: Free area
    8c00-8c0f: VDP write port (data, address)
    8c10-8fff: Free area
    9000-900f: Speech synthesizer read (on-board)
    9010-93ff: Free area
    9400-940f: Speech synthesizer write (on-board)
    9410-97ff: Free area
    9800-980f: System GROM read (data, address)
    9810-9bff: Free area
    9c00-9c0f: System GROM write (data, address)
    9c10-fffb: Free area
    fffc-ffff: NMI vector

    TI-99/8 native mode (CRUS=0)
    ----------------------------
    0000-efff: Free area
    f000-f7ff: 2 KiB SRAM
      f000-f1ff: mapper files (8 files with 16*4 bytes each)
      f200-f7ff: Free RAM
    f800-f80f: Sound chip
    f810-f81f: VDP read (data, status) and write (data, address)
    f820-f82f: Speech synthesizer read/write
    f830-f83f: System GROM read/write
    f840-f86f: Free area
    f870-f87f: Mapper access port
    f880-fffb: Free area
    fffc-ffff: NMI vector

    Note that ROM0 is not visible in the native mode.

    If CRU bit 21 (PTGEN*) is set to 0, Pascal GROMs appear in the LAS in either
    mode. It is highly recommended to use native mode when turning on these
    GROMs, because the area where they appear may be occupied by a program in
    99/4A mode.

    Pascal and Text-to-speech GROM enabled (PTGEN*=0)
    -------------------------------------------------
    f840-f84f: Text-to-speech GROM read/write
    f850-f85f: P-Code library #1 GROM read/write
    f860-f86f: P-Code library #2 GROM read/write

    Physical address space (PAS)
    ============================
    The PAS is 24 bits wide and accessed via the custom mapper chip nicknamed
    "Amigo". The mapper exchanges map definitions with SRAM (see LAS). That
    means, a map can be prepared in SRAM, and for activating it, the mapper
    is accessed on its port, telling it to load or save a map.

    000000-00ffff: 64 KiB console DRAM
    010000-efffff: undefined
    f00000-f03fff: P-Code ROM (not mentioned in [1])
    f04000-feffff: undefined
    ff0000       : unmapped (code for mapper)
    ff0001-ff3fff: undefined
    ff4000-ff5fff: DSR ROM in Peripheral Box, Hexbus DSR (CRU 1700) or additional ROM (CRU 2700)
    ff6000-ff9fff: Cartridge ROM space
    ffa000-ffdfff: 16 KiB ROM1
    ffe000-ffe00f: Interrupt level sense
    ffe010-ffffff: undefined


    CRU map (I/O address space)
    ===========================
    0000-003e: TMS9901 system interface (see ti99_8.c)
    1700-17fe: Hexbus
    2000-26fe: Future external devices
    2700-27fe: Additional ROM ("internal DSR")
    2702: System reset (when set to 1)
    2800-3ffe: Future external devices
    4000-fffe: Future external devices

    The TMS9995 offers the full 15-bit CRU address space. Devices designed for
    the TI-99/4A should only be accessed in the area 1000-1ffe. They will (by
    design) incompletely decode the CRU address and be mirrored in the higher areas.

    Michael Zapf, October 2010
    February 2012: Rewritten as class

    Informations taken from
    [1] ARMADILLO PRODUCT SPECIFICATIONS
    [2] TI-99/8 Graphics Programming Language interpreter

***************************************************************************/

#include "998board.h"

#define TRACE_CRU 0
#define TRACE_MEM 0
#define TRACE_MAP 0
#define TRACE_CONFIG 0
#define TRACE_OSO 0
#define TRACE_SPEECH 0
#define TRACE_DETAIL 0

mainboard8_device::mainboard8_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: bus8z_device(mconfig, MAINBOARD8, "TI-99/8 Main board", tag, owner, clock, "ti998_mainboard", __FILE__),
	m_ready(*this),
	m_oso(*this, OSO_TAG)
	{ }

/***************************************************************************
    CRU access
***************************************************************************/

#define HEXBUS_CRU_BASE 0x1700
#define MAPPER_CRU_BASE 0x2700

READ8Z_MEMBER(mainboard8_device::crureadz)
{
	if (TRACE_CRU) logerror("%s: read CRU %04x ignored\n", tag(), offset);
	// Nothing here.
}

/*
    CRU handling. We handle the internal device at CRU address 0x2700 via
    this mapper component.
*/
WRITE8_MEMBER(mainboard8_device::cruwrite)
{
	if ((offset & 0xff00)==MAPPER_CRU_BASE)
	{
		int bit = (offset & 0xff)>>1;
		switch (bit)
		{
		case 0:
			// Turn on/off the internal DSR
			m_dsr_selected = (data!=0);
			if (TRACE_CRU) logerror("%s: DSR select = %d\n", tag(), data);
			break;
		case 1:
			if (TRACE_CRU) logerror("%s: System reset by CRU request\n", tag());
			machine().schedule_soft_reset();
			break;
		}
		return;
	}

	if ((offset & 0xff00)==HEXBUS_CRU_BASE)
	{
		int bit = (offset & 0xff)>>1;
		switch (bit)
		{
		case 0:
			// Turn on/off the Hexbus DSR
			m_hexbus_selected = (data!=0);
			if (TRACE_CRU) logerror("%s: Hexbus select = %d\n", tag(), data);
			break;
		default:
			if (TRACE_CRU) logerror("%s: Set CRU>%04x (Hexbus) to %d\n", tag(), offset,data);
			break;
		}
		return;
	}

	if ((offset & 0xff00)>=0x0100)
	{
		if (TRACE_CRU) logerror("%s: Set CRU>%04x (unknown) to %d\n", tag(), offset,data);
		return;
	}
}

void mainboard8_device::CRUS_set(bool state)
{
	if (TRACE_CRU) logerror("%s: set CRUS=%d\n", tag(), state);
	m_CRUS = state;
}

/*
    Note that PTGEN is negative logic. We invert these semantics here.
*/
void mainboard8_device::PTGE_set(bool state)
{
	if (TRACE_CRU) logerror("%s: set PTGEN=%d\n", tag(), state? 1:0);
	m_PTGE = state;
}

/***************************************************************************
    Access by address map
***************************************************************************/

/*
    This method is called via the address map.
*/
READ8_MEMBER( mainboard8_device::readm )
{
	UINT8 value = 0;
	bool found = false;
	if (TRACE_MEM) logerror("%s: read from %04x\n", tag(), offset);
	found = access_logical_r(space, offset, &value, mem_mask);
	m_waitcount = 2;

	if (!found)
	{
		// In that case, the address decoder could not find a suitable device.
		// This means the logical address is transformed by the mapper.
		// NOTE: Use "+", not OR. The offset is not a prefix.
		UINT32  pas_address = m_pas_offset[(offset & 0xf000)>>12] + (offset & 0xfff);

		// So now let's do the same as above with physical addresses
		access_physical_r(space, pas_address, &value, mem_mask);

		// The PAS area requires one more wait state, as the address bus
		// is multiplexed
		m_waitcount = 3;
	}

	// Insert wait states and let CPU enter wait state
	m_ready(CLEAR_LINE);

	return value;
}

WRITE8_MEMBER( mainboard8_device::writem )
{
	bool found = false;

	// Look for components responding to the logical address
	found = access_logical_w(space, offset, data, mem_mask);
	m_waitcount = 2;

	if (!found)
	{
		// In that case, the address decoder could not find a suitable device.
		// This means the logical address is transformed by the mapper.
		// NOTE: Use "+", not OR. The offset is not a prefix.
		UINT32 pas_address = m_pas_offset[(offset & 0xf000)>>12] + (offset & 0xfff);

		// So now let's do the same as above with physical addresses
		access_physical_w(space, pas_address, data, mem_mask);

		// The PAS area requires one more wait state, as the address bus
		// is multiplexed
		m_waitcount = 3;
	}

	// Insert wait states and let CPU enter wait state
	m_ready(CLEAR_LINE);
}

/***************************************************************************
    Indirect calls (mapper calls itself)
***************************************************************************/
/*
    This method is called by the mapper itself for
    f870 (NATIVE): mapper: ignore
    8810 (TI99EM): mapper: ignore
    ff4000 (PHYSIC): DSR
*/
READ8Z_MEMBER( mainboard8_device::readz )
{
	if ((offset & 0xffe000)==0xff4000)
	{
		if (m_dsr_selected)
		{
			//  Starts at 0x4000 in the image
			*value = m_rom1[0x4000 | (offset & 0x1fff)];
			if (TRACE_MEM) logerror("%s: (intDSR)  %04x -> %02x\n", tag(), offset, *value);
		}
		else
		{
			if (m_hexbus_selected)
			{
				if ((offset & 0x1ff0)==0x1ff0)
				{
					*value = m_oso->read(space, (offset>>1) & 0x0003);
				}
				else
				{
					//  Starts at 0x6000 in the image
					*value = m_rom1[0x6000 | (offset & 0x1fff)];
					if (TRACE_MEM) logerror("%s: (HexDSR)  %04x -> %02x\n", tag(), offset, *value);
				}
			}
		}
	}
	else
	{
		if (((offset & 0xfff0)==0xf870 && m_CRUS==false)||(((offset & 0xfff0)==0x8810 && m_CRUS==true)))
		{
			if (TRACE_MEM) logerror("%s: read access to mapper ignored: %04x\n", tag(), offset);
		}
	}
}

/*
    This method is called by the mapper itself for
    ff4000 (PHYSIC): DSR. ignore

*/
WRITE8_MEMBER( mainboard8_device::write )
{
	if ((offset & 0xffe000)==0xff4000)
	{
		if (m_hexbus_selected)
		{
			if ((offset & 0x1ff0)==0x1ff0)
			{
				m_oso->write(space, (offset>>1) & 0x0003, data);
			}
			else
			{
				logerror("%s: Write access to Hexbus DSR address %06x ignored\n", tag(), offset);
			}
		}
		else
		{
			if (m_dsr_selected)
			{
				logerror("%s: Write access to internal DSR address %06x ignored\n", tag(), offset);
			}
			else
			{
				logerror("%s: Write access to unmapped DSR space at address %06x ignored\n", tag(), offset);
			}
		}
	}
	else
	{
		if (((offset & 0xfff0)==0xf870 && m_CRUS==false)||(((offset & 0xfff0)==0x8810 && m_CRUS==true)))
		{
			mapwrite(offset, data);
		}
	}
}

/*
    Reconfigure mapper. Writing to this address copies the values in the
    SRAM into the mapper and vice versa.
    Format:
    0000 bbbl; bbb=bank, l=load

    TODO: Emulate properly, making use of HOLD
*/
void mainboard8_device::mapwrite(int offset, UINT8 data)
{
	if ((data & 0xf0)==0x00)
	{
		int bankindx = (data & 0x0e)>>1;
		if (data & 1)
		{
			if (TRACE_MAP) logerror("%s: load mapper from SRAM, bank %d\n", tag(), bankindx);
			// Load from SRAM
			// In reality the CPU is put on HOLD during this transfer
			for (int i=0; i < 16; i++)
			{
				int ptr = (bankindx << 6);
				m_pas_offset[i] =   (m_sram[(i<<2) + ptr] << 24) | (m_sram[(i<<2)+ ptr+1] << 16)
				| (m_sram[(i<<2) + ptr+2] << 8) | (m_sram[(i<<2) + ptr+3]);
				if (TRACE_MAP) logerror("%s: load %d=%08x\n", tag(), i, m_pas_offset[i]);
			}
		}
		else
		{
			if (TRACE_MAP) logerror("%s: store mapper to SRAM, bank %d\n", tag(), bankindx);
			// Store in SRAM
			for (int i=0; i < 16; i++)
			{
				int ptr = (bankindx << 6);
				m_sram[(i<<2) + ptr]    =  (m_pas_offset[i] >> 24)& 0xff;
				m_sram[(i<<2) + ptr +1] =  (m_pas_offset[i] >> 16)& 0xff;
				m_sram[(i<<2) + ptr +2] =  (m_pas_offset[i] >> 8)& 0xff;
				m_sram[(i<<2) + ptr +3] =  (m_pas_offset[i])& 0xff;
				if (TRACE_MAP) logerror("%s: save %d=%08x\n", tag(), i, m_pas_offset[i]);
			}
		}
	}
}

/***************************************************************************
    Lookup methods.
***************************************************************************/

bool mainboard8_device::access_logical_r(address_space& space, offs_t offset, UINT8 *value, UINT8 mem_mask )
{
	bool found = false;
	logically_addressed_device *ldev = m_logcomp.first();
	bus8z_device *bdev = NULL;

	if (TRACE_MEM) logerror("%s: offset=%04x; CRUS=%d, PTGEN=%d\n", tag(), offset, m_CRUS? 1:0, m_PTGE? 0:1);
	while (ldev != NULL)
	{
		if (TRACE_MEM) logerror("%s: checking node=%s\n", tag(), ldev->m_config->name);
		// Check the mode
		if (((ldev->m_config->mode == NATIVE) && (m_CRUS==false))
			|| ((ldev->m_config->mode == TI99EM) && (m_CRUS==true))
			|| ((ldev->m_config->mode == PATGEN) && (m_PTGE==true)))
		{
			if ((offset & ldev->m_config->address_mask)==ldev->m_config->select_pattern)
			{
				switch (ldev->m_kind)
				{
				case MAP8_SRAM:
					*value = m_sram[offset & ~ldev->m_config->address_mask];
					if (TRACE_MEM) logerror("%s: (SRAM) %04x -> %02x\n", tag(), offset, *value);
					break;
				case MAP8_ROM0:
					// Starts at 0000
					*value = m_rom0[offset & ~ldev->m_config->address_mask];
					if (TRACE_MEM) logerror("%s: (ROM0)  %04x -> %02x\n", tag(), offset, *value);
					break;
				case MAP8_DEV:
					// device
					bdev = static_cast<bus8z_device*>(ldev->m_device);
					bdev->readz(space, offset, value, mem_mask);
					if (TRACE_MEM) logerror("%s: (dev %s)  %04x -> %02x\n", tag(), ldev->m_config->name, offset, *value);
					break;
				default:
					if (TRACE_MEM) logerror("%s: Invalid kind for read access: %d\n", tag(), ldev->m_kind);
				}
				found = true;
				if (ldev->m_config->stop==STOP) break;
			}
		}
		ldev = ldev->m_next;
	}
	return found;
}

bool mainboard8_device::access_logical_w(address_space& space, offs_t offset, UINT8 data, UINT8 mem_mask )
{
	bool found = false;
	logically_addressed_device *ldev = m_logcomp.first();
	bus8z_device *bdev = NULL;

	while (ldev != NULL)
	{
		// Check the mode
		if (((ldev->m_config->mode == NATIVE) && (m_CRUS==false))
			|| ((ldev->m_config->mode == TI99EM) && (m_CRUS==true))
			|| ((ldev->m_config->mode == PATGEN) && (m_PTGE==true)))
		{
			if ((offset & ldev->m_config->address_mask)==(ldev->m_config->select_pattern | ldev->m_config->write_select))
			{
				switch (ldev->m_kind)
				{
				case MAP8_SRAM:
					m_sram[offset & ~ldev->m_config->address_mask] = data;
					if (TRACE_MEM) logerror("%s: (SRAM) %04x <- %02x\n", tag(), offset, data);
					break;
				case MAP8_ROM0:
					if (TRACE_MEM) logerror("%s: (ROM0)  %04x <- %02x (ignored)\n", tag(), offset, data);
					break;
				case MAP8_DEV:
					// device
					bdev = static_cast<bus8z_device*>(ldev->m_device);
					bdev->write(space, offset, data, mem_mask);
					if (TRACE_MEM) logerror("%s: (dev %s)  %04x <- %02x\n", tag(), ldev->m_config->name, offset, data);
					break;
				default:
					if (TRACE_MEM) logerror("%s: Invalid kind for write access: %d\n", tag(), ldev->m_kind);
				}
				found = true;
				if (ldev->m_config->stop==STOP) break;
			}
		}
		ldev = ldev->m_next;
	}
	return found;
}


void mainboard8_device::access_physical_r( address_space& space, offs_t pas_address, UINT8 *value, UINT8 mem_mask )
{
	physically_addressed_device *pdev = m_physcomp.first();
	bus8z_device *bdev = NULL;

	while (pdev != NULL)
	{
		if ((pas_address & pdev->m_config->address_mask)==pdev->m_config->select_pattern)
		{
			switch (pdev->m_kind)
			{
			case MAP8_DRAM:
				*value = m_dram[pas_address & ~pdev->m_config->address_mask];
				if (TRACE_MEM) logerror("%s: (DRAM) %06x -> %02x\n", tag(), pas_address, *value);
				break;
			case MAP8_ROM1A0:
				// Starts at 0000 in the image, 8K
				*value = m_rom1[pas_address & 0x1fff];
				if (TRACE_MEM) logerror("%s: (ROM) %06x -> %02x\n", tag(), pas_address, *value);
				break;
			case MAP8_ROM1C0:
				// Starts at 2000 in the image, 8K
				*value = m_rom1[0x2000 | (pas_address & 0x1fff)];
				if (TRACE_MEM) logerror("%s: (ROM)  %06x -> %02x\n", tag(), pas_address, *value);
				break;
			case MAP8_PCODE:
				*value = m_pcode[pas_address & 0x3fff];
				if (TRACE_MEM) logerror("%s: (PCODE) %06x -> %02x\n", tag(), pas_address, *value);
				break;
			case MAP8_INTS:
				// Interrupt sense
				logerror("%s: ILSENSE not implemented.\n", tag());
				break;
			case MAP8_DEV:
				// devices
				bdev = static_cast<bus8z_device*>(pdev->m_device);
				bdev->readz(space, pas_address, value, mem_mask);
				if (TRACE_MEM) logerror("%s: (dev %s)  %06x -> %02x\n", tag(), pdev->m_config->name, pas_address, *value);
				break;
			default:
				logerror("%s: Invalid kind for physical read access: %d\n", tag(), pdev->m_kind);
			}
			if (pdev->m_config->stop==STOP) break;
		}
		pdev = pdev->m_next;
	}
}

void mainboard8_device::access_physical_w( address_space& space, offs_t pas_address, UINT8 data, UINT8 mem_mask )
{
	physically_addressed_device *pdev = m_physcomp.first();
	bus8z_device *bdev = NULL;

	while (pdev != NULL)
	{
		if ((pas_address & pdev->m_config->address_mask)==(pdev->m_config->select_pattern | pdev->m_config->write_select))
		{
			switch (pdev->m_kind)
			{
			case MAP8_DRAM:
				m_dram[pas_address & ~pdev->m_config->address_mask] = data;
				if (TRACE_MEM) logerror("%s: (DRAM) %06x <- %02x\n", tag(), pas_address, data);
				break;
			case MAP8_ROM1A0:
			case MAP8_ROM1C0:
				if (TRACE_MEM) logerror("%s: (ROM1)  %06x <- %02x (ignored)\n", tag(), pas_address, data);
				break;
			case MAP8_PCODE:
				if (TRACE_MEM) logerror("%s: (PCODE)  %06x <- %02x (ignored)\n", tag(), pas_address, data);
				break;
			case MAP8_INTS:
				// Interrupt sense
				logerror("%s: write to ilsense ignored\n", tag());
				break;
			case MAP8_DEV:
				// devices
				bdev = static_cast<bus8z_device*>(pdev->m_device);
				if (TRACE_MEM) logerror("%s: (dev %s)  %06x <- %02x\n", tag(), pdev->m_config->name, pas_address, data);
				bdev->write(space, pas_address, data, mem_mask);
				break;
			default:
				logerror("%s: Invalid kind for physical write access: %d\n", tag(), pdev->m_kind);
			}
			if (pdev->m_config->stop==STOP) break;
		}
		pdev = pdev->m_next;
	}
}

/*
    The mapper is connected to the clock line in order to operate
    the wait state counter.
*/
void mainboard8_device::clock_in(int clock)
{
	if (clock==ASSERT_LINE && m_waitcount!=0)
	{
		m_waitcount--;
		if (m_waitcount==0) m_ready(ASSERT_LINE);
	}
}


/***************************************************************************
    DEVICE LIFECYCLE FUNCTIONS
***************************************************************************/
/*
    We need to do all of the configuration in device_start since we don't have all
    required links earlier.

    Note that device_reset is too late; the initial context switch occurs earlier.
*/
void mainboard8_device::device_start()
{
	logerror("%s: Starting mapper\n", tag());

	// String values of the pseudo constants, used in the configuration.
	const char *const pseudodev[7] = { SRAMNAME, ROM0NAME, ROM1A0NAME, ROM1C0NAME, DRAMNAME, PCODENAME, INTSNAME };

	const mapper8_config *conf = reinterpret_cast<const mapper8_config *>(static_config());

	const mapper8_list_entry *entry = conf->devlist;
	m_ready.resolve_safe();

	m_sram = machine().root_device().memregion(SRAM_TAG)->base();
	m_dram = machine().root_device().memregion(DRAM_TAG)->base();
	m_rom0  = machine().root_device().memregion(ROM0_TAG)->base();
	m_rom1  = machine().root_device().memregion(ROM1_TAG)->base();
	m_pcode  = machine().root_device().memregion(PCODEROM_TAG)->base();

	// Clear the lists
	m_logcomp.reset();
	m_physcomp.reset();

	// Now building the list of active devices at this mapper.
	// Coyping partly from datamux.c.
	if ( entry != NULL )
	{
		bool done = false;
		for (int i=0; !done; i++)
		{
			if (entry[i].name == NULL)
			{
				done = true;
			}
			else
			{
				device_t *dev = NULL;
				mapper8_device_kind kind = MAP8_UNDEF;

				for (int j=1; (j < 8) && (kind == MAP8_UNDEF); j++)
				{
					// Pseudo devices are enumerated as 1 ... 6 (see MAP8_SRAM etc.)
					if (strcmp(entry[i].name, pseudodev[j-1])==0) kind = (mapper8_device_kind)j;
				}
				if (kind==MAP8_UNDEF)
				{
					// This entry points to a "real" device, not to a special constant
					kind = MAP8_DEV;
					dev = machine().device(entry[i].name);
				}
				if (kind != MAP8_DEV || dev != NULL)
				{
					if (entry[i].mode != PHYSIC)
					{
						logically_addressed_device *ad = new logically_addressed_device(kind, (device_t*)dev, entry[i]);
						m_logcomp.append(*ad);
						if (TRACE_CONFIG) logerror("%s: Device %s mounted into logical address space.\n", tag(), entry[i].name);
					}
					else
					{
						physically_addressed_device *ad = new physically_addressed_device(kind, (device_t*)dev, entry[i]);
						m_physcomp.append(*ad);
						if (TRACE_CONFIG) logerror("%s: Device %s mounted into physical address space.\n", tag(), entry[i].name);
					}
				}
				else
				{
					if (TRACE_CONFIG) logerror("%s: Device %s not found.\n", tag(), entry[i].name);
				}
			}
		}
	}
	if (TRACE_CONFIG) logerror("%s: Mapper logical device count = %d\n", tag(), m_logcomp.count());
	if (TRACE_CONFIG) logerror("%s: Mapper physical device count = %d\n", tag(), m_physcomp.count());

	m_dsr_selected = false;
	m_CRUS = true;
	m_PTGE = false;

	// Clean mapper
	for (int i=0; i < 16; i++) m_pas_offset[i] = 0;
}

void mainboard8_device::device_reset()
{
	m_dsr_selected = false;
	m_CRUS = true;
	m_PTGE = false;
	m_waitcount = 0;
	m_hexbus_selected = false;

	// Clean mapper
	for (int i=0; i < 16; i++) m_pas_offset[i] = 0;

	m_ready(ASSERT_LINE);
}

MACHINE_CONFIG_FRAGMENT( ti998_mainboard )
	MCFG_DEVICE_ADD(OSO_TAG, OSO, 0)
MACHINE_CONFIG_END

machine_config_constructor mainboard8_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ti998_mainboard );
}

const device_type MAINBOARD8 = &device_creator<mainboard8_device>;

/***************************************************************************

  Custom chips of the TI-99/8

  ===== OSO: Hexbus interface =====

  The Hexbus is a 4-bit peripheral bus with master/slave coordination. Bytes
  are written over the bus in two passes. Hexbus was the designated standard
  peripheral bus for TI computers before TI left the home computer market.

  Existing devices are floppy drive, RS232 serial adapter, and
  a "Wafertape" drive (kind of tape streamer)

  Registers:  Read   Write  Bits of register
  ----------------------------------------------------------------------------
  Data     :  5FF8     -    ADB3  ADB2  ADB1    ADB0    ADB3  ADB2  ADB1  ADB0
  Status   :  5FFA     -    HSKWT HSKRD BAVIAS  BAVAIS  SBAV  WBUSY RBUSY SHSK
  Control  :  5FFC   5FFA   WIEN  RIEN  BAVIAEN BAVAIEN BAVC  WEN   REN   CR7
  Xmit     :  5FFE   5FF8   XDR0  XDR1  XDR2    XDR3    XDR4  XDR5  XDR6  XDR7

  ADBx = Hexbus data bit X
  HSKWT = Set when a byte has been sent over the bus and HSK has been asserted
  HSKRD = Set when a byte has been received
  BAVIAS = set when the BAV* signal (bus available) transits to active state
  BAVAIS = set when the BAV* signal transits to inactive state (=1)
  SBAV = set when BAV* = 0 (active)
  WBUSY = set when a write action is in progress (two transfers @ 4 bits)
  Reset when HSKWT is set
  RBUSY = set when a read action is in progress (two transfers @ 4 bits)
  Reset when HSKRD is set
  SHSK = set when HSK* is active (0)

  WIEN = Enable interrupt for write completion
  RIEN = Enable interrupt for read completion
  BAVIAEN = BAVIA enable (slave mode)
  BAVAIEN = BAVAI enable (slave mode)
  BAVC = set BAV* line (0=active)
  WEN = set write enable (byte is written from xmit reg)
  REN = set read enable (latch HSK and read byte into data reg)
  CR7 = future extension
  XDRx = transmit register bit

  Hexbus connector (console)
  +---+---+---+---+
  | 4 | 3 | 2 | 1 |      4 = L;    3 = BAV*; 2 = ADB1; 1 = ADB0
  +---+---+---+---+
  | 8 | 7 | 6 | 5 |      8 = ADB3; 7 = ADB2; 6 = nc;   5 = HSK*
  +---+---+---+---+

  TODO: This is just a preliminary implementation to satisfy the operating
        system. When completed we can hopefully emulate a Hexbus floppy and
        use it in Extended Basic II which refuses to work with the PEB cards.
        The Hexbus should then be designed as a slot device.

****************************************************************************/

/* Status register bits */
enum
{
	HSKWT = 0x80,
	HSKRD = 0x40,
	BAVIAS = 0x20,
	BAVAIS = 0x10,
	SBAV = 0x08,
	WBUSY = 0x04,
	RBUSY = 0x02,
	SHSK = 0x01
};

ti998_oso_device::ti998_oso_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: device_t(mconfig, OSO, "OSO Hexbus interface", tag, owner, clock, "ti998_oso", __FILE__)
{
}

READ8_MEMBER( ti998_oso_device::read )
{
	int value = 0;
	offset &= 0x03;
	switch (offset)
	{
	case 0:
		// read 5FF8: read data register
		if (TRACE_OSO) logerror("%s: Read data register = %02x\n", tag(), value);
		value = m_data;
		break;
	case 1:
		// read 5FFA: read status register
		value = m_status;
		if (TRACE_OSO) logerror("%s: Read status %02x\n", tag(), value);
		break;
	case 2:
		// read 5FFC: read control register
		value = m_control;
		if (TRACE_OSO) logerror("%s: Read control register = %02x\n", tag(), value);
		break;
	case 3:
		// read 5FFE: read transmit register
		value = m_xmit;
		if (TRACE_OSO) logerror("%s: Read transmit register = %02x\n", tag(), value);
		break;
	}
	return value;
}

WRITE8_MEMBER( ti998_oso_device::write )
{
	offset &= 0x03;
	switch (offset)
	{
	case 0:
		// write 5FF8: write transmit register
		if (TRACE_OSO) logerror("%s: Write transmit register %02x\n", tag(), data);
		m_xmit = data;
		// We set the status register directly in order to prevent lock-ups
		// until we have a complete Hexbus implementation
		m_status |= HSKWT;
		break;
	case 1:
		// write 5FFA: write control register
		if (TRACE_OSO) logerror("%s: Write control register %02x\n", tag(), data);
		m_control = data;
		break;
	default:
		// write 5FFC, 5FFE: undefined
		if (TRACE_OSO) logerror("%s: Invalid write on %04x: %02x\n", tag(), (offset<<1) | 0x5ff0, data);
		break;
	}
}

void ti998_oso_device::device_start()
{
	m_status = m_xmit = m_control = m_data = 0;
}

const device_type OSO = &device_creator<ti998_oso_device>;


// ========================================================================

/****************************************************************************

    TI-99/8 Speech synthesizer subsystem

    The TI-99/8 contains a speech synthesizer inside the console, so we cannot
    reuse the spchsyn implementation of the P-Box speech synthesizer.
    Accordingly, this is not a ti_expansion_card_device.

    For comments on real timing see ti99/spchsyn.c

    Note that before the REAL_TIMING can be used we must first establish
    the set_address logic in 998board.

*****************************************************************************/

#define TMS5220_ADDRESS_MASK 0x3FFFFUL  /* 18-bit mask for tms5220 address */
#define SPEECHSYN_TAG "speechsyn"
#define REAL_TIMING 0

ti998_spsyn_device::ti998_spsyn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: bus8z_device(mconfig, SPEECH8, "TI-99/8 Onboard Speech synthesizer", tag, owner, clock, "ti998_speech", __FILE__),
	m_ready(*this)
{
}

/*
    Memory read
*/
#if REAL_TIMING
// ======  This is the version with real timing =======
READ8Z_MEMBER( ti998_spsyn_device::readz )
{
	m_vsp->wsq_w(TRUE);
	m_vsp->rsq_w(FALSE);
	*value = m_vsp->read(offset) & 0xff;
	if (TRACE_SPEECH) logerror("%s: read value = %02x\n", tag(), *value);
}

/*
    Memory write
*/
WRITE8_MEMBER( ti998_spsyn_device::write )
{
	m_vsp->rsq_w(m_vsp, TRUE);
	m_vsp->wsq_w(m_vsp, FALSE);
	if (TRACE_SPEECH) logerror("%s: write value = %02x\n", tag(), data);
	m_vsp->write(offset, data);
}

#else
// ======  This is the version without real timing =======

READ8Z_MEMBER( ti998_spsyn_device::readz )
{
	machine().device("maincpu")->execute().adjust_icount(-(18+3));      /* this is just a minimum, it can be more */
	*value = m_vsp->status_r(space, offset, 0xff) & 0xff;
	if (TRACE_SPEECH) logerror("%s: read value = %02x\n", tag(), *value);
}

/*
    Memory write
*/
WRITE8_MEMBER( ti998_spsyn_device::write )
{
	machine().device("maincpu")->execute().adjust_icount(-(54+3));      /* this is just an approx. minimum, it can be much more */

	/* RN: the stupid design of the tms5220 core means that ready is cleared */
	/* when there are 15 bytes in FIFO.  It should be 16.  Of course, if */
	/* it were the case, we would need to store the value on the bus, */
	/* which would be more complex. */
	if (!m_vsp->readyq_r())
	{
		attotime time_to_ready = attotime::from_double(m_vsp->time_to_ready());
		int cycles_to_ready = machine().device<cpu_device>("maincpu")->attotime_to_cycles(time_to_ready);
		if (TRACE_SPEECH && TRACE_DETAIL) logerror("%s: time to ready: %f -> %d\n", tag(), time_to_ready.as_double(), (int) cycles_to_ready);

		machine().device("maincpu")->execute().adjust_icount(-cycles_to_ready);
		machine().scheduler().timer_set(attotime::zero, FUNC_NULL);
	}
	if (TRACE_SPEECH) logerror("%s: write value = %02x\n", tag(), data);
	m_vsp->data_w(space, offset, data);
}
#endif

/**************************************************************************/

WRITE_LINE_MEMBER( ti998_spsyn_device::speech8_ready )
{
	// The TMS5200 implementation uses TRUE/FALSE, not ASSERT/CLEAR semantics
	m_ready((state==0)? ASSERT_LINE : CLEAR_LINE);
	if (TRACE_SPEECH) logerror("%s: READY = %d\n", tag(), (state==0));

#if REAL_TIMING
	// Need to do that here (see explanations in spchsyn.c)
	if (state==0)
	{
		m_vsp->rsq_w(TRUE);
		m_vsp->wsq_w(TRUE);
	}
#endif
}

void ti998_spsyn_device::device_start()
{
	m_ready.resolve_safe();
	m_vsp = subdevice<tms5220_device>(SPEECHSYN_TAG);
	speechrom_device* mem = subdevice<speechrom_device>("vsm");
	mem->set_reverse_bit_order(true);
}

void ti998_spsyn_device::device_reset()
{
	if (TRACE_SPEECH) logerror("%s: reset\n", tag());
}

// Unlike the TI-99/4A, the 99/8 uses the CD2501ECD
// The CD2501ECD is a tms5200/cd2501e with the rate control from the tms5220c added in.
// (it's probably actually a tms5220c die with the cd2501e/tms5200 lpc rom masked onto it)
MACHINE_CONFIG_FRAGMENT( ti998_speech )
	MCFG_DEVICE_ADD("vsm", SPEECHROM, 0)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEECHSYN_TAG, CD2501ECD, 640000L)
	MCFG_TMS52XX_READYQ_HANDLER(WRITELINE(ti998_spsyn_device, speech8_ready))
	MCFG_TMS52XX_SPEECHROM("vsm")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

/* Verified on a real machine: TI-99/8 uses the same speech rom contents
   as the TI speech synthesizer. */
ROM_START( ti998_speech )
	ROM_REGION(0x8000, "vsm", 0)
	ROM_LOAD("cd2325a.vsm", 0x0000, 0x4000, CRC(1f58b571) SHA1(0ef4f178716b575a1c0c970c56af8a8d97561ffe))
	ROM_LOAD("cd2326a.vsm", 0x4000, 0x4000, CRC(65d00401) SHA1(a367242c2c96cebf0e2bf21862f3f6734b2b3020))
ROM_END

machine_config_constructor ti998_spsyn_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ti998_speech );
}

const rom_entry *ti998_spsyn_device::device_rom_region() const
{
	return ROM_NAME( ti998_speech );
}
const device_type SPEECH8 = &device_creator<ti998_spsyn_device>;
