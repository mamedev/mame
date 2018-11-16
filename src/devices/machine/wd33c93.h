// license:BSD-3-Clause
// copyright-holders:ElSemi, R. Belmont, Ryan Holtz
/*
 * wd33c93.h
 *
 */

#ifndef MAME_MACHINE_WD33C93_H
#define MAME_MACHINE_WD33C93_H

#pragma once

#include "legscsi.h"

class wd33c93_device : public legacy_scsi_host_adapter
{
public:
	// construction/destruction
	wd33c93_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: wd33c93_device(mconfig, tag, owner, (uint32_t)0)
	{
	}

	wd33c93_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_cb() { return m_irq_cb.bind(); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	int dma_read_data(int bytes, uint8_t *data);
	void dma_write_data(int bytes, uint8_t *data);
	void clear_dma();
	int get_dma_count();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// wd register names
	enum
	{
		WD_OWN_ID               = 0x00,
		WD_CONTROL              = 0x01,
		WD_TIMEOUT_PERIOD       = 0x02,
		WD_CDB_1                = 0x03,
		WD_CDB_2                = 0x04,
		WD_CDB_3                = 0x05,
		WD_CDB_4                = 0x06,
		WD_CDB_5                = 0x07,
		WD_CDB_6                = 0x08,
		WD_CDB_7                = 0x09,
		WD_CDB_8                = 0x0a,
		WD_CDB_9                = 0x0b,
		WD_CDB_10               = 0x0c,
		WD_CDB_11               = 0x0d,
		WD_CDB_12               = 0x0e,
		WD_TARGET_LUN           = 0x0f,
		WD_COMMAND_PHASE        = 0x10,
		WD_SYNCHRONOUS_TRANSFER = 0x11,
		WD_TRANSFER_COUNT_MSB   = 0x12,
		WD_TRANSFER_COUNT       = 0x13,
		WD_TRANSFER_COUNT_LSB   = 0x14,
		WD_DESTINATION_ID       = 0x15,
		WD_SOURCE_ID            = 0x16,
		WD_SCSI_STATUS          = 0x17,
		WD_COMMAND              = 0x18,
		WD_DATA                 = 0x19,
		WD_QUEUE_TAG            = 0x1a,
		WD_AUXILIARY_STATUS     = 0x1f
	};

	static constexpr int TEMP_INPUT_LEN  = 262144;
	static constexpr int FIFO_SIZE       = 12;

	uint8_t getunit();
	void set_xfer_count(int count);
	int get_xfer_count();
	void complete_immediate(int status);
	void complete_cmd(uint8_t status);
	void unimplemented_cmd();
	void invalid_cmd();
	void reset_cmd();
	void abort_cmd();
	void disconnect_cmd();
	void select_cmd();
	void selectxfer_cmd();
	void negate_ack();
	void xferinfo_cmd();
	void dispatch_command();

	uint8_t		m_sasr;
	uint8_t		m_regs[WD_AUXILIARY_STATUS+1];
	uint8_t		m_fifo[FIFO_SIZE];
	int			m_fifo_pos;
	uint8_t		m_temp_input[TEMP_INPUT_LEN];
	int			m_temp_input_pos;
	uint8_t		m_busphase;
	uint8_t		m_identify;
	bool		m_read_pending;
	emu_timer	*m_cmd_timer;
	emu_timer	*m_service_req_timer;
	emu_timer	*m_deassert_cip_timer;
	devcb_write_line m_irq_cb; /* irq callback */
};

// device type definition
DECLARE_DEVICE_TYPE(WD33C93, wd33c93_device)

#endif // MAME_MACHINE_WD33C93_H
