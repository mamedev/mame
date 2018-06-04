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
#ifndef MAME_INCLUDES_BBC_H
#define MAME_INCLUDES_BBC_H

#pragma once

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
#include "machine/input_merger.h"
#include "video/mc6845.h"
#include "video/saa5050.h"
#include "sound/sn76496.h"
#include "sound/tms5220.h"
#include "sound/samples.h"
#include "imagedev/cassette.h"

#include "bus/bbc/fdc/fdc.h"
#include "bus/bbc/analogue/analogue.h"
#include "bus/bbc/1mhzbus/1mhzbus.h"
#include "bus/bbc/tube/tube.h"
#include "bus/bbc/userport/userport.h"
#include "bus/bbc/joyport/joyport.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "screen.h"

#define RS232_TAG       "rs232"

class bbc_state : public driver_device
{
public:
	bbc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG),
		m_hd6845(*this, "hd6845"),
		m_screen(*this, "screen"),
		m_adlc(*this, "mc6854"),
		m_sn(*this, "sn76489"),
		m_samples(*this, "samples"),
		m_keyboard(*this, "COL%u", 0),
		m_trom(*this, "saa5050"),
		m_tms(*this, "tms5220"),
		m_cassette(*this, "cassette"),
		m_acia(*this, "acia6850"),
		m_acia_clock(*this, "acia_clock"),
		m_rs232(*this, RS232_TAG),
		m_via6522_0(*this, "via6522_0"),
		m_via6522_1(*this, "via6522_1"),
		m_upd7002(*this, "upd7002"),
		m_analog(*this, "analogue"),
		m_joyport(*this, "joyport"),
		m_tube(*this, "tube"),
		m_intube(*this, "intube"),
		m_extube(*this, "extube"),
		m_1mhzbus(*this, "1mhzbus"),
		m_rtc(*this, "rtc"),
		m_fdc(*this, "fdc"),
		m_i8271(*this, "i8271"),
		m_wd1770(*this, "wd1770"),
		m_wd1772(*this, "wd1772"),
		m_exp1(*this, "exp_rom1"),
		m_exp2(*this, "exp_rom2"),
		m_exp3(*this, "exp_rom3"),
		m_exp4(*this, "exp_rom4"),
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
		m_irqs(*this, "irqs"),
		m_palette(*this, "palette"),
		m_bbcconfig(*this, "BBCCONFIG")
	{ }

	enum machine_type_t
	{
		MODELA,
		MODELB,
		BPLUS,
		MASTER,
		COMPACT
	};

	enum monitor_type_t
	{
		COLOUR = 0,
		BLACKWHITE = 1,
		GREEN = 2,
		AMBER = 3
	};

	DECLARE_FLOPPY_FORMATS(floppy_formats_bbc);

	DECLARE_WRITE8_MEMBER(page_selecta_w);
	DECLARE_WRITE8_MEMBER(page_selectb_w);
	DECLARE_WRITE8_MEMBER(bbc_memoryb4_w);
	DECLARE_READ8_MEMBER(bbcbp_fetch_r);
	DECLARE_WRITE8_MEMBER(page_selectbp_w);
	DECLARE_WRITE8_MEMBER(page_selectbp128_w);
	DECLARE_WRITE8_MEMBER(bbc_memorybp4_w);
	DECLARE_WRITE8_MEMBER(bbc_memorybp6_w);
	DECLARE_READ8_MEMBER(bbcm_fetch_r);
	DECLARE_READ8_MEMBER(bbcm_acccon_r);
	DECLARE_WRITE8_MEMBER(bbcm_acccon_w);
	DECLARE_WRITE8_MEMBER(page_selectbm_w);
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

	void init_bbc();
	DECLARE_VIDEO_START(bbc);

	DECLARE_MACHINE_START(bbca);
	DECLARE_MACHINE_RESET(bbca);
	DECLARE_MACHINE_START(bbcb);
	DECLARE_MACHINE_RESET(bbcb);
	DECLARE_MACHINE_RESET(torch);
	DECLARE_MACHINE_START(bbcbp);
	DECLARE_MACHINE_RESET(bbcbp);
	DECLARE_MACHINE_START(bbcm);
	DECLARE_MACHINE_RESET(bbcm);
	DECLARE_MACHINE_START(bbcmc);
	DECLARE_MACHINE_RESET(bbcmc);
	DECLARE_MACHINE_RESET(ltmpbp);
	DECLARE_MACHINE_RESET(ltmpm);
	DECLARE_MACHINE_START(cfa3000);

	DECLARE_PALETTE_INIT(bbc);
	INTERRUPT_GEN_MEMBER(bbcb_keyscan);
	TIMER_CALLBACK_MEMBER(bbc_tape_timer_cb);
	DECLARE_WRITE_LINE_MEMBER(write_acia_clock);
	DECLARE_WRITE_LINE_MEMBER(adlc_irq_w);
	DECLARE_WRITE_LINE_MEMBER(econet_clk_w);
	DECLARE_WRITE_LINE_MEMBER(bus_nmi_w);
	DECLARE_WRITE8_MEMBER(bbcb_via_system_write_porta);
	DECLARE_WRITE8_MEMBER(bbcb_via_system_write_portb);
	DECLARE_READ8_MEMBER(bbcb_via_system_read_porta);
	DECLARE_READ8_MEMBER(bbcb_via_system_read_portb);
	DECLARE_WRITE_LINE_MEMBER(bbc_hsync_changed);
	DECLARE_WRITE_LINE_MEMBER(bbc_vsync_changed);
	DECLARE_WRITE_LINE_MEMBER(bbc_de_changed);
	DECLARE_INPUT_CHANGED_MEMBER(monitor_changed);
	void update_acia_rxd();
	void update_acia_dcd();
	void update_acia_cts();
	DECLARE_WRITE_LINE_MEMBER(bbc_rts_w);
	DECLARE_WRITE_LINE_MEMBER(bbc_txd_w);
	DECLARE_WRITE_LINE_MEMBER(write_rxd_serial);
	DECLARE_WRITE_LINE_MEMBER(write_dcd_serial);
	DECLARE_WRITE_LINE_MEMBER(write_cts_serial);
	DECLARE_INPUT_CHANGED_MEMBER(trigger_reset);
	DECLARE_WRITE_LINE_MEMBER(fdc_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	DECLARE_WRITE_LINE_MEMBER(motor_w);
	DECLARE_WRITE_LINE_MEMBER(side_w);

	UPD7002_GET_ANALOGUE(BBC_get_analogue_input);
	UPD7002_EOC(BBC_uPD7002_EOC);

	void bbc_setup_banks(memory_bank *membank, int banks, uint32_t shift, uint32_t size);
	void bbcm_setup_banks(memory_bank *membank, int banks, uint32_t shift, uint32_t size);

	image_init_result bbc_load_rom(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(exp1_load) { return bbc_load_rom(image, m_exp1); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(exp2_load) { return bbc_load_rom(image, m_exp2); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(exp3_load) { return bbc_load_rom(image, m_exp3); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(exp4_load) { return bbc_load_rom(image, m_exp4); }

	image_init_result bbcm_load_cart(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(bbcm_exp1_load) { return bbcm_load_cart(image, m_exp1); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(bbcm_exp2_load) { return bbcm_load_cart(image, m_exp2); }

	MC6845_UPDATE_ROW(crtc_update_row);

	void bbc_eprom_sockets(machine_config &config);
	void discmon(machine_config &config);
	void discmate(machine_config &config);
	void reutapm(machine_config &config);
	void bbcbp(machine_config &config);
	void abc310(machine_config &config);
	void bbcmc(machine_config &config);
	void bbcmt(machine_config &config);
	void bbcm(machine_config &config);
	void ltmpm(machine_config &config);
	void bbcmet(machine_config &config);
	void cfa3000(machine_config &config);
	void bbcbp128(machine_config &config);
	void pro128s(machine_config &config);
	void bbcm512(machine_config &config);
	void bbcmarm(machine_config &config);
	void abc110(machine_config &config);
	void ltmpbp(machine_config &config);
	void bbcb_de(machine_config &config);
	void bbcb(machine_config &config);
	void bbcmaiv(machine_config &config);
	void bbca(machine_config &config);
	void bbcb_us(machine_config &config);
	void econx25(machine_config &config);
	void acw443(machine_config &config);
	void bbc_base(address_map &map);
	void bbca_mem(address_map &map);
	void bbcb_mem(address_map &map);
	void bbcb_nofdc_mem(address_map &map);
	void bbcbp128_mem(address_map &map);
	void bbcbp_mem(address_map &map);
	void bbcbp_fetch(address_map &map);
	void bbcm_mem(address_map &map);
	void bbcm_fetch(address_map &map);
	void reutapm_mem(address_map &map);

private:
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<hd6845_device> m_hd6845;
	required_device<screen_device> m_screen;
	optional_device<mc6854_device> m_adlc;
	optional_device<sn76489_device> m_sn;
	optional_device<samples_device> m_samples;
	required_ioport_array<13> m_keyboard;
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
	optional_device<bbc_analogue_slot_device> m_analog;
	optional_device<bbc_joyport_slot_device> m_joyport;
	optional_device<bbc_tube_slot_device> m_tube;
	optional_device<bbc_tube_slot_device> m_intube;
	optional_device<bbc_tube_slot_device> m_extube;
	optional_device<bbc_1mhzbus_slot_device> m_1mhzbus;
	optional_device<mc146818_device> m_rtc;
	optional_device<bbc_fdc_slot_device> m_fdc;
	optional_device<i8271_device> m_i8271;
	optional_device<wd1770_device> m_wd1770;
	optional_device<wd1772_device> m_wd1772;
	optional_device<generic_slot_device> m_exp1;
	optional_device<generic_slot_device> m_exp2;
	optional_device<generic_slot_device> m_exp3;
	optional_device<generic_slot_device> m_exp4;

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

	required_device<input_merger_device> m_irqs;

	machine_type_t m_machinetype;

	bool m_os01;            // flag indicating whether OS 0.1 is being used
	int m_monitortype;      // monitor type (colour, green, amber)
	int m_swramtype;        // this stores the setting for the SWRAM type being used

	int m_swrbank;          // This is the latch that holds the sideways ROM bank to read
	bool m_swrbank_ram;     // Does ROM bank contain RAM

	int m_paged_ram;        // BBC B+ memory handling
	int m_vdusel;           // BBC B+ memory handling

	bool m_lk18_ic41_paged_rom;  // BBC Master Paged ROM/RAM select IC41
	bool m_lk19_ic37_paged_rom;  // BBC Master Paged ROM/RAM select IC37

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

	int m_acccon;
	int m_acccon_irr;
	int m_acccon_tst;
	int m_acccon_ifj;
	int m_acccon_itu;
	int m_acccon_y;
	int m_acccon_x;
	int m_acccon_e;
	int m_acccon_d;


	/*
	    The addressable latch
	    This 8 bit addressable latch is operated from port B lines 0-3.
	    PB0-PB2 are set to the required address of the output bit to be set.
	    PB3 is set to the value which should be programmed at that bit.
	    The function of the 8 output bits from this latch are:-

	    B0 - Write Enable to the sound generator IC
	    B1 - READ select on the speech processor (B and B+)
	         R/nW control on CMOS RAM (Master only)
	    B2 - WRITE select on the speech processor
	         DS control on CMOS RAM (Master only)
	    B3 - Keyboard write enable
	    B4,B5 - these two outputs define the number to be added to the
	    start of screen address in hardware to control hardware scrolling:-
	    Mode    Size    Start of screen  Size  No.to add  B5      B4
	    0,1,2   20K     &3000            12K              1       1
	    3       16K     &4000            16K              0       0
	    4,5     10K     &5800 (or &1800) 22K              1       0
	    6       8K      &6000 (or &2000) 24K              0       1
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

	// interrupt state
	int m_adlc_irq;
	int m_bus_nmi;

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
	uint8_t m_serproc_data;
	int m_rxd_serial;
	int m_dcd_serial;
	int m_cts_serial;
	int m_dcd_cass;
	int m_rxd_cass;
	int m_cass_out_enabled;
	int m_txd;
	uint32_t m_nr_high_tones;
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

// this is the real location of the start of the BBC's ram in the emulation
// it can be changed if shadow ram is being used to point at the upper 32K of RAM
	uint8_t *m_video_ram;
	uint8_t m_pixel_bits[256];
	int m_hsync;
	int m_vsync;

	uint8_t m_teletext_latch;

	struct {
		// control register
		int master_cursor_size;
		int width_of_cursor;
		int clock_rate_6845;
		int characters_per_line;
		int teletext_normal_select;
		int flash_colour_select;
		// inputs
		int de;
	} m_video_ula;

	int m_pixels_per_byte;
	int m_cursor_size;

	int m_videoULA_palette0[16];
	int m_videoULA_palette1[16];
	int *m_videoULA_palette_lookup;

	rgb_t out_rgb(rgb_t entry);

	void setvideoshadow(int vdusel);
	void set_pixel_lookup();
	int bbc_keyboard(address_space &space, int data);
	void bbcb_IC32_initialise(bbc_state *state);
	void MC146818_set(address_space &space);
	void MC6850_Receive_Clock(int new_clock);
	void cassette_motor(bool state);
	void bbc_update_nmi();
	uint16_t calculate_video_address(uint16_t ma, uint8_t ra);
	required_device<palette_device> m_palette;
	optional_ioport m_bbcconfig;
};


class torch_state : public bbc_state
{
public:
	using bbc_state::bbc_state;
	static constexpr feature_type imperfect_features() { return feature::KEYBOARD; }
	void torchf(machine_config &config);
	void torchh21(machine_config &config);
	void torchh10(machine_config &config);
};

#endif // MAME_INCLUDES_BBC_H
