// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

Super Star (Recreativos Franco)

Skeleton driver for 8085-based pinball hardware.

TODO:
- Display circuits: i8259, 74164, 74159 and other bits.
- Default layout
- Outputs (solenoids, lamps)
- Inputs (switches)
- NVRAM (5517 ram is battery-backed)
- Manual is missing a couple of pages.

***************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/i8212.h"
#include "sound/ay8910.h"
#include "speaker.h"


namespace {

class supstarf_state : public driver_device
{
public:
	supstarf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_psg(*this, "psg%u", 1U)
		, m_soundlatch(*this, "soundlatch%u", 1U)
	{
	}

	void supstarf(machine_config &config);

private:
	u8 psg_latch_r(offs_t offset);
	void psg_latch_w(offs_t offset, u8 data);
	void port1_w(u8 data);
	void port2_w(u8 data);
	int contacts_r();
	void displays_w(int state);
	void driver_clk_w(offs_t offset, u8 data);
	int phase_detect_r();
	void lights_a_w(u8 data);
	void lights_b_w(u8 data);

	void main_io_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;

	required_device<i8085a_cpu_device> m_maincpu;
	required_device<i8035_device> m_soundcpu;
	required_device_array<ay8910_device, 2> m_psg;
	required_device_array<i8212_device, 2> m_soundlatch;

	u8 m_port1_data = 0;
	bool m_pcs[2]{};
	bool m_latch_select = false;
};

void supstarf_state::main_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x8000, 0x8000).r("soundlatch1", FUNC(i8212_device::read)).w("soundlatch2", FUNC(i8212_device::strobe));
	map(0xc000, 0xc7ff).ram(); // 5517 (2Kx8) at IC11
}

void supstarf_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xff).w(FUNC(supstarf_state::driver_clk_w));
}

void supstarf_state::sound_map(address_map &map)
{
	map(0x000, 0xfff).rom(); // external EPROM
}

void supstarf_state::sound_io_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(supstarf_state::psg_latch_r), FUNC(supstarf_state::psg_latch_w));
}

u8 supstarf_state::psg_latch_r(offs_t offset)
{
	u8 result = 0xff; // AR3 +5v pullup

	for (int d = 0; d < 2; d++)
	{
		if (m_pcs[d])
		{
			m_psg[d]->address_w(offset);
			result &= m_psg[d]->data_r();
		}
	}

	if (m_latch_select)
		result &= m_soundlatch[1]->read();

	return result;
}

void supstarf_state::psg_latch_w(offs_t offset, u8 data)
{
	for (int d = 0; d < 2; d++)
	{
		if (m_pcs[d])
		{
			m_psg[d]->address_w(offset);
			m_psg[d]->data_w(data);
		}
	}

	if (m_latch_select)
		m_soundlatch[0]->strobe(data);
}

void supstarf_state::port1_w(u8 data)
{
	m_port1_data = data;
}

void supstarf_state::port2_w(u8 data)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, BIT(data, 4) ? CLEAR_LINE : ASSERT_LINE);
	if (!BIT(data, 4))
	{
		m_soundlatch[0]->reset();
		m_soundlatch[1]->reset();
		m_psg[0]->reset();
		m_psg[1]->reset();
	}

	m_pcs[0] = !BIT(data, 6);
	m_pcs[1] = !BIT(data, 5);
	m_latch_select = !BIT(data, 7);
}

int supstarf_state::contacts_r()
{
	return 1;
}

void supstarf_state::displays_w(int state)
{
}

void supstarf_state::driver_clk_w(offs_t offset, u8 data)
{
}

int supstarf_state::phase_detect_r()
{
	return 0;
}

void supstarf_state::lights_a_w(u8 data)
{
}

void supstarf_state::lights_b_w(u8 data)
{
}

void supstarf_state::machine_start()
{
	m_pcs[0] = m_pcs[1] = 0;
	m_latch_select = 0;
	m_port1_data = 0xff;

	save_item(NAME(m_pcs));
	save_item(NAME(m_latch_select));
	save_item(NAME(m_port1_data));
}

void supstarf_state::supstarf(machine_config &config)
{
	I8085A(config, m_maincpu, 5.0688_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &supstarf_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &supstarf_state::main_io_map);
	m_maincpu->in_sid_func().set(FUNC(supstarf_state::contacts_r));
	m_maincpu->out_sod_func().set(FUNC(supstarf_state::displays_w));

	I8035(config, m_soundcpu, 5.0688_MHz_XTAL / 2); // from 8085 pin 37 (CLK OUT)
	m_soundcpu->set_addrmap(AS_PROGRAM, &supstarf_state::sound_map);
	m_soundcpu->set_addrmap(AS_IO, &supstarf_state::sound_io_map);
	m_soundcpu->p1_out_cb().set(FUNC(supstarf_state::port1_w));
	m_soundcpu->p2_out_cb().set(FUNC(supstarf_state::port2_w));
	m_soundcpu->t1_in_cb().set(FUNC(supstarf_state::phase_detect_r));

	I8212(config, m_soundlatch[0]);
	m_soundlatch[0]->md_rd_callback().set_constant(0);
	m_soundlatch[0]->int_wr_callback().set_inputline("maincpu", I8085_RST55_LINE);

	I8212(config, m_soundlatch[1]);
	m_soundlatch[1]->md_rd_callback().set_constant(0);
	m_soundlatch[1]->int_wr_callback().set_inputline("soundcpu", MCS48_INPUT_IRQ);
	//m_soundlatch[1]->int_wr_callback().append_inputline(m_maincpu, I8085_READY_LINE);

	SPEAKER(config, "mono").front_center();

	AY8910(config, m_psg[0], 5.0688_MHz_XTAL / 6); // from 8035 pin 1 (T0)
	m_psg[0]->port_a_write_callback().set(FUNC(supstarf_state::lights_a_w));
	m_psg[0]->port_b_write_callback().set(FUNC(supstarf_state::lights_b_w));
	m_psg[0]->add_route(ALL_OUTPUTS, "mono", 0.50);

	AY8910(config, m_psg[1],  5.0688_MHz_XTAL / 6); // from 8035 pin 1 (T0)
	m_psg[1]->port_a_read_callback().set_ioport("JO");
	m_psg[1]->port_b_read_callback().set_ioport("I1");
	m_psg[1]->add_route(ALL_OUTPUTS, "mono", 0.50);
}

static INPUT_PORTS_START(supstarf)
	PORT_START("I1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "I1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "I1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "I1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "I1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "I1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "I1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "I1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "I1:8")

	PORT_START("JM")
	PORT_BIT(0x80, 0x80, IPT_UNKNOWN)
	PORT_BIT(0x40, 0x40, IPT_UNKNOWN)
	PORT_BIT(0x20, 0x20, IPT_UNKNOWN)
	PORT_BIT(0x10, 0x10, IPT_UNKNOWN)
	PORT_BIT(0x08, 0x08, IPT_UNKNOWN)
	PORT_BIT(0x04, 0x04, IPT_UNKNOWN)
	PORT_BIT(0x02, 0x02, IPT_UNKNOWN)
	PORT_BIT(0x01, 0x01, IPT_UNKNOWN)

	PORT_START("JN")
	PORT_BIT(0x80, 0x80, IPT_UNKNOWN)
	PORT_BIT(0x40, 0x40, IPT_UNKNOWN)
	PORT_BIT(0x20, 0x20, IPT_UNKNOWN)
	PORT_BIT(0x10, 0x10, IPT_UNKNOWN)
	PORT_BIT(0x08, 0x08, IPT_UNKNOWN)
	PORT_BIT(0x04, 0x04, IPT_UNKNOWN)
	PORT_BIT(0x02, 0x02, IPT_UNKNOWN)
	PORT_BIT(0x01, 0x01, IPT_UNUSED)

	PORT_START("JO")
	PORT_BIT(0x80, 0x80, IPT_UNKNOWN)
	PORT_BIT(0x40, 0x40, IPT_UNKNOWN)
	PORT_BIT(0x20, 0x20, IPT_UNKNOWN)
	PORT_BIT(0x10, 0x10, IPT_UNKNOWN)
	PORT_BIT(0x0f, 0x0f, IPT_UNUSED)
INPUT_PORTS_END

ROM_START(supstarf)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("m31-a-01187.ic19", 0x0000, 0x4000, CRC(ab8b1148) SHA1(496d3c9664386ae64e94462db2fdd36811a68a87))
	// IC14 for second program ROM is unpopulated

	ROM_REGION(0x1000, "soundcpu", 0)
	ROM_LOAD("2532.ic4", 0x0000, 0x1000, CRC(d6d7eee2) SHA1(60e497c8845320eea01662d894d0b16349ebb7e4))
ROM_END

ROM_START(supstarfa)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("27c128.ic19", 0x0000, 0x4000, CRC(9a440461) SHA1(e2f8dcf95084f755d3a34d77ba2649602687a610))
	// IC14 for second program ROM is unpopulated

	ROM_REGION(0x1000, "soundcpu", 0)
	ROM_LOAD("2532.ic4", 0x0000, 0x1000, BAD_DUMP CRC(b6ef3c7a) SHA1(aabb6f8569685fc3a917a7bb5ebfcc4b20086b15)) // D6 stuck high and probably totally garbage
ROM_END

} // anonymous namespace


GAME( 1986, supstarf,  0,        supstarf, supstarf, supstarf_state, empty_init, ROT0, "Recreativos Franco", "Super Star (Recreativos Franco, set 1)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1986, supstarfa, supstarf, supstarf, supstarf, supstarf_state, empty_init, ROT0, "Recreativos Franco", "Super Star (Recreativos Franco, set 2)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
