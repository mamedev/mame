// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_SEGA_NAOMIG1_H
#define MAME_SEGA_NAOMIG1_H

#pragma once

#include "cpu/sh/sh4.h"

class naomi_g1_device : public device_t
{
public:
	enum {
		DMA_GDROM_IRQ
	};

	typedef delegate<void (uint32_t main_adr, void *dma_ptr, uint32_t length, uint32_t size, bool to_mainram)> dma_cb;

	auto irq_callback() { return irq_cb.bind(); }
	auto ext_irq_callback() { return ext_irq_cb.bind(); }
	auto reset_out_callback() { return reset_out_cb.bind(); }
	void set_dma_cb(dma_cb cb) { _dma_cb = cb; }

	void amap(address_map &map) ATTR_COLD;     // for range 0x005f7400-0x005f74ff
	virtual void submap(address_map &map) = 0; // for range 0x005f7000-0x005f70ff

	uint32_t sb_gdstar_r();   // 5f7404
	void sb_gdstar_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);  // 5f7404
	uint32_t sb_gdlen_r();    // 5f7408
	void sb_gdlen_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);   // 5f7408
	uint32_t sb_gddir_r();    // 5f740c
	void sb_gddir_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);   // 5f740c
	uint32_t sb_gden_r();     // 5f7414
	void sb_gden_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);    // 5f7414
	uint32_t sb_gdst_r();     // 5f7418
	void sb_gdst_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);    // 5f7418

	void sb_g1rrc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);   // 5f7480
	void sb_g1rwc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);   // 5f7484
	void sb_g1frc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);   // 5f7488
	void sb_g1fwc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);   // 5f748c
	void sb_g1crc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);   // 5f7490
	void sb_g1cwc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);   // 5f7494
	void sb_g1gdrc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);  // 5f74a0
	void sb_g1gdwc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);  // 5f74a4
	uint32_t sb_g1sysm_r(offs_t offset, uint32_t mem_mask = ~0);   // 5f74b0
	void sb_g1crdyc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0); // 5f74b4
	void sb_gdapro_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);  // 5f74b8

	uint32_t sb_gdstard_r(offs_t offset, uint32_t mem_mask = ~0);  // 5f74f4
	uint32_t sb_gdlend_r(offs_t offset, uint32_t mem_mask = ~0);   // 5f74f8

protected:
	naomi_g1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void dma_get_position(uint8_t *&base, uint32_t &limit, bool to_maincpu) = 0;
	virtual void dma_advance(uint32_t size) = 0;

	void set_ext_irq(int state) { ext_irq_cb(state); }
	void set_reset_out() { reset_out_cb(ASSERT_LINE); }

	TIMER_CALLBACK_MEMBER(trigger_gdrom_irq);

private:
	uint32_t gdstar, gdlen, gddir, gden, gdst;

	emu_timer *timer;
	devcb_write8 irq_cb;
	devcb_write_line ext_irq_cb;
	devcb_write_line reset_out_cb;
	dma_cb _dma_cb;

	void dma(void *dma_ptr, uint32_t main_adr, uint32_t size, bool to_mainram);
};

#endif // MAME_SEGA_NAOMIG1_H
