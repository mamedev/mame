// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*************************************************************************

    VIC Dual Game board

*************************************************************************/

#include "cpu/mcs48/mcs48.h"
#include "sound/ay8910.h"
#include "sound/discrete.h"
#include "sound/samples.h"

class vicdual_state : public driver_device
{
public:
	vicdual_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_psg(*this, "psg"),
		m_samples(*this, "samples"),
		m_discrete(*this, "discrete"),
		m_coinstate_timer(*this, "coinstate"),
		m_nsub_coinage_timer(*this, "nsub_coin"),
		m_screen(*this, "screen"),
		m_proms(*this, "proms"),
		m_videoram(*this, "videoram"),
		m_characterram(*this, "characterram"),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1"),
		m_in2(*this, "IN2"),
		m_coinage(*this, "COINAGE"),
		m_color_bw(*this, "COLOR_BW"),
		m_fake_lives(*this, "FAKE_LIVES")
	{ }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<ay8910_device> m_psg;
	optional_device<samples_device> m_samples;
	optional_device<discrete_device> m_discrete;
	required_device<timer_device> m_coinstate_timer;
	optional_device<timer_device> m_nsub_coinage_timer;
	required_device<screen_device> m_screen;
	optional_memory_region m_proms;

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_characterram;

	required_ioport m_in0;
	required_ioport m_in1;
	optional_ioport m_in2;
	optional_ioport m_coinage;
	optional_ioport m_color_bw;
	optional_ioport_array<2> m_fake_lives;

	UINT8 m_coin_status;
	UINT8 m_palette_bank;
	UINT8 m_samurai_protection_data;
	int m_nsub_coin_counter;
	int m_nsub_play_counter;
	int m_port1State;
	int m_port2State;
	int m_psgData;
	emu_timer *m_frogs_croak_timer;

	void coin_in();
	void assert_coin_status();

	// common
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(characterram_w);
	DECLARE_WRITE8_MEMBER(palette_bank_w);

	// game specific
	DECLARE_READ8_MEMBER(depthch_io_r);
	DECLARE_WRITE8_MEMBER(depthch_io_w);
	DECLARE_READ8_MEMBER(safari_io_r);
	DECLARE_WRITE8_MEMBER(safari_io_w);
	DECLARE_READ8_MEMBER(frogs_io_r);
	DECLARE_WRITE8_MEMBER(frogs_io_w);
	DECLARE_READ8_MEMBER(headon_io_r);
	DECLARE_READ8_MEMBER(sspaceat_io_r);
	DECLARE_WRITE8_MEMBER(headon_io_w);
	DECLARE_READ8_MEMBER(headon2_io_r);
	DECLARE_WRITE8_MEMBER(headon2_io_w);
	DECLARE_WRITE8_MEMBER(digger_io_w);
	DECLARE_WRITE8_MEMBER(invho2_io_w);
	DECLARE_WRITE8_MEMBER(invds_io_w);
	DECLARE_WRITE8_MEMBER(carhntds_io_w);
	DECLARE_WRITE8_MEMBER(sspacaho_io_w);
	DECLARE_WRITE8_MEMBER(tranqgun_io_w);
	DECLARE_WRITE8_MEMBER(spacetrk_io_w);
	DECLARE_WRITE8_MEMBER(carnival_io_w);
	DECLARE_WRITE8_MEMBER(brdrline_io_w);
	DECLARE_WRITE8_MEMBER(pulsar_io_w);
	DECLARE_WRITE8_MEMBER(heiankyo_io_w);
	DECLARE_WRITE8_MEMBER(alphaho_io_w);
	DECLARE_WRITE8_MEMBER(samurai_protection_w);
	DECLARE_WRITE8_MEMBER(samurai_io_w);
	DECLARE_READ8_MEMBER(nsub_io_r);
	DECLARE_WRITE8_MEMBER(nsub_io_w);
	DECLARE_READ8_MEMBER(invinco_io_r);
	DECLARE_WRITE8_MEMBER(invinco_io_w);

	/*----------- defined in audio/vicdual.c -----------*/
	DECLARE_WRITE8_MEMBER( frogs_audio_w );
	DECLARE_WRITE8_MEMBER( headon_audio_w );
	DECLARE_WRITE8_MEMBER( invho2_audio_w );
	TIMER_CALLBACK_MEMBER( frogs_croak_callback );


	/*----------- defined in audio/carnival.c -----------*/
	DECLARE_WRITE8_MEMBER( carnival_audio_1_w );
	DECLARE_WRITE8_MEMBER( carnival_audio_2_w );
	DECLARE_READ8_MEMBER( carnival_music_port_t1_r );
	DECLARE_WRITE8_MEMBER( carnival_music_port_1_w );
	DECLARE_WRITE8_MEMBER( carnival_music_port_2_w );

	/*----------- defined in audio/depthch.c -----------*/
	DECLARE_WRITE8_MEMBER( depthch_audio_w );

	/*----------- defined in audio/invinco.c -----------*/
	DECLARE_WRITE8_MEMBER( invinco_audio_w );

	/*----------- defined in audio/pulsar.c -----------*/
	DECLARE_WRITE8_MEMBER( pulsar_audio_1_w );
	DECLARE_WRITE8_MEMBER( pulsar_audio_2_w );

	DECLARE_CUSTOM_INPUT_MEMBER(read_coin_status);
	DECLARE_CUSTOM_INPUT_MEMBER(get_64v);
	DECLARE_CUSTOM_INPUT_MEMBER(get_vblank_comp);
	DECLARE_CUSTOM_INPUT_MEMBER(get_composite_blank_comp);
	DECLARE_CUSTOM_INPUT_MEMBER(get_timer_value);
	DECLARE_CUSTOM_INPUT_MEMBER(fake_lives_r);
	DECLARE_CUSTOM_INPUT_MEMBER(samurai_protection_r);
	DECLARE_INPUT_CHANGED_MEMBER(coin_changed);
	DECLARE_INPUT_CHANGED_MEMBER(nsub_coin_in);

	TIMER_DEVICE_CALLBACK_MEMBER(clear_coin_status);
	TIMER_DEVICE_CALLBACK_MEMBER(nsub_coin_pulse);

	DECLARE_MACHINE_START(samurai);
	DECLARE_MACHINE_START(nsub);
	DECLARE_MACHINE_RESET(nsub);
	DECLARE_MACHINE_START(frogs_audio);

	virtual void machine_start() override;

	UINT32 screen_update_bw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_bw_or_color(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_color(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	int get_vcounter();
	int is_cabinet_color();
};

MACHINE_CONFIG_EXTERN( carnival_audio );
MACHINE_CONFIG_EXTERN( depthch_audio );
MACHINE_CONFIG_EXTERN( frogs_audio );
MACHINE_CONFIG_EXTERN( headon_audio );
MACHINE_CONFIG_EXTERN( invinco_audio );
MACHINE_CONFIG_EXTERN( pulsar_audio );
