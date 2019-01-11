// license:BSD-3-Clause
// copyright-holders:Tyson Smith
/*
 * wd33c9x.h
 */

#ifndef MAME_MACHINE_WD33C9X_H
#define MAME_MACHINE_WD33C9X_H

#pragma once

#include "machine/nscsi_bus.h"

class wd33c9x_base_device : public nscsi_device
{
public:
	auto irq_cb() { return m_irq_cb.bind(); }
	auto drq_cb() { return m_drq_cb.bind(); }

	// Direct Addressing Interface
	DECLARE_READ8_MEMBER(dir_r);
	DECLARE_WRITE8_MEMBER(dir_w);

	// Indirect Addressing Interface
	DECLARE_READ8_MEMBER(indir_r);
	DECLARE_WRITE8_MEMBER(indir_w);

	// Alternative Indirect Addressing Interface
	DECLARE_READ8_MEMBER(indir_addr_r);
	DECLARE_WRITE8_MEMBER(indir_addr_w);
	DECLARE_READ8_MEMBER(indir_reg_r);
	DECLARE_WRITE8_MEMBER(indir_reg_w);

	// Master Reset (MR) Interface
	DECLARE_WRITE_LINE_MEMBER(reset_w);

	// DMA Interface (for use with DRQ)
	uint8_t dma_r();
	void dma_w(const uint8_t data);

protected:
	wd33c9x_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void scsi_ctrl_changed() override;

private:
	static const char *const state_names[];
	static const char *const substate_names[];
	static constexpr uint8_t NUM_REGS = 0x20;
	static constexpr uint8_t REGS_MASK = NUM_REGS - 1;
	uint8_t m_addr;
	uint8_t m_regs[NUM_REGS];
	uint8_t m_command_length;
	uint8_t m_last_message;

	void start_command();

	void step(bool timeout);
	void set_scsi_state(uint16_t state);
	void set_scsi_state_sub(uint8_t sub);
	uint16_t m_scsi_state;
	uint8_t m_mode;
	uint8_t m_xfr_phase;

	void load_transfer_count();
	bool decrement_transfer_count();
	uint32_t m_transfer_count;

	uint8_t data_fifo_pop();
	void data_fifo_push(const uint8_t data);
	bool data_fifo_empty() const;
	bool data_fifo_full() const;
	void data_fifo_reset();
	static constexpr uint8_t DATA_FIFO_SIZE = 12;
	uint8_t m_data_fifo[DATA_FIFO_SIZE];
	uint8_t m_data_fifo_pos;
	uint8_t m_data_fifo_size;

	uint32_t send_byte(const uint32_t value = 0, const uint32_t mask = 0);

	uint8_t irq_fifo_pop();
	void irq_fifo_push(const uint8_t status);
	bool irq_fifo_empty() const;
	bool irq_fifo_full() const;
	void irq_fifo_reset();
	static constexpr uint8_t IRQ_FIFO_SIZE = 8;
	uint8_t m_irq_fifo[IRQ_FIFO_SIZE];
	uint8_t m_irq_fifo_pos;
	uint8_t m_irq_fifo_size;

	void update_irq();
	devcb_write_line m_irq_cb;

	void set_drq();
	void clear_drq();
	devcb_write_line m_drq_cb;
	bool m_drq_state;

	void delay(uint32_t cycles);
	void delay_cycles(uint32_t cycles);
	emu_timer *m_timer;

	bool set_command_length(const uint8_t cc);
	uint8_t get_msg_out() const;
};

class wd33c92_device : public wd33c9x_base_device
{
public:
	wd33c92_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: wd33c92_device(mconfig, tag, owner, 0)
	{
	}

	wd33c92_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class wd33c93n_device : public wd33c9x_base_device
{
public:
	wd33c93n_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: wd33c93n_device(mconfig, tag, owner, 0)
	{
	}

	wd33c93n_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class wd33c93a_device : public wd33c9x_base_device
{
public:
	wd33c93a_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: wd33c93a_device(mconfig, tag, owner, 0)
	{
	}

	wd33c93a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class wd33c93b_device : public wd33c9x_base_device
{
public:
	wd33c93b_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: wd33c93b_device(mconfig, tag, owner, 0)
	{
	}

	wd33c93b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(WD33C92, wd33c92_device)
DECLARE_DEVICE_TYPE(WD33C93N, wd33c93n_device)
DECLARE_DEVICE_TYPE(WD33C93A, wd33c93a_device)
DECLARE_DEVICE_TYPE(WD33C93B, wd33c93b_device)

#endif // MAME_MACHINE_WD33C9X_H
