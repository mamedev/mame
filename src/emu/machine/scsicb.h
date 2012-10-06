/*
    SCSICB.h

    Callbacks from SCSI/SASI bus for machines that don't use a SCSI
    controler chip such as the RM Nimbus, which implements it as a bunch of
    74LS series chips.

*/

#pragma once

#ifndef _SCSICB_H_
#define _SCSICB_H_

#include "emu.h"

/***************************************************************************
    MACROS
***************************************************************************/

#define MCFG_SCSICB_ADD(_tag, _intf) \
	MCFG_DEVICE_ADD(_tag, SCSICB, 0) \
	MCFG_DEVICE_CONFIG(_intf)


struct SCSICB_interface
{
	void (*line_change_cb)(device_t *, UINT8 line, UINT8 state);

	devcb_write_line _out_bsy_func;
	devcb_write_line _out_sel_func;
	devcb_write_line _out_cd_func;
	devcb_write_line _out_io_func;
	devcb_write_line _out_msg_func;
	devcb_write_line _out_req_func;
	devcb_write_line _out_rst_func;
};

class scsicb_device : public device_t,
					  public SCSICB_interface
{
public:
	// construction/destruction
	scsicb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	devcb_resolved_write_line out_bsy_func;
	devcb_resolved_write_line out_sel_func;
	devcb_resolved_write_line out_cd_func;
	devcb_resolved_write_line out_io_func;
	devcb_resolved_write_line out_msg_func;
	devcb_resolved_write_line out_req_func;
	devcb_resolved_write_line out_rst_func;

	UINT8 scsi_data_r();
	void scsi_data_w( UINT8 data );

	UINT8 get_scsi_line(UINT8 lineno);
	void set_scsi_line(UINT8 line, UINT8 state);

	DECLARE_READ8_MEMBER( scsi_data_r );
	DECLARE_WRITE8_MEMBER( scsi_data_w );

	DECLARE_READ_LINE_MEMBER( scsi_bsy_r );
	DECLARE_READ_LINE_MEMBER( scsi_sel_r );
	DECLARE_READ_LINE_MEMBER( scsi_cd_r );
	DECLARE_READ_LINE_MEMBER( scsi_io_r );
	DECLARE_READ_LINE_MEMBER( scsi_msg_r );
	DECLARE_READ_LINE_MEMBER( scsi_req_r );
	DECLARE_READ_LINE_MEMBER( scsi_ack_r );
	DECLARE_READ_LINE_MEMBER( scsi_rst_r );

	DECLARE_WRITE_LINE_MEMBER( scsi_bsy_w );
	DECLARE_WRITE_LINE_MEMBER( scsi_sel_w );
	DECLARE_WRITE_LINE_MEMBER( scsi_cd_w );
	DECLARE_WRITE_LINE_MEMBER( scsi_io_w );
	DECLARE_WRITE_LINE_MEMBER( scsi_msg_w );
	DECLARE_WRITE_LINE_MEMBER( scsi_req_w );
	DECLARE_WRITE_LINE_MEMBER( scsi_ack_w );
	DECLARE_WRITE_LINE_MEMBER( scsi_rst_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
};

// device type definition
extern const device_type SCSICB;

#endif
