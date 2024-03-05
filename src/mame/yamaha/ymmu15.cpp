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
INPUT_PORTS_END

class mu15_state : public driver_device
{
public:
	mu15_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
			  //        , m_nvram(*this, "ram")
		, m_lcd(*this, "lcd")
		, m_ram1(*this, "ram1")
		, m_ram2(*this, "ram2")
	{ }

	void mu15(machine_config &config);

private:
	required_device<swx00_device> m_maincpu;
	//  required_device<nvram_device> m_nvram;
	//  required_device<swp00_device> m_swp00;
	required_device<mu5lcd_device> m_lcd;
	required_shared_ptr<u16> m_ram1;
	required_shared_ptr<u16> m_ram2;

	void c_map(address_map &map);
	void s_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;
};

void mu15_state::machine_start()
{
}

void mu15_state::machine_reset()
{
}

void mu15_state::s_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom().region("swx00", 0);
	map(0x400000, 0x40ffff).ram().share(m_ram1);
	map(0xc00000, 0xc03fff).ram().share(m_ram2);
	//  map(0x400000, 0x4007ff).m(m_swp00, FUNC(swp00_device::map));
}

void mu15_state::c_map(address_map &map)
{
}

#if 0
// Analog input right (not sent to the swp, mixing is analog)
u16 mu15_state::adc_ar_r()
{
	return 0x3ff;
}

// Analog input left (not sent to the swp, mixing is analog)
u16 mu15_state::adc_al_r()
{
	return 0x3ff;
}

// Put the host switch to pure midi
u16 mu15_state::adc_midisw_r()
{
	// 000-0bf: midi
	// 0c0-1ff: pc2
	// 200-37f: pc1
	// 380-3ff: mac
	return 0x000;
}

// Battery level
u16 mu15_state::adc_battery_r()
{
	return 0x200;
}

void mu15_state::p6_w(u16 data)
{
	data ^= P6_LCD_ENABLE;
	if(!(cur_p6 & P6_LCD_ENABLE) && (data & P6_LCD_ENABLE)) {
		if(!(cur_p6 & P6_LCD_RW)) {
			if(cur_p6 & P6_LCD_RS)
				m_lcd->data_write(cur_pa);
			else
				m_lcd->control_write(cur_pa);
		}
	}

	cur_p6 = data;
}

u16 mu15_state::p6_r()
{
	return cur_p6;
}

u16 mu15_state::pb_r()
{
	return cur_pb;
}

void mu15_state::pb_w(u16 data)
{
	cur_pb = data;
}

void mu15_state::pa_w(u16 data)
{
	cur_pa = data;
}

void mu15_state::pc_w(u16 data)
{
	cur_pc = data;
}

u16 mu15_state::pa_r()
{
	if((cur_p6 & P6_LCD_ENABLE)) {
		if(cur_p6 & P6_LCD_RW)
			{
				if(cur_p6 & P6_LCD_RS)
					return m_lcd->data_read();
				else
					return m_lcd->control_read();
			} else
			return 0x00;
	}
	return cur_pa;
}

u16 mu15_state::pc_r()
{
	u16 res = cur_pc | 0x7c;
	if(!(cur_pc & 0x01))
		res &= m_ioport_o0->read();
	if(!(cur_pc & 0x02))
		res &= m_ioport_o1->read();
	if(!(cur_pc & 0x80))
		res &= m_ioport_o2->read();
	return res;
}
#endif

void mu15_state::mu15(machine_config &config)
{
	SWX00(config, m_maincpu, 8.4672_MHz_XTAL, 0);
	m_maincpu->set_addrmap(m_maincpu->c_bus_id(), &mu15_state::c_map);
	m_maincpu->set_addrmap(m_maincpu->s_bus_id(), &mu15_state::s_map);

	//  m_maincpu->read_porta().set(FUNC(mu15_state::pa_r));
	//  m_maincpu->read_portb().set(FUNC(mu15_state::pa_r));
	//  m_maincpu->write_portb().set(FUNC(mu15_state::pa_w));

#if 0
	m_maincpu->read_adc<0>().set(FUNC(mu15_state::adc_ar_r));
	m_maincpu->read_adc<1>().set_constant(0);
	m_maincpu->read_adc<2>().set(FUNC(mu15_state::adc_al_r));
	m_maincpu->read_adc<3>().set_constant(0);
	m_maincpu->read_adc<4>().set(FUNC(mu15_state::adc_midisw_r));
	m_maincpu->read_adc<5>().set_constant(0);
	m_maincpu->read_adc<6>().set(FUNC(mu15_state::adc_battery_r));
	m_maincpu->read_adc<7>().set_constant(0);
	m_maincpu->read_port6().set(FUNC(mu15_state::p6_r));
	m_maincpu->write_port6().set(FUNC(mu15_state::p6_w));
	m_maincpu->read_porta().set(FUNC(mu15_state::pa_r));
	m_maincpu->write_porta().set(FUNC(mu15_state::pa_w));
	m_maincpu->read_portb().set(FUNC(mu15_state::pb_r));
	m_maincpu->write_portb().set(FUNC(mu15_state::pb_w));
	m_maincpu->read_portc().set(FUNC(mu15_state::pc_r));
	m_maincpu->write_portc().set(FUNC(mu15_state::pc_w));

	m_maincpu->read_port7().set_constant(0);
	m_maincpu->read_port9().set_constant(0);
#endif

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
