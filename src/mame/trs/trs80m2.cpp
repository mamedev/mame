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

#include "emu.h"
#include "trs80m2.h"

#include "screen.h"
#include "softlist_dev.h"


#define KEYBOARD_TAG "keyboard"


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

uint8_t trs80m2_state::read(offs_t offset)
{
	uint8_t data = 0;

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

void trs80m2_state::write(offs_t offset, uint8_t data)
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

void trs80m2_state::rom_enable_w(uint8_t data)
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

void trs80m2_state::drvslt_w(uint8_t data)
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
	m_floppy = nullptr;

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

uint8_t trs80m2_state::keyboard_r()
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

uint8_t trs80m2_state::rtc_r()
{
	// clear RTC interrupt
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	return 0;
}

uint8_t trs80m2_state::nmi_r()
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

	uint8_t data = 0;

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

void trs80m2_state::nmi_w(uint8_t data)
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
	m_crtc->set_unscaled_clock(12.48_MHz_XTAL / (m_80_40_char_en ? 16 : 8));

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

uint8_t trs80m2_state::fdc_r(offs_t offset)
{
	return m_fdc->read(offset) ^ 0xff;
}

void trs80m2_state::fdc_w(offs_t offset, uint8_t data)
{
	m_fdc->write(offset, data ^ 0xff);
}

void trs80m16_state::tcl_w(uint8_t data)
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

	m_uic->ireq0_w(BIT(data, 4));
	m_uic->ireq1_w(BIT(data, 5));
	m_uic->ireq2_w(BIT(data, 6));

	m_ual = (m_ual & 0x1fe) | BIT(data, 7);
}

void trs80m16_state::ual_w(uint8_t data)
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

void trs80m2_state::z80_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(trs80m2_state::read), FUNC(trs80m2_state::write));
}


//-------------------------------------------------
//  ADDRESS_MAP( z80_io )
//-------------------------------------------------

void trs80m2_state::z80_io(address_map &map)
{
	map.global_mask(0xff);
	map(0xe0, 0xe3).rw(m_pio, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0xe4, 0xe7).rw(FUNC(trs80m2_state::fdc_r), FUNC(trs80m2_state::fdc_w));
	map(0xef, 0xef).w(FUNC(trs80m2_state::drvslt_w));
	map(0xf0, 0xf3).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0xf4, 0xf7).rw(Z80SIO_TAG, FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0xf8, 0xf8).rw(m_dmac, FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0xf9, 0xf9).w(FUNC(trs80m2_state::rom_enable_w));
	map(0xfc, 0xfc).r(FUNC(trs80m2_state::keyboard_r)).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0xfd, 0xfd).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xfe, 0xfe).r(FUNC(trs80m2_state::rtc_r));
	map(0xff, 0xff).rw(FUNC(trs80m2_state::nmi_r), FUNC(trs80m2_state::nmi_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( m16_z80_io )
//-------------------------------------------------

void trs80m16_state::m16_z80_io(address_map &map)
{
	z80_io(map);
	map(0xde, 0xde).w(FUNC(trs80m16_state::tcl_w));
	map(0xdf, 0xdf).w(FUNC(trs80m16_state::ual_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( m68000_mem )
//-------------------------------------------------

void trs80m2_state::m68000_mem(address_map &map)
{
//  map(0x7800d0, 0x7800d1) 9519A (C/D = UDS)
//  map(0x7800d2, 0x7800d3) limit/offset 2
//  map(0x7800d4, 0x7800d5) limit/offset 1
//  map(0x7800d6, 0x7800d7) Z80 IRQ
}



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
	pen_t const *const pen = m_palette->pens();

	int x = 0;

	for (int column = 0; column < x_count; column++)
	{
		uint8_t code = m_video_ram[(ma + column) & 0x7ff];
		offs_t address = ((code & 0x7f) << 4) | (ra & 0x0f);
		uint8_t data = m_char_rom->base()[address];

		int dcursor = (column == cursor_x);
		int drevid = BIT(code, 7);

		for (int bit = 0; bit < 8; bit++)
		{
			int dout = BIT(data, 7);
			int color = (dcursor ^ drevid ^ dout) && de;

			bitmap.pix(vbp + y, hbp + x++) = pen[color];

			data <<= 1;
		}
	}
}

void trs80m2_state::de_w(int state)
{
	m_de = state;
}

void trs80m2_state::vsync_w(int state)
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
}

uint32_t trs80m2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_blnkvid)
	{
		bitmap.fill(rgb_t::black(), cliprect);
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

void trs80m2_state::kb_clock_w(int state)
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

void trs80m2_state::kbd_w(u8 data)
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

uint8_t trs80m2_state::io_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.read_byte(offset);
}

void trs80m2_state::io_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.write_byte(offset, data);
}

//-------------------------------------------------
//  Z80PIO
//-------------------------------------------------

void trs80m2_state::write_centronics_busy(int state)
{
	m_centronics_busy = state;
}

void trs80m2_state::write_centronics_fault(int state)
{
	m_centronics_fault = state;
}

void trs80m2_state::write_centronics_perror(int state)
{
	m_centronics_perror = state;
}

uint8_t trs80m2_state::pio_pa_r()
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

	uint8_t data = 0;

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

void trs80m2_state::pio_pa_w(uint8_t data)
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

void trs80m2_state::strobe_w(int state)
{
	m_centronics->write_strobe(!state);
}

//-------------------------------------------------
//  Z80CTC
//-------------------------------------------------

static void trs80m2_floppies(device_slot_interface &device)
{
	device.option_add("8ssdd", FLOPPY_8_SSDD); // Shugart SA-800
	device.option_add("8dsdd", FLOPPY_8_DSDD); // Shugart SA-850
}


//-------------------------------------------------
//  z80_daisy_config trs80m2_daisy_chain
//-------------------------------------------------

static const z80_daisy_config trs80m2_daisy_chain[] =
{
	{ Z80CTC_TAG },
	{ Z80SIO_TAG },
	{ Z80DMA_TAG },
	{ Z80PIO_TAG },
	{ nullptr }
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
//  machine_config( trs80m2 )
//-------------------------------------------------

void trs80m2_state::trs80m2(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 8_MHz_XTAL / 2);
	m_maincpu->set_daisy_config(trs80m2_daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &trs80m2_state::z80_mem);
	m_maincpu->set_addrmap(AS_IO, &trs80m2_state::z80_io);
	m_maincpu->busack_cb().set(m_dmac, FUNC(z80dma_device::bai_w));

	// video hardware
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_screen_update(FUNC(trs80m2_state::screen_update));
	screen.set_size(640, 480);
	screen.set_visarea(0, 639, 0, 479);

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	MC6845(config, m_crtc, 12.48_MHz_XTAL / 8);
	m_crtc->set_screen(SCREEN_TAG);
	m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(trs80m2_state::crtc_update_row));
	m_crtc->out_de_callback().set(FUNC(trs80m2_state::de_w));
	m_crtc->out_vsync_callback().set(FUNC(trs80m2_state::vsync_w));

	// devices
	FD1791(config, m_fdc, 8_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(m_pio, FUNC(z80pio_device::port_a_write));
	m_fdc->drq_wr_callback().set(m_dmac, FUNC(z80dma_device::rdy_w));
	FLOPPY_CONNECTOR(config, FD1791_TAG":0", trs80m2_floppies, "8dsdd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, FD1791_TAG":1", trs80m2_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, FD1791_TAG":2", trs80m2_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, FD1791_TAG":3", trs80m2_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);

	Z80CTC(config, m_ctc, 8_MHz_XTAL / 2);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->set_clk<0>(8_MHz_XTAL / 2 / 2);
	m_ctc->set_clk<1>(8_MHz_XTAL / 2 / 2);
	m_ctc->set_clk<2>(8_MHz_XTAL / 2 / 2);
	m_ctc->zc_callback<0>().set(Z80SIO_TAG, FUNC(z80sio_device::rxca_w));
	m_ctc->zc_callback<1>().set(Z80SIO_TAG, FUNC(z80sio_device::txca_w));
	m_ctc->zc_callback<2>().set(Z80SIO_TAG, FUNC(z80sio_device::rxtxcb_w));

	Z80DMA(config, m_dmac, 8_MHz_XTAL / 2);
	m_dmac->out_busreq_callback().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSRQ);
	m_dmac->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_dmac->in_mreq_callback().set(FUNC(trs80m2_state::read));
	m_dmac->out_mreq_callback().set(FUNC(trs80m2_state::write));
	m_dmac->in_iorq_callback().set(FUNC(trs80m2_state::io_read_byte));
	m_dmac->out_iorq_callback().set(FUNC(trs80m2_state::io_write_byte));

	Z80PIO(config, m_pio, 8_MHz_XTAL / 2);
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio->in_pa_callback().set(FUNC(trs80m2_state::pio_pa_r));
	m_pio->out_pa_callback().set(FUNC(trs80m2_state::pio_pa_w));
	m_pio->out_pb_callback().set("cent_data_out", FUNC(output_latch_device::write));
	m_pio->out_brdy_callback().set(FUNC(trs80m2_state::strobe_w));

	z80sio_device& sio(Z80SIO(config, Z80SIO_TAG, 8_MHz_XTAL / 2)); // SIO/0
	sio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(m_pio, FUNC(z80pio_device::strobe_b));
	m_centronics->busy_handler().set(FUNC(trs80m2_state::write_centronics_busy));
	m_centronics->fault_handler().set(FUNC(trs80m2_state::write_centronics_fault));
	m_centronics->perror_handler().set(FUNC(trs80m2_state::write_centronics_perror));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	TRS80M2_KEYBOARD(config, m_kb, 0);
	m_kb->clock_wr_callback().set(FUNC(trs80m2_state::kb_clock_w));
	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, KEYBOARD_TAG, 0));
	keyboard.set_keyboard_callback(FUNC(trs80m2_state::kbd_w));

	// internal RAM
	RAM(config, RAM_TAG).set_default_size("64K").set_extra_options("32K,96K,128K,160K,192K,224K,256K,288K,320K,352K,384K,416K,448K,480K,512K");

	// software list
	SOFTWARE_LIST(config, "flop_list").set_original("trs80m2");
}


//-------------------------------------------------
//  machine_config( trs80m16 )
//-------------------------------------------------

void trs80m16_state::trs80m16(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 8_MHz_XTAL / 2);
	m_maincpu->set_daisy_config(trs80m2_daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &trs80m16_state::z80_mem);
	m_maincpu->set_addrmap(AS_IO, &trs80m16_state::m16_z80_io);
	m_maincpu->set_irq_acknowledge_callback(AM9519A_TAG, FUNC(am9519_device::iack_cb));
	m_maincpu->busack_cb().set(m_dmac, FUNC(z80dma_device::bai_w));

	M68000(config, m_subcpu, 24_MHz_XTAL / 4);
	m_subcpu->set_addrmap(AS_PROGRAM, &trs80m16_state::m68000_mem);
	m_subcpu->set_disable();

	// video hardware
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_screen_update(FUNC(trs80m2_state::screen_update));
	screen.set_size(640, 480);
	screen.set_visarea(0, 639, 0, 479);

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	MC6845(config, m_crtc, 12.48_MHz_XTAL / 8);
	m_crtc->set_screen(SCREEN_TAG);
	m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(trs80m2_state::crtc_update_row));
	m_crtc->out_de_callback().set(FUNC(trs80m2_state::de_w));
	m_crtc->out_vsync_callback().set(FUNC(trs80m2_state::vsync_w));

	// devices
	FD1791(config, m_fdc, 8_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(m_pio, FUNC(z80pio_device::port_a_write));
	m_fdc->drq_wr_callback().set(m_dmac, FUNC(z80dma_device::rdy_w));
	FLOPPY_CONNECTOR(config, FD1791_TAG":0", trs80m2_floppies, "8dsdd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, FD1791_TAG":1", trs80m2_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, FD1791_TAG":2", trs80m2_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, FD1791_TAG":3", trs80m2_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);

	Z80CTC(config, m_ctc, 8_MHz_XTAL / 2);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->set_clk<0>(8_MHz_XTAL / 2 / 2);
	m_ctc->set_clk<1>(8_MHz_XTAL / 2 / 2);
	m_ctc->set_clk<2>(8_MHz_XTAL / 2 / 2);
	m_ctc->zc_callback<0>().set(Z80SIO_TAG, FUNC(z80sio_device::rxca_w));
	m_ctc->zc_callback<1>().set(Z80SIO_TAG, FUNC(z80sio_device::txca_w));
	m_ctc->zc_callback<2>().set(Z80SIO_TAG, FUNC(z80sio_device::rxtxcb_w));

	Z80DMA(config, m_dmac, 8_MHz_XTAL / 2);
	m_dmac->out_busreq_callback().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSRQ);
	m_dmac->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_dmac->in_mreq_callback().set(FUNC(trs80m2_state::read));
	m_dmac->out_mreq_callback().set(FUNC(trs80m2_state::write));
	m_dmac->in_iorq_callback().set(FUNC(trs80m2_state::io_read_byte));
	m_dmac->out_iorq_callback().set(FUNC(trs80m2_state::io_write_byte));

	Z80PIO(config, m_pio, 8_MHz_XTAL / 2);
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio->in_pa_callback().set(FUNC(trs80m2_state::pio_pa_r));
	m_pio->out_pa_callback().set(FUNC(trs80m2_state::pio_pa_w));
	m_pio->out_pb_callback().set("cent_data_out", FUNC(output_latch_device::write));
	m_pio->out_brdy_callback().set(FUNC(trs80m2_state::strobe_w));

	z80sio_device& sio(Z80SIO(config, Z80SIO_TAG, 8_MHz_XTAL / 2));
	sio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	AM9519(config, m_uic, 0);
	m_uic->out_int_callback().set_inputline(m_subcpu, M68K_IRQ_5);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(m_pio, FUNC(z80pio_device::strobe_b));
	m_centronics->busy_handler().set(FUNC(trs80m2_state::write_centronics_busy));
	m_centronics->fault_handler().set(FUNC(trs80m2_state::write_centronics_fault));
	m_centronics->perror_handler().set(FUNC(trs80m2_state::write_centronics_perror));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	TRS80M2_KEYBOARD(config, m_kb, 0);
	m_kb->clock_wr_callback().set(FUNC(trs80m2_state::kb_clock_w));
	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, KEYBOARD_TAG, 0));
	keyboard.set_keyboard_callback(FUNC(trs80m2_state::kbd_w));

	// internal RAM
	RAM(config, RAM_TAG).set_default_size("256K").set_extra_options("512K,768K,1M");

	// software list
	SOFTWARE_LIST(config, "flop_list").set_original("trs80m2");
}



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
	ROMX_LOAD( "8043216.u11", 0x0000, 0x0800, CRC(7017a373) SHA1(1c7127fcc99fc351a40d3a3199ba478e783c452e), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "2bff", "Version 2 (1981-07-29)" )
	ROMX_LOAD( "8047316.u11", 0x0000, 0x0800, CRC(c6c71d8b) SHA1(7107e2cbbe769851a4460680c2deff8e76a101b5), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "2119", "Version 3 (1982-05-07)" )
	ROMX_LOAD( "cpu_2119.u11", 0x0000, 0x0800, CRC(7a663049) SHA1(f308439ce266df717bfe79adcdad6024b4faa141), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "1bbe", "Version 3 (1982-05-07, Alt)" )
	ROMX_LOAD( "cpu_11be.u11", 0x0000, 0x0800, CRC(8edceea7) SHA1(3d797acedd8a71a82c695129ca764f85aa9022b2), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "fc86", "Version 4 (1982-11-18)" )
	ROMX_LOAD( "cpu_fc86.u11", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "9733", "Version 5 (1983-07-29)" )
	ROMX_LOAD( "u54.u11", 0x0000, 0x0800, CRC(823924b1) SHA1(aee0625bcbd8620b28ab705e15ad9bea804c8476), ROM_BIOS(5) )

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

//    YEAR  NAME        PARENT   COMPAT  MACHINE   INPUT    CLASS           INIT        COMPANY              FULLNAME            FLAGS
COMP( 1979, trs80m2,    0,       0,      trs80m2,  trs80m2, trs80m2_state,  empty_init, "Tandy Radio Shack", "TRS-80 Model II",  MACHINE_NO_SOUND_HW )
COMP( 1982, trs80m16,   trs80m2, 0,      trs80m16, trs80m2, trs80m16_state, empty_init, "Tandy Radio Shack", "TRS-80 Model 16",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
//COMP( 1983, trs80m12, trs80m2, 0,      trs80m16, trs80m2, trs80m16_state, empty_init, "Tandy Radio Shack", "TRS-80 Model 12",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
//COMP( 1984, trs80m16b,trs80m2, 0,      trs80m16, trs80m2, trs80m16_state, empty_init, "Tandy Radio Shack", "TRS-80 Model 16B", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
//COMP( 1985, tandy6k,  trs80m2, 0,      tandy6k,  trs80m2, tandy6k_state,  empty_init, "Tandy Radio Shack", "Tandy 6000 HD",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
