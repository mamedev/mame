// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Novation BassStation synthesizers.

****************************************************************************/

#include "emu.h"
#include "cpu/mn1880/mn1880.h"
#include "machine/eeprompar.h"


namespace {

class basssta_state : public driver_device
{
public:
	basssta_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_inputs(*this, "IN%u", 0U)
		, m_input_select(0xff)
	{
	}

	void bassstr(machine_config &config);
	void sbasssta(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void input_select_w(u8 data);
	u8 input_r();

	void bassstr_prog(address_map &map) ATTR_COLD;
	void sbasssta_prog(address_map &map) ATTR_COLD;
	void bassstr_data(address_map &map) ATTR_COLD;
	void sbasssta_data(address_map &map) ATTR_COLD;

	required_device<mn1880_device> m_maincpu;
	required_ioport_array<4> m_inputs;

	u8 m_input_select;
};


void basssta_state::machine_start()
{
	save_item(NAME(m_input_select));
}

void basssta_state::input_select_w(u8 data)
{
	m_input_select = data;
}

u8 basssta_state::input_r()
{
	u8 ret = 0xff;
	for (int n = 0; n < 4; n++)
		if (!BIT(m_input_select, n))
			ret &= m_inputs[n]->read();
	return ret;
}

void basssta_state::bassstr_prog(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("program", 0);
}

void basssta_state::sbasssta_prog(address_map &map)
{
	map(0x0000, 0xffff).rom().region("program", 0);
}

void basssta_state::bassstr_data(address_map &map)
{
	map(0x0001, 0x0001).noprw();
	map(0x0003, 0x0003).noprw();
	map(0x000f, 0x000f).noprw();
	map(0x001f, 0x001f).noprw();
	map(0x0060, 0x03cf).ram(); // TODO: this plus everything above is probably internal to CPU
	map(0x8000, 0x87ff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write));
	map(0x8800, 0x8800).r(FUNC(basssta_state::input_r));
	map(0xa800, 0xa800).w(FUNC(basssta_state::input_select_w));
}

void basssta_state::sbasssta_data(address_map &map)
{
	map(0x0000, 0x0003).nopr();
	map(0x0001, 0x0001).nopw();
	map(0x0003, 0x0003).nopw();
	map(0x000f, 0x000f).noprw();
	map(0x0014, 0x0014).noprw();
	map(0x001c, 0x001c).nopr();
	map(0x001e, 0x001e).nopw();
	map(0x001f, 0x001f).noprw();
	map(0x0030, 0x0030).nopw();
	map(0x0032, 0x0032).nopw();
	map(0x0034, 0x0034).nopw();
	map(0x0036, 0x0036).nopw();
	map(0x005d, 0x005d).noprw();
	map(0x0060, 0x07ff).ram(); // TODO: this plus everything above is probably internal to CPU
	map(0x6000, 0x7fff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write));
	map(0x8001, 0x8001).w(FUNC(basssta_state::input_select_w));
	map(0x8002, 0x8004).nopw();
	map(0xc000, 0xc000).r(FUNC(basssta_state::input_r));
}


static INPUT_PORTS_START(basssta)
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END

void basssta_state::bassstr(machine_config &config)
{
	MN1880(config, m_maincpu, 8000000); // type and clock unknown (custom silkscreen; might be MN18P83220)
	m_maincpu->set_addrmap(AS_PROGRAM, &basssta_state::bassstr_prog);
	m_maincpu->set_addrmap(AS_DATA, &basssta_state::bassstr_data);

	EEPROM_2816(config, "eeprom");
}

void basssta_state::sbasssta(machine_config &config)
{
	MN1880(config, m_maincpu, 8000000); // type and clock unknown (custom silkscreen)
	m_maincpu->set_addrmap(AS_PROGRAM, &basssta_state::sbasssta_prog);
	m_maincpu->set_addrmap(AS_DATA, &basssta_state::sbasssta_data);

	EEPROM_2864(config, "eeprom"); // Microchip 28C64AF-15I/L
}

ROM_START(bassstr)
	ROM_REGION(0x8000, "program", 0)
	ROM_LOAD("v1.5.bin", 0x0000, 0x8000, CRC(a8bc9721) SHA1(2c2dc3bee0fabf1e77d02cdf3e53885f2c5b913e)) // ST M27C256B (DIP28)
ROM_END

ROM_START(sbasssta)
	ROM_REGION(0x10000, "program", 0)
	ROM_LOAD("v1.9.bin", 0x00000, 0x10000, CRC(045bf9e3) SHA1(69155026c2497a4731162cadb6b441e00902d3f6))
ROM_END

} // anonymous namespace


SYST(1995, bassstr, 0, 0,  bassstr,  basssta, basssta_state, empty_init, "Novation", "BassStation Rack Analogue Synthesizer Module", MACHINE_IS_SKELETON)
SYST(1997, sbasssta, 0, 0, sbasssta, basssta, basssta_state, empty_init, "Novation", "Super Bass Station", MACHINE_IS_SKELETON)
