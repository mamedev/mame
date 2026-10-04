// license:BSD-3-Clause
// copyright-holders:Wouter van Nifterick
/*************************************************************************************

    Yamaha FS1R
    4-part, 32-voice FM synthesizer with formant synthesis

    CPU:    SH7044F (HD64F7044F), 7 MHz xtal, PLL x4. 256KB on-chip flash.
    EPROM:  IC4 16M (16-bit) on CS0
    SRAM:   IC3 1M (8-bit) on CS1, battery backed
    DRAM:   IC2 4M (16-bit) at 0x01000000
    Sound:  IC10/IC11 YMP706-F tone generators (16 channels each). Their
            channel outputs loop through IC31 YSS236-F (per-note filter)
            and IC11 feeds IC12 YSS236-F (effects, with IC15 4M DRAM).
            24.576 MHz, 512fs into IC13/IC14 LC78834M DACs.
    Panel:  HD44780 LCD bit-banged on PE5-15, LED latch on CS2.
            5x4 switch matrix, rows driven low on PA4, PA9, PB9, PA15, PA3
            in scan order, read back on PE0-3.
            Four endless knobs with two wipers each: FM on AN4/AN3,
            Attack/Release/Formant on AN5/AN6/AN7 through IC25, which
            PE4 switches between the two wipers. AN0 is the battery.

    TODO:
    - Effects (the YSS236 microprogram is not decoded)
    - Individual outputs

**************************************************************************************/

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/sh/sh7042.h"
#include "machine/nvram.h"
#include "sound/ymp706.h"
#include "sound/yss236.h"

#include "mulcd.h"
#include "speaker.h"

#include "fs1r.lh"


namespace {

INPUT_PORTS_START( fs1r )
	PORT_START("KEY0") // PA4
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Value +")    PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x0e, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY1") // PA9
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Cursor <")   PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Cursor >")   PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Exit")       PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Value -")    PORT_CODE(KEYCODE_MINUS)

	PORT_START("KEY2") // PB9
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Mute/Solo")  PORT_CODE(KEYCODE_M)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Part -")     PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Part +")     PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Enter")      PORT_CODE(KEYCODE_ENTER)

	PORT_START("KEY3") // PA15
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Effect")     PORT_CODE(KEYCODE_E)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Voice")      PORT_CODE(KEYCODE_V)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Knob Lower") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Knob Upper") PORT_CODE(KEYCODE_I)

	PORT_START("KEY4") // PA3
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Play")       PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Util")       PORT_CODE(KEYCODE_U)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Search")     PORT_CODE(KEYCODE_S)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Perform")    PORT_CODE(KEYCODE_P)

	// Endless knobs, the value is the angle in firmware steps (64 per turn)
	PORT_START("KNOB1")
	PORT_ADJUSTER(128, "Attack") PORT_MINMAX(0, 255)
	PORT_START("KNOB2")
	PORT_ADJUSTER(128, "Release") PORT_MINMAX(0, 255)
	PORT_START("KNOB3")
	PORT_ADJUSTER(128, "Formant") PORT_MINMAX(0, 255)
	PORT_START("KNOB4")
	PORT_ADJUSTER(128, "FM") PORT_MINMAX(0, 255)
INPUT_PORTS_END

class fs1r_state : public driver_device
{
public:
	fs1r_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcd(*this, "lcd")
		, m_ymp_sub(*this, "ymp706_sub")
		, m_ymp_main(*this, "ymp706_main")
		, m_vop_filter(*this, "yss236_filter")
		, m_vop_effect(*this, "yss236_effect")
		, m_keys(*this, "KEY%u", 0U)
		, m_knobs(*this, "KNOB%u", 1U)
	{ }

	void fs1r(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<sh7043a_device> m_maincpu;
	required_device<fs1rlcd_device> m_lcd;
	required_device<ymp706_device> m_ymp_sub;
	required_device<ymp706_device> m_ymp_main;
	required_device<yss236_device> m_vop_filter;
	required_device<yss236_device> m_vop_effect;
	required_ioport_array<5> m_keys;
	required_ioport_array<4> m_knobs;

	u32 m_pa = 0xffffffff;
	u16 m_pb = 0xffff;
	u16 m_pe = 0;

	void map(address_map &map) ATTR_COLD;

	u16 pe_r();
	void pe_w(u16 data);
	u16 knob_r(int knob, bool second) const;
};

void fs1r_state::machine_start()
{
	save_item(NAME(m_pa));
	save_item(NAME(m_pb));
	save_item(NAME(m_pe));

	m_ymp_sub->set_filter(*m_vop_filter, 0);
	m_ymp_main->set_filter(*m_vop_filter, 16);

	// The filter coefficient step addresses for each of the 16 slots come
	// from tables in the EPROM (CPU address 0x36AE50 + table).
	address_space &s = m_maincpu->space(AS_PROGRAM);
	auto table = [&s] (u32 table, int slot, int size) -> u16
	{
		offs_t const a = 0x36ae50 + table + slot * size;
		return size == 2 ? s.read_word(a) : s.read_byte(a);
	};
	static constexpr u32 taps[6] = { 0xa06e, 0xa07e, 0xa08e, 0xa0ae, 0xa09e, 0xa0be };
	for (int slot = 0; slot < 16; slot++)
	{
		yss236_device::slot_map m;
		m.cut = table(0x9ff4, slot, 2);
		m.reso = table(0xa014, slot, 2);
		m.feg_depth = table(0x9f94, slot, 2);
		m.lfo_depth = table(0x9fb4, slot, 2);
		m.gain = table(0x9f14, slot, 2);
		m.prog = table(0x9fd4, slot, 2);
		for (int t = 0; t < 6; t++)
			m.tap[t] = table(taps[t], slot, 1);
		m_vop_filter->set_slot_map(slot, m);
		m_vop_filter->set_eg_slot(table(0xa0de, slot, 1) & 15, slot);
	}
}

// PE0-3: switch matrix columns. HD44780 bit-bang: DB0-7 = PE5-12,
// E = PE13, RS = PE14, RW = PE15.
u16 fs1r_state::pe_r()
{
	bool const row[5] = { !BIT(m_pa, 4), !BIT(m_pa, 9), !BIT(m_pb, 9), !BIT(m_pa, 15), !BIT(m_pa, 3) };
	u16 data = 0x000f;
	for (int i = 0; i < 5; i++)
		if (row[i])
			data &= m_keys[i]->read();

	if (BIT(m_pe, 13) && BIT(m_pe, 15))
		data |= (BIT(m_pe, 14) ? m_lcd->data_r() : m_lcd->control_r()) << 5;
	return data;
}

// Endless pot: two wipers half a turn apart, each ramping over two thirds
// of a turn and reading 0 in the gap. The firmware (flash 0x2F070) reads
// them as 8 bits, 382 positions per turn, 64 parameter steps per turn.
u16 fs1r_state::knob_r(int knob, bool second) const
{
	int const pos = (m_knobs[knob]->read() * 382 / 64 + (second ? 350 : 159)) % 382;
	return pos <= 255 ? pos << 2 : 0;
}

void fs1r_state::pe_w(u16 data)
{
	if (BIT(m_pe, 13) && !BIT(data, 13) && !BIT(data, 15))
	{
		const u8 v = (data >> 5) & 0xff;
		if (BIT(data, 14))
			m_lcd->data_w(v);
		else
			m_lcd->control_w(v);
	}
	m_pe = data;
}

void fs1r_state::map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().region("flash", 0);
	map(0x200000, 0x3fffff).rom().region("eprom", 0);
	map(0x400000, 0x41ffff).ram().share("sram");

	map(0x800000, 0x8000ff).m(m_vop_effect, FUNC(yss236_device::map));
	map(0x800100, 0x800101).lw16(NAME([this] (u16 data) { m_lcd->set_leds(data & 0xff); }));
	map(0x800200, 0x8002ff).m(m_vop_filter, FUNC(yss236_device::map));

	map(0xc00000, 0xc003ff).m(m_ymp_sub, FUNC(ymp706_device::map));
	map(0xc00400, 0xc007ff).m(m_ymp_main, FUNC(ymp706_device::map));

	map(0x01000000, 0x0107ffff).ram();
}

void fs1r_state::fs1r(machine_config &config)
{
	SH7043A(config, m_maincpu, 7_MHz_XTAL * 4); // actually SH7044F
	m_maincpu->set_addrmap(AS_PROGRAM, &fs1r_state::map);
	m_maincpu->read_porta().set_constant(0); // YMP706 PBUSY on PA5/PA8, never busy
	m_maincpu->write_porta().set([this] (u32 data) { m_pa = data; });
	m_maincpu->write_portb().set([this] (u16 data) { m_pb = data; });
	m_maincpu->read_porte().set(FUNC(fs1r_state::pe_r));
	m_maincpu->write_porte().set(FUNC(fs1r_state::pe_w));
	m_maincpu->read_adc<0>().set_constant(0x200); // battery sense, a low reading shows "Battery Low!"
	m_maincpu->read_adc<3>().set([this] () { return knob_r(3, true); });
	m_maincpu->read_adc<4>().set([this] () { return knob_r(3, false); });
	m_maincpu->read_adc<5>().set([this] () { return knob_r(0, BIT(m_pe, 4)); });
	m_maincpu->read_adc<6>().set([this] () { return knob_r(1, BIT(m_pe, 4)); });
	m_maincpu->read_adc<7>().set([this] () { return knob_r(2, BIT(m_pe, 4)); });

	NVRAM(config, "sram", nvram_device::DEFAULT_ALL_1); // firmware initializes its settings over 0xff, not over 0

	FS1RLCD(config, m_lcd);

	YSS236(config, m_vop_filter, 24.576_MHz_XTAL);
	m_vop_filter->set_filter_chip(true);
	m_vop_filter->irq().set_inputline(m_maincpu, 0);
	YSS236(config, m_vop_effect, 24.576_MHz_XTAL);

	SPEAKER(config, "speaker", 2).front();

	YMP706(config, m_ymp_sub, 24.576_MHz_XTAL);
	m_ymp_sub->add_route(0, m_ymp_main, 1.0, 0);
	m_ymp_sub->add_route(1, m_ymp_main, 1.0, 1);

	YMP706(config, m_ymp_main, 24.576_MHz_XTAL);
	m_ymp_main->set_mix_input(true);
	m_ymp_main->add_route(0, "speaker", 1.0, 0);
	m_ymp_main->add_route(1, "speaker", 1.0, 1);

	MIDI_PORT(config, "mdin_a", midiin_slot, "midiin").rxd_handler().set(m_maincpu, FUNC(sh7043a_device::sci_rx_w<0>));
	MIDI_PORT(config, "mdin_b", midiin_slot, "midiin").rxd_handler().set(m_maincpu, FUNC(sh7043a_device::sci_rx_w<1>));

	auto &mdout(MIDI_PORT(config, "mdout", midiout_slot, "midiout"));
	m_maincpu->write_sci_tx<0>().set(mdout, FUNC(midi_port_device::write_txd));

	config.set_default_layout(layout_fs1r);
}

ROM_START( fs1r )
	ROM_REGION32_BE( 0x40000, "flash", 0 )
	ROM_LOAD( "fs1r_flash_v120.bin", 0x00000, 0x40000, CRC(6b24809c) SHA1(f80034df4634e63bd12a18b0b032f851a0e877fc) )

	ROM_REGION32_BE( 0x200000, "eprom", 0 )
	ROM_LOAD( "fs1r_v120.ic4", 0x000000, 0x200000, CRC(478d21a1) SHA1(78f7fc47eb19ae65c3027e78672a3b919f8ce124) )
ROM_END

} // anonymous namespace


SYST( 1998, fs1r, 0, 0, fs1r, fs1r, fs1r_state, empty_init, "Yamaha", "FS1R", MACHINE_IMPERFECT_SOUND )
