// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood

#ifndef __NEOGEO_INTF_H
#define __NEOGEO_INTF_H

// ======================> device_neogeo_cart_interface

#define DECRYPT_ALL_PARAMS \
	UINT8* cpuregion, UINT32 cpuregion_size,UINT8* spr_region, UINT32 spr_region_size,UINT8* fix_region, UINT32 fix_region_size,UINT8* ym_region, UINT32 ym_region_size,UINT8* ymdelta_region, UINT32 ymdelta_region_size,UINT8* audiocpu_region, UINT32 audio_region_size, UINT8* audiocrypt_region, UINT32 audiocrypt_region_size

#define ACTIVATE_CART_PARAMS \
	running_machine& machine, cpu_device* maincpu, UINT8* cpuregion, UINT32 cpuregion_size, UINT8* fixedregion, UINT32 fixedregion_size

class device_neogeo_cart_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_neogeo_cart_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_neogeo_cart_interface();

	// reading from ROM
	virtual DECLARE_READ16_MEMBER(read_rom) { return 0xffff; }
	virtual void activate_cart(ACTIVATE_CART_PARAMS) { };
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) { };
	virtual int get_fixed_bank_type(void) { return 0; }

	void rom_alloc(UINT32 size) { m_rom.resize(size/sizeof(UINT16)); }
	UINT16* get_rom_base() { return &m_rom[0]; }
	UINT32  get_rom_size() { return m_rom.size()*sizeof(UINT16); }

	void fixed_alloc(UINT32 size) { m_fixed.resize(size); }
	UINT8* get_fixed_base() { return &m_fixed[0]; }
	UINT32  get_fixed_size() { return m_fixed.size(); }

	void audio_alloc(UINT32 size) { m_audio.resize(size); }
	UINT8* get_audio_base() { return &m_audio[0]; }
	UINT32  get_audio_size() { return m_audio.size(); }

	void audiocrypt_alloc(UINT32 size) { m_audiocrypt.resize(size); }
	UINT8* get_audiocrypt_base() { return &m_audiocrypt[0]; }
	UINT32  get_audiocrypt_size() { return m_audiocrypt.size(); }

	void sprites_alloc(UINT32 size) { m_sprites.resize(size); }
	UINT8* get_sprites_base() { return &m_sprites[0]; }
	UINT32  get_sprites_size() { return m_sprites.size(); }
	UINT8* get_sprites_optimized() { return &m_sprites_optimized[0]; }
	UINT32 get_sprites_addrmask() { return m_sprite_gfx_address_mask; }
	std::vector<UINT8>& get_sprites_optimized_arr() { return m_sprites_optimized; }

	void ym_alloc(UINT32 size) { m_ym.resize(size); }
	UINT8* get_ym_base() { return &m_ym[0]; }
	UINT32  get_ym_size() { return m_ym.size(); }

	void ymdelta_alloc(UINT32 size) { m_ymdelta.resize(size); }
	UINT8* get_ymdelta_base() { return &m_ymdelta[0]; }
	UINT32  get_ymdelta_size() { return m_ymdelta.size(); }

	std::vector<UINT16> m_rom;
	std::vector<UINT8> m_fixed;
	std::vector<UINT8> m_sprites;
	std::vector<UINT8> m_sprites_optimized;
	std::vector<UINT8> m_audio;
	std::vector<UINT8> m_ym;
	std::vector<UINT8> m_ymdelta;

	UINT32 m_sprite_gfx_address_mask;



protected:
	// internal state
	std::vector<UINT8> m_audiocrypt;


};

#endif
