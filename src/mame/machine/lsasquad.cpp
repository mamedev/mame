// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/lsasquad.h"

/***************************************************************************

 main <-> sound CPU communication

***************************************************************************/

TIMER_CALLBACK_MEMBER(lsasquad_state::nmi_callback)
{
	if (m_sound_nmi_enable)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	else
		m_pending_nmi = 1;
}

WRITE8_MEMBER(lsasquad_state::lsasquad_sh_nmi_disable_w)
{
	m_sound_nmi_enable = 0;
}

WRITE8_MEMBER(lsasquad_state::lsasquad_sh_nmi_enable_w)
{
	m_sound_nmi_enable = 1;
	if (m_pending_nmi)
	{
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
		m_pending_nmi = 0;
	}
}

WRITE8_MEMBER(lsasquad_state::lsasquad_sound_command_w)
{
	m_sound_pending |= 0x01;
	m_sound_cmd = data;

	//logerror("%04x: sound cmd %02x\n", space.device().safe_pc(), data);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(lsasquad_state::nmi_callback),this), data);
}

READ8_MEMBER(lsasquad_state::lsasquad_sh_sound_command_r)
{
	m_sound_pending &= ~0x01;
	//logerror("%04x: read sound cmd %02x\n", space.device().safe_pc(), m_sound_cmd);
	return m_sound_cmd;
}

WRITE8_MEMBER(lsasquad_state::lsasquad_sh_result_w)
{
	m_sound_pending |= 0x02;
	//logerror("%04x: sound res %02x\n", space.device().safe_pc(), data);
	m_sound_result = data;
}

READ8_MEMBER(lsasquad_state::lsasquad_sound_result_r)
{
	m_sound_pending &= ~0x02;
	//logerror("%04x: read sound res %02x\n", space.device().safe_pc(), m_sound_result);
	return m_sound_result;
}

READ8_MEMBER(lsasquad_state::lsasquad_sound_status_r)
{
	/* bit 0: message pending for sound cpu */
	/* bit 1: message pending for main cpu */
	return m_sound_pending;
}


READ8_MEMBER(lsasquad_state::daikaiju_sh_sound_command_r)
{
	m_sound_pending &= ~0x01;
	m_sound_pending |= 0x02;
	//logerror("%04x: read sound cmd %02x\n", space.device().safe_pc(), m_sound_cmd);
	return m_sound_cmd;
}

READ8_MEMBER(lsasquad_state::daikaiju_sound_status_r)
{
	/* bit 0: message pending for sound cpu */
	/* bit 1: message pending for main cpu */
	return m_sound_pending ^ 3;
}


/***************************************************************************

 LSA Squad 68705 protection interface

 The following is ENTIRELY GUESSWORK!!!

***************************************************************************/

READ8_MEMBER(lsasquad_state::lsasquad_68705_port_a_r)
{
	//logerror("%04x: 68705 port A read %02x\n", space.device().safe_pc(), m_port_a_in);
	return (m_port_a_out & m_ddr_a) | (m_port_a_in & ~m_ddr_a);
}

WRITE8_MEMBER(lsasquad_state::lsasquad_68705_port_a_w)
{
	//logerror("%04x: 68705 port A write %02x\n", space.device().safe_pc(), data);
	m_port_a_out = data;
}

WRITE8_MEMBER(lsasquad_state::lsasquad_68705_ddr_a_w)
{
	m_ddr_a = data;
}



/*
 *  Port B connections:
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  1   W  when 1->0, enables latch which brings the command from main CPU (read from port A)
 *  2   W  when 0->1, copies port A to the latch for the main CPU
 */

READ8_MEMBER(lsasquad_state::lsasquad_68705_port_b_r)
{
	return (m_port_b_out & m_ddr_b) | (m_port_b_in & ~m_ddr_b);
}

WRITE8_MEMBER(lsasquad_state::lsasquad_68705_port_b_w)
{
	//logerror("%04x: 68705 port B write %02x\n", space.device().safe_pc(), data);

	if ((m_ddr_b & 0x02) && (~data & 0x02) && (m_port_b_out & 0x02))
	{
		m_port_a_in = m_from_main;
		if (m_main_sent)
			m_mcu->set_input_line(0, CLEAR_LINE);
		m_main_sent = 0;
		//logerror("read command %02x from main cpu\n", m_port_a_in);
	}

	if ((m_ddr_b & 0x04) && (data & 0x04) && (~m_port_b_out & 0x04))
	{
		//logerror("send command %02x to main cpu\n", m_port_a_out);
		m_from_mcu = m_port_a_out;
		m_mcu_sent = 1;
	}

	m_port_b_out = data;
}

WRITE8_MEMBER(lsasquad_state::lsasquad_68705_ddr_b_w)
{
	m_ddr_b = data;
}

WRITE8_MEMBER(lsasquad_state::lsasquad_mcu_w)
{
	//logerror("%04x: mcu_w %02x\n", space.device().safe_pc(), data);
	m_from_main = data;
	m_main_sent = 1;
	m_mcu->set_input_line(0, ASSERT_LINE);
}

READ8_MEMBER(lsasquad_state::lsasquad_mcu_r)
{
	//logerror("%04x: mcu_r %02x\n", space.device().safe_pc(), m_from_mcu);
	m_mcu_sent = 0;
	return m_from_mcu;
}

READ8_MEMBER(lsasquad_state::lsasquad_mcu_status_r)
{
	int res = ioport("MCU")->read();

	/* bit 0 = when 1, mcu is ready to receive data from main cpu */
	/* bit 1 = when 0, mcu has sent data to the main cpu */
	//logerror("%04x: mcu_status_r\n",space.device().safe_pc());
	if (!m_main_sent)
		res |= 0x01;
	if (!m_mcu_sent)
		res |= 0x02;

	return res;
}

READ8_MEMBER(lsasquad_state::daikaiju_mcu_status_r)
{
	int res = ioport("MCU")->read();

	/* bit 0 = when 1, mcu is ready to receive data from main cpu */
	/* bit 1 = when 0, mcu has sent data to the main cpu */
	//logerror("%04x: mcu_status_r\n",space.device().safe_pc());
	if (!m_main_sent)
		res |= 0x01;
	if (!m_mcu_sent)
		res |= 0x02;

	res |= ((m_sound_pending & 0x02) ^ 2) << 3; //inverted flag
	m_sound_pending &= ~0x02;
	return res;
}
