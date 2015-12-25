// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    Irem M92 hardware

*************************************************************************/

#include "video/bufsprite.h"
#include "sound/okim6295.h"
#include "machine/pic8259.h"

struct M92_pf_layer_info
{
	tilemap_t *     tmap;
	tilemap_t *     wide_tmap;
	UINT16          vram_base;
	UINT16          control[4];
};

class m92_state : public driver_device
{
public:
	enum
	{
			TIMER_SPRITEBUFFER
	};

	m92_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_spriteram(*this, "spriteram"),
			m_vram_data(*this, "vram_data"),
			m_spritecontrol(*this, "spritecontrol"),
			m_maincpu(*this, "maincpu"),
			m_soundcpu(*this, "soundcpu"),
			m_oki(*this, "oki"),
			m_gfxdecode(*this, "gfxdecode"),
			m_screen(*this, "screen"),
			m_palette(*this, "palette"),
			m_upd71059c(*this, "upd71059c")
	{ }

	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<UINT16> m_vram_data;
	required_shared_ptr<UINT16> m_spritecontrol;
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_soundcpu;
	optional_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<pic8259_device> m_upd71059c;

	IRQ_CALLBACK_MEMBER(m92_irq_callback);

	UINT16 m_sound_status;
	UINT32 m_raster_irq_position;
	UINT16 m_videocontrol;
	UINT8 m_sprite_buffer_busy;
	UINT8 m_game_kludge;
	M92_pf_layer_info m_pf_layer[3];
	UINT16 m_pf_master_control[4];
	INT32 m_sprite_list;
	UINT8 m_palette_bank;
	std::vector<UINT16> m_paletteram;

	DECLARE_READ16_MEMBER(m92_eeprom_r);
	DECLARE_WRITE16_MEMBER(m92_eeprom_w);
	DECLARE_WRITE16_MEMBER(m92_coincounter_w);
	DECLARE_WRITE16_MEMBER(m92_bankswitch_w);
	DECLARE_WRITE16_MEMBER(m92_soundlatch_w);
	DECLARE_READ16_MEMBER(m92_sound_status_r);
	DECLARE_READ16_MEMBER(m92_soundlatch_r);
	DECLARE_WRITE16_MEMBER(m92_sound_irq_ack_w);
	DECLARE_WRITE16_MEMBER(m92_sound_status_w);
	DECLARE_WRITE16_MEMBER(m92_sound_reset_w);
	DECLARE_WRITE16_MEMBER(m92_spritecontrol_w);
	DECLARE_WRITE16_MEMBER(m92_videocontrol_w);
	DECLARE_READ16_MEMBER(m92_paletteram_r);
	DECLARE_WRITE16_MEMBER(m92_paletteram_w);
	DECLARE_WRITE16_MEMBER(m92_vram_w);
	DECLARE_WRITE16_MEMBER(m92_pf1_control_w);
	DECLARE_WRITE16_MEMBER(m92_pf2_control_w);
	DECLARE_WRITE16_MEMBER(m92_pf3_control_w);
	DECLARE_WRITE16_MEMBER(m92_master_control_w);
	DECLARE_CUSTOM_INPUT_MEMBER(m92_sprite_busy_r);
	DECLARE_WRITE16_MEMBER(oki_bank_w);
	DECLARE_DRIVER_INIT(majtitl2);
	DECLARE_DRIVER_INIT(ppan);
	DECLARE_DRIVER_INIT(lethalth);
	DECLARE_DRIVER_INIT(m92);
	DECLARE_DRIVER_INIT(m92_bank);
	TILE_GET_INFO_MEMBER(get_pf_tile_info);
	DECLARE_MACHINE_START(m92);
	DECLARE_MACHINE_RESET(m92);
	DECLARE_VIDEO_START(m92);
	DECLARE_VIDEO_START(ppan);
	UINT32 screen_update_m92(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_ppan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(m92_scanline_interrupt);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void ppan_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void m92_update_scroll_positions();
	void m92_draw_tiles(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
	void m92_sprite_interrupt();

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
