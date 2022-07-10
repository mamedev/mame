// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/h8/h83003.h"
#include "mulcd.h"
#include "sound/meg.h"
#include "sound/dspv.h"

#include "debugger.h"
#include "speaker.h"


static INPUT_PORTS_START( vl70 )
	PORT_START("B0")
	PORT_BIT(0x83, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Play")      PORT_CODE(KEYCODE_A)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Effect")    PORT_CODE(KEYCODE_F)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Midi/WX")   PORT_CODE(KEYCODE_X)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Enter")     PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Exit")      PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("B1")
	PORT_BIT(0x83, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Edit")      PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mode")      PORT_CODE(KEYCODE_M)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part -")    PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select <")  PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part +")    PORT_CODE(KEYCODE_CLOSEBRACE)

	PORT_START("B2")
	PORT_BIT(0x83, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Util")      PORT_CODE(KEYCODE_U)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Breath")    PORT_CODE(KEYCODE_B)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Value -")   PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select >")  PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Value +")   PORT_CODE(KEYCODE_EQUALS)
INPUT_PORTS_END

class vl70_state : public driver_device
{
public:
	vl70_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_vl70cpu(*this, "vl70cpu")
		, m_dspv(*this, "dspv")
		, m_meg(*this, "meg")
		, m_lcd(*this, "lcd")
		, m_ioport_b0(*this, "B0")
		, m_ioport_b1(*this, "B1")
		, m_ioport_b2(*this, "B2")
	{ }

	void vl70(machine_config &config);

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

	required_device<h83003_device> m_vl70cpu;
	required_device<dspv_device> m_dspv;
	required_device<meg_device> m_meg;
	required_device<mulcd_device> m_lcd;
	required_ioport m_ioport_b0;
	required_ioport m_ioport_b1;
	required_ioport m_ioport_b2;

	u8 cur_p1, cur_p2, cur_p3, cur_p5, cur_p6, cur_pa, cur_pc, cur_pf, cur_pg;
	u8 cur_ic32;

	u16 adc_zero_r();
	u16 adc_ar_r();
	u16 adc_al_r();
	u16 adc_midisw_r();
	u16 adc_battery_r();
	u16 adc_breath_r();

	void p6_w(u16 data);
	u16 p6_r();
	void pa_w(u16 data);
	u16 pa_r();
	void pb_w(u16 data);
	void pc_w(u16 data);
	u16 pc_r();

	virtual void machine_start() override;
	void vl70_iomap(address_map &map);
	void vl70_map(address_map &map);
};

void vl70_state::machine_start()
{
	cur_p1 = cur_p2 = cur_p3 = cur_p5 = cur_p6 = cur_pa = cur_pc = cur_pf = cur_pg = cur_ic32 = 0xff;
}

void vl70_state::vl70_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("vl70cpu", 0);
	map(0x200000, 0x20ffff).ram(); // 64K work RAM
	map(0x400000, 0x40007f).m(m_dspv, FUNC(dspv_device::map));
	map(0x600000, 0x60001f).m(m_meg, FUNC(meg_device::map));
}

// Grounded adc input
u16 vl70_state::adc_zero_r()
{
	return 0;
}

// Analog input right (also sent to the swp)
u16 vl70_state::adc_ar_r()
{
	return 0;
}

// Analog input left (also sent to the swp)
u16 vl70_state::adc_al_r()
{
	return 0;
}

// Put the host switch to pure midi
u16 vl70_state::adc_midisw_r()
{
	return 0;
}

// Battery level
u16 vl70_state::adc_battery_r()
{
	return 0x200;
}

// Breath controller
u16 vl70_state::adc_breath_r()
{
	return 0x000;
}

void vl70_state::p6_w(u16 data)
{
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

void vl70_state::pb_w(u16 data)
{
	m_lcd->set_leds(bitswap<6>((data >> 2) ^ 0x3f, 5, 3, 1, 4, 2, 0));
}

void vl70_state::pc_w(u16 data)
{
	cur_pc = data;
}

u16 vl70_state::pc_r()
{
	u8 r = 0xff;
	if(!(cur_pc & 0x01))
		r &= m_ioport_b0->read();
	if(!(cur_pc & 0x02))
		r &= m_ioport_b1->read();
	if(!(cur_pc & 0x80))
		r &= m_ioport_b2->read();
	return r;
}

u16 vl70_state::p6_r()
{
	return cur_p6;
}

void vl70_state::pa_w(u16 data)
{
	cur_pa = data;
}

u16 vl70_state::pa_r()
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

void vl70_state::vl70_iomap(address_map &map)
{
	map(h8_device::PORT_6, h8_device::PORT_6).rw(FUNC(vl70_state::p6_r), FUNC(vl70_state::p6_w));
	map(h8_device::PORT_A, h8_device::PORT_A).rw(FUNC(vl70_state::pa_r), FUNC(vl70_state::pa_w));
	map(h8_device::PORT_B, h8_device::PORT_B).w(FUNC(vl70_state::pb_w));
	map(h8_device::PORT_C, h8_device::PORT_C).rw(FUNC(vl70_state::pc_r), FUNC(vl70_state::pc_w));
	map(h8_device::ADC_0, h8_device::ADC_0).r(FUNC(vl70_state::adc_breath_r));
	map(h8_device::ADC_1, h8_device::ADC_6).r(FUNC(vl70_state::adc_zero_r));
	map(h8_device::ADC_2, h8_device::ADC_2).r(FUNC(vl70_state::adc_midisw_r));
	map(h8_device::ADC_3, h8_device::ADC_6).r(FUNC(vl70_state::adc_zero_r));
	map(h8_device::ADC_4, h8_device::ADC_4).r(FUNC(vl70_state::adc_battery_r));
	map(h8_device::ADC_5, h8_device::ADC_6).r(FUNC(vl70_state::adc_zero_r));
	map(h8_device::ADC_6, h8_device::ADC_6).r(FUNC(vl70_state::adc_zero_r));
	map(h8_device::ADC_7, h8_device::ADC_7).r(FUNC(vl70_state::adc_zero_r));
}

void vl70_state::vl70(machine_config &config)
{
	H83003(config, m_vl70cpu, 10_MHz_XTAL);
	m_vl70cpu->set_addrmap(AS_PROGRAM, &vl70_state::vl70_map);
	m_vl70cpu->set_addrmap(AS_IO, &vl70_state::vl70_iomap);

	MULCD(config, m_lcd);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DSPV(config, m_dspv);
	MEG(config, m_meg);

	auto &mdin_a(MIDI_PORT(config, "mdin_a"));
	midiin_slot(mdin_a);
	mdin_a.rxd_handler().set("vl70cpu:sci1", FUNC(h8_sci_device::rx_w));

	auto &mdin_b(MIDI_PORT(config, "mdin_b"));
	midiin_slot(mdin_b);
	mdin_b.rxd_handler().set("vl70cpu:sci0", FUNC(h8_sci_device::rx_w));

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);
	m_vl70cpu->subdevice<h8_sci_device>("sci0")->tx_handler().set(mdout, FUNC(midi_port_device::write_txd));
}

ROM_START( vl70 )
	ROM_REGION( 0x200000, "vl70cpu", 0 )
	ROM_LOAD16_WORD_SWAP( "vl70m_v111_27c160.bin", 0x000000, 0x200000, CRC(efdba9f0) SHA1(cfa9fb7d2a991e4752393c9677e4ddcbe10866c7) )
ROM_END

CONS( 1996, vl70, 0, 0, vl70,  vl70, vl70_state, empty_init, "Yamaha", "VL70-m", MACHINE_NOT_WORKING )
