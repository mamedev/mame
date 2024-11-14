// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Mera-Elzab EC-7915/EC-7950 terminal.

***************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
//#include "machine/i8214.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
//#include "screen.h"


namespace {

class ec7915_state : public driver_device
{
public:
	ec7915_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_chargen(*this, "chargen")
	{
	}

	void ec7915(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_chargen;
};


void ec7915_state::mem_map(address_map &map)
{
	map(0x0000, 0x27ff).rom().region("maincpu", 0);
	map(0x4800, 0x5fff).ram();
	map(0x6000, 0x67ff).ram();
}

void ec7915_state::io_map(address_map &map)
{
	map(0x00, 0x03).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x04, 0x07).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x08, 0x09).rw("usart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x0c, 0x0f).rw("ppi3", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x10, 0x13).w("pit", FUNC(pit8253_device::write));
	//map(0x14, 0x14).w("picu", FUNC(i8214_device::write));
}


static INPUT_PORTS_START(ec7915)
INPUT_PORTS_END


void ec7915_state::ec7915(machine_config &config)
{
	I8080A(config, m_maincpu, 2000000); // КР580ВМ80А (not Z80) + SAB8224P clock generator
	m_maincpu->set_addrmap(AS_PROGRAM, &ec7915_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &ec7915_state::io_map);

	//I8214(config, "picu", 2000000); // CEMI UCY74S414

	PIT8253(config, "pit", 0); // КР580ВИ53

	I8251(config, "usart", 2000000); // КР580ВВ51А
	// 8251 usage appears to prefer synchronous communications

	// 2x NEC D8255AC-2 on main board + КР580ВВ55А on display board
	I8255A(config, "ppi1");
	I8255A(config, "ppi2");
	I8255A(config, "ppi3");
}


ROM_START( ec7915 ) // 6k ram // amber
	ROM_REGION( 0x2800, "maincpu", 0 )
	ROM_LOAD( "50mp_0c10_30_lupper.bin", 0x0000, 0x0800, CRC(e019690f) SHA1(b0ce837a940ad82d2f39bd9d02e3c441cb9e83ed) )
	ROM_LOAD( "50mp_0810_40.bin",        0x0800, 0x0800, CRC(ed7f12d6) SHA1(b6f1da6a74f77cf1d392eee79f5ea168f3626ee5) )
	ROM_LOAD( "50mp_1010_49.bin",        0x1000, 0x0800, CRC(bfddf0e6) SHA1(dff4be8c0403519530e6c9106ab279a3037e074a) )
	ROM_LOAD( "50mp_1810_60.bin",        0x1800, 0x0800, CRC(759f2dc7) SHA1(515778ea213b9204f75f920ef1fbff6c14f9cf3c) )
	ROM_LOAD( "50mp_2c10_30_lower.bin",  0x2000, 0x0800, CRC(1ff59657) SHA1(777ef82e20a0100c0069ee5e7fbac5b3b86e3529) BAD_DUMP ) // fails checksum test

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "char.bin",                0x0000, 0x0800, CRC(e75a6bc4) SHA1(04b56d1f5ab7f2145699555df5ac44d078804821) )
ROM_END

} // anonymous namespace


COMP(198?, ec7915, 0, 0, ec7915, ec7915, ec7915_state, empty_init, "Mera-Elzab", "EC-7915 (EC-7950)", MACHINE_IS_SKELETON)
