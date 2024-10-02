// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo
#ifndef MAME_UNIVERSAL_ZEROHOUR_STARS_H
#define MAME_UNIVERSAL_ZEROHOUR_STARS_H

#pragma once


// used by zerohour, redclash and sraider
class zerohour_stars_device : public device_t
{
public:
	zerohour_stars_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	// configuration helpers
	zerohour_stars_device &has_va_bit(bool va) { m_has_va_bit = va; return *this; } // default yes (sraider does not)

	// public interface
	void set_enable(bool on);
	void update_state(int state);
	void set_speed(u8 speed, u8 mask);
	void draw(bitmap_ind16 &bitmap, rectangle const &cliprect);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 m_enable;
	u8 m_speed;
	u32 m_state;
	u16 m_offset;
	u8 m_count;

	u16 m_pal_offset;
	bool m_has_va_bit;
};


DECLARE_DEVICE_TYPE(ZEROHOUR_STARS, zerohour_stars_device)

#endif // MAME_UNIVERSAL_ZEROHOUR_STARS_H
