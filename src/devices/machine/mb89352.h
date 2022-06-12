// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * mb89352.h
 *
 *  Created on: 16/01/2011
 */

#ifndef MAME_MACHINE_MB89352_H
#define MAME_MACHINE_MB89352_H

#include "legscsi.h"

class mb89352_device : public legacy_scsi_host_adapter
{
public:
	// construction/destruction
	mb89352_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto irq_cb() { return m_irq_cb.bind(); }
	auto drq_cb() { return m_drq_cb.bind(); }

	// any publically accessible interfaces needed for runtime
	uint8_t mb89352_r(offs_t offset);
	void mb89352_w(offs_t offset, uint8_t data);

	void set_phase(int phase);

protected:
	// device-level overrides (none are required, but these are common)
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_stop() override;

	TIMER_CALLBACK_MEMBER(transfer_tick);

private:
	int get_scsi_cmd_len(uint8_t cbyte);
	//void set_ints(uint8_t flag);

	devcb_write_line m_irq_cb;  /* irq callback */
	devcb_write_line m_drq_cb;  /* drq callback */

	uint8_t m_phase;  // current SCSI phase
	uint8_t m_target; // current SCSI target
	uint8_t m_bdid;  // Bus device ID (SCSI ID of the bus?)
	uint8_t m_ints;  // Interrupt Sense
	uint8_t m_temp;  // Temporary register (To/From SCSI bus)
	uint8_t m_data;  // Data register
	uint8_t m_scmd;  // SPC Command register
	uint32_t m_transfer_count;  // byte transfer counter, also used as a timeout counter for selection.
	uint8_t m_int_enable;
	uint8_t m_sel_enable;
	uint8_t m_resel_enable;
	uint8_t m_parity_enable;
	uint8_t m_arbit_enable;
	uint8_t m_busfree_int_enable;
	uint8_t m_line_status;
	uint8_t m_spc_status;
	uint8_t m_error_status;
	uint8_t m_command_index;
	uint8_t m_command[16];
	uint32_t m_transfer_index;
	uint8_t m_buffer[512];

	emu_timer* m_transfer_timer;
};

DECLARE_DEVICE_TYPE(MB89352A, mb89352_device)

#endif // MAME_MACHINE_MB89352_H
