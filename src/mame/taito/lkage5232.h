//
// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino

#include "taito68705.h"

#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "emupal.h"
#include "tilemap.h"


class lkage5232_state : public driver_device
{
public:
	lkage5232_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_vreg(*this, "vreg")
		, m_scroll(*this, "scroll")
		, m_spriteram(*this, "spriteram")
		, m_videoram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
	{ }

	void lkage5232(machine_config &config);

	void init_lkage5232();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_shared_ptr<uint8_t> m_vreg;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;

	/* video-related */
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_tx_tilemap = nullptr;
	uint8_t m_bg_tile_bank = 0U;
	uint8_t m_fg_tile_bank = 0U;
	uint8_t m_tx_tile_bank = 0U;

	int m_sprite_dx = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	int m_sound_nmi_enable = 0U;
	int m_pending_nmi = 0U;
	uint8_t m_exrom_offs[2] = { 0x00, 0x00 };

	void sound_nmi_disable_w(uint8_t data);
	void sound_nmi_enable_w(uint8_t data);
	void sound_command_w(uint8_t data);
	uint8_t sound_status_r(offs_t offset);
	TIMER_CALLBACK_MEMBER(sound_nmi_callback);

	void videoram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void lkage5232_map(address_map &map);
	void lkage5232_sound_map(address_map &map);

	void vreg_w(offs_t offset, uint8_t data);
	uint8_t unk_f0e1_r(offs_t offset);
	uint8_t exrom_data_r(offs_t offset);
	void exrom_offset_w(offs_t offset, uint8_t data);
	uint8_t sound_unk_e000_r(offs_t offset);
};
