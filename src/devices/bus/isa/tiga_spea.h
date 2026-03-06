// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_ISA_TIGA_SPEA_H
#define MAME_BUS_ISA_TIGA_SPEA_H

#pragma once

#include "isa.h"
#include "video/pc_vga_video7.h"

class isa16_fga4he_device :
		public device_t,
		public device_isa16_card_interface
{
public:
	isa16_fga4he_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// beyond Video7 issues: lacks actual TIGA section
	static constexpr feature_type unemulated_features() { return feature::GRAPHICS; }

protected:
	isa16_fga4he_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	void io_isa_map(address_map &map) ATTR_COLD;

	virtual void remap(int space_id, offs_t start, offs_t end) override;

private:
	required_device<ht208_video7_vga_device> m_vga;
	required_memory_region m_bios;
};


DECLARE_DEVICE_TYPE(ISA16_FGA4HE, isa16_fga4he_device)


#endif // MAME_BUS_ISA_TIGA_SPEA_H
