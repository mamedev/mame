// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_SEGA_MAPLE_DC_H
#define MAME_SEGA_MAPLE_DC_H

#pragma once

#include "cpu/sh/sh4.h"

class maple_device;

class maple_dc_device : public device_t
{
public:
	enum {
		DMA_MAPLE_IRQ
	};

	template <typename T>
	maple_dc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: maple_dc_device(mconfig, tag, owner, clock)
	{
		set_maincpu_tag(std::forward<T>(cpu_tag));
	}

	maple_dc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template <typename T> void set_maincpu_tag(T &&cpu_tag) { cpu.set_tag(std::forward<T>(cpu_tag)); }
	auto irq_callback() { return irq_cb.bind(); }

	uint32_t sb_mdstar_r();  // 5f6c04
	void sb_mdstar_w(uint32_t data);
	uint32_t sb_mdtsel_r();    // 5f6c10
	void sb_mdtsel_w(uint32_t data);
	uint32_t sb_mden_r();    // 5f6c14
	void sb_mden_w(uint32_t data);
	uint32_t sb_mdst_r();    // 5f6c18
	void sb_mdst_w(uint32_t data);
	uint32_t sb_msys_r();    // 5f6c80
	void sb_msys_w(uint32_t data);
	void sb_mdapro_w(uint32_t data); // 5f6c8c

	void amap(address_map &map) ATTR_COLD;

	void end_of_reply();
	void register_port(int port, maple_device *device);
	void maple_hw_trigger();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(dma_timer_tick);

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

	required_device<sh4_device> cpu;
	emu_timer *timer;

	uint32_t mdstar, mden, mdst, msys;
	uint32_t mdtsel;

	uint32_t dma_state, dma_adr, dma_port, dma_dest;
	bool dma_endflag;
	devcb_write8 irq_cb;

	void dma_step();
};

DECLARE_DEVICE_TYPE(MAPLE_DC, maple_dc_device)

#endif // MAME_SEGA_MAPLE_DC_H
