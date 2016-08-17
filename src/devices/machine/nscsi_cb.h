// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef NSCSI_CB_H
#define NSCSI_CB_H

#include "emu.h"
#include "machine/nscsi_bus.h"

#define MCFG_NSCSICB_RST_HANDLER(_line) \
	downcast<nscsi_callback_device *>(device)->set_rst_callback(DEVCB_##_line);

#define MCFG_NSCSICB_ATN_HANDLER(_line) \
	downcast<nscsi_callback_device *>(device)->set_atn_callback(DEVCB_##_line);

#define MCFG_NSCSICB_ACK_HANDLER(_line) \
	downcast<nscsi_callback_device *>(device)->set_ack_callback(DEVCB_##_line);

#define MCFG_NSCSICB_REQ_HANDLER(_line) \
	downcast<nscsi_callback_device *>(device)->set_req_callback(DEVCB_##_line);

#define MCFG_NSCSICB_MSG_HANDLER(_line) \
	downcast<nscsi_callback_device *>(device)->set_msg_callback(DEVCB_##_line);

#define MCFG_NSCSICB_IO_HANDLER(_line) \
	downcast<nscsi_callback_device *>(device)->set_io_callback(DEVCB_##_line);

#define MCFG_NSCSICB_CD_HANDLER(_line) \
	downcast<nscsi_callback_device *>(device)->set_cd_callback(DEVCB_##_line);

#define MCFG_NSCSICB_SEL_HANDLER(_line) \
	downcast<nscsi_callback_device *>(device)->set_sel_callback(DEVCB_##_line);

#define MCFG_NSCSICB_BSY_HANDLER(_line) \
	downcast<nscsi_callback_device *>(device)->set_bsy_callback(DEVCB_##_line);

class nscsi_callback_device : public nscsi_device
{
public:
	nscsi_callback_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _line> void set_rst_callback(_line line) { m_write_rst.set_callback(line); }
	template<class _line> void set_atn_callback(_line line) { m_write_atn.set_callback(line); }
	template<class _line> void set_ack_callback(_line line) { m_write_ack.set_callback(line); }
	template<class _line> void set_req_callback(_line line) { m_write_req.set_callback(line); }
	template<class _line> void set_msg_callback(_line line) { m_write_msg.set_callback(line); }
	template<class _line> void set_io_callback(_line line)  { m_write_io.set_callback(line); }
	template<class _line> void set_cd_callback(_line line)  { m_write_cd.set_callback(line); }
	template<class _line> void set_sel_callback(_line line) { m_write_sel.set_callback(line); }
	template<class _line> void set_bsy_callback(_line line) { m_write_bsy.set_callback(line); }

	virtual void scsi_ctrl_changed() override;

	UINT8 read() { return scsi_bus->data_r(); }
	DECLARE_READ8_MEMBER( read ) { return read(); }
	void write(UINT8 data) { scsi_bus->data_w(scsi_refid, data); }
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

	UINT32 m_ctrl;
};

extern const device_type NSCSI_CB;

#endif
