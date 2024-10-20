// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Micro-Term ERGO 201 terminal.

    The ERGO 201 belongs to one half of a paired series of terminals. Half
    of these terminals emulate the VT100 and support a 132 column mode; the
    other half emulate the VT52, ADM3A and ACT-5A instead but do appear to
    have been influenced by the VT100 in overall design.

    The first members of this series, the MIME 340 (non-VT100 emulation)
    and MIME 740 (VT100 emulation), were single units with non-detachable
    keyboards like Micro-Term's older ACT and MIME terminals. They were
    respectively succeeded by the ERGO 2000 and ERGO 3001 with tiltable
    screens and similar but detached keyboards. The ERGO 201 and ERGO 301
    appear to be low-cost redesigns of these two with keyboards even more
    closely resembling the VT100's, with a Plot 10 graphics board as an
    option. Also introduced around this time was the ERGO 4000, a
    portrait-mode terminal displaying 66 lines of 80 characters.

****************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/ay31015.h"
#include "machine/clock.h"
#include "machine/er1400.h"
//#include "machine/ergo201_kbd.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "video/tms9927.h"
#include "screen.h"


namespace {

class ergo201_state : public driver_device
{
public:
	ergo201_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_earom(*this, "earom")
		, m_kbuart(*this, "kbuart")
		, m_vtac(*this, "vtac")
		, m_chargen(*this, "chargen")
	{
	}

	void ergo201(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void earom_latch_w(u8 data);
	u8 status_r();

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<er1400_device> m_earom;
	required_device<ay31015_device> m_kbuart;
	required_device<tms9927_device> m_vtac;
	required_region_ptr<u8> m_chargen;
};

void ergo201_state::machine_start()
{
	m_kbuart->write_tsb(0);
	m_kbuart->write_nb1(1);
	m_kbuart->write_nb2(1);
	m_kbuart->write_np(1);
	m_kbuart->write_cs(1);
	m_kbuart->write_swe(0);
}

u32 ergo201_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void ergo201_state::earom_latch_w(u8 data)
{
	m_earom->c1_w(BIT(data, 0));
	m_earom->data_w(BIT(data, 1));
	m_earom->c3_w(BIT(data, 2));
	m_earom->c2_w(BIT(data, 3));
	m_earom->clock_w(BIT(data, 4));
}

u8 ergo201_state::status_r()
{
	return (m_kbuart->dav_r() << 1) | (m_earom->data_r() << 5);
}

void ergo201_state::mem_map(address_map &map)
{
	map(0x0000, 0x5fff).rom().region("program", 0);
	map(0x1000, 0x1000).nopw();
	map(0x2000, 0x2000).unmapw(); // screen brightness?
	map(0x3000, 0x300f).w(m_vtac, FUNC(tms9927_device::write));
	map(0x4000, 0x4000).w(FUNC(ergo201_state::earom_latch_w));
	map(0x5000, 0x5000).w(m_kbuart, FUNC(ay31015_device::transmit));
	map(0x6000, 0x6000).r(m_kbuart, FUNC(ay31015_device::receive));
	map(0x6001, 0x6001).r(FUNC(ergo201_state::status_r));
	map(0x7000, 0x77ff).ram();
	map(0x8000, 0x87ff).ram();
	map(0xc000, 0xc7ff).ram();
}

void ergo201_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xf4, 0xf7).rw("dart", FUNC(z80dart_device::cd_ba_r), FUNC(z80dart_device::cd_ba_w));
	map(0xf8, 0xfb).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}


static INPUT_PORTS_START(ergo201)
INPUT_PORTS_END

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ "dart" },
	{ nullptr }
};

void ergo201_state::ergo201(machine_config &config)
{
	Z80(config, m_maincpu, 16.313_MHz_XTAL / 4); // SGS Z8400AB1 (divider guessed)
	m_maincpu->set_addrmap(AS_PROGRAM, &ergo201_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &ergo201_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	z80ctc_device &ctc(Z80CTC(config, "ctc", 16.313_MHz_XTAL / 4)); // SGS Z8430AB1
	ctc.set_clk<0>(16.313_MHz_XTAL / 53);
	ctc.set_clk<1>(16.313_MHz_XTAL / 53);
	ctc.set_clk<2>(16.313_MHz_XTAL / 53);
	ctc.zc_callback<0>().set("dart", FUNC(z80dart_device::rxca_w));
	ctc.zc_callback<1>().set("dart", FUNC(z80dart_device::txca_w));
	ctc.zc_callback<2>().set("dart", FUNC(z80dart_device::rxtxcb_w));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80dart_device &dart(Z80DART(config, "dart", 16.313_MHz_XTAL / 4)); // Zilog Z8470A PS
	dart.out_txda_callback().set("dart", FUNC(z80dart_device::rxa_w));
	dart.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	AY31015(config, m_kbuart); // GI AY-3-1015D
	m_kbuart->set_auto_rdav(true);

	clock_device &kbclock(CLOCK(config, "kbclock", 10000));
	kbclock.signal_handler().set(m_kbuart, FUNC(ay31015_device::write_rcp));
	kbclock.signal_handler().append(m_kbuart, FUNC(ay31015_device::write_tcp));

	ER1400(config, m_earom); // GI ER-1400

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(16.313_MHz_XTAL, 855, 0, 720, 318, 0, 300);
	screen.set_screen_update(FUNC(ergo201_state::screen_update));
	screen.set_color(rgb_t::green()); // P31

	CRT5037(config, m_vtac, 16.313_MHz_XTAL / 9); // TI TMS9937NL
	m_vtac->set_char_width(9);
	m_vtac->set_screen("screen");
}


ROM_START(ergo201)
	ROM_REGION(0x6000, "program", 0) // Checksum is off by a few bits, so at least one dump is bad
	ROM_LOAD("201.u18", 0x0000, 0x2000, CRC(fec9fa3c) SHA1(3c1f9de9f62e74fbd1c9b67736d0959c0f6b46b1) BAD_DUMP) // M5L2764K
	ROM_LOAD("r1.0_special_5786.u73", 0x2000, 0x2000, CRC(5a31b6bc) SHA1(6929fa8f6d481790cd43732a9a97a9110ad9fb6c)) // MBM2764-25
	ROM_LOAD("201.u17", 0x4000, 0x2000, CRC(e7197403) SHA1(96fff6fb30eeac616d71da731b0ea79c8cd35096)) // M5L2764K

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("201.u53", 0x0000, 0x1000, CRC(907dac04) SHA1(5bae6680f1ef3f5335a223bdf403e8f3ef272430)) // MBM2732A-35
ROM_END

} // anonymous namespace


COMP(1985, ergo201, 0, 0, ergo201, ergo201, ergo201_state, empty_init, "Micro-Term / Kurzweil Computer Products", "ERGO 201 (Special #9233)", MACHINE_IS_SKELETON)
