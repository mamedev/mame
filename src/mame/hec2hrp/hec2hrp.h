// license:BSD-3-Clause
// copyright-holders:JJ Stacino
/////////////////////////////////////////////////////////////////////
//////   HECTOR HEADER FILE /////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

#ifndef MAME_HEC2HRP_HEC2HRP_H
#define MAME_HEC2HRP_HEC2HRP_H

#pragma once

#include "imagedev/floppy.h"
#include "imagedev/cassette.h"
#include "imagedev/printer.h"
#include "machine/upd765.h"
#include "machine/wd_fdc.h"
#include "machine/ram.h"
#include "sound/discrete.h"  /* for 1 Bit sound*/
#include "sound/sn76477.h"   /* for sn sound*/
#include "emupal.h"

/* Enum status for high memory bank (c000 - ffff)*/
enum
{
	HECTOR_BANK_PROG = 0,               /* first BANK is program ram*/
	HECTOR_BANK_VIDEO                   /* second BANK is Video ram */
};
/* Status for rom memory bank (0000 - 3fff) in MX machine*/
enum
{
	HECTORMX_BANK_PAGE0 = 0,            /* first BANK is base rom*/
	HECTORMX_BANK_PAGE1,                /* second BANK is basic rom */
	HECTORMX_BANK_PAGE2                 /* 3 BANK is monitrix / assemblex rom */
};
/* Status for rom memory bank (0000 - 3fff) in Mini Disc machine*/
enum
{
	HECTOR_BANK_BASE = 0,               /* first BANK is normal rom*/
	HECTOR_BANK_DISC                    /* second BANK is extra rom for mini disc use*/
};
/* Enum status for low memory bank (00000 - 0fff) for DISC II*/
enum
{
	DISCII_BANK_RAM = 0,            /* first BANK is program ram*/
	DISCII_BANK_ROM                 /* second BANK is ROM */
};

class hec2hrp_state : public driver_device
{
public:
	hec2hrp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cassette(*this, "cassette")
		, m_printer(*this, "printer")
		, m_palette(*this, "palette")
		, m_vram(*this,"videoram")
		, m_bank(*this, "bank%u", 0U)
		, m_rom(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_hector_vram(*this,"hector_videoram", 0x4000, ENDIANNESS_LITTLE)
		, m_disc2cpu(*this, "disc2cpu")
		, m_discrete(*this, "discrete")
		, m_sn(*this, "sn76477")
		, m_keyboard(*this, "KEY.%u", 0)
		, m_minidisc_fdc(*this, "wd179x")
		, m_floppy0(*this, "wd179x:0")
		, m_upd_fdc(*this, "upd765")
		, m_upd_connector(*this, "upd765:%u", 0U)
	{}

	void hec2mx80(machine_config &config);
	void hec2hrp(machine_config &config);
	void hec2hrx(machine_config &config);
	void hec2mx40(machine_config &config);
	void hec2mdhrx(machine_config &config);
	void hec2hr(machine_config &config);
	void hector_audio(machine_config &config);

	void init_mx40();
	void init_mdhrx();
	void init_victor();
	void init_hrx();
	void init_interact();
	void hector1(machine_config &config);
	void interact(machine_config &config);
	void interact_common(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void hector_hr(bitmap_ind16 &bitmap, uint8_t *page, int ymax, int yram);
	void hector_reset(bool hr, bool with_d2);
	void keyboard_w(uint8_t data);
	uint8_t keyboard_r(offs_t offset);
	void sn_2000_w(offs_t offset, uint8_t data);
	void sn_2800_w(offs_t offset, uint8_t data);
	uint8_t cassette_r();
	void sn_3000_w(uint8_t data);
	void color_a_w(uint8_t data);
	void color_b_w(uint8_t data);
	bool m_has_disc2;
	bool m_has_minidisc;
	bool m_is_hr;
	bool m_is_extended;
	void init_palette(palette_device &);
	void hector_init();
	void minidisc_control_w(uint8_t data);
	void switch_bank_w(offs_t offset, uint8_t data);
	uint8_t io_8255_r(offs_t offset);
	void io_8255_w(offs_t offset, uint8_t data);
	void mx40_io_port_w(offs_t offset, uint8_t data);
	void mx80_io_port_w(offs_t offset, uint8_t data);

	// disc2 handling
	uint8_t disc2_io00_port_r();
	void disc2_io00_port_w(uint8_t data);
	uint8_t disc2_io20_port_r();
	void disc2_io20_port_w(uint8_t data);
	uint8_t disc2_io30_port_r();
	void disc2_io30_port_w(uint8_t data);
	uint8_t disc2_io40_port_r();
	void disc2_io40_port_w(uint8_t data);
	uint8_t disc2_io50_port_r();
	void disc2_io50_port_w(uint8_t data);

	static void minidisc_formats(format_registration &fr);

	bool m_hector_flag_hr = 0;
	bool m_hector_flag_80c = 0;
	uint8_t m_hector_color[4]{};
	uint8_t m_hector_disc2_data_r_ready = 0;
	uint8_t m_hector_disc2_data_w_ready = 0;
	uint8_t m_hector_disc2_data_read = 0;
	uint8_t m_hector_disc2_data_write = 0;
	bool m_hector_disc2_rnmi = false;
	uint8_t m_state3000 = 0;
	bool m_write_cassette = false;
	emu_timer *m_cassette_timer = nullptr;
	uint8_t m_ck_signal = 0;
	bool m_flag_clk = false;
	double m_pin_value[29][2]{};
	u8 m_au[17]{};
	u8 m_val_mixer = 0;
	u8 m_oldstate3000 = 0;
	u8 m_oldstate1000 = 0;
	uint8_t m_pot0 = 0;
	uint8_t m_pot1 = 0;
	uint8_t m_actions = 0;
	uint8_t m_hector_port_a = 0;
	uint8_t m_hector_port_b = 0;
	uint8_t m_hector_port_c_h = 0;
	uint8_t m_hector_port_c_l = 0;
	uint8_t m_hector_port_cmd = 0;
	bool m_cassette_bit = false;
	bool m_cassette_bit_mem = false;
	uint8_t m_data_k7 = 0;
	int m_counter_write = 0;
	bool m_irq_current_state = false;
	bool m_nmi_current_state = false;

	DECLARE_MACHINE_RESET(interact);
	DECLARE_MACHINE_START(hec2hrp);
	DECLARE_MACHINE_RESET(hec2hrp);
	DECLARE_MACHINE_START(hec2hrx);
	DECLARE_MACHINE_RESET(hec2hrx);
	DECLARE_MACHINE_START(hec2mdhrx);
	DECLARE_MACHINE_RESET(hec2mdhrx);
	uint32_t screen_update_hec2hrp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(cassette_clock);

	void disc2_fdc_interrupt(int state);
	void disc2_fdc_dma_irq(int state);

	void update_state(int Adresse, int Value );
	void init_sn76477();
	void update_sound(uint8_t data);
	void hector_80c(bitmap_ind16 &bitmap, uint8_t *page, int ymax, int yram);
	void hector_disc2_reset();
	uint32_t screen_update_interact(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void interact_mem(address_map &map) ATTR_COLD;

	void hec2hrp_io(address_map &map) ATTR_COLD;
	void hec2hrp_mem(address_map &map) ATTR_COLD;
	void hec2hrx_io(address_map &map) ATTR_COLD;
	void hec2hrx_mem(address_map &map) ATTR_COLD;
	void hec2mdhrx_io(address_map &map) ATTR_COLD;
	void hec2mx40_io(address_map &map) ATTR_COLD;
	void hec2mx80_io(address_map &map) ATTR_COLD;
	void hecdisc2_io(address_map &map) ATTR_COLD;
	void hecdisc2_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	optional_device<printer_image_device> m_printer;
	required_device<palette_device> m_palette;
	optional_shared_ptr<uint8_t> m_vram;
	optional_memory_bank_array<4> m_bank;
	required_region_ptr<u8> m_rom;
	optional_device<ram_device> m_ram;
	memory_share_creator<uint8_t> m_hector_vram;
	optional_device<cpu_device> m_disc2cpu;
	required_device<discrete_device> m_discrete;
	required_device<sn76477_device> m_sn;
	required_ioport_array<9> m_keyboard;
	optional_device<fd1793_device> m_minidisc_fdc;
	optional_device<floppy_connector> m_floppy0;
	optional_device<upd765a_device> m_upd_fdc;
	optional_device_array<floppy_connector, 2> m_upd_connector;
};

#endif // MAME_HEC2HRP_HEC2HRP_H
