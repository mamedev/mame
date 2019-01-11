// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_INCLUDES_MEGADRIV_H
#define MAME_INCLUDES_MEGADRIV_H

#pragma once

#include "coreutil.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/2612intf.h"
#include "sound/sn76496.h"
#include "video/315_5313.h"

/* Megadrive Console Specific */
#include "bus/megadrive/md_slot.h"
#include "bus/megadrive/md_carts.h"
#include "machine/mega32x.h"
#include "machine/megacd.h"

#define MASTER_CLOCK_NTSC 53693175
#define MASTER_CLOCK_PAL  53203424

#define MD_CPU_REGION_SIZE 0x800000


/*----------- defined in machine/megadriv.c -----------*/

INPUT_PORTS_EXTERN( md_common );
INPUT_PORTS_EXTERN( megadriv );
INPUT_PORTS_EXTERN( megadri6 );
INPUT_PORTS_EXTERN( ssf2mdb );
INPUT_PORTS_EXTERN( mk3mdb );

struct genesis_z80_vars
{
	int z80_is_reset;
	int z80_has_bus;
	uint32_t z80_bank_addr;
	std::unique_ptr<uint8_t[]> z80_prgram;
};


class md_base_state : public driver_device
{
public:
	md_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_z80snd(*this,"genesis_snd_z80"),
		m_ymsnd(*this,"ymsnd"),
		m_scan_timer(*this, "md_scan_timer"),
		m_vdp(*this,"gen_vdp"),
		m_megadrive_ram(*this,"megadrive_ram"),
		m_io_reset(*this, "RESET")
	{ }

	required_device<m68000_base_device> m_maincpu;
	optional_device<cpu_device> m_z80snd;
	optional_device<ym2612_device> m_ymsnd;
	optional_device<timer_device> m_scan_timer;
	required_device<sega315_5313_device> m_vdp;
	optional_shared_ptr<uint16_t> m_megadrive_ram;

	optional_ioport m_io_reset;
	ioport_port *m_io_pad_3b[4];
	ioport_port *m_io_pad_6b[4];

	genesis_z80_vars m_genz80;
	int m_version_hi_nibble;

	void init_megadriv_c2();
	void init_megadrie();
	void init_megadriv();
	void init_megadrij();

	DECLARE_READ8_MEMBER(megadriv_68k_YM2612_read);
	DECLARE_WRITE8_MEMBER(megadriv_68k_YM2612_write);
	IRQ_CALLBACK_MEMBER(genesis_int_callback);
	void megadriv_init_common();

	void megadriv_z80_bank_w(uint16_t data);
	DECLARE_WRITE16_MEMBER( megadriv_68k_z80_bank_write );
	DECLARE_WRITE8_MEMBER(megadriv_z80_z80_bank_w);
	DECLARE_READ16_MEMBER( megadriv_68k_io_read );
	DECLARE_WRITE16_MEMBER( megadriv_68k_io_write );
	DECLARE_READ16_MEMBER( megadriv_68k_read_z80_ram );
	DECLARE_WRITE16_MEMBER( megadriv_68k_write_z80_ram );
	DECLARE_READ16_MEMBER( megadriv_68k_check_z80_bus );
	DECLARE_WRITE16_MEMBER( megadriv_68k_req_z80_bus );
	DECLARE_WRITE16_MEMBER ( megadriv_68k_req_z80_reset );
	DECLARE_READ8_MEMBER( z80_read_68k_banked_data );
	DECLARE_WRITE8_MEMBER( z80_write_68k_banked_data );
	DECLARE_WRITE8_MEMBER( megadriv_z80_vdp_write );
	DECLARE_READ8_MEMBER( megadriv_z80_vdp_read );
	DECLARE_READ8_MEMBER( megadriv_z80_unmapped_read );
	TIMER_CALLBACK_MEMBER(megadriv_z80_run_state);

	/* Megadrive / Genesis has 3 I/O ports */
	emu_timer *m_io_timeout[3];
	int m_io_stage[3];
	uint8_t m_megadrive_io_data_regs[3];
	uint8_t m_megadrive_io_ctrl_regs[3];
	uint8_t m_megadrive_io_tx_regs[3];
	read8_delegate m_megadrive_io_read_data_port_ptr;
	write16_delegate m_megadrive_io_write_data_port_ptr;

	WRITE_LINE_MEMBER(vdp_sndirqline_callback_genesis_z80);
	WRITE_LINE_MEMBER(vdp_lv6irqline_callback_genesis_68k);
	WRITE_LINE_MEMBER(vdp_lv4irqline_callback_genesis_68k);

	TIMER_CALLBACK_MEMBER( io_timeout_timer_callback );
	void megadrive_reset_io();
	DECLARE_READ8_MEMBER(megadrive_io_read_data_port_6button);
	DECLARE_READ8_MEMBER(megadrive_io_read_data_port_3button);
	uint8_t megadrive_io_read_ctrl_port(int portnum);
	uint8_t megadrive_io_read_tx_port(int portnum);
	uint8_t megadrive_io_read_rx_port(int portnum);
	uint8_t megadrive_io_read_sctrl_port(int portnum);

	DECLARE_WRITE16_MEMBER(megadrive_io_write_data_port_3button);
	DECLARE_WRITE16_MEMBER(megadrive_io_write_data_port_6button);
	void megadrive_io_write_ctrl_port(int portnum, uint16_t data);
	void megadrive_io_write_tx_port(int portnum, uint16_t data);
	void megadrive_io_write_rx_port(int portnum, uint16_t data);
	void megadrive_io_write_sctrl_port(int portnum, uint16_t data);

	void megadriv_stop_scanline_timer();

	DECLARE_MACHINE_START( megadriv );
	DECLARE_MACHINE_RESET( megadriv );
	DECLARE_VIDEO_START( megadriv );
	uint32_t screen_update_megadriv(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_megadriv);

	DECLARE_WRITE8_MEMBER(megadriv_tas_callback);

	void megadriv_timers(machine_config &config);
	void md_ntsc(machine_config &config);
	void md_pal(machine_config &config);
	void md_bootleg(machine_config &config);
	void dcat16_megadriv_base(machine_config &config);
	void dcat16_megadriv_map(address_map &map);
	void megadriv_map(address_map &map);
	void megadriv_z80_io_map(address_map &map);
	void megadriv_z80_map(address_map &map);
};

class md_cons_state : public md_base_state
{
public:
	md_cons_state(const machine_config &mconfig, device_type type, const char *tag) :
		md_base_state(mconfig, type, tag),
		m_32x(*this,"sega32x"),
		m_segacd(*this,"segacd"),
		m_cart(*this, "mdslot"),
		m_tmss(*this, "tmss")
	{ }

	ioport_port *m_io_ctrlr;
	ioport_port *m_io_pad3b[4];
	ioport_port *m_io_pad6b[2][4];

	optional_device<sega_32x_device> m_32x;
	optional_device<sega_segacd_device> m_segacd;
	optional_device<md_cart_slot_device> m_cart;
	optional_region_ptr<uint16_t> m_tmss;

	void init_mess_md_common();
	void init_genesis();
	void init_md_eur();
	void init_md_jpn();

	READ8_MEMBER(mess_md_io_read_data_port);
	WRITE16_MEMBER(mess_md_io_write_data_port);

	DECLARE_MACHINE_START( md_common );     // setup ioport_port
	DECLARE_MACHINE_START( ms_megadriv );   // setup ioport_port + install cartslot handlers
	DECLARE_MACHINE_START( ms_megacd );     // setup ioport_port + dma delay for cd
	DECLARE_MACHINE_RESET( ms_megadriv );

	DECLARE_WRITE_LINE_MEMBER(screen_vblank_console);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( _32x_cart );

	void _32x_scanline_callback(int x, uint32_t priority, uint16_t &lineptr);
	void _32x_interrupt_callback(int scanline, int irq6);
	void _32x_scanline_helper_callback(int scanline);

	void install_cartslot();
	void install_tmss();
	DECLARE_READ16_MEMBER(tmss_r);
	DECLARE_WRITE16_MEMBER(tmss_swap_w);
	void genesis_32x_scd(machine_config &config);
	void mdj_32x_scd(machine_config &config);
	void ms_megadpal(machine_config &config);
	void dcat16_megadriv_base(machine_config &config);
	void dcat16_megadriv(machine_config &config);
	void md_32x_scd(machine_config &config);
	void mdj_32x(machine_config &config);
	void ms_megadriv(machine_config &config);
	void mdj_scd(machine_config &config);
	void md_32x(machine_config &config);
	void genesis_32x(machine_config &config);
	void md_scd(machine_config &config);
	void genesis_scd(machine_config &config);
	void genesis_tmss(machine_config &config);
};

#endif // MAME_INCLUDES_MEGADRIV_H
