// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************************************************************

has a sticker marked
Part # 04-0008
Main PCB  w/ Dark-Slide
Serial# : 0115

2x Z80



There were several different designs for this, it's possible they used
different speech roms etc.

ToDo:
- work out how the cpus communicate (there's an interrupt handler in main cpu)
- clear up the speech
- inputs
- mechanical matters (camera, printer, etc)

At the moment it simply outputs all the speech strings, one after the other, then stops.

*****************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "sound/dac.h"
#include "speaker.h"


namespace {

class instantm_state : public driver_device
{
public:
	instantm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void instantm(machine_config &config);

private:
	u8 port01_r();
	void port01_w(u8 data);
	void clock_w(int state);

	void main_map(address_map &map) ATTR_COLD;
	void sub_io(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;

	u8 m_port01 = 0;
	bool m_clock_en = false;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
};

// return instruction from main cpu
u8 instantm_state::port01_r()
{
	return m_port01;
}

// tell maincpu the speech is done
void instantm_state::port01_w(u8 data)
{
	// bump to next bit of speech for now
	if ((m_port01 & 15) < 15)
		m_port01++;
	else
		m_clock_en = false;
}

// clock out the speech bytes
void instantm_state::clock_w(int state)
{
	if (m_clock_en)
		m_port01 ^= 0x80;
}



void instantm_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x8000, 0x8000); //.w(FUNC(instantm_state::));
	map(0xc000, 0xc000); //.w(FUNC(instantm_state::));
	map(0xc400, 0xc400); //.w(FUNC(instantm_state::));
	map(0xc800, 0xc800); //.w(FUNC(instantm_state::));
	map(0xcc00, 0xcc00); //.w(FUNC(instantm_state::));
	map(0xec00, 0xec00); //.r(FUNC(instantm_state::));
	map(0xf000, 0xf000); //.r(FUNC(instantm_state::));
	map(0xf400, 0xf400); //.r(FUNC(instantm_state::));
	map(0xfc00, 0xfc00); //.r(FUNC(instantm_state::));
}

// doesn't use ram
void instantm_state::sub_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
	map(0x0000, 0x0000).w("dac", FUNC(dac_byte_interface::data_w));
}

void instantm_state::sub_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).rw(FUNC(instantm_state::port01_r), FUNC(instantm_state::port01_w));
}

static INPUT_PORTS_START( instantm )
INPUT_PORTS_END


void instantm_state::machine_start()
{
}

void instantm_state::machine_reset()
{
	m_port01 = 0xf0; // bit 4 low sends subcpu to a test mode
	m_clock_en = true;
}

// OSC1 = XTAL(3'579'545)

void instantm_state::instantm(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(3'579'545));
	m_maincpu->set_addrmap(AS_PROGRAM, &instantm_state::main_map);

	z80_device &subcpu(Z80(config, "subcpu", XTAL(3'579'545)));
	subcpu.set_addrmap(AS_PROGRAM, &instantm_state::sub_map);
	subcpu.set_addrmap(AS_IO, &instantm_state::sub_io);

	// all guesswork
	clock_device &voice_clock(CLOCK(config, "voice_clock", 24000));
	voice_clock.signal_handler().set(FUNC(instantm_state::clock_w));

	SPEAKER(config, "speaker").front_center();
	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5);
}



ROM_START( instantm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "system5.3beta.u16", 0x00000, 0x02000, CRC(a1701f4b) SHA1(fa5b0234bd2b666e478aa41129479bb6cec2bcf5) )

	ROM_REGION( 0x10000, "subcpu", 0 )
	ROM_LOAD( "speechus10.u20", 0x00000, 0x10000, CRC(1797bcee) SHA1(c6fb7fbe8592dfae3ba44b49b5ce447206515b77) )
ROM_END

} // anonymous namespace


GAME( 199?, instantm, 0, instantm, instantm, instantm_state, empty_init, ROT0, "Capcom / Polaroid", "Polaroid Instant Memories", MACHINE_IS_SKELETON_MECHANICAL )
