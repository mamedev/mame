// license:BSD-3-Clause
// copyright-holders:AJR, O. Galibert
/****************************************************************************

    Korg Polysix (PS-6)

****************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/nvram.h"


namespace {

class polysix_state : public driver_device
{
public:
	polysix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_progmcu(*this, "progmcu")
		, m_keymcu(*this, "keymcu")
		, m_keyboard(*this, "K%u", 0U)
		, m_progboard(*this, "P%u", 0U)
		, m_knobs(*this, "A%x", 0U)
	{
	}

	virtual void machine_start() override ATTR_COLD;
	void polysix(machine_config &config);

private:
	required_device<mcs48_cpu_device> m_progmcu;
	required_device<mcs48_cpu_device> m_keymcu;
	std::unique_ptr<u8[]> m_nvram_ptr;
	required_ioport_array<10> m_keyboard;
	required_ioport_array<8> m_progboard;
	required_ioport_array<16> m_knobs;

	std::array<u8, 8> m_pitch;

	u8 m_effect_speed;
	u8 m_cutoff;
	u8 m_eg_intensity;
	u8 m_resonance;
	u8 m_attack;
	u8 m_decay;
	u8 m_suspend;
	u8 m_release;
	u8 m_keyboard_tracking;
	u8 m_pw_pwm;
	u8 m_pwm_speed;
	u8 m_mg_speed;
	u8 m_mg_delay;
	u8 m_mg_level;

	u16 m_pleds;
	u16 m_pctrl;
	u8 m_kleds;

	u8 m_prog_bus;
	u8 m_prog_p1;
	u8 m_prog_p2;
	u8 m_key_bus;
	u8 m_keyon;
	u8 m_key_ic2;
	u8 m_key_p2;
	u8 m_trigger;

	void key_bus_w(u8 data);
	u8 key_p1_r();
	void key_p2_w(u8 data);
	int key_t0_r();
	int key_t1_r();

	void prog_map(address_map &map);

	u8 prog_r(offs_t offset);
	void prog_w(offs_t offset, u8 data);
	void prog_bus_w(u8 data);
	u8 prog_p1_r();
	void prog_p1_w(u8 data);
	void prog_p2_w(u8 data);
	int prog_t0_r();
	int prog_t1_r();
};

void polysix_state::prog_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(polysix_state::prog_r), FUNC(polysix_state::prog_w));
}

void polysix_state::machine_start()
{
	m_nvram_ptr = make_unique_clear<u8[]>(0x400);
	subdevice<nvram_device>("nvram")->set_base(&m_nvram_ptr[0], 0x400);

	save_pointer(NAME(m_nvram_ptr), 0x400);
	save_item(NAME(m_pitch));
	save_item(NAME(m_pleds));
	save_item(NAME(m_pctrl));
	save_item(NAME(m_kleds));
	save_item(NAME(m_key_bus));
	save_item(NAME(m_keyon));
	save_item(NAME(m_key_ic2));
	save_item(NAME(m_trigger));
	save_item(NAME(m_prog_bus));
	save_item(NAME(m_prog_p1));
	save_item(NAME(m_prog_p2));

	save_item(NAME(m_effect_speed));
	save_item(NAME(m_cutoff));
	save_item(NAME(m_eg_intensity));
	save_item(NAME(m_resonance));
	save_item(NAME(m_attack));
	save_item(NAME(m_decay));
	save_item(NAME(m_suspend));
	save_item(NAME(m_release));
	save_item(NAME(m_keyboard_tracking));
	save_item(NAME(m_pw_pwm));
	save_item(NAME(m_pwm_speed));
	save_item(NAME(m_mg_speed));
	save_item(NAME(m_mg_delay));
	save_item(NAME(m_mg_level));

	std::fill(m_pitch.begin(), m_pitch.end(), 0);

	m_effect_speed = 0;
	m_cutoff = 0;
	m_eg_intensity = 0;
	m_resonance = 0;
	m_attack = 0;
	m_decay = 0;
	m_suspend = 0;
	m_release = 0;
	m_keyboard_tracking = 0;
	m_pw_pwm = 0;
	m_pwm_speed = 0;
	m_mg_speed = 0;
	m_mg_delay = 0;
	m_mg_level = 0;

	m_pleds = 0;
	m_key_bus = 0;
	m_keyon = 0;
	m_key_ic2 = 0;
	m_key_p2 = 0;
	m_trigger = 0;

	m_prog_bus = 0;
	m_prog_p1 = 0;
	m_prog_p2 = 0;
}

u8 polysix_state::prog_r(offs_t offset)
{
	if(BIT(m_prog_p1, 7)) {
		u16 adr = BIT(m_prog_p2, 0) | (offset << 1) | ( BIT(m_prog_p2, 1) << 9);
		logerror("nvram_r %03x = %x\n", adr, m_nvram_ptr[adr]);
		return m_nvram_ptr[adr] | 0xf0;
	}

	return 0xff;
}

void polysix_state::prog_w(offs_t offset, u8 data)
{
	if(BIT(m_prog_p1, 7)) {
		u16 adr = BIT(m_prog_p2, 0) | (offset << 1) | ( BIT(m_prog_p2, 1) << 9);
		logerror("nvram_w %03x = %x\n", adr, data & 0x0f);
		m_nvram_ptr[adr] = data & 0x0f;
	}
}

u8 polysix_state::prog_p1_r()
{
	u8 res = 0xff;
	for(int i=0; i != 8; i++)
		if(!BIT(m_prog_bus, i))
			res &= m_progboard[i]->read();

	if(!BIT(m_prog_p2, 0))
		machine().debug_break();
	return res;
}

void polysix_state::prog_p1_w(u8 data)
{
	u8 chg = data ^ m_prog_p1;
	m_prog_p1 = data;

	if(BIT(chg, 6) && BIT(m_prog_p1, 6)) {
		switch(m_prog_p2 & 0xf) {
		case 0x0: if(m_prog_bus != m_effect_speed) logerror("effect_speed = %02x\n", m_prog_bus); break;
		case 0x1: if(m_prog_bus != m_cutoff) logerror("cutoff = %02x\n", m_prog_bus); break;
		case 0x2: if(m_prog_bus != m_eg_intensity) logerror("eg_intensity = %02x\n", m_prog_bus); break;
		case 0x3: if(m_prog_bus != m_resonance) logerror("resonance = %02x\n", m_prog_bus); break;
		case 0x4: if(m_prog_bus != m_attack) logerror("attack = %02x\n", m_prog_bus); break;
		case 0x5: if(m_prog_bus != m_decay) logerror("decay = %02x\n", m_prog_bus); break;
		case 0x6: if(m_prog_bus != m_suspend) logerror("suspend = %02x\n", m_prog_bus); break;
		case 0x7: if(m_prog_bus != m_release) logerror("release = %02x\n", m_prog_bus); break;
		case 0x8: if(m_prog_bus != m_keyboard_tracking) logerror("keyboard_tracking = %02x\n", m_prog_bus); break;
		case 0x9: if(m_prog_bus != m_pw_pwm) logerror("pw_pwm = %02x\n", m_prog_bus); break;
		case 0xa: if(m_prog_bus != m_pwm_speed) logerror("pwm_speed = %02x\n", m_prog_bus); break;
		case 0xb: if(m_prog_bus != m_mg_speed) logerror("mg_speed = %02x\n", m_prog_bus); break;
		case 0xc: if(m_prog_bus != m_mg_delay) logerror("mg_delay = %02x\n", m_prog_bus); break;
		case 0xd: if(m_prog_bus != m_mg_level) logerror("mg_level = %02x\n", m_prog_bus); break;
		}

		switch(m_prog_p2 & 0xf) {
		case 0x0: m_effect_speed = m_prog_bus; break;
		case 0x1: m_cutoff = m_prog_bus; break;
		case 0x2: m_eg_intensity = m_prog_bus; break;
		case 0x3: m_resonance = m_prog_bus; break;
		case 0x4: m_attack = m_prog_bus; break;
		case 0x5: m_decay = m_prog_bus; break;
		case 0x6: m_suspend = m_prog_bus; break;
		case 0x7: m_release = m_prog_bus; break;
		case 0x8: m_keyboard_tracking = m_prog_bus; break;
		case 0x9: m_pw_pwm = m_prog_bus; break;
		case 0xa: m_pwm_speed = m_prog_bus; break;
		case 0xb: m_mg_speed = m_prog_bus; break;
		case 0xc: m_mg_delay = m_prog_bus; break;
		case 0xd: m_mg_level = m_prog_bus; break;
		}
	}	
}

void polysix_state::prog_p2_w(u8 data)
{
	u8 chg = data ^ m_prog_p2;
	m_prog_p2 = data;

	if(BIT(chg, 4) && !BIT(m_prog_p2, 4))
		m_pctrl = (m_pctrl & 0xff00) | m_prog_bus;

	if(BIT(chg, 5) && !BIT(m_prog_p2, 5))
		m_pctrl = (m_pctrl & 0x00ff) | (m_prog_bus << 8);

	u16 old_leds = m_pleds;

	if(BIT(chg, 6) && !BIT(m_prog_p2, 6))
		m_pleds = (m_pleds & 0xff00) | m_prog_bus;

	if(BIT(chg, 7) && !BIT(m_prog_p2, 7))
		m_pleds = (m_pleds & 0x00ff) | (m_prog_bus << 8);

	if(m_pleds != old_leds)
		logerror("pleds %c%c%c%c%c%c%c%c %c%c%c%c%s%s\n",
				 BIT(m_pleds,  0) ? '1' : '.',
				 BIT(m_pleds,  1) ? '2' : '.',
				 BIT(m_pleds,  2) ? '3' : '.',
				 BIT(m_pleds,  3) ? '4' : '.',
				 BIT(m_pleds,  4) ? '5' : '.',
				 BIT(m_pleds,  5) ? '6' : '.',
				 BIT(m_pleds,  6) ? '7' : '.',
				 BIT(m_pleds,  7) ? '8' : '.',
				 BIT(m_pleds,  8) ? 'A' : '.',
				 BIT(m_pleds,  9) ? 'B' : '.',
				 BIT(m_pleds, 10) ? 'C' : '.',
				 BIT(m_pleds, 11) ? 'D' : '.',
				 BIT(m_pleds, 12) ? " manual" : "",
				 BIT(m_pleds, 13) ? " write" : "");
}

void polysix_state::prog_bus_w(u8 data)
{
	m_prog_bus = data;
}

int polysix_state::prog_t0_r()
{
	return m_prog_bus <= m_knobs[m_prog_p2 & 0xf]->read();
}

int polysix_state::prog_t1_r()
{
	logerror("prog t1\n");
	return 1;
}

u8 polysix_state::key_p1_r()
{
	u8 res = 0xff;
	for(int i=0; i != 8; i++)
		if(!BIT(m_key_bus, i))
			res &= m_keyboard[i]->read();
	if(!BIT(m_key_p2, 0))
		res &= m_keyboard[8]->read();
	if(!BIT(m_key_p2, 1))
		res &= m_keyboard[9]->read();
	return res;
}

void polysix_state::key_bus_w(u8 data)
{
	m_key_bus = data;
}

// cn12 at klm-371
// cn12-17 -> int, aoki, arpeggiator
// cn12-16 = p27, gnd?
// klm-367
// cn05-1 = reset
// cn05-2 = t0 -- 8'
// cn05-3 = t1 -- 4'
// cn05-4 = ic45.7, mg trigger

void polysix_state::key_p2_w(u8 data)
{
	u8 chg = data ^ m_key_p2;
	m_key_p2 = data;

	if(BIT(chg, 3) && !BIT(m_key_p2, 3)) {
		u8 chgk = m_keyon ^ m_key_bus;
		m_keyon = m_key_bus;
		m_trigger |= m_keyon & chgk & 0x3f;
		if(m_trigger)
			logerror("keyon %c%c%c%c%c%c\n",
					 BIT(m_trigger, 0) ? '0' : '.',
					 BIT(m_trigger, 1) ? '1' : '.',
					 BIT(m_trigger, 2) ? '2' : '.',
					 BIT(m_trigger, 3) ? '3' : '.',
					 BIT(m_trigger, 4) ? '4' : '.',
					 BIT(m_trigger, 5) ? '5' : '.');
		m_trigger = 0;
		if(BIT(chgk, 7))
			logerror("ic45 = %02x %s\n", m_keyon, machine().describe_context());
	}

	if(BIT(chg, 4) && !BIT(m_key_p2, 4)) {
		u8 old_leds = m_kleds;
		m_kleds = m_key_bus;
		if(old_leds != m_kleds)
			logerror("kleds%s%s%s%s%s\n",
					 BIT(m_kleds, 0) ? " key-hold" : "",
					 BIT(m_kleds, 1) ? " chord-memory" : "",
					 BIT(m_kleds, 2) ? " unison" : "",
					 BIT(m_kleds, 3) ? " poly" : "",
					 BIT(m_kleds, 4) ? " arpeggiator" : "");
	}

	if(!BIT(m_key_p2, 5)) {
		int index = BIT(data, 0, 3);
		if(m_key_bus != m_pitch[index]) {
			m_pitch[index] = m_key_bus;
			if(index < 6)
				logerror("voice %d pitch %3d\n", index, m_key_bus);
			else
				logerror("key analog #%d = %3d\n", index, m_key_bus);
		}
	}

	if(BIT(chg, 7))
		logerror("cn12-16=%d\n", BIT(m_key_p2, 7));

	//	logerror("key_p2_w row=%x l1=%d l2=%d inh=%d cn12=%d\n", BIT(data, 0, 3), BIT(data, 3), BIT(data, 4), BIT(data, 5), BIT(data, 7));
}

int polysix_state::key_t0_r()
{
	return BIT(m_pctrl, 0);
}

int polysix_state::key_t1_r()
{
	return BIT(m_pctrl, 01);
}

static INPUT_PORTS_START(polysix)
	PORT_START("K0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_C1
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_CS1
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_D1
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_DS1
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_E1
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_F1
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_FS1
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_G1

	PORT_START("K1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_GS1
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_A1
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_AS1
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_B1
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_C2
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_CS2
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_D2
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_DS2

	PORT_START("K2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_E2
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_F2
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_FS2
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_G2
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_GS2
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_A2
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_AS2
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_B2

	PORT_START("K3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_C3
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_CS3
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_D3
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_DS3
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_E3
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_F3
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_FS3
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_G3

	PORT_START("K4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_GS3
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_A3
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_AS3
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_B3
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_C4
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_CS4
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_D4
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_DS4

	PORT_START("K5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_E4
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_F4
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_FS4
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_G4
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_GS4
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_A4
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_AS4
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_B4

	PORT_START("K6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_C5
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_CS5
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_D5
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_DS5
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_E5
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_F5
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_FS5
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_G5

	PORT_START("K7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_GS5
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_A5
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_AS5
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_B5
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_GM_C6
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("K8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("key hold")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("chord memory")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("unison")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("poly")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("arpeggio")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("K9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_CONFNAME(0x02, 0x02, "Arpeggio latch")
	PORT_CONFSETTING(    0x00, "on")
	PORT_CONFSETTING(    0x02, "off")
	PORT_CONFNAME(0x1c, 0x0c, "Arpeggio mode")
	PORT_CONFSETTING(    0x0c, "up")
	PORT_CONFSETTING(    0x14, "updown")
	PORT_CONFSETTING(    0x18, "down")
	PORT_CONFNAME(0xe0, 0x60, "Arpeggio range")
	PORT_CONFSETTING(    0x60, "full")
	PORT_CONFSETTING(    0xa0, "2 octaves")
	PORT_CONFSETTING(    0xc0, "1 octave")

	PORT_START("P0")
	PORT_CONFNAME(0x03, 0x03, "VCO octave")
	PORT_CONFSETTING(    0x01, "16'")
	PORT_CONFSETTING(    0x02, "8'")
	PORT_CONFSETTING(    0x03, "4'")
	PORT_CONFNAME(0x0c, 0x0c, "Waveform")
	PORT_CONFSETTING(    0x04, "Triangle")
	PORT_CONFSETTING(    0x08, "PW")
	PORT_CONFSETTING(    0x0c, "PWM")
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("P1")
	PORT_CONFNAME(0x03, 0x0c, "Sub oscillator")
	PORT_CONFSETTING(    0x01, "off")
	PORT_CONFSETTING(    0x02, "1 oct")
	PORT_CONFSETTING(    0x03, "2 oct")
	PORT_CONFNAME(0x0c, 0x0c, "MG modulation")
	PORT_CONFSETTING(    0x04, "VCO")
	PORT_CONFSETTING(    0x08, "VCF")
	PORT_CONFSETTING(    0x0c, "VCA")
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("P2")
	PORT_CONFNAME(0x01, 0x00, "VCA mode")
	PORT_CONFSETTING(    0x00, "EG")
	PORT_CONFSETTING(    0x01, "square")
	PORT_CONFNAME(0x0e, 0x0e, "Effect")
	PORT_CONFSETTING(    0x0e, "off")
	PORT_CONFSETTING(    0x06, "ensemble")
	PORT_CONFSETTING(    0x0a, "phaser")
	PORT_CONFSETTING(    0x0c, "chorus")
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("P3")
	PORT_CONFNAME(0x0f, 0x0a, "VCA dB")
	PORT_CONFSETTING(    0x0f, "-10")
	PORT_CONFSETTING(    0x0e, "-8")
	PORT_CONFSETTING(    0x0d, "-6")
	PORT_CONFSETTING(    0x0c, "-4")
	PORT_CONFSETTING(    0x0b, "-2")
	PORT_CONFSETTING(    0x0a, "0")
	PORT_CONFSETTING(    0x09, "+2")
	PORT_CONFSETTING(    0x08, "+4")
	PORT_CONFSETTING(    0x07, "+6")
	PORT_CONFSETTING(    0x06, "+8")
	PORT_CONFSETTING(    0x05, "+10")
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("P4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("P5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("P6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("a / to tape") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("b / from tape") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("c / verify") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("d / cancel") PORT_CODE(KEYCODE_D)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("P7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("found / manual")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("write / loading")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("write enable")
	PORT_BIT(0xf8, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("A0")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_CENTERDELTA(0) PORT_NAME("Effect speed / intensity")

	PORT_START("A1")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_CENTERDELTA(0) PORT_NAME("VCF cutoff")

	PORT_START("A2")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_CENTERDELTA(0) PORT_NAME("EG intensity")

	PORT_START("A3")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_CENTERDELTA(0) PORT_NAME("Resonance")

	PORT_START("A4")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_CENTERDELTA(0) PORT_NAME("Attack")

	PORT_START("A5")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_CENTERDELTA(0) PORT_NAME("Decay")

	PORT_START("A6")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_CENTERDELTA(0) PORT_NAME("Sustain")

	PORT_START("A7")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_CENTERDELTA(0) PORT_NAME("Release")

	PORT_START("A8")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_CENTERDELTA(0) PORT_NAME("Keyboard tracking")

	PORT_START("A9")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_CENTERDELTA(0) PORT_NAME("PW/PWM")

	PORT_START("Aa")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_CENTERDELTA(0) PORT_NAME("PWM speed")

	PORT_START("Ab")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_CENTERDELTA(0) PORT_NAME("MG frequency")

	PORT_START("Ac")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_CENTERDELTA(0) PORT_NAME("MG delay")

	PORT_START("Ad")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_CENTERDELTA(0) PORT_NAME("MG level")

	PORT_START("Ae")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("Af")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void polysix_state::polysix(machine_config &config)
{
	I8048(config, m_progmcu, 6_MHz_XTAL);
	m_progmcu->set_addrmap(AS_IO, &polysix_state::prog_map);
	m_progmcu->p1_in_cb().set(FUNC(polysix_state::prog_p1_r));
	m_progmcu->p1_out_cb().set(FUNC(polysix_state::prog_p1_w));
	m_progmcu->p2_out_cb().set(FUNC(polysix_state::prog_p2_w));
	m_progmcu->bus_out_cb().set(FUNC(polysix_state::prog_bus_w));
	m_progmcu->t0_in_cb().set(FUNC(polysix_state::prog_t0_r));
	m_progmcu->t1_in_cb().set(FUNC(polysix_state::prog_t1_r));

	I8049(config, m_keymcu, 6_MHz_XTAL);
	m_keymcu->p1_in_cb().set(FUNC(polysix_state::key_p1_r));
	m_keymcu->p2_out_cb().set(FUNC(polysix_state::key_p2_w));
	m_keymcu->bus_out_cb().set(FUNC(polysix_state::key_bus_w));
	m_keymcu->t0_in_cb().set(FUNC(polysix_state::key_t0_r));
	m_keymcu->t1_in_cb().set(FUNC(polysix_state::key_t1_r));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // TC5514APL-3 + battery
}

ROM_START(polysix)
	ROM_REGION(0x400, "progmcu", 0)
	ROM_LOAD("d8048c-345.bin", 0x000, 0x400, CRC(130fb945) SHA1(feaf59d7694de9c3f8009d883a250039f219d046))

	ROM_REGION(0x800, "keymcu", 0)
	ROM_LOAD("d8049c-217.bin", 0x000, 0x800, CRC(246d7767) SHA1(5b608c750e7fe7832070a53a74df416fd132ecb7))
ROM_END

} // anonymous namespace


SYST(1980, polysix, 0, 0, polysix, polysix, polysix_state, empty_init, "Korg", "Polysix Programmable Polyphonic Synthesizer", MACHINE_IS_SKELETON)
