// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_FS4600_H
#define MAME_BUS_MSX_SLOT_FS4600_H

#pragma once

#include "slot.h"
#include "machine/nvram.h"


DECLARE_DEVICE_TYPE(MSX_SLOT_FS4600, msx_slot_fs4600_device)


#define MCFG_MSX_SLOT_FS4600_ADD(_tag, _startpage, _numpages, _region, _offset) \
	MCFG_MSX_INTERNAL_SLOT_ADD(_tag, MSX_SLOT_FS4600, _startpage, _numpages) \
	msx_slot_fs4600_device::set_rom_start(*device, "^" _region, _offset);

class msx_slot_fs4600_device : public device_t, public msx_internal_slot_interface
{
public:
	msx_slot_fs4600_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	static void set_rom_start(device_t &device, const char *region, uint32_t offset);

	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;

protected:
	virtual void device_start() override;

	void restore_banks();

private:
	required_device<nvram_device> m_nvram;
	required_memory_region m_rom_region;
	uint32_t m_region_offset;
	const uint8_t *m_rom;
	uint8_t m_selected_bank[4];
	const uint8_t *m_bank_base[4];
	uint32_t m_sram_address;
	uint8_t m_sram[0x1000];
	uint8_t m_control;
};


#endif // MAME_BUS_MSX_SLOT_FS4600_H
