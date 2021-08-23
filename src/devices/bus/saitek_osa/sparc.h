// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

    Saitek OSA Module: Kasparov Sparc

***************************************************************************/

#ifndef MAME_BUS_SAITEKOSA_SPARC_H
#define MAME_BUS_SAITEKOSA_SPARC_H

#pragma once

#include "expansion.h"

#include "cpu/sparc/sparc.h"


class saitekosa_sparc_device : public device_t, public device_saitekosa_expansion_interface
{
public:
	// construction/destruction
	saitekosa_sparc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::COMMS; }

	// from host
	virtual u8 data_r() override;

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	optional_device<sparcv8_device> m_maincpu;

	void main_map(address_map &map);
};


DECLARE_DEVICE_TYPE(OSA_SPARC, saitekosa_sparc_device)

#endif // MAME_BUS_SAITEKOSA_SPARC_H
