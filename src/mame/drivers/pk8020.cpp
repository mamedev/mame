// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, AJR
/***************************************************************************

        PK-8020 driver by Miodrag Milanovic
            based on work of Sergey Erokhin from pk8020.narod.ru

        18/07/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "includes/pk8020.h"

#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "machine/clock.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "formats/pk8020_dsk.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

/* Address maps */
void pk8020_state::pk8020_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(pk8020_state::memory_r), FUNC(pk8020_state::memory_w));
}

void pk8020_state::pk8020_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
}

void pk8020_state::devices_map(address_map &map)
{
	map(0x00, 0x03).mirror(4).rw("ct", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x08, 0x0b).mirror(4).rw("iop3", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x10, 0x11).mirror(6).rw("ios1", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x18, 0x1b).mirror(4).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	map(0x20, 0x21).mirror(6).rw("ios2", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x28, 0x29).mirror(6).rw(m_inr, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x30, 0x33).mirror(4).rw("iop2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x38, 0x3b).mirror(4).rw("iop1", FUNC(i8255_device::read), FUNC(i8255_device::write));
}

/* Input ports */
static INPUT_PORTS_START( pk8020 )
	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)

	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("_") PORT_CODE(KEYCODE_EQUALS)

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)

	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_SLASH)

	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STRN") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STOP") PORT_CODE(KEYCODE_F12)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("IZ") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("VZ") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)

	PORT_START("LINE7")
	// All keys in this line are reversed logic
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Alf") PORT_CODE(KEYCODE_F11)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Graf") PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Prf") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Sel") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Upr") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("OO") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)

	PORT_START("LINE8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 7") PORT_CODE(KEYCODE_7_PAD)

	PORT_START("LINE9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num .") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE11")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE12")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE13")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE14")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE15")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


/* F4 Character Displayer */
static const gfx_layout pk8020_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	512,                    /* 512 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_pk8020 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, pk8020_charlayout, 0, 4 )
GFXDECODE_END


FLOPPY_FORMATS_MEMBER( pk8020_state::floppy_formats )
	FLOPPY_PK8020_FORMAT
FLOPPY_FORMATS_END

static void pk8020_floppies(device_slot_interface &device)
{
	device.option_add("qd", FLOPPY_525_QD);
}

/*
 * interrupts
 *
 * 0    external devices
 * 1    uart rx ready
 * 2    uart tx ready
 * 3    lan
 * 4    vblank
 * 5    timer ch2
 * 6    printer
 * 7    floppy
 */
/* Machine driver */
void pk8020_state::pk8020(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, 20_MHz_XTAL / 8); // КР580ВМ80А
	m_maincpu->set_addrmap(AS_PROGRAM, &pk8020_state::pk8020_mem);
	m_maincpu->set_addrmap(AS_IO, &pk8020_state::pk8020_io);
	m_maincpu->set_vblank_int("screen", FUNC(pk8020_state::pk8020_interrupt));
	m_maincpu->set_irq_acknowledge_callback("inr", FUNC(pic8259_device::inta_cb));

	PLS100(config, m_decplm); // КР556РТ2 (82S100 equivalent; D31)

	ADDRESS_MAP_BANK(config, m_devbank);
	m_devbank->set_addrmap(0, &pk8020_state::devices_map);
	m_devbank->set_data_width(8);
	m_devbank->set_addr_width(6);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(20_MHz_XTAL / 2, 640, 0, 512, 312, 0, 256);
	screen.set_screen_update(FUNC(pk8020_state::screen_update_pk8020));
	screen.set_palette(m_palette);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_pk8020);
	PALETTE(config, m_palette, FUNC(pk8020_state::pk8020_palette), 16);

	i8255_device &iop1(I8255(config, "iop1")); // КР580ВВ55А (D17)
	iop1.in_pa_callback().set(FUNC(pk8020_state::ppi_porta_r));
	iop1.out_pb_callback().set(FUNC(pk8020_state::floppy_control_w));
	iop1.out_pc_callback().set(FUNC(pk8020_state::video_page_w));

	i8255_device &iop2(I8255(config, "iop2")); // КР580ВВ55А (D16)
	iop2.out_pc_callback().set(FUNC(pk8020_state::ppi_2_portc_w));

	I8255(config, "iop3"); // КР580ВВ55А (D2)

	pit8253_device &ct(PIT8253(config, "ct")); // КР580ВИ53
	ct.set_clk<0>(20_MHz_XTAL / 10);
	ct.out_handler<0>().set(FUNC(pk8020_state::pit_out0));
	ct.set_clk<1>(20_MHz_XTAL / 10);
	ct.out_handler<1>().set("ios1", FUNC(i8251_device::write_txc));
	ct.out_handler<1>().append("ios1", FUNC(i8251_device::write_rxc));
	ct.set_clk<2>((20_MHz_XTAL / 8) / 164);
	ct.out_handler<2>().set(m_inr, FUNC(pic8259_device::ir5_w));

	PIC8259(config, m_inr); // КР580ВН59
	m_inr->out_int_callback().set_inputline(m_maincpu, 0);

	i8251_device &ios1(I8251(config, "ios1", 20_MHz_XTAL / 10)); // КР580ВВ51А (D10)
	ios1.txd_handler().set("v24", FUNC(rs232_port_device::write_txd));
	ios1.dtr_handler().set("v24", FUNC(rs232_port_device::write_rts));
	ios1.rxrdy_handler().set(m_inr, FUNC(pic8259_device::ir1_w));
	ios1.txrdy_handler().set(m_inr, FUNC(pic8259_device::ir2_w));

	rs232_port_device &v24(RS232_PORT(config, "v24", default_rs232_devices, nullptr));
	v24.rxd_handler().set("ios1", FUNC(i8251_device::write_rxd));
	v24.dsr_handler().set("ios1", FUNC(i8251_device::write_dsr));

	i8251_device &ios2(I8251(config, "ios2", 20_MHz_XTAL / 10)); // КР580ВВ51А (D11)
	ios2.txd_handler().set("line", FUNC(rs232_port_device::write_txd));
	ios2.rxrdy_handler().set(m_inr, FUNC(pic8259_device::ir3_w));

	rs232_port_device &line(RS232_PORT(config, "line", default_rs232_devices, nullptr));
	line.rxd_handler().set("ios2", FUNC(i8251_device::write_rxd));

	clock_device &c1(CLOCK(config, "c1", 20_MHz_XTAL / 8 / 4));
	c1.signal_handler().set("ios2", FUNC(i8251_device::write_txc));
	c1.signal_handler().append("ios2", FUNC(i8251_device::write_rxc));

	KR1818VG93(config, m_fdc, 20_MHz_XTAL / 20); // КР1818ВГ93
	m_fdc->intrq_wr_callback().set(m_inr, FUNC(pic8259_device::ir7_w));

	for (auto &floppy : m_floppy)
		FLOPPY_CONNECTOR(config, floppy, pk8020_floppies, "qd", pk8020_state::floppy_formats);

	SOFTWARE_LIST(config, "flop_list").set_original("korvet_flop");

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.25);
	WAVE(config, "wave", "cassette").add_route(ALL_OUTPUTS, "mono", 0.25);

	CASSETTE(config, "cassette").set_default_state(CASSETTE_PLAY);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("258K").set_default_value(0x00); // 64 + 4*48 + 2 = 258
}

/* ROM definition */

ROM_START( korvet )
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_DEFAULT_BIOS("v11")
	ROM_SYSTEM_BIOS(0, "v11", "v1.1")
	ROMX_LOAD("korvet11.rom", 0x0000, 0x6000, CRC(81bdc2af) SHA1(c3484c3f1f3d252475979283c073286b8661d2b9), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v20", "v2.0")
	ROMX_LOAD("korvet20.rom", 0x0000, 0x6000, CRC(d6c36a45) SHA1(dba67e63457251814ad5c0fe6bb6d584eea5c7d2), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "cpm", "cpm")
	ROMX_LOAD("cpm.rom",      0x0000, 0x4000, CRC(7a38d7f6) SHA1(fec6623291a38990b003e818683cd5edfb494c36), ROM_BIOS(2))

	ROM_REGION(0x2000, "gfx1", 0)
	ROM_LOAD("korvet2.fnt", 0x0000, 0x2000, CRC(fb1cd3d4) SHA1(58f1d6e393253b1e8b497ce0880b6eff6d85b42a))

	ROM_REGION(0xf5, "decplm", 0)
	ROM_LOAD("kr556rt2.d31", 0x00, 0xf5, CRC(3eae3879) SHA1(87f419e26d73d7b2f937c3fcae0415b74da37d97) BAD_DUMP) // devised from documentation
ROM_END

ROM_START( neiva )
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("neiva_d22.bin", 0x0000, 0x2000, CRC(9cc28f67) SHA1(68f390e846e1290df68419d522088d5325682945))
	ROM_LOAD("neiva_d23.bin", 0x2000, 0x2000, CRC(31b53dc4) SHA1(607f2a2d8b1de469125c6c02b9ffc65649b753a2))
	ROM_LOAD("neiva_d24.bin", 0x4000, 0x2000, CRC(d05c80df) SHA1(1ec2fa9983be5579abff7247fc9b98fe50661bd9))

	ROM_REGION(0x2000, "gfx1", 0)
	ROM_LOAD("neiva_d21.bin", 0x0000, 0x2000, CRC(fb1cd3d4) SHA1(58f1d6e393253b1e8b497ce0880b6eff6d85b42a))

	ROM_REGION(0xf5, "decplm", 0)
	ROM_LOAD("kr556rt2.d31", 0x00, 0xf5, CRC(3eae3879) SHA1(87f419e26d73d7b2f937c3fcae0415b74da37d97) BAD_DUMP)
ROM_END

ROM_START( kontur )
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "v1", "v1")
	ROMX_LOAD("kontur.rom",  0x0000, 0x2000, CRC(92cd441e) SHA1(9a0f9079256cefc6169ae4ba2114841d1f380480), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v2", "v2")
	ROMX_LOAD("kontur2.rom", 0x0000, 0x2000, CRC(5256d101) SHA1(22022a3c6882dbc5ea28d7815f00c182bbaef9e1), ROM_BIOS(1))

	ROM_REGION(0x2000, "gfx1", 0)
	ROM_LOAD("kontur.fnt", 0x0000, 0x2000, CRC(14d33790) SHA1(6d5fcb214805c5fc44ef98a97219158ff7826ac0))

	ROM_REGION(0xf5, "decplm", 0)
	ROM_LOAD("kr556rt2.d31", 0x00, 0xf5, CRC(3eae3879) SHA1(87f419e26d73d7b2f937c3fcae0415b74da37d97) BAD_DUMP)
ROM_END

ROM_START( bk8t )
	ROM_REGION(0x6000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("kor1.bin", 0x0000, 0x2000, CRC(f1e16ddc) SHA1(e3a10c9ce3f333928eb0d5f9b84e159e41fae6ca))
	ROM_LOAD("kor2.bin", 0x2000, 0x2000, CRC(d4431d97) SHA1(08f79785846369d410a4183f0d60b856d6d70199))
	ROM_LOAD("kor3.bin", 0x4000, 0x2000, CRC(74781903) SHA1(caaa638afe80eb83fc30b07dd6d1e40b66ddc6d1))

	ROM_REGION(0x2000, "gfx1", 0)
	ROM_LOAD("kor4.bin", 0x0000, 0x2000, CRC(d164bada) SHA1(c334e50fd31b1f42c7668b89772487971a6875cb))

	ROM_REGION(0xf5, "decplm", 0)
	ROM_LOAD("kr556rt2.d31", 0x00, 0xf5, CRC(3eae3879) SHA1(87f419e26d73d7b2f937c3fcae0415b74da37d97) BAD_DUMP)
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY      FULLNAME         FLAGS */
COMP( 1987, korvet, 0,      0,      pk8020,  pk8020, pk8020_state, empty_init, "<unknown>", "PK8020 Korvet", MACHINE_SUPPORTS_SAVE)
COMP( 1987, neiva,  korvet, 0,      pk8020,  pk8020, pk8020_state, empty_init, "<unknown>", "PK8020 Neiva",  MACHINE_SUPPORTS_SAVE)
COMP( 1987, kontur, korvet, 0,      pk8020,  pk8020, pk8020_state, empty_init, "<unknown>", "PK8020 Kontur", MACHINE_SUPPORTS_SAVE)
COMP( 1987, bk8t,   korvet, 0,      pk8020,  pk8020, pk8020_state, empty_init, "<unknown>", "BK-8T",         MACHINE_SUPPORTS_SAVE)
