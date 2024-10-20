// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/*************************************************************************************

    Yamaha MU-50 : 16-part, 32-note polyphonic/multitimbral General MIDI/GS/XG
                   tone module
    Driver by R. Belmont and O. Galibert

    Cost-reduced version of the MU80, uses the SWP00 which is a single-chip
    integrated version of the multi-chip SWP20.  As a consequence has half the
    voices, loses the parametric EQ and the AD inputs.  The sample roms are also
    smaller, and it only has one midi input.

    A wavetable version exists as the DB50XG.

**************************************************************************************/

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/h8/h83003.h"
#include "machine/nvram.h"
#include "sound/swp00.h"

#include "mulcd.h"
#include "speaker.h"

#include "mu50.lh"


namespace {

static INPUT_PORTS_START( mu50 )
	PORT_START("O0")
	PORT_BIT(0x83, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Play")      PORT_CODE(KEYCODE_A)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Edit")      PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mute/Solo") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part -")    PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part +")    PORT_CODE(KEYCODE_CLOSEBRACE)

	PORT_START("O1")
	PORT_BIT(0x83, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Util")      PORT_CODE(KEYCODE_U)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Effect")    PORT_CODE(KEYCODE_F)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Enter")     PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select <")  PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select >")  PORT_CODE(KEYCODE_STOP)

	PORT_START("O2")
	PORT_BIT(0x83, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mode")      PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Exit")      PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Value -")   PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Value +")   PORT_CODE(KEYCODE_EQUALS)
INPUT_PORTS_END

class mu50_state : public driver_device
{
public:
	mu50_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_mu50cpu(*this, "mu50cpu")
		, m_nvram(*this, "ram")
		, m_swp00(*this, "swp00")
		, m_lcd(*this, "lcd")
		, m_ioport_o0(*this, "O0")
		, m_ioport_o1(*this, "O1")
		, m_ioport_o2(*this, "O2")
		, m_ram(*this, "ram")
	{ }

	void mu50(machine_config &config);

private:
	enum {
		P6_LCD_RS     = 0x04,
		P6_LCD_RW     = 0x02,
		P6_LCD_ENABLE = 0x01
	};

	required_device<h83003_device> m_mu50cpu;
	required_device<nvram_device> m_nvram;
	required_device<swp00_device> m_swp00;
	required_device<mulcd_device> m_lcd;
	required_ioport m_ioport_o0;
	required_ioport m_ioport_o1;
	required_ioport m_ioport_o2;
	required_shared_ptr<u16> m_ram;

	u8 cur_p6, cur_p9, cur_pa, cur_pb, cur_pc;

	u16 adc_ar_r();
	u16 adc_al_r();
	u16 adc_midisw_r();
	u16 adc_battery_r();

	void p6_w(u8 data);
	u8 p6_r();
	void p9_w(u8 data);
	void pa_w(u8 data);
	u8 pa_r();
	void pb_w(u8 data);
	u8 pb_r();
	void pc_w(u8 data);
	u8 pc_r();
	void update_contrast();

	void mu50_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
};

void mu50_state::machine_start()
{
	cur_p6 = cur_p9 = cur_pa = cur_pb = cur_pc = 0xff;
}

void mu50_state::machine_reset()
{
	// Active-low, wired to gnd
	m_mu50cpu->set_input_line(0, ASSERT_LINE);
}

void mu50_state::mu50_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("mu50cpu", 0);
	map(0x200000, 0x20ffff).ram().share(m_ram); // 64K work RAM
	map(0x400000, 0x4007ff).m(m_swp00, FUNC(swp00_device::map));
}

// Analog input right (not sent to the swp, mixing is analog)
u16 mu50_state::adc_ar_r()
{
	return 0x3ff;
}

// Analog input left (not sent to the swp, mixing is analog)
u16 mu50_state::adc_al_r()
{
	return 0x3ff;
}

// Put the host switch to pure midi
u16 mu50_state::adc_midisw_r()
{
	// 000-0bf: midi
	// 0c0-1ff: pc2
	// 200-37f: pc1
	// 380-3ff: mac
	return 0x000;
}

// Battery level
u16 mu50_state::adc_battery_r()
{
	return 0x200;
}

void mu50_state::p6_w(u8 data)
{
	data ^= P6_LCD_ENABLE;
	if((cur_p6 & P6_LCD_ENABLE) && !(data & P6_LCD_ENABLE)) {
		if(!(cur_p6 & P6_LCD_RW)) {
			if(cur_p6 & P6_LCD_RS)
				m_lcd->data_write(cur_pa);
			else
				m_lcd->control_write(cur_pa);
		}
	}

	cur_p6 = data;
}

u8 mu50_state::p6_r()
{
	return cur_p6;
}

void mu50_state::p9_w(u8 data)
{
	cur_p9 = data;
	update_contrast();
}

u8 mu50_state::pb_r()
{
	return cur_pb;
}

void mu50_state::update_contrast()
{
	m_lcd->set_contrast(((~cur_p9 >> 3) & 0x6) | (BIT(~cur_pb, 1)));
}

void mu50_state::pb_w(u8 data)
{
	cur_pb = data;
	m_lcd->set_leds((~data >> 2) & 0x1f);
	update_contrast();
}

void mu50_state::pa_w(u8 data)
{
	cur_pa = data;
}

void mu50_state::pc_w(u8 data)
{
	cur_pc = data;
}

u8 mu50_state::pa_r()
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

u8 mu50_state::pc_r()
{
	u8 res = cur_pc | 0x7c;
	if(!(cur_pc & 0x01))
		res &= m_ioport_o0->read();
	if(!(cur_pc & 0x02))
		res &= m_ioport_o1->read();
	if(!(cur_pc & 0x80))
		res &= m_ioport_o2->read();
	return res;
}

void mu50_state::mu50(machine_config &config)
{
	H83003(config, m_mu50cpu, 10_MHz_XTAL);
	m_mu50cpu->set_addrmap(AS_PROGRAM, &mu50_state::mu50_map);
	m_mu50cpu->read_adc<0>().set(FUNC(mu50_state::adc_ar_r));
	m_mu50cpu->read_adc<1>().set_constant(0);
	m_mu50cpu->read_adc<2>().set(FUNC(mu50_state::adc_al_r));
	m_mu50cpu->read_adc<3>().set_constant(0);
	m_mu50cpu->read_adc<4>().set(FUNC(mu50_state::adc_midisw_r));
	m_mu50cpu->read_adc<5>().set_constant(0);
	m_mu50cpu->read_adc<6>().set(FUNC(mu50_state::adc_battery_r));
	m_mu50cpu->read_adc<7>().set_constant(0);
	m_mu50cpu->read_port6().set(FUNC(mu50_state::p6_r));
	m_mu50cpu->write_port6().set(FUNC(mu50_state::p6_w));
	m_mu50cpu->write_port9().set(FUNC(mu50_state::p9_w));
	m_mu50cpu->read_porta().set(FUNC(mu50_state::pa_r));
	m_mu50cpu->write_porta().set(FUNC(mu50_state::pa_w));
	m_mu50cpu->read_portb().set(FUNC(mu50_state::pb_r));
	m_mu50cpu->write_portb().set(FUNC(mu50_state::pb_w));
	m_mu50cpu->read_portc().set(FUNC(mu50_state::pc_r));
	m_mu50cpu->write_portc().set(FUNC(mu50_state::pc_w));

	m_mu50cpu->read_port7().set_constant(0);
	m_mu50cpu->read_port9().set_constant(0);

	NVRAM(config, m_nvram, nvram_device::DEFAULT_NONE);

	MULCD(config, m_lcd);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SWP00(config, m_swp00);
	m_swp00->add_route(0, "lspeaker", 1.0);
	m_swp00->add_route(1, "rspeaker", 1.0);

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set(m_mu50cpu, FUNC(h83003_device::sci_rx_w<1>));

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);
	m_mu50cpu->write_sci_tx<1>().set(mdout, FUNC(midi_port_device::write_txd));

	config.set_default_layout(layout_mu50);
}

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(bios))

ROM_START( mu50 )
	ROM_REGION( 0x80000, "mu50cpu", 0 )
	ROM_SYSTEM_BIOS( 0, "bios0", "xr174c0 (v1.05, Aug. 21, 1995)" )
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "xr174c0.ic7", 0x000000, 0x080000, CRC(902520a4) SHA1(9ca892920598f9fdf08544dac4c0e54e7d46ee3c) )
	ROM_SYSTEM_BIOS( 1, "bios1", "? (v1.04, May 22, 1995)" )
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "yamaha_mu50.bin", 0x000000, 0x080000, CRC(507168ad) SHA1(58c41f10d292cac35ef0e8f93029fbc4685df586) )

	ROM_REGION( 0x400000, "swp00", 0 )
	// Identical to the db50xg roms
	ROM_LOAD( "xq057c0.ic18", 0x000000, 0x200000, CRC(d4adbc7e) SHA1(32f653c7644d060f5a6d63a435ae3a7412386d92) )
	ROM_LOAD( "xq058c0.ic19", 0x200000, 0x200000, CRC(7b68f475) SHA1(adf68689b4842ec5bc9b0ea1bb99cf66d2dec4de) )
ROM_END

} // anonymous namespace


CONS( 1995, mu50, 0, 0, mu50,  mu50, mu50_state, empty_init, "Yamaha", "MU50", 0 )
