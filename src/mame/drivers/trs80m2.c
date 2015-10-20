// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    Tandy Radio Shack TRS-80 Model II/12/16/16B/6000

    http://home.iae.nl/users/pb0aia/cm/modelii.html

*/

/*

    TODO:

    - CP/M won't load prompt (z80dma_do_operation: invalid mode 0 when reading track 0)
    - keyboard CPU ROM
    - graphics board
    - Tandy 6000 HD

        chdman -createblankhd tandy6000hd.chd 306 6 34 256

*/

#include "includes/trs80m2.h"

#define KEYBOARD_TAG "keyboard"


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ8_MEMBER( trs80m2_state::read )
{
	UINT8 data = 0;

	if (offset < 0x800)
	{
		if (m_boot_rom)
		{
			data = m_rom->base()[offset];
		}
		else
		{
			data = m_ram->pointer()[offset];
		}
	}
	else if (offset < 0x8000)
	{
		data = m_ram->pointer()[offset];
	}
	else
	{
		if (m_msel && offset >= 0xf800)
		{
			data = m_video_ram[offset & 0x7ff];
		}
		else if (m_bank)
		{
			offs_t addr = (m_bank << 15) | (offset & 0x7fff);

			if (addr < m_ram->size())
			{
				data = m_ram->pointer()[addr];
			}
		}
	}

	return data;
}

WRITE8_MEMBER( trs80m2_state::write )
{
	if (offset < 0x8000)
	{
		m_ram->pointer()[offset] = data;
	}
	else
	{
		if (m_msel && offset >= 0xf800)
		{
			m_video_ram[offset & 0x7ff] = data;
		}
		else if (m_bank)
		{
			offs_t addr = (m_bank << 15) | (offset & 0x7fff);

			if (addr < m_ram->size())
			{
				m_ram->pointer()[addr] = data;
			}
		}
	}
}

WRITE8_MEMBER( trs80m2_state::rom_enable_w )
{
	/*

	    bit     description

	    0       BOOT ROM
	    1
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	m_boot_rom = BIT(data, 0);
}

WRITE8_MEMBER( trs80m2_state::drvslt_w )
{
	/*

	    bit     signal

	    0       DS1
	    1       DS2
	    2       DS3
	    3       DS4
	    4
	    5
	    6       SDSEL
	    7       FM/MFM

	*/

	// drive select
	m_floppy = NULL;

	if (!BIT(data, 0)) m_floppy = m_floppy0->get_device();
	if (!BIT(data, 1)) m_floppy = m_floppy1->get_device();
	if (!BIT(data, 2)) m_floppy = m_floppy2->get_device();
	if (!BIT(data, 3)) m_floppy = m_floppy3->get_device();

	m_fdc->set_floppy(m_floppy);

	if (m_floppy)
	{
		// side select
		m_floppy->ss_w(!BIT(data, 6));
	}

	// FM/MFM
	m_fdc->dden_w(!BIT(data, 7));
}

READ8_MEMBER( trs80m2_state::keyboard_r )
{
	// clear keyboard interrupt
	if (!m_kbirq)
	{
		m_kbirq = 1;
		m_ctc->trg3(m_kbirq);
		m_kb->busy_w(m_kbirq);
	}

	m_key_bit = 0;

	return m_key_data;
}

READ8_MEMBER( trs80m2_state::rtc_r )
{
	// clear RTC interrupt
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	return 0;
}

READ8_MEMBER( trs80m2_state::nmi_r )
{
	/*

	    bit     signal              description

	    0
	    1
	    2
	    3
	    4       80/40 CHAR EN       80/40 character mode
	    5       ENABLE RTC INT      RTC interrupt enable
	    6       DE                  display enabled
	    7       KBIRQ               keyboard interrupt

	*/

	UINT8 data = 0;

	// 80/40 character mode*/
	data |= m_80_40_char_en << 4;

	// RTC interrupt enable
	data |= m_enable_rtc_int << 5;

	// display enabled
	data |= m_de << 6;

	// keyboard interrupt
	data |= !m_kbirq << 7;

	return data;
}

WRITE8_MEMBER( trs80m2_state::nmi_w )
{
	/*

	    bit     signal              description

	    0                           memory bank select bit 0
	    1                           memory bank select bit 1
	    2                           memory bank select bit 2
	    3                           memory bank select bit 3
	    4       80/40 CHAR EN       80/40 character mode
	    5       ENABLE RTC INT      RTC interrupt enable
	    6       BLNKVID             video display enable
	    7                           video RAM enable

	*/

	// memory bank select
	m_bank = data & 0x0f;

	// 80/40 character mode
	m_80_40_char_en = BIT(data, 4);
	m_crtc->set_clock(m_80_40_char_en ? XTAL_12_48MHz/16 : XTAL_12_48MHz/8);

	// RTC interrupt enable
	m_enable_rtc_int = BIT(data, 5);

	if (m_enable_rtc_int && m_rtc_int)
	{
		// trigger RTC interrupt
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}

	// video display enable
	m_blnkvid = BIT(data, 6);

	// video RAM enable
	m_msel = BIT(data, 7);
}

READ8_MEMBER( trs80m2_state::fdc_r )
{
	return m_fdc->gen_r(offset) ^ 0xff;
}

WRITE8_MEMBER( trs80m2_state::fdc_w )
{
	m_fdc->gen_w(offset, data ^ 0xff);
}

WRITE8_MEMBER( trs80m16_state::tcl_w )
{
	/*

	    bit     description

	    0       CONT0
	    1       CONT1
	    2       HALT
	    3       RESET
	    4       CONT4
	    5       CONT5
	    6       CONT6
	    7       A14

	*/

	m_subcpu->set_input_line(INPUT_LINE_HALT, BIT(data, 2) ? ASSERT_LINE : CLEAR_LINE);
	m_subcpu->set_input_line(INPUT_LINE_RESET, BIT(data, 3) ? ASSERT_LINE : CLEAR_LINE);

	m_pic->ir0_w(BIT(data, 4));
	m_pic->ir1_w(BIT(data, 5));
	m_pic->ir2_w(BIT(data, 6));

	m_ual = (m_ual & 0x1fe) | BIT(data, 7);
}

WRITE8_MEMBER( trs80m16_state::ual_w )
{
	/*

	    bit     description

	    0       A15
	    1       A16
	    2       A17
	    3       A18
	    4       A19
	    5       A20
	    6       A21
	    7       A22

	*/

	m_ual = (data << 1) | BIT(m_ual, 0);
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( z80_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( z80_mem, AS_PROGRAM, 8, trs80m2_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(read, write)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( z80_io )
//-------------------------------------------------

static ADDRESS_MAP_START( z80_io, AS_IO, 8, trs80m2_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xe0, 0xe3) AM_DEVREADWRITE(Z80PIO_TAG, z80pio_device, read, write)
	AM_RANGE(0xe4, 0xe7) AM_READWRITE(fdc_r, fdc_w)
	AM_RANGE(0xef, 0xef) AM_WRITE(drvslt_w)
	AM_RANGE(0xf0, 0xf3) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
	AM_RANGE(0xf4, 0xf7) AM_DEVREADWRITE(Z80SIO_TAG, z80sio0_device, cd_ba_r, cd_ba_w)
	AM_RANGE(0xf8, 0xf8) AM_DEVREADWRITE(Z80DMA_TAG, z80dma_device, read, write)
	AM_RANGE(0xf9, 0xf9) AM_WRITE(rom_enable_w)
	AM_RANGE(0xfc, 0xfc) AM_READ(keyboard_r) AM_DEVWRITE(MC6845_TAG, mc6845_device, address_w)
	AM_RANGE(0xfd, 0xfd) AM_DEVREADWRITE(MC6845_TAG, mc6845_device, register_r, register_w)
	AM_RANGE(0xfe, 0xfe) AM_READ(rtc_r)
	AM_RANGE(0xff, 0xff) AM_READWRITE(nmi_r, nmi_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( m16_z80_io )
//-------------------------------------------------

static ADDRESS_MAP_START( m16_z80_io, AS_IO, 8, trs80m16_state )
	AM_IMPORT_FROM(z80_io)
	AM_RANGE(0xde, 0xde) AM_WRITE(tcl_w)
	AM_RANGE(0xdf, 0xdf) AM_WRITE(ual_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( m68000_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( m68000_mem, AS_PROGRAM, 16, trs80m2_state )
//  AM_RANGE(0x7800d0, 0x7800d1) PIC
//  AM_RANGE(0x7800d2, 0x7800d3) limit/offset 2
//  AM_RANGE(0x7800d4, 0x7800d5) limit/offset 1
//  AM_RANGE(0x7800d6, 0x7800d7) Z80 IRQ
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( trs80m2 )
//-------------------------------------------------

static INPUT_PORTS_START( trs80m2 )
INPUT_PORTS_END



//**************************************************************************
//  VIDEO
//**************************************************************************

MC6845_UPDATE_ROW( trs80m2_state::crtc_update_row )
{
	const pen_t *pen = m_palette->pens();

	int x = 0;

	for (int column = 0; column < x_count; column++)
	{
		UINT8 code = m_video_ram[(ma + column) & 0x7ff];
		offs_t address = ((code & 0x7f) << 4) | (ra & 0x0f);
		UINT8 data = m_char_rom->base()[address];

		int dcursor = (column == cursor_x);
		int drevid = BIT(code, 7);

		for (int bit = 0; bit < 8; bit++)
		{
			int dout = BIT(data, 7);
			int color = (dcursor ^ drevid ^ dout) && de;

			bitmap.pix32(vbp + y, hbp + x++) = pen[color];

			data <<= 1;
		}
	}
}

WRITE_LINE_MEMBER( trs80m2_state::de_w )
{
	m_de = state;
}

WRITE_LINE_MEMBER( trs80m2_state::vsync_w )
{
	if (state)
	{
		m_rtc_int = !m_rtc_int;

		if (m_enable_rtc_int && m_rtc_int)
		{
			// trigger RTC interrupt
			m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		}
	}
}

void trs80m2_state::video_start()
{
	// allocate memory
	m_video_ram.allocate(0x800);
}

UINT32 trs80m2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_blnkvid)
	{
		bitmap.fill(rgb_t::black, cliprect);
	}
	else
	{
		m_crtc->screen_update(screen, bitmap, cliprect);
	}

	return 0;
}



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  TRS80M2_KEYBOARD_INTERFACE( kb_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( trs80m2_state::kb_clock_w )
{
	int kbdata = m_kb->data_r();

	if (m_key_bit == 8)
	{
		if (!m_kbdata && kbdata)
		{
			// trigger keyboard interrupt
			m_kbirq = 0;
			m_ctc->trg3(m_kbirq);
			m_kb->busy_w(m_kbirq);
		}
	}
	else
	{
		if (!m_kbclk && state)
		{
			// shift in keyboard data bit
			m_key_data <<= 1;
			m_key_data |= kbdata;
			m_key_bit++;
		}
	}

	m_kbdata = kbdata;
	m_kbclk = state;
}

WRITE8_MEMBER( trs80m2_state::kbd_w )
{
	// latch key data
	m_key_data = data;

	// trigger keyboard interrupt
	m_kbirq = 0;
	m_ctc->trg3(m_kbirq);
}

//-------------------------------------------------
//  Z80DMA
//-------------------------------------------------

READ8_MEMBER(trs80m2_state::io_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(trs80m2_state::io_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.write_byte(offset, data);
}

//-------------------------------------------------
//  Z80PIO
//-------------------------------------------------

WRITE_LINE_MEMBER( trs80m2_state::write_centronics_busy )
{
	m_centronics_busy = state;
}

WRITE_LINE_MEMBER( trs80m2_state::write_centronics_fault )
{
	m_centronics_fault = state;
}

WRITE_LINE_MEMBER( trs80m2_state::write_centronics_perror )
{
	m_centronics_perror = state;
}

READ8_MEMBER( trs80m2_state::pio_pa_r )
{
	/*

	    bit     signal      description

	    0       INTRQ       FDC INT request
	    1       _TWOSID     2-sided diskette
	    2       _DSKCHG     disk change
	    3       PRIME       prime
	    4       FAULT       printer fault
	    5       PSEL        printer select
	    6       PE          paper empty
	    7       BUSY        printer busy

	*/

	UINT8 data = 0;

	// floppy interrupt
	data |= (m_fdc->intrq_r() ? 0x01 : 0x00);

	// 2-sided diskette
	data |= (m_floppy ? m_floppy->twosid_r() : 1) << 1;

	// disk change
	data |= (m_floppy ? m_floppy->dskchg_r() : 1) << 2;

	// printer fault
	data |= m_centronics_fault << 4;

	// paper empty
	data |= m_centronics_perror << 6;

	// printer busy
	data |= m_centronics_busy << 7;

	return data;
}

WRITE8_MEMBER( trs80m2_state::pio_pa_w )
{
	/*

	    bit     signal      description

	    0       INTRQ       FDC INT request
	    1       _TWOSID     2-sided diskette
	    2       _DSKCHG     disk change
	    3       PRIME       prime
	    4       FAULT       printer fault
	    5       PSEL        printer select
	    6       PE          paper empty
	    7       BUSY        printer busy

	*/

	// prime
	m_centronics->write_init(BIT(data, 3));
}

WRITE_LINE_MEMBER( trs80m2_state::strobe_w )
{
	m_centronics->write_strobe(!state);
}

//-------------------------------------------------
//  Z80CTC
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER(trs80m2_state::ctc_tick)
{
	m_ctc->trg0(1);
	m_ctc->trg0(0);

	m_ctc->trg1(1);
	m_ctc->trg1(0);

	m_ctc->trg2(1);
	m_ctc->trg2(0);
}

static SLOT_INTERFACE_START( trs80m2_floppies )
	SLOT_INTERFACE( "8ssdd", FLOPPY_8_SSDD ) // Shugart SA-800
	SLOT_INTERFACE( "8dsdd", FLOPPY_8_DSDD ) // Shugart SA-850
SLOT_INTERFACE_END


//-------------------------------------------------
//  z80_daisy_config trs80m2_daisy_chain
//-------------------------------------------------

static const z80_daisy_config trs80m2_daisy_chain[] =
{
	{ Z80CTC_TAG },
	{ Z80SIO_TAG },
	{ Z80DMA_TAG },
	{ Z80PIO_TAG },
	{ NULL }
};

//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  machine_start
//-------------------------------------------------

void trs80m2_state::machine_start()
{
	// register for state saving
	save_item(NAME(m_boot_rom));
	save_item(NAME(m_bank));
	save_item(NAME(m_msel));
	save_item(NAME(m_key_latch));
	save_item(NAME(m_key_data));
	save_item(NAME(m_key_bit));
	save_item(NAME(m_kbclk));
	save_item(NAME(m_kbdata));
	save_item(NAME(m_kbirq));
	save_item(NAME(m_blnkvid));
	save_item(NAME(m_80_40_char_en));
	save_item(NAME(m_de));
	save_item(NAME(m_rtc_int));
	save_item(NAME(m_enable_rtc_int));
}

void trs80m16_state::machine_start()
{
	trs80m2_state::machine_start();

	// register for state saving
	save_item(NAME(m_ual));
	save_item(NAME(m_limit));
	save_item(NAME(m_offset));
}


//-------------------------------------------------
//  machine_reset
//-------------------------------------------------

void trs80m2_state::machine_reset()
{
	// clear keyboard interrupt
	m_kbirq = 1;
	m_ctc->trg3(m_kbirq);
	m_kb->busy_w(m_kbirq);

	// enable boot ROM
	m_boot_rom = 1;

	// disable video RAM
	m_msel = 0;
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( trs80m2 )
//-------------------------------------------------

static MACHINE_CONFIG_START( trs80m2, trs80m2_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_8MHz/2)
	MCFG_CPU_CONFIG(trs80m2_daisy_chain)
	MCFG_CPU_PROGRAM_MAP(z80_mem)
	MCFG_CPU_IO_MAP(z80_io)

	// video hardware
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_UPDATE_DRIVER(trs80m2_state, screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)

	MCFG_PALETTE_ADD_MONOCHROME_GREEN("palette")

	MCFG_MC6845_ADD(MC6845_TAG, MC6845, SCREEN_TAG, XTAL_12_48MHz/8)
	MCFG_MC6845_SHOW_BORDER_AREA(true)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(trs80m2_state, crtc_update_row)
	MCFG_MC6845_OUT_DE_CB(WRITELINE(trs80m2_state, de_w))
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(trs80m2_state, vsync_w))

	// devices
	MCFG_FD1791_ADD(FD1791_TAG, XTAL_8MHz/4)
	MCFG_WD_FDC_INTRQ_CALLBACK(DEVWRITE8(Z80PIO_TAG, z80pio_device, pa_w))
	MCFG_WD_FDC_DRQ_CALLBACK(DEVWRITELINE(Z80DMA_TAG, z80dma_device, rdy_w))
	MCFG_FLOPPY_DRIVE_ADD(FD1791_TAG":0", trs80m2_floppies, "8dsdd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FD1791_TAG":1", trs80m2_floppies, NULL,    floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FD1791_TAG":2", trs80m2_floppies, NULL,    floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FD1791_TAG":3", trs80m2_floppies, NULL,    floppy_image_device::default_floppy_formats)

	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, XTAL_8MHz/2)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(DEVWRITELINE(Z80SIO_TAG, z80dart_device, rxca_w))
	MCFG_Z80CTC_ZC1_CB(DEVWRITELINE(Z80SIO_TAG, z80dart_device, txca_w))
	MCFG_Z80CTC_ZC2_CB(DEVWRITELINE(Z80SIO_TAG, z80dart_device, rxtxcb_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("ctc", trs80m2_state, ctc_tick, attotime::from_hz(XTAL_8MHz/2/2))

	MCFG_DEVICE_ADD(Z80DMA_TAG, Z80DMA, XTAL_8MHz/2)
	MCFG_Z80DMA_OUT_BUSREQ_CB(INPUTLINE(Z80_TAG, INPUT_LINE_HALT))
	MCFG_Z80DMA_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80DMA_IN_MREQ_CB(READ8(trs80m2_state, read))
	MCFG_Z80DMA_OUT_MREQ_CB(WRITE8(trs80m2_state, write))
	MCFG_Z80DMA_IN_IORQ_CB(READ8(trs80m2_state, io_read_byte))
	MCFG_Z80DMA_OUT_IORQ_CB(WRITE8(trs80m2_state, io_write_byte))

	MCFG_DEVICE_ADD(Z80PIO_TAG, Z80PIO, XTAL_8MHz/2)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(READ8(trs80m2_state, pio_pa_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(trs80m2_state, pio_pa_w))
	MCFG_Z80PIO_OUT_PB_CB(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_Z80PIO_OUT_BRDY_CB(WRITELINE(trs80m2_state, strobe_w))

	MCFG_Z80SIO0_ADD(Z80SIO_TAG, XTAL_8MHz/2, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(DEVWRITELINE(Z80PIO_TAG, z80pio_device, strobe_b))
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(trs80m2_state, write_centronics_busy))
	MCFG_CENTRONICS_FAULT_HANDLER(WRITELINE(trs80m2_state, write_centronics_fault))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(trs80m2_state, write_centronics_perror))
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)

	MCFG_DEVICE_ADD(TRS80M2_KEYBOARD_TAG, TRS80M2_KEYBOARD, 0)
	MCFG_TRS80M2_KEYBOARD_CLOCK_CALLBACK(WRITELINE(trs80m2_state, kb_clock_w))
	MCFG_DEVICE_ADD(KEYBOARD_TAG, GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(trs80m2_state, kbd_w))

	// internal RAM
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
	MCFG_RAM_EXTRA_OPTIONS("32K,96K,128K,160K,192K,224K,256K,288K,320K,352K,384K,416K,448K,480K,512K")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "trs80m2")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( trs80m16 )
//-------------------------------------------------

static MACHINE_CONFIG_START( trs80m16, trs80m16_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_8MHz/2)
	MCFG_CPU_CONFIG(trs80m2_daisy_chain)
	MCFG_CPU_PROGRAM_MAP(z80_mem)
	MCFG_CPU_IO_MAP(m16_z80_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE(AM9519A_TAG, pic8259_device, inta_cb)

	MCFG_CPU_ADD(M68000_TAG, M68000, XTAL_24MHz/4)
	MCFG_CPU_PROGRAM_MAP(m68000_mem)
	MCFG_DEVICE_DISABLE()

	// video hardware
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_UPDATE_DRIVER(trs80m2_state, screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)

	MCFG_PALETTE_ADD_MONOCHROME_GREEN("palette")

	MCFG_MC6845_ADD(MC6845_TAG, MC6845, SCREEN_TAG, XTAL_12_48MHz/8)
	MCFG_MC6845_SHOW_BORDER_AREA(true)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(trs80m2_state, crtc_update_row)
	MCFG_MC6845_OUT_DE_CB(WRITELINE(trs80m2_state, de_w))
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(trs80m2_state, vsync_w))

	// devices
	MCFG_FD1791_ADD(FD1791_TAG, XTAL_8MHz/4)
	MCFG_WD_FDC_INTRQ_CALLBACK(DEVWRITE8(Z80PIO_TAG, z80pio_device, pa_w))
	MCFG_WD_FDC_DRQ_CALLBACK(DEVWRITELINE(Z80DMA_TAG, z80dma_device, rdy_w))
	MCFG_FLOPPY_DRIVE_ADD(FD1791_TAG":0", trs80m2_floppies, "8dsdd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FD1791_TAG":1", trs80m2_floppies, NULL,    floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FD1791_TAG":2", trs80m2_floppies, NULL,    floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FD1791_TAG":3", trs80m2_floppies, NULL,    floppy_image_device::default_floppy_formats)

	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, XTAL_8MHz/2)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(DEVWRITELINE(Z80SIO_TAG, z80dart_device, rxca_w))
	MCFG_Z80CTC_ZC1_CB(DEVWRITELINE(Z80SIO_TAG, z80dart_device, txca_w))
	MCFG_Z80CTC_ZC2_CB(DEVWRITELINE(Z80SIO_TAG, z80dart_device, rxtxcb_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("ctc", trs80m2_state, ctc_tick, attotime::from_hz(XTAL_8MHz/2/2))

	MCFG_DEVICE_ADD(Z80DMA_TAG, Z80DMA, XTAL_8MHz/2)
	MCFG_Z80DMA_OUT_BUSREQ_CB(INPUTLINE(Z80_TAG, INPUT_LINE_HALT))
	MCFG_Z80DMA_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80DMA_IN_MREQ_CB(READ8(trs80m2_state, read))
	MCFG_Z80DMA_OUT_MREQ_CB(WRITE8(trs80m2_state, write))
	MCFG_Z80DMA_IN_IORQ_CB(READ8(trs80m2_state, io_read_byte))
	MCFG_Z80DMA_OUT_IORQ_CB(WRITE8(trs80m2_state, io_write_byte))

	MCFG_DEVICE_ADD(Z80PIO_TAG, Z80PIO, XTAL_8MHz/2)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(READ8(trs80m2_state, pio_pa_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(trs80m2_state, pio_pa_w))
	MCFG_Z80PIO_OUT_PB_CB(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_Z80PIO_OUT_BRDY_CB(WRITELINE(trs80m2_state, strobe_w))

	MCFG_Z80SIO0_ADD(Z80SIO_TAG, XTAL_8MHz/2, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_PIC8259_ADD(AM9519A_TAG, INPUTLINE(M68000_TAG, M68K_IRQ_5), VCC, NULL )

	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(DEVWRITELINE(Z80PIO_TAG, z80pio_device, strobe_b))
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(trs80m2_state, write_centronics_busy))
	MCFG_CENTRONICS_FAULT_HANDLER(WRITELINE(trs80m2_state, write_centronics_fault))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(trs80m2_state, write_centronics_perror))
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)

	MCFG_DEVICE_ADD(TRS80M2_KEYBOARD_TAG, TRS80M2_KEYBOARD, 0)
	MCFG_TRS80M2_KEYBOARD_CLOCK_CALLBACK(WRITELINE(trs80m2_state, kb_clock_w))
	MCFG_DEVICE_ADD(KEYBOARD_TAG, GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(trs80m2_state, kbd_w))

	// internal RAM
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("256K")
	MCFG_RAM_EXTRA_OPTIONS("512K,768K,1M")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "trs80m2")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

/*

    TRS-80 Model II/16 Z80 CPU Board ROM

    It would seem that every processor board I find has a different ROM on it!  It seems that the early ROMs
    don't boot directly from a hard drive.  But there seems to be many versions of ROMs.  I've placed them in
    order of serial number in the list below.  There also appears to be at least two board revisions, "C" and "D".

    cpu_c8ff.bin/hex:
    Mask Programmable PROM, Equivilant to Intel 2716 EPROM, with checksum C8FF came from a cpu board with
    serial number 120353 out of a Model II with serial number 2002102 and catalog number 26-6002.  The board
    was labeled, "Revision C".  This appears to be an early ROM and according to a very helpful fellow
    collector, Aaron in Australia, doesn't allow boot directly from a hard disk.

    cpu_9733.bin/hex:
    An actual SGS-Ates (Now STMicroelectronics) 2716 EPROM, with checksum 9733 came from a cpu board with
    serial number 161993 out of a pile of random cards that I have.  I don't know what machine it originated
    from.  The board was labeled, "Revision C".  This appears to be a later ROM in that it is able to boot
    directly from an 8MB hard disk.  The EPROM had a windows sticker on it labeled, "U54".

    cpu_2119.bin/hex:
    An actual Texas Instruments TMS2516 EPROM, with checksum 2119 came from a cpu board with serial number
    178892 out of a Model 16 with serial number 64014509 and catalog number 26-4002.  The board was labeled,
    "Revision D".  This appears to be a later ROM and does appear to allow boot directly from an 8MB hard disk.

    cpu_2bff.bin/hex:
    Mask Programmable PROM, Equivilant to Intel 2716 EPROM, with checksum 2BFF came from a cpu board with
    serial number 187173 our of a pile of random cards that I have.  I don't know what machine it originated
    from.  The board was labeled, "Revision D".  This appears to be a later ROM in that it is able to boot
    directly from an 8MB hard disk.

*/

//-------------------------------------------------
//  ROM( trs80m2 )
//-------------------------------------------------

ROM_START( trs80m2 )
	ROM_REGION( 0x800, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS( "9733" )
	ROM_SYSTEM_BIOS( 0, "c8ff", "Version 1" )
	ROMX_LOAD( "8043216.u11", 0x0000, 0x0800, CRC(7017a373) SHA1(1c7127fcc99fc351a40d3a3199ba478e783c452e), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "2bff", "Version 2 (1981-07-29)" )
	ROMX_LOAD( "8047316.u11", 0x0000, 0x0800, CRC(c6c71d8b) SHA1(7107e2cbbe769851a4460680c2deff8e76a101b5), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "2119", "Version 3 (1982-05-07)" )
	ROMX_LOAD( "cpu_2119.u11", 0x0000, 0x0800, CRC(7a663049) SHA1(f308439ce266df717bfe79adcdad6024b4faa141), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "1bbe", "Version 3 (1982-05-07, Alt)" )
	ROMX_LOAD( "cpu_11be.u11", 0x0000, 0x0800, CRC(8edceea7) SHA1(3d797acedd8a71a82c695129ca764f85aa9022b2), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 4, "fc86", "Version 4 (1982-11-18)" )
	ROMX_LOAD( "cpu_fc86.u11", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 5, "9733", "Version 5 (1983-07-29)" )
	ROMX_LOAD( "u54.u11", 0x0000, 0x0800, CRC(823924b1) SHA1(aee0625bcbd8620b28ab705e15ad9bea804c8476), ROM_BIOS(6) )

	ROM_REGION( 0x800, MC6845_TAG, 0 )
	ROM_LOAD( "8043316.u9", 0x0000, 0x0800, CRC(04425b03) SHA1(32a29dc202b7fcf21838289cc3bffc51ef943dab) )
ROM_END


//-------------------------------------------------
//  ROM( trs80m16 )
//-------------------------------------------------

#define rom_trs80m16 rom_trs80m2



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT   INIT     COMPANY             FULLNAME        FLAGS
COMP( 1979, trs80m2,    0,          0,      trs80m2,    trs80m2, driver_device,     0,      "Tandy Radio Shack",    "TRS-80 Model II",  MACHINE_NO_SOUND_HW | MACHINE_IMPERFECT_KEYBOARD )
COMP( 1982, trs80m16,   trs80m2,    0,      trs80m16,   trs80m2, driver_device,     0,      "Tandy Radio Shack",    "TRS-80 Model 16",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_IMPERFECT_KEYBOARD )
//COMP( 1983, trs80m12, trs80m2,    0,      trs80m16,   trs80m2, driver_device,     0,      "Tandy Radio Shack",    "TRS-80 Model 12",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_IMPERFECT_KEYBOARD )
//COMP( 1984, trs80m16b,trs80m2,    0,      trs80m16,   trs80m2, driver_device,     0,      "Tandy Radio Shack",    "TRS-80 Model 16B", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_IMPERFECT_KEYBOARD )
//COMP( 1985, tandy6k,  trs80m2,    0,      tandy6k,    trs80m2, driver_device,     0,      "Tandy Radio Shack",    "Tandy 6000 HD",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_IMPERFECT_KEYBOARD )
