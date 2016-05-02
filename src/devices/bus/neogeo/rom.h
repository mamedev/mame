// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef __NEOGEO_ROM_H
#define __NEOGEO_ROM_H

#include "slot.h"

// ======================> neogeo_rom_device

class neogeo_rom_device : public device_t,
						public device_neogeo_cart_interface
{
public:
	// construction/destruction
	neogeo_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source);
	neogeo_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(rom_r) override;
	virtual DECLARE_WRITE16_MEMBER(banksel_w) override;
};



// device type definition
extern const device_type NEOGEO_ROM;



/*************************************************
 vliner
 **************************************************/

class neogeo_vliner_cart : public neogeo_rom_device
{
public:
	neogeo_vliner_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	virtual DECLARE_READ16_MEMBER(ram_r) override { return m_cart_ram[offset]; }
	virtual DECLARE_WRITE16_MEMBER(ram_w) override { COMBINE_DATA(&m_cart_ram[offset]); }

	virtual int get_fixed_bank_type(void) override { return 0; }
	
	virtual void device_start() override;
	virtual void device_reset() override;
	
private:
	UINT16 m_cart_ram[0x1000];
};

extern const device_type NEOGEO_VLINER_CART;


#endif
