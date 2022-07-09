// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

   M89 hardware for kiddie rides from Falgas

   Falgas M89-4 N/E

   _|_|_|_|___|_|_|_|___|_|_|_|_|_|_|____
  |          _______   _______ _______  |
  | _______  TIC206M   TIC206M TIC206M  |
  |                                     |
  |                   __________________|
  |                  | AY8910A         ||
  |                  |_________________||
  |                                     |
  | 7805CV            :::::::::         |
  |                   __________________|
  |                  | 82C55           ||
  |                  |_________________||
  | TDA7241B             _______________|
  |                     | GM76C28A     ||
  |                     |______________||
  |                    _________________|
  |                   | EPROM          ||
  |                   |________________||
  |    __________           __________  |
  |   |_PAL16V8_|          |SN74LS373N  |
  |  __________       __________________|
  | |_PAL16V8_|      |OKI M80C85A-2    ||
  |                  |_________________||
  |  6.000 MHz Xtal  _________ ________ |
  |                MC14020BCP MC14020BCP|
  | ____________RISER_PCB_______________|
  |_____________________________________|

  The riser PCB contains:
   -4 LEDs (motor on, coin input, timer-sound, light).
   -Bank of 4 dipswitches for timer configuration.
   -Bank of 4 dipswitches for coinage configuration.
   -Volume knob.


***************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
//#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "speaker.h"

namespace
{

class falgasm89_state : public driver_device
{
public:
	falgasm89_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_inputs(*this, "IN%u", 0U)
		, m_psg_pa(0xff)
	{
	}

	void cbully(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void psg_pa_w(u8 data);
	u8 psg_pb_r();

	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<i8085a_cpu_device> m_maincpu;
	required_ioport_array<4> m_inputs;

	u8 m_psg_pa;
};

void falgasm89_state::machine_start()
{
	save_item(NAME(m_psg_pa));
}

void falgasm89_state::psg_pa_w(u8 data)
{
	m_psg_pa = data;
}

u8 falgasm89_state::psg_pb_r()
{
	u8 result = 0xff;
	for (int n = 0; n < 4; n++)
		if (!BIT(m_psg_pa, n))
			result &= m_inputs[n]->read();

	return result;
}

void falgasm89_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0xf800, 0xffff).ram(); // NVRAM?
}

void falgasm89_state::io_map(address_map &map)
{
	map(0x00, 0x00).rw("psg", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x04, 0x04).w("psg", FUNC(ay8910_device::address_w));
}

INPUT_PORTS_START(cbully)
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

void falgasm89_state::cbully(machine_config &config)
{
	I8085A(config, m_maincpu, 6_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &falgasm89_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &falgasm89_state::io_map);

	// I8255(config, "i8255"); // "Coche Bully" has the i8255 socket empty

	SPEAKER(config, "mono").front_center();

	ay8910_device &psg(AY8910(config, "psg", 6_MHz_XTAL / 4)); // divider unknown
	psg.add_route(ALL_OUTPUTS, "mono", 0.50);
	psg.port_a_write_callback().set(FUNC(falgasm89_state::psg_pa_w));
	psg.port_b_read_callback().set(FUNC(falgasm89_state::psg_pb_r));
}

ROM_START(cbully)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("bully-gs_m89-iv_16-1-91.u2", 0x0000, 0x8000, CRC(4cc85230) SHA1(c3851e6610bcb3427f81ecfcd4575603a9edca6e)) // 27C256

	ROM_REGION(0x22e, "plds", 0)
	ROM_LOAD("palce16v8_m894-bt.u11", 0x000, 0x117, NO_DUMP) // Protected
	ROM_LOAD("palce16v8_m894-a.u10",  0x117, 0x117, NO_DUMP) // Protected
ROM_END

} // anonymous namespace

//    YEAR  NAME    PARENT MACHINE INPUT   CLASS            INIT        ROT   COMPANY   FULLNAME       FLAGS
GAME( 1991, cbully, 0,     cbully, cbully, falgasm89_state, empty_init, ROT0, "Falgas", "Coche Bully", MACHINE_IS_SKELETON_MECHANICAL )
