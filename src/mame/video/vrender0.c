#include "driver.h"
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

static UINT16 InternalPalette[256];
static UINT32 LastPalUpdate=0xffffffff;

/*
Pick a rare enough color to disable transparency (that way I save a cmp per loop to check
if I must draw transparent or not. The palette build will take this color in account so
no color in the palette will have this value
*/
#define NOTRANSCOLOR	0xecda

#define RGB32(r,g,b) ((r<<16)|(g<<8)|(b<<0))
#define RGB16(r,g,b) ((r&0xf8)<<8)|((g&0xfc)<<3)|((b&0xf8)>>3)

INLINE UINT16 RGB32TO16(UINT32 rgb)
{
	return (((rgb>>(16+3))&0x1f)<<11)|(((rgb>>(8+2))&0x3f)<<5)|(((rgb>>(3))&0x1f)<<0);
}

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

static _RenderState RenderState;

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

#define EXTRACTR8(Src)	(((Src>>11)<<3)&0xff)
#define EXTRACTG8(Src)	(((Src>>5)<<2)&0xff)
#define EXTRACTB8(Src)	(((Src>>0)<<3)&0xff)

INLINE UINT16 Shade(UINT16 Src,UINT32 Shade)
{
	UINT32 scr=(EXTRACTR8(Src)*((Shade>>16)&0xff))>>8;
	UINT32 scg=(EXTRACTG8(Src)*((Shade>>8)&0xff))>>8;
	UINT32 scb=(EXTRACTB8(Src)*((Shade>>0)&0xff))>>8;
	return RGB16(scr,scg,scb);
}

static UINT16 Alpha(_Quad *Quad,UINT16 Src,UINT16 Dst)
{
	UINT32 scr=(EXTRACTR8(Src)*((Quad->Shade>>16)&0xff))>>8;
	UINT32 scg=(EXTRACTG8(Src)*((Quad->Shade>>8)&0xff))>>8;
	UINT32 scb=(EXTRACTB8(Src)*((Quad->Shade>>0)&0xff))>>8;
	UINT32 dcr=EXTRACTR8(Dst);
	UINT32 dcg=EXTRACTG8(Dst);
	UINT32 dcb=EXTRACTB8(Dst);

	UINT32 smulr,smulg,smulb;
	UINT32 dmulr,dmulg,dmulb;

	switch(Quad->SrcAlpha&0x1f)
	{
		case 0x01:
			smulr=smulg=smulb=0;
			break;
		case 0x02:
			smulr=(Quad->SrcColor>>16)&0xff;
			smulg=(Quad->SrcColor>>8)&0xff;
			smulb=(Quad->SrcColor>>0)&0xff;
			break;
		case 0x04:
			smulr=scr;
			smulg=scg;
			smulb=scb;
			break;
		case 0x08:
			smulr=(Quad->DstColor>>16)&0xff;
			smulg=(Quad->DstColor>>8)&0xff;
			smulb=(Quad->DstColor>>0)&0xff;
			break;
		case 0x10:
			smulr=dcr;
			smulg=dcg;
			smulb=dcb;
			break;
		default:
			smulr=smulg=smulb=0;
			break;
	}
	if(Quad->SrcAlpha&0x20)
	{
		smulr=0x100-smulr;
		smulg=0x100-smulg;
		smulb=0x100-smulb;
	}

	switch(Quad->DstAlpha&0x1f)
	{
		case 0x01:
			dmulr=dmulg=dmulb=0;
			break;
		case 0x02:
			dmulr=(Quad->SrcColor>>16)&0xff;
			dmulg=(Quad->SrcColor>>8)&0xff;
			dmulb=(Quad->SrcColor>>0)&0xff;
			break;
		case 0x04:
			dmulr=scr;
			dmulg=scg;
			dmulb=scb;
			break;
		case 0x08:
			dmulr=(Quad->DstColor>>16)&0xff;
			dmulg=(Quad->DstColor>>8)&0xff;
			dmulb=(Quad->DstColor>>0)&0xff;
			break;
		case 0x10:
			dmulr=dcr;
			dmulg=dcg;
			dmulb=dcb;
			break;
		default:
			dmulr=dmulg=dmulb=0;
			break;
	}
	if(Quad->DstAlpha&0x20)
	{
		dmulr=0x100-dmulr;
		dmulg=0x100-dmulg;
		dmulb=0x100-dmulb;
	}


	dcr=(scr*smulr+dcr*dmulr)>>8;
	if(dcr>0xff)
		dcr=0xff;
	dcg=(scg*smulg+dcg*dmulg)>>8;
	if(dcg>0xff)
		dcg=0xff;
	dcb=(scb*smulb+dcb*dmulb)>>8;
	if(dcb>0xff)
		dcb=0xff;
	return RGB16(dcr,dcg,dcb);
}

#define TILENAME(bpp,t,a) \
static void DrawQuad##bpp##t##a(_Quad *Quad)

//TRUST ON THE COMPILER OPTIMIZATIONS
#define TILETEMPL(bpp,t,a) \
TILENAME(bpp,t,a)\
{\
	UINT32 TransColor=Quad->Trans?RGB32TO16(Quad->TransColor):NOTRANSCOLOR;\
	UINT32 x,y;\
	UINT16 *line=Quad->Dest;\
	UINT32 y_tx=Quad->Tx,y_ty=Quad->Ty;\
	UINT32 x_tx,x_ty;\
	UINT32 Maskw=Quad->TWidth-1;\
	UINT32 Maskh=Quad->THeight-1;\
	UINT32 W=Quad->TWidth>>3;\
\
	for(y=0;y<Quad->h;++y)\
	{\
		UINT16 *pixel=line;\
		x_tx=y_tx;\
		x_ty=y_ty;\
		for(x=0;x<Quad->w;++x)\
		{\
			UINT32 Offset;\
			UINT32 tx=x_tx>>9;\
			UINT32 ty=x_ty>>9;\
			UINT16 Color;\
			if(Quad->Clamp)\
			{\
				if(tx>Maskw)\
					goto Clamped;\
				if(ty>Maskh)\
					goto Clamped;\
			}\
			else\
			{\
				tx&=Maskw;\
				ty&=Maskh;\
			}\
\
			if(t)\
			{\
				UINT32 Index=Quad->Tile[(ty>>3)*(W)+(tx>>3)];\
				Offset=(Index<<6)+((ty&7)<<3)+(tx&7);\
			}\
			else\
				Offset=ty*(Quad->TWidth)+tx;\
\
			if(bpp==4)\
			{\
				UINT8 Texel=Quad->u.Imageb[Offset/2];\
				if(Offset&1)\
					Texel&=0xf;\
				else\
					Texel=(Texel>>4)&0xf;\
				Color=Quad->Pal[Texel];\
			}\
			else if(bpp==8)\
			{\
				UINT8 Texel=Quad->u.Imageb[Offset];\
				Texel=Quad->u.Imageb[Offset];\
				Color=Quad->Pal[Texel];\
			}\
			else if(bpp==16)\
			{\
				Color=Quad->u.Imagew[Offset];\
			}\
			if(Color!=TransColor)\
			{\
				if(a==1)\
					*pixel=Alpha(Quad,Color,*pixel);\
				else if(a==2)\
					*pixel=Shade(Color,Quad->Shade);\
				else\
					*pixel=Color;\
			}\
			++pixel;\
Clamped:\
			x_tx+=Quad->Txdx;\
			x_ty+=Quad->Tydx;\
		}\
		line+=Quad->Pitch;\
		y_tx+=Quad->Txdy;\
		y_ty+=Quad->Tydy;\
	}\
}\

TILETEMPL(16,0,0) TILETEMPL(16,0,1) TILETEMPL(16,0,2)
TILETEMPL(16,1,0) TILETEMPL(16,1,1) TILETEMPL(16,1,2)

TILETEMPL(8,0,0) TILETEMPL(8,0,1) TILETEMPL(8,0,2)
TILETEMPL(8,1,0) TILETEMPL(8,1,1) TILETEMPL(8,1,2)

TILETEMPL(4,0,0) TILETEMPL(4,0,1) TILETEMPL(4,0,2)
TILETEMPL(4,1,0) TILETEMPL(4,1,1) TILETEMPL(4,1,2)

#undef TILENAME
#define TILENAME(bpp,t,a) \
DrawQuad##bpp##t##a


static void DrawQuadFill(_Quad *Quad)
{
	UINT32 x,y;
	UINT16 *line=Quad->Dest;
	UINT16 ShadeColor=RGB32TO16(Quad->Shade);
	for(y=0;y<Quad->h;++y)
	{
		UINT16 *pixel=line;
		for(x=0;x<Quad->w;++x)
		{
			if(Quad->SrcAlpha)
				*pixel=Alpha(Quad,Quad->Shade,*pixel);
			else
				*pixel=ShadeColor;
			++pixel;
		}
		line+=Quad->Pitch;
	}
}

typedef void (*_DrawTemplate)(_Quad *);

static _DrawTemplate DrawImage[]=
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

static _DrawTemplate DrawTile[]=
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

#define Packet(i) program_read_word_32le(PacketPtr+2*i)

//Returns TRUE if the operation was a flip (sync or async)
int ProcessPacket(UINT32 PacketPtr,UINT16 *Dest,UINT8 *TEXTURE)
{
	UINT32 Dx=Packet(1)&0x3ff;
	UINT32 Dy=Packet(2)&0x1ff;
	UINT32 Endx=Packet(3)&0x3ff;
	UINT32 Endy=Packet(4)&0x1ff;
	UINT32 Mode=0;
	UINT16 Packet0=Packet(0);

	if(Packet0&0x81)	//Sync or ASync flip
	{
		LastPalUpdate=0xffffffff;	//Force update palette next frame
		return 1;
	}

	if(Packet0&0x200)
	{
		RenderState.Tx=Packet(5)|((Packet(6)&0x1F)<<16);
		RenderState.Ty=Packet(7)|((Packet(8)&0x1F)<<16);
	}
	else
	{
		RenderState.Tx=0;
		RenderState.Ty=0;
	}
	if(Packet0&0x400)
	{
		RenderState.Txdx=Packet(9)|((Packet(10)&0x1F)<<16);
		RenderState.Tydx=Packet(11)|((Packet(12)&0x1F)<<16);
		RenderState.Txdy=Packet(13)|((Packet(14)&0x1F)<<16);
		RenderState.Tydy=Packet(15)|((Packet(16)&0x1F)<<16);
	}
	else
	{
		RenderState.Txdx=1<<9;
		RenderState.Tydx=0;
		RenderState.Txdy=0;
		RenderState.Tydy=1<<9;
	}
	if(Packet0&0x800)
	{
		RenderState.SrcAlphaColor=Packet(17)|((Packet(18)&0xff)<<16);
		RenderState.SrcBlend=(Packet(18)>>8)&0x3f;
		RenderState.DstAlphaColor=Packet(19)|((Packet(20)&0xff)<<16);
		RenderState.DstBlend=(Packet(20)>>8)&0x3f;
	}
	if(Packet0&0x1000)
		RenderState.ShadeColor=Packet(21)|((Packet(22)&0xFF)<<16);
	if(Packet0&0x2000)
		RenderState.TransColor=Packet(23)|((Packet(24)&0xFF)<<16);
	if(Packet0&0x4000)
	{
		RenderState.TileOffset=Packet(25);
		RenderState.FontOffset=Packet(26);
		RenderState.PalOffset=Packet(27)>>3;
		RenderState.PaletteBank=(Packet(28)>>8)&0xF;
		RenderState.TextureMode=Packet(28)&0x1000;
		RenderState.PixelFormat=(Packet(28)>>6)&3;
		RenderState.Width =8<<((Packet(28)>>0)&0x7);
		RenderState.Height=8<<((Packet(28)>>3)&0x7);
	}


	if(Packet0&0x40 && RenderState.PalOffset!=LastPalUpdate)
	{
		UINT32 *Pal=(UINT32*) (TEXTURE+1024*RenderState.PalOffset);
		UINT16 Trans=RGB32TO16(RenderState.TransColor);
		int i;
		for(i=0;i<256;++i)
		{
			UINT32 p=Pal[i];
			UINT16 v=RGB32TO16(p);
			if((v==Trans && p!=RenderState.TransColor) || v==NOTRANSCOLOR)	//Error due to conversion. caused transparent
			{
				if((v&0x1f)!=0x1f)
					v++;									//Make the color a bit different (blueish) so it's not
				else
					v--;
			}
			InternalPalette[i]=v;						//made transparent by mistake
		}
		LastPalUpdate=RenderState.PalOffset;
	}

	if(Packet0&0x100)
	{
		_Quad Quad;

		Quad.Pitch=512;

//      assert(Endx>=Dx && Endy>=Dy);

		if(Packet0&2)
		{
			Quad.SrcAlpha=RenderState.SrcBlend;
			Quad.DstAlpha=RenderState.DstBlend;
			Quad.SrcColor=RenderState.SrcAlphaColor;
			Quad.DstColor=RenderState.DstAlphaColor;
			Mode=1;
		}
		else
			Quad.SrcAlpha=0;

		Quad.w=1+Endx-Dx;
		Quad.h=1+Endy-Dy;

		Quad.Dest=(UINT16*) Dest;
		Quad.Dest=Quad.Dest+Dx+(Dy*Quad.Pitch);

		Quad.Tx=RenderState.Tx;
		Quad.Ty=RenderState.Ty;
		Quad.Txdx=RenderState.Txdx;
		Quad.Tydx=RenderState.Tydx;
		Quad.Txdy=RenderState.Txdy;
		Quad.Tydy=RenderState.Tydy;
		if(Packet0&0x10)
		{
			Quad.Shade=RenderState.ShadeColor;
			if(!Mode)		//Alpha includes Shade
				Mode=2;
			/*
            //simulate shade with alphablend (SLOW!!!)
            if(!Quad.SrcAlpha && (Packet0&0x8))
            {
                Quad.SrcAlpha=0x21; //1
                Quad.DstAlpha=0x01; //0
            }*/
		}
		else
			Quad.Shade=RGB32(255,255,255);
		Quad.TransColor=RenderState.TransColor;
		Quad.TWidth=RenderState.Width;
		Quad.THeight=RenderState.Height;
		Quad.Trans=Packet0&4;
		//Quad.Trans=0;
		Quad.Clamp=Packet0&0x20;

		if(Packet0&0x8)	//Texture Enable
		{
			Quad.u.Imageb=TEXTURE+128*RenderState.FontOffset;
			Quad.Tile=(UINT16*) (TEXTURE+128*RenderState.TileOffset);
			if(!RenderState.PixelFormat)
				Quad.Pal=InternalPalette+(RenderState.PaletteBank*16);
			else
				Quad.Pal=InternalPalette;
			if(RenderState.TextureMode)	//Tiled
				DrawTile[RenderState.PixelFormat+4*Mode](&Quad);
			else
				DrawImage[RenderState.PixelFormat+4*Mode](&Quad);
		}
		else
			DrawQuadFill(&Quad);
	}
	return 0;
}
