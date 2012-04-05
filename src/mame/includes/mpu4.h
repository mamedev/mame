#define MPU4_MASTER_CLOCK (6880000)
#define VIDEO_MASTER_CLOCK			XTAL_10MHz


#ifdef MAME_DEBUG
#define MPU4VIDVERBOSE 1
#else
#define MPU4VIDVERBOSE 0
#endif

#define LOGSTUFF(x) do { if (MPU4VIDVERBOSE) logerror x; } while (0)
#define LOG2674(x) do { if (MPU4VIDVERBOSE) logerror x; } while (0)



#ifdef MAME_DEBUG
#define MPU4VERBOSE 1
#else
#define MPU4VERBOSE 0
#endif

#define LOG(x)	do { if (MPU4VERBOSE) logerror x; } while (0)
#define LOG_CHR(x)	do { if (MPU4VERBOSE) logerror x; } while (0)
#define LOG_CHR_FULL(x)	do { if (MPU4VERBOSE) logerror x; } while (0)
#define LOG_IC3(x)	do { if (MPU4VERBOSE) logerror x; } while (0)
#define LOG_IC8(x)	do { if (MPU4VERBOSE) logerror x; } while (0)
#define LOG_SS(x)	do { if (MPU4VERBOSE) logerror x; } while (0)





static const UINT8 reel_mux_table[8]= {0,4,2,6,1,5,3,7};//include 7, although I don't think it's used, this is basically a wire swap
static const UINT8 reel_mux_table7[8]= {3,1,5,6,4,2,0,7};
static const UINT8 vsync_table[4] = {3,1,5,7}; //Video related

static const UINT8 bwb_chr_table_common[10]= {0x00,0x04,0x04,0x0c,0x0c,0x1c,0x14,0x2c,0x5c,0x2c};

#define STANDARD_REEL  0	// As originally designed 3/4 reels
#define FIVE_REEL_5TO8 1	// Interfaces to meter port, allows some mechanical metering, but there is significant 'bounce' in the extra reel
#define FIVE_REEL_8TO5 2	// Mounted backwards for space reasons, but different board
#define FIVE_REEL_3TO6 3	// Connected to the centre of the meter connector, taking up meters 3 to 6
#define SIX_REEL_1TO8  4	// Two reels on the meter drives
#define SIX_REEL_5TO8  5	// Like FIVE_REEL_5TO8, but with an extra reel elsewhere
#define SEVEN_REEL     6	// Mainly club machines, significant reworking of reel hardware
#define FLUTTERBOX     7	// Will you start the fans, please!  A fan using a reel mux-like setup, but not actually a reel

#define NO_EXTENDER			0 // As originally designed
#define SMALL_CARD			1
#define LARGE_CARD_A		2 //96 Lamps
#define LARGE_CARD_B		3 //96 Lamps, 16 LEDs - as used by BwB
#define LARGE_CARD_C		4 //Identical to B, no built in LED support

#define CARD_A			1
#define CARD_B			2
#define CARD_C			3

#define TUBES				0
#define HOPPER_DUART_A		1
#define HOPPER_DUART_B		2
#define HOPPER_DUART_C		3
#define HOPPER_NONDUART_A	4
#define HOPPER_NONDUART_B	5

/* Lookup table for CHR data */

struct mpu4_chr_table
{
	UINT8 call;
	UINT8 response;
};

struct bwb_chr_table//dynamically populated table for BwB protection
{
	UINT8 response;
};

/* Video stuff - see mpu4drvr.c */
struct ef9369_t
{
	UINT32 addr;
	UINT16 clut[16];	/* 13-bits - a marking bit and a 444 color */
};

struct bt471_t
{
	UINT8 address;
	UINT8 addr_cnt;
	UINT8 pixmask;
	UINT8 command;
	rgb_t color;
};


class mpu4_state : public driver_device
{
public:
	mpu4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
	{
		return 0;
	}

	int m_mod_number;
	int m_mmtr_data;
	int m_alpha_data_line;
	int m_alpha_clock;
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
	emu_timer *m_ic24_timer;
	int m_expansion_latch;
	int m_global_volume;
	int m_input_strobe;
	UINT8 m_lamp_strobe;
	UINT8 m_lamp_strobe2;
	UINT8 m_lamp_strobe_ext;
	UINT8 m_lamp_strobe_ext_persistence;
	UINT8 m_led_strobe;
	UINT8 m_ay_data;
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
	mpu4_chr_table* m_current_chr_table;
	const bwb_chr_table* m_bwb_chr_table1;
	//Video
	UINT8 m_m6840_irq_state;
	UINT8 m_m6850_irq_state;
	UINT8 m_scn2674_irq_state;
	UINT8 m_m68k_m6809_line;
	UINT8 m_m6809_m68k_line;
	UINT8 m_m68k_acia_cts;
	UINT8 m_m6809_acia_cts;
	UINT8 m_m6809_acia_rts;
	UINT8 m_m6809_acia_dcd;
	int m_gfx_index;
	UINT16 * m_vid_vidram;
	UINT16 * m_vid_mainram;
//  UINT8 m_scn2674_IR[16];
	UINT8 m_scn2674_IR_pointer;
	UINT8 m_scn2674_screen1_l;
	UINT8 m_scn2674_screen1_h;
	UINT8 m_scn2674_cursor_l;
	UINT8 m_scn2674_cursor_h;
	UINT8 m_scn2674_screen2_l;
	UINT8 m_scn2674_screen2_h;
	UINT8 m_scn2674_irq_register;
	UINT8 m_scn2674_status_register;
	UINT8 m_scn2674_irq_mask;
	UINT8 m_scn2674_gfx_enabled;
	UINT8 m_scn2674_display_enabled;
	UINT8 m_scn2674_display_enabled_field;
	UINT8 m_scn2674_display_enabled_scanline;
	UINT8 m_scn2674_cursor_enabled;
	UINT8 m_IR0_scn2674_double_ht_wd;
	UINT8 m_IR0_scn2674_scanline_per_char_row;
	UINT8 m_IR0_scn2674_sync_select;
	UINT8 m_IR0_scn2674_buffer_mode_select;
	UINT8 m_IR1_scn2674_interlace_enable;
	UINT8 m_IR1_scn2674_equalizing_constant;
	UINT8 m_IR2_scn2674_row_table;
	UINT8 m_IR2_scn2674_horz_sync_width;
	UINT8 m_IR2_scn2674_horz_back_porch;
	UINT8 m_IR3_scn2674_vert_front_porch;
	UINT8 m_IR3_scn2674_vert_back_porch;
	UINT8 m_IR4_scn2674_rows_per_screen;
	UINT8 m_IR4_scn2674_character_blink_rate_divisor;
	UINT8 m_IR5_scn2674_character_per_row;
	UINT8 m_IR6_scn2674_cursor_first_scanline;
	UINT8 m_IR6_scn2674_cursor_last_scanline;
	UINT8 m_IR7_scn2674_cursor_underline_position;
	UINT8 m_IR7_scn2674_cursor_rate_divisor;
	UINT8 m_IR7_scn2674_cursor_blink;
	UINT8 m_IR7_scn2674_vsync_width;
	UINT8 m_IR8_scn2674_display_buffer_first_address_LSB;
	UINT8 m_IR9_scn2674_display_buffer_first_address_MSB;
	UINT8 m_IR9_scn2674_display_buffer_last_address;
	UINT8 m_IR10_scn2674_display_pointer_address_lower;
	UINT8 m_IR11_scn2674_display_pointer_address_upper;
	UINT8 m_IR11_scn2674_reset_scanline_counter_on_scrollup;
	UINT8 m_IR11_scn2674_reset_scanline_counter_on_scrolldown;
	UINT8 m_IR12_scn2674_scroll_start;
	UINT8 m_IR12_scn2674_split_register_1;
	UINT8 m_IR13_scn2674_scroll_end;
	UINT8 m_IR13_scn2674_split_register_2;
	UINT8 m_IR14_scn2674_scroll_lines;
	UINT8 m_IR14_scn2674_double_1;
	UINT8 m_IR14_scn2674_double_2;
	UINT8 m_scn2674_horz_front_porch;
	UINT8 m_scn2674_spl1;
	UINT8 m_scn2674_spl2;
	UINT8 m_scn2674_dbl1;
	INT8 m_cur[2];
	UINT8 *m_dealem_videoram;
	int m_rowcounter;
	int m_linecounter;
	struct ef9369_t m_pal;
	struct bt471_t m_bt471;
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_READ8_MEMBER(bankswitch_r);
	DECLARE_WRITE8_MEMBER(bankset_w);
	DECLARE_WRITE8_MEMBER(characteriser_w);
	DECLARE_READ8_MEMBER(characteriser_r);
	DECLARE_WRITE8_MEMBER(bwb_characteriser_w);
	DECLARE_READ8_MEMBER(bwb_characteriser_r);
	DECLARE_WRITE8_MEMBER(mpu4_ym2413_w);
	DECLARE_READ8_MEMBER(mpu4_ym2413_r);
	DECLARE_READ8_MEMBER(crystal_sound_r);
	DECLARE_WRITE8_MEMBER(crystal_sound_w);
	DECLARE_WRITE8_MEMBER(ic3ss_w);
};

/* mpu4.c, used by mpu4vid.c */
extern READ8_DEVICE_HANDLER( pia_ic5_portb_r );
extern WRITE_LINE_DEVICE_HANDLER( pia_ic5_ca2_w );
extern WRITE_LINE_DEVICE_HANDLER( pia_ic5_cb2_w );
extern WRITE_LINE_DEVICE_HANDLER( cpu0_irq );
extern void mpu4_config_common(running_machine &machine);
extern void mpu4_stepper_reset(mpu4_state *state);

MACHINE_CONFIG_EXTERN( mpu4_common );
MACHINE_CONFIG_EXTERN( mpu4_common2 );

MACHINE_CONFIG_EXTERN( mod2     );

extern MACHINE_START( mod2     );
extern const ay8910_interface ay8910_config;

INPUT_PORTS_EXTERN( mpu4 );

