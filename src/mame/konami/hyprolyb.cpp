// license:BSD-3-Clause
// copyright-holders:Chris Hardy
#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "hyprolyb.h"

DEFINE_DEVICE_TYPE(HYPROLYB_ADPCM, hyprolyb_adpcm_device, "hyprolyb_adpcm", "Hyper Olympics Audio")

hyprolyb_adpcm_device::hyprolyb_adpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HYPROLYB_ADPCM, tag, owner, clock)
	, m_audiocpu(*this, ":audiocpu")
	, m_soundlatch2(*this, ":soundlatch2")
	, m_msm(*this, ":msm")
	, m_adpcm_ready(0)
	, m_adpcm_busy(0)
	, m_vck_ready(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hyprolyb_adpcm_device::device_start()
{
	save_item(NAME(m_adpcm_ready));  // only bootlegs
	save_item(NAME(m_adpcm_busy));
	save_item(NAME(m_vck_ready));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hyprolyb_adpcm_device::device_reset()
{
	m_adpcm_ready = 0;
	m_adpcm_busy = 0;
	m_vck_ready = 0;
}

void hyprolyb_adpcm_device::write(uint8_t data)
{
	m_soundlatch2->write(data);
	m_adpcm_ready = 0x80;
}

uint8_t hyprolyb_adpcm_device::busy_r()
{
	return m_adpcm_busy ? 0x10 : 0x00;
}

void hyprolyb_adpcm_device::msm_data_w(uint8_t data)
{
	m_msm->data_w(data);
	m_adpcm_busy = ~data & 0x80;
}

uint8_t hyprolyb_adpcm_device::msm_vck_r()
{
	uint8_t old = m_vck_ready;
	m_vck_ready = 0x00;
	return old;
}

uint8_t hyprolyb_adpcm_device::ready_r()
{
	return m_adpcm_ready;
}

uint8_t hyprolyb_adpcm_device::data_r()
{
	m_adpcm_ready = 0x00;
	return m_soundlatch2->read();
}

void hyprolyb_adpcm_device::vck_callback( int st )
{
	m_vck_ready = 0x80;
}
