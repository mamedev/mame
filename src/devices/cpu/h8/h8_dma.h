// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_dma.h

    H8 DMA Controller

***************************************************************************/

#ifndef __H8_DMA_H__
#define __H8_DMA_H__

#include "h8.h"
#include "h8_intc.h"

struct h8_dma_state {
	UINT32 source, dest;
	INT32 incs, incd;
	UINT32 count;
	int id;
	bool mode_16;
};

#define MCFG_H8_DMA_ADD( _tag ) \
	MCFG_DEVICE_ADD( _tag, H8_DMA, 0 )

#define MCFG_H8_DMA_CHANNEL_ADD( _tag, intc, irq_base, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, va, vb, vc, vd, ve, vf ) \
	MCFG_DEVICE_ADD( _tag, H8_DMA_CHANNEL, 0 )  \
	downcast<h8_dma_channel_device *>(device)->set_info(intc, irq_base, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, va, vb, vc, vd, ve, vf);

class h8_dma_channel_device;

class h8_dma_device : public device_t {
public:
	h8_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(dmawer_r);
	DECLARE_WRITE8_MEMBER(dmawer_w);
	DECLARE_READ8_MEMBER(dmatcr_r);
	DECLARE_WRITE8_MEMBER(dmatcr_w);
	DECLARE_READ16_MEMBER(dmabcr_r);
	DECLARE_WRITE16_MEMBER(dmabcr_w);

	bool trigger_dma(int vector);
	void count_done(int id);
	void clear_dte(int id);

protected:
	required_device<h8_dma_channel_device> dmach0, dmach1;

	virtual void device_start() override;
	virtual void device_reset() override;

	UINT8 dmawer, dmatcr;
	UINT16 dmabcr;
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

	h8_dma_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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

	void set_id(int id);
	void set_bcr(bool fae, bool sae, UINT8 dta, UINT8 dte, UINT8 dtie);
	bool start_test(int vector);
	void count_done(int submodule);
protected:
	required_device<h8_dma_device> dmac;
	required_device<h8_device> cpu;
	h8_intc_device *intc;
	const char *intc_tag;
	h8_dma_state state[2];
	int irq_base;

	int activation_vectors[16];

	UINT32 mar[2];
	UINT16 ioar[2], etcr[2], dmacr;
	UINT8 dta, dte, dtie;
	bool fae, sae;

	virtual void device_start() override;
	virtual void device_reset() override;

	void start(int submodule);
};

extern const device_type H8_DMA;
extern const device_type H8_DMA_CHANNEL;

#endif
