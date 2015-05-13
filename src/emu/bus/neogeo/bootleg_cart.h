// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
#ifndef __NEOGEO_BOOTLEG_CART_H
#define __NEOGEO_BOOTLEG_CART_H

#include "neogeo_slot.h"
#include "banked_cart.h"
#include "sma_prot.h"
#include "bootleg_prot.h"
#include "kog_prot.h"

// ======================> neogeo_bootleg_cart

class neogeo_bootleg_cart : public device_t,
						public device_neogeo_cart_interface
{
public:
	// construction/destruction
	neogeo_bootleg_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source);
	neogeo_bootleg_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock);

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
};



// device type definition
extern const device_type NEOGEO_BOOTLEG_CART;


/*************************************************
 GAROUBL
**************************************************/

class neogeo_bootleg_garoubl_cart : public neogeo_bootleg_cart
{
public:
	neogeo_bootleg_garoubl_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 0; }
};
extern const device_type NEOGEO_BOOTLEG_GAROUBL_CART;


/*************************************************
 CTHD2003
**************************************************/

class neogeo_bootleg_cthd2003_cart : public neogeo_bootleg_cart
{
public:
	neogeo_bootleg_cthd2003_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 0; }
};
extern const device_type NEOGEO_BOOTLEG_CTHD2003_CART;

class neogeo_bootleg_ct2k3sp_cart : public neogeo_bootleg_cart
{
public:
	neogeo_bootleg_ct2k3sp_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 0; }
};
extern const device_type NEOGEO_BOOTLEG_CT2K3SP_CART;

class neogeo_bootleg_ct2k3sa_cart : public neogeo_bootleg_cart
{
public:
	neogeo_bootleg_ct2k3sa_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 0; }
};
extern const device_type NEOGEO_BOOTLEG_CT2K3SA_CART;


/*************************************************
 KF10THEP
**************************************************/

class neogeo_bootleg_kf10thep_cart : public neogeo_bootleg_cart
{
public:
	neogeo_bootleg_kf10thep_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 0; }
};
extern const device_type NEOGEO_BOOTLEG_KF10THEP_CART;


/*************************************************
 KF2K5UNI
**************************************************/

class neogeo_bootleg_kf2k5uni_cart : public neogeo_bootleg_cart
{
public:
	neogeo_bootleg_kf2k5uni_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 0; }
};
extern const device_type NEOGEO_BOOTLEG_KF2K5UNI_CART;

/*************************************************
 KF2K4SE
**************************************************/

class neogeo_bootleg_kf2k4se_cart : public neogeo_bootleg_cart
{
public:
	neogeo_bootleg_kf2k4se_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 0; }
};
extern const device_type NEOGEO_BOOTLEG_KF2K4SE_CART;


/*************************************************
 SVCPLUS
**************************************************/

class neogeo_bootleg_svcplus_cart : public neogeo_bootleg_cart
{
public:
	neogeo_bootleg_svcplus_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 0; }
};
extern const device_type NEOGEO_BOOTLEG_SVCPLUS_CART;

/*************************************************
 SVCPLUSA
**************************************************/

class neogeo_bootleg_svcplusa_cart : public neogeo_bootleg_cart
{
public:
	neogeo_bootleg_svcplusa_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 0; }
};
extern const device_type NEOGEO_BOOTLEG_SVCPLUSA_CART;

/*************************************************
 SAMSHO5B
**************************************************/

class neogeo_bootleg_samsho5b_cart : public neogeo_bootleg_cart
{
public:
	neogeo_bootleg_samsho5b_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 0; }
};
extern const device_type NEOGEO_BOOTLEG_SAMSHO5B_CART;


/*************************************************
 KOF97ORO
**************************************************/

class neogeo_bootleg_kof97oro_cart : public neogeo_bootleg_cart
{
public:
	neogeo_bootleg_kof97oro_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 0; }
};
extern const device_type NEOGEO_BOOTLEG_KOF97ORO_CART;


/*************************************************
 LANS2004
**************************************************/

class neogeo_bootleg_lans2004_cart : public neogeo_bootleg_cart
{
public:
	neogeo_bootleg_lans2004_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 0; }
};
extern const device_type NEOGEO_BOOTLEG_LANS2004_CART;

/*************************************************
 KOF10TH
**************************************************/

class neogeo_bootleg_kof10th_cart : public neogeo_bootleg_cart
{
public:
	neogeo_bootleg_kof10th_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 0; }
};
extern const device_type NEOGEO_BOOTLEG_KOF10TH_CART;



/*************************************************
 KOG
**************************************************/

class neogeo_bootleg_kog_cart : public neogeo_bootleg_cart
{
public:
	neogeo_bootleg_kog_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void activate_cart(ACTIVATE_CART_PARAMS);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS);
	virtual int get_fixed_bank_type(void) { return 0; }

	virtual machine_config_constructor device_mconfig_additions() const;

	required_device<kog_prot_device> m_kog_prot;
};
extern const device_type NEOGEO_BOOTLEG_KOG_CART;







#endif
