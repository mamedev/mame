// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_MACHINE_MSX_S1985_H
#define MAME_MACHINE_MSX_S1985_H


#include "msx_switched.h"


DECLARE_DEVICE_TYPE(MSX_S1985, msx_s1985_device)


class msx_s1985_device : public device_t,
	public msx_switched_interface,
	public device_nvram_interface
{
public:
	msx_s1985_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// msx_switched_interface overrides
	virtual uint8_t switched_read(offs_t offset) override;
	virtual void switched_write(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	bool m_selected;
	uint8_t m_backup_ram_address;
	uint8_t m_backup_ram[0x10];
	uint8_t m_color1;
	uint8_t m_color2;
	uint8_t m_pattern;
};

#endif // MAME_MACHINE_MSX_S1985_H
