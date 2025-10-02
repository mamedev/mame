// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for Yamaha AN1x Control Synthesizer.

    This is roughly similar in principle to the MU-80, but the sound chip is a
    YSS236-F VOP3 instead of SWP20. A second VOP3 (with its own ROM) is on the
    separate DMS board.

*******************************************************************************/

#include "emu.h"
//#include "bus/midi/midi.h"
#include "cpu/h8/h83002.h"
#include "cpu/m6805/hd6305.h"
#include "mulcd.h"
#include "machine/nvram.h"
#include "sound/meg.h"
#include "speaker.h"

namespace {

class an1x_state : public driver_device
{
public:
	an1x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pks(*this, "pks")
		, m_meg(*this, "meg")
	{
	}

	void an1x(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<h83002_device> m_maincpu;
	required_device<cpu_device> m_pks;
	required_device<meg_device> m_meg;
};

void an1x_state::mem_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom().region("program", 0);
	map(0x200000, 0x23ffff).ram().share("nvram");
}

static INPUT_PORTS_START(an1x)
INPUT_PORTS_END

void an1x_state::an1x(machine_config &config)
{
	H83002(config, m_maincpu, 16_MHz_XTAL); // HD6413002FP16
	m_maincpu->set_addrmap(AS_PROGRAM, &an1x_state::mem_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // 2x HM628128BLFP-8 + CR2450 battery

	HD6305V0(config, m_pks, 8_MHz_XTAL).set_disable(); // HD63B05V0E65F

	MULCD(config, "lcd"); // LC7985ND (back-lit)

	SPEAKER(config, "speaker", 2).front();

	MEG(config, m_meg, 11.2896_MHz_XTAL);
}

ROM_START(an1x)
	ROM_REGION16_BE(0x100000, "program", 0)
	ROM_LOAD16_WORD_SWAP("yamaha an1x 1.04.ic2", 0x000000, 0x100000, CRC(f03b8c30) SHA1(778b669a450660f2b15cc385f156dc6676c437f1))

	ROM_REGION(0x1000, "pks", 0)
	ROM_LOAD("xn668a00.bin", 0x0000, 0x1000, NO_DUMP)

	ROM_REGION16_BE(0x20000, "vop3", 0)
	ROM_LOAD("xt113b00.ic9", 0x00000, 0x20000, NO_DUMP) // LH531024

	ROM_REGION16_BE(0x20000, "vop3s", 0)
	ROM_LOAD("xt113b00.ic1", 0x00000, 0x20000, NO_DUMP) // LH531024
ROM_END

} // anonymous namespace

SYST(1997, an1x, 0, 0, an1x, an1x, an1x_state, empty_init, "Yamaha", "AN1x Control Synthesizer", MACHINE_NOT_WORKING)
