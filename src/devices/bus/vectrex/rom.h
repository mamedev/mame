// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __VECTREX_ROM_H
#define __VECTREX_ROM_H

#include "slot.h"


// ======================> vectrex_rom_device

class vectrex_rom_device : public device_t,
						public device_vectrex_cart_interface
{
public:
	// construction/destruction
	vectrex_rom_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	vectrex_rom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override {}
	virtual void device_reset() override {}

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom) override;
};

// ======================> vectrex_rom64k_device

class vectrex_rom64k_device : public vectrex_rom_device
{
public:
	// construction/destruction
	vectrex_rom64k_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom) override;
	virtual DECLARE_WRITE8_MEMBER(write_bank) override;

private:
	int m_bank;
};

// ======================> vectrex_sram_device

class vectrex_sram_device : public vectrex_rom_device
{
public:
	// construction/destruction
	vectrex_sram_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_WRITE8_MEMBER(write_ram) override;
};



// device type definition
extern const device_type VECTREX_ROM_STD;
extern const device_type VECTREX_ROM_64K;
extern const device_type VECTREX_ROM_SRAM;


#endif
