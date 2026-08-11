// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_SEGA_SATURN_SCU_H
#define MAME_SEGA_SATURN_SCU_H

#pragma once

#include "cpu/sh/sh7604.h"
#include "cpu/scudsp/scudsp.h"

#include <tuple>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> saturn_scu_device

class saturn_scu_device : public device_t
{
public:
	// construction/destruction
	saturn_scu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_hostcpu(T &&tag) { m_hostcpu.set_tag(std::forward<T>(tag)); }
	auto main_dtack_cb()  { return m_main_dtack_cb.bind(); }
	auto main_steal_cb()  { return m_main_steal_cb.bind(); }
	auto sound_dtack_cb() { return m_sound_dtack_cb.bind(); }
	auto sound_steal_cb() { return m_sound_steal_cb.bind(); }

	// I/O operations
	void regs_map(address_map &map) ATTR_COLD;

	void vblank_out_w(int state);
	void vblank_in_w(int state);
	void hblank_in_w(int state);
	void vdp1_end_w(int state);
	void sound_req_w(int state);
	void smpc_irq_w(int state);

	IRQ_CALLBACK_MEMBER(irq_ack_cb);

	// bus flags
	static constexpr uint16_t A_BUS       = 0x0100;
	static constexpr uint16_t A_BUS_CS0   = 0x0101;
	static constexpr uint16_t A_BUS_CS1   = 0x0102;
	static constexpr uint16_t A_BUS_DUMMY = 0x0103;
	static constexpr uint16_t A_BUS_CS2   = 0x0104;
	static constexpr uint16_t B_BUS       = 0x0200;
	static constexpr uint16_t B_BUS_SCSP  = 0x0201;
	static constexpr uint16_t B_BUS_VDP1  = 0x0202;
	static constexpr uint16_t B_BUS_VDP2  = 0x0203;
	static constexpr uint16_t B_BUS_SCU   = 0x0204;
	static constexpr uint16_t C_BUS       = 0x0300;

protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override;
	virtual void device_clock_changed() override;

private:
	required_device<scudsp_cpu_device> m_scudsp;
	required_device<sh7604_device> m_hostcpu;
	address_space *m_hostspace;
	devcb_write_line m_main_dtack_cb;
	devcb_write8     m_main_steal_cb;
	devcb_write_line m_sound_dtack_cb;
	devcb_write8     m_sound_steal_cb;

	enum dma_id : int {
		DMALV0_ID = 0,
		DMALV1_ID,
		DMALV2_ID
	};

	enum dma_event_id_t : uint8_t {
		DMA_EVENT_VBLANKIN = 0,
		DMA_EVENT_VBLANKOUT,
		DMA_EVENT_HBLANKIN,
		DMA_EVENT_TIMER0,
		DMA_EVENT_TIMER1,
		DMA_EVENT_SCSP,
		DMA_EVENT_VDP1,
		DMA_EVENT_TRIGGER // DMA activation bit
	};

	enum ist_source_t : uint32_t {
		IST_VBLANK_IN  = 1 << 0,
		IST_VBLANK_OUT = 1 << 1,
		IST_HBLANK_IN  = 1 << 2,
		IST_TIMER_0    = 1 << 3,
		IST_TIMER_1    = 1 << 4,
		IST_DSP_END    = 1 << 5,
		IST_SOUND_REQ  = 1 << 6,
		IST_SMPC       = 1 << 7,
		IST_PAD        = 1 << 8,
		IST_DMALV2     = 1 << 9,
		IST_DMALV1     = 1 << 10,
		IST_DMALV0     = 1 << 11,
		IST_DMAILL     = 1 << 12,
		IST_VDP1_END   = 1 << 13,
		IST_ABUS       = 1 << 15
	};

	// background a.k.a. interrupt flag (paused out of higher priority executed)
	// move a.k.a. operation flag (DMA is executing)
	// wait a.k.a. stand by (a starting period where the DMA goes from idle to operating)
	// the move/wait given here are for documentation purposes only, they are impractical
	// and hot for what we need
	enum dma_status_t : uint32_t {
		DMA_DSP_MOVE      = 1 << 0,  // DDMV
		DMA_DSP_WAIT      = 1 << 1,  // DDWT
		DMA_LV0_MOVE      = 1 << 4,  // D0MV
		DMA_LV0_WAIT      = 1 << 5,  // D0WT
		DMA_LV1_MOVE      = 1 << 8,  // D1MV
		DMA_LV1_WAIT      = 1 << 9,  // D1WT
		DMA_LV2_MOVE      = 1 << 12, // D2MV
		DMA_LV2_WAIT      = 1 << 13, // D2WT
		DMA_LV0_BK        = 1 << 16, // D0BK
		DMA_LV1_BK        = 1 << 17, // D1BK
		DMA_ACCESS_A_BUS  = 1 << 20, // DACSA
		DMA_ACCESS_B_BUS  = 1 << 21, // DACSB
		DMA_ACCESS_DSP    = 1 << 22  // DACSD
	};

	enum dma_state_t : uint32_t {
		DMA_STATE_IDLE      = 0x00,
		DMA_STATE_MOVE      = 0x10,
		DMA_STATE_WAIT      = 0x20,
		DMA_STATE_MOVE_WAIT = 0x30 // shouldn't happen?
	};

	TIMER_CALLBACK_MEMBER(dma_tick_cb);
	TIMER_CALLBACK_MEMBER(timer1_irq_cb);
	emu_timer *m_timer1;
	emu_timer *m_dma_tick_timer;
	uint32_t m_ism;
	uint32_t m_ist;
	uint32_t m_t0c;
	uint32_t m_t1s;
	uint32_t m_dma_status;
	bool m_t1md;
	bool m_tenb;
	int m_current_irq_level;
	uint8_t m_current_vector;
	uint16_t m_timer0_counter;
	uint32_t m_dma_clock_ref;

	void test_pending_irqs();

	// intended to be used as bitwise
	enum dma_mode_t : uint32_t {
		DMA_MODE_RESET      = 0,
		DMA_MODE_CBUS_WRITE = 1,
		DMA_MODE_CD         = 2,
		DMA_MODE_INDIRECT   = 4
	};

	struct dma_channel_t {
		uint32_t    src;       /* Source DMA lv n address*/
		uint32_t    dst;       /* Destination DMA lv n address*/
		uint32_t    src_add;   /* Source Addition for DMA lv n*/
		uint32_t    dst_add;   /* Destination Addition for DMA lv n*/
		uint32_t    size;      /* Transfer DMA size lv n*/
		uint32_t    index;
		uint32_t    live_src;
		uint32_t    live_dst;
		uint32_t    live_size;
		uint32_t    live_count;
		uint8_t     start_factor;
		uint32_t    mode;
		bool        enable_mask;
		bool        indirect_mode;
		bool        indirect_fetch_phase;
		bool        indirect_end_flag;
		bool        rup;
		bool        wup;
		bool        done;
		bool        bbus_sound_access;
		int         transfer_penalty;
	}m_dma[3];

	using dma_transfer_func = void (saturn_scu_device::*)(dma_channel_t &ch);
	static const dma_transfer_func dma_transfer_table[4];

	std::tuple<u16, int> get_address_flags(u32 address, bool write_op);
	void dma_transfer_direct_default(dma_channel_t &ch);
	void dma_transfer_direct_cbus_write(dma_channel_t &ch);
	void dma_transfer_direct_cd(dma_channel_t &ch);
	void dma_transfer_direct_cd_cbus_write(dma_channel_t &ch);

	void trigger_dma_direct(uint8_t level);
	void trigger_dma_indirect(uint8_t level);
	void update_dma_status(int level, dma_state_t state);
	[[maybe_unused]] void dma_single_transfer(uint32_t src, uint32_t dst,uint8_t *src_shift);
	void dma_start_factor_ack(dma_event_id_t event);
	std::tuple<int, int> check_dma_level_round_robin();

	void scudsp_end_w(int state);
	uint16_t scudsp_dma_r(offs_t offset, uint16_t mem_mask = ~0);
	void scudsp_dma_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// DMA
	template <unsigned Level> void dma_map(address_map &map);
	uint32_t dma_status_r();

	// Timers
	void t0_compare_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void t1_setdata_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void t1_mode_w(uint16_t data);
	// Interrupt
	uint32_t irq_mask_r();
	uint32_t irq_status_r();
	void irq_mask_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void irq_status_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t version_r();
};

// device type definition
DECLARE_DEVICE_TYPE(SATURN_SCU, saturn_scu_device)

#endif // MAME_SEGA_SATURN_SCU_H
