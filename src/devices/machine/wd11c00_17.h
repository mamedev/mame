// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Western Digital WD11C00-17 PC/XT Host Interface Logic Device

**********************************************************************/

#pragma once

#ifndef __WD11C00_17__
#define __WD11C00_17__


#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_WD11C00_17_OUT_IRQ5_CB(_devcb) \
	devcb = &wd11c00_17_device::set_out_irq5_callback(*device, DEVCB_##_devcb);

#define MCFG_WD11C00_17_OUT_DRQ3_CB(_devcb) \
	devcb = &wd11c00_17_device::set_out_drq3_callback(*device, DEVCB_##_devcb);

#define MCFG_WD11C00_17_OUT_MR_CB(_devcb) \
	devcb = &wd11c00_17_device::set_out_mr_callback(*device, DEVCB_##_devcb);

#define MCFG_WD11C00_17_OUT_BUSY_CB(_devcb) \
	devcb = &wd11c00_17_device::set_out_busy_callback(*device, DEVCB_##_devcb);

#define MCFG_WD11C00_17_OUT_REQ_CB(_devcb) \
	devcb = &wd11c00_17_device::set_out_req_callback(*device, DEVCB_##_devcb);

#define MCFG_WD11C00_17_OUT_RA3_CB(_devcb) \
	devcb = &wd11c00_17_device::set_out_ra3_callback(*device, DEVCB_##_devcb);

#define MCFG_WD11C00_17_IN_RD322_CB(_devcb) \
	devcb = &wd11c00_17_device::set_in_rd322_callback(*device, DEVCB_##_devcb);

#define MCFG_WD11C00_17_IN_RAMCS_CB(_devcb) \
	devcb = &wd11c00_17_device::set_in_ramcs_callback(*device, DEVCB_##_devcb);

#define MCFG_WD11C00_17_OUT_RAMWR_CB(_devcb) \
	devcb = &wd11c00_17_device::set_out_ramwr_callback(*device, DEVCB_##_devcb);

#define MCFG_WD11C00_17_IN_CS1010_CB(_devcb) \
	devcb = &wd11c00_17_device::set_in_cs1010_callback(*device, DEVCB_##_devcb);

#define MCFG_WD11C00_17_OUT_CS1010_CB(_devcb) \
	devcb = &wd11c00_17_device::set_out_cs1010_callback(*device, DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wd11c00_17_device

class wd11c00_17_device :   public device_t
{
public:
	// construction/destruction
	wd11c00_17_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_out_irq5_callback(device_t &device, _Object object) { return downcast<wd11c00_17_device &>(device).m_out_irq5_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_drq3_callback(device_t &device, _Object object) { return downcast<wd11c00_17_device &>(device).m_out_drq3_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_mr_callback(device_t &device, _Object object) { return downcast<wd11c00_17_device &>(device).m_out_mr_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_busy_callback(device_t &device, _Object object) { return downcast<wd11c00_17_device &>(device).m_out_busy_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_req_callback(device_t &device, _Object object) { return downcast<wd11c00_17_device &>(device).m_out_req_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_ra3_callback(device_t &device, _Object object) { return downcast<wd11c00_17_device &>(device).m_out_ra3_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_rd322_callback(device_t &device, _Object object) { return downcast<wd11c00_17_device &>(device).m_in_rd322_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_ramcs_callback(device_t &device, _Object object) { return downcast<wd11c00_17_device &>(device).m_in_ramcs_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_ramwr_callback(device_t &device, _Object object) { return downcast<wd11c00_17_device &>(device).m_out_ramwr_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_cs1010_callback(device_t &device, _Object object) { return downcast<wd11c00_17_device &>(device).m_in_cs1010_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_cs1010_callback(device_t &device, _Object object) { return downcast<wd11c00_17_device &>(device).m_out_cs1010_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( io_r );
	DECLARE_WRITE8_MEMBER( io_w );

	UINT8 dack_r();
	void dack_w(UINT8 data);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( ireq_w );
	DECLARE_WRITE_LINE_MEMBER( io_w );
	DECLARE_WRITE_LINE_MEMBER( cd_w );
	DECLARE_WRITE_LINE_MEMBER( clct_w );
	DECLARE_WRITE_LINE_MEMBER( mode_w );

	DECLARE_READ_LINE_MEMBER( busy_r );
	DECLARE_READ_LINE_MEMBER( ecc_not_0_r );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	inline void check_interrupt();
	inline void increment_address();
	inline UINT8 read_data();
	inline void write_data(UINT8 data);
	inline void software_reset();
	inline void select();

	devcb_write_line    m_out_irq5_cb;
	devcb_write_line    m_out_drq3_cb;
	devcb_write_line    m_out_mr_cb;
	devcb_write_line    m_out_busy_cb;
	devcb_write_line    m_out_req_cb;
	devcb_write_line    m_out_ra3_cb;
	devcb_read8         m_in_rd322_cb;
	devcb_read8         m_in_ramcs_cb;
	devcb_write8        m_out_ramwr_cb;
	devcb_read8         m_in_cs1010_cb;
	devcb_write8        m_out_cs1010_cb;

	UINT8 m_status;
	UINT8 m_mask;

	offs_t m_ra;

	int m_mode;
	int m_ecc_not_0;

	int m_irq5;
	int m_drq3;
	int m_busy;
	int m_req;
	int m_ra3;
};


// device type definition
extern const device_type WD11C00_17;

#endif
