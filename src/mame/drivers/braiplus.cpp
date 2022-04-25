// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for BraiLab Plus talking computer.

    (from a document translated from Hungarian)
In addition to the Z80 microprocessor on the two-sided printed circuit board,
there are 256 Kbytes of dynamic RAM, 16 Kbytes of EPROM, 2 Z80PIOs, 4 Kbytes
of static RAM, a character generator EPROM, a WD2793 floppy disk controller,
a I8251 PUSART and a MEA-8000 speech synthesizer.

180 Kbytes of 256 Kbytes of RAM is handled as a fixed RAM disk under the
CP/M operating system. Memory management supports the use of a dynamic RAM
fixed disk, provides the appropriate address space for the operating system,
and flips between them and between EPROMs. The two Z80PIOs perform centronics
interface, system clock interrupt handling, and serial programming of the
baud rate with serial interface I8251 and HD-4702.

The increased size of 4 Kbytes memory enabled 25x80 character screen.
Floppy disk holds 792kb (formatted).

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/wd_fdc.h"
#include "machine/z80pio.h"

class braiplus_state : public driver_device
{
public:
	braiplus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pio(*this, "pio%u", 1U)
	{
	}

	void braiplus(machine_config &config);

private:
	void bank_w(u8 data);
	void unknown_w(u8 data);
	u8 unknown1_r();
	u8 unknown2_r();
	u8 unknown_dummy_r();

	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<z80_device> m_maincpu;
	required_device_array<z80pio_device, 2> m_pio;
};


void braiplus_state::bank_w(u8 data)
{
	// TODO: bankswitching
}

void braiplus_state::unknown_w(u8 data)
{
	// TODO: unknown (only high nibble used)
}

u8 braiplus_state::unknown1_r()
{
	// TODO: keyboard input? (only high nibble used)
	return 0;
}

u8 braiplus_state::unknown2_r()
{
	// TODO: keyboard input? (only high nibble used)
	return 0;
}

u8 braiplus_state::unknown_dummy_r()
{
	// TODO: what is this? (value always discarded)
	return 0xff;
}

void braiplus_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
	map(0x8000, 0x8000).r(FUNC(braiplus_state::unknown1_r));
	map(0x8001, 0x8001).r(FUNC(braiplus_state::unknown2_r));
	map(0x8002, 0x8002).r(FUNC(braiplus_state::unknown_dummy_r));
	map(0xa000, 0xbfff).ram();
	map(0xf200, 0xf2ff).ram();
	map(0xf800, 0xffff).ram();
}

void braiplus_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x04, 0x07).rw(m_pio[0], FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x08, 0x0b).rw(m_pio[1], FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x0c, 0x0d).rw("usart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x10, 0x10).rw("fdc", FUNC(wd2793_device::read), FUNC(wd2793_device::write));
	map(0x14, 0x14).w(FUNC(braiplus_state::bank_w));
	map(0x18, 0x18).w(FUNC(braiplus_state::unknown_w));
}


static INPUT_PORTS_START(braiplus)
INPUT_PORTS_END

static const z80_daisy_config daisy_chain[] =
{
	{ "pio1" },
	{ "pio2" },
	{ nullptr }
};

void braiplus_state::braiplus(machine_config &config)
{
	Z80(config, m_maincpu, 4'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &braiplus_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &braiplus_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	Z80PIO(config, m_pio[0], 4'000'000);
	m_pio[0]->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80PIO(config, m_pio[1], 4'000'000);
	m_pio[1]->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	i8251_device &usart(I8251(config, "usart", 2'000'000));
	usart.rxrdy_handler().set(m_pio[1], FUNC(z80pio_device::pa3_w));

	WD2793(config, "fdc", 2'000'000);

	// HACK: what should be driving this?
	CLOCK(config, "clock_hack", 100).signal_handler().set(m_pio[0], FUNC(z80pio_device::pa7_w));
}

ROM_START( braiplus )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "brailabplus.bin", 0x0000, 0x4000, CRC(521d6952) SHA1(f7405520d86fc7abd2dec51d1d016658472f6fe8) )

	ROM_REGION( 0x0800, "chargen", 0 ) // no idea what chargen it uses, using the one from homelab4 for now
	ROM_LOAD( "hl4.chr",   0x0000, 0x0800, BAD_DUMP CRC(f58ee39b) SHA1(49399c42d60a11b218a225856da86a9f3975a78a))
ROM_END

COMP( 1988, braiplus, 0, 0, braiplus, braiplus, braiplus_state, empty_init, "Jozsef and Endre Lukacs", "BraiLab Plus", MACHINE_IS_SKELETON )
