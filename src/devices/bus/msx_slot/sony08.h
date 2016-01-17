// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_SLOT_SONY08_H
#define __MSX_SLOT_SONY08_H

#include "slot.h"
#include "machine/nvram.h"


extern const device_type MSX_SLOT_SONY08;


#define MCFG_MSX_SLOT_SONY08_ADD(_tag, _startpage, _numpages, _region, _offset) \
	MCFG_MSX_INTERNAL_SLOT_ADD(_tag, MSX_SLOT_SONY08, _startpage, _numpages) \
	msx_slot_sony08_device::set_rom_start(*device, _region, _offset);

class msx_slot_sony08_device : public device_t,
							public msx_internal_slot_interface
{
public:
	msx_slot_sony08_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	static void set_rom_start(device_t &device, const char *region, UINT32 offset);

	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;

	void restore_banks();

private:
	required_device<nvram_device> m_nvram;
	const char *m_region;
	UINT32 m_region_offset;
	const UINT8 *m_rom;
	UINT8 m_selected_bank[8];
	const UINT8 *m_bank_base[8];
	UINT8 m_sram[0x4000];

	void map_bank(int bank);
};


#endif
