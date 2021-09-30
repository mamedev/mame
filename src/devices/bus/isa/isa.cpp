// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        ISA bus device

***************************************************************************/

#include "emu.h"
#include "isa.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_SLOT, isa8_slot_device, "isa8_slot", "8-bit ISA slot")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_slot_device - constructor
//-------------------------------------------------
isa8_slot_device::isa8_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_slot_device(mconfig, ISA8_SLOT, tag, owner, clock)
{
}

isa8_slot_device::isa8_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_isa_bus(*this, finder_base::DUMMY_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_slot_device::device_start()
{
	device_isa8_card_interface *const dev = dynamic_cast<device_isa8_card_interface *>(get_card_device());
	const device_isa16_card_interface *intf;
	if (get_card_device() && get_card_device()->interface(intf))
		fatalerror("ISA16 device in ISA8 slot\n");

	if (dev) dev->set_isabus(m_isa_bus);

	// tell isa bus that there is one slot with the specified tag
	downcast<isa8_device &>(*m_isa_bus).add_slot(tag());
}



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA16_SLOT, isa16_slot_device, "isa16_slot", "16-bit ISA slot")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa16_slot_device - constructor
//-------------------------------------------------
isa16_slot_device::isa16_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_slot_device(mconfig, ISA16_SLOT, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa16_slot_device::device_start()
{
	device_isa8_card_interface *const dev = dynamic_cast<device_isa8_card_interface *>(get_card_device());
	if (dev) dev->set_isabus(m_isa_bus);
	// tell isa bus that there is one slot with the specified tag
	dynamic_cast<isa8_device &>(*m_isa_bus).add_slot(tag());
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8, isa8_device, "isa8", "8-bit ISA bus")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_device - constructor
//-------------------------------------------------

isa8_device::isa8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_device(mconfig, ISA8, tag, owner, clock)
{
}

isa8_device::isa8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_mem_config("mem8", ENDIANNESS_LITTLE, 8, 24, 0, address_map_constructor()),
	m_io_config("io8", ENDIANNESS_LITTLE, 8, 16, 0, address_map_constructor()),
	m_mem16_config("mem16", ENDIANNESS_LITTLE, 16, 24, 0, address_map_constructor()),
	m_io16_config("io16", ENDIANNESS_LITTLE, 16, 16, 0, address_map_constructor()),
	m_memspace(*this, finder_base::DUMMY_TAG, -1),
	m_iospace(*this, finder_base::DUMMY_TAG, -1),
	m_memwidth(0),
	m_iowidth(0),
	m_allocspaces(false),
	m_out_irq2_cb(*this),
	m_out_irq3_cb(*this),
	m_out_irq4_cb(*this),
	m_out_irq5_cb(*this),
	m_out_irq6_cb(*this),
	m_out_irq7_cb(*this),
	m_out_drq1_cb(*this),
	m_out_drq2_cb(*this),
	m_out_drq3_cb(*this),
	m_write_iochrdy(*this),
	m_write_iochck(*this)
{
	std::fill(std::begin(m_dma_device), std::end(m_dma_device), nullptr);
	std::fill(std::begin(m_dma_eop), std::end(m_dma_eop), false);
}

device_memory_interface::space_config_vector isa8_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_ISA_MEM,    &m_mem_config),
		std::make_pair(AS_ISA_IO,     &m_io_config),
		std::make_pair(AS_ISA_MEMALT, &m_mem16_config),
		std::make_pair(AS_ISA_IOALT,  &m_io16_config)
	};
}

device_memory_interface::space_config_vector isa16_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_ISA_MEM,    &m_mem16_config),
		std::make_pair(AS_ISA_IO,     &m_io16_config),
		std::make_pair(AS_ISA_MEMALT, &m_mem_config),
		std::make_pair(AS_ISA_IOALT,  &m_io_config)
	};
}

uint8_t isa8_device::mem_r(offs_t offset)
{
	return m_memspace->read_byte(offset);
}

void isa8_device::mem_w(offs_t offset, uint8_t data)
{
	m_memspace->write_byte(offset, data);
}

uint8_t isa8_device::io_r(offs_t offset)
{
	return m_iospace->read_byte(offset);
}

void isa8_device::io_w(offs_t offset, uint8_t data)
{
	m_iospace->write_byte(offset, data);
}

void isa8_device::set_dma_channel(uint8_t channel, device_isa8_card_interface *dev, bool do_eop)
{
	m_dma_device[channel] = dev;
	m_dma_eop[channel] = do_eop;
}

void isa8_device::add_slot(const char *tag)
{
	device_t *dev = subdevice(tag);
	//printf(tag);
	add_slot(dynamic_cast<device_slot_interface *>(dev));
}

void isa8_device::add_slot(device_slot_interface *slot)
{
	m_slot_list.push_front(slot);
}

void isa8_device::remap(int space_id, offs_t start, offs_t end)
{
	for (device_slot_interface *sl : m_slot_list)
	{
		device_t *dev = sl->get_card_device();
		device_isa8_card_interface *isadev = dynamic_cast<device_isa8_card_interface *>(dev);
		isadev->remap(space_id, start, end);
	}
}

//-------------------------------------------------
//  device_config_complete - - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void isa8_device::device_config_complete()
{
	if (m_allocspaces)
	{
		m_memspace.set_tag(*this, DEVICE_SELF, AS_ISA_MEM);
		m_iospace.set_tag(*this, DEVICE_SELF, AS_ISA_IO);
	}
}

//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void isa8_device::device_resolve_objects()
{
	// resolve callbacks
	m_write_iochrdy.resolve_safe();
	m_write_iochck.resolve_safe();

	m_out_irq2_cb.resolve_safe();
	m_out_irq3_cb.resolve_safe();
	m_out_irq4_cb.resolve_safe();
	m_out_irq5_cb.resolve_safe();
	m_out_irq6_cb.resolve_safe();
	m_out_irq7_cb.resolve_safe();
	m_out_drq1_cb.resolve_safe();
	m_out_drq2_cb.resolve_safe();
	m_out_drq3_cb.resolve_safe();

	m_iowidth = m_iospace->data_width();
	m_memwidth = m_memspace->data_width();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_device::device_reset()
{
}


template <typename R, typename W> void isa8_device::install_space(int spacenum, offs_t start, offs_t end, R rhandler, W whandler)
{
	int buswidth;
	address_space *space;

	if (spacenum == AS_ISA_IO)
	{
		space = m_iospace.target();
		buswidth = m_iowidth;
	}
	else if (spacenum == AS_ISA_MEM)
	{
		space = m_memspace.target();
		buswidth = m_memwidth;
	}
	else
	{
		fatalerror("Unknown space passed to isa8_device::install_space!\n");
	}

	switch (buswidth)
	{
		case 8:
			space->install_read_handler(start, end, rhandler, 0);
			space->install_write_handler(start, end, whandler, 0);
			break;
		case 16:
			space->install_read_handler(start, end, rhandler, 0xffff);
			space->install_write_handler(start, end, whandler, 0xffff);
			break;
		case 32:
			if ((start % 4) == 0)
			{
				if ((end - start) == 1)
				{
					space->install_read_handler(start, end + 2, rhandler, 0x0000ffff);
					space->install_write_handler(start, end + 2, whandler, 0x0000ffff);
				}
				else
				{
					space->install_read_handler(start, end, rhandler, 0xffffffff);
					space->install_write_handler(start, end, whandler, 0xffffffff);
				}
			}
			else
			{
				// we handle just misaligned by 2
				space->install_read_handler(start - 2, end, rhandler, 0xffff0000);
				space->install_write_handler(start - 2, end, whandler, 0xffff0000);
			}
			break;
		default:
			fatalerror("ISA8: Bus width %d not supported\n", buswidth);
	}
}

template void isa8_device::install_space<read8_delegate,    write8_delegate   >(int spacenum, offs_t start, offs_t end, read8_delegate rhandler,    write8_delegate whandler);
template void isa8_device::install_space<read8_delegate,    write8m_delegate  >(int spacenum, offs_t start, offs_t end, read8_delegate rhandler,    write8m_delegate whandler);
template void isa8_device::install_space<read8_delegate,    write8s_delegate  >(int spacenum, offs_t start, offs_t end, read8_delegate rhandler,    write8s_delegate whandler);
template void isa8_device::install_space<read8_delegate,    write8sm_delegate >(int spacenum, offs_t start, offs_t end, read8_delegate rhandler,    write8sm_delegate whandler);
template void isa8_device::install_space<read8_delegate,    write8mo_delegate >(int spacenum, offs_t start, offs_t end, read8_delegate rhandler,    write8mo_delegate whandler);
template void isa8_device::install_space<read8_delegate,    write8smo_delegate>(int spacenum, offs_t start, offs_t end, read8_delegate rhandler,    write8smo_delegate whandler);

template void isa8_device::install_space<read8m_delegate,   write8_delegate   >(int spacenum, offs_t start, offs_t end, read8m_delegate rhandler,   write8_delegate whandler);
template void isa8_device::install_space<read8m_delegate,   write8m_delegate  >(int spacenum, offs_t start, offs_t end, read8m_delegate rhandler,   write8m_delegate whandler);
template void isa8_device::install_space<read8m_delegate,   write8s_delegate  >(int spacenum, offs_t start, offs_t end, read8m_delegate rhandler,   write8s_delegate whandler);
template void isa8_device::install_space<read8m_delegate,   write8sm_delegate >(int spacenum, offs_t start, offs_t end, read8m_delegate rhandler,   write8sm_delegate whandler);
template void isa8_device::install_space<read8m_delegate,   write8mo_delegate >(int spacenum, offs_t start, offs_t end, read8m_delegate rhandler,   write8mo_delegate whandler);
template void isa8_device::install_space<read8m_delegate,   write8smo_delegate>(int spacenum, offs_t start, offs_t end, read8m_delegate rhandler,   write8smo_delegate whandler);

template void isa8_device::install_space<read8s_delegate,   write8_delegate   >(int spacenum, offs_t start, offs_t end, read8s_delegate rhandler,   write8_delegate whandler);
template void isa8_device::install_space<read8s_delegate,   write8m_delegate  >(int spacenum, offs_t start, offs_t end, read8s_delegate rhandler,   write8m_delegate whandler);
template void isa8_device::install_space<read8s_delegate,   write8s_delegate  >(int spacenum, offs_t start, offs_t end, read8s_delegate rhandler,   write8s_delegate whandler);
template void isa8_device::install_space<read8s_delegate,   write8sm_delegate >(int spacenum, offs_t start, offs_t end, read8s_delegate rhandler,   write8sm_delegate whandler);
template void isa8_device::install_space<read8s_delegate,   write8mo_delegate >(int spacenum, offs_t start, offs_t end, read8s_delegate rhandler,   write8mo_delegate whandler);
template void isa8_device::install_space<read8s_delegate,   write8smo_delegate>(int spacenum, offs_t start, offs_t end, read8s_delegate rhandler,   write8smo_delegate whandler);

template void isa8_device::install_space<read8sm_delegate,  write8_delegate   >(int spacenum, offs_t start, offs_t end, read8sm_delegate rhandler,  write8_delegate whandler);
template void isa8_device::install_space<read8sm_delegate,  write8m_delegate  >(int spacenum, offs_t start, offs_t end, read8sm_delegate rhandler,  write8m_delegate whandler);
template void isa8_device::install_space<read8sm_delegate,  write8s_delegate  >(int spacenum, offs_t start, offs_t end, read8sm_delegate rhandler,  write8s_delegate whandler);
template void isa8_device::install_space<read8sm_delegate,  write8sm_delegate >(int spacenum, offs_t start, offs_t end, read8sm_delegate rhandler,  write8sm_delegate whandler);
template void isa8_device::install_space<read8sm_delegate,  write8mo_delegate >(int spacenum, offs_t start, offs_t end, read8sm_delegate rhandler,  write8mo_delegate whandler);
template void isa8_device::install_space<read8sm_delegate,  write8smo_delegate>(int spacenum, offs_t start, offs_t end, read8sm_delegate rhandler,  write8smo_delegate whandler);

template void isa8_device::install_space<read8mo_delegate,  write8_delegate   >(int spacenum, offs_t start, offs_t end, read8mo_delegate rhandler,  write8_delegate whandler);
template void isa8_device::install_space<read8mo_delegate,  write8m_delegate  >(int spacenum, offs_t start, offs_t end, read8mo_delegate rhandler,  write8m_delegate whandler);
template void isa8_device::install_space<read8mo_delegate,  write8s_delegate  >(int spacenum, offs_t start, offs_t end, read8mo_delegate rhandler,  write8s_delegate whandler);
template void isa8_device::install_space<read8mo_delegate,  write8sm_delegate >(int spacenum, offs_t start, offs_t end, read8mo_delegate rhandler,  write8sm_delegate whandler);
template void isa8_device::install_space<read8mo_delegate,  write8mo_delegate >(int spacenum, offs_t start, offs_t end, read8mo_delegate rhandler,  write8mo_delegate whandler);
template void isa8_device::install_space<read8mo_delegate,  write8smo_delegate>(int spacenum, offs_t start, offs_t end, read8mo_delegate rhandler,  write8smo_delegate whandler);

template void isa8_device::install_space<read8smo_delegate, write8_delegate   >(int spacenum, offs_t start, offs_t end, read8smo_delegate rhandler, write8_delegate whandler);
template void isa8_device::install_space<read8smo_delegate, write8m_delegate  >(int spacenum, offs_t start, offs_t end, read8smo_delegate rhandler, write8m_delegate whandler);
template void isa8_device::install_space<read8smo_delegate, write8s_delegate  >(int spacenum, offs_t start, offs_t end, read8smo_delegate rhandler, write8s_delegate whandler);
template void isa8_device::install_space<read8smo_delegate, write8sm_delegate >(int spacenum, offs_t start, offs_t end, read8smo_delegate rhandler, write8sm_delegate whandler);
template void isa8_device::install_space<read8smo_delegate, write8mo_delegate >(int spacenum, offs_t start, offs_t end, read8smo_delegate rhandler, write8mo_delegate whandler);
template void isa8_device::install_space<read8smo_delegate, write8smo_delegate>(int spacenum, offs_t start, offs_t end, read8smo_delegate rhandler, write8smo_delegate whandler);

void isa8_device::install_bank(offs_t start, offs_t end, uint8_t *data)
{
	m_memspace->install_ram(start, end, data);
}

void isa8_device::unmap_bank(offs_t start, offs_t end)
{
	m_memspace->unmap_readwrite(start, end);
}

void isa8_device::install_rom(device_t *dev, offs_t start, offs_t end, const char *region)
{
	if (machine().root_device().memregion("isa")) {
		uint8_t *src = dev->memregion(region)->base();
		uint8_t *dest = machine().root_device().memregion("isa")->base() + start - 0xc0000;
		memcpy(dest,src, end - start + 1);
	} else {
		m_memspace->install_rom(start, end, machine().root_device().memregion(dev->subtag(region).c_str())->base());
		m_memspace->unmap_write(start, end);
	}
}

void isa8_device::unmap_rom(offs_t start, offs_t end)
{
	m_memspace->unmap_read(start, end);
}

bool isa8_device::is_option_rom_space_available(offs_t start, int size)
{
	for(int i = 0; i < size; i += 4096) // 4KB granularity should be enough
		if(m_memspace->get_read_ptr(start + i)) return false;
	return true;
}

void isa8_device::unmap_readwrite(offs_t start, offs_t end)
{
	m_memspace->unmap_readwrite(start, end);
}

// interrupt request from isa card
WRITE_LINE_MEMBER( isa8_device::irq2_w ) { m_out_irq2_cb(state); }
WRITE_LINE_MEMBER( isa8_device::irq3_w ) { m_out_irq3_cb(state); }
WRITE_LINE_MEMBER( isa8_device::irq4_w ) { m_out_irq4_cb(state); }
WRITE_LINE_MEMBER( isa8_device::irq5_w ) { m_out_irq5_cb(state); }
WRITE_LINE_MEMBER( isa8_device::irq6_w ) { m_out_irq6_cb(state); }
WRITE_LINE_MEMBER( isa8_device::irq7_w ) { m_out_irq7_cb(state); }

// dma request from isa card
WRITE_LINE_MEMBER( isa8_device::drq1_w ) { m_out_drq1_cb(state); }
WRITE_LINE_MEMBER( isa8_device::drq2_w ) { m_out_drq2_cb(state); }
WRITE_LINE_MEMBER( isa8_device::drq3_w ) { m_out_drq3_cb(state); }

uint8_t isa8_device::dack_r(int line)
{
	if (m_dma_device[line])
		return m_dma_device[line]->dack_r(line);
	return 0xff;
}

void isa8_device::dack_w(int line, uint8_t data)
{
	if (m_dma_device[line])
		return m_dma_device[line]->dack_w(line,data);
}

void isa8_device::dack_line_w(int line, int state)
{
	if (m_dma_device[line])
		m_dma_device[line]->dack_line_w(line, state);
}

void isa8_device::eop_w(int channel, int state)
{
	if (m_dma_eop[channel] && m_dma_device[channel])
		m_dma_device[channel]->eop_w(state);
}

void isa8_device::set_ready(int state)
{
	m_write_iochrdy(state);
}

void isa8_device::nmi()
{
	// active low pulse
	m_write_iochck(0);
	m_write_iochck(1);
}

//**************************************************************************
//  DEVICE CONFIG ISA8 CARD INTERFACE
//**************************************************************************


//**************************************************************************
//  DEVICE ISA8 CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_isa8_card_interface - constructor
//-------------------------------------------------

device_isa8_card_interface::device_isa8_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "isa"),
		m_isa(nullptr), m_isa_dev(nullptr), m_next(nullptr)
{
}


//-------------------------------------------------
//  ~device_isa8_card_interface - destructor
//-------------------------------------------------

device_isa8_card_interface::~device_isa8_card_interface()
{
}

uint8_t device_isa8_card_interface::dack_r(int line)
{
	return 0;
}

void device_isa8_card_interface::dack_w(int line, uint8_t data)
{
}

void device_isa8_card_interface::dack_line_w(int line, int state)
{
}

void device_isa8_card_interface::eop_w(int state)
{
}

void device_isa8_card_interface::set_isa_device()
{
	m_isa = dynamic_cast<isa8_device *>(m_isa_dev);
}


DEFINE_DEVICE_TYPE(ISA16, isa16_device, "isa16", "16-bit ISA bus")

//-------------------------------------------------
//  isa16_device - constructor
//-------------------------------------------------

isa16_device::isa16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_device(mconfig, ISA16, tag, owner, clock),
	m_out_irq10_cb(*this),
	m_out_irq11_cb(*this),
	m_out_irq12_cb(*this),
	m_out_irq14_cb(*this),
	m_out_irq15_cb(*this),
	m_out_drq0_cb(*this),
	m_out_drq5_cb(*this),
	m_out_drq6_cb(*this),
	m_out_drq7_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa16_device::device_start()
{
	isa8_device::device_start();

	// resolve callbacks
	m_out_irq10_cb.resolve_safe();
	m_out_irq11_cb.resolve_safe();
	m_out_irq12_cb.resolve_safe();
	m_out_irq14_cb.resolve_safe();
	m_out_irq15_cb.resolve_safe();

	m_out_drq0_cb.resolve_safe();
	m_out_drq5_cb.resolve_safe();
	m_out_drq6_cb.resolve_safe();
	m_out_drq7_cb.resolve_safe();
}

template<typename R, typename W> void isa16_device::install16_device(offs_t start, offs_t end, R rhandler, W whandler)
{
	int buswidth = m_iowidth;
	switch(buswidth)
	{
		case 16:
			m_iospace->install_readwrite_handler(start, end, rhandler, whandler, 0);
			break;
		case 32:
			m_iospace->install_readwrite_handler(start, end, rhandler, whandler, 0xffffffff);
			if ((start % 4) == 0) {
				if ((end-start)==1) {
					m_iospace->install_readwrite_handler(start, end+2, rhandler, whandler, 0x0000ffff);
				} else {
					m_iospace->install_readwrite_handler(start, end,   rhandler, whandler, 0xffffffff);
				}
			} else {
				// we handle just misaligned by 2
				m_iospace->install_readwrite_handler(start-2, end, rhandler, whandler, 0xffff0000);
			}

			break;
		default:
			fatalerror("ISA16: Bus width %d not supported\n", buswidth);
	}
}

template void isa16_device::install16_device<read16_delegate,    write16_delegate   >(offs_t start, offs_t end, read16_delegate rhandler,    write16_delegate whandler);
template void isa16_device::install16_device<read16s_delegate,   write16s_delegate  >(offs_t start, offs_t end, read16s_delegate rhandler,   write16s_delegate whandler);
template void isa16_device::install16_device<read16sm_delegate,  write16sm_delegate >(offs_t start, offs_t end, read16sm_delegate rhandler,  write16sm_delegate whandler);
template void isa16_device::install16_device<read16smo_delegate, write16smo_delegate>(offs_t start, offs_t end, read16smo_delegate rhandler, write16smo_delegate whandler);

uint16_t isa16_device::mem16_r(offs_t offset, uint16_t mem_mask)
{
	return m_memspace->read_word(offset<<1, mem_mask);
}

void isa16_device::mem16_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_memspace->write_word(offset<<1, data, mem_mask);
}

uint16_t isa16_device::io16_r(offs_t offset, uint16_t mem_mask)
{
	return m_iospace->read_word(offset<<1, mem_mask);
}

void isa16_device::io16_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_iospace->write_word(offset<<1, data, mem_mask);
}

uint16_t isa16_device::mem16_swap_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t rv;
	mem_mask = (mem_mask<<8) | (mem_mask>>8);

	rv = m_memspace->read_word(offset<<1, mem_mask);

	return (rv<<8) | (rv>>8);
}

void isa16_device::mem16_swap_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	mem_mask = (mem_mask<<8) | (mem_mask>>8);
	data = (data<<8) | (data>>8);
	m_memspace->write_word(offset<<1, data, mem_mask);
}

uint16_t isa16_device::io16_swap_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t rv;
	mem_mask = (mem_mask<<8) | (mem_mask>>8);

	rv = m_iospace->read_word(offset<<1, mem_mask);

	return (rv<<8) | (rv>>8);
}

void isa16_device::io16_swap_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	mem_mask = (mem_mask<<8) | (mem_mask>>8);
	data = (data<<8) | (data>>8);
	m_iospace->write_word(offset<<1, data, mem_mask);
}

// interrupt request from isa card
WRITE_LINE_MEMBER( isa16_device::irq10_w ) { m_out_irq10_cb(state); }
WRITE_LINE_MEMBER( isa16_device::irq11_w ) { m_out_irq11_cb(state); }
WRITE_LINE_MEMBER( isa16_device::irq12_w ) { m_out_irq12_cb(state); }
WRITE_LINE_MEMBER( isa16_device::irq14_w ) { m_out_irq14_cb(state); }
WRITE_LINE_MEMBER( isa16_device::irq15_w ) { m_out_irq15_cb(state); }

// dma request from isa card
WRITE_LINE_MEMBER( isa16_device::drq0_w ) { m_out_drq0_cb(state); }
WRITE_LINE_MEMBER( isa16_device::drq5_w ) { m_out_drq5_cb(state); }
WRITE_LINE_MEMBER( isa16_device::drq6_w ) { m_out_drq6_cb(state); }
WRITE_LINE_MEMBER( isa16_device::drq7_w ) { m_out_drq7_cb(state); }

uint16_t isa16_device::dack16_r(int line)
{
	if (m_dma_device[line])
		return dynamic_cast<device_isa16_card_interface *>(m_dma_device[line])->dack16_r(line);
	return 0xffff;
}

void isa16_device::dack16_w(int line, uint16_t data)
{
	if (m_dma_device[line])
		return dynamic_cast<device_isa16_card_interface *>(m_dma_device[line])->dack16_w(line,data);
}

void isa16_device::remap(int space_id, offs_t start, offs_t end)
{
	for (device_slot_interface *sl : m_slot_list)
	{
		device_t *dev = sl->get_card_device();

		if (dev)
		{
			device_isa8_card_interface *isadev8 = dynamic_cast<device_isa8_card_interface *>(dev);
			device_isa16_card_interface *isadev16 = dynamic_cast<device_isa16_card_interface *>(dev);

			if (isadev16)
				isadev16->remap(space_id, start, end);
			else
				if (isadev8)
					isadev8->remap(space_id, start, end);
		}
	}
}

//-------------------------------------------------
//  device_isa16_card_interface - constructor
//-------------------------------------------------

device_isa16_card_interface::device_isa16_card_interface(const machine_config &mconfig, device_t &device)
	: device_isa8_card_interface(mconfig,device), m_isa(nullptr)
{
}


//-------------------------------------------------
//  ~device_isa16_card_interface - destructor
//-------------------------------------------------

device_isa16_card_interface::~device_isa16_card_interface()
{
}

void device_isa16_card_interface::set_isa_device()
{
	m_isa = dynamic_cast<isa16_device *>(m_isa_dev);
}

uint16_t device_isa16_card_interface::dack16_r(int line)
{
	return 0;
}

void device_isa16_card_interface::dack16_w(int line, uint16_t data)
{
}
