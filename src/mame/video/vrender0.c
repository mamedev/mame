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
/*************
Missing:


**************/

typedef struct
{
	UINT16 *Dest;
	UINT32 Pitch;	//in UINT16s
	UINT32 w,h;
	UINT32 Tx;
	UINT32 Ty;
	UINT32 Txdx;
	UINT32 Tydx;
	UINT32 Txdy;
	UINT32 Tydy;
	UINT16 TWidth;
	UINT16 THeight;
	union _u
	{
		UINT8 *Imageb;
		UINT16 *Imagew;
	} u;
	UINT16 *Tile;
	UINT16 *Pal;
	UINT32 TransColor;
	UINT32 Shade;
	UINT8 Clamp;
	UINT8 Trans;
	UINT8 SrcAlpha;
	UINT32 SrcColor;
	UINT8 DstAlpha;
	UINT32 DstColor;
} _Quad;

typedef struct
{
	UINT32 Tx;
	UINT32 Ty;
	UINT32 Txdx;
	UINT32 Tydx;
	UINT32 Txdy;
	UINT32 Tydy;
	UINT32 SrcAlphaColor;
	UINT32 SrcBlend;
	UINT32 DstAlphaColor;
	UINT32 DstBlend;
	UINT32 ShadeColor;
	UINT32 TransColor;
	UINT32 TileOffset;
	UINT32 FontOffset;
	UINT32 PalOffset;
	UINT32 PaletteBank;
	UINT32 TextureMode;
	UINT32 PixelFormat;
	UINT32 Width;
	UINT32 Height;
} _RenderState;

typedef struct _vr0video_state  vr0video_state;
struct _vr0video_state
{
	device_t *cpu;

	UINT16 InternalPalette[256];
	UINT32 LastPalUpdate;

	_RenderState RenderState;
};


/*****************************************************************************
 INLINE FUNCTIONS
 *****************************************************************************/

INLINE vr0video_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == VIDEO_VRENDER0);

	return (vr0video_state *)downcast<vr0video_device *>(device)->token();
}

INLINE const vr0video_interface *get_interface( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == VIDEO_VRENDER0);
	return (const vr0video_interface *) device->static_config();
}

/*****************************************************************************
 IMPLEMENTATION
 *****************************************************************************/

/*
Pick a rare enough color to disable transparency (that way I save a cmp per loop to check
if I must draw transparent or not. The palette build will take this color in account so
no color in the palette will have this value
*/
#define NOTRANSCOLOR	0xecda

#define RGB32(r,g,b) ((r << 16) | (g << 8) | (b << 0))
#define RGB16(r,g,b) ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | ((b & 0xf8) >> 3)

INLINE UINT16 RGB32TO16(UINT32 rgb)
{
	return (((rgb >> (16 + 3)) & 0x1f) << 11) | (((rgb >> (8 + 2)) & 0x3f) << 5) | (((rgb >> (3)) & 0x1f) << 0);
}

#define EXTRACTR8(Src)	(((Src >> 11) << 3) & 0xff)
#define EXTRACTG8(Src)	(((Src >>  5) << 2) & 0xff)
#define EXTRACTB8(Src)	(((Src >>  0) << 3) & 0xff)

INLINE UINT16 Shade(UINT16 Src, UINT32 Shade)
{
	UINT32 scr = (EXTRACTR8(Src) * ((Shade >> 16) & 0xff)) >> 8;
	UINT32 scg = (EXTRACTG8(Src) * ((Shade >>  8) & 0xff)) >> 8;
	UINT32 scb = (EXTRACTB8(Src) * ((Shade >>  0) & 0xff)) >> 8;
	return RGB16(scr, scg, scb);
}

static UINT16 Alpha(_Quad *Quad, UINT16 Src, UINT16 Dst)
{
	UINT32 scr = (EXTRACTR8(Src) * ((Quad->Shade >> 16) & 0xff)) >> 8;
	UINT32 scg = (EXTRACTG8(Src) * ((Quad->Shade >>  8) & 0xff)) >> 8;
	UINT32 scb = (EXTRACTB8(Src) * ((Quad->Shade >>  0) & 0xff)) >> 8;
	UINT32 dcr = EXTRACTR8(Dst);
	UINT32 dcg = EXTRACTG8(Dst);
	UINT32 dcb = EXTRACTB8(Dst);

	UINT32 smulr, smulg, smulb;
	UINT32 dmulr, dmulg, dmulb;

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
static void DrawQuad##bpp##t##a(_Quad *Quad)

//TRUST ON THE COMPILER OPTIMIZATIONS
#define TILETEMPL(bpp, t, a) \
TILENAME(bpp, t, a)\
{\
	UINT32 TransColor = Quad->Trans ? RGB32TO16(Quad->TransColor) : NOTRANSCOLOR;\
	UINT32 x, y;\
	UINT16 *line = Quad->Dest;\
	UINT32 y_tx = Quad->Tx, y_ty = Quad->Ty;\
	UINT32 x_tx, x_ty;\
	UINT32 Maskw = Quad->TWidth - 1;\
	UINT32 Maskh = Quad->THeight - 1;\
	UINT32 W = Quad->TWidth >> 3;\
\
	for (y = 0; y < Quad->h; ++y)\
	{\
		UINT16 *pixel = line;\
		x_tx = y_tx;\
		x_ty = y_ty;\
		for (x = 0; x < Quad->w; ++x)\
		{\
			UINT32 Offset;\
			UINT32 tx = x_tx >> 9;\
			UINT32 ty = x_ty >> 9;\
			UINT16 Color;\
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
			if (t)\
			{\
				UINT32 Index = Quad->Tile[(ty >> 3) * (W) + (tx >> 3)];\
				Offset = (Index << 6) + ((ty & 7) << 3) + (tx & 7);\
			}\
			else\
				Offset = ty * (Quad->TWidth) + tx;\
\
			if (bpp == 4)\
			{\
				UINT8 Texel = Quad->u.Imageb[Offset / 2];\
				if (Offset & 1)\
					Texel &= 0xf;\
				else\
					Texel = (Texel >> 4) & 0xf;\
				Color = Quad->Pal[Texel];\
			}\
			else if (bpp == 8)\
			{\
				UINT8 Texel = Quad->u.Imageb[Offset];\
				Texel = Quad->u.Imageb[Offset];\
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
			++pixel;\
Clamped:\
			x_tx += Quad->Txdx;\
			x_ty += Quad->Tydx;\
		}\
		line += Quad->Pitch;\
		y_tx += Quad->Txdy;\
		y_ty += Quad->Tydy;\
	}\
}\

TILETEMPL(16,0,0) TILETEMPL(16,0,1) TILETEMPL(16,0,2)
TILETEMPL(16,1,0) TILETEMPL(16,1,1) TILETEMPL(16,1,2)

TILETEMPL(8,0,0) TILETEMPL(8,0,1) TILETEMPL(8,0,2)
TILETEMPL(8,1,0) TILETEMPL(8,1,1) TILETEMPL(8,1,2)

TILETEMPL(4,0,0) TILETEMPL(4,0,1) TILETEMPL(4,0,2)
TILETEMPL(4,1,0) TILETEMPL(4,1,1) TILETEMPL(4,1,2)

#undef TILENAME
#define TILENAME(bpp, t, a) \
DrawQuad##bpp##t##a


static void DrawQuadFill(_Quad *Quad)
{
	UINT32 x, y;
	UINT16 *line = Quad->Dest;
	UINT16 ShadeColor = RGB32TO16(Quad->Shade);
	for (y = 0; y < Quad->h; ++y)
	{
		UINT16 *pixel = line;
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

typedef void (*_DrawTemplate)(_Quad *);

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

#define Packet(i) space->read_word(PacketPtr + 2 * i)

//Returns TRUE if the operation was a flip (sync or async)
int vrender0_ProcessPacket(device_t *device, UINT32 PacketPtr, UINT16 *Dest, UINT8 *TEXTURE)
{
	vr0video_state *vr0 = get_safe_token(device);
	address_space *space = vr0->cpu->memory().space(AS_PROGRAM);
	UINT32 Dx = Packet(1) & 0x3ff;
	UINT32 Dy = Packet(2) & 0x1ff;
	UINT32 Endx = Packet(3) & 0x3ff;
	UINT32 Endy = Packet(4) & 0x1ff;
	UINT32 Mode = 0;
	UINT16 Packet0 = Packet(0);

	if (Packet0 & 0x81)	//Sync or ASync flip
	{
		vr0->LastPalUpdate = 0xffffffff;	//Force update palette next frame
		return 1;
	}

	if (Packet0 & 0x200)
	{
		vr0->RenderState.Tx = Packet(5) | ((Packet(6) & 0x1f) << 16);
		vr0->RenderState.Ty = Packet(7) | ((Packet(8) & 0x1f) << 16);
	}
	else
	{
		vr0->RenderState.Tx = 0;
		vr0->RenderState.Ty = 0;
	}
	if (Packet0 & 0x400)
	{
		vr0->RenderState.Txdx = Packet(9)  | ((Packet(10) & 0x1f) << 16);
		vr0->RenderState.Tydx = Packet(11) | ((Packet(12) & 0x1f) << 16);
		vr0->RenderState.Txdy = Packet(13) | ((Packet(14) & 0x1f) << 16);
		vr0->RenderState.Tydy = Packet(15) | ((Packet(16) & 0x1f) << 16);
	}
	else
	{
		vr0->RenderState.Txdx = 1 << 9;
		vr0->RenderState.Tydx = 0;
		vr0->RenderState.Txdy = 0;
		vr0->RenderState.Tydy = 1 << 9;
	}
	if (Packet0 & 0x800)
	{
		vr0->RenderState.SrcAlphaColor = Packet(17) | ((Packet(18) & 0xff) << 16);
		vr0->RenderState.SrcBlend = (Packet(18) >> 8) & 0x3f;
		vr0->RenderState.DstAlphaColor = Packet(19) | ((Packet(20) & 0xff) << 16);
		vr0->RenderState.DstBlend = (Packet(20) >> 8) & 0x3f;
	}
	if (Packet0 & 0x1000)
		vr0->RenderState.ShadeColor = Packet(21) | ((Packet(22) & 0xff) << 16);
	if (Packet0 & 0x2000)
		vr0->RenderState.TransColor = Packet(23) | ((Packet(24) & 0xff) << 16);
	if (Packet0 & 0x4000)
	{
		vr0->RenderState.TileOffset = Packet(25);
		vr0->RenderState.FontOffset = Packet(26);
		vr0->RenderState.PalOffset = Packet(27) >> 3;
		vr0->RenderState.PaletteBank = (Packet(28) >> 8) & 0xf;
		vr0->RenderState.TextureMode = Packet(28) & 0x1000;
		vr0->RenderState.PixelFormat = (Packet(28) >> 6) & 3;
		vr0->RenderState.Width  = 8 << ((Packet(28) >> 0) & 0x7);
		vr0->RenderState.Height = 8 << ((Packet(28) >> 3) & 0x7);
	}

	if (Packet0 & 0x40 && vr0->RenderState.PalOffset != vr0->LastPalUpdate)
	{
		UINT32 *Pal = (UINT32*) (TEXTURE + 1024 * vr0->RenderState.PalOffset);
		UINT16 Trans = RGB32TO16(vr0->RenderState.TransColor);
		int i;
		for (i = 0; i < 256; ++i)
		{
			UINT32 p = Pal[i];
			UINT16 v = RGB32TO16(p);
			if ((v == Trans && p != vr0->RenderState.TransColor) || v == NOTRANSCOLOR)	//Error due to conversion. caused transparent
			{
				if ((v & 0x1f) != 0x1f)
					v++;									//Make the color a bit different (blueish) so it's not
				else
					v--;
			}
			vr0->InternalPalette[i] = v;						//made transparent by mistake
		}
		vr0->LastPalUpdate = vr0->RenderState.PalOffset;
	}

	if (Packet0 & 0x100)
	{
		_Quad Quad;

		Quad.Pitch = 512;

//      assert(Endx >= Dx && Endy >= Dy);

		if (Packet0 & 2)
		{
			Quad.SrcAlpha = vr0->RenderState.SrcBlend;
			Quad.DstAlpha = vr0->RenderState.DstBlend;
			Quad.SrcColor = vr0->RenderState.SrcAlphaColor;
			Quad.DstColor = vr0->RenderState.DstAlphaColor;
			Mode = 1;
		}
		else
			Quad.SrcAlpha = 0;

		Quad.w = 1 + Endx - Dx;
		Quad.h = 1 + Endy - Dy;

		Quad.Dest = (UINT16*) Dest;
		Quad.Dest = Quad.Dest + Dx + (Dy * Quad.Pitch);

		Quad.Tx = vr0->RenderState.Tx;
		Quad.Ty = vr0->RenderState.Ty;
		Quad.Txdx = vr0->RenderState.Txdx;
		Quad.Tydx = vr0->RenderState.Tydx;
		Quad.Txdy = vr0->RenderState.Txdy;
		Quad.Tydy = vr0->RenderState.Tydy;
		if (Packet0 & 0x10)
		{
			Quad.Shade = vr0->RenderState.ShadeColor;
			if (!Mode)		//Alpha includes Shade
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
		Quad.TransColor = vr0->RenderState.TransColor;
		Quad.TWidth = vr0->RenderState.Width;
		Quad.THeight = vr0->RenderState.Height;
		Quad.Trans = Packet0 & 4;
		//Quad.Trans = 0;
		Quad.Clamp = Packet0 & 0x20;

		if (Packet0 & 0x8)	//Texture Enable
		{
			Quad.u.Imageb = TEXTURE + 128 * vr0->RenderState.FontOffset;
			Quad.Tile = (UINT16*) (TEXTURE + 128 * vr0->RenderState.TileOffset);
			if (!vr0->RenderState.PixelFormat)
				Quad.Pal = vr0->InternalPalette + (vr0->RenderState.PaletteBank * 16);
			else
				Quad.Pal = vr0->InternalPalette;
			if (vr0->RenderState.TextureMode)	//Tiled
				DrawTile[vr0->RenderState.PixelFormat + 4 * Mode](&Quad);
			else
				DrawImage[vr0->RenderState.PixelFormat + 4 * Mode](&Quad);
		}
		else
			DrawQuadFill(&Quad);
	}
	return 0;
}

/*****************************************************************************
 DEVICE INTERFACE
 *****************************************************************************/

static DEVICE_START( vr0video )
{
	vr0video_state *vr0 = get_safe_token(device);
	const vr0video_interface *intf = get_interface(device);

	vr0->cpu = device->machine().device(intf->cpu);

	device->save_item(NAME(vr0->InternalPalette));
	device->save_item(NAME(vr0->LastPalUpdate));

	device->save_item(NAME(vr0->RenderState.Tx));
	device->save_item(NAME(vr0->RenderState.Ty));
	device->save_item(NAME(vr0->RenderState.Txdx));
	device->save_item(NAME(vr0->RenderState.Tydx));
	device->save_item(NAME(vr0->RenderState.Txdy));
	device->save_item(NAME(vr0->RenderState.Tydy));
	device->save_item(NAME(vr0->RenderState.SrcAlphaColor));
	device->save_item(NAME(vr0->RenderState.SrcBlend));
	device->save_item(NAME(vr0->RenderState.DstAlphaColor));
	device->save_item(NAME(vr0->RenderState.DstBlend));
	device->save_item(NAME(vr0->RenderState.ShadeColor));
	device->save_item(NAME(vr0->RenderState.TransColor));
	device->save_item(NAME(vr0->RenderState.TileOffset));
	device->save_item(NAME(vr0->RenderState.FontOffset));
	device->save_item(NAME(vr0->RenderState.PalOffset));
	device->save_item(NAME(vr0->RenderState.PaletteBank));
	device->save_item(NAME(vr0->RenderState.TextureMode));
	device->save_item(NAME(vr0->RenderState.PixelFormat));
	device->save_item(NAME(vr0->RenderState.Width));
	device->save_item(NAME(vr0->RenderState.Height));
}

static DEVICE_RESET( vr0video )
{
	vr0video_state *vr0 = get_safe_token(device);

	memset(vr0->InternalPalette, 0, sizeof(vr0->InternalPalette));
	vr0->LastPalUpdate = 0xffffffff;
}

const device_type VIDEO_VRENDER0 = &device_creator<vr0video_device>;

vr0video_device::vr0video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VIDEO_VRENDER0, "VRender0", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(vr0video_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void vr0video_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vr0video_device::device_start()
{
	DEVICE_START_NAME( vr0video )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vr0video_device::device_reset()
{
	DEVICE_RESET_NAME( vr0video )(this);
}


