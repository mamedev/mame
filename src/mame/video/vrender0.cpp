// license:BSD-3-Clause
// copyright-holders:ElSemi
#include "emu.h"
#include "vrender0.h"


/***********************************
        VRENDER ZERO
        VIDEO EMULATION By ElSemi


    The VRender0 is a very special 2D sprite renderer. The spec says it's based on 3D
    technology.
    The device processes "display lists" that contain pointers to the texture, tex coords
    and the step increments on x and y (dxx,dxy,dyx,dyy) allowing ROZ effects on sprites.
    It supports alphablend with programmable factors per channel and for source and dest
    color.

************************************/


/*****************************************************************************
 DEVICE INTERFACE
 *****************************************************************************/

DEFINE_DEVICE_TYPE(VIDEO_VRENDER0, vr0video_device, "vr0video", "MagicEyes VRender0 Video Engine")

vr0video_device::vr0video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VIDEO_VRENDER0, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_idleskip_cb(*this)

{
}

void vr0video_device::regs_map(address_map &map)
{
	map(0x0080, 0x0081).rw(FUNC(vr0video_device::cmd_queue_front_r), FUNC(vr0video_device::cmd_queue_front_w));
	map(0x0082, 0x0083).r(FUNC(vr0video_device::cmd_queue_rear_r));

	map(0x008c, 0x008d).rw(FUNC(vr0video_device::render_control_r), FUNC(vr0video_device::render_control_w));
	map(0x008e, 0x008f).r(FUNC(vr0video_device::display_bank_r));

	map(0x0090, 0x0091).rw(FUNC(vr0video_device::bank1_select_r), FUNC(vr0video_device::bank1_select_w));
	map(0x00a6, 0x00a7).rw(FUNC(vr0video_device::flip_count_r), FUNC(vr0video_device::flip_count_w));
}

READ16_MEMBER(vr0video_device::cmd_queue_front_r)
{
	return m_queue_front & 0x7ff;
}

WRITE16_MEMBER(vr0video_device::cmd_queue_front_w)
{
	COMBINE_DATA(&m_queue_front);
}

READ16_MEMBER(vr0video_device::cmd_queue_rear_r)
{
	return m_queue_rear & 0x7ff;
}

READ16_MEMBER(vr0video_device::render_control_r)
{
	return (m_draw_select<<7) | (m_render_reset<<3) | (m_render_start<<2) | (m_dither_mode);
}

WRITE16_MEMBER(vr0video_device::render_control_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_draw_select = BIT(data, 7);
		m_render_reset = BIT(data, 3);
		m_render_start = BIT(data, 2);
		m_dither_mode = data & 3;

		// initialize pipeline
		// TODO: what happens if reset and start are both 1? Datasheet advises against it.
		if (m_render_reset == true)
			m_queue_front = m_queue_rear = 0;
	}
}

READ16_MEMBER(vr0video_device::display_bank_r)
{
	return m_display_bank;
}

READ16_MEMBER(vr0video_device::bank1_select_r)
{
	return (m_bank1_select)<<15;
}

WRITE16_MEMBER(vr0video_device::bank1_select_w)
{
	m_bank1_select = BIT(data,15);
}

READ16_MEMBER(vr0video_device::flip_count_r)
{
	m_idleskip_cb(m_flip_count != 0);
	return m_flip_count;
}

WRITE16_MEMBER(vr0video_device::flip_count_w)
{
	if (ACCESSING_BITS_0_7)
	{
		int fc = (data) & 0xff;
		if (fc == 1)
			m_flip_count++;
		else if (fc == 0)
			m_flip_count = 0;
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vr0video_device::device_start()
{
	m_idleskip_cb.resolve_safe();

	save_item(NAME(m_InternalPalette));
	save_item(NAME(m_LastPalUpdate));

	save_item(NAME(m_RenderState.Tx));
	save_item(NAME(m_RenderState.Ty));
	save_item(NAME(m_RenderState.Txdx));
	save_item(NAME(m_RenderState.Tydx));
	save_item(NAME(m_RenderState.Txdy));
	save_item(NAME(m_RenderState.Tydy));
	save_item(NAME(m_RenderState.SrcAlphaColor));
	save_item(NAME(m_RenderState.SrcBlend));
	save_item(NAME(m_RenderState.DstAlphaColor));
	save_item(NAME(m_RenderState.DstBlend));
	save_item(NAME(m_RenderState.ShadeColor));
	save_item(NAME(m_RenderState.TransColor));
	save_item(NAME(m_RenderState.TileOffset));
	save_item(NAME(m_RenderState.FontOffset));
	save_item(NAME(m_RenderState.PalOffset));
	save_item(NAME(m_RenderState.PaletteBank));
	save_item(NAME(m_RenderState.TextureMode));
	save_item(NAME(m_RenderState.PixelFormat));
	save_item(NAME(m_RenderState.Width));
	save_item(NAME(m_RenderState.Height));

	save_item(NAME(m_flip_count));
	save_item(NAME(m_queue_rear));
	save_item(NAME(m_queue_front));
	save_item(NAME(m_bank1_select));
	save_item(NAME(m_display_bank));
	save_item(NAME(m_draw_select));
	save_item(NAME(m_render_reset));
	save_item(NAME(m_render_start));
	save_item(NAME(m_dither_mode));
}

void vr0video_device::set_areas(uint8_t *textureram, uint16_t *frameram)
{
	m_textureram = textureram;
	m_frameram = frameram;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vr0video_device::device_reset()
{
	memset(m_InternalPalette, 0, sizeof(m_InternalPalette));
	m_LastPalUpdate = 0xffffffff;
}

/*****************************************************************************
 IMPLEMENTATION
 *****************************************************************************/

struct QuadInfo
{
	uint16_t *Dest;
	uint32_t Pitch;   //in UINT16s
	uint32_t w,h;
	uint32_t Tx;
	uint32_t Ty;
	uint32_t Txdx;
	uint32_t Tydx;
	uint32_t Txdy;
	uint32_t Tydy;
	uint16_t TWidth;
	uint16_t THeight;
	union _u
	{
		uint8_t *Imageb;
		uint16_t *Imagew;
	} u;
	uint16_t *Tile;
	uint16_t *Pal;
	uint32_t TransColor;
	uint32_t Shade;
	uint8_t Clamp;
	uint8_t Trans;
	uint8_t SrcAlpha;
	uint32_t SrcColor;
	uint8_t DstAlpha;
	uint32_t DstColor;
};

/*
Pick a rare enough color to disable transparency (that way I save a cmp per loop to check
if I must draw transparent or not. The palette build will take this color in account so
no color in the palette will have this value
*/
#define NOTRANSCOLOR    0xecda

#define RGB32(r,g,b) ((r << 16) | (g << 8) | (b << 0))
#define RGB16(r,g,b) ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | ((b & 0xf8) >> 3)

static inline uint16_t RGB32TO16(uint32_t rgb)
{
	return (((rgb >> (16 + 3)) & 0x1f) << 11) | (((rgb >> (8 + 2)) & 0x3f) << 5) | (((rgb >> (3)) & 0x1f) << 0);
}

#define EXTRACTR8(Src)  (((Src >> 11) << 3) & 0xff)
#define EXTRACTG8(Src)  (((Src >>  5) << 2) & 0xff)
#define EXTRACTB8(Src)  (((Src >>  0) << 3) & 0xff)

static inline uint16_t Shade(uint16_t Src, uint32_t Shade)
{
	uint32_t scr = (EXTRACTR8(Src) * ((Shade >> 16) & 0xff)) >> 8;
	uint32_t scg = (EXTRACTG8(Src) * ((Shade >>  8) & 0xff)) >> 8;
	uint32_t scb = (EXTRACTB8(Src) * ((Shade >>  0) & 0xff)) >> 8;
	return RGB16(scr, scg, scb);
}

static uint16_t Alpha(QuadInfo *Quad, uint16_t Src, uint16_t Dst)
{
	uint32_t scr = (EXTRACTR8(Src) * ((Quad->Shade >> 16) & 0xff)) >> 8;
	uint32_t scg = (EXTRACTG8(Src) * ((Quad->Shade >>  8) & 0xff)) >> 8;
	uint32_t scb = (EXTRACTB8(Src) * ((Quad->Shade >>  0) & 0xff)) >> 8;
	uint32_t dcr = EXTRACTR8(Dst);
	uint32_t dcg = EXTRACTG8(Dst);
	uint32_t dcb = EXTRACTB8(Dst);

	uint32_t smulr, smulg, smulb;
	uint32_t dmulr, dmulg, dmulb;

	switch (Quad->SrcAlpha & 0x1f)
	{
		case 0x01:
			smulr = smulg = smulb = 0;
			break;
		case 0x02:
			smulr = (Quad->SrcColor >> 16) & 0xff;
			smulg = (Quad->SrcColor >>  8) & 0xff;
			smulb = (Quad->SrcColor >>  0) & 0xff;
			break;
		case 0x04:
			smulr = scr;
			smulg = scg;
			smulb = scb;
			break;
		case 0x08:
			smulr = (Quad->DstColor >> 16) & 0xff;
			smulg = (Quad->DstColor >>  8) & 0xff;
			smulb = (Quad->DstColor >>  0) & 0xff;
			break;
		case 0x10:
			smulr = dcr;
			smulg = dcg;
			smulb = dcb;
			break;
		default:
			smulr = smulg = smulb = 0;
			break;
	}

	if (Quad->SrcAlpha & 0x20)
	{
		smulr = 0x100 - smulr;
		smulg = 0x100 - smulg;
		smulb = 0x100 - smulb;
	}

	switch (Quad->DstAlpha & 0x1f)
	{
		case 0x01:
			dmulr = dmulg = dmulb = 0;
			break;
		case 0x02:
			dmulr = (Quad->SrcColor >> 16) & 0xff;
			dmulg = (Quad->SrcColor >>  8) & 0xff;
			dmulb = (Quad->SrcColor >>  0) & 0xff;
			break;
		case 0x04:
			dmulr = scr;
			dmulg = scg;
			dmulb = scb;
			break;
		case 0x08:
			dmulr = (Quad->DstColor >> 16) & 0xff;
			dmulg = (Quad->DstColor >>  8) & 0xff;
			dmulb = (Quad->DstColor >>  0) & 0xff;
			break;
		case 0x10:
			dmulr = dcr;
			dmulg = dcg;
			dmulb = dcb;
			break;
		default:
			dmulr = dmulg = dmulb = 0;
			break;
	}

	if (Quad->DstAlpha&0x20)
	{
		dmulr = 0x100 - dmulr;
		dmulg = 0x100 - dmulg;
		dmulb = 0x100 - dmulb;
	}


	dcr = (scr * smulr + dcr * dmulr) >> 8;
	if (dcr > 0xff)
		dcr = 0xff;

	dcg = (scg * smulg + dcg * dmulg) >> 8;
	if (dcg > 0xff)
		dcg = 0xff;

	dcb = (scb * smulb + dcb * dmulb) >> 8;
	if (dcb > 0xff)
		dcb = 0xff;

	return RGB16(dcr, dcg, dcb);
}

#define TILENAME(bpp, t, a) \
static void DrawQuad##bpp##t##a(QuadInfo *Quad)

//TRUST ON THE COMPILER OPTIMIZATIONS
#define TILETEMPL(bpp, t, a) \
TILENAME(bpp, t, a)\
{\
	uint32_t TransColor = Quad->Trans ? RGB32TO16(Quad->TransColor) : NOTRANSCOLOR;\
	uint32_t x, y;\
	uint16_t *line = Quad->Dest;\
	uint32_t y_tx = Quad->Tx, y_ty = Quad->Ty;\
	uint32_t x_tx, x_ty;\
	uint32_t Maskw = Quad->TWidth - 1;\
	uint32_t Maskh = Quad->THeight - 1;\
	uint32_t W = Quad->TWidth >> 3;\
\
	for (y = 0; y < Quad->h; ++y)\
	{\
		uint16_t *pixel = line;\
		x_tx = y_tx;\
		x_ty = y_ty;\
		for (x = 0; x < Quad->w; ++x)\
		{\
			uint32_t Offset;\
			uint32_t tx = x_tx >> 9;\
			uint32_t ty = x_ty >> 9;\
			uint16_t Color;\
			if (Quad->Clamp)\
			{\
				if (tx > Maskw)\
					goto Clamped;\
				if (ty > Maskh)\
					goto Clamped;\
			}\
			else\
			{\
				tx &= Maskw;\
				ty &= Maskh;\
			}\
\
			if(t)\
			{\
				uint32_t Index=Quad->Tile[(ty>>3)*(W)+(tx>>3)];\
				Offset=(Index<<6)+((ty&7)<<3)+(tx&7);\
				if(Index==0) goto Clamped;\
			}\
			else\
				Offset = ty * (Quad->TWidth) + tx;\
\
			if (bpp == 4)\
			{\
				uint8_t Texel = Quad->u.Imageb[Offset / 2];\
				if (Offset & 1)\
					Texel &= 0xf;\
				else\
					Texel = (Texel >> 4) & 0xf;\
				Color = Quad->Pal[Texel];\
			}\
			else if (bpp == 8)\
			{\
				uint8_t Texel = Quad->u.Imageb[Offset];\
				Color = Quad->Pal[Texel];\
			}\
			else if (bpp == 16)\
			{\
				Color = Quad->u.Imagew[Offset];\
			}\
			if (Color != TransColor)\
			{\
				if (a == 1)\
					*pixel = Alpha(Quad, Color, *pixel);\
				else if (a == 2)\
					*pixel = Shade(Color, Quad->Shade);\
				else\
					*pixel = Color;\
			}\
			Clamped:\
			++pixel;\
			x_tx += Quad->Txdx;\
			x_ty += Quad->Tydx;\
		}\
		line += Quad->Pitch;\
		y_tx += Quad->Txdy;\
		y_ty += Quad->Tydy;\
	}\
}
TILETEMPL(16,0,0) TILETEMPL(16,0,1) TILETEMPL(16,0,2)
TILETEMPL(16,1,0) TILETEMPL(16,1,1) TILETEMPL(16,1,2)

TILETEMPL(8,0,0) TILETEMPL(8,0,1) TILETEMPL(8,0,2)
TILETEMPL(8,1,0) TILETEMPL(8,1,1) TILETEMPL(8,1,2)

TILETEMPL(4,0,0) TILETEMPL(4,0,1) TILETEMPL(4,0,2)
TILETEMPL(4,1,0) TILETEMPL(4,1,1) TILETEMPL(4,1,2)

#undef TILENAME
#define TILENAME(bpp, t, a) \
DrawQuad##bpp##t##a


static void DrawQuadFill(QuadInfo *Quad)
{
	uint32_t x, y;
	uint16_t *line = Quad->Dest;
	uint16_t ShadeColor = RGB32TO16(Quad->Shade);
	for (y = 0; y < Quad->h; ++y)
	{
		uint16_t *pixel = line;
		for (x = 0; x < Quad->w; ++x)
		{
			if (Quad->SrcAlpha)
				*pixel = Alpha(Quad, Quad->Shade, *pixel);
			else
				*pixel = ShadeColor;
			++pixel;
		}
		line += Quad->Pitch;
	}
}

typedef void (*_DrawTemplate)(QuadInfo *);

static const _DrawTemplate DrawImage[]=
{
	TILENAME(4,0,0),
	TILENAME(8,0,0),
	TILENAME(16,0,0),
	TILENAME(16,0,0),

	TILENAME(4,0,1),
	TILENAME(8,0,1),
	TILENAME(16,0,1),
	TILENAME(16,0,1),

	TILENAME(4,0,2),
	TILENAME(8,0,2),
	TILENAME(16,0,2),
	TILENAME(16,0,2),
};

static const _DrawTemplate DrawTile[]=
{
	TILENAME(4,1,0),
	TILENAME(8,1,0),
	TILENAME(16,1,0),
	TILENAME(16,1,0),

	TILENAME(4,1,1),
	TILENAME(8,1,1),
	TILENAME(16,1,1),
	TILENAME(16,1,1),

	TILENAME(4,1,2),
	TILENAME(8,1,2),
	TILENAME(16,1,2),
	TILENAME(16,1,2),
};

#define Packet(i) space.read_word(PacketPtr + 2 * i)

//Returns true if the operation was a flip (sync or async)
// TODO: async loading actually doesn't stop rendering but just flips the render bank
int vr0video_device::vrender0_ProcessPacket(uint32_t PacketPtr, uint16_t *Dest)
{
	// TODO: this need to be removed
	address_space &space = m_cpu->space(AS_PROGRAM);
	uint8_t *TEXTURE = m_textureram;

	uint32_t Dx = Packet(1) & 0x3ff;
	uint32_t Dy = Packet(2) & 0x1ff;
	uint32_t Endx = Packet(3) & 0x3ff;
	uint32_t Endy = Packet(4) & 0x1ff;
	uint32_t Mode = 0;
	uint16_t Packet0 = Packet(0);

	if (Packet0 & 0x81) //Sync or ASync flip
	{
		m_LastPalUpdate = 0xffffffff;    //Force update palette next frame
		return 1;
	}

	if (Packet0 & 0x200)
	{
		m_RenderState.Tx = Packet(5) | ((Packet(6) & 0x1f) << 16);
		m_RenderState.Ty = Packet(7) | ((Packet(8) & 0x1f) << 16);
	}
	else
	{
		m_RenderState.Tx = 0;
		m_RenderState.Ty = 0;
	}
	if (Packet0 & 0x400)
	{
		m_RenderState.Txdx = Packet(9)  | ((Packet(10) & 0x1f) << 16);
		m_RenderState.Tydx = Packet(11) | ((Packet(12) & 0x1f) << 16);
		m_RenderState.Txdy = Packet(13) | ((Packet(14) & 0x1f) << 16);
		m_RenderState.Tydy = Packet(15) | ((Packet(16) & 0x1f) << 16);
	}
	else
	{
		m_RenderState.Txdx = 1 << 9;
		m_RenderState.Tydx = 0;
		m_RenderState.Txdy = 0;
		m_RenderState.Tydy = 1 << 9;
	}
	if (Packet0 & 0x800)
	{
		m_RenderState.SrcAlphaColor = Packet(17) | ((Packet(18) & 0xff) << 16);
		m_RenderState.SrcBlend = (Packet(18) >> 8) & 0x3f;
		m_RenderState.DstAlphaColor = Packet(19) | ((Packet(20) & 0xff) << 16);
		m_RenderState.DstBlend = (Packet(20) >> 8) & 0x3f;
	}
	if (Packet0 & 0x1000)
		m_RenderState.ShadeColor = Packet(21) | ((Packet(22) & 0xff) << 16);
	if (Packet0 & 0x2000)
		m_RenderState.TransColor = Packet(23) | ((Packet(24) & 0xff) << 16);
	if (Packet0 & 0x4000)
	{
		m_RenderState.TileOffset = Packet(25);
		m_RenderState.FontOffset = Packet(26);
		m_RenderState.PalOffset = Packet(27) >> 3;
		m_RenderState.PaletteBank = (Packet(28) >> 8) & 0xf;
		m_RenderState.TextureMode = Packet(28) & 0x1000;
		m_RenderState.PixelFormat = (Packet(28) >> 6) & 3;
		m_RenderState.Width  = 8 << ((Packet(28) >> 0) & 0x7);
		m_RenderState.Height = 8 << ((Packet(28) >> 3) & 0x7);
	}

	if (Packet0 & 0x40 && m_RenderState.PalOffset != m_LastPalUpdate)
	{
		uint32_t *Pal = (uint32_t*) (TEXTURE + 1024 * m_RenderState.PalOffset);
		uint16_t Trans = RGB32TO16(m_RenderState.TransColor);
		int i;
		for (i = 0; i < 256; ++i)
		{
			uint32_t p = Pal[i];
			uint16_t v = RGB32TO16(p);
			if ((v == Trans && p != m_RenderState.TransColor) || v == NOTRANSCOLOR)  //Error due to conversion. caused transparent
			{
				if ((v & 0x1f) != 0x1f)
					v++;                                    //Make the color a bit different (blueish) so it's not
				else
					v--;
			}
			m_InternalPalette[i] = v;                        //made transparent by mistake
		}
		m_LastPalUpdate = m_RenderState.PalOffset;
	}

	if (Packet0 & 0x100)
	{
		QuadInfo Quad;

		Quad.Pitch = 1024;

//      assert(Endx >= Dx && Endy >= Dy);

		if (Packet0 & 2)
		{
			Quad.SrcAlpha = m_RenderState.SrcBlend;
			Quad.DstAlpha = m_RenderState.DstBlend;
			Quad.SrcColor = m_RenderState.SrcAlphaColor;
			Quad.DstColor = m_RenderState.DstAlphaColor;
			Mode = 1;
		}
		else
			Quad.SrcAlpha = 0;

		Quad.w = 1 + Endx - Dx;
		Quad.h = 1 + Endy - Dy;

		Quad.Dest = (uint16_t*) Dest;
		Quad.Dest = Quad.Dest + Dx + (Dy * Quad.Pitch);

		Quad.Tx = m_RenderState.Tx;
		Quad.Ty = m_RenderState.Ty;
		Quad.Txdx = m_RenderState.Txdx;
		Quad.Tydx = m_RenderState.Tydx;
		Quad.Txdy = m_RenderState.Txdy;
		Quad.Tydy = m_RenderState.Tydy;
		if (Packet0 & 0x10)
		{
			Quad.Shade = m_RenderState.ShadeColor;
			if (!Mode)      //Alpha includes Shade
				Mode = 2;
			/*
			//simulate shade with alphablend (SLOW!!!)
			if (!Quad.SrcAlpha && (Packet0 & 0x8))
			{
			    Quad.SrcAlpha = 0x21; //1
			    Quad.DstAlpha = 0x01; //0
			}*/
		}
		else
			Quad.Shade = RGB32(255,255,255);
		Quad.TransColor = m_RenderState.TransColor;
		Quad.TWidth = m_RenderState.Width;
		Quad.THeight = m_RenderState.Height;
		Quad.Trans = Packet0 & 4;
		//Quad.Trans = 0;
		Quad.Clamp = Packet0 & 0x20;

		if (Packet0 & 0x8)  //Texture Enable
		{
			Quad.u.Imageb = TEXTURE + 128 * m_RenderState.FontOffset;
			Quad.Tile = (uint16_t*) (TEXTURE + 128 * m_RenderState.TileOffset);
			if (!m_RenderState.PixelFormat)
				Quad.Pal = m_InternalPalette + (m_RenderState.PaletteBank * 16);
			else
				Quad.Pal = m_InternalPalette;
			if (m_RenderState.TextureMode)   //Tiled
				DrawTile[m_RenderState.PixelFormat + 4 * Mode](&Quad);
			else
				DrawImage[m_RenderState.PixelFormat + 4 * Mode](&Quad);
		}
		else
			DrawQuadFill(&Quad);
	}
	return 0;
}

void vr0video_device::execute_drawing()
{
	if (m_render_start == false)
		return;

	uint32_t B0 = 0x000000;
	uint32_t B1 = (m_bank1_select == true ? 0x400000 : 0x100000)/2;
	uint16_t *DrawDest;
	uint16_t *Front, *Back;
	int DoFlip = 0;

	if (m_display_bank & 1)
	{
		Front = (m_frameram + B1);
		Back  = (m_frameram + B0);
	}
	else
	{
		Front = (m_frameram + B0);
		Back  = (m_frameram + B1);
	}

	DrawDest = ((m_draw_select == true) ? Front : Back);

	while ((m_queue_rear & 0x7ff) != (m_queue_front & 0x7ff))
	{
		DoFlip = vrender0_ProcessPacket(0x03800000 + m_queue_rear * 64, DrawDest);
		m_queue_rear ++;
		m_queue_rear &= 0x7ff;
		if (DoFlip)
			break;
	}

	if (DoFlip)
	{
		if (m_flip_count)
		{
			m_flip_count--;
			m_display_bank ^= 1;
		}
	}
}

uint32_t vr0video_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t *Visible;
	const uint32_t width = cliprect.width();

	uint32_t B0 = 0x000000;
	uint32_t B1 = (m_bank1_select == true ? 0x400000 : 0x100000)/2;

	if (m_display_bank & 1)
		Visible = (m_frameram + B1);
	else
		Visible = (m_frameram + B0);

	uint32_t const dx = cliprect.left();
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		std::copy_n(&Visible[(y * 1024) + dx], width, &bitmap.pix16(y, dx));

	return 0;
}
