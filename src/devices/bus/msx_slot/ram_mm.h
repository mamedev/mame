// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_SLOT_RAM_MM_H
#define __MSX_SLOT_RAM_MM_H

#include "slot.h"

#define MCFG_MSX_SLOT_RAM_MM_ADD(_tag, _total_size) \
	MCFG_MSX_INTERNAL_SLOT_ADD(_tag, MSX_SLOT_RAM_MM, 0, 4) \
	msx_slot_ram_mm_device::set_total_size(*device, _total_size);

#define MCFG_MSX_SLOT_RAMM_SET_RAMIO_BITS(_ramio_set_bits) \
	msx_slot_ram_mm_device::set_ramio_set_bits(*device, _ramio_set_bits);

class msx_slot_ram_mm_device : public device_t
							, public msx_internal_slot_interface
{
public:
	msx_slot_ram_mm_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	static void set_total_size(device_t &device, UINT32 total_size) { dynamic_cast<msx_slot_ram_mm_device &>(device).m_total_size = total_size; }
	static void set_ramio_set_bits(device_t &device, UINT8 ramio_set_bits) { dynamic_cast<msx_slot_ram_mm_device &>(device).m_ramio_set_bits = ramio_set_bits; }

	virtual void device_start() override;

	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;

	DECLARE_READ8_MEMBER(read_mapper_bank);
	DECLARE_WRITE8_MEMBER(write_mapper_bank);

	void restore_banks();

private:
	std::vector<UINT8> m_ram;
	UINT32 m_total_size;
	UINT8 m_bank_mask;
	UINT8 m_bank_selected[4];
	UINT8 *m_bank_base[4];
	UINT8 m_ramio_set_bits;
};

extern const device_type MSX_SLOT_RAM_MM;

#endif
