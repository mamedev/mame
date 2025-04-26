// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, R. Belmont
/***************************************************************************

        HP DIO and DIO-II bus devices

***************************************************************************/

#include "emu.h"
#include "hp_dio.h"
#include "hp98265a.h"
#include "hp98543.h"
#include "hp98544.h"
#include "hp98550.h"
#include "hp98603a.h"
#include "hp98603b.h"
#include "hp98620.h"
#include "hp98624.h"
#include "hp98628_9.h"
#include "hp98643.h"
#include "hp98644.h"
#include "human_interface.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DIO16_SLOT, bus::hp_dio::dio16_slot_device, "dio16_slot", "16-bit DIO slot")
DEFINE_DEVICE_TYPE(DIO32_SLOT, bus::hp_dio::dio32_slot_device, "dio32_slot", "32-bit DIO-II slot")
DEFINE_DEVICE_TYPE(DIO16,      bus::hp_dio::dio16_device,      "dio16",      "16-bit DIO bus")
DEFINE_DEVICE_TYPE(DIO32,      bus::hp_dio::dio32_device,      "dio32",      "32-bit DIO-II bus")

namespace bus::hp_dio {

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

dio16_slot_device::dio16_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio16_slot_device(mconfig, DIO16_SLOT, tag, owner, clock)
{
}

dio16_slot_device::dio16_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_dio(*this, finder_base::DUMMY_TAG)
{
}

void dio16_slot_device::device_resolve_objects()
{
	device_dio16_card_interface *const card(dynamic_cast<device_dio16_card_interface *>(get_card_device()));
	if (card && m_dio)
		card->set_diobus(*m_dio);
}

void dio16_slot_device::device_validity_check(validity_checker &valid) const
{
	device_t *const card(get_card_device());
	if (card && !dynamic_cast<device_dio16_card_interface *>(card))
		osd_printf_error("Card device %s (%s) does not implement device_dio16_card_interface\n", card->tag(), card->name());
}

void dio16_slot_device::device_start()
{
	device_t *const card(get_card_device());
	if (card && !dynamic_cast<device_dio16_card_interface *>(card))
		throw emu_fatalerror("dio16_slot_device: card device %s (%s) does not implement device_dio16_card_interface\n", card->tag(), card->name());
}

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

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dio32_slot_device::device_start()
{
	dio16_slot_device::device_start();
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
	m_prgspace(*this, finder_base::DUMMY_TAG, -1),
	m_bus_index{0},
	m_irq{0},
	m_dmar{0},
	m_irq1_out_cb(*this),
	m_irq2_out_cb(*this),
	m_irq3_out_cb(*this),
	m_irq4_out_cb(*this),
	m_irq5_out_cb(*this),
	m_irq6_out_cb(*this),
	m_irq7_out_cb(*this),
	m_dmar0_out_cb(*this),
	m_dmar1_out_cb(*this)
{
	std::fill(std::begin(m_cards), std::end(m_cards), nullptr);
	m_prgwidth = 0;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dio16_device::device_start()
{
	m_prgwidth = m_prgspace->data_width();

	save_item(NAME(m_irq));
	save_item(NAME(m_dmar));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dio16_device::device_reset()
{
}

unsigned dio16_device::add_card(device_dio16_card_interface &card)
{
	m_cards.push_back(&card);
	return m_bus_index++;
}

void dio16_device::set_irq(unsigned index, unsigned int level, int state)
{
	bool const changed(bool(state) != BIT(m_irq[level], index));
	if (!changed)
		return;

	if (state)
		m_irq[level] |= (1 << index);
	else
		m_irq[level] &= ~(1 << index);

	if (m_bus_index != index) {
		switch (level) {
		case 0:
			m_irq1_out_cb(state);
			break;
		case 1:
			m_irq2_out_cb(state);
			break;
		case 2:
			m_irq3_out_cb(state);
			break;
		case 3:
			m_irq4_out_cb(state);
			break;
		case 4:
			m_irq5_out_cb(state);
			break;
		case 5:
			m_irq6_out_cb(state);
			break;
		case 6:
			m_irq7_out_cb(state);
			break;
		}
	}
}

void dio16_device::set_dmar(unsigned int index, unsigned int num, int state)
{
	assert(num <= 1);

	bool const changed(bool(state) != BIT(m_dmar[num], index));
	if (!changed)
		return;
	if (state)
		m_dmar[num] |= (1 << index);
	else
		m_dmar[num] &= ~(1 << index);


	for (auto &card : m_cards) {

		if (card->get_index() == index)
			continue;

		switch (num) {
		case 0:
			card->dmar0_in(state);
			break;
		case 1:
			card->dmar1_in(state);
			break;
		}
	}
}

void dio16_device::dmack_w_out(int index, int channel, uint8_t val)
{
	for (auto &card : m_cards) {
		if (card->get_index() == index)
			continue;
		card->dmack_w_in(channel, val);
	}
}

uint8_t dio16_device::dmack_r_out(int index, int channel)
{
	uint8_t ret = 0xff;

	for (auto &card : m_cards) {
		if (card->get_index() == index)
			continue;
		ret &= card->dmack_r_in(channel);
	}
	return ret;
}

void dio16_device::reset_in(int state)
{
	for (auto &card : m_cards) {
		if (card->get_index() != m_bus_index)
			card->reset_in(state);
	}
}

template<typename R, typename W> void dio16_device::install_memory(offs_t start, offs_t end, R rhandler, W whandler) {
	switch (m_prgwidth) {
	case 16:
		m_prgspace->install_readwrite_handler(start, end, rhandler,
							  whandler);
		break;
	case 32:
		m_prgspace->install_readwrite_handler(start, end, rhandler,
							  whandler, 0xffffffff);
		break;
	default:
		fatalerror("DIO: Bus width %d not supported\n", m_prgwidth);
	}
}

template void dio16_device::install_memory<read16_delegate,    write16_delegate   >(offs_t start, offs_t end, read16_delegate rhandler,    write16_delegate whandler);
template void dio16_device::install_memory<read16s_delegate,   write16s_delegate  >(offs_t start, offs_t end, read16s_delegate rhandler,   write16s_delegate whandler);
template void dio16_device::install_memory<read16sm_delegate,  write16sm_delegate >(offs_t start, offs_t end, read16sm_delegate rhandler,  write16sm_delegate whandler);

void dio16_device::install_bank(offs_t start, offs_t end, uint8_t *data)
{
	m_prgspace->install_ram(start, end, data);
}

void dio16_device::unmap_bank(offs_t start, offs_t end)
{
	m_prgspace->unmap_readwrite(start, end);
}

void dio16_device::install_rom(offs_t start, offs_t end, uint8_t *data)
{
	m_prgspace->install_rom(start, end, data);
}

void dio16_device::unmap_rom(offs_t start, offs_t end)
{
	m_prgspace->unmap_read(start, end);
}

void device_dio16_card_interface::set_bus(dio16_device &bus)
{
	m_index = (m_dio_dev = &bus)->add_card(*this);
}

//**************************************************************************
//  DEVICE DIO16 CARD INTERFACE
//**************************************************************************

device_dio16_card_interface::device_dio16_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "hpdio"),
	m_dio_dev(nullptr)
{
}

device_dio16_card_interface::~device_dio16_card_interface()
{
}

void device_dio16_card_interface::interface_pre_start()
{
	if (!m_dio_dev)
		throw emu_fatalerror("device_dio16_card_interface: DIO bus not configured\n");
}

//**************************************************************************
//  DIO32 DEVICE
//**************************************************************************

dio32_device::dio32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio16_device(mconfig, DIO32, tag, owner, clock)
{
}

void dio32_device::device_start()
{
	dio16_device::device_start();
}

//**************************************************************************
//  DEVICE DIO32 CARD INTERFACE
//**************************************************************************

device_dio32_card_interface::device_dio32_card_interface(const machine_config &mconfig, device_t &device) :
	device_dio16_card_interface(mconfig, device)
{
}

device_dio32_card_interface::~device_dio32_card_interface()
{
}

void device_dio32_card_interface::interface_pre_start()
{
	device_dio16_card_interface::interface_pre_start();

	if (m_dio_dev && !dynamic_cast<dio32_device *>(m_dio_dev))
		throw emu_fatalerror("device_dio32_card_interface: DIO32 device %s (%s) in DIO16 slot %s\n", device().tag(), device().name(), m_dio_dev->name());
}

} // namespace bus::hp_dio

void dio16_cards(device_slot_interface & device)
{
	device.option_add("98543", HPDIO_98543);
	device.option_add("98544", HPDIO_98544);
	device.option_add("98603a", HPDIO_98603A);
	device.option_add("98603b", HPDIO_98603B);
	device.option_add("98624", HPDIO_98624);
	device.option_add("98643", HPDIO_98643);
	device.option_add("98644", HPDIO_98644);
	device.option_add("human_interface", HPDIO_HUMAN_INTERFACE);
}

void dio16_hp98x6_cards(device_slot_interface &device)
{
	device.option_add("98628", HPDIO_98628);
	device.option_add("98629", HPDIO_98629);
}

void dio32_cards(device_slot_interface & device)
{
	dio16_cards(device);
	device.option_add("98265a", HPDIO_98265A);
	device.option_add("98550", HPDIO_98550);
	device.option_add("98620", HPDIO_98620);
}
