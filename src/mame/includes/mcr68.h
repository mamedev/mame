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
	DECLARE_READ16_MEMBER(zwackery_6840_r);
	DECLARE_WRITE16_MEMBER(xenophobe_control_w);
	DECLARE_WRITE16_MEMBER(blasted_control_w);
	DECLARE_READ16_MEMBER(spyhunt2_port_0_r);
	DECLARE_READ16_MEMBER(spyhunt2_port_1_r);
	DECLARE_WRITE16_MEMBER(spyhunt2_control_w);
	DECLARE_READ16_MEMBER(archrivl_port_1_r);
	DECLARE_WRITE16_MEMBER(archrivl_control_w);
	DECLARE_WRITE16_MEMBER(pigskin_protection_w);
	DECLARE_READ16_MEMBER(pigskin_protection_r);
	DECLARE_READ16_MEMBER(pigskin_port_1_r);
	DECLARE_READ16_MEMBER(pigskin_port_2_r);
	DECLARE_READ16_MEMBER(trisport_port_1_r);
	DECLARE_WRITE16_MEMBER(mcr68_6840_upper_w);
	DECLARE_WRITE16_MEMBER(mcr68_6840_lower_w);
	DECLARE_READ16_MEMBER(mcr68_6840_upper_r);
	DECLARE_READ16_MEMBER(mcr68_6840_lower_r);
	DECLARE_WRITE8_MEMBER(mcr68_6840_w_common);
	DECLARE_READ16_MEMBER(mcr68_6840_r_common);
	void reload_count(int counter);
	uint16_t compute_counter(int counter);
	DECLARE_WRITE16_MEMBER(mcr68_videoram_w);
	DECLARE_WRITE16_MEMBER(zwackery_videoram_w);
	DECLARE_WRITE16_MEMBER(zwackery_spriteram_w);
	DECLARE_READ8_MEMBER(zwackery_port_2_r);
	DECLARE_DRIVER_INIT(intlaser);
	DECLARE_DRIVER_INIT(pigskin);
	DECLARE_DRIVER_INIT(zwackery);
	DECLARE_DRIVER_INIT(blasted);
	DECLARE_DRIVER_INIT(trisport);
	DECLARE_DRIVER_INIT(xenophob);
	DECLARE_DRIVER_INIT(archrivl);
	DECLARE_DRIVER_INIT(spyhunt2);
	DECLARE_DRIVER_INIT(archrivlb);
	DECLARE_READ16_MEMBER(archrivlb_port_1_r);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(zwackery_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(zwackery_get_fg_tile_info);
	DECLARE_MACHINE_START(zwackery);
	DECLARE_MACHINE_RESET(zwackery);
	DECLARE_VIDEO_START(zwackery);
	DECLARE_MACHINE_START(mcr68);
	DECLARE_MACHINE_RESET(mcr68);
	DECLARE_VIDEO_START(mcr68);
	uint32_t screen_update_zwackery(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_mcr68(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(mcr68_interrupt);
	TIMER_CALLBACK_MEMBER(mcr68_493_off_callback);
	TIMER_CALLBACK_MEMBER(mcr68_493_callback);
	TIMER_CALLBACK_MEMBER(zwackery_493_off_callback);
	TIMER_CALLBACK_MEMBER(zwackery_493_callback);
	TIMER_CALLBACK_MEMBER(counter_fired_callback);
	DECLARE_READ8_MEMBER(zwackery_port_1_r);
	DECLARE_READ8_MEMBER(zwackery_port_3_r);
	DECLARE_WRITE8_MEMBER(zwackery_pia0_w);
	DECLARE_WRITE8_MEMBER(zwackery_pia1_w);
	DECLARE_WRITE_LINE_MEMBER(zwackery_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(zwackery_pia_irq);
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
