// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "nscsi_cb.h"

const device_type NSCSI_CB = &device_creator<nscsi_callback_device>;

nscsi_callback_device::nscsi_callback_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: nscsi_device(mconfig, NSCSI_CB, "SCSI callback (new)", tag, owner, clock, "nscsi_cb", __FILE__),
		m_write_rst(*this),
		m_write_atn(*this),
		m_write_ack(*this),
		m_write_req(*this),
		m_write_msg(*this),
		m_write_io(*this),
		m_write_cd(*this),
		m_write_sel(*this),
		m_write_bsy(*this), m_ctrl(0)
{
}

void nscsi_callback_device::device_start()
{
	// resolve callbacks
	m_write_rst.resolve_safe();
	m_write_atn.resolve_safe();
	m_write_ack.resolve_safe();
	m_write_req.resolve_safe();
	m_write_msg.resolve_safe();
	m_write_io.resolve_safe();
	m_write_cd.resolve_safe();
	m_write_sel.resolve_safe();
	m_write_bsy.resolve_safe();

	// state saving
	save_item(NAME(m_ctrl));
}

void nscsi_callback_device::device_reset()
{
	scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);
	scsi_bus->ctrl_wait(scsi_refid, S_ALL, S_ALL);
}

void nscsi_callback_device::scsi_ctrl_changed()
{
	m_ctrl = scsi_bus->ctrl_r();

	m_write_rst((m_ctrl & S_RST) ? 1 : 0);
	m_write_atn((m_ctrl & S_ATN) ? 1 : 0);
	m_write_ack((m_ctrl & S_ACK) ? 1 : 0);
	m_write_req((m_ctrl & S_REQ) ? 1 : 0);
	m_write_msg((m_ctrl & S_MSG) ? 1 : 0);
	m_write_io((m_ctrl & S_INP) ? 1 : 0);
	m_write_cd((m_ctrl & S_CTL) ? 1 : 0);
	m_write_sel((m_ctrl & S_SEL) ? 1 : 0);
	m_write_bsy((m_ctrl & S_BSY) ? 1 : 0);
}
