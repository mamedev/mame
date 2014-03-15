/*
 * video/konamigx.c - Konami GX video hardware (here there be dragons, and achocode)
 *
 */

#include "emu.h"
#include "video/k053250.h"
#include "includes/konamigx.h"


//#define GX_DEBUG
#define VERBOSE 0

/***************************************************************************/
/*                                                                         */
/*                     2nd-Tier GX/MW Graphics Functions                   */
/*                                                                         */
/***************************************************************************/



static UINT8 *gx_objzbuf, *gx_shdzbuf;



static int layer_colorbase[4];
static INT32 gx_tilebanks[8], gx_oldbanks[8];
static int gx_tilemode, gx_rozenable, psac_colorbase, last_psac_colorbase;
static int gx_specialrozenable; // type 1 roz, with voxel height-map, rendered from 2 source tilemaps (which include height data) to temp bitmap for further processing
static int gx_rushingheroes_hack;
static int gx_le2_textcolour_hack;
static tilemap_t *gx_psac_tilemap, *gx_psac_tilemap2;
static bitmap_ind16* type3_roz_temp_bitmap;
static tilemap_t* gx_psac_tilemap_alt;

static int konamigx_has_dual_screen;
int konamigx_current_frame;
INLINE void set_color_555(palette_device &palette, pen_t color, int rshift, int gshift, int bshift, UINT16 data);
static int konamigx_palformat;
static bitmap_rgb32* dualscreen_left_tempbitmap;
static bitmap_rgb32* dualscreen_right_tempbitmap;

/* On Type-1 the K053936 output is rendered to these temporary bitmaps as raw data
   the 'voxel' effect to give the pixels height is a post-process operation on the
   output of the K053936 (this can clearly be seen in videos as large chunks of
   scenary flicker when in the distance due to single pixels in the K053936 output
   becoming visible / invisible due to drawing precision.

   -- however, progress on this has stalled as our K053936 doesn't seem to give
      the right output for post processing, I suspect the game is using some
      unsupported flipping modes (probably due to the way it's hooked up to the
      rest of the chips) which is causing entirely the wrong output.

   -- furthermore machine/konamigx.c (!) contains it's own implementation of
      the K053936_zoom_draw named K053936GP_zoom_draw ... It really shouldn't do,
      epsecially not in 'machine', which isn't meant to be video related.


   */
static bitmap_ind16 *gxtype1_roz_dstbitmap;
static bitmap_ind16 *gxtype1_roz_dstbitmap2;
static rectangle gxtype1_roz_dstbitmapclip;

static void (*game_tile_callback)(running_machine &machine, int layer, int *code, int *color, int *flags);



/***************************************************************************/
/*                                                                         */
/*                 1st-Tier GX/MW Variables and Functions                  */
/*                                                                         */
/***************************************************************************/

// global system ports access
UINT8  konamigx_wrport1_0, konamigx_wrport1_1;
UINT16 konamigx_wrport2;

// frequently used registers
static int k053247_vrcbk[4];
static int k053247_coreg, k053247_coregshift, k053247_opset;
static int opri, oinprion;
static int vcblk[6], ocblk;
static int vinmix, vmixon, osinmix, osmixon;


void konamigx_state::konamigx_precache_registers(void)
{
	// (see sprite color coding scheme on p.46 & 47)
	static const int coregmasks[5] = {0xf,0xe,0xc,0x8,0x0};
	static const int coregshifts[5]= {4,5,6,7,8};
	int i;

	i = m_k055673->k053247_read_register(0x8/2);
	k053247_vrcbk[0] = (i & 0x000f) << 14;
	k053247_vrcbk[1] = (i & 0x0f00) << 6;
	i = m_k055673->k053247_read_register(0xa/2);
	k053247_vrcbk[2] = (i & 0x000f) << 14;
	k053247_vrcbk[3] = (i & 0x0f00) << 6;

	// COREG == OBJSET2+1C == bit8-11 of OPSET ??? (see p.50 last table, needs p.49 to confirm)
	k053247_opset = m_k055673->k053247_read_register(0xc/2);

	i = k053247_opset & 7; if (i > 4) i = 4;

	k053247_coreg = m_k055673->k053247_read_register(0xc/2)>>8 & 0xf;
	k053247_coreg =(k053247_coreg & coregmasks[i]) << 12;

	k053247_coregshift = coregshifts[i];

	opri     = m_k055555->K055555_read_register(K55_PRIINP_8);
	oinprion = m_k055555->K055555_read_register(K55_OINPRI_ON);
	vcblk[0] = m_k055555->K055555_read_register(K55_PALBASE_A);
	vcblk[1] = m_k055555->K055555_read_register(K55_PALBASE_B);
	vcblk[2] = m_k055555->K055555_read_register(K55_PALBASE_C);
	vcblk[3] = m_k055555->K055555_read_register(K55_PALBASE_D);
	vcblk[4] = m_k055555->K055555_read_register(K55_PALBASE_SUB1);
	vcblk[5] = m_k055555->K055555_read_register(K55_PALBASE_SUB2);
	ocblk    = m_k055555->K055555_read_register(K55_PALBASE_OBJ);
	vinmix   = m_k055555->K055555_read_register(K55_BLEND_ENABLES);
	vmixon   = m_k055555->K055555_read_register(K55_VINMIX_ON);
	osinmix  = m_k055555->K055555_read_register(K55_OSBLEND_ENABLES);
	osmixon  = m_k055555->K055555_read_register(K55_OSBLEND_ON);
}

INLINE int K053247GX_combine_c18(int attrib) // (see p.46)
{
	int c18;

	c18 = (attrib & 0xff)<<k053247_coregshift | k053247_coreg;

	if (konamigx_wrport2 & 4) c18 &= 0x3fff; else
	if (!(konamigx_wrport2 & 8)) c18 = (c18 & 0x3fff) | (attrib<<6 & 0xc000);

	return(c18);
}

INLINE int K055555GX_decode_objcolor(int c18) // (see p.59 7.2.2)
{
	int ocb, opon;

	opon  = oinprion<<8 | 0xff;
	ocb   = (ocblk & 7) << 10;
	c18  &= opon;
	ocb  &=~opon;

	return((ocb | c18) >> k053247_coregshift);
}

INLINE int K055555GX_decode_inpri(int c18) // (see p.59 7.2.2)
{
	int op = opri;

	c18 >>= 8;
	op   &= oinprion;
	c18  &=~oinprion;

	return(c18 | op);
}

static void konamigx_type2_sprite_callback(running_machine &machine, int *code, int *color, int *priority)
{
	int num = *code;
	int c18 = *color;

	*code = k053247_vrcbk[num>>14] | (num & 0x3fff);
	c18 = K053247GX_combine_c18(c18);
	*color = K055555GX_decode_objcolor(c18);
	*priority = K055555GX_decode_inpri(c18);
}

static void konamigx_dragoonj_sprite_callback(running_machine &machine, int *code, int *color, int *priority)
{
	int num, op, pri, c18;

	num = *code;
	*code = k053247_vrcbk[num>>14] | (num & 0x3fff);

	c18  = pri = *color;
	op   = opri;
	pri  = (pri & 0x200) ? 4 : pri>>4 & 0xf;
	op  &= oinprion;
	pri &=~oinprion;
	*priority = pri | op;

	c18 = K053247GX_combine_c18(c18);
	*color = K055555GX_decode_objcolor(c18);
}

static void konamigx_salmndr2_sprite_callback(running_machine &machine, int *code, int *color, int *priority)
{
	int num, op, pri, c18;

	num = *code;
	*code = k053247_vrcbk[num>>14] | (num & 0x3fff);

	c18  = pri = *color;
	op   = opri;
	pri  = pri>>4 & 0x3f;
	op  &= oinprion;
	pri &=~oinprion;
	*priority = pri | op;

	c18 = K053247GX_combine_c18(c18);
	*color = K055555GX_decode_objcolor(c18);
}

static void konamigx_le2_sprite_callback(running_machine &machine, int *code, int *color, int *priority)
{
	int num, op, pri;

	num = *code;
	*code = k053247_vrcbk[num>>14] | (num & 0x3fff);

	pri = *color;
	*color &= 0x1f;

	op   = opri;
	pri &= 0xf0;
	op  &= oinprion;
	pri &=~oinprion;
	*priority = pri | op;
}

static int K055555GX_decode_vmixcolor(int layer, int *color) // (see p.62 7.2.6 and p.27 3.3)
{
	int vcb, shift, pal, vmx, von, pl45, emx;

	vcb    =  vcblk[layer]<<6;
	shift  =  layer<<1;
	pal    =  *color;
	vmx    =  vinmix>>shift & 3;
	von    =  vmixon>>shift & 3;
	emx    =  pl45 = pal>>4 & 3;
	pal   &=  0xf;
	pl45  &=  von;
	vmx   &=  von;
	pl45 <<=  4;
	emx   &= ~von;
	pal   |=  pl45;
	emx   |=  vmx;
	pal   |=  vcb;

	if (gx_le2_textcolour_hack)
		if (layer==0)
			pal |= 0x1c0;

	if (von == 3) emx = -1; // invalidate external mix code if all bits are from internal
	*color =  pal;

	return(emx);
}

#ifdef UNUSED_FUNCTION
int K055555GX_decode_osmixcolor(int layer, int *color) // (see p.63, p.49-50 and p.27 3.3)
{
	int scb, shift, pal, osmx, oson, pl45, emx;

	shift  =  layer<<1;
	pal    =  *color;
	osmx   =  osinmix>>shift & 3;
	oson   =  osmixon>>shift & 3;

	if (layer)
	{
		// layer 1-3 are external tile layers
		scb    =  vcblk[layer+3]<<6;
		emx    =  pl45 = pal>>4 & 3;
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
		emx   = 0; // k053247_read_register(??)>>? & 3;
		osmx &= oson;
		emx  &=~oson;
		emx  |= osmx;
	}

	return(emx);
}
#endif

static void gx_wipezbuf(running_machine &machine, int noshadow)
{
	const rectangle &visarea = machine.first_screen()->visible_area();

	int w = visarea.width();
	int h = visarea.height();

	UINT8 *zptr = gx_objzbuf;
	int ecx = h;

	do { memset(zptr, -1, w); zptr += GX_ZBUFW; } while (--ecx);

	if (!noshadow)
	{
		zptr = gx_shdzbuf;
		w <<= 1;
		ecx = h;
		do { memset(zptr, -1, w); zptr += (GX_ZBUFW<<1); } while (--ecx);
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
#define GX_MAX_SPRITES 512
#define GX_MAX_LAYERS  6
#define GX_MAX_OBJECTS (GX_MAX_SPRITES + GX_MAX_LAYERS)

static struct GX_OBJ { int order, offs, code, color; } *gx_objpool;
static UINT16 *gx_spriteram;
static int gx_objdma, gx_primode;

// mirrored K053247 and K054338 settings
static void (*k053247_callback)(running_machine &machine, int *code,int *color,int *priority);


static int *K054338_shdRGB;


void konamigx_state::konamigx_mixer_init(screen_device &screen, int objdma)
{
	gx_objdma = 0;
	gx_primode = 0;

	gx_objzbuf = &screen.priority().pix8(0);
	gx_shdzbuf = auto_alloc_array(machine(), UINT8, GX_ZBUFSIZE);
	gx_objpool = auto_alloc_array(machine(), struct GX_OBJ, GX_MAX_OBJECTS);

	m_k055673->alt_k053247_export_config(&k053247_callback);
	K054338_export_config(&K054338_shdRGB);

	if (objdma)
	{
		gx_spriteram = auto_alloc_array(machine(), UINT16, 0x1000/2);
		gx_objdma = 1;
	}
	else
		m_k055673->k053247_get_ram(&gx_spriteram);

	m_palette->set_shadow_dRGB32(3,-80,-80,-80, 0);
	K054338_invert_alpha(1);
}

void konamigx_mixer_primode(int mode)
{
	gx_primode = mode;
}

void konamigx_state::konamigx_objdma(void)
{
	UINT16* k053247_ram;
	m_k055673->k053247_get_ram(&k053247_ram);

	if (gx_objdma && gx_spriteram && k053247_ram) memcpy(gx_spriteram, k053247_ram, 0x1000);
}

void konamigx_state::konamigx_mixer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect,
					tilemap_t *sub1, int sub1flags,
					tilemap_t *sub2, int sub2flags,
					int mixerflags, bitmap_ind16 *extra_bitmap, int rushingheroes_hack)
{
	int objbuf[GX_MAX_OBJECTS];
	int shadowon[3], shdpri[3], layerid[6], layerpri[6];

	struct GX_OBJ *objpool, *objptr;
	int cltc_shdpri, /*prflp,*/ disp;

	// buffer can move when it's resized, so refresh the pointer
	gx_objzbuf = &screen.priority().pix8(0);

	// abort if object database failed to initialize
	objpool = gx_objpool;
	if (!objpool) return;

	// clear screen with backcolor and update flicker pulse
	K054338_fill_backcolor(machine(), m_screen, bitmap, konamigx_wrport1_0 & 0x20);


	// abort if video has been disabled
	disp = m_k055555->K055555_read_register(K55_INPUT_ENABLES);
	if (!disp) return;
	cltc_shdpri = K054338_read_register(K338_REG_CONTROL);


	if (!rushingheroes_hack) // Slam Dunk 2 never sets this.  It's either part of the protection, or type4 doesn't use it
	{
		if (!(cltc_shdpri & K338_CTL_KILL)) return;
	}


	// demote shadows by one layer when this bit is set??? (see p.73 8.6)
	cltc_shdpri &= K338_CTL_SHDPRI;

	// wipe z-buffer
	if (mixerflags & GXMIX_NOZBUF)
		mixerflags |= GXMIX_NOSHADOW;
	else
		gx_wipezbuf(machine(), mixerflags & GXMIX_NOSHADOW);

	// cache global parameters
	konamigx_precache_registers();

	// init OBJSET2 and mixer parameters (see p.51 and chapter 7)
	layerid[0] = 0; layerid[1] = 1; layerid[2] = 2; layerid[3] = 3; layerid[4] = 4; layerid[5] = 5;


	// invert layer priority when this flag is set (not used by any GX game?)
	//prflp = K055555_read_register(K55_CONTROL) & K55_CTL_FLIPPRI;

	layerpri[0] = m_k055555->K055555_read_register(K55_PRIINP_0);
	layerpri[1] = m_k055555->K055555_read_register(K55_PRIINP_3);
	layerpri[3] = m_k055555->K055555_read_register(K55_PRIINP_7);
	layerpri[4] = m_k055555->K055555_read_register(K55_PRIINP_9);
	layerpri[5] = m_k055555->K055555_read_register(K55_PRIINP_10);

	int shdprisel;

	if (gx_primode == -1)
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
	if (!(shdprisel & 0x03)) shadowon[0] = 0;
	if (!(shdprisel & 0x0c)) shadowon[1] = 0;
	if (!(shdprisel & 0x30)) shadowon[2] = 0;

	shdpri[0]   = m_k055555->K055555_read_register(K55_SHAD1_PRI);
	shdpri[1]   = m_k055555->K055555_read_register(K55_SHAD2_PRI);
	shdpri[2]   = m_k055555->K055555_read_register(K55_SHAD3_PRI);

	int spri_min = 0;

	shadowon[2] = shadowon[1] = shadowon[0] = 0;

	int k = 0;
	if (!(mixerflags & GXMIX_NOSHADOW))
	{
		int i,j;
		// only enable shadows beyond a +/-7 RGB threshold
		for (j=0,i=0; i<3; j+=3,i++)
		{
			k = K054338_shdRGB[j  ]; if (k < -7 || k > 7) { shadowon[i] = 1; continue; }
			k = K054338_shdRGB[j+1]; if (k < -7 || k > 7) { shadowon[i] = 1; continue; }
			k = K054338_shdRGB[j+2]; if (k < -7 || k > 7) { shadowon[i] = 1; }
		}

		// SHDON specifies layers on which shadows can be projected (see detail on p.65 7.2.8)
		int temp = m_k055555->K055555_read_register(K55_SHD_ON);
		for (i=0; i<4; i++) if (!(temp>>i & 1) && spri_min < layerpri[i]) spri_min = layerpri[i]; // HACK

		// update shadows status
		K054338_update_all_shadows(machine(), rushingheroes_hack, m_palette);
	}

	// pre-sort layers
	for (int j=0; j<5; j++)
	{
		int temp1 = layerpri[j];
		for (int i=j+1; i<6; i++)
		{
			int temp2 = layerpri[i];
			if ((UINT32)temp1 <= (UINT32)temp2)
			{
				layerpri[i] = temp1; layerpri[j] = temp1 = temp2;
				temp2 = layerid[i]; layerid[i] = layerid[j]; layerid[j] = temp2;
			}
		}
	}

	// build object database and create indices
	objptr = objpool;
	int nobj = 0;

	for (int i=5; i>=0; i--)
	{
		int offs;

		int code = layerid[i];
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
			case 4 :
				offs = -128;
				if (sub1flags & 0xf) { if (sub1flags & GXSUB_K053250) offs = -4; else if (sub1) offs = -2; }
			break;
			case 5 :
				offs = -128;
				if (sub2flags & 0xf) { if (sub2flags & GXSUB_K053250) offs = -5; else if (sub2) offs = -3; }
				if (extra_bitmap) offs = -3;
			break;
			default: offs = -1;
		}

		if (offs != -128)
		{
			objptr->order = layerpri[i]<<24;
			objptr->code  = code;
			objptr->offs = offs;
			objptr++;

			objbuf[nobj] = nobj;
			nobj++;
		}
	}

//  i = j = 0xff;
	int l = 0;

	for (int offs=0; offs<0x800; offs+=8)
	{
		int pri = 0;

		if (!(gx_spriteram[offs] & 0x8000)) continue;

		int zcode = gx_spriteram[offs] & 0xff;

		// invert z-order when opset_pri is set (see p.51 OPSET PRI)
		if (k053247_opset & 0x10) zcode = 0xff - zcode;

		int code  = gx_spriteram[offs+1];
		int color = k = gx_spriteram[offs+6];
		l     = gx_spriteram[offs+7];

		(*k053247_callback)(machine(), &code, &color, &pri);

		/*
		    shadow = shadow code
		    spri   = shadow priority
		    temp1  = add solid object
		    temp2  = solid pens draw mode
		    temp3  = add shadow object
		    temp4  = shadow pens draw mode
		*/
		int temp4 = 0;
		int temp3 = 0;
		int temp2 = 0;
		int temp1 = 0;
		int spri = 0;
		int shadow = 0;

		if (color & K055555_FULLSHADOW)
		{
			shadow = 3; // use default intensity and color
			spri = pri; // retain host priority
			temp3 = 1; // add shadow
			temp4 = 5; // draw full shadow
		}
		else
		{
			shadow = k>>10 & 3;
			if (shadow) // object has shadow?
			{
				int k053246_objset1 = m_k055673->k053246_read_register(5);
				if (shadow != 1 || k053246_objset1 & 0x20)
				{
					shadow--;
					temp1 = 1; // add solid
					temp2 = 1; // draw partial solid
					if (shadowon[shadow])
					{
						temp3 = 1; // add shadow
						temp4 = 4; // draw partial shadow
					}
				}
				else
				{
					// drop the entire sprite to shadow if its shadow code is 1 and SD0EN is off (see p.48)
					shadow = 0;
					if (!shadowon[0]) continue;
					temp3 = 1; // add shadow
					temp4 = 5; // draw full shadow
				}
			}
			else
			{
				temp1 = 1; // add solid
				temp2 = 0; // draw full solid
			}

			if (temp1)
			{
				// tag sprite for alpha blending
				if (color>>K055555_MIXSHIFT & 3) temp2 |= 2;
			}

			if (temp3)
			{
				// determine shadow priority
				spri = (k053247_opset & 0x20) ? pri : shdpri[shadow]; // (see p.51 OPSET SDSEL)
			}
		}

		switch (gx_primode & 0xf)
		{
			// Dadandarn zcode suppression
			case  1:
				zcode = 0;
			break;

			// Daisukiss bad shadow filter
			case  4:
				if (k & 0x3000 || k == 0x0800) continue;

			// Tokkae shadow masking (INACCURATE)
			case  5:
				if (spri < spri_min) spri = spri_min;
			break;
		}

		/*
		    default sort order:
		    fedcba9876543210fedcba9876543210
		    xxxxxxxx------------------------ (priority)
		    --------xxxxxxxx---------------- (zcode)
		    ----------------xxxxxxxx-------- (offset)
		    ------------------------xxxx---- (shadow mode)
		    ----------------------------xxxx (shadow code)
		*/
		if (temp1)
		{
			// add objects with solid or alpha pens
			int order = pri<<24 | zcode<<16 | offs<<(8-3) | temp2<<4;
			objptr->order = order;
			objptr->offs  = offs;
			objptr->code  = code;
			objptr->color = color;
			objptr++;

			objbuf[nobj] = nobj;
			nobj++;
		}

		if (temp3 && !(color & K055555_SKIPSHADOW) && !(mixerflags & GXMIX_NOSHADOW))
		{
			// add objects with shadows if enabled
			int order = spri<<24 | zcode<<16 | offs<<(8-3) | temp4<<4 | shadow;
			objptr->order = order;
			objptr->offs  = offs;
			objptr->code  = code;
			objptr->color = color;
			objptr++;

			objbuf[nobj] = nobj;
			nobj++;
		}
	}

	// sort objects in decending order (SLOW)
	k = nobj;
	l = nobj - 1;

	for (int j=0; j<l; j++)
	{
		int temp1 = objbuf[j];
		int temp2 = objpool[temp1].order;
		for (int i=j+1; i<k; i++)
		{
			int temp3 = objbuf[i];
			int temp4 = objpool[temp3].order;
			if ((UINT32)temp2 <= (UINT32)temp4) { temp2 = temp4; objbuf[i] = temp1; objbuf[j] = temp1 = temp3; }
		}
	}


	konamigx_mixer_draw(screen,bitmap,cliprect,sub1,sub1flags,sub2,sub2flags,mixerflags,extra_bitmap,rushingheroes_hack,
		objpool,
		objbuf,
		nobj
		);
}

void konamigx_state::gx_draw_basic_tilemaps(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int mixerflags, int code)
{
	int temp1,temp2,temp3,temp4;
	int i = code<<1;
	int j = mixerflags>>i & 3;
	int k = 0;

	int disp = m_k055555->K055555_read_register(K55_INPUT_ENABLES);
	if (disp & (1<<code))
	{
		if (j == GXMIX_BLEND_NONE)  { temp1 = 0xff; temp2 = temp3 = 0; } else
		if (j == GXMIX_BLEND_FORCE) { temp1 = 0x00; temp2 = mixerflags>>(i+16); temp3 = 3; }
		else
		{
			temp1 = vinmix;
			temp2 = vinmix>>i & 3;
			temp3 = vmixon>>i & 3;
		}

		/* blend layer only when:
		    1) vinmix != 0xff
		    2) its internal mix code is set
		    3) all mix code bits are internal(overriden until tile blending has been implemented)
		    4) 0 > alpha < 255;
		*/
		if (temp1!=0xff && temp2 /*&& temp3==3*/)
		{
			temp4 = K054338_set_alpha_level(temp2);

			if (temp4 <= 0) return;
			if (temp4 < 255) k = TILEMAP_DRAW_ALPHA(temp4);
		}

		if (mixerflags & 1<<(code+12)) k |= K056382_DRAW_FLAG_FORCE_XYSCROLL;

		m_k056832->m_tilemap_draw(screen, bitmap, cliprect, code, k, 0);
	}
}

void konamigx_state::gx_draw_basic_extended_tilemaps_1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int mixerflags, int code, tilemap_t *sub1, int sub1flags, int rushingheroes_hack, int offs)
{
	int temp1,temp2,temp3,temp4;
	int i = code<<1;
	int j = mixerflags>>i & 3;
	int k = 0;
	static int parity = 0;
	parity ^= 1;

	int disp = m_k055555->K055555_read_register(K55_INPUT_ENABLES);
	if ((disp & K55_INP_SUB1) || (rushingheroes_hack))
	{
		int alpha = 255;

		if (j == GXMIX_BLEND_NONE)  { temp1 = 0xff; temp2 = temp3 = 0; } else
		if (j == GXMIX_BLEND_FORCE) { temp1 = 0x00; temp2 = mixerflags>>24; temp3 = 3; }
		else
		{
			temp1 = osinmix;
			temp2 = osinmix>>2 & 3;
			temp3 = osmixon>>2 & 3;
		}

		if (temp1!=0xff && temp2 /*&& temp3==3*/)
		{
			alpha = temp4 = K054338_set_alpha_level(temp2);

			if (temp4 <= 0) return;
			if (temp4 < 255) k = (j == GXMIX_BLEND_FAST) ? ~parity : 1;
		}

		int l = sub1flags & 0xf;

		if (offs == -2)
		{
			int pixeldouble_output = 0;
			const rectangle &visarea = screen.visible_area();
			int width = visarea.width();

			if (width>512) // vsnetscr case
				pixeldouble_output = 1;

			K053936GP_0_zoom_draw(screen.machine(), bitmap, cliprect, sub1, l, k, alpha, pixeldouble_output, m_k053936_0_ctrl_16, m_k053936_0_linectrl_16, m_k053936_0_ctrl, m_k053936_0_linectrl, m_palette);
		}
		else
		{
			screen.machine().device<k053250_device>("k053250_1")->draw(bitmap, cliprect, vcblk[4]<<l, 0, screen.priority(), 0);
		}
	}
}

void konamigx_state::gx_draw_basic_extended_tilemaps_2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int mixerflags, int code, tilemap_t *sub2, int sub2flags, bitmap_ind16 *extra_bitmap, int offs)
{
	int temp1,temp2,temp3,temp4;
	int i = code<<1;
	int j = mixerflags>>i & 3;
//  int k = 0;
//  static int parity = 0;
//  parity ^= 1;

	int disp = m_k055555->K055555_read_register(K55_INPUT_ENABLES);
	if (disp & K55_INP_SUB2)
	{
		//int alpha = 255;
		if (j == GXMIX_BLEND_NONE)  { temp1 = 0xff; temp2 = temp3 = 0; } else
		if (j == GXMIX_BLEND_FORCE) { temp1 = 0x00; temp2 = mixerflags>>26; temp3 = 3; }
		else
		{
			temp1 = osinmix;
			temp2 = osinmix>>4 & 3;
			temp3 = osmixon>>4 & 3;
		}

		if (temp1!=0xff && temp2 /*&& temp3==3*/)
		{
			//alpha =
			temp4 = K054338_set_alpha_level(temp2);

			if (temp4 <= 0) return;
			//if (temp4 < 255) k = (j == GXMIX_BLEND_FAST) ? ~parity : 1;
		}

		int l = sub2flags & 0xf;

		if (offs == -3)
		{
			if (extra_bitmap) // soccer superstars roz layer
			{
				int xx,yy;
				int width = screen.width();
				int height = screen.height();
				const pen_t *paldata = m_palette->pens();

				// the output size of the roz layer has to be doubled horizontally
				// so that it aligns with the sprites and normal tilemaps.  This appears
				// to be done as a post-processing / mixing step effect
				//
				// - todo, use the pixeldouble_output I just added for vsnet instead?
				for (yy=0;yy<height;yy++)
				{
					UINT16* src = &extra_bitmap->pix16(yy);
					UINT32* dst = &bitmap.pix32(yy);
					int shiftpos = 0;
					for (xx=0;xx<width;xx+=2)
					{
						UINT16 dat = src[(((xx/2)+shiftpos))%width];
						if (dat&0xff)
							dst[xx+1] = dst[xx] = paldata[dat];
					}
				}
			}
			else
			{
			//  int pixeldouble_output = 0;
			//  K053936GP_1_zoom_draw(machine, bitmap, cliprect, sub2, l, k, alpha, pixeldouble_output);
			}
		}
		else
			screen.machine().device<k053250_device>("k053250_2")->draw(bitmap, cliprect, vcblk[5]<<l, 0, screen.priority(), 0);
	}
}

void konamigx_state::konamigx_mixer_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect,
					tilemap_t *sub1, int sub1flags,
					tilemap_t *sub2, int sub2flags,
					int mixerflags, bitmap_ind16 *extra_bitmap, int rushingheroes_hack,

					/* passed from above function */
					struct GX_OBJ *objpool,
					int *objbuf,
					int nobj
					)
{
	// traverse draw list
	int disp = m_k055555->K055555_read_register(K55_INPUT_ENABLES);

	for (int count=0; count<nobj; count++)
	{
		struct GX_OBJ *objptr = objpool + objbuf[count];
		int order  = objptr->order;
		int offs   = objptr->offs;
		int code   = objptr->code;
		int color  = objptr->color;

		/* entries >=0 in our list are sprites */
		if (offs >= 0)
		{
			if (!(disp & K55_INP_OBJ)) continue;

			int drawmode = order>>4 & 0xf;

			int alpha = 255;
			int pri = 0;
			int zcode = -1; // negative zcode values turn off z-buffering

			if (drawmode & 2)
			{
				alpha = color>>K055555_MIXSHIFT & 3;
				if (alpha) alpha = K054338_set_alpha_level(alpha);
				if (alpha <= 0) continue;
			}
			color &= K055555_COLORMASK;

			if (drawmode >= 4) m_palette->set_shadow_mode(order & 0x0f);

			if (!(mixerflags & GXMIX_NOZBUF))
			{
				zcode = order>>16 & 0xff;
				pri = order>>24 & 0xff;
			}




			m_k055673->k053247_draw_single_sprite_gxcore(bitmap, cliprect,
				gx_objzbuf, gx_shdzbuf, code, gx_spriteram, offs,
				color, alpha, drawmode, zcode, pri,
				/* non-gx only */
				0,0,NULL,NULL,0
				);
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


/* Run and Gun 2 / Rushing Heroes */
TILE_GET_INFO_MEMBER(konamigx_state::get_gx_psac_tile_info)
{
	int tileno, colour, col, flip = 0;
	if (tile_index&1)
	{
		tileno = m_psacram[tile_index/2] & 0x00001fff;
		col    =(m_psacram[tile_index/2] & 0x00002000)>>13;
		if      (m_psacram[tile_index/2] & 0x00004000) flip |= TILE_FLIPX;
		if      (m_psacram[tile_index/2] & 0x00008000) flip |= TILE_FLIPY;

	}
	else
	{
		tileno = (m_psacram[tile_index/2] & 0x1fff0000)>>16;
		col    = (m_psacram[tile_index/2] & 0x20000000)>>29;
		if       (m_psacram[tile_index/2] & 0x40000000) flip |= TILE_FLIPX;
		if       (m_psacram[tile_index/2] & 0x80000000) flip |= TILE_FLIPY;

	}

	colour = (psac_colorbase << 4) + col;

	SET_TILE_INFO_MEMBER(0, tileno, colour, TILE_FLIPYX(flip));
}

static int konamigx_type3_psac2_actual_bank;
//int konamigx_type3_psac2_actual_last_bank = 0;

WRITE32_MEMBER(konamigx_state::konamigx_type3_psac2_bank_w)
{
	// other bits are used for something...

	COMBINE_DATA(&m_konamigx_type3_psac2_bank[offset]);
	konamigx_type3_psac2_actual_bank = (m_konamigx_type3_psac2_bank[0] & 0x10000000) >> 28;

	/* handle this by creating 2 roz tilemaps instead, otherwise performance dies completely on dual screen mode
	if (konamigx_type3_psac2_actual_bank!=konamigx_type3_psac2_actual_last_bank)
	{
	    gx_psac_tilemap->mark_all_dirty();
	    konamigx_type3_psac2_actual_last_bank = konamigx_type3_psac2_actual_bank;
	}
	*/
}



/* Soccer Superstars (tile and flip bits now TRUSTED) */
TILE_GET_INFO_MEMBER(konamigx_state::get_gx_psac3_tile_info)
	{
	int tileno, colour, flip;
	UINT8 *tmap = memregion("gfx4")->base();

	int base_index = tile_index;

//  if (konamigx_type3_psac2_actual_bank)
//      base_index+=0x20000/2;


	tileno =  tmap[base_index*2] | ((tmap[(base_index*2)+1] & 0x0f)<<8);
	colour = (tmap[(base_index*2)+1]&0xc0)>>6;

	flip = 0;
	if (tmap[(base_index*2)+1] & 0x20) flip |= TILE_FLIPY;
	if (tmap[(base_index*2)+1] & 0x10) flip |= TILE_FLIPX;

	SET_TILE_INFO_MEMBER(0, tileno, colour, flip);
	}

TILE_GET_INFO_MEMBER(konamigx_state::get_gx_psac3_alt_tile_info)
	{
	int tileno, colour, flip;
	UINT8 *tmap = memregion("gfx4")->base()+0x20000;

	int base_index = tile_index;

//  if (konamigx_type3_psac2_actual_bank)
//      base_index+=0x20000/2;


	tileno =  tmap[base_index*2] | ((tmap[(base_index*2)+1] & 0x0f)<<8);
	colour = (tmap[(base_index*2)+1]&0xc0)>>6;

	flip = 0;
	if (tmap[(base_index*2)+1] & 0x20) flip |= TILE_FLIPY;
	if (tmap[(base_index*2)+1] & 0x10) flip |= TILE_FLIPX;

	SET_TILE_INFO_MEMBER(0, tileno, colour, flip);
	}


/* PSAC4 */
/* these tilemaps are weird in both format and content, one of them
   doesn't really look like it should be displayed? - it's height data */
TILE_GET_INFO_MEMBER(konamigx_state::get_gx_psac1a_tile_info)
{
	int tileno, colour, flipx,flipy;
	int flip;
	flip=0;
	colour = 0;

	tileno = (m_psacram[tile_index*2] & 0x00003fff)>>0;

	// scanrows
	//flipx  = (m_psacram[tile_index*2+1] & 0x00800000)>>23;
	//flipy  = (m_psacram[tile_index*2+1] & 0x00400000)>>22;
	// scancols
	flipy  = (m_psacram[tile_index*2+1] & 0x00800000)>>23;
	flipx  = (m_psacram[tile_index*2+1] & 0x00400000)>>22;

	if (flipx) flip |= TILE_FLIPX;
	if (flipy) flip |= TILE_FLIPY;

	SET_TILE_INFO_MEMBER(1, tileno, colour, flip);
}

TILE_GET_INFO_MEMBER(konamigx_state::get_gx_psac1b_tile_info)
{
	int tileno, colour, flipx,flipy;
	int flip;
	flip=0;

	colour = 0;
	tileno = (m_psacram[tile_index*2+1] & 0x00003fff)>>0;

	// scanrows
	//flipx  = (m_psacram[tile_index*2+1] & 0x00800000)>>23;
	//flipy  = (m_psacram[tile_index*2+1] & 0x00400000)>>22;
	// scancols
	flipy  = (m_psacram[tile_index*2+1] & 0x00200000)>>21;
	flipx  = (m_psacram[tile_index*2+1] & 0x00100000)>>20;

	if (flipx) flip |= TILE_FLIPX;
	if (flipy) flip |= TILE_FLIPY;

	SET_TILE_INFO_MEMBER(0, tileno, colour, flip);
}

static void konamigx_type2_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags)
{
	int d = *code;

	*code = (gx_tilebanks[(d & 0xe000)>>13]<<13) + (d & 0x1fff);
	K055555GX_decode_vmixcolor(layer, color);
}

static void konamigx_alpha_tile_callback(running_machine &machine, int layer, int *code, int *color, int *flags)
{
	int mixcode;
	int d = *code;

	mixcode = K055555GX_decode_vmixcolor(layer, color);

	if (mixcode < 0)
		*code = (gx_tilebanks[(d & 0xe000)>>13]<<13) + (d & 0x1fff);
	else
	{
		/* save mixcode and mark tile alpha (unimplemented) */
		*code = 0;

		if (VERBOSE)
			popmessage("skipped alpha tile(layer=%d mix=%d)", layer, mixcode);
	}
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

void konamigx_state::_gxcommoninitnosprites(running_machine &machine)
{
	int i;

	K054338_vh_start(machine, m_k055555);
	m_k055555->K055555_vh_start(machine);

	konamigx_mixer_init(*m_screen, 0);

	for (i = 0; i < 8; i++)
	{
		gx_tilebanks[i] = gx_oldbanks[i] = 0;
	}

	machine.save().save_item(NAME(gx_tilebanks));

	gx_tilemode = 0;

	gx_rozenable = 0;
	gx_specialrozenable = 0;
	gx_rushingheroes_hack = 0;
	gx_le2_textcolour_hack = 0;

	// Documented relative offsets of non-flipped games are (-2, 0, 2, 3),(0, 0, 0, 0).
	// (+ve values move layers to the right and -ve values move layers to the left)
	// In most cases only a constant is needed to add to the X offsets to yield correct
	// displacement. This should be done by the CCU but the CRT timings have not been
	// figured out.
	m_k056832->set_layer_offs(0, -2, 0);
	m_k056832->set_layer_offs(1,  0, 0);
	m_k056832->set_layer_offs(2,  2, 0);
	m_k056832->set_layer_offs(3,  3, 0);

	konamigx_has_dual_screen = 0;
	konamigx_current_frame = 0;
}

void konamigx_state::_gxcommoninit(running_machine &machine)
{
	// (+ve values move objects to the right and -ve values move objects to the left)
	m_k055673->alt_k055673_vh_start(machine, "gfx2", K055673_LAYOUT_GX, -26, -23, konamigx_type2_sprite_callback);

	_gxcommoninitnosprites(machine);
}


VIDEO_START_MEMBER(konamigx_state,konamigx_5bpp)
{
	if (!strcmp(machine().system().name,"sexyparo") || !strcmp(machine().system().name,"sexyparoa"))
		game_tile_callback = konamigx_alpha_tile_callback;
	else
		game_tile_callback = konamigx_type2_tile_callback;

	m_k056832->altK056832_vh_start(machine(), "gfx1", K056832_BPP_5, 0, NULL, game_tile_callback, 0);

	_gxcommoninit(machine());

	/* here are some hand tuned per game scroll offsets to go with the per game visible areas,
	   i see no better way of doing this for now... */

	if (!strcmp(machine().system().name,"tbyahhoo"))
	{
		m_k056832->K056832_set_k055555(m_k055555);
		gx_tilemode = 1;
	} else

	if (!strcmp(machine().system().name,"puzldama"))
	{
		m_k055673->k053247_set_sprite_offs(-46, -23);
		konamigx_mixer_primode(5);
	} else

	if (!strcmp(machine().system().name,"daiskiss"))
	{
		konamigx_mixer_primode(4);
	} else

	if (!strcmp(machine().system().name,"gokuparo") || !strcmp(machine().system().name,"fantjour") || !strcmp(machine().system().name,"fantjoura"))
	{
		m_k055673->k053247_set_sprite_offs(-46, -23);
	} else

	if (!strcmp(machine().system().name,"sexyparo") || !strcmp(machine().system().name,"sexyparoa"))
	{
		m_k055673->k053247_set_sprite_offs(-42, -23);
	}
}

VIDEO_START_MEMBER(konamigx_state,winspike)
{
	m_k056832->altK056832_vh_start(machine(), "gfx1", K056832_BPP_8, 0, NULL, konamigx_alpha_tile_callback, 2);
	m_k055673->alt_k055673_vh_start(machine(), "gfx2", K055673_LAYOUT_LE2, -53, -23, konamigx_type2_sprite_callback);

	_gxcommoninitnosprites(machine());
}

VIDEO_START_MEMBER(konamigx_state,dragoonj)
{
	m_k056832->altK056832_vh_start(machine(), "gfx1", K056832_BPP_5, 1, NULL, konamigx_type2_tile_callback, 0);
	m_k055673->alt_k055673_vh_start(machine(), "gfx2", K055673_LAYOUT_RNG, -53, -23, konamigx_dragoonj_sprite_callback);

	_gxcommoninitnosprites(machine());

	m_k056832->set_layer_offs(0, -2+1, 0);
	m_k056832->set_layer_offs(1,  0+1, 0);
	m_k056832->set_layer_offs(2,  2+1, 0);
	m_k056832->set_layer_offs(3,  3+1, 0);
}

VIDEO_START_MEMBER(konamigx_state,le2)
{
	m_k056832->altK056832_vh_start(machine(), "gfx1", K056832_BPP_8, 1, NULL, konamigx_type2_tile_callback, 0);
	m_k055673->alt_k055673_vh_start(machine(), "gfx2", K055673_LAYOUT_LE2, -46, -23, konamigx_le2_sprite_callback);

	_gxcommoninitnosprites(machine());

	konamigx_mixer_primode(-1); // swapped layer B and C priorities?

	gx_le2_textcolour_hack = 1; // force text layer to use the right palette
	m_k055555->K055555_write_reg(K55_INPUT_ENABLES, 1); // it doesn't turn on the video output at first for the test screens, maybe it should default to ON?
}

VIDEO_START_MEMBER(konamigx_state,konamigx_6bpp)
{
	m_k056832->altK056832_vh_start(machine(), "gfx1", K056832_BPP_6, 0, NULL, konamigx_type2_tile_callback, 0);

	_gxcommoninit(machine());

	if (!strcmp(machine().system().name,"tokkae") || !strcmp(machine().system().name,"tkmmpzdm"))
	{
		m_k055673->k053247_set_sprite_offs(-46, -23);
		konamigx_mixer_primode(5);
	}
}

VIDEO_START_MEMBER(konamigx_state,konamigx_type3)
{
	int width = m_screen->width();
	int height = m_screen->height();

	m_k056832->altK056832_vh_start(machine(), "gfx1", K056832_BPP_6, 0, NULL, konamigx_type2_tile_callback, 1);
	m_k055673->alt_k055673_vh_start(machine(), "gfx2", K055673_LAYOUT_GX6, -132, -23, konamigx_type2_sprite_callback);

	dualscreen_left_tempbitmap = auto_bitmap_rgb32_alloc(machine(), width, height);
	dualscreen_right_tempbitmap = auto_bitmap_rgb32_alloc(machine(), width, height);

	_gxcommoninitnosprites(machine());

	gx_psac_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(konamigx_state::get_gx_psac3_tile_info),this), TILEMAP_SCAN_COLS,  16, 16, 256, 256);
	gx_psac_tilemap_alt = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(konamigx_state::get_gx_psac3_alt_tile_info),this), TILEMAP_SCAN_COLS,  16, 16, 256, 256);

	gx_rozenable = 0;
	gx_specialrozenable = 2;


	/* set up tile layers */
	type3_roz_temp_bitmap = auto_bitmap_ind16_alloc(machine(), width, height);


	//gx_psac_tilemap->set_flip(TILEMAP_FLIPX| TILEMAP_FLIPY);

	K053936_wraparound_enable(0, 1);
//  K053936GP_set_offset(0, -30, -1);
	K053936_set_offset(0, -30, +1);

	m_k056832->set_layer_offs(0,  -52, 0);
	m_k056832->set_layer_offs(1,  -48, 0);
	m_k056832->set_layer_offs(2,  -48, 0);
	m_k056832->set_layer_offs(3,  -48, 0);

	konamigx_has_dual_screen = 1;
	konamigx_palformat = 1;
}

VIDEO_START_MEMBER(konamigx_state,konamigx_type4)
{
	int width = m_screen->width();
	int height = m_screen->height();

	m_k056832->altK056832_vh_start(machine(), "gfx1", K056832_BPP_8, 0, NULL, konamigx_type2_tile_callback, 0);
	m_k055673->alt_k055673_vh_start(machine(), "gfx2", K055673_LAYOUT_GX6, -79, -24, konamigx_type2_sprite_callback); // -23 looks better in intro

	dualscreen_left_tempbitmap = auto_bitmap_rgb32_alloc(machine(), width, height);
	dualscreen_right_tempbitmap = auto_bitmap_rgb32_alloc(machine(), width, height);

	_gxcommoninitnosprites(machine());

	gx_psac_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(konamigx_state::get_gx_psac_tile_info),this), TILEMAP_SCAN_COLS,  16, 16, 128, 128);
	gx_rozenable = 0;
	gx_specialrozenable = 3;

	m_k056832->set_layer_offs(0,  -27, 0);
	m_k056832->set_layer_offs(1,  -25, 0);
	m_k056832->set_layer_offs(2,  -24, 0);
	m_k056832->set_layer_offs(3,  -22, 0);

	K053936_wraparound_enable(0, 0);
	K053936GP_set_offset(0, -36, 1);

	gx_rushingheroes_hack = 1;
	konamigx_has_dual_screen = 1;
	konamigx_palformat = 0;

}

VIDEO_START_MEMBER(konamigx_state,konamigx_type4_vsn)
{
	int width = m_screen->width();
	int height = m_screen->height();

	m_k056832->altK056832_vh_start(machine(), "gfx1", K056832_BPP_8, 0, NULL, konamigx_type2_tile_callback, 2);   // set djmain_hack to 2 to kill layer association or half the tilemaps vanish on screen 0
	m_k055673->alt_k055673_vh_start(machine(), "gfx2", K055673_LAYOUT_GX6, -132, -23, konamigx_type2_sprite_callback);

	dualscreen_left_tempbitmap = auto_bitmap_rgb32_alloc(machine(), width, height);
	dualscreen_right_tempbitmap = auto_bitmap_rgb32_alloc(machine(), width, height);

	_gxcommoninitnosprites(machine());

	gx_psac_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(konamigx_state::get_gx_psac_tile_info),this), TILEMAP_SCAN_COLS,  16, 16, 128, 128);
	gx_rozenable = 0;
	gx_specialrozenable = 3;

	m_k056832->set_layer_offs(0,  -52, 0);
	m_k056832->set_layer_offs(1,  -48, 0);
	m_k056832->set_layer_offs(2,  -48, 0);
	m_k056832->set_layer_offs(3,  -48, 0);

	K053936_wraparound_enable(0, 1); // wraparound doesn't work properly with the custom drawing function anyway, see the crowd in vsnet and rushhero
	K053936GP_set_offset(0, -30, 0);

	gx_rushingheroes_hack = 1;
	konamigx_has_dual_screen = 1;
	konamigx_palformat = 0;
}

VIDEO_START_MEMBER(konamigx_state,konamigx_type4_sd2)
{
	int width = m_screen->width();
	int height = m_screen->height();

	m_k056832->altK056832_vh_start(machine(), "gfx1", K056832_BPP_8, 0, NULL, konamigx_type2_tile_callback, 0);
	m_k055673->alt_k055673_vh_start(machine(), "gfx2", K055673_LAYOUT_GX6, -81, -23, konamigx_type2_sprite_callback);

	dualscreen_left_tempbitmap = auto_bitmap_rgb32_alloc(machine(), width, height);
	dualscreen_right_tempbitmap = auto_bitmap_rgb32_alloc(machine(), width, height);

	_gxcommoninitnosprites(machine());

	gx_psac_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(konamigx_state::get_gx_psac_tile_info),this), TILEMAP_SCAN_COLS,  16, 16, 128, 128);
	gx_rozenable = 0;
	gx_specialrozenable = 3;


	m_k056832->set_layer_offs(0,  -29, -1);
	m_k056832->set_layer_offs(1,  -27, -1);
	m_k056832->set_layer_offs(2,  -26, -1);
	m_k056832->set_layer_offs(3,  -24, -1);


	K053936_wraparound_enable(0, 0);
	K053936GP_set_offset(0, -36, -1);

	gx_rushingheroes_hack = 1;
	konamigx_has_dual_screen = 1;
	konamigx_palformat = 0;

}


VIDEO_START_MEMBER(konamigx_state,konamigx_6bpp_2)
{
	m_k056832->altK056832_vh_start(machine(), "gfx1", K056832_BPP_6, 1, NULL, konamigx_type2_tile_callback, 0);

	if (!strcmp(machine().system().name,"salmndr2") || !strcmp(machine().system().name,"salmndr2a"))
	{
		m_k055673->alt_k055673_vh_start(machine(), "gfx2", K055673_LAYOUT_GX6, -48, -23, konamigx_salmndr2_sprite_callback);

		_gxcommoninitnosprites(machine());
	}
	else
	{
		_gxcommoninit(machine());
	}
}

VIDEO_START_MEMBER(konamigx_state,opengolf)
{
	m_k056832->altK056832_vh_start(machine(), "gfx1", K056832_BPP_5, 0, NULL, konamigx_type2_tile_callback, 0);
	m_k055673->alt_k055673_vh_start(machine(), "gfx2", K055673_LAYOUT_GX6, -53, -23, konamigx_type2_sprite_callback);

	_gxcommoninitnosprites(machine());

	m_k056832->set_layer_offs(0, -2+1, 0);
	m_k056832->set_layer_offs(1,  0+1, 0);
	m_k056832->set_layer_offs(2,  2+1, 0);
	m_k056832->set_layer_offs(3,  3+1, 0);

	gx_psac_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(konamigx_state::get_gx_psac1a_tile_info),this), TILEMAP_SCAN_COLS,  16, 16, 128, 128);
	gx_psac_tilemap2 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(konamigx_state::get_gx_psac1b_tile_info),this), TILEMAP_SCAN_COLS,  16, 16, 128, 128);

	// transparency will be handled manually in post-processing
	//gx_psac_tilemap->set_transparent_pen(0);
	//gx_psac_tilemap2->set_transparent_pen(0);

	gx_rozenable = 0;
	gx_specialrozenable = 1;

	gxtype1_roz_dstbitmap =  auto_bitmap_ind16_alloc(machine(),512,512); // BITMAP_FORMAT_IND16 because we NEED the raw pen data for post-processing
	gxtype1_roz_dstbitmap2 = auto_bitmap_ind16_alloc(machine(),512,512); // BITMAP_FORMAT_IND16 because we NEED the raw pen data for post-processing


	gxtype1_roz_dstbitmapclip.set(0, 512-1, 0, 512-1);


	K053936_wraparound_enable(0, 1);
	K053936GP_set_offset(0, 0, 0);

	// urgh.. the priority bitmap is global, and because our temp bitmaps are bigger than the screen, this causes issues.. so just allocate something huge
	// until there is a better solution, or priority bitmap can be specified manually.
	m_screen->priority().allocate(2048,2048);

}

VIDEO_START_MEMBER(konamigx_state,racinfrc)
{
	m_k056832->altK056832_vh_start(machine(), "gfx1", K056832_BPP_6, 0, NULL, konamigx_type2_tile_callback, 0);
	m_k055673->alt_k055673_vh_start(machine(), "gfx2", K055673_LAYOUT_GX, -53, -23, konamigx_type2_sprite_callback);

	_gxcommoninitnosprites(machine());

	m_k056832->set_layer_offs(0, -2+1, 0);
	m_k056832->set_layer_offs(1,  0+1, 0);
	m_k056832->set_layer_offs(2,  2+1, 0);
	m_k056832->set_layer_offs(3,  3+1, 0);

	gx_psac_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(konamigx_state::get_gx_psac1a_tile_info),this), TILEMAP_SCAN_COLS,  16, 16, 128, 128);
	gx_psac_tilemap2 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(konamigx_state::get_gx_psac1b_tile_info),this), TILEMAP_SCAN_COLS,  16, 16, 128, 128);

	// transparency will be handled manually in post-processing
	//gx_psac_tilemap->set_transparent_pen(0);
	//gx_psac_tilemap2->set_transparent_pen(0);

	gx_rozenable = 0;
	gx_specialrozenable = 1;

	gxtype1_roz_dstbitmap =  auto_bitmap_ind16_alloc(machine(),512,512); // BITMAP_FORMAT_IND16 because we NEED the raw pen data for post-processing
	gxtype1_roz_dstbitmap2 = auto_bitmap_ind16_alloc(machine(),512,512); // BITMAP_FORMAT_IND16 because we NEED the raw pen data for post-processing


	gxtype1_roz_dstbitmapclip.set(0, 512-1, 0, 512-1);


	K053936_wraparound_enable(0, 1);
	K053936GP_set_offset(0, 0, 0);

	// urgh.. the priority bitmap is global, and because our temp bitmaps are bigger than the screen, this causes issues.. so just allocate something huge
	// until there is a better solution, or priority bitmap can be specified manually.
	m_screen->priority().allocate(2048,2048);


}

UINT32 konamigx_state::screen_update_konamigx(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int i, newbank, newbase, dirty, unchained;

	/* if any banks are different from last render, we need to flush the planes */
	for (dirty = 0, i = 0; i < 8; i++)
	{
		newbank = gx_tilebanks[i];
		if (gx_oldbanks[i] != newbank) { gx_oldbanks[i] = newbank; dirty = 1; }
	}

	if (gx_tilemode == 0)
	{
		// driver approximates tile update in mode 0 for speed
		unchained = m_k056832->get_layer_association();
		for (i=0; i<4; i++)
		{
			newbase = m_k055555->K055555_get_palette_index(i)<<6;
			if (layer_colorbase[i] != newbase)
			{
				layer_colorbase[i] = newbase;

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
	if (gx_rozenable)
	{
		last_psac_colorbase = psac_colorbase;
		psac_colorbase = m_k055555->K055555_get_palette_index(6);

		if (psac_colorbase != last_psac_colorbase)
		{
			gx_psac_tilemap->mark_all_dirty();
			if (gx_rozenable == 3)
			{
				gx_psac_tilemap2->mark_all_dirty();
			}
		}
	}

	if (dirty) m_k056832->mark_all_tilemaps_dirty();

	// Type-1
	if (gx_specialrozenable == 1)
	{
		K053936_0_zoom_draw(screen, *gxtype1_roz_dstbitmap, gxtype1_roz_dstbitmapclip,gx_psac_tilemap, 0,0,0); // height data
		K053936_0_zoom_draw(screen, *gxtype1_roz_dstbitmap2,gxtype1_roz_dstbitmapclip,gx_psac_tilemap2,0,0,0); // colour data (+ some voxel height data?)
	}



	if (gx_specialrozenable==3)
	{
		konamigx_mixer(screen, bitmap, cliprect, gx_psac_tilemap, GXSUB_8BPP,0,0,  0, 0, gx_rushingheroes_hack);
	}
	// hack, draw the roz tilemap if W is held
	// todo: fix so that it works with the mixer without crashing(!)
	else if (gx_specialrozenable == 2)
	{
		// we're going to throw half of this away anyway in post-process, so only render what's needed
		rectangle temprect;
		temprect = cliprect;
		temprect.max_x = cliprect.min_x+320;

		if (konamigx_type3_psac2_actual_bank == 1) K053936_0_zoom_draw(screen, *type3_roz_temp_bitmap, temprect,gx_psac_tilemap_alt, 0,0,0); // soccerss playfield
		else K053936_0_zoom_draw(screen, *type3_roz_temp_bitmap, temprect,gx_psac_tilemap, 0,0,0); // soccerss playfield


		konamigx_mixer(screen, bitmap, cliprect, 0, 0, 0, 0, 0, type3_roz_temp_bitmap, gx_rushingheroes_hack);
	}
	else
	{
		konamigx_mixer(screen, bitmap, cliprect, 0, 0, 0, 0, 0, 0, gx_rushingheroes_hack);
	}



	/* Hack! draw type-1 roz layer here for testing purposes only */
	if (gx_specialrozenable == 1)
	{
		const pen_t *paldata = m_palette->pens();

		if ( machine().input().code_pressed(KEYCODE_W) )
		{
			int y,x;

			// make it flicker, to compare positioning
			//if (screen.frame_number() & 1)
			{
				for (y=0;y<256;y++)
				{
					//UINT16* src = &gxtype1_roz_dstbitmap->pix16(y);

					//UINT32* dst = &bitmap.pix32(y);
					// ths K053936 rendering should probably just be flipped
					// this is just kludged to align the racing force 2d logo
					UINT16* src = &gxtype1_roz_dstbitmap2->pix16(y+30);
					UINT32* dst = &bitmap.pix32(256-y);

					for (x=0;x<512;x++)
					{
						UINT16 dat = src[x];
						dst[x] = paldata[dat];
					}
				}
			}

		}

	}

	return 0;
}

UINT32 konamigx_state::screen_update_konamigx_left(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* the video gets demuxed by a board which plugs into the jamma connector */
	konamigx_current_frame^=1;

	if (konamigx_current_frame==1)
	{
		int offset=0;

		if (konamigx_palformat==1)
		{
			for (offset=0;offset<0x4000/4;offset++)
			{
				UINT32 coldat = m_generic_paletteram_32[offset];

				set_color_555(m_palette, offset*2, 0, 5, 10,coldat >> 16);
				set_color_555(m_palette, offset*2+1, 0, 5, 10,coldat & 0xffff);
			}
		}
		else
		{
			for (offset=0;offset<0x8000/4;offset++)
			{
				int r,g,b;

				r = (m_generic_paletteram_32[offset] >>16) & 0xff;
				g = (m_generic_paletteram_32[offset] >> 8) & 0xff;
				b = (m_generic_paletteram_32[offset] >> 0) & 0xff;

				m_palette->set_pen_color(offset,rgb_t(r,g,b));
			}
		}

		screen_update_konamigx( screen, downcast<bitmap_rgb32 &>(*dualscreen_left_tempbitmap), cliprect);
		copybitmap(bitmap, *dualscreen_left_tempbitmap, 0, 0, 0, 0, cliprect);
	}
	else
	{
		copybitmap(bitmap, *dualscreen_left_tempbitmap, 0, 0, 0, 0, cliprect);
	}

	return 0;
}

UINT32 konamigx_state::screen_update_konamigx_right(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (konamigx_current_frame==1)
	{
		copybitmap(bitmap, *dualscreen_right_tempbitmap, 0, 0, 0, 0, cliprect);
	}
	else
	{
		int offset=0;

		if (konamigx_palformat==1)
		{
			for (offset=0;offset<0x4000/4;offset++)
			{
				UINT32 coldat = m_subpaletteram32[offset];

				set_color_555(m_palette, offset*2, 0, 5, 10,coldat >> 16);
				set_color_555(m_palette, offset*2+1, 0, 5, 10,coldat & 0xffff);
			}
		}
		else
		{
			for (offset=0;offset<0x8000/4;offset++)
			{
				int r,g,b;

				r = (m_subpaletteram32[offset] >>16) & 0xff;
				g = (m_subpaletteram32[offset] >> 8) & 0xff;
				b = (m_subpaletteram32[offset] >> 0) & 0xff;

				m_palette->set_pen_color(offset,rgb_t(r,g,b));
			}
		}

		screen_update_konamigx(screen, downcast<bitmap_rgb32 &>(*dualscreen_right_tempbitmap), cliprect);
		copybitmap(bitmap, *dualscreen_right_tempbitmap, 0, 0, 0, 0, cliprect);
	}

	return 0;
}


WRITE32_MEMBER(konamigx_state::konamigx_palette_w)
{
	int r,g,b;

	COMBINE_DATA(&m_generic_paletteram_32[offset]);

	r = (m_generic_paletteram_32[offset] >>16) & 0xff;
	g = (m_generic_paletteram_32[offset] >> 8) & 0xff;
	b = (m_generic_paletteram_32[offset] >> 0) & 0xff;

	m_palette->set_pen_color(offset,rgb_t(r,g,b));
}

#ifdef UNUSED_FUNCTION
WRITE32_MEMBER(konamigx_state::konamigx_palette2_w)
{
	int r,g,b;

	COMBINE_DATA(&state->m_subpaletteram32[offset]);

	r = (state->m_subpaletteram32[offset] >>16) & 0xff;
	g = (state->m_subpaletteram32[offset] >> 8) & 0xff;
	b = (state->m_subpaletteram32[offset] >> 0) & 0xff;

	offset += (0x8000/4);

	m_palette->set_pen_color(offset,rgb_t(r,g,b));
}
#endif

INLINE void set_color_555(palette_device &palette, pen_t color, int rshift, int gshift, int bshift, UINT16 data)
{
	palette.set_pen_color(color, pal5bit(data >> rshift), pal5bit(data >> gshift), pal5bit(data >> bshift));
}

#ifdef UNUSED_FUNCTION
// main monitor for type 3
WRITE32_MEMBER(konamigx_state::konamigx_555_palette_w)
{
	UINT32 coldat;
	COMBINE_DATA(&m_generic_paletteram_32[offset]);

	coldat = m_generic_paletteram_32[offset];

	set_color_555(m_palette, offset*2, 0, 5, 10,coldat >> 16);
	set_color_555(m_palette, offset*2+1, 0, 5, 10,coldat & 0xffff);
}

// sub monitor for type 3
WRITE32_MEMBER(konamigx_state::konamigx_555_palette2_w)
{
	UINT32 coldat;
	COMBINE_DATA(&state->m_subpaletteram32[offset]);
	coldat = state->m_subpaletteram32[offset];

	offset += (0x4000/4);

	set_color_555(m_palette, offset*2, 0, 5, 10,coldat >> 16);
	set_color_555(m_palette, offset*2+1, 0, 5, 10,coldat & 0xffff);
}
#endif


WRITE32_MEMBER(konamigx_state::konamigx_tilebank_w)
{
	if (ACCESSING_BITS_24_31)
		gx_tilebanks[offset*4] = (data>>24)&0xff;
	if (ACCESSING_BITS_16_23)
		gx_tilebanks[offset*4+1] = (data>>16)&0xff;
	if (ACCESSING_BITS_8_15)
		gx_tilebanks[offset*4+2] = (data>>8)&0xff;
	if (ACCESSING_BITS_0_7)
		gx_tilebanks[offset*4+3] = data&0xff;
}

// type 1 RAM-based PSAC tilemap
WRITE32_MEMBER(konamigx_state::konamigx_t1_psacmap_w)
{
	COMBINE_DATA(&m_psacram[offset]);
	gx_psac_tilemap->mark_tile_dirty(offset/2);
	gx_psac_tilemap2->mark_tile_dirty(offset/2);
}

// type 4 RAM-based PSAC tilemap
WRITE32_MEMBER(konamigx_state::konamigx_t4_psacmap_w)
{
	COMBINE_DATA(&m_psacram[offset]);

	gx_psac_tilemap->mark_tile_dirty(offset*2);
	gx_psac_tilemap->mark_tile_dirty((offset*2)+1);
}
