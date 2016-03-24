// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef _NAOMIG1_H_
#define _NAOMIG1_H_

#include "cpu/sh4/sh4.h"

#define MCFG_NAOMI_G1_ADD(_tag, type, _irq_cb)                          \
	MCFG_DEVICE_ADD(_tag, type, 0)                                      \
	downcast<naomi_g1_device *>(device)->set_irq_cb(DEVCB_ ## _irq_cb);

class naomi_g1_device : public device_t
{
public:
	enum {
		DMA_GDROM_IRQ
	};

	typedef delegate<void (UINT32 main_adr, void *dma_ptr, UINT32 length, UINT32 size, bool to_mainram)> dma_cb;

	naomi_g1_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	template<class _cb> void set_irq_cb(_cb cb) { irq_cb.set_callback(cb); }
	void set_dma_cb(dma_cb _cb) { _dma_cb = _cb; }

	DECLARE_ADDRESS_MAP(amap, 32);

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

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void dma_get_position(UINT8 *&base, UINT32 &limit, bool to_maincpu) = 0;
	virtual void dma_advance(UINT32 size) = 0;

private:
	UINT32 gdstar, gdlen, gddir, gden, gdst;

	emu_timer *timer;
	devcb_write8 irq_cb;
	dma_cb _dma_cb;

	void dma(void *dma_ptr, UINT32 main_adr, UINT32 size, bool to_mainram);
};

#endif
