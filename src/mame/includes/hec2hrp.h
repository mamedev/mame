// license:BSD-3-Clause
// copyright-holders:JJ Stacino
/////////////////////////////////////////////////////////////////////
//////   HECTOR HEADER FILE /////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/*
        Hector 2HR+
        Victor
        Hector 2HR
        Hector HRX
        Hector MX40c
        Hector MX80c
        Hector 1
        Interact

        12/05/2009 Skeleton driver - Micko : mmicko@gmail.com
        31/06/2009 Video - Robbbert

        29/10/2009 Update skeleton to functional machine
                    by yo_fr            (jj.stac @ aliceadsl.fr)

                => add Keyboard,
                => add color,
                => add cassette,
                => add sn76477 sound and 1bit sound,
                => add joysticks (stick, pot, fire)
                => add BR/HR switching
                => add bank switch for HRX
                => add device MX80c and bank switching for the ROM
        03/01/2010 Update and clean prog  by yo_fr       (jj.stac@aliceadsl.fr)
                => add the port mapping for keyboard
        20/11/2010 : synchronization between uPD765 and Z80 are now OK, CP/M running! JJStacino
        11/11/2011 : add the minidisque support -3 pouces 1/2 driver-  JJStacino  (jj.stac @ aliceadsl.fr)

            don't forget to keep some information about these machine see DChector project : http://dchector.free.fr/ made by DanielCoulom
            (and thank's to Daniel!) and Yves site : http://hectorvictor.free.fr/ (thank's too Yves!)

    TODO :  Add the cartridge function,
            Adjust the one shot and A/D timing (sn76477)
*/
#ifndef MAME_INCLUDES_HEC2HRP_H
#define MAME_INCLUDES_HEC2HRP_H

#pragma once

#include "imagedev/floppy.h"
#include "imagedev/cassette.h"
#include "imagedev/printer.h"
#include "machine/upd765.h"
#include "machine/wd_fdc.h"
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
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cassette(*this, "cassette"),
		m_printer(*this, "printer"),
		m_palette(*this, "palette"),
		m_disc2cpu(*this, "disc2cpu"),
		m_discrete(*this, "discrete"),
		m_sn(*this, "sn76477"),
		m_videoram(*this,"videoram"),
		m_hector_videoram(*this,"hector_videoram") ,
		m_keyboard(*this, "KEY.%u", 0),
		m_minidisc_fdc(*this, "wd179x"),
		m_floppy0(*this, "wd179x:0"),
		m_upd_fdc(*this, "upd765"),
		m_upd_connector(*this, "upd765:%u", 0U)
	{}

	void hec2mx80(machine_config &config);
	void hec2hrp(machine_config &config);
	void hec2hrx(machine_config &config);
	void hec2mx40(machine_config &config);
	void hec2mdhrx(machine_config &config);
	void hec2hr(machine_config &config);
	void hector_audio(machine_config &config);

	void hector_init();

protected:
	DECLARE_VIDEO_START(hec2hrp);
	void hector_hr(bitmap_ind16 &bitmap, uint8_t *page, int ymax, int yram);
	void hector_reset(int hr, int with_d2);

	DECLARE_WRITE8_MEMBER(keyboard_w);
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_WRITE8_MEMBER(sn_2000_w);
	DECLARE_WRITE8_MEMBER(sn_2800_w);
	DECLARE_READ8_MEMBER(cassette_r);
	DECLARE_WRITE8_MEMBER(sn_3000_w);
	DECLARE_WRITE8_MEMBER(color_a_w);
	DECLARE_WRITE8_MEMBER(color_b_w);

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	optional_device<printer_image_device> m_printer;
	required_device<palette_device> m_palette;

private:
	DECLARE_WRITE8_MEMBER(minidisc_control_w);

	DECLARE_WRITE8_MEMBER(switch_bank_w);
	DECLARE_READ8_MEMBER(io_8255_r);
	DECLARE_WRITE8_MEMBER(io_8255_w);
	DECLARE_WRITE8_MEMBER(mx40_io_port_w);
	DECLARE_WRITE8_MEMBER(mx80_io_port_w);

	// disc2 handling
	DECLARE_READ8_MEMBER(  disc2_io00_port_r);
	DECLARE_WRITE8_MEMBER( disc2_io00_port_w);
	DECLARE_READ8_MEMBER(  disc2_io20_port_r);
	DECLARE_WRITE8_MEMBER( disc2_io20_port_w);
	DECLARE_READ8_MEMBER(  disc2_io30_port_r);
	DECLARE_WRITE8_MEMBER( disc2_io30_port_w);
	DECLARE_READ8_MEMBER(  disc2_io40_port_r);
	DECLARE_WRITE8_MEMBER( disc2_io40_port_w);
	DECLARE_READ8_MEMBER(  disc2_io50_port_r);
	DECLARE_WRITE8_MEMBER( disc2_io50_port_w);

	DECLARE_FLOPPY_FORMATS(minidisc_formats);

	optional_device<cpu_device> m_disc2cpu;
	required_device<discrete_device> m_discrete;
	required_device<sn76477_device> m_sn;
	optional_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_hector_videoram;
	required_ioport_array<9> m_keyboard;

	optional_device<fd1793_device> m_minidisc_fdc;
	optional_device<floppy_connector> m_floppy0;

	optional_device<upd765a_device> m_upd_fdc;
	optional_device_array<floppy_connector, 2> m_upd_connector;

	uint8_t m_hector_flag_hr;
	uint8_t m_hector_flag_80c;
	uint8_t m_hector_color[4];
	uint8_t m_hector_disc2_data_r_ready;
	uint8_t m_hector_disc2_data_w_ready;
	uint8_t m_hector_disc2_data_read;
	uint8_t m_hector_disc2_data_write;
	uint8_t m_hector_disc2_rnmi;
	uint8_t m_state3000;
	uint8_t m_write_cassette;
	emu_timer *m_cassette_timer;
	uint8_t m_ck_signal;
	uint8_t m_flag_clk;
	double m_pin_value[29][2];
	int m_au[17];
	int m_val_mixer;
	int m_oldstate3000;
	int m_oldstate1000;
	uint8_t m_pot0;
	uint8_t m_pot1;
	uint8_t m_actions;
	uint8_t m_hector_port_a;
	uint8_t m_hector_port_b;
	uint8_t m_hector_port_c_h;
	uint8_t m_hector_port_c_l;
	uint8_t m_hector_port_cmd;
	uint8_t m_cassette_bit;
	uint8_t m_cassette_bit_mem;
	uint8_t m_data_k7;
	int m_counter_write;
	int m_irq_current_state;
	int m_nmi_current_state;
	int m_hector_cmd[10];
	int m_hector_nb_cde;
	int m_hector_flag_result;
	int m_print;
	uint8_t m_hector_videoram_hrx[0x04000];

	DECLARE_MACHINE_START(hec2hrp);
	DECLARE_MACHINE_RESET(hec2hrp);
	DECLARE_MACHINE_START(hec2hrx);
	DECLARE_MACHINE_RESET(hec2hrx);
	DECLARE_MACHINE_START(hec2mdhrx);
	DECLARE_MACHINE_RESET(hec2mdhrx);
	uint32_t screen_update_hec2hrp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(cassette_clock);

	DECLARE_WRITE_LINE_MEMBER( disc2_fdc_interrupt );
	DECLARE_WRITE_LINE_MEMBER( disc2_fdc_dma_irq );
	int has_disc2();
	int has_minidisc();
	int is_hr();
	int is_extended();
	void update_state(int Adresse, int Value );
	void init_sn76477();
	void update_sound(address_space &space, uint8_t data);
	void init_palette();
	void hector_80c(bitmap_ind16 &bitmap, uint8_t *page, int ymax, int yram);
	/*----------- defined in machine/hecdisk2.c -----------*/

	void hector_disc2_reset();

	void hec2hrp_io(address_map &map);
	void hec2hrp_mem(address_map &map);
	void hec2hrx_io(address_map &map);
	void hec2hrx_mem(address_map &map);
	void hec2mdhrx_io(address_map &map);
	void hec2mx40_io(address_map &map);
	void hec2mx80_io(address_map &map);
	void hecdisc2_io(address_map &map);
	void hecdisc2_mem(address_map &map);
};

#endif // MAME_INCLUDES_HEC2HRP_H
