// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, R. Belmont
/***************************************************************************

        HP DIO and DIO-II bus devices

***************************************************************************/

#include "emu.h"
#include "hp_dio.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DIO16_SLOT, dio16_slot_device, "dio16_slot", "16-bit DIO slot")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dio16_slot_device - constructor
//-------------------------------------------------
dio16_slot_device::dio16_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio16_slot_device(mconfig, DIO16_SLOT, tag, owner, clock)
{
}

dio16_slot_device::dio16_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_owner(nullptr), m_dio_tag(nullptr)
{
}

void dio16_slot_device::static_set_dio16_slot(device_t &device, device_t *owner, const char *dio_tag)
{
	dio16_slot_device &dio_card = dynamic_cast<dio16_slot_device &>(device);
	dio_card.m_owner = owner;
	dio_card.m_dio_tag = dio_tag;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dio16_slot_device::device_start()
{
	device_dio16_card_interface *dev = dynamic_cast<device_dio16_card_interface *>(get_card_device());
	const device_dio32_card_interface *intf;
	if (get_card_device() && get_card_device()->interface(intf))
		fatalerror("DIO32 device in DIO16 slot\n");

	if (dev) device_dio16_card_interface::static_set_diobus(*dev,m_owner->subdevice(m_dio_tag));
}



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DIO32_SLOT, dio32_slot_device, "dio32_slot", "32-bit DIO-II slot")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dio32_slot_device - constructor
//-------------------------------------------------
dio32_slot_device::dio32_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio16_slot_device(mconfig, DIO32_SLOT, tag, owner, clock)
{
}

void dio32_slot_device::static_set_dio32_slot(device_t &device, device_t *owner, const char *dio_tag)
{
	dio32_slot_device &dio_card = dynamic_cast<dio32_slot_device &>(device);
	dio_card.m_owner = owner;
	dio_card.m_dio_tag = dio_tag;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dio32_slot_device::device_start()
{
	device_dio16_card_interface *dev = dynamic_cast<device_dio16_card_interface *>(get_card_device());
	if (dev) device_dio16_card_interface::static_set_diobus(*dev,m_owner->subdevice(m_dio_tag));
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DIO16, dio16_device, "dio16", "16-bit DIO bus")

void dio16_device::static_set_cputag(device_t &device, const char *tag)
{
	dio16_device &dio = downcast<dio16_device &>(device);
	dio.m_cputag = tag;
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dio16_device - constructor
//-------------------------------------------------

dio16_device::dio16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio16_device(mconfig, DIO16, tag, owner, clock)
{
}

dio16_device::dio16_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_program_config("DIO 16-bit program", ENDIANNESS_LITTLE, 8, 24, 0, nullptr),
	m_program32_config("DIO-II 32-bit program", ENDIANNESS_LITTLE, 16, 32, 0, nullptr),
	m_maincpu(nullptr),
	m_prgspace(nullptr),
	m_out_irq3_cb(*this),
	m_out_irq4_cb(*this),
	m_out_irq5_cb(*this),
	m_out_irq6_cb(*this),
	m_cputag(nullptr)
{
	m_prgwidth = 0;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dio16_device::device_start()
{
	m_out_irq3_cb.resolve_safe();
	m_out_irq4_cb.resolve_safe();
	m_out_irq5_cb.resolve_safe();
	m_out_irq6_cb.resolve_safe();

	m_maincpu = subdevice<cpu_device>(m_cputag);
	m_prgspace = &m_maincpu->space(AS_PROGRAM);
	m_prgwidth = m_maincpu->space_config(AS_PROGRAM)->m_databus_width;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dio16_device::device_reset()
{
}


void dio16_device::install_space(address_spacenum spacenum, offs_t start, offs_t end, read8_delegate rhandler, write8_delegate whandler)
{
	int buswidth;
	address_space *space;

	if (spacenum == AS_PROGRAM)
	{
		space = m_prgspace;
		buswidth = m_prgwidth;
	}
	else
	{
		fatalerror("Unknown space passed to dio16_device::install_space!\n");
	}

	switch(buswidth)
	{
		case 16:
			space->install_readwrite_handler(start, end, rhandler, whandler, 0xffff);
			break;
		case 32:
			if ((start % 4) == 0) {
				if ((end-start)==1) {
					space->install_readwrite_handler(start, end+2, rhandler, whandler, 0x0000ffff);
				} else {
					space->install_readwrite_handler(start, end,   rhandler, whandler, 0xffffffff);
				}
			} else {
				// we handle just misaligned by 2
				space->install_readwrite_handler(start-2, end, rhandler, whandler, 0xffff0000);
			}
			break;
		default:
			fatalerror("DIO16: Bus width %d not supported\n", buswidth);
	}
}


void dio16_device::install_memory(offs_t start, offs_t end, read8_delegate rhandler, write8_delegate whandler)
{
	install_space(AS_PROGRAM, start, end, rhandler, whandler);
}

void dio16_device::install_device(offs_t start, offs_t end, read8_delegate rhandler, write8_delegate whandler)
{
	install_space(AS_IO, start, end, rhandler, whandler);
}


void dio16_device::install_bank(offs_t start, offs_t end, const char *tag, uint8_t *data)
{
	m_prgspace->install_readwrite_bank(start, end, 0, tag );
	machine().root_device().membank(m_prgspace->device().siblingtag(tag).c_str())->set_base(data);
}

void dio16_device::unmap_bank(offs_t start, offs_t end)
{
	m_prgspace->unmap_readwrite(start, end);
}

void dio16_device::install_rom(device_t *dev, offs_t start, offs_t end, const char *tag, const char *region)
{
	if (machine().root_device().memregion("dio")) {
		uint8_t *src = dev->memregion(region)->base();
		uint8_t *dest = machine().root_device().memregion("dio")->base() + start - 0xc0000;
		memcpy(dest,src, end - start + 1);
	} else {
		m_prgspace->install_read_bank(start, end, 0, tag);
		m_prgspace->unmap_write(start, end);
		machine().root_device().membank(m_prgspace->device().siblingtag(tag).c_str())->set_base(machine().root_device().memregion(dev->subtag(region).c_str())->base());
	}
}

void dio16_device::unmap_rom(offs_t start, offs_t end)
{
	m_prgspace->unmap_read(start, end);
}

// interrupt request from dio card
WRITE_LINE_MEMBER( dio16_device::irq3_w ) { m_out_irq3_cb(state); }
WRITE_LINE_MEMBER( dio16_device::irq4_w ) { m_out_irq4_cb(state); }
WRITE_LINE_MEMBER( dio16_device::irq5_w ) { m_out_irq5_cb(state); }
WRITE_LINE_MEMBER( dio16_device::irq6_w ) { m_out_irq6_cb(state); }

//**************************************************************************
//  DEVICE CONFIG DIO16 CARD INTERFACE
//**************************************************************************


//**************************************************************************
//  DEVICE DIO16 CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_dio16_card_interface - constructor
//-------------------------------------------------

device_dio16_card_interface::device_dio16_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_dio(nullptr), m_dio_dev(nullptr), m_next(nullptr)
{
}


//-------------------------------------------------
//  ~device_dio16_card_interface - destructor
//-------------------------------------------------

device_dio16_card_interface::~device_dio16_card_interface()
{
}

void device_dio16_card_interface::static_set_diobus(device_t &device, device_t *dio_device)
{
	device_dio16_card_interface &dio_card = dynamic_cast<device_dio16_card_interface &>(device);
	dio_card.m_dio_dev = dio_device;
}

void device_dio16_card_interface::set_dio_device()
{
	m_dio = dynamic_cast<dio16_device *>(m_dio_dev);
}


DEFINE_DEVICE_TYPE(DIO32, dio32_device, "dio32", "32-bit DIO-II bus")

//-------------------------------------------------
//  dio32_device - constructor
//-------------------------------------------------

dio32_device::dio32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio16_device(mconfig, DIO32, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dio32_device::device_start()
{
	dio16_device::device_start();
}

//-------------------------------------------------
//  device_dio32_card_interface - constructor
//-------------------------------------------------

device_dio32_card_interface::device_dio32_card_interface(const machine_config &mconfig, device_t &device)
	: device_dio16_card_interface(mconfig,device), m_dio(nullptr)
{
}


//-------------------------------------------------
//  ~device_dio32_card_interface - destructor
//-------------------------------------------------

device_dio32_card_interface::~device_dio32_card_interface()
{
}

void device_dio32_card_interface::set_dio_device()
{
	m_dio = dynamic_cast<dio32_device *>(m_dio_dev);
}

