// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "sound/samples.h"
#include "galaga.h"
#include "xevious.h"

/***************************************************************************

 BATTLES CPU4(custum I/O Emulation) I/O Handlers

***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(battles_state::nmi_generate)
{
	m_customio_prev_command = m_customio_command;

	if( m_customio_command & 0x10 )
	{
		if( m_customio_command_count == 0 )
		{
			m_subcpu3->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
		}
		else
		{
			m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
			m_subcpu3->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
		}
	}
	else
	{
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
		m_subcpu3->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
	m_customio_command_count++;
}


uint8_t battles_state::customio0_r()
{
	logerror("%s: custom I/O Read = %02x\n", machine().describe_context(), m_customio_command);
	return m_customio_command;
}

uint8_t battles_state::customio3_r()
{
	int return_data;

	if( m_subcpu3->pc() == 0xAE ){
		/* CPU4 0xAA - 0xB9 : waiting for MB8851 ? */
		return_data =   ( (m_customio_command & 0x10) << 3)
						| 0x00
						| (m_customio_command & 0x0f);
	}else{
		return_data =   ( (m_customio_prev_command & 0x10) << 3)
						| 0x60
						| (m_customio_prev_command & 0x0f);
	}
	logerror("%s: custom I/O Read = %02x\n", machine().describe_context(), return_data);

	return return_data;
}


void battles_state::customio0_w(uint8_t data)
{
	logerror("%s: custom I/O Write = %02x\n", machine().describe_context(), data);

	m_customio_command = data;
	m_customio_command_count = 0;

	switch (data)
	{
		case 0x10:
			m_nmi_timer->reset();
			return; /* nop */
	}
	m_nmi_timer->adjust(attotime::from_usec(166), 0, attotime::from_usec(166));

}

void battles_state::customio3_w(uint8_t data)
{
	logerror("%s: custom I/O Write = %02x\n", machine().describe_context(), data);

	m_customio_command = data;
}



uint8_t battles_state::customio_data0_r(offs_t offset)
{
	logerror("%s: custom I/O parameter %02x Read = %02x\n", machine().describe_context(), offset, m_customio_data);

	return m_customio_data;
}

uint8_t battles_state::customio_data3_r(offs_t offset)
{
	logerror("%s: custom I/O parameter %02x Read = %02x\n", machine().describe_context(), offset, m_customio_data);
	return m_customio_data;
}


void battles_state::customio_data0_w(offs_t offset, uint8_t data)
{
	logerror("%s: custom I/O parameter %02x Write = %02x\n", machine().describe_context(), offset, data);
	m_customio_data = data;
}

void battles_state::customio_data3_w(offs_t offset, uint8_t data)
{
	logerror("%s: custom I/O parameter %02x Write = %02x\n", machine().describe_context(), offset, data);
	m_customio_data = data;
}


void battles_state::cpu4_coin_w(uint8_t data)
{
	m_leds[0] = BIT(data, 1); // Start 1
	m_leds[1] = BIT(data, 0); // Start 2

	machine().bookkeeping().coin_counter_w(0,data & 0x20);
	machine().bookkeeping().coin_counter_w(1,data & 0x10);
	machine().bookkeeping().coin_lockout_global_w(~data & 0x04);
}


void battles_state::noise_sound_w(offs_t offset, uint8_t data)
{
	logerror("%s: 50%02x Write = %02x\n", machine().describe_context(), offset, data);
	if( (m_sound_played == 0) && (data == 0xFF) ){
		if( m_customio[0] == 0x40 ){
			m_samples->start(0, 0);
		}
		else{
			m_samples->start(0, 1);
		}
	}
	m_sound_played = data;
}


uint8_t battles_state::input_port_r(offs_t offset)
{
	switch ( offset )
	{
		default:
		case 0: return ~bitswap<8>(ioport("IN1")->read(),2,3,1,0,6,7,5,4);
		case 1: return ~ioport("IN0")->read() & 15;
		case 2: return ~ioport("IN0")->read() >> 4;
		case 3: return ~ioport("IN1")->read() & 15;
	}
}


WRITE_LINE_MEMBER(battles_state::interrupt_4)
{
	if (state)
		m_subcpu3->set_input_line(0, HOLD_LINE);
}
