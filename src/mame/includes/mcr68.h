// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#include "machine/6821pia.h"
#include "audio/midway.h"
#include "audio/williams.h"

struct counter_state
{
	UINT8           control;
	UINT16          latch;
	UINT16          count;
	emu_timer *     timer;
	UINT8           timer_active;
	attotime        period;
};

class mcr68_state : public driver_device
{
public:
	mcr68_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_chip_squeak_deluxe(*this, "csd"),
		m_sounds_good(*this, "sg"),
		m_turbo_chip_squeak(*this, "tcs"),
		m_cvsd_sound(*this, "cvsd"),
			m_videoram(*this, "videoram"),
			m_spriteram(*this, "spriteram") ,
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	optional_device<midway_chip_squeak_deluxe_device> m_chip_squeak_deluxe;
	optional_device<midway_sounds_good_device> m_sounds_good;
	optional_device<midway_turbo_chip_squeak_device> m_turbo_chip_squeak;
	optional_device<williams_cvsd_sound_device> m_cvsd_sound;

	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_spriteram;
	UINT16 m_control_word;
	UINT8 m_protection_data[5];
	attotime m_timing_factor;
	UINT8 m_sprite_clip;
	INT8 m_sprite_xoffset;
	UINT8 m_m6840_status;
	UINT8 m_m6840_status_read_since_int;
	UINT8 m_m6840_msb_buffer;
	UINT8 m_m6840_lsb_buffer;
	UINT8 m_m6840_irq_state;
	UINT8 m_m6840_irq_vector;
	struct counter_state m_m6840_state[3];
	UINT8 m_v493_irq_state;
	UINT8 m_v493_irq_vector;
	timer_expired_delegate m_v493_callback;
	UINT8 m_zwackery_sound_data;
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
	UINT16 compute_counter(int counter);
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
	UINT32 screen_update_zwackery(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_mcr68(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
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
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	std::unique_ptr<UINT8[]> m_srcdata0;
	std::unique_ptr<UINT8[]> m_srcdata2;
};
