// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CMOS 40105 FIFO Register emulation

**********************************************************************/

#pragma once

#ifndef __CMOS_40105__
#define __CMOS_40105__

#include "emu.h"
#include <queue>



///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_40105_ADD(_tag, _dir, _dor) \
	MCFG_DEVICE_ADD(_tag, CMOS_40105, 0) \
	downcast<cmos_40105_device *>(device)->set_dir_callback(DEVCB_##_dir); \
	downcast<cmos_40105_device *>(device)->set_dor_callback(DEVCB_##_dor);



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> cmos_40105_device

class cmos_40105_device :  public device_t
{
public:
	// construction/destruction
	cmos_40105_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _dir> void set_dir_callback(_dir dir) { m_write_dir.set_callback(dir); }
	template<class _dor> void set_dor_callback(_dor dor) { m_write_dor.set_callback(dor); }

	UINT8 read();
	void write(UINT8 data);

	DECLARE_WRITE_LINE_MEMBER( si_w );
	DECLARE_WRITE_LINE_MEMBER( so_w );

	DECLARE_READ_LINE_MEMBER( dir_r );
	DECLARE_READ_LINE_MEMBER( dor_r );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	devcb_write_line m_write_dir;
	devcb_write_line m_write_dor;

	std::queue<UINT8> m_fifo;

	UINT8 m_d;
	UINT8 m_q;

	int m_dir;
	int m_dor;
	int m_si;
	int m_so;
};


// device type definition
extern const device_type CMOS_40105;



#endif
