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
#include "tilemap.h"


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
		, m_knockers(*this, "knocker%d", 0U)
	{ }

	void gottlieb_core(machine_config &config);
	void cobram3(machine_config &config);
	void screwloo(machine_config &config);
	void gottlieb2(machine_config &config);
	void gottlieb2_ram_rom(machine_config &config);
	void reactor(machine_config &config);
	void tylz(machine_config &config);
	void g2laser(machine_config &config);
	void qbert(machine_config &config);
	void qbert_knocker(machine_config &config);
	void gottlieb1(machine_config &config);
	void gottlieb1_rom(machine_config &config);
	void gottlieb1_votrax(machine_config &config);

	void init_romtiles();
	void init_screwloo();
	void init_vidvince();
	void init_ramtiles();
	void init_stooges();
	void init_qbert();
	void init_qbertqub();

	template <int N> DECLARE_CUSTOM_INPUT_MEMBER(track_delta_r);
	DECLARE_CUSTOM_INPUT_MEMBER(stooges_joystick_r);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

private:
	enum
	{
		TIMER_LASERDISC_PHILIPS,
		TIMER_LASERDISC_BIT_OFF,
		TIMER_LASERDISC_BIT,
		TIMER_NMI_CLEAR
	};

	void qbert_knocker(u8 knock);

	void analog_reset_w(u8 data);
	void general_output_w(u8 data);
	void reactor_output_w(u8 data);
	void stooges_output_w(u8 data);
	void qbertqub_output_w(u8 data);
	void qbert_output_w(u8 data);
	u8 laserdisc_status_r(offs_t offset);
	void laserdisc_select_w(u8 data);
	void laserdisc_command_w(u8 data);
	void sound_w(u8 data);
	void palette_w(offs_t offset, u8 data);
	void video_control_w(u8 data);
	void laserdisc_video_control_w(u8 data);
	void videoram_w(offs_t offset, u8 data);
	void charram_w(offs_t offset, u8 data);
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

	void gottlieb_base_map(address_map &map);
	void gottlieb_ram_map(address_map &map);
	void gottlieb_ram_rom_map(address_map &map);
	void gottlieb_rom_map(address_map &map);
	void reactor_map(address_map &map);

	// devices
	required_device<cpu_device> m_maincpu;
	optional_device<pioneer_pr8210_device> m_laserdisc;
	optional_device<gottlieb_sound_r1_device> m_r1_sound;
	optional_device<gottlieb_sound_r2_device> m_r2_sound;
	optional_device<samples_device> m_knocker_sample;

	required_shared_ptr<u8> m_videoram;
	required_shared_ptr<u8> m_charram;
	required_shared_ptr<u8> m_spriteram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<u8> m_paletteram;

	optional_ioport m_track_x;
	optional_ioport m_track_y;
	output_finder<3> m_leds;  // only used by reactor
	output_finder<1> m_knockers;  // only used by qbert

	u8 m_knocker_prev = 0U;
	u8 m_joystick_select = 0U;
	u8 m_track[2]{};
	emu_timer *m_laserdisc_bit_timer = 0;
	emu_timer *m_laserdisc_philips_timer = 0;
	u8 m_laserdisc_select = 0U;
	u8 m_laserdisc_status = 0U;
	uint16_t m_laserdisc_philips_code = 0U;
	std::unique_ptr<u8[]> m_laserdisc_audio_buffer{};
	uint16_t m_laserdisc_audio_address = 0U;
	int16_t m_laserdisc_last_samples[2]{};
	attotime m_laserdisc_last_time;
	attotime m_laserdisc_last_clock;
	u8 m_laserdisc_zero_seen = 0U;
	u8 m_laserdisc_audio_bits = 0U;
	u8 m_laserdisc_audio_bit_count = 0U;
	u8 m_gfxcharlo = 0U;
	u8 m_gfxcharhi = 0U;
	u8 m_background_priority = 0U;
	u8 m_spritebank = 0U;
	u8 m_transparent0 = 0U;
	tilemap_t *m_bg_tilemap = 0;
	double m_weights[4]{};
};
