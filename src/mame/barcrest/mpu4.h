// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "mpu4_characteriser_bootleg.h"
#include "mpu4_characteriser_pal.h"
#include "mpu4_characteriser_pal_bwb.h"
#include "mpu4_oki_sampled_sound.h"


#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/bacta_datalogger.h"
#include "machine/mc68681.h"
#include "machine/meters.h"
#include "machine/nvram.h"
#include "machine/roc10937.h"
#include "machine/steppers.h"
#include "machine/ticket.h"
#include "machine/timer.h" //hoppers
#include "sound/ay8910.h"
#include "sound/dac.h"


#define MPU4_MASTER_CLOCK           XTAL(6'880'000)

//reel info
#define STANDARD_REEL  0    // As originally designed 3/4 reels
#define FIVE_REEL_5TO8 1    // Interfaces to meter port, allows some mechanical metering, but there is significant 'bounce' in the extra reel
#define FIVE_REEL_8TO5 2    // Mounted backwards for space reasons, but different board
#define FIVE_REEL_3TO6 3    // Connected to the centre of the meter connector, taking up meters 3 to 6
#define SIX_REEL_1TO8  4    // Two reels on the meter drives
#define SIX_REEL_5TO8  5    // Like FIVE_REEL_5TO8, but with an extra reel elsewhere
#define SEVEN_REEL     6    // Mainly club machines, significant reworking of reel hardware
#define FLUTTERBOX     7    // A fan feature using a reel mux-like setup, but not actually a reel

//Lamp extension
#define NO_EXTENDER         0 // As originally designed
#define SMALL_CARD          1
#define LARGE_CARD_A        2 //96 Lamps
#define LARGE_CARD_B        3 //96 Lamps, 16 LEDs - as used by BwB
#define LARGE_CARD_C        4 //Identical to B, no built in LED support

//LED cards
#define CARD_A          1
#define CARD_B          2
#define CARD_C          3
#define SIMPLE_CARD     4


//Hopper info
#define TUBES              0
#define HOPPER_DUART_A     1
#define HOPPER_DUART_B     2
#define HOPPER_DUART_C     3
#define HOPPER_NONDUART_A  4
#define HOPPER_NONDUART_B  5
#define HOPPER_TWIN_HOPPER 6

INPUT_PORTS_EXTERN( mpu4 );
INPUT_PORTS_EXTERN( mpu4_dutch );
INPUT_PORTS_EXTERN( mpu4_dutch_invcoin );
INPUT_PORTS_EXTERN( mpu4_dutch_alt_invcoin );
INPUT_PORTS_EXTERN( mpu4_invcoin );
INPUT_PORTS_EXTERN( mpu4_impcoin );
INPUT_PORTS_EXTERN( mpu4_invimpcoin );
INPUT_PORTS_EXTERN( mpu4_cw );
INPUT_PORTS_EXTERN( mpu420p );
INPUT_PORTS_EXTERN( mpu4jackpot8per );
INPUT_PORTS_EXTERN( mpu4jackpot8tkn );
INPUT_PORTS_EXTERN( mpu4jackpot8tkn20p );
INPUT_PORTS_EXTERN( mpu4jackpot10_20p );
INPUT_PORTS_EXTERN( mpu4jackpot8tkn20p90pc );
INPUT_PORTS_EXTERN( mpu4_70pc );

// currently in mpu4.cpp this may get moved into the driver, or renamed to something more generic based on the setup
INPUT_PORTS_EXTERN( grtecp );

namespace mpu4_traits {
	enum {
		// Reels configurations
		R4,
		R5,
		R5R,    // reversed
		R5A,    // alternative
		R6,
		R6A,    // alternative
		R7,
		R8,

		// Reel types (must be set after reel count)
		RT1,    // 1-3
		RT2,    // 4-12
		RT3,    // 96-3

		// Lamp extenders
		LPS,    // small
		LPLA,   // large A
		LPLB,   // large B
		LPLC,   // large C

		// Led extenders
		LDS,    // simple
		LDA,    // card A
		LDB,    // card B
		LDC,    // card C

		// Hopper
		HT,     // tubes
		HDA,    // duart type A
		HDB,    // duart type B
		HDC,    // duart type C
		HNA,    // non-duart type A
		HNB,    // non-duart type B
		HTW,    // twin

		// Features
		OVER,   // overcurrent detection
		LVDOFF, // Disable 50hz check
		P4L,    // use pia4 port a leds
		SCARDL, // use simple card leds
	};
}


class mpu4_state : public driver_device
{
public:
	mpu4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_vfd(*this, "vfd")
		, m_6840ptm(*this, "ptm_ic2")
		, m_pia3(*this, "pia_ic3")
		, m_pia4(*this, "pia_ic4")
		, m_pia5(*this, "pia_ic5")
		, m_pia6(*this, "pia_ic6")
		, m_pia7(*this, "pia_ic7")
		, m_pia8(*this, "pia_ic8")
		, m_port_mux(*this, {"ORANGE1", "ORANGE2", "BLACK1", "BLACK2", "ORANGE1", "ORANGE2", "DIL1", "DIL2"})
		, m_aux1_port(*this, "AUX1")
		, m_aux2_port(*this, "AUX2")
		, m_bank1(*this, "bank1")
		, m_reel(*this, "reel%u", 0U)
		, m_meters(*this, "meters")
		, m_ay8913(*this, "ay8913")
		, m_alarmdac(*this, "alarmdac")
		, m_dataport(*this, "dataport")
		, m_characteriser(*this, "characteriser")
		, m_characteriser_bl(*this, "characteriser_bl")
		, m_characteriser_blastbank(*this, "characteriser_blastbank")
		, m_characteriser_bwb(*this, "characteriser_bwb")
		, m_okicard(*this, "okicard")
		, m_duart68681(*this, "duart68681")
		, m_hopper1(*this, "hopper")
		, m_hopper2(*this, "hopper2")
		, m_lamps(*this, "lamp%u", 0U)
		, m_mpu4leds(*this, "mpu4led%u", 0U)
		, m_digits(*this, "digit%u", 0U)
		, m_triacs(*this, "triac%u", 0U)
		, m_flutterbox(*this, "flutterbox")

	 { }

	void init_m4();
	void init_m4big();
	void init_m4big_low();

	void mpu4_reels(machine_config &config, uint8_t NumberOfReels, int16_t start_index, int16_t end_index);

	void tr_r4(machine_config &config);
	void tr_r5(machine_config &config);
	void tr_r5r(machine_config &config);
	void tr_r5a(machine_config &config);
	void tr_r6(machine_config &config);
	void tr_r6a(machine_config &config);
	void tr_r7(machine_config &config);
	void tr_r8(machine_config &config);
	void tr_rt1(machine_config &config);
	void tr_rt2(machine_config &config);
	void tr_rt3(machine_config &config);
	void tr_lps(machine_config &config);
	void tr_lpla(machine_config &config);
	void tr_lplb(machine_config &config);
	void tr_lplc(machine_config &config);
	void tr_lds(machine_config &config);
	void tr_lda(machine_config &config);
	void tr_ldb(machine_config &config);
	void tr_ldc(machine_config &config);
	void tr_ht(machine_config &config);
	void tr_hda(machine_config &config);
	void tr_hdb(machine_config &config);
	void tr_hdc(machine_config &config);
	void tr_hna(machine_config &config);
	void tr_hnb(machine_config &config);
	void tr_htw(machine_config &config);
	void tr_over(machine_config &config);
	void tr_lvdoff(machine_config &config);
	void tr_p4l(machine_config &config);
	void tr_scardl(machine_config &config);

	template <typename Class, unsigned Count>
	struct trait_wrapper_impl
	{
	public:
		using config_func = void (Class::*)(machine_config &);

		template <typename... T>
		trait_wrapper_impl(Class *_state, T... traits)
			: state(_state)
			, fragments{ apply(traits)... }
		{
		}

		void operator()(machine_config &config)
		{
			for (auto &t : fragments)
				(state->*t)(config);
		}

	private:
		config_func apply(int trait)
		{
			switch (trait)
			{
			case mpu4_traits::R4:     return &mpu4_state::tr_r4;
			case mpu4_traits::R5:     return &mpu4_state::tr_r5;
			case mpu4_traits::R5R:    return &mpu4_state::tr_r5r;
			case mpu4_traits::R5A:    return &mpu4_state::tr_r5a;
			case mpu4_traits::R6:     return &mpu4_state::tr_r6;
			case mpu4_traits::R6A:    return &mpu4_state::tr_r6a;
			case mpu4_traits::R7:     return &mpu4_state::tr_r7;
			case mpu4_traits::R8:     return &mpu4_state::tr_r8;
			case mpu4_traits::RT1:    return &mpu4_state::tr_rt1;
			case mpu4_traits::RT2:    return &mpu4_state::tr_rt2;
			case mpu4_traits::RT3:    return &mpu4_state::tr_rt3;
			case mpu4_traits::LPS:    return &mpu4_state::tr_lps;
			case mpu4_traits::LPLA:   return &mpu4_state::tr_lpla;
			case mpu4_traits::LPLB:   return &mpu4_state::tr_lplb;
			case mpu4_traits::LPLC:   return &mpu4_state::tr_lplc;
			case mpu4_traits::LDS:    return &mpu4_state::tr_lds;
			case mpu4_traits::LDA:    return &mpu4_state::tr_lda;
			case mpu4_traits::LDB:    return &mpu4_state::tr_ldb;
			case mpu4_traits::LDC:    return &mpu4_state::tr_ldc;
			case mpu4_traits::HT:     return &mpu4_state::tr_ht;
			case mpu4_traits::HDA:    return &mpu4_state::tr_hda;
			case mpu4_traits::HDB:    return &mpu4_state::tr_hdb;
			case mpu4_traits::HDC:    return &mpu4_state::tr_hdc;
			case mpu4_traits::HNA:    return &mpu4_state::tr_hna;
			case mpu4_traits::HNB:    return &mpu4_state::tr_hnb;
			case mpu4_traits::HTW:    return &mpu4_state::tr_htw;
			case mpu4_traits::OVER:   return &mpu4_state::tr_over;
			case mpu4_traits::LVDOFF: return &mpu4_state::tr_lvdoff;
			case mpu4_traits::P4L:    return &mpu4_state::tr_p4l;
			case mpu4_traits::SCARDL: return &mpu4_state::tr_scardl;
			default: return nullptr; // crash later on invalid arguments
			}
		}

		config_func apply(config_func f)
		{
			return f;
		}

		Class *state;
		config_func fragments[Count];
	};

	template <typename T, typename... U>
	auto trait_wrapper(T *state, U... traits)
	{
		return trait_wrapper_impl<T, sizeof...(U)>(state, traits...);
	}


	void mod2_f(machine_config &config);
	void mod2_no_bacta_f(machine_config &config);
	void mod2_cheatchr_f(machine_config &config);

	template<typename... T>
	auto mod2(T... traits)
	{
		return trait_wrapper(this, &mpu4_state::mod2_f, traits...);
	}

	template<typename... T>
	auto mod2_no_bacta(T... traits)
	{
		return trait_wrapper(this, &mpu4_state::mod2_no_bacta_f, traits...);
	}

	template<typename... T>
	auto mod2_cheatchr(T... traits)
	{
		return trait_wrapper(this, &mpu4_state::mod2_cheatchr_f, traits...);
	}


	void mod4oki_f(machine_config &config);
	void mod4oki_no_bacta_f(machine_config &config);
	void mod4oki_cheatchr_f(machine_config &config);
	template<const uint8_t* Table> void mod4oki_cheatchr_pal_f(machine_config &config);
	template<uint8_t Fixed> void mod4oki_bootleg_fixedret_f(machine_config &config);

	template<typename... T>
	auto mod4oki(T... traits)
	{
		return trait_wrapper(this, &mpu4_state::mod4oki_f, traits...);
	}

	template<typename... T>
	auto mod4oki_no_bacta(T... traits)
	{
		return trait_wrapper(this, &mpu4_state::mod4oki_no_bacta_f, traits...);
	}

	template<typename... T>
	auto mod4oki_cheatchr(T... traits)
	{
		return trait_wrapper(this, &mpu4_state::mod4oki_cheatchr_f, traits...);
	}

	template<const uint8_t* Table, typename... T>
	auto mod4oki_cheatchr_pal(T... traits)
	{
		return trait_wrapper(this, &mpu4_state::mod4oki_cheatchr_pal_f<Table>, traits...);
	}

	template<uint8_t Fixed, typename... T>
	auto mod4oki_bootleg_fixedret(T... traits)
	{
		return trait_wrapper(this, &mpu4_state::mod4oki_bootleg_fixedret_f<Fixed>, traits...);
	}


	void mpu4_common(machine_config &config);
	void mpu4base(machine_config &config);
	void mpu4_bacta(machine_config &config);

	void pia_gb_cb2_w(int state);

protected:
	void setup_rom_banks();

	TIMER_CALLBACK_MEMBER(update_ic24);

	void mpu4_memmap(address_map &map) ATTR_COLD;
	void mpu4_memmap_characteriser(address_map &map) ATTR_COLD;
	void mpu4_memmap_bootleg_characteriser(address_map &map) ATTR_COLD;
	void mpu4_memmap_bl_characteriser_blastbank(address_map &map) ATTR_COLD;

	void lamp_extend_small(uint8_t data);
	void lamp_extend_large(uint8_t data, uint8_t column, bool active);
	void led_write_extender(uint8_t latch, uint8_t data, uint8_t column);
	void update_meters();
	void ic23_update();
	void ic24_output(uint8_t data);
	void ic24_setup();
	void update_ay();
	void mpu4_install_mod4oki_space(address_space &space);
	void mpu4_config_common();

	DECLARE_MACHINE_START(mod2);
	DECLARE_MACHINE_RESET(mpu4);
	DECLARE_MACHINE_START(mpu4oki);

	TIMER_DEVICE_CALLBACK_MEMBER(gen_50hz);

	void bankswitch_w(uint8_t data);
	uint8_t bankswitch_r();
	void bankset_w(uint8_t data);

	void cpu0_irq(int state);
	void ic2_o1_callback(int state);
	void ic2_o2_callback(int state);
	void ic2_o3_callback(int state);
	void pia_ic3_porta_w(uint8_t data);
	void pia_ic3_portb_w(uint8_t data);
	void pia_ic3_ca2_w(int state);
	void pia_ic3_cb2_w(int state);
	void pia_ic4_porta_w(uint8_t data);
	void pia_ic4_portb_w(uint8_t data);
	uint8_t pia_ic4_portb_r();
	void pia_ic4_ca2_w(int state);
	void pia_ic4_cb2_w(int state);
	uint8_t pia_ic5_porta_r();
	void pia_ic5_porta_w(uint8_t data);
	void pia_ic5_portb_w(uint8_t data);
	uint8_t pia_ic5_portb_r();
	void pia_ic5_cb2_w(int state);
	void pia_ic6_portb_w(uint8_t data);
	void pia_ic6_porta_w(uint8_t data);
	void pia_ic6_ca2_w(int state);
	void pia_ic6_cb2_w(int state);
	void pia_ic7_porta_w(uint8_t data);
	void pia_ic7_portb_w(uint8_t data);
	uint8_t pia_ic7_portb_r();
	void pia_ic7_ca2_w(int state);
	void pia_ic7_cb2_w(int state);
	uint8_t pia_ic8_porta_r();
	void pia_ic8_portb_w(uint8_t data);
	void pia_ic8_ca2_w(int state);
	void pia_ic8_cb2_w(int state);


	void dataport_rxd(int state);


	uint8_t bootleg806_r(address_space &space, offs_t offset);

	required_device<cpu_device> m_maincpu;
	optional_device<rocvfd_device> m_vfd;
	optional_device<ptm6840_device> m_6840ptm;
	optional_device<pia6821_device> m_pia3;
	optional_device<pia6821_device> m_pia4;
	optional_device<pia6821_device> m_pia5;
	optional_device<pia6821_device> m_pia6;
	optional_device<pia6821_device> m_pia7;
	optional_device<pia6821_device> m_pia8;
	required_ioport_array<8> m_port_mux;
	required_ioport m_aux1_port;
	required_ioport m_aux2_port;
	optional_memory_bank m_bank1;
	optional_device_array<stepper_device, 8> m_reel;
	required_device<meters_device> m_meters;
	optional_device<ay8910_device> m_ay8913;
	required_device<dac_1bit_device> m_alarmdac;
	optional_device<bacta_datalogger_device> m_dataport;
	optional_device<mpu4_characteriser_pal> m_characteriser;
	optional_device<mpu4_characteriser_bl> m_characteriser_bl;
	optional_device<mpu4_characteriser_bl_blastbank> m_characteriser_blastbank;
	optional_device<mpu4_characteriser_pal_bwb> m_characteriser_bwb;
	optional_device<mpu4_oki_sampled_sound> m_okicard;

	optional_device<mc68681_device> m_duart68681;

	optional_device<hopper_device> m_hopper1;
	optional_device<hopper_device> m_hopper2;

	// not all systems have this many lamps/LEDs/digits but the driver is too much of a mess to split up now

	// 0-63 are on PIA IC3 port A (always present)
	// 64-127 are on PIA IC3 port B (always present)
	// 128-132 136-140 144-148 152-156 160-164 168-172 176-180 184-188 are on small lamp extender
	// 128-255 are on large lamp extenders
	output_finder<256> m_lamps;

	// 0-63 are on PIA IC4 port A (always present)
	// 0-143 are on card B (possibly incorrectly mapped?)
	// 64-127 are on card C
	// 0-127 are on large card B
	output_finder<144> m_mpu4leds;

	// 0-7 are on PIA IC4 port A with no LED extender
	// 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128, 136 are on card B (possible incorrectly mapped?)
	// 8-15 are on card C
	// 8-9 are mapped to lamp lines for Connect 4
	// 0-15 are on large card B
	output_finder<144> m_digits;

	output_finder<8> m_triacs;

	output_finder<> m_flutterbox;

	uint8_t m_mmtr_data = 0;
	uint8_t m_ay8913_address = 0;
	uint8_t m_signal_50hz = 0;
	uint8_t m_ic4_input_b = 0;
	uint8_t m_aux1_input = 0;
	uint8_t m_aux2_input = 0;
	uint8_t m_IC23G1 = 0;
	uint8_t m_IC23G2A = 0;
	uint8_t m_IC23G2B = 0;
	uint8_t m_IC23GC = 0;
	uint8_t m_IC23GB = 0;
	uint8_t m_IC23GA = 0;

	uint8_t m_reel_flag = 0;
	bool m_ic23_active = false;
	emu_timer *m_ic24_timer = nullptr;
	uint8_t m_input_strobe = 0;
	uint8_t m_lamp_strobe = 0;
	uint8_t m_lamp_strobe2 = 0;
	uint8_t m_lamp_strobe_ext[2] = { 0, 0 };
	uint8_t m_lamp_strobe_ext_persistence = 0;
	uint8_t m_led_strobe = 0;
	uint8_t m_ay_data = 0;
	uint8_t m_optic_pattern = 0;

	uint8_t m_active_reel = 0;
	uint8_t m_remote_meter = 0;
	uint8_t m_reel_mux = 0;
	uint8_t m_lamp_extender = 0;
	uint8_t m_last_b7 = 0;
	uint8_t m_last_latch = 0;
	bool m_lamp_sense = false;
	bool m_card_live = false;
	uint8_t m_led_extender = 0;
	bool m_bwb_bank = false;
	bool m_default_to_low_bank = false;

	bool m_use_pia4_porta_leds = true;
	uint8_t m_pia4_porta_leds_base = 0;
	uint8_t m_pia4_porta_leds_strobe = 0;

	bool m_use_simplecard_leds = false;
	uint8_t m_simplecard_leds_base = 0;
	uint8_t m_simplecard_leds_strobe = 0;

	uint8_t m_pageval = 0;
	uint8_t m_pageset = 0;
	uint8_t m_hopper_type = 0;
	uint8_t m_reels = 0;
	uint8_t m_chrdata = 0;
	uint8_t m_serial_output = 0;

	uint8_t m_numbanks = 0;

	bool m_link7a_connected = false;
	bool m_link7b_connected = true;

	bool m_overcurrent = false;
	bool m_undercurrent = false;

	bool m_overcurrent_detect = false;
	bool m_undercurrent_detect = false;

	bool m_low_volt_detect = true;

	bool m_use_coinlocks = false;

	bool m_hack_duart_fixed_low = false;

	bool m_hopper1_opto = false;
	bool m_hopper2_opto = false;

	static constexpr uint8_t reel_mux_table[8]= {0,4,2,6,1,5,3,7};//include 7, although I don't think it's used, this is basically a wire swap
	static constexpr uint8_t reel_mux_table7[8]= {3,1,5,6,4,2,0,7};
};



template<const uint8_t* Table> void mpu4_state::mod4oki_cheatchr_pal_f(machine_config &config)
{
	mod4oki_f(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &mpu4_state::mpu4_memmap_characteriser);

	MPU4_CHARACTERISER_PAL(config, m_characteriser, 0);
	m_characteriser->set_cpu_tag("maincpu");
	m_characteriser->set_allow_6809_cheat(true);
	m_characteriser->set_lamp_table(Table);
}

template<uint8_t Fixed> void mpu4_state::mod4oki_bootleg_fixedret_f(machine_config &config)
{
	mod4oki_f(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &mpu4_state::mpu4_memmap_bootleg_characteriser);

	MPU4_CHARACTERISER_BL(config, m_characteriser_bl, 0);
	m_characteriser_bl->set_bl_fixed_return(Fixed);
}
