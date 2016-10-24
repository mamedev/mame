// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for early Williams games

**************************************************************************/


#include "cpu/m6809/m6809.h"
#include "cpu/m6800/m6800.h"
#include "sound/hc55516.h"
#include "machine/ticket.h"
#include "machine/watchdog.h"
#include "machine/6821pia.h"
#include "machine/bankdev.h"
#include "audio/williams.h"

class williams_state : public driver_device
{
public:
	williams_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_nvram(*this, "nvram"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_bankc000(*this, "bankc000"),
		m_watchdog(*this, "watchdog"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_generic_paletteram_8(*this, "paletteram") { }

	enum
	{
		//controlbyte (0xCA00) bit definitions
		WMS_BLITTER_CONTROLBYTE_NO_EVEN = 0x80,
		WMS_BLITTER_CONTROLBYTE_NO_ODD = 0x40,
		WMS_BLITTER_CONTROLBYTE_SHIFT = 0x20,
		WMS_BLITTER_CONTROLBYTE_SOLID = 0x10,
		WMS_BLITTER_CONTROLBYTE_FOREGROUND_ONLY = 0x08,
		WMS_BLITTER_CONTROLBYTE_SLOW = 0x04, //2us blits instead of 1us
		WMS_BLITTER_CONTROLBYTE_DST_STRIDE_256 = 0x02,
		WMS_BLITTER_CONTROLBYTE_SRC_STRIDE_256 = 0x01
	};

	required_shared_ptr<uint8_t> m_nvram;
	required_shared_ptr<uint8_t> m_videoram;
	uint8_t *m_mayday_protection;
	uint8_t m_blitter_config;
	uint16_t m_blitter_clip_address;
	uint8_t m_blitter_window_enable;
	uint8_t m_cocktail;
	uint8_t m_port_select;
	std::unique_ptr<rgb_t[]> m_palette_lookup;
	uint8_t m_blitterram[8];
	uint8_t m_blitter_xor;
	uint8_t m_blitter_remap_index;
	const uint8_t *m_blitter_remap;
	std::unique_ptr<uint8_t[]> m_blitter_remap_lookup;
	void williams_vram_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void williams_cmos_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bubbles_cmos_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void williams_watchdog_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void defender_video_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void defender_bank_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mayday_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sinistar_vram_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t williams_video_counter_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void williams_blitter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value williams_mux_r(ioport_field &field, void *param);
	void init_sinistar();
	void init_stargate();
	void init_playball();
	void init_defender();
	void init_mayday();
	void init_lottofun();
	void init_alienaru();
	void init_defndjeu();
	void init_spdball();
	void init_splat();
	void init_joust();
	void init_alienar();
	void init_robotron();
	void init_bubbles();
	void machine_start_defender();
	void machine_reset_defender();
	void video_start_williams();
	void machine_start_williams();
	void machine_reset_williams();
	void machine_start_williams_common();
	void machine_reset_williams_common();
	uint32_t screen_update_williams(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void williams_count240_off_callback(void *ptr, int32_t param);
	void williams_deferred_snd_cmd_w(void *ptr, int32_t param);
	void williams_va11_callback(timer_device &timer, void *ptr, int32_t param);
	void williams_count240_callback(timer_device &timer, void *ptr, int32_t param);
	void williams_snd_cmd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void playball_snd_cmd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void williams_port_select_w(int state);
	uint8_t williams_49way_port_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t williams_input_port_49way_0_5_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lottofun_coin_lock_w(int state);

	void state_save_register();
	void create_palette_lookup();
	void blitter_init(int blitter_config, const uint8_t *remap_prom);
	inline void blit_pixel(address_space &space, int dstaddr, int srcdata, int controlbyte);
	int blitter_core(address_space &space, int sstart, int dstart, int w, int h, int data);

	void williams_main_irq(int state);
	void williams_main_firq(int state);
	void williams_snd_irq(int state);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<address_map_bank_device> m_bankc000;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	optional_shared_ptr<uint8_t> m_generic_paletteram_8;
};


class blaster_state : public williams_state
{
public:
	blaster_state(const machine_config &mconfig, device_type type, const char *tag)
		: williams_state(mconfig, type, tag),
		m_soundcpu_b(*this, "soundcpu_b"),
		m_blaster_palette_0(*this, "blaster_pal0"),
		m_blaster_scanline_control(*this, "blaster_scan") { }

	optional_device<cpu_device> m_soundcpu_b;
	required_shared_ptr<uint8_t> m_blaster_palette_0;
	required_shared_ptr<uint8_t> m_blaster_scanline_control;

	rgb_t m_blaster_color0;
	uint8_t m_blaster_video_control;
	uint8_t m_vram_bank;
	uint8_t m_rom_bank;

	void blaster_vram_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blaster_bank_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blaster_remap_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blaster_video_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void blaster_deferred_snd_cmd_w(void *ptr, int32_t param);
	void blaster_snd_cmd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void williams_snd_irq_b(int state);

	void init_blaster();
	void machine_start_blaster();
	void machine_reset_blaster();
	void video_start_blaster();
	uint32_t screen_update_blaster(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	inline void update_blaster_banking();
};


class williams2_state : public williams_state
{
public:
	williams2_state(const machine_config &mconfig, device_type type, const char *tag)
		: williams_state(mconfig, type, tag),
		m_bank8000(*this, "bank8000"),
		m_gfxdecode(*this, "gfxdecode"),
		m_williams2_tileram(*this, "williams2_tile") { }

	required_device<address_map_bank_device> m_bank8000;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint8_t> m_williams2_tileram;

	tilemap_t *m_bg_tilemap;
	uint16_t m_tilemap_xscroll;
	uint8_t m_williams2_fg_color;
	uint8_t m_williams2_tilemap_config;

	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void williams2_bank_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void williams2_watchdog_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void williams2_7segment_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void williams2_paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void williams2_fg_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void williams2_bg_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void williams2_tileram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void williams2_xscroll_low_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void williams2_xscroll_high_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void williams2_blit_window_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void williams2_endscreen_off_callback(void *ptr, int32_t param);
	void williams2_va11_callback(timer_device &timer, void *ptr, int32_t param);
	void williams2_endscreen_callback(timer_device &timer, void *ptr, int32_t param);
	void williams2_deferred_snd_cmd_w(void *ptr, int32_t param);
	void williams2_snd_cmd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mysticm_main_irq(int state);
	void tshoot_main_irq(int state);
	uint8_t tshoot_input_port_0_3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tshoot_maxvol_w(int state);
	void tshoot_lamp_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_mysticm();
	void init_tshoot();
	void init_inferno();
	void machine_start_williams2();
	void machine_reset_williams2();
	void video_start_williams2();
	uint32_t screen_update_williams2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


class joust2_state : public williams2_state
{
public:
	joust2_state(const machine_config &mconfig, device_type type, const char *tag)
		: williams2_state(mconfig, type, tag),
		m_cvsd_sound(*this, "cvsd") { }

	required_device<williams_cvsd_sound_device> m_cvsd_sound;
	uint16_t m_joust2_current_sound_data;

	void init_joust2();
	void machine_start_joust2();
	void machine_reset_joust2();
	void joust2_deferred_snd_cmd_w(void *ptr, int32_t param);
	void joust2_snd_cmd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void joust2_pia_3_cb1_w(int state);
};

/*----------- defined in video/williams.c -----------*/

#define WILLIAMS_BLITTER_NONE       0       /* no blitter */
#define WILLIAMS_BLITTER_SC01       1       /* SC-01 blitter */
#define WILLIAMS_BLITTER_SC02       2       /* SC-02 "fixed" blitter */

#define WILLIAMS_TILEMAP_MYSTICM    0       /* IC79 is a 74LS85 comparator */
#define WILLIAMS_TILEMAP_TSHOOT     1       /* IC79 is a 74LS157 selector jumpered to be enabled */
#define WILLIAMS_TILEMAP_JOUST2     2       /* IC79 is a 74LS157 selector jumpered to be disabled */
