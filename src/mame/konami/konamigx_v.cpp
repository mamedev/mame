// license:BSD-3-Clause
// copyright-holders:R. Belmont, Acho A. Tang, Phil Stroffolino, Olivier Galibert
/*
 * video/konamigx.cpp - Konami GX video hardware (here there be dragons, and achocode)
 *
 */

#include "emu.h"
#include "konamigx.h"

#include "k053250.h"


//#define GX_DEBUG
#define VERBOSE 0


static inline void set_color_555(palette_device &palette, pen_t color, int rshift, int gshift, int bshift, u16 data);


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

	m_brightness[0] = u8(m_k054338->register_r(K338_REG_BRI3));
	m_brightness[1] = u8(m_k054338->register_r(K338_REG_BRI3 + 1) >> 8);
	m_brightness[2] = u8(m_k054338->register_r(K338_REG_BRI3 + 1));
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
	int num = code;
	int c18 = color;

	code = m_k053247_vrcbk[num >> 14] | (num & 0x3fff);
	c18 = K053247GX_combine_c18(c18);
	color = K055555GX_decode_objcolor(c18);
	priority_mask = K055555GX_decode_inpri(c18);
}

K055673_CB_MEMBER(konamigx_state::dragoonj_sprite_callback)
{
	int num, op, pri, c18;

	num = code;
	code = m_k053247_vrcbk[num >> 14] | (num & 0x3fff);

	c18  = pri = color;
	op   = m_opri;
	pri  = (pri & 0x200) ? 4 : pri >> 4 & 0xf;
	op  &= m_oinprion;
	pri &=~m_oinprion;
	priority_mask = pri | op;

	c18 = K053247GX_combine_c18(c18);
	color = K055555GX_decode_objcolor(c18);
}

K055673_CB_MEMBER(konamigx_state::salmndr2_sprite_callback)
{
	int num, op, pri, c18;

	num = code;
	code = m_k053247_vrcbk[num >> 14] | (num & 0x3fff);

	c18  = pri = color;
	op   = m_opri;
	pri  = pri >> 4 & 0x3f;
	op  &= m_oinprion;
	pri &=~m_oinprion;
	priority_mask = pri | op;

	c18 = K053247GX_combine_c18(c18);
	color = K055555GX_decode_objcolor(c18);
}

K055673_CB_MEMBER(konamigx_state::le2_sprite_callback)
{
	int num, op, pri;

	num = code;
	code = m_k053247_vrcbk[num >> 14] | (num & 0x3fff);

	pri = color;
	color &= 0x1f;

	op   = m_opri;
	pri &= 0xf0;
	op  &= m_oinprion;
	pri &=~m_oinprion;
	priority_mask = pri | op;
}

int konamigx_state::K055555GX_decode_vmixcolor(int layer, int &color) // (see p.62 7.2.6 and p.27 3.3)
{
	int vcb, shift, pal, vmx, von, pl45, emx;

	vcb    =  m_vcblk[layer] << 6;
	shift  =  layer << 1;
	pal    =  color;
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
	color =  pal;

	return emx;
}

int konamigx_state::K055555GX_decode_osmixcolor(int layer, int &color) // (see p.63, p.49-50 and p.27 3.3)
{
	int scb, shift, pal, osmx, oson, pl45, emx;

	shift  =  layer << 1;
	pal    =  color;
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
		color =  pal;
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

	u8 *zptr = m_gx_objzbuf.get();
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
	const u8 bri_mode = (m_k055555->K055555_read_register(K55_VBRI) >> layer * 2) & 0x03;
	const u8 new_brightness = bri_mode ? m_brightness[bri_mode - 1] : 0xff;

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

	// the screen priority bitmap is free for gx_draw_deferred_shadows
	m_gx_objzbuf = std::make_unique<u8[]>(GX_ZPAGESIZE);
	m_gx_shdzbuf = std::make_unique<u8[]>(GX_ZBUFSIZE);

	m_k054338->export_config(&m_K054338_shdRGB);

	if (objdma)
	{
		m_gx_spriteram_alloc = std::make_unique<u16[]>(0x2000/2);
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
	u16* k053247_ram;
	m_k055673->k053247_get_ram(&k053247_ram);

	if (m_gx_objdma && m_gx_spriteram && k053247_ram) memcpy(m_gx_spriteram, k053247_ram, 0x1000);
}

void konamigx_state::konamigx_mixer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect,
					tilemap_t *sub1, int sub1flags,
					tilemap_t *sub2, int sub2flags,
					int mixerflags, bitmap_ind16 *extra_bitmap, int rushingheroes_hack)
{
	// clear screen with backcolor and update flicker pulse
	if (m_gx_wrport1_0 & 0x20)
		m_k054338->fill_backcolor(bitmap,
									cliprect,
									m_palette->pens() + (m_k055555->K055555_read_register(0) << 9),
									m_k055555->K055555_read_register(1));
	else
		m_k054338->fill_solid_bg(bitmap, cliprect);

	// abort if video has been disabled
	const u8 disp = m_k055555->K055555_read_register(K55_INPUT_ENABLES);
	if (!disp) return;
	u16 cltc_shdpri = m_k054338->register_r(K338_REG_CONTROL);

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
	u8 layerid[6] = { 0, 1, 2, 3, 4, 5 };

	// invert layer priority when this flag is set (not used by any GX game?)
	//int prflp = K055555_read_register(K55_CONTROL) & K55_CTL_FLIPPRI;

	u8 layerpri[6];
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

	// SHDPRISEL filters shadows by different priority comparison methods (see detail on p.66)
	// conditions 1 and 2 are handled by gx_draw_deferred_shadows, 0 is UNIMPLEMENTED
	bool shadowon[3];
	if (!(shdprisel & 0x03)) shadowon[0] = 0;
	if (!(shdprisel & 0x0c)) shadowon[1] = 0;
	if (!(shdprisel & 0x30)) shadowon[2] = 0;

	u8 shdpri[3];
	shdpri[0]   = m_k055555->K055555_read_register(K55_SHAD1_PRI);
	shdpri[1]   = m_k055555->K055555_read_register(K55_SHAD2_PRI);
	shdpri[2]   = m_k055555->K055555_read_register(K55_SHAD3_PRI);

	int spri_min = 0;

	shadowon[2] = shadowon[1] = shadowon[0] = 0;

	int k = 0;
	if (!(mixerflags & GXMIX_NOSHADOW))
	{
		// update shadows status now so it's valid for drawing the current frame
		m_k054338->update_all_shadows(rushingheroes_hack, *m_palette);

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
	std::vector<GX_OBJ> objpool; // layers (including PSAC4 priorities), sprites and shadows
	std::vector<GX_OBJ> deferred_shadows; // shadows that test the topmost screen, see gx_draw_deferred_shadows

	for (int i = 5; i >= 0; i--)
	{
		int offs;

		const u8 code = layerid[i];
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
			    -6 : PSAC4 pixels at one priority
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
			if (code == 5 && m_gx_specialrozenable == 1)
			{
				// PSAC4 supplies a different priority for each terrain sample.
				// Insert its used priorities into the same list as objects and
				// shadows, allowing both to pass behind nearer terrain.
				u8 const mask = m_k055555->K055555_read_register(21); // S2 INPRI ON
				for (int p = 0; p < 256; p++)
				{
					if (m_type1_priority_used[p])
					{
						u8 const pri = (p & ~mask) | (layerpri[i] & mask);
						// OBJ wins a priority tie against SUB2. Draw this
						// terrain before all objects at the same priority,
						// irrespective of their sprite Z code or RAM offset.
						objpool.emplace_back(GX_OBJ{ (u32(pri) << 24) | 0x00ffffff, -6, p, 0 });
					}
				}
				continue;
			}
			const u32 order = layerpri[i] << 24;
			const int color = 0;
			objpool.emplace_back(GX_OBJ{ order, offs, code, color });
		}
	}

	const u32 start_addr = m_type3_spriteram_bank ? 0x800 : 0;

	for (int x = 0; x < 256; ++x)
	{
		const u16 offs = start_addr + x * 8;
		int pri = 0;

		if (!(m_gx_spriteram[offs] & 0x8000)) continue;

		u8 zcode = m_gx_spriteram[offs] & 0xff;

		// invert z-order when opset_pri is set (see p.51 OPSET PRI)
		if (m_k053247_opset & 0x10) zcode = 0xff - zcode;

		int code  = m_gx_spriteram[offs+1];
		int color = k = m_gx_spriteram[offs+6];
		// int l     = m_gx_spriteram[offs+7];

		m_k055673->m_k053247_cb(code, color, pri);

		u8 shadow_draw_mode = 0; // shadow pens draw mode (4-5)
		bool add_shadow = 0;          // add shadow object
		u8 solid_draw_mode = 0;  // solid pens draw mode (0-3)
		bool add_solid = 0;           // add solid object
		u8 spri = 0;             // shadow priority
		u8 shadow = 0;           // shadow code

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
			u32 order = pri << 24 | zcode << 16 | offs << (8 - 3) | solid_draw_mode << 4;
			objpool.emplace_back(GX_OBJ{ order, offs, code, color });
		}

		if (add_shadow && !(color & K055555_SKIPSHADOW) && !(mixerflags & GXMIX_NOSHADOW))
		{
			// add objects with shadows if enabled
			u32 order = spri << 24 | zcode << 16 | offs << (8 - 3) | shadow_draw_mode << 4 | shadow;
			const u8 condition = (color & K055555_FULLSHADOW) ? 3 : (shdprisel >> (shadow * 2) & 3);
			if (condition == 1 || condition == 2)
			{
				deferred_shadows.emplace_back(GX_OBJ{ order, offs, code, color });
			}
			else
			{
				objpool.emplace_back(GX_OBJ{ order, offs, code, color });
			}
		}
	}

	// sort objects in descending order (SLOW)
	// reverse objpool to retain order in case of ties
	std::reverse(objpool.begin(), objpool.end());
	std::stable_sort(
			objpool.begin(),
			objpool.end(),
			[] (const GX_OBJ &a, const GX_OBJ &b) { return a.order > b.order; });

	m_gx_topmost_on = !deferred_shadows.empty();
	if (m_gx_topmost_on)
	{
		// everything drawn from here on records its priority code; the back color is lowest
		screen.priority().fill(0xff, cliprect);
		m_k055673->k053247_set_gx_topmost(&screen.priority(), nullptr);
	}

	konamigx_mixer_draw(screen, bitmap, cliprect, sub1, sub1flags, sub2, sub2flags, mixerflags, extra_bitmap, rushingheroes_hack, objpool);

	if (m_gx_topmost_on)
	{
		std::reverse(deferred_shadows.begin(), deferred_shadows.end());
		std::stable_sort(
				deferred_shadows.begin(),
				deferred_shadows.end(),
				[] (const GX_OBJ &a, const GX_OBJ &b) { return a.order > b.order; });

		gx_draw_deferred_shadows(screen, bitmap, cliprect, deferred_shadows, shdprisel);

		m_k055673->k053247_set_gx_topmost(nullptr, nullptr);
		m_gx_topmost_on = false;
	}
}

void konamigx_state::konamigx_mixer_draw(
		screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect,
		tilemap_t *sub1, int sub1flags,
		tilemap_t *sub2, int sub2flags,
		int mixerflags, bitmap_ind16 *extra_bitmap, int rushingheroes_hack,
		const std::vector<GX_OBJ> &objpool) /* passed from above function */
{
	// traverse draw list
	const u8 disp = m_k055555->K055555_read_register(K55_INPUT_ENABLES);

	for (int count = 0; count < objpool.size(); count++)
	{
		const u32 order = objpool[count].order;
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
					m_gx_objzbuf.get(), m_gx_shdzbuf.get(), code, m_gx_spriteram, offs,
					color, alpha, drawmode, zcode, pri,
					/* non-gx only */
					0, 0, nullptr, nullptr, 0);
		}
		/* the rest are tilemaps of various kinda */
		else
		{
			switch (offs)
			{
				case -6:
					if (disp & K55_INP_SUB2)
					{
						type1_mix_terrain(bitmap, cliprect, code);
					}
					continue;
				case -1:
					gx_draw_basic_tilemaps(screen, bitmap, cliprect, mixerflags, code, order >> 24);
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

// SHD PRI SEL (p.66 7.2.8) enables each shadow code by comparing its priority
// with the priority code of the topmost screen at the pixel:
// 1 = if the shadow's priority is greater
// 2 = if the shadow's priority is equal
// 3 = if the shadow's priority is less
// 1 and 2 depend on the topmost screen, so those shadows are drawn last,
// gated by the priority codes that everything else recorded in screen.priority()
// as it was drawn.
void konamigx_state::gx_draw_deferred_shadows(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, const std::vector<GX_OBJ> &shadows, u8 shdprisel)
{
	bool enable[256];

	for (const GX_OBJ &obj : shadows)
	{
		const u8 spri = obj.order >> 24;
		const u8 shadow = obj.order & 3;
		const u8 condition = shdprisel >> (shadow * 2) & 3;

		for (int pri = 0; pri < 256; pri++)
		{
			enable[pri] = (condition == 2) ? (spri == pri) : (spri > pri);
		}
		enable[0xff] = false; // the back color, or a layer SHD ON keeps shadows off
		m_k055673->k053247_set_gx_topmost(&screen.priority(), enable);

		m_palette->set_shadow_mode(shadow);
		m_k055673->k053247_draw_single_sprite_gxcore(bitmap, cliprect,
				m_gx_objzbuf.get(), m_gx_shdzbuf.get(), obj.code, m_gx_spriteram, obj.offs,
				obj.color & K055555_COLORMASK, 255, obj.order >> 4 & 0xf, obj.order >> 16 & 0xff, spri,
				0, 0, nullptr, nullptr, 0);
	}
}

void konamigx_state::gx_draw_basic_tilemaps(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int mixerflags, u8 layer, u8 pri)
{
	const u8 disp = m_k055555->K055555_read_register(K55_INPUT_ENABLES);

	if (!(disp & (1 << layer))) return;

	set_brightness(layer);

	const u8 layer2 = layer << 1;
	const u8 j = mixerflags >> layer2 & 3;

	u32 flags = 0;
	if (mixerflags & 1 << (layer + 12)) flags |= K056382_DRAW_FLAG_FORCE_XYSCROLL;

	// what the layer records as the topmost screen for gx_draw_deferred_shadows:
	// its priority code, or 0xff like the back color when SHD ON keeps shadows off it
	const u8 topmost = BIT(m_k055555->K055555_read_register(K55_SHD_ON), layer) ? pri : 0xff;

	// Category 0 is the tiles with no mix code of their own: they blend with
	// the layer's internal code (V INMIX where V INMIX ON routes it).
	// Categories 1-3 are the tiles whose colour bits 5:4 carry a mix code
	// (the tile callback, K055555 p.62 7.2.6): each is drawn with that code's
	// K054338 level. Every tile blends with its own code, rather than all of
	// them with the code of whichever tile the callback saw last.
	for (u8 cat = 0; cat < 4; cat++)
	{
		int mix;
		if (j == GXMIX_BLEND_FORCE)
			mix = mixerflags >> (layer2 + 16) & 3;
		else if (cat == 0)
			mix = (m_vinmix >> layer2 & 3) & (m_vmixon >> layer2 & 3);
		else
			mix = cat;
		gx_draw_tilemap_category(screen, bitmap, cliprect, layer, cat, flags, m_k054338->set_alpha_level(mix), topmost);
	}
}

// One category of a layer at one K054338 level: set_alpha_level's 10 bits,
// { MIXPRI, additive, alpha }. MIXPRI is still not implemented.
void konamigx_state::gx_draw_tilemap_category(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u8 layer, u8 category, u32 flags, int level, u8 topmost)
{
	flags |= TILEMAP_DRAW_CATEGORY(category);
	const u8 alpha = level & 0xff;

	// record the layer as the topmost screen, see gx_draw_deferred_shadows
	const u8 top_pri = m_gx_topmost_on ? topmost : 0;
	const u8 top_mask = m_gx_topmost_on ? 0 : 0xff;

	if (!(level & 0x100))
	{
		if (alpha == 0) return;
		if (alpha < 255) flags |= TILEMAP_DRAW_ALPHA(alpha);
		m_k056832->tilemap_draw(screen, bitmap, cliprect, layer, flags, top_pri, top_mask);
		return;
	}

	// Additive (mix set bit 5): the layer's colour at this level is added to
	// what is under it, per channel and clamped, so black adds nothing and
	// stays transparent. tilemap.cpp has no additive draw, so the
	// category goes through an indexed scratch bitmap first.
	if (!m_gx_tile_scratch || m_gx_tile_scratch->width() < bitmap.width() || m_gx_tile_scratch->height() < bitmap.height())
		m_gx_tile_scratch = std::make_unique<bitmap_ind16>(bitmap.width(), bitmap.height());
	m_gx_tile_scratch->fill(0xffff, cliprect);
	m_k056832->tilemap_draw(screen, *m_gx_tile_scratch, cliprect, layer, flags, top_pri, top_mask);

	pen_t const *const pens = m_palette->pens();
	const u32 mul = u32(alpha) + 1;   // 255 is a full add
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		u16 const *const src = &m_gx_tile_scratch->pix(y);
		u32 *const dst = &bitmap.pix(y);
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			if (src[x] == 0xffff) continue;
			const u32 s = pens[src[x]];
			const u32 scaled = ((((s >> 16) & 0xff) * mul >> 8) << 16)
			                 | ((((s >> 8) & 0xff) * mul >> 8) << 8)
			                 | ((s & 0xff) * mul >> 8);
			dst[x] = add_blend_r32(dst[x], scaled);
		}
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
					u16 const *const src = &extra_bitmap->pix(yy);
					u32 *const dst = &bitmap.pix(yy);
					int shiftpos = 0;
					for (int xx = 0; xx < width; xx += 2)
					{
						u16 dat = src[(((xx / 2) + shiftpos)) % width];
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


void konamigx_state::type3_bank_w(offs_t offset, u8 data)
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
	u8 *tmap = memregion("gfx4")->base();

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
	u8 *tmap = memregion("gfx4")->base() + 0x20000;

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
// The two VRAM entries select HROM and CROM independently. The second entry
// supplies both pairs of tile flips and the two extra color bits (PL0/PL1).
// HROM/CROM have 15 tile address bits; Racin' Force only populates half of each.
TILE_GET_INFO_MEMBER(konamigx_state::get_gx_psac1a_tile_info)
{
	u32 const attr = m_psacram[tile_index * 2 + 1];
	int const flip = (BIT(attr, 23) ? TILE_FLIPX : 0) | (BIT(attr, 22) ? TILE_FLIPY : 0);
	u32 const height = m_psacram[tile_index * 2];
	tileinfo.set(1, height & 0x7fff, (height >> 16) & 0xff, flip);
}

TILE_GET_INFO_MEMBER(konamigx_state::get_gx_psac1b_tile_info)
{
	u32 const attr = m_psacram[tile_index * 2 + 1];
	int const flip = (BIT(attr, 21) ? TILE_FLIPX : 0) | (BIT(attr, 20) ? TILE_FLIPY : 0);
	tileinfo.set(0, attr & 0x7fff, (attr >> 18) & 3, flip);
}

K056832_CB_MEMBER(konamigx_state::type2_tile_callback)
{
	int d = code;

	code = (m_gx_tilebanks[(d & 0xe000) >> 13] << 13) + (d & 0x1fff);

	// The tile's own mix code (K055555 p.62 7.2.6): its colour bits 5:4 where
	// V INMIX ON does not route them to the palette, V INMIX's where it does.
	// It becomes the tile's category, and gx_draw_basic_tilemaps draws each
	// category with that code's K054338 level. -1 (V INMIX ON = 3, no bits
	// from the tile) is category 0, the layer's internal code.
	const int emx = K055555GX_decode_vmixcolor(layer, color);
	priority = emx > 0 ? emx : 0;
}

// The mix code was read from attr bits 5:4 (salmndr2) and 7:6 (alpha) here
// before: those are colour bits 5:4 after get_tile_info's FBIT normalisation
// (k056832_shiftmasks for fbits 0 and 1), so both are what the decode reads.
K056832_CB_MEMBER(konamigx_state::salmndr2_tile_callback)
{
	type2_tile_callback(layer, code, color, flags, priority, attr);
}

K056832_CB_MEMBER(konamigx_state::alpha_tile_callback)
{
	type2_tile_callback(layer, code, color, flags, priority, attr);
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
	m_type1_terrain = std::make_unique<bitmap_ind16>(512, 512);
	m_type1_terrain_priority = std::make_unique<bitmap_ind8>(512, 512);
	save_item(NAME(m_type1_bank));
	save_item(NAME(m_type1_lookup));
	save_item(NAME(m_type1_yorigin));
	save_item(NAME(m_type1_yorigin_valid));
	common_init();

	m_k056832->set_layer_offs(0, -2+1, 0);
	m_k056832->set_layer_offs(1,  0+1, 0);
	m_k056832->set_layer_offs(2,  2+1, 0);
	m_k056832->set_layer_offs(3,  3+1, 0);

	m_gx_psac_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(konamigx_state::get_gx_psac1a_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 128, 128);
	m_gx_psac_tilemap2 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(konamigx_state::get_gx_psac1b_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 128, 128);

	// transparency will be handled manually in post-processing
	//m_gx_psac_tilemap->set_transparent_pen(0);
	//m_gx_psac_tilemap2->set_transparent_pen(0);

	m_gx_rozenable = 0;
	m_gx_specialrozenable = 1;

	m_gxtype1_roz_dstbitmap =  std::make_unique<bitmap_ind16>(512, 512); // BITMAP_FORMAT_IND16 because we NEED the raw pen data for post-processing
	m_gxtype1_roz_dstbitmap2 = std::make_unique<bitmap_ind16>(512, 512); // BITMAP_FORMAT_IND16 because we NEED the raw pen data for post-processing

	m_gxtype1_roz_dstbitmapclip.set(0, 512-1, 0, 512-1);

	// draw_roz uses the screen priority bitmap even for these off-screen views.
	m_screen->priority().allocate(512, 512);
}

VIDEO_START_MEMBER(konamigx_state, racinfrc)
{
	m_type1_terrain = std::make_unique<bitmap_ind16>(512, 512);
	m_type1_terrain_priority = std::make_unique<bitmap_ind8>(512, 512);
	save_item(NAME(m_type1_bank));
	save_item(NAME(m_type1_lookup));
	save_item(NAME(m_type1_yorigin));
	save_item(NAME(m_type1_yorigin_valid));
	common_init();

	m_k056832->set_layer_offs(0, -2+1, -16);
	m_k056832->set_layer_offs(1,  0+1, -16);
	m_k056832->set_layer_offs(2,  2+1, -16);
	m_k056832->set_layer_offs(3,  3+1, -16);

	m_gx_psac_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(konamigx_state::get_gx_psac1a_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 128, 128);
	m_gx_psac_tilemap2 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(konamigx_state::get_gx_psac1b_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 128, 128);

	// transparency will be handled manually in post-processing
	//m_gx_psac_tilemap->set_transparent_pen(0);
	//m_gx_psac_tilemap2->set_transparent_pen(0);

	m_gx_rozenable = 0;
	m_gx_specialrozenable = 1;

	m_gxtype1_roz_dstbitmap =  std::make_unique<bitmap_ind16>(512, 512); // BITMAP_FORMAT_IND16 because we NEED the raw pen data for post-processing
	m_gxtype1_roz_dstbitmap2 = std::make_unique<bitmap_ind16>(512, 512); // BITMAP_FORMAT_IND16 because we NEED the raw pen data for post-processing

	m_gxtype1_roz_dstbitmapclip.set(0, 512-1, 0, 512-1);

	// draw_roz uses the screen priority bitmap even for these off-screen views.
	m_screen->priority().allocate(512, 512);
}

void konamigx_state::type1_vblank_w(int state)
{
	if (state)
	{
		// VIDEO_UPDATE_AFTER_VBLANK
		return;
	}

	bool const golf = m_gfxdecode->gfx(0)->granularity() == 256;
	int const phase = golf ? BIT(m_type1_roz->ctrl_r(0x0e), 7) : 0;
	m_type1_yorigin[phase] = (m_type1_psac4_ctrl[0] >> 8) & 0xffff;
	m_type1_yorigin_valid |= 1 << phase;
}

// Approximate PSAC4 height-field renderer. The two games upload matching
// eight-byte PSAC2 and four-byte PSAC4 records, separated by 14 raster lines.
// PSAC4 consumes color, ROM height and a tile-wide height byte.
void konamigx_state::type1_draw_terrain(screen_device &screen)
{
	bool const golf = m_gfxdecode->gfx(0)->granularity() == 256;

	bitmap_ind16 &height = *m_gxtype1_roz_dstbitmap;
	bitmap_ind16 &color = *m_gxtype1_roz_dstbitmap2;
	height.fill(0);
	color.fill(0);
	m_type1_terrain->fill(0xffff);
	m_type1_priority_used.fill(false);
	if (!(m_type1_roz->ctrl_r(7) & 0x20))
		return;

	m_type1_roz->zoom_draw(screen, height, m_gxtype1_roz_dstbitmapclip, m_gx_psac_tilemap, 0, 0, 0);
	m_type1_roz->zoom_draw(screen, color, m_gxtype1_roz_dstbitmapclip, m_gx_psac_tilemap2, 0, 0, 0);

	// Traverse near to far and fill newly exposed portions of each column.
	// This approximates the vertical faces and occlusion without drawing behind
	// nearer terrain. A transparent sample must not hide more distant scenery.
	std::array<int, 512> limit;
	limit.fill(screen.visible_area().max_y + 1);
	for (int line = 1; line <= (golf ? 480 : 255); line++)
	{
		u32 const entry = m_type1_psac4_lram[line];
		u8 const parameter = entry >> 8;
		if (parameter == 0xff)
		{
			continue;
		}

		int const bank = golf && line > 224;
		if (!BIT(m_type1_yorigin_valid, bank))
			continue;
		int const origin = 256 + m_type1_yorigin[bank];
		// The displacement wraps past 0x7f during elevated camera views.
		int const ground = origin - line - s8(entry);
		int const scale = entry >> 16;
		// SRAM 22N maps PR to COL8-10 and MIX. PR0 selects a nibble,
		// PR1-7 address the byte, and BRK0-3 select one of sixteen banks.
		u8 const lookup = type1_lookup_r(parameter >> 1) >> (BIT(parameter, 0) ? 0 : 4);
		for (int x = 0; x < 512; x++)
		{
			u16 const c = color.pix(line + 14, x);
			// Transparency comes from CROM. A combined height of 0xff is
			// valid, including on the flat Racin' Force Konami logo.
			if (!(c & (golf ? 0xff : 0x3f)))
				continue;
			u16 const h = height.pix(line + 14, x);
			int const level = ((h & 0x3f) + (h >> 6)) & 0xff;

			// Racin' Force's LRAM projection uses 6 fractional bits: the
			// terrain height must use the same units as the camera height.
			int const top = std::max(ground - level * scale / (golf ? 256 : 64), 0);
			u16 const pen = (c & 0xff) | ((lookup & 0xf) << 8);
			if (!scale)
			{
				// Flat ROZ artwork (e.g. the Racin' Force title) has no height
				// extrusion; transparency must remain intact between its rows.
				if (ground >= 0 && ground <= screen.visible_area().max_y)
				{
					m_type1_terrain->pix(ground, x) = pen;
					m_type1_terrain_priority->pix(ground, x) = parameter;
					m_type1_priority_used[parameter] = true;
				}
				continue;
			}
			for (int y = top; y < limit[x]; y++)
			{
				m_type1_terrain->pix(y, x) = pen;
				m_type1_terrain_priority->pix(y, x) = parameter;
				m_type1_priority_used[parameter] = true;
			}
			limit[x] = std::min(limit[x], top);
		}
	}
}

void konamigx_state::type1_mix_terrain(bitmap_rgb32 &bitmap, const rectangle &cliprect, u8 priority)
{
	// The Type-1 board provides one external mix bit and no brightness bits.
	u8 const bri_mask = m_k055555->K055555_read_register(K55_OSBRI_ON) >> 4 & 3;
	u8 const bri_mode = (m_k055555->K055555_read_register(K55_OSBRI) >> 4) & bri_mask;
	u8 const brightness = bri_mode ? m_brightness[bri_mode - 1] : 0xff;
	if (m_current_brightness != brightness)
	{
		m_current_brightness = brightness;
		for (int p = 0; p < m_palette->entries(); p++)
			m_palette->set_pen_contrast(p, brightness / 255.0);
	}

	u8 const mix_mask = m_osmixon >> 4 & 3;
	u8 const mix_internal = (m_osinmix >> 4) & mix_mask;
	int alpha[2];
	for (int m = 0; m < 2; m++)
	{
		// Match the GX mixer's existing approximation for additive modes.
		int const level = m_k054338->set_alpha_level((m & ~mix_mask) | mix_internal);
		alpha[m] = (level & 0x100) ? (~level & 0xff) : (level & 0xff);
	}
	pen_t const *const paldata = m_palette->pens();
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			u16 const pen = m_type1_terrain->pix(y, x);
			if (pen != 0xffff && m_type1_terrain_priority->pix(y, x) == priority)
			{
				int const a = alpha[BIT(pen, 11)];
				u32 const rgb = paldata[pen & 0x7ff];
				if (a == 255)
					bitmap.pix(y, x) = rgb;
				else if (a)
					bitmap.pix(y, x) = alpha_blend_r32(bitmap.pix(y, x), rgb, a);
			}
		}
}

u32 konamigx_state::screen_update_konamigx(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
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

	if (m_gx_specialrozenable == 1)
	{
		type1_draw_terrain(screen);
		konamigx_mixer(screen, bitmap, cliprect, nullptr, 0, nullptr, 0, 0, m_type1_terrain.get(), m_gx_rushingheroes_hack);
	}
	else if (m_gx_specialrozenable == 3)
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
		konamigx_mixer(screen, bitmap, cliprect, nullptr, 0, nullptr, 0, 0, nullptr, m_gx_rushingheroes_hack);
	}

	return 0;
}

u32 konamigx_state::screen_update_konamigx_left(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* the video gets demuxed by a board which plugs into the jamma connector */
	m_konamigx_current_frame ^= 1;

	if (m_konamigx_current_frame == 1)
	{
		int offset = 0;

		if (m_konamigx_palformat == 1)
		{
			for (offset = 0; offset < 0x4000/4; offset++)
			{
				u32 coldat = m_generic_paletteram_32[offset];

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

u32 konamigx_state::screen_update_konamigx_right(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_konamigx_current_frame == 1)
	{
		copybitmap(bitmap, *m_dualscreen_right_tempbitmap, 0, 0, 0, 0, cliprect);
	}
	else
	{
		int offset = 0;

		if (m_konamigx_palformat == 1)
		{
			for (offset = 0; offset < 0x4000/4; offset++)
			{
				u32 coldat = m_subpaletteram32[offset];

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

static inline void set_color_555(palette_device &palette, pen_t color, int rshift, int gshift, int bshift, u16 data)
{
	palette.set_pen_color(color, pal5bit(data >> rshift), pal5bit(data >> gshift), pal5bit(data >> bshift));
}

// The CLTC RAM stores only RGB. Its dummy byte ignores writes and always
// reads zero (GX manual, section 8.4.1). Racin' Force relies on this when
// reusing a register from palette fading to calculate the sky's Y scroll.
u32 konamigx_state::konamigx_palette_r(offs_t offset)
{
	return m_palette->read32(offset) & 0x00ffffff;
}

void konamigx_state::konamigx_palette_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_palette->write32(offset, data, mem_mask & 0x00ffffff);
}

// main monitor for type 3
void konamigx_state::konamigx_555_palette_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 coldat;
	COMBINE_DATA(&m_generic_paletteram_32[offset]);

	coldat = m_generic_paletteram_32[offset];

	set_color_555(*m_palette, offset*2, 0, 5, 10, coldat >> 16);
	set_color_555(*m_palette, offset*2 + 1, 0, 5, 10, coldat & 0xffff);
}

// sub monitor for type 3
void konamigx_state::konamigx_555_palette2_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 coldat;
	COMBINE_DATA(&m_subpaletteram32[offset]);
	coldat = m_subpaletteram32[offset];

	offset += (0x4000/4);

	set_color_555(*m_palette, offset*2, 0, 5, 10, coldat >> 16);
	set_color_555(*m_palette, offset*2 + 1, 0, 5, 10, coldat & 0xffff);
}

void konamigx_state::konamigx_tilebank_w(offs_t offset, u32 data, u32 mem_mask)
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
void konamigx_state::konamigx_t1_psacmap_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_psacram[offset]);
	m_gx_psac_tilemap->mark_tile_dirty(offset/2);
	m_gx_psac_tilemap2->mark_tile_dirty(offset/2);
}

// type 4 RAM-based PSAC tilemap
void konamigx_state::konamigx_t4_psacmap_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_psacram[offset]);

	m_gx_psac_tilemap->mark_tile_dirty(offset*2);
	m_gx_psac_tilemap->mark_tile_dirty((offset*2) + 1);
}
