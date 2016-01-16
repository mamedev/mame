// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __PCE_ROM_H
#define __PCE_ROM_H

#include "pce_slot.h"


// ======================> pce_rom_device

class pce_rom_device : public device_t,
						public device_pce_cart_interface
{
public:
	// construction/destruction
	pce_rom_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	pce_rom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override {}
	virtual void device_reset() override {}

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_cart) override;
};

// ======================> pce_cdsys3_device

class pce_cdsys3_device : public pce_rom_device
{
public:
	// construction/destruction
	pce_cdsys3_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_cart) override;
	virtual DECLARE_WRITE8_MEMBER(write_cart) override;
};


// ======================> pce_populous_device

class pce_populous_device : public pce_rom_device
{
public:
	// construction/destruction
	pce_populous_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_cart) override;
	virtual DECLARE_WRITE8_MEMBER(write_cart) override;
};


// ======================> pce_sf2_device

class pce_sf2_device : public pce_rom_device
{
public:
	// construction/destruction
	pce_sf2_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_cart) override;
	virtual DECLARE_WRITE8_MEMBER(write_cart) override;

private:
	UINT8 m_bank_base;
};



// device type definition
extern const device_type PCE_ROM_STD;
extern const device_type PCE_ROM_CDSYS3;
extern const device_type PCE_ROM_POPULOUS;
extern const device_type PCE_ROM_SF2;



#endif
