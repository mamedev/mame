// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "machine/nvram.h"

class seicross_state : public driver_device
{
public:
	seicross_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_nvram(*this, "nvram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_row_scroll(*this, "row_scroll"),
		m_spriteram2(*this, "spriteram2"),
		m_colorram(*this, "colorram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	optional_device<nvram_device> m_nvram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_row_scroll;
	required_shared_ptr<UINT8> m_spriteram2;
	required_shared_ptr<UINT8> m_colorram;
	optional_shared_ptr<UINT8> m_decrypted_opcodes;

	UINT8 m_portb;
	tilemap_t *m_bg_tilemap;
	UINT8 m_irq_mask;

	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(colorram_w);
	DECLARE_READ8_MEMBER(portB_r);
	DECLARE_WRITE8_MEMBER(portB_w);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	INTERRUPT_GEN_MEMBER(vblank_irq);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(seicross);
	DECLARE_DRIVER_INIT(friskytb);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );

	void nvram_init(nvram_device &nvram, void *data, size_t size);
};
