// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/*************************************************************************************

    Yamaha MU-50 : 16-voice polyphonic/multitimbral General MIDI/GS/XG tone modules
    Preliminary driver by R. Belmont and O. Galibert


**************************************************************************************/

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/h8/h83002.h"
#include "machine/mulcd.h"
#include "sound/swp30.h"
#include "sound/meg.h"

#include "debugger.h"
#include "speaker.h"


static INPUT_PORTS_START( mu50 )
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

class mu50_state : public driver_device
{
public:
	mu50_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_mu50cpu(*this, "mu50cpu")
		, m_swp30(*this, "swp30")
		, m_lcd(*this, "lcd")
		, m_ioport_p7(*this, "P7")
		, m_ioport_p8(*this, "P8")
	{ }

	void mu50(machine_config &config);

private:
	enum {
		P2_LCD_RS     = 0x01,
		P2_LCD_RW     = 0x02,
		P2_LCD_ENABLE = 0x04
	};

	enum {
		P6_LCD_RS     = 0x04,
		P6_LCD_RW     = 0x02,
		P6_LCD_ENABLE = 0x01
	};

	enum {
		PA_LCD_RS     = 0x02,
		PA_LCD_ENABLE = 0x20,
		PA_LCD_RW     = 0x40
	};

	required_device<h83002_device> m_mu50cpu;
	required_device<swp30_device> m_swp30;
	required_device<mulcd_device> m_lcd;
	required_ioport m_ioport_p7;
	required_ioport m_ioport_p8;

	u8 cur_p6, cur_pa, cur_pb;

	u16 adc_zero_r();
	u16 adc_ar_r();
	u16 adc_al_r();
	u16 adc_midisw_r();
	u16 adc_battery_r();

	void p6_w(u16 data);
	u16 p6_r();
	void pa_w(u16 data);
	u16 pa_r();
	void pb_w(u16 data);
	u16 pb_r();

	void mu50_iomap(address_map &map);
	void mu50_map(address_map &map);

	virtual void machine_start() override;
};

void mu50_state::machine_start()
{
	cur_p6 = cur_pa = cur_pb = 0xff;
}

void mu50_state::mu50_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("mu50cpu", 0);
	map(0x200000, 0x20ffff).ram(); // 64K work RAM
}

// Grounded adc input
u16 mu50_state::adc_zero_r()
{
	return 0;
}

// Analog input right (also sent to the swp)
u16 mu50_state::adc_ar_r()
{
	return 0;
}

// Analog input left (also sent to the swp)
u16 mu50_state::adc_al_r()
{
	return 0;
}

// Put the host switch to pure midi
u16 mu50_state::adc_midisw_r()
{
	return 0;
}

// Battery level
u16 mu50_state::adc_battery_r()
{
	return 0x200;
}

void mu50_state::p6_w(u16 data)
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

u16 mu50_state::p6_r()
{
	return cur_p6;
}

u16 mu50_state::pb_r()
{
	return cur_pb;
}

void mu50_state::pb_w(u16 data)
{
	cur_pb = data;
}

void mu50_state::pa_w(u16 data)
{
	cur_pa = data;
}

u16 mu50_state::pa_r()
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


void mu50_state::mu50_iomap(address_map &map)
{
	map(h8_device::PORT_6, h8_device::PORT_6).rw(FUNC(mu50_state::p6_r), FUNC(mu50_state::p6_w));
	map(h8_device::PORT_A, h8_device::PORT_A).rw(FUNC(mu50_state::pa_r), FUNC(mu50_state::pa_w));
	map(h8_device::PORT_B, h8_device::PORT_B).rw(FUNC(mu50_state::pb_r), FUNC(mu50_state::pb_w));
	map(h8_device::ADC_0, h8_device::ADC_0).r(FUNC(mu50_state::adc_ar_r));
	map(h8_device::ADC_1, h8_device::ADC_1).r(FUNC(mu50_state::adc_zero_r));
	map(h8_device::ADC_2, h8_device::ADC_2).r(FUNC(mu50_state::adc_al_r));
	map(h8_device::ADC_3, h8_device::ADC_3).r(FUNC(mu50_state::adc_zero_r));
	map(h8_device::ADC_4, h8_device::ADC_4).r(FUNC(mu50_state::adc_midisw_r));
	map(h8_device::ADC_5, h8_device::ADC_6).r(FUNC(mu50_state::adc_zero_r));
	map(h8_device::ADC_6, h8_device::ADC_6).r(FUNC(mu50_state::adc_battery_r));
	map(h8_device::ADC_7, h8_device::ADC_7).r(FUNC(mu50_state::adc_zero_r)); // inputmod from the gate array
}

void mu50_state::mu50(machine_config &config)
{
	H83002(config, m_mu50cpu, 16_MHz_XTAL);
	m_mu50cpu->set_addrmap(AS_PROGRAM, &mu50_state::mu50_map);
	m_mu50cpu->set_addrmap(AS_IO, &mu50_state::mu50_iomap);

	MULCD(config, m_lcd);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	// In truth, swp00
	SWP30(config, m_swp30);
	m_swp30->add_route(0, "lspeaker", 1.0);
	m_swp30->add_route(1, "rspeaker", 1.0);

	auto &mdin_a(MIDI_PORT(config, "mdin_a"));
	midiin_slot(mdin_a);
	mdin_a.rxd_handler().set("mu50cpu:sci1", FUNC(h8_sci_device::rx_w));

	auto &mdin_b(MIDI_PORT(config, "mdin_b"));
	midiin_slot(mdin_b);
	mdin_b.rxd_handler().set("mu50cpu:sci0", FUNC(h8_sci_device::rx_w));

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);
	m_mu50cpu->subdevice<h8_sci_device>("sci0")->tx_handler().set(mdout, FUNC(midi_port_device::write_txd));
}

ROM_START( mu50 )
	ROM_REGION( 0x80000, "mu50cpu", 0 )
	ROM_LOAD16_WORD_SWAP( "yamaha_mu50.bin", 0x000000, 0x080000, CRC(507168ad) SHA1(58c41f10d292cac35ef0e8f93029fbc4685df586) )

	ROM_REGION( 0x1800000, "swp00", ROMREGION_ERASE00 )
ROM_END

CONS( 1995, mu50, 0, 0, mu50,  mu50, mu50_state, empty_init, "Yamaha", "MU50", MACHINE_NOT_WORKING )
