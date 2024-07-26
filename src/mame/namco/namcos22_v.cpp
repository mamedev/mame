// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, hap, R. Belmont
/*

     Namco System22 + System Super22 video hardware emulation

*/

#include "emu.h"
#include "namcos22.h"


// poly constructor

namcos22_renderer::namcos22_renderer(namcos22_state &state) :
	poly_manager<poly3d_t, namcos22_object_data, 4>(state.machine()),
	m_state(state)
	{
		init();
	}

void namcos22_renderer::init()
{
	memset(&m_scenenode_root, 0, sizeof(m_scenenode_root));
	m_scenenode_cur = nullptr;
}


// poly scanline callbacks

// differences between super and non-super
// normal: per-poly fog, shading after fog, global fader (handled elsewhere), no alpha
// super:  shading before fog, per-z fog, 2 faders, alpha, sprites in a separate callback

void namcos22_renderer::renderscanline_poly(int32_t scanline, const extent_t &extent, const namcos22_object_data &extra, int threadid)
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
	int fogfactor = 0xff - extra.fogfactor;
	bool shade_enabled = extra.shade_enabled;
	bool texture_enabled = extra.texture_enabled;
	rgbaint_t fogcolor = extra.fogcolor;
	int prioverchar = extra.prioverchar;
	int penmask = 0xff;
	int penshift = 0;
	int pen = 0;
	rgbaint_t rgb;

	u32 *const dest = &extra.destbase->pix(scanline);
	u8 *const primap = &extra.primap->pix(scanline);
	u16 *const ttmap = m_state.m_texture_tilemap;
	u8 *const ttattr = m_state.m_texture_tileattr.get();
	u8 *const ttdata = m_state.m_texture_tiledata;
	u8 *const tt_ayx_to_pixel = m_state.m_texture_ayx_to_pixel.get();

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

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		float ooz = 1.0f / z;

		// texture mapping
		if (texture_enabled)
		{
			int tx = int(u * ooz) & 0xfff;
			int ty = (int(v * ooz) & 0xfff) | bn;
			int to = (ty << 4 & 0xfff00) | (tx >> 4);
			pen = ttdata[(ttmap[to] << 8) | tt_ayx_to_pixel[ttattr[to] << 8 | (ty << 4 & 0xf0) | (tx & 0xf)]];
			rgb.set(pens[pen >> penshift & penmask]);
		}
		else
			rgb.set(0, 0xff, 0xff, 0xff);

		// poly fog
		if (fogfactor != 0xff)
		{
			rgb.blend(fogcolor, fogfactor);
		}

		// shading after fog
		if (shade_enabled)
		{
			int shade = i * ooz;
			rgb.scale_imm_and_clamp(shade << 2);
		}

		dest[x] = rgb.to_rgba();
		primap[x] = (primap[x] & ~1) | prioverchar;

		u += du;
		v += dv;
		i += di;
		z += dz;
	}
}


void namcos22_renderer::renderscanline_poly_ss22(int32_t scanline, const extent_t &extent, const namcos22_object_data &extra, int threadid)
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
	int fogfactor = 0xff - extra.fogfactor;
	bool shade_enabled = extra.shade_enabled;
	bool texture_enabled = extra.texture_enabled;
	rgbaint_t fogcolor = extra.fogcolor;
	int prioverchar = extra.prioverchar;
	int penmask = 0xff;
	int penshift = 0;
	int pen = 0;
	rgbaint_t rgb;

	const u8 *czram = extra.czram;
	int cz_sdelta = extra.cz_sdelta;
	bool zfog_enabled = extra.zfog_enabled;
	int fadefactor = 0xff - extra.fadefactor;
	int alphafactor = 0xff - extra.alpha;
	bool alpha_enabled = extra.alpha_enabled;
	u8 alpha_pen = m_state.m_poly_alpha_pen;
	bool polyfade_enabled = extra.pfade_enabled;
	rgbaint_t fadecolor = extra.fadecolor;
	rgbaint_t polycolor = extra.polycolor;

	u32 *const dest = &extra.destbase->pix(scanline);
	u8 *const primap = &extra.primap->pix(scanline);
	u16 *const ttmap = m_state.m_texture_tilemap;
	u8 *const ttattr = m_state.m_texture_tileattr.get();
	u8 *const ttdata = m_state.m_texture_tiledata;
	u8 *const tt_ayx_to_pixel = m_state.m_texture_ayx_to_pixel.get();

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

	for (int x = extent.startx; x < extent.stopx; x++)
	{
		float ooz = 1.0f / z;

		// texture mapping
		if (texture_enabled)
		{
			int tx = int(u * ooz) & 0xfff;
			int ty = (int(v * ooz) & 0xfff) | bn;
			int to = (ty << 4 & 0xfff00) | (tx >> 4);
			pen = ttdata[(ttmap[to] << 8) | tt_ayx_to_pixel[ttattr[to] << 8 | (ty << 4 & 0xf0) | (tx & 0xf)]];
			rgb.set(pens[pen >> penshift & penmask]);
		}
		else
			rgb.set(0, 0xff, 0xff, 0xff);

		// shading before fog
		if (shade_enabled)
		{
			int shade = i * ooz;
			rgb.scale_imm_and_clamp(shade << 2);
		}

		// per-z fog
		if (zfog_enabled)
		{
			// discard low byte and clamp to 0-1fff
			int cz = int(ooz) >> 8;
			if (cz > 0x1fff) cz = 0x1fff;
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

		// fade
		if (polyfade_enabled)
		{
			rgb.scale_and_clamp(polycolor);
		}

		if (fadefactor != 0xff)
		{
			rgb.blend(fadecolor, fadefactor);
		}

		// alpha
		if (alphafactor != 0xff && (alpha_enabled || pen == alpha_pen))
		{
			rgb.blend(rgbaint_t(dest[x]), alphafactor);
		}

		dest[x] = rgb.to_rgba();
		primap[x] = (primap[x] & ~1) | prioverchar;

		u += du;
		v += dv;
		i += di;
		z += dz;
	}
}

void namcos22_renderer::renderscanline_sprite(int32_t scanline, const extent_t &extent, const namcos22_object_data &extra, int threadid)
{
	int y_index = extent.param[1].start - extra.flipy;
	float x_index = extent.param[0].start - extra.flipx;
	float dx = extent.param[0].dpdx;
	const pen_t *pal = extra.pens;
	int prioverchar = extra.prioverchar;
	int alphafactor = extra.alpha;
	bool alpha_enabled = extra.alpha_enabled;
	u8 alpha_pen = m_state.m_poly_alpha_pen;
	int fogfactor = 0xff - extra.fogfactor;
	int fadefactor = 0xff - extra.fadefactor;
	rgbaint_t fogcolor(extra.fogcolor);
	rgbaint_t fadecolor(extra.fadecolor);
	u8 *const source = (u8 *)extra.source + y_index * extra.line_modulo;
	u32 *const dest = &extra.destbase->pix(scanline);
	u8 *const primap = &extra.primap->pix(scanline);

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

			if (alphafactor != 0xff && (alpha_enabled || pen == alpha_pen))
			{
				rgb.blend(rgbaint_t(dest[x]), alphafactor);
			}

			dest[x] = rgb.to_rgba();
			primap[x] = (primap[x] & ~1) | prioverchar;
		}
		x_index += dx;
	}
}



/*********************************************************************************************/

void namcos22_renderer::poly3d_drawquad(screen_device &screen, bitmap_rgb32 &bitmap, struct namcos22_scenenode *node)
{
	vertex_t v[4];
	vertex_t clipv[6];
	int clipverts;
	int vertnum;
	int direct = node->data.quad.direct;

	// scene clip
	int cx = 320 + node->data.quad.vx;
	int cy = 240 + node->data.quad.vy;
	m_cliprect.set(cx + node->data.quad.vl, cx - node->data.quad.vr - 1, cy + node->data.quad.vu, cy - node->data.quad.vd - 1);
	m_cliprect &= screen.visible_area();

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

		clipverts = zclip_if_less<4>(4, v, clipv, 0.00001);
		assert(clipverts <= std::size(clipv));
		if (clipverts < 3)
			return;

		for (vertnum = 0; vertnum < clipverts; vertnum++)
		{
			poly3d_t ooz = 1.0 / clipv[vertnum].p[0];
			clipv[vertnum].x = cx + clipv[vertnum].x * ooz;
			clipv[vertnum].y = cy - clipv[vertnum].y * ooz;
			clipv[vertnum].p[0] = ooz;
			clipv[vertnum].p[1] = (clipv[vertnum].p[1] + 0.5) * ooz; // u
			clipv[vertnum].p[2] = (clipv[vertnum].p[2] + 0.5) * ooz; // v
			clipv[vertnum].p[3] = (clipv[vertnum].p[3] + 0.5) * ooz; // bri
		}
	}

	// direct case: don't clip, and treat pv->z as 1/z
	else
	{
		clipverts = 4;
		for (vertnum = 0; vertnum < 4; vertnum++)
		{
			poly3d_t ooz = node->data.quad.v[vertnum].z;
			clipv[vertnum].x = cx + node->data.quad.v[vertnum].x;
			clipv[vertnum].y = cy - node->data.quad.v[vertnum].y;
			clipv[vertnum].p[0] = ooz;
			clipv[vertnum].p[1] = (node->data.quad.v[vertnum].u + 0.5) * ooz;
			clipv[vertnum].p[2] = (node->data.quad.v[vertnum].v + 0.5) * ooz;
			clipv[vertnum].p[3] = (node->data.quad.v[vertnum].bri + 0.5) * ooz;
		}
	}

	int color = node->data.quad.color;
	int cz_value = node->data.quad.cz_value;
	int cz_type = node->data.quad.cz_type;
	int cz_adjust = node->data.quad.cz_adjust;
	int objectflags = node->data.quad.objectflags;

	namcos22_object_data &extra = object_data().next();

	extra.destbase = &bitmap;
	extra.pfade_enabled = false;
	extra.zfog_enabled = false;
	extra.alpha_enabled = false;
	extra.shade_enabled = true;
	extra.texture_enabled = true;
	extra.fadefactor = 0;
	extra.fogfactor = 0;

	extra.pens = &m_state.m_palette->pen((color & 0x7f) << 8);
	extra.primap = &screen.priority();
	extra.bn = node->data.quad.texturebank;
	extra.cmode = node->data.quad.cmode;
	extra.prioverchar = ((node->data.quad.cmode & 7) == 1) ? 1 : 0;
	extra.prioverchar |= m_state.m_is_ss22 ? 2 : 0;

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

		// alpha
		extra.alpha = m_state.m_poly_alpha_factor;
		extra.alpha_enabled = (color & 0x7f) != m_state.m_poly_alpha_color;

		// poly fog
		if (~color & 0x80)
		{
			int bank = m_state.m_czattr[6] >> (cz_type * 2) & 3;
			int bank_enabled = m_state.m_czattr[4] >> (bank * 4) & 4;

			if (bank_enabled)
			{
				s16 delta = m_state.m_czattr[bank];

				// sign-extend
				if (delta < 0) delta |= 0xff00;
				else delta &= 0x00ff;

				extra.fogcolor.set(0, m_state.m_fog_r, m_state.m_fog_g, m_state.m_fog_b);

				if (direct)
				{
					int fogfactor = m_state.m_recalc_czram[bank][cz_value] + delta;
					extra.fogfactor = std::clamp(fogfactor, 0, 0xff);
				}
				else
				{
					extra.zfog_enabled = true;
					extra.cz_sdelta = delta;
					extra.czram = m_state.m_recalc_czram[bank].get();
				}
			}
		}
	}
	else
	{
		// poly fog
		if (~color & 0x80)
		{
			int cz_color = cz_type & nthbyte(&m_state.m_fog_colormask, cz_type);
			extra.fogcolor.set(0, m_state.m_fog_r_per_cztype[cz_color], m_state.m_fog_g_per_cztype[cz_color], m_state.m_fog_b_per_cztype[cz_color]);
			extra.fogfactor = nthbyte(m_state.m_czram, cz_type << 13 | cz_value);
		}
	}

	// disable textures, shading (and maybe more)
	if (objectflags & 0xc00000)
	{
		extra.shade_enabled = false;
		extra.texture_enabled = false;
	}

	if (objectflags & 0x200000)
	{
		// disable textures?
		if ((cz_adjust & 0x7f0000) == 0x3a0000)
			extra.texture_enabled = false;
	}

	// disable poly fog
	if (cz_adjust & 0x800000)
	{
		extra.zfog_enabled = false;
		extra.fogfactor = 0;
	}

	if (m_state.m_is_ss22)
		render_triangle_fan<4>(m_cliprect, render_delegate(&namcos22_renderer::renderscanline_poly_ss22, this), clipverts, clipv);
	else
		render_triangle_fan<4>(m_cliprect, render_delegate(&namcos22_renderer::renderscanline_poly, this), clipverts, clipv);
}


void namcos22_renderer::poly3d_drawsprite(
	screen_device &screen,
	bitmap_rgb32 &dest_bmp,
	u32 code,
	u32 color,
	int flipx, int flipy,
	int sx, int sy,
	int scalex, int scaley,
	int cz_factor,
	int prioverchar,
	bool fade_enabled,
	int alpha
)
{
	gfx_element *gfx = m_state.m_gfxdecode->gfx(2);
	int sprite_screen_height = (scaley * gfx->height() + 0x8000) >> 16;
	int sprite_screen_width = (scalex * gfx->width() + 0x8000) >> 16;
	if (sprite_screen_width && sprite_screen_height)
	{
		poly3d_t fsx = sx;
		poly3d_t fsy = sy;
		poly3d_t fwidth = gfx->width();
		poly3d_t fheight = gfx->height();
		poly3d_t fsw = sprite_screen_width;
		poly3d_t fsh = sprite_screen_height;

		namcos22_object_data &extra = object_data().next();
		vertex_t vert[4];

		extra.fadefactor = 0;
		extra.fogfactor = 0;
		extra.destbase = &dest_bmp;
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
		if (m_state.m_mixer_flags & 2 || fade_enabled)
		{
			extra.fadefactor = m_state.m_screen_fade_factor;
			extra.fadecolor.set(0, m_state.m_screen_fade_r, m_state.m_screen_fade_g, m_state.m_screen_fade_b);
		}

		// sprite fog
		if (~color & 0x80 && cz_factor > 0)
		{
			extra.fogfactor = cz_factor;
			extra.fogcolor.set(0, m_state.m_fog_r, m_state.m_fog_g, m_state.m_fog_b);
		}

		// alpha
		extra.alpha = alpha;
		extra.alpha_enabled = (color & 0x7f) != m_state.m_poly_alpha_color;

		render_polygon<4, 2>(m_cliprect, render_delegate(&namcos22_renderer::renderscanline_sprite, this), vert);
	}
}

void namcos22_renderer::render_sprite(screen_device &screen, bitmap_rgb32 &bitmap, struct namcos22_scenenode *node)
{
	// scene clip
	m_cliprect.set(node->data.sprite.cx_min, node->data.sprite.cx_max, node->data.sprite.cy_min, node->data.sprite.cy_max);
	m_cliprect &= screen.visible_area();

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
				node->data.sprite.prioverchar,
				node->data.sprite.fade_enabled,
				0xff - node->data.sprite.alpha
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
		// use free pool
		m_scenenode_cur = node->next;
	}
	else
	{
		node = &m_scenenode_alloc.emplace_back();
	}
	memset(node, 0, sizeof(*node));
	return node;
}

struct namcos22_scenenode *namcos22_renderer::new_scenenode(running_machine &machine, u32 zsort, namcos22_scenenode_type type)
{
	struct namcos22_scenenode *node = &m_scenenode_root;
	struct namcos22_scenenode *prev = nullptr;
	int hash = 0;

	for (int i = 0; i < 24; i += NAMCOS22_RADIX_BITS)
	{
		hash = (zsort >> (24 - NAMCOS22_RADIX_BITS)) & NAMCOS22_RADIX_MASK;
		struct namcos22_scenenode *next = node->data.nonleaf.next[hash];
		if (!next)
		{
			// lazily allocate tree node for this radix
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
		// first leaf allocation on this branch
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

	wait("render_scene");
}



/*********************************************************************************************/

// slave dsp render

float namcos22_state::dspfloat_to_nativefloat(u32 val)
{
	s16 mantissa = (s16)val;
	float result = (float)mantissa;
	int exponent = (val >> 16) & 0x3f;
	while (exponent < 0x2e)
	{
		result /= 2.0f;
		exponent++;
	}
	return result;
}

// model rendering properties
void namcos22_state::matrix3d_multiply(float a[4][4], float b[4][4])
{
	float result[4][4];

	for (int row = 0; row < 4; row++)
	{
		for (int col = 0; col < 4; col++)
		{
			float sum = 0.0f;
			for (int i = 0; i < 4; i++)
			{
				sum += a[row][i] * b[i][col];
			}
			result[row][col] = sum;
		}
	}

	memcpy(a, result, sizeof(result));
}

void namcos22_state::matrix3d_identity(float m[4][4])
{
	for (int row = 0; row < 4; row++)
	{
		for (int col = 0; col < 4; col++)
		{
			m[row][col] = (row == col) ? 1.0f : 0.0f;
		}
	}
}

void namcos22_state::matrix3d_apply_reflection(float m[4][4])
{
	if (!m_reflection)
		return;

	float r[4][4];
	matrix3d_identity(r);

	if (m_reflection & 0x10)
		r[0][0] = -1.0f;
	if (m_reflection & 0x20)
		r[1][1] = -1.0f;

	matrix3d_multiply(m, r);
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

void namcos22_state::register_normals(int addr, float m[4][4])
{
	for (int i = 0; i < 4; i++)
	{
		float nx = dspfixed_to_nativefloat(point_read(addr + i * 3 + 0));
		float ny = dspfixed_to_nativefloat(point_read(addr + i * 3 + 1));
		float nz = dspfixed_to_nativefloat(point_read(addr + i * 3 + 2));

		// transform normal vector
		transform_normal(&nx, &ny, &nz, m);

		float cx = m_camera_lx;
		float cy = m_camera_ly;
		float cz = m_camera_lz;
		transform_normal(&cx, &cy, &cz, m_viewmatrix);

		float dotproduct = nx*cx + ny*cy + nz*cz;
		if (dotproduct < 0.0f)
			dotproduct = 0.0f;

		m_LitSurfaceInfo[m_LitSurfaceCount++] = m_camera_ambient + m_camera_power * dotproduct;
	}
}


void namcos22_state::draw_direct_poly(const u16 *src)
{
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
	*/
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
	u32 zsort = ((src[1] & 0xfff) << 12) | (src[0] & 0xfff);
	struct namcos22_scenenode *node = m_poly->new_scenenode(machine(), zsort, NAMCOS22_SCENENODE_QUAD);

	if (m_is_ss22)
	{
		node->data.quad.cmode = (src[2] & 0x00f0) >> 4;
		node->data.quad.texturebank = (src[2] & 0x000f);
	}
	else
	{
		node->data.quad.cmode = (src[0 + 4] & 0xf000) >> 12;
		node->data.quad.texturebank = (src[1 + 4] & 0xf000) >> 12;
	}
	node->data.quad.color = (src[2] & 0xff00) >> 8;
	node->data.quad.cz_value = src[3] >> 2 & 0x1fff;
	node->data.quad.cz_type = src[3] & 3;
	node->data.quad.cz_adjust = 0;
	node->data.quad.objectflags = 0;
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

		int mantissa = src[5];
		int exponent = src[4] & 0x3f;

		if (mantissa)
		{
			p->z = mantissa;
			while (exponent < 0x2e)
			{
				p->z /= 2.00f;
				exponent++;
			}
		}
		else
			p->z = (float)0x10000;

		p->x = (s16)src[2];
		p->y = -(s16)src[3];
		p->bri = src[4] >> 8;
		src += 6;
	}

	node->data.quad.direct = 1;
	node->data.quad.vx = 0;
	node->data.quad.vy = 0;
	node->data.quad.vu = -240;
	node->data.quad.vd = -240;
	node->data.quad.vl = -320;
	node->data.quad.vr = -320;
	return;

	// s22 testmode expects:
	node->data.quad.vx = -320;
	node->data.quad.vy = -240;
	node->data.quad.vu = 0;
	node->data.quad.vd = -480;
	node->data.quad.vl = 0;
	node->data.quad.vr = -640;
}

void namcos22_state::blit_single_quad(u32 color, u32 addr, float m[4][4], int polyshift, int flags, int packetformat)
{
	/**
	* @brief render a single quad
	*
	* @param flags
	*     00-1.----.01-0.001- ? (always set/clear)
	*     --x-.----.----.---- ?
	*     ----.xx--.----.---- cz table
	*     ----.--xx.----.---- representative z algorithm?
	*     ----.----.--x-.---- backface cull enable
	*     ----.----.----.---x lighting related?
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
	namcos22_polyvertex v[4];

	for (int i = 0; i < 4; i++)
	{
		namcos22_polyvertex *pv = &v[i];
		pv->x = point_read(0x8 + i * 3 + addr);
		pv->y = point_read(0x9 + i * 3 + addr);
		pv->z = point_read(0xa + i * 3 + addr);
		transform_point(&pv->x, &pv->y, &pv->z, m);
	}

	float zmin = 0.0f;
	float zmax = 0.0f;

	for (int i = 0; i < 4; i++)
	{
		if (i == 0 || v[i].z > zmax) zmax = v[i].z;
		if (i == 0 || v[i].z < zmin) zmin = v[i].z;
	}

	// fully behind camera
	if (zmax < 0.0f)
		return;

	// backface cull one-sided polygons
	if (flags & 0x0020)
	{
		float c1 =
			(v[2].x*((v[0].z*v[1].y)-(v[0].y*v[1].z)))+
			(v[2].y*((v[0].x*v[1].z)-(v[0].z*v[1].x)))+
			(v[2].z*((v[0].y*v[1].x)-(v[0].x*v[1].y)));
		float c2 =
			(v[0].x*((v[2].z*v[3].y)-(v[2].y*v[3].z)))+
			(v[0].y*((v[2].x*v[3].z)-(v[2].z*v[3].x)))+
			(v[0].z*((v[2].y*v[3].x)-(v[2].x*v[3].y)));

		if ((m_cullflip && c1 <= 0.0f && c2 <= 0.0f) || (!m_cullflip && c1 >= 0.0f && c2 >= 0.0f))
			return;
	}

	int zsort = 0;
	if (zmin < 0.0f) zmin = 0.0f;

	switch (flags & 0x300)
	{
		case 0x000:
			zsort = zmin + 0.5f;
			break;

		case 0x100:
			zsort = zmax + 0.5f;
			break;

		default:
			zsort = (zmin + zmax) / 2.0f + 0.5f;
			break;
	}

	if (zsort > 0x1fffff) zsort = 0x1fffff;
	int absolute_priority = m_absolute_priority & 7;

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

	zsort = std::clamp(zsort, 0, 0x1fffff);
	absolute_priority &= 7;
	zsort |= (absolute_priority << 21);

	zmax = std::clamp(zmax, 0.0f, (float)0x1fffff);
	int cz_value = zmax + 0.5f; // not from zsort

	// u, v, bri
	for (int i = 0; i < 4; i++)
	{
		int bri;

		v[i].u = point_read(0 + i * 2 + addr);
		v[i].v = point_read(1 + i * 2 + addr);

		if (m_LitSurfaceCount > 0)
		{
			// lighting (prelim)
			int index = m_LitSurfaceIndex++;
			if (m_LitSurfaceCount > 4)
				index >>= 2;
			index %= m_LitSurfaceCount;

			bri = m_LitSurfaceInfo[index];
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

		v[i].bri = bri;
	}

	// allocate quad
	struct namcos22_scenenode *node = m_poly->new_scenenode(machine(), zsort, NAMCOS22_SCENENODE_QUAD);
	node->data.quad.cmode = (v[0].u >> 12) & 0xf;
	node->data.quad.texturebank = (v[0].v >> 12) & 0xf;
	node->data.quad.color = (color >> 8) & 0xff;
	node->data.quad.cz_value = cz_value >> 8;
	node->data.quad.cz_type = flags >> 10 & 3;
	node->data.quad.cz_adjust = m_cz_adjust;
	node->data.quad.objectflags = m_objectflags;

	for (int i = 0; i < 4; i++)
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
	node->data.quad.vu = m_camera_vu;
	node->data.quad.vd = m_camera_vd;
	node->data.quad.vl = m_camera_vl;
	node->data.quad.vr = m_camera_vr;
}


void namcos22_state::blit_quads(int addr, int len, float m[4][4])
{
	int finish = addr + len;

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
				blit_single_quad(color, addr + 3, m, bias, flags, packetformat);
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
				blit_single_quad(color, addr + 4, m, bias, flags, packetformat);
				break;

			case 0x10: /* vertex lighting */
				/*
				333401 (opcode)
				000000  [count] [type]
				000000  000000  007fff // normal vector
				000000  000000  007fff // normal vector
				000000  000000  007fff // normal vector
				000000  000000  007fff // normal vector

				used in:
				- acedrive/victlap sparks
				- adillor title logo
				- alpinr2b mountains in selection screen
				- alpinr2b spinning yellow best times in attract mode
				- alpinr2b highscore entry letters
				- propcycl score/time
				- propcycl Solitar pillars
				- ridgerac car when entering highscore
				- ridgerac waving flag
				- ridgerac rotating sign before 2nd tunnel
				- timecris Sherudo's knives
				*/
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
				logerror("blit_quads unknown packet length: addr=0x%06x len=0x%x format=0x%x\n", addr-1, packetlength, packetformat);
				return;
		}
		addr += packetlength;

		if (packetformat & 0x800000 && addr != finish)
		{
			logerror("blit_quads unexpected end: addr=0x%06x\n", addr);
			return;
		}
	}
}

void namcos22_state::blit_polyobject(int code, float m[4][4])
{
	// list start address, code 5 is special case for pointram
	int list_addr;
	bool pointram = (code == 0x5);
	if (pointram)
		list_addr = (m_is_ss22) ? 0xf80000 : 0xf00000;
	else
		list_addr = point_read(code);

	m_LitSurfaceCount = 0;
	m_LitSurfaceIndex = 0;

	for (;;)
	{
		int object_addr = point_read(list_addr++);

		if (object_addr < 0)
		{
			if (object_addr == -1)
			{
				// 0xffffff is end-of-list marker
				break;
			}
			else if (!pointram)
			{
				logerror("blit_polyobject unexpected end: addr=0x%06x\n", list_addr-1);
				break;
			}
			object_addr &= 0x00ffffff;
		}

		u32 chunklength = point_read(object_addr++);
		if (chunklength > 0x100)
		{
			logerror("blit_polyobject bad chunk length: addr=0x%06x len=0x%x\n", object_addr-1, chunklength);
			break;
		}

		blit_quads(object_addr, chunklength, m);
	}

	// flag applies to single object (see timecris stage 1-3 car)
	m_objectflags &= ~0x400000;
}


/*******************************************************************************/

void namcos22_state::slavesim_handle_bb0003(const s32 *src)
{
	/*
	    bb0003 or 3b0003   opcode

	    14.00c8            light.ambient     light.power
	    01.0000            reflection,?      light.dx
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
	m_camera_ambient = src[0x1] >> 16 & 0xffff;
	m_camera_power = src[0x1] & 0xffff;

	m_camera_lx = dspfixed_to_nativefloat(src[0x2]);
	m_camera_ly = dspfixed_to_nativefloat(src[0x3]);
	m_camera_lz = dspfixed_to_nativefloat(src[0x4]);

	m_absolute_priority = src[0x3] >> 16 & 0xffff;
	m_camera_vx = signed12(src[0x5] >> 16);
	m_camera_vy = signed12(src[0x5] & 0xffff);
	m_camera_zoom = dspfloat_to_nativefloat(src[0x6]);
	m_camera_vr = dspfloat_to_nativefloat(src[0x7]) * m_camera_zoom - 0.5f;
	m_camera_vl = dspfloat_to_nativefloat(src[0x8]) * m_camera_zoom - 0.5f;
	m_camera_vu = dspfloat_to_nativefloat(src[0x9]) * m_camera_zoom - 0.5f;
	m_camera_vd = dspfloat_to_nativefloat(src[0xa]) * m_camera_zoom - 0.5f;

	m_reflection = src[0x2] >> 16 & 0x30; // z too?
	m_cullflip = (m_reflection == 0x10 || m_reflection == 0x20);

	if (m_reflection & 0x10)
	{
		int vl = m_camera_vl;
		m_camera_vl = m_camera_vr;
		m_camera_vr = vl;
	}
	if (m_reflection & 0x20)
	{
		int vu = m_camera_vu;
		m_camera_vu = m_camera_vd;
		m_camera_vd = vu;
	}

	m_viewmatrix[0][0] = dspfixed_to_nativefloat(src[0x0c]);
	m_viewmatrix[1][0] = dspfixed_to_nativefloat(src[0x0d]);
	m_viewmatrix[2][0] = dspfixed_to_nativefloat(src[0x0e]);

	m_viewmatrix[0][1] = dspfixed_to_nativefloat(src[0x0f]);
	m_viewmatrix[1][1] = dspfixed_to_nativefloat(src[0x10]);
	m_viewmatrix[2][1] = dspfixed_to_nativefloat(src[0x11]);

	m_viewmatrix[0][2] = dspfixed_to_nativefloat(src[0x12]);
	m_viewmatrix[1][2] = dspfixed_to_nativefloat(src[0x13]);
	m_viewmatrix[2][2] = dspfixed_to_nativefloat(src[0x14]);

	matrix3d_apply_reflection(m_viewmatrix);
}

void namcos22_state::slavesim_handle_200002(const s32 *src, int code)
{
	/**
	* 0xfffd
	* 0x0: transform
	* 0x1
	* 0x2
	* 0x5: draw primitive(RAM)
	* >=0x45: draw primitive(ROM)
	*/

	if (code == 0x5 || code >= 0x45)
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

		m[3][0] = signed24(src[0xa]); // xpos
		m[3][1] = signed24(src[0xb]); // ypos
		m[3][2] = signed24(src[0xc]); // zpos

		matrix3d_multiply(m, m_viewmatrix);
		blit_polyobject(code, m);
	}
	else if (code != 0 && code != 2)
	{
		logerror("slavesim_handle_200002:unknown code=0x%x\n", code);
	}
}

void namcos22_state::slavesim_handle_300000(const s32 *src)
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

	matrix3d_apply_reflection(m_viewmatrix);
}

void namcos22_state::slavesim_handle_233002(const s32 *src)
{
	/*
	    00233002 // opcode
	    00000000 // cz adjust
	    0003dd00 // z bias adjust
	    001fffff // object flags
	    00007fff 00000000 00000000
	    00000000 00007fff 00000000
	    00000000 00000000 00007fff
	    00000000 00000000 00000000

	    cz_adjust:
	    00000000: common
	    00020000: adillor arrows on level select screen (no effect?)
	    00310000: propcycl attract mode particles when Solitar rises (unknown effect)
	    00390000: "
	    003d0000: "
	    003a0000: timecris shoot helicopter (white, but shading enabled)
	    00800000: alpinr2b cancel fogging on selection screen
	    00800000: raverace cancel fogging on sky in attract mode

	    objectflags:
	    001fffff: common
	    003fffff: adillor arrows on level select screen
	    003fffff: propcycl attract mode particles when Solitar rises
	    003fffff: timecris shoot helicopter
	    005fffff: timecris shoot other destructible object (opaque white, 1 object)
	    009fffff: cybrcomm shoot enemy with machine gun (opaque white)
	*/
	m_cz_adjust = src[1];
	m_objectshift = src[2];
	m_objectflags = src[3];
}

void namcos22_state::simulate_slavedsp()
{
	/**
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
	const s32 *src = 0x300 + (s32 *)m_polygonram.target();

	if (m_is_ss22)
	{
		src += 4; // FFFE 0400
	}
	else
	{
		src--;
	}

	for (;;)
	{
		// hackery! commands should be streamed, not parsed here
		u16 code = *src++;
		u16 len = *src++;
		s32 index = src - (s32 *)m_polygonram.target();

		// alpinr2b titlescreen includes commands to modify pointram on the fly
		if (m_gametype == NAMCOS22_ALPINE_RACER_2 && code == 0xfff8)
		{
			pdp_handle_commands(index - 2);

			// skip to end for now
			src += 0x56;
			continue;
		}
		else if ((index + len) >= 0x7fff)
		{
			logerror("simulate_slavedsp buffer overflow: len=0x%x code=0x%x addr=0x%x\n", len, code, index);
			return;
		}

		switch (len)
		{
			case 0x15:
				slavesim_handle_bb0003(src); // define viewport
				break;

			case 0x10:
				slavesim_handle_233002(src); // set model rendering options
				break;

			case 0x0a:
				slavesim_handle_300000(src); // modify view transform
				break;

			case 0x0d:
				slavesim_handle_200002(src, code); // render primitive
				break;

			default:
			{
				std::ostringstream polydata;
				int i = 0;
				for (; i < len && i < 0x20; i++)
				{
					util::stream_format(polydata ," %06x", src[i] & 0xffffff);
				}
				if (i < len)
					polydata << " (...)";
				logerror("simulate_slavedsp unknown 3d data: len=0x%x code=0x%x addr=0x%x!%s\n", len, code, index, std::move(polydata).str());
				return;
			}
		}

		src += len;
		src++; // should be 0xffff (GOTO command)
		u16 next = *src++ & 0x7fff; // link to next command
		if (next != (index + len + 1 + 1))
		{
			// end of list, normally with a "goto self"
			if (next != (index + len))
				logerror("simulate_slavedsp unexpected end: addr=0x%x, next=0x%x\n", index+len, next);

			return;
		}
		else if (next == 0x7fff)
		{
			logerror("simulate_slavedsp buffer overflow: next=0x7fff\n");
			return;
		}
	}
}


void namcos22_state::draw_polygons()
{
	if (m_pdp_render_done && m_slave_simulation_active)
	{
		simulate_slavedsp();
		m_poly->wait("draw_polygons");
	}
}

void namcos22_state::render_frame_active()
{
	/**
	* Since pdp_begin and simulate_slavedsp are basically the same thing, when PDP is inactive,
	* the parser here needs to be inactive too. However, propcycl attract mode(during the pauses)
	* doesn't trigger the PDP and still expects polygons to be shown. It doesn't overwrite the PDP
	* loop address, so it must be drawing the previous framebuffer, it's not known exactly where/how
	* this is controlled.
	*/
	if (m_screen->frame_number() > m_pdp_frame && m_render_refresh)
		m_pdp_render_done = false;

	m_render_refresh = false;
}

void namcos22_state::screen_vblank(int state)
{
	if (state)
	{
		// still need to determine active state if frame was skipped
		if (m_skipped_this_frame)
			render_frame_active();
		m_skipped_this_frame = machine().video().skip_this_frame();
	}
}



/*********************************************************************************************/

void namcos22_state::draw_sprite_group(const u32 *src, const u32 *attr, int num_sprites, int deltax, int deltay, int y_lowres)
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
		    xxxx.x---.----.---- | ----.----.----.----  unused
		    ----.-xxx.----.---- | ----.----.----.----  clip target
		    ----.----.xxxx.xxxx | ----.----.----.----  linktype
		    ----.----.----.---- | xxxx.xx--.----.----  no function(?) - 0x400 set in airco22b(invisible sprite)
		    ----.----.----.---- | ----.--x-.----.----  right justify
		    ----.----.----.---- | ----.---x.----.----  bottom justify
		    ----.----.----.---- | ----.----.x---.----  flipx
		    ----.----.----.---- | ----.----.-xxx.----  cols
		    ----.----.----.---- | ----.----.----.x---  flipy
		    ----.----.----.---- | ----.----.----.-xxx  rows

		src[3]
		    xxxx.xxxx.xxxx.xxxx | ----.----.----.----  tile number
		    ----.----.----.---- | xxxx.xxxx.----.----  alpha
		    ----.----.----.---- | ----.----.xxxx.xxxx  no function(?) - set in timecris when increasing alpha, it's probably not 16bit

		attr[0]
		    xxxx.xxxx.----.---- | ----.----.----.----  unused
		    ----.----.xxxx.xxxx | xxxx.xxxx.xxxx.xxxx  z pos

		attr[1]
		    xxxx.xxxx.----.---- | ----.----.----.----  unused
		    ----.----.x---.---- | ----.----.----.----  cz enable
		    ----.----.-xxx.xxxx | ----.----.----.----  color
		    ----.----.----.---- | xxxx.xxxx.----.----  unknown flags? - set in airco22b(0x7/0x1, invisible sprite), propcycl score hinge(0xff), cybrcycc at boot(0x2b)
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
		u32 code = src[3];
		int tile = code >> 16;
		int alpha = (code & 0xff00) >> 8;

		u32 zcoord = attr[0] & 0x00ffffff;
		int color = attr[1] >> 16 & 0xff;
		int cz = attr[1] & 0xff;

		// one of these is to override global fade setting?
		// eg. propcycl time over, where global fade affects score hinge, but not "TIME UP"
		bool fade_enabled = bool(attr[1] & 0x8000);

		// priority over textlayer, trusted by testmode and timecris
		int prioverchar = (cz == 0xfe) ? 1 : 0;

		// set window clipping
		int clip = src[2] >> 23 & 0xe;
		int cx_min = -deltax + (s16)(m_spriteram[0x80|clip] >> 16);
		int cx_max = -deltax + (s16)(m_spriteram[0x80|clip] & 0xffff);
		int cy_min = -deltay + (s16)(m_spriteram[0x81|clip] >> 16);
		int cy_max = -deltay + (s16)(m_spriteram[0x81|clip] & 0xffff);

		if (rows == 0) rows = 8;
		if (cols == 0) cols = 8;

		// right justify
		if (src[2] & 0x0200)
			xpos -= sizex * cols - 1;

		// bottom justify
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
			node->data.sprite.alpha = alpha;
			node->data.sprite.color = color;
			node->data.sprite.cz = cz;
			node->data.sprite.prioverchar = prioverchar;
			node->data.sprite.fade_enabled = fade_enabled;
		}

		src += 4;
		attr += 2;
	}
}

void namcos22_state::draw_sprites()
{
	const u32 *src;
	const u32 *attr;

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

	int base = m_spriteram[0] & 0xffff; // alpines/alpinr2b
	int num_sprites = ((m_spriteram[1] >> 16) - base) + 1;

	// airco22b doesn't use spriteset #1
	if (m_gametype == NAMCOS22_AIR_COMBAT22)
		num_sprites = 0;

	if (sprites_on && num_sprites > 0 && num_sprites < 0x400)
	{
		src = &m_spriteram[0x04000/4 + base*4];
		attr = &m_spriteram[0x20000/4 + base*2];
		draw_sprite_group(src, attr, num_sprites, deltax, deltay, y_lowres);
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
		draw_sprite_group(src, attr, num_sprites, deltax, deltay, y_lowres);
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
		draw_sprite_group(src, attr, num_sprites, deltax, deltay, y_lowres);
	}
}

u32 namcos22s_state::namcos22s_vics_control_r(offs_t offset)
{
	u32 ret = m_vics_control[offset];

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

void namcos22s_state::namcos22s_vics_control_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_vics_control[offset]);
}



/*********************************************************************************************/

TILE_GET_INFO_MEMBER(namcos22_state::get_text_tile_info)
{
	u16 data = nthword(m_textram, tile_index);
	/**
	* xxxx.----.----.---- palette select
	* ----.xx--.----.---- flip
	* ----.--xx.xxxx.xxxx code
	*/
	tileinfo.set(0, data & 0x03ff, data >> 12, TILE_FLIPYX((data & 0x0c00) >> 10));
}

void namcos22_state::namcos22_textram_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 prev = m_textram[offset];
	COMBINE_DATA(&m_textram[offset]);
	if (prev != m_textram[offset])
	{
		m_bgtilemap->mark_tile_dirty(offset * 2);
		m_bgtilemap->mark_tile_dirty(offset * 2 + 1);
	}
	namcos22_cgram_w(offset + 0x1e000/4, data, mem_mask);
}

void namcos22_state::namcos22_cgram_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 prev = m_cgram[offset];
	COMBINE_DATA(&m_cgram[offset]);
	if (prev != m_cgram[offset])
		m_gfxdecode->gfx(0)->mark_dirty(offset/32);
}

void namcos22_state::posirq_update()
{
	int scanline = 2 * m_tilemapattr[4];

	// schedule next timeout
	if (scanline < m_screen->height())
	{
		scanline = (scanline + 480) % m_screen->height(); // 0 = vblank start
		m_posirq_timer->adjust(m_screen->time_until_pos(scanline));
	}
	else
		m_posirq_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(namcos22_state::posirq_callback)
{
	// hblank irq line differs between s22/ss22 (see syscon)
	int line = (m_is_ss22) ? 1 : 0;

	if (m_irq_enabled & (1 << line))
	{
		m_irq_state |= (1 << line);
		m_maincpu->set_input_line(m_syscontrol[line] & 7, ASSERT_LINE);
	}

	posirq_update();
}

void namcos22_state::namcos22_tilemapattr_w(offs_t offset, u16 data, u16 mem_mask)
{
	/*
	0: R/W - x offset
	1: R/W - y offset
	2: R/W - ??? always 0x006e?
	3: ?   - unused?
	4: R/W - posirq scanline
	5: R   - current scanline?
	6: ?   - unused?
	7: R   - ???
	*/

	// alpines changes x scroll mid-screen
	if (offset == 0)
		update_text_rowscroll();

	COMBINE_DATA(&m_tilemapattr[offset]);

	if (offset == 4)
		posirq_update();
}

u16 namcos22_state::namcos22_tilemapattr_r(offs_t offset)
{
	switch (offset)
	{
		case 5:
		{
			// unknown, maybe posirq related
			// dirtdash has slowdowns if high bit is clear, why??
			return 0x8000;
		}

		case 7:
			// unknown
			// timecris reads it everytime the gun triggers and will decline if it's 0xffff
			return 0;

		default:
			break;
	}

	return m_tilemapattr[offset];
}



/**
 * Spot RAM affects how the text layer is blended with the scene.
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

/*
RAM looks like it is a 256 * 4 words table
testmode:
 offs: 0000 0001 0002 0003 - 03f4 03f5 03f6 03f7 03f8 03f9 03fa 03fb 03fc 03fd 03fe 03ff
 data: 00fe 00fe 00fe 00fe - 0001 0001 0001 0001 0000 0000 0000 0000 ffff ffff ffff ffff

tokyowar and timecris test ram 000-4ff, but the only practically usable part seems to be 000-3ff
low byte is indirect pen, high byte is shift amount when spot is in alpha blend mode(pens 0x80-0xff)

*/

u16 namcos22s_state::spotram_r(offs_t offset)
{
	if (offset == 2)
	{
		// read
		u16 ret = m_spotram[m_spotram_address >> 1 & 0x7ff];

		if (!machine().side_effects_disabled())
			m_spotram_address += 2;

		return ret;
	}

	return 0;
}

void namcos22s_state::spotram_w(offs_t offset, u16 data, u16 mem_mask)
{
	switch (offset)
	{
		case 0:
			// set address
			COMBINE_DATA(&m_spotram_address);
			break;

		case 1:
			// write
			COMBINE_DATA(&m_spotram[m_spotram_address >> 1 & 0x7ff]);
			m_spotram_address += 2;
			break;

		case 3:
			// enable
			COMBINE_DATA(&m_spotram_enable);
			break;

		default:
			// 2=read-only
			break;
	}
}

void namcos22s_state::namcos22s_mix_text_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int prival)
{
	const pen_t *pens = m_palette->pens();
	u8 pen = 0;
	rgbaint_t rgb;

	// prepare alpha
	u8 alpha_check12 = nthbyte(m_mixer, 0x12);
	u8 alpha_check13 = nthbyte(m_mixer, 0x13);
	u8 alpha_mask    = nthbyte(m_mixer, 0x14) & 0xf;
	u8 alpha_factor  = nthbyte(m_mixer, 0x15);

	// prepare spot
	bool spot_enabled = (m_spotram_enable & 1) && (m_chipselect & 0xc000);
	int spot_factor = (m_spot_factor < 0x100) ? 0 : m_spot_factor & 0xff;
	int spot_palbase = m_text_palbase >> 8 & 3; // src[x] >> 8 & 3

	// prepare fader
	bool fade_enabled = (m_mixer_flags & 2) && m_screen_fade_factor;
	int fade_factor = 0xff - m_screen_fade_factor;
	rgbaint_t fade_color(0, m_screen_fade_r, m_screen_fade_g, m_screen_fade_b);

	// mix textlayer with poly/sprites
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		u16 const *const src = &m_mix_bitmap->pix(y);
		u32 *const dest = &bitmap.pix(y);
		u8 const *const pri = &screen.priority().pix(y);
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			// skip if transparent or under poly/sprite
			if (pri[x] == prival)
			{
				if (spot_enabled)
				{
					// remap pen
					pen = m_spotram[(src[x] << 2 | spot_palbase) & 0x3ff];
					if (pen < 0x80)
						rgb.set(pens[pen | m_text_palbase]);
					else if (prival != 6)
						continue;
				}
				else
				{
					rgb.set(pens[src[x]]);
					pen = src[x];
				}

				// spot pen becomes brightness factor
				if (spot_enabled && pen >= 0x80)
				{
					rgb.set(rgbaint_t(dest[x]));
					u16 factor = (spot_factor * (pen & 0x7f)) >> 7;
					rgb.scale_imm_and_clamp(0x100 - factor);
				}
				else
				{
					// apply fade
					if (fade_enabled)
						rgb.blend(fade_color, fade_factor);

					// apply alpha
					if (alpha_factor && ((pen & 0xf) == alpha_mask || (pen >= alpha_check12 && pen <= alpha_check13)))
						rgb.blend(rgbaint_t(dest[x]), 0xff - alpha_factor);
				}

				dest[x] = rgb.to_rgba();
			}
		}
	}
}

void namcos22_state::namcos22_mix_text_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pens = m_palette->pens();
	const u8 *rlut = &m_gamma_proms[0x000];
	const u8 *glut = &m_gamma_proms[0x100];
	const u8 *blut = &m_gamma_proms[0x200];

	// prepare fader and shadow factor
	bool fade_enabled = (m_screen_fade_r != 0x100 || m_screen_fade_g != 0x100 || m_screen_fade_b != 0x100);
	u32 fade_r_add = (m_screen_fade_r > 0x100) ? (1 << 16) : 0;
	u32 fade_g_add = (m_screen_fade_g > 0x100) ? (1 << 8) : 0;
	u32 fade_b_add = (m_screen_fade_b > 0x100) ? 1 : 0;
	bool fade_white = fade_r_add || fade_g_add || fade_b_add;

	bool shadow_enabled = (m_mixer_flags & 0x100) != 0; // ? (ridgerac is the only game not using shadow)

	rgbaint_t fade_color(0, m_screen_fade_r, m_screen_fade_g, m_screen_fade_b);
	rgbaint_t rgb_mix[3] = {
		rgbaint_t(0, nthbyte(m_mixer, 0x08), nthbyte(m_mixer, 0x09), nthbyte(m_mixer, 0x0a)), // pen c
		rgbaint_t(0, nthbyte(m_mixer, 0x0b), nthbyte(m_mixer, 0x0c), nthbyte(m_mixer, 0x0d)), // pen d
		rgbaint_t(0, nthbyte(m_mixer, 0x0e), nthbyte(m_mixer, 0x0f), nthbyte(m_mixer, 0x10))  // pen e
	};

	// mix textlayer with polys + do final mix
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		u16 const *const src = &m_mix_bitmap->pix(y);
		u32 *const dest = &bitmap.pix(y);
		u8 const *const pri = &screen.priority().pix(y);
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			u32 pixel = dest[x];

			// skip if transparent or under poly
			if (pri[x] == 2)
			{
				// apply shadow
				u8 pen = src[x];
				if (shadow_enabled && pen >= 0xfc && pen <= 0xfe)
				{
					rgbaint_t rgb(pixel);
					rgb.scale_and_clamp(rgb_mix[pen - 0xfc]);
					pixel = rgb.to_rgba();
				}
				else
					pixel = pens[src[x]];
			}

			// apply global fade
			if (fade_enabled)
			{
				if (fade_white)
				{
					// need to make sure that blacks can scale up
					if (!(pixel & 0xff0000)) pixel += fade_r_add;
					if (!(pixel & 0x00ff00)) pixel += fade_g_add;
					if (!(pixel & 0x0000ff)) pixel += fade_b_add;
				}
				rgbaint_t rgb(pixel);
				rgb.scale_and_clamp(fade_color);
				pixel = rgb.to_rgba();
			}

			// apply gamma
			dest[x] = (rlut[(pixel >> 16) & 0xff] << 16) | (glut[(pixel >> 8) & 0xff] << 8) | blut[pixel & 0xff];
		}
	}
}

void namcos22_state::update_text_rowscroll()
{
	u64 frame = m_screen->frame_number();
	if (frame != m_rs_frame)
	{
		m_rs_frame = frame;
		m_lastrow = 0;
	}

	int scroll_x = (m_tilemapattr[0] - 0x35c) & 0x3ff;
	int y = std::min(m_screen->vpos(), 480);

	// save x scroll value until current scanline
	for (int i = m_lastrow; i < y; i++)
		m_rowscroll[i] = scroll_x;
	m_lastrow = y;
}

void namcos22_state::apply_text_scroll()
{
	update_text_rowscroll();
	int scroll_y = m_tilemapattr[1] & 0x3ff;

	m_bgtilemap->set_scrolly(0, scroll_y);
	for (int i = 0; i < 0x400; i++)
		m_bgtilemap->set_scrollx(i, m_rowscroll[0]);

	// apply current frame x scroll updates to tilemap
	for (int i = 0; i < 480; i++)
		m_bgtilemap->set_scrollx((i + scroll_y + 4) & 0x3ff, m_rowscroll[i]);
}

void namcos22_state::draw_text_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	apply_text_scroll();
	m_bgtilemap->set_palette_offset(m_text_palbase);

	m_bgtilemap->draw(screen, *m_mix_bitmap, cliprect, 0, 2, 3);
	namcos22_mix_text_layer(screen, bitmap, cliprect);
}

void namcos22s_state::draw_text_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	apply_text_scroll();
	m_bgtilemap->set_palette_offset(m_text_palbase);

	m_bgtilemap->draw(screen, *m_mix_bitmap, cliprect, 0, 4, 4);
	namcos22s_mix_text_layer(screen, bitmap, cliprect, 4);
}



/*********************************************************************************************/

void namcos22_state::namcos22_paletteram_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 prev = m_paletteram[offset];
	COMBINE_DATA(&m_paletteram[offset]);
	if (prev != m_paletteram[offset])
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


void namcos22s_state::namcos22s_czattr_w(offs_t offset, u16 data, u16 mem_mask)
{
	/*
	       0    1    2    3    4    5    6    7
	    ^^^^ ^^^^ ^^^^ ^^^^                        cz offset, 8-bit(+high bit for sign) per czbank 0,1,2,3
	                        ^^^^                   flags, nybble per czbank 3,2,1,0 - 8=?, 4=enable in-game, 2=reverse compare, 1=ram write enable if 0
	                             ^^^^              maincpu ram bank on read
	                                  ^^^^         bank select for in-game, 2 bits per cztype 3,2,1,0 (0xe4 = 3,2,1,0)
	                                       ^^^^    ? (only set sometimes in timecris)
	    0000 0000 0000 0000 7555 0000 00e4 0000 // testmode normal - 0=white to black(mid), 1=white to black(weak), 2=white to black(strong), 3=black to white(mid, reverse of 0)
	    7fff 8000 7fff 8000 7555 0000 00e4 0000 // testmode offset - 0=black, 1=white, 2=black, 3=white
	    0000 0000 0000 0000 3111 0000 00e4 0000 // testmode off    - 0=white, 1=white, 2=white, 3=white
	    8001 8001 0000 0000 5555 0000 5555 0000 // aquajet (reg 6 is usually interlaced for double buffering)
	    0000 0000 0000 0000 0b6c 0000 00e4 0000 // cybrcycc
	    00a4 00a4 00a4 00a4 4444 0000 0000 0000 // propcycl in pool
	    ff80 ff80 ff80 ff80 4444 0000 0000 0000 // propcycl ending
	*/
	u16 prev = m_czattr[offset];
	COMBINE_DATA(&m_czattr[offset]);

	if (offset == 4)
	{
		// invalidate if compare function changed
		u16 changed = prev ^ m_czattr[offset];
		for (int bank = 0; bank < 4; bank++)
			m_cz_was_written[bank] |= changed >> (bank * 4) & 2;
	}
}

u16 namcos22s_state::namcos22s_czattr_r(offs_t offset)
{
	return m_czattr[offset];
}

void namcos22s_state::namcos22s_czram_w(offs_t offset, u32 data, u32 mem_mask)
{
	/*
	czram contents, it's basically a big cz compare table

	testmode:
	    o_16          0    1    2    3    4    5    6    7 <  >   f8   f9   fa   fb   fc   fd   fe   ff
	    czram[0] = 0000 001f 003f 005f 007f 009f 00bf 00df .... 1eff 1f1f 1f3f 1f5f 1f7f 1f9f 1fbf 1fdf
	    czram[1] = 003f 007f 00be 00fd 013c 017b 01b9 01f7 .... 1ff9 1ffb 1ffc 1ffd 1ffe 1fff 1fff 1fff
	    czram[2] = 0000 0000 0000 0001 0002 0003 0005 0007 .... 1e45 1e83 1ec2 1f01 1f40 1f7f 1fbf 1fff
	    czram[3] = 1fff 1fdf 1fbf 1f9f 1f7f 1f5f 1f3f 1f1f .... 00ff 00df 00bf 009f 007f 005f 003f 001f
	*/
	for (int bank = 0; bank < 4; bank++)
	{
		// write enable bit
		if (~m_czattr[4] >> (bank * 4) & 1)
		{
			u32 prev = (m_banked_czram[bank][offset * 2] << 16) | m_banked_czram[bank][offset * 2 + 1];
			u32 temp = prev;
			COMBINE_DATA(&temp);
			m_banked_czram[bank][offset * 2] = temp >> 16;
			m_banked_czram[bank][offset * 2 + 1] = temp & 0xffff;
			m_cz_was_written[bank] |= (prev ^ temp);
		}
	}
}

u32 namcos22s_state::namcos22s_czram_r(offs_t offset)
{
	int bank = m_czattr[5] & 3;
	return (m_banked_czram[bank][offset * 2] << 16) | m_banked_czram[bank][offset * 2 + 1];
}

void namcos22s_state::recalc_czram()
{
	for (int bank = 0; bank < 4; bank++)
	{
		// as documented above, ss22 czram is 'just' a big compare table
		// this is very slow when emulating, so let's recalculate it to a simpler lookup table
		if (m_cz_was_written[bank])
		{
			int reverse = (m_czattr[4] >> (bank * 4) & 2) ? 0xff : 0;
			int small_val = 0x2000;
			int small_offset = reverse;
			int large_val = 0;
			int large_offset = reverse ^ 0xff;
			int prev = 0;

			for (int i = 0; i < 0x100; i++)
			{
				int factor = i ^ reverse;
				int val = std::min<u16>(m_banked_czram[bank][factor], 0x2000);
				int start = prev;
				int end = val;

				if (i > 0)
				{
					// discard if compare function doesn't match
					if (start >= end)
						continue;

					// fill range
					for (int j = start; j < end; j++)
						m_recalc_czram[bank][j] = factor;
				}

				// remember largest/smallest for later
				if (val < small_val)
				{
					small_val = val;
					small_offset = factor;
				}
				if (val > large_val)
				{
					large_val = val;
					large_offset = factor;
				}

				prev = val;
			}

			// fill possible leftover ranges
			for (int j = 0; j < small_val; j++)
				m_recalc_czram[bank][j] = small_offset;
			for (int j = large_val; j < 0x2000; j++)
				m_recalc_czram[bank][j] = large_offset;

			m_cz_was_written[bank] = 0;
		}
	}
}


void namcos22_state::update_mixer()
{
	m_poly->wait("update_mixer");
#if 0 // show reg contents
	char msg1[0x1000] = {0}, msg2[0x1000] = {0};
	int st = 0x000 / 16;
	for (int i = st; i < (st+3); i++)
	{
		sprintf(msg2,"%04X %08X %08X %08X %08X\n", i*16, m_mixer[i*4+0], m_mixer[i*4+1], m_mixer[i*4+2], m_mixer[i*4+3]);
		strcat(msg1,msg2);
	}
	if (1) // + other non-super regs
	if (!m_is_ss22)
	{
		strcat(msg1,"\n");
		for (int i = 8; i <= 0x20; i += 8)
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
		00824000: ffffff00 00000000 0000007f 00ff006f fe00eded 0f700000 0000037f 00010007 // alpine surfer
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
		    0d,0e           spot factor
		    0f              polygon alpha color mask
		    10              polygon alpha pen mask
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
		m_poly_fade_b        = nthbyte(m_mixer, 0x02);
		m_fog_r              = nthbyte(m_mixer, 0x05);
		m_fog_g              = nthbyte(m_mixer, 0x06);
		m_fog_b              = nthbyte(m_mixer, 0x07);
		m_spot_factor        = nthbyte(m_mixer, 0x0e) << 8 | nthbyte(m_mixer, 0x0d);
		m_poly_alpha_color   = nthbyte(m_mixer, 0x0f);
		m_poly_alpha_pen     = nthbyte(m_mixer, 0x10);
		m_poly_alpha_factor  = nthbyte(m_mixer, 0x11);
		m_screen_fade_r      = nthbyte(m_mixer, 0x16);
		m_screen_fade_g      = nthbyte(m_mixer, 0x17);
		m_screen_fade_b      = nthbyte(m_mixer, 0x18);
		m_screen_fade_factor = nthbyte(m_mixer, 0x19);
		m_mixer_flags        = nthbyte(m_mixer, 0x1a);
		m_text_palbase       = nthbyte(m_mixer, 0x1b) << 8 & 0x7f00;

		m_poly_fade_enabled = (m_mixer[0] & 0xffffff00) != 0xffffff00;
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
		    04              bgcolor palette base?
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
		m_mixer_flags         = nthbyte(m_mixer, 0x00) << 8 | nthbyte(m_mixer, 0x01);
		m_bg_palbase          = nthbyte(m_mixer, 0x04) << 8 & 0x7f00;
		m_text_palbase        = nthbyte(m_mixer, 0x07) << 8 & 0x7f00;
		m_screen_fade_r       = nthbyte(m_mixer, 0x11) << 8 | nthbyte(m_mixer, 0x12); // 0x0100 = 1.0
		m_screen_fade_g       = nthbyte(m_mixer, 0x13) << 8 | nthbyte(m_mixer, 0x14);
		m_screen_fade_b       = nthbyte(m_mixer, 0x15) << 8 | nthbyte(m_mixer, 0x16);

		// raverace is the only game using multiple fog colors (city smog, cars under tunnels, brake disc in attract mode)
		m_fog_colormask       = m_mixer[0x84/4];

		// fog color per cz type
		for (int i = 0; i < 4; i++)
		{
			m_fog_r_per_cztype[i] = nthbyte(m_mixer, 0x0100+i);
			m_fog_g_per_cztype[i] = nthbyte(m_mixer, 0x0180+i);
			m_fog_b_per_cztype[i] = nthbyte(m_mixer, 0x0200+i);
		}
	}
}



/*********************************************************************************************/

u32 namcos22s_state::screen_update_namcos22s(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	render_frame_active();
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
	u8 layer = nthbyte(m_mixer, 0x1f);
	if (layer & 4) draw_text_layer(screen, bitmap, cliprect);
	if (layer & 2) draw_sprites();
	if (layer & 1) draw_polygons();
	m_poly->render_scene(screen, bitmap);
	if (layer & 4) namcos22s_mix_text_layer(screen, bitmap, cliprect, 6);

	// apply gamma
	const u8 *rlut = (const u8 *)&m_mixer[0x100/4];
	const u8 *glut = (const u8 *)&m_mixer[0x200/4];
	const u8 *blut = (const u8 *)&m_mixer[0x300/4];
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		u32 *const dest = &bitmap.pix(y);
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
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

u32 namcos22_state::screen_update_namcos22(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	render_frame_active();
	update_mixer();
	update_palette();
	screen.priority().fill(0, cliprect);

	// background color
	int bg_color = m_bg_palbase | 0xff;
	bitmap.fill(m_palette->pen(bg_color), cliprect);

	// layers
	draw_polygons();
	m_poly->render_scene(screen, bitmap);
	draw_text_layer(screen, bitmap, cliprect); // text layer + final mix

	return 0;
}



/*********************************************************************************************/

void namcos22_state::init_tables()
{
	m_dirtypal = std::make_unique<u8[]>(0x8000/4);
	save_pointer(NAME(m_dirtypal), 0x8000/4);
	memset(m_dirtypal.get(), 1, 0x8000/4);
	memset(m_paletteram, 0, 0x8000);

	matrix3d_identity(m_viewmatrix);
	memset(m_polygonram, 0xcc, m_polygonram.bytes());

	if (!m_is_ss22)
	{
		save_item(NAME(m_fog_r_per_cztype));
		save_item(NAME(m_fog_g_per_cztype));
		save_item(NAME(m_fog_b_per_cztype));
	}

	// init pointrom
	m_pointrom_size = memregion("pointrom")->bytes()/3;
	m_pointrom = std::make_unique<s32[]>(m_pointrom_size);
	u8* pointrom_low = memregion("pointrom")->base();
	u8* pointrom_mid = pointrom_low + m_pointrom_size;
	u8* pointrom_high = pointrom_mid + m_pointrom_size;
	for (int i = 0; i < m_pointrom_size; i++)
	{
		m_pointrom[i] = signed24(pointrom_high[i] << 16 | pointrom_mid[i] << 8 | pointrom_low[i]);
	}

	m_pointram = make_unique_clear<u32[]>(0x20000);
	save_pointer(NAME(m_pointram), 0x20000);

	// force all texture tiles to be decoded now
	for (int i = 0; i < m_gfxdecode->gfx(1)->elements(); i++)
		m_gfxdecode->gfx(1)->get_data(i);

	m_texture_tilemap = (u16 *)memregion("textilemap")->base();
	m_texture_tiledata = (u8 *)m_gfxdecode->gfx(1)->get_data(0);
	m_texture_tileattr = std::make_unique<u8[]>(0x080000*2);

	// unpack textures
	u8 *packed_tileattr = 0x200000 + (u8 *)memregion("textilemap")->base();
	u8 *unpacked_tileattr = m_texture_tileattr.get();
	for (int i = 0; i < 0x80000; i++)
	{
		*unpacked_tileattr++ = (*packed_tileattr) >> 4;
		*unpacked_tileattr++ = (*packed_tileattr) & 0xf;
		packed_tileattr++;
	}

	// make attr/y/x lookup table
	m_texture_ayx_to_pixel = std::make_unique<u8[]>(16*16*16);
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

	// following setup is System22 specific
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

void namcos22s_state::init_tables()
{
	namcos22_state::init_tables();

	// init spotram (super22 only)
	m_spotram = make_unique_clear<u16[]>(0x800);
	save_pointer(NAME(m_spotram), 0x800);

	// init czram tables (super22 only)
	for (int bank = 0; bank < 4; bank++)
	{
		m_banked_czram[bank] = make_unique_clear<u16[]>(0x100);
		m_recalc_czram[bank] = make_unique_clear<u8[]>(0x2000);
		m_cz_was_written[bank] = 1;

		save_pointer(NAME(m_banked_czram[bank]), 0x100, bank);
		save_pointer(NAME(m_recalc_czram[bank]), 0x2000, bank);
	}

	save_item(NAME(m_czattr));
	save_item(NAME(m_cz_was_written));
}

void namcos22_state::video_start()
{
	m_is_ss22 = (m_iomcu == nullptr);
	init_tables();

	m_posirq_timer = timer_alloc(FUNC(namcos22_state::posirq_callback), this);

	m_mix_bitmap = std::make_unique<bitmap_ind16>(640, 480);
	m_bgtilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(namcos22_state::get_text_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_bgtilemap->set_scroll_rows(64 * 16); // fake
	m_bgtilemap->set_transparent_pen(0xf);

	m_gfxdecode->gfx(0)->set_source((u8 *)m_cgram.target());

	m_poly = std::make_unique<namcos22_renderer>(*this);
}
