// license:BSD-3-Clause
// copyright-holders:Robbbert
// PINBALL
// Skeleton driver for Stargame pinballs (2 x Z80, Z80CTC, DAC, AY8910, MEA8000).
// Hardware listing and ROM definitions from PinMAME.



#include "emu.h"
#include "machine/genpin.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/mea8000.h"
#include "speaker.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"


class stargame_state : public genpin_class
{
public:
	stargame_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_ctc(*this, "ctc")
		{ }

	DECLARE_WRITE8_MEMBER(rint_w);
	DECLARE_MACHINE_RESET(stargame);

	void stargame(machine_config &config);
	void audiocpu_io(address_map &map);
	void audiocpu_map(address_map &map);
	void maincpu_io(address_map &map);
	void maincpu_map(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<z80ctc_device> m_ctc;
};

void stargame_state::maincpu_map(address_map &map)
{
	map(0x0000, 0x3fff).mirror(0x4000).rom();
	map(0x8000, 0x87ff).mirror(0x7800).ram();
}

void stargame_state::maincpu_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).mirror(0x09).w("soundlatch", FUNC(generic_latch_8_device::write)); // CSON - command to the sound cpu - NMI to sound cpu
	map(0x10, 0x1f).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x20, 0x2f); // CSW2 - input lines: P0 to D4; P1 to D5; DAPRI to D6; TEST to D7
	map(0x30, 0x3f); // CSW1 - input lines
	map(0x40, 0x4f); // CDIG - score display
	map(0x50, 0x5f); // CDRIV
	map(0x60, 0x67).w("mainlatch", FUNC(ls259_device::write_d0)); // CPOR
	map(0x68, 0x69).mirror(0x06); // CPOR : 68=CKPRI; 69=CKDIS
	map(0x70, 0x73).mirror(0x0c).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

void stargame_state::audiocpu_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x4001).mirror(0x3ffe).rw("mea8000", FUNC(mea8000_device::read), FUNC(mea8000_device::write));
	map(0x8000, 0x87ff).mirror(0x3800).ram();
	map(0xc000, 0xdfff).w(this, FUNC(stargame_state::rint_w)); // RINT - turn off interrupt of the audiocpu
	map(0xe000, 0xffff).rw("soundlatch", FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::acknowledge_w)); // COMAND - acknowledge NMI and read the sound command
}

void stargame_state::audiocpu_io(address_map &map)
{
	map.global_mask(0x01);
	map(0x00, 0x00).r("ay", FUNC(ay8910_device::data_r));
	map(0x01, 0x01).w("ay", FUNC(ay8910_device::data_address_w));
}

static INPUT_PORTS_START( stargame )
INPUT_PORTS_END

WRITE8_MEMBER( stargame_state::rint_w )
{
	m_audiocpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

MACHINE_RESET_MEMBER( stargame_state, stargame )
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE); // doesn't do anything? cpu should be stuck in reset until allowed to start by SRESET line
}

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};


MACHINE_CONFIG_START(stargame_state::stargame)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, 15000000 / 4) // clock line marked as CK4 and derived from 15MHz crystal
	MCFG_DEVICE_PROGRAM_MAP(maincpu_map)
	MCFG_DEVICE_IO_MAP(maincpu_io)
	MCFG_Z80_DAISY_CHAIN(daisy_chain)

	MCFG_DEVICE_ADD("audiocpu", Z80, 15000000 / 3) // ? check divider - clock line marked as CK6 and derived from 15MHz crystal
	MCFG_DEVICE_PROGRAM_MAP(audiocpu_map)
	MCFG_DEVICE_IO_MAP(audiocpu_io)

	MCFG_MACHINE_RESET_OVERRIDE(stargame_state, stargame)

	/* video hardware */
	//MCFG_DEFAULT_LAYOUT()

	MCFG_DEVICE_ADD("ctc", Z80CTC, 15000000 / 4)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(INPUTLINE("audiocpu", INPUT_LINE_IRQ0))    // SINT - turn on interrupt of the audiocpu

	/* sound hardware */
	genpin_audio(config);
	SPEAKER(config, "measnd").front_center();
	MCFG_DEVICE_ADD("mea8000", MEA8000, 15000000 / 4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "measnd", 1.0)
	SPEAKER(config, "aysnd").front_center();
	MCFG_DEVICE_ADD("ay", AY8910, 15000000 / 8) // clock line marked as CK2 and derived from 15MHz crystal
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "aysnd", 0.25)

	MCFG_DEVICE_ADD("mainlatch", LS259, 0)
	MCFG_ADDRESSABLE_LATCH_Q0_OUT_CB(NOOP) // DADIS
	MCFG_ADDRESSABLE_LATCH_Q1_OUT_CB(NOOP) // DAPRI
	MCFG_ADDRESSABLE_LATCH_Q2_OUT_CB(NOOP) // RJUEGO
	MCFG_ADDRESSABLE_LATCH_Q3_OUT_CB(NOOP) // RFLIPPER
	MCFG_ADDRESSABLE_LATCH_Q4_OUT_CB(NOOP) // to AUXILLIAR socket
	MCFG_ADDRESSABLE_LATCH_Q5_OUT_CB(NOOP) // RFDIS
	MCFG_ADDRESSABLE_LATCH_Q6_OUT_CB(INPUTLINE("audiocpu", INPUT_LINE_RESET)) MCFG_DEVCB_INVERT // SRESET
	MCFG_ADDRESSABLE_LATCH_Q7_OUT_CB(NOOP) // MAKRES

	MCFG_GENERIC_LATCH_8_ADD("soundlatch")
	MCFG_GENERIC_LATCH_DATA_PENDING_CB(INPUTLINE("audiocpu", INPUT_LINE_NMI))

	MCFG_WATCHDOG_ADD("watchdog")
MACHINE_CONFIG_END

ROM_START(spcship)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("sss-1g.bin", 0x0000, 0x4000, CRC(119a3064) SHA1(d915ecf44279a9e16a50a723eb9523afec1fb380))
	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("sss-1a0.bin", 0x0000, 0x4000, CRC(eae78e63) SHA1(9fa3587ae3ee6f674bb16102680e70069e9d275e))
ROM_END


ROM_START(whtforce)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("m5l.bin", 0x0000, 0x4000, CRC(22495322) SHA1(b34a34dec875f215d566d18a5e877b9185a22ab7))
	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("sound.bin", 0x0000, 0x4000, CRC(4b2a1580) SHA1(62133fd186b1aab4f5aecfbff8151ba416328021))
ROM_END


GAME( 1986, spcship,  0, stargame, stargame, stargame_state, empty_init, ROT0, "Stargame", "Space Ship (Pinball)",  MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, whtforce, 0, stargame, stargame, stargame_state, empty_init, ROT0, "Stargame", "White Force",           MACHINE_IS_SKELETON_MECHANICAL )
