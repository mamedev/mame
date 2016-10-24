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
	uint32_t z80_bank_addr;
	std::unique_ptr<uint8_t[]> z80_prgram;
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

	uint8_t megadriv_68k_YM2612_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void megadriv_68k_YM2612_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	int genesis_int_callback(device_t &device, int irqline);
	void megadriv_init_common();

	void megadriv_z80_bank_w(uint16_t data);
	void megadriv_68k_z80_bank_write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void megadriv_z80_z80_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t megadriv_68k_io_read(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void megadriv_68k_io_write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t megadriv_68k_read_z80_ram(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void megadriv_68k_write_z80_ram(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t megadriv_68k_check_z80_bus(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void megadriv_68k_req_z80_bus(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void megadriv_68k_req_z80_reset(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t z80_read_68k_banked_data(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void z80_write_68k_banked_data(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void megadriv_z80_vdp_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t megadriv_z80_vdp_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t megadriv_z80_unmapped_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void megadriv_z80_run_state(void *ptr, int32_t param);

	/* Megadrive / Genesis has 3 I/O ports */
	emu_timer *m_io_timeout[3];
	int m_io_stage[3];
	uint8_t m_megadrive_io_data_regs[3];
	uint8_t m_megadrive_io_ctrl_regs[3];
	uint8_t m_megadrive_io_tx_regs[3];
	read8_delegate m_megadrive_io_read_data_port_ptr;
	write16_delegate m_megadrive_io_write_data_port_ptr;

	void vdp_sndirqline_callback_genesis_z80(int state);
	void vdp_lv6irqline_callback_genesis_68k(int state);
	void vdp_lv4irqline_callback_genesis_68k(int state);

	void io_timeout_timer_callback(void *ptr, int32_t param);
	void megadrive_reset_io();
	uint8_t megadrive_io_read_data_port_6button(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t megadrive_io_read_data_port_3button(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t megadrive_io_read_ctrl_port(int portnum);
	uint8_t megadrive_io_read_tx_port(int portnum);
	uint8_t megadrive_io_read_rx_port(int portnum);
	uint8_t megadrive_io_read_sctrl_port(int portnum);

	void megadrive_io_write_data_port_3button(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void megadrive_io_write_data_port_6button(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void megadrive_io_write_ctrl_port(int portnum, uint16_t data);
	void megadrive_io_write_tx_port(int portnum, uint16_t data);
	void megadrive_io_write_rx_port(int portnum, uint16_t data);
	void megadrive_io_write_sctrl_port(int portnum, uint16_t data);

	void megadriv_stop_scanline_timer();

	void machine_start_megadriv();
	void machine_reset_megadriv();
	void video_start_megadriv();
	uint32_t screen_update_megadriv(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof_megadriv(screen_device &screen, bool state);

	void megadriv_tas_callback(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
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
	optional_region_ptr<uint16_t> m_tmss;

	void init_mess_md_common();
	void init_genesis();
	void init_md_eur();
	void init_md_jpn();

	uint8_t mess_md_io_read_data_port(address_space &space, offs_t offset, uint8_t mem_mask);
	void mess_md_io_write_data_port(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask);

	void machine_start_md_common();     // setup ioport_port
	void machine_start_ms_megadriv();   // setup ioport_port + install cartslot handlers
	void machine_start_ms_megacd();     // setup ioport_port + dma delay for cd
	void machine_reset_ms_megadriv();

	void screen_eof_console(screen_device &screen, bool state);

	image_init_result device_image_load__32x_cart(device_image_interface &image);

	void _32x_scanline_callback(int x, uint32_t priority, uint16_t &lineptr);
	void _32x_interrupt_callback(int scanline, int irq6);
	void _32x_scanline_helper_callback(int scanline);

	void install_cartslot();
	void install_tmss();
	uint16_t tmss_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void tmss_swap_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
};
