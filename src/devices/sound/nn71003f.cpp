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
	  m_miso(*this)
{
}

void nn71003f_device::device_start()
{
	save_item(NAME(m_ss));
	save_item(NAME(m_sclk));
	save_item(NAME(m_mosi));
}

void nn71003f_device::device_reset()
{
	m_ss = 0;
	m_sclk = 0;
	m_mosi = 0;
}

void nn71003f_device::ss_w(int state)
{
	if(state == m_ss)
		return;
	m_ss = state;
	if(!m_ss)
		m_spi_cnt = 0;
}

void nn71003f_device::sclk_w(int state)
{
	if(state == m_sclk)
		return;
	m_sclk = state;
	if(!m_sclk)
		return;
	m_spi_byte = (m_spi_byte << 1) | m_mosi;
	m_spi_cnt ++;
	if(m_spi_cnt & 7)
		return;

	logerror("SPI %x: %02x\n", m_spi_cnt >> 3, m_spi_byte);
}

void nn71003f_device::mosi_w(int state)
{
	if(state == m_mosi)
		return;
	m_mosi = state;
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

void nn71003f_device::sound_stream_update(sound_stream &stream)
{
}

