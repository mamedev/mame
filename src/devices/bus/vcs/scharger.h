// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __VCS_SCHARGER_H
#define __VCS_SCHARGER_H

#include "rom.h"
#include "imagedev/cassette.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> a26_rom_ss_device

class a26_rom_ss_device : public a26_rom_f6_device
{
public:
	// construction/destruction
	a26_rom_ss_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_reset() override;

	required_device<cassette_image_device> m_cassette;

	// reading and writing
	virtual uint8_t read_rom(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;

private:
	cpu_device   *m_maincpu;
	inline uint8_t read_byte(uint32_t offset);

	int m_base_banks[2];
	uint8_t m_reg;
	uint8_t m_write_delay, m_ram_write_enabled, m_rom_enabled;
	uint32_t m_byte_started;
	uint16_t m_last_address;
	uint32_t m_diff_adjust;
};


// device type definition
extern const device_type A26_ROM_SUPERCHARGER;

#endif
