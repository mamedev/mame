/**********************************************************************

    DEC VT Terminal video emulation
    [ DC012 and DC011 emulation ]

    01/05/2009 Initial implementation [Miodrag Milanovic]

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "emu.h"
#include "vtvideo.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define	VERBOSE			0

#define	LOG(x)		do { if (VERBOSE) logerror x; } while (0)

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _vt_video_t vt_video_t;
struct _vt_video_t
{
	devcb_resolved_read8		in_ram_func;
	devcb_resolved_write8		clear_video_interrupt;

	screen_device *screen;	/* screen */
	UINT8 *gfx;		/* content of char rom */

	int lba7;

	// dc012 attributes
	UINT8 scroll_latch;
	UINT8 blink_flip_flop;
	UINT8 reverse_field;
	UINT8 basic_attribute;
	// dc011 attributes
	UINT8 columns;
	UINT8 height;
	UINT8 skip_lines;
	UINT8 frequency;
	UINT8 interlaced;
};

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE vt_video_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == VT100_VIDEO);

	return (vt_video_t *)downcast<legacy_device_base *>(device)->token();
}

INLINE const vt_video_interface *get_interface(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == VT100_VIDEO);

	return (const vt_video_interface *) device->static_config();
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static void vt_video_recompute_parameters(device_t *device)
{
	vt_video_t *vt = get_safe_token(device);
	int horiz_pix_total = 0;
	int vert_pix_total = 0;
	rectangle visarea;

	horiz_pix_total = vt->columns * 10;
	vert_pix_total  = 25 * 10;

	visarea.set(0, horiz_pix_total - 1, 0, vert_pix_total - 1);

	vt->screen->configure(horiz_pix_total, vert_pix_total, visarea,
				vt->screen->frame_period().attoseconds);
}
READ8_DEVICE_HANDLER( vt_video_lba7_r )
{
	vt_video_t *vt = get_safe_token(device);
	return vt->lba7;
}


WRITE8_DEVICE_HANDLER( vt_video_dc012_w )
{
	vt_video_t *vt = get_safe_token(device);

	if ((data & 0x08)==0) {
		if ((data & 0x04)==0) {
			// set lower part scroll
			vt->scroll_latch = (vt->scroll_latch & 0x0c) | (data & 0x03);
		} else {
			// set higher part scroll
			vt->scroll_latch = (vt->scroll_latch & 0x03) | ((data & 0x03) << 2);
		}
	} else {
		switch( data & 0x0f) {
			case 0x08:
				// toggle blink flip flop
				vt->blink_flip_flop = (vt->blink_flip_flop==0) ? 1 : 0;
				break;
			case 0x09:
				// clear vertical frequency interrupt;
				vt->clear_video_interrupt(0, 0);
				break;
			case 0x0A:
				// set reverse field on
				vt->reverse_field = 1;
				break;
			case 0x0B:
				// set reverse field off
				vt->reverse_field = 0;
				break;
			case 0x0C:
				// set basic attribute to underline
				vt->basic_attribute = 0;
				vt->blink_flip_flop = 0;
				break;
			case 0x0D:
				// set basic attribute to reverse video
				vt->basic_attribute = 1;
				vt->blink_flip_flop = 0;
				break;
			case 0x0E:
			case 0x0F:
				// reserved for future specification
				vt->blink_flip_flop = 0;
				break;
		}
	}
}


WRITE8_DEVICE_HANDLER( vt_video_dc011_w )
{
	vt_video_t *vt = get_safe_token(device);
	if (BIT(data,5)==0) {
		UINT8 col = vt->columns;
		if (BIT(data,4)==0) {
			vt->columns = 80;
		} else {
			vt->columns = 132;
		}
		if (col!=vt->columns) {
			vt_video_recompute_parameters(device);
		}
		vt->interlaced = 1;
	} else {
		if (BIT(data,4)==0) {
			vt->frequency = 60;
			vt->skip_lines = 2;
		} else {
			vt->frequency = 50;
			vt->skip_lines = 5;
		}
		vt->interlaced = 0;
	}
}

WRITE8_DEVICE_HANDLER( vt_video_brightness_w )
{
	//palette_set_color_rgb(device->machine(), 1, data, data, data);
}

static void vt_video_display_char(device_t *device,bitmap_ind16 &bitmap, UINT8 code,
	int x, int y,UINT8 scroll_region,UINT8 display_type)
{
	UINT8 line=0;
	int i,b,bit=0,prevbit,invert=0,j;
	int double_width = (display_type==2) ? 1 : 0;
	vt_video_t *vt = get_safe_token(device);

	for (i = 0; i < 10; i++)
	{

		switch(display_type) {
			case 0 : // bottom half, double height
					 j = (i >> 1)+5; break;
			case 1 : // top half, double height
					 j = (i >> 1); break;
			case 2 : // double width
			case 3 : // normal
					 j = i;	break;
			default : j = 0; break;
		}
		// modify line since that is how it is stored in rom
		if (j==0) j=15; else j=j-1;

		line = vt->gfx[(code & 0x7f)*16 + j];
		if (vt->basic_attribute==1) {
			if ((code & 0x80)==0x80)
				invert = 1;
			else
				invert = 0;
		}

		for (b = 0; b < 8; b++)
		{
			prevbit = bit;
			bit = (((line << b) & 0x80) ? 1 : 0);
			if (double_width) {
				bitmap.pix16(y*10+i, x*20+b*2)   =  (bit|prevbit)^invert;
				bitmap.pix16(y*10+i, x*20+b*2+1) =  bit^invert;
			} else {
				bitmap.pix16(y*10+i, x*10+b) =  (bit|prevbit)^invert;
			}
		}
		prevbit = bit;
		// char interleave is filled with last bit
		if (double_width) {
			bitmap.pix16(y*10+i, x*20+16) =  (bit|prevbit)^invert;
			bitmap.pix16(y*10+i, x*20+17) =  bit^invert;
			bitmap.pix16(y*10+i, x*20+18) =  bit^invert;
			bitmap.pix16(y*10+i, x*20+19) =  bit^invert;
		} else {
			bitmap.pix16(y*10+i, x*10+8) =  (bit|prevbit)^invert;
			bitmap.pix16(y*10+i, x*10+9) =  bit^invert;
		}
	}
}

void vt_video_update(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	vt_video_t *vt = get_safe_token(device);

	UINT16 addr = 0;
	int line = 0;
	int xpos = 0;
	int ypos = 0;
	UINT8 code;
	int x = 0;
	UINT8 scroll_region = 1; // binary 1
	UINT8 display_type = 3;  // binary 11
	UINT16 temp =0;

	if (vt->in_ram_func(0) !=0x7f) return;

	while(line < (vt->height + vt->skip_lines)) {
		code =  vt->in_ram_func(addr + xpos);
		if (code == 0x7f) {
			// end of line, fill empty till end of line
			if (line >= vt->skip_lines) {
				for(x = xpos; x < ((display_type==2) ? (vt->columns / 2) : vt->columns); x++ )
				{
					vt_video_display_char(device,bitmap,code,x,ypos,scroll_region,display_type);
				}
			}
			// move to new data
			temp = vt->in_ram_func(addr+xpos+1)*256 + vt->in_ram_func(addr+xpos+2);
			addr = (temp) & 0x1fff;
			// if A12 is 1 then it is 0x2000 block, if 0 then 0x4000 (AVO)
			if (addr & 0x1000) addr &= 0xfff; else addr |= 0x2000;
			scroll_region = (temp >> 15) & 1;
			display_type  = (temp >> 13) & 3;
			if (line >= vt->skip_lines) {
				ypos++;
			}
			xpos=0;
			line++;
		} else {
			// display regular char
			if (line >= vt->skip_lines) {
				vt_video_display_char(device,bitmap,code,xpos,ypos,scroll_region,display_type);
			}
			xpos++;
			if (xpos > vt->columns) {
				line++;
				xpos=0;
			}
		}
	}

}

static void rainbow_video_display_char(device_t *device,bitmap_ind16 &bitmap, UINT8 code,
	int x, int y,UINT8 scroll_region,UINT8 display_type)
{
	UINT8 line=0;
	int i,b,bit=0,j;
	int double_width = (display_type==2) ? 1 : 0;
	vt_video_t *vt = get_safe_token(device);

	for (i = 0; i < 10; i++)
	{

		switch(display_type) {
			case 0 : // bottom half, double height
					 j = (i >> 1)+5; break;
			case 1 : // top half, double height
					 j = (i >> 1); break;
			case 2 : // double width
			case 3 : // normal
					 j = i;	break;
			default : j = 0; break;
		}
		// modify line since that is how it is stored in rom
		if (j==0) j=15; else j=j-1;

		line = vt->gfx[code*16 + j];
		if (vt->basic_attribute==1) {
			if ((code & 0x80)==0x80) {
				line = line ^ 0xff;
			}
		}

		for (b = 0; b < 8; b++)
		{
			bit = ((line << b) & 0x80) ? 1 : 0;
			if (double_width) {
				bitmap.pix16(y*10+i, x*20+b*2)   =  bit;
				bitmap.pix16(y*10+i, x*20+b*2+1) =  bit;
			} else {
				bitmap.pix16(y*10+i, x*10+b) =  bit;
			}
		}
		// char interleave is filled with last bit
		if (double_width) {
			bitmap.pix16(y*10+i, x*20+16) =  bit;
			bitmap.pix16(y*10+i, x*20+17) =  bit;
			bitmap.pix16(y*10+i, x*20+18) =  bit;
			bitmap.pix16(y*10+i, x*20+19) =  bit;
		} else {
			bitmap.pix16(y*10+i, x*10+8) =  bit;
			bitmap.pix16(y*10+i, x*10+9) =  bit;
		}
	}
}
void rainbow_video_update(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	vt_video_t *vt = get_safe_token(device);

	UINT16 addr = 0;
	UINT16 attr_addr = 0;
	int line = 0;
	int xpos = 0;
	int ypos = 0;
	UINT8 code;
	int x = 0;
	UINT8 scroll_region = 1; // binary 1
	UINT8 display_type = 3;  // binary 11
	UINT16 temp =0;

	while(line < (vt->height + vt->skip_lines)) {
		code =  vt->in_ram_func(addr + xpos);
		if (code == 0xff) {
			// end of line, fill empty till end of line
			if (line >= vt->skip_lines) {
				for(x = xpos; x < ((display_type==2) ? (vt->columns / 2) : vt->columns); x++ )
				{
					rainbow_video_display_char(device,bitmap,code,x,ypos,scroll_region,display_type);
				}
			}
			// move to new data
			temp = vt->in_ram_func(addr+xpos+2)*256 + vt->in_ram_func(addr+xpos+1);
			addr = (temp) & 0x0fff;
			attr_addr = ((temp) & 0x1fff) - 2;
			// if A12 is 1 then it is 0x2000 block, if 0 then 0x4000 (AVO)
			if (temp & 0x1000) attr_addr &= 0xfff; else attr_addr |= 0x1000;
			temp = vt->in_ram_func(attr_addr);
			scroll_region = (temp) & 1;
			display_type  = (temp>> 1) & 3;
			if (line >= vt->skip_lines) {
				ypos++;
			}
			xpos=0;
			line++;
		} else {
			// display regular char
			if (line >= vt->skip_lines) {
				rainbow_video_display_char(device,bitmap,code,xpos,ypos,scroll_region,display_type);
			}
			xpos++;
			if (xpos > vt->columns) {
				line++;
				xpos=0;
			}
		}
	}

}
/*-------------------------------------------------
    DEVICE_START( vt_video )
-------------------------------------------------*/
static TIMER_CALLBACK(lba7_change)
{
	device_t *device = (device_t *)ptr;
	vt_video_t *vt = get_safe_token(device);

	vt->lba7 = (vt->lba7) ? 0 : 1;
}

static DEVICE_START( vt_video )
{
	vt_video_t *vt = get_safe_token(device);
	const vt_video_interface *intf = get_interface(device);

	/* resolve callbacks */
	vt->in_ram_func.resolve(intf->in_ram_func, *device);
	vt->clear_video_interrupt.resolve(intf->clear_video_interrupt, *device);

	/* get the screen device */
	vt->screen = device->machine().device<screen_device>(intf->screen_tag);
	assert(vt->screen != NULL);

	vt->gfx = device->machine().root_device().memregion(intf->char_rom_region_tag)->base();
	assert(vt->gfx != NULL);

	// LBA7 is scan line frequency update
	device->machine().scheduler().timer_pulse(attotime::from_nsec(31778), FUNC(lba7_change), 0, (void *) device);
}


/*-------------------------------------------------
    DEVICE_RESET( vt_video )
-------------------------------------------------*/

static DEVICE_RESET( vt_video )
{
	vt_video_t *vt = get_safe_token(device);
	palette_set_color_rgb(device->machine(), 0, 0x00, 0x00, 0x00); // black
	palette_set_color_rgb(device->machine(), 1, 0xff, 0xff, 0xff); // white

	vt->height = 25;
	vt->lba7 = 0;

	vt->scroll_latch = 0;
	vt->blink_flip_flop = 0;
	vt->reverse_field = 0;
	vt->basic_attribute = 0;

	vt->columns = 80;
	vt->frequency = 60;
	vt->interlaced = 1;
	vt->skip_lines = 2; // for 60Hz
}

/*-------------------------------------------------
    DEVICE_GET_INFO( vt100_video )
-------------------------------------------------*/

DEVICE_GET_INFO( vt100_video )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(vt_video_t);					break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;									break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(vt_video);		break;
		case DEVINFO_FCT_STOP:							/* Nothing */									break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(vt_video);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "VT100 Video");					break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "VTxxx Video");					break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright MESS Team");			break;
	}
}

DEFINE_LEGACY_DEVICE(VT100_VIDEO, vt100_video);
