// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_NAOMIG1_H
#define MAME_MACHINE_NAOMIG1_H

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
	void set_dma_cb(dma_cb cb) { _dma_cb = cb; }

	void amap(address_map &map);
	virtual void submap(address_map &map) = 0;

	DECLARE_READ32_MEMBER(sb_gdstar_r);   // 5f7404
	DECLARE_WRITE32_MEMBER(sb_gdstar_w);  // 5f7404
	DECLARE_READ32_MEMBER(sb_gdlen_r);    // 5f7408
	DECLARE_WRITE32_MEMBER(sb_gdlen_w);   // 5f7408
	DECLARE_READ32_MEMBER(sb_gddir_r);    // 5f740c
	DECLARE_WRITE32_MEMBER(sb_gddir_w);   // 5f740c
	DECLARE_READ32_MEMBER(sb_gden_r);     // 5f7414
	DECLARE_WRITE32_MEMBER(sb_gden_w);    // 5f7414
	DECLARE_READ32_MEMBER(sb_gdst_r);     // 5f7418
	DECLARE_WRITE32_MEMBER(sb_gdst_w);    // 5f7418

	DECLARE_WRITE32_MEMBER(sb_g1rrc_w);   // 5f7480
	DECLARE_WRITE32_MEMBER(sb_g1rwc_w);   // 5f7484
	DECLARE_WRITE32_MEMBER(sb_g1frc_w);   // 5f7488
	DECLARE_WRITE32_MEMBER(sb_g1fwc_w);   // 5f748c
	DECLARE_WRITE32_MEMBER(sb_g1crc_w);   // 5f7490
	DECLARE_WRITE32_MEMBER(sb_g1cwc_w);   // 5f7494
	DECLARE_WRITE32_MEMBER(sb_g1gdrc_w);  // 5f74a0
	DECLARE_WRITE32_MEMBER(sb_g1gdwc_w);  // 5f74a4
	DECLARE_READ32_MEMBER(sb_g1sysm_r);   // 5f74b0
	DECLARE_WRITE32_MEMBER(sb_g1crdyc_w); // 5f74b4
	DECLARE_WRITE32_MEMBER(sb_gdapro_w);  // 5f74b8

	DECLARE_READ32_MEMBER(sb_gdstard_r);  // 5f74f4
	DECLARE_READ32_MEMBER(sb_gdlend_r);   // 5f74f8

protected:
	enum { G1_TIMER_ID = 0x42 };

	naomi_g1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void dma_get_position(uint8_t *&base, uint32_t &limit, bool to_maincpu) = 0;
	virtual void dma_advance(uint32_t size) = 0;

private:
	uint32_t gdstar, gdlen, gddir, gden, gdst;

	emu_timer *timer;
	devcb_write8 irq_cb;
	dma_cb _dma_cb;

	void dma(void *dma_ptr, uint32_t main_adr, uint32_t size, bool to_mainram);
};

#endif // MAME_MACHINE_NAOMIG1_H
