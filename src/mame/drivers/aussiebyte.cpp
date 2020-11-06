// license:BSD-3-Clause
// copyright-holders:Robbbert
/*************************************************************************************************

    The Aussie Byte II Single-Board Computer, created by SME Systems, Melbourne, Australia.
    Also known as the Knight 2000 Microcomputer.

    Status:
    Boots up from floppy.
    Output to serial terminal and to 6545 are working. Serial keyboard works.

    Developed in conjunction with members of the MSPP. Written in July, 2015.

    ToDo:
    - CRT8002 attributes controller
    - Graphics
    - Hard drive controllers and drives
    - Test Centronics printer
    - PIO connections

    Note of MAME restrictions:
    - Votrax doesn't sound anything like the real thing
    - WD1001/WD1002 device is not emulated
    - CRT8002 device is not emulated

**************************************************************************************************/

/***********************************************************

    Includes

************************************************************/
#include "emu.h"
#include "includes/aussiebyte.h"

#include "screen.h"
#include "speaker.h"


/***********************************************************

    Address Maps

************************************************************/

void aussiebyte_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).bankr("bankr0").bankw("bankw0");
	map(0x4000, 0x7fff).bankrw("bank1");
	map(0x8000, 0xbfff).bankrw("bank2");
	map(0xc000, 0xffff).ram();
}

void aussiebyte_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x03).rw("sio1", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x04, 0x07).rw(m_pio1, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x08, 0x0b).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x0c, 0x0f).noprw(); // winchester interface
	map(0x10, 0x13).rw(m_fdc, FUNC(wd2797_device::read), FUNC(wd2797_device::write));
	map(0x14, 0x14).rw(m_dma, FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x15, 0x15).w(FUNC(aussiebyte_state::port15_w)); // boot rom disable
	map(0x16, 0x16).w(FUNC(aussiebyte_state::port16_w)); // fdd select
	map(0x17, 0x17).w(FUNC(aussiebyte_state::port17_w)); // DMA mux
	map(0x18, 0x18).w(FUNC(aussiebyte_state::port18_w)); // fdc select
	map(0x19, 0x19).r(FUNC(aussiebyte_state::port19_r)); // info port
	map(0x1a, 0x1a).w(FUNC(aussiebyte_state::port1a_w)); // membank
	map(0x1b, 0x1b).w(FUNC(aussiebyte_state::port1b_w)); // winchester control
	map(0x1c, 0x1f).w(FUNC(aussiebyte_state::port1c_w)); // gpebh select
	map(0x20, 0x23).rw(m_pio2, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x24, 0x27).rw("sio2", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x28, 0x28).r(FUNC(aussiebyte_state::port28_r)).w(m_votrax, FUNC(votrax_sc01_device::write));
	map(0x2c, 0x2c).w(m_votrax, FUNC(votrax_sc01_device::inflection_w));
	map(0x30, 0x30).w(FUNC(aussiebyte_state::address_w));
	map(0x31, 0x31).r(m_crtc, FUNC(mc6845_device::status_r));
	map(0x32, 0x32).w(FUNC(aussiebyte_state::register_w));
	map(0x33, 0x33).r(FUNC(aussiebyte_state::port33_r));
	map(0x34, 0x34).w(FUNC(aussiebyte_state::port34_w)); // video control
	map(0x35, 0x35).w(FUNC(aussiebyte_state::port35_w)); // data to vram and aram
	map(0x36, 0x36).r(FUNC(aussiebyte_state::port36_r)); // data from vram and aram
	map(0x37, 0x37).r(FUNC(aussiebyte_state::port37_r)); // read dispen flag
	map(0x40, 0x4f).rw(FUNC(aussiebyte_state::rtc_r), FUNC(aussiebyte_state::rtc_w));
}

/***********************************************************

    Keyboard

************************************************************/
static INPUT_PORTS_START( aussiebyte )
INPUT_PORTS_END

/***********************************************************

    I/O Ports

************************************************************/
void aussiebyte_state::port15_w(u8 data)
{
	m_bankr0->set_entry(m_port15); // point at ram
	m_port15 = true;
}

/* FDD select
0 Drive Select bit O
1 Drive Select bit 1
2 Drive Select bit 2
3 Drive Select bit 3
  - These bits connect to a 74LS145 binary to BCD converter.
  - Drives 0 to 3 are 5.25 inch, 4 to 7 are 8 inch, 9 and 0 are not used.
  - Currently we only support drive 0.
4 Side Select to Disk Drives.
5 Disable 5.25 inch floppy spindle motors.
6 Unused.
7 Enable write precompensation on WD2797 controller. */
void aussiebyte_state::port16_w(u8 data)
{
	floppy_image_device *m_floppy = nullptr;
	if ((data & 15) == 0)
		m_floppy = m_floppy0->get_device();
	else
	if ((data & 15) == 1)
		m_floppy = m_floppy1->get_device();

	m_fdc->set_floppy(m_floppy);

	if (m_floppy)
	{
		m_floppy->mon_w(BIT(data, 5));
		m_floppy->ss_w(BIT(data, 4));
	}
}

/* DMA select
0 - FDC
1 - SIO Ch A
2 - SIO Ch B
3 - Winchester bus
4 - SIO Ch C
5 - SIO Ch D
6 - Ext ready 1
7 - Ext ready 2 */
void aussiebyte_state::port17_w(u8 data)
{
	m_port17 = data & 7;
	m_dma->rdy_w(BIT(m_port17_rdy, data));
}

/* FDC params
2 EXC: WD2797 clock frequency. H = 5.25"; L = 8"
3 WIEN: WD2797 Double density select. */
void aussiebyte_state::port18_w(u8 data)
{
	m_fdc->set_unscaled_clock(BIT(data, 2) ? 1e6 : 2e6);
	m_fdc->dden_w(BIT(data, 3));
}

u8 aussiebyte_state::port19_r()
{
	return m_port19;
}

// Memory banking
void aussiebyte_state::port1a_w(u8 data)
{
	data &= 7;
	switch (data)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			m_port1a = data*3+1;
			if (m_port15)
				m_bankr0->set_entry(data*3+1);
			m_bankw0->set_entry(data*3+1);
			m_bank1->set_entry(data*3+2);
			m_bank2->set_entry(data*3+3);
			break;
		case 5:
			m_port1a = 1;
			if (m_port15)
				m_bankr0->set_entry(1);
			m_bankw0->set_entry(1);
			m_bank1->set_entry(2);
			m_bank2->set_entry(13);
			break;
		case 6:
			m_port1a = 14;
			if (m_port15)
				m_bankr0->set_entry(14);
			m_bankw0->set_entry(14);
			m_bank1->set_entry(15);
			//m_bank2->set_entry(0); // open bus
			break;
		case 7:
			m_port1a = 1;
			if (m_port15)
				m_bankr0->set_entry(1);
			m_bankw0->set_entry(1);
			m_bank1->set_entry(4);
			m_bank2->set_entry(13);
			break;
	}
}

// Winchester control
void aussiebyte_state::port1b_w(u8 data)
{
}

// GPEHB control
void aussiebyte_state::port1c_w(u8 data)
{
}

void aussiebyte_state::port20_w(u8 data)
{
	m_speaker->level_w(BIT(data, 7));
	m_rtc->cs_w(BIT(data, 0));
	m_rtc->hold_w(BIT(data, 0));
}

u8 aussiebyte_state::port28_r()
{
	return m_port28;
}

/***********************************************************

    RTC

************************************************************/
u8 aussiebyte_state::rtc_r(offs_t offset)
{
	m_rtc->read_w(1);
	m_rtc->address_w(offset);
	u8 data = m_rtc->data_r();
	m_rtc->read_w(0);
	return data;
}

void aussiebyte_state::rtc_w(offs_t offset, u8 data)
{
	m_rtc->address_w(offset);
	m_rtc->data_w(data);
	m_rtc->write_w(1);
	m_rtc->write_w(0);
}

/***********************************************************

    DMA

************************************************************/
u8 aussiebyte_state::memory_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

void aussiebyte_state::memory_write_byte(offs_t offset, u8 data)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	prog_space.write_byte(offset, data);
}

u8 aussiebyte_state::io_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.read_byte(offset);
}

void aussiebyte_state::io_write_byte(offs_t offset, u8 data)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	prog_space.write_byte(offset, data);
}

WRITE_LINE_MEMBER( aussiebyte_state::busreq_w )
{
// since our Z80 has no support for BUSACK, we assume it is granted immediately
	m_maincpu->set_input_line(Z80_INPUT_LINE_BUSRQ, state);
	m_dma->bai_w(state); // tell dma that bus has been granted
}

/***********************************************************

    DMA selector

************************************************************/
WRITE_LINE_MEMBER( aussiebyte_state::sio1_rdya_w )
{
	m_port17_rdy = (m_port17_rdy & 0xfd) | (u8)(state << 1);
	if (m_port17 == 1)
		m_dma->rdy_w(state);
}

WRITE_LINE_MEMBER( aussiebyte_state::sio1_rdyb_w )
{
	m_port17_rdy = (m_port17_rdy & 0xfb) | (u8)(state << 2);
	if (m_port17 == 2)
		m_dma->rdy_w(state);
}

WRITE_LINE_MEMBER( aussiebyte_state::sio2_rdya_w )
{
	m_port17_rdy = (m_port17_rdy & 0xef) | (u8)(state << 4);
	if (m_port17 == 4)
		m_dma->rdy_w(state);
}

WRITE_LINE_MEMBER( aussiebyte_state::sio2_rdyb_w )
{
	m_port17_rdy = (m_port17_rdy & 0xdf) | (u8)(state << 5);
	if (m_port17 == 5)
		m_dma->rdy_w(state);
}


/***********************************************************

    Video

************************************************************/

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

/***************************************************************

    Daisy Chain

****************************************************************/

static const z80_daisy_config daisy_chain_intf[] =
{
	{ "dma" },
	{ "pio2" },
	{ "sio1" },
	{ "sio2" },
	{ "pio1" },
	{ "ctc" },
	{ nullptr }
};


/***********************************************************

    Floppy Disk

************************************************************/

WRITE_LINE_MEMBER( aussiebyte_state::fdc_intrq_w )
{
	u8 data = (m_port19 & 0xbf) | (state ? 0x40 : 0);
	m_port19 = data;
}

WRITE_LINE_MEMBER( aussiebyte_state::fdc_drq_w )
{
	u8 data = (m_port19 & 0x7f) | (state ? 0x80 : 0);
	m_port19 = data;
	state ^= 1; // inverter on pin38 of fdc
	m_port17_rdy = (m_port17_rdy & 0xfe) | (u8)state;
	if (m_port17 == 0)
		m_dma->rdy_w(state);
}

static void aussiebyte_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}


/***********************************************************

    Quickload

    This loads a .COM file to address 0x100 then jumps
    there. Sometimes .COM has been renamed to .CPM to
    prevent windows going ballistic. These can be loaded
    as well.

************************************************************/

QUICKLOAD_LOAD_MEMBER(aussiebyte_state::quickload_cb)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);

	if (quickload_size >= 0xfd00)
		return image_init_result::FAIL;

	/* RAM must be banked in */
	m_port15 = true;    // disable boot rom
	m_port1a = 4;
	m_bankr0->set_entry(m_port1a); /* enable correct program bank */
	m_bankw0->set_entry(m_port1a);

	/* Avoid loading a program if CP/M-80 is not in memory */
	if ((prog_space.read_byte(0) != 0xc3) || (prog_space.read_byte(5) != 0xc3))
	{
		machine_reset();
		return image_init_result::FAIL;
	}

	/* Load image to the TPA (Transient Program Area) */
	for (u16 i = 0; i < quickload_size; i++)
	{
		u8 data;
		if (image.fread( &data, 1) != 1)
			return image_init_result::FAIL;
		prog_space.write_byte(i+0x100, data);
	}

	/* clear out command tail */
	prog_space.write_byte(0x80, 0); prog_space.write_byte(0x81, 0);

	/* Roughly set SP basing on the BDOS position */
	m_maincpu->set_state_int(Z80_SP, 256 * prog_space.read_byte(7) - 0x400);
	m_maincpu->set_pc(0x100);                // start program

	return image_init_result::PASS;
}

/***********************************************************

    Machine Driver

************************************************************/
void aussiebyte_state::machine_reset()
{
	m_port15 = false;
	m_port17 = 0;
	m_port17_rdy = 0;
	m_port1a = 1;
	m_alpha_address = 0;
	m_graph_address = 0;
	m_bankr0->set_entry(16); // point at rom
	m_bankw0->set_entry(1); // always write to ram
	m_bank1->set_entry(2);
	m_bank2->set_entry(3);
	m_maincpu->reset();
}

void aussiebyte_state::machine_start()
{
	m_vram = std::make_unique<u8[]>(0x10000);
	m_aram = std::make_unique<u8[]>(0x800);
	m_ram = make_unique_clear<u8[]>(0x40000);
	save_pointer(NAME(m_vram), 0x10000);
	save_pointer(NAME(m_aram), 0x800);
	save_pointer(NAME(m_ram),  0x40000);
	save_item(NAME(m_port15));
	save_item(NAME(m_port17));
	save_item(NAME(m_port17_rdy));
	save_item(NAME(m_port19));
	save_item(NAME(m_port1a));
	save_item(NAME(m_port28));
	save_item(NAME(m_port34));
	save_item(NAME(m_port35));
	save_item(NAME(m_video_index));
	save_item(NAME(m_cnt));
	save_item(NAME(m_alpha_address));
	save_item(NAME(m_graph_address));
	save_item(NAME(m_centronics_busy));

	// Main ram is divided into 16k blocks (0-15). The boot rom is block number 16.
	// For convenience, bank 0 is permanently assigned to C000-FFFF

	m_bankr0->configure_entries(0, 16, m_p_mram, 0x4000);
	m_bankw0->configure_entries(0, 16, m_p_mram, 0x4000);
	m_bank1->configure_entries(0, 16, m_p_mram, 0x4000);
	m_bank2->configure_entries(0, 16, m_p_mram, 0x4000);
	m_bankr0->configure_entry(16, memregion("roms")->base());
}


void aussiebyte_state::aussiebyte(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &aussiebyte_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &aussiebyte_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain_intf);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(16_MHz_XTAL, 952, 0, 640, 336, 0, 288);
	screen.set_screen_update("crtc", FUNC(sy6545_1_device::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_crt8002);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
	VOTRAX_SC01(config, m_votrax, 720000); // 720kHz? needs verify
	m_votrax->ar_callback().set([this] (bool state) { m_port28 = state ? 0 : 1; });
	m_votrax->add_route(ALL_OUTPUTS, "mono", 1.00);

	/* devices */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->set_data_input_buffer("cent_data_in");
	m_centronics->busy_handler().set([this] (bool state) { m_centronics_busy = state; });
	INPUT_BUFFER(config, "cent_data_in");
	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	Z80CTC(config, m_ctc, 16_MHz_XTAL / 4);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->set_clk<0>(4.9152_MHz_XTAL / 4);
	m_ctc->set_clk<1>(4.9152_MHz_XTAL / 4);
	m_ctc->set_clk<2>(4.9152_MHz_XTAL / 4);
	m_ctc->zc_callback<0>().set("sio1", FUNC(z80sio_device::rxca_w));
	m_ctc->zc_callback<0>().append("sio1", FUNC(z80sio_device::txca_w));
	m_ctc->zc_callback<1>().set("sio1", FUNC(z80sio_device::rxtxcb_w));
	m_ctc->zc_callback<1>().append("sio2", FUNC(z80sio_device::rxca_w));
	m_ctc->zc_callback<1>().append("sio2", FUNC(z80sio_device::txca_w));
	m_ctc->zc_callback<2>().set("ctc", FUNC(z80ctc_device::trg3));
	m_ctc->zc_callback<2>().append("sio2", FUNC(z80sio_device::rxtxcb_w));

	Z80DMA(config, m_dma, 16_MHz_XTAL / 4);
	m_dma->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_dma->out_busreq_callback().set(FUNC(aussiebyte_state::busreq_w));
	// BAO, not used
	m_dma->in_mreq_callback().set(FUNC(aussiebyte_state::memory_read_byte));
	m_dma->out_mreq_callback().set(FUNC(aussiebyte_state::memory_write_byte));
	m_dma->in_iorq_callback().set(FUNC(aussiebyte_state::io_read_byte));
	m_dma->out_iorq_callback().set(FUNC(aussiebyte_state::io_write_byte));

	Z80PIO(config, m_pio1, 16_MHz_XTAL / 4);
	m_pio1->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio1->out_pa_callback().set("cent_data_out", FUNC(output_latch_device::write));
	m_pio1->in_pb_callback().set("cent_data_in", FUNC(input_buffer_device::read));
	m_pio1->out_ardy_callback().set(m_centronics, FUNC(centronics_device::write_strobe)).invert();

	Z80PIO(config, m_pio2, 16_MHz_XTAL / 4);
	m_pio2->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio2->out_pa_callback().set(FUNC(aussiebyte_state::port20_w));

	z80sio_device& sio1(Z80SIO(config, "sio1", 16_MHz_XTAL / 4));
	sio1.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	sio1.out_wrdya_callback().set(FUNC(aussiebyte_state::sio1_rdya_w));
	sio1.out_wrdyb_callback().set(FUNC(aussiebyte_state::sio1_rdyb_w));

	z80sio_device& sio2(Z80SIO(config, "sio2", 16_MHz_XTAL / 4));
	sio2.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	sio2.out_wrdya_callback().set(FUNC(aussiebyte_state::sio2_rdya_w));
	sio2.out_wrdyb_callback().set(FUNC(aussiebyte_state::sio2_rdyb_w));
	sio2.out_txda_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	sio2.out_dtra_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	sio2.out_rtsa_callback().set("rs232", FUNC(rs232_port_device::write_rts));

	RS232_PORT(config, m_rs232, default_rs232_devices, "keyboard");
	m_rs232->rxd_handler().set("sio2", FUNC(z80sio_device::rxa_w));

	WD2797(config, m_fdc, 16_MHz_XTAL / 16);
	m_fdc->intrq_wr_callback().set(FUNC(aussiebyte_state::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(aussiebyte_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", aussiebyte_floppies, "525qd", floppy_image_device::default_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", aussiebyte_floppies, "525qd", floppy_image_device::default_floppy_formats).enable_sound(true);

	/* devices */
	SY6545_1(config, m_crtc, 16_MHz_XTAL / 8);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(aussiebyte_state::crtc_update_row));
	m_crtc->set_on_update_addr_change_callback(FUNC(aussiebyte_state::crtc_update_addr));

	MSM5832(config, m_rtc, 32.768_kHz_XTAL);

	/* quickload */
	QUICKLOAD(config, "quickload", "com,cpm", attotime::from_seconds(3)).set_load_callback(FUNC(aussiebyte_state::quickload_cb));

	SOFTWARE_LIST(config, "flop_list").set_original("aussiebyte");
}


/***********************************************************

    Game driver

************************************************************/


ROM_START(aussieby)
	ROM_REGION(0x4000, "roms", 0) // Size of bank 16
	ROM_LOAD( "knight_boot_0000.u27", 0x0000, 0x1000, CRC(1f200437) SHA1(80d1d208088b325c16a6824e2da605fb2b00c2ce) )

	ROM_REGION(0x800, "chargen", 0)
	ROM_LOAD( "8002.bin", 0x0000, 0x0800, CRC(fdd6eb13) SHA1(a094d416e66bdab916e72238112a6265a75ca690) )
ROM_END

//    YEAR  NAME      PARENT  COMPAT  MACHINE     INPUT       CLASS             INIT        COMPANY        FULLNAME          FLAGS
COMP( 1984, aussieby, 0,      0,      aussiebyte, aussiebyte, aussiebyte_state, empty_init, "SME Systems", "Aussie Byte II", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
