// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Ferguson / Digital Research Computers Big Board II

2009-05-12 Skeleton driver.

This is very much under construction.

Despite the name, this is not like the xerox or bigboard at all.

It is compatible only if the software uses the same published
calls to the bios. Everything else is different.

80 = sio ce
84 = ctca ce
88 = ctcb ce
8c = dma ce
c0 = prog
c4 = status 7,6,5,4 = sw1-4; 3 = kbdstb; 2 = motor; 1 = rxdb; 0 = rxda
c8 = sys1
cc = sys2
d0 = kbd
d4 = 1793 ce
d8 = port7
dc = 6845 ce


Difficulties encountered:

The FDC has a INTRQ pin, the diagram says it goes to page 6, but
it just vanishes instead.

What works:

Turn it on, wait for cursor to appear in the top corner. Press Enter.
Now you can enter commands.

Memory banking:

0000-7FFF are controlled by bit 0 of port C8, and select ROM&video, or RAM
8000-FFFF control if RAM is onboard, or on S100 bus (do not know what controls this)
We do not emulate the S100, so therefore banks 1&2 are the same as 3&4.
The switching from port C8 is emulated.

ToDo:
- Finish floppy disk support (i have no boot disk)
- (optional) Connect SIO to RS232.
- (optional) Connect up the SASI, Centronics and other interfaces on ports D8-DB.
- (optional) Connect up the programming port C0-C3.
- (optional) Connect up the numerous board jumpers.
- Need software

Monitor commands:
B - boot from disk
C - copy memory
D - dump memory
F - fill memory
G - go
I - in port
M - modify memory
O - out port
R - read a sector
T - test memory
V - compare blocks of memory
X - change banks

****************************************************************************/



#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/z80daisy.h"
#include "machine/74259.h"
#include "machine/clock.h"
#include "machine/keyboard.h"
#include "machine/wd_fdc.h"
#include "machine/z80ctc.h"
#include "machine/z80dma.h"
#include "machine/z80sio.h"
#include "sound/beep.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class bigbord2_state : public driver_device
{
public:
	bigbord2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_p_ram(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
		, m_ctc1(*this, "ctc1")
		, m_ctc2(*this, "ctc2")
		, m_sio(*this, "sio")
		, m_dma(*this, "dma")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_syslatch1(*this, "syslatch1")
		, m_dsw(*this, "DSW")
		, m_bankr(*this, "bankr")
		, m_bankv(*this, "bankv")
		, m_banka(*this, "banka")
	{
	}

	DECLARE_WRITE_LINE_MEMBER(side_select_w);
	DECLARE_WRITE_LINE_MEMBER(smc1_w);
	DECLARE_WRITE_LINE_MEMBER(smc2_w);
	DECLARE_WRITE_LINE_MEMBER(head_load_w);
	DECLARE_WRITE_LINE_MEMBER(disk_motor_w);
	DECLARE_WRITE8_MEMBER(syslatch2_w);
	DECLARE_READ8_MEMBER(status_port_r);
	DECLARE_READ8_MEMBER(kbd_r);
	void kbd_put(u8 data);
	DECLARE_WRITE_LINE_MEMBER(clock_w);
	DECLARE_WRITE_LINE_MEMBER(busreq_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z1_w);
	DECLARE_WRITE_LINE_MEMBER(sio_wrdya_w);
	DECLARE_WRITE_LINE_MEMBER(sio_wrdyb_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	void init_bigbord2();
	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_READ8_MEMBER(io_read_byte);
	DECLARE_WRITE8_MEMBER(io_write_byte);
	MC6845_UPDATE_ROW(crtc_update_row);

	void bigbord2(machine_config &config);
	void bigbord2_io(address_map &map);
	void bigbord2_mem(address_map &map);
private:
	u8 crt8002(u8 ac_ra, u8 ac_chr, u8 ac_attr, uint16_t ac_cnt, bool ac_curs);
	u8 m_term_data;
	u8 m_term_status;
	uint16_t m_cnt;
	bool m_cc[8];
	floppy_image_device *m_floppy;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	address_space *m_mem;
	address_space *m_io;
	required_device<palette_device> m_palette;
	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_p_ram;
	required_region_ptr<u8> m_p_chargen;
	required_device<z80ctc_device> m_ctc1;
	required_device<z80ctc_device> m_ctc2;
	required_device<z80sio_device> m_sio;
	required_device<z80dma_device> m_dma;
	required_device<mb8877_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<ls259_device> m_syslatch1;
	required_ioport m_dsw;
	required_memory_bank m_bankr;
	required_memory_bank m_bankv;
	required_memory_bank m_banka;
};

/* Status port
    0 = RXDA
    1 = RXDB
    2 = MOTOR
    3 = KBDSTB
    4 = DIPSW 1
    5 = DIPSW 2
    6 = DIPSW 3
    7 = DIPSW 4 */

READ8_MEMBER(bigbord2_state::status_port_r)
{
	u8 ret = m_term_status | 3 | (m_syslatch1->q6_r() << 2) | m_dsw->read();
	m_term_status = 0;
	return ret;
}

// KBD port - read ascii value of key pressed

READ8_MEMBER(bigbord2_state::kbd_r)
{
	u8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

void bigbord2_state::kbd_put(u8 data)
{
	if (data)
	{
		m_term_data = data;
		m_term_status = 8;
		m_ctc1->trg0(0);
		m_ctc1->trg0(1);
	}
}

WRITE_LINE_MEMBER( bigbord2_state::sio_wrdya_w )
{
	m_cc[0] = state;
}

WRITE_LINE_MEMBER( bigbord2_state::sio_wrdyb_w )
{
	m_cc[1] = state;
}

WRITE_LINE_MEMBER( bigbord2_state::fdc_drq_w )
{
	m_cc[2] = state;
}


/* Z80 DMA */


WRITE_LINE_MEMBER( bigbord2_state::busreq_w )
{
// since our Z80 has no support for BUSACK, we assume it is granted immediately
	m_maincpu->set_input_line(Z80_INPUT_LINE_BUSRQ, state);
	m_dma->bai_w(state); // tell dma that bus has been granted
}

READ8_MEMBER( bigbord2_state::memory_read_byte )
{
	return m_mem->read_byte(offset);
}

WRITE8_MEMBER( bigbord2_state::memory_write_byte )
{
	m_mem->write_byte(offset, data);
}

READ8_MEMBER( bigbord2_state::io_read_byte )
{
	return m_io->read_byte(offset);
}

WRITE8_MEMBER( bigbord2_state::io_write_byte )
{
	m_io->write_byte(offset, data);
}


/* Read/Write Handlers */

WRITE_LINE_MEMBER(bigbord2_state::side_select_w)
{
	if (m_floppy)
		m_floppy->ss_w(state);
}

WRITE_LINE_MEMBER(bigbord2_state::smc1_w)
{
	// connects to "U6 (FDC9216B)" which drives the fdc "rawread" and "rclk" pins
}

WRITE_LINE_MEMBER(bigbord2_state::smc2_w)
{
	// connects to "U6 (FDC9216B)" which drives the fdc "rawread" and "rclk" pins
}

WRITE_LINE_MEMBER(bigbord2_state::head_load_w)
{
	// connects to HLD pin on floppy drive
}

WRITE_LINE_MEMBER(bigbord2_state::disk_motor_w)
{
	// motor on
	if (m_floppy)
		m_floppy->mon_w(state ? 0 : 1);
}

WRITE8_MEMBER(bigbord2_state::syslatch2_w)
{
	/*

	    bit     signal      description

	    0,1,2   operates a 74LS151 for 8 individual inputs to DMA RDY
	      0     W/RDYA      channel A of SIO
	      1     W/RDYB      channel B of SIO
	      2     DRQ         DRQ on fdc
	      3     JB7 pin 1
	      4     JB7 pin 2
	      5     JB7 pin 3
	      6     JB7 pin 4
	      7     JB7 pin 5
	    3       /TEST       test pin on FDC
	    4       DS3         drive 3 select
	    5       DS2         drive 2 select
	    6       DS1         drive 1 select
	    7       DS0         drive 0 select

	*/

	/* drive select */
	m_floppy = nullptr;
	if (BIT(data, 7)) m_floppy = m_floppy0->get_device();
	if (BIT(data, 6)) m_floppy = m_floppy1->get_device();
	//if (BIT(data, 5)) m_floppy = m_floppy2->get_device();
	//if (BIT(data, 4)) m_floppy = m_floppy3->get_device();

	m_fdc->set_floppy(m_floppy);
	if (m_floppy)
	{
		m_floppy->ss_w(m_syslatch1->q1_r());
		m_floppy->mon_w(m_syslatch1->q6_r() ? 0 : 1);
	}

	m_dma->rdy_w(m_cc[data & 7]);
}



/* Memory Maps */

void bigbord2_state::bigbord2_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).bankrw("bankr");
	map(0x1000, 0x5fff).ram();
	map(0x6000, 0x6fff).bankrw("bankv");
	map(0x7000, 0x7fff).bankrw("banka");
	map(0x8000, 0xffff).ram();
}

void bigbord2_state::bigbord2_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x80, 0x83).rw(m_sio, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w)); // u16
	map(0x84, 0x87).rw(m_ctc1, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write)); // u37 has issues
	map(0x88, 0x8b).rw(m_ctc2, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write)); // u21
	map(0x8c, 0x8f).rw(m_dma, FUNC(z80dma_device::bus_r), FUNC(z80dma_device::bus_w)); // u62
	map(0xc0, 0xc3).w("proglatch", FUNC(ls259_device::write_nibble_d3)); // u41 - eprom programming port
	map(0xc4, 0xc7).r(FUNC(bigbord2_state::status_port_r)); // u11
	map(0xc8, 0xcb).w(m_syslatch1, FUNC(ls259_device::write_nibble_d3)); // u14
	map(0xcc, 0xcf).w(FUNC(bigbord2_state::syslatch2_w));
	map(0xd0, 0xd3).r(FUNC(bigbord2_state::kbd_r)); // u1
	map(0xd4, 0xd7).rw(m_fdc, FUNC(mb8877_device::read), FUNC(mb8877_device::write)); // u10
	//AM_RANGE(0xd8, 0xdb) AM_READWRITE(portd8_r, portd8_w) // various external data ports; DB = centronics printer
	map(0xd9, 0xd9).w("outlatch1", FUNC(ls259_device::write_nibble_d3)); // u96
	map(0xdc, 0xdc).mirror(2).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w)); // u30
	map(0xdd, 0xdd).mirror(2).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
}


/* Input Ports */

static INPUT_PORTS_START( bigbord2 )
	PORT_START("DSW")
	PORT_BIT( 0xf, 0, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x10, "Switch 4") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x20, 0x00, "Switch 3") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x40, 0x00, "Switch 2") PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x80, 0x00, "Switch 1") PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
INPUT_PORTS_END


/* Z80 CTC */

WRITE_LINE_MEMBER( bigbord2_state::clock_w )
{
	m_ctc2->trg0(state);
	m_ctc2->trg1(state);
	if (m_floppy)
		m_ctc1->trg1(m_floppy->idx_r());
}

// there's a multitude of optional jumpers in this area, but this will do
WRITE_LINE_MEMBER( bigbord2_state::ctc_z1_w )
{
	m_sio->rxca_w(state);
	m_sio->txca_w(state);
}

/* Z80 Daisy Chain */

static const z80_daisy_config daisy_chain[] =
{
	{ "dma" },
	{ "ctc1" },
	{ "ctc2" },
	{ "sio" },
	{ nullptr }
};

/* WD1793 Interface */

static void bigbord2_floppies(device_slot_interface &device)
{
	device.option_add("8dsdd", FLOPPY_8_DSDD);
}


/* Machine Initialization */

void bigbord2_state::machine_start()
{
	/* register for state saving */
	save_item(NAME(m_term_data));
	m_floppy = nullptr;
}

void bigbord2_state::machine_reset()
{
	u8 i;
	for (i = 0; i < 8; i++)
		m_cc[i] = 1;
	m_cc[2] = 0;
	m_bankr->set_entry(0);
	m_bankv->set_entry(0);
	m_banka->set_entry(0);
}

void bigbord2_state::init_bigbord2()
{
	m_mem = &m_maincpu->space(AS_PROGRAM);
	m_io = &m_maincpu->space(AS_IO);
	m_bankr->configure_entries(0, 2, &m_p_ram[0x0000], 0x10000);
	m_bankv->configure_entries(0, 2, &m_p_ram[0x6000], 0x10000);
	m_banka->configure_entries(0, 2, &m_p_ram[0x7000], 0x10000);
}


/* Screen */

/* F4 Character Displayer */
static const gfx_layout crt8002_charlayout =
{
	8, 12,                   /* 7 x 11 characters */
	128,                  /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_crt8002 )
	GFXDECODE_ENTRY( "chargen", 0x0000, crt8002_charlayout, 0, 1 )
GFXDECODE_END

u8 bigbord2_state::crt8002(u8 ac_ra, u8 ac_chr, u8 ac_attr, uint16_t ac_cnt, bool ac_curs)
{
	u8 gfx = 0;
	switch (ac_attr & 3)
	{
		case 0: // lores gfx
			switch (ac_ra)
			{
				case 0:
				case 1:
				case 2:
					gfx = (BIT(ac_chr, 7) ? 0xf8 : 0) | (BIT(ac_chr, 3) ? 7 : 0);
					break;
				case 3:
				case 4:
				case 5:
					gfx = (BIT(ac_chr, 6) ? 0xf8 : 0) | (BIT(ac_chr, 2) ? 7 : 0);
					break;
				case 6:
				case 7:
				case 8:
					gfx = (BIT(ac_chr, 5) ? 0xf8 : 0) | (BIT(ac_chr, 1) ? 7 : 0);
					break;
				default:
					gfx = (BIT(ac_chr, 4) ? 0xf8 : 0) | (BIT(ac_chr, 0) ? 7 : 0);
					break;
			}
			break;
		case 1: // external mode
			gfx = bitswap<8>(ac_chr, 0,1,2,3,4,5,6,7);
			break;
		case 2: // thin gfx
			break;
		case 3: // alpha
			gfx = m_p_chargen[((ac_chr & 0x7f)<<4) | ac_ra];
			break;
	}

	if (BIT(ac_attr, 3) & (ac_ra == 11)) // underline
		gfx = 0xff;
	if (BIT(ac_attr, 2) & ((ac_ra == 5) | (ac_ra == 6))) // strike-through
		gfx = 0xff;
	if (BIT(ac_attr, 6) & BIT(ac_cnt, 13)) // flash
		gfx = 0;
	if (BIT(ac_attr, 5)) // blank
		gfx = 0;
	if (ac_curs && BIT(ac_cnt, 14)) // cursor
		gfx ^= 0xff;
	if (BIT(ac_attr, 4)) // reverse video
		gfx ^= 0xff;
	return gfx;
}

MC6845_UPDATE_ROW( bigbord2_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	u8 chr,gfx,attr;
	uint16_t mem,x;
	uint32_t *p = &bitmap.pix32(y);
	ra &= 15;
	m_cnt++;

	for (x = 0; x < x_count; x++)
	{
		mem = (ma + x) & 0x7ff;
		attr = m_p_ram[mem + 0x7000];
		chr = m_p_ram[mem + 0x6000];

		/* process attributes */
		gfx = crt8002(ra, chr, attr, m_cnt, (x==cursor_x));

		/* Display a scanline of a character */
		*p++ = palette[BIT( gfx, 7 )];
		*p++ = palette[BIT( gfx, 6 )];
		*p++ = palette[BIT( gfx, 5 )];
		*p++ = palette[BIT( gfx, 4 )];
		*p++ = palette[BIT( gfx, 3 )];
		*p++ = palette[BIT( gfx, 2 )];
		*p++ = palette[BIT( gfx, 1 )];
		*p++ = palette[BIT( gfx, 0 )];
	}
}

/* Machine Drivers */

#define MAIN_CLOCK 8_MHz_XTAL / 2

MACHINE_CONFIG_START(bigbord2_state::bigbord2)
	/* basic machine hardware */
	Z80(config, m_maincpu, MAIN_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &bigbord2_state::bigbord2_mem);
	m_maincpu->set_addrmap(AS_IO, &bigbord2_state::bigbord2_io);
	m_maincpu->set_daisy_config(daisy_chain);

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(10.69425_MHz_XTAL, 700, 0, 560, 260, 0, 240)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)
	GFXDECODE(config, "gfxdecode", m_palette, gfx_crt8002);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	CLOCK(config, "ctc_clock", MAIN_CLOCK).signal_handler().set(FUNC(bigbord2_state::clock_w));

	/* devices */
	Z80DMA(config, m_dma, MAIN_CLOCK);
	m_dma->out_busreq_callback().set(FUNC(bigbord2_state::busreq_w));
	m_dma->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_dma->in_mreq_callback().set(FUNC(bigbord2_state::memory_read_byte));
	m_dma->out_mreq_callback().set(FUNC(bigbord2_state::memory_write_byte));
	m_dma->in_iorq_callback().set(FUNC(bigbord2_state::io_read_byte));
	m_dma->out_iorq_callback().set(FUNC(bigbord2_state::io_write_byte));

	Z80SIO(config, m_sio, MAIN_CLOCK);
	m_sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_sio->out_synca_callback().set(m_ctc1, FUNC(z80ctc_device::trg2));
	m_sio->out_wrdya_callback().set(FUNC(bigbord2_state::sio_wrdya_w));
	m_sio->out_wrdyb_callback().set(FUNC(bigbord2_state::sio_wrdyb_w));

	Z80CTC(config, m_ctc1, MAIN_CLOCK);
	m_ctc1->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80CTC(config, m_ctc2, MAIN_CLOCK);
	m_ctc2->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc2->zc_callback<0>().set(m_sio, FUNC(z80sio_device::rxtxcb_w));    // to SIO Ch B
	m_ctc2->zc_callback<1>().set(FUNC(bigbord2_state::ctc_z1_w));  // to SIO Ch A
	m_ctc2->zc_callback<2>().set(m_ctc2, FUNC(z80ctc_device::trg3));

	MB8877(config, m_fdc, 16_MHz_XTAL / 8); // 2MHz for 8 inch, or 1MHz otherwise (jumper-selectable)
	//m_fdc->intrq_wr_callback().set_inputline(m_maincpu, ??); // info missing from schematic
	FLOPPY_CONNECTOR(config, "fdc:0", bigbord2_floppies, "8dsdd", floppy_image_device::default_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", bigbord2_floppies, "8dsdd", floppy_image_device::default_floppy_formats).enable_sound(true);

	mc6845_device &crtc(MC6845(config, "crtc", 16_MHz_XTAL / 8));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(bigbord2_state::crtc_update_row), this);
	crtc.out_vsync_callback().set(m_ctc1, FUNC(z80ctc_device::trg3));

	ls259_device &proglatch(LS259(config, "proglatch")); // U41
	proglatch.q_out_cb<6>().set("outlatch1", FUNC(ls259_device::clear_w)); // FCRST - also resets the 8877

	LS259(config, m_syslatch1, 0); // U14
	m_syslatch1->q_out_cb<0>().set_membank(m_bankr); // D_S
	m_syslatch1->q_out_cb<0>().append_membank(m_bankv);
	m_syslatch1->q_out_cb<0>().append_membank(m_banka);
	m_syslatch1->q_out_cb<1>().set(FUNC(bigbord2_state::side_select_w)); // SIDSEL
	m_syslatch1->q_out_cb<2>().set(FUNC(bigbord2_state::smc1_w)); // SMC1
	m_syslatch1->q_out_cb<3>().set(FUNC(bigbord2_state::smc2_w)); // SMC2
	m_syslatch1->q_out_cb<4>().set(m_fdc, FUNC(mb8877_device::dden_w)); // DDEN
	m_syslatch1->q_out_cb<5>().set(FUNC(bigbord2_state::head_load_w)); // HLD
	m_syslatch1->q_out_cb<6>().set(FUNC(bigbord2_state::disk_motor_w)); // MOTOR
	m_syslatch1->q_out_cb<7>().set("beeper", FUNC(beep_device::set_state)); // BELL

	MCFG_DEVICE_ADD("outlatch1", LS259, 0) // U96

	/* keyboard */
	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(bigbord2_state::kbd_put));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("beeper", BEEP, 950) // actual frequency is unknown
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


/* ROMs */


ROM_START( bigbord2 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "bigbrdii.bin", 0x0000, 0x1000, CRC(c588189e) SHA1(4133903171ee8b9fcf12cc72de843af782b4a645) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "8002.bin", 0x0000, 0x0800, CRC(fdd6eb13) SHA1(a094d416e66bdab916e72238112a6265a75ca690) )
ROM_END
/* System Drivers */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT           COMPANY                       FULLNAME        FLAGS
COMP( 1982, bigbord2, 0,      0,      bigbord2, bigbord2, bigbord2_state, init_bigbord2, "Digital Research Computers", "Big Board II", MACHINE_NOT_WORKING )
