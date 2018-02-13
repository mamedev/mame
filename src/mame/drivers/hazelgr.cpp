// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Hazel Grove Fruit Machine HW
 unknown platform! z80 based..

*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/clock.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"


class haze_state : public driver_device
{
public:
	haze_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void haze(machine_config &config);
	void io_map(address_map &map);
	void mem_map(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
};



ADDRESS_MAP_START(haze_state::mem_map)
	AM_RANGE(0x0000, 0x17ff) AM_ROM
	AM_RANGE(0x9000, 0x9fff) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(haze_state::io_map)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x83) AM_DEVREADWRITE("ctc1", z80ctc_device, read, write) // irq 17E0 => 0183(ch3)
	AM_RANGE(0x90, 0x93) AM_DEVREADWRITE("pio1", z80pio_device, read_alt, write_alt) // 91 irq 17F8 => 0A5E
	AM_RANGE(0xa0, 0xa3) AM_DEVREADWRITE("pio2", z80pio_device, read_alt, write_alt) // not programmed to interrupt
	AM_RANGE(0xb0, 0xb3) AM_DEVREADWRITE("pio3", z80pio_device, read_alt, write_alt) // not programmed to interrupt
	AM_RANGE(0xc0, 0xc3) AM_DEVREADWRITE("pio4", z80pio_device, read_alt, write_alt) // not programmed to interrupt
	AM_RANGE(0xc4, 0xc7) AM_DEVREADWRITE("ctc2", z80ctc_device, read, write) // irq 17E8 => 023D(ch0),0366(ch1),02BB(ch2),0378(ch3)
	AM_RANGE(0xc8, 0xcb) AM_DEVREADWRITE("ctc3", z80ctc_device, read, write) // irq 17F0 => 030E(ch0),038A(ch1)
ADDRESS_MAP_END


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
MACHINE_CONFIG_START(haze_state::haze)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,2000000)         /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_Z80_DAISY_CHAIN(daisy_chain)

	MCFG_DEVICE_ADD("ctc_clock", CLOCK, 1'000'000)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("ctc1", z80ctc_device, trg3))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("ctc2", z80ctc_device, trg0))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("ctc2", z80ctc_device, trg1))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("ctc2", z80ctc_device, trg2))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("ctc2", z80ctc_device, trg3))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("ctc3", z80ctc_device, trg0))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("ctc3", z80ctc_device, trg1))

	MCFG_DEVICE_ADD("ctc1", Z80CTC, 1'000'000 )
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("ctc2", Z80CTC, 1'000'000 )
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("ctc3", Z80CTC, 1'000'000 )
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("pio1", Z80PIO, 1'000'000 )
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(IOPORT("TEST"))

	MCFG_DEVICE_ADD("pio2", Z80PIO, 1'000'000 )

	MCFG_DEVICE_ADD("pio3", Z80PIO, 1'000'000 )

	MCFG_DEVICE_ADD("pio4", Z80PIO, 1'000'000 )
MACHINE_CONFIG_END

ROM_START( hg_frd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fd v 3-2 a.bin", 0x0000, 0x0800, CRC(d8c276e6) SHA1(9902554d40fb1f24ea5e43f8bbfb508b3a96e90b) )
	ROM_LOAD( "fd v 3-2 b.bin", 0x0800, 0x0800, CRC(c8654bdf) SHA1(342fa389b80fb9519e3fad488cea2063e88b30fa) )
	ROM_LOAD( "fd v 3-2 c.bin", 0x1000, 0x0800, CRC(77bb8d8c) SHA1(65b7dd8024747175c3bd5bc341e2e1a92699f1c6) )
ROM_END


GAME( 198?,  hg_frd,  0,  haze,  haze, haze_state,  0,  ROT0,  "Hazel Grove",    "Fruit Deuce (Hazel Grove)",     MACHINE_IS_SKELETON_MECHANICAL)
