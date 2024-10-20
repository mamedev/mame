// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Western Digital WD2010 Winchester Disk Controller

**********************************************************************/

#ifndef MAME_MACHINE_WD2010_H
#define MAME_MACHINE_WD2010_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wd2010_device

class wd2010_device :   public device_t
{
public:
	// construction/destruction
	wd2010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_intrq_callback() { return m_out_intrq_cb.bind(); }
	auto out_bdrq_callback() { return m_out_bdrq_cb.bind(); }
	auto out_bcr_callback() { return m_out_bcr_cb.bind(); }
	auto in_brdy_callback() { return m_in_brdy_cb.bind(); }
	auto in_bcs_callback() { return m_in_bcs_cb.bind(); }
	auto out_bcs_callback() { return m_out_bcs_cb.bind(); }
	auto out_dirin_callback() { return m_out_dirin_cb.bind(); }
	auto out_step_callback() { return m_out_step_cb.bind(); }
	auto out_rwc_callback() { return m_out_rwc_cb.bind(); }
	auto out_wg_callback() { return m_out_wg_cb.bind(); }
	auto in_drdy_callback() { return m_in_drdy_cb.bind(); }
	auto in_index_callback() { return m_in_index_cb.bind(); }
	auto in_wf_callback() { return m_in_wf_cb.bind(); }
	auto in_tk000_callback() { return m_in_tk000_cb.bind(); }
	auto in_sc_callback() { return m_in_sc_cb.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void buffer_ready(bool state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(command_complete);
	TIMER_CALLBACK_MEMBER(complete_write);
	TIMER_CALLBACK_MEMBER(deassert_write);
	TIMER_CALLBACK_MEMBER(deassert_read);
	TIMER_CALLBACK_MEMBER(next_sector);

private:
	void compute_correction(uint8_t data);
	void set_parameter(uint8_t data);
	void restore(uint8_t data);
	void seek(uint8_t data);
	void read_sector(uint8_t data);
	void write_sector(uint8_t data);
	void scan_id(uint8_t data);
	void update_sdh(uint8_t new_sector_size, uint8_t new_head, uint16_t new_cylinder, uint8_t new_sectornr);
	void auto_scan_id(uint8_t data);
	void format(uint8_t data);

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

	uint8_t m_status;
	uint8_t m_error;
	uint8_t m_task_file[8];

	emu_timer   *m_cmd_timer;
	emu_timer   *m_complete_write_timer;
	emu_timer   *m_deassert_write_timer;
	emu_timer   *m_deassert_read_timer;
	emu_timer   *m_next_sector_timer;

	void complete_write_sector(uint8_t status);
	void complete_cmd(uint8_t status);
	void complete_immediate(uint8_t status);

	bool is_buffer_ready;

	uint32_t m_present_cylinder; // Present Cylinder Position Register
};

// device type definition
DECLARE_DEVICE_TYPE(WD2010, wd2010_device)

#endif // MAME_MACHINE_WD2010_H
