// license:GPL-2.0+
// copyright-holders:Raphael Nabet
/*************************************************************************

    shared/mitcrt.h

    CRT video emulation for TX-0 and PDP-1

*************************************************************************/

#ifndef MAME_SHARED_MITCRT_H
#define MAME_SHARED_MITCRT_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> crt_device

class crt_device : public device_t
{
public:
	crt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_num_levels(int levels) { m_num_intensity_levels = levels; }
	void set_offsets(int x_offset, int y_offset)
	{
		m_window_offset_x = x_offset;
		m_window_offset_y = y_offset;
	}
	void set_size(int width, int height)
	{
		m_window_width = width;
		m_window_height = height;
	}

	void plot(int x, int y);
	void eof();
	void update(bitmap_ind16 &bitmap);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	struct crt_point
	{
		crt_point() { }

		int intensity = 0;  /* current intensity of the pixel */
							/* a node is not in the list when (intensity == -1) */
		int next = 0;       /* index of next pixel in list */
	};

	std::unique_ptr<crt_point[]> m_list; /* array of (crt_window_width*crt_window_height) point */
	std::unique_ptr<int[]> m_list_head;  /* head of the list of lit pixels (index in the array) */
						/* keep a separate list for each display line (makes the video code slightly faster) */

	int m_decay_counter;  /* incremented each frame (tells for how many frames the CRT has decayed between two screen refresh) */

	/* CRT window */
	int m_num_intensity_levels;
	int m_window_offset_x;
	int m_window_offset_y;
	int m_window_width;
	int m_window_height;
};

DECLARE_DEVICE_TYPE(CRT, crt_device)



#endif // MAME_SHARED_MITCRT_H
