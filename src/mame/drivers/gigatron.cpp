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
#include "screen.h"
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


private:

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void prog_map(address_map &map);
	void data_map(address_map &map);

	required_device<cpu_device> m_maincpu;
};

uint32_t gigatron_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

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
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(gigatron_state::screen_update));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

}

ROM_START( gigatron )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_SYSTEM_BIOS(0, "v4", "Gigatron ROM V4")
	ROMX_LOAD( "ROMv4.rom",  0x0000, 0x20000, CRC(78995109) SHA1(2395fc48e64099836111f5aeca39ddbf4650ea4e),ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v3", "Gigatron ROM V3")
	ROMX_LOAD( "ROMv3.rom",  0x0000, 0x20000, CRC(1536efbe) SHA1(959268069e761a01d620396eedb9abc1ee63c421),ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v2", "Gigatron ROM V2")
	ROMX_LOAD( "ROMv2.rom",  0x0000, 0x20000, CRC(b4a3d936) SHA1(c93f417d589144b912c79f85b9e942d66242c2c3),ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v1", "Gigatron ROM V1")
	ROMX_LOAD( "ROMv1.rom",  0x0000, 0x20000, CRC(8ea5a2af) SHA1(e5758d5cc467c3476bd8f992fd45dfcdf06d0430),ROM_BIOS(3))
ROM_END

COMP(2018, gigatron, 0, 0, gigatron, gigatron, gigatron_state, empty_init, "Marcel van Kervinck", "Gigatron TTL Microcomputer", MACHINE_IS_SKELETON)
