// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "includes/megadriv.h"
#include "bus/megadrive/md_slot.h"
#include "bus/megadrive/md_carts.h"
#include "machine/mega32x.h"
#include "machine/megacd.h"


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
