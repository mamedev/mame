/* */

#include "emu.h"
#include "k053936.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)



/***************************************************************************/
/*                                                                         */
/*                                 053936                                  */
/*                                                                         */
/***************************************************************************/

/*

053936
------
Evolution of the 051316. The data bus is 16-bit instead of 8-bit.
When used in "simple" mode it can generate the same effects of the 051316, but it
doesn't have internal tilemap RAM, so it just generates a couple of X/Y coordinates
indicating the pixel to display at each moment. Therefore, the tilemap and tile
sizes are not fixed.
The important addition over the 051316 is 512x4 words of internal RAM used to control
rotation and zoom scanline by scanline instead that on the whole screen, allowing for
effects like linescroll (Super Slams) or 3D rotation of the tilemap (Golfing Greats,
Premier Soccer).

control registers
000 X counter starting value / 256
001 Y counter starting value / 256
002 ["simple" mode only] amount to add to the X counter after each line (0 = no rotation)
003 ["simple" mode only] amount to add to the Y counter after each line
004 ["simple" mode only] amount to add to the X counter after each horizontal pixel
005 ["simple" mode only] amount to add to the Y counter after each horizontal pixel (0 = no rotation)
006 x------- -------- when set, register (line*4)+2 must be multiplied by 256
    -x------ -------- when set, registers 002 and 003 must be multiplied by 256
    --xxxxxx -------- clipping for the generated address? usually 3F, Premier Soccer
                      sets it to 07 before penalty kicks
    -------- x------- when set, register (line*4)+3 must be multiplied by 256
    -------- -x------ when set, registers 004 and 005 must be multiplied by 256
    -------- --xxxxxx clipping for the generated address? usually 3F, Premier Soccer
                      sets it to 0F before penalty kicks
007 -------- -x------ enable "super" mode
    -------- --x----- unknown (enable address clipping from register 006?)
    -------- ---x---- unknown
    -------- ------x- (not sure) enable clipping with registers 008-00b
008 min x screen coordinate to draw to (only when enabled by register 7)
009 max x screen coordinate to draw to (only when enabled by register 7)
00a min y screen coordinate to draw to (only when enabled by register 7)
00b max y screen coordinate to draw to (only when enabled by register 7)
00c unknown
00d unknown
00e unknown
00f unknown

additional control from extra RAM:
(line*4)+0 X counter starting value / 256 (add to register 000)
(line*4)+1 Y counter starting value / 256 (add to register 001)
(line*4)+2 amount to add to the X counter after each horizontal pixel
(line*4)+3 amount to add to the Y counter after each horizontal pixel

*/


#define K053936_MAX_CHIPS 2

static int K053936_offset[K053936_MAX_CHIPS][2];
static int K053936_wraparound[K053936_MAX_CHIPS];

// there is another implementation of this in  machine/konamigx.c (!)
//  why?

static void K053936_zoom_draw(int chip,UINT16 *ctrl,UINT16 *linectrl, bitmap_ind16 &bitmap,const rectangle &cliprect,tilemap_t *tmap,int flags,UINT32 priority, int glfgreat_hack)
{
	if (!tmap)
		return;

	if (ctrl[0x07] & 0x0040)
	{
		UINT32 startx,starty;
		int incxx,incxy;
		rectangle my_clip;
		int y,maxy;

		// Racin' Force will get to here if glfgreat_hack is enabled, and it ends
		// up setting a maximum y value of '13', thus causing nothing to be drawn.
		// It looks like the roz output should be flipped somehow as it seems to be
		// displaying the wrong areas of the tilemap and is rendered upside down,
		// although due to the additional post-processing the voxel renderer performs
		// it's difficult to know what the output SHOULD be.  (hold W in Racin' Force
		// to see the chip output)

		if (((ctrl[0x07] & 0x0002) && ctrl[0x09]) && (glfgreat_hack))   /* wrong, but fixes glfgreat */
		{
			my_clip.min_x = ctrl[0x08] + K053936_offset[chip][0]+2;
			my_clip.max_x = ctrl[0x09] + K053936_offset[chip][0]+2 - 1;
			if (my_clip.min_x < cliprect.min_x)
				my_clip.min_x = cliprect.min_x;
			if (my_clip.max_x > cliprect.max_x)
				my_clip.max_x = cliprect.max_x;

			y = ctrl[0x0a] + K053936_offset[chip][1]-2;
			if (y < cliprect.min_y)
				y = cliprect.min_y;
			maxy = ctrl[0x0b] + K053936_offset[chip][1]-2 - 1;
			if (maxy > cliprect.max_y)
				maxy = cliprect.max_y;
		}
		else
		{
			my_clip.min_x = cliprect.min_x;
			my_clip.max_x = cliprect.max_x;

			y = cliprect.min_y;
			maxy = cliprect.max_y;
		}

		while (y <= maxy)
		{
			UINT16 *lineaddr = linectrl + 4*((y - K053936_offset[chip][1]) & 0x1ff);
			my_clip.min_y = my_clip.max_y = y;



			startx = 256 * (INT16)(lineaddr[0] + ctrl[0x00]);
			starty = 256 * (INT16)(lineaddr[1] + ctrl[0x01]);
			incxx  =       (INT16)(lineaddr[2]);
			incxy  =       (INT16)(lineaddr[3]);

			if (ctrl[0x06] & 0x8000) incxx *= 256;
			if (ctrl[0x06] & 0x0080) incxy *= 256;

			startx -= K053936_offset[chip][0] * incxx;
			starty -= K053936_offset[chip][0] * incxy;

			tmap->draw_roz(bitmap, my_clip, startx << 5,starty << 5,
					incxx << 5,incxy << 5,0,0,
					K053936_wraparound[chip],
					flags,priority);

			y++;
		}
	}
	else    /* "simple" mode */
	{
		UINT32 startx,starty;
		int incxx,incxy,incyx,incyy;

		startx = 256 * (INT16)(ctrl[0x00]);
		starty = 256 * (INT16)(ctrl[0x01]);
		incyx  =       (INT16)(ctrl[0x02]);
		incyy  =       (INT16)(ctrl[0x03]);
		incxx  =       (INT16)(ctrl[0x04]);
		incxy  =       (INT16)(ctrl[0x05]);

		if (ctrl[0x06] & 0x4000) { incyx *= 256; incyy *= 256; }
		if (ctrl[0x06] & 0x0040) { incxx *= 256; incxy *= 256; }

		startx -= K053936_offset[chip][1] * incyx;
		starty -= K053936_offset[chip][1] * incyy;

		startx -= K053936_offset[chip][0] * incxx;
		starty -= K053936_offset[chip][0] * incxy;

		tmap->draw_roz(bitmap, cliprect, startx << 5,starty << 5,
				incxx << 5,incxy << 5,incyx << 5,incyy << 5,
				K053936_wraparound[chip],
				flags,priority);
	}

#if 0
if (machine.input().code_pressed(KEYCODE_D))
	popmessage("%04x %04x %04x %04x\n%04x %04x %04x %04x\n%04x %04x %04x %04x\n%04x %04x %04x %04x",
			ctrl[0x00],
			ctrl[0x01],
			ctrl[0x02],
			ctrl[0x03],
			ctrl[0x04],
			ctrl[0x05],
			ctrl[0x06],
			ctrl[0x07],
			ctrl[0x08],
			ctrl[0x09],
			ctrl[0x0a],
			ctrl[0x0b],
			ctrl[0x0c],
			ctrl[0x0d],
			ctrl[0x0e],
			ctrl[0x0f]);
#endif
}


void K053936_0_zoom_draw(bitmap_ind16 &bitmap,const rectangle &cliprect,tilemap_t *tmap,int flags,UINT32 priority, int glfgreat_hack)
{
	UINT16 *ctrl = reinterpret_cast<UINT16 *>(tmap->machine().root_device().memshare("k053936_0_ctrl")->ptr());
	UINT16 *linectrl = reinterpret_cast<UINT16 *>(tmap->machine().root_device().memshare("k053936_0_line")->ptr());
	K053936_zoom_draw(0,ctrl,linectrl,bitmap,cliprect,tmap,flags,priority, glfgreat_hack);
}

void K053936_wraparound_enable(int chip, int status)
{
	K053936_wraparound[chip] = status;
}


void K053936_set_offset(int chip, int xoffs, int yoffs)
{
	K053936_offset[chip][0] = xoffs;
	K053936_offset[chip][1] = yoffs;
}







/***************************************************************************/
/*                                                                         */
/*                                 053936                                  */
/*                                                                         */
/***************************************************************************/

const device_type K053936 = &device_creator<k053936_device>;

k053936_device::k053936_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K053936, "Konami 053936", tag, owner, clock, "k053936", __FILE__),
	m_ctrl(NULL),
	m_linectrl(NULL)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void k053936_device::device_config_complete()
{
	// inherit a copy of the static data
	const k053936_interface *intf = reinterpret_cast<const k053936_interface *>(static_config());
	if (intf != NULL)
	*static_cast<k053936_interface *>(this) = *intf;
	
	// or initialize to defaults if none provided
	else
	{
		m_wrap = 0;
		m_xoff = 0;
		m_yoff = 0;
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k053936_device::device_start()
{
	m_ctrl = auto_alloc_array_clear(machine(), UINT16, 0x20);
	m_linectrl = auto_alloc_array_clear(machine(), UINT16, 0x4000);

	save_pointer(NAME(m_ctrl), 0x20);
	save_pointer(NAME(m_linectrl), 0x4000);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k053936_device::device_reset()
{
	memset(m_ctrl, 0, 0x20);
}


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

WRITE16_MEMBER( k053936_device::ctrl_w )
{
	COMBINE_DATA(&m_ctrl[offset]);
}

READ16_MEMBER( k053936_device::ctrl_r )
{
	return m_ctrl[offset];
}

WRITE16_MEMBER( k053936_device::linectrl_w )
{
	COMBINE_DATA(&m_linectrl[offset]);
}

READ16_MEMBER( k053936_device::linectrl_r )
{
	return m_linectrl[offset];
}

// there is another implementation of this in  video/konamigx.c (!)
//  why? shall they be merged?
void k053936_device::zoom_draw( bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t *tmap, int flags, UINT32 priority, int glfgreat_hack )
{
	if (!tmap)
		return;

	if (m_ctrl[0x07] & 0x0040)
	{
		UINT32 startx, starty;
		int incxx, incxy;
		rectangle my_clip;
		int y, maxy;

		// Racin' Force will get to here if glfgreat_hack is enabled, and it ends
		// up setting a maximum y value of '13', thus causing nothing to be drawn.
		// It looks like the roz output should be flipped somehow as it seems to be
		// displaying the wrong areas of the tilemap and is rendered upside down,
		// although due to the additional post-processing the voxel renderer performs
		// it's difficult to know what the output SHOULD be.  (hold W in Racin' Force
		// to see the chip output)

		if (((m_ctrl[0x07] & 0x0002) && m_ctrl[0x09]) && (glfgreat_hack)) /* wrong, but fixes glfgreat */
		{
			my_clip.min_x = m_ctrl[0x08] + m_xoff + 2;
			my_clip.max_x = m_ctrl[0x09] + m_xoff + 2 - 1;
			if (my_clip.min_x < cliprect.min_x)
				my_clip.min_x = cliprect.min_x;
			if (my_clip.max_x > cliprect.max_x)
				my_clip.max_x = cliprect.max_x;

			y = m_ctrl[0x0a] + m_yoff - 2;
			if (y < cliprect.min_y)
				y = cliprect.min_y;
			maxy = m_ctrl[0x0b] + m_yoff - 2 - 1;
			if (maxy > cliprect.max_y)
				maxy = cliprect.max_y;
		}
		else
		{
			my_clip.min_x = cliprect.min_x;
			my_clip.max_x = cliprect.max_x;

			y = cliprect.min_y;
			maxy = cliprect.max_y;
		}

		while (y <= maxy)
		{
			UINT16 *lineaddr = m_linectrl + 4 * ((y - m_yoff) & 0x1ff);

			my_clip.min_y = my_clip.max_y = y;

			startx = 256 * (INT16)(lineaddr[0] + m_ctrl[0x00]);
			starty = 256 * (INT16)(lineaddr[1] + m_ctrl[0x01]);
			incxx  =       (INT16)(lineaddr[2]);
			incxy  =       (INT16)(lineaddr[3]);

			if (m_ctrl[0x06] & 0x8000)
				incxx *= 256;

			if (m_ctrl[0x06] & 0x0080)
				incxy *= 256;

			startx -= m_xoff * incxx;
			starty -= m_xoff * incxy;

			tmap->draw_roz(bitmap, my_clip, startx << 5,starty << 5,
					incxx << 5,incxy << 5,0,0,
					m_wrap,
					flags,priority);

			y++;
		}
	}
	else    /* "simple" mode */
	{
		UINT32 startx, starty;
		int incxx, incxy, incyx, incyy;

		startx = 256 * (INT16)(m_ctrl[0x00]);
		starty = 256 * (INT16)(m_ctrl[0x01]);
		incyx  =       (INT16)(m_ctrl[0x02]);
		incyy  =       (INT16)(m_ctrl[0x03]);
		incxx  =       (INT16)(m_ctrl[0x04]);
		incxy  =       (INT16)(m_ctrl[0x05]);

		if (m_ctrl[0x06] & 0x4000)
		{
			incyx *= 256;
			incyy *= 256;
		}

		if (m_ctrl[0x06] & 0x0040)
		{
			incxx *= 256;
			incxy *= 256;
		}

		startx -= m_yoff * incyx;
		starty -= m_yoff * incyy;

		startx -= m_xoff * incxx;
		starty -= m_xoff * incxy;

		tmap->draw_roz(bitmap, cliprect, startx << 5,starty << 5,
				incxx << 5,incxy << 5,incyx << 5,incyy << 5,
				m_wrap,
				flags,priority);
	}

#if 0
if (machine.input().code_pressed(KEYCODE_D))
	popmessage("%04x %04x %04x %04x\n%04x %04x %04x %04x\n%04x %04x %04x %04x\n%04x %04x %04x %04x",
			m_ctrl[0x00],
			m_ctrl[0x01],
			m_ctrl[0x02],
			m_ctrl[0x03],
			m_ctrl[0x04],
			m_ctrl[0x05],
			m_ctrl[0x06],
			m_ctrl[0x07],
			m_ctrl[0x08],
			m_ctrl[0x09],
			m_ctrl[0x0a],
			m_ctrl[0x0b],
			m_ctrl[0x0c],
			m_ctrl[0x0d],
			m_ctrl[0x0e],
			m_ctrl[0x0f]);
#endif
}


