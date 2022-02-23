// license:BSD-3-Clause
// copyright-holders:Gordon Jefferyes, Nigel Barnes
/*****************************************************************************
 *
 * BBC Model B/B+/Master/Compact
 *
 * Driver by Gordon Jefferyes <mess_bbc@romvault.com>
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_BBC_H
#define MAME_INCLUDES_BBC_H

#pragma once

#include "cpu/m6502/m6502.h"
#include "cpu/m6502/m65sc02.h"
#include "imagedev/floppy.h"
#include "machine/74259.h"
#include "machine/6522via.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/mc6854.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "machine/upd7002.h"
#include "machine/mc146818.h"
#include "machine/i2cmem.h"
#include "machine/bankdev.h"
#include "machine/input_merger.h"
#include "video/mc6845.h"
#include "video/saa5050.h"
#include "sound/sn76496.h"
#include "sound/tms5220.h"
#include "sound/samples.h"
#include "imagedev/cassette.h"

#include "bus/rs232/rs232.h"
#include "bus/centronics/ctronics.h"
#include "bus/econet/econet.h"
#include "bus/bbc/rom/slot.h"
#include "bus/bbc/fdc/fdc.h"
#include "bus/bbc/analogue/analogue.h"
#include "bus/bbc/1mhzbus/1mhzbus.h"
#include "bus/bbc/modem/modem.h"
#include "bus/bbc/internal/internal.h"
#include "bus/bbc/tube/tube.h"
#include "bus/bbc/userport/userport.h"
#include "bus/bbc/exp/exp.h"
#include "bus/bbc/joyport/joyport.h"
#include "bus/bbc/cart/slot.h"

#include "emupal.h"
#include "screen.h"


class bbc_state : public driver_device
{
public:
	bbc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_hd6845(*this, "hd6845")
		, m_screen(*this, "screen")
		, m_irqs(*this, "irqs")
		, m_palette(*this, "palette")
		, m_adlc(*this, "mc6854")
		, m_sn(*this, "sn76489")
		, m_samples(*this, "samples")
		, m_keyboard(*this, "COL%u", 0)
		, m_trom(*this, "saa5050")
		, m_tms(*this, "tms5220")
		, m_cassette(*this, "cassette")
		, m_acia(*this, "acia6850")
		, m_acia_clock(*this, "acia_clock")
		, m_latch(*this, "latch")
		, m_rs232(*this, "rs423")
		, m_via6522_0(*this, "via6522_0")
		, m_via6522_1(*this, "via6522_1")
		, m_upd7002(*this, "upd7002")
		, m_analog(*this, "analogue")
		, m_joyport(*this, "joyport")
		, m_tube(*this, "tube")
		, m_intube(*this, "intube")
		, m_extube(*this, "extube")
		, m_1mhzbus(*this, "1mhzbus")
		, m_modem(*this, "modem")
		, m_userport(*this, "userport")
		, m_internal(*this, "internal")
		, m_exp(*this, "exp")
		, m_rtc(*this, "rtc")
		, m_i2cmem(*this, "i2cmem")
		, m_fdc(*this, "fdc")
		, m_wd_fdc(*this, "wd_fdc")
		, m_rom(*this, "romslot%u", 0U)
		, m_cart(*this, "cartslot%u", 1U)
		, m_region_mos(*this, "mos")
		, m_region_swr(*this, "swr")
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
		, m_bankdev(*this, "bankdev")
		, m_bbcconfig(*this, "BBCCONFIG")
		, m_motor_led(*this, "motor_led")
	{ }

	enum class monitor_type
	{
		COLOUR,
		BLACKWHITE,
		GREEN,
		AMBER
	};

	static void floppy_formats(format_registration &fr);

	uint8_t bbc_ram_r(offs_t offset);
	void bbc_ram_w(offs_t offset, uint8_t data);
	uint8_t bbc_romsel_r(offs_t offset);
	void bbc_romsel_w(offs_t offset, uint8_t data);
	uint8_t bbc_paged_r(offs_t offset);
	void bbc_paged_w(offs_t offset, uint8_t data);
	uint8_t bbc_mos_r(offs_t offset);
	void bbc_mos_w(offs_t offset, uint8_t data);
	uint8_t bbc_fred_r(offs_t offset);
	void bbc_fred_w(offs_t offset, uint8_t data);
	uint8_t bbc_jim_r(offs_t offset);
	void bbc_jim_w(offs_t offset, uint8_t data);
	uint8_t bbcbp_fetch_r(offs_t offset);
	void bbcbp_romsel_w(offs_t offset, uint8_t data);
	uint8_t bbcbp_paged_r(offs_t offset);
	void bbcbp_paged_w(offs_t offset, uint8_t data);
	uint8_t bbcm_fetch_r(offs_t offset);
	uint8_t bbcm_acccon_r();
	void bbcm_acccon_w(uint8_t data);
	void bbcm_romsel_w(offs_t offset, uint8_t data);
	uint8_t bbcm_paged_r(offs_t offset);
	void bbcm_paged_w(offs_t offset, uint8_t data);
	uint8_t bbcm_hazel_r(offs_t offset);
	void bbcm_hazel_w(offs_t offset, uint8_t data);
	uint8_t bbcm_tube_r(offs_t offset);
	void bbcm_tube_w(offs_t offset, uint8_t data);
	uint8_t bbcmc_paged_r(offs_t offset);
	void bbcmc_paged_w(offs_t offset, uint8_t data);
	void bbcbp_drive_control_w(uint8_t data);
	void bbcm_drive_control_w(uint8_t data);
	void serial_ula_w(uint8_t data);
	void video_ula_w(offs_t offset, uint8_t data);
	uint8_t bbc_fe_r() { return 0xfe; }

	DECLARE_VIDEO_START(bbc);

	INTERRUPT_GEN_MEMBER(bbcb_keyscan);
	TIMER_CALLBACK_MEMBER(tape_timer_cb);
	TIMER_CALLBACK_MEMBER(reset_timer_cb);
	DECLARE_WRITE_LINE_MEMBER(write_acia_clock);
	DECLARE_WRITE_LINE_MEMBER(adlc_irq_w);
	DECLARE_WRITE_LINE_MEMBER(bus_nmi_w);
	DECLARE_WRITE_LINE_MEMBER(snd_enable_w);
	DECLARE_WRITE_LINE_MEMBER(speech_rsq_w);
	DECLARE_WRITE_LINE_MEMBER(speech_wsq_w);
	DECLARE_WRITE_LINE_MEMBER(kbd_enable_w);
	uint8_t via_system_porta_r();
	void via_system_porta_w(uint8_t data);
	uint8_t via_system_portb_r();
	void via_system_portb_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(lpstb_w);
	DECLARE_WRITE_LINE_MEMBER(bbc_hsync_changed);
	DECLARE_WRITE_LINE_MEMBER(bbc_vsync_changed);
	DECLARE_WRITE_LINE_MEMBER(bbc_de_changed);
	DECLARE_INPUT_CHANGED_MEMBER(reset_palette);
	void update_palette(monitor_type monitor_type);

	void update_acia_rxd();
	void update_acia_dcd();
	void update_acia_cts();
	DECLARE_WRITE_LINE_MEMBER(write_rts);
	DECLARE_WRITE_LINE_MEMBER(write_txd);
	DECLARE_WRITE_LINE_MEMBER(write_rxd);
	DECLARE_WRITE_LINE_MEMBER(write_dcd);
	DECLARE_WRITE_LINE_MEMBER(write_cts);

	DECLARE_INPUT_CHANGED_MEMBER(trigger_reset);
	DECLARE_WRITE_LINE_MEMBER(fdc_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);

	int get_analogue_input(int channel_number);
	void upd7002_eoc(int data);

	std::string get_rom_name(uint8_t* header);
	void insert_device_rom(memory_region *rom);
	void setup_device_roms();

	MC6845_UPDATE_ROW(crtc_update_row);

	void bbca(machine_config &config);
	void bbcb(machine_config &config);
	void bbcb_de(machine_config &config);
	void bbcb_us(machine_config &config);
	void sist1(machine_config &config);

	void bbca_mem(address_map &map);
	void bbc_base(address_map &map);
	void bbcb_mem(address_map &map);

	void init_bbc();
	void init_ltmp();
	void init_cfa();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;
	virtual void video_reset() override;

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<mc6845_device> m_hd6845;
	required_device<screen_device> m_screen;
	required_device<input_merger_device> m_irqs;
	required_device<palette_device> m_palette;
	optional_device<mc6854_device> m_adlc;
	optional_device<sn76489a_device> m_sn;
	optional_device<samples_device> m_samples;
	required_ioport_array<16> m_keyboard;
	optional_device<saa5050_device> m_trom;
	optional_device<tms5220_device> m_tms;
	optional_device<cassette_image_device> m_cassette;
	optional_device<acia6850_device> m_acia;
	optional_device<clock_device> m_acia_clock;
	required_device<ls259_device> m_latch;
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
	optional_device<bbc_modem_slot_device> m_modem;
	optional_device<bbc_userport_slot_device> m_userport;
	optional_device<bbc_internal_slot_device> m_internal;
	optional_device<bbc_exp_slot_device> m_exp;
	optional_device<mc146818_device> m_rtc;
	optional_device<i2cmem_device> m_i2cmem;
	optional_device<bbc_fdc_slot_device> m_fdc;
	optional_device<wd_fdc_digital_device_base> m_wd_fdc;
	optional_device_array<bbc_romslot_device, 16> m_rom;
	optional_device_array<bbc_cartslot_device, 2> m_cart;

	required_memory_region m_region_mos;
	required_memory_region m_region_swr;
	optional_memory_bank m_bank1; //           bbcbp bbcbp128 bbcm
	optional_memory_bank m_bank2; //           bbcbp bbcbp128 bbcm
	optional_device<address_map_bank_device> m_bankdev; //    bbcm
	optional_ioport m_bbcconfig;

	output_finder<> m_motor_led;

	int m_romsel;           // This is the latch that holds the sideways ROM bank to read
	int m_paged_ram;        // BBC B+ memory handling
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

	int m_acccon;
	int m_acccon_irr;
	int m_acccon_tst;
	int m_acccon_ifj;
	int m_acccon_itu;
	int m_acccon_y;
	int m_acccon_x;
	int m_acccon_e;
	int m_acccon_d;

	void mc146818_set();
	int m_mc146818_as;      // 6522 port b bit 7
	int m_mc146818_ce;      // 6522 port b bit 6

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
	uint8_t m_vula_ctrl;

	struct video_nula {
		uint8_t palette_mode;
		uint8_t horiz_offset;
		uint8_t left_blank;
		uint8_t disable;
		uint8_t attr_mode;
		uint8_t attr_text;
		uint8_t flash[8];
		uint8_t palette_byte;
		uint8_t palette_write;
	} m_vnula;

	int m_pixels_per_byte;
	int m_cursor_size;

	uint8_t m_vula_palette[16];
	uint8_t m_vula_palette_lookup[16];

	void setvideoshadow(int vdusel);
	void set_pixel_lookup();
	uint8_t bus_video_data();
	int bbc_keyboard(int data);

	void mc6850_receive_clock(int new_clock);
	void cassette_motor(bool state);
	void update_nmi();
	uint16_t calculate_video_address(uint16_t ma, uint8_t ra);

private:
	emu_timer *m_reset_timer;
};


class torch_state : public bbc_state
{
public:
	using bbc_state::bbc_state;
	static constexpr feature_type imperfect_features() { return feature::KEYBOARD; }

	void torchf(machine_config &config);
	void torchh(machine_config &config);
	void torch301(machine_config &config);
	void torch725(machine_config &config);
};


class bbcbp_state : public bbc_state
{
public:
	using bbc_state::bbc_state;

	void bbcbp(machine_config &config);
	void bbcbp128(machine_config &config);
	void abc110(machine_config &config);
	void acw443(machine_config &config);
	void abc310(machine_config &config);
	void cfa3000bp(machine_config &config);
	void econx25(machine_config &config);
	void reutapm(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void bbcbp_mem(address_map &map);
	void reutapm_mem(address_map &map);
	void bbcbp_fetch(address_map &map);
};


class bbcm_state : public bbc_state
{
public:
	bbcm_state(const machine_config &mconfig, device_type type, const char *tag)
		: bbc_state(mconfig, type, tag)
		, m_power_led(*this, "power_led")
	{ }

	void bbcm(machine_config &config);
	void bbcmt(machine_config &config);
	void bbcmet(machine_config &config);
	void bbcmaiv(machine_config &config);
	void bbcm512(machine_config &config);
	void bbcmarm(machine_config &config);
	void cfa3000(machine_config &config);
	void daisy(machine_config &config);
	void discmon(machine_config &config);
	void discmate(machine_config &config);
	void mpc800(machine_config &config);
	void mpc900(machine_config &config);
	void mpc900gx(machine_config &config);
	void bbcmc(machine_config &config);
	void pro128s(machine_config &config);
	void autoc15(machine_config &config);

	static void mpc_prisma_default(device_t *device);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void bbcm_mem(address_map &map);
	void bbcm_bankdev(address_map &map);
	void bbcmet_bankdev(address_map &map);
	void bbcmc_mem(address_map &map);
	void bbcmc_bankdev(address_map &map);
	void autoc15_bankdev(address_map &map);
	void bbcm_fetch(address_map &map);

	output_finder<> m_power_led;
};


#endif // MAME_INCLUDES_BBC_H
