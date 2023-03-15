// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

#include "emu.h"
#include "cpu/z80/z80.h"
#include "lsasquad.h"

/***************************************************************************

 main <-> sound CPU communication

***************************************************************************/

void lsasquad_state::sh_nmi_disable_w(uint8_t data)
{
	m_soundnmi->in_w<1>(0);
}

void lsasquad_state::sh_nmi_enable_w(uint8_t data)
{
	m_soundnmi->in_w<1>(1);
}

uint8_t lsasquad_state::lsasquad_sound_status_r()
{
	// bit 0: message pending for sound cpu
	// bit 1: message pending for main cpu
	return (m_soundlatch->pending_r() ? 1 : 0) | (m_soundlatch2->pending_r() ? 2 : 0);
}


uint8_t lsasquad_state::daikaiju_sound_status_r()
{
	// bit 0: message pending for sound cpu
	// bit 1: message pending for main cpu
	return (m_soundlatch->pending_r() ? 2 : 1);
}

uint8_t lsasquad_state::lsasquad_mcu_status_r()
{
	int res = m_bmcu_port->read();

	// bit 0 = when 1, mcu is ready to receive data from main cpu
	// bit 1 = when 0, mcu has sent data to the main cpu
	//logerror("%04x: mcu_status_r\n", m_maincpu->pc());
	if (m_bmcu)
	{
		if (CLEAR_LINE == m_bmcu->host_semaphore_r())
			res |= 0x01;
		if (CLEAR_LINE == m_bmcu->mcu_semaphore_r())
			res |= 0x02;
	}

	return res;
}

uint8_t lsasquad_state::daikaiju_mcu_status_r()
{
	int res = m_bmcu_port->read();

	// bit 0 = when 1, mcu is ready to receive data from main cpu
	// bit 1 = when 0, mcu has sent data to the main cpu
	//logerror("%04x: mcu_status_r\n", m_maincpu->pc());
	if (m_bmcu)
	{
		if (CLEAR_LINE == m_bmcu->host_semaphore_r())
			res |= 0x01;
		if (CLEAR_LINE == m_bmcu->mcu_semaphore_r())
			res |= 0x02;
	}

	res |= ((m_soundlatch->pending_r() & 0x02) ^ 2) << 3; //inverted flag
	return res;
}
