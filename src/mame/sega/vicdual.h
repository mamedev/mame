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
#include "screen.h"
#include "vicdual_a.h"
#include "vicdual-97271p.h"
#include "vicdual-97269pb.h"

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
		m_vicdual_sound(*this, "vicdual_sound"),
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
	void samurai(machine_config &config);
	void sspaceat(machine_config &config);
	void digger(machine_config &config);
	void depthch(machine_config &config);
	void depthch_audio(machine_config &config);
	void carhntds(machine_config &config);
	void alphaho(machine_config &config);

	int coin_status_r();
	int get_64v();
	int vblank_comp_r();
	int cblank_comp_r();
	int timer_value_r();
	template <int Param> int fake_lives_r();
	template <int N> int samurai_protection_r();
	DECLARE_INPUT_CHANGED_MEMBER(coin_changed);

protected:
	required_device<cpu_device> m_maincpu;
	optional_device<samples_device> m_samples;
	optional_device<discrete_sound_device> m_discrete;
	required_device<timer_device> m_coinstate_timer;
	optional_device<timer_device> m_nsub_coinage_timer;
	required_device<screen_device> m_screen;
	optional_device<vicdual_audio_device_base> m_vicdual_sound;
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
	void videoram_w(offs_t offset, uint8_t data);
	void characterram_w(offs_t offset, uint8_t data);
	void palette_bank_w(uint8_t data);

	// game specific
	uint8_t depthch_io_r(offs_t offset);
	void depthch_io_w(offs_t offset, uint8_t data);
	uint8_t safari_io_r(offs_t offset);
	void safari_io_w(offs_t offset, uint8_t data);
	uint8_t frogs_io_r(offs_t offset);
	void frogs_io_w(offs_t offset, uint8_t data);
	uint8_t headon_io_r(offs_t offset);
	uint8_t sspaceat_io_r(offs_t offset);
	void headon_io_w(offs_t offset, uint8_t data);
	DECLARE_MACHINE_RESET(headon2);
	uint8_t headon2_io_r(offs_t offset);
	void headon2_io_w(offs_t offset, uint8_t data);
	void digger_io_w(offs_t offset, uint8_t data);
	void invho2_io_w(offs_t offset, uint8_t data);
	void invds_io_w(offs_t offset, uint8_t data);
	void carhntds_io_w(offs_t offset, uint8_t data);
	void sspacaho_io_w(offs_t offset, uint8_t data);
	void headonn_io_w(offs_t offset, uint8_t data);
	void spacetrk_io_w(offs_t offset, uint8_t data);
	void brdrline_io_w(offs_t offset, uint8_t data);
	void pulsar_io_w(offs_t offset, uint8_t data);
	void heiankyo_io_w(offs_t offset, uint8_t data);
	void alphaho_io_w(offs_t offset, uint8_t data);
	void samurai_protection_w(uint8_t data);
	void samurai_io_w(offs_t offset, uint8_t data);
	uint8_t invinco_io_r(offs_t offset);
	void invinco_io_w(offs_t offset, uint8_t data);

	/*----------- defined in audio/vicdual.cpp -----------*/
	void headon_audio_w(uint8_t data);
	void invho2_audio_w(uint8_t data);

	/*----------- defined in audio/depthch.cpp -----------*/
	void depthch_audio_w(uint8_t data);

	/*----------- defined in audio/invinco.cpp -----------*/
	void invinco_audio_w(uint8_t data);

	/*----------- defined in audio/pulsar.cpp -----------*/
	void pulsar_audio_1_w(uint8_t data);
	void pulsar_audio_2_w(uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(clear_coin_status);

	DECLARE_MACHINE_START(samurai);

	virtual void machine_start() override ATTR_COLD;

	uint32_t screen_update_bw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bw_or_color(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_color(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	int get_vcounter();
	int is_cabinet_color();
	virtual pen_t choose_pen(uint8_t x, uint8_t y, pen_t back_pen);

	void alphaho_io_map(address_map &map) ATTR_COLD;
	void brdrline_io_map(address_map &map) ATTR_COLD;
	void carhntds_dualgame_map(address_map &map) ATTR_COLD;
	void carhntds_io_map(address_map &map) ATTR_COLD;
	void depthch_io_map(address_map &map) ATTR_COLD;
	void depthch_map(address_map &map) ATTR_COLD;
	void digger_io_map(address_map &map) ATTR_COLD;
	void frogs_io_map(address_map &map) ATTR_COLD;
	void frogs_map(address_map &map) ATTR_COLD;
	void headon2_io_map(address_map &map) ATTR_COLD;
	void headon2_map(address_map &map) ATTR_COLD;
	void headon_io_map(address_map &map) ATTR_COLD;
	void headon_map(address_map &map) ATTR_COLD;
	void headonn_io_map(address_map &map) ATTR_COLD;
	void heiankyo_io_map(address_map &map) ATTR_COLD;
	void invds_io_map(address_map &map) ATTR_COLD;
	void invho2_io_map(address_map &map) ATTR_COLD;
	void invinco_io_map(address_map &map) ATTR_COLD;
	void invinco_map(address_map &map) ATTR_COLD;
	void pulsar_io_map(address_map &map) ATTR_COLD;
	void safari_io_map(address_map &map) ATTR_COLD;
	void safari_map(address_map &map) ATTR_COLD;
	void samurai_io_map(address_map &map) ATTR_COLD;
	void samurai_map(address_map &map) ATTR_COLD;
	void spacetrk_io_map(address_map &map) ATTR_COLD;
	void sspacaho_io_map(address_map &map) ATTR_COLD;
	void sspaceat_io_map(address_map &map) ATTR_COLD;
	void vicdual_dualgame_map(address_map &map) ATTR_COLD;
};

class tranqgun_state : public vicdual_state
{
public:
	tranqgun_state(const machine_config &mconfig, device_type type, const char *tag) :
		vicdual_state(mconfig, type, tag)
	{ }

	void brdrlinet(machine_config &config);
	void tranqgun(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void tranqgun_io_map(address_map &map) ATTR_COLD;
	void tranqgun_io_w(offs_t offset, uint8_t data);

	uint8_t tranqgun_prot_r(offs_t offset);
	void tranqgun_prot_w(offs_t offset, uint8_t data);

	void brdrlinet_dualgame_map(address_map &map) ATTR_COLD;
	void tranqgun_dualgame_map(address_map &map) ATTR_COLD;

	uint8_t m_tranqgun_prot_return = 0;

	uint8_t brdrlinet_prot_r();
	void brdrlinet_prot_w(uint8_t data);
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

	int m_nsub_coin_counter = 0;
	int m_nsub_play_counter = 0;

	uint8_t nsub_io_r(offs_t offset);
	void nsub_io_w(offs_t offset, uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(nsub_coin_pulse);

	DECLARE_MACHINE_START(nsub);
	DECLARE_MACHINE_RESET(nsub);

	virtual pen_t choose_pen(uint8_t x, uint8_t y, pen_t back_pen) override;
	void nsub_io_map(address_map &map) ATTR_COLD;
	void nsub_map(address_map &map) ATTR_COLD;
};

class carnival_state : public vicdual_state
{
public:
	carnival_state(const machine_config &mconfig, device_type type, const char *tag) :
		vicdual_state(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu"),
		m_psg(*this, "psg"),
		m_pit(*this, "pit"),
		m_dac(*this, "dac%u", 0)
	{ }

	void carnival(machine_config &config);
	void carnivalb(machine_config &config);

	void carnivala_audio(machine_config &config);
	void carnivalb_audio(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	required_device<i8035_device> m_audiocpu;
	optional_device<ay8910_device> m_psg;
	optional_device<pit8253_device> m_pit;
	optional_device_array<dac_bit_interface, 3> m_dac;

	void carnival_io_map(address_map &map) ATTR_COLD;
	void mboard_map(address_map &map) ATTR_COLD;

	int m_musicData = 0;
	int m_musicBus = 0;

	void carnival_io_w(offs_t offset, uint8_t data);

	/*----------- defined in audio/carnival.cpp -----------*/
	void carnival_audio_1_w(uint8_t data);
	void carnival_audio_2_w(uint8_t data);
	int carnival_music_port_t1_r();
	void carnivala_music_port_1_w(uint8_t data);
	void carnivala_music_port_2_w(uint8_t data);
	void carnival_psg_latch();
	void carnivalb_music_port_1_w(uint8_t data);
	void carnivalb_music_port_2_w(uint8_t data);
};

class carnivalh_state : public carnival_state
{
public:
	carnivalh_state(const machine_config &mconfig, device_type type, const char *tag) :
		carnival_state(mconfig, type, tag)
	{ }

	void carnivalh(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	uint8_t carnivalh_prot_r(offs_t offset);
	void carnivalh_prot_w(offs_t offset, uint8_t data);

	void carnivalh_dualgame_map(address_map &map) ATTR_COLD;

	uint16_t m_previousaddress = 0;
	uint8_t m_previousvalue = 0;
};

class headonsa_state : public vicdual_state
{
public:
	headonsa_state(const machine_config &mconfig, device_type type, const char *tag) :
		vicdual_state(mconfig, type, tag)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(headonsa_coin_inserted);
};
