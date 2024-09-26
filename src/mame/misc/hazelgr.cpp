// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Hazel Grove Fruit Machine HW
 unknown platform! z80 based..

*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/clock.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"


namespace {

class haze_state : public driver_device
{
public:
	haze_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void haze(machine_config &config);

private:
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
};



void haze_state::mem_map(address_map &map)
{
	map(0x0000, 0x17ff).rom();
	map(0x9000, 0x9fff).ram();
}

void haze_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x80, 0x83).rw("ctc1", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write)); // irq 17E0 => 0183(ch3)
	map(0x90, 0x93).rw("pio1", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt)); // 91 irq 17F8 => 0A5E
	map(0xa0, 0xa3).rw("pio2", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt)); // not programmed to interrupt
	map(0xb0, 0xb3).rw("pio3", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt)); // not programmed to interrupt
	map(0xc0, 0xc3).rw("pio4", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt)); // not programmed to interrupt
	map(0xc4, 0xc7).rw("ctc2", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write)); // irq 17E8 => 023D(ch0),0366(ch1),02BB(ch2),0378(ch3)
	map(0xc8, 0xcb).rw("ctc3", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write)); // irq 17F0 => 030E(ch0),038A(ch1)
}


static INPUT_PORTS_START( haze )
	PORT_START("TEST")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
INPUT_PORTS_END


static const z80_daisy_config daisy_chain[] =
{
	{ "ctc1" },
	{ "ctc2" },
	{ "ctc3" },
	{ "pio1" },
	{ nullptr }
};

// All frequencies are guesswork, in an effort to get something to happen
void haze_state::haze(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 2000000);         /* ? MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &haze_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &haze_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	clock_device &ctc_clock(CLOCK(config, "ctc_clock", 1'000'000));
	ctc_clock.signal_handler().set("ctc1", FUNC(z80ctc_device::trg3));
	ctc_clock.signal_handler().append("ctc2", FUNC(z80ctc_device::trg0));
	ctc_clock.signal_handler().append("ctc2", FUNC(z80ctc_device::trg1));
	ctc_clock.signal_handler().append("ctc2", FUNC(z80ctc_device::trg2));
	ctc_clock.signal_handler().append("ctc2", FUNC(z80ctc_device::trg3));
	ctc_clock.signal_handler().append("ctc3", FUNC(z80ctc_device::trg0));
	ctc_clock.signal_handler().append("ctc3", FUNC(z80ctc_device::trg1));

	z80ctc_device& ctc1(Z80CTC(config, "ctc1", 1'000'000));
	ctc1.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80ctc_device& ctc2(Z80CTC(config, "ctc2", 1'000'000));
	ctc2.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80ctc_device& ctc3(Z80CTC(config, "ctc3", 1'000'000));
	ctc3.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80pio_device& pio1(Z80PIO(config, "pio1", 1'000'000));
	pio1.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	pio1.in_pa_callback().set_ioport("TEST");

	Z80PIO(config, "pio2", 1'000'000);

	Z80PIO(config, "pio3", 1'000'000);

	Z80PIO(config, "pio4", 1'000'000);
}

ROM_START( hg_frd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fd v 3-2 a.bin", 0x0000, 0x0800, CRC(d8c276e6) SHA1(9902554d40fb1f24ea5e43f8bbfb508b3a96e90b) )
	ROM_LOAD( "fd v 3-2 b.bin", 0x0800, 0x0800, CRC(c8654bdf) SHA1(342fa389b80fb9519e3fad488cea2063e88b30fa) )
	ROM_LOAD( "fd v 3-2 c.bin", 0x1000, 0x0800, CRC(77bb8d8c) SHA1(65b7dd8024747175c3bd5bc341e2e1a92699f1c6) )
ROM_END

} // anonymous namespace


GAME( 198?, hg_frd, 0, haze, haze, haze_state, empty_init, ROT0, "Hazel Grove", "Fruit Deuce (Hazel Grove)", MACHINE_IS_SKELETON_MECHANICAL)
