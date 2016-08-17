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
#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "machine/z80dma.h"
#include "video/mc6845.h"
#include "machine/keyboard.h"
#include "sound/beep.h"
#include "machine/wd_fdc.h"
#include "machine/clock.h"


class bigbord2_state : public driver_device
{
public:
	bigbord2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_ctc1(*this, "ctc1")
		, m_ctc2(*this, "ctc2")
		, m_sio(*this, "sio")
		, m_dma(*this, "dma")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_beeper(*this, "beeper")
		, m_dsw(*this, "DSW")
		, m_bankr(*this, "bankr")
		, m_bankv(*this, "bankv")
		, m_banka(*this, "banka")
	{
	}

	DECLARE_WRITE8_MEMBER(portc0_w );
	DECLARE_WRITE8_MEMBER(portc8_w );
	DECLARE_WRITE8_MEMBER(portcc_w );
	DECLARE_READ8_MEMBER(portc4_r);
	DECLARE_READ8_MEMBER(portd0_r);
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_WRITE_LINE_MEMBER(clock_w);
	DECLARE_WRITE_LINE_MEMBER(busreq_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z1_w);
	DECLARE_WRITE_LINE_MEMBER(sio_wrdya_w);
	DECLARE_WRITE_LINE_MEMBER(sio_wrdyb_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	DECLARE_DRIVER_INIT(bigbord2);
	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_READ8_MEMBER(io_read_byte);
	DECLARE_WRITE8_MEMBER(io_write_byte);
	MC6845_UPDATE_ROW(crtc_update_row);
	required_device<palette_device> m_palette;

private:
	UINT8 crt8002(UINT8 ac_ra, UINT8 ac_chr, UINT8 ac_attr, UINT16 ac_cnt, bool ac_curs);
	UINT8 *m_p_chargen;                 /* character ROM */
	UINT8 *m_p_videoram;                    /* Video RAM */
	UINT8 *m_p_attribram;                   /* Attribute RAM */
	UINT8 m_term_data;
	UINT8 m_term_status;
	UINT16 m_cnt;
	bool m_c8[8];
	bool m_cc[8];
	floppy_image_device *m_floppy;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	address_space *m_mem;
	address_space *m_io;
	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc1;
	required_device<z80ctc_device> m_ctc2;
	required_device<z80sio_device> m_sio;
	required_device<z80dma_device> m_dma;
	required_device<mb8877_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<beep_device> m_beeper;
	required_ioport m_dsw;
	required_memory_bank m_bankr;
	required_memory_bank m_bankv;
	required_memory_bank m_banka;
};

// Eprom programming port
WRITE8_MEMBER( bigbord2_state::portc0_w )
{
}

/* Status port
    0 = RXDA
    1 = RXDB
    2 = MOTOR
    3 = KBDSTB
    4 = DIPSW 1
    5 = DIPSW 2
    6 = DIPSW 3
    7 = DIPSW 4 */

READ8_MEMBER( bigbord2_state::portc4_r )
{
	UINT8 ret = m_term_status | 3 | (m_c8[6]<<2) | m_dsw->read();
	m_term_status = 0;
	return ret;
}

// KBD port - read ascii value of key pressed

READ8_MEMBER( bigbord2_state::portd0_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

WRITE8_MEMBER( bigbord2_state::kbd_put )
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


WRITE8_MEMBER( bigbord2_state::portc8_w )
{
	/*

	    This port uses a 74LS259, which allows individual bits
	    to be switched on and off, while the other bits are
	    unaffected.

	    bit     signal      description

	    0       D_S         memory bank
	    1       SIDSEL      side select
	    2       SMC1        u6 data separator pin 5
	    3       SMC2        u6 data separator pin 6
	    4       DDEN        density
	    5       HLD         head load
	    6       MOTOR       disk motor
	    7       BELL        beeper pulse

	*/

	m_c8[data&7] = BIT(data, 3);

	switch (data&7)
	{
		case 0:
			// memory bank
			m_bankr->set_entry(m_c8[0]);
			m_bankv->set_entry(m_c8[0]);
			m_banka->set_entry(m_c8[0]);
			break;
		case 1:
			// side select
			if (m_floppy)
			{
				m_floppy->ss_w(m_c8[1]);
			}

			break;

		case 2:
		case 3:
			// these connect to "U6 (FDC9216B)" which drives the fdc "rawread" and "rclk" pins
			break;
		case 4:
			// density
			m_fdc->dden_w(m_c8[4]);
			break;
		case 5:
			// connects to HLD pin on floppy drive
			break;
		case 6:
			// motor on
			if (m_floppy)
			{
				m_floppy->mon_w(m_c8[6]? 0 : 1);
			}
			break;
		case 7:
			// beeper
			m_beeper->set_state(m_c8[7]);
			break;
	}
}

WRITE8_MEMBER( bigbord2_state::portcc_w )
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

	m_dma->rdy_w(m_cc[data & 7]);
}



/* Memory Maps */

static ADDRESS_MAP_START( bigbord2_mem, AS_PROGRAM, 8, bigbord2_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_RAMBANK("bankr")
	AM_RANGE(0x1000, 0x5fff) AM_RAM
	AM_RANGE(0x6000, 0x6fff) AM_RAMBANK("bankv")
	AM_RANGE(0x7000, 0x7fff) AM_RAMBANK("banka")
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( bigbord2_io, AS_IO, 8, bigbord2_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x80, 0x83) AM_DEVREADWRITE("sio", z80sio_device, ba_cd_r, ba_cd_w) // u16
	AM_RANGE(0x84, 0x87) AM_DEVREADWRITE("ctc1", z80ctc_device, read, write) // u37 has issues
	AM_RANGE(0x88, 0x8b) AM_DEVREADWRITE("ctc2", z80ctc_device, read, write) // u21
	AM_RANGE(0x8C, 0x8F) AM_DEVREADWRITE("dma", z80dma_device, read, write) // u62
	AM_RANGE(0xC0, 0xC3) AM_WRITE(portc0_w) // eprom programming port
	AM_RANGE(0xC4, 0xC7) AM_READ (portc4_r)
	AM_RANGE(0xC8, 0xCB) AM_WRITE(portc8_w)
	AM_RANGE(0xCC, 0xCF) AM_WRITE(portcc_w)
	AM_RANGE(0xD0, 0xD3) AM_READ (portd0_r)
	AM_RANGE(0xD4, 0xD7) AM_DEVREADWRITE("fdc", mb8877_t, read, write) // u10
	//AM_RANGE(0xD8, 0xDB) AM_READWRITE(portd8_r, portd8_w) // various external data ports; DB = centronics printer
	AM_RANGE(0xDC, 0xDC) AM_MIRROR(2) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w) // u30
	AM_RANGE(0xDD, 0xDD) AM_MIRROR(2) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
ADDRESS_MAP_END


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

static SLOT_INTERFACE_START( bigbord2_floppies )
	SLOT_INTERFACE( "drive0", FLOPPY_8_DSDD )
	SLOT_INTERFACE( "drive1", FLOPPY_8_DSDD )
SLOT_INTERFACE_END


/* Video */

void bigbord2_state::video_start()
{
	/* find memory regions */
	m_p_chargen = memregion("chargen")->base();
	m_p_videoram = memregion("maincpu")->base()+0x6000;
	m_p_attribram = memregion("maincpu")->base()+0x7000;
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
	UINT8 i;
	for (i = 0; i < 8; i++)
	{
		m_c8[i] = 0;
		m_cc[i] = 1;
	}
	m_cc[2] = 0;
	m_beeper->set_state(0);
	m_bankr->set_entry(0);
	m_bankv->set_entry(0);
	m_banka->set_entry(0);
}

DRIVER_INIT_MEMBER(bigbord2_state,bigbord2)
{
	m_mem = &m_maincpu->space(AS_PROGRAM);
	m_io = &m_maincpu->space(AS_IO);
	UINT8 *RAM = memregion("maincpu")->base();
	m_bankr->configure_entries(0, 2, &RAM[0x0000], 0x10000);
	m_bankv->configure_entries(0, 2, &RAM[0x6000], 0x10000);
	m_banka->configure_entries(0, 2, &RAM[0x7000], 0x10000);
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

static GFXDECODE_START( crt8002 )
	GFXDECODE_ENTRY( "chargen", 0x0000, crt8002_charlayout, 0, 1 )
GFXDECODE_END

UINT8 bigbord2_state::crt8002(UINT8 ac_ra, UINT8 ac_chr, UINT8 ac_attr, UINT16 ac_cnt, bool ac_curs)
{
	UINT8 gfx = 0;
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
			gfx = BITSWAP8(ac_chr, 0,1,2,3,4,5,6,7);
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
	UINT8 chr,gfx,attr;
	UINT16 mem,x;
	UINT32 *p = &bitmap.pix32(y);
	ra &= 15;
	m_cnt++;

	for (x = 0; x < x_count; x++)
	{
		mem = (ma + x) & 0x7ff;
		attr = m_p_attribram[mem];
		chr = m_p_videoram[mem];

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

#define MAIN_CLOCK XTAL_8MHz / 2

static MACHINE_CONFIG_START( bigbord2, bigbord2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MAIN_CLOCK)
	MCFG_CPU_PROGRAM_MAP(bigbord2_mem)
	MCFG_CPU_IO_MAP(bigbord2_io)
	MCFG_Z80_DAISY_CHAIN(daisy_chain)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_10_69425MHz, 700, 0, 560, 260, 0, 240)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", crt8002)
	MCFG_PALETTE_ADD_MONOCHROME("palette")

	MCFG_DEVICE_ADD("ctc_clock", CLOCK, MAIN_CLOCK)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(bigbord2_state, clock_w))

	/* devices */
	MCFG_DEVICE_ADD("dma", Z80DMA, MAIN_CLOCK)
	MCFG_Z80DMA_OUT_BUSREQ_CB(WRITELINE(bigbord2_state, busreq_w))
	MCFG_Z80DMA_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80DMA_IN_MREQ_CB(READ8(bigbord2_state, memory_read_byte))
	MCFG_Z80DMA_OUT_MREQ_CB(WRITE8(bigbord2_state, memory_write_byte))
	MCFG_Z80DMA_IN_IORQ_CB(READ8(bigbord2_state, io_read_byte))
	MCFG_Z80DMA_OUT_IORQ_CB(WRITE8(bigbord2_state, io_write_byte))

	MCFG_Z80SIO_ADD("sio", MAIN_CLOCK, 0, 0, 0, 0)
	MCFG_Z80SIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80SIO_OUT_SYNCA_CB(DEVWRITELINE("ctc1", z80ctc_device, trg2))
	MCFG_Z80SIO_OUT_WRDYA_CB(WRITELINE(bigbord2_state, sio_wrdya_w))
	MCFG_Z80SIO_OUT_WRDYB_CB(WRITELINE(bigbord2_state, sio_wrdyb_w))

	MCFG_DEVICE_ADD("ctc1", Z80CTC, MAIN_CLOCK)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("ctc2", Z80CTC, MAIN_CLOCK)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(DEVWRITELINE("sio", z80sio_device, rxtxcb_w))    // to SIO Ch B
	MCFG_Z80CTC_ZC1_CB(WRITELINE(bigbord2_state, ctc_z1_w))  // to SIO Ch A
	MCFG_Z80CTC_ZC2_CB(DEVWRITELINE("ctc2", z80ctc_device, trg3))

	MCFG_MB8877_ADD("fdc", XTAL_16MHz / 8) // 2MHz for 8 inch, or 1MHz otherwise (jumper-selectable)
	//MCFG_WD_FDC_INTRQ_CALLBACK(INPUTLINE("maincpu", ??)) // info missing from schematic
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", bigbord2_floppies, "drive0", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", bigbord2_floppies, "drive1", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	MCFG_MC6845_ADD("crtc", MC6845, "screen", XTAL_16MHz / 8)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(bigbord2_state, crtc_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(DEVWRITELINE("ctc1", z80ctc_device, trg3))

	/* keyboard */
	MCFG_DEVICE_ADD("keyboard", GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(bigbord2_state, kbd_put))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 950) // actual frequency is unknown
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

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT       INIT        COMPANY                      FULLNAME        FLAGS */
COMP( 1982, bigbord2,   bigboard,   0,      bigbord2,   bigbord2, bigbord2_state,   bigbord2, "Digital Research Computers", "Big Board II", MACHINE_NOT_WORKING )
