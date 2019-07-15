// license:BSD-3-Clause
// copyright-holders:Fabrice Frances, Nicola Salmoria, Aaron Giles
/***************************************************************************

    Gottlieb hardware

***************************************************************************/

#include "audio/gottlieb.h"
#include "cpu/i86/i86.h"
#include "cpu/m6502/m6502.h"
#include "sound/samples.h"
#include "machine/ldpr8210.h"
#include "emupal.h"
#include "screen.h"


#define GOTTLIEB_VIDEO_HCOUNT   318
#define GOTTLIEB_VIDEO_HBLANK   256
#define GOTTLIEB_VIDEO_VCOUNT   256
#define GOTTLIEB_VIDEO_VBLANK   240




// ======================> gottlieb_state

// shared driver state
class gottlieb_state : public driver_device
{
public:
	gottlieb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_laserdisc(*this, "laserdisc")
		, m_r1_sound(*this, "r1sound")
		, m_r2_sound(*this, "r2sound")
		, m_knocker_sample(*this, "knocker_sam")
		, m_videoram(*this, "videoram")
		, m_charram(*this, "charram")
		, m_spriteram(*this, "spriteram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_paletteram(*this, "paletteram")
		, m_track_x(*this, "TRACKX")
		, m_track_y(*this, "TRACKY")
		, m_leds(*this, "led%u", 0U)
	{ }

	void gottlieb_core(machine_config &config);
	void cobram3(machine_config &config);
	void screwloo(machine_config &config);
	void gottlieb2(machine_config &config);
	void reactor(machine_config &config);
	void tylz(machine_config &config);
	void g2laser(machine_config &config);
	void qbert(machine_config &config);
	void qbert_knocker(machine_config &config);
	void gottlieb1(machine_config &config);
	void gottlieb1_votrax(machine_config &config);

	void init_romtiles();
	void init_screwloo();
	void init_vidvince();
	void init_ramtiles();
	void init_stooges();
	void init_qbert();
	void init_qbertqub();

	DECLARE_CUSTOM_INPUT_MEMBER(analog_delta_r);
	DECLARE_CUSTOM_INPUT_MEMBER(stooges_joystick_r);

private:
	enum
	{
		TIMER_LASERDISC_PHILIPS,
		TIMER_LASERDISC_BIT_OFF,
		TIMER_LASERDISC_BIT,
		TIMER_NMI_CLEAR
	};

	void qbert_knocker(uint8_t knock);

	DECLARE_WRITE8_MEMBER(analog_reset_w);
	void general_output_w(u8 data);
	DECLARE_WRITE8_MEMBER(reactor_output_w);
	DECLARE_WRITE8_MEMBER(stooges_output_w);
	DECLARE_WRITE8_MEMBER(qbertqub_output_w);
	DECLARE_WRITE8_MEMBER(qbert_output_w);
	DECLARE_READ8_MEMBER(laserdisc_status_r);
	DECLARE_WRITE8_MEMBER(laserdisc_select_w);
	DECLARE_WRITE8_MEMBER(laserdisc_command_w);
	void sound_w(u8 data);
	void palette_w(offs_t offset, u8 data);
	void video_control_w(u8 data);
	void laserdisc_video_control_w(u8 data);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(charram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_screwloo_bg_tile_info);
	DECLARE_VIDEO_START(screwloo);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);
	TIMER_CALLBACK_MEMBER(laserdisc_philips_callback);
	TIMER_CALLBACK_MEMBER(laserdisc_bit_off_callback);
	TIMER_CALLBACK_MEMBER(laserdisc_bit_callback);
	TIMER_CALLBACK_MEMBER(nmi_clear);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	inline void audio_end_state();
	void audio_process_clock(bool logit);
	void audio_handle_zero_crossing(const attotime &zerotime, bool logit);
	void laserdisc_audio_process(int samplerate, int samples, const int16_t *ch0, const int16_t *ch1);

	void gottlieb_map(address_map &map);
	void reactor_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// devices
	required_device<cpu_device> m_maincpu;
	optional_device<pioneer_pr8210_device> m_laserdisc;
	optional_device<gottlieb_sound_r1_device> m_r1_sound;
	optional_device<gottlieb_sound_r2_device> m_r2_sound;
	optional_device<samples_device> m_knocker_sample;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_charram;
	required_shared_ptr<uint8_t> m_spriteram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_paletteram;

	optional_ioport m_track_x;
	optional_ioport m_track_y;
	output_finder<3> m_leds;  // only used by reactor

	uint8_t m_knocker_prev;
	uint8_t m_joystick_select;
	uint8_t m_track[2];
	emu_timer *m_laserdisc_bit_timer;
	emu_timer *m_laserdisc_philips_timer;
	uint8_t m_laserdisc_select;
	uint8_t m_laserdisc_status;
	uint16_t m_laserdisc_philips_code;
	std::unique_ptr<uint8_t[]> m_laserdisc_audio_buffer;
	uint16_t m_laserdisc_audio_address;
	int16_t m_laserdisc_last_samples[2];
	attotime m_laserdisc_last_time;
	attotime m_laserdisc_last_clock;
	uint8_t m_laserdisc_zero_seen;
	uint8_t m_laserdisc_audio_bits;
	uint8_t m_laserdisc_audio_bit_count;
	uint8_t m_gfxcharlo;
	uint8_t m_gfxcharhi;
	uint8_t m_background_priority;
	uint8_t m_spritebank;
	uint8_t m_transparent0;
	tilemap_t *m_bg_tilemap;
	double m_weights[4];
};
