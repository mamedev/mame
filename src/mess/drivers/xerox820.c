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

    http://users.telenet.be/lust/Xerox820/index.htm
    http://www.classiccmp.org/dunfield/img41867/system.htm
    http://www.microcodeconsulting.com/z80/plus2.htm

    Note:
    - MK-82 have same roms as original Big Board
    - MK-83 have 256K of RAM

    8-inch formats
    77 tracks, 1 head, 26 sectors, 128 bytes sector length, first sector id 1
    77 tracks, 1 head, 26 sectors, 256 bytes sector length, first sector id 1

    5.25-inch formats
    40 tracks, 1 head, 18 sectors, 128 bytes sector length, first sector id 1
    40 tracks, 2 heads, 18 sectors, 128 bytes sector length, first sector id 1

*/


#include "includes/xerox820.h"

/* Read/Write Handlers */

void xerox820_state::bankswitch(int bank)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	UINT8 *ram = m_ram->pointer();

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
	UINT8 *ram = m_ram->pointer();

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

READ8_MEMBER( xerox820_state::fdc_r )
{
	return m_fdc->gen_r(offset) ^ 0xff;
}

WRITE8_MEMBER( xerox820_state::fdc_w )
{
	m_fdc->gen_w(offset, data ^ 0xff);
}

WRITE8_MEMBER( xerox820_state::scroll_w )
{
	m_scroll = (offset >> 8) & 0x1f;
}

#ifdef UNUSED_CODE
WRITE8_MEMBER( xerox820_state::x120_system_w )
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

WRITE8_MEMBER( xerox820ii_state::bell_w )
{
	m_speaker->level_w(offset);
}

WRITE8_MEMBER( xerox820ii_state::slden_w )
{
	m_fdc->dden_w(offset);
}

WRITE8_MEMBER( xerox820ii_state::chrom_w )
{
	m_chrom = offset;
}

WRITE8_MEMBER( xerox820ii_state::lowlite_w )
{
	m_lowlite = data;
}

WRITE8_MEMBER( xerox820ii_state::sync_w )
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

static ADDRESS_MAP_START( xerox820_mem, AS_PROGRAM, 8, xerox820_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x3000, 0x3fff) AM_RAM AM_SHARE("video_ram")
	AM_RANGE(0x4000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( xerox820_io, AS_IO, 8, xerox820_state )
	AM_RANGE(0x00, 0x00) AM_MIRROR(0xff03) AM_DEVWRITE(COM8116_TAG, com8116_device, str_w)
	AM_RANGE(0x04, 0x07) AM_MIRROR(0xff00) AM_DEVREADWRITE(Z80SIO_TAG, z80sio0_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x08, 0x0b) AM_MIRROR(0xff00) AM_DEVREADWRITE(Z80PIO_GP_TAG, z80pio_device, read_alt, write_alt)
	AM_RANGE(0x0c, 0x0c) AM_MIRROR(0xff03) AM_DEVWRITE(COM8116_TAG, com8116_device, stt_w)
	AM_RANGE(0x10, 0x13) AM_MIRROR(0xff00) AM_READWRITE(fdc_r, fdc_w)
	AM_RANGE(0x14, 0x14) AM_MIRROR(0xff03) AM_MASK(0xff00) AM_WRITE(scroll_w)
	AM_RANGE(0x18, 0x1b) AM_MIRROR(0xff00) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
	AM_RANGE(0x1c, 0x1f) AM_MIRROR(0xff00) AM_DEVREADWRITE(Z80PIO_KB_TAG, z80pio_device, read_alt, write_alt)
ADDRESS_MAP_END

static ADDRESS_MAP_START( xerox820ii_mem, AS_PROGRAM, 8, xerox820ii_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x3000, 0x3fff) AM_RAM AM_SHARE("video_ram")
	AM_RANGE(0xc000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( xerox820ii_io, AS_IO, 8, xerox820ii_state )
	AM_IMPORT_FROM(xerox820_io)
	AM_RANGE(0x28, 0x29) AM_MIRROR(0xff00) AM_WRITE(bell_w)
	AM_RANGE(0x30, 0x31) AM_MIRROR(0xff00) AM_WRITE(slden_w)
	AM_RANGE(0x34, 0x35) AM_MIRROR(0xff00) AM_WRITE(chrom_w)
	AM_RANGE(0x36, 0x36) AM_MIRROR(0xff00) AM_WRITE(lowlite_w)
	AM_RANGE(0x68, 0x69) AM_MIRROR(0xff00) AM_WRITE(sync_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( xerox168_mem, AS_PROGRAM, 16, xerox820ii_state )
	AM_RANGE(0x00000, 0x3ffff) AM_RAM
	AM_RANGE(0xff000, 0xfffff) AM_ROM AM_REGION(I8086_TAG, 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mk83_mem, AS_PROGRAM, 8, xerox820_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x3000, 0x6fff) AM_RAM
	AM_RANGE(0x7000, 0x7fff) AM_RAM AM_SHARE("video_ram")
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END


/* Input Ports */

static INPUT_PORTS_START( xerox820 )
	// inputs defined in machine/keyboard.c
INPUT_PORTS_END

TIMER_CALLBACK_MEMBER( bigboard_state::bigboard_beepoff )
{
	m_beeper->set_state(0);
}

/* Z80 PIO */

READ8_MEMBER( xerox820_state::kbpio_pa_r )
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

	UINT8 data = 0;

	// keyboard
	data |= m_kbpio->rdy_b() << 3;

	// floppy
	data |= m_8n5 << 4;
	data |= m_400_460 << 5;

	return data;
};

WRITE8_MEMBER( xerox820_state::kbpio_pa_w )
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
	floppy_image_device *floppy = NULL;

	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	if (BIT(data, 1)) floppy = m_floppy1->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		int _8n5 = (floppy->get_form_factor() == floppy_image::FF_8);

		if (m_8n5 != _8n5)
		{
			m_8n5 = _8n5;

			m_fdc->set_unscaled_clock(m_8n5 ? XTAL_20MHz/10 : XTAL_20MHz/20);
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

WRITE8_MEMBER( bigboard_state::kbpio_pa_w )
{
	xerox820_state::kbpio_pa_w(space, offset, data);

	/* beeper on bigboard */
	if (BIT(data, 5) & (!m_bit5))
	{
		machine().scheduler().timer_set(attotime::from_msec(40), timer_expired_delegate(FUNC(bigboard_state::bigboard_beepoff),this));
		m_beeper->set_state(1);
	}
	m_bit5 = BIT(data, 5);
}

READ8_MEMBER( xerox820_state::kbpio_pb_r )
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
};

WRITE8_MEMBER( xerox820ii_state::rdpio_pb_w )
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
	{ NULL }
};

/* WD1771 Interface */

static SLOT_INTERFACE_START( xerox820_floppies )
	SLOT_INTERFACE( "sa400", FLOPPY_525_SSSD_35T ) // Shugart SA-400
	SLOT_INTERFACE( "sa450", FLOPPY_525_DD ) // Shugart SA-450
	SLOT_INTERFACE( "sa800", FLOPPY_8_SSDD ) // Shugart SA-800
	SLOT_INTERFACE( "sa850", FLOPPY_8_DSDD ) // Shugart SA-850
SLOT_INTERFACE_END

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

/* COM8116 Interface */

WRITE_LINE_MEMBER( xerox820_state::fr_w )
{
	m_sio->rxca_w(state);
	m_sio->txca_w(state);
}

/* Video */

UINT32 xerox820_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,chr,gfx;
	UINT16 sy=0,ma=(m_scroll + 1) * 0x80,x;
	const pen_t *pen=m_palette->pens();

	m_framecnt++;

	for (y = 0; y < 24; y++)
	{
		if (ma > 0xb80) ma = 0;

		for (ra = 0; ra < 10; ra++)
		{
			UINT32 *p = &bitmap.pix32(sy++);

			for (x = ma; x < ma + 80; x++)
			{
				if (ra < 8)
				{
					chr = m_video_ram[x & XEROX820_VIDEORAM_MASK] ^ 0x80;

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
	m_beeper->set_frequency(950);

	m_fdc->reset();
}

void xerox820ii_state::machine_reset()
{
	bankswitch(1);

	m_fdc->reset();
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

static GFXDECODE_START( xerox820 )
	GFXDECODE_ENTRY( "chargen", 0x0000, xerox820_charlayout, 0, 1 )
GFXDECODE_END

static GFXDECODE_START( xerox820ii )
	GFXDECODE_ENTRY( "chargen", 0x0000, xerox820_charlayout, 0, 1 )
	GFXDECODE_ENTRY( "chargen", 0x0800, xerox820_gfxlayout, 0, 1 )
GFXDECODE_END

/* Machine Drivers */

static MACHINE_CONFIG_START( xerox820, xerox820_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_20MHz/8)
	MCFG_CPU_PROGRAM_MAP(xerox820_mem)
	MCFG_CPU_IO_MAP(xerox820_io)
	MCFG_CPU_CONFIG(xerox820_daisy_chain)

	/* video hardware */
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(xerox820_state, screen_update)
	MCFG_SCREEN_RAW_PARAMS(XTAL_10_69425MHz, 700, 0, 560, 260, 0, 240)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", xerox820)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	/* devices */
	MCFG_DEVICE_ADD(Z80PIO_KB_TAG, Z80PIO, XTAL_20MHz/8)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(READ8(xerox820_state, kbpio_pa_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(xerox820_state, kbpio_pa_w))
	MCFG_Z80PIO_IN_PB_CB(READ8(xerox820_state, kbpio_pb_r))

	MCFG_DEVICE_ADD(Z80PIO_GP_TAG, Z80PIO, XTAL_20MHz/8)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, XTAL_20MHz/8)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(DEVWRITELINE(Z80CTC_TAG, z80ctc_device, trg1))
	MCFG_Z80CTC_ZC2_CB(DEVWRITELINE(Z80CTC_TAG, z80ctc_device, trg3))
	//MCFG_TIMER_DRIVER_ADD_PERIODIC("ctc", xerox820_state, ctc_tick, attotime::from_hz(XTAL_20MHz/8))

	MCFG_FD1771_ADD(FD1771_TAG, XTAL_20MHz/20)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(xerox820_state, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(xerox820_state, fdc_drq_w))
	MCFG_FLOPPY_DRIVE_ADD(FD1771_TAG":0", xerox820_floppies, "sa400", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FD1771_TAG":1", xerox820_floppies, "sa400", floppy_image_device::default_floppy_formats)

	MCFG_Z80SIO0_ADD(Z80SIO_TAG, XTAL_20MHz/8, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_TXDB_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRB_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSB_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_RS232_PORT_ADD(RS232_A_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80sio0_device, rxa_w))

	MCFG_RS232_PORT_ADD(RS232_B_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80sio0_device, rxb_w))

	MCFG_DEVICE_ADD(COM8116_TAG, COM8116, XTAL_5_0688MHz)
	MCFG_COM8116_FR_HANDLER(WRITELINE(xerox820_state, fr_w))
	MCFG_COM8116_FT_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80dart_device, rxtxcb_w))

	MCFG_DEVICE_ADD(KEYBOARD_TAG, XEROX_820_KEYBOARD, 0)
	MCFG_XEROX_820_KEYBOARD_KBSTB_CALLBACK(DEVWRITELINE(Z80PIO_KB_TAG, z80pio_device, strobe_b))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")

	// software lists
	MCFG_SOFTWARE_LIST_ADD("flop_list", "xerox820")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( bigboard, xerox820, bigboard_state )
	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00) /* bigboard only */
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( xerox820ii, xerox820ii_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_16MHz/4)
	MCFG_CPU_PROGRAM_MAP(xerox820ii_mem)
	MCFG_CPU_IO_MAP(xerox820ii_io)
	MCFG_CPU_CONFIG(xerox820_daisy_chain)

	/* video hardware */
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(xerox820ii_state, screen_update)
	MCFG_SCREEN_RAW_PARAMS(XTAL_10_69425MHz, 700, 0, 560, 260, 0, 240)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", xerox820ii)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* devices */
	MCFG_DEVICE_ADD(Z80PIO_KB_TAG, Z80PIO, XTAL_16MHz/4)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(READ8(xerox820_state, kbpio_pa_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(xerox820_state, kbpio_pa_w))
	MCFG_Z80PIO_IN_PB_CB(READ8(xerox820_state, kbpio_pb_r))

	MCFG_DEVICE_ADD(Z80PIO_GP_TAG, Z80PIO, XTAL_16MHz/4)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD(Z80PIO_RD_TAG, Z80PIO, XTAL_20MHz/8)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(DEVREAD8("sasi_data_in", input_buffer_device, read))
	MCFG_Z80PIO_OUT_PA_CB(DEVWRITE8("sasi_data_out", output_latch_device, write))
	MCFG_Z80PIO_OUT_ARDY_CB(WRITELINE(xerox820ii_state, rdpio_pardy_w))
	MCFG_Z80PIO_IN_PB_CB(DEVREAD8("sasi_ctrl_in", input_buffer_device, read))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(xerox820ii_state, rdpio_pb_w))

	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, XTAL_16MHz/4)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(DEVWRITELINE(Z80CTC_TAG, z80ctc_device, trg1))
	MCFG_Z80CTC_ZC2_CB(DEVWRITELINE(Z80CTC_TAG, z80ctc_device, trg3))
	//MCFG_TIMER_DRIVER_ADD_PERIODIC("ctc", xerox820_state, ctc_tick, attotime::from_hz(XTAL_16MHz/4))

	MCFG_FD1797_ADD(FD1797_TAG, XTAL_16MHz/8)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(xerox820_state, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(xerox820_state, fdc_drq_w))
	MCFG_FLOPPY_DRIVE_ADD(FD1797_TAG":0", xerox820_floppies, "sa450", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FD1797_TAG":1", xerox820_floppies, "sa450", floppy_image_device::default_floppy_formats)

	MCFG_Z80SIO0_ADD(Z80SIO_TAG, XTAL_16MHz/4, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_TXDB_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRB_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSB_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_RS232_PORT_ADD(RS232_A_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80sio0_device, rxa_w))

	MCFG_RS232_PORT_ADD(RS232_B_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80sio0_device, rxb_w))

	MCFG_DEVICE_ADD(COM8116_TAG, COM8116, XTAL_5_0688MHz)
	MCFG_COM8116_FR_HANDLER(WRITELINE(xerox820_state, fr_w))
	MCFG_COM8116_FT_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80dart_device, rxtxcb_w))

	MCFG_DEVICE_ADD(KEYBOARD_TAG, XEROX_820_KEYBOARD, 0)
	MCFG_XEROX_820_KEYBOARD_KBSTB_CALLBACK(DEVWRITELINE(Z80PIO_KB_TAG, z80pio_device, strobe_b))

	// SASI bus
	MCFG_DEVICE_ADD(SASIBUS_TAG, SCSI_PORT, 0)
	MCFG_SCSI_DATA_INPUT_BUFFER("sasi_data_in")
	MCFG_SCSI_BSY_HANDLER(DEVWRITELINE("sasi_ctrl_in", input_buffer_device, write_bit0)) MCFG_DEVCB_XOR(1)
	MCFG_SCSI_MSG_HANDLER(DEVWRITELINE("sasi_ctrl_in", input_buffer_device, write_bit1)) MCFG_DEVCB_XOR(1)
	MCFG_SCSI_CD_HANDLER(DEVWRITELINE("sasi_ctrl_in", input_buffer_device, write_bit2)) MCFG_DEVCB_XOR(1)
	MCFG_SCSI_REQ_HANDLER(DEVWRITELINE("sasi_ctrl_in", input_buffer_device, write_bit3)) MCFG_DEVCB_XOR(1)
	MCFG_SCSI_IO_HANDLER(DEVWRITELINE("sasi_ctrl_in", input_buffer_device, write_bit4)) MCFG_DEVCB_XOR(1)

	MCFG_SCSIDEV_ADD(SASIBUS_TAG ":" SCSI_PORT_DEVICE1, "harddisk", SA1403D, SCSI_ID_0)

	MCFG_SCSI_OUTPUT_LATCH_ADD("sasi_data_out", SASIBUS_TAG)
	MCFG_DEVICE_ADD("sasi_data_in", INPUT_BUFFER, 0)
	MCFG_DEVICE_ADD("sasi_ctrl_in", INPUT_BUFFER, 0)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")

	// software lists
	MCFG_SOFTWARE_LIST_ADD("flop_list", "xerox820ii")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( xerox168, xerox820ii )
	MCFG_CPU_ADD(I8086_TAG, I8086, 4770000)
	MCFG_CPU_PROGRAM_MAP(xerox168_mem)

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("192K")
	MCFG_RAM_EXTRA_OPTIONS("320K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mk83, xerox820 )
	MCFG_CPU_MODIFY(Z80_TAG)
	MCFG_CPU_PROGRAM_MAP(mk83_mem)
MACHINE_CONFIG_END

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
	ROMX_LOAD( "x820v10.u64", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "x820v10.u63", 0x0800, 0x0800, NO_DUMP, ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v20", "Xerox Monitor v2.0" )
	ROMX_LOAD( "x820v20.u64", 0x0000, 0x0800, CRC(2fc227e2) SHA1(b4ea0ae23d281a687956e8a514cb364a1372678e), ROM_BIOS(2) )
	ROMX_LOAD( "x820v20.u63", 0x0800, 0x0800, CRC(bc11f834) SHA1(4fd2b209a6e6ff9b0c41800eb5228c34a0d7f7ef), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "smart23", "MICROCode SmartROM v2.3" )
	ROMX_LOAD( "mxkx25a.u64", 0x0000, 0x0800, CRC(7ec5f100) SHA1(5d0ff35a51aa18afc0d9c20ef99ff5d9d3f2075b), ROM_BIOS(3) )
	ROMX_LOAD( "mxkx25b.u63", 0x0800, 0x0800, CRC(a7543798) SHA1(886e617e1003d13f86f33085cbd49391b77291a3), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "plus2", "MICROCode Plus2 v0.2a" )
	ROMX_LOAD( "p2x25a.u64",  0x0000, 0x0800, CRC(3ccd7a8f) SHA1(6e46c88f03fc7289595dd6bec95e23bb13969525), ROM_BIOS(4) )
	ROMX_LOAD( "p2x25b.u63",  0x0800, 0x0800, CRC(1e580391) SHA1(e91f8ce82586df33c0d6d02eb005e8079f4de67d), ROM_BIOS(4) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "x820.u92", 0x0000, 0x0800, CRC(b823fa98) SHA1(ad0ea346aa257a53ad5701f4201896a2b3a0f928) )
ROM_END

ROM_START( x820ii )
	ROM_REGION( 0x2000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS( "v404" )
	ROM_SYSTEM_BIOS( 0, "v400", "Balcones Operating System v4.00" ) // Initial U.S. 3-ROM set: support for double density disks
	ROMX_LOAD( "v400.u33", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "v400.u34", 0x0800, 0x0800, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "v400.u35", 0x1000, 0x0800, NO_DUMP, ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v401", "Balcones Operating System v4.01" ) // Corrected overflow problem with large data files
	ROMX_LOAD( "v401.u33", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(2) )
	ROMX_LOAD( "v401.u34", 0x0800, 0x0800, NO_DUMP, ROM_BIOS(2) )
	ROMX_LOAD( "v401.u35", 0x1000, 0x0800, NO_DUMP, ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "v402", "Balcones Operating System v4.02" ) // Rank Xerox (European) boot ROM version of US 4.01
	ROMX_LOAD( "v402.u33", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(3) )
	ROMX_LOAD( "v402.u34", 0x0800, 0x0800, NO_DUMP, ROM_BIOS(3) )
	ROMX_LOAD( "v402.u35", 0x1000, 0x0800, NO_DUMP, ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "v403", "Balcones Operating System v4.03" ) // Incorporate programmable communications option and support for the low-profile keyboard (4-ROM set and type-ahead input buffer)
	ROMX_LOAD( "v403.u33", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(4) )
	ROMX_LOAD( "v403.u34", 0x0800, 0x0800, NO_DUMP, ROM_BIOS(4) )
	ROMX_LOAD( "v403.u35", 0x1000, 0x0800, NO_DUMP, ROM_BIOS(4) )
	ROMX_LOAD( "v403.u36", 0x1800, 0x0800, NO_DUMP, ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 4, "v404", "Balcones Operating System v4.04" ) // Changes sign-on message from Xerox 820-II to Xerox
	ROMX_LOAD( "537p3652.u33", 0x0000, 0x0800, CRC(7807cfbb) SHA1(bd3cc5cc5c59c84a50747aae5c17eb4617b0dbc3), ROM_BIOS(5) )
	ROMX_LOAD( "537p3653.u34", 0x0800, 0x0800, CRC(a9c6c0c3) SHA1(c2da9d1bf0da96e6b8bfa722783e411d2fe6deb9), ROM_BIOS(5) )
	ROMX_LOAD( "537p3654.u35", 0x1000, 0x0800, CRC(a8a07223) SHA1(e8ae1ebf2d7caf76771205f577b88ae493836ac9), ROM_BIOS(5) )
	ROMX_LOAD( "v404.u36", 0x1800, 0x0800, NO_DUMP, ROM_BIOS(5) ) // fitted for low-profile keyboard only
	ROM_SYSTEM_BIOS( 5, "v50", "Balcones Operating System v5.0" ) // Operating system modifications for DEM and new 5.25" disk controller (4 new boot ROMs)
	ROMX_LOAD( "v50.u33", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(6) )
	ROMX_LOAD( "v50.u34", 0x0800, 0x0800, NO_DUMP, ROM_BIOS(6) )
	ROMX_LOAD( "v50.u35", 0x1000, 0x0800, NO_DUMP, ROM_BIOS(6) )
	ROMX_LOAD( "v50.u36", 0x1800, 0x0800, NO_DUMP, ROM_BIOS(6) ) // fitted for low-profile keyboard only

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9) )
	ROM_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3) )
ROM_END

ROM_START( x168 )
	ROM_REGION( 0x2000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS( "v404" )
	ROM_SYSTEM_BIOS( 0, "v404", "Balcones Operating System v4.04" ) // Changes sign-on message from Xerox 820-II to Xerox
	ROMX_LOAD( "537p3652.u33", 0x0000, 0x0800, CRC(7807cfbb) SHA1(bd3cc5cc5c59c84a50747aae5c17eb4617b0dbc3), ROM_BIOS(1) )
	ROMX_LOAD( "537p3653.u34", 0x0800, 0x0800, CRC(a9c6c0c3) SHA1(c2da9d1bf0da96e6b8bfa722783e411d2fe6deb9), ROM_BIOS(1) )
	ROMX_LOAD( "537p3654.u35", 0x1000, 0x0800, CRC(a8a07223) SHA1(e8ae1ebf2d7caf76771205f577b88ae493836ac9), ROM_BIOS(1) )
	ROMX_LOAD( "v404.u36", 0x1800, 0x0800, NO_DUMP, ROM_BIOS(1) ) // fitted for low-profile keyboard only
	ROM_SYSTEM_BIOS( 1, "v50", "Balcones Operating System v5.0" ) // Operating system modifications for DEM and new 5.25" disk controller (4 new boot ROMs)
	ROMX_LOAD( "v50.u33", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(2) )
	ROMX_LOAD( "v50.u34", 0x0800, 0x0800, NO_DUMP, ROM_BIOS(2) )
	ROMX_LOAD( "v50.u35", 0x1000, 0x0800, NO_DUMP, ROM_BIOS(2) )
	ROMX_LOAD( "v50.u36", 0x1800, 0x0800, NO_DUMP, ROM_BIOS(2) ) // fitted for low-profile keyboard only

	ROM_REGION( 0x1000, I8086_TAG, 0 )
	ROM_LOAD( "8086.u33", 0x0000, 0x1000, CRC(ee49e3dc) SHA1(a5f20c74fc53f9d695d8894534ab69a39e2c38d8) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "x820ii.u57", 0x0000, 0x0800, CRC(1a50f600) SHA1(df4470c80611c14fa7ea8591f741fbbecdfe4fd9) )
	ROM_LOAD( "x820ii.u58", 0x0800, 0x0800, CRC(aca4b9b3) SHA1(77f41470b0151945b8d3c3a935fc66409e9157b3) )
ROM_END

ROM_START( mk83 )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_LOAD( "2732mk83.bin", 0x0000, 0x1000, CRC(a845c7e1) SHA1(3ccf629c5cd384953794ac4a1d2b45678bd40e92) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "2716mk83.bin", 0x0000, 0x0800, CRC(10bf0d81) SHA1(7ec73670a4d9d6421a5d6a4c4edc8b7c87923f6c) )
ROM_END

/* System Drivers */

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT       INIT    COMPANY                         FULLNAME        FLAGS */
COMP( 1980, bigboard,   0,          0,      bigboard,   xerox820, driver_device,   0,      "Digital Research Computers",   "Big Board",     0 )
COMP( 1981, x820,       bigboard,   0,      xerox820,   xerox820, driver_device,   0,      "Xerox",                        "Xerox 820",     GAME_NO_SOUND_HW )
COMP( 1982, mk82,       bigboard,   0,      bigboard,   xerox820, driver_device,   0,      "Scomar",                       "MK-82",         0 )
COMP( 1983, x820ii,     0,          0,      xerox820ii, xerox820, driver_device,   0,      "Xerox",                        "Xerox 820-II",  GAME_NOT_WORKING )
COMP( 1983, x168,       x820ii,     0,      xerox168,   xerox820, driver_device,   0,      "Xerox",                        "Xerox 16/8",    GAME_NOT_WORKING )
COMP( 1983, mk83,       x820ii,     0,      mk83,       xerox820, driver_device,   0,      "Scomar",                       "MK-83",         GAME_NOT_WORKING | GAME_NO_SOUND_HW )
