// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_MACHINE_MSX_KANJI12_H
#define MAME_MACHINE_MSX_KANJI12_H


#include "msx_switched.h"


DECLARE_DEVICE_TYPE(MSX_KANJI12, msx_kanji12_device)


class msx_kanji12_device : public device_t,
	public msx_switched_interface
{
public:
	msx_kanji12_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	void set_rom_start(const char *region) { m_rom_region.set_tag(region); }

	// msx_switched_interface overrides
	virtual uint8_t switched_read(offs_t offset) override;
	virtual void switched_write(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	required_memory_region m_rom_region;

	bool m_selected;
	uint8_t m_row;
	uint8_t m_col;
	uint32_t m_address;
};

#endif // MAME_MACHINE_MSX_KANJI12_H
