// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_MACHINE_53C810_H
#define MAME_MACHINE_53C810_H

#pragma once

#include "legscsi.h"

class lsi53c810_device : public legacy_scsi_host_adapter
{
public:
	typedef device_delegate<void (int state)> irq_delegate;
	typedef device_delegate<void (uint32_t src, uint32_t dst, int length, int byteswap)> dma_delegate;
	typedef device_delegate<uint32_t (uint32_t dsp)> fetch_delegate;

	// construction/destruction
	lsi53c810_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename... T> void set_irq_callback(T &&... args)
	{
		m_irq_cb.set(std::forward<T>(args)...);
	}

	template <typename... T> void set_dma_callback(T &&... args)
	{
		m_dma_cb.set(std::forward<T>(args)...);
	}

	template <typename... T> void set_fetch_callback(T &&... args)
	{
		m_fetch_cb.set(std::forward<T>(args)...);
	}

	uint8_t reg_r(int offset);
	void reg_w(int offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	typedef delegate<void ()> opcode_handler_delegate;
	opcode_handler_delegate dma_opcode[256];

	irq_delegate m_irq_cb;
	dma_delegate m_dma_cb;
	fetch_delegate m_fetch_cb;

	uint32_t FETCH();
	void dmaop_invalid();
	void dmaop_move_memory();
	void dmaop_interrupt();
	void dmaop_block_move();
	void dmaop_select();
	void dmaop_wait_disconnect();
	void dmaop_wait_reselect();
	void dmaop_set();
	void dmaop_clear();
	void dmaop_move_from_sfbr();
	void dmaop_move_to_sfbr();
	void dmaop_read_modify_write();
	int scripts_compute_branch();
	uint32_t scripts_get_jump_dest();
	void dmaop_jump();
	void dmaop_call();
	void dmaop_return();
	void dmaop_store();
	void dmaop_load();
	void dma_exec();
	void add_opcode(uint8_t op, uint8_t mask, opcode_handler_delegate handler);
	uint32_t lsi53c810_dasm_fetch(uint32_t pc);
	unsigned lsi53c810_dasm(char *buf, uint32_t pc);

	uint8_t scntl0 = 0;
	uint8_t scntl1 = 0;
	uint8_t scntl2 = 0;
	uint8_t scntl3 = 0;
	uint8_t scid = 0;
	uint8_t sxfer = 0;
	uint8_t socl = 0;
	uint8_t istat = 0;
	uint8_t dstat = 0;
	uint8_t sstat0 = 0;
	uint8_t sstat1 = 0;
	uint8_t sstat2 = 0;
	uint8_t dien = 0;
	uint8_t dcntl = 0;
	uint8_t dmode = 0;
	uint32_t temp = 0;
	uint32_t dsa = 0;
	uint32_t dsp = 0;
	uint32_t dsps = 0;
	uint32_t dcmd = 0;
	uint8_t sien0 = 0;
	uint8_t sien1 = 0;
	uint8_t stime0 = 0;
	uint8_t respid = 0;
	uint8_t stest1 = 0;
	uint8_t scratch_a[4]{};
	uint8_t scratch_b[4]{};
	int dma_icount = 0;
	int halted = 0;
	int carry = 0;
};

// device type definition
DECLARE_DEVICE_TYPE(LSI53C810, lsi53c810_device)

#endif // MAME_MACHINE_53C810_H
