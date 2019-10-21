// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, MetalliC
/***************************************************************************

        Vector06c driver by Miodrag Milanovic

        10/07/2008 Preliminary driver.

boot from ROM cart:
 hold F2 then system reset (press F11), then press F12

boot from FDD:
 press F12 after initial boot was load (indicated in screen lower part)
 hold Ctrl ("YC" key) during MicroDOS start to format RAM disk (required by some games)

TODO:
 - correct CPU speed / latency emulation, each machine cycle takes here 4 clocks,
   i.e. INX B 4+1 will be 2*4=8clocks, SHLD addr is 4+3+3+3+3 so it will be 5*4=20clocks and so on
 - "Card Game" wont work, jump to 0 instead of vblank interrupt RST7, something direct.explicit or banking related ?
 - border emulaton
 - separate base unexpanded Vector06C configuration
 - slotify AY8910 sound boards ?

****************************************************************************/

#include "emu.h"
#include "includes/vector06.h"

#include "formats/vector06_dsk.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

/* Address maps */
void vector06_state::vector06_mem(address_map &map)
{
	map(0x0000, 0xffff).bankrw("bank1");
	map(0x0000, 0x7fff).bankr("bank2");
	map(0xa000, 0xdfff).bankrw("bank3");
}

void vector06_state::vector06_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();

	map(0x00, 0x03).lrw8("ppi8255_rw", [this](offs_t offset) -> u8 { return m_ppi8255->read(offset^3); }, [this](offs_t offset, u8 data) { m_ppi8255->write(offset^3, data); });
	map(0x04, 0x07).lrw8("ppi8255_2_rw", [this](offs_t offset) -> u8 { return m_ppi8255_2->read(offset^3); }, [this](offs_t offset, u8 data) { m_ppi8255_2->write(offset^3, data); });
	map(0x08, 0x0b).lrw8("pit8253_rw", [this](offs_t offset) -> u8 { return m_pit8253->read(offset^3); }, [this](offs_t offset, u8 data) { m_pit8253->write(offset^3, data); });
	map(0x0c, 0x0c).w(FUNC(vector06_state::vector06_color_set));
	map(0x10, 0x10).w(FUNC(vector06_state::vector06_ramdisk_w));
	map(0x14, 0x15).rw(m_ay, FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_address_w));
	map(0x18, 0x18).rw(m_fdc, FUNC(kr1818vg93_device::data_r), FUNC(kr1818vg93_device::data_w));
	map(0x19, 0x19).rw(m_fdc, FUNC(kr1818vg93_device::sector_r), FUNC(kr1818vg93_device::sector_w));
	map(0x1a, 0x1a).rw(m_fdc, FUNC(kr1818vg93_device::track_r), FUNC(kr1818vg93_device::track_w));
	map(0x1b, 0x1b).rw(m_fdc, FUNC(kr1818vg93_device::status_r), FUNC(kr1818vg93_device::cmd_w));
	map(0x1c, 0x1c).w(FUNC(vector06_state::vector06_disc_w));
}

/* Input ports */
static INPUT_PORTS_START( vector06 )
	PORT_START("LINE.0")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_DEL)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BkSp") PORT_CODE(KEYCODE_BACKSPACE)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_START("LINE.1")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Home") PORT_CODE(KEYCODE_HOME)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PgUp") PORT_CODE(KEYCODE_PGUP)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)
	PORT_START("LINE.2")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_START("LINE.3")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("'") PORT_CODE(KEYCODE_INSERT)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("=") PORT_CODE(KEYCODE_EQUALS)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH)
	PORT_START("LINE.4")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_QUOTE)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
	PORT_START("LINE.5")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
	PORT_START("LINE.6")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
	PORT_START("LINE.7")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("~") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
	PORT_START("LINE.8")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Rus/Lat") PORT_CODE(KEYCODE_LALT)
	PORT_START("RESET")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F11)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset 2") PORT_CODE(KEYCODE_F12)

INPUT_PORTS_END


FLOPPY_FORMATS_MEMBER( vector06_state::floppy_formats )
	FLOPPY_VECTOR06_FORMAT
FLOPPY_FORMATS_END

static void vector06_floppies(device_slot_interface &device)
{
	device.option_add("qd", FLOPPY_525_QD);
}


/* Machine driver */
void vector06_state::vector06(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, 3000000); // actual speed is wrong due to unemulated latency
	m_maincpu->set_addrmap(AS_PROGRAM, &vector06_state::vector06_mem);
	m_maincpu->set_addrmap(AS_IO, &vector06_state::vector06_io);
	m_maincpu->out_status_func().set(FUNC(vector06_state::vector06_status_callback));
	m_maincpu->set_vblank_int("screen", FUNC(vector06_state::vector06_interrupt));
	m_maincpu->set_irq_acknowledge_callback(FUNC(vector06_state::vector06_irq_callback));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(256+64, 256+64);
	m_screen->set_visarea(0, 256+64-1, 0, 256+64-1);
	m_screen->set_screen_update(FUNC(vector06_state::screen_update_vector06));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, palette_device::BLACK, 16);

	SPEAKER(config, "mono").front_center();

	/* devices */
	I8255(config, m_ppi8255);
	m_ppi8255->in_pb_callback().set(FUNC(vector06_state::vector06_8255_portb_r));
	m_ppi8255->in_pc_callback().set(FUNC(vector06_state::vector06_8255_portc_r));
	m_ppi8255->out_pa_callback().set(FUNC(vector06_state::vector06_8255_porta_w));
	m_ppi8255->out_pb_callback().set(FUNC(vector06_state::vector06_8255_portb_w));

	I8255(config, m_ppi8255_2);
	m_ppi8255_2->in_pb_callback().set(FUNC(vector06_state::vector06_romdisk_portb_r));
	m_ppi8255_2->out_pa_callback().set(FUNC(vector06_state::vector06_romdisk_porta_w));
	m_ppi8255_2->out_pb_callback().set(FUNC(vector06_state::vector06_romdisk_portb_w));
	m_ppi8255_2->out_pc_callback().set(FUNC(vector06_state::vector06_romdisk_portc_w));

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	KR1818VG93(config, m_fdc, 1_MHz_XTAL);

	FLOPPY_CONNECTOR(config, "wd1793:0", vector06_floppies, "qd", vector06_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "wd1793:1", vector06_floppies, "qd", vector06_state::floppy_formats);
	SOFTWARE_LIST(config, "flop_list").set_original("vector06_flop");

	/* cartridge */
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "vector06_cart", "bin,emr");
	SOFTWARE_LIST(config, "cart_list").set_original("vector06_cart");

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("320K").set_default_value(0);

	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	PIT8253(config, m_pit8253, 0);
	m_pit8253->set_clk<0>(1500000);
	m_pit8253->set_clk<1>(1500000);
	m_pit8253->set_clk<2>(1500000);
	m_pit8253->out_handler<0>().set(FUNC(vector06_state::speaker_w));
	m_pit8253->out_handler<1>().set(FUNC(vector06_state::speaker_w));
	m_pit8253->out_handler<2>().set(FUNC(vector06_state::speaker_w));

	// optional
	AY8910(config, m_ay, 1773400).add_route(ALL_OUTPUTS, "mono", 0.50);
}

/* ROM definition */

ROM_START( vector06 )
	ROM_REGION( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "unboot32k", "Universal Boot 32K")
	ROMX_LOAD( "unboot32k.rt", 0x10000, 0x8000, CRC(28c9b5cd) SHA1(8cd7fb658896a7066ae93b10eaafa0f12139ad81), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "unboot2k", "Universal Boot 2K")
	ROMX_LOAD( "unboot2k.rt",  0x10000, 0x0800, CRC(4c80dc31) SHA1(7e5e3acfdbea2e52b0d64c5868821deaec383815), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "coman", "Boot Coman")
	ROMX_LOAD( "coman.rt",     0x10000, 0x0800, CRC(f8c4a85a) SHA1(47fa8b02f09a1d06aa63a2b90b2597b1d93d976f), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "bootbyte", "Boot Byte")
	ROMX_LOAD( "bootbyte.rt",  0x10000, 0x0800, CRC(3b42fd9d) SHA1(a112f4fe519bc3dbee85b09040d4804a17c9eda2), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "bootos", "Boot OS")
	ROMX_LOAD( "bootos.rt",    0x10000, 0x0200, CRC(46bef038) SHA1(6732f4a360cd38112c53c458842d31f5b035cf59), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "boot512", "Boot 512")
	ROMX_LOAD( "boot512.rt",   0x10000, 0x0200, CRC(a0b1c6b2) SHA1(f6fe15cb0974aed30f9b7aa72133324a66d1ed3f), ROM_BIOS(5))
ROM_END

ROM_START( vec1200 )
	ROM_REGION( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "vec1200.bin", 0x10000, 0x2000, CRC(37349224) SHA1(060fbb2c1a89040c929521cfd58cb6f1431a8b75))

	ROM_REGION( 0x0200, "palette", 0 )
	ROM_LOAD( "palette.bin", 0x0000, 0x0200, CRC(74b7376b) SHA1(fb56b60babd7e6ed68e5f4e791ad2800d7ef6729))
ROM_END

ROM_START( pk6128c )
	ROM_REGION( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "6128.bin", 0x10000, 0x4000, CRC(d4f68433) SHA1(ef5ac75f9240ca8996689c23642d4e47e5e774d8))
ROM_END

ROM_START( krista2 )
	ROM_REGION( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "krista2.epr", 0x10000, 0x0200, CRC(df5440b0) SHA1(bcbbb3cc10aeb17c1262b45111d20279266b9ba4))
	ROM_LOAD( "krista2.pal", 0x0000, 0x0200, CRC(b243da33) SHA1(9af7873e6f8bf452c8d831833ffb02dce833c095))
ROM_END
/* Driver */

/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY      FULLNAME       FLAGS */
COMP( 1987, vector06, 0,        0,      vector06, vector06, vector06_state, empty_init, "<unknown>", "Vector 06c",  0)
COMP( 1987, vec1200,  vector06, 0,      vector06, vector06, vector06_state, empty_init, "<unknown>", "Vector 1200", MACHINE_NOT_WORKING)
COMP( 1987, pk6128c,  vector06, 0,      vector06, vector06, vector06_state, empty_init, "<unknown>", "PK-6128c",    MACHINE_NOT_WORKING)
COMP( 1987, krista2,  vector06, 0,      vector06, vector06, vector06_state, empty_init, "<unknown>", "Krista-2",    MACHINE_NOT_WORKING)
