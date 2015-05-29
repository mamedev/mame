// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
#ifndef __NEOGEO_PVC_CART_H
#define __NEOGEO_PVC_CART_H

#include "neogeo_slot.h"
#include "banked_cart.h"
#include "pcm2_prot.h"
#include "cmc_prot.h"
#include "pvc_prot.h"

// ======================> neogeo_pvc_cart

class neogeo_pvc_cart : public device_t,
						public device_neogeo_cart_interface
{
public:
	// construction/destruction
	neogeo_pvc_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source);
	neogeo_pvc_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read_rom);

	virtual void activate_cart(ACTIVATE_CART_PARAMS)
	{
		m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size);
		m_pvc_prot->install_pvc_protection(maincpu,m_banked_cart);
	}

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) {}
	virtual int get_fixed_bank_type(void) { return 0; }

	required_device<neogeo_banked_cart_device> m_banked_cart;
	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
	required_device<pvc_prot_device> m_pvc_prot;

};



// device type definition
extern const device_type NEOGEO_PVC_CART;


/*************************************************
 MSLUG5
**************************************************/

class neogeo_pvc_mslug5_cart : public neogeo_pvc_cart
{
public:
	neogeo_pvc_mslug5_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 1; }
};
extern const device_type NEOGEO_PVC_MSLUG5_CART;

/*************************************************
 SVC
**************************************************/

class neogeo_pvc_svc_cart : public neogeo_pvc_cart
{
public:
	neogeo_pvc_svc_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 2; }
};
extern const device_type NEOGEO_PVC_SVC_CART;


/*************************************************
 KOF2003
**************************************************/

class neogeo_pvc_kof2003_cart : public neogeo_pvc_cart
{
public:
	neogeo_pvc_kof2003_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 2; }
};
extern const device_type NEOGEO_PVC_KOF2003_CART;


/*************************************************
 KOF2003H
**************************************************/

class neogeo_pvc_kof2003h_cart : public neogeo_pvc_cart
{
public:
	neogeo_pvc_kof2003h_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 2; }
};
extern const device_type NEOGEO_PVC_KOF2003H_CART;



#endif
