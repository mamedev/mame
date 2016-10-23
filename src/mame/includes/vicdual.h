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
		m_fake_lives(*this, "FAKE_LIVES.%u", 0)
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

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_characterram;

	required_ioport m_in0;
	required_ioport m_in1;
	optional_ioport m_in2;
	optional_ioport m_coinage;
	optional_ioport m_color_bw;
	optional_ioport_array<2> m_fake_lives;

	uint8_t m_coin_status;
	uint8_t m_palette_bank;
	uint8_t m_samurai_protection_data;
	int m_nsub_coin_counter;
	int m_nsub_play_counter;
	int m_port1State;
	int m_port2State;
	int m_psgData;
	int m_psgBus;
	emu_timer *m_frogs_croak_timer;

	void coin_in();
	void assert_coin_status();

	// common
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void characterram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void palette_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// game specific
	uint8_t depthch_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void depthch_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t safari_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void safari_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t frogs_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void frogs_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t headon_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sspaceat_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void headon_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t headon2_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void headon2_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void digger_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void invho2_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void invds_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void carhntds_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sspacaho_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tranqgun_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spacetrk_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void carnival_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void brdrline_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pulsar_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void heiankyo_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void alphaho_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void samurai_protection_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void samurai_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t nsub_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void nsub_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t invinco_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void invinco_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	/*----------- defined in audio/vicdual.c -----------*/
	void frogs_audio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void headon_audio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void invho2_audio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	TIMER_CALLBACK_MEMBER( frogs_croak_callback );

	/*----------- defined in audio/carnival.c -----------*/
	void carnival_audio_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void carnival_audio_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t carnival_music_port_t1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void carnival_music_port_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void carnival_music_port_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void carnival_psg_latch(address_space &space);

	/*----------- defined in audio/depthch.c -----------*/
	void depthch_audio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	/*----------- defined in audio/invinco.c -----------*/
	void invinco_audio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	/*----------- defined in audio/pulsar.c -----------*/
	void pulsar_audio_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pulsar_audio_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

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

	void machine_start_samurai();
	void machine_start_nsub();
	void machine_reset_nsub();
	void machine_start_frogs_audio();

	virtual void machine_start() override;

	uint32_t screen_update_bw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bw_or_color(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_color(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	int get_vcounter();
	int is_cabinet_color();
};
