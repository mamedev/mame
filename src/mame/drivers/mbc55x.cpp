// license:BSD-3-Clause
// copyright-holders:Phill Harvey-Smith
/*
    drivers/mbc55x.cpp

    Machine driver for the Sanyo MBC-550 and MBC-555.

    Phill Harvey-Smith
    2011-01-29.

*/


#include "emu.h"
#include "includes/mbc55x.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"
#include "machine/i8087.h"
#include "machine/input_merger.h"
#include "machine/mbc55x_kbd.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"


void mbc55x_state::mbc55x_mem(address_map &map)
{
	map(0x00000, 0x0FFFF).bankrw(RAM_BANK00_TAG);
	map(0x10000, 0x1FFFF).bankrw(RAM_BANK01_TAG);
	map(0x20000, 0x2FFFF).bankrw(RAM_BANK02_TAG);
	map(0x30000, 0x3FFFF).bankrw(RAM_BANK03_TAG);
	map(0x40000, 0x4FFFF).bankrw(RAM_BANK04_TAG);
	map(0x50000, 0x5FFFF).bankrw(RAM_BANK05_TAG);
	map(0x60000, 0x6FFFF).bankrw(RAM_BANK06_TAG);
	map(0x70000, 0x7FFFF).bankrw(RAM_BANK07_TAG);
	map(0x80000, 0x8FFFF).bankrw(RAM_BANK08_TAG);
	map(0x90000, 0x9FFFF).bankrw(RAM_BANK09_TAG);
	map(0xA0000, 0xAFFFF).bankrw(RAM_BANK0A_TAG);
	map(0xB0000, 0xBFFFF).bankrw(RAM_BANK0B_TAG);
	map(0xC0000, 0xCFFFF).bankrw(RAM_BANK0C_TAG);
	map(0xD0000, 0xDFFFF).bankrw(RAM_BANK0D_TAG);
	map(0xE0000, 0xEFFFF).bankrw(RAM_BANK0E_TAG);
	map(0xF0000, 0xF3FFF).bankrw(RED_PLANE_TAG);
	map(0xF4000, 0xF7FFF).bankrw(BLUE_PLANE_TAG);
	map(0xF8000, 0xFBFFF).noprw();
	map(0xFC000, 0xFDFFF).rom().nopw().region(MAINCPU_TAG, 0x0000).mirror(0x002000);
}

void mbc55x_state::mbc55x_io(address_map &map)
{
	map(0x0000, 0x0000).select(0x003e).rw(FUNC(mbc55x_state::iodecode_r), FUNC(mbc55x_state::iodecode_w));
}

void mbc55x_state::mbc55x_iodecode(address_map &map)
{
	map(0x00, 0x01).mirror(0x02).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x04, 0x07).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	map(0x08, 0x08).mirror(0x03).rw(FUNC(mbc55x_state::vram_page_r), FUNC(mbc55x_state::vram_page_w));
	map(0x0c, 0x0f).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x10, 0x13).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x14, 0x15).mirror(0x02).rw("sio", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x18, 0x18).mirror(0x02).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x19, 0x19).mirror(0x02).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x1c, 0x1d).mirror(0x02).rw(m_kb_uart, FUNC(i8251_device::read), FUNC(i8251_device::write));
}

uint8_t mbc55x_state::iodecode_r(offs_t offset)
{
	return m_iodecode->read8(offset >> 1);
}

void mbc55x_state::iodecode_w(offs_t offset, uint8_t data)
{
	m_iodecode->write8(offset >> 1, data);
}

/* 8255 Configuration */

uint8_t mbc55x_state::game_io_r()
{
	u8 result = m_gameio->sw3_r();
	result |= m_gameio->sw2_r() << 1;
	result |= m_gameio->sw1_r() << 2;
	result |= m_gameio->sw0_r() << 3;

	for (int i = 0; i < 4; i++)
		if (machine().time().as_double() < m_ls123_clear_time[i])
			result |= 1 << (4 + i);

	return result;
}

uint8_t mbc55x_state::printer_status_r()
{
	return m_printer_status;
}

void mbc55x_state::printer_data_w(uint8_t data)
{
	m_printer->write_data7(!BIT(data, 7));
	m_printer->write_data6(!BIT(data, 6));
	m_printer->write_data5(!BIT(data, 5));
	m_printer->write_data4(!BIT(data, 4));
	m_printer->write_data3(!BIT(data, 3));
	m_printer->write_data2(!BIT(data, 2));
	m_printer->write_data1(!BIT(data, 1));
	m_printer->write_data0(!BIT(data, 0));

	m_gameio->an0_w(!BIT(data, 0));
	m_gameio->an1_w(!BIT(data, 1));
	m_gameio->an2_w(!BIT(data, 2));
	m_gameio->an3_w(!BIT(data, 3));
	m_gameio->an4_w(!BIT(data, 4));
	m_gameio->strobe_w(!BIT(data, 5));

	if (m_ls123_strobe != BIT(data, 7))
	{
		if (BIT(data, 7))
		{
			m_ls123_clear_time[0] = machine().time().as_double() + m_x_calibration * m_gameio->pdl0_r();
			m_ls123_clear_time[1] = machine().time().as_double() + m_y_calibration * m_gameio->pdl1_r();
			m_ls123_clear_time[2] = machine().time().as_double() + m_x_calibration * m_gameio->pdl2_r();
			m_ls123_clear_time[3] = machine().time().as_double() + m_y_calibration * m_gameio->pdl3_r();
		}

		m_ls123_strobe = BIT(data, 7);
	}
}

void mbc55x_state::disk_select_w(uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	switch (data & 0x03)
	{
	case 0: floppy = m_floppy[0]->get_device(); break;
	case 1: floppy = m_floppy[1]->get_device(); break;
	case 2: floppy = m_floppy[2]->get_device(); break;
	case 3: floppy = m_floppy[3]->get_device(); break;
	}

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		floppy->mon_w(0);
		floppy->ss_w(BIT(data, 2));
	}

	m_printer->write_strobe(!BIT(data, 3));
}

WRITE_LINE_MEMBER(mbc55x_state::printer_busy_w)
{
	m_printer_status = (m_printer_status & 0xef) | (state ? 0x10 : 0x00);
}

WRITE_LINE_MEMBER(mbc55x_state::printer_paper_end_w)
{
	m_printer_status = (m_printer_status & 0xdf) | (state ? 0x20 : 0x00);
}

WRITE_LINE_MEMBER(mbc55x_state::printer_select_w)
{
	m_printer_status = (m_printer_status & 0xbf) | (state ? 0x40 : 0x00);
}


void mbc55x_state::set_ram_size()
{
	address_space   &space      = m_maincpu->space( AS_PROGRAM );
	int             ramsize     = m_ram->size();
	int             nobanks     = ramsize / RAM_BANK_SIZE;
	char            bank[10];
	int             bankno;
	uint8_t           *ram        = &m_ram->pointer()[0];
	uint8_t           *map_base;
	int             bank_base;


	logerror("Ramsize is %d bytes\n",ramsize);
	logerror("RAM_BANK_SIZE=%d, nobanks=%d\n",RAM_BANK_SIZE,nobanks);

	// Main memory mapping

	for(bankno=0; bankno<RAM_BANK_COUNT; bankno++)
	{
		sprintf(bank,"bank%x",bankno);
		bank_base=bankno*RAM_BANK_SIZE;
		map_base=&ram[bank_base];

		if(bankno<nobanks)
		{
			membank(bank)->set_base(map_base);
			space.install_readwrite_bank(bank_base, bank_base+(RAM_BANK_SIZE-1), membank(bank));
			logerror("Mapping bank %d at %05X to RAM\n",bankno,bank_base);
		}
		else
		{
			space.nop_readwrite(bank_base, bank_base+(RAM_BANK_SIZE-1));
			logerror("Mapping bank %d at %05X to NOP\n",bankno,bank_base);
		}
	}

	// Graphics red and blue plane memory mapping, green is in main memory
	membank(RED_PLANE_TAG)->set_base(&m_video_mem[RED_PLANE_OFFSET]);
	space.install_readwrite_bank(RED_PLANE_MEMBASE, RED_PLANE_MEMBASE+(COLOUR_PLANE_SIZE-1), membank(RED_PLANE_TAG));
	membank(BLUE_PLANE_TAG)->set_base(&m_video_mem[BLUE_PLANE_OFFSET]);
	space.install_readwrite_bank(BLUE_PLANE_MEMBASE, BLUE_PLANE_MEMBASE+(COLOUR_PLANE_SIZE-1), membank(BLUE_PLANE_TAG));
}

void mbc55x_state::machine_reset()
{
	set_ram_size();
}

void mbc55x_state::machine_start()
{
	// FIXME: values copied from apple2.cpp
	m_x_calibration = attotime::from_nsec(10800).as_double();
	m_y_calibration = attotime::from_nsec(10800).as_double();

	m_printer_status = 0xff;

	m_ls123_strobe = true;
	std::fill(std::begin(m_ls123_clear_time), std::end(m_ls123_clear_time), 0.0);

	m_kb_uart->write_cts(0);
}


static INPUT_PORTS_START( mbc55x )
INPUT_PORTS_END


FLOPPY_FORMATS_MEMBER( mbc55x_state::floppy_formats )
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END


// MBC-550 : 1 x 5.25" disk-drive (160 KB)
// MBC-555 : 2 x 5.25" disk-drive (160 KB)
// MBC-555-2 : 2 x 5.25" disk-drive (360 KB)
// MBC-555-3 : 2 x 5.25" disk-drive (720 KB)

static void mbc55x_floppies(device_slot_interface &device)
{
	device.option_add("ssdd", FLOPPY_525_SSDD);
	device.option_add("dd", FLOPPY_525_DD);
	device.option_add("qd", FLOPPY_525_QD);
}


void mbc55x_state::mbc55x(machine_config &config)
{
	/* basic machine hardware */
	I8088(config, m_maincpu, 14.318181_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &mbc55x_state::mbc55x_mem);
	m_maincpu->set_addrmap(AS_IO, &mbc55x_state::mbc55x_io);
	m_maincpu->set_irq_acknowledge_callback(PIC8259_TAG, FUNC(pic8259_device::inta_cb));
	m_maincpu->esc_opcode_handler().set("coproc", FUNC(i8087_device::insn_w));
	m_maincpu->esc_data_handler().set("coproc", FUNC(i8087_device::addr_w));

	i8087_device &i8087(I8087(config, "coproc", 14.318181_MHz_XTAL / 4));
	i8087.set_space_88(m_maincpu, AS_PROGRAM);
	i8087.irq().set(m_pic, FUNC(pic8259_device::ir6_w));
	i8087.busy().set_inputline("maincpu", INPUT_LINE_TEST);

	ADDRESS_MAP_BANK(config, m_iodecode);
	m_iodecode->endianness(ENDIANNESS_LITTLE);
	m_iodecode->data_width(8);
	m_iodecode->addr_width(5);
	m_iodecode->set_addrmap(0, &mbc55x_state::mbc55x_iodecode);

	mbc55x_keyboard_device &keyboard(MBC55X_KEYBOARD(config, "keyboard"));
	keyboard.txd_callback().set(m_kb_uart, FUNC(i8251_device::write_rxd));

	/* video hardware */
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_raw(14.318181_MHz_XTAL, 896, 0, 640, 262, 0, 200);
	screen.set_screen_update(VID_MC6845_NAME, FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, FUNC(mbc55x_state::mbc55x_palette), SCREEN_NO_COLOURS * 3);

	RAM(config, RAM_TAG).set_default_size("128K").set_extra_options("128K,192K,256K,320K,384K,448K,512K,576K,640K");

	/* sound hardware */
	SPEAKER(config, MONO_TAG).front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, MONO_TAG, 0.75);

	/* Devices */
	I8251(config, m_kb_uart, 14.318181_MHz_XTAL / 8);
	m_kb_uart->txd_handler().set("speaker", FUNC(speaker_sound_device::level_w)).invert();
	m_kb_uart->rts_handler().set(m_printer, FUNC(centronics_device::write_init)).invert();
	m_kb_uart->rxrdy_handler().set(m_pic, FUNC(pic8259_device::ir3_w));

	PIT8253(config, m_pit);
	m_pit->out_handler<0>().set(m_pic, FUNC(pic8259_device::ir0_w));
	m_pit->out_handler<0>().append(m_pit, FUNC(pit8253_device::write_clk1));
	m_pit->out_handler<1>().set(m_pic, FUNC(pic8259_device::ir1_w));
	m_pit->set_clk<2>(14.318181_MHz_XTAL / 8);
	m_pit->out_handler<2>().set("sio", FUNC(i8251_device::write_txc));
	m_pit->out_handler<2>().append("sio", FUNC(i8251_device::write_rxc));
	m_pit->out_handler<2>().append("line", FUNC(rs232_port_device::write_etc));

	clock_device &clk_78_6khz(CLOCK(config, "clk_78.6khz", 14.318181_MHz_XTAL / 14 / 13));
	clk_78_6khz.signal_handler().set(m_pit, FUNC(pit8253_device::write_clk0));
	clk_78_6khz.signal_handler().append(m_kb_uart, FUNC(i8251_device::write_txc));
	clk_78_6khz.signal_handler().append(m_kb_uart, FUNC(i8251_device::write_rxc));

	PIC8259(config, m_pic);
	m_pic->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(mbc55x_state::game_io_r));
	m_ppi->out_pb_callback().set(FUNC(mbc55x_state::printer_data_w));
	m_ppi->in_pc_callback().set(FUNC(mbc55x_state::printer_status_r));
	m_ppi->out_pc_callback().set(FUNC(mbc55x_state::disk_select_w));

	HD6845S(config, m_crtc, 14.318181_MHz_XTAL / 8); // HD46505SP-1
	m_crtc->set_screen(SCREEN_TAG);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(mbc55x_state::crtc_update_row));
	m_crtc->out_vsync_callback().set(FUNC(mbc55x_state::vid_vsync_changed));
	m_crtc->out_hsync_callback().set(FUNC(mbc55x_state::vid_hsync_changed));

	/* Backing storage */
	FD1793(config, m_fdc, 14.318181_MHz_XTAL / 14); // M5W1793-02P (clock is nominally 1 MHz)
	m_fdc->intrq_wr_callback().set(m_pic, FUNC(pic8259_device::ir5_w));

	FLOPPY_CONNECTOR(config, m_floppy[0], mbc55x_floppies, "qd", mbc55x_state::floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], mbc55x_floppies, "qd", mbc55x_state::floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[2], mbc55x_floppies, nullptr, mbc55x_state::floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[3], mbc55x_floppies, nullptr, mbc55x_state::floppy_formats);

	/* Software list */
	SOFTWARE_LIST(config, "disk_list").set_original("mbc55x");

	isa8_device &isa(ISA8(config, "isa", 14.318181_MHz_XTAL / 4));
	isa.set_memspace(m_maincpu, AS_PROGRAM);
	isa.set_iospace(m_maincpu, AS_IO);
	isa.irq7_callback().set(m_pic, FUNC(pic8259_device::ir7_w)); // all other IRQ and DRQ lines are NC
	isa.iochck_callback().set_inputline(m_maincpu, INPUT_LINE_NMI).invert();

	ISA8_SLOT(config, "external", 0, "isa", pc_isa8_cards, nullptr, false);

	i8251_device &sio(I8251(config, "sio", 14.318181_MHz_XTAL / 8)); // on separate board, through 20-pin header
	sio.dtr_handler().set("line", FUNC(rs232_port_device::write_dtr));
	sio.txd_handler().set("line", FUNC(rs232_port_device::write_txd));
	sio.rts_handler().set("line", FUNC(rs232_port_device::write_rts));
	sio.rxrdy_handler().set("sioint", FUNC(input_merger_device::in_w<0>));
	sio.txrdy_handler().set("sioint", FUNC(input_merger_device::in_w<1>));

	rs232_port_device &serial(RS232_PORT(config, "line", default_rs232_devices, nullptr));
	serial.rxd_handler().set("sio", FUNC(i8251_device::write_rxd));
	serial.dsr_handler().set("sio", FUNC(i8251_device::write_dsr));
	serial.cts_handler().set("sio", FUNC(i8251_device::write_cts));

	INPUT_MERGER_ANY_HIGH(config, "sioint").output_handler().set(m_pic, FUNC(pic8259_device::ir2_w));

	APPLE2_GAMEIO(config, m_gameio, apple2_gameio_device::default_options, nullptr);
	m_gameio->set_sw_pullups(true); // 3300 ohm pullups to 5.0V on pins 2-4 and 16

	CENTRONICS(config, m_printer, centronics_devices, nullptr);
	m_printer->busy_handler().set(FUNC(mbc55x_state::printer_busy_w)).invert(); // LS14 Schmitt trigger
	m_printer->busy_handler().append(m_pic, FUNC(pic8259_device::ir4_w)).invert();
	m_printer->perror_handler().set(FUNC(mbc55x_state::printer_paper_end_w));
	m_printer->select_handler().set(FUNC(mbc55x_state::printer_select_w));
}


ROM_START( mbc55x )
	ROM_REGION(0x4000, MAINCPU_TAG, 0)

	ROM_SYSTEM_BIOS(0, "v120", "mbc55x BIOS v1.20 (1983)")
	ROMX_LOAD("mbc55x-v120.rom", 0x0000, 0x2000, CRC(b439b4b8) SHA1(6e8df0f3868e3fd0229a5c2720d6c01e46815cab), ROM_BIOS(0))
ROM_END

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY  FULLNAME   FLAGS
COMP( 1983, mbc55x, 0,      0,      mbc55x,  mbc55x, mbc55x_state, empty_init, "Sanyo", "MBC-55x", 0 )
