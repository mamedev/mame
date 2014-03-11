#include "sound/okim6295.h"

class speedspn_state : public driver_device
{
public:
	speedspn_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_attram(*this, "attram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT8> m_attram;
	tilemap_t *m_tilemap;
	UINT8 m_display_disable;
	int m_bank_vidram;
	UINT8* m_vidram;
	DECLARE_READ8_MEMBER(speedspn_irq_ack_r);
	DECLARE_WRITE8_MEMBER(speedspn_banked_rom_change);
	DECLARE_WRITE8_MEMBER(speedspn_sound_w);
	DECLARE_WRITE8_MEMBER(speedspn_vidram_w);
	DECLARE_WRITE8_MEMBER(speedspn_attram_w);
	DECLARE_READ8_MEMBER(speedspn_vidram_r);
	DECLARE_WRITE8_MEMBER(speedspn_banked_vidram_change);
	DECLARE_WRITE8_MEMBER(speedspn_global_display_w);
	DECLARE_WRITE8_MEMBER(oki_banking_w);
	TILE_GET_INFO_MEMBER(get_speedspn_tile_info);
	virtual void video_start();
	UINT32 screen_update_speedspn(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
