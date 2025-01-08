// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

MC-80.3x driver by Miodrag Milanovic

2009-05-12 Skeleton driver.
2009-05-15 Initial implementation
2011-09-01 Modernised

mc80.3x: http://www.ycdt.net/mc80.3x/

mc8030: very little info available. The area from FFD8-FFFF is meant for
interrupt vectors and so on, but most of it is zeroes. Appears the keyboard
is an ascii keyboard with built-in beeper. It communicates via the SIO.
The asp ctc needs at least 2 triggers. The purpose of the zve pio is unknown.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "emupal.h"
#include "screen.h"
#include "machine/clock.h"
#include "bus/rs232/rs232.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"


namespace {

class mc8030_state : public driver_device
{
public:
	mc8030_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void mc8030(machine_config &config);

private:
	void zve_write_protect_w(uint8_t data);
	void vis_w(offs_t offset, uint8_t data);
	void eprom_prog_w(uint8_t data);
	uint8_t zve_port_a_r();
	uint8_t zve_port_b_r();
	void zve_port_a_w(uint8_t data);
	void zve_port_b_w(uint8_t data);
	uint8_t asp_port_a_r();
	uint8_t asp_port_b_r();
	void asp_port_a_w(uint8_t data);
	void asp_port_b_w(uint8_t data);
	void machine_start() override ATTR_COLD;
	uint32_t screen_update_mc8030(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	std::unique_ptr<u8[]> m_vram;
	required_device<z80_device> m_maincpu;
};


void mc8030_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	//  ZRE 4 * 2KB
	map(0x0000, 0x1fff).rom(); // ZRE ROM's 4 * 2716
	map(0x2000, 0x27ff).rom(); // SPE ROM's 2 * 2708
	map(0x2800, 0x3fff).rom(); // For extension
	map(0x4000, 0xbfff).ram(); // SPE RAM
	map(0xc000, 0xffff).ram(); // ZRE RAM
}

void mc8030_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x30, 0x3f).mirror(0xff00).noprw(); //"mass storage"
	map(0x80, 0x83).mirror(0xff00).rw("zve_ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write)); // user CTC
	map(0x84, 0x87).mirror(0xff00).rw("zve_pio", FUNC(z80pio_device::read), FUNC(z80pio_device::write)); // PIO unknown usage
	map(0x88, 0x8f).mirror(0xff00).w(FUNC(mc8030_state::zve_write_protect_w));
	map(0xc0, 0xcf).select(0xff00).w(FUNC(mc8030_state::vis_w));
	map(0xd0, 0xd3).mirror(0xff00).rw("asp_sio", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w)); // keyboard & IFSS?
	map(0xd4, 0xd7).mirror(0xff00).rw("asp_ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write)); // sio bauds, KMBG? and kbd
	map(0xd8, 0xdb).mirror(0xff00).rw("asp_pio", FUNC(z80pio_device::read), FUNC(z80pio_device::write)); // external bus
	map(0xe0, 0xef).mirror(0xff00).w(FUNC(mc8030_state::eprom_prog_w));
}

/* Input ports */
static INPUT_PORTS_START( mc8030 )
INPUT_PORTS_END


void  mc8030_state::zve_write_protect_w(uint8_t data)
{
}

void mc8030_state::vis_w(offs_t offset, uint8_t data)
{
	// reg C
	// 7 6 5 4 -- module
	//         3 - 0 left half, 1 right half
	//           2 1 0
	//           =====
	//           0 0 0 - dark
	//           0 0 1 - light
	//           0 1 0 - in reg pixel
	//           0 1 1 - negate in reg pixel
	//           1 0 x - operation code in B reg
	// reg B
	//
	uint16_t addr = ((offset & 0xff00) >> 2) | ((offset & 0x08) << 2) | (data >> 3);
	uint8_t c = 1 << (data & 7);
	if (BIT(offset, 0))
		m_vram[addr] |= c;
	else
		m_vram[addr] &= ~c;
}

void mc8030_state::eprom_prog_w(uint8_t data)
{
}

uint8_t mc8030_state::zve_port_a_r()
{
	return 0xff;
}

uint8_t mc8030_state::zve_port_b_r()
{
	return 0xff;
}

uint8_t mc8030_state::asp_port_a_r()
{
	return 0xff;
}

uint8_t mc8030_state::asp_port_b_r()
{
	return 0xff;
}


uint32_t mc8030_state::screen_update_mc8030(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t ma=0;

	for(uint16_t y = 0; y < 256; y++ )
	{
		uint16_t *p = &bitmap.pix(y);
		for (uint16_t x = ma; x < ma + 64; x++)
		{
			uint8_t const gfx = m_vram[x^0x3fff];

			/* Display a scanline of a character */
			*p++ = BIT(gfx, 7);
			*p++ = BIT(gfx, 6);
			*p++ = BIT(gfx, 5);
			*p++ = BIT(gfx, 4);
			*p++ = BIT(gfx, 3);
			*p++ = BIT(gfx, 2);
			*p++ = BIT(gfx, 1);
			*p++ = BIT(gfx, 0);
		}
		ma+=64;
	}
	return 0;
}

// this is a guess there is no information available
static const z80_daisy_config daisy_chain[] =
{
	{ "asp_ctc" },      /* System ctc */
	{ "asp_pio" },      /* System pio */
	{ "asp_sio" },      /* sio */
	{ "zve_pio" },      /* User pio */
	{ "zve_ctc" },      /* User ctc */
	{ nullptr }
};

void mc8030_state::machine_start()
{
	m_vram = make_unique_clear<u8[]>(0x4000);
	save_pointer(NAME(m_vram), 0x4000);
}

void mc8030_state::mc8030(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(2'457'600));
	m_maincpu->set_addrmap(AS_PROGRAM, &mc8030_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &mc8030_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(mc8030_state::screen_update_mc8030));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* Devices */
	z80pio_device& zve_pio(Z80PIO(config, "zve_pio", XTAL(2'457'600)));
	zve_pio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	zve_pio.in_pa_callback().set(FUNC(mc8030_state::zve_port_a_r));
	//zve_pio.out_pa_callback().set(FUNC(mc8030_state::zve_port_a_w));
	zve_pio.in_pb_callback().set(FUNC(mc8030_state::zve_port_b_r));
	//zve_pio.out_pb_callback().set(FUNC(mc8030_state::zve_port_b_w));

	z80ctc_device& zve_ctc(Z80CTC(config, "zve_ctc", XTAL(2'457'600)));
	zve_ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	// ZC0, ZC1, ZC2 for user

	z80pio_device& asp_pio(Z80PIO(config, "asp_pio", XTAL(2'457'600)));
	asp_pio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	asp_pio.in_pa_callback().set(FUNC(mc8030_state::asp_port_a_r));
	//asp_pio.out_pa_callback().set(FUNC(mc8030_state::asp_port_a_w));
	asp_pio.in_pb_callback().set(FUNC(mc8030_state::asp_port_b_r));
	//asp_pio.out_pb_callback().set(FUNC(mc8030_state::asp_port_b_w));

	z80ctc_device& asp_ctc(Z80CTC(config, "asp_ctc", XTAL(2'457'600)));
	asp_ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	// ZC0: to SIO CLK CH A
	// ZC1: to SIO CLK CH B
	// ZC2: KMBG (??)

	clock_device &uart_clock(CLOCK(config, "uart_clock", 153600));
	uart_clock.signal_handler().set("asp_sio", FUNC(z80sio_device::txca_w));
	uart_clock.signal_handler().append("asp_sio", FUNC(z80sio_device::rxca_w));

	z80sio_device& sio(Z80SIO(config, "asp_sio", 4800));
	// SIO CH A in = keyboard; out = beeper; CH B = IFSS (??)
	sio.out_txda_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	sio.out_dtra_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	sio.out_rtsa_callback().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "keyboard"));
	rs232.rxd_handler().set("asp_sio", FUNC(z80sio_device::rxa_w));
	rs232.cts_handler().set("asp_sio", FUNC(z80sio_device::ctsa_w));
}

/* ROM definition */
ROM_START( mc8030 )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "zve_1.rom", 0x0000, 0x0800, CRC(31ec0159) SHA1(a97ea9eb733c462e77d625a7942134e45d911c0a))
	ROM_LOAD( "zve_2.rom", 0x0800, 0x0800, CRC(5104983d) SHA1(7516274904042f4fc6813aa8b2a75c0a64f9b937))
	ROM_LOAD( "zve_3.rom", 0x1000, 0x0800, CRC(4bcfd727) SHA1(d296e587098e70270ad60db8edaa685af368b849))
	ROM_LOAD( "zve_4.rom", 0x1800, 0x0800, CRC(f949ae43) SHA1(68c324cf5578497db7ae65da5695fcb30493f612))
	ROM_LOAD( "spe_1.rom", 0x2000, 0x0400, CRC(826f609c) SHA1(e77ff6c180f5a6d7756d076173ae264a0e26f066))
	ROM_LOAD( "spe_2.rom", 0x2400, 0x0400, CRC(98320040) SHA1(6baf87e196f1ccdf44912deafa6042becbfb0679))

	ROM_REGION( 0x4000, "user1", 0 )
	// marked as "80.3x"
	ROM_LOAD( "mc80.3-x-2c00-c63c.bin", 0x2c00, 0x0400, CRC(469be754) SHA1(a7fea257a1c0970349f75504c0870a2649b50303) )
	ROM_LOAD( "mc80.3-ccd-3000-f10a.bin", 0x3000, 0x0400, CRC(7d220128) SHA1(bb6070c9d460ec7ea1a1b46b19ca9520d55e127c) ) // 80.3x ccd test system
	ROM_LOAD( "mc80.3-x-3800-7280.bin", 0x3800, 0x0400, CRC(09976efb) SHA1(1a708adbf1cd68d450a9bfccafe1f82e755e5885) )
	// marked as "80.3x rk"
	ROM_LOAD( "mc80.3-rk-3000-5642.bin", 0x3000, 0x0400, CRC(280b2211) SHA1(d2c05ff7f7ea534776bf7e92263f1c10192e5385) )
	ROM_LOAD( "mc80.3-rk-3400-c6a7.bin", 0x3400, 0x0400, CRC(fc5656f3) SHA1(0ad5abb6536665719693063bf8da2993238c84dd) )
	ROM_LOAD( "mc80.3-rk-3800-1678.bin", 0x3800, 0x0400, CRC(34d7e1cf) SHA1(14d3e49f34e0c2a95967613538b33a671998e7a8) )
	// marked as "80.30e v1"
	ROM_LOAD( "mc80.30e-0000.bin", 0x0000, 0x0800, CRC(ebdf766f) SHA1(d06f1e4467104f59554168d17cd15b98d107375e) )
	// marked as "80.30e v2"
	ROM_LOAD( "mc80.30e-0000v2.bin", 0x0000, 0x0800, CRC(259b55e9) SHA1(6e8fd84f1b225f33bc0fd30ecc6e30b8063eaeed) )
	ROM_LOAD( "mc80.30e-0800v2.bin", 0x0800, 0x0800, CRC(fe7a01a7) SHA1(6531cde5b9dea2a15a813598937aa3d9540a8066) )
	// marked as "80.31e"
	ROM_LOAD( "mc80.31e-2400-d0d7.bin", 0x2400, 0x0400, CRC(43c22046) SHA1(a179fe83b5cbbbc5f92a4b2ef1012099ccb333d9) )
	ROM_LOAD( "mc80.31e-3000-0d15.bin", 0x3000, 0x0400, CRC(cf6f090b) SHA1(0bc352ca42f41cfe7e28052c099dcd020b776dd5) )
	ROM_LOAD( "mc80.31e-3400-9993.bin", 0x3400, 0x0400, CRC(918d2b55) SHA1(b6dec17e996c464cf189a699d24d270494540b49) )
	ROM_LOAD( "mc80.31e-3800-7a4c.bin", 0x3800, 0x0400, CRC(62d9f989) SHA1(a20b731daed51270d86f486751302055eb93dd1c) )
	// marked as "80.31e?"
	ROM_LOAD( "mc80.3s-2000.bin", 0x2000, 0x0400, CRC(28f1df56) SHA1(9752c9eab3d9f72c23b5f5618a5db1a038953e29) )
	ROM_LOAD( "mc80.3s-2400.bin", 0x2400, 0x0400, CRC(43c22046) SHA1(a179fe83b5cbbbc5f92a4b2ef1012099ccb333d9) )
	ROM_LOAD( "mc80.3s-2800.bin", 0x2800, 0x0400, CRC(4b52deb1) SHA1(f8a9ddb4363f8389990fd263985e882a73265c5d) )
	ROM_LOAD( "mc80.3s-2c00.bin", 0x2c00, 0x0400, CRC(a13d8302) SHA1(1fcdcd6b7af8ef4b18a0658a1a50d0db26b7f214) )
	ROM_LOAD( "mc80.3s-3000.bin", 0x3000, 0x0400, CRC(cf6f090b) SHA1(0bc352ca42f41cfe7e28052c099dcd020b776dd5) )
	ROM_LOAD( "mc80.3s-3400.bin", 0x3400, 0x0400, CRC(918d2b55) SHA1(b6dec17e996c464cf189a699d24d270494540b49) )
	ROM_LOAD( "mc80.3s-3800.bin", 0x3800, 0x0400, CRC(6104646b) SHA1(630f7c57e928db0eb4070139a66f2d313a6314b4) )
	ROM_LOAD( "mc80.3s-3c00.bin", 0x3c00, 0x0400, CRC(2f82d032) SHA1(fe8f642b94a0ba8852ec56d8cbb7a52bb7e5d55a) )
	// marked as "80.33 original"
	ROM_LOAD( "mc80.33-1000.bin", 0x1000, 0x0800, CRC(c7e062b1) SHA1(81b999655b32d9b39287a08896a274278a2f739c) )
	// random set, zve_1 - 4 same as main set
	ROM_LOAD( "spe_1a.rom",    0x2000, 0x000800, CRC(37c71c68) SHA1(951650698b00f65facf5ccfbd8dd13628a93425d) )
	ROM_LOAD( "spe_2a.rom",    0x2800, 0x000400, CRC(9ec8f287) SHA1(cdf5a9583d898814ba480ffbc8d906a642c6dc81) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY                FULLNAME       FLAGS
COMP( 198?, mc8030, 0,      0,      mc8030,  mc8030, mc8030_state, empty_init, "VEB Elektronik Gera", "MC-80.30/31", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | ORIENTATION_FLIP_X | MACHINE_SUPPORTS_SAVE )
