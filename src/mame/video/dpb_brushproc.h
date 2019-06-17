// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    dpb_brushproc.h
    DPB-7000/1 - Brush Processor Card

	TODO:
	- Everything

***************************************************************************/

#ifndef MAME_VIDEO_DPB_BRUSHPROC_H
#define MAME_VIDEO_DPB_BRUSHPROC_H

#pragma once

#include "machine/am25s55x.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dpb7000_brushproc_card_device

class dpb7000_brushproc_card_device : public device_t
{
public:
	// construction/destruction
	dpb7000_brushproc_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void store1_w(uint8_t data);
	void store2_w(uint8_t data);
	void ext_w(uint8_t data);
	void brush_w(uint8_t data);

	void k_w(uint8_t data);
	void k_en_w(int state);
	void k_zero_w(int state);
	void k_inv_w(int state);

	void func_w(uint8_t data);
	void sel_lum1_w(int state);
	void sel_lum2_w(int state);
	void sel_eh_w(int state);
	void b_bus_ah_w(int state);
	void fixed_col_select_w(int state);

	auto store1() { return m_store1.bind(); }
	auto store2() { return m_store2.bind(); }
	auto cbus() { return m_cbus.bind(); }
	auto pck() { return m_pck.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	uint8_t m_store_in[2];
	uint8_t m_ext_in;
	uint8_t m_brush_in;

	uint8_t m_k_in;
	bool m_k_enable;
	bool m_k_zero;
	bool m_k_invert;

	uint8_t m_func;

	bool m_sel_luma[2];
	bool m_sel_eh;
	bool m_b_bus_ah;
	bool m_fcs;

	devcb_write8 m_store1;
	devcb_write8 m_store2;
	devcb_write8 m_cbus;
	devcb_write_line m_pck;
	required_device<am25s558_device> m_mult_fa;
	required_device<am25s558_device> m_mult_ga;
	required_device<am25s558_device> m_mult_gd;
};

// device type definition
DECLARE_DEVICE_TYPE(DPB7000_BRUSHPROC, dpb7000_brushproc_card_device)

#endif // MAME_VIDEO_DPB_BRUSHPROC_H
