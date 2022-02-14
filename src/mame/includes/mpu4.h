// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/nvram.h"
#include "machine/timer.h"

#include "cpu/m6809/m6809.h"
#include "sound/ay8910.h"
#include "sound/okim6376.h"
#include "sound/upd7759.h"
#include "sound/ymopl.h"
#include "machine/steppers.h"
#include "machine/roc10937.h"
#include "machine/meters.h"

#include "machine/bacta_datalogger.h"

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





static const uint8_t reel_mux_table[8]= {0,4,2,6,1,5,3,7};//include 7, although I don't think it's used, this is basically a wire swap
static const uint8_t reel_mux_table7[8]= {3,1,5,6,4,2,0,7};

static const uint8_t bwb_chr_table_common[10]= {0x00,0x04,0x04,0x0c,0x0c,0x1c,0x14,0x2c,0x5c,0x2c};

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

/* Lookup table for CHR data */

struct mpu4_chr_table
{
	uint8_t call;
	uint8_t response;
};

struct bwb_chr_table//dynamically populated table for BwB protection
{
	uint8_t response;
};


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
		, m_lamps(*this, "lamp%u", 0U)
		, m_mpu4leds(*this, "mpu4led%u", 0U)
		, m_digits(*this, "digit%u", 0U)
		, m_triacs(*this, "triac%u", 0U)
		, m_current_chr_table(nullptr)
	 { }

	void init_m4default_alt();
	void init_crystali();
	void init_m4tst2();
	void init_crystal();
	void init_m_frkstn();
	void init_m4default_big();
	void init_m4default();
	void init_m4default_banks();
	void init_m4default_reels();
	void init_m4_low_volt_alt();
	void init_m4_aux1_invert();
	void init_m4_aux2_invert();
	void init_m4_door_invert();
	void init_m4_five_reel_std();
	void init_m4_five_reel_rev();
	void init_m4_five_reel_alt();
	void init_m4_six_reel_std();
	void init_m4_six_reel_alt();
	void init_m4_seven_reel();
	void init_m4_small_extender();
	void init_m4_large_extender_a();
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
	void init_m_ccelbr();
	void init_m4gambal();
	void init_m4debug();
	void init_m4_showstring();
	void init_m4_showstring_mod4yam();
	void init_m4_debug_mod4yam();
	void init_m4_showstring_mod2();
	void init_m4_showstring_big();
	void init_connect4();
	void init_m4altreels();//legacy, will be removed once things are sorted out
	void init_m_grtecp();//legacy, will be removed once things are sorted out RE: CHR
	void init_m4tenten();
	void init_m4actbnk();
	void init_m4actclb();
	void init_m4actpak();
	void init_m4addr();
	void init_m4aao();
	void init_m4alladv();
	void init_m4alpha();
	void init_m4andycp();
	void init_m4andybt();
	void init_m4andyfh();
	void init_m4andyge();
	void init_m4apachg();

	void bwboki(machine_config &config);
	void mod2(machine_config &config);
	void mod2_alt(machine_config &config);
	void mod4oki(machine_config &config);
	void mod4oki_5r(machine_config &config);
	void mod4oki_alt(machine_config &config);
	void mod4yam(machine_config &config);
	void mpu4_common(machine_config &config);
	void mpu4_common2(machine_config &config);
	void mpu4crys(machine_config &config);
	void mpu4_std_3reel(machine_config &config);
	void mpu4_type2_3reel(machine_config &config);
	void mpu4_type3_3reel(machine_config &config);
	void mpu4_type4_3reel(machine_config &config);
	void mpu4_bwb_3reel(machine_config &config);
	void mpu4_std_4reel(machine_config &config);
	void mpu4_type2_4reel(machine_config &config);
	void mpu4_type3_4reel(machine_config &config);
	void mpu4_type4_4reel(machine_config &config);
	void mpu4_bwb_4reel(machine_config &config);
	void mpu4_std_5reel(machine_config &config);
	void mpu4_type2_5reel(machine_config &config);
	void mpu4_type3_5reel(machine_config &config);
	void mpu4_type4_5reel(machine_config &config);
	void mpu4_bwb_5reel(machine_config &config);
	void mpu4_std_6reel(machine_config &config);
	void mpu4_type2_6reel(machine_config &config);
	void mpu4_type3_6reel(machine_config &config);
	void mpu4_type4_6reel(machine_config &config);
	void mpu4_bwb_6reel(machine_config &config);
	void mpu4_std_7reel(machine_config &config);
	void mpu4_type2_7reel(machine_config &config);
	void mpu4_type3_7reel(machine_config &config);
	void mpu4_type4_7reel(machine_config &config);
	void mpu4_bwb_7reel(machine_config &config);
	void mpu4base(machine_config &config);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	void mpu4_memmap(address_map &map);
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
	DECLARE_MACHINE_START(mpu4oki_alt);
	DECLARE_MACHINE_START(mod4oki_5r);
	DECLARE_MACHINE_START(mod2_alt);
	DECLARE_MACHINE_START(mpu4bwb);
	DECLARE_MACHINE_START(mpu4cry);
	TIMER_DEVICE_CALLBACK_MEMBER(gen_50hz);
	template <unsigned N> DECLARE_WRITE_LINE_MEMBER(reel_optic_cb) { if (state) m_optic_pattern |= (1 << N); else m_optic_pattern &= ~(1 << N); }
		uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
	{
		return 0;
	}

	void bankswitch_w(uint8_t data);
	uint8_t bankswitch_r();
	void bankset_w(uint8_t data);
	void characteriser_w(offs_t offset, uint8_t data);
	uint8_t characteriser_r(address_space &space, offs_t offset);
	void bwb_characteriser_w(offs_t offset, uint8_t data);
	uint8_t bwb_characteriser_r(offs_t offset);
	void mpu4_ym2413_w(offs_t offset, uint8_t data);
	uint8_t mpu4_ym2413_r(offs_t offset);
	uint8_t crystal_sound_r();
	void crystal_sound_w(uint8_t data);
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

	enum
	{
		TIMER_IC24
	};

	int m_mod_number;
	int m_mmtr_data;
	int m_ay8913_address;
	int m_serial_data;
	int m_signal_50hz;
	int m_ic4_input_b;
	int m_aux1_input;
	int m_aux2_input;
	int m_IC23G1;
	int m_IC23G2A;
	int m_IC23G2B;
	int m_IC23GC;
	int m_IC23GB;
	int m_IC23GA;
	int m_prot_col;
	int m_lamp_col;
	int m_init_col;
	int m_reel_flag;
	int m_ic23_active;
	int m_led_lamp;
	int m_link7a_connected;
	int m_low_volt_detect_disable = 0;
	int m_aux1_invert;
	int m_aux2_invert;
	int m_door_invert;
	emu_timer *m_ic24_timer;
	int m_expansion_latch;
	int m_global_volume;
	int m_input_strobe = 0;
	uint8_t m_lamp_strobe;
	uint8_t m_lamp_strobe2;
	uint8_t m_lamp_strobe_ext;
	uint8_t m_lamp_strobe_ext_persistence;
	uint8_t m_led_strobe;
	uint8_t m_ay_data;
	int m_optic_pattern;

	int m_active_reel;
	int m_remote_meter = 0;
	int m_reel_mux;
	int m_lamp_extender;
	int m_last_b7;
	int m_last_latch;
	int m_lamp_sense;
	int m_card_live;
	int m_led_extender;
	int m_bwb_bank = 0;
	int m_chr_state;
	int m_chr_counter;
	int m_chr_value;
	int m_bwb_return;
	int m_pageval;
	int m_pageset;
	int m_hopper;
	int m_reels = 0;
	int m_chrdata;
	int m_t1;
	int m_t3l;
	int m_t3h;
	uint8_t m_numbanks = 0;
	mpu4_chr_table* m_current_chr_table;
	const bwb_chr_table* m_bwb_chr_table1;
};

INPUT_PORTS_EXTERN( mpu4 );
