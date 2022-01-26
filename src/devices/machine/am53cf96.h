// license:BSD-3-Clause
// copyright-holders:smf
/*
 * am53cf96.h
 *
 */

#ifndef MAME_MACHINE_AM53CF96_H
#define MAME_MACHINE_AM53CF96_H

#pragma once

#include "legscsi.h"

class am53cf96_device : public legacy_scsi_host_adapter
{
public:
	// construction/destruction
	am53cf96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_handler() { return m_irq_handler.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void dma_read_data(int bytes, uint8_t *pData);
	void dma_write_data(int bytes, uint8_t *pData);

protected:
	// 53CF96 register set
	enum
	{
		REG_XFERCNTLOW = 0, // read = current xfer count lo byte, write = set xfer count lo byte
		REG_XFERCNTMID,     // read = current xfer count mid byte, write = set xfer count mid byte
		REG_FIFO,           // read/write = FIFO
		REG_COMMAND,        // read/write = command

		REG_STATUS,         // read = status, write = destination SCSI ID (4)
		REG_IRQSTATE,       // read = IRQ status, write = timeout         (5)
		REG_INTSTATE,       // read = internal state, write = sync xfer period (6)
		REG_FIFOSTATE,      // read = FIFO status, write = sync offset
		REG_CTRL1,          // read/write = control 1
		REG_CLOCKFCTR,      // clock factor (write only)
		REG_TESTMODE,       // test mode (write only)
		REG_CTRL2,          // read/write = control 2
		REG_CTRL3,          // read/write = control 3
		REG_CTRL4,          // read/write = control 4
		REG_XFERCNTHI,      // read = current xfer count hi byte, write = set xfer count hi byte
		REG_DATAALIGN       // data alignment (write only)
	};

	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

private:
	static constexpr device_timer_id TIMER_TRANSFER = 0;

	uint8_t scsi_regs[32];
	uint8_t fifo[16];
	uint8_t fptr;
	uint8_t xfer_state;
	uint8_t last_id;

	emu_timer* m_transfer_timer;
	devcb_write_line m_irq_handler;
};

// device type definition
DECLARE_DEVICE_TYPE(AM53CF96, am53cf96_device)

#endif // MAME_MACHINE_AM53CF96_H
