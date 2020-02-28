// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*************************************************************************

    VIC Dual Game board

*************************************************************************/

#include "cpu/mcs48/mcs48.h"
#include "machine/pit8253.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/discrete.h"
#include "sound/samples.h"
#include "sound/volt_reg.h"
#include "screen.h"
#include "audio/vicdual-97271p.h"
#include "video/vicdual-97269pb.h"

class vicdual_state : public driver_device
{
public:
	vicdual_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
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

	void vicdual_root(machine_config &config);
	void vicdual_dualgame_root(machine_config &config);
	void heiankyo(machine_config &config);
	void headon(machine_config &config);
	void headon_audio(machine_config &config);
	void sspacaho(machine_config &config);
	void headonn(machine_config &config);
	void invho2(machine_config &config);
	void frogs(machine_config &config);
	void frogs_audio(machine_config &config);
	void headons(machine_config &config);
	void invinco(machine_config &config);
	void invinco_audio(machine_config &config);
	void invds(machine_config &config);
	void headon2(machine_config &config);
	void pulsar(machine_config &config);
	void pulsar_audio(machine_config &config);
	void spacetrk(machine_config &config);
	void headon2bw(machine_config &config);
	void safari(machine_config &config);
	void brdrline(machine_config &config);
	void brdrline_audio(machine_config &config);
	void samurai(machine_config &config);
	void sspaceat(machine_config &config);
	void digger(machine_config &config);
	void depthch(machine_config &config);
	void depthch_audio(machine_config &config);
	void carhntds(machine_config &config);
	void alphaho(machine_config &config);
	void tranqgun(machine_config &config);
	void tranqgun_audio(machine_config &config);

	DECLARE_READ_LINE_MEMBER(coin_status_r);
	DECLARE_READ_LINE_MEMBER(get_64v);
	DECLARE_READ_LINE_MEMBER(vblank_comp_r);
	DECLARE_READ_LINE_MEMBER(cblank_comp_r);
	DECLARE_READ_LINE_MEMBER(timer_value_r);
	template <int Param> DECLARE_READ_LINE_MEMBER(fake_lives_r);
	template <int N> DECLARE_READ_LINE_MEMBER(samurai_protection_r);
	DECLARE_INPUT_CHANGED_MEMBER(coin_changed);

protected:
	required_device<cpu_device> m_maincpu;
	optional_device<samples_device> m_samples;
	optional_device<discrete_sound_device> m_discrete;
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
	int m_port1State;
	int m_port2State;
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
	DECLARE_MACHINE_RESET(headon2);
	DECLARE_READ8_MEMBER(headon2_io_r);
	DECLARE_WRITE8_MEMBER(headon2_io_w);
	DECLARE_WRITE8_MEMBER(digger_io_w);
	DECLARE_WRITE8_MEMBER(invho2_io_w);
	DECLARE_WRITE8_MEMBER(invds_io_w);
	DECLARE_WRITE8_MEMBER(carhntds_io_w);
	DECLARE_WRITE8_MEMBER(sspacaho_io_w);
	DECLARE_WRITE8_MEMBER(headonn_io_w);
	DECLARE_WRITE8_MEMBER(tranqgun_io_w);
	DECLARE_WRITE8_MEMBER(spacetrk_io_w);
	DECLARE_WRITE8_MEMBER(brdrline_io_w);
	DECLARE_WRITE8_MEMBER(pulsar_io_w);
	DECLARE_WRITE8_MEMBER(heiankyo_io_w);
	DECLARE_WRITE8_MEMBER(alphaho_io_w);
	DECLARE_WRITE8_MEMBER(samurai_protection_w);
	DECLARE_WRITE8_MEMBER(samurai_io_w);
	DECLARE_READ8_MEMBER(invinco_io_r);
	DECLARE_WRITE8_MEMBER(invinco_io_w);

	/*----------- defined in audio/vicdual.cpp -----------*/
	DECLARE_WRITE8_MEMBER( frogs_audio_w );
	DECLARE_WRITE8_MEMBER( headon_audio_w );
	DECLARE_WRITE8_MEMBER( invho2_audio_w );
	DECLARE_WRITE8_MEMBER( brdrline_audio_w );
	DECLARE_WRITE8_MEMBER( brdrline_audio_aux_w );
	TIMER_CALLBACK_MEMBER( frogs_croak_callback );

	/*----------- defined in audio/depthch.cpp -----------*/
	DECLARE_WRITE8_MEMBER( depthch_audio_w );

	/*----------- defined in audio/invinco.cpp -----------*/
	DECLARE_WRITE8_MEMBER( invinco_audio_w );

	/*----------- defined in audio/pulsar.cpp -----------*/
	DECLARE_WRITE8_MEMBER( pulsar_audio_1_w );
	DECLARE_WRITE8_MEMBER( pulsar_audio_2_w );

	/*----------- defined in audio/tranqgun.cpp -----------*/
	DECLARE_WRITE8_MEMBER( tranqgun_audio_w );

	TIMER_DEVICE_CALLBACK_MEMBER(clear_coin_status);

	DECLARE_MACHINE_START(samurai);
	DECLARE_MACHINE_START(frogs_audio);

	virtual void machine_start() override;

	uint32_t screen_update_bw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bw_or_color(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_color(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	int get_vcounter();
	int is_cabinet_color();
	virtual pen_t choose_pen(uint8_t x, uint8_t y, pen_t back_pen);

	void alphaho_io_map(address_map &map);
	void brdrline_io_map(address_map &map);
	void carhntds_dualgame_map(address_map &map);
	void carhntds_io_map(address_map &map);
	void depthch_io_map(address_map &map);
	void depthch_map(address_map &map);
	void digger_io_map(address_map &map);
	void frogs_io_map(address_map &map);
	void frogs_map(address_map &map);
	void headon2_io_map(address_map &map);
	void headon2_map(address_map &map);
	void headon_io_map(address_map &map);
	void headon_map(address_map &map);
	void headonn_io_map(address_map &map);
	void heiankyo_io_map(address_map &map);
	void invds_io_map(address_map &map);
	void invho2_io_map(address_map &map);
	void invinco_io_map(address_map &map);
	void invinco_map(address_map &map);
	void pulsar_io_map(address_map &map);
	void safari_io_map(address_map &map);
	void safari_map(address_map &map);
	void samurai_io_map(address_map &map);
	void samurai_map(address_map &map);
	void spacetrk_io_map(address_map &map);
	void sspacaho_io_map(address_map &map);
	void sspaceat_io_map(address_map &map);
	void tranqgun_io_map(address_map &map);
	void vicdual_dualgame_map(address_map &map);
};

class nsub_state : public vicdual_state
{
public:
	nsub_state(const machine_config &mconfig, device_type type, const char *tag) :
		vicdual_state(mconfig, type, tag),
		m_s97269pb(*this,"s97269pb"),
		m_s97271p(*this,"s97271p")
	{ }

	void nsub(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(nsub_coin_in);

private:
	required_device<s97269pb_device> m_s97269pb;
	required_device<s97271p_device> m_s97271p;

	int m_nsub_coin_counter;
	int m_nsub_play_counter;

	DECLARE_READ8_MEMBER(nsub_io_r);
	DECLARE_WRITE8_MEMBER(nsub_io_w);

	TIMER_DEVICE_CALLBACK_MEMBER(nsub_coin_pulse);

	DECLARE_MACHINE_START(nsub);
	DECLARE_MACHINE_RESET(nsub);

	virtual pen_t choose_pen(uint8_t x, uint8_t y, pen_t back_pen) override;
	void nsub_io_map(address_map &map);
	void nsub_map(address_map &map);
};

class carnival_state : public vicdual_state
{
public:
	carnival_state(const machine_config &mconfig, device_type type, const char *tag) :
		vicdual_state(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu"),
		m_psg(*this, "psg"),
		m_pit(*this, "pit"),
		m_dac(*this, "dac%u", 0),
		m_vref(*this, "vref%u", 0)
	{ }

	void carnival(machine_config &config);
	void carnivalb(machine_config &config);
	void carnivalh(machine_config &config);

	void carnivala_audio(machine_config &config);
	void carnivalb_audio(machine_config &config);

protected:
	virtual void machine_start() override;

	required_device<i8035_device> m_audiocpu;
	optional_device<ay8910_device> m_psg;
	optional_device<pit8253_device> m_pit;
	optional_device_array<dac_bit_interface, 3> m_dac;
	optional_device_array<voltage_regulator_device, 3> m_vref;

	void carnival_io_map(address_map &map);
	void mboard_map(address_map &map);

	int m_musicData;
	int m_musicBus;

	DECLARE_WRITE8_MEMBER(carnival_io_w);

	/*----------- defined in audio/carnival.cpp -----------*/
	DECLARE_WRITE8_MEMBER( carnival_audio_1_w );
	DECLARE_WRITE8_MEMBER( carnival_audio_2_w );
	DECLARE_READ_LINE_MEMBER( carnival_music_port_t1_r );
	DECLARE_WRITE8_MEMBER( carnivala_music_port_1_w );
	DECLARE_WRITE8_MEMBER( carnivala_music_port_2_w );
	void carnival_psg_latch();
	DECLARE_WRITE8_MEMBER( carnivalb_music_port_1_w );
	DECLARE_WRITE8_MEMBER( carnivalb_music_port_2_w );
};

class headonsa_state : public vicdual_state
{
public:
	headonsa_state(const machine_config &mconfig, device_type type, const char *tag) :
		vicdual_state(mconfig, type, tag)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(headonsa_coin_inserted);
};
