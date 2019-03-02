// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_NSCSI_CB_H
#define MAME_MACHINE_NSCSI_CB_H

#pragma once

#include "machine/nscsi_bus.h"


class nscsi_callback_device : public nscsi_device
{
public:
	nscsi_callback_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto rst_callback() { return m_write_rst.bind(); }
	auto atn_callback() { return m_write_atn.bind(); }
	auto ack_callback() { return m_write_ack.bind(); }
	auto req_callback() { return m_write_req.bind(); }
	auto msg_callback() { return m_write_msg.bind(); }
	auto io_callback()  { return m_write_io.bind(); }
	auto cd_callback()  { return m_write_cd.bind(); }
	auto sel_callback() { return m_write_sel.bind(); }
	auto bsy_callback() { return m_write_bsy.bind(); }

	virtual void scsi_ctrl_changed() override;

	uint8_t read() { return scsi_bus->data_r(); }
	DECLARE_READ8_MEMBER( read ) { return read(); }
	void write(uint8_t data) { scsi_bus->data_w(scsi_refid, data); }
	DECLARE_WRITE8_MEMBER( write ) { write(data); }

	DECLARE_READ_LINE_MEMBER( rst_r ) { return (m_ctrl & S_RST) ? 1 : 0; }
	DECLARE_READ_LINE_MEMBER( atn_r ) { return (m_ctrl & S_ATN) ? 1 : 0; }
	DECLARE_READ_LINE_MEMBER( ack_r ) { return (m_ctrl & S_ACK) ? 1 : 0; }
	DECLARE_READ_LINE_MEMBER( req_r ) { return (m_ctrl & S_REQ) ? 1 : 0; }
	DECLARE_READ_LINE_MEMBER( msg_r ) { return (m_ctrl & S_MSG) ? 1 : 0; }
	DECLARE_READ_LINE_MEMBER( io_r )  { return (m_ctrl & S_INP) ? 1 : 0; }
	DECLARE_READ_LINE_MEMBER( cd_r )  { return (m_ctrl & S_CTL) ? 1 : 0; }
	DECLARE_READ_LINE_MEMBER( sel_r ) { return (m_ctrl & S_SEL) ? 1 : 0; }
	DECLARE_READ_LINE_MEMBER( bsy_r ) { return (m_ctrl & S_BSY) ? 1 : 0; }

	DECLARE_WRITE_LINE_MEMBER( rst_w ) { scsi_bus->ctrl_w(scsi_refid, state ? S_RST : 0, S_RST); }
	DECLARE_WRITE_LINE_MEMBER( atn_w ) { scsi_bus->ctrl_w(scsi_refid, state ? S_ATN : 0, S_ATN); }
	DECLARE_WRITE_LINE_MEMBER( ack_w ) { scsi_bus->ctrl_w(scsi_refid, state ? S_ACK : 0, S_ACK); }
	DECLARE_WRITE_LINE_MEMBER( req_w ) { scsi_bus->ctrl_w(scsi_refid, state ? S_REQ : 0, S_REQ); }
	DECLARE_WRITE_LINE_MEMBER( msg_w ) { scsi_bus->ctrl_w(scsi_refid, state ? S_MSG : 0, S_MSG); }
	DECLARE_WRITE_LINE_MEMBER( io_w )  { scsi_bus->ctrl_w(scsi_refid, state ? S_INP : 0, S_INP); }
	DECLARE_WRITE_LINE_MEMBER( cd_w )  { scsi_bus->ctrl_w(scsi_refid, state ? S_CTL : 0, S_CTL); }
	DECLARE_WRITE_LINE_MEMBER( sel_w ) { scsi_bus->ctrl_w(scsi_refid, state ? S_SEL : 0, S_SEL); }
	DECLARE_WRITE_LINE_MEMBER( bsy_w ) { scsi_bus->ctrl_w(scsi_refid, state ? S_BSY : 0, S_BSY); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	devcb_write_line m_write_rst;
	devcb_write_line m_write_atn;
	devcb_write_line m_write_ack;
	devcb_write_line m_write_req;
	devcb_write_line m_write_msg;
	devcb_write_line m_write_io;
	devcb_write_line m_write_cd;
	devcb_write_line m_write_sel;
	devcb_write_line m_write_bsy;

	uint32_t m_ctrl;
};

DECLARE_DEVICE_TYPE(NSCSI_CB, nscsi_callback_device)

#endif // MAME_MACHINE_NSCSI_CB_H
