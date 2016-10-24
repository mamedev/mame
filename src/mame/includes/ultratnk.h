// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Stefan Jokisch
/*************************************************************************

    Atari Ultra Tank hardware

*************************************************************************/

#include "machine/watchdog.h"
#include "sound/discrete.h"

class ultratnk_state : public driver_device
{
public:
	enum
	{
		TIMER_NMI
	};

	ultratnk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_shared_ptr<uint8_t> m_videoram;
	int m_da_latch;
	int m_collision[4];
	tilemap_t* m_playfield;
	bitmap_ind16 m_helper;
	uint8_t ultratnk_wram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ultratnk_analog_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ultratnk_coin_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ultratnk_collision_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ultratnk_options_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ultratnk_wram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ultratnk_collision_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ultratnk_da_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ultratnk_led_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ultratnk_led_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ultratnk_lockout_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ultratnk_video_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value get_collision(ioport_field &field, void *param);
	ioport_value get_joystick(ioport_field &field, void *param);
	void ultratnk_fire_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ultratnk_fire_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ultratnk_attract_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ultratnk_explosion_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ultratnk_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_ultratnk(palette_device &palette);
	uint32_t screen_update_ultratnk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_ultratnk(screen_device &screen, bool state);
	void nmi_callback(void *ptr, int32_t param);
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
