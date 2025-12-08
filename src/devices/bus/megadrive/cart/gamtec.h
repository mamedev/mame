// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_MEGADRIVE_CART_GAMTEC_H
#define MAME_BUS_MEGADRIVE_CART_GAMTEC_H

#pragma once

#include "machine/nvram.h"

#include "rom.h"
#include "slot.h"

class megadrive_unl_tilesmj2_device : public megadrive_rom_device
{
public:
	megadrive_unl_tilesmj2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
};

class megadrive_unl_elfwor_device : public megadrive_rom_device
{
public:
	megadrive_unl_elfwor_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
};

class megadrive_unl_smouse_device : public megadrive_rom_device
{
public:
	megadrive_unl_smouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
};

class megadrive_unl_yasech_device : public megadrive_rom_device
{
public:
	megadrive_unl_yasech_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	u16 nvram_r(offs_t offset);
	void nvram_w(offs_t offset, u16 data, u16 mem_mask);

private:
	required_device<nvram_device> m_nvram;
	std::unique_ptr<uint8_t[]> m_nvram_ptr;

};

class megadrive_unl_777casino_device : public megadrive_rom_device
{
public:
	megadrive_unl_777casino_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
};

class megadrive_unl_soulblade_device : public megadrive_rom_device
{
public:
	megadrive_unl_soulblade_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
};

class megadrive_unl_suprbubl_device : public megadrive_rom_device
{
public:
	megadrive_unl_suprbubl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
};

class megadrive_unl_cjmjclub_device : public megadrive_rom_device
{
public:
	megadrive_unl_cjmjclub_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
};

class megadrive_unl_mjlov_device : public megadrive_rom_device
{
public:
	megadrive_unl_mjlov_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
};

class megadrive_unl_redcliff_device : public device_t,
									  public device_megadrive_cart_interface
{
public:
	megadrive_unl_redcliff_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

//  bool check_rom(std::string &message) ATTR_COLD;
	memory_bank_creator m_rom;
	std::vector<u8> m_decrypted_rom;
};

class megadrive_unl_squirrelk_device : public megadrive_rom_device
{
public:
	megadrive_unl_squirrelk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
private:
	u8 m_latch;
};

class megadrive_unl_lionking2_device : public megadrive_rom_device
{
public:
	megadrive_unl_lionking2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
private:
	u8 m_latch[2];
};

class megadrive_unl_chinf3_device : public megadrive_rom_device
{
public:
	megadrive_unl_chinf3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
private:
	memory_bank_creator m_page_rom;
	memory_view m_page_view;

	u8 m_prot_latch[4];
};


DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_TILESMJ2,  megadrive_unl_tilesmj2_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_ELFWOR,    megadrive_unl_elfwor_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_SMOUSE,    megadrive_unl_smouse_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_YASECH,    megadrive_unl_yasech_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_777CASINO, megadrive_unl_777casino_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_SOULBLADE, megadrive_unl_soulblade_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_SUPRBUBL,  megadrive_unl_suprbubl_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_CJMJCLUB,  megadrive_unl_cjmjclub_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_MJLOV,     megadrive_unl_mjlov_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_REDCLIFF,  megadrive_unl_redcliff_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_SQUIRRELK, megadrive_unl_squirrelk_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_LIONKING2, megadrive_unl_lionking2_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_CHINF3,    megadrive_unl_chinf3_device)

#endif // MAME_BUS_MEGADRIVE_CART_GAMTEC_H
