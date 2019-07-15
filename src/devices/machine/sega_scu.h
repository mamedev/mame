// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

    Sega System Control Unit (c) 1995 Sega

***************************************************************************/

#ifndef MAME_MACHINE_SEGA_SCU_H
#define MAME_MACHINE_SEGA_SCU_H

#pragma once

#include "cpu/sh/sh2.h"
#include "cpu/scudsp/scudsp.h"

#define IRQ_VBLANK_IN  1 << 0
#define IRQ_VBLANK_OUT 1 << 1
#define IRQ_HBLANK_IN  1 << 2
#define IRQ_TIMER_0    1 << 3
#define IRQ_TIMER_1    1 << 4
#define IRQ_DSP_END    1 << 5
#define IRQ_SOUND_REQ  1 << 6
#define IRQ_SMPC       1 << 7
#define IRQ_PAD        1 << 8
#define IRQ_DMALV2     1 << 9
#define IRQ_DMALV1     1 << 10
#define IRQ_DMALV0     1 << 11
#define IRQ_DMAILL     1 << 12
#define IRQ_VDP1_END   1 << 13
#define IRQ_ABUS       1 << 15


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sega_scu_device

class sega_scu_device : public device_t
{
public:
	// construction/destruction
	sega_scu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void regs_map(address_map &map);

	DECLARE_WRITE_LINE_MEMBER(vblank_out_w);
	DECLARE_WRITE_LINE_MEMBER(vblank_in_w);
	DECLARE_WRITE_LINE_MEMBER(hblank_in_w);
	DECLARE_WRITE_LINE_MEMBER(vdp1_end_w);
	void check_scanline_timers(int scanline,int y_step);
	DECLARE_WRITE_LINE_MEMBER(sound_req_w);
	DECLARE_WRITE_LINE_MEMBER(smpc_irq_w);

	template <typename T> void set_hostcpu(T &&tag) { m_hostcpu.set_tag(std::forward<T>(tag)); }

protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_reset_after_children() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	required_device<scudsp_cpu_device> m_scudsp;

	enum {
		DMALV0_ID = 0,
		DMALV1_ID,
		DMALV2_ID
	};

	emu_timer *m_dma_timer[3];
	uint32_t m_ism;
	uint32_t m_ist;
	uint32_t m_t0c;
	uint32_t m_t1s;
	uint32_t m_status;
	bool m_t1md;
	bool m_tenb;

	required_device<sh2_device> m_hostcpu;
	address_space *m_hostspace;
	void test_pending_irqs();

	struct {
		uint32_t    src;       /* Source DMA lv n address*/
		uint32_t    dst;       /* Destination DMA lv n address*/
		uint32_t    src_add;   /* Source Addition for DMA lv n*/
		uint32_t    dst_add;   /* Destination Addition for DMA lv n*/
		uint32_t    size;      /* Transfer DMA size lv n*/
		uint32_t    index;
		uint8_t     start_factor;
		bool        enable_mask;
		bool        indirect_mode;
		bool        rup;
		bool        wup;
	}m_dma[3];

	uint32_t dma_common_r(uint8_t offset,uint8_t level);
	void dma_common_w(uint8_t offset,uint8_t level,uint32_t data);
	void handle_dma_direct(uint8_t level);
	void handle_dma_indirect(uint8_t level);
	void update_dma_status(uint8_t level,bool state);
	void dma_single_transfer(uint32_t src, uint32_t dst,uint8_t *src_shift);
	void dma_start_factor_ack(uint8_t event);

	DECLARE_WRITE_LINE_MEMBER(scudsp_end_w);
	DECLARE_READ16_MEMBER(scudsp_dma_r);
	DECLARE_WRITE16_MEMBER(scudsp_dma_w);

	// DMA
	DECLARE_READ32_MEMBER(dma_lv0_r);
	DECLARE_WRITE32_MEMBER(dma_lv0_w);
	DECLARE_READ32_MEMBER(dma_lv1_r);
	DECLARE_WRITE32_MEMBER(dma_lv1_w);
	DECLARE_READ32_MEMBER(dma_lv2_r);
	DECLARE_WRITE32_MEMBER(dma_lv2_w);
	DECLARE_READ32_MEMBER(dma_status_r);

	// Timers
	DECLARE_WRITE32_MEMBER(t0_compare_w);
	DECLARE_WRITE32_MEMBER(t1_setdata_w);
	DECLARE_WRITE16_MEMBER(t1_mode_w);
	// Interrupt
	DECLARE_READ32_MEMBER(irq_mask_r);
	DECLARE_READ32_MEMBER(irq_status_r);
	DECLARE_WRITE32_MEMBER(irq_mask_w);
	DECLARE_WRITE32_MEMBER(irq_status_w);
	DECLARE_READ32_MEMBER(version_r);
};


// device type definition
DECLARE_DEVICE_TYPE(SEGA_SCU, sega_scu_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_MACHINE_SEGA_SCU_H
