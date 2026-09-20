// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    TODO:

	- boot is waiting for $c0 input
    - keyboard
    - floppy

*/


#include "emu.h"
#include "prof180x.h"

uint8_t prof180x_state::read(offs_t offset)
{
	if (offset < 0x4000)
	{
		switch ((m_mm0 << 1) | m_mm1)
		{
		case 0:
			return memregion(HD64180_TAG)->base()[offset];

		case 1:
		case 3:
			return m_ram->read(offset);

		case 2:
			return 0xff;
		}
	}
	else if (offset < 0x8000)
	{
		if (((m_mm0 << 1) | m_mm1) != 3)
			return m_ram->read(offset);
		return 0xff;
	}
	else if (offset < m_ram->size())
	{
		return m_ram->read(offset);
	}
	else if (offset >= 0x40000 && offset < (0x40000 + m_ram->size()))
	{
		return m_ram->read(offset - 0x40000);
	}

	return 0xff;
}

void prof180x_state::write(offs_t offset, uint8_t data)
{
	if (offset < 0x4000)
	{
		if (((m_mm0 << 1) | m_mm1) != 2)
			m_ram->write(offset, data);
	}
	else if (offset < 0x8000)
	{
		if (((m_mm0 << 1) | m_mm1) != 3)
			m_ram->write(offset, data);
	}
	else if (offset < m_ram->size())
	{
		m_ram->write(offset, data);
	}
	else if (offset >= 0x40000 && offset < (0x40000 + m_ram->size()))
	{
		m_ram->write(offset - 0x40000, data);
	}
}

void prof180x_state::c0_flag_w(int state)
{
	// C0 (DATA)
	m_c0 = state;
	if (m_mk3835)
		m_mk3835->io_w(state);
	if (m_pcf8583)
		m_pcf8583->sda_w(state);
}

void prof180x_state::c1_flag_w(int state)
{
	// C1 (M0)
	m_c1 = state;
	if (m_mk3835)
		m_mk3835->sclk_w(state);
}

void prof180x_state::c2_flag_w(int state)
{
	// C2 (M1)
	m_c2 = state;
}

void prof180x_state::mini_flag_w(int state)
{
}

void prof180x_state::mm0_flag_w(int state)
{
	m_mm0 = state;
}

void prof180x_state::peps_flag_w(int state)
{
}

void prof180x_state::mm1_flag_w(int state)
{
	m_mm1 = state;
}

uint8_t prof180x_state::status0_r()
{
	/*

	    bit     description

	    0       BUSY
	    1
	    2
	    3
	    4       B-E
	    5       IDX
	    6
	    7       MOT

	*/

	return 0;
}

uint8_t prof180x_state::status1_r()
{
	/*

	    bit     description

	    0       FREE
	    1
	    2
	    3
	    4       J18
	    5       J19
	    6
	    7       TDO

	*/

	return !(m_mk3835 ? m_mk3835->io_r() : m_pcf8583->sda_r()) << 7;
}

uint8_t prof180x_state::status_r(offs_t offset)
{
	return BIT(offset, 8) ? status1_r() : status0_r();
}

void prof180x_state::prof180x_mem(address_map &map)
{
	map(0x00000, 0x7ffff).rw(FUNC(prof180x_state::read), FUNC(prof180x_state::write));
}

void prof180x_state::prof180x_io(address_map &map)
{
	map(0x0000, 0x003f).noprw(); // Z180 internal registers
	map(0x00d8, 0x00d8).mirror(0xff00).w("syslatch", FUNC(ls259_device::write_nibble_d0));
	map(0x00d9, 0x00d9).select(0xff00).r(FUNC(prof180x_state::status_r));
	map(0x00da, 0x00da).mirror(0xff00).rw(FDC9268_TAG, FUNC(upd765a_device::dma_r), FUNC(upd765a_device::dma_w));
	map(0x00db, 0x00db).mirror(0xff00).w("cent_data_out", FUNC(output_latch_device::write));
	map(0x00dc, 0x00dd).mirror(0xff00).m(FDC9268_TAG, FUNC(upd765a_device::map));
}

static INPUT_PORTS_START( prof180x )
INPUT_PORTS_END

static void prof180x_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END

void prof180x_state::machine_start()
{
	// register for state saving
	save_item(NAME(m_c0));
	save_item(NAME(m_c1));
	save_item(NAME(m_c2));
	save_item(NAME(m_mm0));
	save_item(NAME(m_mm1));
}

void prof180x_state::prof180x_base(machine_config &config)
{
	/* basic machine hardware */
	z180_device &maincpu(HD64180RP(config, HD64180_TAG, XTAL(18'432'000)));
	maincpu.set_addrmap(AS_PROGRAM, &prof180x_state::prof180x_mem);
	maincpu.set_addrmap(AS_IO, &prof180x_state::prof180x_io);
	maincpu.txa0_wr_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	maincpu.rts0_wr_callback().set("rs232a", FUNC(rs232_port_device::write_rts));
	maincpu.txa1_wr_callback().set("rs232b", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, "terminal"));
	rs232a.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
	rs232a.rxd_handler().set(maincpu, FUNC(z180_device::rxa0_w));
	rs232a.dcd_handler().set(maincpu, FUNC(z180_device::dcd0_w));
	rs232a.cts_handler().set(maincpu, FUNC(z180_device::cts0_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(maincpu, FUNC(z180_device::rxa1_w));
	rs232b.cts_handler().set(maincpu, FUNC(z180_device::rxs_cts1_w));

	ls259_device &syslatch(LS259(config, "syslatch")); // Z41
	syslatch.q_out_cb<0>().set(FUNC(prof180x_state::c0_flag_w));
	syslatch.q_out_cb<1>().set(FUNC(prof180x_state::c1_flag_w));
	syslatch.q_out_cb<2>().set(FUNC(prof180x_state::c2_flag_w));
	syslatch.q_out_cb<3>().set(FUNC(prof180x_state::mini_flag_w));
	syslatch.q_out_cb<4>().set(FUNC(prof180x_state::mm0_flag_w));
	syslatch.q_out_cb<5>().set([this] (int state) {
		if (m_mk3835)
			m_mk3835->ce_w(!state);
		if (m_pcf8583)
			m_pcf8583->scl_w(!state);
	});
	syslatch.q_out_cb<6>().set(FUNC(prof180x_state::peps_flag_w));
	syslatch.q_out_cb<7>().set(FUNC(prof180x_state::mm1_flag_w));

	/* devices */
	UPD765A(config, FDC9268_TAG, XTAL(8'000'000), false, true);
	FLOPPY_CONNECTOR(config, FDC9268_TAG ":0", prof180x_floppies, "35dd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, FDC9268_TAG ":1", prof180x_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, FDC9268_TAG ":2", prof180x_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, FDC9268_TAG ":3", prof180x_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");

	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(latch);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("128K").set_extra_options("256K,512K");

	/* software lists */
	SOFTWARE_LIST(config, "flop_list").set_original("prof180");
}

void prof180x_state::prof180x(machine_config &config)
{
	prof180x_base(config);
	MK3835(config, m_mk3835);
}

void prof180x_state::prof181x(machine_config &config)
{
	prof180x_base(config);
	PCF8583(config, m_pcf8583, XTAL(32'768)).set_a0(1);
}

ROM_START( prof180x )
	ROM_REGION( 0x10000, HD64180_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "pmon13", "pmon v1.3" )
	ROMX_LOAD( "pmon1_3.z16", 0x00000, 0x04000, CRC(32986688) SHA1(a6229d7e66ef699722ca3d41179fe3f1b75185d4), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "pmon14", "pmon v1.4" )
	ROMX_LOAD( "pmon1_4.z16", 0x00000, 0x04000, CRC(ed03f49f) SHA1(e016f9e0b89ab64c6203e2e46501d0b09f74ee9b), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "eboot1", "eboot1" )
	ROMX_LOAD( "eboot1.z16", 0x00000, 0x08000, CRC(7a164b3c) SHA1(69367804b5cbc0633e3d7bbbcc256c2c8c9e7aca), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "epmon1", "epmon1" )
	ROMX_LOAD( "epmon1.z16", 0x00000, 0x10000, CRC(27aabfb4) SHA1(41adf038c474596dbf7d387a1a7f33ed86aa7869), ROM_BIOS(3) )

	ROM_REGION( 0x157, "plds", 0 )
	ROM_LOAD( "pal14l8.z10", 0x000, 0x157, NO_DUMP )
ROM_END

ROM_START( prof181x )
	ROM_REGION( 0x10000, HD64180_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "pmon15", "pmon v1.5" )
	ROMX_LOAD( "pmon1_5.u13", 0x00000, 0x04000, CRC(f43d185c) SHA1(a7a219b3d48c74602b3116cfcd34e44d6e7bc423), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "pmon", "pmon" )
	ROMX_LOAD( "pmon.u13",    0x00000, 0x04000, CRC(4f3732d7) SHA1(7dc27262db4e0c8f109470253b9364a216909f2c), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "eboot2", "eboot2" )
	ROMX_LOAD( "eboot2.u13",  0x00000, 0x08000, CRC(0c2d4301) SHA1(f1a4f457e287b19e14d8ccdbc0383f183d8a3efe), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "epmon2", "epmon2" )
	ROMX_LOAD( "epmon2.u13",  0x00000, 0x10000, CRC(3b8a7b59) SHA1(33741f0725e3eaa21c6881c712579b3c1fd30607), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "epmon3", "epmon3" )
	ROMX_LOAD( "epmon3.u13",  0x00000, 0x10000, CRC(51313af1) SHA1(60c293171a1c7cb9a5ff6d681e61894f44fddbd1), ROM_BIOS(4) )

	ROM_REGION( 0x157, "plds", 0 ) // converted from JED files
	ROM_LOAD( "pal20v8.u14", 0x000, 0x157, CRC(46da52b0) SHA1(c11362223c0d5c57c6ef970e66d674b89d8e7784) )
	ROM_LOAD( "pal20v8.u15", 0x000, 0x157, CRC(19fef936) SHA1(579ad23ee3c0b1c64c584383f9c8085c6ce3d094) )
	ROM_LOAD( "pal20v8.u19", 0x000, 0x157, CRC(69348c3b) SHA1(6eb8432660eb9b639a95b1973a54dab8b99f10ef) )
	ROM_LOAD( "pal20v8.u21", 0x000, 0x157, CRC(6df4e281) SHA1(602fa4637cd9356acc31b2adfb3a084fd5a0bfcb) )
ROM_END

/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY                 FULLNAME     FLAGS */
COMP( 1986, prof180x, 0,        0,      prof180x, prof180x, prof180x_state, empty_init, "Conitec Datensysteme", "PROF-180X", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1992, prof181x, prof180x, 0,      prof181x, prof180x, prof180x_state, empty_init, "Conitec Datensysteme", "PROF-181X", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
