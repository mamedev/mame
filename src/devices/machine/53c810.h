// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_MACHINE_53C810_H
#define MAME_MACHINE_53C810_H

#pragma once

#include "legscsi.h"

#define LSI53C810_IRQ_CB(name)  void name(int state)
#define LSI53C810_DMA_CB(name)  void name(uint32_t src, uint32_t dst, int length, int byteswap)
#define LSI53C810_FETCH_CB(name)  uint32_t name(uint32_t dsp)


class lsi53c810_device : public legacy_scsi_host_adapter
{
public:
	typedef device_delegate<void (int state)> irq_delegate;
	typedef device_delegate<void (uint32_t src, uint32_t dst, int length, int byteswap)> dma_delegate;
	typedef device_delegate<uint32_t (uint32_t dsp)> fetch_delegate;

	// construction/destruction
	lsi53c810_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename Object> void set_irq_callback(Object &&cb) { m_irq_cb = std::forward<Object>(cb); }
	template <typename Object> void set_dma_callback(Object &&cb) { m_dma_cb = std::forward<Object>(cb); }
	template <typename Object> void set_fetch_callback(Object &&cb) { m_fetch_cb = std::forward<Object>(cb); }

	uint8_t reg_r(int offset);
	void reg_w(int offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	typedef delegate<void (void)> opcode_handler_delegate;
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

	uint8_t scntl0;
	uint8_t scntl1;
	uint8_t scntl2;
	uint8_t scntl3;
	uint8_t scid;
	uint8_t sxfer;
	uint8_t socl;
	uint8_t istat;
	uint8_t dstat;
	uint8_t sstat0;
	uint8_t sstat1;
	uint8_t sstat2;
	uint8_t dien;
	uint8_t dcntl;
	uint8_t dmode;
	uint32_t temp;
	uint32_t dsa;
	uint32_t dsp;
	uint32_t dsps;
	uint32_t dcmd;
	uint8_t sien0;
	uint8_t sien1;
	uint8_t stime0;
	uint8_t respid;
	uint8_t stest1;
	uint8_t scratch_a[4];
	uint8_t scratch_b[4];
	int dma_icount;
	int halted;
	int carry;
};

// device type definition
DECLARE_DEVICE_TYPE(LSI53C810, lsi53c810_device)


#define MCFG_LSI53C810_IRQ_CB(_class, _method) \
	downcast<lsi53c810_device &>(*device).set_irq_callback(lsi53c810_device::irq_delegate(&_class::_method, #_class "::" #_method, this));

#define MCFG_LSI53C810_DMA_CB(_class, _method) \
	downcast<lsi53c810_device &>(*device).set_dma_callback(lsi53c810_device::dma_delegate(&_class::_method, #_class "::" #_method, this));

#define MCFG_LSI53C810_FETCH_CB(_class, _method) \
	downcast<lsi53c810_device &>(*device).set_fetch_callback(lsi53c810_device::fetch_delegate(&_class::_method, #_class "::" #_method, this));

#endif // MAME_MACHINE_53C810_H
