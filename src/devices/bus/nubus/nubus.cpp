// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  nubus.cpp - NuBus bus and card emulation

  by R. Belmont, based heavily on Miodrag Milanovic's ISA8/16 implementation

***************************************************************************/

#include "emu.h"
#include "nubus.h"

#include <algorithm>


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(NUBUS_SLOT, nubus_slot_device, "nubus_slot", "NuBus slot")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_slot_device - constructor
//-------------------------------------------------
nubus_slot_device::nubus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	nubus_slot_device(mconfig, NUBUS_SLOT, tag, owner, clock)
{
}

nubus_slot_device::nubus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_single_card_slot_interface(mconfig, *this),
	m_nubus(*this, finder_base::DUMMY_TAG),
	m_nubus_slottag(nullptr)
{
}

//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void nubus_slot_device::device_resolve_objects()
{
	device_nubus_card_interface *dev = get_card_device();

	if (dev)
	{
		dev->set_nubus_tag(m_nubus.target(), m_nubus_slottag);
		m_nubus->add_nubus_card(*dev);
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_slot_device::device_start()
{
}

DEFINE_DEVICE_TYPE(NUBUS, nubus_device, "nubus", "NuBus")

nubus_device::nubus_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	nubus_device(mconfig, NUBUS, tag, owner, clock)
{
}

nubus_device::nubus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_space(*this, finder_base::DUMMY_TAG, -1),
	m_mem_config("nubus", ENDIANNESS_BIG, 32, 32),
	m_out_irq9_cb(*this),
	m_out_irqa_cb(*this),
	m_out_irqb_cb(*this),
	m_out_irqc_cb(*this),
	m_out_irqd_cb(*this),
	m_out_irqe_cb(*this),
	m_bus_mode(nubus_mode_t::NORMAL),
	m_addr_mask(0xffffffff)
{
}

nubus_device::~nubus_device()
{
}

device_memory_interface::space_config_vector nubus_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_DATA, &m_mem_config)};
}

void nubus_device::device_start()
{
	switch (m_bus_mode)
	{
		case nubus_mode_t::NORMAL:
			m_space->install_read_handler(0x90000000, 0xefffffff, emu::rw_delegate(*this, FUNC(nubus_device::bus_memory_r<0x90000000>)));
			m_space->install_write_handler(0x90000000, 0xefffffff, emu::rw_delegate(*this, FUNC(nubus_device::bus_memory_w<0x90000000>)));
			m_space->install_read_handler(0xf9000000, 0xfeffffff, emu::rw_delegate(*this, FUNC(nubus_device::bus_memory_r<0xf9000000>)));
			m_space->install_write_handler(0xf9000000, 0xfeffffff, emu::rw_delegate(*this, FUNC(nubus_device::bus_memory_w<0xf9000000>)));

			// NuBus mappings
			// RAM
			this->space(AS_DATA).install_read_handler(0x00000000, 0x3fffffff, emu::rw_delegate(*this, FUNC(nubus_device::host_memory_r<0x00000000>)));
			this->space(AS_DATA).install_write_handler(0x00000000, 0x3fffffff, emu::rw_delegate(*this, FUNC(nubus_device::host_memory_w<0x00000000>)));
			// I/O
			this->space(AS_DATA).install_read_handler(0xf0000000, 0xf07fffff, emu::rw_delegate(*this, FUNC(nubus_device::host_memory_r<0x50000000>)));
			this->space(AS_DATA).install_write_handler(0xf0000000, 0xf07fffff, emu::rw_delegate(*this, FUNC(nubus_device::host_memory_w<0x50000000>)));
			// ROM
			this->space(AS_DATA).install_read_handler(0xf0800000, 0xf0ffffff, emu::rw_delegate(*this, FUNC(nubus_device::host_memory_r<0x40000000>)));
			this->space(AS_DATA).install_write_handler(0xf0800000, 0xf0ffffff, emu::rw_delegate(*this, FUNC(nubus_device::host_memory_w<0x40000000>)));

			break;

		case nubus_mode_t::QUADRA_DAFB:
			m_space->install_read_handler(0xa0000000, 0xefffffff, emu::rw_delegate(*this, FUNC(nubus_device::bus_memory_r<0xa0000000>)));
			m_space->install_write_handler(0xa0000000, 0xefffffff, emu::rw_delegate(*this, FUNC(nubus_device::bus_memory_w<0xa0000000>)));
			m_space->install_read_handler(0xfa000000, 0xfeffffff, emu::rw_delegate(*this, FUNC(nubus_device::bus_memory_r<0xfa000000>)));
			m_space->install_write_handler(0xfa000000, 0xfeffffff, emu::rw_delegate(*this, FUNC(nubus_device::bus_memory_w<0xfa000000>)));

			this->space(AS_DATA).install_read_handler(0x00000000, 0x3fffffff, emu::rw_delegate(*this, FUNC(nubus_device::host_memory_r<0x00000000>)));
			this->space(AS_DATA).install_write_handler(0x00000000, 0x3fffffff, emu::rw_delegate(*this, FUNC(nubus_device::host_memory_w<0x00000000>)));
			break;

		// Original LC PDS omits A28, A29, and A30 so in 32-bit mode, 0xfxxxxxxx addresses appear here as 0x8xxxxxxx.
		// In 24-bit mode, we will see addresses of the form 0x00exxxxxx.
		case nubus_mode_t::LC_PDS:
			m_space->install_read_handler(0x80000000, 0x80ffffff, emu::rw_delegate(*this, FUNC(nubus_device::bus_memory_r<0x80000000>)));
			m_space->install_write_handler(0x80000000, 0x80ffffff, emu::rw_delegate(*this, FUNC(nubus_device::bus_memory_w<0x80000000>)));
			m_space->install_read_handler(0x00e00000, 0x00efffff, emu::rw_delegate(*this, FUNC(nubus_device::bus_memory_r<0x80e00000>)));
			m_space->install_write_handler(0x00e00000, 0x00efffff, emu::rw_delegate(*this, FUNC(nubus_device::bus_memory_w<0x80e00000>)));

			this->space(AS_DATA).install_read_handler(0x00000000, 0x009fffff, emu::rw_delegate(*this, FUNC(nubus_device::host_memory_r<0x00000000>)));
			this->space(AS_DATA).install_write_handler(0x00000000, 0x009fffff, emu::rw_delegate(*this, FUNC(nubus_device::host_memory_w<0x00000000>)));
			break;

		// LC III and later PDS is very similar to SE/30, and allows phantom slotting to $C, $D, and $E
		case nubus_mode_t::LC32_PDS:
			m_space->install_read_handler(0xc0000000, 0xefffffff, emu::rw_delegate(*this, FUNC(nubus_device::bus_memory_r<0xc0000000>)));
			m_space->install_write_handler(0xc0000000, 0xefffffff, emu::rw_delegate(*this, FUNC(nubus_device::bus_memory_w<0xc0000000>)));
			m_space->install_read_handler(0xfc000000, 0xfeffffff, emu::rw_delegate(*this, FUNC(nubus_device::bus_memory_r<0xfc000000>)));
			m_space->install_write_handler(0xfc000000, 0xfeffffff, emu::rw_delegate(*this, FUNC(nubus_device::bus_memory_w<0xfc000000>)));

			this->space(AS_DATA).install_read_handler(0x00000000, 0x3fffffff, emu::rw_delegate(*this, FUNC(nubus_device::host_memory_r<0x00000000>)));
			this->space(AS_DATA).install_write_handler(0x00000000, 0x3fffffff, emu::rw_delegate(*this, FUNC(nubus_device::host_memory_w<0x00000000>)));
			break;

		case nubus_mode_t::SE30:
			m_space->install_read_handler(0x90000000, 0xdfffffff, emu::rw_delegate(*this, FUNC(nubus_device::bus_memory_r<0x90000000>)));
			m_space->install_write_handler(0x90000000, 0xdfffffff, emu::rw_delegate(*this, FUNC(nubus_device::bus_memory_w<0x90000000>)));
			m_space->install_read_handler(0xf9000000, 0xfdffffff, emu::rw_delegate(*this, FUNC(nubus_device::bus_memory_r<0xf9000000>)));
			m_space->install_write_handler(0xf9000000, 0xfdffffff, emu::rw_delegate(*this, FUNC(nubus_device::bus_memory_w<0xf9000000>)));

			this->space(AS_DATA).install_read_handler(0x00000000, 0x3fffffff, emu::rw_delegate(*this, FUNC(nubus_device::host_memory_r<0x00000000>)));
			this->space(AS_DATA).install_write_handler(0x00000000, 0x3fffffff, emu::rw_delegate(*this, FUNC(nubus_device::host_memory_w<0x00000000>)));
			break;
	}
}

void nubus_device::add_nubus_card(device_nubus_card_interface &card)
{
	m_device_list.emplace_back(card);
}

template <u32 Base>
u32 nubus_device::host_memory_r(offs_t offset, u32 mem_mask)
{
	return m_space->read_dword(Base + (offset * 4), mem_mask);
}

template <u32 Base>
void nubus_device::host_memory_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_space->write_dword(Base + (offset * 4), data, mem_mask);
}

template u32    nubus_device::host_memory_r<0x00000000>(offs_t offset, u32 mem_mask);
template void   nubus_device::host_memory_w<0x00000000>(offs_t offset, u32 data, u32 mem_mask);

template <u32 Base>
u32 nubus_device::bus_memory_r(offs_t offset, u32 mem_mask)
{
	return this->space(AS_DATA).read_dword(Base + (offset * 4), mem_mask);
}

template <u32 Base>
void nubus_device::bus_memory_w(offs_t offset, u32 data, u32 mem_mask)
{
	this->space(AS_DATA).write_dword(Base + (offset * 4), data, mem_mask);
}

template u32    nubus_device::bus_memory_r<0x90000000>(offs_t offset, u32 mem_mask);
template void   nubus_device::bus_memory_w<0x90000000>(offs_t offset, u32 data, u32 mem_mask);
template u32    nubus_device::bus_memory_r<0xc0000000>(offs_t offset, u32 mem_mask);
template void   nubus_device::bus_memory_w<0xc0000000>(offs_t offset, u32 data, u32 mem_mask);
template u32    nubus_device::bus_memory_r<0xf9000000>(offs_t offset, u32 mem_mask);
template void   nubus_device::bus_memory_w<0xf9000000>(offs_t offset, u32 data, u32 mem_mask);

template u32    nubus_device::bus_memory_r<0xa0000000>(offs_t offset, u32 mem_mask);
template void   nubus_device::bus_memory_w<0xa0000000>(offs_t offset, u32 data, u32 mem_mask);
template u32    nubus_device::bus_memory_r<0xfa000000>(offs_t offset, u32 mem_mask);
template void   nubus_device::bus_memory_w<0xfa000000>(offs_t offset, u32 data, u32 mem_mask);

template u32    nubus_device::bus_memory_r<0xfc000000>(offs_t offset, u32 mem_mask);
template void   nubus_device::bus_memory_w<0xfc000000>(offs_t offset, u32 data, u32 mem_mask);

template u32    nubus_device::bus_memory_r<0x00e00000>(offs_t offset, u32 mem_mask);
template void   nubus_device::bus_memory_w<0x00e00000>(offs_t offset, u32 data, u32 mem_mask);
template u32    nubus_device::bus_memory_r<0x80000000>(offs_t offset, u32 mem_mask);
template void   nubus_device::bus_memory_w<0x80000000>(offs_t offset, u32 data, u32 mem_mask);
template u32    nubus_device::bus_memory_r<0x80e00000>(offs_t offset, u32 mem_mask);
template void   nubus_device::bus_memory_w<0x80e00000>(offs_t offset, u32 data, u32 mem_mask);

template <typename R, typename W>
void nubus_device::install_device(offs_t start, offs_t end, R rhandler, W whandler, u32 mask)
{
	start &= m_addr_mask;
	end &= m_addr_mask;
	space(AS_DATA).install_readwrite_handler(start, end, rhandler, whandler, mask);
}

template void nubus_device::install_device<read8_delegate,     write8_delegate    >(offs_t start, offs_t end, read8_delegate rhandler,     write8_delegate whandler, u32 mask);
template void nubus_device::install_device<read8s_delegate,    write8s_delegate   >(offs_t start, offs_t end, read8s_delegate rhandler,    write8s_delegate whandler, u32 mask);
template void nubus_device::install_device<read8sm_delegate,   write8sm_delegate  >(offs_t start, offs_t end, read8sm_delegate rhandler,   write8sm_delegate whandler, u32 mask);
template void nubus_device::install_device<read8smo_delegate,  write8smo_delegate >(offs_t start, offs_t end, read8smo_delegate rhandler,  write8smo_delegate whandler, u32 mask);
template void nubus_device::install_device<read16_delegate,    write16_delegate   >(offs_t start, offs_t end, read16_delegate rhandler,    write16_delegate whandler, u32 mask);
template void nubus_device::install_device<read16s_delegate,   write16s_delegate  >(offs_t start, offs_t end, read16s_delegate rhandler,   write16s_delegate whandler, u32 mask);
template void nubus_device::install_device<read16sm_delegate,  write16sm_delegate >(offs_t start, offs_t end, read16sm_delegate rhandler,  write16sm_delegate whandler, u32 mask);
template void nubus_device::install_device<read16smo_delegate, write16smo_delegate>(offs_t start, offs_t end, read16smo_delegate rhandler, write16smo_delegate whandler, u32 mask);
template void nubus_device::install_device<read32_delegate,    write32_delegate   >(offs_t start, offs_t end, read32_delegate rhandler,    write32_delegate whandler, u32 mask);
template void nubus_device::install_device<read32s_delegate,   write32s_delegate  >(offs_t start, offs_t end, read32s_delegate rhandler,   write32s_delegate whandler, u32 mask);
template void nubus_device::install_device<read32sm_delegate,  write32sm_delegate >(offs_t start, offs_t end, read32sm_delegate rhandler,  write32sm_delegate whandler, u32 mask);
template void nubus_device::install_device<read32smo_delegate, write32smo_delegate>(offs_t start, offs_t end, read32smo_delegate rhandler, write32smo_delegate whandler, u32 mask);

template <typename R>
void nubus_device::install_readonly_device(offs_t start, offs_t end, R rhandler, u32 mask)
{
	start &= m_addr_mask;
	end &= m_addr_mask;
	space(AS_DATA).install_read_handler(start, end, rhandler, mask);
}

template void nubus_device::install_readonly_device<read8_delegate    >(offs_t start, offs_t end, read8_delegate rhandler,     u32 mask);
template void nubus_device::install_readonly_device<read8s_delegate   >(offs_t start, offs_t end, read8s_delegate rhandler,    u32 mask);
template void nubus_device::install_readonly_device<read8sm_delegate  >(offs_t start, offs_t end, read8sm_delegate rhandler,   u32 mask);
template void nubus_device::install_readonly_device<read8smo_delegate >(offs_t start, offs_t end, read8smo_delegate rhandler,  u32 mask);
template void nubus_device::install_readonly_device<read16_delegate   >(offs_t start, offs_t end, read16_delegate rhandler,    u32 mask);
template void nubus_device::install_readonly_device<read16s_delegate  >(offs_t start, offs_t end, read16s_delegate rhandler,   u32 mask);
template void nubus_device::install_readonly_device<read16sm_delegate >(offs_t start, offs_t end, read16sm_delegate rhandler,  u32 mask);
template void nubus_device::install_readonly_device<read16smo_delegate>(offs_t start, offs_t end, read16smo_delegate rhandler, u32 mask);
template void nubus_device::install_readonly_device<read32_delegate   >(offs_t start, offs_t end, read32_delegate rhandler,    u32 mask);
template void nubus_device::install_readonly_device<read32s_delegate  >(offs_t start, offs_t end, read32s_delegate rhandler,   u32 mask);
template void nubus_device::install_readonly_device<read32sm_delegate >(offs_t start, offs_t end, read32sm_delegate rhandler,  u32 mask);
template void nubus_device::install_readonly_device<read32smo_delegate>(offs_t start, offs_t end, read32smo_delegate rhandler, u32 mask);

template <typename W>
void nubus_device::install_writeonly_device(offs_t start, offs_t end, W whandler, u32 mask)
{
	start &= m_addr_mask;
	end &= m_addr_mask;
	space(AS_DATA).install_write_handler(start, end, whandler, mask);
}

template void nubus_device::install_writeonly_device<write8_delegate    >(offs_t start, offs_t end, write8_delegate whandler,     u32 mask);
template void nubus_device::install_writeonly_device<write8s_delegate   >(offs_t start, offs_t end, write8s_delegate whandler,    u32 mask);
template void nubus_device::install_writeonly_device<write8sm_delegate  >(offs_t start, offs_t end, write8sm_delegate whandler,   u32 mask);
template void nubus_device::install_writeonly_device<write8smo_delegate >(offs_t start, offs_t end, write8smo_delegate whandler,  u32 mask);
template void nubus_device::install_writeonly_device<write16_delegate   >(offs_t start, offs_t end, write16_delegate whandler,    u32 mask);
template void nubus_device::install_writeonly_device<write16s_delegate  >(offs_t start, offs_t end, write16s_delegate whandler,   u32 mask);
template void nubus_device::install_writeonly_device<write16sm_delegate >(offs_t start, offs_t end, write16sm_delegate whandler,  u32 mask);
template void nubus_device::install_writeonly_device<write16smo_delegate>(offs_t start, offs_t end, write16smo_delegate whandler, u32 mask);
template void nubus_device::install_writeonly_device<write32_delegate   >(offs_t start, offs_t end, write32_delegate whandler,    u32 mask);
template void nubus_device::install_writeonly_device<write32s_delegate  >(offs_t start, offs_t end, write32s_delegate whandler,   u32 mask);
template void nubus_device::install_writeonly_device<write32sm_delegate >(offs_t start, offs_t end, write32sm_delegate whandler,  u32 mask);
template void nubus_device::install_writeonly_device<write32smo_delegate>(offs_t start, offs_t end, write32smo_delegate whandler, u32 mask);

void nubus_device::install_bank(offs_t start, offs_t end, void *data)
{
	start &= m_addr_mask;
	end &= m_addr_mask;
	space(AS_DATA).install_ram(start, end, data);
}

void nubus_device::install_view(offs_t start, offs_t end, memory_view &view)
{
	start &= m_addr_mask;
	end &= m_addr_mask;
	space(AS_DATA).install_view(start, end, view);
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
void nubus_device::irq9_w(int state) { m_out_irq9_cb(state); }
void nubus_device::irqa_w(int state) { m_out_irqa_cb(state); }
void nubus_device::irqb_w(int state) { m_out_irqb_cb(state); }
void nubus_device::irqc_w(int state) { m_out_irqc_cb(state); }
void nubus_device::irqd_w(int state) { m_out_irqd_cb(state); }
void nubus_device::irqe_w(int state) { m_out_irqe_cb(state); }

DEFINE_DEVICE_TYPE(MACSE30_PDS_BUS, se30_pds_bus_device, "se30bus", "Mac SE/30 expansion bus");

//**************************************************************************
//  DEVICE CONFIG NUBUS CARD INTERFACE
//**************************************************************************


//**************************************************************************
//  DEVICE NUBUS CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_nubus_card_interface - constructor
//-------------------------------------------------

device_nubus_card_interface::device_nubus_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "nubus"),
	m_nubus(nullptr),
	m_nubus_slottag(nullptr),
	m_slot(0)
{
}


//-------------------------------------------------
//  ~device_nubus_card_interface - destructor
//-------------------------------------------------

device_nubus_card_interface::~device_nubus_card_interface()
{
}

void device_nubus_card_interface::interface_pre_start()
{
	if ((!strncmp(m_nubus_slottag, "pds030", 6)) || (!strncmp(m_nubus_slottag, "siexp", 5)))
	{
		m_slot = 0x9;   // '030 PDS slots phantom slot as whatever they want but default to 9
	}
	else if (!strncmp(m_nubus_slottag, "lcpds", 6))
	{
		m_slot = 0xe; // LC PDS phantom slots as slot $E in most cases
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

	if (!m_nubus)
	{
		fatalerror("Can't find NuBus device\n");
	}
}

void device_nubus_card_interface::install_bank(offs_t start, offs_t end, void *data)
{
	nubus().install_bank(start, end, data);
}

void device_nubus_card_interface::install_view(offs_t start, offs_t end, memory_view &view)
{
	nubus().install_view(start, end, view);
}

void device_nubus_card_interface::install_declaration_rom(const char *romregion, bool mirror_all_mb, bool reverse_rom)
{
	bool inverted = false;

	u8 *rom = device().memregion(romregion)->base();
	u32 romlen = device().memregion(romregion)->bytes();

//  printf("ROM length is %x, last bytes are %02x %02x\n", romlen, rom[romlen-2], rom[romlen-1]);

	if (reverse_rom)
	{
		for (u32 idx = 0, endptr = romlen-1; idx < endptr; idx++, endptr--)
		{
			using std::swap;
			swap(rom[idx], rom[endptr]);
		}
	}

	u8 byteLanes = rom[romlen-1];
	// check if all bits are inverted
	if (rom[romlen-2] == 0xff)
	{
		byteLanes ^= 0xff;
		inverted = true;
	}

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
			m_declaration_rom.resize(romlen * 4);
			std::fill_n(&m_declaration_rom[0], romlen * 4, 0);
			for (int i = 0; i < romlen; i++)
			{
				m_declaration_rom[BYTE4_XOR_BE(i*4)] = rom[i];
			}
			romlen *= 4;
			break;

		case 0xd2:  // lane 1 only
			m_declaration_rom.resize(romlen * 4);
			std::fill_n(&m_declaration_rom[0], romlen * 4, 0);
			for (int i = 0; i < romlen; i++)
			{
				m_declaration_rom[BYTE4_XOR_BE((i*4)+1)] = rom[i];
			}
			romlen *= 4;
			break;

		case 0xb4:  // lane 2 only
			m_declaration_rom.resize(romlen * 4);
			std::fill_n(&m_declaration_rom[0], romlen * 4, 0);
			for (int i = 0; i < romlen; i++)
			{
				m_declaration_rom[BYTE4_XOR_BE((i*4)+2)] = rom[i];
			}
			romlen *= 4;
			break;

		case 0x78:  // lane 3 only
			m_declaration_rom.resize(romlen * 4);
			std::fill_n(&m_declaration_rom[0], romlen * 4, 0);
			for (int i = 0; i < romlen; i++)
			{
				m_declaration_rom[BYTE4_XOR_BE((i*4)+3)] = rom[i];
			}
			romlen *= 4;
			break;

		case 0xc3:  // lanes 0, 1
			m_declaration_rom.resize(romlen * 2);
			std::fill_n(&m_declaration_rom[0], romlen * 2, 0);
			for (int i = 0; i < romlen / 2; i++)
			{
				m_declaration_rom[BYTE4_XOR_BE((i*4)+0)] = rom[(i*2)];
				m_declaration_rom[BYTE4_XOR_BE((i*4)+1)] = rom[(i*2)+1];
			}
			romlen *= 2;
			break;

		case 0xa5:  // lanes 0, 2
			m_declaration_rom.resize(romlen * 2);
			std::fill_n(&m_declaration_rom[0], romlen * 2, 0);
			for (int i = 0; i < romlen/2; i++)
			{
				m_declaration_rom[BYTE4_XOR_BE((i*4)+0)] = rom[(i*2)];
				m_declaration_rom[BYTE4_XOR_BE((i*4)+2)] = rom[(i*2)+1];
			}
			romlen *= 2;
			break;

		case 0x3c:  // lanes 2,3
			m_declaration_rom.resize(romlen * 2);
			std::fill_n(&m_declaration_rom[0], romlen * 2, 0);
			for (int i = 0; i < romlen/2; i++)
			{
				m_declaration_rom[BYTE4_XOR_BE((i*4)+2)] = rom[(i*2)];
				m_declaration_rom[BYTE4_XOR_BE((i*4)+3)] = rom[(i*2)+1];
			}
			romlen *= 2;
			break;

		default:
			fatalerror("nubus: unhandled byteLanes value %02x\n", byteLanes);
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
	u32 addr = get_slotspace() + 0x01000000;
	addr -= romlen;
	if (mirror_all_mb)  // mirror the declaration ROM across all 16 megs of the slot space
	{
		u32 off = 0;
		while(off < 0x1000000) {
			nubus().install_bank(addr + off, addr+off+romlen-1, &m_declaration_rom[0]);
			off += romlen;
		}
	}
	else
	{
		nubus().install_bank(addr, addr+romlen-1, &m_declaration_rom[0]);
	}
}
