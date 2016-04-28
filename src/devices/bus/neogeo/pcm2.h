// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef __NEOGEO_PCM2_H
#define __NEOGEO_PCM2_H

#include "slot.h"
#include "rom.h"
#include "prot_cmc.h"
#include "prot_pcm2.h"

// ======================> neogeo_pcm2_cart

class neogeo_pcm2_cart : public neogeo_rom_device
{
public:
	// construction/destruction
	neogeo_pcm2_cart(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT16 clock, const char *shortname, const char *source);
	neogeo_pcm2_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
//	virtual DECLARE_READ16_MEMBER(read_rom) override;

//	virtual void activate_cart(ACTIVATE_CART_PARAMS) override { m_banked_cart->install_banks(machine, maincpu, cpuregion, cpuregion_size); }
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override {}
	virtual int get_fixed_bank_type(void) override { return 0; }

	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
};

// device type definition
extern const device_type NEOGEO_PCM2_CART;


/*************************************************
 mslug4
**************************************************/

class neogeo_pcm2_mslug4_cart : public neogeo_pcm2_cart
{
public:
	neogeo_pcm2_mslug4_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }
};

extern const device_type NEOGEO_PCM2_MSLUG4_CART;


/*************************************************
 ms4plus
 **************************************************/

class neogeo_pcm2_ms4plus_cart : public neogeo_pcm2_cart
{
public:
	neogeo_pcm2_ms4plus_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }
};

extern const device_type NEOGEO_PCM2_MS4PLUS_CART;


/*************************************************
 rotd
**************************************************/

class neogeo_pcm2_rotd_cart : public neogeo_pcm2_cart
{
public:
	neogeo_pcm2_rotd_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }
};

extern const device_type NEOGEO_PCM2_ROTD_CART;


/*************************************************
 pnyaa
**************************************************/

class neogeo_pcm2_pnyaa_cart : public neogeo_pcm2_cart
{
public:
	neogeo_pcm2_pnyaa_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 1; }
};

extern const device_type NEOGEO_PCM2_PNYAA_CART;


#endif
