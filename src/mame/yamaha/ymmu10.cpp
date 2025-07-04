// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/*************************************************************************************

    Yamaha MU-10 : 16-part, 32-note polyphonic/multitimbral General MIDI/GS/XG
                   tone module
    Driver by R. Belmont and O. Galibert

    Essentially a screen-less MU10 (SWP00, identical wave rom) with a gate-array based
    hack to connect the wave rom space to the adcs so that effects can be applied to
    analog inputs.  Entirely controlled through midi.

**************************************************************************************/

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/h8/h83002.h"
#include "sound/swp00.h"
#include "machine/nvram.h"

#include "debugger.h"
#include "speaker.h"


namespace {

static INPUT_PORTS_START( mu10 )
INPUT_PORTS_END

class mu10_state : public driver_device
{
public:
	mu10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "ram")
		, m_swp00(*this, "swp00")
		, m_ram(*this, "ram")
	{ }

	void mu10(machine_config &config);

private:
	required_device<h83002_device> m_maincpu;
	required_device<nvram_device> m_nvram;
	required_device<swp00_device> m_swp00;
	required_shared_ptr<u16> m_ram;

	u8 cur_p6, cur_pa, cur_pb;

	u16 adc_battery_r();
	u16 adc_midisw_r();

	void p6_w(u8 data);
	void pa_w(u8 data);
	void pb_w(u8 data);
	u8 pb_r();

	void mu10_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
};

void mu10_state::machine_start()
{
	cur_p6 = cur_pa = cur_pb = 0xff;
}

void mu10_state::machine_reset()
{
	// Active-low, wired to gnd
	m_maincpu->set_input_line(0, ASSERT_LINE);
}

void mu10_state::mu10_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("maincpu", 0);
	map(0x400000, 0x4007ff).m(m_swp00, FUNC(swp00_device::map));
	map(0x600000, 0x607fff).ram().share(m_ram); // 32K work RAM
}

// Battery level
u16 mu10_state::adc_battery_r()
{
	return 0x200;
}

// Put the host switch to pure midi
u16 mu10_state::adc_midisw_r()
{
	// 000-0bf: midi
	// 0c0-1ff: pc2
	// 200-37f: pc1
	// 380-3ff: mac
	return 0x000;
}

void mu10_state::p6_w(u8 data)
{
	cur_p6 = data;
	logerror("reset swp %d dac %d\n", BIT(data, 2), BIT(data, 0));
}

void mu10_state::pb_w(u8 data)
{
	cur_pb = data;
	logerror("led %d gain %d\n", BIT(data, 0), BIT(data, 2));
}

u8 mu10_state::pb_r()
{
	// bit 3 = a/d plugged in
	return 8;
}

void mu10_state::pa_w(u8 data)
{
	cur_pa = data;
	logerror("mac host pin 1 %d\n", !BIT(data, 2));
}

void mu10_state::mu10(machine_config &config)
{
	H83002(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mu10_state::mu10_map);
	m_maincpu->read_adc<0>().set(FUNC(mu10_state::adc_battery_r));
	m_maincpu->read_adc<4>().set_constant(0);
	m_maincpu->read_adc<5>().set_constant(0);
	m_maincpu->read_adc<6>().set_constant(0);
	m_maincpu->read_adc<7>().set(FUNC(mu10_state::adc_midisw_r));
	m_maincpu->write_port6().set(FUNC(mu10_state::p6_w));
	m_maincpu->write_porta().set(FUNC(mu10_state::pa_w));
	m_maincpu->write_portb().set(FUNC(mu10_state::pb_w));
	m_maincpu->read_portb().set(FUNC(mu10_state::pb_r));

	NVRAM(config, m_nvram, nvram_device::DEFAULT_NONE);

	SPEAKER(config, "speaker", 2).front();

	SWP00(config, m_swp00);
	m_swp00->add_route(0, "speaker", 1.0, 0);
	m_swp00->add_route(1, "speaker", 1.0, 1);

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set(m_maincpu, FUNC(h83002_device::sci_rx_w<1>));

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);
	m_maincpu->write_sci_tx<1>().set(mdout, FUNC(midi_port_device::write_txd));
}

ROM_START( mu10 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	// Ver0.05 94-Nov-24
	ROM_LOAD16_WORD_SWAP( "xs289a0.ic07", 0x000000, 0x080000, CRC(6438c5a7) SHA1(4c2bb0c53a756d64cf6315b85642cab3b86e39fc) )

	ROM_REGION( 0x400000, "swp00", 0 )
	// Same content as the mu10 roms but grouped into one rom
	ROM_LOAD( "xr709a0.ic11", 0x000000, 0x400000, CRC(4261c0bc) SHA1(5286b933976f9e3a9ca82523b8d591b616cf9479) )
ROM_END

} // anonymous namespace


CONS( 1994, mu10, 0, 0, mu10,  mu10, mu10_state, empty_init, "Yamaha", "MU10", 0 )
