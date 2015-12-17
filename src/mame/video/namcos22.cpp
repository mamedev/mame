// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, hap, R. Belmont
/**
 * video hardware for Namco System22
 *
 * TODO:
 *
 * - emulate slave dsp!
 * - texture u/v mapping is often 1 pixel off, resulting in many glitch lines/gaps between textures. The glitch may be in MAME core:
 *       it used to be much worse with the legacy_poly_manager
 * - tokyowar tanks are not shootable, same for timecris helicopter, there's still a very small hitbox but almost impossible to hit.
 *       airco22b may have a similar problem. (is this related to dsp? or cpu?)
 * - find out how/where vics num_sprites is determined exactly, currently a workaround is needed for airco22b and dirtdash
 * - improve ss22 fogging:
 *       + scene changes too rapidly sometimes, eg. dirtdash snow level finish (see attract), or aquajet going down the waterfall
 *       + 100% fog if you start dirtdash at the hill level
 * - improve ss22 lighting, eg. mountains in alpinr2b selection screen
 * - improve ss22 spot:
 *       + dirtdash spotlight is opaque for a short time when exiting the jungle level
 *       + dirtdash speedometer has wrong colors when in the jungle level
 *       + dirtdash record time message creates a 'gap' in the spotlight when entering the jungle level (possibly just a game bug?)
 * - add layer enable in system 22, see bugs in cybrcomm and victlapw
 * - window clipping is wrong in acedrvrw, victlapw
 * - ridgerac waving flag title screen is missing, just an empty beach scenery instead
 * - global offset is wrong in non-super22 servicemode video test, and above that, it flickers in acedrvrw, victlapw
 * - dirtdash polys are broken at the start section of the mountain level, maybe bad rom?
 * - alpinr2b skiier selection screen should have mirrored models (easiest to see with cursor on the red pants guy). specular reflection?
 * - propcycl scoreboard sprite part should fade out in attract mode and just before game over, fader or fog related?
 * - ridgerac fogging isn't applied to the upper/side part of the sky (best seen when driving down a hill), it's fine in ridgera2
 *       czram contents is rather odd here and partly cleared (probably the cause?):
 *        $0000-$0d7f   - gradual increase from $00-$7c
 *        $0d80-$0fff   - $73, huh, why lower?
 *        $1000-$19ff   - $00, huh!? (it's specifically cleared, memsetting czram at boot does not fix the issue)
 *        $1a00-$0dff   - $77
 *        $1e00-$1fff   - $78
 *
 * - lots of smaller issues
 *
 *
 *******************************/

#include "emu.h"
#include "includes/namcos22.h"


// poly constructor
namcos22_renderer::namcos22_renderer(namcos22_state &state)
	: poly_manager<float, namcos22_object_data, 4, 8000>(state.machine()),
		m_state(state)
	{ }

void namcos22_renderer::reset()
{
	memset(&m_scenenode_root, 0, sizeof(m_scenenode_root));
	m_scenenode_cur = nullptr;

	m_clipx = 320;
	m_clipy = 240;
	m_cliprect.set(0, 639, 0, 479);
}



/*********************************************************************************************/

// poly scanline callbacks
void namcos22_renderer::renderscanline_uvi_full(INT32 scanline, const extent_t &extent, const namcos22_object_data &extra, int threadid)
{
	float z = extent.param[0].start;
	float u = extent.param[1].start;
	float v = extent.param[2].start;
	float i = extent.param[3].start;
	float dz = extent.param[0].dpdx;
	float du = extent.param[1].dpdx;
	float dv = extent.param[2].dpdx;
	float di = extent.param[3].dpdx;
	int bn = extra.bn * 0x1000;
	const pen_t *pens = extra.pens;
	const UINT8 *czram = extra.czram;
	int cz_adjust = extra.cz_adjust;
	int cz_sdelta = extra.cz_sdelta;
	int zfog_enabled = extra.zfog_enabled;
	int fogfactor = 0xff - extra.fogfactor;
	int fadefactor = 0xff - extra.fadefactor;
	int alphafactor = 0xff - m_state.m_poly_translucency;
	rgbaint_t fogcolor = extra.fogcolor;
	rgbaint_t fadecolor = extra.fadecolor;
	rgbaint_t polycolor = extra.polycolor;
	int polyfade_enabled = extra.pfade_enabled;
	int penmask = 0xff;
	int penshift = 0;
	int prioverchar = extra.prioverchar;
	UINT32 *dest = &extra.destbase->pix32(scanline);
	UINT8 *primap = &extra.primap->pix8(scanline);
	UINT16 *ttmap = m_state.m_texture_tilemap;
	UINT8 *ttattr = m_state.m_texture_tileattr.get();
	UINT8 *ttdata = m_state.m_texture_tiledata;
	UINT8 *tt_ayx_to_pixel = m_state.m_texture_ayx_to_pixel.get();

	if (extra.cmode & 4)
	{
		pens += 0xec + ((extra.cmode & 8) << 1);
		penmask = 0x03;
		penshift = 2 * (~extra.cmode & 3);
	}
	else if (extra.cmode & 2)
	{
		pens += 0xe0 + ((extra.cmode & 8) << 1);
		penmask = 0x0f;
		penshift = 4 * (~extra.cmode & 1);
	}

	// slight differences between super and non-super, do the branch here for optimization
	// normal: 1 fader, no alpha, shading after fog
	// super:  2 faders, alpha, shading before fog
	if (m_state.m_is_ss22)
	{
		for (int x = extent.startx; x < extent.stopx; x++)
		{
			float ooz = 1.0f / z;
			INT32 tx = (int)(u * ooz);
			INT32 ty = (int)(v * ooz) + bn;
			INT32 to = ((ty & 0xfff0) << 4) | ((tx & 0xff0) >> 4);
			INT32 pen = ttdata[(ttmap[to] << 8) | tt_ayx_to_pixel[ttattr[to] << 8 | (ty << 4 & 0xf0) | (tx & 0xf)]];
			// pen = 0x55; // debug: disable textures

			rgbaint_t rgb(pens[pen >> penshift & penmask]);

			// apply shading before fog
			INT32 shade = i*ooz;
			rgb.scale_imm_and_clamp(shade << 2);

			// per-z distance fogging
			if (zfog_enabled)
			{
				int cz = ooz + cz_adjust;
				// discard low byte and clamp to 0-1fff
				if ((UINT32)cz < 0x200000) cz >>= 8;
				else cz = (cz < 0) ? 0 : 0x1fff;
				fogfactor = czram[cz] + cz_sdelta;
				if (fogfactor > 0)
				{
					if (fogfactor > 0xff) fogfactor = 0xff;
					rgb.blend(fogcolor, 0xff - fogfactor);
				}
			}
			else if (fogfactor != 0xff) // direct
			{
				rgb.blend(fogcolor, fogfactor);
			}

			if (polyfade_enabled)
			{
				rgb.scale_and_clamp(polycolor);
			}

			if (fadefactor != 0xff)
			{
				rgb.blend(fadecolor, fadefactor);
			}

			if (alphafactor != 0xff)
			{
				rgb.blend(rgbaint_t(dest[x]), alphafactor);
			}

			dest[x] = rgb.to_rgba();
			primap[x] |= prioverchar;

			u += du;
			v += dv;
			i += di;
			z += dz;
		}
	}
	else
	{
		for (int x = extent.startx; x < extent.stopx; x++)
		{
			float ooz = 1.0f / z;
			int tx = (int)(u * ooz);
			int ty = (int)(v * ooz) + bn;
			int to = ((ty & 0xfff0) << 4) | ((tx & 0xff0) >> 4);
			int pen = ttdata[(ttmap[to] << 8) | tt_ayx_to_pixel[ttattr[to] << 8 | (ty << 4 & 0xf0) | (tx & 0xf)]];
			// pen = 0x55; // debug: disable textures

			rgbaint_t rgb(pens[pen >> penshift & penmask]);

			// per-z distance fogging
			if (zfog_enabled)
			{
				int cz = ooz + cz_adjust;
				// discard low byte and clamp to 0-1fff
				if ((UINT32)cz < 0x200000) cz >>= 8;
				else cz = (cz < 0) ? 0 : 0x1fff;
				fogfactor = czram[NATIVE_ENDIAN_VALUE_LE_BE(3, 0) ^ cz];
				if (fogfactor != 0)
				{
					rgb.blend(fogcolor, 0xff - fogfactor);
				}
			}
			else if (fogfactor != 0xff) // direct
			{
				rgb.blend(fogcolor, fogfactor);
			}

			// apply shading after fog
			int shade = i*ooz;
			rgb.scale_imm_and_clamp(shade << 2);

			if (polyfade_enabled)
			{
				rgb.scale_and_clamp(polycolor);
			}

			dest[x] = rgb.to_rgba();
			primap[x] |= prioverchar;

			u += du;
			v += dv;
			i += di;
			z += dz;
		}
	}
}


void namcos22_renderer::renderscanline_sprite(INT32 scanline, const extent_t &extent, const namcos22_object_data &extra, int threadid)
{
	int y_index = extent.param[1].start - extra.flipy;
	float x_index = extent.param[0].start - extra.flipx;
	float dx = extent.param[0].dpdx;
	const pen_t *pal = extra.pens;
	int prioverchar = extra.prioverchar;
	int alphafactor = extra.alpha;
	int fogfactor = 0xff - extra.fogfactor;
	int fadefactor = 0xff - extra.fadefactor;
	rgbaint_t fogcolor(extra.fogcolor);
	rgbaint_t fadecolor(extra.fadecolor);
	UINT8 *source = (UINT8 *)extra.source + y_index * extra.line_modulo;
	UINT32 *dest = &extra.destbase->pix32(scanline);
	UINT8 *primap = &extra.primap->pix8(scanline);

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		int pen = source[(int)x_index];
		if (pen != 0xff)
		{
			rgbaint_t rgb(pal[pen]);

			if (fogfactor != 0xff)
			{
				rgb.blend(fogcolor, fogfactor);
			}

			if (fadefactor != 0xff)
			{
				rgb.blend(fadecolor, fadefactor);
			}

			if (alphafactor != 0xff)
			{
				rgb.blend(rgbaint_t(dest[x]), alphafactor);
			}

			dest[x] = rgb.to_rgba();
			primap[x] |= prioverchar;
		}
		x_index += dx;
	}
}



/*********************************************************************************************/

void namcos22_renderer::poly3d_drawquad(screen_device &screen, bitmap_rgb32 &bitmap, struct namcos22_scenenode *node)
{
	namcos22_object_data &extra = object_data_alloc();
	vertex_t v[4];
	vertex_t clipv[6];
	int clipverts;
	int vertnum;

	int direct = node->data.quad.direct;
	int flags = node->data.quad.flags;
	int color = node->data.quad.color;
	int cz_adjust = node->data.quad.cz_adjust;

	extra.destbase = &bitmap;
	extra.pfade_enabled = 0;
	extra.zfog_enabled = 0;
	extra.fadefactor = 0;
	extra.fogfactor = 0;

	extra.pens = &m_state.m_palette->pen((color & 0x7f) << 8);
	extra.primap = &screen.priority();
	extra.bn = node->data.quad.texturebank;
	extra.flags = flags;
	extra.cz_adjust = cz_adjust;
	extra.cmode = node->data.quad.cmode;
	extra.prioverchar = ((node->data.quad.cmode & 7) == 1) ? 1 : 0;
	extra.prioverchar |= m_state.m_is_ss22 ? 2 : 0;

	// scene clip
	int cx = 320 + node->data.quad.vx;
	int cy = 240 + node->data.quad.vy;
	m_clipx = cx;
	m_clipy = cy;
	m_cliprect.set(cx + node->data.quad.vw, cx - node->data.quad.vw, cy + node->data.quad.vh, cy - node->data.quad.vh);
	if (m_cliprect.min_x < 0)   m_cliprect.min_x = 0;
	if (m_cliprect.max_x > 639) m_cliprect.max_x = 639;
	if (m_cliprect.min_y < 0)   m_cliprect.min_y = 0;
	if (m_cliprect.max_y > 479) m_cliprect.max_y = 479;

	// non-direct case: project and z-clip
	if (!direct)
	{
		for (vertnum = 0; vertnum < 4; vertnum++)
		{
			v[vertnum].x = node->data.quad.v[vertnum].x;
			v[vertnum].y = node->data.quad.v[vertnum].y;
			v[vertnum].p[0] = node->data.quad.v[vertnum].z;
			v[vertnum].p[1] = node->data.quad.v[vertnum].u;
			v[vertnum].p[2] = node->data.quad.v[vertnum].v;
			v[vertnum].p[3] = node->data.quad.v[vertnum].bri;
		}

		clipverts = zclip_if_less(4, v, clipv, 4, 10.0f);
		assert(clipverts <= ARRAY_LENGTH(clipv));
		if (clipverts < 3)
			return;

		for (vertnum = 0; vertnum < clipverts; vertnum++)
		{
			float ooz = 1.0f / clipv[vertnum].p[0];
			clipv[vertnum].x = m_clipx + clipv[vertnum].x * ooz;
			clipv[vertnum].y = m_clipy - clipv[vertnum].y * ooz;
			clipv[vertnum].p[0] = ooz;
			clipv[vertnum].p[1] = (clipv[vertnum].p[1] + 0.5f) * ooz;
			clipv[vertnum].p[2] = (clipv[vertnum].p[2] + 0.5f) * ooz;
			clipv[vertnum].p[3] = (clipv[vertnum].p[3] + 0.5f) * ooz;
		}
	}

	// direct case: don't clip, and treat pv->z as 1/z
	else
	{
		clipverts = 4;
		for (vertnum = 0; vertnum < 4; vertnum++)
		{
			float ooz = node->data.quad.v[vertnum].z;
			clipv[vertnum].x = m_clipx + node->data.quad.v[vertnum].x;
			clipv[vertnum].y = m_clipy - node->data.quad.v[vertnum].y;
			clipv[vertnum].p[0] = ooz;
			clipv[vertnum].p[1] = (node->data.quad.v[vertnum].u + 0.5f) * ooz;
			clipv[vertnum].p[2] = (node->data.quad.v[vertnum].v + 0.5f) * ooz;
			clipv[vertnum].p[3] = (node->data.quad.v[vertnum].bri + 0.5f) * ooz;
		}
	}

	if (m_state.m_is_ss22)
	{
		// global fade
		if (m_state.m_mixer_flags & 1)
		{
			extra.fadefactor = m_state.m_screen_fade_factor;
			extra.fadecolor.set(0, m_state.m_screen_fade_r, m_state.m_screen_fade_g, m_state.m_screen_fade_b);
		}

		// poly fade
		extra.pfade_enabled = m_state.m_poly_fade_enabled;
		extra.polycolor.set(0, m_state.m_poly_fade_r, m_state.m_poly_fade_g, m_state.m_poly_fade_b);

		/* poly fog (not completely accurate yet)

		czram contents, it's basically a big cz compare table

		testmode:
		    o_16          0    1    2    3    4    5    6    7 <  >   f8   f9   fa   fb   fc   fd   fe   ff
		    czram[0] = 1fff 1fdf 1fbf 1f9f 1f7f 1f5f 1f3f 1f1f .... 00ff 00df 00bf 009f 007f 005f 003f 001f
		    czram[1] = 0000 0000 0000 0001 0002 0003 0005 0007 .... 1e45 1e83 1ec2 1f01 1f40 1f7f 1fbf 1fff
		    czram[2] = 003f 007f 00be 00fd 013c 017b 01b9 01f7 .... 1ff9 1ffb 1ffc 1ffd 1ffe 1fff 1fff 1fff
		    czram[3] = 0000 001f 003f 005f 007f 009f 00bf 00df .... 1eff 1f1f 1f3f 1f5f 1f7f 1f9f 1fbf 1fdf

		airco22b demo mode, fog color: 76 9a c3
		    o_16          0    1    2    3    4    5    6    7 <  >   f8   f9   fa   fb   fc   fd   fe   ff
		    czram[0] = 0000 00e4 0141 0189 01c6 01fb 022c 0258 .... 13bb 13c4 13cd 13d6 13df 13e7 13f0 13f9

		alpinerd (1st course), fog color: c8 c8 c8
		    o_16          0    1    2    3    4    5    6    7 <  >   ec   ed   ee   ef   f0   f1   f2 - ff
		    czram[0] = 00c8 00ca 00cc 00ce 00d0 00d2 00d4 00d6 .... 02a0 02a2 02a4 02a6 02a8 1fff 1fff ....

		alpinr2b (1st course), fog color: ff ff ff
		alpinr2b start of race: - gets gradually filled from left to right, initial contents filled with 1fff? - game should be foggy here
		    o_16          0    1    2    3    4    5    6    7 <  >   67   68   69   6a   6b   6c   6d - ff
		    czram[0] = 01cd 01d7 01e1 01eb 01f5 01ff 0209 0213 .... 05d3 05dd 05e7 05f1 05fb 1fff 1fff ....
		    other banks unused, zerofilled
		alpinr2b mid race: - gets gradually filled from right to left, initial contents above - game should not be foggy here
		    o_16          0    1    2    3    4    5    6    7 <  >   ec   ed   ee   ef   f0   f1   f2 - ff
		    czram[0] = 1ffe 1fff 1fff 1fff 1fff 1fff 1fff 1fff .... 1fff 1fff 1fff 1fff 1fff 1fff 1fff 1fff

		cybrcycc (1st course), fog color: 80 80 c0 - 2nd course has same cz table, but fog color 00 00 00
		    o_16          0    1    2    3    4    5    6    7 <  >   d4   d5   d6   d7   d8   d9   da - ff
		    czram[0] = 0000 0011 0021 0031 0041 0051 0060 0061 .... 04e0 04e4 04e7 04eb 04ee 1fff 1fff ....

		tokyowar, fog color: 80 c0 ff - it uses cztype 1 too by accident? (becomes fogfactor 0)
		    o_16          0    1    2    3    4    5    6    7 <  >   f8   f9   fa   fb   fc   fd   fe   ff
		    czram[0] = 0000 01c5 0244 029f 02e7 0325 035b 038b .... 0eaf 0ec7 0ee0 0efc 0f1b 0f3f 0f6a 0faa
		    czram[1] = 0000 0000 0000 0000 0000 0000 0000 0000 .... 0000 0000 0000 0000 0000 0000 0000 0000
		    czram[2] = 0000 0000 0000 0000 0000 0000 0000 0000 .... 0000 0000 0000 0000 0000 0000 0000 0000
		    czram[3] = 0000 00e8 0191 0206 0265 02b7 0301 0345 .... 1c7e 1cbc 1d00 1d4a 1d9c 1dfb 1e70 1f19

		*/

		/*  czattr: - assumed that it's write-only
		       0    2    4    6    8    a    c    e
		    ^^^^ ^^^^ ^^^^ ^^^^                        cz offset, signed16 per cztype 0,1,2,3
		                        ^^^^                   flags, nybble per cztype 3,2,1,0 - 4 probably means enable
		                             ^^^^              maincpu ram access bank
		                                  ^^^^         flags, nybble per cztype 3,2,1,0 - ?
		                                       ^^^^    ? (only set sometimes in timecris)
		    0000 0000 0000 0000 7555 0000 00e4 0000 // testmode normal - 0=white to black(mid), 1=white to black(weak), 2=white to black(strong), 3=black to white(mid, reverse of 0)
		    7fff 8000 7fff 8000 7555 0000 00e4 0000 // testmode offset - 0=black, 1=white, 2=black, 3=white
		    0000 0000 0000 0000 3111 0000 00e4 0000 // testmode off    - 0=white, 1=white, 2=white, 3=white
		    0000 0000 0000 0000 4444 0000 0000 0000 // propcycl solitar
		    0004 0004 0004 0004 4444 0000 0000 0000 // propcycl out pool
		    00a4 00a4 00a4 00a4 4444 0000 0000 0000 // propcycl in pool
		    ff80 ff80 ff80 ff80 4444 0000 0000 0000 // propcycl ending
		    ff80 ff80 ff80 ff80 0000 0000 0000 0000 // propcycl hs entry
		    0000 0000 0000 0000 0b6c 0000 00e4 0000 // cybrcycc
		    0000 0000 0000 0000 5554 0000 00e4 0000 // airco22b
		    ff01 ff01 0000 0000 4444 0000 0000 0000 // alpinerd
		    0000 0000 0000 0000 4455 0000 000a 0000 // alpinr2b
		    8001 8001 0000 0000 1111 0000 5555 0000 // aquajet (reg 8 is either 1111 or 5555, reg c is usually interlaced)
		    0000 0000 0000 0000 5554 0000 0000 0000 // tokyowar
		*/
		if (~color & 0x80)
		{
			int cztype = flags & 3;
			if (nthword(m_state.m_czattr, 4) & (4 << (cztype * 4)))
			{
				int delta = (INT16)nthword(m_state.m_czattr, cztype);
				extra.fogcolor.set(0, m_state.m_fog_r, m_state.m_fog_g, m_state.m_fog_b);
				if (direct)
				{
					int cz = ((flags & 0x1fff00) + cz_adjust) >> 8;
					if (cz < 0) cz = 0;
					else if (cz > 0x1fff) cz = 0x1fff;

					int fogfactor = m_state.m_recalc_czram[cztype][cz] + delta;
					if (fogfactor > 0)
					{
						if (fogfactor > 0xff) fogfactor = 0xff;
						extra.fogfactor = fogfactor;
					}
				}
				else
				{
					extra.zfog_enabled = 1;
					extra.cz_sdelta = delta;
					extra.czram = m_state.m_recalc_czram[cztype];
				}
			}
		}
	}

	else
	{
		// global fade
		if (m_state.m_mixer_flags & 1)
		{
			extra.pfade_enabled = m_state.m_poly_fade_enabled;
			extra.polycolor.set(0, m_state.m_poly_fade_r, m_state.m_poly_fade_g, m_state.m_poly_fade_b);
		}

		// poly fog
		if (~color & 0x80)
		{
			int cztype = flags & 3;
			int czcolor = cztype & nthbyte(&m_state.m_fog_colormask, cztype);
			extra.fogcolor.set(0, m_state.m_fog_r_per_cztype[czcolor], m_state.m_fog_g_per_cztype[czcolor], m_state.m_fog_b_per_cztype[czcolor]);

			if (direct)
			{
				// direct case, cz value is preset
				int cz = ((flags & 0x1fff00) + cz_adjust) >> 8;
				if (cz < 0) cz = 0;
				else if (cz > 0x1fff) cz = 0x1fff;
				extra.fogfactor = nthbyte(m_state.m_czram, cztype << 13 | cz);
			}
			else
			{
				extra.zfog_enabled = 1;
				extra.czram = (UINT8*)&m_state.m_czram[cztype << (13-2)];
			}
		}
	}

	render_triangle_fan(m_cliprect, render_delegate(FUNC(namcos22_renderer::renderscanline_uvi_full), this), 4, clipverts, clipv);
}


void namcos22_renderer::poly3d_drawsprite(
	screen_device &screen,
	bitmap_rgb32 &dest_bmp,
	UINT32 code,
	UINT32 color,
	int flipx, int flipy,
	int sx, int sy,
	int scalex, int scaley,
	int cz_factor,
	int prioverchar,
	int alpha
)
{
	gfx_element *gfx = m_state.m_gfxdecode->gfx(2);
	int sprite_screen_height = (scaley * gfx->height() + 0x8000) >> 16;
	int sprite_screen_width = (scalex * gfx->width() + 0x8000) >> 16;
	if (sprite_screen_width && sprite_screen_height)
	{
		float fsx = sx;
		float fsy = sy;
		float fwidth = gfx->width();
		float fheight = gfx->height();
		float fsw = sprite_screen_width;
		float fsh = sprite_screen_height;

		namcos22_object_data &extra = object_data_alloc();
		vertex_t vert[4];

		extra.fadefactor = 0;
		extra.fogfactor = 0;
		extra.flags = 0;

		extra.destbase = &dest_bmp;
		extra.alpha = alpha;
		extra.prioverchar = 2 | prioverchar;
		extra.line_modulo = gfx->rowbytes();
		extra.flipx = flipx;
		extra.flipy = flipy;
		extra.pens = &m_state.m_palette->pen(gfx->colorbase() + gfx->granularity() * (color & 0x7f));
		extra.primap = &screen.priority();
		extra.source = gfx->get_data(code % gfx->elements());

		vert[0].x = fsx;
		vert[0].y = fsy;
		vert[0].p[0] = 0;
		vert[0].p[1] = 0;
		vert[1].x = fsx + fsw;
		vert[1].y = fsy;
		vert[1].p[0] = fwidth;
		vert[1].p[1] = 0;
		vert[2].x = fsx + fsw;
		vert[2].y = fsy + fsh;
		vert[2].p[0] = fwidth;
		vert[2].p[1] = fheight;
		vert[3].x = fsx;
		vert[3].y = fsy + fsh;
		vert[3].p[0] = 0;
		vert[3].p[1] = fheight;

		// global fade
		if (m_state.m_mixer_flags & 2)
		{
			extra.fadefactor = m_state.m_screen_fade_factor;
			extra.fadecolor.set(0, m_state.m_screen_fade_r, m_state.m_screen_fade_g, m_state.m_screen_fade_b);
		}

		// fog, 0xfe is a special case for sprite priority over textlayer
		if (~color & 0x80 && cz_factor > 0 && cz_factor != 0xfe)
		{
			// or does it fetch from poly-cz ram? that will break timecris though
			extra.fogfactor = cz_factor;
			extra.fogcolor.set(0, m_state.m_fog_r, m_state.m_fog_g, m_state.m_fog_b);
		}

		render_triangle_fan(m_cliprect, render_delegate(FUNC(namcos22_renderer::renderscanline_sprite), this), 2, 4, vert);
	}
}


void namcos22_renderer::render_sprite(screen_device &screen, bitmap_rgb32 &bitmap, struct namcos22_scenenode *node)
{
	// scene clip
	m_cliprect.set(node->data.sprite.cx_min, node->data.sprite.cx_max, node->data.sprite.cy_min, node->data.sprite.cy_max);
	if (m_cliprect.min_x < 0)   m_cliprect.min_x = 0;
	if (m_cliprect.max_x > 639) m_cliprect.max_x = 639;
	if (m_cliprect.min_y < 0)   m_cliprect.min_y = 0;
	if (m_cliprect.max_y > 479) m_cliprect.max_y = 479;

	int offset = 0;

	for (int row = 0; row < node->data.sprite.rows; row++)
	{
		for (int col = 0; col < node->data.sprite.cols; col++)
		{
			int code = node->data.sprite.tile;
			if (node->data.sprite.linktype == 0xff)
				code += offset;
			else
				code += nthword(&m_state.m_spriteram[0x800/4], offset + node->data.sprite.linktype*4);

			poly3d_drawsprite(
				screen,
				bitmap,
				code,
				node->data.sprite.color,
				node->data.sprite.flipx,
				node->data.sprite.flipy,
				node->data.sprite.xpos + col * node->data.sprite.sizex,
				node->data.sprite.ypos + row * node->data.sprite.sizey,
				(node->data.sprite.sizex << 16) / 32,
				(node->data.sprite.sizey << 16) / 32,
				node->data.sprite.cz,
				node->data.sprite.pri,
				0xff - node->data.sprite.translucency
			);
			offset++;
		}
	}
}



/*********************************************************************************************/

void namcos22_renderer::free_scenenode(struct namcos22_scenenode *node)
{
	node->next = m_scenenode_cur;
	m_scenenode_cur = node;
}

struct namcos22_scenenode *namcos22_renderer::alloc_scenenode(running_machine &machine, struct namcos22_scenenode *node)
{
	if (node)
	{
		/* use free pool */
		m_scenenode_cur = node->next;
	}
	else
	{
		node = auto_alloc(machine, struct namcos22_scenenode);
	}
	memset(node, 0, sizeof(*node));
	return node;
}

struct namcos22_scenenode *namcos22_renderer::new_scenenode(running_machine &machine, UINT32 zsort, namcos22_scenenode_type type)
{
	struct namcos22_scenenode *node = &m_scenenode_root;
	struct namcos22_scenenode *prev = nullptr;
	int hash = 0;

	for (int i = 0; i < 24; i += NAMCOS22_RADIX_BITS)
	{
		hash = (zsort >> 20) & NAMCOS22_RADIX_MASK;
		struct namcos22_scenenode *next = node->data.nonleaf.next[hash];
		if (!next)
		{
			/* lazily allocate tree node for this radix */
			next = alloc_scenenode(machine, m_scenenode_cur);
			next->type = NAMCOS22_SCENENODE_NONLEAF;
			node->data.nonleaf.next[hash] = next;
		}
		prev = node;
		node = next;
		zsort <<= NAMCOS22_RADIX_BITS;
	}

	if (node->type == NAMCOS22_SCENENODE_NONLEAF)
	{
		/* first leaf allocation on this branch */
		node->type = type;
		return node;
	}
	else
	{
		struct namcos22_scenenode *leaf = alloc_scenenode(machine, m_scenenode_cur);
		leaf->type = type;
		leaf->next = node;
		prev->data.nonleaf.next[hash] = leaf;
		return leaf;
	}
}

void namcos22_renderer::render_scene_nodes(screen_device &screen, bitmap_rgb32 &bitmap, struct namcos22_scenenode *node)
{
	if (node)
	{
		if (node->type == NAMCOS22_SCENENODE_NONLEAF)
		{
			for (int i = NAMCOS22_RADIX_BUCKETS - 1; i >= 0; i--)
			{
				render_scene_nodes(screen, bitmap, node->data.nonleaf.next[i]);
			}
			free_scenenode(node);
		}
		else
		{
			while (node)
			{
				struct namcos22_scenenode *next = node->next;

				switch (node->type)
				{
					case NAMCOS22_SCENENODE_QUAD:
						poly3d_drawquad(screen, bitmap, node);
						break;

					case NAMCOS22_SCENENODE_SPRITE:
						render_sprite(screen, bitmap, node);
						break;

					default:
						fatalerror("invalid node->type\n");
				}
				free_scenenode(node);
				node = next;
			}
		}
	}
}

void namcos22_renderer::render_scene(screen_device &screen, bitmap_rgb32 &bitmap)
{
	struct namcos22_scenenode *node = &m_scenenode_root;
	for (int i = NAMCOS22_RADIX_BUCKETS - 1; i >= 0; i--)
	{
		render_scene_nodes(screen, bitmap, node->data.nonleaf.next[i]);
		node->data.nonleaf.next[i] = nullptr;
	}

	m_clipx = 320;
	m_clipy = 240;
	m_cliprect.set(0, 639, 0, 479);

	wait("render_scene");
}



/*********************************************************************************************/

// slave dsp render

float namcos22_state::dspfloat_to_nativefloat(UINT32 val)
{
	INT16 mantissa = (INT16)val;
	float result = mantissa;//?((float)mantissa):((float)0x10000);
	int exponent = (val >> 16) & 0xff;
	while (exponent < 0x2e)
	{
		result /= 2.0f;
		exponent++;
	}
	return result;
}

/* modal rendering properties */
void namcos22_state::matrix3d_multiply(float a[4][4], float b[4][4])
{
	float temp[4][4];

	for (int row = 0; row < 4;  row++)
	{
		for (int col = 0; col < 4; col++)
		{
			float sum = 0.0f;
			for (int i = 0; i < 4; i++)
			{
				sum += a[row][i] * b[i][col];
			}
			temp[row][col] = sum;
		}
	}

	memcpy(a, temp, sizeof(temp));
}

void namcos22_state::matrix3d_identity(float m[4][4])
{
	for (int r = 0; r < 4; r++)
	{
		for (int c = 0; c < 4; c++)
		{
			m[r][c] = (r == c) ? 1.0 : 0.0;
		}
	}
}

void namcos22_state::transform_point(float *vx, float *vy, float *vz, float m[4][4])
{
	float x = *vx;
	float y = *vy;
	float z = *vz;

	*vx = m[0][0]*x + m[1][0]*y + m[2][0]*z + m[3][0];
	*vy = m[0][1]*x + m[1][1]*y + m[2][1]*z + m[3][1];
	*vz = m[0][2]*x + m[1][2]*y + m[2][2]*z + m[3][2];
}

void namcos22_state::transform_normal(float *nx, float *ny, float *nz, float m[4][4])
{
	float x = *nx;
	float y = *ny;
	float z = *nz;

	*nx = m[0][0]*x + m[1][0]*y + m[2][0]*z;
	*ny = m[0][1]*x + m[1][1]*y + m[2][1]*z;
	*nz = m[0][2]*x + m[1][2]*y + m[2][2]*z;
}

void namcos22_state::register_normals(INT32 addr, float m[4][4])
{
	for (int i = 0; i < 4; i++)
	{
		float nx = dspfixed_to_nativefloat(point_read(addr + i * 3 + 0));
		float ny = dspfixed_to_nativefloat(point_read(addr + i * 3 + 1));
		float nz = dspfixed_to_nativefloat(point_read(addr + i * 3 + 2));

		/* transform normal vector */
		transform_normal(&nx, &ny, &nz, m);
		float dotproduct = nx*m_camera_lx + ny*m_camera_ly + nz*m_camera_lz;
		if (dotproduct < 0.0f)
			dotproduct = 0.0f;

		m_LitSurfaceInfo[m_LitSurfaceCount++] = m_camera_ambient + m_camera_power * dotproduct;
	}
}


void namcos22_state::draw_direct_poly(const UINT16 *src)
{
	if (machine().video().skip_this_frame())
		return;

	int polys_enabled = m_is_ss22 ? nthbyte(m_mixer, 0x1f) & 1 : 1;
	if (!polys_enabled) return;
	/**
	* word#0:
	*    x--------------- end-of-display-list marker
	*    ----xxxxxxxxxxxx priority (lo)
	*
	* word#1:
	*    ----xxxxxxxxxxxx priority (hi)
	*
	* word#2:
	*    xxxxxxxx-------- PAL (high bit is fog enable)
	*    --------xxxx---- CMODE (color mode for texture unpack)
	*    ------------xxxx BN (texture bank)
	*
	* word#3:
	*    -xxxxxxxxxxxxx-- ZC
	*    --------------xx depth cueing table select
	*
	* for each vertex:
	*    xxxx xxxx // u,v
	*
	*    xxxx xxxx // sx,sy
	*
	*    xx-- ---- // BRI
	*    --xx xxxx // zpos
	*/
	UINT32 zsort = ((src[1] & 0xfff) << 12) | (src[0] & 0xfff);
	struct namcos22_scenenode *node = m_poly->new_scenenode(machine(), zsort, NAMCOS22_SCENENODE_QUAD);
	int cztype = src[3] & 3;

	if (m_is_ss22)
	{
		cztype ^= 3; // ? not sure, but this makes testmode look like on a pcb (only 1 pcb checked)
		node->data.quad.cmode = (src[2] & 0x00f0) >> 4;
		node->data.quad.texturebank = (src[2] & 0x000f);
	}
	else
	{
		node->data.quad.cmode = (src[0 + 4] & 0xf000) >> 12;
		node->data.quad.texturebank = (src[1 + 4] & 0xf000) >> 12;
	}
	node->data.quad.cz_adjust = m_cz_adjust;
	node->data.quad.flags = (src[3] << 6 & 0x1fff00) | cztype;
	node->data.quad.color = (src[2] & 0xff00) >> 8;
	src += 4;

	for (int i = 0; i < 4; i++)
	{
		namcos22_polyvertex *p = &node->data.quad.v[i];
		if (m_is_ss22)
		{
			p->u = src[0] >> 4;
			p->v = src[1] >> 4;
		}
		else
		{
			p->u = src[0] & 0x0fff;
			p->v = src[1] & 0x0fff;
		}

		int mantissa = (INT16)src[5];
		float zf = (float)mantissa;
		int exponent = (src[4]) & 0xff;
		if (mantissa)
		{
			while (exponent < 0x2e)
			{
				zf /= 2.0f;
				exponent++;
			}
			if (m_is_ss22)
				p->z = zf;
			else
				p->z = 1.0f / zf;
		}
		else
		{
			zf = (float)0x10000;
			exponent = 0x40 - exponent;
			while (exponent < 0x2e)
			{
				zf /= 2.0f;
				exponent++;
			}
			p->z = 1.0f / zf;
		}

		p->x = (INT16)src[2];
		p->y = -(INT16)src[3];
		p->bri = src[4] >> 8;
		src += 6;
	}

	node->data.quad.direct = 1;
	node->data.quad.vx = 0;
	node->data.quad.vy = 0;
	node->data.quad.vw = -320;
	node->data.quad.vh = -240;
}

/**
 * @brief render a single quad
 *
 * @param flags
 *     00-1.----.01-0.001- ? (always set/clear)
 *     --x-.----.----.---- ?
 *     ----.xx--.----.---- cz table
 *     ----.--xx.----.---- representative z algorithm?
 *     ----.----.--x-.---- backface cull enable
 *     ----.----.----.---x ?
 *
 *      1163 // sky
 *      1262 // score (front)
 *      1242 // score (hinge)
 *      1243 // ?
 *      1063 // n/a
 *      1243 // various (2-sided?)
 *      1263 // everything else (1-sided?)
 *      1663 // ?
 *
 * @param color
 *      xxxxxxxx -------- -------- flat shading factor
 *      -------- x------- -------- fog enable
 *      -------- -xxxxxxx -------- palette select
 *      -------- -------- xxxxxxxx unused?
 *
 * @param polyshift
 *    0x1fbd0 - sky+sea
 *    0x0c350 - mountains
 *    0x09c40 - boats, surf, road, buildings
 *    0x07350 - guardrail
 *    0x061a8 - red car
 */
void namcos22_state::blit_single_quad(bitmap_rgb32 &bitmap, UINT32 color, UINT32 addr, float m[4][4], INT32 polyshift, int flags, int packetformat)
{
	int absolute_priority = m_absolute_priority;
	INT32 zsort;
	float zmin = 0.0f;
	float zmax = 0.0f;
	namcos22_polyvertex v[4];
	int i;

	for (i = 0; i < 4; i++)
	{
		namcos22_polyvertex *pv = &v[i];
		pv->x = point_read(0x8 + i * 3 + addr);
		pv->y = point_read(0x9 + i * 3 + addr);
		pv->z = point_read(0xa + i * 3 + addr);
		transform_point(&pv->x, &pv->y, &pv->z, m);
	}

	/* backface cull one-sided polygons */
	if (flags & 0x0020 &&
		(v[2].x*((v[0].z*v[1].y)-(v[0].y*v[1].z)))+
		(v[2].y*((v[0].x*v[1].z)-(v[0].z*v[1].x)))+
		(v[2].z*((v[0].y*v[1].x)-(v[0].x*v[1].y))) >= 0 &&

		(v[0].x*((v[2].z*v[3].y)-(v[2].y*v[3].z)))+
		(v[0].y*((v[2].x*v[3].z)-(v[2].z*v[3].x)))+
		(v[0].z*((v[2].y*v[3].x)-(v[2].x*v[3].y))) >= 0)
	{
		return;
	}

	for (i = 0; i < 4; i++)
	{
		namcos22_polyvertex *pv = &v[i];
		int bri;

		pv->u = point_read(0 + i * 2 + addr);
		pv->v = point_read(1 + i * 2 + addr);

		if (i == 0 || pv->z > zmax) zmax = pv->z;
		if (i == 0 || pv->z < zmin) zmin = pv->z;

		if (m_LitSurfaceCount)
		{
			// lighting (prelim)
			bri = m_LitSurfaceInfo[m_LitSurfaceIndex % m_LitSurfaceCount];
			if (m_SurfaceNormalFormat == 0x6666)
			{
				if (i == 3)
					m_LitSurfaceIndex++;
			}
			else if (m_SurfaceNormalFormat == 0x4000)
				m_LitSurfaceIndex++;
			else
				logerror("unknown normal format: 0x%x\n", m_SurfaceNormalFormat);
		}
		else if (packetformat & 0x40)
		{
			// gourad shading
			bri = (point_read(i + addr) >> 16) & 0xff;
		}
		else
		{
			// flat shading
			bri = color >> 16 & 0xff;
		}

		pv->bri = bri;
	}

	if (zmin < 0.0f) zmin = 0.0f;
	if (zmax < 0.0f) zmax = 0.0f;

	switch (flags & 0x300)
	{
		case 0x000:
			zsort = (INT32)zmin;
			break;

		case 0x100:
			zsort = (INT32)zmax;
			break;

		default:
			zsort = (INT32)((zmin + zmax) / 2.0f);
			break;
	}

	/* relative: representative z + shift values
	* 1x.xxxx.xxxxxxxx.xxxxxxxx fixed z value
	* 0x.xx--.--------.-------- absolute priority shift
	* 0-.--xx.xxxxxxxx.xxxxxxxx z-representative value shift
	*/
	if (polyshift & 0x200000)
		zsort = polyshift & 0x1fffff;
	else
	{
		zsort += signed18(polyshift);
		absolute_priority += (polyshift & 0x1c0000) >> 18;
	}

	if (m_objectshift & 0x200000)
		zsort = m_objectshift & 0x1fffff;
	else
	{
		zsort += signed18(m_objectshift);
		absolute_priority += (m_objectshift & 0x1c0000) >> 18;
	}

	if (zsort < 0) zsort = 0;
	else if (zsort > 0x1fffff) zsort = 0x1fffff;
	absolute_priority &= 7;
	zsort |= (absolute_priority << 21);

	// allocate quad
	struct namcos22_scenenode *node = m_poly->new_scenenode(machine(), zsort, NAMCOS22_SCENENODE_QUAD);
	node->data.quad.cmode = (v[0].u >> 12) & 0xf;
	node->data.quad.texturebank = (v[0].v >> 12) & 0xf;
	node->data.quad.color = (color >> 8) & 0xff;
	node->data.quad.flags = flags >> 10 & 3;
	node->data.quad.cz_adjust = m_cz_adjust;

	for (i = 0; i < 4; i++)
	{
		namcos22_polyvertex *p = &node->data.quad.v[i];
		p->x = v[i].x * m_camera_zoom;
		p->y = v[i].y * m_camera_zoom;
		p->z = v[i].z;
		p->u = v[i].u & 0xfff;
		p->v = v[i].v & 0xfff;
		p->bri = v[i].bri;
	}

	node->data.quad.direct = 0;
	node->data.quad.vx = m_camera_vx;
	node->data.quad.vy = m_camera_vy;
	node->data.quad.vw = m_camera_vw;
	node->data.quad.vh = m_camera_vh;
}


void namcos22_state::blit_quads(bitmap_rgb32 &bitmap, INT32 addr, float m[4][4], INT32 base)
{
//  int additionalnormals = 0;
	int chunklength = point_read(addr++);
	int finish = addr + chunklength;

	if (chunklength > 0x100)
		fatalerror("bad packet length\n");

	while (addr < finish)
	{
		int packetlength = point_read(addr++);
		int packetformat = point_read(addr + 0);
		int flags, color, bias;

		/**
		* packetformat:
		*      800000 final packet in chunk
		*      080000 ?
		*      020000 color word exists?
		*      010000 z-offset word exists?
		*      002000 ?
		*      001000 z-offset word exists?
		*      000400 ?
		*      000080 tex# or UV or CMODE?
		*      000040 use I
		*      000001 ?
		*/
		switch (packetlength)
		{
			case 0x17:
				/**
				* word 0: opcode (8a24c0)
				* word 1: flags
				* word 2: color
				*/
				flags = point_read(addr + 1);
				color = point_read(addr + 2);
				bias = 0;
				blit_single_quad(bitmap, color, addr + 3, m, bias, flags, packetformat);
				break;

			case 0x18:
				/**
				* word 0: opcode (0b3480 for first N-1 quads or 8b3480 for final quad in primitive)
				* word 1: flags
				* word 2: color
				* word 3: depth bias
				*/
				flags = point_read(addr + 1);
				color = point_read(addr + 2);
				bias  = point_read(addr + 3);
				blit_single_quad(bitmap, color, addr + 4, m, bias, flags, packetformat);
				break;

			case 0x10: /* vertex lighting */
				/*
				333401 (opcode)
				000000  [count] [type]
				000000  000000  007fff // normal vector
				000000  000000  007fff // normal vector
				000000  000000  007fff // normal vector
				000000  000000  007fff // normal vector
				*/
//              additionalnormals = point_read(addr+2);
				m_SurfaceNormalFormat = point_read(addr + 3);
				m_LitSurfaceCount = 0;
				m_LitSurfaceIndex = 0;
				register_normals(addr + 4, m);
				break;

			case 0x0d: /* additional normals */
				/*
				300401 (opcode)
				007b09 ffdd04 0004c2
				007a08 ffd968 0001c1
				ff8354 ffe401 000790
				ff84f7 ffdd04 0004c2
				*/
				register_normals(addr + 1, m);
				break;

			default:
				break;
		}
		addr += packetlength;
	}
}

void namcos22_state::blit_polyobject(bitmap_rgb32 &bitmap, int code, float m[4][4])
{
	UINT32 addr1 = point_read(code);
	m_LitSurfaceCount = 0;
	m_LitSurfaceIndex = 0;

	for (;;)
	{
		INT32 addr2 = point_read(addr1++);
		if (addr2 < 0)
			break;
		blit_quads(bitmap, addr2, m, code);
	}
}


/**
 * master DSP can write directly to render device via port 0xc.
 * This is used for "direct drawn" polygons, and "direct draw from point rom"
 * feature - both opcodes exist in Ridge Racer's display-list processing
 *
 * record format:
 *  header (3 words)
 *      polyshift
 *      color
 *      flags
 *
 *  per-vertex data (4*6 words)
 *      u,v
 *      sx,sy
 *      intensity;z.exponent
 *      z.mantissa
 *
 * master DSP can specify 3d objects indirectly (along with view transforms),
 * via the "transmit" PDP opcode.  the "render device" sends quad data to the slave DSP
 * viewspace clipping and projection
 *
 * most "3d object" references are 0x45 and greater.  references less than 0x45 are "special"
 * commands, using a similar point rom format.  the point rom header may point to point ram.
 *
 * slave DSP reads records via port 4
 * its primary purpose is applying lighting calculations
 * the slave DSP forwards draw commands to a "draw device"
 */

/*******************************************************************************/

/**
 * 0xfffd
 * 0x0: transform
 * 0x1
 * 0x2
 * 0x5: transform
 * >=0x45: draw primitive
 */
void namcos22_state::slavesim_handle_bb0003(const INT32 *src)
{
	/*
	    bb0003 or 3b0003

	    14.00c8            light.ambient     light.power
	    01.0000            ?                 light.dx
	    06.5a82            window priority   light.dy
	    00.a57e            ?                 light.dz

	    c8.0081            vx=200,vy=129
	    29.6092            zoom = 772.5625
	    1e.95f8 1e.95f8    0.5858154296875   0.5858154296875 // 452
	    1e.b079 1e.b079    0.6893463134765   0.6893463134765 // 532
	    29.58e8            711.25 (border? see time crisis)

	    7ffe 0000 0000
	    0000 7ffe 0000
	    0000 0000 7ffe
	*/
	m_camera_ambient = src[0x1] >> 16;
	m_camera_power = src[0x1] & 0xffff;

	m_camera_lx = dspfixed_to_nativefloat(src[0x2]);
	m_camera_ly = dspfixed_to_nativefloat(src[0x3]);
	m_camera_lz = dspfixed_to_nativefloat(src[0x4]);

	m_absolute_priority = src[0x3] >> 16;
	m_camera_vx = (INT16)(src[5] >> 16);
	m_camera_vy = (INT16)(src[5] & 0xffff);
	m_camera_zoom = dspfloat_to_nativefloat(src[6]);
	m_camera_vw = dspfloat_to_nativefloat(src[7]) * m_camera_zoom;
	m_camera_vh = dspfloat_to_nativefloat(src[9]) * m_camera_zoom;

	m_viewmatrix[0][0] = dspfixed_to_nativefloat(src[0x0c]);
	m_viewmatrix[1][0] = dspfixed_to_nativefloat(src[0x0d]);
	m_viewmatrix[2][0] = dspfixed_to_nativefloat(src[0x0e]);

	m_viewmatrix[0][1] = dspfixed_to_nativefloat(src[0x0f]);
	m_viewmatrix[1][1] = dspfixed_to_nativefloat(src[0x10]);
	m_viewmatrix[2][1] = dspfixed_to_nativefloat(src[0x11]);

	m_viewmatrix[0][2] = dspfixed_to_nativefloat(src[0x12]);
	m_viewmatrix[1][2] = dspfixed_to_nativefloat(src[0x13]);
	m_viewmatrix[2][2] = dspfixed_to_nativefloat(src[0x14]);

	transform_normal(&m_camera_lx, &m_camera_ly, &m_camera_lz, m_viewmatrix);
}

void namcos22_state::slavesim_handle_200002(bitmap_rgb32 &bitmap, const INT32 *src)
{
	if (m_PrimitiveID >= 0x45)
	{
		float m[4][4]; /* row major */

		matrix3d_identity(m);

		m[0][0] = dspfixed_to_nativefloat(src[0x1]);
		m[1][0] = dspfixed_to_nativefloat(src[0x2]);
		m[2][0] = dspfixed_to_nativefloat(src[0x3]);

		m[0][1] = dspfixed_to_nativefloat(src[0x4]);
		m[1][1] = dspfixed_to_nativefloat(src[0x5]);
		m[2][1] = dspfixed_to_nativefloat(src[0x6]);

		m[0][2] = dspfixed_to_nativefloat(src[0x7]);
		m[1][2] = dspfixed_to_nativefloat(src[0x8]);
		m[2][2] = dspfixed_to_nativefloat(src[0x9]);

		m[3][0] = src[0xa]; /* xpos */
		m[3][1] = src[0xb]; /* ypos */
		m[3][2] = src[0xc]; /* zpos */

		matrix3d_multiply(m, m_viewmatrix);
		blit_polyobject(bitmap, m_PrimitiveID, m);
	}
	else if (m_PrimitiveID != 0 && m_PrimitiveID != 2)
	{
		logerror("slavesim_handle_200002:unk code=0x%x\n", m_PrimitiveID);
		// ridgerac title screen waving flag: 0x5
	}
}

void namcos22_state::slavesim_handle_300000(const INT32 *src)
{
	m_viewmatrix[0][0] = dspfixed_to_nativefloat(src[1]);
	m_viewmatrix[1][0] = dspfixed_to_nativefloat(src[2]);
	m_viewmatrix[2][0] = dspfixed_to_nativefloat(src[3]);

	m_viewmatrix[0][1] = dspfixed_to_nativefloat(src[4]);
	m_viewmatrix[1][1] = dspfixed_to_nativefloat(src[5]);
	m_viewmatrix[2][1] = dspfixed_to_nativefloat(src[6]);

	m_viewmatrix[0][2] = dspfixed_to_nativefloat(src[7]);
	m_viewmatrix[1][2] = dspfixed_to_nativefloat(src[8]);
	m_viewmatrix[2][2] = dspfixed_to_nativefloat(src[9]);
}

void namcos22_state::slavesim_handle_233002(const INT32 *src)
{
	/*
	00233002
	   00000000 // cz adjust (signed24)
	   0003dd00 // z bias adjust
	   001fffff // far plane?
	   00007fff 00000000 00000000
	   00000000 00007fff 00000000
	   00000000 00000000 00007fff
	   00000000 00000000 00000000
	*/
	m_cz_adjust = signed24(src[1]);
	m_objectshift = src[2];
}

void namcos22_state::simulate_slavedsp(bitmap_rgb32 &bitmap)
{
	const INT32 *src = 0x300 + (INT32 *)m_polygonram.target();
	INT16 len;

	matrix3d_identity(m_viewmatrix);

	if (m_is_ss22)
	{
		src += 4; /* FFFE 0400 */
	}
	else
	{
		src--;
	}

	for (;;)
	{
		INT16 next;
		m_PrimitiveID = *src++;
		len  = (INT16)*src++;

		switch (len)
		{
			case 0x15:
				slavesim_handle_bb0003(src); /* define viewport */
				break;

			case 0x10:
				slavesim_handle_233002(src); /* set modal rendering options */
				break;

			case 0x0a:
				slavesim_handle_300000(src); /* modify view transform */
				break;

			case 0x0d:
				slavesim_handle_200002(bitmap, src); /* render primitive */
				break;

			default:
				logerror("unk 3d data(%d) addr=0x%x!", len, (int)(src-(INT32*)m_polygonram.target()));
				{
					int i;
					for (i = 0; i < len; i++)
					{
						logerror(" %06x", src[i] & 0xffffff);
					}
					logerror("\n");
				}
				return;
		}

		/* hackery! commands should be streamed, not parsed here */
		src += len;
		src++; /* always 0xffff */
		next = (INT16)*src++; /* link to next command */
		if ((next & 0x7fff) != (src - (INT32 *)m_polygonram.target()))
		{
			/* end of list */
			break;
		}
	}
}

void namcos22_state::draw_polygons(bitmap_rgb32 &bitmap)
{
	if (m_slave_simulation_active)
	{
		simulate_slavedsp(bitmap);
		m_poly->wait("draw_polygons");
	}
}



/*********************************************************************************************/

void namcos22_state::draw_sprite_group(bitmap_rgb32 &bitmap, const rectangle &cliprect, const UINT32 *src, const UINT32 *attr, int num_sprites, int deltax, int deltay, int y_lowres)
{
	for (int i = 0; i < num_sprites; i++)
	{
		/*
		src[0]
		    xxxx.xxxx.xxxx.xxxx | ----.----.----.----  x pos
		    ----.----.----.---- | xxxx.xxxx.xxxx.xxxx  y pos

		src[1]
		    xxxx.xxxx.xxxx.xxxx | ----.----.----.----  x size
		    ----.----.----.---- | xxxx.xxxx.xxxx.xxxx  y size

		src[2]
		    xxxx.x---.----.---- | ----.----.----.----  no function
		    ----.-xxx.----.---- | ----.----.----.----  clip target
		    ----.----.xxxx.xxxx | ----.----.----.----  linktype
		    ----.----.----.---- | xxxx.xx--.----.----  no function(?) - set in airco22b
		    ----.----.----.---- | ----.--x-.----.----  right justify
		    ----.----.----.---- | ----.---x.----.----  bottom justify
		    ----.----.----.---- | ----.----.x---.----  flipx
		    ----.----.----.---- | ----.----.-xxx.----  cols
		    ----.----.----.---- | ----.----.----.x---  flipy
		    ----.----.----.---- | ----.----.----.-xxx  rows

		src[3]
		    xxxx.xxxx.xxxx.xxxx | ----.----.----.----  tile number
		    ----.----.----.---- | xxxx.xxxx.----.----  translucency
		    ----.----.----.---- | ----.----.xxxx.xxxx  no function(?) - set in timecris

		attr[0]
		    xxxx.xxxx.----.---- | ----.----.----.----  no function
		    ----.----.xxxx.xxxx | xxxx.xxxx.xxxx.xxxx  z pos

		attr[1]
		    xxxx.xxxx.----.---- | ----.----.----.----  no function
		    ----.----.x---.---- | ----.----.----.----  cz enable
		    ----.----.-xxx.xxxx | ----.----.----.----  color
		    ----.----.----.---- | xxxx.xxxx.----.----  no function(?) - set in airco22b, propcycl
		    ----.----.----.---- | ----.----.xxxx.xxxx  cz factor (fog aka depth cueing)
		*/
		int xpos = (src[0] >> 16) - deltax;
		int ypos = (src[0] & 0xffff) - deltay;
		int sizex = src[1] >> 16;
		int sizey = src[1] & 0xffff;
		int flipy = src[2] >> 3 & 0x1;
		int rows = src[2] & 0x7;
		int linktype = (src[2] & 0x00ff0000) >> 16;
		int flipx = (src[2] >> 7) & 0x1;
		int cols = (src[2] >> 4) & 0x7;
		UINT32 code = src[3];
		int tile = code >> 16;
		int translucency = (code & 0xff00) >> 8;

		UINT32 zcoord = attr[0] & 0x00ffffff;
		int color = attr[1] >> 16;
		int cz = attr[1] & 0xff;

		// priority over textlayer, trusted by testmode and timecris
		int pri = ((attr[1] & 0xffff) == 0x00fe);

		// set window clipping
		int clip = src[2] >> 23 & 0xe;
		int cx_min = -deltax + (INT16)(m_spriteram[0x80|clip] >> 16);
		int cx_max = -deltax + (INT16)(m_spriteram[0x80|clip] & 0xffff);
		int cy_min = -deltay + (INT16)(m_spriteram[0x81|clip] >> 16);
		int cy_max = -deltay + (INT16)(m_spriteram[0x81|clip] & 0xffff);

		if (rows == 0) rows = 8;
		if (cols == 0) cols = 8;

		/* right justify */
		if (src[2] & 0x0200)
			xpos -= sizex * cols - 1;

		/* bottom justify */
		if (src[2] & 0x0100)
			ypos -= sizey * rows - 1;

		if (flipy)
		{
			ypos += sizey * rows - 1;
			sizey = -sizey;
		}

		if (flipx)
		{
			xpos += sizex * cols - 1;
			sizex = -sizex;
		}

		if (y_lowres)
		{
			sizey *= 2;
			ypos *= 2;
		}

		if (sizex && sizey)
		{
			struct namcos22_scenenode *node = m_poly->new_scenenode(machine(), zcoord, NAMCOS22_SCENENODE_SPRITE);

			node->data.sprite.tile = tile;
			node->data.sprite.flipx = flipx;
			node->data.sprite.flipy = flipy;
			node->data.sprite.cols = cols;
			node->data.sprite.rows = rows;
			node->data.sprite.linktype = linktype;
			node->data.sprite.xpos = xpos;
			node->data.sprite.ypos = ypos;
			node->data.sprite.cx_min = cx_min;
			node->data.sprite.cx_max = cx_max;
			node->data.sprite.cy_min = cy_min;
			node->data.sprite.cy_max = cy_max;
			node->data.sprite.sizex = sizex;
			node->data.sprite.sizey = sizey;
			node->data.sprite.translucency = translucency;
			node->data.sprite.color = color;
			node->data.sprite.cz = cz;
			node->data.sprite.pri = pri;
		}
		src += 4;
		attr += 2;
	}
}

void namcos22_state::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const UINT32 *src;
	const UINT32 *attr;

#if 0 // show reg contents
	int i;
	char msg1[0x1000]={0}, msg2[0x1000]={0};
	// 980000-98023f (spriteram header)
	for (i=0x00;i<0x02;i++) {
		sprintf(msg2,"98%04X %08X %08X %08X %08X\n",i*16,m_spriteram[i*4+0],m_spriteram[i*4+1],m_spriteram[i*4+2],m_spriteram[i*4+3]);
		strcat(msg1,msg2);
	}
	for (i=0x20;i<0x24;i++) {
		sprintf(msg2,"98%04X %08X %08X %08X %08X\n",i*16,m_spriteram[i*4+0],m_spriteram[i*4+1],m_spriteram[i*4+2],m_spriteram[i*4+3]);
		strcat(msg1,msg2);
	}
	strcat(msg1,"\n");
	// 940000-94007c (vics control)
	for (i=0x00;i<0x08;i++) {
		sprintf(msg2,"94%04X %08X %08X %08X %08X\n",i*16,m_vics_control[i*4+0],m_vics_control[i*4+1],m_vics_control[i*4+2],m_vics_control[i*4+3]);
		strcat(msg1,msg2);
	}
	if (machine().input().code_pressed(KEYCODE_S))
		popmessage("%s",msg1);
	else popmessage("[S] shows sprite/vics regs");
#endif
	/*
	    0x980000:   00060000 00010000 02ff0000 000007ff
	                   ^                                 misc control
	                    ^^^^                             base
	                         ^^^^                        base + num sprites
	                             ^^^^     ^^^^           deltax
	                                           ^^^^      deltay

	    0x980010:   00200020 028004ff 032a0509 00000000
	                ^^^^^^^^                             character size?
	                         ^^^^^^^^                    window-x related?
	                                  ^^^^^^^^           window-y related?

	    0x980200-0x98023f:   window clipping registers
	    0x980400-0x9805ff:   hzoom table
	    0x980600-0x9807ff:   vzoom table
	    0x980800-0x980fff:   link table

	    eight words per sprite, start address at 0x984000
	    additional sorting/color data for sprite at 0x9a0000
	*/

	/* misc control bits function:
	    bit 0:      sprites on
	    bit 1:      ??? (always set, except in alpinr2b. it's not x-resolution)
	    bit 2:      y-resolution? (always set, except in cybrcycc)
	*/
	int sprites_on = (m_spriteram[0] >> 16 & 1) ? 0 : 1;
	int y_lowres = (m_spriteram[0] >> 16 & 4) ? 0 : 1;

	int deltax = (m_spriteram[1] & 0xffff) + (m_spriteram[2] & 0xffff) + 0x2d;
	int deltay = (m_spriteram[3] >> 16) + (0x2a >> y_lowres);

	int base = m_spriteram[0] & 0xffff; // alpinesa/alpinr2b
	int num_sprites = ((m_spriteram[1] >> 16) - base) + 1;

	// airco22b doesn't use spriteset #1
	if (m_gametype == NAMCOS22_AIR_COMBAT22)
		num_sprites = 0;

	if (sprites_on && num_sprites > 0 && num_sprites < 0x400)
	{
		src = &m_spriteram[0x04000/4 + base*4];
		attr = &m_spriteram[0x20000/4 + base*2];
		draw_sprite_group(bitmap, cliprect, src, attr, num_sprites, deltax, deltay, y_lowres);
	}

	/* VICS RAM provides two additional banks (also many unknown regs here) */
	/*
	0x940000 -x------       sprite chip busy?
	0x940018 xxxx----       clr.w   $940018.l

	0x940030 xxxxxxxx       0x0600000 - misc control
	0x940034 xxxxxxxx       0x3070b0f

	0x940040 xxxxxxxx       sprite attribute size             high bit means busy?
	0x940048 xxxxxxxx       sprite attribute list baseaddr    high bit means busy?
	0x940050 xxxxxxxx       sprite color size                 high bit means busy?
	0x940058 xxxxxxxx       sprite color list baseaddr        high bit means busy?

	0x940060..0x94007c      set#2
	*/

	sprites_on = (m_vics_control[0x30/4] >> 24 & 1) ? 0 : 1;
	y_lowres = (m_vics_control[0x30/4] >> 24 & 4) ? 0 : 1;

	// where do the games store the number of sprites to be processed by vics???
	// the current default implementation (using spritelist size) is clearly wrong and causes problems in dirtdash and airco22b
	num_sprites = m_vics_control[0x40/4] >> 4 & 0x1ff; // no +1

	// dirtdash sprite list starts at xxx4, number of sprites is stored in xxx0, it doesn't use set#2
	if (m_gametype == NAMCOS22_DIRT_DASH)
		num_sprites = (m_vics_data[(m_vics_control[0x48/4] & 0x4000)/4] & 0xff) + 1;

	if (sprites_on && num_sprites > 0)
	{
		src = &m_vics_data[(m_vics_control[0x48/4] & 0xffff)/4];
		attr = &m_vics_data[(m_vics_control[0x58/4] & 0xffff)/4];
		draw_sprite_group(bitmap, cliprect, src, attr, num_sprites, deltax, deltay, y_lowres);
	}

	num_sprites = m_vics_control[0x60/4] >> 4 & 0x1ff; // no +1

	// airco22b number of sprites for set#2 is stored in set#1 - it does not use set 1, or main set for sprites
	if (m_gametype == NAMCOS22_AIR_COMBAT22)
	{
		sprites_on = (m_vics_data[(m_vics_control[0x48/4] & 0xffff)/4] >> 16 & 1) ? 0 : 1;
		num_sprites = (m_vics_data[(m_vics_control[0x48/4] & 0xffff)/4+1] >> 16) + 1;
	}

	if (sprites_on && num_sprites > 0)
	{
		src = &m_vics_data[(m_vics_control[0x68/4] & 0xffff)/4];
		attr = &m_vics_data[(m_vics_control[0x78/4] & 0xffff)/4];
		draw_sprite_group(bitmap, cliprect, src, attr, num_sprites, deltax, deltay, y_lowres);
	}
}

READ32_MEMBER(namcos22_state::namcos22s_vics_control_r)
{
	UINT32 ret = m_vics_control[offset];

	switch (offset*4)
	{
		// reg 0, status register?
		// high byte is read in timecris and lower half is expected to be 0
		case 0x00:
			ret = 0;
			break;

		// sprite attr/color size regs: high bit is busy/ready?
		// dirtdash reads these and waits for it to become 0
		case 0x40: case 0x50: case 0x60: case 0x70:
			ret &= 0x7fffffff;
			break;

		default:
			break;
	}
	return ret;
}

WRITE32_MEMBER(namcos22_state::namcos22s_vics_control_w)
{
	COMBINE_DATA(&m_vics_control[offset]);
}



/*********************************************************************************************/

TILE_GET_INFO_MEMBER(namcos22_state::get_text_tile_info)
{
	UINT16 data = nthword(m_textram, tile_index);
	/**
	* xxxx.----.----.---- palette select
	* ----.xx--.----.---- flip
	* ----.--xx.xxxx.xxxx code
	*/
	SET_TILE_INFO_MEMBER(0, data & 0x03ff, data >> 12, TILE_FLIPYX((data & 0x0c00) >> 10));
}

WRITE32_MEMBER(namcos22_state::namcos22_textram_w)
{
	COMBINE_DATA(&m_textram[offset]);
	m_bgtilemap->mark_tile_dirty(offset * 2);
	m_bgtilemap->mark_tile_dirty(offset * 2 + 1);
}

WRITE32_MEMBER(namcos22_state::namcos22_cgram_w)
{
	COMBINE_DATA(&m_cgram[offset]);
	m_gfxdecode->gfx(0)->mark_dirty(offset/32);
}

READ32_MEMBER(namcos22_state::namcos22_tilemapattr_r)
{
	switch (offset)
	{
		case 2:
		{
			UINT16 lo, hi = (m_tilemapattr[offset] & 0xffff0000) >> 16;
			// assume current scanline, 0x1ff if in vblank (used in alpinesa)
			// or maybe relative to posirq?
			if (m_screen->vblank()) lo = 0x1ff;
			else lo = m_screen->vpos() >> 1;
			// dirtdash has slowdowns if high bit is clear, why??
			return hi << 16 | lo | 0x8000;
		}

		case 3:
			// don't know, maybe also scanline related
			// timecris reads it everytime the gun triggers and will decline if it's 0xffff
			return 0;

		default:
			break;
	}

	return m_tilemapattr[offset];
}

WRITE32_MEMBER(namcos22_state::namcos22_tilemapattr_w)
{
	/*
	0.hiword    R/W     x offset
	0.loword    R/W     y offset, bit 9 for interlacing?(cybrcomm, tokyowar)
	1.hiword    R/W     ??? always 0x006e?
	1.loword    ?       unused?
	2.hiword    R/W     posirq scanline? - not hooked up yet
	2.loword    R       assume current scanline
	3.hiword    ?       unused?
	3.loword    R       ???
	*/
	COMBINE_DATA(&m_tilemapattr[offset]);
//  popmessage("%08x\n%08x\n%08x\n%08x\n",m_tilemapattr[0],m_tilemapattr[1],m_tilemapattr[2],m_tilemapattr[3]);
}


/**
 * Spot RAM affects how the text layer is blended with the scene, it is not yet known exactly how.
 * It isn't directly memory mapped, but rather ports are used to populate and poll it.
 *
 * See Time Crisis "SPOT RAM" self test for sample use, maybe also used in-game, but where?
 * It is also used in Dirt Dash night section. Other games don't seem to use it.
 *
 * 0x860000: set read and write address (TRUSTED by Tokyo Wars POST)
 * 0x860002: write data
 * 0x860004: read data
 * 0x860006: enable
*/

// tokyowar and timecris test ram 000-4ff, but the only practically usable part seems to be 000-3ff
#define SPOTRAM_SIZE (0x800)

/*
RAM looks like it is a 256 * 4 words table
testmode:
 offs: 0000 0001 0002 0003 - 03f4 03f5 03f6 03f7 03f8 03f9 03fa 03fb 03fc 03fd 03fe 03ff
 data: 00fe 00fe 00fe 00fe - 0001 0001 0001 0001 0000 0000 0000 0000 ffff ffff ffff ffff

is the high byte of each word used? it's usually 00, and in dirtdash always 02

low byte of each word:
 byte 0 looks like a blend factor
 bytes 1,2,3 a secondary brightness factor per rgb channel(?)

*/

READ32_MEMBER(namcos22_state::namcos22s_spotram_r)
{
	if (offset == 1)
	{
		// read
		if (m_spot_read_address >= SPOTRAM_SIZE)
		{
			m_spot_read_address = 0;
		}
		return m_spotram[m_spot_read_address++] << 16;
	}
	return 0;
}

WRITE32_MEMBER(namcos22_state::namcos22s_spotram_w)
{
	if (offset == 0)
	{
		if (ACCESSING_BITS_16_31)
		{
			// set address
			m_spot_read_address  = data >> (16 + 1);
			m_spot_write_address = data >> (16 + 1);
		}
		else
		{
			// write
			if (m_spot_write_address >= SPOTRAM_SIZE)
			{
				m_spot_write_address = 0;
			}
			m_spotram[m_spot_write_address++] = data;
		}
	}
	else
	{
		if (ACCESSING_BITS_0_15)
		{
			// enable
			m_spot_enable = data & 1;
		}
	}
}

void namcos22_state::namcos22s_mix_text_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int prival)
{
	const pen_t *pens = m_palette->pens();
	UINT16 *src;
	UINT32 *dest;
	UINT8 *pri;

	// prepare alpha
	UINT8 alpha_check12 = nthbyte(m_mixer, 0x12);
	UINT8 alpha_check13 = nthbyte(m_mixer, 0x13);
	UINT8 alpha_mask    = nthbyte(m_mixer, 0x14);
	UINT8 alpha_factor  = nthbyte(m_mixer, 0x15);

	// prepare spot
	int spot_flags = m_mixer_flags >> 16;
	bool spot_enabled = (spot_flags & 1) && (spot_flags & 0xc);
	int spot_limit = (spot_flags & 2) ? m_spot_limit : 0xff;

	// prepare fader
	bool fade_enabled = (m_mixer_flags & 2) && m_screen_fade_factor;
	int fade_factor = 0xff - m_screen_fade_factor;
	rgbaint_t fade_color(0, m_screen_fade_r, m_screen_fade_g, m_screen_fade_b);

	// mix textlayer with poly/sprites
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		src = &m_mix_bitmap->pix16(y);
		dest = &bitmap.pix32(y);
		pri = &screen.priority().pix8(y);
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			// skip if transparent or under poly/sprite
			if (pri[x] == prival)
			{
				rgbaint_t rgb(pens[src[x]]);

				// apply alpha
				if (alpha_factor)
				{
					UINT8 pen = src[x] & 0xff;
					if ((pen & 0xf) == alpha_mask || pen == alpha_check12 || pen == alpha_check13)
					{
						rgb.blend(rgbaint_t(dest[x]), 0xff - alpha_factor);
					}
				}

				// apply spot
				if (spot_enabled)
				{
					UINT8 pen = src[x] & 0xff;
					rgbaint_t mix(dest[x]);
					if (spot_flags & 8)
					{
						// mix with per-channel brightness
						rgbaint_t shade(0, (0xff - (m_spotram[pen << 2 | 1] & 0xff)) << 2, (0xff - (m_spotram[pen << 2 | 2] & 0xff)) << 2, (0xff - (m_spotram[pen << 2 | 3] & 0xff)) << 2);
						mix.scale_and_clamp(shade);
					}

					int spot_factor = 0xff - (m_spotram[pen << 2] & 0xff);
					if (spot_factor < spot_limit)
					{
						rgb.blend(mix, spot_factor);
					}
				}

				if (fade_enabled)
				{
					rgb.blend(fade_color, fade_factor);
				}

				dest[x] = rgb.to_rgba();
			}
		}
	}
}

void namcos22_state::namcos22_mix_text_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pens = m_palette->pens();
	UINT16 *src;
	UINT32 *dest;
	UINT8 *pri;

	// prepare fader and shadow factor
	bool fade_enabled = m_mixer_flags & 2 && m_poly_fade_enabled;
	bool shadow_enabled = (m_mixer_flags & 0x100) != 0; // ? (ridgerac is the only game not using shadow)

	rgbaint_t fade_color(0, m_poly_fade_r, m_poly_fade_g, m_poly_fade_b);
	rgbaint_t rgb_mix[3] = {
		rgbaint_t(0, nthbyte(m_mixer, 0x08), nthbyte(m_mixer, 0x09), nthbyte(m_mixer, 0x0a)), // pen c
		rgbaint_t(0, nthbyte(m_mixer, 0x0b), nthbyte(m_mixer, 0x0c), nthbyte(m_mixer, 0x0d)), // pen d
		rgbaint_t(0, nthbyte(m_mixer, 0x0e), nthbyte(m_mixer, 0x0f), nthbyte(m_mixer, 0x10))  // pen e
	};

	// mix textlayer with poly/sprites
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		src = &m_mix_bitmap->pix16(y);
		dest = &bitmap.pix32(y);
		pri = &screen.priority().pix8(y);
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			// skip if transparent or under poly
			if (pri[x] == 2)
			{
				// apply shadow
				rgbaint_t rgb;
				switch (src[x] & 0xff)
				{
					case 0xfc:
					case 0xfd:
					case 0xfe:
						if (shadow_enabled)
						{
							rgb.set(dest[x]);
							rgb.scale_and_clamp(rgb_mix[(src[x] & 0xf) - 0xc]);
							break;
						}
						// (fall through)
					default:
						rgb.set(pens[src[x]]);
						break;
				}

				if (fade_enabled)
				{
					rgb.scale_and_clamp(fade_color);
				}

				// BTANB note: fading to white does not affect color channels set to 00,
				// eg. a rr-gg-bb of 3f-7f-00 will fade to ff-ff-00 and not ff-ff-ff
				// seen in victlapw attract mode

				dest[x] = rgb.to_rgba();
			}
		}
	}
}

void namcos22_state::draw_text_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int scroll_x = (m_tilemapattr[0] >> 16) - 0x35c;
	int scroll_y = m_tilemapattr[0] & 0xffff;

	m_bgtilemap->set_scrollx(0, scroll_x & 0x3ff);
	m_bgtilemap->set_scrolly(0, scroll_y & 0x3ff);
	m_bgtilemap->set_palette_offset(m_text_palbase * 256);

	if (m_is_ss22)
	{
		m_bgtilemap->draw(screen, *m_mix_bitmap, cliprect, 0, 4, 4);
		namcos22s_mix_text_layer(screen, bitmap, cliprect, 4);
	}
	else
	{
		m_bgtilemap->draw(screen, *m_mix_bitmap, cliprect, 0, 2, 3);
		namcos22_mix_text_layer(screen, bitmap, cliprect);
	}
}



/*********************************************************************************************/

WRITE32_MEMBER(namcos22_state::namcos22_paletteram_w)
{
	COMBINE_DATA(&m_paletteram[offset]);
	m_dirtypal[offset & (0x7fff/4)] = 1;
}

void namcos22_state::update_palette()
{
	for (int i = 0; i < 0x8000/4; i++)
	{
		if (m_dirtypal[i])
		{
			for (int j = 0; j < 4; j++)
			{
				int which = i * 4 + j;
				int r = nthbyte(m_paletteram, which + 0x00000);
				int g = nthbyte(m_paletteram, which + 0x08000);
				int b = nthbyte(m_paletteram, which + 0x10000);
				m_palette->set_pen_color(which, rgb_t(r, g, b));
			}
			m_dirtypal[i] = 0;
		}
	}
}


WRITE32_MEMBER(namcos22_state::namcos22s_czram_w)
{
	int bank = nthword(m_czattr, 0xa/2) & 3;
	UINT32 prev = (m_banked_czram[bank][offset * 2] << 16) | m_banked_czram[bank][offset * 2 + 1];
	UINT32 temp = prev;
	COMBINE_DATA(&temp);
	data = temp;
	m_banked_czram[bank][offset * 2] = data >> 16;
	m_banked_czram[bank][offset * 2 + 1] = data & 0xffff;
	m_cz_was_written[bank] |= (prev ^ data);
}

READ32_MEMBER(namcos22_state::namcos22s_czram_r)
{
	int bank = nthword(m_czattr, 0xa/2) & 3;
	return (m_banked_czram[bank][offset * 2] << 16) | m_banked_czram[bank][offset * 2 + 1];
}

void namcos22_state::recalc_czram()
{
	for (int table = 0; table < 4; table++)
	{
		// as documented above, ss22 czram is 'just' a big compare table
		// this is very slow when emulating, so let's recalculate it to a simpler lookup table
		if (m_cz_was_written[table])
		{
			int small_val = 0x2000;
			int small_offset = 0;
			int large_val = 0;
			int large_offset = 0;
			int prev = 0x2000;

			for (int i = 0; i < 0x100; i++)
			{
				int val = m_banked_czram[table][i];

				// discard if larger than 1fff
				if (val > 0x1fff) val = prev;
				if (prev > 0x1fff)
				{
					prev = val;
					continue;
				}

				int start = prev;
				int end = val;
				if (start > end)
				{
					start = val;
					end = prev;
				}
				prev = val;

				// fill range
				for (int j = start; j < end; j++)
					m_recalc_czram[table][j] = i;

				// remember largest/smallest for later
				if (val < small_val)
				{
					small_val = val;
					small_offset = i;
				}
				if (val > large_val)
				{
					large_val = val;
					large_offset = i;
				}
			}

			// fill possible leftover ranges
			for (int j = 0; j < small_val; j++)
				m_recalc_czram[table][j] = small_offset;
			for (int j = large_val; j < 0x2000; j++)
				m_recalc_czram[table][j] = large_offset;

			m_cz_was_written[table] = 0;
		}
	}
}


void namcos22_state::update_mixer()
{
	int i;
	m_poly->wait("update_mixer");
#if 0 // show reg contents
	char msg1[0x1000] = {0}, msg2[0x1000] = {0};
	int st = 0x000 / 16;
	for (i = st; i < (st+3); i++)
	{
		sprintf(msg2,"%04X %08X %08X %08X %08X\n", i*16, m_mixer[i*4+0], m_mixer[i*4+1], m_mixer[i*4+2], m_mixer[i*4+3]);
		strcat(msg1,msg2);
	}
	if (1) // + other non-super regs
	if (!m_is_ss22)
	{
		strcat(msg1,"\n");
		for (i = 8; i <= 0x20; i += 8)
		{
			sprintf(msg2,"%04X %08X %08X %08X %08X\n", i*16, m_mixer[i*4+0], m_mixer[i*4+1], m_mixer[i*4+2], m_mixer[i*4+3]);
			strcat(msg1,msg2);
		}
	}
	popmessage("%s",msg1);
#endif

	if (m_is_ss22)
	{
/*
           0 1 2 3  4 5 6 7  8 9 a b  c d e f 10       14       18       1c
00824000: ffffff00 00000000 0000007f 00ff0000 1000ff00 0f000000 00ff007f 00010007 // time crisis
00824000: ffffff00 00000000 1830407f 00800000 0000007f 0f000000 0000037f 00010007 // trans sprite
00824000: ffffff00 00000000 3040307f 00000000 0080007f 0f000000 0000037f 00010007 // trans poly
00824000: ffffff00 00000000 1800187f 00800000 0080007f 0f000000 0000037f 00010007 // trans poly(2)
00824000: ffffff00 00000000 1800187f 00000000 0000007f 0f800000 0000037f 00010007 // trans text

    00,01,02        polygon fade rgb
    03
    04
    05,06,07        world fog rgb
    08,09,0a        background color
    0b
    0c
    0d              spot factor limit value
    0e              enable spot factor limit
    0f
    10
    11              global polygon alpha factor
    12,13           textlayer alpha pen comparison
    14              textlayer alpha pen mask?
    15              textlayer alpha factor
    16,17,18        global fade rgb
    19              global fade factor
    1a              fade target flags
    1b              textlayer palette base
    1c
    1d
    1e
    1f              layer enable
*/
		m_poly_fade_r        = nthbyte(m_mixer, 0x00);
		m_poly_fade_g        = nthbyte(m_mixer, 0x01);
		m_poly_fade_b        = nthbyte(m_mixer, 0x02); m_poly_fade_enabled = (m_poly_fade_r == 0xff && m_poly_fade_g == 0xff && m_poly_fade_b == 0xff) ? 0 : 1;
		m_fog_r              = nthbyte(m_mixer, 0x05);
		m_fog_g              = nthbyte(m_mixer, 0x06);
		m_fog_b              = nthbyte(m_mixer, 0x07);
		m_spot_limit         = nthbyte(m_mixer, 0x0d);
		m_poly_translucency  = nthbyte(m_mixer, 0x11);
		m_screen_fade_r      = nthbyte(m_mixer, 0x16);
		m_screen_fade_g      = nthbyte(m_mixer, 0x17);
		m_screen_fade_b      = nthbyte(m_mixer, 0x18);
		m_screen_fade_factor = nthbyte(m_mixer, 0x19);
		m_mixer_flags        = nthbyte(m_mixer, 0x1a);
		m_text_palbase       = nthbyte(m_mixer, 0x1b) & 0x7f;

		// put spot-specific flags into high word
		m_mixer_flags |= m_spot_enable << 16;
		m_mixer_flags |= (nthbyte(m_mixer, 0x0e) & 1) << 17;
		m_mixer_flags |= (m_chipselect & 0xc000) << 4;
	}
	else
	{
/*
90020000: 4f030000 7f00007f 4d4d4d42 0c00c0c0
90020010: c0010001 00010000 00000000 00000000
90020080: 00010101 01010102 00000000 00000000
900200c0: 00000000 00000000 00000000 03000000
90020100: fff35000 00000000 00000000 00000000
90020180: ff713700 00000000 00000000 00000000
90020200: ff100000 00000000 00000000 00000000

    00,01           display flags
    02
    03
    04
    05
    06
    07              textlayer palette base?
    08,09,0a        textlayer pen c shadow rgb
    0b,0c,0d        textlayer pen d shadow rgb
    0e,0f,10        textlayer pen e shadow rgb
    11,12           global fade factor red
    13,14           global fade factor green
    15,16           global fade factor blue
    80-87           fog color mask?
    100,180,200     fog rgb 0
    101,181,201     fog rgb 1
    102,182,202     fog rgb 2
    103,183,203     fog rgb 3
*/
		m_mixer_flags        = nthbyte(m_mixer, 0x00) << 8 | nthbyte(m_mixer, 0x01);
		m_poly_fade_r        = nthbyte(m_mixer, 0x11) << 8 | nthbyte(m_mixer, 0x12); // 0x0100 = 1.0
		m_poly_fade_g        = nthbyte(m_mixer, 0x13) << 8 | nthbyte(m_mixer, 0x14);
		m_poly_fade_b        = nthbyte(m_mixer, 0x15) << 8 | nthbyte(m_mixer, 0x16);
		m_poly_fade_enabled  = (m_poly_fade_r == 0x100 && m_poly_fade_g == 0x100 && m_poly_fade_b == 0x100) ? 0 : 1;

		// raveracw is the only game using multiple fog colors (city smog, cars under tunnels, brake disc in attract mode)
		m_fog_colormask      = m_mixer[0x84/4];

		// fog color per cz type
		for (i = 0; i < 4; i++)
		{
			m_fog_r_per_cztype[i] = nthbyte(m_mixer, 0x0100+i);
			m_fog_g_per_cztype[i] = nthbyte(m_mixer, 0x0180+i);
			m_fog_b_per_cztype[i] = nthbyte(m_mixer, 0x0200+i);
		}

		m_text_palbase = 0x7f;
	}
}



/*********************************************************************************************/

UINT32 namcos22_state::screen_update_namcos22s(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	update_mixer();
	update_palette();
	recalc_czram();
	screen.priority().fill(0, cliprect);

	// background color
	rgbaint_t bg_color(0, nthbyte(m_mixer, 0x08), nthbyte(m_mixer, 0x09), nthbyte(m_mixer, 0x0a));
	if (m_mixer_flags & 1 && m_screen_fade_factor)
	{
		rgbaint_t fade_color(0, m_screen_fade_r, m_screen_fade_g, m_screen_fade_b);
		bg_color.blend(fade_color, 0xff - m_screen_fade_factor);
	}
	bitmap.fill(bg_color.to_rgba(), cliprect);

	// layers
	UINT8 layer = nthbyte(m_mixer, 0x1f);
	if (layer & 4) draw_text_layer(screen, bitmap, cliprect);
	if (layer & 2) draw_sprites(bitmap, cliprect);
	if (layer & 1) draw_polygons(bitmap);
	m_poly->render_scene(screen, bitmap);
	if (layer & 4) namcos22s_mix_text_layer(screen, bitmap, cliprect, 6);

	// apply gamma
	const UINT8 *rlut = (const UINT8 *)&m_mixer[0x100/4];
	const UINT8 *glut = (const UINT8 *)&m_mixer[0x200/4];
	const UINT8 *blut = (const UINT8 *)&m_mixer[0x300/4];
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT32 *dest = &bitmap.pix32(y);
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			int rgb = dest[x];
			int r = rlut[NATIVE_ENDIAN_VALUE_LE_BE(3, 0) ^ ((rgb >> 16) & 0xff)];
			int g = glut[NATIVE_ENDIAN_VALUE_LE_BE(3, 0) ^ ((rgb >> 8) & 0xff)];
			int b = blut[NATIVE_ENDIAN_VALUE_LE_BE(3, 0) ^ (rgb & 0xff)];
			dest[x] = (r << 16) | (g << 8) | b;
		}
	}

	return 0;
}

UINT32 namcos22_state::screen_update_namcos22(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	update_mixer();
	update_palette();
	screen.priority().fill(0, cliprect);

	// background color
	bitmap.fill(m_palette->pen(0x7fff), cliprect);

	// layers
	draw_polygons(bitmap);
	m_poly->render_scene(screen, bitmap);
	draw_text_layer(screen, bitmap, cliprect);

	// apply gamma
	const UINT8 *rlut = &m_gamma_proms[0x000];
	const UINT8 *glut = &m_gamma_proms[0x100];
	const UINT8 *blut = &m_gamma_proms[0x200];
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT32 *dest = &bitmap.pix32(y);
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			int rgb = dest[x];
			int r = rlut[(rgb >> 16) & 0xff];
			int g = glut[(rgb >> 8) & 0xff];
			int b = blut[rgb & 0xff];
			dest[x] = (r << 16) | (g << 8) | b;
		}
	}

	return 0;
}



/*********************************************************************************************/

void namcos22_state::init_tables()
{
	m_dirtypal = std::make_unique<UINT8[]>(0x8000/4);
	memset(m_dirtypal.get(), 1, 0x8000/4);
	memset(m_paletteram, 0, 0x8000);

	memset(m_polygonram, 0xcc, m_polygonram.bytes());

	// init spotram (super22 only)
	if (m_is_ss22)
		m_spotram = auto_alloc_array_clear(machine(), UINT16, SPOTRAM_SIZE);

	// init czram tables (super22 only)
	if (m_is_ss22)
	{
		for (int table = 0; table < 4; table++)
		{
			m_banked_czram[table] = auto_alloc_array_clear(machine(), UINT16, 0x100);
			m_recalc_czram[table] = auto_alloc_array_clear(machine(), UINT8, 0x2000);
			m_cz_was_written[table] = 1;
		}
	}

	// init pointrom
	m_pointrom_size = memregion("pointrom")->bytes()/3;
	m_pointrom = auto_alloc_array(machine(), INT32, m_pointrom_size);
	UINT8* pointrom_low = memregion("pointrom")->base();
	UINT8* pointrom_mid = pointrom_low + m_pointrom_size;
	UINT8* pointrom_high = pointrom_mid + m_pointrom_size;
	for (int i = 0; i < m_pointrom_size; i++)
	{
		m_pointrom[i] = signed24(pointrom_high[i] << 16 | pointrom_mid[i] << 8 | pointrom_low[i]);
	}

	m_pointram = auto_alloc_array_clear(machine(), UINT32, 0x20000);

	// force all texture tiles to be decoded now
	for (int i = 0; i < m_gfxdecode->gfx(1)->elements(); i++)
		m_gfxdecode->gfx(1)->get_data(i);

	m_texture_tilemap = (UINT16 *)memregion("textilemap")->base();
	m_texture_tiledata = (UINT8 *)m_gfxdecode->gfx(1)->get_data(0);
	m_texture_tileattr = std::make_unique<UINT8[]>(0x080000*2);

	// unpack textures
	UINT8 *packed_tileattr = 0x200000 + (UINT8 *)memregion("textilemap")->base();
	UINT8 *unpacked_tileattr = m_texture_tileattr.get();
	for (int i = 0; i < 0x80000; i++)
	{
		*unpacked_tileattr++ = (*packed_tileattr) >> 4;
		*unpacked_tileattr++ = (*packed_tileattr) & 0xf;
		packed_tileattr++;
	}

	// make attr/y/x lookup table
	m_texture_ayx_to_pixel = std::make_unique<UINT8[]>(16*16*16);
	for (int attr = 0; attr < 16; attr++)
	{
		for (int y = 0; y < 16; y++)
		{
			for (int x = 0; x < 16; x++)
			{
				int ix = x, iy = y;

				if (attr & 4)
					ix = 15 - ix;

				if (attr & 2)
					iy = 15 - iy;

				if (attr & 8)
				{
					int temp = ix;
					ix = iy;
					iy = temp;
				}

				m_texture_ayx_to_pixel[attr << 8 | y << 4 | x] = (iy << 4) | ix;
			}
		}
	}

	// following setup is Namco System 22 specific
	switch (m_gametype)
	{
		case NAMCOS22_RIDGE_RACER:
		case NAMCOS22_RIDGE_RACER2:
		case NAMCOS22_ACE_DRIVER:
		case NAMCOS22_CYBER_COMMANDO:
		{
			for (int i = 0; i < 0x100000; i++)
			{
				int tile = m_texture_tilemap[i];
				int attr = m_texture_tileattr[i];
				if ((attr & 0x1) == 0)
				{
					tile = (tile & 0x3fff) | 0x8000;
					m_texture_tilemap[i] = tile;
				}
			}
			break;
		}

		default:
			break;
	}
}


void namcos22_state::video_start()
{
	m_is_ss22 = (m_iomcu == nullptr);
	init_tables();

	m_mix_bitmap = std::make_unique<bitmap_ind16>(640, 480);
	m_bgtilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(namcos22_state::get_text_tile_info), this), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_bgtilemap->set_transparent_pen(0xf);

	m_gfxdecode->gfx(0)->set_source((UINT8 *)m_cgram.target());

	m_poly = auto_alloc(machine(), namcos22_renderer(*this));
}
