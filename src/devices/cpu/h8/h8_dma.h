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
	uint32_t source, dest;
	int32_t incs, incd;
	uint32_t count;
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
	h8_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t dmawer_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dmawer_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t dmatcr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dmatcr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t dmabcr_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dmabcr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	bool trigger_dma(int vector);
	void count_done(int id);
	void clear_dte(int id);

protected:
	required_device<h8_dma_channel_device> dmach0, dmach1;

	virtual void device_start() override;
	virtual void device_reset() override;

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

	void set_info(const char *intc, int irq_base, int v0, int v1, int v2, int v3, int v4, int v5, int v6, int v7, int v8, int v9, int va, int vb, int vc, int vd, int ve, int vf);

	uint16_t marah_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void marah_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t maral_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void maral_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t ioara_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void ioara_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t etcra_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void etcra_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t marbh_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void marbh_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t marbl_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void marbl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t ioarb_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void ioarb_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t etcrb_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void etcrb_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dmacr_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dmacr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void set_id(int id);
	void set_bcr(bool fae, bool sae, uint8_t dta, uint8_t dte, uint8_t dtie);
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

	uint32_t mar[2];
	uint16_t ioar[2], etcr[2], dmacr;
	uint8_t dta, dte, dtie;
	bool fae, sae;

	virtual void device_start() override;
	virtual void device_reset() override;

	void start(int submodule);
};

extern const device_type H8_DMA;
extern const device_type H8_DMA_CHANNEL;

#endif
