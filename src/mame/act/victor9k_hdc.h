// license:BSD-3-Clause
// copyright-holders:D. Donohoe
#ifndef MAME_ACT_VICTOR9K_HDC_H
#define MAME_ACT_VICTOR9K_HDC_H

#pragma once

#include "machine/nscsi_bus.h"

class victor_9000_hdc_device : public nscsi_device, public nscsi_slot_card_interface
{
public:
	victor_9000_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_handler() { return m_irq_handler.bind(); }

	auto dma_read() { return m_dma_r.bind(); }
	auto dma_write() { return m_dma_w.bind(); }

	// Memory mapped registers
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	virtual void scsi_ctrl_changed() override;

protected:
	victor_9000_hdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
private:
	/*
	 * Register Definitions
	 *
	 * 0x00: Control
	 *
	 *     0: DMA-ON Value (0: DMA Off, 1: DMA On)
	 *     1: Lockout Mode
	 *     2: DMA-ON Latch: if set, latch DMA-ON state from bit 0.
	 *     3: Write Mode: if set, DMA direction is controller -> memory
	 *     4: Target Selection
	 *     5: Controller Reset
	 *
	 * 0x10: Data read/write (non-DMA accesses)
	 *
	 * 0x20: Status
	 *
	 *     0: I(/O)     0: Host -> Controller, 1: Controller -> Host
	 *     1: C(/D)     0: Data, 1: Command
	 *     2: BSY       1 = Busy
	 *     3: REQ       1 = Request
	 *     4: MSG       1 = Message
	 *
	 * 0x80: DMA address Low Byte
	 * 0xa0: DMA address Middle Byte
	 * 0xc0: DMA address High Byte
	 */
	enum {
		// Register address offsets
		R_CONTROL		= 0x00,
		R_DATA			= 0x10,
		R_STATUS		= 0x20,
		R_ADDR_L		= 0x80,
		R_ADDR_M		= 0xa0,
		R_ADDR_H		= 0xc0,

		// Control register bit fields
		R_C_DMAON_VALUE	= 0x01,
		R_C_LOCKOUT		= 0x02,
		R_C_DMAON_LATCH	= 0x04,
		R_C_WMODE		= 0x08,
		R_C_SELECT		= 0x10,
		R_C_RESET		= 0x20,

		// Status register bit fields
		R_S_INP			= 0x01,
		R_S_CMD			= 0x02,
		R_S_BSY			= 0x04,
		R_S_REQ			= 0x08,
		R_S_MSG			= 0x10,
	};

	TIMER_CALLBACK_MEMBER(ctrl_change_handler);

	void update_ints(bool status);

	devcb_read8 m_dma_r;
	devcb_write8 m_dma_w;

	devcb_write_line m_irq_handler;

	emu_timer *m_ctrl_timer;

	bool m_dma_on;
	bool m_dma_write;

	uint32_t m_dma_addr;

	bool m_asserting_ack;
	bool m_non_dma_req;

	uint8_t m_ctrl;
	uint8_t m_data;

	uint32_t m_bus_ctrl;

	bool m_irq_state;
};

DECLARE_DEVICE_TYPE(VICTOR_9000_HDC, victor_9000_hdc_device)

#endif // MAME_ACT_VICTOR9K_HDC_H
