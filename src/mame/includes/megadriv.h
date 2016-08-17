// license:BSD-3-Clause
// copyright-holders:David Haywood

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

MACHINE_CONFIG_EXTERN( megadriv_timers );
MACHINE_CONFIG_EXTERN( md_ntsc );
MACHINE_CONFIG_EXTERN( md_pal );
MACHINE_CONFIG_EXTERN( md_bootleg );    // for topshoot.c & hshavoc.c


struct genesis_z80_vars
{
	int z80_is_reset;
	int z80_has_bus;
	UINT32 z80_bank_addr;
	std::unique_ptr<UINT8[]> z80_prgram;
};


class md_base_state : public driver_device
{
public:
	md_base_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_z80snd(*this,"genesis_snd_z80"),
		m_ymsnd(*this,"ymsnd"),
		m_vdp(*this,"gen_vdp"),
		m_snsnd(*this, "snsnd"),
		m_megadrive_ram(*this,"megadrive_ram"),
		m_io_reset(*this, "RESET")
	{ }
	required_device<m68000_base_device> m_maincpu;
	optional_device<cpu_device> m_z80snd;
	optional_device<ym2612_device> m_ymsnd;
	required_device<sega315_5313_device> m_vdp;
	required_device<sn76496_base_device> m_snsnd;
	optional_shared_ptr<UINT16> m_megadrive_ram;


	optional_ioport m_io_reset;
	ioport_port *m_io_pad_3b[4];
	ioport_port *m_io_pad_6b[4];

	genesis_z80_vars m_genz80;
	int m_version_hi_nibble;

	DECLARE_DRIVER_INIT(megadriv_c2);
	DECLARE_DRIVER_INIT(megadrie);
	DECLARE_DRIVER_INIT(megadriv);
	DECLARE_DRIVER_INIT(megadrij);

	DECLARE_READ8_MEMBER(megadriv_68k_YM2612_read);
	DECLARE_WRITE8_MEMBER(megadriv_68k_YM2612_write);
	IRQ_CALLBACK_MEMBER(genesis_int_callback);
	void megadriv_init_common();

	void megadriv_z80_bank_w(UINT16 data);
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
	UINT8 m_megadrive_io_data_regs[3];
	UINT8 m_megadrive_io_ctrl_regs[3];
	UINT8 m_megadrive_io_tx_regs[3];
	read8_delegate m_megadrive_io_read_data_port_ptr;
	write16_delegate m_megadrive_io_write_data_port_ptr;

	WRITE_LINE_MEMBER(vdp_sndirqline_callback_genesis_z80);
	WRITE_LINE_MEMBER(vdp_lv6irqline_callback_genesis_68k);
	WRITE_LINE_MEMBER(vdp_lv4irqline_callback_genesis_68k);

	TIMER_CALLBACK_MEMBER( io_timeout_timer_callback );
	void megadrive_reset_io();
	DECLARE_READ8_MEMBER(megadrive_io_read_data_port_6button);
	DECLARE_READ8_MEMBER(megadrive_io_read_data_port_3button);
	UINT8 megadrive_io_read_ctrl_port(int portnum);
	UINT8 megadrive_io_read_tx_port(int portnum);
	UINT8 megadrive_io_read_rx_port(int portnum);
	UINT8 megadrive_io_read_sctrl_port(int portnum);

	DECLARE_WRITE16_MEMBER(megadrive_io_write_data_port_3button);
	DECLARE_WRITE16_MEMBER(megadrive_io_write_data_port_6button);
	void megadrive_io_write_ctrl_port(int portnum, UINT16 data);
	void megadrive_io_write_tx_port(int portnum, UINT16 data);
	void megadrive_io_write_rx_port(int portnum, UINT16 data);
	void megadrive_io_write_sctrl_port(int portnum, UINT16 data);

	void megadriv_stop_scanline_timer();

	DECLARE_MACHINE_START( megadriv );
	DECLARE_MACHINE_RESET( megadriv );
	DECLARE_VIDEO_START( megadriv );
	UINT32 screen_update_megadriv(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof_megadriv(screen_device &screen, bool state);

	DECLARE_WRITE8_MEMBER(megadriv_tas_callback);
};

class md_cons_state : public md_base_state
{
public:
	md_cons_state(const machine_config &mconfig, device_type type, const char *tag)
	: md_base_state(mconfig, type, tag),
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
	optional_region_ptr<UINT16> m_tmss;

	DECLARE_DRIVER_INIT(mess_md_common);
	DECLARE_DRIVER_INIT(genesis);
	DECLARE_DRIVER_INIT(md_eur);
	DECLARE_DRIVER_INIT(md_jpn);

	READ8_MEMBER(mess_md_io_read_data_port);
	WRITE16_MEMBER(mess_md_io_write_data_port);

	DECLARE_MACHINE_START( md_common );     // setup ioport_port
	DECLARE_MACHINE_START( ms_megadriv );   // setup ioport_port + install cartslot handlers
	DECLARE_MACHINE_START( ms_megacd );     // setup ioport_port + dma delay for cd
	DECLARE_MACHINE_RESET( ms_megadriv );

	void screen_eof_console(screen_device &screen, bool state);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( _32x_cart );

	void _32x_scanline_callback(int x, UINT32 priority, UINT16 &lineptr);
	void _32x_interrupt_callback(int scanline, int irq6);
	void _32x_scanline_helper_callback(int scanline);

	void install_cartslot();
	void install_tmss();
	DECLARE_READ16_MEMBER(tmss_r);
	DECLARE_WRITE16_MEMBER(tmss_swap_w);
};
