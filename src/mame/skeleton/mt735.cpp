// license:BSD-3-Clause
// copyright-holders:Cowering, Olivier Galibert
/*
  Brother MT735 thermal printer
*/


#include "emu.h"
#include "cpu/m68000/m68000.h"

class mt735_state : public driver_device
{
public:
	mt735_state(const machine_config &mconfig, device_type type, const char *tag);

	void mt735(machine_config &config);

private:
	required_device<m68000_device> m_cpu;

	uint8_t p4_r();
	uint8_t p5_r();

	virtual void machine_start() override;
	virtual void machine_reset() override;
	void mt735_map(address_map &map);
};

mt735_state::mt735_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_cpu(*this, "maincpu")
{
}

void mt735_state::machine_start()
{
}

void mt735_state::machine_reset()
{
}

uint8_t mt735_state::p4_r()
{
	logerror("p4_r (%06x)\n", m_cpu->pc());
	return 0xe0;
}

uint8_t mt735_state::p5_r()
{
	logerror("p5_r (%06x)\n", m_cpu->pc());
	return 0x00;
}

void mt735_state::mt735_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().region("maincpu", 0);
	map(0x278000, 0x287fff).ram();
	map(0x400000, 0x4fffff).ram();
	map(0xff8004, 0xff8004).r(FUNC(mt735_state::p4_r));
	map(0xff8005, 0xff8005).r(FUNC(mt735_state::p5_r));
}

static INPUT_PORTS_START( mt735 )
INPUT_PORTS_END

void mt735_state::mt735(machine_config &config)
{
	M68000(config, m_cpu, XTAL(48'000'000)/6);
	m_cpu->set_addrmap(AS_PROGRAM, &mt735_state::mt735_map);
}

ROM_START( mt735 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "spg_m_e_ic103.bin", 0, 0x20000, CRC(1ab58bc9) SHA1(c10d50f38819c037d28435b77e09f2b6923e8369) )
	ROM_LOAD16_BYTE( "spg_m_o_ic102.bin", 1, 0x20000, CRC(84d8446b) SHA1(b1cedd8b09556eb8118f79b012aeec5b61e3ff32) )
ROM_END

COMP( ????, mt735, 0, 0, mt735, mt735, mt735_state, empty_init, "Brother", "MT735", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
