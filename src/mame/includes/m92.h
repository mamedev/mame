// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    Irem M92 hardware

*************************************************************************/

#include "video/bufsprite.h"
#include "sound/okim6295.h"
#include "machine/gen_latch.h"
#include "machine/pic8259.h"
#include "machine/timer.h"
#include "screen.h"

struct M92_pf_layer_info
{
	tilemap_t *     tmap;
	tilemap_t *     wide_tmap;
	uint16_t          vram_base;
	uint16_t          control[4];
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
			m_upd71059c(*this, "upd71059c"),
			m_soundlatch(*this, "soundlatch")
	{ }

	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<uint16_t> m_vram_data;
	required_shared_ptr<uint16_t> m_spritecontrol;
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_soundcpu;
	optional_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<pic8259_device> m_upd71059c;
	optional_device<generic_latch_8_device> m_soundlatch;

	emu_timer *m_spritebuffer_timer;
	uint16_t m_sound_status;
	uint32_t m_raster_irq_position;
	uint16_t m_videocontrol;
	uint8_t m_sprite_buffer_busy;
	M92_pf_layer_info m_pf_layer[3];
	uint16_t m_pf_master_control[4];
	int32_t m_sprite_list;
	uint8_t m_palette_bank;
	std::vector<uint16_t> m_paletteram;

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
	uint32_t screen_update_m92(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ppan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(m92_scanline_interrupt);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void ppan_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void m92_update_scroll_positions();
	void m92_draw_tiles(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);

	void m92(machine_config &config);
	void inthunt(machine_config &config);
	void lethalth(machine_config &config);
	void ppan(machine_config &config);
	void hook(machine_config &config);
	void psoldier(machine_config &config);
	void rtypeleo(machine_config &config);
	void gunforc2(machine_config &config);
	void nbbatman2bl(machine_config &config);
	void bmaster(machine_config &config);
	void nbbatman(machine_config &config);
	void uccops(machine_config &config);
	void dsoccr94j(machine_config &config);
	void gunforce(machine_config &config);
	void majtitl2(machine_config &config);
	void mysticri(machine_config &config);
	void lethalth_map(address_map &map);
	void m92_map(address_map &map);
	void m92_portmap(address_map &map);
	void ppan_portmap(address_map &map);
	void sound_map(address_map &map);
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
