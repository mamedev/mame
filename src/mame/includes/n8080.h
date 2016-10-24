// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
#include "cpu/mcs48/mcs48.h"
#include "sound/dac.h"
#include "sound/sn76477.h"

class n8080_state : public driver_device
{
public:
	n8080_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_n8080_dac(*this, "n8080_dac"),
		m_helifire_dac(*this, "helifire_dac"),
		m_sn(*this, "snsnd"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_colorram;      // for helifire

	/* video-related */
	emu_timer* m_cannon_timer;
	int m_spacefev_red_screen;
	int m_spacefev_red_cannon;
	int m_sheriff_color_mode;
	int m_sheriff_color_data;
	int m_helifire_flash;
	uint8_t m_helifire_LSFR[63];
	unsigned m_helifire_mv;
	unsigned m_helifire_sc; /* IC56 */

	/* sound-related */
	int m_n8080_hardware;
	emu_timer* m_sound_timer[3];
	int m_helifire_dac_phase;
	double m_helifire_dac_volume;
	double m_helifire_dac_timing;
	uint16_t m_prev_sound_pins;
	uint16_t m_curr_sound_pins;
	int m_mono_flop[3];
	uint8_t m_prev_snd_data;

	/* other */
	unsigned m_shift_data;
	unsigned m_shift_bits;
	int m_inte;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<dac_bit_interface> m_n8080_dac;
	optional_device<dac_8bit_r2r_device> m_helifire_dac;
	optional_device<sn76477_device> m_sn;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	void n8080_shift_bits_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void n8080_shift_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t n8080_shift_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void n8080_video_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void n8080_sound_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void n8080_sound_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t n8080_8035_p1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t n8080_8035_t0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t n8080_8035_t1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t helifire_8035_t0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t helifire_8035_t1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t helifire_8035_external_ram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t helifire_8035_p2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void n8080_dac_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void helifire_sound_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void n8080_inte_callback(int state);
	void n8080_status_callback(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	void machine_reset_spacefev();
	void video_start_spacefev();
	void palette_init_n8080(palette_device &palette);
	void machine_reset_sheriff();
	void video_start_sheriff();
	void machine_reset_helifire();
	void video_start_helifire();
	void palette_init_helifire(palette_device &palette);
	void sound_start_spacefev();
	void sound_reset_spacefev();
	void sound_start_sheriff();
	void sound_reset_sheriff();
	void sound_start_helifire();
	void sound_reset_helifire();
	void machine_start_n8080();
	void machine_reset_n8080();
	uint32_t screen_update_spacefev(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_sheriff(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_helifire(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_helifire(screen_device &screen, bool state);
	void spacefev_stop_red_cannon(void *ptr, int32_t param);
	void rst1_tick(timer_device &timer, void *ptr, int32_t param);
	void rst2_tick(timer_device &timer, void *ptr, int32_t param);
	void spacefev_vco_voltage_timer(timer_device &timer, void *ptr, int32_t param);
	void helifire_dac_volume_timer(timer_device &timer, void *ptr, int32_t param);
	void spacefev_start_red_cannon(  );
	void helifire_next_line(  );
	void spacefev_update_SN76477_status();
	void sheriff_update_SN76477_status();
	void update_SN76477_status();
	void start_mono_flop( int n, const attotime &expire );
	void stop_mono_flop( int n );
	void stop_mono_flop_callback(void *ptr, int32_t param);
	void spacefev_sound_pins_changed();
	void sheriff_sound_pins_changed();
	void helifire_sound_pins_changed();
	void sound_pins_changed();
	void delayed_sound_1( int data );
	void delayed_sound_1_callback(void *ptr, int32_t param);
	void delayed_sound_2( int data );
	void delayed_sound_2_callback(void *ptr, int32_t param);
};

/*----------- defined in audio/n8080.c -----------*/

MACHINE_CONFIG_EXTERN( spacefev_sound );
MACHINE_CONFIG_EXTERN( sheriff_sound );
MACHINE_CONFIG_EXTERN( helifire_sound );
