// license:BSD-3-Clause
// copyright-holders:Fabrice Frances, Nicola Salmoria, Aaron Giles
/***************************************************************************

    Gottlieb hardware

***************************************************************************/

#include "emu.h"
#include "audio/gottlieb.h"
#include "cpu/i86/i86.h"
#include "cpu/m6502/m6502.h"
#include "sound/samples.h"
#include "machine/ldpr8210.h"


#define GOTTLIEB_VIDEO_HCOUNT   318
#define GOTTLIEB_VIDEO_HBLANK   256
#define GOTTLIEB_VIDEO_VCOUNT   256
#define GOTTLIEB_VIDEO_VBLANK   240




// ======================> gottlieb_state

// shared driver state
class gottlieb_state : public driver_device
{
public:
	enum
	{
		TIMER_LASERDISC_PHILIPS,
		TIMER_LASERDISC_BIT_OFF,
		TIMER_LASERDISC_BIT,
		TIMER_NMI_CLEAR
	};

	gottlieb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_laserdisc(*this, "laserdisc"),
			m_r1_sound(*this, "r1sound"),
			m_r2_sound(*this, "r2sound"),
			m_knocker_sample(*this, "knocker_sam"),
			m_videoram(*this, "videoram"),
			m_charram(*this, "charram"),
			m_spriteram(*this, "spriteram"),
			m_gfxdecode(*this, "gfxdecode"),
			m_screen(*this, "screen"),
			m_palette(*this, "palette"),
			m_generic_paletteram_8(*this, "paletteram")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	optional_device<pioneer_pr8210_device> m_laserdisc;
	optional_device<gottlieb_sound_r1_device> m_r1_sound;
	optional_device<gottlieb_sound_r2_device> m_r2_sound;
	optional_device<samples_device> m_knocker_sample;

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_charram;
	required_shared_ptr<UINT8> m_spriteram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<UINT8> m_generic_paletteram_8;

	UINT8 m_knocker_prev;
	UINT8 m_joystick_select;
	UINT8 m_track[2];
	emu_timer *m_laserdisc_bit_timer;
	emu_timer *m_laserdisc_philips_timer;
	UINT8 m_laserdisc_select;
	UINT8 m_laserdisc_status;
	UINT16 m_laserdisc_philips_code;
	UINT8 *m_laserdisc_audio_buffer;
	UINT16 m_laserdisc_audio_address;
	INT16 m_laserdisc_last_samples[2];
	attotime m_laserdisc_last_time;
	attotime m_laserdisc_last_clock;
	UINT8 m_laserdisc_zero_seen;
	UINT8 m_laserdisc_audio_bits;
	UINT8 m_laserdisc_audio_bit_count;
	UINT8 m_gfxcharlo;
	UINT8 m_gfxcharhi;
	UINT8 m_background_priority;
	UINT8 m_spritebank;
	UINT8 m_transparent0;
	tilemap_t *m_bg_tilemap;
	double m_weights[4];

	void qbert_knocker(UINT8 knock);

	DECLARE_WRITE8_MEMBER(gottlieb_analog_reset_w);
	DECLARE_WRITE8_MEMBER(general_output_w);
	DECLARE_WRITE8_MEMBER(reactor_output_w);
	DECLARE_WRITE8_MEMBER(stooges_output_w);
	DECLARE_WRITE8_MEMBER(qbertqub_output_w);
	DECLARE_WRITE8_MEMBER(qbert_output_w);
	DECLARE_READ8_MEMBER(laserdisc_status_r);
	DECLARE_WRITE8_MEMBER(laserdisc_select_w);
	DECLARE_WRITE8_MEMBER(laserdisc_command_w);
	DECLARE_WRITE8_MEMBER(gottlieb_sh_w);
	DECLARE_WRITE8_MEMBER(gottlieb_paletteram_w);
	DECLARE_WRITE8_MEMBER(gottlieb_video_control_w);
	DECLARE_WRITE8_MEMBER(gottlieb_laserdisc_video_control_w);
	DECLARE_WRITE8_MEMBER(gottlieb_videoram_w);
	DECLARE_WRITE8_MEMBER(gottlieb_charram_w);
	DECLARE_CUSTOM_INPUT_MEMBER(analog_delta_r);
	DECLARE_CUSTOM_INPUT_MEMBER(stooges_joystick_r);
	DECLARE_DRIVER_INIT(romtiles);
	DECLARE_DRIVER_INIT(screwloo);
	DECLARE_DRIVER_INIT(vidvince);
	DECLARE_DRIVER_INIT(ramtiles);
	DECLARE_DRIVER_INIT(stooges);
	DECLARE_DRIVER_INIT(qbert);
	DECLARE_DRIVER_INIT(qbertqub);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_screwloo_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_VIDEO_START(screwloo);
	UINT32 screen_update_gottlieb(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(gottlieb_interrupt);
	TIMER_CALLBACK_MEMBER(laserdisc_philips_callback);
	TIMER_CALLBACK_MEMBER(laserdisc_bit_off_callback);
	TIMER_CALLBACK_MEMBER(laserdisc_bit_callback);
	TIMER_CALLBACK_MEMBER(nmi_clear);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	inline void audio_end_state();
	void audio_process_clock(int logit);
	void audio_handle_zero_crossing(const attotime &zerotime, int logit);
	void laserdisc_audio_process(laserdisc_device &device, int samplerate, int samples, const INT16 *ch0, const INT16 *ch1);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
