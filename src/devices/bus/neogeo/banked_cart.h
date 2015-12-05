// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood


#pragma once

#ifndef __NEOGEO_BANKED_CART__
#define __NEOGEO_BANKED_CART__

extern const device_type NEOGEO_BANKED_CART;

#define MCFG_NEOGEO_BANKED_CART_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, NEOGEO_BANKED_CART, 0)


class neogeo_banked_cart_device :  public device_t
{
public:
	// construction/destruction
	neogeo_banked_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	memory_bank*   m_bank_cartridge;
	UINT32     m_main_cpu_bank_address;
	UINT8* m_region;
	UINT32 m_region_size;


	void install_banks(running_machine& machine, cpu_device* maincpu, UINT8* region, UINT32 region_size);
	WRITE16_MEMBER(main_cpu_bank_select_w);
	void neogeo_set_main_cpu_bank_address(UINT32 bank_address);
	void _set_main_cpu_bank_address();
	void init_banks(void);

protected:
	virtual void device_start();
	virtual void device_reset();
	void postload();
};

#endif
