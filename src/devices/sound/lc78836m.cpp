// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

  Sanyo LC78836M Digital Audio 16-bit D/A Converter
  with On-Chip Digital Filters
             ____   ____
     REFH 1 |    \_/    | 24 AVDD
    VrefH 2 |           | 23 CH1OUT
     MUTE 3 |           | 22 AGND
      D/N 4 |           | 21 CH2OUT
     BCLK 5 |           | 20 REFL
     DATA 6 | LC78836M  | 19 VrefL
     LRCK 7 |           | 18 CKSL2
     DVDD 8 |           | 17 CKSL1
    CKOUT 9 |           | 16 FS2
    XOUT 10 |           | 15 FS1
     XIN 11 |           | 14 EMP
    DGND 12 |___________| 13 INITB


    TOOD:
    - INITB, DN aren't implemented
    - Filters aren't implemented

***************************************************************************/

#include "emu.h"
#include "lc78836m.h"

// #define VERBOSE (LOG_GENERAL)

#include "logmacro.h"


DEFINE_DEVICE_TYPE(LC78836M, lc78836m_device, "lc78836m", "Sanyo LC78836M Digital Audio 16-bit D/A Converter")


lc78836m_device::lc78836m_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, LC78836M, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_cksl1(0), m_cksl2(0)
	, m_fs1(0), m_fs2(0)
	, m_emp(0)
{
}

void lc78836m_device::device_start()
{
	save_item(NAME(m_cksl1));
	save_item(NAME(m_cksl2));
	save_item(NAME(m_fs1));
	save_item(NAME(m_fs2));
	save_item(NAME(m_emp));

	save_item(NAME(m_mute));
	save_item(NAME(m_lrck));
	save_item(NAME(m_data));
	save_item(NAME(m_bclk));
	save_item(NAME(m_sample));
	save_item(NAME(m_sample_bit));
	save_item(NAME(m_clock_fs));

	save_item(NAME(m_sample_ch1));
	save_item(NAME(m_sample_ch2));
	save_item(NAME(m_att));

	m_stream = stream_alloc(2, 2, m_clock);
}

void lc78836m_device::device_reset()
{
	m_mute = 0;
	m_lrck = 0;
	m_data = 0;
	m_bclk = 0;
	m_sample = 0;
	m_sample_bit = 0;
	m_att = 1024;
	m_sample_ch1 = m_sample_ch2 = 0;

	update_clock();
}

void lc78836m_device::device_clock_changed()
{
	update_clock();
}

void lc78836m_device::sound_stream_update(sound_stream &stream)
{
	stream.put(0, 0, m_sample_ch1 * m_att / 1024.0);
	m_sample_ch1 = 0;

	stream.put(1, 0, m_sample_ch2 * m_att / 1024.0);
	m_sample_ch2 = 0;

	if (m_mute && m_att > 0)
		m_att--;
	else if (!m_mute && m_att < 1024)
		m_att++;

}

void lc78836m_device::mute_w(int state)
{
	// Soft mute
	m_mute = state;
}

void lc78836m_device::lrck_w(int state)
{
	// CH1 when high, CH2 when low
	m_lrck = state;
}

void lc78836m_device::data_w(int state)
{
	m_data = state;
}

void lc78836m_device::bclk_w(int state)
{
	if (!m_bclk && state) {
		m_sample |= m_data << m_sample_bit;
		m_sample_bit++;

		if (m_sample_bit >= 16) {
			sound_stream::sample_t sample = m_sample / double(std::numeric_limits<int16_t>::max());

			if (m_lrck)
				m_sample_ch1 = sample;
			else
				m_sample_ch2 = sample;

			m_sample = 0;
			m_sample_bit = 0;

			m_stream->update();
		}
	}

	m_bclk = state;
}

void lc78836m_device::initb_w(int state)
{
	// Initialization signal input
	// LSI is initialized when set low
	m_initb = state;
}

void lc78836m_device::cksl1_w(int state)
{
	// System clock selection bit 1
	if (state != m_cksl1) {
		m_cksl1 = state;
		update_clock();
	}
}

void lc78836m_device::cksl2_w(int state)
{
	// System clock selection bit 2
	if (state != m_cksl2) {
		m_cksl2 = state;
		update_clock();
	}
}

void lc78836m_device::fs1_w(int state)
{
	// De-emphasis filter mode selection bit 1
	m_fs1 = state;
}

void lc78836m_device::fs2_w(int state)
{
	// De-emphasis filter mode selection bit 2
	m_fs2 = state;
}

void lc78836m_device::emp_w(int state)
{
	// De-emphasis filter on/off
	m_emp = state;
}

void lc78836m_device::update_clock()
{
	if (m_cksl1 == 0 && m_cksl2 == 0)
		m_clock_fs = 384;
	else if (m_cksl1 == 0 && m_cksl2 == 1)
		m_clock_fs = 392;
	else if (m_cksl1 == 1 && m_cksl2 == 0)
		m_clock_fs = 448;
	else if (m_cksl1 == 1 && m_cksl2 == 1)
		m_clock_fs = 512;

	const uint32_t new_sample_rate = m_clock / m_clock_fs;
	if (m_stream != nullptr && new_sample_rate != m_stream->sample_rate()) {
		LOG("sample rate changed to %d\n", new_sample_rate);
		m_stream->set_sample_rate(new_sample_rate);
	}
}
