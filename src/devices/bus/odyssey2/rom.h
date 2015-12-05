// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __O2_ROM_H
#define __O2_ROM_H

#include "slot.h"


// ======================> o2_rom_device

class o2_rom_device : public device_t,
						public device_o2_cart_interface
{
public:
	// construction/destruction
	o2_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	o2_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom04);
	virtual DECLARE_READ8_MEMBER(read_rom0c);

	virtual void write_bank(int bank) override;

protected:
	int m_bank_base;
};

// ======================> o2_rom12_device

class o2_rom12_device : public o2_rom_device
{
public:
	// construction/destruction
	o2_rom12_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom04);
	virtual DECLARE_READ8_MEMBER(read_rom0c);
};

// ======================> o2_rom16_device

class o2_rom16_device : public o2_rom_device
{
public:
	// construction/destruction
	o2_rom16_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom04);
	virtual DECLARE_READ8_MEMBER(read_rom0c);
};



// device type definition
extern const device_type O2_ROM_STD;
extern const device_type O2_ROM_12K;
extern const device_type O2_ROM_16K;


#endif
