// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Nippon Steel Corp NN71003F mpeg audio decoder

// No info could be found anywhere.  Function has (minimally) been
// found from pin connection tracing on a hrdvd board.

#include "emu.h"
#include "nn71003f.h"

DEFINE_DEVICE_TYPE(NN71003F, nn71003f_device, "nn71003f", "NN71003F mpeg audio chip")

nn71003f_device::nn71003f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NN71003F, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  m_irq_cb(*this)
{
}

void nn71003f_device::device_start()
{
	m_irq_cb.resolve_safe();
	save_item(NAME(m_cmd_atn));
	save_item(NAME(m_cmd_clk));
	save_item(NAME(m_cmd_dat));
}

void nn71003f_device::device_reset()
{
	m_cmd_atn = 0;
	m_cmd_clk = 0;
	m_cmd_dat = 0;
}

void nn71003f_device::cmd_atn_w(int state)
{
	if(state == m_cmd_atn)
		return;
	m_cmd_atn = state;
	if(!m_cmd_atn)
		m_cmd_cnt = 0;
}

void nn71003f_device::cmd_clk_w(int state)
{
	if(state == m_cmd_clk)
		return;
	m_cmd_clk = state;
	if(!m_cmd_clk)
		return;
	m_cmd_byte = (m_cmd_byte << 1) | m_cmd_dat;
	m_cmd_cnt ++;
	if(m_cmd_cnt & 7)
		return;

	logerror("CMD %x: %02x\n", m_cmd_cnt >> 3, m_cmd_byte);
}

void nn71003f_device::cmd_dat_w(int state)
{
	if(state == m_cmd_dat)
		return;
	m_cmd_dat = state;
}

void nn71003f_device::frm_w(int state)
{
	logerror("frm_w %d\n", state);
}

void nn71003f_device::dat_w(int state)
{
	logerror("dat_w %d\n", state);
}

void nn71003f_device::clk_w(int state)
{
	logerror("clk_w %d\n", state);
}

void nn71003f_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
}

