// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
/*************************************************************************

    Centuri Aztarac hardware

*************************************************************************/

#include "emu.h"
#include "includes/aztarac.h"

#define AVECTOR(x, y, color, intensity) \
m_vector->add_point (m_xcenter + ((x) << 16), m_ycenter - ((y) << 16), color, intensity)



inline void aztarac_state::read_vectorram(UINT16 *vectorram, int addr, int *x, int *y, int *c)
{
	*c = vectorram[addr] & 0xffff;
	*x = vectorram[addr + 0x800] & 0x03ff;
	*y = vectorram[addr + 0x1000] & 0x03ff;
	if (*x & 0x200) *x |= 0xfffffc00;
	if (*y & 0x200) *y |= 0xfffffc00;
}

WRITE16_MEMBER(aztarac_state::ubr_w)
{
	int x, y, c, intensity, xoffset, yoffset, color;
	int defaddr, objaddr=0, ndefs;

	if (data) /* data is the global intensity (always 0xff in Aztarac). */
	{
		m_vector->clear_list();

		while (1)
		{
			read_vectorram(m_vectorram, objaddr, &xoffset, &yoffset, &c);
			objaddr++;

			if (c & 0x4000)
				break;

			if ((c & 0x2000) == 0)
			{
				defaddr = (c >> 1) & 0x7ff;
				AVECTOR (xoffset, yoffset, 0, 0);

				read_vectorram(m_vectorram, defaddr, &x, &ndefs, &c);
				ndefs++;

				if (c & 0xff00)
				{
					/* latch color only once */
					intensity = (c >> 8);
					color = VECTOR_COLOR222(c & 0x3f);
					while (ndefs--)
					{
						defaddr++;
						read_vectorram(m_vectorram, defaddr, &x, &y, &c);
						if ((c & 0xff00) == 0)
							AVECTOR (x + xoffset, y + yoffset, 0, 0);
						else
							AVECTOR (x + xoffset, y + yoffset, color, intensity);
					}
				}
				else
				{
					/* latch color for every definition */
					while (ndefs--)
					{
						defaddr++;
						read_vectorram(m_vectorram, defaddr, &x, &y, &c);
						color = VECTOR_COLOR222(c & 0x3f);
						AVECTOR (x + xoffset, y + yoffset, color, c >> 8);
					}
				}
			}
		}
	}
}


void aztarac_state::video_start()
{
	const rectangle &visarea = m_screen->visible_area();

	int xmin = visarea.min_x;
	int ymin = visarea.min_y;
	int xmax = visarea.max_x;
	int ymax = visarea.max_y;

	m_xcenter=((xmax + xmin) / 2) << 16;
	m_ycenter=((ymax + ymin) / 2) << 16;
}
