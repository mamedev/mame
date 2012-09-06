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


typedef struct _SCSICB_interface SCSICB_interface;
struct _SCSICB_interface
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

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
};

// device type definition
extern const device_type SCSICB;

#endif
