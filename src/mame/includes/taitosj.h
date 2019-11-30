// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

#include "machine/taitosjsec.h"

#include "machine/input_merger.h"

#include "sound/dac.h"
#include "sound/discrete.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"


class taitosj_state : public driver_device
{
public:
	taitosj_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram_1(*this, "videoram_1"),
		m_videoram_2(*this, "videoram_2"),
		m_videoram_3(*this, "videoram_3"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_characterram(*this, "characterram"),
		m_scroll(*this, "scroll"),
		m_colscrolly(*this, "colscrolly"),
		m_gfxpointer(*this, "gfxpointer"),
		m_colorbank(*this, "colorbank"),
		m_video_mode(*this, "video_mode"),
		m_video_priority(*this, "video_priority"),
		m_collision_reg(*this, "collision_reg"),
		m_kikstart_scrollram(*this, "kikstart_scroll"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "bmcu"),
		m_soundnmi(*this, "soundnmi"),
		m_soundnmi2(*this, "soundnmi2"),
		m_dac(*this, "dac"),
		m_dacvol(*this, "dacvol"),
		m_ay1(*this, "ay1"),
		m_ay2(*this, "ay2"),
		m_ay3(*this, "ay3"),
		m_ay4(*this, "ay4"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	void mcu(machine_config &config);
	void nomcu(machine_config &config);
	void kikstart(machine_config &config);

	void init_alpinea();
	void init_alpine();
	void init_taitosj();
	void init_junglhbr();
	void init_spacecr();

	DECLARE_CUSTOM_INPUT_MEMBER(input_port_4_f0_r);
	template <int Player> DECLARE_CUSTOM_INPUT_MEMBER(kikstart_gear_r);

private:
	required_shared_ptr<uint8_t> m_videoram_1;
	required_shared_ptr<uint8_t> m_videoram_2;
	required_shared_ptr<uint8_t> m_videoram_3;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_paletteram;
	required_shared_ptr<uint8_t> m_characterram;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_colscrolly;
	required_shared_ptr<uint8_t> m_gfxpointer;
	required_shared_ptr<uint8_t> m_colorbank;
	required_shared_ptr<uint8_t> m_video_mode;
	required_shared_ptr<uint8_t> m_video_priority;
	required_shared_ptr<uint8_t> m_collision_reg;
	optional_shared_ptr<uint8_t> m_kikstart_scrollram;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<taito_sj_security_mcu_device> m_mcu;
	required_device<input_merger_device> m_soundnmi;
	required_device<input_merger_device> m_soundnmi2;
	required_device<dac_8bit_r2r_device> m_dac;
	required_device<discrete_sound_device> m_dacvol;
	required_device<ay8910_device> m_ay1;
	required_device<ay8910_device> m_ay2;
	required_device<ay8910_device> m_ay3;
	required_device<ay8910_device> m_ay4;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	typedef void (taitosj_state::*copy_layer_func_t)(bitmap_ind16 &,
									const rectangle &, int, int *, rectangle *);
	uint8_t m_input_port_4_f0;
	uint8_t m_kikstart_gears[2];

	uint8_t m_spacecr_prot_value;
	uint8_t m_protection_value;
	uint32_t m_address;
	uint8_t m_soundlatch_data;
	bool m_soundlatch_flag;  // 74ls74 1/2 @ GAME BOARD IC42
	bool m_sound_semaphore2;  // 74ls74 2/2 @ GAME BOARD IC42
	bitmap_ind16 m_layer_bitmap[3];
	bitmap_ind16 m_sprite_sprite_collbitmap1;
	bitmap_ind16 m_sprite_sprite_collbitmap2;
	bitmap_ind16 m_sprite_layer_collbitmap1;
	bitmap_ind16 m_sprite_layer_collbitmap2[3];
	int m_draw_order[32][4];
	DECLARE_WRITE8_MEMBER(soundlatch_w);
	DECLARE_WRITE8_MEMBER(sound_semaphore2_w);
	TIMER_CALLBACK_MEMBER(soundlatch_w_cb);
	TIMER_CALLBACK_MEMBER(soundlatch_clear7_w_cb);
	TIMER_CALLBACK_MEMBER(sound_semaphore2_w_cb);
	TIMER_CALLBACK_MEMBER(sound_semaphore2_clear_w_cb);
	DECLARE_READ8_MEMBER(soundlatch_r);
	DECLARE_WRITE8_MEMBER(soundlatch_clear7_w);
	DECLARE_READ8_MEMBER(soundlatch_flags_r);
	DECLARE_WRITE8_MEMBER(sound_semaphore2_clear_w);
	DECLARE_WRITE8_MEMBER(taitosj_bankswitch_w);
	DECLARE_READ8_MEMBER(taitosj_fake_data_r);
	DECLARE_WRITE8_MEMBER(taitosj_fake_data_w);
	DECLARE_READ8_MEMBER(taitosj_fake_status_r);
	DECLARE_READ8_MEMBER(mcu_mem_r);
	DECLARE_WRITE8_MEMBER(mcu_mem_w);
	DECLARE_WRITE_LINE_MEMBER(mcu_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(mcu_busrq_w);
	DECLARE_READ8_MEMBER(spacecr_prot_r);
	DECLARE_WRITE8_MEMBER(alpine_protection_w);
	DECLARE_WRITE8_MEMBER(alpinea_bankswitch_w);
	DECLARE_READ8_MEMBER(alpine_port_2_r);
	DECLARE_READ8_MEMBER(taitosj_gfxrom_r);
	DECLARE_WRITE8_MEMBER(taitosj_characterram_w);
	DECLARE_WRITE8_MEMBER(junglhbr_characterram_w);
	DECLARE_WRITE8_MEMBER(taitosj_collision_reg_clear_w);

	DECLARE_WRITE8_MEMBER(taitosj_sndnmi_msk_w);
	DECLARE_WRITE8_MEMBER(input_port_4_f0_w);
	DECLARE_WRITE8_MEMBER(taitosj_dacvol_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_taitosj(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_kikstart(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void init_common();
	void reset_common();
	void set_pens();
	void compute_draw_order();
	inline int get_sprite_xy(uint8_t which, uint8_t* sx, uint8_t* sy);
	inline gfx_element *get_sprite_gfx_element(uint8_t which);
	void check_sprite_sprite_collision();
	void calculate_sprite_areas(int *sprites_on, rectangle *sprite_areas);
	int check_sprite_layer_bitpattern(int which, rectangle *sprite_areas);
	void check_sprite_layer_collision(int *sprites_on, rectangle *sprite_areas);
	void draw_layers();
	void draw_sprites(bitmap_ind16 &bitmap);
	void check_collision(int *sprites_on, rectangle *sprite_areas);
	int check_sprite_sprite_bitpattern(int sx1, int sy1, int which1,int sx2, int sy2, int which2);
	void taitosj_copy_layer(bitmap_ind16 &bitmap, const rectangle &cliprect,int which, int *sprites_on, rectangle *sprite_areas);
	void kikstart_copy_layer(bitmap_ind16 &bitmap, const rectangle &cliprect,int which, int *sprites_on, rectangle *sprite_areas);
	void copy_layer(bitmap_ind16 &bitmap, const rectangle &cliprect,copy_layer_func_t copy_layer_func, int which, int *sprites_on, rectangle *sprite_areas);
	void copy_layers(bitmap_ind16 &bitmap, const rectangle &cliprect,copy_layer_func_t copy_layer_func, int *sprites_on, rectangle *sprite_areas);
	int video_update_common(bitmap_ind16 &bitmap, const rectangle &cliprect, copy_layer_func_t copy_layer_func);

	void kikstart_main_map(address_map &map);
	void taitosj_audio_map(address_map &map);
	void taitosj_main_mcu_map(address_map &map);
	void taitosj_main_nomcu_map(address_map &map);
};
