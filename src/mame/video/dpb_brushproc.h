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

#include "machine/74381.h"
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
	void cbus_w(uint8_t data);

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

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	void update_prom_signals();
	void update_pal_signals();
	void update_k_product();
	void update_ext_product();
	void update_final_product();
	void update_brush_value();
	void update_compare_b_value();
	uint8_t update_brush_alu_result();

	void set_oe1(int state);
	void set_oe2(int state);
	void set_oe3(int state);
	void set_oe4(int state);
	void set_mask_sel_h(int state);
	void set_16bit_h(int state);
	void set_proc_sel_h(int state);
	void set_k_eq_il(int state);

	uint8_t get_compare_b();

	uint8_t m_store_in[2];
	uint8_t m_store_out[2];
	uint8_t m_ext_in;
	uint8_t m_brush_in;
	uint8_t m_cbus_in;

	uint8_t m_k_in;
	bool m_k_enable;
	bool m_k_zero;
	bool m_k_invert;

	uint8_t m_k_product;
	uint8_t m_ext_product;
	uint16_t m_final_product;
	uint16_t m_final_result;

	uint8_t m_brush_value;
	uint8_t m_final_brush_value;
	uint8_t m_compare_b_value;

	uint8_t m_func;

	bool m_subtract_result;

	bool m_sel_luma[2];
	bool m_sel_eh;
	bool m_b_bus_ah;
	bool m_fcs;

	bool m_oe[4];
	bool m_use_store1_for_brush;
	bool m_use_store2_for_brush;
	bool m_use_ext_for_brush;
	bool m_use_store1_or_ext_for_brush;
	bool m_use_store2_for_store1;
	bool m_enable_store_ext_multiplicand;
	bool m_output_16bit;
	bool m_pal_proc_in;
	bool m_disable_k_data;

	uint16_t m_prom_addr;
	uint8_t *m_prom_base;
	uint8_t m_prom_out;

	uint16_t m_pal_in;
	uint8_t *m_pal_base;
	uint16_t m_pal_data_out;
	bool m_pal_bpinv_out;
	bool m_pal_sel_out;

	devcb_write8 m_store1;
	devcb_write8 m_store2;

	required_device<am25s558_device> m_mult_fa;
	required_device<am25s558_device> m_mult_ga;
	required_device<am25s558_device> m_mult_gd;
	required_device<sn74s381_device> m_alu_he;
	required_device<sn74s381_device> m_alu_ge;
	required_device<sn74s381_device> m_alu_fe;
	required_device<sn74s381_device> m_alu_ee;
	required_memory_region m_prom;
	required_memory_region m_pal;
};

// device type definition
DECLARE_DEVICE_TYPE(DPB7000_BRUSHPROC, dpb7000_brushproc_card_device)

#endif // MAME_VIDEO_DPB_BRUSHPROC_H
