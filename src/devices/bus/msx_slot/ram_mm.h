// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_RAM_MM_H
#define MAME_BUS_MSX_SLOT_RAM_MM_H

#include "slot.h"

class msx_slot_ram_mm_device : public device_t, public msx_internal_slot_interface
{
public:
	msx_slot_ram_mm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	msx_slot_ram_mm_device &set_total_size(uint32_t total_size) { m_total_size = total_size; return *this; }
	msx_slot_ram_mm_device &set_ramio_bits(uint8_t ramio_set_bits) { m_ramio_set_bits = ramio_set_bits; return *this; }

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override;
	virtual void device_post_load() override;

	void restore_banks();

private:
	uint8_t read_mapper_bank(offs_t offset);
	void write_mapper_bank(offs_t offset, uint8_t data);

	std::vector<uint8_t> m_ram;
	uint32_t m_total_size;
	uint8_t m_bank_mask;
	uint8_t m_bank_selected[4];
	uint8_t *m_bank_base[4];
	uint8_t m_ramio_set_bits;
};

DECLARE_DEVICE_TYPE(MSX_SLOT_RAM_MM, msx_slot_ram_mm_device)

#endif // MAME_BUS_MSX_SLOT_RAM_MM_H
