
#include "emu.h"
#include "k054338.h"


#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

/***************************************************************************/
/*                                                                         */
/*                                 054338                                  */
/*                                                                         */
/***************************************************************************/

// k054338 alpha blend / final mixer (normally used with the 55555)
// because the implementation is video dependant, this is just a
// register-handling shell.

const device_type K054338 = &device_creator<k054338_device>;

k054338_device::k054338_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K054338, "Konami 054338 Mixer Device", tag, owner, clock, "k054338", __FILE__),
	device_video_interface(mconfig, *this),
	m_alpha_inv(0),
	m_k055555_tag(NULL)
	//m_regs[32],
	//m_shd_rgb[9],
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k054338_device::device_start()
{
	m_k055555 = m_k055555_tag ? machine().device<k055555_device>(m_k055555_tag) : NULL;

	save_item(NAME(m_regs));
	save_item(NAME(m_shd_rgb));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k054338_device::device_reset()
{
	memset(m_regs, 0, sizeof(UINT16)*32);
	memset(m_shd_rgb, 0, sizeof(int)*9);
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

WRITE16_MEMBER( k054338_device::word_w )
{
	COMBINE_DATA(m_regs + offset);
}

WRITE32_MEMBER( k054338_device::long_w )
{
	offset <<= 1;
	word_w(space, offset, data >> 16, mem_mask >> 16);
	word_w(space, offset + 1, data, mem_mask);
}

// returns a 16-bit '338 register
int  k054338_device::register_r( int reg )
{
	return m_regs[reg];
}

void k054338_device::update_all_shadows( int rushingheroes_hack, palette_device *palette )
{
	int i, d;
	int noclip = m_regs[K338_REG_CONTROL] & K338_CTL_CLIPSL;

	for (i = 0; i < 9; i++)
	{
		d = m_regs[K338_REG_SHAD1R + i] & 0x1ff;
		if (d >= 0x100)
			d -= 0x200;
		m_shd_rgb[i] = d;
	}

	if (!rushingheroes_hack)
	{
		palette->set_shadow_dRGB32(0, m_shd_rgb[0], m_shd_rgb[1], m_shd_rgb[2], noclip);
		palette->set_shadow_dRGB32(1, m_shd_rgb[3], m_shd_rgb[4], m_shd_rgb[5], noclip);
		palette->set_shadow_dRGB32(2, m_shd_rgb[6], m_shd_rgb[7], m_shd_rgb[8], noclip);
	}
	else // rushing heroes seems to specify shadows in another format, or it's not being interpreted properly.
	{
		palette->set_shadow_dRGB32(0, -80, -80, -80, 0);
		palette->set_shadow_dRGB32(1, -80, -80, -80, 0);
		palette->set_shadow_dRGB32(2, -80, -80, -80, 0);
	}
}

// k054338 BG color fill
void k054338_device::fill_solid_bg( bitmap_rgb32 &bitmap )
{
	UINT32 bgcolor;
	UINT32 *pLine;
	int x, y;

	bgcolor = (register_r(K338_REG_BGC_R) & 0xff) << 16;
	bgcolor |= register_r(K338_REG_BGC_GB);

	/* and fill the screen with it */
	for (y = 0; y < bitmap.height(); y++)
	{
		pLine = &bitmap.pix32(y);
		for (x = 0; x < bitmap.width(); x++)
			*pLine++ = bgcolor;
	}
}

// Unified k054338/K055555 BG color fill
void k054338_device::fill_backcolor( bitmap_rgb32 &bitmap, int mode ) // (see p.67)
{
	int clipx, clipy, clipw, cliph, i, dst_pitch;
	int BGC_CBLK, BGC_SET;
	UINT32 *dst_ptr, *pal_ptr;
	int bgcolor;
	const rectangle &visarea = m_screen->visible_area();

	clipx = visarea.min_x & ~3;
	clipy = visarea.min_y;
	clipw = (visarea.max_x - clipx + 4) & ~3;
	cliph = visarea.max_y - clipy + 1;

	dst_ptr = &bitmap.pix32(clipy);
	dst_pitch = bitmap.rowpixels();
	dst_ptr += clipx;

	BGC_SET = 0;
	pal_ptr = machine().driver_data()->m_generic_paletteram_32;

	if (!mode || m_k055555 == NULL)
	{
		// single color output from CLTC
		bgcolor = (int)(m_regs[K338_REG_BGC_R] & 0xff) << 16 | (int)m_regs[K338_REG_BGC_GB];
	}
	else
	{
		BGC_CBLK = m_k055555->k055555_read_register(m_k055555, 0);
		BGC_SET  = m_k055555->k055555_read_register(m_k055555, 1);

		pal_ptr += BGC_CBLK << 9;

		// single color output from PCU2
		if (!(BGC_SET & 2))
		{
			bgcolor = *pal_ptr;
			mode = 0;
		}
		else bgcolor = 0;
	}

	if (!mode)
	{
		// single color fill
		dst_ptr += clipw;
		i = clipw = -clipw;
		do
		{
			do
			{
				dst_ptr[i] = dst_ptr[i+1] = dst_ptr[i+2] = dst_ptr[i+3] = bgcolor;
			}
			while (i += 4);

			dst_ptr += dst_pitch;
			i = clipw;
		}
		while (--cliph);
	}
	else
	{
		if (!(BGC_SET & 1))
		{
			// vertical gradient fill
			pal_ptr += clipy;
			dst_ptr += clipw;
			bgcolor = *pal_ptr++;
			i = clipw = -clipw;
			do
			{
				do
				{
					dst_ptr[i] = dst_ptr[i+1] = dst_ptr[i+2] = dst_ptr[i+3] = bgcolor;
				}
				while (i += 4);

				dst_ptr += dst_pitch;
				bgcolor = *pal_ptr++;
				i = clipw;
			}
			while (--cliph);
		}
		else
		{
			// horizontal gradient fill
			pal_ptr += clipx;
			clipw <<= 2;
			do
			{
				memcpy(dst_ptr, pal_ptr, clipw);
				dst_ptr += dst_pitch;
			}
			while (--cliph);
		}
	}
}

// addition blending unimplemented (requires major changes to drawgfx and tilemap.c)
int k054338_device::set_alpha_level( int pblend )
{
	UINT16 *regs;
	int ctrl, mixpri, mixset, mixlv;

	if (pblend <= 0 || pblend > 3)
	{
		return (255);
	}

	regs   = m_regs;
	ctrl   = m_regs[K338_REG_CONTROL];
	mixpri = ctrl & K338_CTL_MIXPRI;
	mixset = regs[K338_REG_PBLEND + (pblend >> 1 & 1)] >> (~pblend << 3 & 8);
	mixlv  = mixset & 0x1f;

	if (m_alpha_inv)
		mixlv = 0x1f - mixlv;

	if (!(mixset & 0x20))
	{
		mixlv = (mixlv << 3) | (mixlv >> 2);
	}
	else
	{
		if (!mixpri)
		{
			// source x alpha  +  target (clipped at 255)
		}
		else
		{
			// source  +  target x alpha (clipped at 255)
		}

		// DUMMY
		if (mixlv && mixlv < 0x1f)
			mixlv = 0x10;

		mixlv = (mixlv << 3) | (mixlv >> 2);

		if (VERBOSE)
			popmessage("MIXSET%1d %s addition mode: %02x", pblend, (mixpri) ? "dst" : "src", mixset & 0x1f);
	}

	return mixlv;
}

void k054338_device::invert_alpha( int invert )
{
	m_alpha_inv = invert;
}


void k054338_device::export_config( int **shd_rgb )
{
	*shd_rgb = m_shd_rgb;
}

// debug handler

READ16_MEMBER( k054338_device::word_r )
{
	return(m_regs[offset]);
}       // CLTC
