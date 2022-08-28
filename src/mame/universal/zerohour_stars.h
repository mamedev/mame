// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo
#ifndef MAME_VIDEO_ZEROHOUR_STARS_H
#define MAME_VIDEO_ZEROHOUR_STARS_H

#pragma once


// used by zerohour, redclash and sraider
class zerohour_stars_device : public device_t
{
public:
	zerohour_stars_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	void set_enable(bool on);
	void update_state();
	void set_speed(u8 speed, u8 mask);
	void draw(bitmap_ind16 &bitmap, rectangle const &cliprect, u8 pal_offs, bool has_va, u8 firstx, u8 lastx);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	u8  m_enable;
	u8  m_speed;
	u32 m_state;
	u16 m_offset;
	u8  m_count;
};


DECLARE_DEVICE_TYPE(ZEROHOUR_STARS, zerohour_stars_device)

#endif // MAME_VIDEO_ZEROHOUR_STARS_H
