// license:BSD-3-Clause
// copyright-holders:stonedDiscord
/*

Bally Wulff Technik 2000
German Fruit Machines / Gambling Machines

CPU: Motorola MC68008P10 DIP-48
RAM: Fujitsu MB84256A
RTC: Epson RTC-72421A
Timer: Motorola MC6840P
Serial: ST EF6850P
Audio: Yamaha YM2149F

Zentraleinheit 200.600.00

TODO:
Verify crystal.
Most of the memory map is guessed.
Trace the lamp outs.
*/


#include "emu.h"

#include "cpu/m68000/m68008.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "machine/msm6242.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/roc10937.h"

#include "speaker.h"

namespace {

class t2000_state : public driver_device
{
public:
	t2000_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nvram(*this, "nvram"),
		m_aysnd(*this, "aysnd"),
		m_ptm(*this, "ptm"),
		m_acia(*this, "acia"),
		m_rtc(*this, "rtc")
	{ }

	void t2000(machine_config &config) ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	// devices
	required_device<m68008_device> m_maincpu;
	required_device<nvram_device> m_nvram;
	required_device<ym2149_device> m_aysnd;
	required_device<ptm6840_device> m_ptm;
	required_device<acia6850_device> m_acia;
	required_device<rtc72421_device> m_rtc;
};

void t2000_state::mem_map(address_map &map)
{
	map(0x00000, 0x3ffff).rom();
	map(0x40000, 0x47fff).ram().share("nvram"); //84256A
	map(0x80000, 0x8000f).rw(m_rtc, FUNC(rtc72421_device::read), FUNC(rtc72421_device::write));
	map(0xc0000, 0xc0007).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0xc0010, 0xc0010).w(m_aysnd, FUNC(ym2149_device::address_w));
	map(0xc0012, 0xc0012).rw(m_aysnd, FUNC(ym2149_device::data_r), FUNC(ym2149_device::data_w));
	map(0xc0020, 0xc0020).rw(m_acia, FUNC(acia6850_device::status_r), FUNC(acia6850_device::control_w));
	map(0xc0022, 0xc0022).rw(m_acia, FUNC(acia6850_device::data_r), FUNC(acia6850_device::data_w));
	map(0xd0000, 0xd000f).noprw(); // multiplexed inputs and outputs
}

static INPUT_PORTS_START( t2000 )
	PORT_START("IN0")
INPUT_PORTS_END


void t2000_state::t2000(machine_config &config)
{
	M68008(config, m_maincpu, 16_MHz_XTAL / 2); // guess
	m_maincpu->set_addrmap(AS_PROGRAM, &t2000_state::mem_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // battery backed

	YM2149(config, m_aysnd, 16_MHz_XTAL / 8); // guess
	m_aysnd->add_route(ALL_OUTPUTS, "mono", 1);

	SPEAKER(config, "mono").front_center();

	PTM6840(config, m_ptm, 16_MHz_XTAL / 4); // guess
	m_ptm->irq_callback().set_inputline("maincpu", M68K_IRQ_1);

	ACIA6850(config, m_acia);

	RTC72421(config, m_rtc, XTAL(32'768));
}

ROM_START( bmonop )
    ROM_REGION( 0x40000, "maincpu", 0 )
    ROM_LOAD("monopoly_ic2_27c1001.bin", 0x00000, 0x20000, CRC(ec000687) SHA1(4f8aec5eeece21681b2430c209eede223e30673e))
	ROM_LOAD("monopoly_ic4_27c1001.bin", 0x20000, 0x20000, CRC(d8c26b59) SHA1(ca78fe99bbba46ee49fcca261947a345e8badb50))
ROM_END

ROM_START( glorias )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "gloria_super_dm_pr1.bin", 0x00000, 0x20000, CRC(4f5615a7) SHA1(9264d4dc1bb651ad8c4f84873e6e14ebbe9cd477) )
	ROM_LOAD( "gloria_super_dm_pr2.bin", 0x20000, 0x20000, CRC(34964967) SHA1(4dd4a918fcd00a35aca443cbbce1ee0cf0c25c3b) )
ROM_END

ROM_START( graffiti )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "graffiti_281_0_e6.0.bin", 0x00000, 0x20000, CRC(90333f4c) SHA1(a81de22627f86c4889cbd65ff7a45a3d38966cc8) )
	ROM_LOAD( "graffiti_281_2_e6.0.bin", 0x20000, 0x20000, CRC(0d94cab0) SHA1(a6a27208f0bc1c529d60c0574f00305f64fb7ade) )
ROM_END

ROM_START( roxyc )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "roxy_classic_dm_pr1.bin", 0x00000, 0x20000, CRC(23d7169c) SHA1(e154e57e8ca03dce190178a0221a059f9b00085e) )
	ROM_LOAD( "roxy_classic_dm_pr2.bin", 0x20000, 0x20000, CRC(f7e86f09) SHA1(8144378332b21bfe0a91c9124d37afcc9946d367) )
ROM_END

} // anonymous namespace

GAME(1994, bmonop,   0, t2000, t2000, t2000_state, empty_init, ROT0, "Rototron", "Monopoly",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_REQUIRES_ARTWORK )
GAME(1995, graffiti, 0, t2000, t2000, t2000_state, empty_init, ROT0, "Rototron", "Graffiti",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_REQUIRES_ARTWORK )
GAME(1997, glorias,  0, t2000, t2000, t2000_state, empty_init, ROT0, "Rototron", "Gloria Super", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_REQUIRES_ARTWORK )
GAME(1997, roxyc,    0, t2000, t2000, t2000_state, empty_init, ROT0, "Rototron", "Roxy Classic", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_REQUIRES_ARTWORK )
