// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/*******************************************************************
 *
 * Epson QX-10 Expansion emulation
 *
 *******************************************************************/
#include "emu.h"
#include "option.h"

DEFINE_DEVICE_TYPE(EPSON_QX_OPTION_BUS_SLOT, bus::epson_qx::option_slot_device, "epson_qx_option_slot", "QX-10 Option slot")
DEFINE_DEVICE_TYPE(EPSON_QX_OPTION_BUS, bus::epson_qx::option_bus_device, "epson_qx_option_bus", "QX-10 Option Bus")

namespace bus::epson_qx {
//**************************************************************************
//  EPSON SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  option_bus_slot_device - constructor
//-------------------------------------------------
option_slot_device::option_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EPSON_QX_OPTION_BUS_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_option_expansion_interface>(mconfig, *this),
	m_bus(*this, finder_base::DUMMY_TAG),
	m_dmas_w_cb(*this),
	m_dmas_r_cb(*this),
	m_dmaf_w_cb(*this),
	m_dmaf_r_cb(*this),
	m_eopf_cb(*this),
	m_eops_cb(*this),
	m_slot(-1)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void option_slot_device::device_start()
{
	m_dmas_w_cb.resolve_safe();
	m_dmas_r_cb.resolve_safe(0xff);
	m_dmaf_w_cb.resolve();
	m_dmaf_r_cb.resolve();

	m_eopf_cb.resolve_safe();
	m_eops_cb.resolve_safe();

	device_option_expansion_interface *const intf(get_card_device());
	if (intf) {
		intf->set_option_bus(*m_bus, m_slot);
	}

	m_bus->add_slot(*this);
}

WRITE_LINE_MEMBER(option_slot_device::eopf) { m_eopf_cb(state); }
WRITE_LINE_MEMBER(option_slot_device::eops) { m_eops_cb(state); }

WRITE_LINE_MEMBER(option_slot_device::inth1_w) { (*m_bus).set_inth1_line(state, m_slot); }
WRITE_LINE_MEMBER(option_slot_device::inth2_w) { (*m_bus).set_inth2_line(state, m_slot); }
WRITE_LINE_MEMBER(option_slot_device::intl_w) { (*m_bus).set_intl_line(state, m_slot); }

WRITE_LINE_MEMBER(option_slot_device::drqf_w) { (*m_bus).set_drqf_line(state, m_slot); }
WRITE_LINE_MEMBER(option_slot_device::drqs_w) { (*m_bus).set_drqs_line(state, m_slot); }

WRITE_LINE_MEMBER(option_slot_device::rdyf_w) { (*m_bus).set_rdyf_line(state, m_slot); }
WRITE_LINE_MEMBER(option_slot_device::rdys_w) { (*m_bus).set_rdys_line(state, m_slot); }

//**************************************************************************
//  EPSON OPTION BUS DEVICE
//**************************************************************************

//-------------------------------------------------
//  option_bus_device - constructor
//-------------------------------------------------
option_bus_device::option_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EPSON_QX_OPTION_BUS, tag, owner, clock),
	m_iospace(*this, finder_base::DUMMY_TAG, -1),
	m_inth1_cb(*this),
	m_inth2_cb(*this),
	m_intl_cb(*this),
	m_drqf_cb(*this),
	m_drqs_cb(*this),
	m_rdyf_cb(*this),
	m_rdys_cb(*this),
	m_inth1(0),
	m_inth2(0),
	m_drqf(0),
	m_rdyf(0),
	m_rdys(0)
{
}

void option_bus_device::add_slot(option_slot_device &slot)
{
	m_slot_list.push_back(&slot);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void option_bus_device::device_start()
{
	m_inth1_cb.resolve_safe();
	m_inth2_cb.resolve_safe();
	m_intl_cb.resolve_all_safe();
	m_drqf_cb.resolve_safe();
	m_drqs_cb.resolve_all_safe();
	m_rdyf_cb.resolve_safe();
	m_rdys_cb.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific startup
//-------------------------------------------------
void option_bus_device::device_reset()
{
	m_inth1 = 0;
	m_inth2 = 0;
	m_drqf = 0;
	m_rdyf = 0;
	m_rdys = 0;
}

void option_bus_device::dackf_w(uint8_t data)
{
	for (int i = 0; i < m_slot_list.size(); ++i) {
		if (!m_slot_list[i]->m_dmaf_w_cb.isnull()) {
			m_slot_list[i]->m_dmaf_w_cb(data);
		}
	}
}

uint8_t option_bus_device::dackf_r()
{
	uint8_t value = 0;
	bool found = false;
	for (int i = 0; i < m_slot_list.size(); ++i) {
		if (!m_slot_list[i]->m_dmaf_r_cb.isnull()) {
			found = true;
			value |= m_slot_list[i]->m_dmaf_r_cb();
		}
	}
	return (found ? value : 0xff);
}

void option_bus_device::set_inth1_line(uint8_t state, uint8_t slot)
{
	m_inth1 = state ? (m_inth1 | (1 << (slot & 0x07))) : (m_inth1 & ~(1 << (slot & 0x07)));
	m_inth1_cb(m_inth1 != 0);
}

void option_bus_device::set_inth2_line(uint8_t state, uint8_t slot)
{
	m_inth2 = state ? (m_inth2 | (1 << (slot & 0x07))) : (m_inth2 & ~(1 << (slot & 0x07)));
	m_inth2_cb(m_inth2 != 0);
}

void option_bus_device::set_drqf_line(uint8_t state, uint8_t slot)
{
	m_drqf = !state ? (m_drqf | (1 << (slot & 0x07))) : (m_drqf & ~(1 << (slot & 0x07)));
	m_drqf_cb(m_drqf == 0);
}

void option_bus_device::set_rdyf_line(uint8_t state, uint8_t slot)
{
	m_rdyf = !state ? (m_rdyf | (1 << (slot & 0x07))) : (m_rdyf & ~(1 << (slot & 0x07)));
	m_rdyf_cb(m_rdyf == 0);
}

void option_bus_device::set_rdys_line(uint8_t state, uint8_t slot)
{
	m_rdys = !state ? (m_rdys | (1 << (slot & 0x07))) : (m_rdys & ~(1 << (slot & 0x07)));
	m_rdys_cb(m_rdys == 0);
}


//**************************************************************************
//  EPSON OPTION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  option_device_expansion_interface - constructor
//-------------------------------------------------
device_option_expansion_interface::device_option_expansion_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "qx_option_bus"),
	m_bus(nullptr),
	m_slot(-1)
{
}

//---------------------------------------------------
//  interface_pre_start - device-specific pre startup
//---------------------------------------------------
void device_option_expansion_interface::interface_pre_start()
{
	if (!m_bus || m_slot == -1)
		throw device_missing_dependencies();
}

//-------------------------------------------------
//  SLOT_INTERFACE( option_bus_devices )
//-------------------------------------------------

void option_bus_devices(device_slot_interface &device)
{
}

}  // namespace bus::epson_qx
