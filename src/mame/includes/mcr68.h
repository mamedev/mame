// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Bryan McPhail
/***************************************************************************

    Midway MCR-68k system

***************************************************************************/

#include "machine/6821pia.h"
#include "machine/watchdog.h"
#include "audio/midway.h"
#include "audio/williams.h"

struct counter_state
{
	uint8_t           control;
	uint16_t          latch;
	uint16_t          count;
	emu_timer *     timer;
	uint8_t           timer_active;
	attotime        period;
};

class mcr68_state : public driver_device
{
public:
	mcr68_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_chip_squeak_deluxe(*this, "csd"),
		m_sounds_good(*this, "sg"),
		m_turbo_chip_squeak(*this, "tcs"),
		m_cvsd_sound(*this, "cvsd"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram") ,
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	optional_device<midway_chip_squeak_deluxe_device> m_chip_squeak_deluxe;
	optional_device<midway_sounds_good_device> m_sounds_good;
	optional_device<midway_turbo_chip_squeak_device> m_turbo_chip_squeak;
	optional_device<williams_cvsd_sound_device> m_cvsd_sound;

	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_spriteram;
	uint16_t m_control_word;
	uint8_t m_protection_data[5];
	attotime m_timing_factor;
	uint8_t m_sprite_clip;
	int8_t m_sprite_xoffset;
	uint8_t m_m6840_status;
	uint8_t m_m6840_status_read_since_int;
	uint8_t m_m6840_msb_buffer;
	uint8_t m_m6840_lsb_buffer;
	uint8_t m_m6840_irq_state;
	uint8_t m_m6840_irq_vector;
	struct counter_state m_m6840_state[3];
	uint8_t m_v493_irq_state;
	uint8_t m_v493_irq_vector;
	timer_expired_delegate m_v493_callback;
	uint8_t m_zwackery_sound_data;
	attotime m_m6840_counter_periods[3];
	attotime m_m6840_internal_counter_period;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	uint16_t zwackery_6840_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void xenophobe_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void blasted_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t spyhunt2_port_0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t spyhunt2_port_1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void spyhunt2_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t archrivl_port_1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void archrivl_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void pigskin_protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t pigskin_protection_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t pigskin_port_1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t pigskin_port_2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t trisport_port_1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mcr68_6840_upper_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mcr68_6840_lower_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t mcr68_6840_upper_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t mcr68_6840_lower_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mcr68_6840_w_common(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t mcr68_6840_r_common(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void reload_count(int counter);
	uint16_t compute_counter(int counter);
	void mcr68_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void zwackery_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void zwackery_spriteram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t zwackery_port_2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void init_intlaser();
	void init_pigskin();
	void init_zwackery();
	void init_blasted();
	void init_trisport();
	void init_xenophob();
	void init_archrivl();
	void init_spyhunt2();
	void init_archrivlb();
	uint16_t archrivlb_port_1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void zwackery_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void zwackery_get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_zwackery();
	void machine_reset_zwackery();
	void video_start_zwackery();
	void machine_start_mcr68();
	void machine_reset_mcr68();
	void video_start_mcr68();
	uint32_t screen_update_zwackery(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_mcr68(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mcr68_interrupt(device_t &device);
	void mcr68_493_off_callback(void *ptr, int32_t param);
	void mcr68_493_callback(void *ptr, int32_t param);
	void zwackery_493_off_callback(void *ptr, int32_t param);
	void zwackery_493_callback(void *ptr, int32_t param);
	void counter_fired_callback(void *ptr, int32_t param);
	uint8_t zwackery_port_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t zwackery_port_3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void zwackery_pia0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void zwackery_pia1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void zwackery_ca2_w(int state);
	void zwackery_pia_irq(int state);
	void mcr68_update_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);
	void zwackery_update_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);
	void mcr68_common_init();
	void update_mcr68_interrupts();
	inline void update_interrupts();
	void subtract_from_counter(int counter, int count);
	void mcr68_common_init(int clip, int xoffset);
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	std::unique_ptr<uint8_t[]> m_srcdata0;
	std::unique_ptr<uint8_t[]> m_srcdata2;
};
