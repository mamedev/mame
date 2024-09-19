// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-11-02 Skeleton

Vectrix Graphics Processor. The VX384 was the main model, with 384K of RAM, and used an analog monitor.
 The VX128 was the cheaper model with 128K of RAM and less colours. It used a TTL-level monitor.
 Don't know which one this is, but VX384 is assumed.

It replaced your serial or centronics printer, which then plugged into the unit instead. The unit could be considered as a
terminal which could decode simple commands into complex graphics.


************************************************************************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/i8251.h"


namespace {

class vectrix_state : public driver_device
{
public:
	vectrix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		//      , m_maincpu(*this, "maincpu")
	{ }

	void vectrix(machine_config &config);

private:
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	//  required_device<cpu_device> m_maincpu;
};

void vectrix_state::mem_map(address_map &map)
{
	map(0x00000, 0x07fff).ram();
	map(0x0c000, 0x0ffff).rom().region("roms", 0);
	map(0xfc000, 0xfffff).rom().region("roms", 0);
}

void vectrix_state::io_map(address_map &map)
{
	map(0x3000, 0x3001).rw("uart1", FUNC(i8251_device::read), FUNC(i8251_device::write));
}

static INPUT_PORTS_START( vectrix )
INPUT_PORTS_END

void vectrix_state::vectrix(machine_config &config)
{
	i8088_cpu_device &maincpu(I8088(config, "maincpu", XTAL(14'318'181)/3));  // no idea of clock
	maincpu.set_addrmap(AS_PROGRAM, &vectrix_state::mem_map);
	maincpu.set_addrmap(AS_IO, &vectrix_state::io_map);

	I8251(config, "uart1", 0);
}

ROM_START( vectrix )
	ROM_REGION( 0x4000, "roms", 0 )
	ROM_LOAD( "vectrixl.bin", 0x0000, 0x2000, CRC(10b93e38) SHA1(0b1a23d384bfde4cd27c482f667eedd94f8f2406) )
	ROM_LOAD( "vectrixr.bin", 0x2000, 0x2000, CRC(33f9b06b) SHA1(6a1dffe5c2c0254824a8dddb8543f86d9ad8f173) )
ROM_END

} // anonymous namespace


COMP( 1983, vectrix, 0, 0, vectrix, vectrix, vectrix_state, empty_init, "Vectrix", "VX384 Graphics Processor Terminal", MACHINE_IS_SKELETON )
