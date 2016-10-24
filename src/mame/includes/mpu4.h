// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/nvram.h"

#include "cpu/m6809/m6809.h"
#include "sound/ay8910.h"
#include "sound/okim6376.h"
#include "sound/ym2413.h"
#include "sound/upd7759.h"
#include "machine/steppers.h"
#include "machine/roc10937.h"
#include "machine/meters.h"


#define MPU4_MASTER_CLOCK (6880000)
#define VIDEO_MASTER_CLOCK          XTAL_10MHz


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
#define FLUTTERBOX     7    // Will you start the fans, please!  A fan using a reel mux-like setup, but not actually a reel

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
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_vfd(*this, "vfd"),
			m_6840ptm(*this, "ptm_ic2"),
			m_pia3(*this, "pia_ic3"),
			m_pia4(*this, "pia_ic4"),
			m_pia5(*this, "pia_ic5"),
			m_pia6(*this, "pia_ic6"),
			m_pia7(*this, "pia_ic7"),
			m_pia8(*this, "pia_ic8"),
			m_port_mux(*this, {"ORANGE1", "ORANGE2", "BLACK1", "BLACK2", "ORANGE1", "ORANGE2", "DIL1", "DIL2"}),
			m_aux1_port(*this, "AUX1"),
			m_aux2_port(*this, "AUX2"),
			m_bank1(*this, "bank1"),
			m_msm6376(*this, "msm6376"),
			m_reel0(*this, "reel0"),
			m_reel1(*this, "reel1"),
			m_reel2(*this, "reel2"),
			m_reel3(*this, "reel3"),
			m_reel4(*this, "reel4"),
			m_reel5(*this, "reel5"),
			m_reel6(*this, "reel6"),
			m_reel7(*this, "reel7"),
			m_palette(*this, "palette"),
			m_meters(*this, "meters")
	{}

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
	{
		return 0;
	}

	void bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t bankswitch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bankset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void characteriser_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t characteriser_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bwb_characteriser_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t bwb_characteriser_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mpu4_ym2413_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mpu4_ym2413_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t crystal_sound_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void crystal_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ic3ss_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cpu0_irq(int state);
	void ic2_o1_callback(int state);
	void ic2_o2_callback(int state);
	void ic2_o3_callback(int state);
	void pia_ic3_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pia_ic3_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pia_ic3_ca2_w(int state);
	void pia_ic3_cb2_w(int state);
	void pia_ic4_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pia_ic4_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pia_ic4_portb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pia_ic4_ca2_w(int state);
	void pia_ic4_cb2_w(int state);
	uint8_t pia_ic5_porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pia_ic5_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pia_ic5_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pia_ic5_portb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pia_ic5_ca2_w(int state);
	void pia_ic5_cb2_w(int state);
	void pia_ic6_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pia_ic6_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pia_ic6_ca2_w(int state);
	void pia_ic6_cb2_w(int state);
	void pia_ic7_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pia_ic7_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pia_ic7_portb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pia_ic7_ca2_w(int state);
	void pia_ic7_cb2_w(int state);
	uint8_t pia_ic8_porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pia_ic8_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pia_ic8_ca2_w(int state);
	void pia_ic8_cb2_w(int state);
	void pia_gb_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pia_gb_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pia_gb_portb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pia_gb_ca2_w(int state);
	void pia_gb_cb2_w(int state);
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
	void machine_start_mod2();
	void machine_reset_mpu4();
	void machine_start_mpu4yam();
	void machine_start_mpu4oki();
	void machine_start_mpu4oki_alt();
	void machine_start_mod4oki_5r();
	void machine_start_mod2_alt();
	void machine_start_mpu4bwb();
	void machine_start_mpu4cry();
	void gen_50hz(timer_device &timer, void *ptr, int32_t param);
	void reel0_optic_cb(int state) { if (state) m_optic_pattern |= 0x01; else m_optic_pattern &= ~0x01; }
	void reel1_optic_cb(int state) { if (state) m_optic_pattern |= 0x02; else m_optic_pattern &= ~0x02; }
	void reel2_optic_cb(int state) { if (state) m_optic_pattern |= 0x04; else m_optic_pattern &= ~0x04; }
	void reel3_optic_cb(int state) { if (state) m_optic_pattern |= 0x08; else m_optic_pattern &= ~0x08; }
	void reel4_optic_cb(int state) { if (state) m_optic_pattern |= 0x10; else m_optic_pattern &= ~0x10; }
	void reel5_optic_cb(int state) { if (state) m_optic_pattern |= 0x20; else m_optic_pattern &= ~0x20; }
	void reel6_optic_cb(int state) { if (state) m_optic_pattern |= 0x40; else m_optic_pattern &= ~0x40; }
	void reel7_optic_cb(int state) { if (state) m_optic_pattern |= 0x80; else m_optic_pattern &= ~0x80; }
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void lamp_extend_small(int data);
	void lamp_extend_large(int data,int column,int active);
	void led_write_latch(int latch, int data, int column);
	void update_meters();
	void ic23_update();
	void ic24_output(int data);
	void ic24_setup();
	void update_ay(device_t *device);
	void mpu4_install_mod4yam_space(address_space &space);
	void mpu4_install_mod4oki_space(address_space &space);
	void mpu4_install_mod4bwb_space(address_space &space);
	void mpu4_config_common();

	required_device<cpu_device> m_maincpu;
	optional_device<roc10937_t> m_vfd;
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
	optional_device<okim6376_device> m_msm6376;
	optional_device<stepper_device> m_reel0;
	optional_device<stepper_device> m_reel1;
	optional_device<stepper_device> m_reel2;
	optional_device<stepper_device> m_reel3;
	optional_device<stepper_device> m_reel4;
	optional_device<stepper_device> m_reel5;
	optional_device<stepper_device> m_reel6;
	optional_device<stepper_device> m_reel7;
	optional_device<palette_device> m_palette;
	required_device<meters_device> m_meters;

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
	int m_low_volt_detect_disable;
	int m_aux1_invert;
	int m_aux2_invert;
	int m_door_invert;
	emu_timer *m_ic24_timer;
	int m_expansion_latch;
	int m_global_volume;
	int m_input_strobe;
	uint8_t m_lamp_strobe;
	uint8_t m_lamp_strobe2;
	uint8_t m_lamp_strobe_ext;
	uint8_t m_lamp_strobe_ext_persistence;
	uint8_t m_led_strobe;
	uint8_t m_ay_data;
	int m_optic_pattern;

	int m_active_reel;
	int m_remote_meter;
	int m_reel_mux;
	int m_lamp_extender;
	int m_last_b7;
	int m_last_latch;
	int m_lamp_sense;
	int m_card_live;
	int m_led_extender;
	int m_bwb_bank;
	int m_chr_state;
	int m_chr_counter;
	int m_chr_value;
	int m_bwb_return;
	int m_pageval;
	int m_pageset;
	int m_hopper;
	int m_reels;
	int m_chrdata;
	int m_t1;
	int m_t3l;
	int m_t3h;
	uint8_t m_numbanks;
	mpu4_chr_table* m_current_chr_table;
	const bwb_chr_table* m_bwb_chr_table1;
};

MACHINE_CONFIG_EXTERN( mpu4_common );
MACHINE_CONFIG_EXTERN( mpu4_common2 );

MACHINE_CONFIG_EXTERN( mod2     );
MACHINE_CONFIG_EXTERN( mod4oki_alt );
MACHINE_CONFIG_EXTERN( mod4oki_5r );
MACHINE_CONFIG_EXTERN( mod2_alt );

INPUT_PORTS_EXTERN( mpu4 );
