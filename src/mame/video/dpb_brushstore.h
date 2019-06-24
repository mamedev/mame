// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    dpb_brushstore.h
    DPB-7000/1 - Brush Store Card

***************************************************************************/

#ifndef MAME_VIDEO_DPB_BRUSHSTORE_H
#define MAME_VIDEO_DPB_BRUSHSTORE_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dpb7000_brush_store_card_device

class dpb7000_brush_store_card_device : public device_t
{
public:
	// construction/destruction
	dpb7000_brush_store_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint16_t read();
	void write(uint16_t data);

	void addr_w(uint8_t data);
	void a0_chr_w(int state);
	void ras_w(int state);
	void cas_w(int state);

	void lumen_w(int state);
	void chren_w(int state);

	void ca0_w(int state);
	void ksel_w(int state);

	void fcs_w(int state);
	void func_w(uint8_t data);

	void b_bus_a_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	enum : size_t
	{
		STRIPE_CHR,
		STRIPE_K,
		STRIPE_LUM,
		STRIPE_COUNT
	};

	uint8_t *m_pal_base;

	uint8_t m_addr;
	uint8_t m_a0_chr;

	bool m_ras;
	bool m_cas;

	uint8_t m_rav[STRIPE_COUNT];
	uint8_t m_cav[STRIPE_COUNT];
	bool m_bck;

	bool m_lumen;
	bool m_chren;

	bool m_ca0;

	bool m_ksel;
	bool m_fcs;
	uint8_t m_func;
	bool m_b_bus_a;

	std::unique_ptr<uint8_t[]> m_stripes[STRIPE_COUNT];

	// Output Lines
	devcb_write_line m_store_write_out;
	devcb_write8 m_k_data_out;
	devcb_write8 m_lum_data_out;
	devcb_write8 m_chr_data_out;

	// Devices
	required_memory_region m_pal;
};

// device type definition
DECLARE_DEVICE_TYPE(DPB7000_BRUSHSTORE, dpb7000_brush_store_card_device)

#endif // MAME_VIDEO_DPB_BRUSHSTORE_H
