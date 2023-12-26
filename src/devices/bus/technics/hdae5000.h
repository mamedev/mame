// license:BSD-3-Clause
// copyright-holders:Antoine Mine, Olivier Galibert, Felipe Sanches
//
//    HD-AE5000 emulation
//
#ifndef MAME_BUS_HD_AE5000_H
#define MAME_BUS_HD_AE5000_H

#include "kn5000_extension.h"
#include "bus/ata/hdd.h"
#include "machine/i8255.h"

class hdae5000_device : public device_t, public kn5000_extension_interface
{
public:
	hdae5000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hdae5000_device() = default;

	virtual void rom_map(address_map &map) override;
	virtual void io_map(address_map &map) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	required_device<ide_hdd_device> m_hdd;
	required_device<i8255_device> m_ppi;
	required_memory_region m_rom;
};

DECLARE_DEVICE_TYPE(HDAE5000, hdae5000_device)

#endif
