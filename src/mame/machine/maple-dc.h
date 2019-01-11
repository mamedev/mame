// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_MAPLE_DC_H
#define MAME_MACHINE_MAPLE_DC_H

#pragma once

#include "cpu/sh/sh4.h"

#define MCFG_MAPLE_DC_ADD(_tag, _maincpu_tag, _irq_cb)  \
	MCFG_DEVICE_ADD(_tag, MAPLE_DC, 0) \
	downcast<maple_dc_device &>(*device).set_maincpu_tag(_maincpu_tag); \
	downcast<maple_dc_device &>(*device).set_irq_cb(_irq_cb);

class maple_device;

class maple_dc_device : public device_t
{
public:
	maple_dc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void set_maincpu_tag(const char *new_maincpu_tag) { maincpu_tag = new_maincpu_tag; }
	void set_irq_cb(void (*new_irq_cb)(running_machine &)) { irq_cb = new_irq_cb; }

	DECLARE_READ32_MEMBER(sb_mdstar_r);  // 5f6c04
	DECLARE_WRITE32_MEMBER(sb_mdstar_w);
	DECLARE_READ32_MEMBER(sb_mdtsel_r);    // 5f6c10
	DECLARE_WRITE32_MEMBER(sb_mdtsel_w);
	DECLARE_READ32_MEMBER(sb_mden_r);    // 5f6c14
	DECLARE_WRITE32_MEMBER(sb_mden_w);
	DECLARE_READ32_MEMBER(sb_mdst_r);    // 5f6c18
	DECLARE_WRITE32_MEMBER(sb_mdst_w);
	DECLARE_READ32_MEMBER(sb_msys_r);    // 5f6c80
	DECLARE_WRITE32_MEMBER(sb_msys_w);
	DECLARE_WRITE32_MEMBER(sb_mdapro_w); // 5f6c8c

	void amap(address_map &map);

	void end_of_reply();
	void register_port(int port, maple_device *device);
	void maple_hw_trigger();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	enum {
		DMA_IDLE,
		DMA_DONE,
		DMA_SEND,
		DMA_WAIT_NOP,
		DMA_WAIT_RESET,
		DMA_WAIT_REPLY,
		DMA_GOT_REPLY,
		DMA_TIMEOUT
	};

	maple_device *devices[4];

	sh4_device *cpu;
	emu_timer *timer;

	uint32_t mdstar, mden, mdst, msys;
	uint32_t mdtsel;

	uint32_t dma_state, dma_adr, dma_port, dma_dest;
	bool dma_endflag;
	void (*irq_cb)(running_machine &);

	void dma_step();

	const char *maincpu_tag;
};

DECLARE_DEVICE_TYPE(MAPLE_DC, maple_dc_device)

#endif // MAME_MACHINE_MAPLE_DC_H
