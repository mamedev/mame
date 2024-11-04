// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef MAME_BUS_NEOGEO_SLOT_H
#define MAME_BUS_NEOGEO_SLOT_H

#pragma once

#include "imagedev/cartrom.h"


/* PCB */
enum
{
	NEOGEO_STD = 0,
	NEOGEO_FATFURY2,
	NEOGEO_KOF98,
	NEOGEO_MSLUGX,
	NEOGEO_ZUPAPA,
	NEOGEO_MSLUG3H,
	NEOGEO_GANRYU,
	NEOGEO_S1945P,
	NEOGEO_PREIS2,
	NEOGEO_BANGBD,
	NEOGEO_NITD,
	NEOGEO_SENGOK3,
	NEOGEO_KOF99K,
	NEOGEO_KOF2K1,
	NEOGEO_KOF2KN,
	NEOGEO_KOF99,
	NEOGEO_GAROU,
	NEOGEO_GAROUH,
	NEOGEO_MSLUG3,
	NEOGEO_MSLUG3A,
	NEOGEO_KOF2K,
	NEOGEO_MSLUG4,
	NEOGEO_MSLUG4P,
	NEOGEO_ROTD,
	NEOGEO_PNYAA,
	NEOGEO_KOF2K2,
	NEOGEO_MATRIM,
	NEOGEO_SAMSHO5,
	NEOGEO_SAMSHO5S,
	NEOGEO_KOF2K2P,
	NEOGEO_MSLUG5,
	NEOGEO_SVC,
	NEOGEO_KOF2K3,
	NEOGEO_KOF2K3H,
	NEOGEO_CTHD2K3,
	NEOGEO_CT2K3SP,
	NEOGEO_CT2K3SA,
	NEOGEO_MATRIMBL,
	NEOGEO_SVCBOOT,
	NEOGEO_SVCPLUS,
	NEOGEO_SVCPLUSA,
	NEOGEO_SVCSPLUS,
	NEOGEO_KOF2K2B,
	NEOGEO_KOF2K2MP,
	NEOGEO_KOF2K2MP2,
	NEOGEO_KOF2K3B,
	NEOGEO_KOF2K3P,
	NEOGEO_KOF2K3UP,
	NEOGEO_GAROUBL,
	NEOGEO_KOF97ORO,
	NEOGEO_KF10THEP,
	NEOGEO_KF2K5UNI,
	NEOGEO_KF2K4SE,
	NEOGEO_LANS2K4,
	NEOGEO_SAMSHO5B,
	NEOGEO_MSLUG3B6,
	NEOGEO_MSLUG5P,
	NEOGEO_MSLUG5B,
	NEOGEO_KOG,
	NEOGEO_SBP,
	NEOGEO_KOF10TH,
	NEOGEO_VLINER,
	NEOGEO_JOCKEYGP,
};


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

// ======================> device_neogeo_cart_interface

#define DECRYPT_ALL_PARAMS \
	uint8_t* cpuregion, uint32_t cpuregion_size,uint8_t* spr_region, uint32_t spr_region_size,uint8_t* fix_region, uint32_t fix_region_size,uint8_t* ym_region, uint32_t ym_region_size,uint8_t* ymdelta_region, uint32_t ymdelta_region_size,uint8_t* audiocpu_region, uint32_t audio_region_size, uint8_t* audiocrypt_region, uint32_t audiocrypt_region_size

class device_neogeo_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_neogeo_cart_interface();

	// reading from ROM
	virtual uint16_t rom_r(offs_t offset) { return 0xffff; }
	virtual void banksel_w(uint16_t data) { }
	virtual uint16_t ram_r(offs_t offset) { return 0xffff; }
	virtual void ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { }
	virtual uint16_t protection_r(address_space &space, offs_t offset) { return 0xffff; }
	virtual void protection_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { }
	virtual uint16_t addon_r(offs_t offset) { return 0xffff; }
	virtual uint32_t get_bank_base(uint16_t sel) { return 0; }
	virtual uint32_t get_special_bank() { return 0; }
	virtual uint16_t get_helper() { return 0; }

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) { }
	virtual int get_fixed_bank_type() { return 0; }

	void rom_alloc(uint32_t size) { m_rom.resize(size/sizeof(uint16_t)); }
	uint16_t* get_rom_base()  { return m_rom.size() > 0 ? &m_rom[0] : nullptr; }
	uint32_t get_rom_size() { return m_rom.size() * sizeof(uint16_t); }
	uint16_t* get_region_rom_base()  { if (m_region_rom.found()) return &m_region_rom->as_u16(0); return nullptr; }
	uint32_t get_region_rom_size() { if (m_region_rom.found()) return m_region_rom->bytes(); return 0; }

	void fixed_alloc(uint32_t size) { m_fixed.resize(size); }
	uint8_t* get_fixed_base() { return m_fixed.size() > 0 ? &m_fixed[0] : nullptr; }
	uint32_t get_fixed_size() { return m_fixed.size(); }
	uint8_t* get_region_fixed_base()  { if (m_region_fixed.found()) return m_region_fixed->base(); return nullptr; }
	uint32_t get_region_fixed_size() { if (m_region_fixed.found()) return m_region_fixed->bytes(); return 0; }

	void audio_alloc(uint32_t size) { m_audio.resize(size); }
	uint8_t* get_audio_base() { return m_audio.size() > 0 ? &m_audio[0] : nullptr; }
	uint32_t get_audio_size() { return m_audio.size(); }
	uint8_t* get_region_audio_base()  { if (m_region_audio.found()) return m_region_audio->base(); return nullptr; }
	uint32_t get_region_audio_size() { if (m_region_audio.found()) return m_region_audio->bytes(); return 0; }

	void audiocrypt_alloc(uint32_t size) { m_audiocrypt.resize(size); }
	uint8_t* get_audiocrypt_base() { return m_audiocrypt.size() > 0 ? &m_audiocrypt[0] : nullptr; }
	uint32_t get_audiocrypt_size() { return m_audiocrypt.size(); }
	uint8_t* get_region_audiocrypt_base()  { if (m_region_audiocrypt.found()) return m_region_audiocrypt->base(); return nullptr; }
	uint32_t get_region_audiocrypt_size() { if (m_region_audiocrypt.found()) return m_region_audiocrypt->bytes(); return 0; }

	// TODO: review sprite code later!!
	void sprites_alloc(uint32_t size) { m_sprites.resize(size); }
	uint8_t* get_sprites_base() { return m_sprites.size() > 0 ? &m_sprites[0] : nullptr; }
	uint32_t get_sprites_size() { return m_sprites.size(); }
	uint8_t* get_region_sprites_base()  { if (m_region_spr.found()) return m_region_spr->base(); return nullptr; }
	uint32_t get_region_sprites_size() { if (m_region_spr.found()) return m_region_spr->bytes(); return 0; }

	void ym_alloc(uint32_t size) { m_ym.resize(size); }
	uint8_t* get_ym_base() { return m_ym.size() > 0 ? &m_ym[0] : nullptr; }
	uint32_t get_ym_size() { return m_ym.size(); }
	uint8_t* get_region_ym_base()  { if (m_region_ym.found()) return m_region_ym->base(); return nullptr; }
	uint32_t get_region_ym_size() { if (m_region_ym.found()) return m_region_ym->bytes(); return 0; }

	void ymdelta_alloc(uint32_t size) { m_ymdelta.resize(size); }
	uint8_t* get_ymdelta_base() { return m_ymdelta.size() > 0 ? &m_ymdelta[0] : nullptr; }
	uint32_t get_ymdelta_size() { return m_ymdelta.size(); }
	uint8_t* get_region_ymdelta_base()  { if (m_region_ymd.found()) return m_region_ymd->base(); return nullptr; }
	uint32_t get_region_ymdelta_size() { if (m_region_ymd.found()) return m_region_ymd->bytes(); return 0; }

	// this is only used to setup optimized sprites when loading on multi-slot drivers from softlist
	// therefore, we do not need a separate region accessor!
	uint8_t* get_sprites_opt_base() { return m_sprites_opt.size() > 0 ? &m_sprites_opt[0] : nullptr; }
	uint32_t get_sprites_opt_size() { return m_sprites_opt.size(); }
	void optimize_sprites(uint8_t* region_sprites, uint32_t region_sprites_size);

protected:
	device_neogeo_cart_interface(const machine_config &mconfig, device_t &device);

	// these are allocated when loading from softlist
	std::vector<uint16_t> m_rom;
	std::vector<uint8_t> m_fixed;
	std::vector<uint8_t> m_audio;
	std::vector<uint8_t> m_sprites;
	std::vector<uint8_t> m_ym;
	std::vector<uint8_t> m_ymdelta;
	std::vector<uint8_t> m_audiocrypt;

	std::vector<uint8_t> m_sprites_opt;

	// these replace m_rom, etc. above for non-user configurable carts!
	optional_memory_region  m_region_rom;
	optional_memory_region  m_region_fixed;
	optional_memory_region  m_region_audio;
	optional_memory_region  m_region_audiocrypt;
	optional_memory_region  m_region_spr;
	optional_memory_region  m_region_ym;
	optional_memory_region  m_region_ymd;

	uint32_t get_region_mask(uint8_t* rgn, uint32_t rgn_size);
};


// ======================> neogeo_cart_slot_device

class neogeo_cart_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_neogeo_cart_interface>
{
public:
	// construction/destruction
	template <typename T>
	neogeo_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: neogeo_cart_slot_device(mconfig, tag, owner, (uint16_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	neogeo_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock);
	virtual ~neogeo_cart_slot_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "neo_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	// reading and writing
	uint16_t rom_r(offs_t offset);
	void banksel_w(uint16_t data);
	uint16_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t protection_r(address_space &space, offs_t offset);
	void protection_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t addon_r(offs_t offset);

	void set_cart_type(const char *slot);
	int get_type() { return m_type; }

	uint16_t* get_rom_base()  {
		if (m_cart) {
			if (!user_loadable()) return m_cart->get_region_rom_base(); else return m_cart->get_rom_base();
		}
		return nullptr;
	}
	uint32_t get_rom_size()   {
		if (m_cart) {
			if (!user_loadable()) return m_cart->get_region_rom_size(); else return m_cart->get_rom_size();
		}
		return 0;
	}
	uint8_t* get_fixed_base()  {
		if (m_cart) {
			if (!user_loadable()) return m_cart->get_region_fixed_base(); else return m_cart->get_fixed_base();
		}
		return nullptr;
	}
	uint32_t get_fixed_size() {
		if (m_cart) {
			if (!user_loadable()) return m_cart->get_region_fixed_size(); else return m_cart->get_fixed_size();
		}
		return 0;
	}
	uint8_t* get_sprites_base()  {
		if (m_cart) {
			if (!user_loadable()) return m_cart->get_region_sprites_base(); else return m_cart->get_sprites_base();
		}
		return nullptr;
	}
	uint32_t get_sprites_size()   {
		if (m_cart) {
			if (!user_loadable()) return m_cart->get_region_sprites_size(); else return m_cart->get_sprites_size();
		}
		return 0;
	}
	uint8_t* get_audio_base()  {
		if (m_cart) {
			if (!user_loadable()) return m_cart->get_region_audio_base(); else return m_cart->get_audio_base();
		}
		return nullptr;
	}
	uint32_t get_audio_size() {
		if (m_cart) {
			if (!user_loadable()) return m_cart->get_region_audio_size(); else return m_cart->get_audio_size();
		}
		return 0;
	}
	uint8_t* get_audiocrypt_base()  {
		if (m_cart) {
			if (!user_loadable()) return m_cart->get_region_audiocrypt_base(); else return m_cart->get_audiocrypt_base();
		}
		return nullptr;
	}
	uint32_t get_audiocrypt_size()    {
		if (m_cart) {
			if (!user_loadable()) return m_cart->get_region_audiocrypt_size(); else return m_cart->get_audiocrypt_size();
		}
		return 0;
	}
	uint8_t* get_ym_base()  {
		if (m_cart) {
			if (!user_loadable()) return m_cart->get_region_ym_base(); else return m_cart->get_ym_base();
		}
		return nullptr;
	}
	uint32_t get_ym_size()    {
		if (m_cart) {
			if (!user_loadable()) return m_cart->get_region_ym_size(); else return m_cart->get_ym_size();
		}
		return 0;
	}
	uint8_t* get_ymdelta_base()  {
		if (m_cart) {
			if (!user_loadable()) return m_cart->get_region_ymdelta_base(); else return m_cart->get_ymdelta_base();
		}
		return nullptr;
	}
	uint32_t get_ymdelta_size()   {
		if (m_cart) {
			if (!user_loadable()) return m_cart->get_region_ymdelta_size(); else return m_cart->get_ymdelta_size();
		}
		return 0;
	}

	uint8_t* get_sprites_opt_base() { return m_cart ? m_cart->get_sprites_opt_base() : nullptr; }
	uint32_t get_sprites_opt_size() { return m_cart ? m_cart->get_sprites_opt_size() : 0; }

	int get_fixed_bank_type()            { return m_cart ? m_cart->get_fixed_bank_type() : 0; }
	uint32_t get_bank_base(uint16_t sel) { return m_cart ? m_cart->get_bank_base(sel) : 0; }
	uint32_t get_special_bank()          { return m_cart ? m_cart->get_special_bank() : 0; }
	uint16_t get_helper()                { return m_cart ? m_cart->get_helper() : 0; }

	void late_decrypt_all() { if (m_cart) m_cart->decrypt_all(
										(uint8_t*)get_rom_base(), get_rom_size(),
										get_sprites_base(), get_sprites_size(),
										get_fixed_base(), get_fixed_size(),
										get_ym_base(), get_ym_size(),
										get_ymdelta_base(), get_ymdelta_size(),
										get_audio_base(), get_audio_size(),
										get_audiocrypt_base(), get_audiocrypt_size());  }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	int m_type;
	device_neogeo_cart_interface*       m_cart;
};


// device type definition
DECLARE_DEVICE_TYPE(NEOGEO_CART_SLOT, neogeo_cart_slot_device)

#endif // MAME_BUS_NEOGEO_SLOT_H
