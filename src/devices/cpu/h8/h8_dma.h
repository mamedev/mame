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
	enum {
		ACTIVE                 = 0x0001, // DMA is configured
		SUSPENDED              = 0x0002, // DMA currently suspended until trigger happens
		SUSPEND_AFTER_TRANSFER = 0x0004, // Auto-suspend DMA after each transfer
		BLOCK                  = 0x0008, // FAE block mode (cleared on last block)
		REPEAT                 = 0x0010, // SAE repeat mode
		MODE_16                = 0x0020, // Transfer 16-bits values
		EAT_INTERRUPT          = 0x0040, // Discard interrupt when used as trigger
		TEND_INTERRUPT         = 0x0080, // Interrupt on end of transfer
		SOURCE_DECREMENT       = 0x0100, // Decrement source instead of increment (folded into incs/incd)
		DEST_DECREMENT         = 0x0200, // Decrement source instead of increment (folded into incs/incd)
		SOURCE_IDLE            = 0x0400, // Don't increment/decrement source (folded into incs/incd)
		DEST_IDLE              = 0x0800, // Don't increment/decrement destination (folded into incs/incd)
		MAR_IS_DEST            = 0x1000, // MAR is destination in SAE (folded), destibation is the block in fae block
		FAE                    = 0x2000, // FAE mode (for interrupt generation)
	};

	u32 m_source, m_dest;
	s32 m_incs, m_incd;
	u32 m_count, m_bcount;
	u16 m_flags;
	u8  m_id;
	s8  m_trigger_vector;
};

class h8gen_dma_channel_device;

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

class h8h_dma_device;
class h8s_dma_device;

DECLARE_DEVICE_TYPE(H8H_DMA,         h8h_dma_device)
DECLARE_DEVICE_TYPE(H8S_DMA,         h8s_dma_device)

class h8gen_dma_device : public device_t {
public:
	bool trigger_dma(int vector);
	void count_last(int id);
	void count_done(int id);

	void set_input(int inputnum, int state);
	void start_stop_test();

protected:
	required_device<h8_device> m_cpu;
	optional_device_array<h8gen_dma_channel_device, 4> m_dmach;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual u8 active_channels() const = 0;

	h8gen_dma_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock = 0);
};

class h8h_dma_device : public h8gen_dma_device
{
public:
	h8h_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template<typename T> h8h_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu)
		: h8h_dma_device(mconfig, tag, owner)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
	}

	u8 active_channels() const override;
};


class h8s_dma_device : public h8gen_dma_device
{
public:
	h8s_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template<typename T> h8s_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu)
		: h8s_dma_device(mconfig, tag, owner)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
	}

	u8 dmawer_r();
	void dmawer_w(u8 data);
	u8 dmatcr_r();
	void dmatcr_w(u8 data);
	u16 dmabcr_r();
	void dmabcr_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void channel_done(int id);
	int channel_mode(int id, bool block) const;
	std::tuple<bool, bool, bool> get_fae_dtie_dta(int id) const;

protected:
	u8 m_dmawer, m_dmatcr;
	u16 m_dmabcr;

	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

	u8 active_channels() const override;
};



class h8gen_dma_channel_device : public device_t {
public:
	enum {
		NONE       =  0,
		DREQ_LEVEL = -1,
		DREQ_EDGE  = -2,
		AUTOREQ_CS = -3,
		AUTOREQ_B  = -4,
	};

	enum {
		FAE_NORMAL,
		FAE_BLOCK,
		SAE,
		SAE_DACK,
	};

	h8_dma_state m_state[2];

	h8gen_dma_channel_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	u16 marah_r();
	void marah_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 maral_r();
	void maral_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 ioara_r();
	u8 ioara8_r();
	void ioara_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void ioara8_w(u8 data);
	u16 etcra_r();
	void etcra_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 marbh_r();
	void marbh_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 marbl_r();
	void marbl_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 ioarb_r();
	u8 ioarb8_r();
	void ioarb_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void ioarb8_w(u8 data);
	u16 etcrb_r();
	void etcrb_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void set_id(int id);
	void count_done(int submodule);
	void start_stop_test();
	bool transfer_test_interrupt(int vector);
	void set_dreq(int state);
	void start(int submodule);

protected:
	required_device<h8_device> m_cpu;
	required_device<h8_intc_device> m_intc;
	int m_irq_base;
	u32 m_ioar_mask; // ff0000 for h8s, ffff00 for h8h

	u32 m_mar[2];
	u16 m_ioar[2], m_etcr[2];
	bool m_dreq;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void dma_done(int subchannel);
	virtual int channel_mode() const = 0;
	virtual u16 channel_flags(int submodule) const = 0;
	virtual s8 trigger_vector(int submodule) const = 0;
};

class h8h_dma_channel_device : public h8gen_dma_channel_device
{
public:
	h8h_dma_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template<typename T, typename U, typename V> h8h_dma_channel_device(const machine_config &mconfig, const char *tag, device_t *owner,
																		T &&cpu, U &&dma, V &&intc, bool has_adc, bool targets_sci1)
		: h8h_dma_channel_device(mconfig, tag, owner)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
		m_dma.set_tag(std::forward<U>(dma));
		m_intc.set_tag(std::forward<V>(intc));
		m_has_adc = has_adc;
		m_targets_sci1 = targets_sci1;
	}

	u8 dtcra_r();
	void dtcra_w(u8 data);
	u8 dtcrb_r();
	void dtcrb_w(u8 data);

	u8 active_channels() const;

protected:
	required_device<h8h_dma_device> m_dma;
	u8 m_dtcr[2];
	bool m_has_adc;
	bool m_targets_sci1;

	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	void dma_done(int subchannel) override;

	int channel_mode() const override;
	u16 channel_flags(int submodule) const override;
	s8 trigger_vector(int submodule) const override;
};

class h8s_dma_channel_device : public h8gen_dma_channel_device
{
public:
	h8s_dma_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template<typename T, typename U, typename V> h8s_dma_channel_device(const machine_config &mconfig, const char *tag, device_t *owner,
																		T &&cpu, U &&dma, V &&intc)
		: h8s_dma_channel_device(mconfig, tag, owner)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
		m_dma.set_tag(std::forward<U>(dma));
		m_intc.set_tag(std::forward<V>(intc));
	}

	u16 dmacr_r();
	void dmacr_w(offs_t offset, u16 data, u16 mem_mask = ~0);

protected:
	u16 m_dmacr;

	required_device<h8s_dma_device> m_dma;
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	void dma_done(int subchannel) override;

	int channel_mode() const override;
	u16 channel_flags(int submodule) const override;
	s8 trigger_vector(int submodule) const override;
};

DECLARE_DEVICE_TYPE(H8H_DMA_CHANNEL, h8h_dma_channel_device)
DECLARE_DEVICE_TYPE(H8S_DMA_CHANNEL, h8s_dma_channel_device)

#endif // MAME_CPU_H8_H8_DMA_H
