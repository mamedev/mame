// license:BSD-3-Clause
// copyright-holders:AJR, R. Belmont
/****************************************************************************

    Novation Drum Station drum machine.
    Emulation by R. Belmont, based on a skeleton by AJR

    TODO: verify the clocks and the AD1859 control, mute and reset wiring.

****************************************************************************/

#include "emu.h"
#include "bus/midi/midi.h"
#include "cpu/adsp2100/adsp2100.h"
#include "cpu/mn1880/mn1880.h"
#include "machine/eeprompar.h"
#include "sound/ad1859.h"
#include "video/pwm.h"

#include "speaker.h"

#include "drumsta.lh"


namespace {

class drumsta_state : public driver_device
{
public:
	drumsta_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dsp(*this, "dsp")
		, m_samples(*this, "samples")
		, m_dac(*this, "dac%u", 0U)
		, m_display(*this, "display")
		, m_keys(*this, "KEY%u", 0U)
		, m_switches(*this, "SW%u", 0U)
		, m_knobs(*this, "AN%u", 0U)
		, m_leds(*this, "led%u", 0U)
		, m_port3(0)
		, m_segments(0)
		, m_input_select(0xff)
		, m_control_latch(0)
		, m_idma_latch(0)
		, m_dsp_flags(0)
		, m_sport_bank{}
		, m_sport_selected{}
	{
	}

	void drumsta(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void control_latch_w(u8 data);
	void idma_lsb_w(u8 data);
	void idma_msb_w(u8 data);
	void port3_w(u8 data);
	void leds_w(u8 data);
	void segments_w(u8 data);
	u8 keys_r();
	u8 switches_r();
	u8 analog_r();
	u8 byte_memory_r(offs_t offset);
	void dsp_flags_w(u8 data);
	template <int N> void sport_tfs_w(int state);
	template <int N> void sport_dt_w(u16 data);

	void drumsta_prog(address_map &map) ATTR_COLD;
	void drumsta_data(address_map &map) ATTR_COLD;
	void adsp2181_prog(address_map &map) ATTR_COLD;
	void adsp2181_data(address_map &map) ATTR_COLD;

	required_device<mn1880023_device> m_maincpu;
	required_device<adsp2181_device> m_dsp;
	required_region_ptr<u8> m_samples;
	required_device_array<ad1859_device, 4> m_dac;
	required_device<pwm_display_device> m_display;
	required_ioport_array<4> m_keys;
	required_ioport_array<4> m_switches;
	optional_ioport_array<24> m_knobs;
	output_finder<8> m_leds;
	u8 m_port3;
	u8 m_segments;
	u8 m_input_select;

	u8 m_control_latch;
	u8 m_idma_latch;
	u8 m_dsp_flags;
	u8 m_sport_bank[2];
	bool m_sport_selected[2];
};


void drumsta_state::control_latch_w(u8 data)
{
	m_control_latch = data;
}

void drumsta_state::idma_msb_w(u8 data)
{
	m_idma_latch = data;
}

void drumsta_state::idma_lsb_w(u8 data)
{
	switch (m_control_latch & 0xc0)
	{
	case 0x00:
		m_dsp->idma_data_w(u16(m_idma_latch) << 8 | data);
		break;

	case 0xc0:
		m_dsp->idma_addr_w(u16(m_idma_latch) << 8 | data);
		break;
	}
}

void drumsta_state::machine_start()
{
	save_item(NAME(m_port3));
	save_item(NAME(m_segments));
	save_item(NAME(m_input_select));
	save_item(NAME(m_control_latch));
	save_item(NAME(m_idma_latch));
	save_item(NAME(m_dsp_flags));
	save_item(NAME(m_sport_bank));
	save_item(NAME(m_sport_selected));
}


u8 drumsta_state::byte_memory_r(offs_t offset)
{
	return m_samples[offset & 0x7ffff];
}

void drumsta_state::dsp_flags_w(u8 data)
{
	m_dsp_flags = data;
}

template <int N>
void drumsta_state::sport_tfs_w(int state)
{
	// PF1/PF2 gate each frame sync to one DAC pair (inferred)
	if (state)
	{
		m_sport_selected[N] = (m_dsp_flags & 6) == 2 || (m_dsp_flags & 6) == 4;
		m_sport_bank[N] = BIT(m_dsp_flags, 2);
	}
}

template <int N>
void drumsta_state::sport_dt_w(u16 data)
{
	if (!m_sport_selected[N])
	{
		return;
	}

	// send the whole word at once; the DAC alternates left and right itself
	auto &dac = *m_dac[2 * N + m_sport_bank[N]];
	dac.lrclk_w(1);
	dac.bclk_w(1);
	dac.bclk_w(0);
	dac.lrclk_w(0);
	for (int bit = 15; bit >= 0; bit--)
	{
		dac.bclk_w(1);
		dac.sdata_w(BIT(data, bit));
		dac.bclk_w(0);
	}
}

void drumsta_state::port3_w(u8 data)
{
	m_port3 = data;
	m_display->matrix(BIT(m_port3, 5) ? 1 : 2, m_segments);
}

void drumsta_state::leds_w(u8 data)
{
	for (u32 i = 0; i < 8; i++)
	{
		m_leds[i] = !BIT(data, i);
	}
}

void drumsta_state::segments_w(u8 data)
{
	// bits 7-0: d, dp, e, c, b, a, f, g
	m_segments = bitswap<8>(data, 6, 0, 1, 5, 7, 4, 3, 2);
	m_display->matrix(BIT(m_port3, 5) ? 1 : 2, m_segments);
}

u8 drumsta_state::keys_r()
{
	u8 data = 0xff;
	for (u32 i = 0; i < 4; i++)
	{
		if (!BIT(m_input_select, i + 4))
		{
			data &= m_keys[i]->read();
		}
	}
	return data;
}

u8 drumsta_state::switches_r()
{
	u8 data = 0xff;
	for (u32 i = 0; i < 4; i++)
	{
		if (!BIT(m_input_select, i))
		{
			data &= m_switches[i]->read();
		}
	}
	return data;
}

u8 drumsta_state::analog_r()
{
	// three 8-channel muxes, active-low enables on bits 3-5
	for (u32 i = 0; i < 3; i++)
	{
		if (!BIT(m_control_latch, i + 3))
		{
			return m_knobs[8 * i + (m_control_latch & 7)].read_safe(100) * 255 / 100;
		}
	}
	return 0xff;
}

void drumsta_state::machine_reset()
{
	m_input_select = 0xff;
	m_control_latch = 0;
	m_idma_latch = 0;
	m_port3 = 0;
	m_segments = 0;
	leds_w(0xff);
	m_display->clear();
}

void drumsta_state::drumsta_prog(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("program", 0);
}

void drumsta_state::drumsta_data(address_map &map)
{
	map(0x8000, 0x8000).w(FUNC(drumsta_state::idma_msb_w));
	map(0x8800, 0x8800).w(FUNC(drumsta_state::idma_lsb_w));
	map(0x9000, 0x9000).w(FUNC(drumsta_state::control_latch_w));
	map(0x9800, 0x9800).lw8(NAME([this] (u8 data) { m_input_select = data; }));
	map(0xa000, 0xa000).w(FUNC(drumsta_state::leds_w));
	map(0xa800, 0xa800).w(FUNC(drumsta_state::segments_w));
	map(0xb000, 0xb7ff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write));
	map(0xb800, 0xb800).nopw(); // watchdog?
}

void drumsta_state::adsp2181_prog(address_map &map)
{
	map(0x0000, 0x3fff).ram();
}

void drumsta_state::adsp2181_data(address_map &map)
{
	map(0x0000, 0x3fdf).ram();
}


static INPUT_PORTS_START(drumsta)
	PORT_START("KEY0")
	PORT_BIT(0xc3, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Drum Kit 909") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("5") PORT_CODE(KEYCODE_5)

	PORT_START("KEY1")
	PORT_BIT(0xc3, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Drum Kit 808") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("- / Compare / Save Type") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("6") PORT_CODE(KEYCODE_6)

	PORT_START("KEY2")
	PORT_BIT(0xc7, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("7") PORT_CODE(KEYCODE_7)

	PORT_START("KEY3")
	PORT_BIT(0xc3, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mode") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("+ / Write / Save") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("8") PORT_CODE(KEYCODE_8)

	PORT_START("AUDITION")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Audition / Auto Trigger") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0xef, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("SW0")
	PORT_BIT(0x1f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_CONFNAME(0x60, 0x20, "Tom Select")
	PORT_CONFSETTING(0x20, "Low")
	PORT_CONFSETTING(0x60, "Mid")
	PORT_CONFSETTING(0x40, "High")
	PORT_CONFNAME(0x80, 0x00, "Write")
	PORT_CONFSETTING(0x00, "Protect")
	PORT_CONFSETTING(0x80, "Enable")

	PORT_START("SW1")
	PORT_BIT(0x1f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_CONFNAME(0x60, 0x20, "Rimshot / Handclap / Cowbell Select")
	PORT_CONFSETTING(0x20, "Rimshot")
	PORT_CONFSETTING(0x60, "Handclap")
	PORT_CONFSETTING(0x40, "Cowbell")
	PORT_CONFNAME(0x80, 0x00, "Hi-Hat Select")
	PORT_CONFSETTING(0x00, "Closed")
	PORT_CONFSETTING(0x80, "Open")

	PORT_START("SW2")
	PORT_BIT(0x1f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_CONFNAME(0x60, 0x20, "Conga Select")
	PORT_CONFSETTING(0x20, "Low")
	PORT_CONFSETTING(0x60, "Mid")
	PORT_CONFSETTING(0x40, "High")
	PORT_CONFNAME(0x80, 0x00, "Cymbal Select")
	PORT_CONFSETTING(0x00, "Crash")
	PORT_CONFSETTING(0x80, "Ride")

	PORT_START("SW3")
	PORT_BIT(0x9f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_CONFNAME(0x60, 0x20, "Congas / Maracas / Claves Select")
	PORT_CONFSETTING(0x20, "Congas")
	PORT_CONFSETTING(0x60, "Maracas")
	PORT_CONFSETTING(0x40, "Claves")

	PORT_START("AN7")
	PORT_ADJUSTER(50, "Bass Drum Tune")

	PORT_START("AN3")
	PORT_ADJUSTER(100, "Bass Drum Level")

	PORT_START("AN5")
	PORT_ADJUSTER(50, "Bass Drum Attack / Tone")

	PORT_START("AN1")
	PORT_ADJUSTER(50, "Bass Drum Decay")

	PORT_START("AN6")
	PORT_ADJUSTER(50, "Snare Drum Tune")

	PORT_START("AN2")
	PORT_ADJUSTER(100, "Snare Drum Level")

	PORT_START("AN4")
	PORT_ADJUSTER(50, "Snare Drum Tone")

	PORT_START("AN0")
	PORT_ADJUSTER(50, "Snare Drum Snappy")

	PORT_START("AN15")
	PORT_ADJUSTER(50, "Tom Tune")

	PORT_START("AN11")
	PORT_ADJUSTER(100, "Tom Level")

	PORT_START("AN13")
	PORT_ADJUSTER(50, "Tom Decay")

	PORT_START("AN9")
	PORT_ADJUSTER(50, "Rimshot / Handclap / Cowbell Tune")

	PORT_START("AN14")
	PORT_ADJUSTER(100, "Rimshot / Handclap / Cowbell Level")

	PORT_START("AN10")
	PORT_ADJUSTER(50, "Hi-Hat Tune")

	PORT_START("AN12")
	PORT_ADJUSTER(100, "Hi-Hat Level")

	PORT_START("AN8")
	PORT_ADJUSTER(50, "Hi-Hat Decay")

	PORT_START("AN18")
	PORT_ADJUSTER(50, "Cymbal Tune / Tone")

	PORT_START("AN22")
	PORT_ADJUSTER(100, "Cymbal Level")

	PORT_START("AN17")
	PORT_ADJUSTER(50, "Cymbal Decay")

	PORT_START("AN21")
	PORT_ADJUSTER(50, "Conga / Maracas / Claves Tune")

	PORT_START("AN19")
	PORT_ADJUSTER(100, "Conga / Maracas / Claves Level")

	PORT_START("AN23")
	PORT_ADJUSTER(100, "Master Volume")

INPUT_PORTS_END

void drumsta_state::drumsta(machine_config &config)
{
	MN1880023(config, m_maincpu, 8'000'000); // custom silkscreen; clock unconfirmed
	m_maincpu->set_addrmap(AS_PROGRAM, &drumsta_state::drumsta_prog);
	m_maincpu->set_addrmap(AS_DATA, &drumsta_state::drumsta_data);
	m_maincpu->set_port_input_mask(2, 0x10);
	m_maincpu->set_port_input_mask(8, 0xff);
	m_maincpu->set_port_input_mask(9, 0x3c);
	m_maincpu->port_in_cb<2>().set_ioport("AUDITION");
	m_maincpu->port_out_cb<3>().set(FUNC(drumsta_state::port3_w));
	m_maincpu->port_in_cb<8>().set(FUNC(drumsta_state::switches_r));
	m_maincpu->port_in_cb<9>().set(FUNC(drumsta_state::keys_r));
	m_maincpu->ad_in_cb<0>().set(FUNC(drumsta_state::analog_r));
	m_maincpu->uart_txd_cb().set("mdout", FUNC(midi_port_device::write_txd));
	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(m_maincpu, FUNC(mn1880023_device::uart_rxd_w));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	PWM_DISPLAY(config, m_display).set_size(2, 8);
	m_display->set_segmask(3, 0xff);
	config.set_default_layout(layout_drumsta);

	ADSP2181(config, m_dsp, 32'000'000); // twice a 16 MHz CLKIN, unverified
	m_dsp->set_addrmap(AS_PROGRAM, &drumsta_state::adsp2181_prog);
	m_dsp->set_addrmap(AS_DATA, &drumsta_state::adsp2181_data);
	m_dsp->set_internal_peripherals(true);
	m_dsp->bdma_read().set(FUNC(drumsta_state::byte_memory_r));
	m_dsp->pf_out().set(FUNC(drumsta_state::dsp_flags_w));
	m_dsp->sport_tfs<0>().set(FUNC(drumsta_state::sport_tfs_w<0>));
	m_dsp->sport_tfs<1>().set(FUNC(drumsta_state::sport_tfs_w<1>));
	m_dsp->sport_dt<0>().set(FUNC(drumsta_state::sport_dt_w<0>));
	m_dsp->sport_dt<1>().set(FUNC(drumsta_state::sport_dt_w<1>));

	SPEAKER(config, "speaker", 2).front();
	SPEAKER(config, "individual", 6);
	// dac3 is the stereo mix, dac0-dac2 feed individual outputs 6 down to 1
	for (u32 i = 0; i < 4; i++)
	{
		auto &dac = AD1859(config, m_dac[i]);
		dac.set_input_format(ad1859_device::DSP);
		dac.set_word_length(16);
		for (u32 ch = 0; ch < 2; ch++)
		{
			if (i == 3)
			{
				dac.add_route(ch, "speaker", 1.0, ch);
			}
			else
			{
				dac.add_route(ch, "individual", 1.0, 5 - (2 * i + ch));
			}
		}
	}

	EEPROM_2864(config, "eeprom"); // Atmel AT28C64
}

ROM_START(drumsta)
	ROM_REGION(0x8000, "program", 0)
	ROM_LOAD("v1.2.u15", 0x0000, 0x8000, CRC(ab9a8654) SHA1(1914254c714d35031a456c71b1f9f9191df9126d))

	ROM_REGION(0x80000, "samples", 0)
	ROM_LOAD("v1.2.u28", 0x00000, 0x80000, CRC(dbbc9cfe) SHA1(61474c0bc6cfff3efe95527c57e4891f886b02aa))

	// handmade defaults, factory EEPROM undumped: MIDI channel 10, auto-trigger 32
	ROM_REGION(0x2000, "eeprom", ROMREGION_ERASE00)
	ROM_FILL(0x07fb, 1, 10)
	ROM_FILL(0x07fc, 1, 32)
	ROM_FILL(0x07fe, 1, 10)
ROM_END

} // anonymous namespace


SYST(1995, drumsta, 0, 0, drumsta, drumsta, drumsta_state, empty_init, "Novation", "Drum Station", MACHINE_IMPERFECT_SOUND)
