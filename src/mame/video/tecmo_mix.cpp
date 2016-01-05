// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "tecmo_mix.h"


const device_type TECMO_MIXER = &device_creator<tecmo_mix_device>;

tecmo_mix_device::tecmo_mix_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TECMO_MIXER, "Tecmo 16-bit Mixer", tag, owner, clock, "tecmo_mix", __FILE__),
		device_video_interface(mconfig, *this),
		m_sprpri_shift(0),
		m_sprbln_shift(0),
		m_sprcol_shift(0),

		m_spblend_source(0),
		m_fgblend_source(0),

		m_bgblend_comp(0),
		m_fgblend_comp(0),
		m_txblend_comp(0),
		m_spblend_comp(0),

		m_bgregular_comp(0),
		m_fgregular_comp(0),
		m_txregular_comp(0),
		m_spregular_comp(0),

		m_revspritetile(0),
		m_bgpen(0)

{
}


void tecmo_mix_device::device_start()
{
}

void tecmo_mix_device::device_reset()
{
}



void tecmo_mix_device::set_mixer_shifts(device_t &device, int sprpri_shift, int sprbln_shift, int sprcol_shift)
{
	tecmo_mix_device &dev = downcast<tecmo_mix_device &>(device);
	dev.m_sprpri_shift = sprpri_shift;
	dev.m_sprbln_shift = sprbln_shift;
	dev.m_sprcol_shift = sprcol_shift;
}

void tecmo_mix_device::set_blendcols(device_t &device, int bgblend_comp, int fgblend_comp, int txblend_comp, int spblend_comp)
{
	tecmo_mix_device &dev = downcast<tecmo_mix_device &>(device);
	dev.m_bgblend_comp = bgblend_comp;
	dev.m_fgblend_comp = fgblend_comp;
	dev.m_txblend_comp = txblend_comp;
	dev.m_spblend_comp = spblend_comp;
}

void tecmo_mix_device::set_regularcols(device_t &device, int bgregular_comp, int fgregular_comp, int txregular_comp, int spregular_comp)
{
	tecmo_mix_device &dev = downcast<tecmo_mix_device &>(device);
	dev.m_bgregular_comp = bgregular_comp;
	dev.m_fgregular_comp = fgregular_comp;
	dev.m_txregular_comp = txregular_comp;
	dev.m_spregular_comp = spregular_comp;
}

void tecmo_mix_device::set_blendsource(device_t &device, int spblend_source, int fgblend_source)
{
	tecmo_mix_device &dev = downcast<tecmo_mix_device &>(device);
	dev.m_spblend_source = spblend_source;
	dev.m_fgblend_source = fgblend_source;
}

void tecmo_mix_device::set_revspritetile(device_t &device)
{
	tecmo_mix_device &dev = downcast<tecmo_mix_device &>(device);
	dev.m_revspritetile = 3;
}

void tecmo_mix_device::set_bgpen(device_t &device, int bgpen)
{
	tecmo_mix_device &dev = downcast<tecmo_mix_device &>(device);
	dev.m_bgpen = bgpen;
}


void tecmo_mix_device::mix_bitmaps(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, palette_device* palette, bitmap_ind16* bitmap_bg, bitmap_ind16* bitmap_fg, bitmap_ind16* bitmap_tx, bitmap_ind16* bitmap_sp)
{
	//int frame = (screen.frame_number()) & 1;
	// note this game has no tx layer, comments relate to other drivers

	int y, x;
	const pen_t *paldata = palette->pens();

	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT32 *dd = &bitmap.pix32(y);
		UINT16 *sd2 = &bitmap_sp->pix16(y);
		UINT16 *fg = &bitmap_fg->pix16(y);
		UINT16 *bg = &bitmap_bg->pix16(y);

		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			UINT16 sprpixel = (sd2[x]);

			UINT16 m_sprpri = (sprpixel >> m_sprpri_shift) & 0x3;
			UINT16 m_sprbln = (sprpixel >> m_sprbln_shift) & 0x1;
			UINT16 m_sprcol = (sprpixel >> m_sprcol_shift) & 0xf;


			sprpixel = (sprpixel & 0xf) | (m_sprcol << 4);

			//sprpixel &= 0xff;

			UINT16 fgpixel = (fg[x]);
			UINT16 fgbln = (fgpixel & 0x0100) >> 8;
			fgpixel &= 0xff;

			UINT16 bgpixel = (bg[x]);
			bgpixel &= 0xff;

			if (sprpixel&0xf)
			{
				if (m_sprpri == (0 ^ m_revspritetile)) // behind all
				{
					if (fgpixel & 0xf) // is the fg used?
					{
						if (fgbln)
						{
							dd[x] = rand();
						}
						else
						{
							// solid FG
							dd[x] = paldata[fgpixel + m_fgregular_comp];
						}
					}
					else if (bgpixel & 0x0f)
					{
						// solid BG
						dd[x] = paldata[bgpixel + m_bgregular_comp];
					}
					else
					{
						if (m_sprbln)
						{ // sprite is blended with bgpen?
							dd[x] = rand();
						}
						else
						{
							// solid sprite
							dd[x] = paldata[sprpixel + m_spregular_comp];
						}

					}
				}
				else  if (m_sprpri == (1 ^ m_revspritetile)) // above bg, behind tx, fg
				{
					if (fgpixel & 0xf) // is the fg used?
					{
						if (fgbln)
						{
							if (m_sprbln)
							{
								// needs if bgpixel & 0xf check?

								// fg is used and blended with sprite, sprite is used and blended with bg?  -- used on 'trail' of ball when ball is under the transparent area
								dd[x] = paldata[bgpixel + m_bgblend_comp] + paldata[sprpixel + m_spblend_source]; // WRONG??
							}
							else
							{
								// fg is used and blended with opaque sprite
								dd[x] = paldata[fgpixel + m_fgblend_source] + paldata[sprpixel + m_spblend_comp];
							}
						}
						else
						{
							// fg is used and opaque
							dd[x] = paldata[fgpixel + m_fgregular_comp];
						}

					}
					else
					{
						if (m_sprbln)
						{
							// needs if bgpixel & 0xf check?

							//fg isn't used, sprite is used and blended with bg? -- used on trail of ball / flippers (looks odd)  -- some ninja gaiden enemy deaths (when behind fg) (looks ok?)  (maybe we need to check for colour saturation?)
							dd[x] = paldata[bgpixel + m_bgblend_comp] + paldata[sprpixel + m_spblend_source];
						}
						else
						{
							// fg isn't used, sprite is used and is opaque
							dd[x] = paldata[sprpixel + m_spregular_comp];
						}
					}


				}
				else if (m_sprpri == (2 ^ m_revspritetile)) // above bg,fg, behind tx
				{
					if (m_sprbln)
					{
						if (fgpixel & 0xf) // is the fg used?
						{
							if (fgbln)
							{
								// blended sprite over blended fg pixel?
								dd[x] =  rand();
							}
							else
							{
								// blended sprite over solid fgpixel?
								dd[x] = paldata[fgpixel + m_fgblend_comp] + paldata[sprpixel + m_spblend_source];
							}
						}
						else // needs if bgpixel & 0xf check?
						{
							// blended sprite over solid bg pixel
							dd[x] = paldata[bgpixel + m_bgblend_comp] + paldata[sprpixel + m_spblend_source];
						//  dd[x] =  rand();
						}



					}
					else
					{
						dd[x] = paldata[sprpixel + m_spregular_comp];
						//dd[x] = rand();
						// the bad tiles on the wildfang map (shown between levels) are drawn here.. why? looks like they should be transparent?
						// most wildfang sprites use this and are fine, so what's going wrong?
					}
				}

				else if (m_sprpri == (3 ^ m_revspritetile)) // above all?
				{
					if (m_sprbln)
					{
						// unusued by this game?
						dd[x] = rand();
					}
					else
					{
						dd[x] = paldata[sprpixel + m_spregular_comp];
					}

				}
			}
			else // NON SPRITE CASES
			{
				if (fgpixel & 0x0f)
				{
					if (fgbln)
					{
						// needs if bgpixel & 0xf check?
						dd[x] = paldata[fgpixel + m_fgblend_source] + paldata[bgpixel + m_bgblend_comp];

					}
					else
					{
						dd[x] = paldata[fgpixel + m_fgregular_comp];
					}

				}
				else if (bgpixel & 0x0f)
				{
					dd[x] = paldata[bgpixel + m_bgregular_comp];
				}
				else
				{
					dd[x] = paldata[m_bgpen];// pen 0x200 on raiga  0xb00 on spbactn
				}
			}
		}
	}
}
