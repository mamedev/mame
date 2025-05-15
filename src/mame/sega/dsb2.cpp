// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

  Sega Z80 Digital Sound Board

  used for Model 1/2/3

***************************************************************************/


#include "emu.h"
#include "dsb2.h"
#include "machine/clock.h"

#include <algorithm>

DEFINE_DEVICE_TYPE(DSB2, dsb2_device, "dsb2_device", "Sega 68k-based Digital Sound Board 2")

dsb2_device::dsb2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, DSB2, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_ourcpu(*this, "mpegcpu"),
	m_uart(*this, "uart"),
	m_mpeg_rom(*this, "mpeg"),
	m_rxd_handler(*this),
	m_mp_start(0),
	m_mp_end(0),
	m_mp_vol(0x7f),
	m_mp_pan(0),
	m_mp_state(0),
	m_lp_start(0),
	m_lp_end(0),
	m_start(0),
	m_end(0),
	m_mp_pos(0),
	m_audio_pos(0),
	m_audio_avail(0)
{
}


void dsb2_device::dsb2_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom().region("mpegcpu", 0);
	map(0xc00000, 0xc00003).rw(m_uart, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
//	map(0xd00001) system control?
//	map(0xd20001) flsbeats reads here
//	map(0xe00001) acknowledge FIFO writes?
//	map(0xe00003) MPEG FIFO writes
	// MPEG status
	map(0xe80001, 0xe80001).lr8(NAME([this] () { return 0x01; }));
	map(0xf00000, 0xf1ffff).ram();
}

void dsb2_device::device_add_mconfig(machine_config &config)
{
	// TODO: unknown clocks
	// TODO: 1 kHz timer for irq2
	M68000(config, m_ourcpu, 8000000);
	m_ourcpu->set_addrmap(AS_PROGRAM, &dsb2_device::dsb2_map);

	I8251(config, m_uart, 4000000);
	m_uart->rxrdy_handler().set_inputline(m_ourcpu, INPUT_LINE_IRQ1);
	m_uart->txd_handler().set(FUNC(dsb2_device::output_txd));

	clock_device &uart_clock(CLOCK(config, "uart_clock", 500000)); // 16 times 31.25MHz (standard Sega/MIDI sound data rate)
	uart_clock.signal_handler().set("uart", FUNC(i8251_device::write_rxc));
	uart_clock.signal_handler().append("uart", FUNC(i8251_device::write_txc));
}


void dsb2_device::device_start()
{
	m_decoder.reset(new mpeg_audio(&m_mpeg_rom[0], mpeg_audio::L2, false, 0));
	stream_alloc(0, 2, 32000);

	save_item(NAME(m_mp_start));
	save_item(NAME(m_mp_end));
	save_item(NAME(m_mp_vol));
	save_item(NAME(m_mp_pan));
	save_item(NAME(m_mp_state));
	save_item(NAME(m_lp_start));
	save_item(NAME(m_lp_end));
	save_item(NAME(m_start));
	save_item(NAME(m_end));
	save_item(NAME(m_mp_pos));
	save_item(NAME(m_audio_pos));
	save_item(NAME(m_audio_avail));
}

void dsb2_device::device_reset()
{
	m_start = m_end = 0;
	m_audio_pos = m_audio_avail = 0;
	std::fill(std::begin(m_audio_buf), std::end(m_audio_buf), 0);
	m_mp_vol = 0x7f;
	m_mp_state = 0;

	m_uart->write_cts(0);
}


void dsb2_device::device_stop()
{
	m_decoder.reset();
}

void dsb2_device::write_txd(int state)
{
	m_uart->write_rxd(state);
}

void dsb2_device::output_txd(int state)
{
	m_rxd_handler(state);
}

void dsb2_device::sound_stream_update(sound_stream &stream)
{
	//int samples = stream.samples();
	//int sampindex = 0;

	// ...
}
