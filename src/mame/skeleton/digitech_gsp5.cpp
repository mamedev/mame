// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for DigiTech GSP 5 guitar effects processor.

****************************************************************************/

#include "emu.h"
//#include "bus/midi/midi.h"
#include "cpu/m6800/m6801.h"
#include "machine/nvram.h"

namespace {

class gsp5_state : public driver_device
{
public:
	gsp5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void gsp5(machine_config &config);

private:
	void ls_w(offs_t offset, u8 data);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

void gsp5_state::ls_w(offs_t offset, u8 data)
{
	logerror("%s: Writing $%02X to LS%c latch\n", machine().describe_context(), data, 'A' + offset);
}

void gsp5_state::mem_map(address_map &map)
{
	map(0x2000, 0x2006).mirror(0x1ff8).w(FUNC(gsp5_state::ls_w));
	map(0x4000, 0x5fff).ram().share("nvram");
	map(0x8000, 0xffff).rom().region("eprom", 0);
}

static INPUT_PORTS_START(gsp5)
INPUT_PORTS_END

void gsp5_state::gsp5(machine_config &config)
{
	HD6303R(config, m_maincpu, 4_MHz_XTAL); // HD6303RP
	m_maincpu->set_addrmap(AS_PROGRAM, &gsp5_state::mem_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // KM6264A-10 + 3V battery

	//DSP256(config, "dsp", 32_MHz_XTAL);
}

ROM_START(gsp5)
	ROM_REGION(0x8000, "eprom", 0)
	ROM_LOAD("gsp-5_v1.02_1190.u53", 0x0000, 0x8000, CRC(02868045) SHA1(2e0682e3020ef8a5e1aa0fa28ac84450e08cbf1b)) // 27C256
	// Socket for second EPROM (27C128) exists at U63 but is unpopulated
ROM_END

} // anonymous namespace

SYST(1989, gsp5, 0, 0, gsp5, gsp5, gsp5_state, empty_init, "DigiTech", "GSP 5 Guitar Effects Processor/Preamp", MACHINE_IS_SKELETON)
