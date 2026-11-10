// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_A800_PBI_KMKIDE_H
#define MAME_BUS_A800_PBI_KMKIDE_H

#pragma once

#include "slot.h"

#include "bus/ata/ataintf.h"

class kmkide_ata_device : public atari_pbi_card_device
{
public:
	kmkide_ata_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual int pdvs_slot_w(u8 data) override;

private:
	required_device<ata_interface_device> m_ata;
	required_memory_region m_bios;
	memory_bank_creator m_rambank;
	std::vector<u8> m_ram;
	required_ioport m_jp;

	u8 ide_cs0_r(offs_t offset);
	void ide_cs0_w(offs_t offset, u8 data);
	u8 ide_cs1_r(offs_t offset);
	void ide_cs1_w(offs_t offset, u8 data);
	template <unsigned N> void ram_bank_w(offs_t offset, u8 data);

	u8 m_latch[2];
	u8 m_pbi_id;
};

DECLARE_DEVICE_TYPE(KMKIDE_ATA, kmkide_ata_device)


#endif // MAME_BUS_A800_PBI_KMKIDE_H
