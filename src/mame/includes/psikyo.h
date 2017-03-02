// license:BSD-3-Clause
// copyright-holders:Luca Elia,Olivier Galibert,Paul Priest
/*************************************************************************

    Psikyo Games

*************************************************************************/
#include "sound/okim6295.h"
#include "screen.h"

class psikyo_state : public driver_device
{
public:
	psikyo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_vregs(*this, "vregs"),
		m_bootleg_spritebuffer(*this, "boot_spritebuf"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint32_t> m_spriteram;
	required_shared_ptr<uint32_t> m_vram_0;
	required_shared_ptr<uint32_t> m_vram_1;
	required_shared_ptr<uint32_t> m_vregs;
	optional_shared_ptr<uint32_t> m_bootleg_spritebuffer;
	std::unique_ptr<uint32_t[]>       m_spritebuf1;
	std::unique_ptr<uint32_t[]>       m_spritebuf2;

	/* video-related */
	tilemap_t        *m_tilemap_0_size0;
	tilemap_t        *m_tilemap_0_size1;
	tilemap_t        *m_tilemap_0_size2;
	tilemap_t        *m_tilemap_0_size3;
	tilemap_t        *m_tilemap_1_size0;
	tilemap_t        *m_tilemap_1_size1;
	tilemap_t        *m_tilemap_1_size2;
	tilemap_t        *m_tilemap_1_size3;
	int            m_tilemap_0_bank;
	int            m_tilemap_1_bank;
	int            m_ka302c_banking;

	/* misc */
	uint8_t          m_soundlatch;
	int            m_z80_nmi;
	int            m_mcu_status;

	/* devices */
	optional_device<cpu_device> m_audiocpu;

	/* game-specific */
	// 1945 MCU
	uint8_t          m_s1945_mcu_direction;
	uint8_t          m_s1945_mcu_latch1;
	uint8_t          m_s1945_mcu_latch2;
	uint8_t          m_s1945_mcu_inlatch;
	uint8_t          m_s1945_mcu_index;
	uint8_t          m_s1945_mcu_latching;
	uint8_t          m_s1945_mcu_mode;
	uint8_t          m_s1945_mcu_control;
	uint8_t          m_s1945_mcu_bctrl;
	const uint8_t    *m_s1945_mcu_table;
	DECLARE_READ32_MEMBER(sngkace_input_r);
	DECLARE_READ32_MEMBER(gunbird_input_r);
	DECLARE_WRITE32_MEMBER(psikyo_soundlatch_w);
	DECLARE_WRITE32_MEMBER(s1945_soundlatch_w);
	DECLARE_WRITE32_MEMBER(s1945_mcu_w);
	DECLARE_READ32_MEMBER(s1945_mcu_r);
	DECLARE_READ32_MEMBER(s1945_input_r);
	DECLARE_READ32_MEMBER(s1945bl_oki_r);
	DECLARE_WRITE32_MEMBER(s1945bl_oki_w);
	DECLARE_READ8_MEMBER(psikyo_soundlatch_r);
	DECLARE_WRITE8_MEMBER(psikyo_clear_nmi_w);
	DECLARE_WRITE8_MEMBER(sngkace_sound_bankswitch_w);
	DECLARE_WRITE8_MEMBER(gunbird_sound_bankswitch_w);
	DECLARE_WRITE32_MEMBER(psikyo_vram_0_w);
	DECLARE_WRITE32_MEMBER(psikyo_vram_1_w);
	DECLARE_CUSTOM_INPUT_MEMBER(z80_nmi_r);
	DECLARE_CUSTOM_INPUT_MEMBER(mcu_status_r);
	DECLARE_DRIVER_INIT(s1945a);
	DECLARE_DRIVER_INIT(s1945j);
	DECLARE_DRIVER_INIT(sngkace);
	DECLARE_DRIVER_INIT(s1945);
	DECLARE_DRIVER_INIT(s1945bl);
	DECLARE_DRIVER_INIT(tengai);
	DECLARE_DRIVER_INIT(s1945jn);
	DECLARE_DRIVER_INIT(gunbird);
	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_VIDEO_START(sngkace);
	DECLARE_VIDEO_START(psikyo);
	uint32_t screen_update_psikyo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_psikyo_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_psikyo(screen_device &screen, bool state);
	TIMER_CALLBACK_MEMBER(psikyo_soundlatch_callback);
	void psikyo_switch_banks( int tmap, int bank );
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int trans_pen );
	void draw_sprites_bootleg( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int trans_pen );
	int tilemap_width( int size );
	void s1945_mcu_init(  );
	required_device<cpu_device> m_maincpu;
	optional_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
