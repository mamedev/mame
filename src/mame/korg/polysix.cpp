// license:BSD-3-Clause
// copyright-holders:AJR, O. Galibert
/****************************************************************************

    Korg Polysix (PS-6)

****************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "imagedev/cassette.h"
#include "machine/nvram.h"
#include "softlist_dev.h"
#include "speaker.h"


class polysix_sound_block : public device_t, public device_sound_interface
{
public:
	polysix_sound_block(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void set_pitch(int channel, u8 pitch);
	void set_gates(u8 gates);
	void set_effect_speed(u8 value);
	void set_cutoff(u8 value);
	void set_eg_intensity(u8 value);
	void set_resonance(u8 value);
	void set_attack(u8 value);
	void set_decay(u8 value);
	void set_suspend(u8 value);
	void set_release(u8 value);
	void set_keyboard_tracking(u8 value);
	void set_pw_pwm(u8 value);
	void set_pwm_speed(u8 value);
	void set_mg_speed(u8 value);
	void set_mg_delay(u8 value);
	void set_mg_level(u8 value);
	void set_control_low(u8 value);
	void set_control_high(u8 value);

	u8 get_control_low();

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	virtual void device_start() override;
	virtual void device_reset() override;

private:
	static const std::array<float, 0x100> phase_step;
	static const std::array<float, 0x100> pwm_phase_step;
	static const std::array<float, 0x100> pw_threshold;

	sound_stream *m_stream;

	std::array<float, 6> m_phase;
	std::array<float, 6> m_organ_eg;

	float m_pwm_phase;

	std::array<u8, 6> m_pitch;
	u8 m_gates;
	u8 m_current_gates;

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

	u8 m_control_low;
	u8 m_control_high;
};

DEFINE_DEVICE_TYPE(POLYSIX_SOUND_BLOCK, polysix_sound_block, "polysix_sound_block", "Korg Polysix sound block")

// Phase step at 48KHz for a given pitch for the main oscillator.
// Real range is 0 (C0) to 84 (C7), since it's a 61-keys, 5 octaves
// keyboard with +/- 1 octave transpose capability.

const std::array<float, 0x100> polysix_sound_block::phase_step = []() {
	std::array<float, 0x100> steps;
	// Tune A4 = 440Hz
	for(int i=0; i != 0x100; i++)
		steps[i] = 440.0 * pow(2, (i - 12*4 - 9)/12.0) / 48000;
	return steps;
}();

// Phase step for the pwm.  Actual curve is unclear but looks linear.

const std::array<float, 0x100> polysix_sound_block::pwm_phase_step = []() {
	std::array<float, 0x100> steps;
	// 0 = 0Hz, max = 20Hz, possibly linear?
	for(int i=0; i != 0x100; i++)
		steps[i] = 20 * (i/255.0) / 48000;
	return steps;
}();

// Threshold for the pulse width, linear between 0.5 and 1.1, where 1
// is the actual max value.

const std::array<float, 0x100> polysix_sound_block::pw_threshold = []() {
	std::array<float, 0x100> thr;
	for(int i=0; i != 0x100; i++)
		thr[i] = 0.5 + 0.6 * (i/255.0);
	return thr;
}();

polysix_sound_block::polysix_sound_block(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, POLYSIX_SOUND_BLOCK, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
{
}

void polysix_sound_block::device_start()
{
	m_stream = stream_alloc(0, 1, 48000);

	save_item(NAME(m_phase));
	save_item(NAME(m_organ_eg));
	save_item(NAME(m_pwm_phase));

	save_item(NAME(m_pitch));
	save_item(NAME(m_gates));
	save_item(NAME(m_current_gates));
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
	save_item(NAME(m_control_low));
	save_item(NAME(m_control_high));

	std::fill(m_phase.begin(), m_phase.end(), 0);
	std::fill(m_organ_eg.begin(), m_organ_eg.end(), 0);
	m_pwm_phase = 0;

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
	m_control_low = 0;
	m_control_high = 0;
	m_gates = 0;
	m_current_gates = 0;
}

void polysix_sound_block::device_reset()
{
}

void polysix_sound_block::set_pitch(int channel, u8 pitch)
{
	if(m_pitch[channel] != pitch) {
		m_stream->update();
		m_pitch[channel] = pitch;

		static const char *const notes[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
		logerror("channel %d pitch %02x %s%d\n", channel, pitch, notes[pitch % 12], (pitch/12));
	}
}

void polysix_sound_block::set_gates(u8 gates)
{
	if(gates != m_gates) {
		m_stream->update();
		m_gates = gates;
	}
}

void polysix_sound_block::set_effect_speed(u8 value)
{
	if(value != m_effect_speed) {
		m_stream->update();
		m_effect_speed = value;
		logerror("effect_speed = %02x\n", value);
	}
}

void polysix_sound_block::set_cutoff(u8 value)
{
	if(value != m_cutoff) {
		m_stream->update();
		m_cutoff = value;
		logerror("cutoff = %02x\n", value);
	}
}

void polysix_sound_block::set_eg_intensity(u8 value)
{
	if(value != m_eg_intensity) {
		m_stream->update();
		m_eg_intensity = value;
		logerror("eg_intensity = %02x\n", value);
	}
}

void polysix_sound_block::set_resonance(u8 value)
{
	if(value != m_resonance) {
		m_stream->update();
		m_resonance = value;
		logerror("resonance = %02x\n", value);
	}
}

void polysix_sound_block::set_attack(u8 value)
{
	if(value != m_attack) {
		m_stream->update();
		m_attack = value;
		logerror("attack = %02x\n", value);
	}
}

void polysix_sound_block::set_decay(u8 value)
{
	if(value != m_decay) {
		m_stream->update();
		m_decay = value;
		logerror("decay = %02x\n", value);
	}
}

void polysix_sound_block::set_suspend(u8 value)
{
	if(value != m_suspend) {
		m_stream->update();
		m_suspend = value;
		logerror("suspend = %02x\n", value);
	}
}

void polysix_sound_block::set_release(u8 value)
{
	if(value != m_release) {
		m_stream->update();
		m_release = value;
		logerror("release = %02x\n", value);
	}
}

void polysix_sound_block::set_keyboard_tracking(u8 value)
{
	if(value != m_keyboard_tracking) {
		m_stream->update();
		m_keyboard_tracking = value;
		logerror("keyboard_tracking = %02x\n", value);
	}
}

void polysix_sound_block::set_pw_pwm(u8 value)
{
	if(value != m_pw_pwm) {
		m_stream->update();
		m_pw_pwm = value;
		logerror("pw_pwm = %02x\n", value);
	}
}

void polysix_sound_block::set_pwm_speed(u8 value)
{
	if(value != m_pwm_speed) {
		m_stream->update();
		m_pwm_speed = value;
		logerror("pwm_speed = %02x\n", value);
	}
}

void polysix_sound_block::set_mg_speed(u8 value)
{
	if(value != m_mg_speed) {
		m_stream->update();
		m_mg_speed = value;
		logerror("mg_speed = %02x\n", value);
	}
}

void polysix_sound_block::set_mg_delay(u8 value)
{
	if(value != m_mg_delay) {
		m_stream->update();
		m_mg_delay = value;
		logerror("mg_delay = %02x\n", value);
	}
}

void polysix_sound_block::set_mg_level(u8 value)
{
	if(value != m_mg_level) {
		m_stream->update();
		m_mg_level = value;
		logerror("mg_level = %02x\n", value);
	}
}

void polysix_sound_block::set_control_low(u8 value)
{
	if(value != m_control_low) {
		m_stream->update();
		m_control_low = value;
		logerror("control low vco=%s wave=%s sub=%s mod=%s\n",
				 BIT(value, 0, 2) == 0 ? "16'" : BIT(value, 0, 2) == 1 ? "8'" : BIT(value, 0, 2) == 2 ? "4'" : "?",
				 BIT(value, 2, 2) == 1 ? "tri" : BIT(value, 2, 2) == 0 ? "pw" : BIT(value, 2, 2) == 2 ? "pwm" : "?",
				 BIT(value, 4, 2) == 0 ? "off" : BIT(value, 4, 2) == 1 ? "1oct" : BIT(value, 4, 2) == 2 ? "2oct" : "?",
				 BIT(value, 6, 2) == 0 ? "vca" : BIT(value, 6, 2) == 1 ? "vcf" : BIT(value, 6, 2) == 2 ? "vco" : "?"
				 );
	}
}

void polysix_sound_block::set_control_high(u8 value)
{
	if(value != m_control_high) {
		m_stream->update();
		m_control_high = value;
		logerror("control high vca-mode=%s chorus=%s phase=%s ens=%s p.vol=%x\n",
				 BIT(value, 0) ? "eg" : "square",
				 BIT(value, 1) ? "on" : "off",
				 BIT(value, 2) ? "on" : "off",
				 BIT(value, 3) ? "on" : "off",
				 BIT(value, 4, 4)
				 );
	}
}

u8 polysix_sound_block::get_control_low()
{
	return m_control_low;
}

// #*#*#*#*#*#*#*#
void polysix_sound_block::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	for(int sample=0; sample != outputs[0].samples(); sample++) {
		//		u8 trigger = m_gates & ~m_current_gates;
		float out = 0;

		// Step the pwm phase (common to all channels)
		m_pwm_phase += pwm_phase_step[m_pwm_speed];
		while(m_pwm_phase >= 2)
			m_pwm_phase -= 2;

		//  Wrap into a triangle
		float pwm_phase = m_pwm_phase >= 1 ? 2 - m_pwm_phase : m_pwm_phase;

		//  Compute the threshold
		float pw_thr = pw_threshold[m_pw_pwm];
		if(BIT(m_control_low, 3) || 1)
			// PWM mode, the modulation multiplies the threshold part over 0.5 with the phase wrapped between 0.2 and 1
			pw_thr = 0.5 + (pw_thr - 0.5) * (0.2 + pwm_phase * 0.8);

		for(int channel = 0; channel != 6; channel ++) {
			if((m_gates ^ m_current_gates) & (1 << channel))
				logerror("channel %d %s\n", channel, (m_gates & (1<<channel)) ? "keyon" : "keyoff");

			// Step the phase
			m_phase[channel] += phase_step[m_pitch[channel]];
			while(m_phase[channel] >= 4)
				m_phase[channel] -= 4;

			int subosc_step = int(m_phase[channel]);
			float phase = m_phase[channel] - subosc_step;

			// Generate the initial wave in the [-1, 1] range
			float wave;
			if(BIT(m_control_low, 2))
				// triangle
				wave = 2*(phase - 0.5);

			else
				// PW(M)
				wave = phase >= pw_thr ? 1 : -1;

			// Add the sub-oscillator, if active
			if(BIT(m_control_low, 4, 2))
				wave += (subosc_step & (BIT(m_control_low, 5, 1) ? 2 : 1)) ? 1 : 0;

			// Step the organ EG
			if(BIT(m_gates, channel))
				// When gate is on, charge a 0.047uF cap through a 10K resistor
				m_organ_eg[channel] += (1-m_organ_eg[channel])*0.0433581893516088;   // 1-exp(-1/(10e3 * 0.047e-6 * 48000))
			else
				// When gate is off, discharge a 0.047uF cap through a 230K resistor
				m_organ_eg[channel] -= m_organ_eg[channel]*0.00192537196422815;   // 1-exp(-1/(230e3 * 0.047e-6 * 48000))
			
			out += wave * m_organ_eg[channel];
		}

		m_current_gates = m_gates;
		outputs[0].put_clamp(sample, out/2);
	}
}




class polysix_state : public driver_device
{
public:
	polysix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_progmcu(*this, "progmcu")
		, m_keymcu(*this, "keymcu")
		, m_sound(*this, "soundblock")
		, m_keyboard(*this, "K%u", 0U)
		, m_progboard(*this, "P%u", 0U)
		, m_knobs(*this, "A%x", 0U)
		, m_tape_enable(*this, "TAPE")
		, m_tape(*this, "tape")
	{
	}

	virtual void machine_start() override ATTR_COLD;
	void polysix(machine_config &config);

 	INPUT_CHANGED_MEMBER(tape_enable_w);

private:
	required_device<mcs48_cpu_device> m_progmcu;
	required_device<mcs48_cpu_device> m_keymcu;
	required_device<polysix_sound_block> m_sound;
	std::unique_ptr<u8[]> m_nvram_ptr;
	required_ioport_array<10> m_keyboard;
	required_ioport_array<8> m_progboard;
	required_ioport_array<16> m_knobs;
	required_ioport m_tape_enable;
	required_device<cassette_image_device> m_tape;

	u16 m_pleds;
	u8 m_kleds;

	u8 m_prog_bus;
	u8 m_prog_p1;
	u8 m_prog_p2;
	u8 m_key_bus;
	u8 m_keyon;
	u8 m_key_p2;
	u8 m_trigger;

	bool m_tape_enabled;

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
	save_item(NAME(m_pleds));
	save_item(NAME(m_kleds));
	save_item(NAME(m_key_bus));
	save_item(NAME(m_keyon));
	save_item(NAME(m_trigger));
	save_item(NAME(m_prog_bus));
	save_item(NAME(m_prog_p1));
	save_item(NAME(m_prog_p2));

	save_item(NAME(m_tape_enabled));

	m_pleds = 0;
	m_key_bus = 0;
	m_keyon = 0;
	m_key_p2 = 0;
	m_trigger = 0;

	m_prog_bus = 0;
	m_prog_p1 = 0;
	m_prog_p2 = 0;

	m_tape_enabled = false;
}

u8 polysix_state::prog_r(offs_t offset)
{
	if(BIT(m_prog_p1, 7)) {
		u16 adr = BIT(m_prog_p2, 0) | (offset << 1) | ( BIT(m_prog_p2, 1) << 9);
		return m_nvram_ptr[adr] | 0xf0;
	}

	return 0xff;
}

void polysix_state::prog_w(offs_t offset, u8 data)
{
	if(BIT(m_prog_p1, 7)) {
		u16 adr = BIT(m_prog_p2, 0) | (offset << 1) | ( BIT(m_prog_p2, 1) << 9);
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
		case 0x0: m_sound->set_effect_speed(m_prog_bus); break;
		case 0x1: m_sound->set_cutoff(m_prog_bus); break;
		case 0x2: m_sound->set_eg_intensity(m_prog_bus); break;
		case 0x3: m_sound->set_resonance(m_prog_bus); break;
		case 0x4: m_sound->set_attack(m_prog_bus); break;
		case 0x5: m_sound->set_decay(m_prog_bus); break;
		case 0x6: m_sound->set_suspend(m_prog_bus); break;
		case 0x7: m_sound->set_release(m_prog_bus); break;
		case 0x8: m_sound->set_keyboard_tracking(m_prog_bus); break;
		case 0x9: m_sound->set_pw_pwm(m_prog_bus); break;
		case 0xa: m_sound->set_pwm_speed(m_prog_bus); break;
		case 0xb: m_sound->set_mg_speed(m_prog_bus); break;
		case 0xc: m_sound->set_mg_delay(m_prog_bus); break;
		case 0xd: m_sound->set_mg_level(m_prog_bus); break;
		}
	}	
}

void polysix_state::prog_p2_w(u8 data)
{
	u8 chg = data ^ m_prog_p2;
	m_prog_p2 = data;

	if(BIT(chg, 4) && !BIT(m_prog_p2, 4))
		m_sound->set_control_low(m_prog_bus);

	if(BIT(chg, 5) && !BIT(m_prog_p2, 5))
		m_sound->set_control_high(m_prog_bus);

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
				 BIT(m_pleds, 12) ? " manual/found" : "",
				 BIT(m_pleds, 13) ? " write/loading" : "");
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
	//	logerror("prog t1 %f\n", m_tape->input());
	return m_tape->input() >= 0;
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
// cn12-17 -> int, acki, arpeggiator
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
		m_sound->set_gates(m_tape_enabled ? 0 : m_key_bus & 0x3f);
		// TODO logerror("ic45 = %02x %s\n", m_key_bus, machine().describe_context());
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
		if(index < 6)
			m_sound->set_pitch(index, m_key_bus);
		else
			; // TODO
	}

	if(BIT(chg, 7))
		logerror("cn12-16=%d\n", BIT(m_key_p2, 7));

	//	logerror("key_p2_w row=%x l1=%d l2=%d inh=%d cn12=%d\n", BIT(data, 0, 3), BIT(data, 3), BIT(data, 4), BIT(data, 5), BIT(data, 7));
}

int polysix_state::key_t0_r()
{
	return !BIT(m_sound->get_control_low(), 0);
}

int polysix_state::key_t1_r()
{
	return !BIT(m_sound->get_control_low(), 01);
}

INPUT_CHANGED_MEMBER(polysix_state::tape_enable_w)
{
	logerror("leds tape enable %s\n", newval ? "on" : "off");
	m_tape_enabled = newval;
	m_progmcu->set_input_line(0, newval);
	if(newval)
		m_keyon = 0;
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
	PORT_CONFNAME(0x03, 0x02, "VCO octave")
	PORT_CONFSETTING(    0x03, "16'")
	PORT_CONFSETTING(    0x02, "8'")
	PORT_CONFSETTING(    0x01, "4'")
	PORT_CONFNAME(0x0c, 0x08, "Waveform")
	PORT_CONFSETTING(    0x08, "Triangle")
	PORT_CONFSETTING(    0x0c, "PW")
	PORT_CONFSETTING(    0x04, "PWM")
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("P1")
	PORT_CONFNAME(0x03, 0x01, "Sub oscillator")
	PORT_CONFSETTING(    0x03, "off")
	PORT_CONFSETTING(    0x02, "1 oct")
	PORT_CONFSETTING(    0x01, "2 oct")
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
	PORT_CONFNAME(0x04, 0x04, "write enable")
	PORT_CONFSETTING(   0x00, "enabled")
	PORT_CONFSETTING(   0x04, "disabled")
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

	PORT_START("TAPE")
	PORT_CONFNAME(0x01, 0x00, "Tape access") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(polysix_state::tape_enable_w), 0)
	PORT_CONFSETTING(   0x01, "enabled")
	PORT_CONFSETTING(   0x00, "disabled")

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

	CASSETTE(config, m_tape);
	m_tape->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED);
	m_tape->set_interface("polysix");

	SOFTWARE_LIST(config, "polysix").set_original("polysix");

	POLYSIX_SOUND_BLOCK(config, m_sound);
	m_sound->add_route(0, "speaker", 1.0);

	SPEAKER(config, "speaker").front_center();
}

ROM_START(polysix)
	ROM_REGION(0x400, "progmcu", 0)
	ROM_LOAD("d8048c-345.bin", 0x000, 0x400, CRC(130fb945) SHA1(feaf59d7694de9c3f8009d883a250039f219d046))

	ROM_REGION(0x800, "keymcu", 0)
	ROM_LOAD("d8049c-217.bin", 0x000, 0x800, CRC(246d7767) SHA1(5b608c750e7fe7832070a53a74df416fd132ecb7))

	ROM_REGION(0x400, "nvram", 0)
	ROM_LOAD("nvram.bin", 0x000, 0x400, CRC(1a913972) SHA1(891c7c2aa2f3cd54e3dd6847fad58ab831838c3e))
ROM_END


SYST(1980, polysix, 0, 0, polysix, polysix, polysix_state, empty_init, "Korg", "Polysix Programmable Polyphonic Synthesizer", MACHINE_IS_SKELETON)
