// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_RAM_MM_H
#define MAME_BUS_MSX_SLOT_RAM_MM_H

#include "slot.h"

#define MCFG_MSX_SLOT_RAM_MM_ADD(_tag, _total_size) \
	MCFG_MSX_INTERNAL_SLOT_ADD(_tag, MSX_SLOT_RAM_MM, 0, 4) \
	downcast<msx_slot_ram_mm_device &>(*device).set_total_size(_total_size);

#define MCFG_MSX_SLOT_RAMM_SET_RAMIO_BITS(_ramio_set_bits) \
	downcast<msx_slot_ram_mm_device &>(*device).set_ramio_set_bits(_ramio_set_bits);

class msx_slot_ram_mm_device : public device_t, public msx_internal_slot_interface
{
public:
	msx_slot_ram_mm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_total_size(uint32_t total_size) { m_total_size = total_size; }
	void set_ramio_set_bits(uint8_t ramio_set_bits) { m_ramio_set_bits = ramio_set_bits; }

	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;

	DECLARE_READ8_MEMBER(read_mapper_bank);
	DECLARE_WRITE8_MEMBER(write_mapper_bank);

protected:
	virtual void device_start() override;

	void restore_banks();

private:
	std::vector<uint8_t> m_ram;
	uint32_t m_total_size;
	uint8_t m_bank_mask;
	uint8_t m_bank_selected[4];
	uint8_t *m_bank_base[4];
	uint8_t m_ramio_set_bits;
};

DECLARE_DEVICE_TYPE(MSX_SLOT_RAM_MM, msx_slot_ram_mm_device)

#endif // MAME_BUS_MSX_SLOT_RAM_MM_H
