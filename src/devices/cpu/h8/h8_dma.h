// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_dma.h

    H8 DMA Controller

***************************************************************************/

#ifndef MAME_CPU_H8_H8_DMA_H
#define MAME_CPU_H8_H8_DMA_H

#pragma once

#include "h8.h"
#include "h8_intc.h"

struct h8_dma_state {
	uint32_t source, dest;
	int32_t incs, incd;
	uint32_t count;
	int id;
	bool autoreq; // activate by auto-request
	bool suspended;
	bool mode_16;
};

class h8_dma_channel_device;

enum {
	// mind the order, all DREQ, TEND need to be sequential
	H8_INPUT_LINE_DREQ0 = INPUT_LINE_IRQ9 + 1,
	H8_INPUT_LINE_DREQ1,
	H8_INPUT_LINE_DREQ2,
	H8_INPUT_LINE_DREQ3,

	H8_INPUT_LINE_TEND0,
	H8_INPUT_LINE_TEND1,
	H8_INPUT_LINE_TEND2,
	H8_INPUT_LINE_TEND3,
};

class h8_dma_device : public device_t {
public:
	h8_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	DECLARE_READ8_MEMBER(dmawer_r);
	DECLARE_WRITE8_MEMBER(dmawer_w);
	DECLARE_READ8_MEMBER(dmatcr_r);
	DECLARE_WRITE8_MEMBER(dmatcr_w);
	DECLARE_READ16_MEMBER(dmabcr_r);
	DECLARE_WRITE16_MEMBER(dmabcr_w);

	bool trigger_dma(int vector);
	void count_last(int id);
	void count_done(int id);
	void clear_dte(int id);

	void set_input(int inputnum, int state);

protected:
	required_device<h8_dma_channel_device> dmach0, dmach1;

	virtual void device_start() override;
	virtual void device_reset() override;

	bool dreq[2];

	uint8_t dmawer, dmatcr;
	uint16_t dmabcr;
};

class h8_dma_channel_device : public device_t {
public:
	enum {
		NONE       = -1,
		DREQ_LEVEL = -2,
		DREQ_EDGE  = -3
	};

	enum {
		MODE8_MEM_MEM,
		MODE8_DACK_MEM,
		MODE8_MEM_DACK,
		MODE16_MEM_MEM,
		MODE16_DACK_MEM,
		MODE16_MEM_DACK
	};

	h8_dma_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	h8_dma_channel_device(const machine_config &mconfig, const char *tag, device_t *owner,
			const char *intc, int irq_base, int v0, int v1, int v2, int v3, int v4, int v5, int v6, int v7, int v8,
			int v9 = h8_dma_channel_device::NONE,
			int va = h8_dma_channel_device::NONE,
			int vb = h8_dma_channel_device::NONE,
			int vc = h8_dma_channel_device::NONE,
			int vd = h8_dma_channel_device::NONE,
			int ve = h8_dma_channel_device::NONE,
			int vf = h8_dma_channel_device::NONE)
		: h8_dma_channel_device(mconfig, tag, owner, 0)
	{
		set_info(intc, irq_base, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, va, vb, vc, vd, ve, vf);
	}
	void set_info(const char *intc, int irq_base, int v0, int v1, int v2, int v3, int v4, int v5, int v6, int v7, int v8, int v9, int va, int vb, int vc, int vd, int ve, int vf);

	DECLARE_READ16_MEMBER(marah_r);
	DECLARE_WRITE16_MEMBER(marah_w);
	DECLARE_READ16_MEMBER(maral_r);
	DECLARE_WRITE16_MEMBER(maral_w);
	DECLARE_READ16_MEMBER(ioara_r);
	DECLARE_WRITE16_MEMBER(ioara_w);
	DECLARE_READ16_MEMBER(etcra_r);
	DECLARE_WRITE16_MEMBER(etcra_w);
	DECLARE_READ16_MEMBER(marbh_r);
	DECLARE_WRITE16_MEMBER(marbh_w);
	DECLARE_READ16_MEMBER(marbl_r);
	DECLARE_WRITE16_MEMBER(marbl_w);
	DECLARE_READ16_MEMBER(ioarb_r);
	DECLARE_WRITE16_MEMBER(ioarb_w);
	DECLARE_READ16_MEMBER(etcrb_r);
	DECLARE_WRITE16_MEMBER(etcrb_w);
	DECLARE_READ16_MEMBER(dmacr_r);
	DECLARE_WRITE16_MEMBER(dmacr_w);

	// H8H DMA
	DECLARE_READ8_MEMBER(dtcra_r);
	DECLARE_WRITE8_MEMBER(dtcra_w);
	DECLARE_READ8_MEMBER(dtcrb_r);
	DECLARE_WRITE8_MEMBER(dtcrb_w);

	void set_id(int id);
	void set_bcr(bool fae, bool sae, uint8_t dta, uint8_t dte, uint8_t dtie);
	bool start_test(int vector);
	void count_last(int submodule);
	void count_done(int submodule);
protected:
	required_device<h8_dma_device> dmac;
	required_device<h8_device> cpu;
	h8_intc_device *intc;
	const char *intc_tag;
	h8_dma_state state[2];
	int irq_base;

	int activation_vectors[16];

	uint32_t mar[2];
	uint16_t ioar[2], etcr[2], dmacr;
	uint8_t dtcr[2]; // H8H
	uint8_t dta, dte, dtie;
	bool fae; // Full-Address Mode
	bool sae; // Short-Address Mode

	virtual void device_start() override;
	virtual void device_reset() override;

	void h8h_sync(); // call set_bcr with contents from DTCR
	void start(int submodule);
};

DECLARE_DEVICE_TYPE(H8_DMA,         h8_dma_device)
DECLARE_DEVICE_TYPE(H8_DMA_CHANNEL, h8_dma_channel_device)

#endif // MAME_CPU_H8_H8_DMA_H
