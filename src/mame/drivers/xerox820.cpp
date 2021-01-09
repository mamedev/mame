// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

        Xerox 820

        12/05/2009 Skeleton driver.

****************************************************************************/

/*

    TODO:

    - Xerox 820
        - floppy format has 3xcd at the end of track data
            :u109: write track 0
            :u109: track description 16xff ... 109xff 3xcd
        - keyboard conflicts with optional serial terminal
    - Xerox 820-II
        - floppy (read/write to FDC triggers Z80 WAIT)
        - Winchester
            - Shugart SA1004 (chs=256,4,40 ss=256)
            - Shugart SA606 (chs=160,6, ss=256)
            - Shugart SA1403D controller
    - Xerox 16/8
    - Emerald Microware X120 board
    - type in Monitor v1.0 from manual
    - ASCII keyboard
    - low-profile keyboard

    http://www.vintagesbc.it/?page_id=233
    http://mccworkshop.com/computers/comphistory7.htm
    http://bitsavers.org/bits/Xerox/820/
    http://bitsavers.org/bits/Xerox/820-II/
    http://www.classiccmp.org/dunfield/img54306/system.htm

    Note:
    - MK-82 have same roms as original Big Board
    - MK-83 have 256K of RAM

    8-inch formats
    77 tracks, 1 head, 26 sectors, 128 bytes sector length, first sector id 1
    77 tracks, 1 head, 26 sectors, 256 bytes sector length, first sector id 1

    5.25-inch formats
    40 tracks, 1 head, 18 sectors, 128 bytes sector length, first sector id 1
    40 tracks, 2 heads, 18 sectors, 128 bytes sector length, first sector id 1

    SmartROM and Plus2 ROM both come for 2.5MHz or 4MHz systems, and there is another distinction between variants for generic or Xerox keyboards
    http://www.microcodeconsulting.com/z80/plus2.htm
    http://www.microcodeconsulting.com/z80/smartrom.htm

*/


#include "emu.h"
#include "includes/xerox820.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"


/* Read/Write Handlers */

void xerox820_state::bankswitch(int bank)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	uint8_t *ram = m_ram->pointer();

	if (bank)
	{
		/* ROM */
		program.install_rom(0x0000, 0x0fff, m_rom->base());
		program.unmap_readwrite(0x1000, 0x1fff);
		program.install_ram(0x3000, 0x3fff, m_video_ram);
	}
	else
	{
		/* RAM */
		program.install_ram(0x0000, 0x3fff, ram);
	}
}

void xerox820ii_state::bankswitch(int bank)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	uint8_t *ram = m_ram->pointer();

	if (bank)
	{
		/* ROM */
		program.install_rom(0x0000, 0x1fff, m_rom->base());
		program.unmap_readwrite(0x2000, 0x2fff);
		program.install_ram(0x3000, 0x3fff, m_video_ram);
		program.unmap_readwrite(0x4000, 0xbfff);
	}
	else
	{
		/* RAM */
		program.install_ram(0x0000, 0xbfff, ram);
	}
}

uint8_t xerox820_state::fdc_r(offs_t offset)
{
	return m_fdc->read(offset) ^ 0xff;
}

void xerox820_state::fdc_w(offs_t offset, uint8_t data)
{
	m_fdc->write(offset, data ^ 0xff);
}

void xerox820_state::scroll_w(offs_t offset, uint8_t data)
{
	m_scroll = (offset >> 8) & 0x1f;
}

#ifdef UNUSED_CODE
void xerox820_state::x120_system_w(uint8_t data)
{
	/*

	    bit     signal      description

	    0       DSEL0       drive select bit 0 (01=A, 10=B, 00=C, 11=D)
	    1       DSEL1       drive select bit 1
	    2       SIDE        side select
	    3       VATT        video attribute (0=inverse, 1=blinking)
	    4       BELL        bell trigger
	    5       DENSITY     density (0=double, 1=single)
	    6       _MOTOR      disk motor (0=on, 1=off)
	    7       BANK        memory bank switch (0=RAM, 1=ROM/video)

	*/
}
#endif

void xerox820ii_state::bell_w(offs_t offset, uint8_t data)
{
	m_speaker->level_w(offset);
}

void xerox820ii_state::slden_w(offs_t offset, uint8_t data)
{
	m_fdc->dden_w(offset);
}

void xerox820ii_state::chrom_w(offs_t offset, uint8_t data)
{
	m_chrom = offset;
}

void xerox820ii_state::lowlite_w(uint8_t data)
{
	m_lowlite = data;
}

void xerox820ii_state::sync_w(offs_t offset, uint8_t data)
{
	if (offset)
	{
		/* set external clocks for synchronous sio A */
	}
	else
	{
		/* set internal clocks for asynchronous sio A */
	}
}

/* Memory Maps */

void xerox820_state::xerox820_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x3000, 0x3fff).ram().share("video_ram");
	map(0x4000, 0xffff).ram();
}

void xerox820_state::xerox820_io(address_map &map)
{
	map(0x00, 0x00).mirror(0xff03).w(COM8116_TAG, FUNC(com8116_device::str_w));
	map(0x04, 0x07).mirror(0xff00).rw(m_sio, FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0x08, 0x0b).mirror(0xff00).rw(Z80PIO_GP_TAG, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x0c, 0x0c).mirror(0xff03).w(COM8116_TAG, FUNC(com8116_device::stt_w));
	map(0x10, 0x13).mirror(0xff00).rw(FUNC(xerox820_state::fdc_r), FUNC(xerox820_state::fdc_w));
	map(0x14, 0x14).mirror(0x0003).select(0xff00).w(FUNC(xerox820_state::scroll_w));
	map(0x18, 0x1b).mirror(0xff00).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x1c, 0x1f).mirror(0xff00).rw(m_kbpio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
}

void xerox820ii_state::xerox820ii_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x3000, 0x3fff).ram().share("video_ram");
	map(0xc000, 0xffff).ram();
}

void xerox820ii_state::xerox820ii_io(address_map &map)
{
	xerox820_io(map);
	map(0x28, 0x29).mirror(0xff00).w(FUNC(xerox820ii_state::bell_w));
	map(0x30, 0x31).mirror(0xff00).w(FUNC(xerox820ii_state::slden_w));
	map(0x34, 0x35).mirror(0xff00).w(FUNC(xerox820ii_state::chrom_w));
	map(0x36, 0x36).mirror(0xff00).w(FUNC(xerox820ii_state::lowlite_w));
	map(0x68, 0x69).mirror(0xff00).w(FUNC(xerox820ii_state::sync_w));
}

void xerox820ii_state::xerox168_mem(address_map &map)
{
	map(0x00000, 0x3ffff).ram();
	map(0xff000, 0xfffff).rom().region(I8086_TAG, 0);
}

void xerox820_state::mk83_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x3000, 0x6fff).ram();
	map(0x7000, 0x7fff).ram().share("video_ram");
	map(0x8000, 0xffff).ram();
}


/* Input Ports */

static INPUT_PORTS_START( xerox820 )
	// inputs defined in machine/keyboard.c
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER(bigboard_state::beep_timer)
{
	m_beeper->set_state(0);
}

/* Z80 PIO */

uint8_t xerox820_state::kbpio_pa_r()
{
	/*

	    bit     signal          description

	    0
	    1
	    2
	    3       PBRDY           keyboard data available
	    4       8/N5            8"/5.25" disk select (0=5.25", 1=8")
	    5       400/460         double sided disk detect (only on Etch 2 PCB) (0=SS, 1=DS)
	    6
	    7

	*/

	uint8_t data = 0;

	// keyboard
	data |= m_kbpio->rdy_b() << 3;

	// floppy
	data |= m_8n5 << 4;
	data |= m_400_460 << 5;

	return data;
}

void xerox820_state::kbpio_pa_w(uint8_t data)
{
	/*

	    bit     signal          description

	    0       _DVSEL1         drive select 1
	    1       _DVSEL2         drive select 2
	    2       SIDE            side select
	    3
	    4
	    5
	    6       NCSET2          display character set (inverted and connected to chargen A10)
	    7       BANK            bank switching (0=RAM, 1=ROM/videoram)

	*/

	/* drive select */
	floppy_image_device *floppy = nullptr;

	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	if (BIT(data, 1)) floppy = m_floppy1->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		int _8n5 = (floppy->get_form_factor() == floppy_image::FF_8);

		if (m_8n5 != _8n5)
		{
			m_8n5 = _8n5;

			m_fdc->set_unscaled_clock(m_8n5 ? 20_MHz_XTAL / 10 : 20_MHz_XTAL / 20);
		}

		m_400_460 = !floppy->twosid_r();

		floppy->mon_w(0);

		floppy->ss_w(BIT(data, 2));
	}

	/* display character set */
	m_ncset2 = !BIT(data, 6);

	/* bank switching */
	bankswitch(BIT(data, 7));
}

void bigboard_state::kbpio_pa_w(uint8_t data)
{
	xerox820_state::kbpio_pa_w(data);

	/* beeper on bigboard */
	if (BIT(data, 5) & (!m_bit5))
	{
		m_beep_timer->adjust(attotime::from_msec(40));
		m_beeper->set_state(1);
	}
	m_bit5 = BIT(data, 5);
}

uint8_t xerox820_state::kbpio_pb_r()
{
	/*

	    bit     description

	    0       KB0
	    1       KB1
	    2       KB2
	    3       KB3
	    4       KB4
	    5       KB5
	    6       KB6
	    7       KB7

	*/

	return m_kb->read() ^ 0xff;
}

void xerox820ii_state::rdpio_pb_w(uint8_t data)
{
	/*

	    bit     description

	    0       NBSY
	    1       NMSG
	    2       NC/D
	    3       NREQ
	    4       NI/O
	    5       NSEL
	    6       LS74 Q
	    7       NRST
	*/

	m_sasibus->write_sel(!BIT(data, 5));
	m_sasibus->write_rst(!BIT(data, 7));
	// TODO: LS74 Q
}

WRITE_LINE_MEMBER( xerox820ii_state::rdpio_pardy_w )
{
	// TODO
}

/* Z80 CTC */

TIMER_DEVICE_CALLBACK_MEMBER( xerox820_state::ctc_tick )
{
	m_ctc->trg0(1);
	m_ctc->trg0(0);
}

/* Z80 Daisy Chain */

static const z80_daisy_config xerox820_daisy_chain[] =
{
	{ Z80SIO_TAG },
	{ Z80PIO_KB_TAG },
	{ Z80PIO_GP_TAG },
	{ Z80CTC_TAG },
	{ nullptr }
};



/***********************************************************

    Quickload

    This loads a .COM file to address 0x100 then jumps
    there. Sometimes .COM has been renamed to .CPM to
    prevent windows going ballistic. These can be loaded
    as well.

************************************************************/

QUICKLOAD_LOAD_MEMBER(xerox820_state::quickload_cb)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);

	if (image.length() >= 0xfd00)
		return image_init_result::FAIL;

	bankswitch(0);

	/* Avoid loading a program if CP/M-80 is not in memory */
	if ((prog_space.read_byte(0) != 0xc3) || (prog_space.read_byte(5) != 0xc3))
	{
		machine_reset();
		return image_init_result::FAIL;
	}

	/* Load image to the TPA (Transient Program Area) */
	uint16_t quickload_size = image.length();
	for (uint16_t i = 0; i < quickload_size; i++)
	{
		uint8_t data;
		if (image.fread( &data, 1) != 1)
			return image_init_result::FAIL;
		prog_space.write_byte(i+0x100, data);
	}

	/* clear out command tail */
	prog_space.write_byte(0x80, 0);   prog_space.write_byte(0x81, 0);

	/* Roughly set SP basing on the BDOS position */
	m_maincpu->set_state_int(Z80_SP, 256 * prog_space.read_byte(7) - 300);
	m_maincpu->set_pc(0x100);   // start program

	return image_init_result::PASS;
}



/* WD1771 Interface */

static void xerox820_floppies(device_slot_interface &device)
{
	device.option_add("sa400", FLOPPY_525_SSSD_35T); // Shugart SA-400, 35 trk drive
	device.option_add("sa400l", FLOPPY_525_SSSD); // Shugart SA-400, 40 trk drive
	device.option_add("sa450", FLOPPY_525_DD); // Shugart SA-450
	device.option_add("sa800", FLOPPY_8_SSDD); // Shugart SA-800
	device.option_add("sa850", FLOPPY_8_DSDD); // Shugart SA-850
}

void xerox820_state::update_nmi()
{
	int halt = m_maincpu->state_int(Z80_HALT);
	int state = (halt && (m_fdc_irq || m_fdc_drq)) ? ASSERT_LINE : CLEAR_LINE;

	m_maincpu->set_input_line(INPUT_LINE_NMI, state);
}

WRITE_LINE_MEMBER( xerox820_state::fdc_intrq_w )
{
	m_fdc_irq = state;

	update_nmi();
}

WRITE_LINE_MEMBER( xerox820_state::fdc_drq_w )
{
	m_fdc_drq = state;

	update_nmi();
}

/* Video */

uint32_t xerox820_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t sy=0,ma=(m_scroll + 1) * 0x80;
	pen_t const *const pen=m_palette->pens();

	m_framecnt++;

	for (uint8_t y = 0; y < 24; y++)
	{
		if (ma > 0xb80) ma = 0;

		for (uint8_t ra = 0; ra < 10; ra++)
		{
			uint32_t *p = &bitmap.pix(sy++);

			for (uint16_t x = ma; x < ma + 80; x++)
			{
				uint8_t gfx;
				if (ra < 8)
				{
					uint8_t chr = m_video_ram[x & XEROX820_VIDEORAM_MASK] ^ 0x80;

					/* Take care of flashing characters */
					if ((chr < 0x80) && (m_framecnt & 0x08))
						chr |= 0x80;

					/* get pattern of pixels for that character scanline */
					gfx = m_char_rom->base()[(m_ncset2 << 10) | (chr<<3) | ra ];
				}
				else
					gfx = 0xff;

				/* Display a scanline of a character (7 pixels) */
				*p++ = pen[0];
				*p++ = pen[BIT(gfx, 4) ^ 1];
				*p++ = pen[BIT(gfx, 3) ^ 1];
				*p++ = pen[BIT(gfx, 2) ^ 1];
				*p++ = pen[BIT(gfx, 1) ^ 1];
				*p++ = pen[BIT(gfx, 0) ^ 1];
				*p++ = pen[0];
			}
		}
		ma+=128;
	}
	return 0;
}

/* Machine Initialization */

void xerox820_state::machine_start()
{
	// state saving
	save_item(NAME(m_scroll));
	save_item(NAME(m_ncset2));
	save_item(NAME(m_vatt));
	save_item(NAME(m_fdc_irq));
	save_item(NAME(m_fdc_drq));
	save_item(NAME(m_8n5));
	save_item(NAME(m_400_460));

	m_ncset2 = 0;
}

void xerox820_state::machine_reset()
{
	bankswitch(1);

	m_fdc->reset();
}

void bigboard_state::machine_reset()
{
	bankswitch(1);

	/* bigboard has a one-pulse output to drive a user-supplied beeper */
	m_beeper->set_state(0);

	m_fdc->reset();
}

void xerox820ii_state::machine_reset()
{
	bankswitch(1);

	m_fdc->reset();

	m_sio->synca_w(1);
	m_sio->syncb_w(1);
}


/* F4 Character Displayer */
static const gfx_layout xerox820_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8, 8*8,  9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*8                 /* every char takes 8 bytes */
};

static const gfx_layout xerox820_gfxlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8, 8*8,  9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_xerox820 )
	GFXDECODE_ENTRY( "chargen", 0x0000, xerox820_charlayout, 0, 1 )
GFXDECODE_END

static GFXDECODE_START( gfx_xerox820ii )
	GFXDECODE_ENTRY( "chargen", 0x0000, xerox820_charlayout, 0, 1 )
	GFXDECODE_ENTRY( "chargen", 0x0800, xerox820_gfxlayout, 0, 1 )
GFXDECODE_END

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_300 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_300 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_ODD )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

/* Machine Drivers */

void xerox820_state::xerox820(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 20_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &xerox820_state::xerox820_mem);
	m_maincpu->set_addrmap(AS_IO, &xerox820_state::xerox820_io);
	m_maincpu->set_daisy_config(xerox820_daisy_chain);

	/* video hardware */
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(xerox820_state::screen_update));
	screen.set_raw(10.69425_MHz_XTAL, 700, 0, 560, 260, 0, 240);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_xerox820);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* devices */
	Z80PIO(config, m_kbpio, 20_MHz_XTAL / 8);
	m_kbpio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_kbpio->in_pa_callback().set(FUNC(xerox820_state::kbpio_pa_r));
	m_kbpio->out_pa_callback().set(FUNC(xerox820_state::kbpio_pa_w));
	m_kbpio->in_pb_callback().set(FUNC(xerox820_state::kbpio_pb_r));

	z80pio_device& pio_gp(Z80PIO(config, Z80PIO_GP_TAG, 20_MHz_XTAL / 8));
	pio_gp.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80CTC(config, m_ctc, 20_MHz_XTAL / 8);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(m_ctc, FUNC(z80ctc_device::trg1));
	m_ctc->zc_callback<2>().set(m_ctc, FUNC(z80ctc_device::trg3));
	//TIMER(config, "ctc").configure_periodic(FUNC(xerox820_state::ctc_tick), attotime::from_hz(20_MHz_XTAL / 8));

	FD1771(config, m_fdc, 20_MHz_XTAL / 20);
	m_fdc->intrq_wr_callback().set(FUNC(xerox820_state::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(xerox820_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, FD1771_TAG":0", xerox820_floppies, "sa400l", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, FD1771_TAG":1", xerox820_floppies, "sa400l", floppy_image_device::default_floppy_formats);

	Z80SIO(config, m_sio, 20_MHz_XTAL / 8); // MK3884 (SIO/0)
	m_sio->out_txda_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_txd));
	m_sio->out_dtra_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsa_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_rts));
	m_sio->out_txdb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_txd));
	m_sio->out_dtrb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_rts));
	m_sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	rs232_port_device &rs232a(RS232_PORT(config, RS232_A_TAG, default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_sio, FUNC(z80sio_device::rxa_w));
	rs232a.rxd_handler().append(m_sio, FUNC(z80sio_device::synca_w));
	rs232a.cts_handler().set(m_sio, FUNC(z80sio_device::ctsa_w));
	rs232a.dcd_handler().set(m_sio, FUNC(z80sio_device::dcda_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232_B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_sio, FUNC(z80sio_device::rxb_w));
	rs232b.rxd_handler().append(m_sio, FUNC(z80sio_device::syncb_w));
	rs232b.cts_handler().set(m_sio, FUNC(z80sio_device::ctsb_w));
	rs232b.dcd_handler().set(m_sio, FUNC(z80sio_device::dcdb_w));
	rs232b.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	com8116_device &dbrg(COM8116(config, COM8116_TAG, 5.0688_MHz_XTAL));
	dbrg.fr_handler().set(m_sio, FUNC(z80sio_device::rxca_w));
	dbrg.fr_handler().append(m_sio, FUNC(z80sio_device::txca_w));
	dbrg.ft_handler().set(m_sio, FUNC(z80sio_device::rxtxcb_w));

	XEROX_820_KEYBOARD(config, m_kb, 0);
	m_kb->kbstb_wr_callback().set(m_kbpio, FUNC(z80pio_device::strobe_b));

	/* internal ram */
	RAM(config, m_ram).set_default_size("64K");

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("xerox820");
	QUICKLOAD(config, "quickload", "com,cpm", attotime::from_seconds(3)).set_load_callback(FUNC(xerox820_state::quickload_cb));
}

void bigboard_state::bigboard(machine_config &config)
{
	xerox820(config);
	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 950).add_route(ALL_OUTPUTS, "mono", 1.00); /* bigboard only */
	TIMER(config, m_beep_timer).configure_generic(FUNC(bigboard_state::beep_timer));
}

void xerox820ii_state::xerox820ii(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &xerox820ii_state::xerox820ii_mem);
	m_maincpu->set_addrmap(AS_IO, &xerox820ii_state::xerox820ii_io);
	m_maincpu->set_daisy_config(xerox820_daisy_chain);

	/* video hardware */
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(xerox820ii_state::screen_update));
	screen.set_raw(10.69425_MHz_XTAL, 700, 0, 560, 260, 0, 240);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_xerox820ii);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* devices */
	Z80PIO(config, m_kbpio, 16_MHz_XTAL / 4);
	m_kbpio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_kbpio->in_pa_callback().set(FUNC(xerox820_state::kbpio_pa_r));
	m_kbpio->out_pa_callback().set(FUNC(xerox820_state::kbpio_pa_w));
	m_kbpio->in_pb_callback().set(FUNC(xerox820_state::kbpio_pb_r));

	z80pio_device& pio_gp(Z80PIO(config, Z80PIO_GP_TAG, 16_MHz_XTAL / 4));
	pio_gp.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80pio_device& pio_rd(Z80PIO(config, Z80PIO_RD_TAG, 20_MHz_XTAL / 8));
	pio_rd.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	pio_rd.in_pa_callback().set("sasi_data_in", FUNC(input_buffer_device::read));
	pio_rd.out_pa_callback().set("sasi_data_out", FUNC(output_latch_device::write));
	pio_rd.out_ardy_callback().set(FUNC(xerox820ii_state::rdpio_pardy_w));
	pio_rd.in_pb_callback().set("sasi_ctrl_in", FUNC(input_buffer_device::read));
	pio_rd.out_pb_callback().set(FUNC(xerox820ii_state::rdpio_pb_w));

	Z80CTC(config, m_ctc, 16_MHz_XTAL / 4);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(m_ctc, FUNC(z80ctc_device::trg1));
	m_ctc->zc_callback<2>().set(m_ctc, FUNC(z80ctc_device::trg3));
	//TIMER(config, "ctc").configure_periodic(FUNC(xerox820_state::ctc_tick), attotime::from_hz(16_MHz_XTAL / 4));

	FD1797(config, m_fdc, 16_MHz_XTAL / 8);
	m_fdc->intrq_wr_callback().set(FUNC(xerox820_state::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(xerox820_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, FD1797_TAG":0", xerox820_floppies, "sa450", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, FD1797_TAG":1", xerox820_floppies, "sa450", floppy_image_device::default_floppy_formats);

	Z80SIO(config, m_sio, 16_MHz_XTAL / 4); // MK3884 (SIO/0)
	m_sio->out_txda_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_txd));
	m_sio->out_dtra_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsa_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_rts));
	m_sio->out_txdb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_txd));
	m_sio->out_dtrb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_rts));
	m_sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	rs232_port_device &rs232a(RS232_PORT(config, RS232_A_TAG, default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_sio, FUNC(z80sio_device::rxa_w));
	rs232a.cts_handler().set(m_sio, FUNC(z80sio_device::ctsa_w));
	rs232a.dcd_handler().set(m_sio, FUNC(z80sio_device::dcda_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232_B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_sio, FUNC(z80sio_device::rxb_w));
	rs232b.cts_handler().set(m_sio, FUNC(z80sio_device::ctsb_w));
	rs232b.dcd_handler().set(m_sio, FUNC(z80sio_device::dcdb_w));

	com8116_device &dbrg(COM8116(config, COM8116_TAG, 5.0688_MHz_XTAL));
	dbrg.fr_handler().set(m_sio, FUNC(z80sio_device::rxca_w));
	dbrg.fr_handler().append(m_sio, FUNC(z80sio_device::txca_w));
	dbrg.ft_handler().set(m_sio, FUNC(z80sio_device::rxtxcb_w));

	XEROX_820_KEYBOARD(config, m_kb, 0);
	m_kb->kbstb_wr_callback().set(m_kbpio, FUNC(z80pio_device::strobe_b));

	// SASI bus
	SCSI_PORT(config, m_sasibus, 0);
	m_sasibus->set_data_input_buffer("sasi_data_in");
	m_sasibus->bsy_handler().set("sasi_ctrl_in", FUNC(input_buffer_device::write_bit0)).exor(1);
	m_sasibus->msg_handler().set("sasi_ctrl_in", FUNC(input_buffer_device::write_bit1)).exor(1);
	m_sasibus->cd_handler().set("sasi_ctrl_in", FUNC(input_buffer_device::write_bit2)).exor(1);
	m_sasibus->req_handler().set("sasi_ctrl_in", FUNC(input_buffer_device::write_bit3)).exor(1);
	m_sasibus->io_handler().set("sasi_ctrl_in", FUNC(input_buffer_device::write_bit4)).exor(1);
	m_sasibus->set_slot_device(1, "harddisk", SA1403D, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_0));

	output_latch_device &sasi_data_out(OUTPUT_LATCH(config, "sasi_data_out"));
	m_sasibus->set_output_latch(sasi_data_out);
	INPUT_BUFFER(config, "sasi_data_in");
	INPUT_BUFFER(config, "sasi_ctrl_in");

	/* internal ram */
	RAM(config, m_ram).set_default_size("64K");

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("xerox820ii");
	QUICKLOAD(config, "quickload", "com,cpm", attotime::from_seconds(3)).set_load_callback(FUNC(xerox820_state::quickload_cb));
}

void xerox820ii_state::xerox168(machine_config &config)
{
	xerox820ii(config);
	i8086_cpu_device &i8086(I8086(config, I8086_TAG, 4770000));
	i8086.set_addrmap(AS_PROGRAM, &xerox820ii_state::xerox168_mem);

	/* internal ram */
	m_ram->set_default_size("192K").set_extra_options("320K");
}

void xerox820_state::mk83(machine_config & config)
{
	xerox820(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &xerox820_state::mk83_mem);
}

/* ROMs */

ROM_START( bigboard )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_LOAD( "bigboard.u67", 0x0000, 0x0800, CRC(5a85a228) SHA1(d51a2cbd0aae80315bda9530275aabfe8305364e) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "bigboard.u73", 0x0000, 0x0800, CRC(10bf0d81) SHA1(7ec73670a4d9d6421a5d6a4c4edc8b7c87923f6c) )
ROM_END

#define rom_mk82 rom_bigboard

ROM_START( x820 )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS( "v20" )
	ROM_SYSTEM_BIOS( 0, "v10", "Xerox Monitor v1.0" )
	ROMX_LOAD( "x820v10.u64", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(0) )
	ROMX_LOAD( "x820v10.u63", 0x0800, 0x0800, NO_DUMP, ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v20", "Xerox Monitor v2.0" )
	ROMX_LOAD( "x820v20.u64", 0x0000, 0x0800, CRC(2fc227e2) SHA1(b4ea0ae23d281a687956e8a514cb364a1372678e), ROM_BIOS(1) )
	ROMX_LOAD( "x820v20.u63", 0x0800, 0x0800, CRC(bc11f834) SHA1(4fd2b209a6e6ff9b0c41800eb5228c34a0d7f7ef), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "smart23", "MICROCode SmartROM v2.3" )
	ROMX_LOAD( "mxkx25a.u64", 0x0000, 0x0800, CRC(7ec5f100) SHA1(5d0ff35a51aa18afc0d9c20ef99ff5d9d3f2075b), ROM_BIOS(2) )
	ROMX_LOAD( "mxkx25b.u63", 0x0800, 0x0800, CRC(a7543798) SHA1(886e617e1003d13f86f33085cbd49391b77291a3), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "plus2", "MICROCode Plus2 v0.2a" )
	ROMX_LOAD( "p2x25a.u64",  0x0000, 0x0800, CRC(3ccd7a8f) SHA1(6e46c88f03fc7289595dd6bec95e23bb13969525), ROM_BIOS(3) )
	ROMX_LOAD( "p2x25b.u63",  0x0800, 0x0800, CRC(1e580391) SHA1(e91f8ce82586df33c0d6d02eb005e8079f4de67d), ROM_BIOS(3) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "x820.u92", 0x0000, 0x0800, CRC(b823fa98) SHA1(ad0ea346aa257a53ad5701f4201896a2b3a0f928) )
ROM_END

ROM_START( x820ii )
	ROM_REGION( 0x2000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS( "v404" )

	ROM_SYSTEM_BIOS( 0, "v400", "Balcones Operating System v4.00" ) // Initial U.S. 3-ROM set: support for double density disks
	ROMX_LOAD( "v400.u33", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(0) )
	ROMX_LOAD( "v400.u34", 0x0800, 0x0800, NO_DUMP, ROM_BIOS(0) )
	ROMX_LOAD( "v400.u35", 0x1000, 0x0800, NO_DUMP, ROM_BIOS(0) )

	ROM_SYSTEM_BIOS( 1, "v401", "Balcones Operating System v4.01" ) // Corrected overflow problem with large data files
	ROMX_LOAD( "v401.u33", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "v401.u34", 0x0800, 0x0800, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "v401.u35", 0x1000, 0x0800, NO_DUMP, ROM_BIOS(1) )

	ROM_SYSTEM_BIOS( 2, "v402", "Balcones Operating System v4.02" ) // Rank Xerox (European) boot ROM version of US 4.01
	ROMX_LOAD( "u33.4.02.rom", 0x0000, 0x0800, CRC(d9eb668e) SHA1(6acbef96e4e6526c58e068b7849fb9cce2ea2a10), ROM_BIOS(2) )
	ROMX_LOAD( "u34.4.02.rom", 0x0800, 0x0800, CRC(62181209) SHA1(2238aec096d19af9307bb294532f66f53dd7dfc3), ROM_BIOS(2) )
	ROMX_LOAD( "u35.4.02.rom", 0x1000, 0x0800, CRC(e22fbf6d) SHA1(6c162f79d42611176b0f1c0e8a4eeb07492beca1), ROM_BIOS(2) )
	ROMX_LOAD( "u36.rx11.4.02.rom", 0x1800, 0x0800, CRC(b6a239ce) SHA1(330d28fa8ec006d48d948b1c5e714ffced88fe90), ROM_BIOS(2) ) // supports low-profile keyboard, no input with the ROM present

	ROM_SYSTEM_BIOS( 3, "v403", "Balcones Operating System v4.03" ) // Incorporate programmable communications option and support for the low-profile keyboard (4-ROM set and type-ahead input buffer)
	ROMX_LOAD( "v403.u33", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(3) )
	ROMX_LOAD( "v403.u34", 0x0800, 0x0800, NO_DUMP, ROM_BIOS(3) )
	ROMX_LOAD( "v403.u35", 0x1000, 0x0800, NO_DUMP, ROM_BIOS(3) )
	ROMX_LOAD( "v403.u36", 0x1800, 0x0800, NO_DUMP, ROM_BIOS(3) )

	ROM_SYSTEM_BIOS( 4, "v404", "Balcones Operating System v4.04" ) // Changes sign-on message from Xerox 820-II to Xerox
	ROMX_LOAD( "537p3652.u33", 0x0000, 0x0800, CRC(7807cfbb) SHA1(bd3cc5cc5c59c84a50747aae5c17eb4617b0dbc3), ROM_BIOS(4) )
	ROMX_LOAD( "537p3653.u34", 0x0800, 0x0800, CRC(a9c6c0c3) SHA1(c2da9d1bf0da96e6b8bfa722783e411d2fe6deb9), ROM_BIOS(4) )
	ROMX_LOAD( "537p3654.u35", 0x1000, 0x0800, CRC(a8a07223) SHA1(e8ae1ebf2d7caf76771205f577b88ae493836ac9), ROM_BIOS(4) )
	ROMX_LOAD( "v404.u36", 0x1800, 0x0800, NO_DUMP, ROM_BIOS(4) ) // fitted for low-profile keyboard only

	ROM_SYSTEM_BIOS( 5, "v50", "Balcones Operating System v5.0" ) // Operating system modifications for DEM and new 5.25" disk controller (4 new boot ROMs)
	ROMX_LOAD( "u33.5.0_537p10828.bin", 0x0000, 0x0800, CRC(a17af0f1) SHA1(b1d9a151ed4558f49b3cdc1adbf348b54da48877), ROM_BIOS(5) )
	ROMX_LOAD( "u34.5.0_537p10829.bin", 0x0800, 0x0800, CRC(c9f5182e) SHA1(ac830848614cea984c849a42687ea2944d6765d9), ROM_BIOS(5) )
	ROMX_LOAD( "u35.5.0_537p10830.bin", 0x1000, 0x0800, CRC(278fa75f) SHA1(f47cf9eb30366211280f93a8460523fcc53eebe9), ROM_BIOS(5) )
	ROMX_LOAD( "v500.u36", 0x1800, 0x0800, NO_DUMP, ROM_BIOS(5) )

	ROM_SYSTEM_BIOS( 6, "v50v018", "Balcones Operating System v5.0 v018" ) // shows ROM ERROR
	ROMX_LOAD( "537p10828.u33.5.0.bin", 0x0000, 0x0800, CRC(a17af0f1) SHA1(b1d9a151ed4558f49b3cdc1adbf348b54da48877), ROM_BIOS(6) )
	ROMX_LOAD( "537p10829.u34.5.0.bin", 0x0800, 0x0800, CRC(c9f5182e) SHA1(ac830848614cea984c849a42687ea2944d6765d9), ROM_BIOS(6) )
	ROMX_LOAD( "537p10830.u35.5.0.bin", 0x1000, 0x0800, BAD_DUMP CRC(cc4e1c2b) SHA1(375bbed76d9088dec82b9599cd810727d3e605f3), ROM_BIOS(6) )
	ROMX_LOAD( "537p10831.u36.5.0.bin", 0x1800, 0x0800, CRC(cda7f598) SHA1(08ffd18959e1708136076c82486b8d121a04fa23), ROM_BIOS(6) )

	ROM_REGION( 0x1000, "chargen", 0 )

	ROMX_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9), ROM_BIOS(0) )
	ROMX_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3), ROM_BIOS(0) )

	ROMX_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9), ROM_BIOS(1) )
	ROMX_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3), ROM_BIOS(1) )

	ROMX_LOAD( "u57.04.north.rom", 0x0000, 0x0800, CRC(eda727a2) SHA1(292cd8a0dc6699c3a2091b20c0fc63d97a266fbf), ROM_BIOS(2) )
	ROMX_LOAD( "u58.03.north.rom", 0x0800, 0x0800, CRC(a2e514f3) SHA1(8ac22dd0cf0324a857718adf67b41912864893a3), ROM_BIOS(2)  )

	ROMX_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9), ROM_BIOS(3) )
	ROMX_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3), ROM_BIOS(3) )

	ROMX_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9), ROM_BIOS(4) )
	ROMX_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3), ROM_BIOS(4) )

	ROMX_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9), ROM_BIOS(5) )
	ROMX_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3), ROM_BIOS(5) )

	ROMX_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9), ROM_BIOS(6) )
	ROMX_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3), ROM_BIOS(6) )

ROM_END

ROM_START( x168 )
	ROM_REGION( 0x2000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS( "v404" )
	ROM_SYSTEM_BIOS( 0, "v404", "Balcones Operating System v4.04" ) // Changes sign-on message from Xerox 820-II to Xerox
	ROMX_LOAD( "537p3652.u33", 0x0000, 0x0800, CRC(7807cfbb) SHA1(bd3cc5cc5c59c84a50747aae5c17eb4617b0dbc3), ROM_BIOS(0) )
	ROMX_LOAD( "537p3653.u34", 0x0800, 0x0800, CRC(a9c6c0c3) SHA1(c2da9d1bf0da96e6b8bfa722783e411d2fe6deb9), ROM_BIOS(0) )
	ROMX_LOAD( "537p3654.u35", 0x1000, 0x0800, CRC(a8a07223) SHA1(e8ae1ebf2d7caf76771205f577b88ae493836ac9), ROM_BIOS(0) )
	ROMX_LOAD( "v404.u36", 0x1800, 0x0800, NO_DUMP, ROM_BIOS(0) ) // fitted for low-profile keyboard only

	ROM_SYSTEM_BIOS( 1, "v50", "Balcones Operating System v5.0" ) // Operating system modifications for DEM and new 5.25" disk controller (4 new boot ROMs)
	ROMX_LOAD( "l5.u33.rom", 0x0000, 0x0800, CRC(a17af0f1) SHA1(b1d9a151ed4558f49b3cdc1adbf348b54da48877), ROM_BIOS(1) )
	ROMX_LOAD( "l5.u34.rom", 0x0800, 0x0800, CRC(c9f5182e) SHA1(ac830848614cea984c849a42687ea2944d6765d9), ROM_BIOS(1) )
	ROMX_LOAD( "l5.u35.rom", 0x1000, 0x0800, BAD_DUMP CRC(44c8dbf8) SHA1(cba925b425a7a5ca68dc9fed10ea33e100704bf4), ROM_BIOS(1) )   // shows ROM ERROR and is different from Xerox 820-II v50
	ROMX_LOAD( "u36.rx024.rom", 0x1800, 0x0800, CRC(a7f1d677) SHA1(8c2a442f3a691f2e181a640d65f767ce3b51d711), ROM_BIOS(1) ) // fitted for low-profile keyboard only

	ROM_REGION( 0x1000, I8086_TAG, 0 )
	ROM_LOAD( "8086.u33", 0x0000, 0x1000, CRC(ee49e3dc) SHA1(a5f20c74fc53f9d695d8894534ab69a39e2c38d8) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROMX_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9), ROM_BIOS(0) )
	ROMX_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3), ROM_BIOS(0) )

	ROMX_LOAD( "u57.04.north.rom", 0x0000, 0x0800, CRC(eda727a2) SHA1(292cd8a0dc6699c3a2091b20c0fc63d97a266fbf), ROM_BIOS(1) )
	ROMX_LOAD( "u58.03.north.rom", 0x0800, 0x0800, CRC(a2e514f3) SHA1(8ac22dd0cf0324a857718adf67b41912864893a3), ROM_BIOS(1)  )
ROM_END

ROM_START( mk83 )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_LOAD( "2732mk83.bin", 0x0000, 0x1000, CRC(a845c7e1) SHA1(3ccf629c5cd384953794ac4a1d2b45678bd40e92) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "2716mk83.bin", 0x0000, 0x0800, CRC(10bf0d81) SHA1(7ec73670a4d9d6421a5d6a4c4edc8b7c87923f6c) )
ROM_END

/* System Drivers */

//    YEAR  NAME      PARENT    COMPAT  MACHINE     INPUT     CLASS             INIT        COMPANY                       FULLNAME        FLAGS
COMP( 1980, bigboard, 0,        0,      bigboard,   xerox820, bigboard_state,   empty_init, "Digital Research Computers", "Big Board",    0 )
COMP( 1981, x820,     bigboard, 0,      xerox820,   xerox820, xerox820_state,   empty_init, "Xerox",                      "Xerox 820",    MACHINE_NO_SOUND_HW )
COMP( 1982, mk82,     bigboard, 0,      bigboard,   xerox820, bigboard_state,   empty_init, "Scomar",                     "MK-82",        0 )
COMP( 1983, x820ii,   0,        0,      xerox820ii, xerox820, xerox820ii_state, empty_init, "Xerox",                      "Xerox 820-II", MACHINE_NOT_WORKING )
COMP( 1983, x168,     x820ii,   0,      xerox168,   xerox820, xerox820ii_state, empty_init, "Xerox",                      "Xerox 16/8",   MACHINE_NOT_WORKING )
COMP( 1983, mk83,     bigboard, 0,      mk83,       xerox820, xerox820_state,   empty_init, "Scomar",                     "MK-83",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
