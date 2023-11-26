// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/sh/sh7014.h"

#include "debugger.h"
#include "speaker.h"

class psr540_state : public driver_device {
public:
	psr540_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_boot(*this, "boot")
	{ }

	void psr540(machine_config &config);

private:
	required_device<sh2_device> m_maincpu;
	required_shared_ptr<u32> m_boot;

	void map(address_map &map);

	void machine_start() override;
};

void psr540_state::machine_start()
{
	m_boot[0] = 0x1000;
	m_boot[0x400] = 0xdf03d004;
	m_boot[0x401] = 0x400b0009;
	m_boot[0x402] = 0xaffe0009;
	m_boot[0x404] = 0x01400000;
	m_boot[0x405] = 0x4d9010;
	m_boot[0x499c/4] = 0x000b0009;
}

void psr540_state::psr540(machine_config &config)
{
	SH2_SH7014(config, m_maincpu, 7_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &psr540_state::map);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}

void psr540_state::map(address_map &map)
{
	map(0x0000000, 0x003ffff).ram().share(m_boot).unmapw();
	map(0x0400000, 0x07fffff).rom().region("program_rom", 0);
	map(0x1000000, 0x13fffff).ram(); // dram
}

static INPUT_PORTS_START( psr540 )
INPUT_PORTS_END

ROM_START( psr540 )
	ROM_REGION32_BE( 0x400000, "program_rom", 0 )
	ROM_LOAD16_WORD_SWAP( "xw25320.ic310", 0, 0x400000, CRC(e8d29e49) SHA1(079e0ccf6cf5d5bd2d2d82076b09dd702fcd1421))

	ROM_REGION16_LE( 0x600000, "swx00", 0)
	ROM_LOAD16_WORD_SWAP( "xw25320.ic310", 0, 0x400000, CRC(c7c4736d) SHA1(ff1052eb076557071ed8652e6c2fc0925144fbd5))
	ROM_LOAD16_WORD_SWAP( "xw25320.ic310", 0, 0x200000, CRC(9ef56c4e) SHA1(f26b588f9bcfd7bdbf1c0b38e4a1ea57e2f29f10))
ROM_END

SYST( 1999, psr540, 0, 0, psr540, psr540, psr540_state, empty_init, "Yamaha", "PSR540", MACHINE_IS_SKELETON )
