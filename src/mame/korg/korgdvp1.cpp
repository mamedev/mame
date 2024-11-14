// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Korg DVP-1 MIDI vocoder.

****************************************************************************/

#include "emu.h"
//#include "bus/midi/midi.h"
#include "cpu/upd7810/upd7810.h"
#include "cpu/tms32010/tms32010.h"
#include "machine/nvram.h"


namespace {

class korgdvp1_state : public driver_device
{
public:
	korgdvp1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dsp(*this, "dsp%u", 1U)
	{
	}

	void dvp1(machine_config &config);

private:
	u8 inputs_r();
	u8 dsp_int_r();
	void control_w(u8 data);
	void leds_w(offs_t offset, u8 data);
	void dsp_w(offs_t offset, u8 data);

	void main_map(address_map &map) ATTR_COLD;

	required_device<upd7810_device> m_maincpu;
	required_device_array<tms32010_device, 2> m_dsp;
};


u8 korgdvp1_state::inputs_r()
{
	return 0xff;
}

u8 korgdvp1_state::dsp_int_r()
{
	// PB3 = BUSY
	// PB6 = INT0
	// PB7 = INT1
	return 0xff;
}

void korgdvp1_state::control_w(u8 data)
{
	// PB0, PB1 = input select (HC139)
	// PB2 = MUTE
	// PB4 = DSP RES
	// PB5 = ROM/RAM
}

void korgdvp1_state::leds_w(offs_t offset, u8 data)
{
	for (int i = 0; i < 8; i++)
		if (BIT(offset, i))
			logerror("%s: Writing %02X to A%d LEDs\n", machine().describe_context(), data, i);
}

void korgdvp1_state::dsp_w(offs_t offset, u8 data)
{
	for (int i = 0; i < 2; i++)
		if (!BIT(offset, 9 - i))
			logerror("%s: Writing %02X to DSP %d\n", machine().describe_context(), data, i + 1);
}

void korgdvp1_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("program", 0);
	map(0x8000, 0x87ff).mirror(0x1800).ram().share("nvram");
	map(0xa000, 0xa0ff).mirror(0x1f00).w(FUNC(korgdvp1_state::leds_w));
	map(0xc000, 0xc000).select(0x300).mirror(0x1cff).w(FUNC(korgdvp1_state::dsp_w));
}


static INPUT_PORTS_START(dvp1)
INPUT_PORTS_END

void korgdvp1_state::dvp1(machine_config &config)
{
	UPD7810(config, m_maincpu, 12_MHz_XTAL); // µPD7811-161-36 (according to parts list) but with both mode pins pulled up
	m_maincpu->set_addrmap(AS_PROGRAM, &korgdvp1_state::main_map);
	m_maincpu->pa_in_cb().set(FUNC(korgdvp1_state::inputs_r));
	m_maincpu->pb_in_cb().set(FUNC(korgdvp1_state::dsp_int_r));
	m_maincpu->pb_out_cb().set(FUNC(korgdvp1_state::control_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // HM6116LP-4 + 3V lithium battery

	TMS32010(config, m_dsp[0], 20_MHz_XTAL).set_disable();
	TMS32010(config, m_dsp[1], 20_MHz_XTAL).set_disable();
}

ROM_START(dvp1)
	ROM_REGION(0x8000, "program", 0) // Version: SEP 28, 1985
	ROM_LOAD("850803.ic6", 0x0000, 0x8000, CRC(1170db85) SHA1(4ce773dd22c56982b9493f89dce62111eec596b3)) // MBM27256-25

	ROM_REGION16_LE(0xc00, "dsp1", 0) // 1536 x 16 internal bootloader
	ROM_LOAD("tms320m10nl.ic9", 0x000, 0xc00, NO_DUMP)

	ROM_REGION16_LE(0xc00, "dsp2", 0) // 1536 x 16 internal bootloader (almost certainly identical to DSP 1)
	ROM_LOAD("tms320m10nl.ic8", 0x000, 0xc00, NO_DUMP)
ROM_END

} // anonymous namespace


SYST(1985, dvp1, 0, 0, dvp1, dvp1, korgdvp1_state, empty_init, "Korg", "DVP-1 Digital Voice Processor", MACHINE_IS_SKELETON)
