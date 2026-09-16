// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************

    Panasonic MN1252 LCD controller

***************************************************************************/

#ifndef MAME_VIDEO_MN1252_H
#define MAME_VIDEO_MN1252_H

#pragma once

class mn1252_device : public device_t
{
public:
	mn1252_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	u16 output(offs_t digit) const;

	void data_w(u8 data);
	void ce_w(int state);
	void std_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static const u8 OUTPUT_DIGITS[0x40];

	u8 m_data;
	u8 m_first_nibble;
	u8 m_nibble_count;
	u8 m_ce, m_std;

	u16 m_output[6];
};

DECLARE_DEVICE_TYPE(MN1252, mn1252_device)

#endif // MAME_VIDEO_MN1252_H
