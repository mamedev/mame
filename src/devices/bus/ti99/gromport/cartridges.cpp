// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************
    Cartridge implementation

    Every cartridge is an instance of ti99_cartridge_device, implementing the
    device_image_interface. This means it is capable of loading cartridge
    data into its memory locations. All memory locations are organised as
    regions.

    The different cartridge versions are realised by different PCB instances.
    All PCBs are subclassed from ti99_cartridge_pcb.

***************************************************************************/
#include "emu.h"
#include "cartridges.h"

#define LOG_WARN         (1U<<1)   // Warnings
#define LOG_CONFIG       (1U<<2)   // Configuration
#define LOG_CHANGE       (1U<<3)   // Cart change
#define LOG_BANKSWITCH   (1U<<4)   // Bank switch operation
#define LOG_CRU          (1U<<5)   // CRU access
#define LOG_READ         (1U<<6)   // Read operation
#define LOG_WRITE        (1U<<7)   // Write operation
#define LOG_GROM         (1U<<8)   // GROM access
#define LOG_RPK          (1U<<9)   // RPK handler
#define LOG_WARNW        (1U<<10)  // Warn when writing to cartridge space

#define VERBOSE ( LOG_GENERAL | LOG_WARN )
#include "logmacro.h"

DEFINE_DEVICE_TYPE_NS(TI99_CART, bus::ti99::gromport, ti99_cartridge_device, "ti99cart", "TI-99 cartridge")

namespace bus::ti99::gromport {

#define CARTGROM_TAG "grom_contents"
#define CARTROM_TAG "rom_contents"
#define GROM3_TAG "grom3"
#define GROM4_TAG "grom4"
#define GROM5_TAG "grom5"
#define GROM6_TAG "grom6"
#define GROM7_TAG "grom7"

enum
{
	PCB_STANDARD=1,
	PCB_PAGED12K,
	PCB_PAGED16K,
	PCB_MINIMEM,
	PCB_SUPER,
	PCB_MBX,
	PCB_PAGED379I,
	PCB_PAGED378,
	PCB_PAGED377,
	PCB_PAGEDCRU,
	PCB_GROMEMU,
	PCB_PAGED7
};

static const pcb_type pcbdefs[] =
{
	{ PCB_STANDARD, "standard" },
	{ PCB_PAGED12K, "paged12k" },
	{ PCB_PAGED16K, "paged" },
	{ PCB_MINIMEM, "minimem" },
	{ PCB_SUPER, "super" },
	{ PCB_MBX, "mbx" },
	{ PCB_PAGED379I, "paged379i" },
	{ PCB_PAGED378, "paged378" },
	{ PCB_PAGED377, "paged377" },
	{ PCB_PAGEDCRU, "pagedcru" },
	{ PCB_GROMEMU, "gromemu" },
	{ PCB_PAGED7, "paged7" },
	{ 0, nullptr}
};

static const pcb_type sw_pcbdefs[] =
{
	{ PCB_STANDARD, "standard" },
	{ PCB_PAGED12K, "paged12k" },
	{ PCB_PAGED16K, "paged16k" },
	{ PCB_MINIMEM, "minimem" },
	{ PCB_SUPER, "super" },
	{ PCB_MBX, "mbx" },
	{ PCB_GROMEMU, "gromemu" },
	{ PCB_PAGED7, "paged7" },
	{ 0, nullptr}
};

ti99_cartridge_device::ti99_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
:   device_t(mconfig, TI99_CART, tag, owner, clock),
	device_image_interface(mconfig, *this),
	m_pcbtype(0),
	m_slot(0),
	m_pcb(nullptr),
	m_connector(nullptr),
	m_rpk(nullptr)
{
}

void ti99_cartridge_device::prepare_cartridge()
{
	int rom2_length;

	uint8_t* grom_ptr;
	uint8_t* rom_ptr;

	memory_region *regg;
	memory_region *regr;

	// Initialize some values.
	m_pcb->m_rom_page = 0;
	m_pcb->m_rom_ptr = nullptr;
	m_pcb->m_ram_size = 0;
	m_pcb->m_ram_ptr = nullptr;
	m_pcb->m_ram_page = 0;

	for (int i=0; i < 5; i++) m_pcb->m_grom[i] = nullptr;

	m_pcb->m_grom_size = loaded_through_softlist() ? get_software_region_length("grom") : m_rpk->get_resource_length("grom_socket");
	LOGMASKED(LOG_CONFIG, "grom_socket.size=0x%04x\n", m_pcb->m_grom_size);

	if (m_pcb->m_grom_size > 0)
	{
		regg = memregion(CARTGROM_TAG);
		grom_ptr = loaded_through_softlist() ? get_software_region("grom") : m_rpk->get_contents_of_socket("grom_socket");
		memcpy(regg->base(), grom_ptr, m_pcb->m_grom_size);
		m_pcb->m_grom_ptr = regg->base();   // for gromemu
		m_pcb->m_grom_address = 0;          // for gromemu

		// Find the GROMs and keep their pointers
		m_pcb->set_grom_pointer(0, subdevice(GROM3_TAG));
		if (m_pcb->m_grom_size > 0x2000) m_pcb->set_grom_pointer(1, subdevice(GROM4_TAG));
		if (m_pcb->m_grom_size > 0x4000) m_pcb->set_grom_pointer(2, subdevice(GROM5_TAG));
		if (m_pcb->m_grom_size > 0x6000) m_pcb->set_grom_pointer(3, subdevice(GROM6_TAG));
		if (m_pcb->m_grom_size > 0x8000) m_pcb->set_grom_pointer(4, subdevice(GROM7_TAG));
	}

	m_pcb->m_rom_size = loaded_through_softlist() ? get_software_region_length("rom") : m_rpk->get_resource_length("rom_socket");
	m_pcb->m_bank_mask = 0;

	if (m_pcb->m_rom_size > 0)
	{
		if (m_pcb->m_rom_size > 0x200000) fatalerror("Cartridge ROM size exceeding 2 MiB");
		LOGMASKED(LOG_CONFIG, "rom size=0x%04x\n", m_pcb->m_rom_size);

		// Determine the bank mask for flexible ROM sizes in gromemu
		int rsizet = m_pcb->m_rom_size;
		int msizet = 0x2000;

		while (msizet < rsizet)
		{
			m_pcb->m_bank_mask = (m_pcb->m_bank_mask<<1) | 1;
			msizet <<= 1;
		}
		LOGMASKED(LOG_CONFIG, "rom bank mask=0x%04x\n", m_pcb->m_bank_mask);

		regr = memregion(CARTROM_TAG);
		rom_ptr = loaded_through_softlist() ? get_software_region("rom") : m_rpk->get_contents_of_socket("rom_socket");
		memcpy(regr->base(), rom_ptr, m_pcb->m_rom_size);
		// Set both pointers to the same region for now
		m_pcb->m_rom_ptr = regr->base();
	}

	// Softlist uses only one ROM area, no second socket
	if (!loaded_through_softlist())
	{
		rom2_length = m_rpk->get_resource_length("rom2_socket");
		if (rom2_length > 0)
		{
			// sizes do not differ between rom and rom2
			// We use the large cartrom space for the second bank as well
			regr = memregion(CARTROM_TAG);
			rom_ptr = m_rpk->get_contents_of_socket("rom2_socket");
			memcpy(regr->base() + 0x2000, rom_ptr, rom2_length);

			// Configurations with ROM1+ROM2 have exactly two banks; only
			// the first 8K are used from ROM1.
			m_pcb->m_bank_mask = 1;
			LOGMASKED(LOG_CONFIG, "rom bank mask=0x0001 (using rom/rom2)\n");
		}
	}

	// (NV)RAM cartridges
	if (loaded_through_softlist())
	{
		// Do we have NVRAM?
		if (get_software_region("nvram")!=nullptr)
		{
			m_pcb->m_ram_size = get_software_region_length("nvram");
			m_pcb->m_nvram.resize(m_pcb->m_ram_size);
			m_pcb->m_ram_ptr = &m_pcb->m_nvram[0];
			battery_load(m_pcb->m_ram_ptr, m_pcb->m_ram_size, 0xff);
		}

		// Do we have RAM?
		if (get_software_region("ram")!=nullptr)
		{
			m_pcb->m_ram_size = get_software_region_length("ram");
			m_pcb->m_ram.resize(m_pcb->m_ram_size);
			m_pcb->m_ram_ptr = &m_pcb->m_ram[0];
		}
	}
	else
	{
		m_pcb->m_ram_size = m_rpk->get_resource_length("ram_socket");
		if (m_pcb->m_ram_size > 0)
		{
			// TODO: Consider to use a region as well. If so, do not forget to memcpy.
			m_pcb->m_ram_ptr = m_rpk->get_contents_of_socket("ram_socket");
		}
	}
}

/*
    Find the index of the cartridge name. We assume the format
    <name><number>, i.e. the number is the longest string from the right
    which can be interpreted as a number. Subtract 1.
*/
int ti99_cartridge_device::get_index_from_tagname()
{
	const char *mytag = tag();
	int maxlen = strlen(mytag);
	int i;

	for (i=maxlen-1; i >=0; i--)
		if (mytag[i] < 48 || mytag[i] > 57) break;

	if (i==maxlen-1) return 0;
	return atoi(mytag+i+1)-1;
}

image_init_result ti99_cartridge_device::call_load()
{
	// File name is in m_basename
	// return true = error
	LOGMASKED(LOG_CHANGE, "Loading %s in slot %s\n", basename());

	if (loaded_through_softlist())
	{
		LOGMASKED(LOG_CONFIG, "Using softlists\n");
		int i = 0;
		const char* pcb = get_feature("pcb");
		do
		{
			if (strcmp(pcb, sw_pcbdefs[i].name)==0)
			{
				m_pcbtype = sw_pcbdefs[i].id;
				break;
			}
			i++;
		} while (sw_pcbdefs[i].id != 0);
		LOGMASKED(LOG_CONFIG, "Cartridge type is %s (%d)\n", pcb, m_pcbtype);
		m_rpk = nullptr;
	}
	else
	{
		auto reader = std::make_unique<rpk_reader>(pcbdefs);
		try
		{
			m_rpk = reader->open(machine().options(), filename(), machine().system().name);
			m_pcbtype = m_rpk->get_type();
		}
		catch (rpk_exception& err)
		{
			LOGMASKED(LOG_WARN, "Failed to load cartridge '%s': %s\n", basename(), err.to_string().c_str());
			m_rpk = nullptr;
			m_err = IMAGE_ERROR_INVALIDIMAGE;
			return image_init_result::FAIL;
		}
	}

	switch (m_pcbtype)
	{
	case PCB_STANDARD:
		LOGMASKED(LOG_CONFIG, "Standard PCB\n");
		m_pcb = std::make_unique<ti99_standard_cartridge>();
		break;
	case PCB_PAGED12K:
		LOGMASKED(LOG_CONFIG, "Paged PCB 12K\n");
		m_pcb = std::make_unique<ti99_paged12k_cartridge>();
		break;
	case PCB_PAGED16K:
		LOGMASKED(LOG_CONFIG, "Paged PCB 16K\n");
		m_pcb = std::make_unique<ti99_paged16k_cartridge>();
		break;
	case PCB_PAGED7:
		LOGMASKED(LOG_CONFIG, "Paged PCB 7000\n");
		m_pcb = std::make_unique<ti99_paged7_cartridge>();
		break;
	case PCB_MINIMEM:
		LOGMASKED(LOG_CONFIG, "Minimem PCB\n");
		m_pcb = std::make_unique<ti99_minimem_cartridge>();
		break;
	case PCB_SUPER:
		LOGMASKED(LOG_CONFIG, "Superspace PCB\n");
		m_pcb = std::make_unique<ti99_super_cartridge>();
		break;
	case PCB_MBX:
		LOGMASKED(LOG_CONFIG, "MBX PCB\n");
		m_pcb = std::make_unique<ti99_mbx_cartridge>();
		break;
	case PCB_PAGED379I:
		LOGMASKED(LOG_CONFIG, "Paged379i PCB\n");
		m_pcb = std::make_unique<ti99_paged379i_cartridge>();
		break;
	case PCB_PAGED378:
		LOGMASKED(LOG_CONFIG, "Paged378 PCB\n");
		m_pcb = std::make_unique<ti99_paged378_cartridge>();
		break;
	case PCB_PAGED377:
		LOGMASKED(LOG_CONFIG, "Paged377 PCB\n");
		m_pcb = std::make_unique<ti99_paged377_cartridge>();
		break;
	case PCB_PAGEDCRU:
		LOGMASKED(LOG_CONFIG, "PagedCRU PCB\n");
		m_pcb = std::make_unique<ti99_pagedcru_cartridge>();
		break;
	case PCB_GROMEMU:
		LOGMASKED(LOG_CONFIG, "Grom Emulation PCB\n");
		m_pcb = std::make_unique<ti99_gromemu_cartridge>();
		break;
	}

	prepare_cartridge();
	m_pcb->set_cartridge(this);
	m_pcb->set_tag(tag());
	m_slot = get_index_from_tagname();
	m_connector->insert(m_slot, this);
	return image_init_result::PASS;
}

void ti99_cartridge_device::call_unload()
{
	LOGMASKED(LOG_CHANGE, "Unload\n");
	if (m_rpk != nullptr)
	{
		m_rpk->close(); // will write NVRAM contents
		delete m_rpk;
	}
	else
	{
		// Softlist
		bool has_nvram = (get_software_region("nvram")!=nullptr);
		if (has_nvram)
		{
			int nvsize = get_software_region_length("nvram");
			battery_save(m_pcb->m_ram_ptr, nvsize);
		}
	}

	m_pcb = nullptr;
	m_connector->remove(m_slot);
}

void ti99_cartridge_device::set_slot(int i)
{
	m_slot = i;
}

void ti99_cartridge_device::readz(offs_t offset, uint8_t *value)
{
	if (m_pcb != nullptr)
		m_pcb->readz(offset, value);
}

void ti99_cartridge_device::write(offs_t offset, uint8_t data)
{
	if (m_pcb != nullptr)
		m_pcb->write(offset, data);
}

void ti99_cartridge_device::crureadz(offs_t offset, uint8_t *value)
{
	if (m_pcb != nullptr) m_pcb->crureadz(offset, value);
}

void ti99_cartridge_device::cruwrite(offs_t offset, uint8_t data)
{
	if (m_pcb != nullptr) m_pcb->cruwrite(offset, data);
}

WRITE_LINE_MEMBER( ti99_cartridge_device::ready_line )
{
	m_connector->ready_line(state);
}

WRITE_LINE_MEMBER( ti99_cartridge_device::romgq_line )
{
	if (m_pcb != nullptr)
	{
		m_pcb->romgq_line(state);
		m_readrom = state;
	}
}

/*
    Combined select lines
*/
void ti99_cartridge_device::set_gromlines(line_state mline, line_state moline, line_state gsq)
{
	if (m_pcb != nullptr) m_pcb->set_gromlines(mline, moline, gsq);
}

WRITE_LINE_MEMBER(ti99_cartridge_device::gclock_in)
{
	if (m_pcb != nullptr) m_pcb->gclock_in(state);
}

bool ti99_cartridge_device::is_grom_idle()
{
	return (m_pcb != nullptr)? m_pcb->is_grom_idle() : false;
}

void ti99_cartridge_device::device_config_complete()
{
	m_connector = static_cast<cartridge_connector_device*>(owner());
}

/*
    5 GROMs that may be contained in a cartridge
*/
void ti99_cartridge_device::device_add_mconfig(machine_config& config)
{
	TMC0430(config, GROM3_TAG, CARTGROM_TAG, 0x0000, 3).ready_cb().set(FUNC(ti99_cartridge_device::ready_line));
	TMC0430(config, GROM4_TAG, CARTGROM_TAG, 0x2000, 4).ready_cb().set(FUNC(ti99_cartridge_device::ready_line));
	TMC0430(config, GROM5_TAG, CARTGROM_TAG, 0x4000, 5).ready_cb().set(FUNC(ti99_cartridge_device::ready_line));
	TMC0430(config, GROM6_TAG, CARTGROM_TAG, 0x6000, 6).ready_cb().set(FUNC(ti99_cartridge_device::ready_line));
	TMC0430(config, GROM7_TAG, CARTGROM_TAG, 0x8000, 7).ready_cb().set(FUNC(ti99_cartridge_device::ready_line));
}

/*
    Memory area for one cartridge. For most cartridges we only need 8 KiB for
    ROM contents, but cartridges of the "paged377" type have up to 2 MiB
    organised as selectable banks, so we must be sure there is enough space.
*/
ROM_START( cartridge_memory )
	ROM_REGION(0xa000, CARTGROM_TAG, ROMREGION_ERASE00)
	ROM_REGION(0x200000, CARTROM_TAG, ROMREGION_ERASE00)
ROM_END

const tiny_rom_entry *ti99_cartridge_device::device_rom_region() const
{
	return ROM_NAME( cartridge_memory );
}

/***************************************************************************
    Cartridge types
    Cartridges differ by the circuits on their PCB which hosts the ROMs.
    Some cartridges also have RAM, and some allow for switching between
    ROMs.

    Standard cartridge

    GROM space
    6000     77ff   8000     97ff   a000     b7ff   c000     d7ff   e000     f7ff
    |== GROM3 ==|...|== GROM4 ==|...|== GROM5 ==|...|== GROM6 ==|...|== GROM7 ==|


    ROM space
    6000          7000        7fff
    |             |              |
    |========== ROM1 ============|   (or RAM, e.g. in Myarc XB II)

***************************************************************************/

ti99_cartridge_pcb::ti99_cartridge_pcb()
	: m_cart(nullptr),
		m_grom_idle(false),
		m_grom_size(0),
		m_rom_size(0),
		m_ram_size(0),
		m_rom_ptr(nullptr),
		m_ram_ptr(nullptr),
		m_romspace_selected(false),
		m_rom_page(0),
		m_grom_ptr(nullptr),
		m_grom_address(0),
		m_ram_page(0),
		m_tag(nullptr)
{
}

void ti99_cartridge_pcb::set_cartridge(ti99_cartridge_device *cart)
{
	m_cart = cart;
}

void ti99_cartridge_pcb::gromreadz(uint8_t* value)
{
	for (auto & elem : m_grom)
	{
		if (elem != nullptr)
		{
			elem->readz(value);
		}
	}
}

void ti99_cartridge_pcb::gromwrite(uint8_t data)
{
	for (auto & elem : m_grom)
	{
		if (elem != nullptr)
		{
			elem->write(data);
		}
	}
}

/*
    TI-99/4A cartridges can only occupy 8 KiB of CPU RAM space. For TI-99/8
    cartridges with up to 16 KiB we need a new PCB type. Unfortunately, such
    cartridges were never developed.
*/
void ti99_cartridge_pcb::readz(offs_t offset, uint8_t *value)
{
	if (m_romspace_selected)
	{
		if (m_rom_ptr!=nullptr)
		{
			*value = m_rom_ptr[offset & 0x1fff];
		}
		else
		{
			// Check if we have RAM in the ROM socket
			if ((offset & 0x1fff) < m_ram_size)
				*value = m_ram_ptr[offset & 0x1fff];
		}
	}
	else
	{
		// Will not return anything when not selected (preceding gsq=ASSERT)
		gromreadz(value);
	}
}

void ti99_cartridge_pcb::write(offs_t offset, uint8_t data)
{
	if (m_romspace_selected)
	{
		// Do not warn by default; devices like Horizon will create a lot of
		// meaningless warnings at this point
		if (m_ram_ptr == nullptr) LOGMASKED(LOG_WARNW, "Cannot write to cartridge ROM space at %04x\n", offset | 0x6000);
		else
		{
			// Check if we have RAM in the ROM socket
			if ((offset & 0x1fff) < m_ram_size)
				m_ram_ptr[offset & 0x1fff] = data;
			else
				LOGMASKED(LOG_WARN, "Cannot write to cartridge RAM space at %04x\n", offset | 0x6000);
		}
	}
	else
	{
		// Will not change anything when not selected (preceding gsq=ASSERT)
		gromwrite(data);
	}
}

void ti99_cartridge_pcb::crureadz(offs_t offset, uint8_t *value)
{
}

void ti99_cartridge_pcb::cruwrite(offs_t offset, uint8_t data)
{
}

void ti99_cartridge_pcb::set_grom_pointer(int number, device_t *dev)
{
	m_grom[number] = static_cast<tmc0430_device*>(dev);
}


WRITE_LINE_MEMBER( ti99_cartridge_pcb::romgq_line )
{
	m_romspace_selected = (state==ASSERT_LINE);
}

// Propagate to all GROMs

/*
    Combined select lines
*/
void ti99_cartridge_pcb::set_gromlines(line_state mline, line_state moline, line_state gsq)
{
	for (auto& elem : m_grom)
	{
		if (elem != nullptr)
		{
			elem->set_lines(mline, moline, gsq);
			if (gsq==ASSERT_LINE) m_grom_idle = false;
		}
	}
}

WRITE_LINE_MEMBER(ti99_cartridge_pcb::gclock_in)
{
	for (auto& elem : m_grom)
	{
		if (elem != nullptr)
		{
			elem->gclock_in(state);
			m_grom_idle = elem->idle();
		}
	}
}


/*****************************************************************************
  Cartridge type: Paged (12K, Extended Basic)

  The Extended Basic cartridge consists of several GROMs which are
  treated in the usual way, and two ROMs (4K and 8K).

  GROM space
  6000     77ff   8000     97ff   a000     b7ff   c000     d7ff   e000     f7ff
  |== GROM3 ==|...|== GROM4 ==|...|== GROM5 ==|...|== GROM6 ==|...|== GROM7 ==|

  ROM space
  6000          7000        7fff
  |             |              |
  |             |==== ROM2a ===|     Bank 0    write to 6000, 6004, ... 7ffc
  |=== ROM1 ====|              |
                |==== ROM2b ===|     Bank 1    write to 6002, 6006, ... 7ffe

  The 4K ROM is mapped into the 6000-6FFF space.
  The 8K ROM is actually composed of two banks of 4K which are mapped into
  the 7000-7FFF space. Bank 0 is visible after a write access to 6000 / 6004 /
  6008 ... , while bank 1 is visible after writing to 6002 / 6006 / 600A / ...

******************************************************************************/

void ti99_paged12k_cartridge::readz(offs_t offset, uint8_t *value)
{
	if (m_romspace_selected)
	{
		// rom_ptr: 0000-0fff = rom1
		//          2000-2fff = rom2a
		//          3000-3fff = rom2b
		if ((offset & 0x1000)==0)
			*value = m_rom_ptr[offset & 0x0fff];
		else
			*value = m_rom_ptr[(offset & 0x0fff) | 0x2000 | (m_rom_page << 12)];
	}
	else
	{
		// Will not return anything when not selected (preceding gsq=ASSERT)
		gromreadz(value);
	}
}

void ti99_paged12k_cartridge::write(offs_t offset, uint8_t data)
{
	if (m_romspace_selected)
	{
		m_rom_page = (offset >> 1) & 1;
		if ((offset & 1)==0)
			LOGMASKED(LOG_BANKSWITCH, "Set ROM page = %d (writing to %04x)\n", m_rom_page, (offset | 0x6000));
	}
	else
	{
		// Will not change anything when not selected (preceding gsq=ASSERT)
		gromwrite(data);
	}
}

/*****************************************************************************
  Cartridge type: Paged (16K)

  GROM space
  6000     77ff   8000     97ff   a000     b7ff   c000     d7ff   e000     f7ff
  |== GROM3 ==|...|== GROM4 ==|...|== GROM5 ==|...|== GROM6 ==|...|== GROM7 ==|

  ROM space
  6000         7000        7fff
  |             |             |
  |========== ROM1 ===========|     Bank 0    write to 6000, 6004, ... 7ffc
  |             |             |
  |========== ROM2 ===========|     Bank 1    write to 6002, 6006, ... 7ffe

  This cartridge consists of GROM memory and 2 pages of standard ROM.
    The page is set by writing any value to a location in
    the address area, where an even word offset sets the page to 0 and an
    odd word offset sets the page to 1 (e.g. 6000 = bank 0, and
    6002 = bank 1).
******************************************************************************/

void ti99_paged16k_cartridge::readz(offs_t offset, uint8_t *value)
{
	if (m_romspace_selected)
	{
		*value = m_rom_ptr[(offset & 0x1fff) | (m_rom_page << 13)];
	}
	else
	{
		// Will not return anything when not selected (preceding gsq=ASSERT)
		gromreadz(value);
	}
}

void ti99_paged16k_cartridge::write(offs_t offset, uint8_t data)
{
	if (m_romspace_selected)
	{
		m_rom_page = (offset >> 1) & 1;
		if ((offset & 1)==0)
			LOGMASKED(LOG_BANKSWITCH, "Set ROM page = %d (writing to %04x)\n", m_rom_page, (offset | 0x6000));
	}
	else
	{
		// Will not change anything when not selected (preceding gsq=ASSERT)
		gromwrite(data);
	}
}

/*****************************************************************************
  Cartridge type: Mini Memory
    GROM: 6 KiB (occupies G>6000 to G>77ff)
    ROM: 4 KiB (6000-6fff)
    RAM: 4 KiB (7000-7fff, battery-backed)

    GROM space
    6000     77ff
    |== GROM3 ==|

    ROM space
    6000         7000        7fff
    |             |             |
    |=== ROM1 ====|             |
                  |=== NVRAM ===|

******************************************************************************/

/* Read function for the minimem cartridge. */
void ti99_minimem_cartridge::readz(offs_t offset, uint8_t *value)
{
	if (m_romspace_selected)
	{
		if ((offset & 0x1000)==0x0000)
		{
			if (m_rom_ptr!=nullptr)    // Super-Minimem seems to have no ROM
			{
				*value = m_rom_ptr[offset & 0x0fff];
			}
		}
		else
		{
			*value = m_ram_ptr[offset & 0x0fff];
		}
	}
	else
	{
		gromreadz(value);
	}
}

/* Write function for the minimem cartridge. */
void ti99_minimem_cartridge::write(offs_t offset, uint8_t data)
{
	if (m_romspace_selected)
	{
		if ((offset & 0x1000)==0x0000)
		{
			LOGMASKED(LOG_WARN, "Write access to cartridge ROM at address %04x ignored", offset);
		}
		else
		{
			m_ram_ptr[offset & 0x0fff] = data;
		}
	}
	else
	{
		gromwrite(data);
	}
}

/*****************************************************************************
    Cartridge type: SuperSpace II

    SuperSpace is intended as a user-definable blank cartridge containing
    buffered RAM. It has an Editor/Assembler GROM which helps the user to load
    the user program into the cartridge. If the user program has a suitable
    header, the console recognizes the cartridge as runnable, and
    assigns a number in the selection screen. Switching the RAM banks in this
    cartridge is achieved by setting CRU bits (the system serial interface).

    GROM:    Editor/Assembler GROM
    ROM:     none
    RAM:     32 KiB (0x6000-0x7fff, 4 banks)
    Banking: via CRU write

    GROM space
    6000                       77ff
    |==== GROM3 (Editor/Assm) ====|

    ROM space
    6000         7000        7fff
    |             |             |
    |======== NVRAM 0 ==========|      Bank 0       CRU>0802
    |             |             |
    |======== NVRAM 1 ==========|      Bank 1       CRU>0806
    |             |             |
    |======== NVRAM 2 ==========|      Bank 2       CRU>080a
    |             |             |
    |======== NVRAM 3 ==========|      Bank 3       CRU>080e

******************************************************************************/

/* Read function for the super cartridge. */
void ti99_super_cartridge::readz(offs_t offset, uint8_t *value)
{
	if (m_romspace_selected)
	{
		if (m_ram_ptr != nullptr)
		{
			*value = m_ram_ptr[(m_ram_page << 13) | (offset & 0x1fff)];
		}
	}
	else
	{
		gromreadz(value);
	}
}

/* Write function for the super cartridge. */
void ti99_super_cartridge::write(offs_t offset, uint8_t data)
{
	if (m_romspace_selected)
	{
		m_ram_ptr[(m_ram_page << 13) | (offset & 0x1fff)] = data;
	}
	else
	{
		gromwrite(data);
	}
}

void ti99_super_cartridge::crureadz(offs_t offset, uint8_t *value)
{
	// offset is the bit number. The CRU base address is already divided  by 2.

	// ram_page contains the bank number. We have a maximum of
	// 4 banks; the Super Space II manual says:
	//
	// Banks are selected by writing a bit pattern to CRU address >0800:
	//
	// Bank #   Value
	// 0        >02  = 0000 0010
	// 1        >08  = 0000 1000
	// 2        >20  = 0010 0000
	// 3        >80  = 1000 0000
	//
	// With the bank number (0, 1, 2, or 3) in R0:
	//
	// BNKSW   LI    R12,>0800   Set CRU address
	//         LI    R1,2        Load Shift Bit
	//         SLA   R0,1        Align Bank Number
	//         JEQ   BNKS1       Skip shift if Bank 0
	//         SLA   R1,0        Align Shift Bit
	// BNKS1   LDCR  R1,0        Switch Banks
	//         SRL   R0,1        Restore Bank Number (optional)
	//         RT

	if ((offset & 0xfff0) == 0x0800)
	{
		LOGMASKED(LOG_CRU, "CRU accessed at %04x\n", offset);
		uint8_t val = 0x02 << (m_ram_page << 1);
		*value = BIT(val, (offset & 0x000e) >> 1);
	}
}

void ti99_super_cartridge::cruwrite(offs_t offset, uint8_t data)
{
	if ((offset & 0xfff0) == 0x0800)
	{
		LOGMASKED(LOG_CRU, "CRU accessed at %04x\n", offset);
		if (data != 0)
		{
			m_ram_page = (offset-0x0802)>>2;
			if ((offset & 1)==0)
				LOGMASKED(LOG_BANKSWITCH, "Set RAM page = %d (CRU address %04x)\n", m_ram_page, offset);
		}
	}
}

/*****************************************************************************
  Cartridge type: MBX
    GROM: up to 5 GROMs (sockets for a maximum of 3 GROMs, but may be stacked)
    ROM: up to 16 KiB (in up to 2 banks of 8KiB each)
    RAM: 1022 B (0x6c00-0x6ffd, overrides ROM in that area)
    ROM mapper: 6ffe

    TODO: Some MBX cartridges assume the presence of the MBX system
    (special user interface box with speech input/output)
    and will not run without it. This MBX hardware is not emulated yet.

    GROM space
    6000     77ff   8000     97ff   a000     b7ff   c000     d7ff   e000     f7ff
    |== GROM3 ==|...|== GROM4 ==|...|== GROM5 ==|...|== GROM6 ==|...|== GROM7 ==|

    ROM space
    6000             6c00    7000                 7fff
    |                 |       |                     |
    |                 |       |===== ROM bank 0 ====|       6ffe = 00
    |                 |= RAM =|                     |
    |=== ROM bank 0 ==|       |===== ROM bank 1 ====|       6ffe = 01
                              |                     |
                              |===== ROM bank 2 ====|       6ffe = 02
                              |                     |
                              |===== ROM bank 3 ====|       6ffe = 03

    The 16K ROM is composed of four 4K banks, which can be selected by writing
    the bank number to address 6ffe. This also affects the RAM so that the
    bank number is stored in RAM and may also be read from there.

    The mapper does not decode the LSB of the address, so it changes value when
    a write operation is done on 6FFF. Since the TI console always writes the
    odd byte first, then the even byte, the last byte written is actually 6FFE.

    ROM bank 0 (ROM area 0000-0fff) is always visible in the space 6000-6bff.

    RAM is implemented by two 1024x4 RAM circuits and is not affected by banking.

******************************************************************************/

/* Read function for the mbx cartridge. */
void ti99_mbx_cartridge::readz(offs_t offset, uint8_t *value)
{
	if (m_romspace_selected)
	{
		if (m_ram_ptr != nullptr && (offset & 0x1c00)==0x0c00)
		{
			// Also reads the value of 6ffe
			*value = m_ram_ptr[offset & 0x03ff];
			LOGMASKED(LOG_READ, "%04x (RAM) -> %02x\n", offset + 0x6000, *value);
		}
		else
		{
			if (m_rom_ptr!=nullptr)
			{
				if ((offset & 0x1000)==0)  // 6000 area
					*value = m_rom_ptr[offset];
				else  // 7000 area
					*value = m_rom_ptr[(offset & 0x0fff) | (m_rom_page << 12)];

				LOGMASKED(LOG_READ, "%04x(%04x) -> %02x\n", offset + 0x6000, offset | (m_rom_page<<13), *value);
			}
		}
	}
	else
	{
		gromreadz(value);
	}
}

/* Write function for the mbx cartridge. */
void ti99_mbx_cartridge::write(offs_t offset, uint8_t data)
{
	if (m_romspace_selected)
	{
		if ((offset & 0x1c00)==0x0c00)  // RAM area
		{
			if ((offset & 0x0ffe) == 0x0ffe)   // Mapper, backed by RAM; reacts to bots 6fff and 6ffe
			{
				// Valid values are 0, 1, 2, 3
				m_rom_page = data & 3;
				if ((offset & 1)==0)
					LOGMASKED(LOG_BANKSWITCH, "Set ROM page = %d (writing to %04x)\n", m_rom_page, (offset | 0x6000));
			}

			if (m_ram_ptr != nullptr)
				m_ram_ptr[offset & 0x03ff] = data;
			else
				LOGMASKED(LOG_WARN, "Write access to %04x but no RAM present\n", offset+0x6000);
		}
	}
	else
	{
		gromwrite(data);
	}
}


/*****************************************************************************
  Cartridge type: paged7
    GROM: up to 5 GROMs (sockets for a maximum of 3 GROMs, but may be stacked)
    ROM: up to 16 KiB (in up to 2 banks of 8KiB each)
    ROM mapper: 7000, 7002, 7004, 7006

    GROM space
    6000     77ff   8000     97ff   a000     b7ff   c000     d7ff   e000     f7ff
    |== GROM3 ==|...|== GROM4 ==|...|== GROM5 ==|...|== GROM6 ==|...|== GROM7 ==|

    ROM space
    6000                    7000                 7fff
    |                        |                     |
    |                        |===== ROM bank 0 ====|       write to 7000
    |                        |                     |
    |=== ROM bank 0 =========|===== ROM bank 1 ====|       write to 7002
                             |                     |
                             |===== ROM bank 2 ====|       write to 7004
                             |                     |
                             |===== ROM bank 3 ====|       write to 7006

    This is very similar to the MBX scheme, only that the bank switch is
    done by writing to the first words of the 7000 space. Also, there is no
    additional RAM.

******************************************************************************/

/* Read function for the paged7 cartridge. */
void ti99_paged7_cartridge::readz(offs_t offset, uint8_t *value)
{
	if (m_romspace_selected)
	{
		if (m_rom_ptr!=nullptr)
		{
			if ((offset & 0x1000)==0x0000)  // 6000 area
				*value = m_rom_ptr[offset];
			else  // 7000 area
				*value = m_rom_ptr[(offset & 0x0fff) | (m_rom_page << 12)];

			LOGMASKED(LOG_READ, "%04x(%04x) -> %02x\n", offset + 0x6000, offset | (m_rom_page<<13), *value);
		}
	}
	else
	{
		gromreadz(value);
	}
}

/* Write function for the paged7 cartridge. */
void ti99_paged7_cartridge::write(offs_t offset, uint8_t data)
{
	if (m_romspace_selected)
	{
		// 0111 0000 0000 0110
		if ((offset & 0x1ff9) == 0x1000)   // Mapper
		{
			// Valid values are 0, 1, 2, 3
			m_rom_page = (offset>>1) & 3;
			if ((offset & 1)==0)
				LOGMASKED(LOG_BANKSWITCH, "Set ROM page = %d (writing to %04x)\n", m_rom_page, (offset | 0x6000));
		}
	}
	else
	{
		gromwrite(data);
	}
}

/*****************************************************************************
  Cartridge type: paged379i
    This cartridge consists of one 16 KiB, 32 KiB, 64 KiB, or 128 KiB EEPROM
    which is organised in 2, 4, 8, or 16 pages of 8 KiB each. The complete
    memory contents must be stored in one dump file.
    The pages are selected by writing a value to some memory locations. Due to
    using the inverted outputs of the LS379 latch, setting the inputs of the
    latch to all 0 selects the highest bank, while setting to all 1 selects the
    lowest. There are some cartridges (16 KiB) which are using this scheme, and
    there are new hardware developments mainly relying on this scheme.

    Writing to       selects page (16K/32K/64K/128K)
    >6000            1 / 3 / 7 / 15
    >6002            0 / 2 / 6 / 14
    >6004            1 / 1 / 5 / 13
    >6006            0 / 0 / 4 / 12
    >6008            1 / 3 / 3 / 11
    >600A            0 / 2 / 2 / 10
    >600C            1 / 1 / 1 / 9
    >600E            0 / 0 / 0 / 8
    >6010            1 / 3 / 7 / 7
    >6012            0 / 2 / 6 / 6
    >6014            1 / 1 / 5 / 5
    >6016            0 / 0 / 4 / 4
    >6018            1 / 3 / 3 / 3
    >601A            0 / 2 / 2 / 2
    >601C            1 / 1 / 1 / 1
    >601E            0 / 0 / 0 / 0

    The paged379i cartrige does not have any GROMs.

    ROM space
    6000         7000        7fff
    |             |             |
    |========== ROM 1 ==========|      Bank 0      Write to 601e
    |             |             |
    |========== ROM 2 ==========|      Bank 1      Write to 601c
    |             |             |
    |            ...            |
    |             |             |
    |========== ROM 16 =========|      Bank 15     Write to 6000


******************************************************************************/

/* Read function for the paged379i cartridge. */
void ti99_paged379i_cartridge::readz(offs_t offset, uint8_t *value)
{
	if (m_romspace_selected)
		*value = m_rom_ptr[(m_rom_page<<13) | (offset & 0x1fff)];
}

/* Write function for the paged379i cartridge. Only used to set the bank. */
void ti99_paged379i_cartridge::write(offs_t offset, uint8_t data)
{
	// Bits: 011x xxxx xxxb bbbx
	// x = don't care, bbbb = bank
	if (m_romspace_selected)
	{
		// This is emulation magic to automatically adapt to different ROM sizes

		// Each block has 8 KiB. We assume that m_rom_size is a power of 2.
		// Thus the number of blocks is also a power of 2.
		// To get the required number of address lines, we just have to subtract 1.
		// The SN74LS379 only has four flipflops, so we limit the lines to 4.
		int mask = ((m_rom_size / 8192) - 1) & 0x0f;

		// The page is determined by the inverted outputs.
		m_rom_page = (~offset)>>1 & mask;
		if ((offset & 1)==0)
			LOGMASKED(LOG_BANKSWITCH, "Set ROM page = %d (writing to %04x)\n", m_rom_page, (offset | 0x6000));
	}
}

/*****************************************************************************
  Cartridge type: paged378
  This type is intended for high-capacity cartridges of up to 512 KiB
  plus GROM space of 120KiB (not supported yet)
  For smaller ROMs, the ROM is automatically mirrored in the bank space.

  Due to its huge GROM space it is also called the "UberGROM"

  The cartridge may also be used without GROM.

    ROM space
    6000         7000        7fff
    |             |             |
    |========== ROM 1 ==========|      Bank 0      Write to 6000
    |             |             |
    |========== ROM 2 ==========|      Bank 1      Write to 6002
    |             |             |
    |            ...            |
    |             |             |
    |========== ROM 64 =========|      Bank 63     Write to 607e

******************************************************************************/

/* Read function for the paged378 cartridge. */
void ti99_paged378_cartridge::readz(offs_t offset, uint8_t *value)
{
	if (m_romspace_selected)
		*value = m_rom_ptr[(m_rom_page<<13) | (offset & 0x1fff)];
}

/* Write function for the paged378 cartridge. Only used to set the bank. */
void ti99_paged378_cartridge::write(offs_t offset, uint8_t data)
{
	// Bits: 011x xxxx xbbb bbbx
	// x = don't care, bbbb = bank
	if (m_romspace_selected)
	{
		// Auto-adapt to the size of the ROM
		int mask = ((m_rom_size / 8192) - 1) & 0x3f;
		m_rom_page = ((offset >> 1)&mask);

		if ((offset & 1)==0)
			LOGMASKED(LOG_BANKSWITCH, "Set ROM page = %d (writing to %04x)\n", m_rom_page, (offset | 0x6000));
	}
}

/*****************************************************************************
  Cartridge type: paged377
  This type is intended for high-capacity cartridges of up to 2 MiB

  The paged379i cartrige does not have any GROMs.

    ROM space
    6000         7000        7fff
    |             |             |
    |========== ROM 1 ==========|      Bank 0      Write to 6000
    |             |             |
    |========== ROM 2 ==========|      Bank 1      Write to 6002
    |             |             |
    |            ...            |
    |             |             |
    |========== ROM 256 ========|      Bank 255    Write to 61fe


******************************************************************************/

/* Read function for the paged377 cartridge. */
void ti99_paged377_cartridge::readz(offs_t offset, uint8_t *value)
{
	if (m_romspace_selected)
		*value = m_rom_ptr[(m_rom_page<<13) | (offset & 0x1fff)];
}

/* Write function for the paged377 cartridge. Only used to set the bank. */
void ti99_paged377_cartridge::write(offs_t offset, uint8_t data)
{
	// Bits: 011x xxxb bbbb bbbx
	// x = don't care, bbbb = bank
	if (m_romspace_selected)
	{
		m_rom_page = ((offset >> 1)&0x00ff);
		if ((offset & 1)==0)
			LOGMASKED(LOG_BANKSWITCH, "Set ROM page = %d (writing to %04x)\n", m_rom_page, (offset | 0x6000));
	}
}

/*****************************************************************************
  Cartridge type: pagedcru
    This cartridge consists of one 16 KiB, 32 KiB, or 64 KiB EEPROM which is
    organised in 2, 4, or 8 pages of 8 KiB each. We assume there is only one
    dump file of the respective size.
    The pages are selected by writing a value to the CRU. This scheme is
    similar to the one used for the SuperSpace cartridge, with the exception
    that we are using ROM only, and we can have up to 8 pages.

    Bank     Value written to CRU>0800
    0      >0002  = 0000 0000 0000 0010
    1      >0008  = 0000 0000 0000 1000
    2      >0020  = 0000 0000 0010 0000
    3      >0080  = 0000 0000 1000 0000
    4      >0200  = 0000 0010 0000 0000
    5      >0800  = 0000 1000 0000 0000
    6      >2000  = 0010 0000 0000 0000
    7      >8000  = 1000 0000 0000 0000

    No GROMs used in this type.

    ROM space
    6000         7000        7fff
    |             |             |
    |=========  ROM 1 ==========|      Bank 0       CRU>0802
    |             |             |
    |=========  ROM 2 ==========|      Bank 1       CRU>0806
    |             |             |
                 ...
    |             |             |
    |=========  ROM 8 ==========|      Bank 7       CRU>081e

******************************************************************************/

/* Read function for the pagedcru cartridge. */
void ti99_pagedcru_cartridge::readz(offs_t offset, uint8_t *value)
{
	if (m_romspace_selected)
		*value = m_rom_ptr[(m_rom_page<<13) | (offset & 0x1fff)];
}

/* Write function for the pagedcru cartridge. No effect. */
void ti99_pagedcru_cartridge::write(offs_t offset, uint8_t data)
{
	return;
}

void ti99_pagedcru_cartridge::crureadz(offs_t offset, uint8_t *value)
{
	int page = m_rom_page;
	if ((offset & 0xf800)==0x0800)
	{
		int bit = (offset & 0x001e)>>1;
		if (bit != 0)
		{
			page = page-(bit/2);  // 4 page flags per 8 bits
		}
		*value = (offset & 0x000e) == (page * 4 + 2) ? 1 : 0;
	}
}

void ti99_pagedcru_cartridge::cruwrite(offs_t offset, uint8_t data)
{
	if ((offset & 0xf800)==0x0800)
	{
		int bit = (offset & 0x001e)>>1;
		if (data != 0 && bit > 0)
		{
			m_rom_page = (bit-1)/2;
			LOGMASKED(LOG_BANKSWITCH, "Set ROM page = %d (CRU address %d)\n", m_rom_page, offset);
		}
	}
}

/*****************************************************************************
  Cartridge type: GROM emulation/paged

  This cartridge offers GROM address space without real GROM circuits. The GROMs
  are emulated by a normal EPROM with a circuit that mimics GROM behavior.
  Each simulated GROM offers 8K (real GROMs only offer 6K).

  Some assumptions:
  - No readable address counter. This means the parallel console GROMs
    will deliver the address when reading.
  - No wait states. Reading is generally faster than with real GROMs.
  - No wrapping at 8K boundaries.
  - One or two ROM sockets; if one socket, the standard bank switch scheme is
    used

  Typical cartridges: Third-party cartridges

  For the sake of simplicity, we register GROMs like the other PCB types, but
  we implement special access methods for the GROM space.

  Super-MiniMemory is also included here. We assume a RAM area at addresses
  7000-7fff for this cartridge if the RAM option is used.


  GROM space
  6000     77ff   8000     97ff   a000     b7ff   c000     d7ff   e000    ffff
  |=========================== emulated GROM ================================|

  ROM space
  6000         7000        7fff
  |             |             |
  |========== ROM1 ===========|     Bank 0
  |             |             |
  |========== ROM2 ===========|     Bank 1
  ...                       ...
  |========== ROMn ===========|     Bank n-1

  Depending on the number of banks, a number of address bits is used to select
  the bank:

  Write to 011x xxxx xxxx xxx0 -> Set bank number to xxxxxxxxxxxx

  The number xxxxxxxxxxxx has just enough bits to encode the highest bank number.
  Higher bits are ignored.

  If rom2_socket is used, we assume that rom_socket and rom2_socket contain
  8K ROM each, so we have exactly two banks, regardless of the ROM length.

******************************************************************************/

void ti99_gromemu_cartridge::set_gromlines(line_state mline, line_state moline, line_state gsq)
{
	if (m_grom_ptr != nullptr)
	{
		m_grom_selected = (gsq == ASSERT_LINE);
		m_grom_read_mode = (mline == ASSERT_LINE);
		m_grom_address_mode = (moline == ASSERT_LINE);
	}
}

void ti99_gromemu_cartridge::readz(offs_t offset, uint8_t *value)
{
	if (m_grom_selected)
	{
		if (m_grom_read_mode) gromemureadz(offset, value);
	}
	else
	{
		if (m_ram_ptr != nullptr)
		{
			// Variant of the cartridge which emulates MiniMemory. We don't introduce
			// another type for this single cartridge.
			if ((offset & 0x1000)==0x1000)
			{
				*value = m_ram_ptr[offset & 0x0fff];
				return;
			}
		}

		if (m_rom_ptr == nullptr) return;
		*value = m_rom_ptr[(offset & 0x1fff) | (m_rom_page << 13)];
	}
}

void ti99_gromemu_cartridge::write(offs_t offset, uint8_t data)
{
	if (m_romspace_selected)
	{
		if (m_ram_ptr != nullptr)
		{
			// Lines for Super-Minimem; see above
			if ((offset & 0x1000)==0x1000) {
				m_ram_ptr[offset & 0x0fff] = data;
			}
			return; // no paging
		}

		m_rom_page = (offset >> 1) & m_bank_mask;

		if ((offset & 1)==0)
			LOGMASKED(LOG_BANKSWITCH, "Set ROM page = %d (writing to %04x)\n", m_rom_page, (offset | 0x6000));
	}
	else
	{
		// Will not change anything when not selected (preceding gsq=ASSERT)
		if (m_grom_selected)
		{
			if (!m_grom_read_mode) gromemuwrite(offset, data);
		}
	}
}

void ti99_gromemu_cartridge::gromemureadz(offs_t offset, uint8_t *value)
{
	// Similar to the GKracker implemented above, we do not have a readable
	// GROM address counter but use the one from the console GROMs.
	if (m_grom_address_mode) return;

	int id = ((m_grom_address & 0xe000)>>13)&0x07;
	if (id > 2)
	{
		// Cartridge space (0x6000 - 0xffff)
		*value = m_grom_ptr[m_grom_address-0x6000]; // use the GROM memory
	}

	// The GROM emulation does not wrap at 8K boundaries.
	m_grom_address = (m_grom_address + 1) & 0xffff;

	// Reset the write address flipflop.
	m_waddr_LSB = false;
}

void ti99_gromemu_cartridge::gromemuwrite(offs_t offset, uint8_t data)
{
	// Set GROM address
	if (m_grom_address_mode)
	{
		if (m_waddr_LSB == true)
		{
			// Accept low address byte (second write)
			m_grom_address = (m_grom_address << 8) | data;
			m_waddr_LSB = false;
		}
		else
		{
			// Accept high address byte (first write)
			m_grom_address = data;
			m_waddr_LSB = true;
		}
	}
	else
	{
		LOGMASKED(LOG_WARN, "Ignoring write to GROM area at address %04x\n", m_grom_address);
	}
}


/****************************************************************************

    RPK loader

    RPK format support

    A RPK file ("rompack") contains a collection of dump files and a layout
    file that defines the kind of circuit board (PCB) used in the cartridge
    and the mapping of dumps to sockets on the board.

Example:
    <?xml version="1.0" encoding="utf-8"?>
    <romset>
        <resources>
            <rom id="gromimage" file="ed-assmg.bin" />
        </resources>
        <configuration>
            <pcb type="standard">
                <socket id="grom_socket" uses="gromimage"/>
            </pcb>
        </configuration>
    </romset>

DTD:
    <!ELEMENT romset (resources, configuration)>
    <!ELEMENT resources (rom|ram)+>
    <!ELEMENT rom EMPTY>
    <!ELEMENT ram EMPTY>
    <!ELEMENT configuration (pcb)>
    <!ELEMENT pcb (socket)+>
    <!ELEMENT socket EMPTY>
    <!ATTLIST romset version CDATA #IMPLIED>
    <!ATTLIST rom id ID #REQUIRED
    <!ATTLIST rom file CDATA #REQUIRED>
    <!ATTLIST rom crc CDATA #IMPLIED>
    <!ATTLIST rom sha1 CDATA #IMPLIED>
    <!ATTLIST ram id ID #REQUIRED>
    <!ATTLIST ram type (volatile|persistent) #IMPLIED>
    <!ATTLIST ram store (internal|external) #IMPLIED>
    <!ATTLIST ram file CDATA #IMPLIED>
    <!ATTLIST ram length CDATA #REQUIRED>
    <!ATTLIST pcb type CDATA #REQUIRED>
    <!ATTLIST socket id ID #REQUIRED>
    <!ATTLIST socket uses IDREF #REQUIRED>

****************************************************************************/

#undef LOG_OUTPUT_FUNC
#define LOG_OUTPUT_FUNC printf

/****************************************
    RPK class
****************************************/
/*
    Constructor.
*/
ti99_cartridge_device::rpk::rpk(emu_options& options, const char* sysname)
	:m_options(options), m_type(0)
//,m_system_name(sysname)
{
	m_sockets.clear();
}

ti99_cartridge_device::rpk::~rpk()
{
	LOGMASKED(LOG_RPK, "[RPK handler] Destroy RPK\n");
}

/*
    Deliver the contents of the socket by name of the socket.
*/
uint8_t* ti99_cartridge_device::rpk::get_contents_of_socket(const char *socket_name)
{
	auto socket = m_sockets.find(socket_name);
	if (socket == m_sockets.end()) return nullptr;
	return socket->second->get_contents();
}

/*
    Deliver the length of the contents of the socket by name of the socket.
*/
int ti99_cartridge_device::rpk::get_resource_length(const char *socket_name)
{
	auto socket = m_sockets.find(socket_name);
	if (socket == m_sockets.end()) return 0;
	return socket->second->get_content_length();
}

void ti99_cartridge_device::rpk::add_socket(const char* id, std::unique_ptr<rpk_socket> newsock)
{
	m_sockets.emplace(id, std::move(newsock));
}

/*-------------------------------------------------
    rpk_close - closes a rpk
    Saves the contents of the NVRAMs and frees all memory.
-------------------------------------------------*/

void ti99_cartridge_device::rpk::close()
{
	// Save the NVRAM contents
	for(auto &socket : m_sockets)
	{
		if (socket.second->persistent_ram())
		{
			// try to open the battery file and write it if possible
			if (!socket.second->get_contents() || (socket.second->get_content_length() <= 0))
				throw emu_fatalerror("ti99_cartridge_device::rpk::close: Buffer is null or length is 0");

			emu_file file(m_options.nvram_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
			osd_file::error filerr = file.open(socket.second->get_pathname());
			if (filerr == osd_file::error::NONE)
				file.write(socket.second->get_contents(), socket.second->get_content_length());

		}
		socket.second->cleanup();
	}
}

/**************************************************************
    RPK socket (location in the PCB where a chip is plugged in;
    not a network socket)
***************************************************************/

ti99_cartridge_device::rpk_socket::rpk_socket(const char* id, int length, std::unique_ptr<uint8_t []> &&contents, std::string &&pathname)
	: m_id(id), m_length(length), m_contents(std::move(contents)), m_pathname(std::move(pathname))
{
}

ti99_cartridge_device::rpk_socket::rpk_socket(const char* id, int length, std::unique_ptr<uint8_t []> &&contents)
	: rpk_socket(id, length, std::move(contents), "")
{
}

/*
    Locate a file in the ZIP container
*/
int ti99_cartridge_device::rpk_reader::find_file(util::archive_file &zip, const char *filename, uint32_t crc)
{
	for (int header = zip.first_file(); header >= 0; header = zip.next_file())
	{
		// Ignore directories
		if (!zip.current_is_directory())
		{
			// We don't check for CRC == 0.
			if (crc != 0)
			{
				// if the CRC and name both match, we're good
				// if the CRC matches and the name doesn't, we're still good
				if (zip.current_crc() == crc)
					return header;
			}
			else
			{
				if (core_stricmp(zip.current_name().c_str(), filename) == 0)
				{
					return header;
				}
			}
		}
	}
	return -1;
}

/*
    Load a rom resource and put it in a pcb socket instance.
*/
std::unique_ptr<ti99_cartridge_device::rpk_socket> ti99_cartridge_device::rpk_reader::load_rom_resource(util::archive_file &zip, util::xml::data_node const* rom_resource_node, const char* socketname)
{
	const char* file;
	const char* crcstr;
	const char* sha1;
	util::archive_file::error ziperr;
	uint32_t crc;
	int length;
	std::unique_ptr<uint8_t []> contents;
	int header;

	// find the file attribute (required)
	file = rom_resource_node->get_attribute_string("file", nullptr);
	if (file == nullptr) throw rpk_exception(RPK_INVALID_LAYOUT, "<rom> must have a 'file' attribute");

	LOGMASKED(LOG_RPK, "[RPK handler] Loading ROM contents for socket '%s' from file %s\n", socketname, file);

	// check for crc
	crcstr = rom_resource_node->get_attribute_string("crc", nullptr);
	if (crcstr==nullptr)
	{
		// no CRC, just find the file in the RPK
		header = find_file(zip, file, 0);
	}
	else
	{
		crc = strtoul(crcstr, nullptr, 16);
		header = find_file(zip, file, crc);
	}
	if (header < 0) throw rpk_exception(RPK_INVALID_FILE_REF, "File not found or CRC check failed");

	length = zip.current_uncompressed_length();

	// Allocate storage
	try { contents = make_unique_clear<uint8_t []>(length); }
	catch (std::bad_alloc const &) { throw rpk_exception(RPK_OUT_OF_MEMORY); }

	// and unzip file from the zip file
	ziperr = zip.decompress(contents.get(), length);
	if (ziperr != util::archive_file::error::NONE)
	{
		if (ziperr == util::archive_file::error::UNSUPPORTED) throw rpk_exception(RPK_ZIP_UNSUPPORTED);
		else throw rpk_exception(RPK_ZIP_ERROR);
	}

	// check for sha1
	sha1 = rom_resource_node->get_attribute_string("sha1", nullptr);
	if (sha1 != nullptr)
	{
		util::hash_collection actual_hashes;
		actual_hashes.compute(contents.get(), length, util::hash_collection::HASH_TYPES_CRC_SHA1);

		util::hash_collection expected_hashes;
		expected_hashes.add_from_string(util::hash_collection::HASH_SHA1, sha1);

		if (actual_hashes != expected_hashes) throw rpk_exception(RPK_INVALID_FILE_REF, "SHA1 check failed");
	}

	// Create a socket instance
	return std::make_unique<rpk_socket>(socketname, length, std::move(contents));
}

/*
    Load a ram resource and put it in a pcb socket instance.
*/
std::unique_ptr<ti99_cartridge_device::rpk_socket> ti99_cartridge_device::rpk_reader::load_ram_resource(emu_options &options, util::xml::data_node const* ram_resource_node, const char* socketname, const char* system_name)
{
	const char* length_string;
	const char* ram_type;
	const char* ram_filename;
	std::string ram_pname;
	unsigned int length;
	std::unique_ptr<uint8_t []> contents;

	// find the length attribute
	length_string = ram_resource_node->get_attribute_string("length", nullptr);
	if (length_string == nullptr) throw rpk_exception(RPK_MISSING_RAM_LENGTH);

	// parse it
	char suffix = '\0';
	sscanf(length_string, "%u%c", &length, &suffix);
	switch(tolower(suffix))
	{
		case 'k': // kilobytes
			length *= 1024;
			break;

		case 'm':
			/* megabytes */
			length *= 1024*1024;
			break;

		case '\0':
			break;

		default:  // failed
			throw rpk_exception(RPK_INVALID_RAM_SPEC);
	}

	// Allocate memory for this resource
	try { contents = make_unique_clear<uint8_t []>(length); }
	catch (std::bad_alloc const &) { throw rpk_exception(RPK_OUT_OF_MEMORY); }

	LOGMASKED(LOG_RPK, "[RPK handler] Allocating RAM buffer (%d bytes) for socket '%s'\n", length, socketname);

	// That's it for pure RAM. Now check whether the RAM is "persistent", i.e. NVRAM.
	// In that case we must load it from the NVRAM directory.
	// The file name is given in the RPK file; the subdirectory is the system name.
	ram_type = ram_resource_node->get_attribute_string("type", nullptr);
	if (ram_type != nullptr)
	{
		if (strcmp(ram_type, "persistent")==0)
		{
			// Get the file name (required if persistent)
			ram_filename = ram_resource_node->get_attribute_string("file", nullptr);
			if (ram_filename==nullptr)
				throw rpk_exception(RPK_INVALID_RAM_SPEC, "<ram type='persistent'> must have a 'file' attribute");

			ram_pname = std::string(system_name).append(PATH_SEPARATOR).append(ram_filename);
			// load, and fill rest with 00
			LOGMASKED(LOG_RPK, "[RPK handler] Loading NVRAM contents from '%s'\n", ram_pname.c_str());

			// Load the NVRAM contents
			if (!contents || (length <= 0))
				throw emu_fatalerror("ti99_cartridge_device::rpk_reader::load_ram_resource: Buffer is null or length is 0");

			// try to open the battery file and read it if possible
			emu_file file(options.nvram_directory(), OPEN_FLAG_READ);
			osd_file::error filerr = file.open(ram_pname);
			int bytes_read = 0;
			if (filerr == osd_file::error::NONE)
				bytes_read = file.read(contents.get(), length);

			// fill remaining bytes (if necessary)
			std::fill_n(&contents[bytes_read], length - bytes_read, 0x00);
		}
	}

	// Create a socket instance
	return std::make_unique<rpk_socket>(socketname, length, std::move(contents), std::move(ram_pname));
}

/*-------------------------------------------------
    rpk_open - open a RPK file
    options - parameters from the settings; we need it only for the NVRAM directory
    system_name - name of the driver (also just for NVRAM handling)
-------------------------------------------------*/

ti99_cartridge_device::rpk* ti99_cartridge_device::rpk_reader::open(emu_options &options, const char *filename, const char *system_name)
{
	util::archive_file::error ziperr;

	util::archive_file::ptr zipfile;

	std::vector<char> layout_text;

	int i;

	auto newrpk = new rpk(options, system_name);

	try
	{
		/* open the ZIP file */
		ziperr = util::archive_file::open_zip(filename, zipfile);
		if (ziperr != util::archive_file::error::NONE) throw rpk_exception(RPK_NOT_ZIP_FORMAT);

		/* find the layout.xml file */
		if (find_file(*zipfile, "layout.xml", 0) < 0) throw rpk_exception(RPK_MISSING_LAYOUT);

		/* reserve space for the layout file contents (+1 for the termination) */
		layout_text.resize(zipfile->current_uncompressed_length() + 1);

		/* uncompress the layout text */
		ziperr = zipfile->decompress(&layout_text[0], zipfile->current_uncompressed_length());
		if (ziperr != util::archive_file::error::NONE)
		{
			if (ziperr == util::archive_file::error::UNSUPPORTED) throw rpk_exception(RPK_ZIP_UNSUPPORTED);
			else throw rpk_exception(RPK_ZIP_ERROR);
		}

		layout_text[zipfile->current_uncompressed_length()] = '\0';  // Null-terminate

		/* parse the layout text */
		util::xml::file::ptr const layout_xml = util::xml::file::string_read(&layout_text[0], nullptr);
		if (!layout_xml) throw rpk_exception(RPK_XML_ERROR);

		// Now we work within the XML tree

		// romset is the root node
		util::xml::data_node const *const romset_node = layout_xml->get_child("romset");
		if (!romset_node) throw rpk_exception(RPK_INVALID_LAYOUT, "document element must be <romset>");

		// resources is a child of romset
		util::xml::data_node const *const resources_node = romset_node->get_child("resources");
		if (!resources_node) throw rpk_exception(RPK_INVALID_LAYOUT, "<romset> must have a <resources> child");

		// configuration is a child of romset; we're actually interested in ...
		util::xml::data_node const *const configuration_node = romset_node->get_child("configuration");
		if (!configuration_node) throw rpk_exception(RPK_INVALID_LAYOUT, "<romset> must have a <configuration> child");

		// ... pcb, which is a child of configuration
		util::xml::data_node const *const pcb_node = configuration_node->get_child("pcb");
		if (!pcb_node) throw rpk_exception(RPK_INVALID_LAYOUT, "<configuration> must have a <pcb> child");

		// We'll try to find the PCB type on the provided type list.
		char const *const pcb_type = pcb_node->get_attribute_string("type", nullptr);
		if (!pcb_type) throw rpk_exception(RPK_INVALID_LAYOUT, "<pcb> must have a 'type' attribute");
		LOGMASKED(LOG_RPK, "[RPK handler] Cartridge says it has PCB type '%s'\n", pcb_type);

		i=0;
		do
		{
			if (strcmp(pcb_type, m_types[i].name)==0)
			{
				newrpk->m_type = m_types[i].id;
				break;
			}
			i++;
		} while (m_types[i].id != 0);

		if (m_types[i].id==0) throw rpk_exception(RPK_UNKNOWN_PCB_TYPE);

		// Find the sockets and load their respective resource
		for (util::xml::data_node const *socket_node = pcb_node->get_first_child();  socket_node != nullptr; socket_node = socket_node->get_next_sibling())
		{
			if (strcmp(socket_node->get_name(), "socket")!=0) throw rpk_exception(RPK_INVALID_LAYOUT, "<pcb> element has only <socket> children");
			char const *const id = socket_node->get_attribute_string("id", nullptr);
			if (!id) throw rpk_exception(RPK_INVALID_LAYOUT, "<socket> must have an 'id' attribute");
			char const *const uses_name = socket_node->get_attribute_string("uses", nullptr);
			if (!uses_name) throw rpk_exception(RPK_INVALID_LAYOUT, "<socket> must have a 'uses' attribute");

			bool found = false;
			// Locate the resource node
			for (util::xml::data_node const *resource_node = resources_node->get_first_child(); resource_node != nullptr; resource_node = resource_node->get_next_sibling())
			{
				char const *const resource_name = resource_node->get_attribute_string("id", nullptr);
				if (!resource_name) throw rpk_exception(RPK_INVALID_LAYOUT, "resource node must have an 'id' attribute");

				if (strcmp(resource_name, uses_name)==0)
				{
					// found it
					if (strcmp(resource_node->get_name(), "rom")==0)
					{
						newrpk->add_socket(id, load_rom_resource(*zipfile, resource_node, id));
					}
					else
					{
						if (strcmp(resource_node->get_name(), "ram")==0)
						{
							newrpk->add_socket(id, load_ram_resource(options, resource_node, id, system_name));
						}
						else throw rpk_exception(RPK_INVALID_LAYOUT, "resource node must be <rom> or <ram>");
					}
					found = true;
				}
			}
			if (!found) throw rpk_exception(RPK_INVALID_RESOURCE_REF, uses_name);
		}
	}
	catch (rpk_exception &)
	{
		newrpk->close();

		// rethrow the exception
		throw;
	}

	return newrpk;
}

} // end namespace bus::ti99::gromport
