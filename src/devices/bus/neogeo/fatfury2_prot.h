// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood

#pragma once

#ifndef __FATFURY2_PROT__
#define __FATFURY2_PROT__

#include "banked_cart.h"

extern const device_type FATFURY2_PROT;

#define MCFG_FATFURY2_PROT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, FATFURY2_PROT, 0)


class fatfury2_prot_device :  public device_t
{
public:
	// construction/destruction
	fatfury2_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ16_MEMBER( fatfury2_protection_16_r );
	DECLARE_WRITE16_MEMBER( fatfury2_protection_16_w );
	void fatfury2_install_protection(cpu_device* maincpu, neogeo_banked_cart_device* bankdev);


	neogeo_banked_cart_device* m_bankdev;
	UINT32     m_fatfury2_prot_data;

protected:
	virtual void device_start();
	virtual void device_reset();
};

#endif
