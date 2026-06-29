// license:BSD-3-Clause
// copyright-holders:A. Lenard
/***************************************************************************

    ZBI bus & slot device

TODO:
 - non-masked interrupt
 - non-vectored interrupt
 - multimicro request
 - CPU request

***************************************************************************/

#include "emu.h"
#include "zbi.h"

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  zbi_bus_device - constructor
//-------------------------------------------------
zbi_bus_device::zbi_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ZBI_BUS, tag, owner, clock)
	, z80_daisy_chain_interface(mconfig, *this)
	, m_iospace_installer(nullptr)
	, m_daisy_chain(nullptr)
{
}

//-------------------------------------------------
//  ~zbi_bus_device - destructor
//-------------------------------------------------

zbi_bus_device::~zbi_bus_device()
{
	if (m_daisy_chain)
		delete [] m_daisy_chain;
}

void zbi_bus_device::device_start()
{
}

void zbi_bus_device::device_reset()
{
	if (m_iospace_installer)
		iospace()->unmap_readwrite(0, (1 << iospace()->space_config().addr_width()) - 1);
}

void zbi_bus_device::interface_post_start()
{
	osd_printf_verbose("ZBI_BUS: Starting interface, total devices: %d, in Z80 daisy chain: %d\n", m_device_list.size(), m_daisy.size());

	m_daisy_chain = new z80_daisy_config[m_daisy.size() + 1];
	for (size_t i = 0; i < m_daisy.size(); i++)
	{
		m_daisy_chain[i].devname = m_daisy[i].c_str();
	}
	m_daisy_chain[m_daisy.size()].devname = nullptr;

	set_daisy_config(m_daisy_chain);

	z80_daisy_chain_interface::interface_post_start();
}

void zbi_bus_device::set_bus_clock(u32 clock)
{
	set_clock(clock);
	notify_clock_changed();
}

void zbi_bus_device::add_card(device_zbi_card_interface &card)
{
	card.m_bus = this;
	m_device_list.emplace_back(card);
}

void zbi_bus_device::assign_iospace_installer(address_space_installer *installer)
{
	if (m_iospace_installer != nullptr )
		throw emu_fatalerror("IO address installer already set on ZBI bus !!!");
	m_iospace_installer = installer;
}

address_space_installer *zbi_bus_device::iospace() const
{
	if (m_iospace_installer == nullptr )
		throw emu_fatalerror("IO address installer not set on ZBI bus !!! Add CPU board.");
	return m_iospace_installer;
}

void zbi_bus_device::memerr_w(int state)
{
	for (device_zbi_card_interface &entry : m_device_list)
		entry.card_memerr_w(state);
}

void zbi_bus_device::busreq_w(int state)
{
	for (device_zbi_card_interface &entry : m_device_list)
		entry.card_busreq_w(state);
}

void zbi_bus_device::busack_w(int state)
{
	for (device_zbi_card_interface &entry : m_device_list)
	{
		// Did this device make the request?
		if (entry.busdaisy_req_state() == ASSERT_LINE)
		{
			if (state)
				entry.busdaisy_req_ack();	// Acknowledge request
			else
				busreq_w(ASSERT_LINE);		// Trigger bus request on behalf of waiting device

			break;
		}
	}
}

void zbi_bus_device::cpureq_w(int state)
{
	for (device_zbi_card_interface &entry : m_device_list)
		entry.card_cpureq_w(state);
}

void zbi_bus_device::cpuavail_w(int state)
{
	for (device_zbi_card_interface &entry : m_device_list)
		entry.card_cpuavail_w(state);
}

void zbi_bus_device::mmreq_w(int state)
{
	for (device_zbi_card_interface &entry : m_device_list)
		entry.card_mmreq_w(state);
}

void zbi_bus_device::vi_w(int state)
{
	// Ignore the sent device state and forward the daisy chain state instead
	int vi_daisy_state = daisy_update_irq_state();

	for (device_zbi_card_interface &entry : m_device_list)
		entry.card_vi_w(vi_daisy_state);
}

void zbi_bus_device::nvi_w(int state)
{
	for (device_zbi_card_interface &entry : m_device_list)
		entry.card_nvi_w(state);
}

void zbi_bus_device::nmi_w(int state)
{
	for (device_zbi_card_interface &entry : m_device_list)
		entry.card_nmi_w(state);
}

void zbi_bus_device::ns_w(int state)
{
	for (device_zbi_card_interface &entry : m_device_list)
		entry.card_ns_w(state);
}

uint8_t zbi_bus_device::ram8_r(offs_t offset)
{
	uint8_t result = 0;
	for (device_zbi_card_interface &entry : m_device_list)
		result |= entry.card_ram8_r(offset);
	return result;
}

void zbi_bus_device::ram8_w(offs_t offset, uint8_t data)
{
	for (device_zbi_card_interface &entry : m_device_list)
		entry.card_ram8_w(offset, data);
}

uint16_t zbi_bus_device::ram16_r(offs_t offset, uint16_t mask)
{
	uint16_t result = 0;
	for (device_zbi_card_interface &entry : m_device_list)
		result |= entry.card_ram16_r(offset, mask);
	return result;
}

void zbi_bus_device::ram16_w(offs_t offset, uint16_t data, uint16_t mask)
{
	for (device_zbi_card_interface &entry : m_device_list)
		entry.card_ram16_w(offset, data, mask);
}

uint32_t zbi_bus_device::ram32_r(offs_t offset, uint32_t mask)
{
	uint32_t result = 0;
	for (device_zbi_card_interface &entry : m_device_list)
		result |= entry.card_ram32_r(offset, mask);
	return result;
}

void zbi_bus_device::ram32_w(offs_t offset, uint32_t data, uint32_t mask)
{
	for (device_zbi_card_interface &entry : m_device_list)
		entry.card_ram32_w(offset, data, mask);
}

uint16_t zbi_bus_device::viack_r()
{
	device_z80daisy_interface *intf = daisy_get_irq_device();
	return intf ? intf->z80daisy_irq_ack() : 0;
}

uint16_t zbi_bus_device::nviack_r()
{
	// TODO
	return 0;
}

uint16_t zbi_bus_device::nmiack_r()
{
	// TODO

	return 0;
}

//-------------------------------------------------
//  device_zbi_card_interface
//-------------------------------------------------

device_zbi_card_interface::device_zbi_card_interface(const machine_config &mconfig, device_t &device)
	: device_z80daisy_interface(mconfig, device)
	, m_bus(nullptr)
{
}

//-------------------------------------------------
//  z80daisy_irq_state - return the overall IRQ
//  state for this device
//-------------------------------------------------

int device_zbi_card_interface::z80daisy_irq_state()
{
	return 0;
}

//-------------------------------------------------
//  z80daisy_irq_ack - acknowledge an IRQ and
//  return the appropriate vector
//-------------------------------------------------

int device_zbi_card_interface::z80daisy_irq_ack()
{
	return 0xff;
}

//-------------------------------------------------
//  z80daisy_irq_reti - clear the interrupt
//  pending state to allow other interrupts through
//-------------------------------------------------

void device_zbi_card_interface::z80daisy_irq_reti()
{
}

int device_zbi_card_interface::busdaisy_req_state()
{
	return 0;
}

void device_zbi_card_interface::busdaisy_req_ack()
{
}

//-------------------------------------------------
//  zbi_slot_device - constructor
//-------------------------------------------------
zbi_slot_device::zbi_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ZBI_SLOT, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
	, m_bus(*this, finder_base::DUMMY_TAG)
{
}

//-------------------------------------------------
//  zbi_slot_device - device-specific startup
//-------------------------------------------------
void zbi_slot_device::device_start()
{
}

void zbi_slot_device::device_resolve_objects()
{
	device_zbi_card_interface *const card(dynamic_cast<device_zbi_card_interface *>(get_card_device()));

	if (card)
	{
		m_bus->add_card(*card);
	}
}

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************
DEFINE_DEVICE_TYPE(ZBI_BUS, zbi_bus_device, "zbi_bus", "ZBI bus")
DEFINE_DEVICE_TYPE(ZBI_SLOT, zbi_slot_device, "zbi_slot", "ZBI bus slot")
