// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// CQ 90-028 - QDD drive controller built from a motorola 6852 (serial chip)
//
// Handles n? QDD drives (QD 90-128)

#ifndef MAME_BUS_THOMSON_CQ90_028_H
#define MAME_BUS_THOMSON_CQ90_028_H

#include "extension.h"
#include "machine/mc6852.h"

class cq90_028_device : public device_t, public thomson_extension_interface
{
public:
	cq90_028_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~cq90_028_device() = default;

	static constexpr feature_type unemulated_features() { return feature::DISK; }

	virtual void rom_map(address_map &map) override ATTR_COLD;
	virtual void io_map(address_map &map) override ATTR_COLD;

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<mc6852_device> m_serial;
	required_memory_region m_rom;

	void drive_w(u8 data);
	void motor_w(u8 data);
	u8 status_r();
};

DECLARE_DEVICE_TYPE(CQ90_028, cq90_028_device)

#endif
