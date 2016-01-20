// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "sound/samples.h"
#include "includes/galaga.h"

/***************************************************************************

 BATTLES CPU4(custum I/O Emulation) I/O Handlers

***************************************************************************/

void xevious_state::battles_customio_init()
{
	m_battles_customio_command = 0;
	m_battles_customio_prev_command = 0;
	m_battles_customio_command_count = 0;
	m_battles_customio_data = 0;
	m_battles_sound_played = 0;
}


TIMER_DEVICE_CALLBACK_MEMBER(xevious_state::battles_nmi_generate)
{
	m_battles_customio_prev_command = m_battles_customio_command;

	if( m_battles_customio_command & 0x10 )
	{
		if( m_battles_customio_command_count == 0 )
		{
			m_subcpu3->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
		}
		else
		{
			m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
			m_subcpu3->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
		}
	}
	else
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
		m_subcpu3->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
	m_battles_customio_command_count++;
}


READ8_MEMBER( xevious_state::battles_customio0_r )
{
	logerror("CPU0 %04x: custom I/O Read = %02x\n",space.device().safe_pc(),m_battles_customio_command);
	return m_battles_customio_command;
}

READ8_MEMBER( xevious_state::battles_customio3_r )
{
	int return_data;

	if( space.device().safe_pc() == 0xAE ){
		/* CPU4 0xAA - 0xB9 : waiting for MB8851 ? */
		return_data =   ( (m_battles_customio_command & 0x10) << 3)
						| 0x00
						| (m_battles_customio_command & 0x0f);
	}else{
		return_data =   ( (m_battles_customio_prev_command & 0x10) << 3)
						| 0x60
						| (m_battles_customio_prev_command & 0x0f);
	}
	logerror("CPU3 %04x: custom I/O Read = %02x\n",space.device().safe_pc(),return_data);

	return return_data;
}


WRITE8_MEMBER( xevious_state::battles_customio0_w )
{
	timer_device *timer = machine().device<timer_device>("battles_nmi");

	logerror("CPU0 %04x: custom I/O Write = %02x\n",space.device().safe_pc(),data);

	m_battles_customio_command = data;
	m_battles_customio_command_count = 0;

	switch (data)
	{
		case 0x10:
			timer->reset();
			return; /* nop */
	}
	timer->adjust(attotime::from_usec(166), 0, attotime::from_usec(166));

}

WRITE8_MEMBER( xevious_state::battles_customio3_w )
{
	logerror("CPU3 %04x: custom I/O Write = %02x\n",space.device().safe_pc(),data);

	m_battles_customio_command = data;
}



READ8_MEMBER( xevious_state::battles_customio_data0_r )
{
	logerror("CPU0 %04x: custom I/O parameter %02x Read = %02x\n",space.device().safe_pc(),offset,m_battles_customio_data);

	return m_battles_customio_data;
}

READ8_MEMBER( xevious_state::battles_customio_data3_r )
{
	logerror("CPU3 %04x: custom I/O parameter %02x Read = %02x\n",space.device().safe_pc(),offset,m_battles_customio_data);
	return m_battles_customio_data;
}


WRITE8_MEMBER( xevious_state::battles_customio_data0_w )
{
	logerror("CPU0 %04x: custom I/O parameter %02x Write = %02x\n",space.device().safe_pc(),offset,data);
	m_battles_customio_data = data;
}

WRITE8_MEMBER( xevious_state::battles_customio_data3_w )
{
	logerror("CPU3 %04x: custom I/O parameter %02x Write = %02x\n",space.device().safe_pc(),offset,data);
	m_battles_customio_data = data;
}


WRITE8_MEMBER( xevious_state::battles_CPU4_coin_w )
{
	output().set_led_value(0,data & 0x02); // Start 1
	output().set_led_value(1,data & 0x01); // Start 2

	machine().bookkeeping().coin_counter_w(0,data & 0x20);
	machine().bookkeeping().coin_counter_w(1,data & 0x10);
	machine().bookkeeping().coin_lockout_global_w(~data & 0x04);
}


WRITE8_MEMBER( xevious_state::battles_noise_sound_w )
{
	logerror("CPU3 %04x: 50%02x Write = %02x\n",space.device().safe_pc(),offset,data);
	if( (m_battles_sound_played == 0) && (data == 0xFF) ){
		if( m_customio[0] == 0x40 ){
			m_samples->start(0, 0);
		}
		else{
			m_samples->start(0, 1);
		}
	}
	m_battles_sound_played = data;
}


READ8_MEMBER( xevious_state::battles_input_port_r )
{
	switch ( offset )
	{
		default:
		case 0: return ~BITSWAP8(ioport("IN0H")->read(),7,6,5,4,2,3,1,0);
		case 1: return ~ioport("IN1L")->read();
		case 2: return ~ioport("IN1H")->read();
		case 3: return ~ioport("IN0L")->read();
	}
}


INTERRUPT_GEN_MEMBER(xevious_state::battles_interrupt_4)
{
	device.execute().set_input_line(0, HOLD_LINE);
}
