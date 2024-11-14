// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP Expansion Board VP-575 emulation

**********************************************************************/

#ifndef MAME_BUS_VIP_VP575_H
#define MAME_BUS_VIP_VP575_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vp575_device

class vp575_device : public device_t,
						public device_vip_expansion_card_interface
{
public:
	// construction/destruction
	vp575_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_vip_expansion_card_interface overrides
	virtual uint8_t vip_program_r(offs_t offset, int cs, int cdef, int *minh) override;
	virtual void vip_program_w(offs_t offset, uint8_t data, int cdef, int *minh) override;
	virtual uint8_t vip_io_r(offs_t offset) override;
	virtual void vip_io_w(offs_t offset, uint8_t data) override;
	virtual uint8_t vip_dma_r(offs_t offset) override;
	virtual void vip_dma_w(offs_t offset, uint8_t data) override;
	virtual uint32_t vip_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;
	virtual int vip_ef1_r() override;
	virtual int vip_ef3_r() override;
	virtual int vip_ef4_r() override;
	virtual void vip_sc_w(int n, int sc) override;
	virtual void vip_q_w(int state) override;
	virtual void vip_tpb_w(int state) override;
	virtual void vip_run_w(int state) override;

private:
	static constexpr unsigned MAX_SLOTS = 5;

	void exp1_int_w(int state) { m_int[0] = state; update_interrupts(); }
	void exp2_int_w(int state) { m_int[1] = state; update_interrupts(); }
	void exp3_int_w(int state) { m_int[2] = state; update_interrupts(); }
	void exp4_int_w(int state) { m_int[3] = state; update_interrupts(); }
	void exp5_int_w(int state) { m_int[4] = state; update_interrupts(); }

	void exp1_dma_out_w(int state) { m_dma_out[0] = state; update_interrupts(); }
	void exp2_dma_out_w(int state) { m_dma_out[1] = state; update_interrupts(); }
	void exp3_dma_out_w(int state) { m_dma_out[2] = state; update_interrupts(); }
	void exp4_dma_out_w(int state) { m_dma_out[3] = state; update_interrupts(); }
	void exp5_dma_out_w(int state) { m_dma_out[4] = state; update_interrupts(); }

	void exp1_dma_in_w(int state) { m_dma_in[0] = state; update_interrupts(); }
	void exp2_dma_in_w(int state) { m_dma_in[1] = state; update_interrupts(); }
	void exp3_dma_in_w(int state) { m_dma_in[2] = state; update_interrupts(); }
	void exp4_dma_in_w(int state) { m_dma_in[3] = state; update_interrupts(); }
	void exp5_dma_in_w(int state) { m_dma_in[4] = state; update_interrupts(); }

	void update_interrupts();

	required_device_array<vip_expansion_slot_device, MAX_SLOTS> m_expansion_slot;

	int m_int[MAX_SLOTS];
	int m_dma_out[MAX_SLOTS];
	int m_dma_in[MAX_SLOTS];
};


// device type definition
DECLARE_DEVICE_TYPE(VP575, vp575_device)

#endif // MAME_BUS_VIP_VP575_H
