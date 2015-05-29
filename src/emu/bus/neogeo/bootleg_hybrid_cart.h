// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
#ifndef __NEOGEO_BOOTLEG_HYBRID_HYBRID_CART_H
#define __NEOGEO_BOOTLEG_HYBRID_HYBRID_CART_H

#include "neogeo_slot.h"
#include "banked_cart.h"
#include "bootleg_prot.h"
#include "pcm2_prot.h"
#include "cmc_prot.h"
#include "kof2002_prot.h"
#include "pvc_prot.h"

// ======================> neogeo_bootleg_hybrid_hybrid_cart

class neogeo_bootleg_hybrid_hybrid_cart : public device_t,
						public device_neogeo_cart_interface
{
public:
	// construction/destruction
	neogeo_bootleg_hybrid_hybrid_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source);
	neogeo_bootleg_hybrid_hybrid_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read_rom);

	virtual void activate_cart(ACTIVATE_CART_PARAMS) {}
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) {}
	virtual int get_fixed_bank_type(void) { return 0; }

	required_device<neogeo_banked_cart_device> m_banked_cart;
	required_device<ngbootleg_prot_device> m_bootleg_prot;
	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
	required_device<kof2002_prot_device> m_kof2002_prot;
	required_device<pvc_prot_device> m_pvc_prot;
};



// device type definition
extern const device_type NEOGEO_BOOTLEG_HYBRID_HYBRID_CART;



/*************************************************
 MSLUG3B6
**************************************************/

class neogeo_bootleg_hybrid_mslug3b6_cart : public neogeo_bootleg_hybrid_hybrid_cart
{
public:
	neogeo_bootleg_hybrid_mslug3b6_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 0; }
};
extern const device_type NEOGEO_BOOTLEG_HYBRID_MSLUG3B6_CART;


/*************************************************
 KOF2002B
**************************************************/

class neogeo_bootleg_hybrid_kof2002b_cart : public neogeo_bootleg_hybrid_hybrid_cart
{
public:
	neogeo_bootleg_hybrid_kof2002b_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 0; }
};
extern const device_type NEOGEO_BOOTLEG_HYBRID_KOF2002B_CART;

class neogeo_bootleg_hybrid_kf2k2mp_cart : public neogeo_bootleg_hybrid_hybrid_cart
{
public:
	neogeo_bootleg_hybrid_kf2k2mp_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 0; }
};
extern const device_type NEOGEO_BOOTLEG_HYBRID_KF2K2MP_CART;

class neogeo_bootleg_hybrid_kf2k2mp2_cart : public neogeo_bootleg_hybrid_hybrid_cart
{
public:
	neogeo_bootleg_hybrid_kf2k2mp2_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 0; }
};
extern const device_type NEOGEO_BOOTLEG_HYBRID_KF2K2MP2_CART;


/*************************************************
 MATRIMBL
**************************************************/

class neogeo_bootleg_hybrid_matrimbl_cart : public neogeo_bootleg_hybrid_hybrid_cart
{
public:
	neogeo_bootleg_hybrid_matrimbl_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 2; }
};
extern const device_type NEOGEO_BOOTLEG_HYBRID_MATRIMBL_CART;

/*************************************************
 MS5PLUS
**************************************************/

class neogeo_bootleg_hybrid_ms5plus_cart : public neogeo_bootleg_hybrid_hybrid_cart
{
public:
	neogeo_bootleg_hybrid_ms5plus_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 1; }
};
extern const device_type NEOGEO_BOOTLEG_HYBRID_MS5PLUS_CART;

/*************************************************
 SVCBOOT
**************************************************/

class neogeo_bootleg_hybrid_svcboot_cart : public neogeo_bootleg_hybrid_hybrid_cart
{
public:
	neogeo_bootleg_hybrid_svcboot_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 0; }
};
extern const device_type NEOGEO_BOOTLEG_HYBRID_SVCBOOT_CART;

class neogeo_bootleg_hybrid_svcsplus_cart : public neogeo_bootleg_hybrid_hybrid_cart
{
public:
	neogeo_bootleg_hybrid_svcsplus_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 0; }
};
extern const device_type NEOGEO_BOOTLEG_HYBRID_SVCSPLUS_CART;



/*************************************************
 KF2K3BL
**************************************************/

class neogeo_bootleg_hybrid_kf2k3bl_cart : public neogeo_bootleg_hybrid_hybrid_cart
{
public:
	neogeo_bootleg_hybrid_kf2k3bl_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 0; }
};
extern const device_type NEOGEO_BOOTLEG_HYBRID_KF2K3BL_CART;

class neogeo_bootleg_hybrid_kf2k3pl_cart : public neogeo_bootleg_hybrid_hybrid_cart
{
public:
	neogeo_bootleg_hybrid_kf2k3pl_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 0; }
};
extern const device_type NEOGEO_BOOTLEG_HYBRID_KF2K3PL_CART;


class neogeo_bootleg_hybrid_kf2k3upl_cart : public neogeo_bootleg_hybrid_hybrid_cart
{
public:
	neogeo_bootleg_hybrid_kf2k3upl_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 0; }
};
extern const device_type NEOGEO_BOOTLEG_HYBRID_KF2K3UPL_CART;



#endif
