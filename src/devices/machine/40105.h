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
	cmos_40105_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _dir> void set_dir_callback(_dir dir) { m_write_dir.set_callback(dir); }
	template<class _dor> void set_dor_callback(_dor dor) { m_write_dor.set_callback(dor); }

	uint8_t read();
	void write(uint8_t data);

	void si_w(int state);
	void so_w(int state);

	int dir_r();
	int dor_r();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_write_line m_write_dir;
	devcb_write_line m_write_dor;

	std::queue<uint8_t> m_fifo;

	uint8_t m_d;
	uint8_t m_q;

	int m_dir;
	int m_dor;
	int m_si;
	int m_so;
};


// device type definition
extern const device_type CMOS_40105;



#endif
