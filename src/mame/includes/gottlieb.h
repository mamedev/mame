// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Gottlieb hardware

***************************************************************************/

#include "cpu/i86/i86.h"
#include "cpu/m6502/m6502.h"
#include "machine/6532riot.h"
#include "sound/dac.h"
#include "sound/ay8910.h"
#include "sound/sp0250.h"
#include "sound/samples.h"
#include "sound/votrax.h"
#include "machine/ldpr8210.h"


// set to 0 to enable Votrax device and disable samples
#define USE_FAKE_VOTRAX         (1)


#define GOTTLIEB_VIDEO_HCOUNT   318
#define GOTTLIEB_VIDEO_HBLANK   256
#define GOTTLIEB_VIDEO_VCOUNT   256
#define GOTTLIEB_VIDEO_VBLANK   240


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern const device_type GOTTLIEB_SOUND_REV1;
extern const device_type GOTTLIEB_SOUND_REV1_WITH_VOTRAX;
extern const device_type GOTTLIEB_SOUND_REV2;



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_GOTTLIEB_SOUND_R1_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, GOTTLIEB_SOUND_REV1, 0)
#define MCFG_GOTTLIEB_SOUND_R1_ADD_VOTRAX(_tag) \
	MCFG_DEVICE_ADD(_tag, GOTTLIEB_SOUND_REV1_WITH_VOTRAX, 0)

#define MCFG_GOTTLIEB_SOUND_R2_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, GOTTLIEB_SOUND_REV2, 0)
#define MCFG_GOTTLIEB_SOUND_R2_ADD_COBRAM3(_tag) \
	MCFG_DEVICE_ADD(_tag, GOTTLIEB_SOUND_REV2, 0) \
	gottlieb_sound_r2_device::static_enable_cobram3_mods(*device);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> gottlieb_sound_r1_device

// rev 1 sound board, with unpopulated VOTRAX
class gottlieb_sound_r1_device :    public device_t,
									public device_mixer_interface
{
public:
	// construction/destruction
	gottlieb_sound_r1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	gottlieb_sound_r1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock, bool populate_votrax);

	// read/write
	DECLARE_WRITE8_MEMBER( write );

	// internal communications
	DECLARE_WRITE_LINE_MEMBER( snd_interrupt );
	DECLARE_WRITE8_MEMBER( r6532_portb_w );
	DECLARE_WRITE8_MEMBER( votrax_data_w );
	DECLARE_WRITE8_MEMBER( speech_clock_dac_w );
	DECLARE_WRITE_LINE_MEMBER( votrax_request );

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;
	virtual void device_start();

private:
	// devices
	required_device<m6502_device>       m_audiocpu;
	required_device<riot6532_device>    m_riot;
	required_device<dac_device>         m_dac;
	optional_device<votrax_sc01_device> m_votrax;

	// internal state
	//bool            m_populate_votrax;
	UINT8           m_last_speech_clock;

#if USE_FAKE_VOTRAX
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
private:
	void fake_votrax_data_w(UINT8 data);
	void trigger_sample(UINT8 data);
	optional_device<samples_device> m_samples;
	UINT8 m_score_sample;
	UINT8 m_random_offset;
	UINT8 m_votrax_queue[100];
	UINT8 m_votrax_queuepos;
#endif
};

// fully populated rev 1 sound board
class gottlieb_sound_r1_with_votrax_device : public gottlieb_sound_r1_device
{
public:
	// construction/destruction
	gottlieb_sound_r1_with_votrax_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;
};


// ======================> gottlieb_sound_r2_device

// fully populated rev 2 sound board
class gottlieb_sound_r2_device :    public device_t,
									public device_mixer_interface
{
public:
	// construction/destruction
	gottlieb_sound_r2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	static void static_enable_cobram3_mods(device_t &device);

	// read/write
	DECLARE_WRITE8_MEMBER( write );

	// internal communications
	DECLARE_READ8_MEMBER( speech_data_r );
	DECLARE_READ8_MEMBER( audio_data_r );
	DECLARE_WRITE8_MEMBER( signal_audio_nmi_w );
	DECLARE_WRITE8_MEMBER( nmi_rate_w );
	CUSTOM_INPUT_MEMBER( speech_drq_custom_r );
	DECLARE_WRITE8_MEMBER( dac_w );
	DECLARE_WRITE8_MEMBER( speech_control_w );
	DECLARE_WRITE8_MEMBER( sp0250_latch_w );
	DECLARE_WRITE8_MEMBER( psg_latch_w );

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	// internal helpers
	void nmi_timer_adjust();
	void nmi_state_update();

	// timer IDs
	enum
	{
		TID_NMI_GENERATE,
		TID_NMI_CLEAR,
		TID_SOUND_LATCH_WRITE
	};

	// devices
	required_device<m6502_device>   m_audiocpu;
	required_device<m6502_device>   m_speechcpu;
	required_device<dac_device>     m_dac;
	required_device<ay8913_device>  m_ay1;
	required_device<ay8913_device>  m_ay2;
	optional_device<sp0250_device>  m_sp0250;

	// internal state
	bool        m_cobram3_mod;
	emu_timer * m_nmi_timer;
	UINT8       m_nmi_rate;
	UINT8       m_nmi_state;
	UINT8       m_audiocpu_latch;
	UINT8       m_speechcpu_latch;
	UINT8       m_speech_control;
	UINT8       m_last_command;
	UINT8       m_dac_data[2];
	UINT8       m_psg_latch;
	UINT8       m_psg_data_latch;
	UINT8       m_sp0250_latch;
};


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
			m_palette(*this, "palette")
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
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
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
	void audio_handle_zero_crossing(attotime zerotime, int logit);
	void laserdisc_audio_process(laserdisc_device &device, int samplerate, int samples, const INT16 *ch0, const INT16 *ch1);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};

/*----------- defined in audio/gottlieb.c -----------*/

MACHINE_CONFIG_EXTERN( qbert_knocker );

#if USE_FAKE_VOTRAX
MACHINE_CONFIG_EXTERN( reactor_samples );
MACHINE_CONFIG_EXTERN( qbert_samples );
#endif
