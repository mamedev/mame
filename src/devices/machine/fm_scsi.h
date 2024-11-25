// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * fm_scsi.h
 *
 * SCSI controller used in Fujitsu FMR-50, FMR-60, and FM-Towns
 *
 */

#ifndef MAME_MACHINE_FM_SCSI_H
#define MAME_MACHINE_FM_SCSI_H

#include "machine/legscsi.h"


class fmscsi_device : public legacy_scsi_host_adapter
{
public:
	// construction/destruction
	fmscsi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }
	auto drq_handler() { return m_drq_handler.bind(); }

	// any publically accessible interfaces needed for runtime
	uint8_t fmscsi_data_r(void);
	void fmscsi_data_w(uint8_t data);
	uint8_t fmscsi_status_r(void);
	void fmscsi_control_w(uint8_t data);
	uint8_t fmscsi_r(offs_t offset);
	void fmscsi_w(offs_t offset, uint8_t data);

	TIMER_CALLBACK_MEMBER(set_phase);
	int get_phase(void);
	void set_input_line(uint8_t line, uint8_t state);
	uint8_t get_input_line(uint8_t line);
	void set_output_line(uint8_t line, uint8_t state);
	uint8_t get_output_line(uint8_t line);

protected:
	// device-level overrides (none are required, but these are common)
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_transfer);

private:
	int get_scsi_cmd_len(uint8_t cbyte);
	void stop_transfer();

	devcb_write_line m_irq_handler;
	devcb_write_line m_drq_handler;

	uint8_t m_command[32];
	//uint8_t m_result[32];
	uint8_t m_command_index;
	int m_result_length;
	uint32_t m_result_index;
	uint8_t m_input_lines;
	uint8_t m_output_lines;
	uint8_t m_data;
	uint8_t m_last_id;
	uint8_t m_phase;
	uint8_t m_target;
	uint8_t m_buffer[512];
	emu_timer* m_transfer_timer;
	emu_timer* m_phase_timer;
};

DECLARE_DEVICE_TYPE(FMSCSI, fmscsi_device)

#endif // MAME_MACHINE_FM_SCSI_H
