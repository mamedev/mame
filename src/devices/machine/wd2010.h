// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Western Digital WD2010 Winchester Disk Controller

**********************************************************************/

#pragma once

#ifndef __WD2010__
#define __WD2010__


#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_WD2010_OUT_INTRQ_CB(_devcb) \
	devcb = &wd2010_device::set_out_intrq_callback(*device, DEVCB_##_devcb);

#define MCFG_WD2010_OUT_BDRQ_CB(_devcb) \
	devcb = &wd2010_device::set_out_bdrq_callback(*device, DEVCB_##_devcb);

#define MCFG_WD2010_OUT_BCR_CB(_devcb) \
	devcb = &wd2010_device::set_out_bcr_callback(*device, DEVCB_##_devcb);

#define MCFG_WD2010_IN_BRDY_CB(_devcb) \
	devcb = &wd2010_device::set_in_brdy_callback(*device, DEVCB_##_devcb);

#define MCFG_WD2010_IN_BCS_CB(_devcb) \
	devcb = &wd2010_device::set_in_bcs_callback(*device, DEVCB_##_devcb);

#define MCFG_WD2010_OUT_BCS_CB(_devcb) \
	devcb = &wd2010_device::set_out_bcs_callback(*device, DEVCB_##_devcb);

#define MCFG_WD2010_OUT_DIRIN_CB(_devcb) \
	devcb = &wd2010_device::set_out_dirin_callback(*device, DEVCB_##_devcb);

#define MCFG_WD2010_OUT_STEP_CB(_devcb) \
	devcb = &wd2010_device::set_out_step_callback(*device, DEVCB_##_devcb);

#define MCFG_WD2010_OUT_RWC_CB(_devcb) \
	devcb = &wd2010_device::set_out_rwc_callback(*device, DEVCB_##_devcb);

#define MCFG_WD2010_OUT_WG_CB(_devcb) \
	devcb = &wd2010_device::set_out_wg_callback(*device, DEVCB_##_devcb);

#define MCFG_WD2010_IN_DRDY_CB(_devcb) \
	devcb = &wd2010_device::set_in_drdy_callback(*device, DEVCB_##_devcb);

#define MCFG_WD2010_IN_INDEX_CB(_devcb) \
	devcb = &wd2010_device::set_in_index_callback(*device, DEVCB_##_devcb);

#define MCFG_WD2010_IN_WF_CB(_devcb) \
	devcb = &wd2010_device::set_in_wf_callback(*device, DEVCB_##_devcb);

#define MCFG_WD2010_IN_TK000_CB(_devcb) \
	devcb = &wd2010_device::set_in_tk000_callback(*device, DEVCB_##_devcb);

#define MCFG_WD2010_IN_SC_CB(_devcb) \
	devcb = &wd2010_device::set_in_sc_callback(*device, DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wd2010_device

class wd2010_device :   public device_t
{
public:
	// construction/destruction
	wd2010_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_out_intrq_callback(device_t &device, _Object object) { return downcast<wd2010_device &>(device).m_out_intrq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_bdrq_callback(device_t &device, _Object object) { return downcast<wd2010_device &>(device).m_out_bdrq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_bcr_callback(device_t &device, _Object object) { return downcast<wd2010_device &>(device).m_out_bcr_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_brdy_callback(device_t &device, _Object object) { return downcast<wd2010_device &>(device).m_in_brdy_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_bcs_callback(device_t &device, _Object object) { return downcast<wd2010_device &>(device).m_in_bcs_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_bcs_callback(device_t &device, _Object object) { return downcast<wd2010_device &>(device).m_out_bcs_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dirin_callback(device_t &device, _Object object) { return downcast<wd2010_device &>(device).m_out_dirin_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_step_callback(device_t &device, _Object object) { return downcast<wd2010_device &>(device).m_out_step_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_rwc_callback(device_t &device, _Object object) { return downcast<wd2010_device &>(device).m_out_rwc_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_wg_callback(device_t &device, _Object object) { return downcast<wd2010_device &>(device).m_out_wg_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_drdy_callback(device_t &device, _Object object) { return downcast<wd2010_device &>(device).m_in_drdy_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_index_callback(device_t &device, _Object object) { return downcast<wd2010_device &>(device).m_in_index_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_wf_callback(device_t &device, _Object object) { return downcast<wd2010_device &>(device).m_in_wf_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_tk000_callback(device_t &device, _Object object) { return downcast<wd2010_device &>(device).m_in_tk000_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_sc_callback(device_t &device, _Object object) { return downcast<wd2010_device &>(device).m_in_sc_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	void buffer_ready(bool state);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	void compute_correction(UINT8 data);
	void set_parameter(UINT8 data);
	void restore(UINT8 data);
	void seek(UINT8 data);
	void read_sector(UINT8 data);
	void write_sector(UINT8 data);
	void scan_id(UINT8 data);
	void update_sdh(UINT8 new_sector_size, UINT8 new_head, UINT16 new_cylinder, UINT8 new_sectornr);
	void auto_scan_id(UINT8 data);
	void format(UINT8 data);

	devcb_write_line    m_out_intrq_cb;
	devcb_write_line    m_out_bdrq_cb;
	devcb_write_line    m_out_bcr_cb;
	devcb_read8         m_in_bcs_cb;
	devcb_read_line     m_in_brdy_cb;
	devcb_write8        m_out_bcs_cb;
	devcb_write_line    m_out_dirin_cb;
	devcb_write_line    m_out_step_cb;
	devcb_write_line    m_out_rwc_cb;
	devcb_write_line    m_out_wg_cb;
	devcb_read_line     m_in_drdy_cb;
	devcb_read_line     m_in_index_cb;
	devcb_read_line     m_in_wf_cb;
	devcb_read_line     m_in_tk000_cb;
	devcb_read_line     m_in_sc_cb;

	UINT8 m_status;
	UINT8 m_error;
	UINT8 m_task_file[8];

	emu_timer   *cmd_timer;
	emu_timer   *complete_write_when_buffer_ready_high;
	emu_timer   *deassert_write_when_buffer_ready_low;
	emu_timer   *deassert_read_when_buffer_ready_high;

	void complete_write_sector(UINT8 status);
	void complete_cmd(UINT8 status);
	void complete_immediate(UINT8 status);

	bool is_buffer_ready;

	UINT32 m_present_cylinder; // Present Cylinder Position Register
};

// device type definition
extern const device_type WD2010;

#endif
