// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo
/***************************************************************************

  zerohour/redclash/sraider stars

  These functions emulate the star generator board
  All this comes from the schematics for Zero Hour

  It has a 17-bit LFSR which has a period of 2^17-1 clocks
  (This is one pixel shy of "two screens" worth.)
  So, there are two starfields drawn on alternate frames
  These will scroll at a rate controlled by the speed register

  I'm basically doing the same thing by drawing each
  starfield on alternate frames, and then offsetting them

***************************************************************************/

#include "emu.h"
#include "zerohour_stars.h"


DEFINE_DEVICE_TYPE(ZEROHOUR_STARS, zerohour_stars_device, "zerohour_stars", "Zero Hour/Red Clash/Space Raider starfield")


zerohour_stars_device::zerohour_stars_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ZEROHOUR_STARS, tag, owner, clock)
	, m_enable(0)
	, m_speed(0)
	, m_state(0)
	, m_offset(0)
	, m_count(0)
{
	// set default configuration
	m_pal_offset = 0x60;
	m_has_va_bit = true;
}

void zerohour_stars_device::device_start()
{
	save_item(NAME(m_enable));
	save_item(NAME(m_speed));
	save_item(NAME(m_state));
	save_item(NAME(m_offset));
	save_item(NAME(m_count));
}

void zerohour_stars_device::device_reset()
{
	m_enable = 0;
	m_speed = 0;
	m_state = 0;
	m_offset = 0;
	m_count = 0;
}

// This line can reset the LFSR to zero and disables the star generator
void zerohour_stars_device::set_enable(bool on)
{
	if (!m_enable && on)
		m_offset = 0;

	m_enable = on ? 1 : 0;
}

// This sets up which starfield to draw and the offset, to be called from screen_vblank_*()
void zerohour_stars_device::update_state(int state)
{
	// end of vblank
	if (!state && m_enable)
	{
		m_count = m_count ? 0 : 1;
		if (!m_count)
		{
			m_offset += ((m_speed * 2) - 0x09);
			m_offset %= 256 * 256;
			m_state = 0;
		}
		else
		{
			m_state = 0x1fc71;
		}
	}
}

// Set the speed register (3 bits)
void zerohour_stars_device::set_speed(u8 speed, u8 mask)
{
	// 0 left/down fastest (-9/2 pix per frame)
	// 1 left/down faster  (-7/2 pix per frame)
	// 2 left/down fast    (-5/2 pix per frame)
	// 3 left/down medium  (-3/2 pix per frame)
	// 4 left/down slow    (-1/2 pix per frame)
	// 5 right/up slow     (+1/2 pix per frame)
	// 6 right/up medium   (+3/2 pix per frame)
	// 7 right/up fast     (+5/2 pix per frame)
	m_speed = (m_speed & ~mask) | (speed & mask);
}

// Draw the stars
void zerohour_stars_device::draw(bitmap_ind16 &bitmap, rectangle const &cliprect)
{
	if (m_enable)
	{
		u32 state(m_state);
		for (int i = 0; (256 * 256) > i; ++i)
		{
			u8 const xloc((m_offset + i) & 0x00ff);
			u8 const yloc(((m_offset + i) >> 8) & 0x00ff);

			bool const tempbit(!(state & 0x10000));
			bool const feedback((state & 0x00020) ? !tempbit : tempbit);

			bool const hcond(BIT(xloc + 8, 4));
			bool const vcond(m_has_va_bit || BIT(yloc, 0));

			if (cliprect.contains(xloc, yloc) && (hcond == vcond))
			{
				if (((state & 0x000ff) == 0x000ff) && !feedback)
					bitmap.pix(yloc, xloc) = m_pal_offset + (state >> 9 & 0x1f);
			}

			// update LFSR state
			state = ((state << 1) & 0x1fffe) | (feedback ? 1 : 0);
		}
	}
}
