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

#include <math.h>

#include "emu.h"

#include "video/crt.h"


struct point 
{
	int intensity;		/* current intensity of the pixel */
							/* a node is not in the list when (intensity == -1) */
	int next;			/* index of next pixel in list */
};

enum
{
	intensity_pixel_not_in_list = -1	/* special value that tells that the node is not in list */
};

struct crt_t 
{
	point *list;		/* array of (crt_window_width*crt_window_height) point */
	int *list_head;	/* head of the list of lit pixels (index in the array) */
							/* keep a separate list for each display line (makes the video code slightly faster) */

	int decay_counter;	/* incremented each frame (tells for how many frames the CRT has decayed between two screen refresh) */

	/* CRT window */
	int num_intensity_levels;
	int window_offset_x, window_offset_y;
	int window_width, window_height;
};


INLINE crt_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == CRT);

	return (crt_t *)downcast<crt_device *>(device)->token();
}

static DEVICE_START( crt )
{
	crt_t *crt = get_safe_token(device);
	const crt_interface *intf = (const crt_interface *)device->static_config();
	int width = intf->width;
	int height = intf->height;
	int i;

	crt->num_intensity_levels = intf->num_levels;
	crt->window_offset_x = intf->offset_x;
	crt->window_offset_y = intf->offset_y;
	crt->window_width = width;
	crt->window_height = height;

	/* alloc the arrays */
	crt->list = auto_alloc_array(device->machine(), point, width * height);

	crt->list_head = auto_alloc_array(device->machine(), int, height);

	/* fill with black and set up list as empty */
	for (i=0; i<(width * height); i++)
	{
		crt->list[i].intensity = intensity_pixel_not_in_list;
	}

	for (i=0; i<height; i++)
		crt->list_head[i] = -1;

	crt->decay_counter = 0;
}


const device_type CRT = &device_creator<crt_device>;

crt_device::crt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CRT, "CRT Video", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(crt_t));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void crt_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void crt_device::device_start()
{
	DEVICE_START_NAME( crt )(this);
}



/*
    crt_plot

    schedule a pixel to be plotted
*/
void crt_plot(device_t *device, int x, int y)
{
	crt_t *crt = get_safe_token(device);
	point *node;
	int list_index;

	/* compute pixel coordinates */
	if (x<0) x=0;
	if (y<0) y=0;
	if ((x>(crt->window_width-1)) || ((y>crt->window_height-1)))
		return;
	y = (crt->window_height-1) - y;

	/* find entry in list */
	list_index = x + y*crt->window_width;

	node = &crt->list[list_index];

	if (node->intensity == intensity_pixel_not_in_list)
	{	/* insert node in list if it is not in it */
		node->next = crt->list_head[y];
		crt->list_head[y] = list_index;
	}
	/* set intensity */
	node->intensity = crt->num_intensity_levels;
}


/*
    crt_eof

    keep track of time
*/
void crt_eof(device_t *device)
{
	crt_t *crt = get_safe_token(device);
	crt->decay_counter++;
}


/*
    crt_update

    update the bitmap
*/
void crt_update(device_t *device, bitmap_ind16 &bitmap)
{
	crt_t *crt = get_safe_token(device);
	int i, p_i;
	int y;

	//if (crt->decay_counter)
	{
		/* some time has elapsed: let's update the screen */
		for (y=0; y<crt->window_height; y++)
		{
			UINT16 *line = &bitmap.pix16(y+crt->window_offset_y);

			p_i = -1;

			for (i=crt->list_head[y]; (i != -1); i=crt->list[i].next)
			{
				point *node = &crt->list[i];
				int x = (i % crt->window_width) + crt->window_offset_x;

				if (node->intensity == crt->num_intensity_levels)
					/* new pixel: set to max intensity */
					node->intensity = crt->num_intensity_levels-1;
				else
				{
					/* otherwise, apply intensity decay */
					node->intensity -= crt->decay_counter;
					if (node->intensity < 0)
						node->intensity = 0;
				}

				/* draw pixel on screen */
				//plot_pixel(bitmap, x, y+crt->window_offset_y, node->intensity);
				line[x] = node->intensity;

				if (node->intensity != 0)
					p_i = i;	/* current node will be next iteration's previous node */
				else
				{	/* delete current node */
					node->intensity = intensity_pixel_not_in_list;
					if (p_i != -1)
						crt->list[p_i].next = node->next;
					else
						crt->list_head[y] = node->next;
				}
			}
		}

		crt->decay_counter = 0;
	}
}
