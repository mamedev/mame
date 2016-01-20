// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood


#pragma once

#include "banked_cart.h"

#ifndef __SMA_PROT__
#define __SMA_PROT__

extern const device_type SMA_PROT;

#define MCFG_SMA_PROT_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SMA_PROT, 0)


class sma_prot_device :  public device_t
{
public:
	// construction/destruction
	sma_prot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);


	DECLARE_WRITE16_MEMBER( kof99_bankswitch_w );
	DECLARE_WRITE16_MEMBER( garou_bankswitch_w );
	DECLARE_WRITE16_MEMBER( garouh_bankswitch_w );
	DECLARE_WRITE16_MEMBER( mslug3_bankswitch_w );
	DECLARE_WRITE16_MEMBER( kof2000_bankswitch_w );
	DECLARE_READ16_MEMBER( prot_9a37_r );
	DECLARE_READ16_MEMBER( sma_random_r );
	void reset_sma_rng();
	void sma_install_random_read_handler(cpu_device* maincpu, int addr1, int addr2 );
	void kof99_install_protection(cpu_device* maincpu, neogeo_banked_cart_device* bankdev);
	void garou_install_protection(cpu_device* maincpu, neogeo_banked_cart_device* bankdev);
	void garouh_install_protection(cpu_device* maincpu, neogeo_banked_cart_device* bankdev);
	void mslug3_install_protection(cpu_device* maincpu, neogeo_banked_cart_device* bankdev);
	void kof2000_install_protection(cpu_device* maincpu, neogeo_banked_cart_device* bankdev);
	neogeo_banked_cart_device* m_bankdev;
	void kof99_decrypt_68k(UINT8* base);
	void garou_decrypt_68k(UINT8* base);
	void garouh_decrypt_68k(UINT8* base);
	void mslug3_decrypt_68k(UINT8* base);
	void kof2000_decrypt_68k(UINT8* base);

	UINT16     m_sma_rng;
protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

#endif
