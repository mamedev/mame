// license:BSD-3-Clause
// copyright-holders:Phill Harvey-Smith, Carl
/*
    drivers/rmnimbus.c

    Research machines Nimbus.

    2009-11-14, P.Harvey-Smith.

*/

#include "emu.h"
#include "rmnimbus.h"
#include "rmnkbd.h"

#include "cpu/mcs51/mcs51.h"
#include "imagedev/floppy.h"

#include "bus/isa/fdc.h"
#include "bus/rs232/rs232.h"
#include "bus/scsi/acb4070.h"
#include "bus/scsi/s1410.h"
#include "bus/scsi/scsihd.h"

#include "softlist.h"
#include "speaker.h"


static void rmnimbus_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

static void keyboard(device_slot_interface &device)
{
	device.option_add("rmnkbd", RMNIMBUS_KEYBOARD);
}

void rmnimbus_state::nimbus_mem(address_map &map)
{
	map(0x00000, 0x1FFFF).bankrw(RAM_BANK00_TAG);
	map(0x20000, 0x3FFFF).bankrw(RAM_BANK01_TAG);
	map(0x40000, 0x5FFFF).bankrw(RAM_BANK02_TAG);
	map(0x60000, 0x7FFFF).bankrw(RAM_BANK03_TAG);
	map(0x80000, 0x9FFFF).bankrw(RAM_BANK04_TAG);
	map(0xA0000, 0xBFFFF).bankrw(RAM_BANK05_TAG);
	map(0xC0000, 0xDFFFF).bankrw(RAM_BANK06_TAG);
	map(0xE0000, 0xEFFFF).bankrw(RAM_BANK07_TAG);
	map(0xF0000, 0xFFFFF).rom().region(MAINCPU_TAG, 0x0f0000);
}

void rmnimbus_state::nimbus_io(address_map &map)
{
	map(0x0000, 0x0031).rw(FUNC(rmnimbus_state::nimbus_video_io_r), FUNC(rmnimbus_state::nimbus_video_io_w));
	map(0x0080, 0x0080).rw(FUNC(rmnimbus_state::nimbus_mcu_r), FUNC(rmnimbus_state::nimbus_mcu_w));
	map(0x0092, 0x0092).rw(FUNC(rmnimbus_state::nimbus_iou_r), FUNC(rmnimbus_state::nimbus_iou_w));
	map(0x00a0, 0x00a0).r(FUNC(rmnimbus_state::nimbus_joystick_r));
	map(0x00a0, 0x00a3).w(FUNC(rmnimbus_state::nimbus_joystick_select));
	map(0x00a4, 0x00a4).rw(FUNC(rmnimbus_state::nimbus_mouse_js_r), FUNC(rmnimbus_state::nimbus_mouse_js_w));
	map(0x00c0, 0x00cf).rw(FUNC(rmnimbus_state::nimbus_pc8031_r), FUNC(rmnimbus_state::nimbus_pc8031_w)).umask16(0x00ff);
	map(0x00e0, 0x00ef).rw(AY8910_TAG, FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w)).umask16(0x00ff);
	map(0x00f0, 0x00f7).rw(m_z80sio, FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w)).umask16(0x00ff);
	map(0x0400, 0x0400).w(FUNC(rmnimbus_state::fdc_ctl_w));
	map(0x0408, 0x040f).rw(m_fdc, FUNC(wd2793_device::read), FUNC(wd2793_device::write)).umask16(0x00ff);
	map(0x0410, 0x041f).rw(FUNC(rmnimbus_state::scsi_r), FUNC(rmnimbus_state::scsi_w)).umask16(0x00ff);
	map(0x0480, 0x049f).m(m_via, FUNC(via6522_device::map)).umask16(0x00ff);
}


static INPUT_PORTS_START( nimbus )
	PORT_START("config")
	PORT_CONFNAME( 0x01, 0x01, "Mouse mode" )
	PORT_CONFSETTING( 0x00, "Emulate incremental encoders" )
	PORT_CONFSETTING( 0x01, "Simulate BIOS handler" )

	PORT_START(JOYSTICK0_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START(JOYSTICK1_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START(MOUSE_BUTTON_TAG)  /* Mouse buttons */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Mouse Button 2") PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Mouse Button 1") PORT_CODE(MOUSECODE_BUTTON1)

	PORT_START(MOUSEX_TAG) /* Mouse - X AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START(MOUSEY_TAG) /* Mouse - Y AXIS */
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

INPUT_PORTS_END

void rmnimbus_state::nimbus_iocpu_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
}

void rmnimbus_state::nimbus_iocpu_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x000FF).rw(FUNC(rmnimbus_state::nimbus_pc8031_iou_r), FUNC(rmnimbus_state::nimbus_pc8031_iou_w));
}

void rmnimbus_state::nimbus(machine_config &config)
{
	/* basic machine hardware */
	I80186(config, m_maincpu, 16000000); // the cpu is a 10Mhz part but the serial clocks are wrong unless it runs at 8Mhz
	m_maincpu->set_addrmap(AS_PROGRAM, &rmnimbus_state::nimbus_mem);
	m_maincpu->set_addrmap(AS_IO, &rmnimbus_state::nimbus_io);
	m_maincpu->read_slave_ack_callback().set(FUNC(rmnimbus_state::cascade_callback));
	m_maincpu->tmrout0_handler().set(m_z80sio, FUNC(z80sio_device::rxca_w));
	m_maincpu->tmrout1_handler().set(m_z80sio, FUNC(z80sio_device::rxtxcb_w));

	I8031(config, m_iocpu, 11059200);
	m_iocpu->set_addrmap(AS_PROGRAM, &rmnimbus_state::nimbus_iocpu_mem);
	m_iocpu->set_addrmap(AS_IO, &rmnimbus_state::nimbus_iocpu_io);
	m_iocpu->port_in_cb<1>().set(FUNC(rmnimbus_state::nimbus_pc8031_port1_r));
	m_iocpu->port_out_cb<1>().set(FUNC(rmnimbus_state::nimbus_pc8031_port1_w));
	m_iocpu->port_in_cb<3>().set(FUNC(rmnimbus_state::nimbus_pc8031_port3_r));
	m_iocpu->port_out_cb<3>().set(FUNC(rmnimbus_state::nimbus_pc8031_port3_w));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(4.433619_MHz_XTAL * 2, 650, 0, 640, 260, 0, 250);
	m_screen->set_screen_update(FUNC(rmnimbus_state::screen_update_nimbus));
	//m_screen->set_video_attributes(VIDEO_UPDATE_SCANLINE);
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(16);

	/* Backing storage */
	WD2793(config, m_fdc, 2000000);
	m_fdc->set_force_ready(true);
	m_fdc->intrq_wr_callback().set(FUNC(rmnimbus_state::nimbus_fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(rmnimbus_state::nimbus_fdc_drq_w));
	m_fdc->enmf_rd_callback().set(FUNC(rmnimbus_state::nimbus_fdc_enmf_r));
	FLOPPY_CONNECTOR(config, FDC_TAG":0", rmnimbus_floppies, "35dd", isa8_fdc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, FDC_TAG":1", rmnimbus_floppies, "35dd", isa8_fdc_device::floppy_formats);

	SCSI_PORT(config, m_scsibus);
	m_scsibus->set_data_input_buffer("scsi_data_in");
	m_scsibus->msg_handler().set(FUNC(rmnimbus_state::write_scsi_msg));
	m_scsibus->bsy_handler().set(FUNC(rmnimbus_state::write_scsi_bsy));
	m_scsibus->io_handler().set(FUNC(rmnimbus_state::write_scsi_io));
	m_scsibus->cd_handler().set(FUNC(rmnimbus_state::write_scsi_cd));
	m_scsibus->req_handler().set(FUNC(rmnimbus_state::write_scsi_req));

	m_scsibus->set_slot_device(1, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_0));
	m_scsibus->set_slot_device(2, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_1));
	m_scsibus->set_slot_device(3, "harddisk", ACB4070, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_2));
	m_scsibus->set_slot_device(4, "harddisk", S1410, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_3));

	OUTPUT_LATCH(config, m_scsi_data_out);
	m_scsibus->set_output_latch(*m_scsi_data_out);

	INPUT_BUFFER(config, m_scsi_data_in);

	OUTPUT_LATCH(config, m_scsi_ctrl_out);
	m_scsi_ctrl_out->bit_handler<0>().set(m_scsibus, FUNC(scsi_port_device::write_rst));
	m_scsi_ctrl_out->bit_handler<1>().set(m_scsibus, FUNC(scsi_port_device::write_sel));
	m_scsi_ctrl_out->bit_handler<2>().set(FUNC(rmnimbus_state::write_scsi_iena));

	RAM(config, m_ram);
	m_ram->set_default_size("1536K");
	m_ram->set_extra_options("128K,256K,384K,512K,640K,1024K");

	/* Peripheral chips */
	Z80SIO(config, m_z80sio, 4000000); // Z0844006PSC (SIO/0)
	m_z80sio->out_txdb_callback().set("rs232b", FUNC(rs232_port_device::write_txd));
	m_z80sio->out_dtrb_callback().set("rs232b", FUNC(rs232_port_device::write_dtr));
	m_z80sio->out_rtsb_callback().set("rs232b", FUNC(rs232_port_device::write_rts));
	m_z80sio->out_int_callback().set(FUNC(rmnimbus_state::sio_interrupt));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", keyboard, "rmnkbd"));
	rs232a.rxd_handler().set(m_z80sio, FUNC(z80sio_device::rxa_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_z80sio, FUNC(z80sio_device::rxb_w));
	rs232b.dcd_handler().set(m_z80sio, FUNC(z80sio_device::dcdb_w));
	rs232b.ri_handler().set(m_z80sio, FUNC(z80sio_device::syncb_w));
	rs232b.cts_handler().set(m_z80sio, FUNC(z80sio_device::ctsb_w));

	EEPROM_93C06_16BIT(config, m_eeprom);

	MOS6522(config, m_via, 1000000);
	m_via->writepa_handler().set("cent_data_out", FUNC(output_latch_device::write));
	m_via->writepb_handler().set(FUNC(rmnimbus_state::nimbus_via_write_portb));
	m_via->ca2_handler().set(m_centronics, FUNC(centronics_device::write_strobe));
	m_via->irq_handler().set(m_maincpu, FUNC(i80186_cpu_device::int3_w));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(m_via, FUNC(via6522_device::write_ca1)).invert();

	output_latch_device &cent_data(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data);

	/* sound hardware */
	SPEAKER(config, MONO_TAG).front_center();
	ay8910_device &ay8910(AY8910(config, AY8910_TAG, 2000000));
	ay8910.port_a_write_callback().set(FUNC(rmnimbus_state::nimbus_sound_ay8910_porta_w));
	ay8910.port_b_write_callback().set(FUNC(rmnimbus_state::nimbus_sound_ay8910_portb_w));
	ay8910.add_route(ALL_OUTPUTS, MONO_TAG, 0.75);

	msm5205_device &msm5205(MSM5205(config, MSM5205_TAG, 384000));
	msm5205.vck_callback().set(FUNC(rmnimbus_state::nimbus_msm5205_vck)); /* VCK function */
	msm5205.set_prescaler_selector(msm5205_device::S48_4B);      /* 8 kHz */
	msm5205.add_route(ALL_OUTPUTS, MONO_TAG, 0.75);

	SOFTWARE_LIST(config, "disk_list").set_original("nimbus");

	m_maincpu->set_dasm_override(FUNC(rmnimbus_state::dasm_override));
}

/*
Known unavailable BIOS set:
    Another version of v1.31a labelled: SYS1 16128 31/10/86, SYS2 16129 28/10/86
    Another version of v1.32c labelled: SYS1 17130 10.03.87, SYS 2. 17131 09.03.87
    Another version of v1.32c labelled: SYS 1 21323 2/12/88, SYS 2 21324 5/1/89
    v1.33c labelled "RESEARCH MACHINE P.N. 32857(32858) PC186 SYS1(SYS2) V1.33C", no date labelled
    Another version of v1.40d labelled: 24693 IC30 02/7/91(AA), 24694 IC27 28/6/91(AA)
*/
ROM_START( nimbus )
	ROM_REGION( 0x100000, MAINCPU_TAG, 0 )

	ROM_SYSTEM_BIOS(0, "v125a", "Nimbus BIOS v1.25a (1985-12-02)")
	ROMX_LOAD("sys1-1.25a_13484_6-11-85_m5m27256p.rom", 0xf0001, 0x8000, CRC(5870df28) SHA1(12e1a7d22439d512b221c355d641d113f0e6568e), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("sys2-1.25a_13485_2-12-85_m5m27256p.rom", 0xf0000, 0x8000, CRC(15888320) SHA1(32cc2485468c6a9944e505162e319a283eef8a84), ROM_SKIP(1) | ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "v131a", "Nimbus BIOS v1.31a (1986-06-18)")
	ROMX_LOAD("sys1-1.31a-16128-1986-06-18.rom", 0xf0001, 0x8000, CRC(6416eb05) SHA1(1b640163a7efbc24381c7b24976a8609c066959b), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("sys2-1.31a-16129-1986-06-18.rom", 0xf0000, 0x8000, CRC(b224359d) SHA1(456bbe37afcd4429cca76ba2d6bd534dfda3fc9c), ROM_SKIP(1) | ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "v132c", "Nimbus BIOS v1.32c (1987-03-21)")
	ROMX_LOAD("sys1-1.32c_17130_21-3-87_m5m27256p_63210c.rom", 0xf0001, 0x8000, CRC(91b88b29) SHA1(1db5e3ff8bb237f006b53d5f91b080f99e95a58e), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("sys2-1.32c_17131_21-3-87_m5m27256p_63210c.rom", 0xf0000, 0x8000, CRC(4876cc4f) SHA1(4348b293f5c443453c08663809bfa5c462222137), ROM_SKIP(1) | ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "v132d", "Nimbus BIOS v1.32d (1989-01-20)")
	ROMX_LOAD("sys1-1.32d_21323_17-1-89_m5m27256p_63210c.rom", 0xf0001, 0x8000, CRC(e0ecbc02) SHA1(b5cb16df23bd30af5556660364e4733790f99164), ROM_SKIP(1) | ROM_BIOS(3))
	ROMX_LOAD("sys2-1.32d_21324_20-1-89_m5m27256p_63210c.rom", 0xf0000, 0x8000, CRC(8ef4a357) SHA1(29309cb8bfe9256d4684f3e6575e3720b0dcacd4), ROM_SKIP(1) | ROM_BIOS(3))

	ROM_SYSTEM_BIOS(4, "v132f", "Nimbus BIOS v1.32f (1989-11-29)")
	ROMX_LOAD("sys-1-1.32f-22779-1989-11-29.rom", 0xf0001, 0x8000, CRC(786c31e8) SHA1(da7f828f7f96087518bea1a3d89fee59b283b4ba), ROM_SKIP(1) | ROM_BIOS(4))
	ROMX_LOAD("sys-2-1.32f-22778-1989-11-29.rom", 0xf0000, 0x8000, CRC(0be3db64) SHA1(af806405ec6fbc20385705f90d5059a47de17b08), ROM_SKIP(1) | ROM_BIOS(4))

	ROM_SYSTEM_BIOS(5, "v140d", "Nimbus BIOS v1.40d (1990-02-19)")
	ROMX_LOAD("rt-1.40d-24694-ic27-1990-02-19.rom", 0xf0001, 0x8000, CRC(b8d3dc0b) SHA1(82e0dcdc6c7a83339af68d6cb61211fcb14bed88), ROM_SKIP(1) | ROM_BIOS(5))
	ROMX_LOAD("left-1.40d-24693-ic30-1990-02-15.rom", 0xf0000, 0x8000, CRC(b0826b0b) SHA1(3baa369a0e7ef138ca29aae0ee8a89ab670a02b9), ROM_SKIP(1) | ROM_BIOS(5))

	ROM_REGION( 0x4000, IOCPU_TAG, 0 )
	ROM_LOAD("hexec-v1.02u-13488-1985-10-29.rom", 0x0000, 0x1000, CRC(75c6adfd) SHA1(0f11e0b7386c6368d20e1fc7a6196d670f924825))

	ROM_REGION16_LE( 0x20, ER59256_TAG, 0 ) // default eeprom data
	ROM_LOAD("er59256", 0x00, 0x20, CRC(1a39de76) SHA1(0b6607f008dd92d6ab9af62b0b042fc3f5f4461c))
ROM_END

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS           INIT        COMPANY              FULLNAME  FLAGS
COMP( 1986, nimbus, 0,      0,      nimbus,  nimbus, rmnimbus_state, empty_init, "Research Machines", "Nimbus", 0)
