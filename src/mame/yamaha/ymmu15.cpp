// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*************************************************************************************

    Yamaha MU-15 : 16-part, 32-note polyphonic/multitimbral General MIDI/XG
                   tone module
    Driver by O. Galibert

    Uses a SWX00 that includes both the synth and an h8 core.  Program and samples
    share the rom.

**************************************************************************************/

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/h8/swx00.h"
#include "mu5lcd.h"
#include "machine/nvram.h"

#include "debugger.h"
#include "speaker.h"


namespace {

static INPUT_PORTS_START( mu15 )
	PORT_START("SA")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part -")         PORT_CODE(KEYCODE_9)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part +")         PORT_CODE(KEYCODE_0)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Value -")        PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Value +")        PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("XG Bank")        PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Play/Edit")      PORT_CODE(KEYCODE_OPENBRACE)

	PORT_START("SB")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D2# Mute")       PORT_CODE(KEYCODE_J)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C2# Solo")       PORT_CODE(KEYCODE_H)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A1# Note Shft")  PORT_CODE(KEYCODE_F)
	PORT_BIT(0x38, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("SC")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G1# Pan")        PORT_CODE(KEYCODE_D)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F1# Vol")        PORT_CODE(KEYCODE_S)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D1# Init All")   PORT_CODE(KEYCODE_8)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C1# Dump Out")   PORT_CODE(KEYCODE_7)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A0# Locl Ctrl")  PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G0# Velocity")   PORT_CODE(KEYCODE_4)

	PORT_START("SD")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F0# Mstr Tune")  PORT_CODE(KEYCODE_3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Oct Up")         PORT_CODE(KEYCODE_2)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Oct Down")       PORT_CODE(KEYCODE_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("E0 V Dry Lvl")   PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F0 V Ins Sys")   PORT_CODE(KEYCODE_W)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G0 V Type")      PORT_CODE(KEYCODE_E)

	PORT_START("SE")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A0 V Send Lvl")  PORT_CODE(KEYCODE_R)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("B0 C Type")      PORT_CODE(KEYCODE_T)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C1 C Send Lvl")  PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D1 R Type")      PORT_CODE(KEYCODE_U)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("E1 R Send Lvl")  PORT_CODE(KEYCODE_I)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F1 V Rate")      PORT_CODE(KEYCODE_Z)

	PORT_START("SF")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G1 V Dpth")      PORT_CODE(KEYCODE_X)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A1 V Dely")      PORT_CODE(KEYCODE_C)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("B1 E Atck Time") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C2 E Rels Time") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D2 F Cutoff")    PORT_CODE(KEYCODE_N)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("E2 F Reso")      PORT_CODE(KEYCODE_M)
INPUT_PORTS_END

class mu15_state : public driver_device
{
public:
	mu15_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		//, m_nvram(*this, "ram")
		, m_lcd(*this, "lcd")
		, m_ram1(*this, "ram1")
		, m_ram2(*this, "ram2")
		, m_ioport(*this, "S%c", 'A')

	{ }

	void mu15(machine_config &config);

private:
	required_device<swx00_device> m_maincpu;
	//  required_device<nvram_device> m_nvram;
	required_device<mu5lcd_device> m_lcd;
	required_shared_ptr<u16> m_ram1;
	required_shared_ptr<u16> m_ram2;
	required_ioport_array<6> m_ioport;

	u16 m_pdt;
	u8 m_cmah;

	void c_map(address_map &map) ATTR_COLD;
	void s_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	u16 pdt_r();
	void pdt_w(u16 data);
	u8 pad_r();
	void cmah_w(u8 data);
};

void mu15_state::machine_start()
{
}

void mu15_state::machine_reset()
{
}

void mu15_state::c_map(address_map &map)
{
	map(0x200000, 0x21ffff).ram().share(m_ram1); // ic12, cs1
	map(0xc00000, 0xc07fff).ram().share(m_ram2); // ic13, cs5, saved
}

void mu15_state::s_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("swx00", 0);
}

u16 mu15_state::pdt_r()
{
	u8 bus = 0;
	if((m_cmah & 8) && (m_cmah & 4)) {
		if(m_cmah & 2)
			bus = m_lcd->dr_r();
		else
			bus = m_lcd->status_r();
	}
	return ((bus & 0xc0) << 8) | ((bus & 0x3f) << 6);
}

void mu15_state::cmah_w(u8 data)
{
	u8 old = m_cmah;
	m_cmah = data;
	if((old & 8) && !(m_cmah & 8) && !(m_cmah & 4) && !(old & 4)) {
		u8 v = ((m_pdt >> 8) & 0xc0) | ((m_pdt >> 6) & 0x3f);
		if(m_cmah & 2)
			m_lcd->dr_w(v);
		else
			m_lcd->ir_w(v);
	}
}

void mu15_state::pdt_w(u16 data)
{
	m_pdt = data;
}

u8 mu15_state::pad_r()
{
	u8 r = 0xff;
	for(int i=0; i != 6; i++)
		if(!BIT(m_pdt, 5-i))
			r &= m_ioport[i]->read();
	return r;
}

void mu15_state::mu15(machine_config &config)
{
	SWX00(config, m_maincpu, 8.4672_MHz_XTAL, 0);
	m_maincpu->set_addrmap(swx00_device::AS_C, &mu15_state::c_map);
	m_maincpu->set_addrmap(swx00_device::AS_S, &mu15_state::s_map);

	m_maincpu->add_route(0, "lspeaker", 1.0);
	m_maincpu->add_route(1, "rspeaker", 1.0);

	// Nothing connected to sclki, yet...
	m_maincpu->sci_set_external_clock_period(0, attotime::from_hz(500000));
	m_maincpu->sci_set_external_clock_period(1, attotime::from_hz(500000));

	m_maincpu->read_adc<0>().set_constant(0x000); // Host mode
	m_maincpu->read_adc<1>().set_constant(0x000); // GND
	m_maincpu->read_adc<2>().set_constant(0x3ff); // Battery level
	m_maincpu->read_adc<3>().set_constant(0x000); // GND

	m_maincpu->read_pdt().set(FUNC(mu15_state::pdt_r));
	m_maincpu->write_pdt().set(FUNC(mu15_state::pdt_w));
	m_maincpu->read_pad().set(FUNC(mu15_state::pad_r));
	m_maincpu->write_cmah().set(FUNC(mu15_state::cmah_w));

	//  NVRAM(config, m_nvram, nvram_device::DEFAULT_NONE);

	MU5LCD(config, m_lcd);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	// sci0 goes to the host connector

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set(m_maincpu, FUNC(swx00_device::sci_rx_w<1>));

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);
	m_maincpu->write_sci_tx<1>().set(mdout, FUNC(midi_port_device::write_txd));
}

ROM_START( mu15 )
	ROM_REGION16_BE( 0x400000, "swx00", 0 )
	// v1.01, Nov. 28, 1998
	ROM_LOAD16_WORD_SWAP( "xv684c0.bin", 0x000000, 0x400000, CRC(e4046aef) SHA1(e286f83ed1fb90e0f98fe565b58112da18f88b5a) )
ROM_END

} // anonymous namespace


CONS( 1998, mu15, 0, 0, mu15,  mu15, mu15_state, empty_init, "Yamaha", "MU15", MACHINE_NOT_WORKING )
