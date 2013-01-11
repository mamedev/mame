/**********************************************************************

    Western Digital WD2010 Winchester Disk Controller

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __WD2010__
#define __WD2010__


#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_WD2010_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, WD2010, _clock) \
	MCFG_DEVICE_CONFIG(_config)


#define WD2010_INTERFACE(_name) \
	const wd2010_interface (_name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wd2010_interface

struct wd2010_interface
{
	devcb_write_line    m_out_intrq_cb;
	devcb_write_line    m_out_bdrq_cb;
	devcb_write_line    m_out_bcr_cb;
	devcb_read8         m_in_bcs_cb;
	devcb_write8        m_out_bcs_cb;
	devcb_write_line    m_out_dirin_cb;
	devcb_write_line    m_out_step_cb;
	devcb_write_line    m_out_rwc_cb;
	devcb_read_line     m_in_drdy_cb;
	devcb_read_line     m_in_index_cb;
	devcb_read_line     m_in_wf_cb;
	devcb_read_line     m_in_tk000_cb;
	devcb_read_line     m_in_sc_cb;
};


// ======================> wd2010_device

class wd2010_device :   public device_t,
						public wd2010_interface
{
public:
	// construction/destruction
	wd2010_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();

private:
	void compute_correction(UINT8 data);
	void set_parameter(UINT8 data);
	void restore(UINT8 data);
	void seek(UINT8 data);
	void read_sector(UINT8 data);
	void write_sector(UINT8 data);
	void scan_id(UINT8 data);
	void format(UINT8 data);

	devcb_resolved_write_line   m_out_intrq_func;
	devcb_resolved_write_line   m_out_bdrq_func;
	devcb_resolved_write_line   m_out_bcr_func;
	devcb_resolved_read8        m_in_bcs_func;
	devcb_resolved_write8       m_out_bcs_func;
	devcb_resolved_write_line   m_out_dirin_func;
	devcb_resolved_write_line   m_out_step_func;
	devcb_resolved_write_line   m_out_rwc_func;
	devcb_resolved_read_line    m_in_drdy_func;
	devcb_resolved_read_line    m_in_index_func;
	devcb_resolved_read_line    m_in_wf_func;
	devcb_resolved_read_line    m_in_tk000_func;
	devcb_resolved_read_line    m_in_sc_func;

	UINT8 m_status;
	UINT8 m_error;
	UINT8 m_task_file[8];
};


// device type definition
extern const device_type WD2010;

#endif
