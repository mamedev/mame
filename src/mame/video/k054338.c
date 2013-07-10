
#include "emu.h"
#include "k054338.h"
#include "k055555.h"


#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

/***************************************************************************/
/*                                                                         */
/*                                 054338                                  */
/*                                                                         */
/***************************************************************************/

static UINT16 k54338_regs[32];
static int K054338_shdRGB[9];
static int K054338_alphainverted;


// K054338 alpha blend / final mixer (normally used with the 55555)
// because the implementation is video dependant, this is just a
// register-handling shell.
void K054338_vh_start(running_machine &machine)
{
	memset(k54338_regs, 0, sizeof(UINT16)*32);
	memset(K054338_shdRGB, 0, sizeof(int)*9);
	K054338_alphainverted = 1;

	machine.save().save_item(NAME(k54338_regs));
}

WRITE16_HANDLER( K054338_word_w )
{
	COMBINE_DATA(k54338_regs + offset);
}

WRITE32_HANDLER( K054338_long_w )
{
	offset <<= 1;
	K054338_word_w(space, offset, data>>16, mem_mask>>16);
	K054338_word_w(space, offset+1, data, mem_mask);
}

// returns a 16-bit '338 register
int K054338_read_register(int reg)
{
	return k54338_regs[reg];
}

void K054338_update_all_shadows(running_machine &machine, int rushingheroes_hack)
{
	int i, d;
	int noclip = k54338_regs[K338_REG_CONTROL] & K338_CTL_CLIPSL;

	for (i=0; i<9; i++)
	{
		d = k54338_regs[K338_REG_SHAD1R+i] & 0x1ff;
		if (d >= 0x100) d -= 0x200;
		K054338_shdRGB[i] = d;
	}

	if (!rushingheroes_hack)
	{
		palette_set_shadow_dRGB32(machine, 0, K054338_shdRGB[0], K054338_shdRGB[1], K054338_shdRGB[2], noclip);
		palette_set_shadow_dRGB32(machine, 1, K054338_shdRGB[3], K054338_shdRGB[4], K054338_shdRGB[5], noclip);
		palette_set_shadow_dRGB32(machine, 2, K054338_shdRGB[6], K054338_shdRGB[7], K054338_shdRGB[8], noclip);
	}
	else // rushing heroes seems to specify shadows in another format, or it's not being interpreted properly.
	{
		palette_set_shadow_dRGB32(machine, 0, -80, -80, -80, 0);
		palette_set_shadow_dRGB32(machine, 1, -80, -80, -80, 0);
		palette_set_shadow_dRGB32(machine, 2, -80, -80, -80, 0);
	}
}

// Unified K054338/K055555 BG color fill
void K054338_fill_backcolor(running_machine &machine, bitmap_rgb32 &bitmap, int mode) // (see p.67)
{
	int clipx, clipy, clipw, cliph, i, dst_pitch;
	int BGC_CBLK, BGC_SET;
	UINT32 *dst_ptr, *pal_ptr;
	int bgcolor;
	const rectangle &visarea = machine.primary_screen->visible_area();
	driver_device *state = machine.driver_data();

	clipx = visarea.min_x & ~3;
	clipy = visarea.min_y;
	clipw = (visarea.max_x - clipx + 4) & ~3;
	cliph = visarea.max_y - clipy + 1;

	dst_ptr = &bitmap.pix32(clipy);
	dst_pitch = bitmap.rowpixels();
	dst_ptr += clipx;

	BGC_SET = 0;
	pal_ptr = state->m_generic_paletteram_32;

	if (!mode)
	{
		// single color output from CLTC
		bgcolor = (int)(k54338_regs[K338_REG_BGC_R]&0xff)<<16 | (int)k54338_regs[K338_REG_BGC_GB];
	}
	else
	{
		BGC_CBLK = K055555_read_register(0);
		BGC_SET  = K055555_read_register(1);
		pal_ptr += BGC_CBLK << 9;

		// single color output from PCU2
		if (!(BGC_SET & 2)) { bgcolor = *pal_ptr; mode = 0; } else bgcolor = 0;
	}

	if (!mode)
	{
		// single color fill
		dst_ptr += clipw;
		i = clipw = -clipw;
		do
		{
			do { dst_ptr[i] = dst_ptr[i+1] = dst_ptr[i+2] = dst_ptr[i+3] = bgcolor; } while (i += 4);
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
				do { dst_ptr[i] = dst_ptr[i+1] = dst_ptr[i+2] = dst_ptr[i+3] = bgcolor; } while (i += 4);
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
int K054338_set_alpha_level(int pblend)
{
	UINT16 *regs;
	int ctrl, mixpri, mixset, mixlv;

	if (pblend <= 0 || pblend > 3)
	{
		return(255);
	}

	regs   = k54338_regs;
	ctrl   = k54338_regs[K338_REG_CONTROL];
	mixpri = ctrl & K338_CTL_MIXPRI;
	mixset = regs[K338_REG_PBLEND + (pblend>>1 & 1)] >> (~pblend<<3 & 8);
	mixlv  = mixset & 0x1f;

	if (K054338_alphainverted) mixlv = 0x1f - mixlv;

	if (!(mixset & 0x20))
	{
		mixlv = mixlv<<3 | mixlv>>2;
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
		if (mixlv && mixlv<0x1f) mixlv = 0x10;
		mixlv = mixlv<<3 | mixlv>>2;

		if (VERBOSE)
			popmessage("MIXSET%1d %s addition mode: %02x",pblend,(mixpri)?"dst":"src",mixset&0x1f);
	}

	return(mixlv);
}

void K054338_invert_alpha(int invert)
{
	K054338_alphainverted = invert;
}

void K054338_export_config(int **shdRGB)
{
	*shdRGB = K054338_shdRGB;
}


