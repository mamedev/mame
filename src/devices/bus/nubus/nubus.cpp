// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  nubus.c - NuBus bus and card emulation

  by R. Belmont, based heavily on Miodrag Milanovic's ISA8/16 implementation

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "nubus.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type NUBUS_SLOT = &device_creator<nubus_slot_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_slot_device - constructor
//-------------------------------------------------
nubus_slot_device::nubus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, NUBUS_SLOT, "NUBUS_SLOT", tag, owner, clock, "nubus_slot", __FILE__),
		device_slot_interface(mconfig, *this),
	m_nubus_tag(nullptr),
	m_nubus_slottag(nullptr)
{
}

nubus_slot_device::nubus_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_slot_interface(mconfig, *this), m_nubus_tag(nullptr), m_nubus_slottag(nullptr)
{
}

void nubus_slot_device::static_set_nubus_slot(device_t &device, const char *tag, const char *slottag)
{
	nubus_slot_device &nubus_card = dynamic_cast<nubus_slot_device &>(device);
	nubus_card.m_nubus_tag = tag;
	nubus_card.m_nubus_slottag = slottag;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_slot_device::device_start()
{
	device_nubus_card_interface *dev = dynamic_cast<device_nubus_card_interface *>(get_card_device());

	if (dev) device_nubus_card_interface::static_set_nubus_tag(*dev, m_nubus_tag, m_nubus_slottag);
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type NUBUS = &device_creator<nubus_device>;

void nubus_device::static_set_cputag(device_t &device, const char *tag)
{
	nubus_device &nubus = downcast<nubus_device &>(device);
	nubus.m_cputag = tag;
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_device - constructor
//-------------------------------------------------

nubus_device::nubus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, NUBUS, "NUBUS", tag, owner, clock, "nubus", __FILE__), m_maincpu(nullptr),
		m_out_irq9_cb(*this),
		m_out_irqa_cb(*this),
		m_out_irqb_cb(*this),
		m_out_irqc_cb(*this),
		m_out_irqd_cb(*this),
		m_out_irqe_cb(*this), m_cputag(nullptr)
{
}

nubus_device::nubus_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source), m_maincpu(nullptr),
		m_out_irq9_cb(*this),
		m_out_irqa_cb(*this),
		m_out_irqb_cb(*this),
		m_out_irqc_cb(*this),
		m_out_irqd_cb(*this),
		m_out_irqe_cb(*this), m_cputag(nullptr)
{
}
//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_device::device_start()
{
	m_maincpu = machine().device<cpu_device>(m_cputag);
	// resolve callbacks
	m_out_irq9_cb.resolve_safe();
	m_out_irqa_cb.resolve_safe();
	m_out_irqb_cb.resolve_safe();
	m_out_irqc_cb.resolve_safe();
	m_out_irqd_cb.resolve_safe();
	m_out_irqe_cb.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_device::device_reset()
{
}

void nubus_device::add_nubus_card(device_nubus_card_interface *card)
{
	m_device_list.append(*card);
}

void nubus_device::install_device(offs_t start, offs_t end, read8_delegate rhandler, write8_delegate whandler, UINT32 mask)
{
	m_maincpu = machine().device<cpu_device>(m_cputag);
	int buswidth = m_maincpu->space_config(AS_PROGRAM)->m_databus_width;
	switch(buswidth)
	{
		case 32:
			m_maincpu->space(AS_PROGRAM).install_readwrite_handler(start, end, rhandler, whandler, mask);
			break;
		case 64:
			m_maincpu->space(AS_PROGRAM).install_readwrite_handler(start, end, rhandler, whandler, ((UINT64)mask<<32)|mask);
			break;
		default:
			fatalerror("NUBUS: Bus width %d not supported\n", buswidth);
	}
}

void nubus_device::install_device(offs_t start, offs_t end, read16_delegate rhandler, write16_delegate whandler, UINT32 mask)
{
	m_maincpu = machine().device<cpu_device>(m_cputag);
	int buswidth = m_maincpu->space_config(AS_PROGRAM)->m_databus_width;
	switch(buswidth)
	{
		case 32:
			m_maincpu->space(AS_PROGRAM).install_readwrite_handler(start, end, rhandler, whandler, mask);
			break;
		case 64:
			m_maincpu->space(AS_PROGRAM).install_readwrite_handler(start, end, rhandler, whandler, ((UINT64)mask<<32)|mask);
			break;
		default:
			fatalerror("NUBUS: Bus width %d not supported\n", buswidth);
	}
}

void nubus_device::install_device(offs_t start, offs_t end, read32_delegate rhandler, write32_delegate whandler, UINT32 mask)
{
	m_maincpu = machine().device<cpu_device>(m_cputag);
	int buswidth = m_maincpu->space_config(AS_PROGRAM)->m_databus_width;
	switch(buswidth)
	{
		case 32:
			m_maincpu->space(AS_PROGRAM).install_readwrite_handler(start, end, rhandler, whandler, mask);
			break;
		case 64:
			m_maincpu->space(AS_PROGRAM).install_readwrite_handler(start, end, rhandler, whandler, ((UINT64)mask<<32)|mask);
			break;
		default:
			fatalerror("NUBUS: Bus width %d not supported\n", buswidth);
	}
}

void nubus_device::install_readonly_device(offs_t start, offs_t end, read32_delegate rhandler, UINT32 mask)
{
	m_maincpu = machine().device<cpu_device>(m_cputag);
	int buswidth = m_maincpu->space_config(AS_PROGRAM)->m_databus_width;
	switch(buswidth)
	{
		case 32:
			m_maincpu->space(AS_PROGRAM).install_read_handler(start, end, rhandler, mask);
			break;
		case 64:
			m_maincpu->space(AS_PROGRAM).install_read_handler(start, end, rhandler, ((UINT64)mask<<32)|mask);
			break;
		default:
			fatalerror("NUBUS: Bus width %d not supported\n", buswidth);
	}
}

void nubus_device::install_writeonly_device(offs_t start, offs_t end, write32_delegate whandler, UINT32 mask)
{
	m_maincpu = machine().device<cpu_device>(m_cputag);
	int buswidth = m_maincpu->space_config(AS_PROGRAM)->m_databus_width;
	switch(buswidth)
	{
		case 32:
			m_maincpu->space(AS_PROGRAM).install_write_handler(start, end, whandler, mask);
			break;
		case 64:
			m_maincpu->space(AS_PROGRAM).install_write_handler(start, end, whandler, ((UINT64)mask<<32)|mask);
			break;
		default:
			fatalerror("NUBUS: Bus width %d not supported\n", buswidth);
	}
}

void nubus_device::install_bank(offs_t start, offs_t end, offs_t mask, offs_t mirror, const char *tag, UINT8 *data)
{
//  printf("install_bank: %s @ %x->%x mask %x mirror %x\n", tag, start, end, mask, mirror);
	m_maincpu = machine().device<cpu_device>(m_cputag);
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_readwrite_bank(start, end, mask, mirror, tag );
	machine().root_device().membank(tag)->set_base(data);
}

void nubus_device::set_irq_line(int slot, int state)
{
	switch (slot)
	{
		case 0x9:   irq9_w(state);  break;
		case 0xa:   irqa_w(state);  break;
		case 0xb:   irqb_w(state);  break;
		case 0xc:   irqc_w(state);  break;
		case 0xd:   irqd_w(state);  break;
		case 0xe:   irqe_w(state);  break;
	}
}

// interrupt request from nubus card
WRITE_LINE_MEMBER( nubus_device::irq9_w ) { m_out_irq9_cb(state); }
WRITE_LINE_MEMBER( nubus_device::irqa_w ) { m_out_irqa_cb(state); }
WRITE_LINE_MEMBER( nubus_device::irqb_w ) { m_out_irqb_cb(state); }
WRITE_LINE_MEMBER( nubus_device::irqc_w ) { m_out_irqc_cb(state); }
WRITE_LINE_MEMBER( nubus_device::irqd_w ) { m_out_irqd_cb(state); }
WRITE_LINE_MEMBER( nubus_device::irqe_w ) { m_out_irqe_cb(state); }

//**************************************************************************
//  DEVICE CONFIG NUBUS CARD INTERFACE
//**************************************************************************


//**************************************************************************
//  DEVICE NUBUS CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_nubus_card_interface - constructor
//-------------------------------------------------

device_nubus_card_interface::device_nubus_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_nubus(nullptr),
		m_nubus_tag(nullptr), m_nubus_slottag(nullptr), m_slot(0), m_next(nullptr)
{
}


//-------------------------------------------------
//  ~device_nubus_card_interface - destructor
//-------------------------------------------------

device_nubus_card_interface::~device_nubus_card_interface()
{
}

void device_nubus_card_interface::static_set_nubus_tag(device_t &device, const char *tag, const char *slottag)
{
	device_nubus_card_interface &nubus_card = dynamic_cast<device_nubus_card_interface &>(device);
	nubus_card.m_nubus_tag = tag;
	nubus_card.m_nubus_slottag = slottag;
}

void device_nubus_card_interface::set_nubus_device()
{
	if (!strncmp(m_nubus_slottag, "pds030", 6))
	{
		m_slot = 0x9;   // '030 PDS slots phantom slot as NuBus slots $9, $A, and $B
	}
	else if (!strncmp(m_nubus_slottag, "lcpds", 6))
	{
		m_slot = 0xe;   // LC PDS slots phantom slot as NuBus slot $E
	}
	else
	{
		// extract the slot number from the last digit of the slot tag
		int tlen = strlen(m_nubus_slottag);

		if (m_nubus_slottag[tlen-1] == '9')
		{
			m_slot = (m_nubus_slottag[tlen-1] - '9') + 9;
		}
		else
		{
			m_slot = (m_nubus_slottag[tlen-1] - 'a') + 0xa;
		}
	}

	if (m_slot < 9 || m_slot > 0xe)
	{
		fatalerror("Slot %x out of range for Apple NuBus\n", m_slot);
	}

	m_nubus = dynamic_cast<nubus_device *>(device().machine().device(m_nubus_tag));
	m_nubus->add_nubus_card(this);
}

void device_nubus_card_interface::install_bank(offs_t start, offs_t end, offs_t mask, offs_t mirror, const char *tag, UINT8 *data)
{
	char bank[256];

	// append an underscore and the slot name to the bank so it's guaranteed unique
	strcpy(bank, tag);
	strcat(bank, "_");
	strcat(bank, m_nubus_slottag);

	m_nubus->install_bank(start, end, mask, mirror, bank, data);
}

void device_nubus_card_interface::install_declaration_rom(device_t *dev, const char *romregion, bool mirror_all_mb, bool reverse_rom)
{
	bool inverted = false;

	UINT8 *rom = device().machine().root_device().memregion(dev->subtag(romregion).c_str())->base();
	UINT32 romlen = device().machine().root_device().memregion(dev->subtag(romregion).c_str())->bytes();

//  printf("ROM length is %x, last bytes are %02x %02x\n", romlen, rom[romlen-2], rom[romlen-1]);

	if (reverse_rom)
	{
		UINT8 temp;
		UINT32 endptr = romlen-1;

		for (UINT32 idx = 0; idx < romlen / 2; idx++)
		{
			temp = rom[idx];
			rom[idx] = rom[endptr];
			rom[endptr] = temp;
			endptr--;
		}
	}

	UINT8 byteLanes = rom[romlen-1];
	// check if all bits are inverted
	if (rom[romlen-2] == 0xff)
	{
		byteLanes ^= 0xff;
		inverted = true;
	}

	#if 0
	FILE *f;
	f = fopen("romout.bin", "wb");
	fwrite(rom, romlen, 1, f);
	fclose(f);
	#endif

	switch (byteLanes)
	{
		case 0x0f:  // easy case: all 4 lanes (still must scramble for 32-bit BE bus though)
			m_declaration_rom.resize(romlen);
			for (int i = 0; i < romlen; i++)
			{
				m_declaration_rom[BYTE4_XOR_BE(i)] = rom[i];
			}
			break;

		case 0xe1:  // lane 0 only
			m_declaration_rom.resize(romlen*4);
			memset(&m_declaration_rom[0], 0, romlen*4);
			for (int i = 0; i < romlen; i++)
			{
				m_declaration_rom[BYTE4_XOR_BE(i*4)] = rom[i];
			}
			romlen *= 4;
			break;

		case 0xd2:  // lane 1 only
			m_declaration_rom.resize(romlen*4);
			memset(&m_declaration_rom[0], 0, romlen*4);
			for (int i = 0; i < romlen; i++)
			{
				m_declaration_rom[BYTE4_XOR_BE((i*4)+1)] = rom[i];
			}
			romlen *= 4;
			break;

		case 0xb4:  // lane 2 only
			m_declaration_rom.resize(romlen*4);
			memset(&m_declaration_rom[0], 0, romlen*4);
			for (int i = 0; i < romlen; i++)
			{
				m_declaration_rom[BYTE4_XOR_BE((i*4)+2)] = rom[i];
			}
			romlen *= 4;
			break;

		case 0x78:  // lane 3 only
			m_declaration_rom.resize(romlen*4);
			memset(&m_declaration_rom[0], 0, romlen*4);
			for (int i = 0; i < romlen; i++)
			{
				m_declaration_rom[BYTE4_XOR_BE((i*4)+3)] = rom[i];
			}
			romlen *= 4;
			break;

		case 0xc3:  // lanes 0, 1
			m_declaration_rom.resize(romlen*2);
			memset(&m_declaration_rom[0], 0, romlen*4);
			for (int i = 0; i < romlen/2; i++)
			{
				m_declaration_rom[BYTE4_XOR_BE((i*4)+0)] = rom[(i*2)];
				m_declaration_rom[BYTE4_XOR_BE((i*4)+1)] = rom[(i*2)+1];
			}
			romlen *= 2;
			break;

		case 0xa5:  // lanes 0, 2
			m_declaration_rom.resize(romlen*2);
			memset(&m_declaration_rom[0], 0, romlen*4);
			for (int i = 0; i < romlen/2; i++)
			{
				m_declaration_rom[BYTE4_XOR_BE((i*4)+0)] = rom[(i*2)];
				m_declaration_rom[BYTE4_XOR_BE((i*4)+2)] = rom[(i*2)+1];
			}
			romlen *= 2;
			break;

		case 0x3c:  // lanes 2,3
			m_declaration_rom.resize(romlen*2);
			memset(&m_declaration_rom[0], 0, romlen*4);
			for (int i = 0; i < romlen/2; i++)
			{
				m_declaration_rom[BYTE4_XOR_BE((i*4)+2)] = rom[(i*2)];
				m_declaration_rom[BYTE4_XOR_BE((i*4)+3)] = rom[(i*2)+1];
			}
			romlen *= 2;
			break;

		default:
			fatalerror("NuBus: unhandled byteLanes value %02x\n", byteLanes);
	}

	// the slot manager can supposedly handle inverted ROMs by itself, but let's do it for it anyway
	if (inverted)
	{
		for (int i = 0; i < romlen; i++)
		{
			m_declaration_rom[i] ^= 0xff;
		}
	}

	// now install the ROM
	UINT32 addr = get_slotspace() + 0x01000000;
	char bankname[128];
	strcpy(bankname, "rom_");
	strcat(bankname, m_nubus_slottag);
	addr -= romlen;
//  printf("Installing ROM at %x, length %x\n", addr, romlen);
	if (mirror_all_mb)  // mirror the declaration ROM across all 16 megs of the slot space
	{
		m_nubus->install_bank(addr, addr+romlen-1, 0, 0x00f00000, bankname, &m_declaration_rom[0]);
	}
	else
	{
		m_nubus->install_bank(addr, addr+romlen-1, 0, 0, bankname, &m_declaration_rom[0]);
	}
}
