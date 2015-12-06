// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
#include "sound/dac.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"

class senjyo_state : public driver_device
{
public:
	senjyo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pio(*this, "z80pio"),
		m_dac(*this, "dac"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_radar_palette(*this, "radar_palette"),
		m_spriteram(*this, "spriteram"),
		m_fgscroll(*this, "fgscroll"),
		m_scrollx1(*this, "scrollx1"),
		m_scrolly1(*this, "scrolly1"),
		m_scrollx2(*this, "scrollx2"),
		m_scrolly2(*this, "scrolly2"),
		m_scrollx3(*this, "scrollx3"),
		m_scrolly3(*this, "scrolly3"),
		m_fgvideoram(*this, "fgvideoram"),
		m_fgcolorram(*this, "fgcolorram"),
		m_bg1videoram(*this, "bg1videoram"),
		m_bg2videoram(*this, "bg2videoram"),
		m_bg3videoram(*this, "bg3videoram"),
		m_radarram(*this, "radarram"),
		m_bgstripesram(*this, "bgstripesram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio;
	required_device<dac_device> m_dac;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<palette_device> m_radar_palette;

	/* memory pointers */
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_fgscroll;
	required_shared_ptr<UINT8> m_scrollx1;
	required_shared_ptr<UINT8> m_scrolly1;
	required_shared_ptr<UINT8> m_scrollx2;
	required_shared_ptr<UINT8> m_scrolly2;
	required_shared_ptr<UINT8> m_scrollx3;
	required_shared_ptr<UINT8> m_scrolly3;
	required_shared_ptr<UINT8> m_fgvideoram;
	required_shared_ptr<UINT8> m_fgcolorram;
	required_shared_ptr<UINT8> m_bg1videoram;
	required_shared_ptr<UINT8> m_bg2videoram;
	required_shared_ptr<UINT8> m_bg3videoram;
	required_shared_ptr<UINT8> m_radarram;
	required_shared_ptr<UINT8> m_bgstripesram;
	optional_shared_ptr<UINT8> m_decrypted_opcodes;

	// game specific initialization
	int m_is_senjyo;
	int m_scrollhack;

	UINT8 m_sound_cmd;
	int m_single_volume;
	int m_sound_state;
	int m_bgstripes;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg1_tilemap;
	tilemap_t *m_bg2_tilemap;
	tilemap_t *m_bg3_tilemap;

	DECLARE_WRITE8_MEMBER(flip_screen_w);
	DECLARE_WRITE8_MEMBER(starforb_scrolly2);
	DECLARE_WRITE8_MEMBER(starforb_scrollx2);
	DECLARE_WRITE8_MEMBER(fgvideoram_w);
	DECLARE_WRITE8_MEMBER(fgcolorram_w);
	DECLARE_WRITE8_MEMBER(bg1videoram_w);
	DECLARE_WRITE8_MEMBER(bg2videoram_w);
	DECLARE_WRITE8_MEMBER(bg3videoram_w);
	DECLARE_WRITE8_MEMBER(volume_w);
	DECLARE_WRITE_LINE_MEMBER(sound_line_clock);
	DECLARE_WRITE8_MEMBER(sound_cmd_w);
	DECLARE_WRITE8_MEMBER(irq_ctrl_w);
	DECLARE_READ8_MEMBER(pio_pa_r);

	DECLARE_PALETTE_DECODER(IIBBGGRR);
	DECLARE_PALETTE_INIT(radar);

	DECLARE_DRIVER_INIT(starfora);
	DECLARE_DRIVER_INIT(senjyo);
	DECLARE_DRIVER_INIT(starfore);
	DECLARE_DRIVER_INIT(starforc);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(senjyo_bg1_tile_info);
	TILE_GET_INFO_MEMBER(starforc_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_bg3_tile_info);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_bgbitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_radar(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect,int priority);
};

/*----------- defined in audio/senjyo.c -----------*/
extern const z80_daisy_config senjyo_daisy_chain[];
