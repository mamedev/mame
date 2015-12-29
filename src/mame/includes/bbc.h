// license:BSD-3-Clause
// copyright-holders:Gordon Jefferyes, Nigel Barnes
/*****************************************************************************
 *
 * includes/bbc.h
 *
 * BBC Model B
 *
 * Driver by Gordon Jefferyes <mess_bbc@romvault.com>
 *
 ****************************************************************************/

#ifndef BBC_H_
#define BBC_H_

#include "bus/rs232/rs232.h"
#include "machine/6522via.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/mc6854.h"
#include "machine/ram.h"
#include "machine/i8271.h"
#include "machine/wd_fdc.h"
#include "machine/upd7002.h"
#include "machine/mc146818.h"
#include "video/mc6845.h"
#include "video/saa5050.h"
#include "sound/sn76496.h"
#include "sound/tms5220.h"
#include "imagedev/cassette.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#define RS232_TAG       "rs232"

enum machine_type_t
{
	MODELA,
	MODELB,
	BPLUS,
	MASTER,
	COMPACT
};

class bbc_state : public driver_device
{
public:
	bbc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG),
		m_hd6845(*this, "hd6845"),
		m_adlc(*this, "mc6854"),
		m_sn(*this, "sn76489"),
		m_trom(*this, "saa5050"),
		m_tms(*this, "tms5220"),
		m_cassette(*this, "cassette"),
		m_acia(*this, "acia6850"),
		m_acia_clock(*this, "acia_clock"),
		m_rs232(*this, RS232_TAG),
		m_via6522_0(*this, "via6522_0"),
		m_via6522_1(*this, "via6522_1"),
		m_upd7002(*this, "upd7002"),
		m_rtc(*this, "rtc"),
		m_i8271(*this, "i8271"),
		m_wd1770(*this, "wd1770"),
		m_wd1772(*this, "wd1772"),
		m_exp1(*this, "exp_rom1"),
		m_exp2(*this, "exp_rom2"),
		m_exp3(*this, "exp_rom3"),
		m_exp4(*this, "exp_rom4"),
		m_joy0(*this, "JOY0"),
		m_joy1(*this, "JOY1"),
		m_joy2(*this, "JOY2"),
		m_joy3(*this, "JOY3"),
		m_region_maincpu(*this, "maincpu"),
		m_region_os(*this, "os"),
		m_region_opt(*this, "option"),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2"),
		m_bank3(*this, "bank3"),
		m_bank4(*this, "bank4"),
		m_bank5(*this, "bank5"),
		m_bank6(*this, "bank6"),
		m_bank7(*this, "bank7"),
		m_bank8(*this, "bank8"),
		m_ACCCON_IRR(CLEAR_LINE),
		m_via_system_irq(CLEAR_LINE),
		m_via_user_irq(CLEAR_LINE),
		m_acia_irq(CLEAR_LINE),
		m_palette(*this, "palette")
	{ }

	DECLARE_FLOPPY_FORMATS(floppy_formats_bbc);
	DECLARE_FLOPPY_FORMATS(floppy_formats_bbcm);
	DECLARE_FLOPPY_FORMATS(floppy_formats_bbcmc);

	DECLARE_WRITE8_MEMBER(bbc_page_selecta_w);
	DECLARE_WRITE8_MEMBER(bbc_memorya1_w);
	DECLARE_WRITE8_MEMBER(bbc_page_selectb_w);
	DECLARE_WRITE8_MEMBER(bbc_memoryb3_w);
	DECLARE_WRITE8_MEMBER(bbc_memoryb4_w);
	DECLARE_WRITE8_MEMBER(bbc_page_selectbp_w);
	DECLARE_WRITE8_MEMBER(bbc_memorybp1_w);
	DECLARE_WRITE8_MEMBER(bbc_memorybp2_w);
	DECLARE_WRITE8_MEMBER(bbc_memorybp4_w);
	DECLARE_WRITE8_MEMBER(bbc_memorybp4_128_w);
	DECLARE_WRITE8_MEMBER(bbc_memorybp6_128_w);
	DECLARE_READ8_MEMBER(bbcm_ACCCON_read);
	DECLARE_WRITE8_MEMBER(bbcm_ACCCON_write);
	DECLARE_WRITE8_MEMBER(page_selectbm_w);
	DECLARE_WRITE8_MEMBER(bbc_memorybm1_w);
	DECLARE_WRITE8_MEMBER(bbc_memorybm2_w);
	DECLARE_WRITE8_MEMBER(bbc_memorybm4_w);
	DECLARE_WRITE8_MEMBER(bbc_memorybm5_w);
	DECLARE_WRITE8_MEMBER(bbc_memorybm7_w);
	DECLARE_READ8_MEMBER(bbcm_r);
	DECLARE_WRITE8_MEMBER(bbcm_w);
	DECLARE_WRITE8_MEMBER(bbc_SerialULA_w);

	DECLARE_WRITE8_MEMBER(bbc_wd1770_status_w);
	DECLARE_READ8_MEMBER(bbcm_wd177xl_read);
	DECLARE_WRITE8_MEMBER(bbcm_wd1770l_write);
	DECLARE_WRITE8_MEMBER(bbcm_wd1772l_write);
	DECLARE_WRITE8_MEMBER(bbc_videoULA_w);
	DECLARE_READ8_MEMBER(bbc_fe_r);
	DECLARE_DIRECT_UPDATE_MEMBER(bbcbp_direct_handler);
	DECLARE_DIRECT_UPDATE_MEMBER(bbcm_direct_handler);

	DECLARE_DRIVER_INIT(bbc);

	DECLARE_MACHINE_START(bbca);
	DECLARE_MACHINE_RESET(bbca);
	DECLARE_VIDEO_START(bbca);

	DECLARE_MACHINE_START(bbcb);
	DECLARE_MACHINE_RESET(bbcb);
	DECLARE_VIDEO_START(bbcb);

	DECLARE_MACHINE_START(torch);
	DECLARE_MACHINE_RESET(torch);

	DECLARE_MACHINE_START(bbcbp);
	DECLARE_MACHINE_RESET(bbcbp);
	DECLARE_VIDEO_START(bbcbp);

	DECLARE_MACHINE_START(bbcm);
	DECLARE_MACHINE_RESET(bbcm);
	DECLARE_VIDEO_START(bbcm);

	DECLARE_MACHINE_START(bbcmc);
	DECLARE_MACHINE_RESET(bbcmc);

	DECLARE_PALETTE_INIT(bbc);
	INTERRUPT_GEN_MEMBER(bbcb_vsync);
	INTERRUPT_GEN_MEMBER(bbcb_keyscan);
	TIMER_CALLBACK_MEMBER(bbc_tape_timer_cb);
	DECLARE_WRITE_LINE_MEMBER(write_acia_clock);
	DECLARE_WRITE_LINE_MEMBER(bbcb_acia6850_irq_w);
	DECLARE_WRITE_LINE_MEMBER(econet_clk_w);
	DECLARE_WRITE8_MEMBER(bbcb_via_system_write_porta);
	DECLARE_WRITE8_MEMBER(bbcb_via_system_write_portb);
	DECLARE_READ8_MEMBER(bbcb_via_system_read_porta);
	DECLARE_READ8_MEMBER(bbcb_via_system_read_portb);
	DECLARE_WRITE_LINE_MEMBER(bbcb_via_system_irq_w);
	DECLARE_READ8_MEMBER(bbcb_via_user_read_portb);
	DECLARE_WRITE8_MEMBER(bbcb_via_user_write_portb);
	DECLARE_WRITE_LINE_MEMBER(bbcb_via_user_irq_w);
	DECLARE_WRITE_LINE_MEMBER(bbc_vsync);
	void update_acia_rxd();
	void update_acia_dcd();
	void update_acia_cts();
	DECLARE_WRITE_LINE_MEMBER(bbc_rts_w);
	DECLARE_WRITE_LINE_MEMBER(bbc_txd_w);
	DECLARE_WRITE_LINE_MEMBER(write_rxd_serial);
	DECLARE_WRITE_LINE_MEMBER(write_dcd_serial);
	DECLARE_WRITE_LINE_MEMBER(write_cts_serial);
	DECLARE_INPUT_CHANGED_MEMBER( trigger_reset );
	DECLARE_WRITE_LINE_MEMBER(fdc_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	DECLARE_WRITE_LINE_MEMBER(motor_w);
	DECLARE_WRITE_LINE_MEMBER(side_w);

	UPD7002_GET_ANALOGUE(BBC_get_analogue_input);
	UPD7002_EOC(BBC_uPD7002_EOC);

	void bbc_setup_banks(memory_bank *membank, int banks, UINT32 shift, UINT32 size);
	void bbcm_setup_banks(memory_bank *membank, int banks, UINT32 shift, UINT32 size);

	int bbc_load_rom(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(exp1_load) { return bbc_load_rom(image, m_exp1); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(exp2_load) { return bbc_load_rom(image, m_exp2); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(exp3_load) { return bbc_load_rom(image, m_exp3); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(exp4_load) { return bbc_load_rom(image, m_exp4); }

	int bbcm_load_cart(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(bbcm_exp1_load) { return bbcm_load_cart(image, m_exp1); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(bbcm_exp2_load) { return bbcm_load_cart(image, m_exp2); }

	MC6845_UPDATE_ROW(crtc_update_row);

private:
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<hd6845_device> m_hd6845;
	optional_device<mc6854_device> m_adlc;
	optional_device<sn76489_device> m_sn;
public: // HACK FOR MC6845
	optional_device<saa5050_device> m_trom;
	optional_device<tms5220_device> m_tms;
	optional_device<cassette_image_device> m_cassette;
	optional_device<acia6850_device> m_acia;
	optional_device<clock_device> m_acia_clock;
	optional_device<rs232_port_device> m_rs232;
	required_device<via6522_device> m_via6522_0;
	optional_device<via6522_device> m_via6522_1;
	optional_device<upd7002_device> m_upd7002;
	optional_device<mc146818_device> m_rtc;
	optional_device<i8271_device> m_i8271;
	optional_device<wd1770_t> m_wd1770;
	optional_device<wd1772_t> m_wd1772;
	required_device<generic_slot_device> m_exp1;
	required_device<generic_slot_device> m_exp2;
	optional_device<generic_slot_device> m_exp3;
	optional_device<generic_slot_device> m_exp4;
	optional_ioport m_joy0, m_joy1, m_joy2, m_joy3;

	required_memory_region m_region_maincpu;
	required_memory_region m_region_os;
	required_memory_region m_region_opt;
	required_memory_bank m_bank1; // bbca bbcb bbcbp bbcbp128 bbcm
	optional_memory_bank m_bank2; //           bbcbp bbcbp128 bbcm
	optional_memory_bank m_bank3; // bbca bbcb
	required_memory_bank m_bank4; // bbca bbcb bbcbp bbcbp128 bbcm
	optional_memory_bank m_bank5; //                          bbcm
	optional_memory_bank m_bank6; //           bbcbp bbcbp128
	required_memory_bank m_bank7; // bbca bbcb bbcbp bbcbp128 bbcm
	optional_memory_bank m_bank8; //                          bbcm

	void check_interrupts();

	machine_type_t m_machinetype;

	bool m_os01;            // flag indicating whether OS 0.1 is being used
	int m_SWRAMtype;        // this stores the DIP switch setting for the SWRAM type being used
	int m_Speech;           // this stores the CONF setting for Speech enabled/disabled

	int m_ACCCON_IRR;       // IRQ inputs

	int m_rombank;          // This is the latch that holds the sideways ROM bank to read

	int m_userport;         // This stores the sideways RAM latch type.
							// Acorn and others use the bbc_rombank latch to select the write bank to be used.(type 0)
							// Solidisc use the BBC's userport to select the write bank to be used (type 1)

	int m_pagedRAM;         // BBC B+ memory handling
	int m_vdusel;           // BBC B+ memory handling

							/*
							ACCCON

							b7 IRR  1=Causes an IRQ to the processor
							b6 TST  1=Selects &FC00-&FEFF read from OS-ROM
							b5 IFJ  1=Internal 1 MHz bus
							        0=External 1MHz bus
							b4 ITU  1=Internal Tube
							        0=External Tube
							b3 Y    1=Read/Write HAZEL &C000-&DFFF RAM
							        0=Read/Write ROM &C000-&DFFF OS-ROM
							b2 X    1=Read/Write LYNNE
							        0=Read/WRITE main memory &3000-&8000
							b1 E    1=Causes shadow if VDU code
							        0=Main all the time
							b0 D    1=Display LYNNE as screen
							        0=Display main RAM screen

							ACCCON is a read/write register
							*/

	int m_ACCCON;
	int m_ACCCON_TST;
	int m_ACCCON_IFJ;
	int m_ACCCON_ITU;
	int m_ACCCON_Y;
	int m_ACCCON_X;
	int m_ACCCON_E;
	int m_ACCCON_D;


							/*
							The addressable latch
							This 8 bit addressable latch is operated from port B lines 0-3.
							PB0-PB2 are set to the required address of the output bit to be set.
							PB3 is set to the value which should be programmed at that bit.
							The function of the 8 output bits from this latch are:-

							B0 - Write Enable to the sound generator IC
							B1 - READ select on the speech processor
							B2 - WRITE select on the speech processor
							B3 - Keyboard write enable
							B4,B5 - these two outputs define the number to be added to the
							start of screen address in hardware to control hardware scrolling:-
							Mode    Size    Start of screen  Number to add  B5      B4
							0,1,2   20K &3000        12K        1       1
							3       16K &4000        16K        0   0
							4,5     10K &5800 (or &1800) 22K        1   0
							6       8K  &6000 (or &2000) 24K        0   1
							B6 - Operates the CAPS lock LED  (Pin 17 keyboard connector)
							B7 - Operates the SHIFT lock LED (Pin 16 keyboard connector)
							*/

	int m_b0_sound;
	int m_b1_speech_read;
	int m_b2_speech_write;
	int m_b3_keyboard;
	int m_b4_video0;
	int m_b5_video1;
	int m_b6_caps_lock_led;
	int m_b7_shift_lock_led;

	int m_MC146818_WR;      // FE30 bit 1 replaces  b1_speech_read
	int m_MC146818_DS;      // FE30 bit 2 replaces  b2_speech_write
	int m_MC146818_AS;      // 6522 port b bit 7
	int m_MC146818_CE;      // 6522 port b bit 6

	int m_via_system_porta;
	int m_via_system_irq;
	int m_via_user_irq;
	int m_acia_irq;

	int m_column;           // this is a counter in the keyboard circuit


							/***************************************
							  BBC 2C199 Serial Interface Cassette
							****************************************/

	double m_last_dev_val;
	int m_wav_len;
	int m_len0;
	int m_len1;
	int m_len2;
	int m_len3;
	int m_mc6850_clock;
	UINT8 m_serproc_data;
	int m_rxd_serial;
	int m_dcd_serial;
	int m_cts_serial;
	int m_dcd_cass;
	int m_rxd_cass;
	int m_cass_out_enabled;
	int m_txd;
	UINT32 m_nr_high_tones;
	int m_cass_out_samples_to_go;
	int m_cass_out_bit;
	int m_cass_out_phase;
	emu_timer *m_tape_timer;


							/**************************************
							   WD1770 disc control
							***************************************/

	int m_drive_control;
	int m_fdc_irq;
	int m_fdc_drq;

							/**************************************
							   Video Code
							***************************************/

	int m_memorySize;


// this is the real location of the start of the BBC's ram in the emulation
// it can be changed if shadow ram is being used to point at the upper 32K of RAM

// this is the screen memory location of the next pixels to be drawn

// this is a more global variable to store the bitmap variable passed in in the bbc_vh_screenrefresh function

// this is the X and Y screen location in emulation pixels of the next pixels to be drawn


	unsigned char *m_BBC_Video_RAM;
	UINT16 *m_BBC_display;
	UINT16 *m_BBC_display_left;
	UINT16 *m_BBC_display_right;
	bitmap_ind16 *m_BBC_bitmap;
	int m_y_screen_pos;
	unsigned char m_pixel_bits[256];
	int m_BBC_HSync;
	int m_BBC_VSync;

	int m_Teletext_Latch;
	int m_VideoULA_CR;
	int m_VideoULA_CR_counter;
	int m_videoULA_Reg;
	int m_videoULA_master_cursor_size;
	int m_videoULA_width_of_cursor;
	int m_videoULA_6845_clock_rate;
	int m_videoULA_characters_per_line;
	int m_videoULA_teletext_normal_select;
	int m_videoULA_flash_colour_select;

	int m_pixels_per_byte;
	int m_emulation_pixels_per_real_pixel;
	int m_emulation_pixels_per_byte;

	int m_emulation_cursor_size;
	int m_cursor_state;

	int m_videoULA_palette0[16];
	int m_videoULA_palette1[16];
	int *m_videoULA_palette_lookup;

	void bbc_setvideoshadow(int vdusel);
	void common_init(int memorySize);
	void set_pixel_lookup();
	int vdudriverset();
	int bbcm_vdudriverset();
	int bbc_keyboard(address_space &space, int data);
	void bbcb_IC32_initialise(bbc_state *state);
	void MC146818_set(address_space &space);
	void MC6850_Receive_Clock(int new_clock);
	void BBC_Cassette_motor(unsigned char status);
	void bbc_update_nmi();
	unsigned int calculate_video_address(int ma,int ra);
	required_device<palette_device> m_palette;
};

#endif /* BBC_H_ */
