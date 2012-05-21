/*************************************************************************

    Centuri Aztarac hardware

*************************************************************************/

#include "emu.h"
#include "video/vector.h"
#include "includes/aztarac.h"

#define AVECTOR(m, x, y, color, intensity) \
vector_add_point (m, m_xcenter + ((x) << 16), m_ycenter - ((y) << 16), color, intensity)



INLINE void read_vectorram(UINT16 *vectorram, int addr, int *x, int *y, int *c)
{
    *c = vectorram[addr] & 0xffff;
    *x = vectorram[addr + 0x800] & 0x03ff;
    *y = vectorram[addr + 0x1000] & 0x03ff;
    if (*x & 0x200) *x |= 0xfffffc00;
    if (*y & 0x200) *y |= 0xfffffc00;
}

WRITE16_MEMBER(aztarac_state::aztarac_ubr_w)
{
    UINT16 *vectorram = m_vectorram;
    int x, y, c, intensity, xoffset, yoffset, color;
    int defaddr, objaddr=0, ndefs;

    if (data) /* data is the global intensity (always 0xff in Aztarac). */
    {
        vector_clear_list();

        while (1)
        {
            read_vectorram(vectorram, objaddr, &xoffset, &yoffset, &c);
            objaddr++;

            if (c & 0x4000)
                break;

            if ((c & 0x2000) == 0)
            {
                defaddr = (c >> 1) & 0x7ff;
                AVECTOR (machine(), xoffset, yoffset, 0, 0);

                read_vectorram(vectorram, defaddr, &x, &ndefs, &c);
                ndefs++;

                if (c & 0xff00)
                {
                    /* latch color only once */
                    intensity = (c >> 8);
                    color = VECTOR_COLOR222(c & 0x3f);
                    while (ndefs--)
                    {
                        defaddr++;
                        read_vectorram(vectorram, defaddr, &x, &y, &c);
                        if ((c & 0xff00) == 0)
                            AVECTOR (machine(), x + xoffset, y + yoffset, 0, 0);
                        else
                            AVECTOR (machine(), x + xoffset, y + yoffset, color, intensity);
                    }
                }
                else
                {
                    /* latch color for every definition */
                    while (ndefs--)
                    {
                        defaddr++;
                        read_vectorram(vectorram, defaddr, &x, &y, &c);
                        color = VECTOR_COLOR222(c & 0x3f);
                        AVECTOR (machine(), x + xoffset, y + yoffset, color, c >> 8);
                    }
                }
            }
        }
    }
}


VIDEO_START( aztarac )
{
	aztarac_state *state = machine.driver_data<aztarac_state>();
	const rectangle &visarea = machine.primary_screen->visible_area();

	int xmin = visarea.min_x;
	int ymin = visarea.min_y;
	int xmax = visarea.max_x;
	int ymax = visarea.max_y;

	state->m_xcenter=((xmax + xmin) / 2) << 16;
	state->m_ycenter=((ymax + ymin) / 2) << 16;

	VIDEO_START_CALL(vector);
}
