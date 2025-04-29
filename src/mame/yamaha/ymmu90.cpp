// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/*************************************************************************************

    Yamaha MU-90 : 32-part, 64-voice polyphonic/multitimbral General MIDI/GS/XG
                    tone module
    Preliminary driver by R. Belmont and O. Galibert

    The successor to the mu80, with the swp20/meg/eq combo remplaced by an all-in-one swp30.
    Exists in rackable (mu90r) version but we don't have that firmware variant and in
    screenless version (mu90b).

    Sound roms are a subset of the mu100's.

    MU90 CPU: Hitachi H8/3002 (HD6432655F), strapped for mode 4 (24-bit address, 16-bit data, no internal ROM)
    Sound ASIC: Yamaha XS725A0/SWP30
    RAM: 1 MSM51008 (1 meg * 1 bit = 128KBytes)


**************************************************************************************/

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/h8/h83002.h"
#include "mulcd.h"
#include "sound/swp30.h"

#include "debugger.h"
#include "speaker.h"


namespace {

static INPUT_PORTS_START( mu90 )
	PORT_START("P7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Enter")     PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Effect")    PORT_CODE(KEYCODE_F)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Util")      PORT_CODE(KEYCODE_U)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part +")    PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part -")    PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mute/Solo") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Edit")      PORT_CODE(KEYCODE_E)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Play")      PORT_CODE(KEYCODE_A)

	PORT_START("P8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mode")      PORT_CODE(KEYCODE_M)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Eq")        PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Exit")      PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select >")  PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select <")  PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Value +")   PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Value -")   PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

class mu90_state : public driver_device
{
public:
	mu90_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_swp30(*this, "swp30")
		, m_lcd(*this, "lcd")
		, m_ioport_p7(*this, "P7")
		, m_ioport_p8(*this, "P8")
	{ }

	void mu90(machine_config &config);
	void mu90b(machine_config &config);

private:
	required_device<h83002_device> m_maincpu;
	required_device<swp30_device> m_swp30;
	optional_device<mulcd_device> m_lcd;
	required_ioport m_ioport_p7;
	required_ioport m_ioport_p8;

	u8 cur_pa, cur_pb;
	u8 cur_ic34;

	u16 adc_ar_r();
	u16 adc_al_r();
	u16 adc_midisw_r();
	u16 adc_battery_r();

	void pa_w(u8 data);
	void pb_w(u8 data);
	u8 pb_r();

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void mu90_map(address_map &map) ATTR_COLD;
	void swp30_map(address_map &map) ATTR_COLD;
};

void mu90_state::machine_start()
{
	cur_pa = cur_pb = 0xff;
}

void mu90_state::machine_reset()
{
	// Active-low, wired to gnd
	m_maincpu->set_input_line(0, ASSERT_LINE);
}

void mu90_state::mu90_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom().region("maincpu", 0);
	map(0x200000, 0x20ffff).ram(); // 64K work RAM
	map(0x400000, 0x401fff).m(m_swp30, FUNC(swp30_device::map));
}

void mu90_state::swp30_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("swp30", 0);
}

// Analog input right (also sent to the swp)
u16 mu90_state::adc_ar_r()
{
	return 0;
}

// Analog input left (also sent to the swp)
u16 mu90_state::adc_al_r()
{
	return 0;
}

// Put the host switch to pure midi
u16 mu90_state::adc_midisw_r()
{
	return 0;
}

// Battery level
u16 mu90_state::adc_battery_r()
{
	return 0x200;
}

void mu90_state::pb_w(u8 data)
{
	cur_pb = data;
}

u8 mu90_state::pb_r()
{
	u8 res = 0xff;
	if(m_lcd && (cur_pa & 0x20)) {
		if(cur_pa & 0x40) {
			if(cur_pa & 0x02)
				res &= m_lcd->data_read();
			else
				res &= m_lcd->control_read();
		}
	}

	if(!(cur_pa & 0x10)) {
		if(!(cur_ic34 & 0x20))
			res &= m_ioport_p7->read();
		if(!(cur_ic34 & 0x40))
			res &= m_ioport_p8->read();
	}

	return res;
}

void mu90_state::pa_w(u8 data)
{
	if(!(cur_pa & 0x01) && (data & 0x01)) {
		if(m_lcd)
			m_lcd->set_contrast(cur_pb & 7);
		logerror("ad1 input level %s\n", cur_pb & 0x80 ? "line" : "mic");
		logerror("ad2 input level %s\n", cur_pb & 0x40 ? "line" : "mic");
	}

	if(m_lcd && (cur_pa & 0x20) && !(data & 0x20)) {
		if(!(cur_pa & 0x40)) {
			if(cur_pa & 0x02)
				m_lcd->data_write(cur_pb);
			else
				m_lcd->control_write(cur_pb);
		}
	}

	if(!(cur_pa & 0x08) && (data & 0x08)) {
		if(m_lcd)
			m_lcd->set_leds((cur_pb & 0x1f) | ((cur_pb & 0x80) >> 2));
		cur_ic34 = cur_pb;
	}

	cur_pa = data;
}

void mu90_state::mu90b(machine_config &config)
{
	H83002(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mu90_state::mu90_map);
	m_maincpu->read_adc<0>().set(FUNC(mu90_state::adc_ar_r));
	m_maincpu->read_adc<1>().set_constant(0);
	m_maincpu->read_adc<2>().set(FUNC(mu90_state::adc_al_r));
	m_maincpu->read_adc<3>().set_constant(0);
	m_maincpu->read_adc<4>().set(FUNC(mu90_state::adc_midisw_r));
	m_maincpu->read_adc<5>().set_constant(0);
	m_maincpu->read_adc<6>().set(FUNC(mu90_state::adc_battery_r));
	m_maincpu->read_adc<7>().set_constant(0);
	m_maincpu->read_port6().set_constant(0);
	m_maincpu->write_porta().set(FUNC(mu90_state::pa_w));
	m_maincpu->read_portb().set(FUNC(mu90_state::pb_r));
	m_maincpu->write_portb().set(FUNC(mu90_state::pb_w));

	SPEAKER(config, "speaker", 2).front();

	SWP30(config, m_swp30);
	m_swp30->set_addrmap(AS_DATA, &mu90_state::swp30_map);
	m_swp30->add_route(0, "speaker", 1.0, 0);
	m_swp30->add_route(1, "speaker", 1.0, 1);

	auto &mdin_a(MIDI_PORT(config, "mdin_a"));
	midiin_slot(mdin_a);
	mdin_a.rxd_handler().set(m_maincpu, FUNC(h83002_device::sci_rx_w<1>));

	auto &mdin_b(MIDI_PORT(config, "mdin_b"));
	midiin_slot(mdin_b);
	mdin_b.rxd_handler().set(m_maincpu, FUNC(h83002_device::sci_rx_w<0>));

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);
	m_maincpu->write_sci_tx<0>().set(mdout, FUNC(midi_port_device::write_txd));
}

void mu90_state::mu90(machine_config &config)
{
	mu90b(config);

	MULCD(config, m_lcd);
}

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(bios))

ROM_START( mu90 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_DEFAULT_BIOS("v101")
	ROM_SYSTEM_BIOS( 0, "v101", "xs519d0 (v1.01, Nov. 27, 1996)" )
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "xs519d0.ic9", 0x000000, 0x100000, CRC(6fc85b41) SHA1(05068eddcaf5be3a57f3a412a95c204849011b34) )

	ROM_REGION32_LE( 0x800000, "swp30", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD( "xs518a0.ic22", 0x0000000, 0x400000, CRC(2550d44f) SHA1(fd3cce228c7d389a2fde25c808a5b26080588cba) )
	ROM_LOAD32_WORD( "xs743a0.ic23", 0x0000002, 0x400000, CRC(a9109a6c) SHA1(a67bb49378a38a2d809bd717d286e18bc6496db0) )
ROM_END

ROM_START( mu90b )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_DEFAULT_BIOS("v101")
	ROM_SYSTEM_BIOS( 0, "v101", "xt040c0 (v1.01, Dec. 26, 2005)" )
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "xt040c0.ic9", 0x000000, 0x100000, CRC(66fe5896) SHA1(811a8c7f0e8aac7a8807922d5add0fbfc07e1cfd) )

	ROM_REGION32_LE( 0x800000, "swp30", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD( "xs518a0.ic22", 0x0000000, 0x400000, CRC(2550d44f) SHA1(fd3cce228c7d389a2fde25c808a5b26080588cba) )
	ROM_LOAD32_WORD( "xs743a0.ic23", 0x0000002, 0x400000, CRC(a9109a6c) SHA1(a67bb49378a38a2d809bd717d286e18bc6496db0) )
ROM_END

} // anonymous namespace


SYST( 1996, mu90,     0, 0, mu90,  mu90, mu90_state, empty_init, "Yamaha", "MU90",  MACHINE_NOT_WORKING )
SYST( 2005, mu90b, mu90, 0, mu90b, mu90, mu90_state, empty_init, "Yamaha", "MU90B", MACHINE_NOT_WORKING )
