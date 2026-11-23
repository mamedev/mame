// license:BSD-3-Clause
// copyright-holders:Chris Toshok
/*
 * apollo_dn300_disk.h - the Apollo DN300/DN320 Winchester/floppy controller
 */

#ifndef MAME_APOLLO_APOLLO_DN300_DISK_H
#define MAME_APOLLO_APOLLO_DN300_DISK_H

#pragma once

#include "machine/ansi_disk.h"

#include "imagedev/floppy.h"
#include "machine/msm5832.h"
#include "machine/upd765.h"

class apollo_dn300_disk_ctrlr_device :
	public device_t
{
public:
	//construction/destruction
	apollo_dn300_disk_ctrlr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_callback() { return irq_cb.bind(); }
	auto drq_wr_callback() { return drq_cb.bind(); }

	uint8_t dma_read_byte(offs_t offset);
	void dma_write_byte(offs_t offset, uint8_t data);

	void map(address_map &map);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	required_device_array<ansi_disk_device, 2> our_disks;

private:
	void end_of_controller_op();

	// controller-level irq/drq signals
	devcb_write_line irq_cb;
	devcb_write_line drq_cb;

	// WDC-specific
	void wdc_write(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	uint8_t wdc_read(offs_t offset, uint8_t mem_mask = ~0);

	uint8_t m_wdc_selected_drive;

	// write registers
	uint8_t m_wdc_ansi_cmd;
	uint8_t m_wdc_ansi_parm_out;
	uint8_t m_wdc_sector;
	uint8_t m_wdc_cylinder_hi;
	uint8_t m_wdc_cylinder_lo;
	uint8_t m_wdc_head;
	uint8_t m_wdc_interrupt_control;
	uint8_t m_controller_command;

	// read registers
	uint8_t m_wdc_attention_status;
	uint8_t m_wdc_ansi_parm_in;
	uint8_t m_wdc_drive_num_of_status;
	uint8_t m_controller_status_high;
	uint8_t m_controller_status_low;

	void execute_command();

	// internal buffer to help with our simulated reading (we read a byte
	// at a time, but the dma is done a word at a time.)
	uint8_t m_bytes_read;
	char m_buffer[2];

	int m_word_transfer_count;

	// our ansi disks generate sector/index pulses, which we need to
	// listen for in order to tell when the head is over the correct spot
	// (to assert/deassert the gate lines and start a read/write operation.)
	void ansi_index_pulse(ansi_disk_device *);
	void ansi_sector_pulse(ansi_disk_device *);

	int m_pulsed_sector;

	bool m_start_read_sector;
	bool m_start_write_sector;
	void check_for_sector_read();
	void check_for_sector_write();

	// callbacks representing the other ANSI disk signal lines
	void ansi_disk_attention(ansi_disk_device *disk, bool state);
	void ansi_disk_busy(ansi_disk_device *disk, bool state);
	void ansi_disk_ref_clock_tick(ansi_disk_device *disk);
	bool m_handling_attn;

	// for our simulated reading, we consume a byte at a time, instead of doing the NRZ
	// clocked reading that the real hardware does.  This function is called by the ansi_disk_device
	// for each byte of data in a sector via a timer tick, to make everything async.  The timer
	// is ticked to roughly how fast the real hardware would be reading the data, so... maybe that's
	// good enough?
	void ansi_disk_read_data(ansi_disk_device *disk, uint8_t data);


	// FDC-specific
	required_device<upd765a_device> m_fdc;
	required_device<floppy_connector> m_floppy;
	void fdc_irq(int state);

	// the controller card's own floppy registers beside the NEC 765A
	void fdc_control_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t fdc_msr_r(offs_t, uint8_t mem_mask);

	bool m_floppy_drq_state;

	// write registers
	uint8_t m_fdc_control;

	// Calendar-specific
	required_device<msm5832_device> m_rtc;

	// the controller card's interface to the OKI MSM5832
	void calendar_ctrl_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	void calendar_data_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	uint8_t calendar_data_r(offs_t offset, uint8_t mem_mask = ~0);

	// write registers
	uint8_t m_calendar_ctrl;
};

// device type definition
DECLARE_DEVICE_TYPE(APOLLO_DN300_DISK_CTRLR, apollo_dn300_disk_ctrlr_device)

#endif // MAME_APOLLO_APOLLO_DN300_DISK_H
