// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "nscsi_bus.h"

#include <cassert>
#include <cstring>

#define LOG_CONTROL     (1U << 1)

//#define VERBOSE (LOG_CONTROL)
//#define LOG_OUTPUT_FUNC osd_printf_info



#include "logmacro.h"

DEFINE_DEVICE_TYPE(NSCSI_BUS,       nscsi_bus_device, "nscsi_bus",       "SCSI Bus (new)")
DEFINE_DEVICE_TYPE(NSCSI_CONNECTOR, nscsi_connector,  "nscsi_connector", "SCSI Connector Abstraction (new)")


nscsi_bus_device::nscsi_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NSCSI_BUS, tag, owner, clock),
	m_external_devices(*this, finder_base::DUMMY_TAG, 0),
	m_bsy_handler(*this),
	m_data(0),
	m_ctrl(0)
{
	m_devcnt = 0;
	std::fill(std::begin(m_dev), std::end(m_dev), dev_t{ nullptr, 0, 0, 0 });
}

void nscsi_bus_device::device_start()
{
	m_data = 0;
	m_ctrl = 0;
	save_item(NAME(m_data));
	save_item(NAME(m_ctrl));
	for(int i=0; i<m_devcnt; i++) {
		save_item(NAME(m_dev[i].m_data), i);
		save_item(NAME(m_dev[i].m_ctrl), i);
		save_item(NAME(m_dev[i].m_wait_ctrl), i);
	}
}

void nscsi_bus_device::device_reset()
{
}

void nscsi_bus_device::regen_data()
{
	m_data = 0;
	for(int i=0; i<m_devcnt; i++)
		m_data |= m_dev[i].m_data;
}

void nscsi_bus_device::regen_ctrl(int refid)
{
	static char const *const phase[8] = {
		"dout", "din ", "cmd ", "stat", "4   ", "5   ", "mout", "min "
	};

	uint32_t octrl = m_ctrl;
	m_ctrl = 0;
	for(int i=0; i<m_devcnt; i++)
		m_ctrl |= m_dev[i].m_ctrl;

	if(VERBOSE & LOG_CONTROL) {
		LOGMASKED(LOG_CONTROL, "ctrl %c%c%c%c%c%c%c%c%c %s %04x\n",
					m_ctrl & nscsi_device_interface::S_RST ? 'R' : '.',
					m_ctrl & nscsi_device_interface::S_ATN ? 'A' : '.',
					m_ctrl & nscsi_device_interface::S_ACK ? 'K' : '.',
					m_ctrl & nscsi_device_interface::S_REQ ? 'Q' : '.',
					m_ctrl & nscsi_device_interface::S_SEL ? 'S' : '.',
					m_ctrl & nscsi_device_interface::S_BSY ? 'B' : '.',
					m_ctrl & nscsi_device_interface::S_MSG ? 'M' : '.',
					m_ctrl & nscsi_device_interface::S_CTL ? 'C' : '.',
					m_ctrl & nscsi_device_interface::S_INP ? 'I' : '.',
					phase[m_ctrl & 7],
					m_data);
		for(int i=0; i<m_devcnt; i++)
			if(m_dev[i].m_ctrl) {
				LOGMASKED(LOG_CONTROL, "dev%d=%s%s%s%s%s%s%s%s%s\n", i,
							m_dev[i].m_ctrl & nscsi_device_interface::S_RST ? "R" : "",
							m_dev[i].m_ctrl & nscsi_device_interface::S_ATN ? "A" : "",
							m_dev[i].m_ctrl & nscsi_device_interface::S_ACK ? "K" : "",
							m_dev[i].m_ctrl & nscsi_device_interface::S_REQ ? "Q" : "",
							m_dev[i].m_ctrl & nscsi_device_interface::S_SEL ? "S" : "",
							m_dev[i].m_ctrl & nscsi_device_interface::S_BSY ? "B" : "",
							m_dev[i].m_ctrl & nscsi_device_interface::S_MSG ? "M" : "",
							m_dev[i].m_ctrl & nscsi_device_interface::S_CTL ? "C" : "",
							m_dev[i].m_ctrl & nscsi_device_interface::S_INP ? "I" : "");
			}
	}

	octrl = octrl ^ m_ctrl;
	if(octrl) {
		for(int i=0; i<m_devcnt; i++)
			if(i != refid && (m_dev[i].m_wait_ctrl & octrl))
				m_dev[i].m_dev->scsi_ctrl_changed();

		if (octrl & nscsi_device_interface::S_BSY)
			m_bsy_handler((m_ctrl & nscsi_device_interface::S_BSY) ? ASSERT_LINE : CLEAR_LINE);
	}
}

uint32_t nscsi_bus_device::data_r() const
{
	return m_data;
}

uint32_t nscsi_bus_device::ctrl_r() const
{
	return m_ctrl;
}

void nscsi_bus_device::ctrl_w(int refid, uint32_t lines, uint32_t mask)
{
	uint32_t c = m_dev[refid].m_ctrl;
	m_dev[refid].m_ctrl = (c & ~mask) | (lines & mask);
	regen_ctrl(refid);
}

void nscsi_bus_device::data_w(int refid, uint32_t lines)
{
	m_dev[refid].m_data = lines;
	regen_data();
}

void nscsi_bus_device::ctrl_wait(int refid, uint32_t lines, uint32_t mask)
{
	uint32_t w = m_dev[refid].m_wait_ctrl;
	m_dev[refid].m_wait_ctrl = (w & ~mask) | (lines & mask);
}

void nscsi_bus_device::device_resolve_objects()
{
	for(int i=0; i<16; i++) {
		nscsi_device_interface *sdev = nullptr;
		if(m_external_devices[i])
			sdev = m_external_devices[i];
		else {
			device_t *subdev = subdevice(string_format("%d", i));
			sdev = subdev ? downcast<nscsi_connector &>(*subdev).get_device() : nullptr;
		}
		if(sdev) {
			int rid = m_devcnt++;
			m_dev[rid].m_dev = sdev;
			sdev->connect_to_bus(this, rid, i);
		}
	}
}


nscsi_connector::nscsi_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NSCSI_CONNECTOR, tag, owner, clock),
	device_single_card_slot_interface<nscsi_slot_card_interface>(mconfig, *this)
{
}

nscsi_connector::~nscsi_connector()
{
}

void nscsi_connector::device_start()
{
}

nscsi_device_interface *nscsi_connector::get_device()
{
	nscsi_slot_card_interface *const connected = get_card_device();
	if (connected)
		return connected->device().subdevice<nscsi_device_interface>(connected->m_nscsi.finder_tag());
	else
		return nullptr;
}

nscsi_slot_card_interface::nscsi_slot_card_interface(const machine_config &mconfig, device_t &device, const char *nscsi_tag) :
	device_interface(device, "nscsi"),
	m_nscsi(device, nscsi_tag)
{
}

nscsi_device_interface::nscsi_device_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "nscsi-dev")
{
	m_scsi_id = m_scsi_refid = -1;
	m_scsi_bus = nullptr;
}

void nscsi_device_interface::connect_to_bus(nscsi_bus_device *bus, int refid, int default_scsi_id)
{
	m_scsi_bus = bus;
	m_scsi_refid = refid;
	m_scsi_id = default_scsi_id;
}

void nscsi_device_interface::scsi_ctrl_changed()
{
}

void nscsi_device_interface::interface_pre_start()
{
	device().save_item(NAME(m_scsi_id));
}
