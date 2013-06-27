/*

scsicb.h

Implementation of a raw SCSI/SASI bus for machines that don't use a SCSI
controler chip such as the RM Nimbus, which implements it as a bunch of
74LS series chips.

*/

#pragma once

#ifndef _SCSICB_H_
#define _SCSICB_H_

#include "scsidev.h"

class scsicb_device : public scsidev_device
{
public:
	// construction/destruction
	scsicb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb2_base &set_bsy_handler(device_t &device, _Object object) { return downcast<scsicb_device &>(device).m_bsy_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_sel_handler(device_t &device, _Object object) { return downcast<scsicb_device &>(device).m_sel_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_cd_handler(device_t &device, _Object object) { return downcast<scsicb_device &>(device).m_cd_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_io_handler(device_t &device, _Object object) { return downcast<scsicb_device &>(device).m_io_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_msg_handler(device_t &device, _Object object) { return downcast<scsicb_device &>(device).m_msg_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_req_handler(device_t &device, _Object object) { return downcast<scsicb_device &>(device).m_req_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_ack_handler(device_t &device, _Object object) { return downcast<scsicb_device &>(device).m_ack_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_atn_handler(device_t &device, _Object object) { return downcast<scsicb_device &>(device).m_atn_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_rst_handler(device_t &device, _Object object) { return downcast<scsicb_device &>(device).m_rst_handler.set_callback(object); }

	virtual void scsi_in( UINT32 data, UINT32 mask );

	UINT8 scsi_data_r();
	void scsi_data_w( UINT8 data );

	DECLARE_READ8_MEMBER( scsi_data_r );
	DECLARE_WRITE8_MEMBER( scsi_data_w );

	DECLARE_READ_LINE_MEMBER( scsi_bsy_r );
	DECLARE_READ_LINE_MEMBER( scsi_sel_r );
	DECLARE_READ_LINE_MEMBER( scsi_cd_r );
	DECLARE_READ_LINE_MEMBER( scsi_io_r );
	DECLARE_READ_LINE_MEMBER( scsi_msg_r );
	DECLARE_READ_LINE_MEMBER( scsi_req_r );
	DECLARE_READ_LINE_MEMBER( scsi_ack_r );
	DECLARE_READ_LINE_MEMBER( scsi_atn_r );
	DECLARE_READ_LINE_MEMBER( scsi_rst_r );

	DECLARE_WRITE_LINE_MEMBER( scsi_bsy_w );
	DECLARE_WRITE_LINE_MEMBER( scsi_sel_w );
	DECLARE_WRITE_LINE_MEMBER( scsi_cd_w );
	DECLARE_WRITE_LINE_MEMBER( scsi_io_w );
	DECLARE_WRITE_LINE_MEMBER( scsi_msg_w );
	DECLARE_WRITE_LINE_MEMBER( scsi_req_w );
	DECLARE_WRITE_LINE_MEMBER( scsi_ack_w );
	DECLARE_WRITE_LINE_MEMBER( scsi_atn_w );
	DECLARE_WRITE_LINE_MEMBER( scsi_rst_w );

protected:
	// device-level overrides
	virtual void device_start();

private:
	UINT8 get_scsi_line(UINT32 mask);
	void set_scsi_line(UINT32 mask, UINT8 state);
	void trigger_callback(UINT32 update_mask, UINT32 line_mask, devcb2_write_line &write_line);
	const char *get_line_name(UINT32 mask);

	devcb2_write_line m_bsy_handler;
	devcb2_write_line m_sel_handler;
	devcb2_write_line m_cd_handler;
	devcb2_write_line m_io_handler;
	devcb2_write_line m_msg_handler;
	devcb2_write_line m_req_handler;
	devcb2_write_line m_ack_handler;
	devcb2_write_line m_atn_handler;
	devcb2_write_line m_rst_handler;

	UINT32 linestate;
};

#define MCFG_SCSICB_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SCSICB, 0)

#define MCFG_SCSICB_BSY_HANDLER(_devcb) \
	devcb = &scsicb_device::set_bsy_handler(*device, DEVCB2_##_devcb);
#define MCFG_SCSICB_SEL_HANDLER(_devcb) \
	devcb = &scsicb_device::set_sel_handler(*device, DEVCB2_##_devcb);
#define MCFG_SCSICB_CD_HANDLER(_devcb) \
	devcb = &scsicb_device::set_cd_handler(*device, DEVCB2_##_devcb);
#define MCFG_SCSICB_IO_HANDLER(_devcb) \
	devcb = &scsicb_device::set_io_handler(*device, DEVCB2_##_devcb);
#define MCFG_SCSICB_MSG_HANDLER(_devcb) \
	devcb = &scsicb_device::set_msg_handler(*device, DEVCB2_##_devcb);
#define MCFG_SCSICB_REQ_HANDLER(_devcb) \
	devcb = &scsicb_device::set_req_handler(*device, DEVCB2_##_devcb);
#define MCFG_SCSICB_ACK_HANDLER(_devcb) \
	devcb = &scsicb_device::set_ack_handler(*device, DEVCB2_##_devcb);
#define MCFG_SCSICB_ATN_HANDLER(_devcb) \
	devcb = &scsicb_device::set_atn_handler(*device, DEVCB2_##_devcb);
#define MCFG_SCSICB_RST_HANDLER(_devcb) \
	devcb = &scsicb_device::set_rst_handler(*device, DEVCB2_##_devcb);
// device type definition
extern const device_type SCSICB;

#endif
