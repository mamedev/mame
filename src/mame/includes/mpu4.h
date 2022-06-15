// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"
#include "machine/timer.h"

#include "cpu/m6809/m6809.h"
#include "sound/ay8910.h"
#include "sound/okim6376.h"
#include "sound/upd7759.h"
#include "sound/ymopl.h"

#include "machine/bacta_datalogger.h"
#include "machine/meters.h"
#include "machine/mpu4_characteriser_bootleg.h"
#include "machine/mpu4_characteriser_pal.h"
#include "machine/mpu4_characteriser_pal_bwb.h"
#include "machine/roc10937.h"
#include "machine/steppers.h"


#include "emupal.h"


#define MPU4_MASTER_CLOCK           XTAL(6'880'000)
#define VIDEO_MASTER_CLOCK          XTAL(10'000'000)

#ifdef MAME_DEBUG
#define MPU4VIDVERBOSE 1
#else
#define MPU4VIDVERBOSE 0
#endif

#define LOGSTUFF(x) do { if (MPU4VIDVERBOSE) logerror x; } while (0)



#ifdef MAME_DEBUG
#define MPU4VERBOSE 1
#else
#define MPU4VERBOSE 0
#endif

#define LOG(x)  do { if (MPU4VERBOSE) logerror x; } while (0)
#define LOG_CHR(x)  do { if (MPU4VERBOSE) logerror x; } while (0)
#define LOG_CHR_FULL(x) do { if (MPU4VERBOSE) logerror x; } while (0)
#define LOG_IC3(x)  do { if (MPU4VERBOSE) logerror x; } while (0)
#define LOG_IC8(x)  do { if (MPU4VERBOSE) logerror x; } while (0)
#define LOG_SS(x)   do { if (MPU4VERBOSE) logerror x; } while (0)


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
#define TUBES               0
#define HOPPER_DUART_A      1
#define HOPPER_DUART_B      2
#define HOPPER_DUART_C      3
#define HOPPER_NONDUART_A   4
#define HOPPER_NONDUART_B   5

INPUT_PORTS_EXTERN( mpu4 );
INPUT_PORTS_EXTERN( mpu4_invcoin );
INPUT_PORTS_EXTERN( mpu4_impcoin );
INPUT_PORTS_EXTERN( mpu4_invimpcoin );
INPUT_PORTS_EXTERN( mpu4_cw );
INPUT_PORTS_EXTERN( mpu420p );
INPUT_PORTS_EXTERN( mpu4jackpot8per );
INPUT_PORTS_EXTERN( mpu4jackpot8tkn );
INPUT_PORTS_EXTERN( mpu4jackpot8tkn20p );
INPUT_PORTS_EXTERN( mpu4jackpot8tkn20p90pc );

// currently in mpu4.cpp this may get moved into the driver, or renamed to something more generic based on the setup
INPUT_PORTS_EXTERN( grtecp );

class mpu4_state : public driver_device
{
public:
	mpu4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_vfd(*this, "vfd")
		, m_6840ptm(*this, "ptm_ic2")
		, m_ptm_ic3ss(*this, "ptm_ic3ss")
		, m_pia3(*this, "pia_ic3")
		, m_pia4(*this, "pia_ic4")
		, m_pia5(*this, "pia_ic5")
		, m_pia6(*this, "pia_ic6")
		, m_pia7(*this, "pia_ic7")
		, m_pia8(*this, "pia_ic8")
		, m_pia_ic4ss(*this, "pia_ic4ss")
		, m_port_mux(*this, {"ORANGE1", "ORANGE2", "BLACK1", "BLACK2", "ORANGE1", "ORANGE2", "DIL1", "DIL2"})
		, m_aux1_port(*this, "AUX1")
		, m_aux2_port(*this, "AUX2")
		, m_bank1(*this, "bank1")
		, m_msm6376(*this, "msm6376")
		, m_reel(*this, "reel%u", 0U)
		, m_palette(*this, "palette")
		, m_meters(*this, "meters")
		, m_ym2413(*this, "ym2413")
		, m_ay8913(*this, "ay8913")
		, m_dataport(*this, "dataport")
		, m_characteriser(*this, "characteriser")
		, m_characteriser_bl(*this, "characteriser_bl")
		, m_characteriser_blastbank(*this, "characteriser_blastbank")
		, m_characteriser_bwb(*this, "characteriser_bwb")
		, m_duart68681(*this, "duart68681")
		, m_lamps(*this, "lamp%u", 0U)
		, m_mpu4leds(*this, "mpu4led%u", 0U)
		, m_digits(*this, "digit%u", 0U)
		, m_triacs(*this, "triac%u", 0U)
	 { }

	void init_m4default_alt();
	void init_m4default();
	void init_m4default_big();
	void init_m4default_big_low();


	void init_m4default_big_aux2inv();
	void init_m4default_806prot();

	void init_m4tst2();

	void init_m4default_banks();
	void init_m4default_reels();
	void init_m4_low_volt_alt();


	void init_m4_five_reel_std();
	void init_m4_five_reel_rev();
	void init_m4_five_reel_alt();
	void init_m4_six_reel_std();
	void init_m4_six_reel_alt();
	void init_m4_seven_reel();
	void init_m4_small_extender();
	void init_m4_large_extender_b();
	void init_m4_large_extender_c();
	void init_m4_hopper_tubes();
	void init_m4_hopper_duart_a();
	void init_m4_hopper_duart_b();
	void init_m4_hopper_duart_c();
	void init_m4_hopper_nonduart_a();
	void init_m4_hopper_nonduart_b();
	void init_m4_led_a();
	void init_m4_led_b();
	void init_m4_led_c();
	void init_m4_led_simple();


	void init_m4_andycp10c();
	void init_m_blsbys();
	void init_m_oldtmr();
	void init_m4tst();
	void init_big_extenda();

	void init_m4altreels();//legacy, will be removed once things are sorted out
	void init_m4altreels_big();

	void bwboki(machine_config &config);
	void bwboki_chr(machine_config &config);

	template<const uint32_t* Key> void bwboki_chr_cheat(machine_config &config)
	{
		bwboki(config);
		m_maincpu->set_addrmap(AS_PROGRAM, &mpu4_state::mpu4_memmap_characteriser_bwb);
		MPU4_CHARACTERISER_PAL_BWB(config, m_characteriser_bwb, 0);
		m_characteriser_bwb->set_common_key(Key[0] & 0xff);
		m_characteriser_bwb->set_other_key(Key[1]);
	}




	void mod2(machine_config &config);
	void mod2_cheatchr(machine_config &config);
	void mod2_chr(machine_config &config);

	template<const uint8_t ReelNo, uint8_t Type>
	void mpu4_add_reel(machine_config& config);

	template<uint8_t Type, uint8_t NumberOfReels>
	void mpu4_reels(machine_config &config);

	template<const uint8_t* Table> void mod2_cheatchr_pal(machine_config &config)
	{
		mod2(config);

		m_maincpu->set_addrmap(AS_PROGRAM, &mpu4_state::mpu4_memmap_characteriser);

		MPU4_CHARACTERISER_PAL(config, m_characteriser, 0);
		m_characteriser->set_cpu_tag("maincpu");
		m_characteriser->set_allow_6809_cheat(true);
		m_characteriser->set_lamp_table(Table);
	}

	template<const uint8_t* Table> void mod2_alt_cheatchr_pal(machine_config &config)
	{
		mod2_alt(config);

		m_maincpu->set_addrmap(AS_PROGRAM, &mpu4_state::mpu4_memmap_characteriser);

		MPU4_CHARACTERISER_PAL(config, m_characteriser, 0);
		m_characteriser->set_cpu_tag("maincpu");
		m_characteriser->set_allow_6809_cheat(true);
		m_characteriser->set_lamp_table(Table);
	}

	template<const uint8_t* Table> void mod4oki_cheatchr_pal(machine_config &config)
	{
		mod4oki(config);

		m_maincpu->set_addrmap(AS_PROGRAM, &mpu4_state::mpu4_memmap_characteriser);

		MPU4_CHARACTERISER_PAL(config, m_characteriser, 0);
		m_characteriser->set_cpu_tag("maincpu");
		m_characteriser->set_allow_6809_cheat(true);
		m_characteriser->set_lamp_table(Table);
	}

	template<const uint8_t* Table> void mod4oki_alt_cheatchr_pal(machine_config &config)
	{
		mod4oki_alt(config);

		m_maincpu->set_addrmap(AS_PROGRAM, &mpu4_state::mpu4_memmap_characteriser);

		MPU4_CHARACTERISER_PAL(config, m_characteriser, 0);
		m_characteriser->set_cpu_tag("maincpu");
		m_characteriser->set_allow_6809_cheat(true);
		m_characteriser->set_lamp_table(Table);
	}

	template<const uint8_t* Table> void mod4yam_cheatchr_pal(machine_config &config)
	{
		mod4yam(config);

		m_maincpu->set_addrmap(AS_PROGRAM, &mpu4_state::mpu4_memmap_characteriser);

		MPU4_CHARACTERISER_PAL(config, m_characteriser, 0);
		m_characteriser->set_cpu_tag("maincpu");
		m_characteriser->set_allow_6809_cheat(true);
		m_characteriser->set_lamp_table(Table);
	}

	template<const uint8_t* Table> void mod4oki_5r_cheatchr_pal(machine_config &config)
	{
		mod4oki_5r(config);

		m_maincpu->set_addrmap(AS_PROGRAM, &mpu4_state::mpu4_memmap_characteriser);

		MPU4_CHARACTERISER_PAL(config, m_characteriser, 0);
		m_characteriser->set_cpu_tag("maincpu");
		m_characteriser->set_allow_6809_cheat(true);
		m_characteriser->set_lamp_table(Table);
	}

	template<uint8_t Fixed> void mod4oki_5r_bootleg_fixedret(machine_config &config)
	{
		mod4oki_5r(config);

		m_maincpu->set_addrmap(AS_PROGRAM, &mpu4_state::mpu4_memmap_bootleg_characteriser);

		MPU4_CHARACTERISER_BL(config, m_characteriser_bl, 0);
		m_characteriser_bl->set_bl_fixed_return(Fixed);
	}

	void mod2_cheatchr_table(machine_config &config, const uint8_t* table);

	// bootleg mod2
	template<uint8_t Fixed> void mod2_bootleg_fixedret(machine_config &config)
	{
		mod2(config);

		m_maincpu->set_addrmap(AS_PROGRAM, &mpu4_state::mpu4_memmap_bootleg_characteriser);

		MPU4_CHARACTERISER_BL(config, m_characteriser_bl, 0);
		m_characteriser_bl->set_bl_fixed_return(Fixed);
	}

	template<uint8_t Fixed> void mod4yam_bootleg_fixedret(machine_config &config)
	{
		mod4yam(config);

		m_maincpu->set_addrmap(AS_PROGRAM, &mpu4_state::mpu4_memmap_bootleg_characteriser);

		MPU4_CHARACTERISER_BL(config, m_characteriser_bl, 0);
		m_characteriser_bl->set_bl_fixed_return(Fixed);
	}

	template<uint8_t Fixed> void mod4oki_bootleg_fixedret(machine_config &config)
	{
		mod4oki(config);

		m_maincpu->set_addrmap(AS_PROGRAM, &mpu4_state::mpu4_memmap_bootleg_characteriser);

		MPU4_CHARACTERISER_BL(config, m_characteriser_bl, 0);
		m_characteriser_bl->set_bl_fixed_return(Fixed);
	}

	template<uint8_t Fixed> void mod4oki_alt_bootleg_fixedret(machine_config &config)
	{
		mod4oki_alt(config);

		m_maincpu->set_addrmap(AS_PROGRAM, &mpu4_state::mpu4_memmap_bootleg_characteriser);

		MPU4_CHARACTERISER_BL(config, m_characteriser_bl, 0);
		m_characteriser_bl->set_bl_fixed_return(Fixed);
	}


	void mod2_chr_blastbnk(machine_config &config);
	void mod2_chr_copcash(machine_config &config);

	void mod2_alt(machine_config &config);
	void mod2_alt_cheatchr(machine_config &config);
	void mod2_alt_cheatchr_table(machine_config &config, const uint8_t* table);

	void mod4oki_5r(machine_config &config);
	void mod4oki_5r_chr(machine_config &config);
	void mod4oki_5r_cheatchr(machine_config &config);
	void mod4oki_5r_cheatchr_table(machine_config &config, const uint8_t* table);

	void mod4oki_alt(machine_config &config);
	void mod4oki_alt_cheatchr(machine_config &config);
	void mod4oki_alt_cheatchr_table(machine_config& config, const uint8_t* table);

	void mod4oki(machine_config &config);
	void mod4oki_cheatchr(machine_config &config);
	void mod4oki_cheatchr_table(machine_config &config, const uint8_t* table);
	void mod4oki_chr(machine_config &config);

	void mod4yam(machine_config &config);
	void mod4yam_cheatchr(machine_config &config);
	void mod4yam_cheatchr_table(machine_config& config, const uint8_t* table);
	void mod4yam_chr(machine_config &config);

	void mpu4_common(machine_config &config);
	void mpu4_common2(machine_config &config);
	void mpu4crys(machine_config &config);
	void mpu4base(machine_config &config);

protected:
	TIMER_CALLBACK_MEMBER(update_ic24);

	void mpu4_memmap(address_map &map);
	void mpu4_memmap_characteriser(address_map &map);
	void mpu4_memmap_bootleg_characteriser(address_map &map);
	void mpu4_memmap_bl_characteriser_blastbank(address_map &map);
	void mpu4_memmap_characteriser_bwb(address_map &map);

	void lamp_extend_small(int data);
	void lamp_extend_large(int data,int column,int active);
	void led_write_extender(int latch, int data, int column);
	void update_meters();
	void ic23_update();
	void ic24_output(int data);
	void ic24_setup();
	void update_ay(device_t *device);
	void mpu4_install_mod4yam_space(address_space &space);
	void mpu4_install_mod4oki_space(address_space &space);
	void mpu4_install_mod4bwb_space(address_space &space);
	void mpu4_config_common();

	DECLARE_MACHINE_START(mod2);
	DECLARE_MACHINE_RESET(mpu4);
	DECLARE_MACHINE_START(mpu4yam);
	DECLARE_MACHINE_START(mpu4oki);
	DECLARE_MACHINE_START(mpu4bwb);
	DECLARE_MACHINE_START(mpu4cry);

	TIMER_DEVICE_CALLBACK_MEMBER(gen_50hz);

	template <unsigned N> DECLARE_WRITE_LINE_MEMBER(reel_optic_cb) { if (state) m_optic_pattern |= (1 << N); else m_optic_pattern &= ~(1 << N); }

	void bankswitch_w(uint8_t data);
	uint8_t bankswitch_r();
	void bankset_w(uint8_t data);

	void mpu4_ym2413_w(offs_t offset, uint8_t data);
	uint8_t mpu4_ym2413_r(offs_t offset);

	void ic3ss_w(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(cpu0_irq);
	DECLARE_WRITE_LINE_MEMBER(ic2_o1_callback);
	DECLARE_WRITE_LINE_MEMBER(ic2_o2_callback);
	DECLARE_WRITE_LINE_MEMBER(ic2_o3_callback);
	void pia_ic3_porta_w(uint8_t data);
	void pia_ic3_portb_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(pia_ic3_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pia_ic3_cb2_w);
	void pia_ic4_porta_w(uint8_t data);
	void pia_ic4_portb_w(uint8_t data);
	uint8_t pia_ic4_portb_r();
	DECLARE_WRITE_LINE_MEMBER(pia_ic4_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pia_ic4_cb2_w);
	uint8_t pia_ic5_porta_r();
	void pia_ic5_porta_w(uint8_t data);
	void pia_ic5_portb_w(uint8_t data);
	uint8_t pia_ic5_portb_r();
	DECLARE_WRITE_LINE_MEMBER(pia_ic5_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pia_ic5_cb2_w);
	void pia_ic6_portb_w(uint8_t data);
	void pia_ic6_porta_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(pia_ic6_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pia_ic6_cb2_w);
	void pia_ic7_porta_w(uint8_t data);
	void pia_ic7_portb_w(uint8_t data);
	uint8_t pia_ic7_portb_r();
	DECLARE_WRITE_LINE_MEMBER(pia_ic7_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pia_ic7_cb2_w);
	uint8_t pia_ic8_porta_r();
	void pia_ic8_portb_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(pia_ic8_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pia_ic8_cb2_w);
	void pia_gb_porta_w(uint8_t data);
	void pia_gb_portb_w(uint8_t data);
	uint8_t pia_gb_portb_r();
	DECLARE_WRITE_LINE_MEMBER(pia_gb_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pia_gb_cb2_w);

	DECLARE_WRITE_LINE_MEMBER(dataport_rxd);

	uint8_t hack_duart_r()
	{
		return machine().rand() & 0x10;
	}

	uint8_t bootleg806_r(address_space &space, offs_t offset);

	required_device<cpu_device> m_maincpu;
	optional_device<rocvfd_device> m_vfd;
	optional_device<ptm6840_device> m_6840ptm;
	optional_device<ptm6840_device> m_ptm_ic3ss;
	optional_device<pia6821_device> m_pia3;
	optional_device<pia6821_device> m_pia4;
	optional_device<pia6821_device> m_pia5;
	optional_device<pia6821_device> m_pia6;
	optional_device<pia6821_device> m_pia7;
	optional_device<pia6821_device> m_pia8;
	optional_device<pia6821_device> m_pia_ic4ss;
	required_ioport_array<8> m_port_mux;
	required_ioport m_aux1_port;
	required_ioport m_aux2_port;
	optional_memory_bank m_bank1;
	optional_device<okim6376_device> m_msm6376;
	optional_device_array<stepper_device, 8> m_reel;
	optional_device<palette_device> m_palette;
	required_device<meters_device> m_meters;
	optional_device<ym2413_device> m_ym2413;
	optional_device<ay8913_device> m_ay8913;
	optional_device<bacta_datalogger_device> m_dataport;
	optional_device<mpu4_characteriser_pal> m_characteriser;
	optional_device<mpu4_characteriser_bl> m_characteriser_bl;
	optional_device<mpu4_characteriser_bl_blastbank> m_characteriser_blastbank;
	optional_device<mpu4_characteriser_pal_bwb> m_characteriser_bwb;

	optional_device<mc68681_device> m_duart68681;

	// not all systems have this many lamps/LEDs/digits but the driver is too much of a mess to split up now

	// 0-63 are on PIA IC3 port A (always present)
	// 64-127 are on PIA IC3 port B (always present)
	// 128-132 136-140 144-148 152-156 160-164 168-172 176-180 184-188 are on small lamp extender
	// 128-255 are on large lamp externders
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

	int m_mod_number = 0;
	int m_mmtr_data = 0;
	int m_ay8913_address = 0;
	int m_serial_data = 0;
	int m_signal_50hz = 0;
	int m_ic4_input_b = 0;
	int m_aux1_input = 0;
	int m_aux2_input = 0;
	int m_IC23G1 = 0;
	int m_IC23G2A = 0;
	int m_IC23G2B = 0;
	int m_IC23GC = 0;
	int m_IC23GB = 0;
	int m_IC23GA = 0;

	int m_reel_flag = 0;
	int m_ic23_active = 0;
	int m_led_lamp = 0;
	int m_link7a_connected = 0;
	int m_low_volt_detect_disable = 0;
	emu_timer *m_ic24_timer = nullptr;
	int m_expansion_latch = 0;
	int m_global_volume = 0;
	int m_input_strobe = 0;
	uint8_t m_lamp_strobe = 0;
	uint8_t m_lamp_strobe2 = 0;
	uint8_t m_lamp_strobe_ext = 0;
	uint8_t m_lamp_strobe_ext_persistence = 0;
	uint8_t m_led_strobe = 0;
	uint8_t m_ay_data = 0;
	int m_optic_pattern = 0;

	int m_active_reel = 0;
	int m_remote_meter = 0;
	int m_reel_mux = 0;
	int m_lamp_extender = 0;
	int m_last_b7 = 0;
	int m_last_latch = 0;
	int m_lamp_sense = 0;
	int m_card_live = 0;
	int m_led_extender = 0;
	int m_bwb_bank = 0;
	bool m_default_to_low_bank = false;

	int m_pageval = 0;
	int m_pageset = 0;
	int m_hopper = 0;
	int m_reels = 0;
	int m_chrdata = 0;
	int m_t1 = 0;
	int m_t3l = 0;
	int m_t3h = 0;
	uint8_t m_numbanks = 0;

	static constexpr uint8_t reel_mux_table[8]= {0,4,2,6,1,5,3,7};//include 7, although I don't think it's used, this is basically a wire swap
	static constexpr uint8_t reel_mux_table7[8]= {3,1,5,6,4,2,0,7};
};

