// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

  Sanyo LC78836M Digital Audio 16-bit D/A Converter
  with On-Chip Digital Filters

***************************************************************************/

#ifndef MAME_SOUND_LC78836M_H
#define MAME_SOUND_LC78836M_H

#pragma once

class lc78836m_device : public device_t, public device_sound_interface
{
public:
	lc78836m_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void mute_w(int state);

	void lrck_w(int state);
	void data_w(int state);
	void bclk_w(int state);

	void cksl1_w(int state);
	void cksl2_w(int state);

	void fs1_w(int state);
	void fs2_w(int state);

	void emp_w(int state);

	void initb_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	virtual void sound_stream_update(sound_stream &stream) override;

private:
	void update_clock();

	sound_stream *m_stream;

	uint8_t m_mute;
	uint8_t m_cksl1, m_cksl2;
	uint16_t m_clock_fs;
	uint8_t m_fs1, m_fs2;
	uint8_t m_emp;
	uint8_t m_initb;

	uint8_t m_bclk, m_lrck, m_data;
	uint8_t m_sample_bit;
	int16_t m_sample;

	sound_stream::sample_t m_sample_ch1, m_sample_ch2;
	double m_att;
};


DECLARE_DEVICE_TYPE(LC78836M, lc78836m_device)

#endif // MAME_SOUND_LC78836M_H
