// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#include "emu.h"
#include "msx.h"
#include "msx_keyboard.h"
#include "msx_s1990.h"
#include "msx_systemflags.h"
#include "bus/midi/midi.h"
#include "bus/msx/slot/disk.h"
#include "bus/msx/slot/music.h"
#include "bus/msx/slot/panasonic08r.h"
#include "bus/msx/slot/ram_mm.h"
#include "bus/msx/slot/rom.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "softlist_dev.h"

#include "msx_turbor.lh"

using namespace msx_keyboard;


/***************************************************************************

  MSX Turbo-R machine drivers

The R800 and Z80 see exactly the same memory layouts, only one of the two CPUs
is active at any one time. The S1990 controls which CPU is active and can inject
wait states depending on which cpu is active.

TODO:
- sfg extensions may not work due to irq callbacks being registered with the Z80 only.
- v9958 commands seem to execute too fast. Several software items hang because of this.
- verify midi interface operation on fsa1gt.
- Implement DAC/PCM related hardware/filter and hook it up to the S1990.
- Microphone input.

***************************************************************************/

namespace {

class msxtr_state : public msx2p_base_state
{
public:
	msxtr_state(const machine_config &mconfig, device_type type, const char *tag)
		: msx2p_base_state(mconfig, type, tag, 21.477272_MHz_XTAL, 6)
		, m_r800(*this, "r800")
		, m_s1990(*this, "s1990")
		, m_pause_switch(*this, "PAUSE")
		, m_firmware_switch(*this, "FIRMWARE")
		, m_pause_led(*this, "pause_led")
		, m_r800_led(*this, "r800_led")
		, m_pcmdac(*this, "pcmdac")
	{
	}

	void fsa1st(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void turbor(ay8910_type ay8910_type, machine_config &config, const internal_layout &layout);
	void turbor_add_softlists(machine_config &config);
	void s1990_mem_map(address_map &map) ATTR_COLD;
	void s1990_io_map(address_map &map) ATTR_COLD;
	void cpu_mem_map(address_map &map) ATTR_COLD;
	void cpu_io_map(address_map &map) ATTR_COLD;

	virtual void setup_slot_spaces(msx_internal_slot_interface &device) override;
	virtual address_space& get_io_space() override;

	void pause_led_w(int state);
	void r800_led_w(int state);
	void pcm_dac_w(u8 data);
	void pcm_sample_hold_w(int state);
	void pcm_select_w(int state);
	void pcm_filter_w(int state);
	void muting_w(int state);
	int pcm_comp_r();

	required_device<r800_device> m_r800;
	required_device<msx_s1990_device> m_s1990;
	required_ioport m_pause_switch;
	required_ioport m_firmware_switch;
	output_finder<> m_pause_led;
	output_finder<> m_r800_led;
	required_device<dac_8bit_r2r_device> m_pcmdac;
	u8 m_pcm_last_sample;
	u8 m_pcm_held_sample;
	u8 m_pcm_sample_hold;
};

class fsa1gt_state : public msxtr_state
{
public:
	fsa1gt_state(const machine_config &mconfig, device_type type, const char *tag)
		: msxtr_state(mconfig, type, tag)
		, m_i8251(*this, "i8251")
		, m_i8254(*this, "i8254")
		, m_midiin(*this, "midiin_port")
		, m_midiout(*this, "midiout_port")
		, m_dtr(false)
		, m_rts(false)
		, m_rxrdy(false)
		, m_timer2_ff(false)
	{
	}

	void fsa1gt(machine_config &config);

private:
	required_device<i8251_device> m_i8251;
	required_device<pit8254_device> m_i8254;
	required_device<midi_port_device> m_midiin;
	required_device<midi_port_device> m_midiout;
	bool m_dtr;
	bool m_rts;
	bool m_rxrdy;
	bool m_timer2_ff;

	void s1990_io_map(address_map &map) ATTR_COLD;
	void dtr_w(int state);
	void rts_w(int state);
	void rxrdy_w(int state);
	void timer0_w(int state);
	void timer2_w(int state);
	void update_midi_int_state();
	void clear_timer2_ff(u8 data);
};


address_space& msxtr_state::get_io_space()
{
	return m_s1990->space(AS_IO);
}

void msxtr_state::setup_slot_spaces(msx_internal_slot_interface &device)
{
	device.set_memory_space(m_s1990, AS_PROGRAM);
	device.set_io_space(m_s1990, AS_IO);
	// TODO: This is used for signalling irq vectors. But that should go to the currently active cpu not just the z80.
	device.set_maincpu(m_maincpu);
}

void msxtr_state::machine_start()
{
	msx2p_base_state::machine_start();
	m_pause_led.resolve();
	m_r800_led.resolve();

	save_item(NAME(m_pcm_last_sample));
	save_item(NAME(m_pcm_held_sample));
	save_item(NAME(m_pcm_sample_hold));
}

void msxtr_state::machine_reset()
{
	msx2p_base_state::machine_reset();
	m_pause_led = 0;
	m_r800_led = 0;
}

void msxtr_state::s1990_mem_map(address_map &map)
{
	memory_map(map);
}

void msxtr_state::s1990_io_map(address_map &map)
{
	msx2plus_io_map(map);
	map(0xa4, 0xa4).rw(m_s1990, FUNC(msx_s1990_device::pmcnt), FUNC(msx_s1990_device::pmdac));
	map(0xa5, 0xa5).rw(m_s1990, FUNC(msx_s1990_device::pmstat), FUNC(msx_s1990_device::pmcntl));
	map(0xa7, 0xa7).portr(m_pause_switch.finder_tag());
	map(0xa7, 0xa7).w(m_s1990, FUNC(msx_s1990_device::pause_w));
	map(0xe4, 0xe4).w(m_s1990, FUNC(msx_s1990_device::reg_index_write));
	map(0xe5, 0xe5).rw(m_s1990, FUNC(msx_s1990_device::regs_read), FUNC(msx_s1990_device::regs_write));
	map(0xe6, 0xe6).w(m_s1990, FUNC(msx_s1990_device::counter_write));
	map(0xe6, 0xe7).r(m_s1990, FUNC(msx_s1990_device::counter_read));
}

void msxtr_state::cpu_mem_map(address_map &map)
{
	map(0x0000, 0xffff).rw(m_s1990, FUNC(msx_s1990_device::mem_read), FUNC(msx_s1990_device::mem_write));
}

void msxtr_state::cpu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xff).rw(m_s1990, FUNC(msx_s1990_device::io_read), FUNC(msx_s1990_device::io_write));
}

void msxtr_state::pause_led_w(int state)
{
	m_pause_led = state;
}

void msxtr_state::r800_led_w(int state)
{
	m_r800_led = state;
}

void msxtr_state::turbor(ay8910_type ay8910_type, machine_config &config, const internal_layout &layout)
{
	msx2plus_base(ay8910_type, config, layout);

	m_maincpu->set_addrmap(AS_PROGRAM, &msxtr_state::cpu_mem_map);
	m_maincpu->set_addrmap(AS_IO, &msxtr_state::cpu_io_map);

	R800(config, m_r800, 28.636363_MHz_XTAL);
	m_r800->set_addrmap(AS_PROGRAM, &msxtr_state::cpu_mem_map);
	m_r800->set_addrmap(AS_IO, &msxtr_state::cpu_io_map);

	MSX_S1990(config, m_s1990, 28.636363_MHz_XTAL / 4);
	m_s1990->set_addrmap(AS_PROGRAM, &msxtr_state::s1990_mem_map);
	m_s1990->set_addrmap(AS_IO, &msxtr_state::s1990_io_map);
	m_s1990->set_z80_tag(m_maincpu);
	m_s1990->set_r800_tag(m_r800);
	m_s1990->pause_led_callback().set(FUNC(msxtr_state::pause_led_w));
	m_s1990->r800_led_callback().set(FUNC(msxtr_state::r800_led_w));
	m_s1990->firmware_switch_callback().set_ioport(m_firmware_switch);
	m_s1990->dac_write_callback().set(FUNC(msxtr_state::pcm_dac_w));
	m_s1990->sample_hold_callback().set(FUNC(msxtr_state::pcm_sample_hold_w));
	m_s1990->select_callback().set(FUNC(msxtr_state::pcm_select_w));
	m_s1990->filter_callback().set(FUNC(msxtr_state::pcm_filter_w));
	m_s1990->muting_callback().set(FUNC(msxtr_state::muting_w));
	m_s1990->comp_callback().set(FUNC(msxtr_state::pcm_comp_r));

	// TODO: Do IRQ requests go to both cpus (only to be ignored by the inactive cpu) or only to the currently active cpu?
	m_mainirq->output_handler().append_inputline(m_r800, INPUT_LINE_IRQ0);

	DAC_8BIT_R2R(config, m_pcmdac).add_route(ALL_OUTPUTS, m_speaker, 1.0); // Unknown DAC type

	// Software lists
	turbor_add_softlists(config);
}

void msxtr_state::pcm_dac_w(u8 data)
{
	m_pcmdac->write(data);
	m_pcm_last_sample = data;
}

void msxtr_state::pcm_sample_hold_w(int state)
{
	if (!m_pcm_sample_hold && state)
		m_pcm_held_sample = m_pcm_last_sample;
	m_pcm_sample_hold = state;
}

void msxtr_state::pcm_select_w(int state)
{
	// TODO pcm select
}

void msxtr_state::pcm_filter_w(int state)
{
	// TODO enable pcm filter
}

int msxtr_state::pcm_comp_r()
{
	// TODO pcm output compare
	return 0;
}

void msxtr_state::muting_w(int state)
{
	// TODO mute all sound
}

void msxtr_state::turbor_add_softlists(machine_config &config)
{
	if (m_hw_def.has_cartslot())
	{
		SOFTWARE_LIST(config, "cart_list").set_original("msxr_cart");
		SOFTWARE_LIST(config, "msx2p_cart_list").set_compatible("msx2p_cart");
		SOFTWARE_LIST(config, "msx2_cart_list").set_compatible("msx2_cart");
		SOFTWARE_LIST(config, "msx1_cart_list").set_compatible("msx1_cart");
	}

	if (m_hw_def.has_fdc())
	{
		SOFTWARE_LIST(config, "flop_list").set_original("msxr_flop");
		SOFTWARE_LIST(config, "msx2p_flop_list").set_compatible("msx2p_flop");
		SOFTWARE_LIST(config, "msx2_flop_list").set_compatible("msx2_flop");
		SOFTWARE_LIST(config, "msx1_flop_list").set_compatible("msx1_flop");
	}
}

/* MSX Turbo-R - Panasonic FS-A1GT */

ROM_START(fsa1gt)
	ROM_REGION(0x400000, "firmware", 0)
	// This should be 2 1MB roms at locations IC20 and IC23.
	ROM_LOAD("a1gtfirm.rom", 0, 0x400000, CRC(feefeadc) SHA1(e779c338eb91a7dea3ff75f3fde76b8af22c4a3a) BAD_DUMP)

	ROM_REGION(0x40000, "kanji", 0)
	ROM_LOAD("a1gtkfn.rom", 0, 0x40000, CRC(1f6406fb) SHA1(5aff2d9b6efc723bc395b0f96f0adfa83cc54a49))
ROM_END

void fsa1gt_state::s1990_io_map(address_map &map)
{
	msxtr_state::s1990_io_map(map);
	map(0xe8, 0xe9).rw(m_i8251, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xea, 0xea).w(FUNC(fsa1gt_state::clear_timer2_ff));
	map(0xec, 0xef).rw(m_i8254, FUNC(pit8254_device::read), FUNC(pit8254_device::write));
}

void fsa1gt_state::fsa1gt(machine_config &config)
{
	// AY8910 (in T9769)
	// FDC: tc8566af, 1 3.5" DSDD drive
	// 2 Cartridge slots
	// T9769C + S1990
	// FM built-in
	// Microphone
	// MIDI-in
	// MIDI-out
	// firmware switch
	// pause button
	// ren-sha turbo slider

	add_internal_slot(config, MSX_SLOT_ROM, "bios", 0, 0, 0, 2, "firmware", 0x050000);
	add_internal_slot(config, MSX_SLOT_MUSIC, "msxmusic", 0, 2, 1, 1, "firmware", 0x07c000).set_ym2413_tag(m_ym2413);
	add_internal_slot(config, MSX_SLOT_ROM, "opt", 0, 3, 1, 1, "firmware", 0x048000);
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x80000);   // 512KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "ext", 3, 1, 0, 1, "firmware", 0x070000);
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "firmware", 0x074000);
	add_internal_disk(config, MSX_SLOT_DISK4_TC8566, "dos", 3, 2, 1, 3, "firmware", 0x060000);
	add_internal_slot(config, MSX_SLOT_PANASONIC08R, "firmware", 3, 3, 0, 4, "firmware").set_sram_size(0x8000).set_mm_tag("ram_mm");

	MSX_SYSTEMFLAGS(config, "sysflags", m_maincpu, 0x00);

	msx_ym2413(config);

	m_hw_def.has_cassette(false);
	turbor(SND_AY8910, config, layout_msx_turbor);
	m_s1990->set_addrmap(AS_IO, &fsa1gt_state::s1990_io_map);

	I8251(config, m_i8251, 16_MHz_XTAL / 4); // Not sure about this
	m_i8251->txd_handler().set(m_midiout, FUNC(midi_port_device::write_txd));
	m_i8251->dtr_handler().set(*this, FUNC(fsa1gt_state::dtr_w));
	m_i8251->rts_handler().set(*this, FUNC(fsa1gt_state::rts_w));
	m_i8251->rxrdy_handler().set(*this, FUNC(fsa1gt_state::rxrdy_w));

	PIT8254(config, m_i8254);
	m_i8254->set_clk<0>(16_MHz_XTAL / 4);
	m_i8254->set_clk<2>(16_MHz_XTAL/ 4);
	m_i8254->out_handler<0>().set(*this, FUNC(fsa1gt_state::timer0_w));
	m_i8254->out_handler<2>().set(*this, FUNC(fsa1gt_state::timer2_w));

	MIDI_PORT(config, m_midiin, midiin_slot, "midiin").rxd_handler().set(m_i8251, FUNC(i8251_device::rx_w));
	MIDI_PORT(config, m_midiout, midiout_slot, "midiout");
}

void fsa1gt_state::rts_w(int state)
{
	m_rts = state;
	update_midi_int_state();
}

void fsa1gt_state::rxrdy_w(int state)
{
	m_rxrdy = state;
	update_midi_int_state();
}

void fsa1gt_state::dtr_w(int state)
{
	m_dtr = state;
	update_midi_int_state();
}

void fsa1gt_state::timer0_w(int state)
{
	m_i8251->tx_clock_w(state);
	m_i8251->rx_clock_w(state);
}

void fsa1gt_state::timer2_w(int state)
{
	m_timer2_ff = m_timer2_ff | state;
	m_i8254->write_clk1(state);
	update_midi_int_state();
}

void fsa1gt_state::clear_timer2_ff(u8 data)
{
	m_timer2_ff = false;
	update_midi_int_state();
}

void fsa1gt_state::update_midi_int_state()
{
	m_i8251->write_dsr(m_timer2_ff && m_dtr);
	m_mainirq->in_w<3>(!((m_timer2_ff && m_dtr) || (m_rts && m_rxrdy)));
}

/* MSX Turbo-R - Panasonic FS-A1ST */

ROM_START(fsa1st)
	ROM_REGION(0x400000, "firmware", 0)
	// This should be either 3 512KB roms or 1 1MB and 1 512KB rom (at IC12 and IC18?). Both variants are known to exist.
	ROM_LOAD("a1stfirm.rom", 0, 0x400000, CRC(139ac99c) SHA1(c212b11fda13f83dafed688c54d098e7e47ab225) BAD_DUMP)

	ROM_REGION(0x40000, "kanji", 0)
	ROM_LOAD("a1stkfn.rom", 0, 0x40000, CRC(1f6406fb) SHA1(5aff2d9b6efc723bc395b0f96f0adfa83cc54a49))
ROM_END

void msxtr_state::fsa1st(machine_config &config)
{
	// AY8910 (in T9769)
	// FDC: tc8566af, 1 3.5" DSDD drive
	// T9769C + S1990
	// 2 Cartridge slots
	// FM built-in
	// microphone
	// firmware switch
	// pause button
	// ren-sha turbo slider

	add_internal_slot(config, MSX_SLOT_ROM, "bios", 0, 0, 0, 2, "firmware", 0x050000);
	add_internal_slot(config, MSX_SLOT_MUSIC, "msxmusic", 0, 2, 1, 1, "firmware", 0x07c000).set_ym2413_tag(m_ym2413);
	add_internal_slot(config, MSX_SLOT_ROM, "opt", 0, 3, 1, 1, "firmware", 0x048000);
	add_cartridge_slot<1>(config, 1);
	add_cartridge_slot<2>(config, 2);
	add_internal_slot(config, MSX_SLOT_RAM_MM, "ram_mm", 3, 0, 0, 4).set_total_size(0x40000);   // 256KB Mapper RAM
	add_internal_slot(config, MSX_SLOT_ROM, "ext", 3, 1, 0, 1, "firmware", 0x070000);
	add_internal_slot(config, MSX_SLOT_ROM, "kdr", 3, 1, 1, 2, "firmware", 0x074000);
	add_internal_disk(config, MSX_SLOT_DISK4_TC8566, "dos", 3, 2, 1, 3, "firmware", 0x060000);
	add_internal_slot(config, MSX_SLOT_PANASONIC08R, "firmware", 3, 3, 0, 4, "firmware").set_sram_size(0x4000).set_mm_tag("ram_mm");

	MSX_SYSTEMFLAGS(config, "sysflags", m_maincpu, 0x00);

	msx_ym2413(config);

	m_hw_def.has_cassette(false);
	turbor(SND_AY8910, config, layout_msx_turbor);
}

INPUT_PORTS_START(msxtr)
	PORT_INCLUDE(msx2jp)

	PORT_START("PAUSE")
	PORT_CONFNAME(0x01, 0x00, "Pause") PORT_CHANGED_MEMBER("s1990", msx_s1990_device, pause_callback, 0)
	PORT_CONFSETTING(0x00, "off")
	PORT_CONFSETTING(0x01, "on")

	PORT_START("FIRMWARE")
	PORT_CONFNAME(0x01, 0x01, "Firmware")
	PORT_CONFSETTING(0x01, "enabled")
	PORT_CONFSETTING(0x00, "disabled")
INPUT_PORTS_END

} // anonymous namespace

/* MSX Turbo-R */
COMP(1991, fsa1gt, 0, 0, fsa1gt, msxtr, fsa1gt_state, empty_init, "Panasonic", "FS-A1GT (MSX Turbo-R, Japan)", MACHINE_NOT_WORKING)
COMP(1991, fsa1st, 0, 0, fsa1st, msxtr, msxtr_state,  empty_init, "Panasonic", "FS-A1ST (MSX Turbo-R, Japan)", MACHINE_NOT_WORKING)
