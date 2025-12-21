// license:BSD-3-Clause
// copyright-holders:R. Belmont, Felipe Sanches
/***************************************************************************

    acvirus.cpp - Access Virus series

    Skeleton driver by R. Belmont

    Hardware in brief:
        Virus A: SAB 80C535-N    (12 MHz), DSP56303 @ 66 MHz
        Virus B: SAB 80C535-N    (12 MHz), DSP56311 @ ??? MHz (illegible on PCB photo I've seen)
        Virus C: SAF 80C515-L24N (24 MHz), DSP56362 @ 120 MHz

        Virus Rack is same h/w as B, Rack XL is the same h/w as C.
        Virus Classic is supposed to be the same h/w as B but not proven.

    The various 80C5xx chips are i8051-based SoCs with additional I/O ports,
    256 bytes of internal RAM like the 8052, and an analog/digital converter.

    The top 4 bits of port P5 select the bank at 0x8000.

    Hardware Notes:
    The DSP has three SRAM chips, probably 128 kbyte each
    for a total of 128 kwords, mapped to address 0x20000. All three DSP
    buses (P, X, Y) point to the same external memory. There's another 128
    kbyte of battery backed SRAM for the 8051.

    The firmware image fits exactly in an AM29F040-120PC flash chip, and is
    bank switched into the 8051 program address space. The lower 0x8000
    bytes of the address space always points to the first 0x8000 bytes of
    flash (except during firmware upgrade, as I assume the programming
    routine has do run from RAM). The upper 0x8000 bytes of the address
    space can point to any 0x8000 sized bank in flash. A bank switch routine
    is at 0x64B8, and will switch to e.g. bank 2 (offset 0x10000) when A =
    0x20. The low nibble is usually zero, but not always, and I don't know
    how it's interpreted.

    Banks 0-2 contain OS code and data, banks 3-6 contain DSP code and data,
    and banks 8-14 seem to contain factory default settings. There are flash
    programming routines at the beginning of banks 7 and 15, and two at the
    end of bank 6. Not sure why there are so many, and not all are
    identical, so there's probably additional bank switching logic to match.
    All display a charming "DO NOT TOUCH ME" message while programming. :)

    The same bank switching also seems to affect external memory, but I'm
    not sure how the smaller SRAM is mapped. Some external memory locations
    are used for other tasks, like communicating with the DSP.

    The initial DSP program and data upload routine is at 0x1FAA. After
    setting up the bus, it churns out all the 24-bit words in banks 3-6
    (except for headers) as one stream. The DSP will interpret the first
    word as a length, the second as address, and the following "length"
    words will be stored at that address in program memory before execution
    starts there. This is just a very short bootstrap program, which takes
    care of receiving the remaining words in chunks. Each chunks starts with
    three words - a command, an address, and optionally length. Commands 0-2
    store data in P, X, or Y memory respectively. Command 3 splits each
    24-bit word into two 12-bit values and store each of them as a 24-bit
    word in Y memory. Command 4 starts execution at the specified address,
    and doesn't have a length.

***************************************************************************/

#include "emu.h"

#include "bus/midi/midi.h"
#include "cpu/dsp563xx/dsp56303.h"
#include "cpu/dsp563xx/dsp56311.h"
#include "cpu/dsp563xx/dsp56362.h"
#include "cpu/dsp563xx/dsp56364.h"
#include "cpu/mcs51/sab80c535.h"
#include "machine/intelfsh.h"
#include "video/hd44780.h"
#include "video/pwm.h"

#include "emupal.h"
#include "speaker.h"
#include "screen.h"

#include "virusa.lh"
#include "virusb.lh"
#include "virusc.lh"
#include "viruscl.lh"
#include "virusrck.lh"
#include "virusrckxl.lh"


namespace {

class acvirus_state : public driver_device
{
public:
	acvirus_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_banked_ram(*this, "banked_ram", 4 * 0x8000, ENDIANNESS_LITTLE),
		m_lcdc(*this, "lcdc"),
		m_dsp(*this, "dsp"),
		m_rombank(*this, "rombank"),
		m_rambank(*this, "rambank"),
		m_row(*this, "ROW%u", 0U),
		m_knob(*this, "knob_%u", 0U),
		m_leds(*this, "leds"),
		m_mdin(*this, "mdin"),
		m_scan(0),
		m_an_select(0),
		m_led_pattern(0),
		m_mdin_bit(false)
	{ }

	void virus_common(machine_config &config) ATTR_COLD;
	void virusa(machine_config &config) ATTR_COLD;
	void virusb(machine_config &config) ATTR_COLD;
	void virusc(machine_config &config) ATTR_COLD;
	void viruscl(machine_config &config) ATTR_COLD;
	void virusrck(machine_config &config) ATTR_COLD;
	void virusrckxl(machine_config &config) ATTR_COLD;

	void init_virus() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<sab80c535_device> m_maincpu;
	memory_share_creator<u8> m_banked_ram;
	required_device<hd44780_device> m_lcdc;
	required_device<dsp563xx_device> m_dsp;
	required_memory_bank m_rombank;
	required_memory_bank m_rambank;
	required_ioport_array<8> m_row;
	required_ioport_array<32> m_knob;
	required_device<pwm_display_device> m_leds;
	required_device<midi_port_device> m_mdin;

	void prog_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;
	void dsp_p_map(address_map &map) ATTR_COLD;
	void virusc_dsp_p_map(address_map &map) ATTR_COLD;
	void dsp_x_map(address_map &map) ATTR_COLD;
	void dsp_y_map(address_map &map) ATTR_COLD;

	u8 p1_r();
	u8 p3_r();
	u8 p4_r();
	void p1_w(u8 data);
	void p3_w(u8 data);
	void p4_w(u8 data);
	void p5_w(u8 data);

	u8 p402_r();

	void red_palette_init(palette_device &palette) ATTR_COLD;
	void green_palette_init(palette_device &palette) ATTR_COLD;

	u8 m_scan;
	u8 m_an_select;
	u8 m_led_pattern;
	bool m_mdin_bit;
};


void acvirus_state::machine_start()
{
	m_rombank->configure_entries(0, 16, memregion("maincpu")->base(), 0x8000);
	m_rombank->set_entry(3);

	m_rambank->configure_entries(0, 4, m_banked_ram, 0x8000);
	m_rambank->set_entry(0);

	save_item(NAME(m_scan));
	save_item(NAME(m_an_select));
	save_item(NAME(m_led_pattern));
	save_item(NAME(m_mdin_bit));
}

void acvirus_state::machine_reset()
{
}

u8 acvirus_state::p1_r()
{
	return ~0x10; // m_lcdc ready?
}


u8 acvirus_state::p3_r()
{
	return m_mdin_bit ? 0xfe : 0xff; // MIDI in at P3.0
}

void acvirus_state::p1_w(u8 data)
{
	m_lcdc->db_w(((data << 3) & 0xf0) | 0x08);
	m_lcdc->e_w(BIT(data, 5));
	m_lcdc->rw_w(BIT(data, 6));
	m_lcdc->rs_w(BIT(data, 7));
}

void acvirus_state::p3_w(u8 data)
{
	m_an_select = (data >> 4) & 3;
}

u8 acvirus_state::p4_r()
{
	return m_row[m_scan & 7]->read();
}

void acvirus_state::p4_w(u8 data)
{
	m_leds->write_mx(data);
	// logerror("LED write_mx data: %02X\n", data);

	if (BIT(m_scan, 3))
		m_led_pattern = data;
}

void acvirus_state::p5_w(u8 data)
{
	m_rombank->set_entry((data >> 4) & 15);

	if (BIT(data, 3))
		m_rambank->set_entry((data >> 4) & 3);

	m_scan = data & 15;
	m_leds->matrix(1 << m_scan, m_led_pattern);
	// logerror("LED matrix: %d pattern: %02X\n", m_scan, m_led_pattern);
}

void acvirus_state::prog_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0); // fixed 32K of flash image
	map(0x8000, 0xffff).bankr(m_rombank);
}

void acvirus_state::data_map(address_map &map)
{
	map(0x0000, 0x7fff).ram();
	map(0x0400, 0x0407).rw(m_dsp, FUNC(dsp563xx_device::hi08_r), FUNC(dsp563xx_device::hi08_w));
	map(0x8000, 0xffff).bankrw(m_rambank);
}

void acvirus_state::dsp_p_map(address_map &map)
{
	map(0x20000, 0x3ffff).ram();
}

void acvirus_state::virusc_dsp_p_map(address_map &map)
{
	map(0x00c00, 0x00fff).ram(); // FIXME: configured extension of internal RAM
	map(0x20000, 0x3ffff).ram();
}

void acvirus_state::dsp_x_map(address_map &map)
{
	map(0x20000, 0x3ffff).ram();
}

void acvirus_state::dsp_y_map(address_map &map)
{
	map(0x20000, 0x3ffff).ram();
}

void acvirus_state::green_palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(142, 241, 0));
	palette.set_pen_color(1, rgb_t(0, 48, 0));
}

void acvirus_state::red_palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(70, 23, 26));
	palette.set_pen_color(1, rgb_t(234, 56, 57));
}

void acvirus_state::virus_common(machine_config &config)
{
	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(
			[this] (int state) { m_mdin_bit = state; });

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);

	m_maincpu->set_addrmap(AS_PROGRAM, &acvirus_state::prog_map);
	m_maincpu->set_addrmap(AS_DATA,    &acvirus_state::data_map);
	m_maincpu->port_in_cb<1>().set(FUNC(acvirus_state::p1_r));
	m_maincpu->port_out_cb<1>().set(FUNC(acvirus_state::p1_w));
	m_maincpu->port_in_cb<3>().set(FUNC(acvirus_state::p3_r));
	m_maincpu->port_out_cb<3>().set(FUNC(acvirus_state::p3_w));
	m_maincpu->port_out_cb<3>().append(mdout, FUNC(midi_port_device::write_txd)).bit(1);
	m_maincpu->port_in_cb<4>().set(FUNC(acvirus_state::p4_r));
	m_maincpu->port_out_cb<4>().set(FUNC(acvirus_state::p4_w));
	m_maincpu->port_out_cb<5>().set(FUNC(acvirus_state::p5_w));
	m_maincpu->an0_func().set([this] { return m_knob[4*0 + m_an_select]->read(); });
	m_maincpu->an1_func().set([this] { return m_knob[4*1 + m_an_select]->read(); });
	m_maincpu->an2_func().set([this] { return m_knob[4*2 + m_an_select]->read(); });
	m_maincpu->an3_func().set([this] { return m_knob[4*3 + m_an_select]->read(); });
	m_maincpu->an4_func().set([this] { return m_knob[4*4 + m_an_select]->read(); });
	m_maincpu->an5_func().set([this] { return m_knob[4*5 + m_an_select]->read(); });
	m_maincpu->an6_func().set([this] { return m_knob[4*6 + m_an_select]->read(); });
	m_maincpu->an7_func().set([this] { return m_knob[4*7 + m_an_select]->read(); });

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6*16, 8*2+1);
	screen.set_visarea_full();
	screen.set_palette("palette");

	/* Actual device is LM16255 */
	HD44780(config, m_lcdc, 270000); // TODO: clock not measured, datasheet typical clock used
	m_lcdc->set_lcd_size(2, 16);

	PWM_DISPLAY(config, m_leds).set_size(16, 8);

	SPEAKER(config, "speaker", 2).front();
}

void acvirus_state::virusa(machine_config &config)
{
	SAB80C535(config, m_maincpu, XTAL(12'000'000));

	virus_common(config);

	DSP56303(config, m_dsp, 66_MHz_XTAL);
	m_dsp->set_addrmap(dsp563xx_device::AS_P, &acvirus_state::dsp_p_map);
	m_dsp->set_addrmap(dsp563xx_device::AS_X, &acvirus_state::dsp_x_map);
	m_dsp->set_addrmap(dsp563xx_device::AS_Y, &acvirus_state::dsp_y_map);
	m_dsp->set_hard_omr(0xe);

	PALETTE(config, "palette", FUNC(acvirus_state::green_palette_init), 2);
	config.set_default_layout(layout_virusa);
}

void acvirus_state::virusb(machine_config &config)
{
	SAB80C535(config, m_maincpu, XTAL(12'000'000));

	virus_common(config);

	DSP56311(config, m_dsp, 108_MHz_XTAL);
	m_dsp->set_addrmap(dsp563xx_device::AS_P, &acvirus_state::dsp_p_map);
	m_dsp->set_addrmap(dsp563xx_device::AS_X, &acvirus_state::dsp_x_map);
	m_dsp->set_addrmap(dsp563xx_device::AS_Y, &acvirus_state::dsp_y_map);
	m_dsp->set_hard_omr(0xe);

	PALETTE(config, "palette", FUNC(acvirus_state::green_palette_init), 2);
	config.set_default_layout(layout_virusb);
}

void acvirus_state::virusc(machine_config &config)
{
	SAB80C535(config, m_maincpu, XTAL(24'000'000)); // 515 really

	virus_common(config);

	DSP56362(config, m_dsp, 136_MHz_XTAL);
	m_dsp->set_addrmap(dsp563xx_device::AS_P, &acvirus_state::virusc_dsp_p_map);
	m_dsp->set_addrmap(dsp563xx_device::AS_X, &acvirus_state::dsp_x_map);
	m_dsp->set_addrmap(dsp563xx_device::AS_Y, &acvirus_state::dsp_y_map);
	m_dsp->set_hard_omr(0xe);

	PALETTE(config, "palette", FUNC(acvirus_state::red_palette_init), 2);
	config.set_default_layout(layout_virusc);
}

void acvirus_state::viruscl(machine_config &config)
{
	virusb(config);
	config.set_default_layout(layout_viruscl);
}

void acvirus_state::virusrck(machine_config &config)
{
	virusb(config);
	config.set_default_layout(layout_virusrck);
}

void acvirus_state::virusrckxl(machine_config &config)
{
	virusc(config);
	config.set_default_layout(layout_virusrckxl);
}

INPUT_PORTS_START( virusa_knobs )
	PORT_START("knob_0")
	PORT_ADJUSTER(64, "Master Volume") PORT_MINMAX(0, 127)

	PORT_START("knob_1")
	PORT_ADJUSTER(64, "Definable 1") PORT_MINMAX(0, 127)

	PORT_START("knob_2")
	PORT_ADJUSTER(64, "Definable 2") PORT_MINMAX(0, 127)

	PORT_START("knob_3")
	PORT_ADJUSTER(64, "LFO 1: Rate") PORT_MINMAX(0, 127)

	PORT_START("knob_4")
	PORT_ADJUSTER(64, "LFO 2: Rate") PORT_MINMAX(0, 127)

	PORT_START("knob_5")
	PORT_ADJUSTER(64, "Osc 1: Shape") PORT_MINMAX(0, 127)

	PORT_START("knob_6")
	PORT_ADJUSTER(64, "Osc 1: Wave/PW") PORT_MINMAX(0, 127)

	PORT_START("knob_7")
	PORT_ADJUSTER(64, "Osc 2: Shape") PORT_MINMAX(0, 127)

	PORT_START("knob_8")
	PORT_ADJUSTER(64, "Osc 2: Wave/PW") PORT_MINMAX(0, 127)

	PORT_START("knob_9")
	PORT_ADJUSTER(64, "Osc 2 Detune") PORT_MINMAX(0, 127)

	PORT_START("knob_10")
	PORT_ADJUSTER(64, "Osc 2 Semitone") PORT_MINMAX(0, 127)

	PORT_START("knob_11")
	PORT_ADJUSTER(64, "Osc 2 FM Amount") PORT_MINMAX(0, 127)

	PORT_START("knob_12")
	PORT_ADJUSTER(64, "Mixer Osc Bal") PORT_MINMAX(0, 127)

	PORT_START("knob_13")
	PORT_ADJUSTER(64, "Mixer Sub Osc") PORT_MINMAX(0, 127)

	PORT_START("knob_14")
	PORT_ADJUSTER(64, "Mixer Osc Volume") PORT_MINMAX(0, 127)

	PORT_START("knob_15")
	PORT_ADJUSTER(64, "Value (Program)") PORT_MINMAX(0, 127)

	PORT_START("knob_16")
	PORT_ADJUSTER(64, "Cutoff") PORT_MINMAX(0, 127)

	PORT_START("knob_17")
	PORT_ADJUSTER(64, "Cutoff 2") PORT_MINMAX(0, 127)

	PORT_START("knob_18")
	PORT_ADJUSTER(64, "Filter Attack") PORT_MINMAX(0, 127)

	PORT_START("knob_19")
	PORT_ADJUSTER(64, "Amp Attack") PORT_MINMAX(0, 127)

	PORT_START("knob_20")
	PORT_ADJUSTER(64, "Filter Resonance") PORT_MINMAX(0, 127)

	PORT_START("knob_21")
	PORT_ADJUSTER(64, "Filter EnvAmount") PORT_MINMAX(0, 127)

	PORT_START("knob_22")
	PORT_ADJUSTER(64, "Filter Key Follow") PORT_MINMAX(0, 127)

	PORT_START("knob_23")
	PORT_ADJUSTER(64, "Filter Balance") PORT_MINMAX(0, 127)

	PORT_START("knob_24")
	PORT_ADJUSTER(64, "Filter Decay") PORT_MINMAX(0, 127)

	PORT_START("knob_25")
	PORT_ADJUSTER(64, "Filter Sustain") PORT_MINMAX(0, 127)

	PORT_START("knob_26")
	PORT_ADJUSTER(64, "Filter Time") PORT_MINMAX(0, 127)

	PORT_START("knob_27")
	PORT_ADJUSTER(64, "Filter Release") PORT_MINMAX(0, 127)

	PORT_START("knob_28")
	PORT_ADJUSTER(64, "Amp Decay") PORT_MINMAX(0, 127)

	PORT_START("knob_29")
	PORT_ADJUSTER(64, "Amp Sustain") PORT_MINMAX(0, 127)

	PORT_START("knob_30")
	PORT_ADJUSTER(64, "Amp Time") PORT_MINMAX(0, 127)

	PORT_START("knob_31")
	PORT_ADJUSTER(64, "Amp Release") PORT_MINMAX(0, 127)
INPUT_PORTS_END


INPUT_PORTS_START( virusc_knobs )
	PORT_START("knob_0")
	PORT_ADJUSTER(64, "LFOs: Rate") PORT_MINMAX(0, 127)

	PORT_START("knob_1")
	PORT_ADJUSTER(64, "Effects: Send") PORT_MINMAX(0, 127)

	PORT_START("knob_2")
	PORT_ADJUSTER(64, "Master Volume") PORT_MINMAX(0, 127)

	PORT_START("knob_3")
	PORT_ADJUSTER(64, "Osc: Shape") PORT_MINMAX(0, 127)

	PORT_START("knob_4")
	PORT_ADJUSTER(64, "Effects: Delay / Rev Time") PORT_MINMAX(0, 127) // "Delay Decay"

	PORT_START("knob_5")
	PORT_ADJUSTER(64, "Effects: Intensity") PORT_MINMAX(0, 127)

	PORT_START("knob_6")
	PORT_ADJUSTER(64, "Effects: Type / Mix") PORT_MINMAX(0, 127)

	PORT_START("knob_7")
	PORT_ADJUSTER(64, "Effects: Feedback / Damping") PORT_MINMAX(0, 127) // "Delay Feedback"

	PORT_START("knob_8")
	PORT_ADJUSTER(64, "Mix: Osc Bal") PORT_MINMAX(0, 127)

	PORT_START("knob_9")
	PORT_ADJUSTER(64, "Soft Knob 1") PORT_MINMAX(0, 127)

	PORT_START("knob_10")
	PORT_ADJUSTER(64, "Soft Knob 2 / Value") PORT_MINMAX(0, 127)

	PORT_START("knob_11")
	PORT_ADJUSTER(64, "Mix: Sub Osc") PORT_MINMAX(0, 127)

	PORT_START("knob_12")
	PORT_ADJUSTER(64, "Osc: Wave Sel / PW") PORT_MINMAX(0, 127)

	PORT_START("knob_13")
	PORT_ADJUSTER(64, "Osc: Detune 2/3") PORT_MINMAX(0, 127)

	PORT_START("knob_14")
	PORT_ADJUSTER(64, "Osc: FM Amount") PORT_MINMAX(0, 127)

	PORT_START("knob_15")
	PORT_ADJUSTER(64, "Osc: Semitone") PORT_MINMAX(0, 127)

	PORT_START("knob_16")
	PORT_ADJUSTER(64, "Amp: Attack") PORT_MINMAX(0, 127)

	PORT_START("knob_17")
	PORT_ADJUSTER(64, "Filters: Attack") PORT_MINMAX(0, 127)

	PORT_START("knob_18")
	PORT_ADJUSTER(64, "Filters: Cutoff 2") PORT_MINMAX(0, 127)

	PORT_START("knob_19")
	PORT_ADJUSTER(64, "Amp: Decay") PORT_MINMAX(0, 127)

	PORT_START("knob_20")
	PORT_ADJUSTER(64, "Mix: Osc Vol") PORT_MINMAX(0, 127)

	PORT_START("knob_21")
	PORT_ADJUSTER(64, "Filters: Cutoff") PORT_MINMAX(0, 127)

	PORT_START("knob_22")
	PORT_ADJUSTER(64, "Mix: Noise") PORT_MINMAX(0, 127)

	PORT_START("knob_23")
	PORT_ADJUSTER(64, "Mix: Ring Mod") PORT_MINMAX(0, 127)

	PORT_START("knob_24")
	PORT_ADJUSTER(64, "Filters: Release") PORT_MINMAX(0, 127)

	PORT_START("knob_25")
	PORT_ADJUSTER(64, "Filters: Balance") PORT_MINMAX(0, 127)

	PORT_START("knob_26")
	PORT_ADJUSTER(64, "Amp: Sustain") PORT_MINMAX(0, 127)

	PORT_START("knob_27")
	PORT_ADJUSTER(64, "Amp: Release") PORT_MINMAX(0, 127)

	PORT_START("knob_28")
	PORT_ADJUSTER(64, "Filters: Decay") PORT_MINMAX(0, 127)

	PORT_START("knob_29")
	PORT_ADJUSTER(64, "Filters: Sustain") PORT_MINMAX(0, 127)

	PORT_START("knob_30")
	PORT_ADJUSTER(64, "Filters: Resonance") PORT_MINMAX(0, 127)

	PORT_START("knob_31")
	PORT_ADJUSTER(64, "Filters: Env Amount") PORT_MINMAX(0, 127)
INPUT_PORTS_END


static INPUT_PORTS_START( virusa )
	PORT_INCLUDE( virusa_knobs )

	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_1) PORT_NAME("Key Follow")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_V) PORT_NAME("Multi")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_2) PORT_NAME("LFO 1: Env Mode")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_Q) PORT_NAME("Key Trigger") // hold at boot: LED & Knob tests
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_X) PORT_NAME("Demo: Ctrl") // a.k.a. "System"
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_C) PORT_NAME("Demo: Edit")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_W) PORT_NAME("LFO 2: Env Mode") // hold at boot: Test Knobs
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_T) PORT_NAME("FEnvMod")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_5) PORT_NAME("Distortion")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_0) PORT_NAME("Filters: Select 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_B) PORT_NAME("Single")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_9) PORT_NAME("Filters: Select 1")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_Y) PORT_NAME("Oscilators: Sync")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_A) PORT_NAME("Transpose -") // hold at boot: RAM test
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_S) PORT_NAME("Transpose +") // hold at boot: Press a key!
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_E) PORT_NAME("LFO 2: Amount") // hold at boot: "      0      " / "AMP.Release KNOB"
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_R) PORT_NAME("LFO 2: Shape")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_3) PORT_NAME("LFO 1: Amount")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_4) PORT_NAME("LFO 1: Shape") // hold at boot: "Initialize" / "Edit-Buffers"
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Store") // hold at boot to access OS update screen
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_N) PORT_NAME("Value -")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_J) PORT_NAME("Parameter <")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_K) PORT_NAME("Parameter >")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_M) PORT_NAME("Value +")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_6) PORT_NAME("Filter 1: Mode") // hold at boot: ROM/RAM checks
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_7) PORT_NAME("Filter 2: Mode")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_8) PORT_NAME("Filter Routing")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW4")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW5")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW6")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW7")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


static INPUT_PORTS_START( virusb )
	PORT_INCLUDE( virusa_knobs )

	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_1) PORT_NAME("LFO 1: Edit")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_V) PORT_NAME("Multi")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_2) PORT_NAME("LFO 1: Env Mode")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_Q) PORT_NAME("LFO 2: Edit") // hold at boot: LED & Knob tests
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_X) PORT_NAME("Demo: Ctrl") // a.k.a. "System"
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_C) PORT_NAME("Demo: Edit")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_W) PORT_NAME("LFO 2: Env Mode") // hold at boot: Test Knobs
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_Z) PORT_NAME("Effects") // a.k.a. "EfxEdit"

	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_T) PORT_NAME("Oscilators: Edit")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_5) PORT_NAME("Filters: Edit")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_0) PORT_NAME("Filters: Select 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_B) PORT_NAME("Single")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_9) PORT_NAME("Filters: Select 1")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_Y) PORT_NAME("Oscilators: Sync")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_O) PORT_NAME("Part +") // boot screen shows "Indigo"
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_A) PORT_NAME("Transpose -") // hold at boot: RAM test
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_S) PORT_NAME("Transpose +") // hold at boot: "Press a key!"
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_E) PORT_NAME("LFO 2: Amount") // hold at boot: "AMP.Rel0  e KNOB"
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_R) PORT_NAME("LFO 2: Shape")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_3) PORT_NAME("LFO 1: Amount")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_4) PORT_NAME("LFO 1: Shape") // hold at boot: "Initialize" / "Edit-Buffers"
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Store") // hold at boot to access OS update screen
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_N) PORT_NAME("Value -")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_J) PORT_NAME("Parameter <")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_K) PORT_NAME("Parameter >")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_M) PORT_NAME("Value +")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_6) PORT_NAME("Filter 1: Mode") // hold at boot: ROM/RAM checks
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_7) PORT_NAME("Filter 2: Mode")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_8) PORT_NAME("Filter Routing")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_I) PORT_NAME("Part -")

	PORT_START("ROW4")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW5")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW6")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW7")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


static INPUT_PORTS_START( virusc )
	PORT_INCLUDE( virusc_knobs )

	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_1) PORT_NAME("Arp: Edit")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_2) PORT_NAME("Effects: Edit")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_3) PORT_NAME("Delay: Edit") // hold at boot: RAM test & key test
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_4) PORT_NAME("Arp: On")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_5) PORT_NAME("Effects: Select")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_6) PORT_NAME("LFOs/Mod: Edit")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_7) PORT_NAME("LFOs/Mod: Amount")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_8) PORT_NAME("LFOs/Mod: Select")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_9) PORT_NAME("LFOs/Mod: Shape")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_0) PORT_NAME("Edit")//?
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_Q) PORT_NAME("Global")//?
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_W) PORT_NAME("Osc: Edit")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_E) PORT_NAME("Osc: Sync")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_R) PORT_NAME("Osc 1: Select")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_T) PORT_NAME("Random")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_Y) PORT_NAME("Undo") // hold at boot: Teste EditMode / KeyTab
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_U) PORT_NAME("Transpose -") // hold at boot: Key test
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_I) PORT_NAME("Transpose +") // hold at boot: LED test (change LED with "Value +" & "Value -")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Store") // hold at boot: OS update
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_O) PORT_NAME("Multi") // hold at boot: Knob test
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_P) PORT_NAME("Single") // hold at boot: "AMP.Release KNOB 0"
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_A) PORT_NAME("Osc 2: Select")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_S) PORT_NAME("Part -")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_D) PORT_NAME("Parameter <")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_F) PORT_NAME("Value -")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_G) PORT_NAME("Part +")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_H) PORT_NAME("Parameter >")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_J) PORT_NAME("Value +")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_K) PORT_NAME("Osc 3: Select")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_L) PORT_NAME("Osc 3 On")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_Z) PORT_NAME("Filters: Edit")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_X) PORT_NAME("Filter 1 Mode") // hold at boot: "GlobalBuff checks"
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_C) PORT_NAME("Filter 2 Mode")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_V) PORT_NAME("Filter 1 Select")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_B) PORT_NAME("Filter 2 Select")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW5")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW6")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW7")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( virusrck )
	PORT_INCLUDE( virusa_knobs )

	PORT_START("ROW0")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW1")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW2")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW3")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW4")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_Q) PORT_NAME("Power")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_A) PORT_NAME("Part -") // hold at boot: "Displaytyp: 77 unbekannt" and then RAM test and Keys test
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_D) PORT_NAME("Multi") // hold at boot: LEDs test / Knobs test
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_U) PORT_NAME("Parameter -")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_J) PORT_NAME("Value -")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_P) PORT_NAME("Edit Up") // hold at boot: "GlobalBuff checks"
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_W) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Store") // hold at boot: OS update
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_S) PORT_NAME("Part +") // hold at boot: Key test
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_F) PORT_NAME("Single")// hold at boot: Knobs test
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_I) PORT_NAME("Parameter +")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_K) PORT_NAME("Value +")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_L) PORT_NAME("Edit Down")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW7")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


ROM_START( virusa )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v2.8", "OS 2.8")
	ROMX_LOAD( "virus_a_28.bin", 0x000000, 0x080000, CRC(087cd808) SHA1(fe3310a165c208473822455c75ee5b2a6de34bc8), ROM_BIOS(0))
ROM_END

ROM_START( virusb )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v4.9t", "OS 4.9 T (049) - Jan 9, 2004")
	ROMX_LOAD( "virus_bt_490x049.bin", 0x000000, 0x080000, CRC(4ffc928a) SHA1(ee4b83e2eb1f01c73e37e2ff1d2edd653a0dcf5b), ROM_BIOS(0))
ROM_END

ROM_START( virusc )
	ROM_DEFAULT_BIOS("v6.6")
	ROM_SYSTEM_BIOS(0, "v6.5", "OS 6.5 (352) Nov 10, 2003")
	ROM_SYSTEM_BIOS(1, "v6.6", "OS 6.6 (368) Dec 14, 2004")

	ROM_REGION(0x80000, "maincpu", 0)
	ROMX_LOAD("virus_c_650x352.bin", 0x000000, 0x080000, CRC(d44a9468) SHA1(fad9b896b39a43a1d46acb1d780b78b775a609b8), ROM_BIOS(0))
	ROMX_LOAD("virus_c_660x368.bin", 0x000000, 0x080000, CRC(923ed4bc) SHA1(4c34546889d90522a8d85300c471f18339f88f72), ROM_BIOS(1))
ROM_END

ROM_START( virusrck )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v2.1t", "OS 2.1 T (071) - Jan 31, 2003")
	ROMX_LOAD( "virus_rt_210x071.bin", 0x000000, 0x080000, CRC(62b2bcc1) SHA1(241467bcb563736472a6e61f6c9c532590664500), ROM_BIOS(0))
ROM_END

ROM_START( virusrckxl )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v6.5", "OS 6.5 (079) Nov 10, 2003")
	ROMX_LOAD( "virus_xl_650x079.bin", 0x000000, 0x080000, CRC(d0721c46) SHA1(b7c292b66ba3690a4a50592e17321b9c4147621d), ROM_BIOS(0))
ROM_END

ROM_START( viruscl )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v4.9", "OS 4.9 (061) Mar 26, 2004")
	ROMX_LOAD( "virus_cl_061_release.bin", 0x000000, 0x080000, CRC(a202e443) SHA1(33d5f4ebbacc817ab1e5dd572e8dc755f6c5e253), ROM_BIOS(0))
ROM_END

} // anonymous namespace


SYST( 1997, virusa,     0, 0, virusa, virusa, acvirus_state, empty_init, "Access", "Virus A", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
SYST( 1999, virusb,     0, 0, virusb, virusb, acvirus_state, empty_init, "Access", "Virus B (Ver. T)", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
SYST( 2002, virusc,     0, 0, virusc, virusc, acvirus_state, empty_init, "Access", "Virus C", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
SYST( 2001, virusrck,   0, 0, virusrck, virusrck, acvirus_state, empty_init, "Access", "Virus Rack (Ver. T)", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
SYST( 2002, virusrckxl, 0, 0, virusrckxl, virusrck, acvirus_state, empty_init, "Access", "Virus Rack XL", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
SYST( 2004, viruscl,    0, 0, viruscl, virusb, acvirus_state, empty_init, "Access", "Virus Classic", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
