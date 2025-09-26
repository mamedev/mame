// license:BSD-3-Clause
// copyright-holders:R. Belmont, Acho A. Tang, Phil Stroffolino, Olivier Galibert
/*
 * video/konamigx.cpp - Konami GX video hardware (here there be dragons, and achocode)
 *
 */

#include "emu.h"
#include "k053250.h"
#include "konamigx.h"


//#define GX_DEBUG
#define VERBOSE 0


static inline void set_color_555(palette_device &palette, pen_t color, int rshift, int gshift, int bshift, uint16_t data);


void konamigx_state::konamigx_precache_registers(void)
{
	// (see sprite color coding scheme on p.46 & 47)
	static const int coregmasks[5] = { 0xf, 0xe, 0xc, 0x8, 0x0 };
	static const int coregshifts[5]= { 4, 5, 6, 7, 8 };
	int i;

	i = m_k055673->k053247_read_register(0x8/2);
	m_k053247_vrcbk[0] = (i & 0x000f) << 14;
	m_k053247_vrcbk[1] = (i & 0x0f00) << 6;
	i = m_k055673->k053247_read_register(0xa/2);
	m_k053247_vrcbk[2] = (i & 0x000f) << 14;
	m_k053247_vrcbk[3] = (i & 0x0f00) << 6;

	// COREG == OBJSET2+1C == bit8-11 of OPSET ??? (see p.50 last table, needs p.49 to confirm)
	m_k053247_opset = m_k055673->k053247_read_register(0xc/2);

	i = m_k053247_opset & 7; if (i > 4) i = 4;

	m_k053247_coreg = m_k055673->k053247_read_register(0xc/2) >> 8 & 0xf;
	m_k053247_coreg =(m_k053247_coreg & coregmasks[i]) << 12;

	m_k053247_coregshift = coregshifts[i];

	m_opri     = m_k055555->K055555_read_register(K55_PRIINP_8);
	m_oinprion = m_k055555->K055555_read_register(K55_OINPRI_ON);
	m_vcblk[0] = m_k055555->K055555_read_register(K55_PALBASE_A);
	m_vcblk[1] = m_k055555->K055555_read_register(K55_PALBASE_B);
	m_vcblk[2] = m_k055555->K055555_read_register(K55_PALBASE_C);
	m_vcblk[3] = m_k055555->K055555_read_register(K55_PALBASE_D);
	m_vcblk[4] = m_k055555->K055555_read_register(K55_PALBASE_SUB1);
	m_vcblk[5] = m_k055555->K055555_read_register(K55_PALBASE_SUB2);
	m_ocblk    = m_k055555->K055555_read_register(K55_PALBASE_OBJ);
	m_vinmix   = m_k055555->K055555_read_register(K55_BLEND_ENABLES);
	m_vmixon   = m_k055555->K055555_read_register(K55_VINMIX_ON);
	m_osinmix  = m_k055555->K055555_read_register(K55_OSBLEND_ENABLES);
	m_osmixon  = m_k055555->K055555_read_register(K55_OSBLEND_ON);

	m_brightness[0] = uint8_t(m_k054338->register_r(K338_REG_BRI3));
	m_brightness[1] = uint8_t(m_k054338->register_r(K338_REG_BRI3 + 1) >> 8);
	m_brightness[2] = uint8_t(m_k054338->register_r(K338_REG_BRI3 + 1));
}

inline int konamigx_state::K053247GX_combine_c18(int attrib) // (see p.46)
{
	int c18;

	c18 = (attrib & 0xff) << m_k053247_coregshift | m_k053247_coreg;

	if (m_gx_wrport2 & 4) c18 &= 0x3fff; else
	if (!(m_gx_wrport2 & 8)) c18 = (c18 & 0x3fff) | (attrib << 6 & 0xc000);

	return(c18);
}

inline int konamigx_state::K055555GX_decode_objcolor(int c18) // (see p.59 7.2.2)
{
	int ocb, opon;

	opon  = m_oinprion << 8 | 0xff;
	ocb   = (m_ocblk & 7) << 10;
	c18  &= opon;
	ocb  &=~opon;

	return (ocb | c18) >> m_k053247_coregshift;
}

inline int konamigx_state::K055555GX_decode_inpri(int c18) // (see p.59 7.2.2)
{
	int op = m_opri;

	c18 >>= 8;
	op   &= m_oinprion;
	c18  &=~m_oinprion;

	return c18 | op;
}

K055673_CB_MEMBER(konamigx_state::type2_sprite_callback)
{
	int num = *code;
	int c18 = *color;

	*code = m_k053247_vrcbk[num >> 14] | (num & 0x3fff);
	c18 = K053247GX_combine_c18(c18);
	*color = K055555GX_decode_objcolor(c18);
	*priority_mask = K055555GX_decode_inpri(c18);
}

K055673_CB_MEMBER(konamigx_state::dragoonj_sprite_callback)
{
	int num, op, pri, c18;

	num = *code;
	*code = m_k053247_vrcbk[num >> 14] | (num & 0x3fff);

	c18  = pri = *color;
	op   = m_opri;
	pri  = (pri & 0x200) ? 4 : pri >> 4 & 0xf;
	op  &= m_oinprion;
	pri &=~m_oinprion;
	*priority_mask = pri | op;

	c18 = K053247GX_combine_c18(c18);
	*color = K055555GX_decode_objcolor(c18);
}

K055673_CB_MEMBER(konamigx_state::salmndr2_sprite_callback)
{
	int num, op, pri, c18;

	num = *code;
	*code = m_k053247_vrcbk[num >> 14] | (num & 0x3fff);

	c18  = pri = *color;
	op   = m_opri;
	pri  = pri >> 4 & 0x3f;
	op  &= m_oinprion;
	pri &=~m_oinprion;
	*priority_mask = pri | op;

	c18 = K053247GX_combine_c18(c18);
	*color = K055555GX_decode_objcolor(c18);
}

K055673_CB_MEMBER(konamigx_state::le2_sprite_callback)
{
	int num, op, pri;

	num = *code;
	*code = m_k053247_vrcbk[num >> 14] | (num & 0x3fff);

	pri = *color;
	*color &= 0x1f;

	op   = m_opri;
	pri &= 0xf0;
	op  &= m_oinprion;
	pri &=~m_oinprion;
	*priority_mask = pri | op;
}

int konamigx_state::K055555GX_decode_vmixcolor(int layer, int *color) // (see p.62 7.2.6 and p.27 3.3)
{
	int vcb, shift, pal, vmx, von, pl45, emx;

	vcb    =  m_vcblk[layer] << 6;
	shift  =  layer << 1;
	pal    =  *color;
	vmx    =  m_vinmix >> shift & 3;
	von    =  m_vmixon >> shift & 3;
	emx    =  pl45 = pal >> 4 & 3;
	pal   &=  0xf;
	pl45  &=  von;
	vmx   &=  von;
	pl45 <<=  4;
	emx   &= ~von;
	pal   |=  pl45;
	emx   |=  vmx;
	pal   |=  vcb;

	//if (m_gx_le2_textcolour_hack)
	//  if (layer == 0)
	//      pal |= 0x1c0;

	if (von == 3) emx = -1; // invalidate external mix code if all bits are from internal
	*color =  pal;

	return emx;
}

int konamigx_state::K055555GX_decode_osmixcolor(int layer, int *color) // (see p.63, p.49-50 and p.27 3.3)
{
	int scb, shift, pal, osmx, oson, pl45, emx;

	shift  =  layer << 1;
	pal    =  *color;
	osmx   =  m_osinmix >> shift & 3;
	oson   =  m_osmixon >> shift & 3;

	if (layer)
	{
		// layer 1-3 are external tile layers
		scb    =  m_vcblk[layer + 3] << 6;
		emx    =  pl45 = pal >> 4 & 3;
		pal   &=  0xf;
		pl45  &=  oson;
		osmx  &=  oson;
		pl45 <<=  4;
		emx   &= ~oson;
		pal   |=  pl45;
		emx   |=  osmx;
		pal   |=  scb;

		if (oson == 3) emx = -1; // invalidate external mix code if all bits are from internal
		*color =  pal;
	}
	else
	{
		// layer 0 is the sprite layer with different attributes decode; detail on p.49 (missing)
		emx   = 0; // k053247_read_register(??) >> ? & 3;
		osmx &= oson;
		emx  &=~oson;
		emx  |= osmx;
	}

	return(emx);
}

void konamigx_state::wipezbuf(int noshadow)
{
	const rectangle &visarea = m_screen->visible_area();

	int w = visarea.width();
	int h = visarea.height();

	uint8_t *zptr = m_gx_objzbuf;
	int ecx = h;

	do { memset(zptr, -1, w); zptr += GX_ZBUFW; } while (--ecx);

	if (!noshadow)
	{
		zptr = m_gx_shdzbuf.get();
		w <<= 1;
		ecx = h;
		do { memset(zptr, -1, w); zptr += (GX_ZBUFW << 1); } while (--ecx);
	}
}

void konamigx_state::set_brightness(int layer)
{
	const uint8_t bri_mode = (m_k055555->K055555_read_register(K55_VBRI) >> layer * 2) & 0x03;
	const uint8_t new_brightness = bri_mode ? m_brightness[bri_mode - 1] : 0xff;

	if (m_current_brightness != new_brightness)
	{
		m_current_brightness = new_brightness;
		for (int x = 0; x < m_palette->entries(); ++x)
		{
			m_palette->set_pen_contrast(x, m_current_brightness / 255.0);
		}
	}
}

/*
 * Sprite Format
 * ------------------
 *
 * Word | Bit(s)           | Use
 * -----+-fedcba9876543210-+----------------
 *   0  | x--------------- | active (show this sprite)
 *   0  | -x-------------- | maintain aspect ratio (when set, zoom y acts on both axis)
 *   0  | --x------------- | flip y
 *   0  | ---x------------ | flip x
 *   0  | ----xxxx-------- | sprite size (see below)
 *   0  | --------xxxxxxxx | zcode
 *   1  | xxxxxxxxxxxxxxxx | sprite code
 *   2  | ------xxxxxxxxxx | y position
 *   3  | ------xxxxxxxxxx | x position
 *   4  | xxxxxxxxxxxxxxxx | zoom y (0x40 = normal, <0x40 = enlarge, >0x40 = reduce)
 *   5  | xxxxxxxxxxxxxxxx | zoom x (0x40 = normal, <0x40 = enlarge, >0x40 = reduce)
 *   6  | x--------------- | mirror y (top half is drawn as mirror image of the bottom)
 *   6  | -x-------------- | mirror x (right half is drawn as mirror image of the left)
 *   6  | --xx------------ | reserved (sprites with these two bits set don't seem to be graphics data at all)
 *   6  | ----xx---------- | shadow code: 0=off, 0x400=preset1, 0x800=preset2, 0xc00=preset3
 *   6  | ------xx-------- | effect code: flicker, upper palette, full shadow...etc. (game dependent)
 *   6  | --------xxxxxxxx | "color", but depends on external connections (implies priority)
 *   7  | xxxxxxxxxxxxxxxx | game dependent
 *
 * shadow enables transparent shadows. Note that it applies to the last sprite pen ONLY.
 * The rest of the sprite remains normal.
 */
#define GX_MAX_SPRITES 256*2 // 256 sprites + 256 shadows
#define GX_MAX_LAYERS  6
#define GX_MAX_OBJECTS (GX_MAX_SPRITES + GX_MAX_LAYERS)

void konamigx_state::konamigx_mixer_init(screen_device &screen, int objdma)
{
	m_gx_objdma = 0;
	m_gx_primode = 0;

	m_gx_objzbuf = &screen.priority().pix(0);
	m_gx_shdzbuf = std::make_unique<uint8_t[]>(GX_ZBUFSIZE);

	m_k054338->export_config(&m_K054338_shdRGB);

	if (objdma)
	{
		m_gx_spriteram_alloc = std::make_unique<uint16_t[]>(0x2000/2);
		m_gx_spriteram = m_gx_spriteram_alloc.get();
		m_gx_objdma = 1;
	}
	else
		m_k055673->k053247_get_ram(&m_gx_spriteram);

	m_palette->set_shadow_dRGB32(3, -80, -80, -80, 0);
	m_k054338->invert_alpha(1);
}

void konamigx_state::konamigx_mixer_primode(int mode)
{
	m_gx_primode = mode;
}

void konamigx_state::konamigx_objdma(void)
{
	uint16_t* k053247_ram;
	m_k055673->k053247_get_ram(&k053247_ram);

	if (m_gx_objdma && m_gx_spriteram && k053247_ram) memcpy(m_gx_spriteram, k053247_ram, 0x1000);
}

void konamigx_state::konamigx_mixer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect,
					tilemap_t *sub1, int sub1flags,
					tilemap_t *sub2, int sub2flags,
					int mixerflags, bitmap_ind16 *extra_bitmap, int rushingheroes_hack)
{
	// buffer can move when it's resized, so refresh the pointer
	m_gx_objzbuf = &screen.priority().pix(0);

	// clear screen with backcolor and update flicker pulse
	if (m_gx_wrport1_0 & 0x20)
		m_k054338->fill_backcolor(bitmap,
									cliprect,
									m_palette->pens() + (m_k055555->K055555_read_register(0) << 9),
									m_k055555->K055555_read_register(1));
	else
		m_k054338->fill_solid_bg(bitmap, cliprect);

	// abort if video has been disabled
	const uint8_t disp = m_k055555->K055555_read_register(K55_INPUT_ENABLES);
	if (!disp) return;
	uint16_t cltc_shdpri = m_k054338->register_r(K338_REG_CONTROL);

	// Slam Dunk 2 never sets this.  It's either part of the protection, or type4 doesn't use it
	if (!rushingheroes_hack)
	{
		if (!(cltc_shdpri & K338_CTL_KILL)) return;
	}

	// demote shadows by one layer when this bit is set??? (see p.73 8.6)
	cltc_shdpri &= K338_CTL_SHDPRI;

	// wipe z-buffer
	if (mixerflags & GXMIX_NOZBUF)
		mixerflags |= GXMIX_NOSHADOW;
	else
		wipezbuf(mixerflags & GXMIX_NOSHADOW);

	// cache global parameters
	konamigx_precache_registers();

	// init OBJSET2 and mixer parameters (see p.51 and chapter 7)
	uint8_t layerid[6] = { 0, 1, 2, 3, 4, 5 };

	// invert layer priority when this flag is set (not used by any GX game?)
	//int prflp = K055555_read_register(K55_CONTROL) & K55_CTL_FLIPPRI;

	uint8_t layerpri[6];
	layerpri[0] = m_k055555->K055555_read_register(K55_PRIINP_0);
	layerpri[1] = m_k055555->K055555_read_register(K55_PRIINP_3);
	layerpri[3] = m_k055555->K055555_read_register(K55_PRIINP_7);
	layerpri[4] = m_k055555->K055555_read_register(K55_PRIINP_9);
	layerpri[5] = m_k055555->K055555_read_register(K55_PRIINP_10);

	int shdprisel;

	if (m_gx_primode == -1)
	{
		// Lethal Enforcer hack (requires pixel color comparison)
		layerpri[2] = m_k055555->K055555_read_register(K55_PRIINP_3) + 0x20;
		shdprisel = 0x3f;
	}
	else
	{
		layerpri[2] = m_k055555->K055555_read_register(K55_PRIINP_6);
		shdprisel = m_k055555->K055555_read_register(K55_SHD_PRI_SEL);
	}

	// SHDPRISEL filters shadows by different priority comparison methods (UNIMPLEMENTED, see detail on p.66)
	bool shadowon[3];
	if (!(shdprisel & 0x03)) shadowon[0] = 0;
	if (!(shdprisel & 0x0c)) shadowon[1] = 0;
	if (!(shdprisel & 0x30)) shadowon[2] = 0;

	uint8_t shdpri[3];
	shdpri[0]   = m_k055555->K055555_read_register(K55_SHAD1_PRI);
	shdpri[1]   = m_k055555->K055555_read_register(K55_SHAD2_PRI);
	shdpri[2]   = m_k055555->K055555_read_register(K55_SHAD3_PRI);

	int spri_min = 0;

	shadowon[2] = shadowon[1] = shadowon[0] = 0;

	int k = 0;
	if (!(mixerflags & GXMIX_NOSHADOW))
	{
		// only enable shadows beyond a +/-7 RGB threshold
		for (int j = 0, i = 0; i < 3; j += 3, i++)
		{
			k = m_K054338_shdRGB[j  ]; if (k < -7 || k > 7) { shadowon[i] = 1; continue; }
			k = m_K054338_shdRGB[j+1]; if (k < -7 || k > 7) { shadowon[i] = 1; continue; }
			k = m_K054338_shdRGB[j+2]; if (k < -7 || k > 7) { shadowon[i] = 1; }
		}

		// SHDON specifies layers on which shadows can be projected (see detail on p.65 7.2.8)
		int temp = m_k055555->K055555_read_register(K55_SHD_ON);
		for (int i = 0; i < 4; i++) if (!BIT(temp, i) && spri_min < layerpri[i]) spri_min = layerpri[i]; // HACK

		// update shadows status
		m_k054338->update_all_shadows(rushingheroes_hack, *m_palette);
	}

	// pre-sort layers
	for (int j = 0; j < 5; j++)
	{
		for (int i = j + 1; i < 6; i++)
		{
			if (layerpri[j] <= layerpri[i])
			{
				using std::swap;
				swap(layerpri[j], layerpri[i]);
				swap(layerid[j], layerid[i]);
			}
		}
	}

	// build object database and create indices
	std::vector<GX_OBJ> objpool; // max size: 6 layers + 256 sprites + 256 shadows

	for (int i = 5; i >= 0; i--)
	{
		int offs;

		const uint8_t code = layerid[i];
		switch (code)
		{
			/*
			    Background layers are represented by negative offset values as follow:

			    0+ : normal sprites
			    -1 : tile layer A - D
			    -2 : K053936 ROZ+ layer 1
			    -3 : K053936 ROZ+ layer 2
			    -4 : K053250 LVC layer 1
			    -5 : K053250 LVC layer 2
			*/
			case 4:
				offs = -128;
				if (sub1flags & 0xf) { if (sub1flags & GXSUB_K053250) offs = -4; else if (sub1) offs = -2; }
			break;
			case 5:
				offs = -128;
				if (sub2flags & 0xf) { if (sub2flags & GXSUB_K053250) offs = -5; else if (sub2) offs = -3; }
				if (extra_bitmap) offs = -3;
			break;
			default: offs = -1;
		}

		if (offs != -128)
		{
			const uint32_t order = layerpri[i] << 24;
			const int color = 0;
			objpool.emplace_back(GX_OBJ{ order, offs, code, color });
		}
	}

	const uint32_t start_addr = m_type3_spriteram_bank ? 0x800 : 0;

	for (int x = 0; x < 256; ++x)
	{
		const uint16_t offs = start_addr + x * 8;
		int pri = 0;

		if (!(m_gx_spriteram[offs] & 0x8000)) continue;

		uint8_t zcode = m_gx_spriteram[offs] & 0xff;

		// invert z-order when opset_pri is set (see p.51 OPSET PRI)
		if (m_k053247_opset & 0x10) zcode = 0xff - zcode;

		int code  = m_gx_spriteram[offs+1];
		int color = k = m_gx_spriteram[offs+6];
		// int l     = m_gx_spriteram[offs+7];

		m_k055673->m_k053247_cb(&code, &color, &pri);

		uint8_t shadow_draw_mode = 0; // shadow pens draw mode (4-5)
		bool add_shadow = 0;          // add shadow object
		uint8_t solid_draw_mode = 0;  // solid pens draw mode (0-3)
		bool add_solid = 0;           // add solid object
		uint8_t spri = 0;             // shadow priority
		uint8_t shadow = 0;           // shadow code

		if (color & K055555_FULLSHADOW)
		{
			shadow = 3; // use default intensity and color
			spri = pri; // retain host priority
			add_shadow = 1;
			shadow_draw_mode = 5; // draw full shadow
		}
		else
		{
			shadow = k >> 10 & 3;
			if (shadow) // object has shadow?
			{
				int k053246_objset1 = m_k055673->k053246_read_register(5);
				if (shadow != 1 || k053246_objset1 & 0x20)
				{
					shadow--;
					add_solid = 1;
					solid_draw_mode = 1; // draw partial solid
					if (shadowon[shadow])
					{
						add_shadow = 1;
						shadow_draw_mode = 4; // draw partial shadow
					}
				}
				else
				{
					// drop the entire sprite to shadow if its shadow code is 1 and SD0EN is off (see p.48)
					shadow = 0;
					if (!shadowon[0]) continue;
					add_shadow = 1;
					shadow_draw_mode = 5; // draw full shadow
				}
			}
			else
			{
				add_solid = 1;
				solid_draw_mode = 0; // draw full solid
			}

			if (add_solid)
			{
				// tag sprite for alpha blending
				if (color >> K055555_MIXSHIFT & 3) solid_draw_mode |= 2;
			}

			if (add_shadow)
			{
				// determine shadow priority
				spri = (m_k053247_opset & 0x20) ? pri : shdpri[shadow]; // (see p.51 OPSET SDSEL)
			}
		}

		switch (m_gx_primode & 0xf)
		{
			// Dadandarn zcode suppression
			case 1:
				zcode = 0;
				break;

			// Daisukiss bad shadow filter
			case 4:
				if (k & 0x3000 || k == 0x0800) continue;
				[[fallthrough]];

			// Tokkae shadow masking (INACCURATE)
			case 5:
				if (spri < spri_min) spri = spri_min;
				break;
		}

		/*
		    default sort order:
		    fedcba98 76543210 fedcba98 76543210
		    xxxxxxxx -------- -------- -------- (priority)
		    -------- xxxxxxxx -------- -------- (zcode)
		    -------- -------- xxxxxxxx -------- (offset)
		    -------- -------- -------- xxxx---- (shadow mode)
		    -------- -------- -------- ------xx (shadow code)
		*/
		if (add_solid)
		{
			// add objects with solid or alpha pens
			uint32_t order = pri << 24 | zcode << 16 | offs << (8 - 3) | solid_draw_mode << 4;
			objpool.emplace_back(GX_OBJ{ order, offs, code, color });
		}

		if (add_shadow && !(color & K055555_SKIPSHADOW) && !(mixerflags & GXMIX_NOSHADOW))
		{
			// add objects with shadows if enabled
			uint32_t order = spri << 24 | zcode << 16 | offs << (8 - 3) | shadow_draw_mode << 4 | shadow;
			objpool.emplace_back(GX_OBJ{ order, offs, code, color });
		}
	}

	// sort objects in descending order (SLOW)
	// reverse objpool to retain order in case of ties
	std::reverse(objpool.begin(), objpool.end());
	std::stable_sort(
			objpool.begin(),
			objpool.end(),
			[] (const GX_OBJ &a, const GX_OBJ &b) { return a.order > b.order; });

	konamigx_mixer_draw(screen, bitmap, cliprect, sub1, sub1flags, sub2, sub2flags, mixerflags, extra_bitmap, rushingheroes_hack, objpool);
}

void konamigx_state::konamigx_mixer_draw(
		screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect,
		tilemap_t *sub1, int sub1flags,
		tilemap_t *sub2, int sub2flags,
		int mixerflags, bitmap_ind16 *extra_bitmap, int rushingheroes_hack,
		const std::vector<GX_OBJ> &objpool) /* passed from above function */
{
	// traverse draw list
	const uint8_t disp = m_k055555->K055555_read_register(K55_INPUT_ENABLES);

	for (int count = 0; count < objpool.size(); count++)
	{
		const uint32_t order = objpool[count].order;
		const int offs = objpool[count].offs;
		const int code = objpool[count].code;
		int color = objpool[count].color;

		/* entries >=0 in our list are sprites */
		if (offs >= 0)
		{
			if (!(disp & K55_INP_OBJ)) continue;

			int drawmode = order >> 4 & 0xf;

			int alpha = 255;
			int pri = 0;
			int zcode = -1; // negative zcode values turn off z-buffering

			if (drawmode & 2)
			{
				alpha = color >> K055555_MIXSHIFT & 3;
				if (alpha) alpha = m_k054338->set_alpha_level(alpha);
				if (alpha <= 0) continue;
			}
			color &= K055555_COLORMASK;

			if (drawmode >= 4) m_palette->set_shadow_mode(order & 0x03);

			if (!(mixerflags & GXMIX_NOZBUF))
			{
				zcode = order >> 16 & 0xff;
				pri = order >> 24 & 0xff;
			}

			m_k055673->k053247_draw_single_sprite_gxcore(bitmap, cliprect,
					m_gx_objzbuf, m_gx_shdzbuf.get(), code, m_gx_spriteram, offs,
					color, alpha, drawmode, zcode, pri,
					/* non-gx only */
					0, 0, nullptr, nullptr, 0);
		}
		/* the rest are tilemaps of various kinda */
		else
		{
			switch (offs)
			{
				case -1:
					gx_draw_basic_tilemaps(screen, bitmap, cliprect, mixerflags, code);
					continue;
				case -2:
				case -4:
					gx_draw_basic_extended_tilemaps_1(screen, bitmap, cliprect, mixerflags, code, sub1, sub1flags, rushingheroes_hack, offs);
				continue;
				case -3:
				case -5:
					gx_draw_basic_extended_tilemaps_2(screen, bitmap, cliprect, mixerflags, code, sub2, sub2flags, extra_bitmap, offs);
				continue;
			}
			continue;
		}
	}
}

void konamigx_state::gx_draw_basic_tilemaps(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int mixerflags, uint8_t layer)
{
	const uint8_t disp = m_k055555->K055555_read_register(K55_INPUT_ENABLES);

	if (disp & (1 << layer))
	{
		set_brightness(layer);

		const uint8_t layer2 = layer << 1;
		const uint8_t j = mixerflags >> layer2 & 3;

		// keep internal and external mix codes separated, so the external mix code can be applied to category 1 tiles
		uint8_t mix_mode_internal = 0;
		uint8_t mix_mode_external = 0;

		if (j == GXMIX_BLEND_FORCE)
			mix_mode_internal = mixerflags >> (layer2 + 16) & 3; // hack
		else
		{
			const uint8_t v_inmix_on_layer = m_vmixon >> layer2 & 3;
			const uint8_t v_inmix_layer = m_vinmix >> layer2 & 3;
			const uint8_t tile_mix_code = uint32_t(mixerflags) >> 30;

			mix_mode_internal = v_inmix_layer & v_inmix_on_layer;
			mix_mode_external = tile_mix_code & ~v_inmix_on_layer;
		}

		int flags = TILEMAP_DRAW_CATEGORY(0);
		int flags2 = TILEMAP_DRAW_CATEGORY(1);

		if (mixerflags & 1 << (layer + 12))
		{
			flags |= K056382_DRAW_FLAG_FORCE_XYSCROLL;
			flags2 |= K056382_DRAW_FLAG_FORCE_XYSCROLL;
		}

		// FIXME: implement mixpri and additive
		// hack: mask out mixpri bit. if additive bit set, mask it out and invert alpha.
		// this makes additive alpha effects look OK until they are properly handled.
		int alpha = m_k054338->set_alpha_level(mix_mode_internal) & 0x1ff;
		if (alpha & 0x100)
		{
			alpha &= 0xff;
			if (alpha) alpha = ~alpha & 0xff;
		}

		int alpha2 = m_k054338->set_alpha_level(mix_mode_external) & 0x1ff;
		if (alpha2 & 0x100) alpha2 = ~alpha2 & 0xff;

		if (alpha < 255) flags |= TILEMAP_DRAW_ALPHA(alpha);

		if (alpha2 < 255)
		{
			// tiles with mix codes are put into category 1.
			// draw them in a separate pass for per-tile blending if necessary.
			flags2 |= TILEMAP_DRAW_ALPHA(alpha2);
			m_k056832->tilemap_draw(screen, bitmap, cliprect, layer, flags2, 0);
		}
		else
		{
			// if no alpha is being applied to category 1 (tile mix code) tiles,
			// draw all tiles with one tilemap_draw call
			flags |= TILEMAP_DRAW_ALL_CATEGORIES;
		}
		m_k056832->tilemap_draw(screen, bitmap, cliprect, layer, flags, 0);
	}
}

void konamigx_state::gx_draw_basic_extended_tilemaps_1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int mixerflags, int code, tilemap_t *sub1, int sub1flags, int rushingheroes_hack, int offs)
{
	int temp1, temp2, temp3, temp4;
	int i = code << 1;
	int j = mixerflags >> i & 3;
	int k = 0;

	int disp = m_k055555->K055555_read_register(K55_INPUT_ENABLES);
	if ((disp & K55_INP_SUB1) || (rushingheroes_hack))
	{
		int alpha = 255;

		if (j == GXMIX_BLEND_NONE)  { temp1 = 0xff; temp2 = temp3 = 0; } else
		if (j == GXMIX_BLEND_FORCE) { temp1 = 0x00; temp2 = mixerflags >> 24; temp3 = 3; }
		else
		{
			temp1 = m_osinmix;
			temp2 = m_osinmix >> 2 & 3;
			temp3 = m_osmixon >> 2 & 3;
		}

		if (temp1 != 0xff && temp2 /*&& temp3 == 3*/)
		{
			alpha = temp4 = m_k054338->set_alpha_level(temp2) & 0xff;

			if (temp4 <= 0) return;
			if (temp4 < 255) k = 1;
		}

		int l = sub1flags & 0xf;

		if (offs == -2)
		{
			int pixeldouble_output = 0;
			const rectangle &visarea = screen.visible_area();
			int width = visarea.width();

			if (width > 512) // vsnetscr case
				pixeldouble_output = 1;

			K053936GP_0_zoom_draw(machine(), bitmap, cliprect, sub1, l, k, alpha, pixeldouble_output, m_k053936_0_ctrl_16, m_k053936_0_linectrl_16, m_k053936_0_ctrl, m_k053936_0_linectrl, *m_palette);
		}
		else
		{
			m_k053250_1->draw(bitmap, cliprect, m_vcblk[4] << l, 0, screen.priority(), 0);
		}
	}
}

void konamigx_state::gx_draw_basic_extended_tilemaps_2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int mixerflags, int code, tilemap_t *sub2, int sub2flags, bitmap_ind16 *extra_bitmap, int offs)
{
	int temp1,temp2,temp3,temp4;
	int i = code << 1;
	int j = mixerflags >> i & 3;

	int disp = m_k055555->K055555_read_register(K55_INPUT_ENABLES);
	if (disp & K55_INP_SUB2)
	{
		//int alpha = 255;
		if (j == GXMIX_BLEND_NONE)  { temp1 = 0xff; temp2 = temp3 = 0; } else
		if (j == GXMIX_BLEND_FORCE) { temp1 = 0x00; temp2 = mixerflags >> 26; temp3 = 3; }
		else
		{
			temp1 = m_osinmix;
			temp2 = m_osinmix >> 4 & 3;
			temp3 = m_osmixon >> 4 & 3;
		}

		if (temp1 != 0xff && temp2 /*&& temp3==3*/)
		{
			//alpha =
			temp4 = m_k054338->set_alpha_level(temp2) & 0xff;

			if (temp4 <= 0) return;
			//if (temp4 < 255) k = 1;
		}

		int l = sub2flags & 0xf;

		if (offs == -3)
		{
			if (extra_bitmap) // soccer superstars roz layer
			{
				int width = screen.width();
				int height = screen.height();
				pen_t const *const paldata = m_palette->pens();

				// the output size of the roz layer has to be doubled horizontally
				// so that it aligns with the sprites and normal tilemaps.  This appears
				// to be done as a post-processing / mixing step effect
				//
				// - todo, use the pixeldouble_output I just added for vsnet instead?
				for (int yy = 0; yy < height; yy++)
				{
					uint16_t const *const src = &extra_bitmap->pix(yy);
					uint32_t *const dst = &bitmap.pix(yy);
					int shiftpos = 0;
					for (int xx = 0; xx < width; xx += 2)
					{
						uint16_t dat = src[(((xx / 2) + shiftpos)) % width];
						if (dat & 0xff)
							dst[xx + 1] = dst[xx] = paldata[dat];
					}
				}
			}
			else
			{
				// int pixeldouble_output = 0;
				// K053936GP_1_zoom_draw(machine, bitmap, cliprect, sub2, l, k, alpha, pixeldouble_output);
			}
		}
		else
			m_k053250_2->draw(bitmap, cliprect, m_vcblk[5] << l, 0, screen.priority(), 0);
	}
}

/* Run and Gun 2 / Rushing Heroes */
TILE_GET_INFO_MEMBER(konamigx_state::get_gx_psac_tile_info)
{
	int tileno, colour, col, flip = 0;
	if (tile_index & 1)
	{
		tileno = m_psacram[tile_index/2] & 0x00001fff;
		col    =(m_psacram[tile_index/2] & 0x00002000) >> 13;
		if      (m_psacram[tile_index/2] & 0x00004000) flip |= TILE_FLIPX;
		if      (m_psacram[tile_index/2] & 0x00008000) flip |= TILE_FLIPY;

	}
	else
	{
		tileno = (m_psacram[tile_index/2] & 0x1fff0000) >> 16;
		col    = (m_psacram[tile_index/2] & 0x20000000) >> 29;
		if       (m_psacram[tile_index/2] & 0x40000000) flip |= TILE_FLIPX;
		if       (m_psacram[tile_index/2] & 0x80000000) flip |= TILE_FLIPY;

	}

	colour = (m_psac_colorbase << 4) + col;

	tileinfo.set(0, tileno, colour, TILE_FLIPYX(flip));
}


void konamigx_state::type3_bank_w(offs_t offset, uint8_t data)
{
	// other bits are used for something...

	if (offset == 0)
	{
		m_type3_psac2_bank = (data & 0x10) >> 4;
		// swap sprite display bank for left/right screens
		// bit 6 works for soccerss, doesn't for type4 (where they never enable it)
		// so the best candidate is bit 0
		//m_type3_spriteram_bank = (data & 0x40) >> 6;
		m_type3_spriteram_bank = (data & 0x01);
	}
	else
		logerror("Write to type3 bank %02x address %02x\n",offset, data);

	/* handle this by creating 2 roz tilemaps instead, otherwise performance dies completely on dual screen mode
	if (m_konamigx_type3_psac2_actual_bank!=m_konamigx_type3_psac2_actual_last_bank)
	{
	    m_gx_psac_tilemap->mark_all_dirty();
	    m_konamigx_type3_psac2_actual_last_bank = m_konamigx_type3_psac2_actual_bank;
	}
	*/
}



/* Soccer Superstars (tile and flip bits now TRUSTED) */
TILE_GET_INFO_MEMBER(konamigx_state::get_gx_psac3_tile_info)
{
	int tileno, colour, flip;
	uint8_t *tmap = memregion("gfx4")->base();

	int base_index = tile_index;

//  if (m_konamigx_type3_psac2_actual_bank)
//      base_index+=0x20000/2;

	tileno =  tmap[base_index*2] | ((tmap[(base_index*2) + 1] & 0x0f) << 8);
	colour = (tmap[(base_index*2) + 1] & 0xc0) >> 6;

	flip = 0;
	if (tmap[(base_index*2) + 1] & 0x20) flip |= TILE_FLIPY;
	if (tmap[(base_index*2) + 1] & 0x10) flip |= TILE_FLIPX;

	tileinfo.set(0, tileno, colour, flip);
}

TILE_GET_INFO_MEMBER(konamigx_state::get_gx_psac3_alt_tile_info)
{
	int tileno, colour, flip;
	uint8_t *tmap = memregion("gfx4")->base() + 0x20000;

	int base_index = tile_index;

//  if (m_konamigx_type3_psac2_actual_bank)
//      base_index+=0x20000/2;

	tileno =  tmap[base_index*2] | ((tmap[(base_index*2) + 1] & 0x0f) << 8);
	colour = (tmap[(base_index*2) + 1] & 0xc0) >> 6;

	flip = 0;
	if (tmap[(base_index*2)+1] & 0x20) flip |= TILE_FLIPY;
	if (tmap[(base_index*2)+1] & 0x10) flip |= TILE_FLIPX;

	tileinfo.set(0, tileno, colour, flip);
}


/*
 * PSAC4
 *
 * racinfrc title screen prints this watermark in ROZ at origin 0,0:
 * SYSTEM NWK250^tm
 * -THE HI-SPEED PSAC4 DRIVER
 * \tWITH SKIPPING+DROPPING REDUCED
 * -SCREEN SYSTEM
 * \t2^16x2^16 IMAGINALLY AREA (sic)
 * -CHARACTER SYSTEM
 *
 * (c)KONAMI 1993
 *
 */
/* these tilemaps are weird in both format and content, one of them
   doesn't really look like it should be displayed? - it's height data */
TILE_GET_INFO_MEMBER(konamigx_state::get_gx_psac1a_tile_info)
{
	int tileno, colour, flipx, flipy;
	int flip = 0;
	colour = 0;

	tileno = (m_psacram[tile_index*2] & 0x00003fff) >> 0;

	// scanrows
	//flipx  = (m_psacram[tile_index*2+1] & 0x00800000) >> 23;
	//flipy  = (m_psacram[tile_index*2+1] & 0x00400000) >> 22;
	// scancols
	flipy  = (m_psacram[tile_index*2+1] & 0x00800000) >> 23;
	flipx  = (m_psacram[tile_index*2+1] & 0x00400000) >> 22;

	if (flipx) flip |= TILE_FLIPX;
	if (flipy) flip |= TILE_FLIPY;

	tileinfo.set(1, tileno, colour, flip);
}

TILE_GET_INFO_MEMBER(konamigx_state::get_gx_psac1b_tile_info)
{
	int tileno, colour, flipx,flipy;
	int flip = 0;

	// FIXME: has at least 32 entries of valid colors
	// (from z-value as color depth effect?)
	colour = (m_psacram[tile_index*2 + 1] & 0x000c'0000) >> 18;
	// TODO: 0x7fff mask for opengolf
	tileno = (m_psacram[tile_index*2 + 1] & 0x0000'3fff) >> 0;

	// scanrows
	//flipx  = (m_psacram[tile_index*2 + 1] & 0x0080'0000) >> 23;
	//flipy  = (m_psacram[tile_index*2 + 1] & 0x0040'0000) >> 22;
	// scancols
	flipy  = (m_psacram[tile_index*2 + 1] & 0x0020'0000) >> 21;
	flipx  = (m_psacram[tile_index*2 + 1] & 0x0010'0000) >> 20;

	if (flipx) flip |= TILE_FLIPX;
	if (flipy) flip |= TILE_FLIPY;

	tileinfo.set(0, tileno, colour, flip);
}

K056832_CB_MEMBER(konamigx_state::type2_tile_callback)
{
	int d = *code;

	*code = (m_gx_tilebanks[(d & 0xe000) >> 13] << 13) + (d & 0x1fff);
	K055555GX_decode_vmixcolor(layer, color);
}

K056832_CB_MEMBER(konamigx_state::salmndr2_tile_callback)
{
	const uint8_t mix_code = attr >> 4 & 3;
	if (mix_code)
	{
		*priority = 1;
		m_last_alpha_tile_mix_code = mix_code;
	}

	int d = *code;

	*code = (m_gx_tilebanks[(d & 0xe000) >> 13] << 13) + (d & 0x1fff);
	K055555GX_decode_vmixcolor(layer, color);
}

K056832_CB_MEMBER(konamigx_state::alpha_tile_callback)
{
	const uint8_t mix_code = attr >> 6 & 3;
	if (mix_code)
	{
		*priority = 1;
		m_last_alpha_tile_mix_code = mix_code;
	}
	int d = *code;

	*code = (m_gx_tilebanks[(d & 0xe000) >> 13] << 13) + (d & 0x1fff);
	K055555GX_decode_vmixcolor(layer, color);
}

/*
> bits 8-13 are the low priority bits
> i.e. pri 0-5
> pri 6-7 can be either 1, bits 14,15 or bits 16,17
> contro.bit 2 being 0 forces the 1
> when control.bit 2 is 1, control.bit 3 selects between the two
> 0 selects 16,17
> that gives you the entire 8 bits of the sprite priority
> ok, lemme see if I've got this.  bit2 = 0 means the top bits are 11, bit2=1 means the top bits are bits 14/15 (of the whatever word?) else
+16+17?
> bit3=1 for the second

 *   6  | ---------xxxxxxx | "color", but depends on external connections


> there are 8 color lines entering the 5x5
> that means the palette is 4 bits, not 5 as you currently have
> the bits 4-9 are the low priority bits
> bits 10/11 or 12/13 are the two high priority bits, depending on the control word
> and bits 14/15 are the shadow bits
> mix0/1 and brit0/1 come from elsewhere
> they come from the '673 all right, but not from word 6
> and in fact the top address bits are highly suspect
> only 18 of the address bits go to the roms
> the next 2 go to cai0/1 and the next 4 to bk0-3
> (the '246 indexes the roms, the '673 reads the result)
> the roms are 64 bits wide
> so, well, the top bits of the code are suspicious
*/

void konamigx_state::common_init()
{
	konamigx_mixer_init(*m_screen, 0);

	for (int i = 0; i < 8; i++)
	{
		m_gx_tilebanks[i] = m_gx_oldbanks[i] = 0;
	}

	save_pointer(NAME(m_gx_spriteram), 0x800);
	save_item(NAME(m_gx_tilebanks));
	save_item(NAME(m_k053247_vrcbk));
	save_item(NAME(m_k053247_coreg));
	save_item(NAME(m_k053247_coregshift));
	save_item(NAME(m_k053247_opset));
	save_item(NAME(m_opri));
	save_item(NAME(m_oinprion));
	save_item(NAME(m_vcblk));
	save_item(NAME(m_ocblk));
	save_item(NAME(m_vinmix));
	save_item(NAME(m_vmixon));
	save_item(NAME(m_osinmix));
	save_item(NAME(m_osmixon));
	save_item(NAME(m_current_brightness));
	save_item(NAME(m_brightness));
	save_item(NAME(m_last_alpha_tile_mix_code));

	m_gx_tilemode = 0;

	m_gx_rozenable = 0;
	m_gx_specialrozenable = 0;
	m_gx_rushingheroes_hack = 0;

	// Documented relative offsets of non-flipped games are (-2, 0, 2, 3),(0, 0, 0, 0).
	// (+ve values move layers to the right and -ve values move layers to the left)
	// In most cases only a constant is needed to add to the X offsets to yield correct
	// displacement. This should be done by the CCU but the CRT timings have not been
	// figured out.
	m_k056832->set_layer_offs(0, -2, 0);
	m_k056832->set_layer_offs(1,  0, 0);
	m_k056832->set_layer_offs(2,  2, 0);
	m_k056832->set_layer_offs(3,  3, 0);

	m_konamigx_has_dual_screen = 0;
	m_konamigx_current_frame = 0;
}


VIDEO_START_MEMBER(konamigx_state, konamigx_5bpp)
{
	common_init();

	if (!strcmp(machine().system().name, "tbyahhoo") || !strcmp(machine().system().name, "mtwinbee"))
		m_gx_tilemode = 1;
	else if (!strcmp(machine().system().name, "crzcross") || !strcmp(machine().system().name, "puzldama"))
		konamigx_mixer_primode(5);
	else if (!strcmp(machine().system().name, "daiskiss"))
		konamigx_mixer_primode(4);
}

VIDEO_START_MEMBER(konamigx_state, dragoonj)
{
	common_init();

	m_k056832->set_layer_offs(0, -2+1, 0);
	m_k056832->set_layer_offs(1,  0+1, 0);
	m_k056832->set_layer_offs(2,  2+1, 0);
	m_k056832->set_layer_offs(3,  3+1, 0);
}

VIDEO_START_MEMBER(konamigx_state, le2)
{
	common_init();

	konamigx_mixer_primode(-1); // swapped layer B and C priorities?
}

VIDEO_START_MEMBER(konamigx_state, konamigx_6bpp)
{
	common_init();
	konamigx_mixer_primode(5);
}

VIDEO_START_MEMBER(konamigx_state, konamigx_type3)
{
	int width = m_screen->width();
	int height = m_screen->height();

	m_dualscreen_left_tempbitmap = std::make_unique<bitmap_rgb32>(width, height);
	m_dualscreen_right_tempbitmap = std::make_unique<bitmap_rgb32>(width, height);

	common_init();

	m_gx_psac_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(konamigx_state::get_gx_psac3_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 256, 256);
	m_gx_psac_tilemap_alt = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(konamigx_state::get_gx_psac3_alt_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 256, 256);

	m_gx_rozenable = 0;
	m_gx_specialrozenable = 2;

	/* set up tile layers */
	m_type3_roz_temp_bitmap = std::make_unique<bitmap_ind16>(width, height);

	//m_gx_psac_tilemap->set_flip(TILEMAP_FLIPX| TILEMAP_FLIPY);

	K053936_wraparound_enable(0, 1);
//  K053936GP_set_offset(0, -30, -1);
	K053936_set_offset(0, -30, +1);

	m_k056832->set_layer_offs(0, -52, 0);
	m_k056832->set_layer_offs(1, -48, 0);
	m_k056832->set_layer_offs(2, -48, 0);
	m_k056832->set_layer_offs(3, -48, 0);

	m_konamigx_has_dual_screen = 1;
	m_konamigx_palformat = 1;
}

VIDEO_START_MEMBER(konamigx_state, konamigx_type4)
{
	int width = m_screen->width();
	int height = m_screen->height();

	m_dualscreen_left_tempbitmap = std::make_unique<bitmap_rgb32>(width, height);
	m_dualscreen_right_tempbitmap = std::make_unique<bitmap_rgb32>(width, height);

	common_init();

	m_gx_psac_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(konamigx_state::get_gx_psac_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 128, 128);
	m_gx_rozenable = 0;
	m_gx_specialrozenable = 3;

	m_k056832->set_layer_offs(0, -27, 0);
	m_k056832->set_layer_offs(1, -25, 0);
	m_k056832->set_layer_offs(2, -24, 0);
	m_k056832->set_layer_offs(3, -22, 0);

	K053936_wraparound_enable(0, 0);
	K053936GP_set_offset(0, -36, 1);

	m_gx_rushingheroes_hack = 1;
	m_konamigx_has_dual_screen = 1;
	m_konamigx_palformat = 0;

}

VIDEO_START_MEMBER(konamigx_state, konamigx_type4_vsn)
{
	int width = m_screen->width();
	int height = m_screen->height();

	m_dualscreen_left_tempbitmap = std::make_unique<bitmap_rgb32>( width, height);
	m_dualscreen_right_tempbitmap = std::make_unique<bitmap_rgb32>( width, height);

	common_init();

	m_gx_psac_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(konamigx_state::get_gx_psac_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 128, 128);
	m_gx_rozenable = 0;
	m_gx_specialrozenable = 3;

	m_k056832->set_layer_offs(0, -52, 0);
	m_k056832->set_layer_offs(1, -48, 0);
	m_k056832->set_layer_offs(2, -48, 0);
	m_k056832->set_layer_offs(3, -48, 0);

	K053936_wraparound_enable(0, 1); // wraparound doesn't work properly with the custom drawing function anyway, see the crowd in vsnet and rushhero
	K053936GP_set_offset(0, -30, 0);

	m_gx_rushingheroes_hack = 1;
	m_konamigx_has_dual_screen = 1;
	m_konamigx_palformat = 0;
}

VIDEO_START_MEMBER(konamigx_state, konamigx_type4_sd2)
{
	int width = m_screen->width();
	int height = m_screen->height();

	m_dualscreen_left_tempbitmap = std::make_unique<bitmap_rgb32>( width, height);
	m_dualscreen_right_tempbitmap = std::make_unique<bitmap_rgb32>( width, height);

	common_init();

	m_gx_psac_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(konamigx_state::get_gx_psac_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 128, 128);
	m_gx_rozenable = 0;
	m_gx_specialrozenable = 3;

	m_k056832->set_layer_offs(0, -29, -1);
	m_k056832->set_layer_offs(1, -27, -1);
	m_k056832->set_layer_offs(2, -26, -1);
	m_k056832->set_layer_offs(3, -24, -1);

	K053936_wraparound_enable(0, 0);
	K053936GP_set_offset(0, -36, -1);

	m_gx_rushingheroes_hack = 1;
	m_konamigx_has_dual_screen = 1;
	m_konamigx_palformat = 0;

}

VIDEO_START_MEMBER(konamigx_state, opengolf)
{
	common_init();

	m_k056832->set_layer_offs(0, -2+1, 0);
	m_k056832->set_layer_offs(1,  0+1, 0);
	m_k056832->set_layer_offs(2,  2+1, 0);
	m_k056832->set_layer_offs(3,  3+1, 0);

	m_gx_psac_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(konamigx_state::get_gx_psac1a_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 128, 128);
	m_gx_psac_tilemap2 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(konamigx_state::get_gx_psac1b_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 128, 128);

	// transparency will be handled manually in post-processing
	//m_gx_psac_tilemap->set_transparent_pen(0);
	//m_gx_psac_tilemap2->set_transparent_pen(0);

	m_gx_rozenable = 0;
	m_gx_specialrozenable = 1;

	m_gxtype1_roz_dstbitmap =  std::make_unique<bitmap_ind16>(512, 512); // BITMAP_FORMAT_IND16 because we NEED the raw pen data for post-processing
	m_gxtype1_roz_dstbitmap2 = std::make_unique<bitmap_ind16>(512, 512); // BITMAP_FORMAT_IND16 because we NEED the raw pen data for post-processing

	m_gxtype1_roz_dstbitmapclip.set(0, 512-1, 0, 512-1);

	K053936_wraparound_enable(0, 1);
	K053936GP_set_offset(0, 0, 0);

	// urgh.. the priority bitmap is global, and because our temp bitmaps are bigger than the screen, this causes issues.. so just allocate something huge
	// until there is a better solution, or priority bitmap can be specified manually.
	m_screen->priority().allocate(2048, 2048);

}

VIDEO_START_MEMBER(konamigx_state, racinfrc)
{
	common_init();

	m_k056832->set_layer_offs(0, -2+1, -16);
	m_k056832->set_layer_offs(1,  0+1, -16);
	m_k056832->set_layer_offs(2,  2+1, -16);
	m_k056832->set_layer_offs(3,  3+1, -16);

	m_gx_psac_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(konamigx_state::get_gx_psac1a_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 128, 128);
	m_gx_psac_tilemap2 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(konamigx_state::get_gx_psac1b_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 128, 128);

	// transparency will be handled manually in post-processing
	//m_gx_psac_tilemap->set_transparent_pen(0);
	//m_gx_psac_tilemap2->set_transparent_pen(0);

	m_gx_rozenable = 0;
	m_gx_specialrozenable = 1;

	m_gxtype1_roz_dstbitmap =  std::make_unique<bitmap_ind16>(512, 512); // BITMAP_FORMAT_IND16 because we NEED the raw pen data for post-processing
	m_gxtype1_roz_dstbitmap2 = std::make_unique<bitmap_ind16>(512, 512); // BITMAP_FORMAT_IND16 because we NEED the raw pen data for post-processing

	m_gxtype1_roz_dstbitmapclip.set(0, 512-1, 0, 512-1);

	K053936_wraparound_enable(0, 1);
	K053936GP_set_offset(0, 0, 0);

	// urgh.. the priority bitmap is global, and because our temp bitmaps are bigger than the screen, this causes issues.. so just allocate something huge
	// until there is a better solution, or priority bitmap can be specified manually.
	m_screen->priority().allocate(2048, 2048);


}

uint32_t konamigx_state::screen_update_konamigx(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int i, newbank, newbase, dirty, unchained;

	/* if any banks are different from last render, we need to flush the planes */
	for (dirty = 0, i = 0; i < 8; i++)
	{
		newbank = m_gx_tilebanks[i];
		if (m_gx_oldbanks[i] != newbank) { m_gx_oldbanks[i] = newbank; dirty = 1; }
	}

	if (m_gx_tilemode == 0)
	{
		// driver approximates tile update in mode 0 for speed
		unchained = m_k056832->get_layer_association();
		for (i = 0; i < 4; i++)
		{
			newbase = m_k055555->K055555_get_palette_index(i) << 6;
			if (m_layer_colorbase[i] != newbase)
			{
				m_layer_colorbase[i] = newbase;

				if (unchained)
					m_k056832->mark_plane_dirty(i);
				else
					dirty = 1;
			}
		}
	}
	else
	{
		// altK056832 does all the tracking in mode 1 for accuracy (Twinbee needs this)
	}

	// sub2 is PSAC colorbase on GX
	if (m_gx_rozenable)
	{
		m_last_psac_colorbase = m_psac_colorbase;
		m_psac_colorbase = m_k055555->K055555_get_palette_index(6);

		if (m_psac_colorbase != m_last_psac_colorbase)
		{
			m_gx_psac_tilemap->mark_all_dirty();
			if (m_gx_rozenable == 3)
			{
				m_gx_psac_tilemap2->mark_all_dirty();
			}
		}
	}

	if (dirty) m_k056832->mark_all_tilemaps_dirty();

	// Type-1
	if (m_gx_specialrozenable == 1)
	{
		//K053936_0_zoom_draw(screen, *m_gxtype1_roz_dstbitmap, m_gxtype1_roz_dstbitmapclip,m_gx_psac_tilemap, 0, 0, 0); // height data
		K053936_0_zoom_draw(screen, *m_gxtype1_roz_dstbitmap2,m_gxtype1_roz_dstbitmapclip,m_gx_psac_tilemap2, 0, 0, 0); // colour data (+ some voxel height data?)
	}

	if (m_gx_specialrozenable == 3)
	{
		konamigx_mixer(screen, bitmap, cliprect, m_gx_psac_tilemap, GXSUB_8BPP,nullptr,0,  0, nullptr, m_gx_rushingheroes_hack);
	}
	// todo: fix so that it works with the mixer without crashing(!)
	else if (m_gx_specialrozenable == 2)
	{
		// we're going to throw half of this away anyway in post-process, so only render what's needed
		rectangle temprect;
		temprect = cliprect;
		temprect.max_x = cliprect.min_x + 320;

		if (m_type3_psac2_bank == 1) K053936_0_zoom_draw(screen, *m_type3_roz_temp_bitmap, temprect,m_gx_psac_tilemap_alt, 0, 0, 0); // soccerss playfield
		else K053936_0_zoom_draw(screen, *m_type3_roz_temp_bitmap, temprect,m_gx_psac_tilemap, 0, 0, 0); // soccerss playfield


		konamigx_mixer(screen, bitmap, cliprect, nullptr, 0, nullptr, 0, 0, m_type3_roz_temp_bitmap.get(), m_gx_rushingheroes_hack);
	}
	else
	{
		int mixerflags = m_last_alpha_tile_mix_code << 30;
		konamigx_mixer(screen, bitmap, cliprect, nullptr, 0, nullptr, 0, mixerflags, nullptr, m_gx_rushingheroes_hack);
	}

	// HACK: draw type-1 roz layer here for testing purposes only
	if (m_gx_specialrozenable == 1)
	{
		pen_t const *const paldata = m_palette->pens();

		// draw the roz tilemap if W is held
		if ( machine().input().code_pressed(KEYCODE_W) )
		{
			// make it flicker, to compare positioning
			//if (screen.frame_number() & 1)
			{
				for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
				{
					//uint32_t *const dst = &bitmap.pix(y);
					// ths K053936 rendering should probably just be flipped
					// this is just kludged to align the racing force 2d logo
					uint16_t const *const src = &m_gxtype1_roz_dstbitmap2->pix(y);
					//uint16_t const *const src = &m_gxtype1_roz_dstbitmap->pix(y);

					uint32_t *const dst = &bitmap.pix((256 + 16) - y);

					for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
					{
						uint16_t const dat = src[x];
						dst[x] = paldata[dat];
					}
				}
			}

		}

	}

	return 0;
}

uint32_t konamigx_state::screen_update_konamigx_left(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* the video gets demuxed by a board which plugs into the jamma connector */
	m_konamigx_current_frame ^= 1;

	if (m_konamigx_current_frame == 1)
	{
		int offset=0;

		if (m_konamigx_palformat == 1)
		{
			for (offset = 0; offset < 0x4000/4; offset++)
			{
				uint32_t coldat = m_generic_paletteram_32[offset];

				set_color_555(*m_palette, offset*2, 0, 5, 10,coldat >> 16);
				set_color_555(*m_palette, offset*2+1, 0, 5, 10,coldat & 0xffff);
			}
		}
		else
		{
			for (offset = 0; offset < 0x8000/4; offset++)
			{
				int r = (m_generic_paletteram_32[offset] >>16) & 0xff;
				int g = (m_generic_paletteram_32[offset] >> 8) & 0xff;
				int b = (m_generic_paletteram_32[offset] >> 0) & 0xff;

				m_palette->set_pen_color(offset,rgb_t(r,g,b));
			}
		}

		screen_update_konamigx( screen, downcast<bitmap_rgb32 &>(*m_dualscreen_left_tempbitmap), cliprect);
		copybitmap(bitmap, *m_dualscreen_left_tempbitmap, 0, 0, 0, 0, cliprect);
	}
	else
	{
		copybitmap(bitmap, *m_dualscreen_left_tempbitmap, 0, 0, 0, 0, cliprect);
	}

	return 0;
}

uint32_t konamigx_state::screen_update_konamigx_right(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_konamigx_current_frame == 1)
	{
		copybitmap(bitmap, *m_dualscreen_right_tempbitmap, 0, 0, 0, 0, cliprect);
	}
	else
	{
		int offset=0;

		if (m_konamigx_palformat == 1)
		{
			for (offset = 0; offset < 0x4000/4; offset++)
			{
				uint32_t coldat = m_subpaletteram32[offset];

				set_color_555(*m_palette, offset*2, 0, 5, 10,coldat >> 16);
				set_color_555(*m_palette, offset*2+1, 0, 5, 10,coldat & 0xffff);
			}
		}
		else
		{
			for (offset = 0; offset < 0x8000/4; offset++)
			{
				int r = (m_subpaletteram32[offset] >>16) & 0xff;
				int g = (m_subpaletteram32[offset] >> 8) & 0xff;
				int b = (m_subpaletteram32[offset] >> 0) & 0xff;

				m_palette->set_pen_color(offset,rgb_t(r,g,b));
			}
		}

		screen_update_konamigx(screen, downcast<bitmap_rgb32 &>(*m_dualscreen_right_tempbitmap), cliprect);
		copybitmap(bitmap, *m_dualscreen_right_tempbitmap, 0, 0, 0, 0, cliprect);
	}

	return 0;
}

static inline void set_color_555(palette_device &palette, pen_t color, int rshift, int gshift, int bshift, uint16_t data)
{
	palette.set_pen_color(color, pal5bit(data >> rshift), pal5bit(data >> gshift), pal5bit(data >> bshift));
}

// main monitor for type 3
void konamigx_state::konamigx_555_palette_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t coldat;
	COMBINE_DATA(&m_generic_paletteram_32[offset]);

	coldat = m_generic_paletteram_32[offset];

	set_color_555(*m_palette, offset*2, 0, 5, 10, coldat >> 16);
	set_color_555(*m_palette, offset*2 + 1, 0, 5, 10, coldat & 0xffff);
}

// sub monitor for type 3
void konamigx_state::konamigx_555_palette2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t coldat;
	COMBINE_DATA(&m_subpaletteram32[offset]);
	coldat = m_subpaletteram32[offset];

	offset += (0x4000/4);

	set_color_555(*m_palette, offset*2, 0, 5, 10, coldat >> 16);
	set_color_555(*m_palette, offset*2 + 1, 0, 5, 10, coldat & 0xffff);
}

void konamigx_state::konamigx_tilebank_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_24_31)
		m_gx_tilebanks[offset*4] = (data >> 24) & 0xff;
	if (ACCESSING_BITS_16_23)
		m_gx_tilebanks[offset*4 + 1] = (data >> 16) & 0xff;
	if (ACCESSING_BITS_8_15)
		m_gx_tilebanks[offset*4 + 2] = (data >> 8) & 0xff;
	if (ACCESSING_BITS_0_7)
		m_gx_tilebanks[offset*4 + 3] = data & 0xff;
}

// type 1 RAM-based PSAC tilemap
void konamigx_state::konamigx_t1_psacmap_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_psacram[offset]);
	m_gx_psac_tilemap->mark_tile_dirty(offset/2);
	m_gx_psac_tilemap2->mark_tile_dirty(offset/2);
}

// type 4 RAM-based PSAC tilemap
void konamigx_state::konamigx_t4_psacmap_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_psacram[offset]);

	m_gx_psac_tilemap->mark_tile_dirty(offset*2);
	m_gx_psac_tilemap->mark_tile_dirty((offset*2) + 1);
}
