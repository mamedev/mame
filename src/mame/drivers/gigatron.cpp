// license:BSD-3-Clause
// copyright-holders:Sterophonick
/***************************************************************************

   Skeleton driver for Gigatron TTL Microcomputer
   Driver by Sterophonick

***************************************************************************/

//There is a pre-existing emulator here https://github.com/PhilThomas/gigatron
//It's just a matter of translating it to MAME.

#include "emu.h"
#include "cpu/gigatron/gigatron.h"
#include "machine/nvram.h"
#include "speaker.h"

#define MAIN_CLOCK 6250000

class gigatron_state : public driver_device
{
public:
	gigatron_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void gigatron(machine_config &config);
	
	void init_gigatron();

private:

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void prog_map(address_map &map);
	void data_map(address_map &map);

	required_device<cpu_device> m_maincpu;
};

void gigatron_state::prog_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
}

void gigatron_state::data_map(address_map &map)
{
}

void gigatron_state::machine_start()
{
}

void gigatron_state::machine_reset()
{
}

static INPUT_PORTS_START(gigatron)
INPUT_PORTS_END

void gigatron_state::gigatron(machine_config &config)
{
	GTRON(config, m_maincpu, MAIN_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &gigatron_state::prog_map);
	m_maincpu->set_addrmap(AS_DATA, &gigatron_state::data_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(640, 480);
	screen.set_visarea(0, 640, 0, 480);

	SPEAKER(config, "mono").front_center();

}

ROM_START( gigatron )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "gigatron.rom",  0x0000, 0x20000, CRC(78995109) SHA1(2395fc48e64099836111f5aeca39ddbf4650ea4e) )
ROM_END

COMP(2018, gigatron,         0, gigatron, gigatron, gigatron_state, empty_init, ROT0, "Marcel van Kervinck", "Gigatron TTL Microcomputer", MACHINE_IS_SKELETON)