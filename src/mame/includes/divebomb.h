// license:BSD-3-Clause
// copyright-holders:David Haywood, Phil Bennett
/*************************************************************************

    Kyuukoukabakugekitai - Dive Bomber Squad

*************************************************************************/

#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "video/k051316.h"

#define XTAL1 XTAL_24MHz

class divebomb_state : public driver_device
{
public:
	divebomb_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
	m_spritecpu(*this, "spritecpu"),
	m_fgcpu(*this, "fgcpu"),
	m_rozcpucpu(*this, "rozcpu"),
	m_bank1(*this, "bank1"),
	m_fgram(*this, "fgram"),
	m_spriteram(*this, "spriteram"),
	m_gfxdecode(*this, "gfxdecode"),
	m_palette(*this, "palette"),
	m_k051316_1(*this, "k051316_1"),
	m_k051316_2(*this, "k051316_2")
	{ }

	required_device<cpu_device> m_spritecpu;
	required_device<cpu_device> m_fgcpu;
	required_device<cpu_device> m_rozcpucpu;
	required_memory_bank m_bank1;
	required_shared_ptr<uint8_t> m_fgram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<k051316_device> m_k051316_1;
	required_device<k051316_device> m_k051316_2;

	tilemap_t *m_fg_tilemap;
	uint8_t to_spritecpu;
	uint8_t to_rozcpu;
	uint8_t from_sprite;
	uint8_t from_roz;
	bool has_fromsprite;
	bool has_fromroz;

	uint8_t roz_pal;
	bool roz1_enable;
	bool roz2_enable;
	bool roz1_wrap;
	bool roz2_wrap;

	void machine_reset_divebomb();
	void machine_start_divebomb();
	void video_start_divebomb();
	void palette_init_divebomb(palette_device &palette);

	void update_irqs();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void decode_proms(const uint8_t* rgn, int size, int index, bool inv);
	uint32_t screen_update_divebomb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	K051316_CB_MEMBER(zoom_callback_1);
	K051316_CB_MEMBER(zoom_callback_2);

	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	uint8_t fgcpu_roz_comm_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fgcpu_roz_comm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t fgcpu_spr_comm_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fgcpu_spr_comm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t fgcpu_comm_flags_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fgram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void spritecpu_port00_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t spritecpu_comm_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void spritecpu_comm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void rozcpu_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rozcpu_wrap1_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rozcpu_enable1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rozcpu_enable2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rozcpu_wrap2_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t rozcpu_comm_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void rozcpu_comm_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rozcpu_pal_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
};
