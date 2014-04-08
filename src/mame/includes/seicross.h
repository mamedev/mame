#include "machine/nvram.h"

class seicross_state : public driver_device
{
public:
	seicross_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_row_scroll(*this, "row_scroll"),
		m_spriteram2(*this, "spriteram2"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_nvram(*this, "nvram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_row_scroll;
	required_shared_ptr<UINT8> m_spriteram2;
	required_shared_ptr<UINT8> m_colorram;

	UINT8 m_portb;
	tilemap_t *m_bg_tilemap;
	void nvram_init(nvram_device &nvram, void *data, size_t size);

	UINT8 m_irq_mask;
	DECLARE_WRITE8_MEMBER(seicross_videoram_w);
	DECLARE_WRITE8_MEMBER(seicross_colorram_w);
	DECLARE_READ8_MEMBER(friskyt_portB_r);
	DECLARE_WRITE8_MEMBER(friskyt_portB_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(seicross);
	UINT32 screen_update_seicross(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	optional_device<nvram_device> m_nvram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
