// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        ISA bus device

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "isa.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA8_SLOT = &device_creator<isa8_slot_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_slot_device - constructor
//-------------------------------------------------
isa8_slot_device::isa8_slot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, ISA8_SLOT, "ISA8_SLOT", tag, owner, clock, "isa8_slot", __FILE__),
		device_slot_interface(mconfig, *this),
	m_owner(nullptr)
{
}

isa8_slot_device::isa8_slot_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_slot_interface(mconfig, *this), m_owner(nullptr)
{
}

void isa8_slot_device::static_set_isa8_slot(device_t &device, device_t *owner, const char *isa_tag)
{
	isa8_slot_device &isa_card = dynamic_cast<isa8_slot_device &>(device);
	isa_card.m_owner = owner;
	isa_card.m_isa_tag = isa_tag;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_slot_device::device_start()
{
	device_isa8_card_interface *dev = dynamic_cast<device_isa8_card_interface *>(get_card_device());
	const device_isa16_card_interface *intf;
	if (get_card_device()->interface(intf))
		fatalerror("Error ISA16 device in ISA8 slot\n");

	if (dev) device_isa8_card_interface::static_set_isabus(*dev,m_owner->subdevice(m_isa_tag));
}



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA16_SLOT = &device_creator<isa16_slot_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa16_slot_device - constructor
//-------------------------------------------------
isa16_slot_device::isa16_slot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
		isa8_slot_device(mconfig, ISA16_SLOT, "ISA16_SLOT", tag, owner, clock, "isa16_slot", __FILE__)
{
}

void isa16_slot_device::static_set_isa16_slot(device_t &device, device_t *owner, const char *isa_tag)
{
	isa16_slot_device &isa_card = dynamic_cast<isa16_slot_device &>(device);
	isa_card.m_owner = owner;
	isa_card.m_isa_tag = isa_tag;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa16_slot_device::device_start()
{
	device_isa8_card_interface *dev = dynamic_cast<device_isa8_card_interface *>(get_card_device());
	if (dev) device_isa8_card_interface::static_set_isabus(*dev,m_owner->subdevice(m_isa_tag));
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA8 = &device_creator<isa8_device>;

void isa8_device::static_set_cputag(device_t &device, std::string tag)
{
	isa8_device &isa = downcast<isa8_device &>(device);
	isa.m_cputag = tag;
}

void isa8_device::static_set_custom_spaces(device_t &device)
{
	isa8_device &isa = downcast<isa8_device &>(device);

	isa.m_allocspaces = true;
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void isa8_device::device_config_complete()
{
	m_maincpu = subdevice<cpu_device>(m_cputag);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_device - constructor
//-------------------------------------------------

isa8_device::isa8_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, ISA8, "ISA8", tag, owner, clock, "isa8", __FILE__),
		device_memory_interface(mconfig, *this),
		m_program_config("ISA 8-bit program", ENDIANNESS_LITTLE, 8, 24, 0, nullptr),
		m_io_config("ISA 8-bit I/O", ENDIANNESS_LITTLE, 8, 16, 0, nullptr),
		m_program16_config("ISA 16-bit program", ENDIANNESS_LITTLE, 16, 24, 0, nullptr),
		m_io16_config("ISA 16-bit I/O", ENDIANNESS_LITTLE, 16, 16, 0, nullptr), m_maincpu(nullptr), m_iospace(nullptr), m_prgspace(nullptr),
		m_out_irq2_cb(*this),
		m_out_irq3_cb(*this),
		m_out_irq4_cb(*this),
		m_out_irq5_cb(*this),
		m_out_irq6_cb(*this),
		m_out_irq7_cb(*this),
		m_out_drq1_cb(*this),
		m_out_drq2_cb(*this),
		m_out_drq3_cb(*this),
		m_write_iochck(*this)
{
	for(int i=0;i<8;i++)
	{
		m_dma_device[i] = nullptr;
		m_dma_eop[i] = false;
	}
	m_nmi_enabled = false;
	m_iowidth = m_prgwidth = 0;
	m_allocspaces = false;
}

isa8_device::isa8_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_memory_interface(mconfig, *this),
		m_program_config("ISA 8-bit program", ENDIANNESS_LITTLE, 8, 24, 0, nullptr),
		m_io_config("ISA 8-bit I/O", ENDIANNESS_LITTLE, 8, 16, 0, nullptr),
		m_program16_config("ISA 16-bit program", ENDIANNESS_LITTLE, 16, 24, 0, nullptr),
		m_io16_config("ISA 16-bit I/O", ENDIANNESS_LITTLE, 16, 16, 0, nullptr), m_maincpu(nullptr), m_iospace(nullptr), m_prgspace(nullptr),
		m_out_irq2_cb(*this),
		m_out_irq3_cb(*this),
		m_out_irq4_cb(*this),
		m_out_irq5_cb(*this),
		m_out_irq6_cb(*this),
		m_out_irq7_cb(*this),
		m_out_drq1_cb(*this),
		m_out_drq2_cb(*this),
		m_out_drq3_cb(*this),
		m_write_iochck(*this)
{
	for(int i=0;i<8;i++)
	{
		m_dma_device[i] = nullptr;
		m_dma_eop[i] = false;
	}
	m_nmi_enabled = false;
	m_iowidth = m_prgwidth = 0;
	m_allocspaces = false;
}

READ8_MEMBER(isa8_device::prog_r)
{
	return m_prgspace->read_byte(offset);
}

WRITE8_MEMBER(isa8_device::prog_w)
{
	m_prgspace->write_byte(offset, data);
}

READ8_MEMBER(isa8_device::io_r)
{
	return m_iospace->read_byte(offset);
}

WRITE8_MEMBER(isa8_device::io_w)
{
	m_iospace->write_byte(offset, data);
}

void isa8_device::set_dma_channel(UINT8 channel, device_isa8_card_interface *dev, bool do_eop)
{
	m_dma_device[channel] = dev;
	m_dma_eop[channel] = do_eop;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_device::device_start()
{
	// resolve callbacks
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

	m_maincpu = subdevice<cpu_device>(m_cputag);

	if (m_allocspaces)
	{
		m_iospace = &space(AS_IO);
		m_prgspace = &space(AS_PROGRAM);
		m_iowidth = m_iospace->data_width();
		m_prgwidth = m_prgspace->data_width();
	}
	else    // use host CPU's program and I/O spaces directly
	{
		m_iospace = &m_maincpu->space(AS_IO);
		m_iowidth = m_maincpu->space_config(AS_IO)->m_databus_width;
		m_prgspace = &m_maincpu->space(AS_PROGRAM);
		m_prgwidth = m_maincpu->space_config(AS_PROGRAM)->m_databus_width;
	}
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_device::device_reset()
{
}


void isa8_device::install_space(address_spacenum spacenum, offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_delegate rhandler, write8_delegate whandler)
{
	int buswidth;
	address_space *space;

	if (spacenum == AS_IO)
	{
		space = m_iospace;
		buswidth = m_iowidth;
	}
	else if (spacenum == AS_PROGRAM)
	{
		space = m_prgspace;
		buswidth = m_prgwidth;
	}
	else
	{
		fatalerror("Unknown space passed to isa8_device::install_space!\n");
	}

	switch(buswidth)
	{
		case 8:
			space->install_readwrite_handler(start, end, mask, mirror, rhandler, whandler, 0);
			break;
		case 16:
			space->install_readwrite_handler(start, end, mask, mirror, rhandler, whandler, 0xffff);
			break;
		case 32:
			if ((start % 4) == 0) {
				if ((end-start)==1) {
					space->install_readwrite_handler(start, end+2, mask, mirror, rhandler, whandler, 0x0000ffff);
				} else {
					space->install_readwrite_handler(start, end,   mask, mirror, rhandler, whandler, 0xffffffff);
				}
			} else {
				// we handle just misalligned by 2
				space->install_readwrite_handler(start-2, end, mask, mirror, rhandler, whandler, 0xffff0000);
			}
			break;
		default:
			fatalerror("ISA8: Bus width %d not supported\n", buswidth);
	}
}


void isa8_device::install_memory(offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_delegate rhandler, write8_delegate whandler)
{
	install_space(AS_PROGRAM, start, end, mask, mirror, rhandler, whandler);
}

void isa8_device::install_device(offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_delegate rhandler, write8_delegate whandler)
{
	install_space(AS_IO, start, end, mask, mirror, rhandler, whandler);
}


void isa8_device::install_bank(offs_t start, offs_t end, offs_t mask, offs_t mirror, std::string tag, UINT8 *data)
{
	m_prgspace->install_readwrite_bank(start, end, mask, mirror, tag );
	machine().root_device().membank(tag)->set_base(data);
}

void isa8_device::unmap_bank(offs_t start, offs_t end, offs_t mask, offs_t mirror)
{
	m_prgspace->unmap_readwrite(start, end, mask, mirror);
}

void isa8_device::install_rom(device_t *dev, offs_t start, offs_t end, offs_t mask, offs_t mirror, std::string tag, const char *region)
{
	if (machine().root_device().memregion("isa")) {
		UINT8 *src = dev->memregion(region)->base();
		UINT8 *dest = machine().root_device().memregion("isa")->base() + start - 0xc0000;
		memcpy(dest,src, end - start + 1);
	} else {
		m_prgspace->install_read_bank(start, end, mask, mirror, tag);
		m_prgspace->unmap_write(start, end, mask, mirror);
		machine().root_device().membank(tag)->set_base(machine().root_device().memregion(dev->subtag(region).c_str())->base());
	}
}

void isa8_device::unmap_rom(offs_t start, offs_t end, offs_t mask, offs_t mirror)
{
	m_prgspace->unmap_read(start, end, mask, mirror);
}

bool isa8_device::is_option_rom_space_available(offs_t start, int size)
{
	m_maincpu = machine().device<cpu_device>(m_cputag);
	for(int i = 0; i < size; i += 4096) // 4KB granularity should be enough
		if(m_prgspace->get_read_ptr(start + i)) return false;
	return true;
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

UINT8 isa8_device::dack_r(int line)
{
	if (m_dma_device[line])
		return m_dma_device[line]->dack_r(line);
	return 0xff;
}

void isa8_device::dack_w(int line,UINT8 data)
{
	if (m_dma_device[line])
		return m_dma_device[line]->dack_w(line,data);
}

void isa8_device::eop_w(int channel, int state)
{
	if (m_dma_eop[channel] && m_dma_device[channel])
		m_dma_device[channel]->eop_w(state);
}

void isa8_device::nmi()
{
	if (m_write_iochck.isnull())
	{
		if (m_nmi_enabled)
		{
			m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE );
		}
	}
	else
	{
		m_write_iochck(0);
		m_write_iochck(1);
	}
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
	: device_slot_card_interface(mconfig, device),
		m_isa(nullptr), m_isa_dev(nullptr), m_next(nullptr)
{
}


//-------------------------------------------------
//  ~device_isa8_card_interface - destructor
//-------------------------------------------------

device_isa8_card_interface::~device_isa8_card_interface()
{
}

UINT8 device_isa8_card_interface::dack_r(int line)
{
	return 0;
}
void device_isa8_card_interface::dack_w(int line,UINT8 data)
{
}
void device_isa8_card_interface::eop_w(int state)
{
}

void device_isa8_card_interface::static_set_isabus(device_t &device, device_t *isa_device)
{
	device_isa8_card_interface &isa_card = dynamic_cast<device_isa8_card_interface &>(device);
	isa_card.m_isa_dev = isa_device;
}

void device_isa8_card_interface::set_isa_device()
{
	m_isa = dynamic_cast<isa8_device *>(m_isa_dev);
}


const device_type ISA16 = &device_creator<isa16_device>;

//-------------------------------------------------
//  isa16_device - constructor
//-------------------------------------------------

isa16_device::isa16_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
		isa8_device(mconfig, ISA16, "ISA16", tag, owner, clock, "isa16", __FILE__),
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
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void isa16_device::device_config_complete()
{
	m_maincpu = mconfig().device<cpu_device>(m_cputag);
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

void isa16_device::install16_device(offs_t start, offs_t end, offs_t mask, offs_t mirror, read16_delegate rhandler, write16_delegate whandler)
{
	int buswidth = m_prgwidth;
	switch(buswidth)
	{
		case 16:
			m_iospace->install_readwrite_handler(start, end, mask, mirror, rhandler, whandler, 0);
			break;
		case 32:
			m_iospace->install_readwrite_handler(start, end, mask, mirror, rhandler, whandler, 0xffffffff);
			if ((start % 4) == 0) {
				if ((end-start)==1) {
					m_iospace->install_readwrite_handler(start, end+2, mask, mirror, rhandler, whandler, 0x0000ffff);
				} else {
					m_iospace->install_readwrite_handler(start, end,   mask, mirror, rhandler, whandler, 0xffffffff);
				}
			} else {
				// we handle just misalligned by 2
				m_iospace->install_readwrite_handler(start-2, end, mask, mirror, rhandler, whandler, 0xffff0000);
			}

			break;
		default:
			fatalerror("ISA16: Bus width %d not supported\n", buswidth);
	}
}

READ16_MEMBER(isa16_device::prog16_r)
{
	return m_prgspace->read_word(offset<<1, mem_mask);
}

WRITE16_MEMBER(isa16_device::prog16_w)
{
	m_prgspace->write_word(offset<<1, data, mem_mask);
}

READ16_MEMBER(isa16_device::io16_r)
{
	return m_iospace->read_word(offset<<1, mem_mask);
}

WRITE16_MEMBER(isa16_device::io16_w)
{
	m_iospace->write_word(offset<<1, data, mem_mask);
}

READ16_MEMBER(isa16_device::prog16_swap_r)
{
	UINT16 rv;
	mem_mask = (mem_mask<<8) | (mem_mask>>8);

	rv = m_prgspace->read_word(offset<<1, mem_mask);

	return (rv<<8) | (rv>>8);
}

WRITE16_MEMBER(isa16_device::prog16_swap_w)
{
	mem_mask = (mem_mask<<8) | (mem_mask>>8);
	data = (data<<8) | (data>>8);
	m_prgspace->write_word(offset<<1, data, mem_mask);
}

READ16_MEMBER(isa16_device::io16_swap_r)
{
	UINT16 rv;
	mem_mask = (mem_mask<<8) | (mem_mask>>8);

	rv = m_iospace->read_word(offset<<1, mem_mask);

	return (rv<<8) | (rv>>8);
}

WRITE16_MEMBER(isa16_device::io16_swap_w)
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

UINT16 isa16_device::dack16_r(int line)
{
	if (m_dma_device[line])
		return dynamic_cast<device_isa16_card_interface *>(m_dma_device[line])->dack16_r(line);
	return 0xffff;
}

void isa16_device::dack16_w(int line,UINT16 data)
{
	if (m_dma_device[line])
		return dynamic_cast<device_isa16_card_interface *>(m_dma_device[line])->dack16_w(line,data);
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

UINT16 device_isa16_card_interface::dack16_r(int line)
{
	return 0;
}

void device_isa16_card_interface::dack16_w(int line,UINT16 data)
{
}
