// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    mupid/Infonova C2A2
	Grundig PTC-100

	- 2x Z80
	- 128 + 8 KB RAM
	- 8035
	- M58990P-1 ADC

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/adc0808.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class mupid2_state : public driver_device
{
public:
	mupid2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_cpu1(*this, "cpu1"),
		m_cpu2(*this, "cpu2"),
		m_kbdcpu(*this, "kbdcpu")
		{ }

		void c2a2(machine_config &config);

private:
	DECLARE_READ8_MEMBER(kbd_bus_r);
	DECLARE_READ8_MEMBER(kbd_p1_r);
	DECLARE_WRITE8_MEMBER(kbd_p1_w);
	DECLARE_WRITE8_MEMBER(kbd_p2_w);

	required_device<cpu_device> m_cpu1;
	required_device<cpu_device> m_cpu2;
	required_device<i8035_device> m_kbdcpu;

	void cpu1_mem(address_map &map);
	void cpu1_io(address_map &map);
	void cpu2_mem(address_map &map);
	void cpu2_io(address_map &map);
	void kbdcpu_mem(address_map &map);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void mupid2_state::cpu1_mem(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0xffff).ram();
}

void mupid2_state::cpu1_io(address_map &map)
{
	// something at 0xa0 and 0xc0
}

void mupid2_state::cpu2_mem(address_map &map)
{
	map(0x0000, 0xffff).noprw();
}

void mupid2_state::cpu2_io(address_map &map)
{
}

void mupid2_state::kbdcpu_mem(address_map &map)
{
	map(0x000, 0x3ff).mirror(0xc00).rom();
}


//**************************************************************************
//  INPUTS
//**************************************************************************

INPUT_PORTS_START( mupid2 )
INPUT_PORTS_END

READ8_MEMBER(mupid2_state::kbd_bus_r)
{
//	logerror("kbd_bus_r\n");
	return 0xff;
}

READ8_MEMBER(mupid2_state::kbd_p1_r)
{
//	logerror("kbd_p1_r\n");
	return 0xff;
}

WRITE8_MEMBER(mupid2_state::kbd_p1_w)
{
//	logerror("kbd_p1_w: %02x\n", data);
}

WRITE8_MEMBER(mupid2_state::kbd_p2_w)
{
//	logerror("kbd_p2_w: %02x\n", data);
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void mupid2_state::c2a2(machine_config &config)
{
	Z80(config, m_cpu1, 4000000);
	m_cpu1->set_addrmap(AS_PROGRAM, &mupid2_state::cpu1_mem);
	m_cpu1->set_addrmap(AS_IO, &mupid2_state::cpu1_io);

	Z80(config, m_cpu2, 4000000);
	m_cpu2->set_addrmap(AS_PROGRAM, &mupid2_state::cpu2_mem);
	m_cpu2->set_addrmap(AS_IO, &mupid2_state::cpu2_io);

	I8035(config, m_kbdcpu, 4000000);
	m_kbdcpu->set_addrmap(AS_PROGRAM, &mupid2_state::kbdcpu_mem);
	m_kbdcpu->bus_in_cb().set(FUNC(mupid2_state::kbd_bus_r));
	m_kbdcpu->p1_in_cb().set(FUNC(mupid2_state::kbd_p1_r));
	m_kbdcpu->p1_out_cb().set(FUNC(mupid2_state::kbd_p1_w));
	m_kbdcpu->p2_out_cb().set(FUNC(mupid2_state::kbd_p2_w));

	M58990(config, "adc", 1000000);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( mupid2 )
	ROM_REGION(0x8000, "cpu1", 0)
	ROM_LOAD("0090.7012.40.02_27.09.85.u40", 0x0000, 0x4000, CRC(0b320f46) SHA1(064e1b1697b9b767f89be7c1e3d20e1157324791))
	ROM_LOAD("0090.7012.39.02_27.09.85.u39", 0x4000, 0x4000, CRC(b2fb634c) SHA1(70ced48d0a27a661ddd7fbc529a891bd0bcec926))

	ROM_REGION(0x800, "kbdcpu", 0)
	ROM_LOAD("motronic_0090.6820.06.00", 0x000, 0x400, CRC(78b6d827) SHA1(200f2b33c518a889f8c6a5f4dd4443ac76884b21))
	ROM_CONTINUE(0x000, 0x400)
ROM_END

ROM_START( mupid2i )
	ROM_REGION(0xc000, "cpu1", 0)
	ROM_LOAD("kv_2.5.90_c2a2_ffd3.u40", 0x0000, 0x8000, CRC(a77ccb92) SHA1(9588e07ee0d4f06b346b7f5b58b8086a1b6ef140))
	ROM_LOAD("kh_2.5.90_c2a2_6860.u39", 0x8000, 0x4000, CRC(cdf64a6d) SHA1(a4a76ac761de016c7a196120a1c9fef6016c171c))

	ROM_REGION(0x800, "kbdcpu", 0)
	ROM_LOAD("motronic_0090.6820.06.00", 0x000, 0x400, CRC(78b6d827) SHA1(200f2b33c518a889f8c6a5f4dd4443ac76884b21))
	ROM_CONTINUE(0x000, 0x400)
ROM_END

ROM_START( ptc100 )
	ROM_REGION(0x8000, "cpu1", 0)
	ROM_LOAD("mup.u40", 0x0000, 0x4000, CRC(b4ac8ccb) SHA1(01bc818ec571d099176b6f69aaa736bb5410dd8e))
	ROM_LOAD("mup.u39", 0x4000, 0x4000, CRC(9812fefc) SHA1(bb4e69eba504dae6065094d9668be2b0478b0433))

	ROM_REGION(0x800, "kbdcpu", 0)
	ROM_LOAD("kbc.bin", 0x000, 0x400, CRC(72502f02) SHA1(4adb5c55691e1c53a3364d97e64d194be4886b52))
	ROM_CONTINUE(0x000, 0x400)
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY     FULLNAME            FLAGS
COMP( 1985, mupid2,  0,      0,      c2a2,    mupid2, mupid2_state, empty_init, "mupid",    "Post-Mupid C2A2", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1990, mupid2i, mupid2, 0,      c2a2,    mupid2, mupid2_state, empty_init, "Infonova", "C2A2",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 198?, ptc100,  mupid2, 0,      c2a2,    mupid2, mupid2_state, empty_init, "Grundig",  "PTC-100",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
