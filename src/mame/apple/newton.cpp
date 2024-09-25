// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Apple Newton devices skeleton driver

    CPUs:
        Newton MessagePad:        ARM 610 (20MHz)
        ExpertPad PI-7000: ARM 610 (20MHz)
        Newton MessagePad 100:    ARM 610 (20MHz)
        Newton MessagePad 110:    ARM 610 (20MHz)
        Newton MessagePad 120:    ARM 610 (20MHz)
        Marco:             ARM 610 (20MHz)
        Newton MessagePad 130:    ARM 610 (20MHz)

        eMate 300:         ARM 710a (25MHz)

        Newton MessagePad 2000:   StrongARM SA-110 (162MHz)
        Newton MessagePad 2100:   StrongARM SA-110 (162MHz)

****************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "machine/ram.h"
#include "emupal.h"
#include "screen.h"


namespace {

class newton_state : public driver_device
{
public:
	newton_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, "ram")
	{ }

	void gen1(machine_config &config);
	void mp120(machine_config &config);
	void marco(machine_config &config);
	void mp130(machine_config &config);
	void emate(machine_config &config);
	void mp2000(machine_config &config);
	void mp2100(machine_config &config);

protected:
	void mem_map(address_map &map) ATTR_COLD;

	uint32_t tick_count_r();

	required_device<arm7_cpu_device> m_maincpu;
	required_device<ram_device> m_ram;

	uint32_t m_ram_size = 0;
};

uint32_t newton_state::tick_count_r()
{
	return (uint32_t)m_maincpu->total_cycles();
}

void newton_state::mem_map(address_map &map)
{
	map(0x00000000, 0x007fffff).mirror(0x00800000).rom().region("maincpu", 0);
	map(0x02000000, 0x023fffff).ram(); // Actually Flash
	map(0x04000000, 0x04ffffff).rw(m_ram, FUNC(ram_device::read), FUNC(ram_device::write));
	map(0x0f181800, 0x0f181803).r(FUNC(newton_state::tick_count_r));
	map(0x0f001800, 0x0f001803).lrw32(NAME([this](){ return m_ram_size; }), NAME([this](uint32_t data) { m_ram_size = data; }));
}

static INPUT_PORTS_START( newton )
INPUT_PORTS_END

void newton_state::gen1(machine_config &config)
{
	ARM7(config, m_maincpu, XTAL(20'000'000)); // really ARM610
	m_maincpu->set_addrmap(AS_PROGRAM, &newton_state::mem_map);

	RAM(config, m_ram);
	m_ram->set_default_size("640K");
}

void newton_state::mp120(machine_config &config)
{
	gen1(config);
	m_ram->set_default_size("1M");
	m_ram->set_extra_options("2M");
}

void newton_state::marco(machine_config &config)
{
	gen1(config);
	m_ram->set_extra_options("687K");
}

void newton_state::mp130(machine_config &config)
{
	gen1(config);
	m_ram->set_default_size("2560K");
}

void newton_state::emate(machine_config &config)
{
	ARM710A(config, m_maincpu, 162000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &newton_state::mem_map);

	RAM(config, m_ram);
	m_ram->set_default_size("1M");
	m_ram->set_extra_options("2M");
}

void newton_state::mp2000(machine_config &config)
{
	ARM710A(config, m_maincpu, 162000000); // really SA110
	m_maincpu->set_addrmap(AS_PROGRAM, &newton_state::mem_map);

	RAM(config, m_ram);
	m_ram->set_default_size("1M");
}

void newton_state::mp2100(machine_config &config)
{
	mp2000(config);
	m_ram->set_default_size("4M");
}

/* ROM definition */
ROM_START( newtnotp )
	ROM_REGION32_LE( 0x800000, "maincpu", 0 )
	ROMX_LOAD( "v10b1.rom", 0x000000, 0x400000, CRC(9fec5b35) SHA1(87ae4afe72814117f9100b67c6fda7010463a0f8), ROM_REVERSE | ROM_GROUPDWORD )
ROM_END

ROM_START( newtonmp )
	ROM_REGION32_LE( 0x800000, "maincpu", 0 )
	ROM_SYSTEM_BIOS(0, "v100", "v1.00")
	ROMX_LOAD( "v100.rom", 0x000000, 0x400000, CRC(bab51f17) SHA1(5e754fe7db01ec3c331ff4d71b2dc4565eafce98), ROM_BIOS(0) | ROM_REVERSE | ROM_GROUPDWORD )
	ROM_SYSTEM_BIOS(1, "v13", "v1.3")
	ROMX_LOAD( "v13.rom", 0x000000, 0x400000, CRC(8976832c) SHA1(964d07743589cf854fbbcd5e51e3289b739d8050), ROM_BIOS(1) | ROM_REVERSE | ROM_GROUPDWORD )
ROM_END

ROM_START( spi7000 )
	ROM_REGION32_LE( 0x800000, "maincpu", 0 )
	ROMX_LOAD( "v110.rom", 0x000000, 0x400000, CRC(0a2e0d96) SHA1(1e8e4c74ca19eee120b2647b267e2c467b668f1f), ROM_REVERSE | ROM_GROUPDWORD )
ROM_END

ROM_START( mp110 )
	ROM_REGION32_LE( 0x800000, "maincpu", 0 )
	ROMX_LOAD( "v12.rom", 0x000000, 0x400000, CRC(291aac40) SHA1(517094fc26702b82d558d0c9d677a91de25d0b7f), ROM_REVERSE | ROM_GROUPDWORD )
ROM_END

ROM_START( mp120 )
	ROM_REGION32_LE( 0x800000, "maincpu", 0 )
	ROMX_LOAD( "v13.rom", 0x000000, 0x400000, CRC(d8a34419) SHA1(deda5023dbcb0c11bd6384e444a800c4a271312c), ROM_REVERSE | ROM_GROUPDWORD )
ROM_END

ROM_START( motmarco )
	ROM_REGION32_LE( 0x800000, "maincpu", 0 )
	ROMX_LOAD( "v13 444347.rom", 0x000000, 0x400000, CRC(ad79abc5) SHA1(5c4731008ac402b8f0be37158482b61b36e247cc), ROM_REVERSE | ROM_GROUPDWORD )
ROM_END

ROM_START( mp130 )
	ROM_REGION32_LE( 0x800000, "maincpu", 0 )
	ROMX_LOAD( "v2x.rom", 0x000000, 0x800000, CRC(88ac9c6c) SHA1(44e33b72328974ed0ac41a13fe0e56bf97d15be3), ROM_REVERSE | ROM_GROUPDWORD )
ROM_END

ROM_START( emate )
	ROM_REGION32_LE( 0x800000, "maincpu", 0 )
	ROMX_LOAD( "emate300.rom", 0x000000, 0x800000, CRC(782ea604) SHA1(dcc42e45a6914c7a771819856a1fa05892fe0519), ROM_REVERSE | ROM_GROUPDWORD )
ROM_END

ROM_START( mp2000 )
	ROM_REGION32_LE( 0x800000, "maincpu", 0 )
	ROMX_LOAD( "mp2000.rom", 0x000000, 0x800000, CRC(9001b0f8) SHA1(06751fa69b791febae7267e0486aa15eea933a53), ROM_REVERSE | ROM_GROUPDWORD )
ROM_END

ROM_START( mp2100 )
	ROM_REGION32_LE( 0x800000, "maincpu", 0 )
	ROMX_LOAD( "mp2100.rom", 0x000000, 0x800000, CRC(81d5efc6) SHA1(82a191652b2689ce0e254ee11c6f43c84b5185cc), ROM_REVERSE | ROM_GROUPDWORD )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT   CLASS         INIT        COMPANY           FULLNAME     FLAGS
CONS( 1992, newtnotp, 0,      0,      gen1,     newton, newton_state, empty_init, "Apple Computer", "Newton Notepad (prototype)", MACHINE_IS_SKELETON )
CONS( 1993, newtonmp, 0,      0,      gen1,     newton, newton_state, empty_init, "Apple Computer", "Newton MessagePad", MACHINE_IS_SKELETON )
CONS( 1993, spi7000,  0,      0,      gen1,     newton, newton_state, empty_init, "Sharp", "ExpertPad PI-7000", MACHINE_IS_SKELETON )
CONS( 1994, mp110,    0,      0,      gen1,     newton, newton_state, empty_init, "Apple Computer", "Newton MessagePad 110", MACHINE_IS_SKELETON )
CONS( 1995, mp120,    0,      0,      mp120,    newton, newton_state, empty_init, "Apple Computer", "Newton MessagePad 120", MACHINE_IS_SKELETON )
CONS( 1995, motmarco, 0,      0,      marco,    newton, newton_state, empty_init, "Motorola", "Marco", MACHINE_IS_SKELETON )
CONS( 1996, mp130,    0,      0,      mp130,    newton, newton_state, empty_init, "Apple Computer", "Newton MessagePad 130", MACHINE_IS_SKELETON )
CONS( 1997, emate,    0,      0,      emate,    newton, newton_state, empty_init, "Apple Computer", "eMate 300", MACHINE_IS_SKELETON )
CONS( 1997, mp2000,   0,      0,      mp2000,   newton, newton_state, empty_init, "Apple Computer", "Newton MessagePad 2000", MACHINE_IS_SKELETON )
CONS( 1997, mp2100,   0,      0,      mp2100,   newton, newton_state, empty_init, "Apple Computer", "Newton MessagePad 2100", MACHINE_IS_SKELETON )
