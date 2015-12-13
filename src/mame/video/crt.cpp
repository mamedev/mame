// license:GPL-2.0+
// copyright-holders:Raphael Nabet
/*
    video/crt.c

    CRT video emulation for TX-0 and PDP-1.


Theory of operation:

    What makes such CRT devices so odd is that there is no video processor, no
    scan logic, no refresh logic.  The beam position and intensity is
    controlled by the program completely: in order to draw an object, the
    program must direct the beam to each point of the object, and in order to
    refresh it, the program must redraw the object periodically.

    Since the refresh rates are highly variable (completely controlled by the
    program), I need to simulate CRT remanence: the intensity of each pixel on
    display decreases regularly.  In order to keep this efficient, I keep a
    list of non-black pixels, and only process these pixels on each refresh.
    In order to improve efficiency further, I keep a distinct list for each
    line of the display: I have found that it improves drawing speed slightly
    (probably because it improves the cache hit rate).


    Raphael Nabet 2002-2004
    Based on earlier work by Chris Salomon
*/

#include "emu.h"
#include "video/crt.h"


// special value that tells that the node is not in list
enum
{
	intensity_pixel_not_in_list = -1
};


// device type definition
const device_type CRT = &device_creator<crt_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  crt_device - constructor
//-------------------------------------------------

crt_device::crt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CRT, "CRT Video", tag, owner, clock, "crt", __FILE__),
		m_list(nullptr),
		m_list_head(nullptr),
		m_decay_counter(0),
		m_num_intensity_levels(0),
		m_window_offset_x(0),
		m_window_offset_y(0),
		m_window_width(0),
		m_window_height(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void crt_device::device_start()
{
	/* alloc the arrays */
	m_list = auto_alloc_array(machine(), crt_point, m_window_width * m_window_height);
	m_list_head = auto_alloc_array(machine(), int, m_window_height);

	/* fill with black and set up list as empty */
	for (int i = 0; i < (m_window_width * m_window_height); i++)
		m_list[i].intensity = intensity_pixel_not_in_list;

	for (int i = 0; i < m_window_height; i++)
		m_list_head[i] = -1;

	m_decay_counter = 0;
}


//
//    crt_plot
//
//    schedule a pixel to be plotted
//
void crt_device::plot(int x, int y)
{
	crt_point *node;
	int list_index;

	/* compute pixel coordinates */
	if (x<0) x=0;
	if (y<0) y=0;
	if ((x>(m_window_width-1)) || ((y>m_window_height-1)))
		return;
	y = (m_window_height-1) - y;

	/* find entry in list */
	list_index = x + y*m_window_width;

	node = &m_list[list_index];

	if (node->intensity == intensity_pixel_not_in_list)
	{   /* insert node in list if it is not in it */
		node->next = m_list_head[y];
		m_list_head[y] = list_index;
	}
	/* set intensity */
	node->intensity = m_num_intensity_levels;
}


//
//  crt_eof
//
//  keep track of time
//
void crt_device::eof()
{
	m_decay_counter++;
}


//
//  crt_update
//
//  update the bitmap
//
void crt_device::update(bitmap_ind16 &bitmap)
{
	int i, p_i;
	int y;

	//if (m_decay_counter)
	{
		/* some time has elapsed: let's update the screen */
		for (y=0; y<m_window_height; y++)
		{
			UINT16 *line = &bitmap.pix16(y+m_window_offset_y);

			p_i = -1;

			for (i=m_list_head[y]; (i != -1); i=m_list[i].next)
			{
				crt_point *node = &m_list[i];
				int x = (i % m_window_width) + m_window_offset_x;

				if (node->intensity == m_num_intensity_levels)
					/* new pixel: set to max intensity */
					node->intensity = m_num_intensity_levels-1;
				else
				{
					/* otherwise, apply intensity decay */
					node->intensity -= m_decay_counter;
					if (node->intensity < 0)
						node->intensity = 0;
				}

				/* draw pixel on screen */
				//plot_pixel(bitmap, x, y+m_window_offset_y, node->intensity);
				line[x] = node->intensity;

				if (node->intensity != 0)
					p_i = i;    /* current node will be next iteration's previous node */
				else
				{   /* delete current node */
					node->intensity = intensity_pixel_not_in_list;
					if (p_i != -1)
						m_list[p_i].next = node->next;
					else
						m_list_head[y] = node->next;
				}
			}
		}

		m_decay_counter = 0;
	}
}
