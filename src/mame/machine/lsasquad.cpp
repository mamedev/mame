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

READ8_MEMBER(lsasquad_state::lsasquad_mcu_status_r)
{
	int res = ioport("MCU")->read();

	/* bit 0 = when 1, mcu is ready to receive data from main cpu */
	/* bit 1 = when 0, mcu has sent data to the main cpu */
	//logerror("%04x: mcu_status_r\n",space.device().safe_pc());
	if (m_bmcu)
	{
		if (CLEAR_LINE == m_bmcu->host_semaphore_r())
			res |= 0x01;
		if (CLEAR_LINE == m_bmcu->mcu_semaphore_r())
			res |= 0x02;
	}

	return res;
}

READ8_MEMBER(lsasquad_state::daikaiju_mcu_status_r)
{
	int res = ioport("MCU")->read();

	/* bit 0 = when 1, mcu is ready to receive data from main cpu */
	/* bit 1 = when 0, mcu has sent data to the main cpu */
	//logerror("%04x: mcu_status_r\n",space.device().safe_pc());
	if (m_bmcu)
	{
		if (CLEAR_LINE == m_bmcu->host_semaphore_r())
			res |= 0x01;
		if (CLEAR_LINE == m_bmcu->mcu_semaphore_r())
			res |= 0x02;
	}

	res |= ((m_sound_pending & 0x02) ^ 2) << 3; //inverted flag
	m_sound_pending &= ~0x02;
	return res;
}
