// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Sega Digital Sound Board 2

**************************************************************************************************/


#include "emu.h"
#include "dsb2.h"
#include "machine/clock.h"

#include <algorithm>

#define VERBOSE 0
#include "logmacro.h"

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
	m_lp_start(0),
	m_lp_end(0),
	m_start(0),
	m_end(0),
	m_mp_pos(0),
	m_audio_pos(0),
	m_audio_avail(0),
	m_command(mpeg_command_t::IDLE),
	m_player(mpeg_player_t::NOT_PLAYING)
{
}


void dsb2_device::dsb2_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom().region("mpegcpu", 0);
	map(0xc00000, 0xc00003).rw(m_uart, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0xd00001, 0xd00001).w(FUNC(dsb2_device::system_control_w));
//  map(0xd20001) flsbeats reads here
//  map(0xe00001) acknowledge FIFO writes?
	map(0xe00003, 0xe00003).w(FUNC(dsb2_device::fifo_w));
	// MPEG status
	map(0xe80001, 0xe80001).lr8(NAME([] () { return 0x01; }));
	map(0xf00000, 0xf1ffff).ram();
}

void dsb2_device::device_add_mconfig(machine_config &config)
{
	// TODO: unknown clocks
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

	m_timer_1kHz = timer_alloc(FUNC(dsb2_device::timer_irq_cb), this);

	save_item(NAME(m_mp_start));
	save_item(NAME(m_mp_end));
	save_item(NAME(m_mp_vol));
	save_item(NAME(m_mp_pan));
//  save_item(NAME(m_command));
//  save_item(NAME(m_player));
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
	m_command = IDLE;
	m_player = NOT_PLAYING;

	m_uart->write_cts(0);

	m_timer_1kHz->adjust(attotime::from_hz(1000), 0, attotime::from_hz(1000));
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

TIMER_CALLBACK_MEMBER(dsb2_device::timer_irq_cb)
{
	m_ourcpu->set_input_line(2, HOLD_LINE);
}

void dsb2_device::system_control_w(offs_t offset, u8 data)
{
	LOG("$d00001: write %02x\n", data);
}

void dsb2_device::fifo_w(offs_t offset, u8 data)
{
	switch(m_command)
	{
		case mpeg_command_t::START_ADDRESS_HI:
			m_start &= 0x00ffff;
			m_start |= (uint32_t)data << 16;
			m_command = mpeg_command_t::START_ADDRESS_MD;
			break;
		case mpeg_command_t::START_ADDRESS_MD:
			m_start &= 0xff00ff;
			m_start |= (uint32_t)data << 8;
			m_command = mpeg_command_t::START_ADDRESS_LO;
			break;
		case mpeg_command_t::START_ADDRESS_LO:
			m_start &= 0xffff00;
			m_start |= data;

			m_mp_start = m_start;
			m_command = mpeg_command_t::IDLE;
			break;

		case mpeg_command_t::END_ADDRESS_HI:
			m_end &= 0x00ffff;
			m_end |= (uint32_t)data << 16;
			m_command = mpeg_command_t::END_ADDRESS_MD;
			break;
		case mpeg_command_t::END_ADDRESS_MD:
			m_end &= 0xff00ff;
			m_end |= (uint32_t)data << 8;
			m_command = mpeg_command_t::END_ADDRESS_LO;
			break;
		case mpeg_command_t::END_ADDRESS_LO:
			m_end &= 0xffff00;
			m_end |= data;

			m_mp_end = m_end;
			m_command = mpeg_command_t::IDLE;
			break;

		case mpeg_command_t::IDLE:
		default:
		{
			if ((data & 0xfe) == 0x14)
				m_command = mpeg_command_t::START_ADDRESS_HI;
			if ((data & 0xfe) == 0x24)
				m_command = mpeg_command_t::END_ADDRESS_HI;
			if ((data & 0xfe) == 0x74)
			{
				m_mp_pos = m_mp_start * 8;
				m_player = mpeg_player_t::PLAYING;
			}

			if ((data & 0xfe) == 0x84)
			{
				m_player = mpeg_player_t::NOT_PLAYING;
				m_audio_pos = m_audio_avail = 0;
			}
		}
	}
}

void dsb2_device::sound_stream_update(sound_stream &stream)
{
	int samples = stream.samples();
	int sampindex = 0;
	for (;;)
	{
		while (samples && (m_audio_pos < m_audio_avail))
		{
			switch (m_mp_pan)
			{
				case 0: // stereo
					stream.put_int(0, sampindex, m_audio_buf[m_audio_pos*2] * m_mp_vol, 32768 * 128);
					stream.put_int(1, sampindex, m_audio_buf[m_audio_pos*2+1] * m_mp_vol, 32768 * 128);
					sampindex++;
					break;

				case 1: // left only
					stream.put_int(0, sampindex, m_audio_buf[m_audio_pos*2] * m_mp_vol, 32768 * 128);
					stream.put_int(1, sampindex, m_audio_buf[m_audio_pos*2] * m_mp_vol, 32768 * 128);
					sampindex++;
					break;

				case 2: // right only
					stream.put_int(0, sampindex, m_audio_buf[m_audio_pos*2+1] * m_mp_vol, 32768 * 128);
					stream.put_int(1, sampindex, m_audio_buf[m_audio_pos*2+1] * m_mp_vol, 32768 * 128);
					sampindex++;
					break;
			}
			m_audio_pos++;
			samples--;
		}

		if (!samples)
		{
			break;
		}

		if (m_player == mpeg_player_t::NOT_PLAYING)
		{
			break;
		}
		else
		{
			int sample_rate, channel_count;
			bool const ok = m_decoder->decode_buffer(m_mp_pos, m_mp_end * 8, m_audio_buf, m_audio_avail, sample_rate, channel_count);

			if (ok)
			{
				m_audio_pos = 0;
			}
			else
			{
				//if (m_mp_state == 2)
				//{
				//  if (m_mp_pos == m_lp_start * 8)
				//  {
				//      // We're looping on un-decodable crap, abort abort abort
				//      m_mp_state = 0;
				//  }
				//  m_mp_pos = m_lp_start * 8;
//
				//  if (m_lp_end)
				//  {
				//      m_mp_end = m_lp_end;
				//  }
				//}
				//else
				{
					m_player = mpeg_player_t::NOT_PLAYING;
				}
			}
		}
	}
}
