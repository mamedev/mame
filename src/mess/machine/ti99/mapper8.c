// license:MAME|LGPL-2.1+
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

    Informations taken from
    [1] ARMADILLO PRODUCT SPECIFICATIONS
    [2] TI-99/8 Graphics Programming Language interpreter

    Format of map table entry (not emulated)

    * bit 0: WTPROT: page is write protected if 1
    * bit 1: XPROT: page is execute protected if 1
    * bit 2: RDPROT: page is read protected if 1
    * bit 3: reserved, value is ignored
    * bits 4-7: reserved, always forced to 0
    * bits 8-23: page base address in 24-bit virtual address space

    Format of mapper control register:
    * bit 0-4: unused???
    * bit 5-6: map file to load/save (0 for file 0, 1 for file 1, etc.)
    * bit 7: 0 -> load map file from RAM, 1 -> save map file to RAM

    Format of mapper status register (cleared by read):
    * bit 0: WPE - Write-Protect Error
    * bit 1: XCE - eXeCute Error
    * bit 2: RPE - Read-Protect Error
    * bits 3-7: unused???

    Memory error interrupts are enabled by setting WTPROT/XPROT/RDPROT.  When
    an error occurs, the tms9901 INT1* pin is pulled low (active).  The pin
    remains low until the mapper status register is read.

24-bit address map:
    * >000000->00ffff: console RAM
    * >010000->feffff: expansion?
    * >ff0000->ff0fff: empty???
    * >ff1000->ff3fff: unused???
    * >ff4000->ff5fff: DSR space
    * >ff6000->ff7fff: cartridge space
    * >ff8000->ff9fff(???): >4000 ROM (normally enabled with a write to CRU >2700)
    * >ffa000->ffbfff(?): >2000 ROM
    * >ffc000->ffdfff(?): >6000 ROM


CRU map:
    Since the tms9995 supports full 15-bit CRU addresses, the >1000->17ff
    (>2000->2fff) range was assigned to support up to 16 extra expansion slot.
    The good thing with using >1000->17ff is the fact that older expansion
    cards that only decode 12 address bits will think that addresses
    >1000->17ff refer to internal TI99 peripherals (>000->7ff range), which
    suppresses any risk of bus contention.
    * >0000->001f (>0000->003e): tms9901
      - P4: 1 -> MMD (Memory Mapped Devices?) at >8000, ROM enabled
      - P5: 1 -> no P-CODE GROMs
    * >0800->17ff (>1000->2ffe): Peripheral CRU space
    * >1380->13ff (>2700->27fe): Internal DSR, with two output bits:
      - >2700: Internal DSR select (parts of Basic and various utilities)
      - >2702: SBO -> hardware reset


Memory map (TMS9901 P4 == 1):
    When TMS9901 P4 output is set, locations >8000->9fff are ignored by mapper.
    * >8000->83ff: SRAM (>8000->80ff is used by the mapper DMA controller
      to hold four map files) (r/w)
    * >8400: sound port (w)
    * >8410->87ff: SRAM (r/w)
    * >8800: VDP data read port (r)
    * >8802: VDP status read port (r)
    * >8810: memory mapper status and control registers (r/w)
    * >8c00: VDP data write port (w)
    * >8c02: VDP address and register write port (w)
    * >9000: speech synthesizer read port (r)
    * >9400: speech synthesizer write port (w)
    * >9800 GPL data read port (r)
    * >9802 GPL address read port (r)
    * >9c00 GPL data write port -- unused (w)
    * >9c02 GPL address write port (w)


Memory map (TMS9901 P5 == 0):
    When TMS9901 P5 output is cleared, locations >f840->f8ff(?) are ignored by
    mapper.
    * >f840: data port for P-code grom library 0 (r?)
    * >f850: data port for P-code grom library 1 (r?)
    * >f860: data port for P-code grom library 2 (r?)
    * >f842: address port for P-code grom library 0 (r/w?)
    * >f852: address port for P-code grom library 1 (r/w?)
    * >f862: address port for P-code grom library 2 (r/w?)

    Michael Zapf, October 2010
    February 2012: Rewritten as class

***************************************************************************/

#include "mapper8.h"

#define TRACE_CRU 0
#define TRACE_MEM 0
#define TRACE_MAP 0
#define TRACE_CONFIG 0

#define LOG logerror

mainboard8_device::mainboard8_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: bus8z_device(mconfig, MAINBOARD8, "TI-99/8 Main board", tag, owner, clock, "ti998_mainboard", __FILE__),
	m_oso(*this, OSO_TAG)
	{ }

/***************************************************************************
    CRU access
***************************************************************************/

#define HEXBUS_CRU_BASE 0x1700
#define MAPPER_CRU_BASE 0x2700

READ8Z_MEMBER(mainboard8_device::crureadz)
{
	if (TRACE_CRU) LOG("mainboard_998: read CRU %04x ignored\n", offset);
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
			if (TRACE_CRU) LOG("mainboard_998: DSR select = %d\n", data);
			break;
		case 1:
			if (TRACE_CRU) LOG("mainboard_998: System reset by CRU request\n");
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
			if (TRACE_CRU) LOG("mainboard_998: Hexbus select = %d\n", data);
			break;
		default:
			if (TRACE_CRU) LOG("mainboard_998: Set CRU>%04x (Hexbus) to %d\n",offset,data);
			break;
		}
		return;
	}

	if ((offset & 0xff00)>=0x0100)
	{
		if (TRACE_CRU) LOG("mainboard_998: Set CRU>%04x (unknown) to %d\n",offset,data);
		return;
	}
}

void mainboard8_device::CRUS_set(bool state)
{
	if (TRACE_CRU) LOG("mainboard_998: set CRUS=%d\n", state);
	m_CRUS = state;
}

/*
    Note that PTGEN is negative logic. We invert these semantics here.
*/
void mainboard8_device::PTGE_set(bool state)
{
	if (TRACE_CRU) LOG("mainboard_998: set PTGEN=%d\n", state? 1:0);
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
	if (TRACE_MEM) LOG("mainboard_998: read from %04x\n", offset);
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
			if (TRACE_MEM) LOG("mainboard_998: (intDSR)  %04x -> %02x\n", offset, *value);
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
					if (TRACE_MEM) LOG("mainboard_998: (HexDSR)  %04x -> %02x\n", offset, *value);
				}
			}
		}
	}
	else
	{
		if (((offset & 0xfff0)==0xf870 && m_CRUS==false)||(((offset & 0xfff0)==0x8810 && m_CRUS==true)))
		{
			if (TRACE_MEM) LOG("mainboard_998: read access to mapper ignored: %04x\n", offset);
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
				LOG("mainboard_998: Write access to Hexbus DSR address %06x ignored\n", offset);
			}
		}
		else
		{
			if (m_dsr_selected)
			{
				LOG("mainboard_998: Write access to internal DSR address %06x ignored\n", offset);
			}
			else
			{
				LOG("mainboard_998: Write access to unmapped DSR space at address %06x ignored\n", offset);
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
			if (TRACE_MAP) LOG("mainboard_998: load mapper from SRAM, bank %d\n", bankindx);
			// Load from SRAM
			// In reality the CPU is put on HOLD during this transfer
			for (int i=0; i < 16; i++)
			{
				int ptr = (bankindx << 6);
				m_pas_offset[i] =   (m_sram[(i<<2) + ptr] << 24) | (m_sram[(i<<2)+ ptr+1] << 16)
				| (m_sram[(i<<2) + ptr+2] << 8) | (m_sram[(i<<2) + ptr+3]);
				if (TRACE_MAP) LOG("mainboard_998: load %d=%08x\n", i, m_pas_offset[i]);
			}
		}
		else
		{
			if (TRACE_MAP) LOG("mainboard_998: store mapper to SRAM, bank %d\n", bankindx);
			// Store in SRAM
			for (int i=0; i < 16; i++)
			{
				int ptr = (bankindx << 6);
				m_sram[(i<<2) + ptr]    =  (m_pas_offset[i] >> 24)& 0xff;
				m_sram[(i<<2) + ptr +1] =  (m_pas_offset[i] >> 16)& 0xff;
				m_sram[(i<<2) + ptr +2] =  (m_pas_offset[i] >> 8)& 0xff;
				m_sram[(i<<2) + ptr +3] =  (m_pas_offset[i])& 0xff;
				if (TRACE_MAP) LOG("mainboard_998: save %d=%08x\n", i, m_pas_offset[i]);
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

	if (TRACE_MEM) LOG("mainboard_998: offset=%04x; CRUS=%d, PTGEN=%d\n", offset, m_CRUS? 1:0, m_PTGE? 0:1);
	while (ldev != NULL)
	{
		if (TRACE_MEM) LOG("mainboard_998: checking node=%s\n", ldev->m_config->name);
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
					if (TRACE_MEM) LOG("mainboard_998: (SRAM) %04x -> %02x\n", offset, *value);
					break;
				case MAP8_ROM0:
					// Starts at 0000
					*value = m_rom0[offset & ~ldev->m_config->address_mask];
					if (TRACE_MEM) LOG("mainboard_998: (ROM0)  %04x -> %02x\n", offset, *value);
					break;
				case MAP8_DEV:
					// device
					bdev = static_cast<bus8z_device*>(ldev->m_device);
					bdev->readz(space, offset, value, mem_mask);
					if (TRACE_MEM) LOG("mainboard_998: (dev %s)  %04x -> %02x\n", ldev->m_config->name, offset, *value);
					break;
				default:
					if (TRACE_MEM) LOG("mainboard_998: Invalid kind for read access: %d\n", ldev->m_kind);
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
					if (TRACE_MEM) LOG("mainboard_998: (SRAM) %04x <- %02x\n", offset, data);
					break;
				case MAP8_ROM0:
					if (TRACE_MEM) LOG("mainboard_998: (ROM0)  %04x <- %02x (ignored)\n", offset, data);
					break;
				case MAP8_DEV:
					// device
					bdev = static_cast<bus8z_device*>(ldev->m_device);
					bdev->write(space, offset, data, mem_mask);
					if (TRACE_MEM) LOG("mainboard_998: (dev %s)  %04x <- %02x\n", ldev->m_config->name, offset, data);
					break;
				default:
					if (TRACE_MEM) LOG("mainboard_998: Invalid kind for write access: %d\n", ldev->m_kind);
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
				if (TRACE_MEM) LOG("mainboard_998: (DRAM) %06x -> %02x\n", pas_address, *value);
				break;
			case MAP8_ROM1A0:
				// Starts at 0000 in the image, 8K
				*value = m_rom1[pas_address & 0x1fff];
				if (TRACE_MEM) LOG("mainboard_998: (ROM) %06x -> %02x\n", pas_address, *value);
				break;
			case MAP8_ROM1C0:
				// Starts at 2000 in the image, 8K
				*value = m_rom1[0x2000 | (pas_address & 0x1fff)];
				if (TRACE_MEM) LOG("mainboard_998: (ROM)  %06x -> %02x\n", pas_address, *value);
				break;
			case MAP8_INTS:
				// Interrupt sense
				LOG("mainboard_998: ILSENSE not implemented.\n");
				break;
			case MAP8_DEV:
				// devices
				bdev = static_cast<bus8z_device*>(pdev->m_device);
				bdev->readz(space, pas_address, value, mem_mask);
				if (TRACE_MEM) LOG("mainboard_998: (dev %s)  %06x -> %02x\n", pdev->m_config->name, pas_address, *value);
				break;
			default:
				LOG("mainboard_998: Invalid kind for physical read access: %d\n", pdev->m_kind);
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
				if (TRACE_MEM) LOG("mainboard_998: (DRAM) %06x <- %02x\n", pas_address, data);
				break;
			case MAP8_ROM1A0:
			case MAP8_ROM1C0:
				if (TRACE_MEM) LOG("mainboard_998: (ROM1)  %06x <- %02x (ignored)\n", pas_address, data);
				break;
			case MAP8_INTS:
				// Interrupt sense
				LOG("ti99_8: write to ilsense ignored\n");
				break;
			case MAP8_DEV:
				// devices
				bdev = static_cast<bus8z_device*>(pdev->m_device);
				if (TRACE_MEM) LOG("mainboard_998: (dev %s)  %06x <- %02x\n", pdev->m_config->name, pas_address, data);
				bdev->write(space, pas_address, data, mem_mask);
				break;
			default:
				LOG("mainboard_998: Invalid kind for physical write access: %d\n", pdev->m_kind);
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
	LOG("ti99_8: Starting mapper\n");

	// String values of the pseudo constants, used in the configuration.
	const char *const pseudodev[6] = { SRAMNAME, ROM0NAME, ROM1A0NAME, ROM1C0NAME, DRAMNAME, INTSNAME };

	const mapper8_config *conf = reinterpret_cast<const mapper8_config *>(static_config());

	const mapper8_list_entry *entry = conf->devlist;
	m_ready.resolve(conf->ready, *this);

	m_sram = machine().root_device().memregion(SRAM_TAG)->base();
	m_dram = machine().root_device().memregion(DRAM_TAG)->base();
	m_rom0  = machine().root_device().memregion(ROM0_TAG)->base();
	m_rom1  = machine().root_device().memregion(ROM1_TAG)->base();

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

				for (int j=1; (j < 7) && (kind == MAP8_UNDEF); j++)
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
						if (TRACE_CONFIG) LOG("mainboard_998: Device %s mounted into logical address space.\n", entry[i].name);
					}
					else
					{
						physically_addressed_device *ad = new physically_addressed_device(kind, (device_t*)dev, entry[i]);
						m_physcomp.append(*ad);
						if (TRACE_CONFIG) LOG("mainboard_998: Device %s mounted into physical address space.\n", entry[i].name);
					}
				}
				else
				{
					if (TRACE_CONFIG) LOG("mainboard_998: Device %s not found.\n", entry[i].name);
				}
			}
		}
	}
	if (TRACE_CONFIG) LOG("Mapper logical device count = %d\n", m_logcomp.count());
	if (TRACE_CONFIG) LOG("Mapper physical device count = %d\n", m_physcomp.count());

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
  OSO: Hexbus interface

****************************************************************************/

enum
{
	HSKWT = 0x80
};

ti998_oso_device::ti998_oso_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: device_t(mconfig, OSO, "OSO Hexbus interface", tag, owner, clock, "ti998_oso", __FILE__)
{
	LOG("ti998/oso: Creating OSO\n");
}

READ8_MEMBER( ti998_oso_device::read )
{
	int value = 0;
	offset &= 0x03;
	LOG("ti998/oso: OSO chip read access %04x -> %02x\n", (offset<<1) | 0x5ff0, value);
	switch (offset)
	{
	case 0:
		// read 5FF8: read data register
		break;
	case 1:
		// read 5FFA: read status register
		// We return handshake_write=1 to prevent lock-ups (until the hexbus is properly implemented)
		value = HSKWT;
		break;
	case 2:
		// read 5FFC: read control register
		break;
	case 3:
		// read 5FFE: read transmit register
		break;
	}

	return value;
}

WRITE8_MEMBER( ti998_oso_device::write )
{
	offset &= 0x03;
	LOG("ti998/oso: OSO chip write access %04x <- %02x\n", (offset<<1) | 0x5ff0, data);
}

void ti998_oso_device::device_start()
{
}

const device_type OSO = &device_creator<ti998_oso_device>;
