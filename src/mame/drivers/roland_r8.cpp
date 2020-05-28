// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Roland R-8 drum machine.

****************************************************************************/

#include "emu.h"
#include "cpu/upd78k/upd78k2.h"
#include "machine/nvram.h"
#include "sound/rolandpcm.h"
#include "speaker.h"

class roland_r8_state : public driver_device
{
public:
	roland_r8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pcm(*this, "pcm")
	{
	}

	void r8(machine_config &config);
	void r8m(machine_config &config);
	void r8mk2(machine_config &config);

private:
	void mk1_map(address_map &map);
	void mk2_map(address_map &map);

	required_device<upd78k2_device> m_maincpu;
	required_device<mb87419_mb87420_device> m_pcm;
};


void roland_r8_state::mk1_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom().region("maincpu", 0);
	map(0x20000, 0x27fff).ram().share("nvram");
	map(0x70000, 0x7001f).rw(m_pcm, FUNC(mb87419_mb87420_device::read), FUNC(mb87419_mb87420_device::write));
}

void roland_r8_state::mk2_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom().region("maincpu", 0);
	map(0x20000, 0x27fff).ram().share("nvram1");
	map(0x28000, 0x2ffff).ram().share("nvram2");
	map(0x70000, 0x7001f).rw(m_pcm, FUNC(mb87419_mb87420_device::read), FUNC(mb87419_mb87420_device::write));
}


static INPUT_PORTS_START(r8)
INPUT_PORTS_END


void roland_r8_state::r8(machine_config &config)
{
	UPD78210(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_r8_state::mk1_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // SRM20256LC-12 + battery

	//bu3904s_device &fsk(BU3904S(config, "fsk", 12_MHz_XTAL));
	//fsk.xint_callback().set_inputline(m_maincpu, upd78k2_device::INTP0_LINE);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MB87419_MB87420(config, m_pcm, 33.8688_MHz_XTAL);
	//m_pcm->int_callback().set_inputline(m_maincpu, upd78k2_device::INTP1_LINE);
	m_pcm->set_device_rom_tag("wavedata");
	m_pcm->add_route(0, "lspeaker", 1.0);
	m_pcm->add_route(1, "rspeaker", 1.0);
}

void roland_r8_state::r8m(machine_config &config)
{
	r8(config);

	UPD78213(config.replace(), m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_r8_state::mk1_map);
}

void roland_r8_state::r8mk2(machine_config &config)
{
	UPD78213(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_r8_state::mk2_map);

	NVRAM(config, "nvram1", nvram_device::DEFAULT_ALL_0); // SRM20256LC-10 + battery
	NVRAM(config, "nvram2", nvram_device::DEFAULT_ALL_0); // SRM20256LC-10 + battery

	//bu3904s_device &fsk(BU3904S(config, "fsk", 12_MHz_XTAL));
	//fsk.xint_callback().set_inputline(m_maincpu, upd78k2_device::INTP0_LINE);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MB87419_MB87420(config, m_pcm, 33.8688_MHz_XTAL);
	//m_pcm->int_callback().set_inputline(m_maincpu, upd78k2_device::INTP1_LINE);
	m_pcm->set_device_rom_tag("wavedata");
	m_pcm->add_route(0, "lspeaker", 1.0);
	m_pcm->add_route(1, "rspeaker", 1.0);
}


ROM_START(r8)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("roland r-8_2.02_27c010.bin", 0x00000, 0x20000, CRC(45d0f64f) SHA1(55f0831db74cbdeae20cd7f1ff28af27dafba9b9))

	ROM_REGION(0x100000, "wavedata", ROMREGION_ERASE00)
	ROM_LOAD("r15179929-mn234000rle.ic30", 0x000000, 0x080000, NO_DUMP)
	ROM_LOAD("r15179930-mn234000rlf.ic31", 0x080000, 0x080000, NO_DUMP)
ROM_END

ROM_START(r8m)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("rolandr8mv104.bin", 0x00000, 0x20000, CRC(5e95e2f6) SHA1(b4e1a8f15f72a9db9aa8fd41ee3c3ebd10460587))

	ROM_REGION(0x100000, "wavedata", ROMREGION_ERASE00) // same ROMs as R-8 assumed
	ROM_LOAD("r15179929-mn234000rle.bin", 0x000000, 0x080000, NO_DUMP)
	ROM_LOAD("r15179930-mn234000rlf.bin", 0x080000, 0x080000, NO_DUMP)
ROM_END

ROM_START(r8mk2)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("roland r8 mkii eprom v1.0.3.bin", 0x00000, 0x20000, CRC(128a9a0c) SHA1(94bd8c76efe270754219f2899f31b62fc4f9060d))

	ROM_REGION(0x200000, "wavedata", ROMREGION_ERASE00)
	ROM_LOAD("r15209440-upd27c8001eacz-025.ic30", 0x000000, 0x080000, NO_DUMP)
	ROM_LOAD("r15209441-upd27c8001eacz-026.ic31", 0x080000, 0x080000, NO_DUMP)
	ROM_LOAD("r15209442-upd27c8001eacz-027.ic82", 0x100000, 0x080000, NO_DUMP)
ROM_END


SYST(1989, r8,    0,  0, r8,    r8, roland_r8_state, empty_init, "Roland", "R-8 Human Rhythm Composer (v2.02)", MACHINE_IS_SKELETON)
SYST(1990, r8m,   r8, 0, r8m,   r8, roland_r8_state, empty_init, "Roland", "R-8M Total Percussion Sound Module (v1.04)", MACHINE_IS_SKELETON)
SYST(1992, r8mk2, 0,  0, r8mk2, r8, roland_r8_state, empty_init, "Roland", "R-8 Mk II Human Rhythm Composer (v1.0.3)", MACHINE_IS_SKELETON)
