/*************************************************************************

    Centuri Aztarac hardware

*************************************************************************/

#include "emu.h"
#include "video/vector.h"
#include "includes/aztarac.h"

#define AVECTOR(m, x, y, color, intensity) \
vector_add_point (m, xcenter + ((x) << 16), ycenter - ((y) << 16), color, intensity)

UINT16 *aztarac_vectorram;

static int xcenter, ycenter;

INLINE void read_vectorram (int addr, int *x, int *y, int *c)
{
    *c = aztarac_vectorram[addr] & 0xffff;
    *x = aztarac_vectorram[addr + 0x800] & 0x03ff;
    *y = aztarac_vectorram[addr + 0x1000] & 0x03ff;
    if (*x & 0x200) *x |= 0xfffffc00;
    if (*y & 0x200) *y |= 0xfffffc00;
}

WRITE16_HANDLER( aztarac_ubr_w )
{
    int x, y, c, intensity, xoffset, yoffset, color;
    int defaddr, objaddr=0, ndefs;

    if (data) /* data is the global intensity (always 0xff in Aztarac). */
    {
        vector_clear_list();

        while (1)
        {
            read_vectorram (objaddr, &xoffset, &yoffset, &c);
            objaddr++;

            if (c & 0x4000)
                break;

            if ((c & 0x2000) == 0)
            {
                defaddr = (c >> 1) & 0x7ff;
                AVECTOR (space->machine, xoffset, yoffset, 0, 0);

                read_vectorram (defaddr, &x, &ndefs, &c);
				ndefs++;

                if (c & 0xff00)
                {
                    /* latch color only once */
                    intensity = (c >> 8);
					color = VECTOR_COLOR222(c & 0x3f);
                    while (ndefs--)
                    {
                        defaddr++;
                        read_vectorram (defaddr, &x, &y, &c);
                        if ((c & 0xff00) == 0)
                            AVECTOR (space->machine, x + xoffset, y + yoffset, 0, 0);
                        else
                            AVECTOR (space->machine, x + xoffset, y + yoffset, color, intensity);
                    }
                }
                else
                {
                    /* latch color for every definition */
                    while (ndefs--)
                    {
                        defaddr++;
                        read_vectorram (defaddr, &x, &y, &c);
						color = VECTOR_COLOR222(c & 0x3f);
                        AVECTOR (space->machine, x + xoffset, y + yoffset, color, c >> 8);
                    }
                }
            }
        }
    }
}


VIDEO_START( aztarac )
{
	const rectangle &visarea = machine->primary_screen->visible_area();

	int xmin = visarea.min_x;
	int ymin = visarea.min_y;
	int xmax = visarea.max_x;
	int ymax = visarea.max_y;

	xcenter=((xmax + xmin) / 2) << 16;
	ycenter=((ymax + ymin) / 2) << 16;

	VIDEO_START_CALL(vector);
}
