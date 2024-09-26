// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

#include "machine/input_merger.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/discrete.h"

#include "taitosjsec.h"

#include "emupal.h"
#include "screen.h"


class taitosj_state : public driver_device
{
public:
	taitosj_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram_%u", 1U),
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
		m_gfx(*this, "gfx"),
		m_mainbank(*this, "mainbank"),
		m_in2(*this, "IN2"),
		m_gear(*this, "GEARP%u", 1U),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "bmcu"),
		m_soundnmi(*this, "soundnmi%u", 1U),
		m_dac(*this, "dac"),
		m_dacvol(*this, "dacvol"),
		m_ay(*this, "ay%u", 1U),
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

	ioport_value input_port_4_f0_r();
	template <int Player> ioport_value kikstart_gear_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	required_shared_ptr_array<uint8_t, 3> m_videoram;
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
	required_region_ptr<uint8_t> m_gfx;
	required_memory_bank m_mainbank;

	required_ioport m_in2;
	optional_ioport_array<2> m_gear;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<taito_sj_security_mcu_device> m_mcu;
	required_device_array<input_merger_device, 2> m_soundnmi;
	required_device<dac_8bit_r2r_device> m_dac;
	required_device<discrete_sound_device> m_dacvol;
	required_device_array<ay8910_device, 4> m_ay;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	typedef void (taitosj_state::*copy_layer_func_t)(bitmap_ind16 &,
									const rectangle &, int, int *, rectangle *);
	uint8_t m_input_port_4_f0 = 0;
	uint8_t m_kikstart_gears[2]{};

	uint8_t m_spacecr_prot_value = 0;
	uint8_t m_protection_value = 0;
	uint32_t m_address = 0;
	uint8_t m_soundlatch_data = 0;
	bool m_soundlatch_flag = false;  // 74ls74 1/2 @ GAME BOARD IC42
	bool m_sound_semaphore2 = false;  // 74ls74 2/2 @ GAME BOARD IC42
	bitmap_ind16 m_layer_bitmap[3];
	bitmap_ind16 m_sprite_sprite_collbitmap1;
	bitmap_ind16 m_sprite_sprite_collbitmap2;
	bitmap_ind16 m_sprite_layer_collbitmap1;
	bitmap_ind16 m_sprite_layer_collbitmap2[3];
	int m_draw_order[32][4]{};
	void soundlatch_w(uint8_t data);
	void sound_semaphore2_w(uint8_t data);
	TIMER_CALLBACK_MEMBER(soundlatch_w_cb);
	TIMER_CALLBACK_MEMBER(soundlatch_clear7_w_cb);
	TIMER_CALLBACK_MEMBER(sound_semaphore2_w_cb);
	TIMER_CALLBACK_MEMBER(sound_semaphore2_clear_w_cb);
	uint8_t soundlatch_r();
	void soundlatch_clear7_w(uint8_t data);
	uint8_t soundlatch_flags_r();
	void sound_semaphore2_clear_w(uint8_t data);
	void bankswitch_w(uint8_t data);
	uint8_t fake_data_r();
	void fake_data_w(uint8_t data);
	uint8_t fake_status_r();
	uint8_t mcu_mem_r(offs_t offset);
	void mcu_mem_w(offs_t offset, uint8_t data);
	void mcu_intrq_w(int state);
	void mcu_busrq_w(int state);
	uint8_t spacecr_prot_r();
	void alpine_protection_w(uint8_t data);
	void alpinea_bankswitch_w(uint8_t data);
	uint8_t alpine_port_2_r();
	uint8_t gfxrom_r();
	void characterram_w(offs_t offset, uint8_t data);
	void junglhbr_characterram_w(offs_t offset, uint8_t data);
	void collision_reg_clear_w(uint8_t data);

	void sndnmi_msk_w(uint8_t data);
	void input_port_4_f0_w(uint8_t data);
	void dacvol_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
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
	int check_sprite_sprite_bitpattern(int sx1, int sy1, int which1, int sx2, int sy2, int which2);
	void copy_layer(bitmap_ind16 &bitmap, const rectangle &cliprect,int which, int *sprites_on, rectangle *sprite_areas);
	void kikstart_copy_layer(bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int *sprites_on, rectangle *sprite_areas);
	void copy_layer(bitmap_ind16 &bitmap, const rectangle &cliprect, copy_layer_func_t copy_layer_func, int which, int *sprites_on, rectangle *sprite_areas);
	void copy_layers(bitmap_ind16 &bitmap, const rectangle &cliprect, copy_layer_func_t copy_layer_func, int *sprites_on, rectangle *sprite_areas);
	int video_update_common(bitmap_ind16 &bitmap, const rectangle &cliprect, copy_layer_func_t copy_layer_func);

	void kikstart_main_map(address_map &map) ATTR_COLD;
	void taitosj_audio_map(address_map &map) ATTR_COLD;
	void main_mcu_map(address_map &map) ATTR_COLD;
	void main_nomcu_map(address_map &map) ATTR_COLD;
};
