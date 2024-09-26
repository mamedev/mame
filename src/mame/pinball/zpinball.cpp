// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************
PINBALL
Skeleton driver for EFO "Z-Pinball" hardware.

ToDo:
- Inputs
- Outputs
- Screen
- Sound
- Mechanical sounds

****************************************************************************/

#include "emu.h"

#include "efo_zsu.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/z80ctc.h"
#include "sound/saa1099.h"

#include "speaker.h"


namespace {

class zpinball_state : public driver_device
{
public:
	zpinball_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_zpumpu(*this, "zpumpu")
		, m_zpuctc(*this, "zpuctc")
		, m_zsu(*this, "zsu")
		, m_pal_input(0)
		, m_hc165_data(0)
		, m_shift_clock(false)
		, m_shift_enabled(false)
	{
	}

	void zpinball(machine_config &config);
	void eballchps(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u8 pal_r();
	void pal_w(u8 data);
	void shift_load_w(u8 data);

	void shift_toggle_w(int state);
	void clock_off_w(int state);

	u8 in1_r();
	u8 in2_r();
	void out1_w(u8 data);
	void out2_w(u8 data);
	void out3_w(u8 data);
	void strobes_w(u8 data);
	void o0_w(u8 data);
	void o1_w(u8 data);
	void o2_w(u8 data);
	void o3_w(u8 data);
	void o4_w(u8 data);
	void o5_w(u8 data);
	void o6_w(u8 data);

	void zpu_mem(address_map &map) ATTR_COLD;
	void zpu_io(address_map &map) ATTR_COLD;

	required_device<z80_device> m_zpumpu;
	required_device<z80ctc_device> m_zpuctc;
	required_device<efo_zsu_device> m_zsu;

	u8 m_pal_input;
	u8 m_hc165_data;
	bool m_shift_clock;
	bool m_shift_enabled;
};


void zpinball_state::machine_start()
{
	save_item(NAME(m_pal_input));
	save_item(NAME(m_hc165_data));
	save_item(NAME(m_shift_clock));
	save_item(NAME(m_shift_enabled));
}

void zpinball_state::machine_reset()
{
	m_shift_clock = false;
	m_shift_enabled = false;
	m_zpuctc->subdevice("ch0")->set_unscaled_clock(0);

	// Clear latches
	out1_w(0);
	out2_w(0);
	out3_w(0);
	strobes_w(0);
	o0_w(0);
	o1_w(0);
	o2_w(0);
	o3_w(0);
	o4_w(0);
	o5_w(0);
	o6_w(0);
}


u8 zpinball_state::pal_r()
{
	// TODO: at least simulate this, according to PinMAME returning 0x9b is enough to circumvent the protection
	return m_pal_input;
}

void zpinball_state::pal_w(u8 data)
{
	m_pal_input = data;
}

void zpinball_state::shift_load_w(u8 data)
{
	m_hc165_data = data;
	m_shift_enabled = true;
	m_zpuctc->subdevice("ch0")->set_unscaled_clock(8_MHz_XTAL / 4);
}

void zpinball_state::shift_toggle_w(int state)
{
	if (state && m_shift_enabled)
	{
		m_shift_clock = !m_shift_clock;
		if (m_shift_clock)
			m_hc165_data <<= 1;
	}
}

void zpinball_state::clock_off_w(int state)
{
	if (state)
	{
		m_shift_clock = false;
		m_shift_enabled = false;
		m_zpuctc->subdevice("ch0")->set_unscaled_clock(0);
	}
}

u8 zpinball_state::in1_r()
{
	// TODO
	return 0;
}

u8 zpinball_state::in2_r()
{
	// TODO
	return 0;
}

void zpinball_state::out1_w(u8 data)
{
	logerror("%s: out1_w(0x%02X)\n", machine().describe_context(), data);
}

void zpinball_state::out2_w(u8 data)
{
	logerror("%s: out2_w(0x%02X)\n", machine().describe_context(), data);
}

void zpinball_state::out3_w(u8 data)
{
	logerror("%s: out3_w(0x%02X)\n", machine().describe_context(), data);
}

void zpinball_state::strobes_w(u8 data)
{
	logerror("%s: strobes_w(0x%02X)\n", machine().describe_context(), data);
}

void zpinball_state::o0_w(u8 data)
{
	logerror("%s: o0_w(0x%02X)\n", machine().describe_context(), data);
}

void zpinball_state::o1_w(u8 data)
{
	logerror("%s: o1_w(0x%02X)\n", machine().describe_context(), data);
}

void zpinball_state::o2_w(u8 data)
{
	logerror("%s: o2_w(0x%02X)\n", machine().describe_context(), data);
}

void zpinball_state::o3_w(u8 data)
{
	logerror("%s: o3_w(0x%02X)\n", machine().describe_context(), data);
}

void zpinball_state::o4_w(u8 data)
{
	logerror("%s: o4_w(0x%02X)\n", machine().describe_context(), data);
}

void zpinball_state::o5_w(u8 data)
{
	logerror("%s: o5_w(0x%02X)\n", machine().describe_context(), data);
}

void zpinball_state::o6_w(u8 data)
{
	logerror("%s: o6_w(0x%02X)\n", machine().describe_context(), data);
}


void zpinball_state::zpu_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("zpurom", 0);
	map(0x8000, 0x87ff).mirror(0x1800).ram().share("nvram");
	map(0xa000, 0xa000).mirror(0x1fff).rw(FUNC(zpinball_state::pal_r), FUNC(zpinball_state::pal_w));
	map(0xc000, 0xc000).mirror(0x1ff8).w(FUNC(zpinball_state::o0_w));
	map(0xc001, 0xc001).mirror(0x1ff8).w(FUNC(zpinball_state::o1_w));
	map(0xc002, 0xc002).mirror(0x1ff8).w(FUNC(zpinball_state::o2_w));
	map(0xc003, 0xc003).mirror(0x1ff8).w(FUNC(zpinball_state::o3_w));
	map(0xc004, 0xc004).mirror(0x1ff8).w(FUNC(zpinball_state::o4_w));
	map(0xc005, 0xc005).mirror(0x1ff8).w(FUNC(zpinball_state::o5_w));
	map(0xc006, 0xc006).mirror(0x1ff8).w(FUNC(zpinball_state::o6_w));
	map(0xe000, 0xe001).mirror(0x1ffe).w("saa", FUNC(saa1099_device::write));
}

void zpinball_state::zpu_io(address_map &map)
{
	map.global_mask(0x1f);
	map(0x00, 0x03).rw(m_zpuctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x04, 0x04).mirror(3).r(FUNC(zpinball_state::in1_r));
	map(0x08, 0x08).mirror(3).r(FUNC(zpinball_state::in2_r));
	map(0x0c, 0x0c).mirror(3).w(FUNC(zpinball_state::out1_w));
	map(0x10, 0x10).mirror(3).w(FUNC(zpinball_state::shift_load_w));
	map(0x14, 0x14).mirror(3).w(FUNC(zpinball_state::out2_w));
	map(0x18, 0x18).mirror(3).w(FUNC(zpinball_state::strobes_w));
	map(0x1c, 0x1c).mirror(3).w(FUNC(zpinball_state::out3_w));
}


static INPUT_PORTS_START(zpinball)
INPUT_PORTS_END


static const z80_daisy_config daisy_chain[] =
{
	{ "zpuctc" },
	{ nullptr }
};

void zpinball_state::zpinball(machine_config &config)
{
	Z80(config, m_zpumpu, 8_MHz_XTAL / 2); // Z80A
	m_zpumpu->set_addrmap(AS_PROGRAM, &zpinball_state::zpu_mem);
	m_zpumpu->set_addrmap(AS_IO, &zpinball_state::zpu_io);
	m_zpumpu->set_daisy_config(daisy_chain);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // 6116 + battery

	Z80CTC(config, m_zpuctc, 8_MHz_XTAL / 2);
	m_zpuctc->set_clk<0>(8_MHz_XTAL / 4);
	m_zpuctc->set_clk<2>(100); // rectified from power supply
	m_zpuctc->set_clk<3>(100);
	m_zpuctc->intr_callback().set_inputline(m_zpumpu, INPUT_LINE_IRQ0);
	m_zpuctc->zc_callback<0>().set(FUNC(zpinball_state::shift_toggle_w));
	m_zpuctc->zc_callback<0>().append(m_zpuctc, FUNC(z80ctc_device::trg1));
	m_zpuctc->zc_callback<1>().set(FUNC(zpinball_state::clock_off_w));

	SPEAKER(config, "mono").front_center();
	SAA1099(config, "saa", 8_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 1.0);

	EFO_ZSU1(config, m_zsu);
}

void zpinball_state::eballchps(machine_config &config)
{
	zpinball(config);
	EFO_ZSU(config.replace(), m_zsu);
}


// Eight Ball Champ (Maibesa) on EFO "Z-Pinball" hardware - very different from the Bally original
// actual year uncertain; schematic in manual says hardware was designed in 1986, so probably not 1985 as claimed
ROM_START(eballchps)
	ROM_REGION(0x8000, "zpurom", 0)
	ROM_LOAD("u18-jeb 5a0 - cpu.bin", 0x0000, 0x8000, CRC(87615a7d) SHA1(b27ca2d863040a2641f88f9bd3143467a83f181b))

	ROM_REGION(0x28000, "zsu:soundcpu", 0)
	ROM_LOAD("u3-ebe a02 - sonido.bin", 0x00000, 0x8000, CRC(34be32ee) SHA1(ce0271540164639f28d617753760ecc479b6b0d0))
	ROM_LOAD("u4-ebe b01 - sonido.bin", 0x08000, 0x8000, CRC(d696c4e8) SHA1(501e18c258e6d42819d25d72e1907984a6cfeecb))
	ROM_LOAD("u5-ebe c01 - sonido.bin", 0x10000, 0x8000, CRC(fe78d7ef) SHA1(ed91c51dd230854a007f88446011f786759687ca))
	ROM_LOAD("u6-ebe d02 - sonido.bin", 0x18000, 0x8000, CRC(a507081b) SHA1(72d025852a12f455981c61a405f97eaaac9c6fac))
ROM_END

// Cobra (Playbar)
ROM_START(cobrapb)
	ROM_REGION(0x8000, "zpurom", 0)
	ROM_LOAD("u18 - jcb 4 a0 - cpu.bin", 0x0000, 0x8000, CRC(c663910e) SHA1(c38692343f114388259c4e7b7943e5be934189ca))

	ROM_REGION(0x28000, "zsu:soundcpu", 0)
	ROM_LOAD("u3 - scb 1 a0 - sonido.bin", 0x00000, 0x8000, CRC(d3675770) SHA1(882ce748308f2d78cccd28fc8cd64fe69bd223e3))
	ROM_LOAD("u4 - scb 1 b0 - sonido.bin", 0x08000, 0x8000, CRC(e8e1bdbb) SHA1(215bdfab751cb0ea47aa529df0ac30976de4f772))
	ROM_LOAD("u5 - scb 1 c0 - sonido.bin", 0x10000, 0x8000, CRC(c36340ab) SHA1(cd662457959de3a929ba02779e2046ed18b797e2))
ROM_END

// Come Back (Nondum)
ROM_START(comeback)
	ROM_REGION(0x8000, "zpurom", 0)
	ROM_LOAD("jco_6a0.u18", 0x0000, 0x8000, CRC(31268ca1) SHA1(d6132d021e808d107dd29c7da0fbb4bc887339a7))

	ROM_REGION(0x28000, "zsu:soundcpu", 0)
	ROM_LOAD("cbs_3a0.u3", 0x00000, 0x8000, CRC(d0f55dc9) SHA1(91186e2cbe248323380418911240a9a5887063fb))
	ROM_LOAD("cbs_3b0.u4", 0x08000, 0x8000, CRC(1da16d36) SHA1(9f7a27ae23064c1183a346ff042e6cba148257c7))
	ROM_LOAD("cbs_1c0.u5", 0x10000, 0x8000, CRC(794ae588) SHA1(adaa5e69232523369a6a2da865ac05102cc04ec8))
ROM_END

} // Anonymous namespace


GAME(1986, eballchps, eballchp, eballchps, zpinball, zpinball_state, empty_init, ROT0, "Bally (Maibesa license)", "Eight Ball Champ (Spain, Z-Pinball hardware)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1987, cobrapb,   0,        zpinball,  zpinball, zpinball_state, empty_init, ROT0, "Playbar", "Cobra (Playbar)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(198?, comeback,  0,        zpinball,  zpinball, zpinball_state, empty_init, ROT0, "Nondum / CIFA", "Come Back", MACHINE_IS_SKELETON_MECHANICAL)
