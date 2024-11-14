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


namespace {

class bigbord2_state : public driver_device
{
public:
	bigbord2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
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
		, m_bankw(*this, "bankw")
		, m_bankv(*this, "bankv")
		, m_banka(*this, "banka")
		, m_bankv1(*this, "bankv1")
		, m_banka1(*this, "banka1")
	{ }

	void bigbord2(machine_config &config);
	void init_bigbord2();

private:
	void side_select_w(int state);
	void smc1_w(int state);
	void smc2_w(int state);
	void head_load_w(int state);
	void disk_motor_w(int state);
	void syslatch2_w(u8 data);
	u8 status_port_r();
	u8 kbd_r();
	void kbd_put(u8 data);
	void clock_w(int state);
	void ctc_z1_w(int state);
	void sio_wrdya_w(int state);
	void sio_wrdyb_w(int state);
	void fdc_drq_w(int state);
	u8 memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, u8 data);
	u8 io_read_byte(offs_t offset);
	void io_write_byte(offs_t offset, u8 data);
	MC6845_UPDATE_ROW(crtc_update_row);
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	u8 crt8002(u8 ac_ra, u8 ac_chr, u8 ac_attr, uint16_t ac_cnt, bool ac_curs);
	u8 m_term_data = 0U;
	u8 m_term_status = 0U;
	uint16_t m_cnt = 0U;
	bool m_cc[8]{};
	floppy_image_device *m_floppy;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	address_space *m_mem;
	address_space *m_io;
	std::unique_ptr<u8[]> m_vram; // video ram 2k
	std::unique_ptr<u8[]> m_aram; // attribute ram 2k
	std::unique_ptr<u8[]> m_ram;  // main ram 64k
	std::unique_ptr<u8[]> m_dummy;  // black hole for write to rom
	required_device<palette_device> m_palette;
	required_device<z80_device> m_maincpu;
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
	required_memory_bank m_bankw;
	required_memory_bank m_bankv;
	required_memory_bank m_banka;
	required_memory_bank m_bankv1;
	required_memory_bank m_banka1;
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

u8 bigbord2_state::status_port_r()
{
	u8 ret = m_term_status | 3 | (m_syslatch1->q6_r() << 2) | m_dsw->read();
	m_term_status = 0;
	return ret;
}

// KBD port - read ascii value of key pressed

u8 bigbord2_state::kbd_r()
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

void bigbord2_state::sio_wrdya_w(int state)
{
	m_cc[0] = state;
}

void bigbord2_state::sio_wrdyb_w(int state)
{
	m_cc[1] = state;
}

void bigbord2_state::fdc_drq_w(int state)
{
	m_cc[2] = state;
}


/* Z80 DMA */


u8 bigbord2_state::memory_read_byte(offs_t offset)
{
	return m_mem->read_byte(offset);
}

void bigbord2_state::memory_write_byte(offs_t offset, u8 data)
{
	m_mem->write_byte(offset, data);
}

u8 bigbord2_state::io_read_byte(offs_t offset)
{
	return m_io->read_byte(offset);
}

void bigbord2_state::io_write_byte(offs_t offset, u8 data)
{
	m_io->write_byte(offset, data);
}


/* Read/Write Handlers */

void bigbord2_state::side_select_w(int state)
{
	if (m_floppy)
		m_floppy->ss_w(state);
}

void bigbord2_state::smc1_w(int state)
{
	// connects to "U6 (FDC9216B)" which drives the fdc "rawread" and "rclk" pins
}

void bigbord2_state::smc2_w(int state)
{
	// connects to "U6 (FDC9216B)" which drives the fdc "rawread" and "rclk" pins
}

void bigbord2_state::head_load_w(int state)
{
	// connects to HLD pin on floppy drive
}

void bigbord2_state::disk_motor_w(int state)
{
	// motor on
	if (m_floppy)
		m_floppy->mon_w(state ? 0 : 1);
}

void bigbord2_state::syslatch2_w(u8 data)
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

void bigbord2_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x5fff).bankr(m_bankr).bankw(m_bankw);
	map(0x6000, 0x67ff).bankrw(m_bankv);
	map(0x6800, 0x6fff).bankrw(m_bankv1);
	map(0x7000, 0x77ff).bankrw(m_banka);
	map(0x7800, 0x7fff).bankrw(m_banka1);
	map(0x8000, 0xffff).ram();
}

void bigbord2_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x80, 0x83).rw(m_sio, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w)); // u16
	map(0x84, 0x87).rw(m_ctc1, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write)); // u37 has issues
	map(0x88, 0x8b).rw(m_ctc2, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write)); // u21
	map(0x8c, 0x8f).rw(m_dma, FUNC(z80dma_device::read), FUNC(z80dma_device::write)); // u62
	map(0xc0, 0xc3).w("proglatch", FUNC(ls259_device::write_nibble_d3)); // u41 - eprom programming port
	map(0xc4, 0xc7).r(FUNC(bigbord2_state::status_port_r)); // u11
	map(0xc8, 0xcb).w(m_syslatch1, FUNC(ls259_device::write_nibble_d3)); // u14
	map(0xcc, 0xcf).w(FUNC(bigbord2_state::syslatch2_w));
	map(0xd0, 0xd3).r(FUNC(bigbord2_state::kbd_r)); // u1
	map(0xd4, 0xd7).rw(m_fdc, FUNC(mb8877_device::read), FUNC(mb8877_device::write)); // u10
	//map(0xd8, 0xdb).rw(FUNC(bigbord2_state::portd8_r), FUNC(bigbord2_state::portd8_w)); // various external data ports; DB = centronics printer
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

void bigbord2_state::clock_w(int state)
{
	m_ctc2->trg0(state);
	m_ctc2->trg1(state);
	if (m_floppy)
		m_ctc1->trg1(m_floppy->idx_r());
}

// there's a multitude of optional jumpers in this area, but this will do
void bigbord2_state::ctc_z1_w(int state)
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
	save_pointer(NAME(m_vram), 0x2000);
	save_pointer(NAME(m_aram), 0x2000);
	save_pointer(NAME(m_ram),  0x8000);

	/* register for state saving */
	save_item(NAME(m_term_data));
	save_item(NAME(m_term_status));
	save_item(NAME(m_cnt));
	save_item(NAME(m_cc));

	m_floppy = nullptr;
}

void bigbord2_state::machine_reset()
{
	u8 i;
	for (i = 0; i < 8; i++)
		m_cc[i] = 1;
	m_cc[2] = 0;
	m_bankr->set_entry(0);
	m_bankw->set_entry(0);
	m_bankv->set_entry(0);
	m_banka->set_entry(0);
	m_bankv1->set_entry(0);
	m_banka1->set_entry(0);
}

void bigbord2_state::init_bigbord2()
{
	m_mem = &m_maincpu->space(AS_PROGRAM);
	m_io = &m_maincpu->space(AS_IO);
	m_vram = std::make_unique<u8[]>(0x2000);
	m_aram = std::make_unique<u8[]>(0x2000);
	m_ram = make_unique_clear<u8[]>(0x8000);
	m_dummy = std::make_unique<u8[]>(0x6000);

	u8 *v = m_vram.get();
	u8 *a = m_aram.get();
	u8 *r = m_ram.get();
	u8 *d = m_dummy.get();
	u8 *m = memregion("maincpu")->base();
	m_bankr->configure_entry( 0, &m[0]);
	m_bankr->configure_entry( 1, r);
	m_bankw->configure_entry( 0, d);
	m_bankw->configure_entry( 1, r);
	m_bankv->configure_entry( 0, v);
	m_bankv->configure_entry( 1, r+0x6000);
	m_bankv1->configure_entry(0, v);
	m_bankv1->configure_entry(1, r+0x6800);
	m_banka->configure_entry( 0, a);
	m_banka->configure_entry( 1, r+0x7000);
	m_banka1->configure_entry(0, a);
	m_banka1->configure_entry(1, r+0x7800);
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
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	uint32_t *p = &bitmap.pix(y);
	ra &= 15;
	m_cnt++;

	for (u16 x = 0; x < x_count; x++)
	{
		u16 mem = (ma + x) & 0x7ff;
		u8 attr = m_aram[mem];
		u8 chr = m_vram[mem];

		/* process attributes */
		u8 gfx = crt8002(ra, chr, attr, m_cnt, (x==cursor_x));

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

void bigbord2_state::bigbord2(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MAIN_CLOCK);  // U39
	m_maincpu->set_addrmap(AS_PROGRAM, &bigbord2_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &bigbord2_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);
	m_maincpu->busack_cb().set(m_dma, FUNC(z80dma_device::bai_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(10.69425_MHz_XTAL, 700, 0, 560, 260, 0, 240);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	GFXDECODE(config, "gfxdecode", m_palette, gfx_crt8002);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	CLOCK(config, "ctc_clock", MAIN_CLOCK).signal_handler().set(FUNC(bigbord2_state::clock_w));

	/* devices */
	Z80DMA(config, m_dma, MAIN_CLOCK);  // U62
	m_dma->out_busreq_callback().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSRQ);
	m_dma->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_dma->in_mreq_callback().set(FUNC(bigbord2_state::memory_read_byte));
	m_dma->out_mreq_callback().set(FUNC(bigbord2_state::memory_write_byte));
	m_dma->in_iorq_callback().set(FUNC(bigbord2_state::io_read_byte));
	m_dma->out_iorq_callback().set(FUNC(bigbord2_state::io_write_byte));

	Z80SIO(config, m_sio, MAIN_CLOCK); // U16
	m_sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_sio->out_synca_callback().set(m_ctc1, FUNC(z80ctc_device::trg2));
	m_sio->out_wrdya_callback().set(FUNC(bigbord2_state::sio_wrdya_w));
	m_sio->out_wrdyb_callback().set(FUNC(bigbord2_state::sio_wrdyb_w));

	Z80CTC(config, m_ctc1, MAIN_CLOCK); // U37
	m_ctc1->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80CTC(config, m_ctc2, MAIN_CLOCK); // U21
	m_ctc2->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc2->zc_callback<0>().set(m_sio, FUNC(z80sio_device::rxtxcb_w));    // to SIO Ch B
	m_ctc2->zc_callback<1>().set(FUNC(bigbord2_state::ctc_z1_w));  // to SIO Ch A
	m_ctc2->zc_callback<2>().set(m_ctc2, FUNC(z80ctc_device::trg3));

	MB8877(config, m_fdc, 16_MHz_XTAL / 8); // U10 : 2MHz for 8 inch, or 1MHz otherwise (jumper-selectable)
	//m_fdc->intrq_wr_callback().set_inputline(m_maincpu, ??); // info missing from schematic
	m_fdc->drq_wr_callback().set(FUNC(bigbord2_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", bigbord2_floppies, "8dsdd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", bigbord2_floppies, "8dsdd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	mc6845_device &crtc(MC6845(config, "crtc", 16_MHz_XTAL / 8));  // U30
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(bigbord2_state::crtc_update_row));
	crtc.out_vsync_callback().set(m_ctc1, FUNC(z80ctc_device::trg3));

	ls259_device &proglatch(LS259(config, "proglatch")); // U41
	// d0=to U42; d1=DECODE; d3=PGM; d4=VPPENB; d5=STD-B8
	proglatch.q_out_cb<6>().set("outlatch1", FUNC(ls259_device::clear_w)); // FCRST - also resets the 8877

	LS259(config, m_syslatch1, 0); // U14
	m_syslatch1->q_out_cb<0>().set_membank(m_bankr); // D_S
	m_syslatch1->q_out_cb<0>().append_membank(m_bankv);
	m_syslatch1->q_out_cb<0>().append_membank(m_banka);
	m_syslatch1->q_out_cb<0>().append_membank(m_bankw);
	m_syslatch1->q_out_cb<0>().append_membank(m_bankv1);
	m_syslatch1->q_out_cb<0>().append_membank(m_banka1);
	m_syslatch1->q_out_cb<1>().set(FUNC(bigbord2_state::side_select_w)); // SIDSEL
	m_syslatch1->q_out_cb<2>().set(FUNC(bigbord2_state::smc1_w)); // SMC1
	m_syslatch1->q_out_cb<3>().set(FUNC(bigbord2_state::smc2_w)); // SMC2
	m_syslatch1->q_out_cb<4>().set(m_fdc, FUNC(mb8877_device::dden_w)); // DDEN
	m_syslatch1->q_out_cb<5>().set(FUNC(bigbord2_state::head_load_w)); // HLD
	m_syslatch1->q_out_cb<6>().set(FUNC(bigbord2_state::disk_motor_w)); // MOTOR
	m_syslatch1->q_out_cb<7>().set("beeper", FUNC(beep_device::set_state)); // BELL

	LS259(config, "outlatch1", 0); // U96

	/* keyboard */
	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(bigbord2_state::kbd_put));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, "beeper", 950).add_route(ALL_OUTPUTS, "mono", 0.50); // actual frequency is unknown
}


/* ROMs */

ROM_START( bigbord2 )
	// for optional roms and eproms
	ROM_REGION( 0x6000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bigbrdii.u85", 0x0000, 0x1000, CRC(c588189e) SHA1(4133903171ee8b9fcf12cc72de843af782b4a645) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "8002.u52", 0x0000, 0x0800, CRC(fdd6eb13) SHA1(a094d416e66bdab916e72238112a6265a75ca690) )

	ROM_REGION( 0x1800, "proms", 0)
	ROM_LOAD( "pal16l8.u23", 0x0000, 0x0400, NO_DUMP )
	ROM_LOAD( "pal10l8.u34", 0x0400, 0x0400, NO_DUMP )
ROM_END

} // anonymous namespace


/* System Drivers */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT           COMPANY                       FULLNAME        FLAGS
COMP( 1982, bigbord2, 0,      0,      bigbord2, bigbord2, bigbord2_state, init_bigbord2, "Digital Research Computers", "Big Board II", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
