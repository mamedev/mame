// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef MAME_BUS_NEOGEO_PCM2_H
#define MAME_BUS_NEOGEO_PCM2_H

#include "slot.h"
#include "rom.h"
#include "prot_cmc.h"
#include "prot_pcm2.h"

// ======================> neogeo_pcm2_cart_device

class neogeo_pcm2_cart_device : public neogeo_rom_device
{
public:
	// construction/destruction
	neogeo_pcm2_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock);

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override { }
	virtual int get_fixed_bank_type() override { return 0; }

protected:
	neogeo_pcm2_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<cmc_prot_device> m_cmc_prot;
	required_device<pcm2_prot_device> m_pcm2_prot;
};

// device type definition
DECLARE_DEVICE_TYPE(NEOGEO_PCM2_CART, neogeo_pcm2_cart_device)


/*************************************************
 mslug4
**************************************************/

class neogeo_pcm2_mslug4_cart_device : public neogeo_pcm2_cart_device
{
public:
	neogeo_pcm2_mslug4_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
};

DECLARE_DEVICE_TYPE(NEOGEO_PCM2_MSLUG4_CART, neogeo_pcm2_mslug4_cart_device)


/*************************************************
 ms4plus
 **************************************************/

class neogeo_pcm2_ms4plus_cart_device : public neogeo_pcm2_cart_device
{
public:
	neogeo_pcm2_ms4plus_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }
};

DECLARE_DEVICE_TYPE(NEOGEO_PCM2_MS4PLUS_CART, neogeo_pcm2_ms4plus_cart_device)


/*************************************************
 rotd
**************************************************/

class neogeo_pcm2_rotd_cart_device : public neogeo_pcm2_cart_device
{
public:
	neogeo_pcm2_rotd_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
};

DECLARE_DEVICE_TYPE(NEOGEO_PCM2_ROTD_CART, neogeo_pcm2_rotd_cart_device)


/*************************************************
 pnyaa
**************************************************/

class neogeo_pcm2_pnyaa_cart_device : public neogeo_pcm2_cart_device
{
public:
	neogeo_pcm2_pnyaa_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 1; }
};

DECLARE_DEVICE_TYPE(NEOGEO_PCM2_PNYAA_CART, neogeo_pcm2_pnyaa_cart_device)


#endif // MAME_BUS_NEOGEO_PCM2_H
