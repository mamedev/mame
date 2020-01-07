// license:BSD-3-Clause
// copyright-holders:ElSemi
/*****************************************************************************************
        VRENDER ZERO
        VIDEO EMULATION By ElSemi


    The VRender0 is a very special 2D sprite renderer. The spec says it's based on 3D
    technology.
    The device processes "display lists" that contain pointers to the texture, tex coords
    and the step increments on x and y (dxx,dxy,dyx,dyy) allowing ROZ effects on sprites.
    It supports alphablend with programmable factors per channel and for source and dest
    color.

    TODO:
    - Dither Mode;
    - Draw select to Front buffer is untested, speculatively gonna be used for raster
      effects;
    - screen_update doesn't honor CRT Display Start registers,
      so far only psattack changes it on-the-fly, for unknown reasons;

*****************************************************************************************/

#include "emu.h"
#include "vrender0.h"

#include <algorithm>

/*****************************************************************************
 DEVICE INTERFACE
 *****************************************************************************/

DEFINE_DEVICE_TYPE(VIDEO_VRENDER0, vr0video_device, "vr0video", "MagicEyes VRender0 Video Engine")

vr0video_device::vr0video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VIDEO_VRENDER0, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, device_palette_interface(mconfig, *this)
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
	for (int color = 0; color < 32*64*32; color++)
	{
		u8 r = pal5bit((color >> 11) & 0x1f);
		u8 g = pal6bit((color >> 5) & 0x3f);
		u8 b = pal5bit((color >> 0) & 0x1f);
		set_pen_color(color, r, g, b);
	}

	for (int color = 0; color < 0x100; color++)
	{
		m_multiply_lookup[color] = std::make_unique<u8[]>(0x100 + 1);
		m_additive_lookup[color] = std::make_unique<u8[]>(0x100 + 1);
		for (int level = 0; level < 0x100 + 1; level++)
		{
			m_multiply_lookup[color][level] = std::max(0, std::min(0xff, (color * level) >> 8));
			m_additive_lookup[color][level] = std::min(0xff, color + level);
		}
	}
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

void vr0video_device::set_areas(u16 *textureram, u16 *frameram)
{
	m_textureram = (u8 *)textureram;
	m_packetram = textureram;
	m_frameram = frameram;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vr0video_device::device_reset()
{
	memset(m_InternalPalette, 0, sizeof(m_InternalPalette));
	m_LastPalUpdate = 0xffffffff;

	m_DisplayDest = m_DrawDest = m_frameram;
}

/*****************************************************************************
 IMPLEMENTATION
 *****************************************************************************/

/*
Pick a rare enough color to disable transparency (that way I save a cmp per loop to check
if I must draw transparent or not. The palette build will take this color in account so
no color in the palette will have this value
*/
static constexpr unsigned ALPHA_BIT = 1;
static constexpr unsigned SHADE_BIT = 2;

static constexpr u16 NOTRANSCOLOR = 0xecda;

static inline u32 RGB32(u8 r, u8 g, u8 b) { return (r << 16) | (g << 8) | (b << 0); }
static inline u16 RGB16(u8 r, u8 g, u8 b) { return ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | ((b & 0xf8) >> 3); }

static inline u16 RGB32TO16(u32 rgb)
{
	return (((rgb >> (16 + 3)) & 0x1f) << 11) | (((rgb >> (8 + 2)) & 0x3f) << 5) | (((rgb >> (3)) & 0x1f) << 0);
}

static inline u8 EXTRACTR8(u16 Src) { return ((Src >> 11) << 3) & 0xff; }
static inline u8 EXTRACTG8(u16 Src) { return ((Src >>  5) << 2) & 0xff; }
static inline u8 EXTRACTB8(u16 Src) { return ((Src >>  0) << 3) & 0xff; }

inline void vr0video_device::DrawPixel(QuadInfo *Quad, u16 *Dst, u16 Src)
{
	if (Quad->Blend & ALPHA_BIT)
		*Dst = Alpha(Quad, Src, *Dst);
	else if (Quad->Blend & SHADE_BIT)
		*Dst = Shade(Quad, Src);
	else
		*Dst = Src;
}

inline u16 vr0video_device::Shade(QuadInfo *Quad, u16 Src)
{
	u32 scr = m_multiply_lookup[EXTRACTR8(Src)][(Quad->Shade >> 16) & 0xff];
	u32 scg = m_multiply_lookup[EXTRACTG8(Src)][(Quad->Shade >>  8) & 0xff];
	u32 scb = m_multiply_lookup[EXTRACTB8(Src)][(Quad->Shade >>  0) & 0xff];
	return RGB16(scr, scg, scb);
}

u16 vr0video_device::Alpha(QuadInfo *Quad, u16 Src, u16 Dst)
{
	u32 scr = EXTRACTR8(Src);
	u32 scg = EXTRACTG8(Src);
	u32 scb = EXTRACTB8(Src);
	if (Quad->Blend & SHADE_BIT)
	{
		scr = m_multiply_lookup[scr][(Quad->Shade >> 16) & 0xff];
		scg = m_multiply_lookup[scg][(Quad->Shade >>  8) & 0xff];
		scb = m_multiply_lookup[scb][(Quad->Shade >>  0) & 0xff];
	}
	u32 dcr = EXTRACTR8(Dst);
	u32 dcg = EXTRACTG8(Dst);
	u32 dcb = EXTRACTB8(Dst);

	u32 smulr, smulg, smulb;
	u32 dmulr, dmulg, dmulb;

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

	if (Quad->DstAlpha & 0x20)
	{
		dmulr = 0x100 - dmulr;
		dmulg = 0x100 - dmulg;
		dmulb = 0x100 - dmulb;
	}

	dcr = m_additive_lookup[m_multiply_lookup[scr][smulr]][m_multiply_lookup[dcr][dmulr]];
	dcg = m_additive_lookup[m_multiply_lookup[scg][smulg]][m_multiply_lookup[dcg][dmulg]];
	dcb = m_additive_lookup[m_multiply_lookup[scb][smulb]][m_multiply_lookup[dcb][dmulb]];

	return RGB16(dcr, dcg, dcb);
}

void vr0video_device::DrawQuadTexture(QuadInfo *Quad)
{
	const u32 TransColor = Quad->Trans ? RGB32TO16(Quad->TransColor) : NOTRANSCOLOR;
	u16 *line = Quad->Dest;
	u32 y_tx = Quad->Tx, y_ty = Quad->Ty;
	const u32 Maskw = Quad->TWidth - 1;
	const u32 Maskh = Quad->THeight - 1;
	const u32 W = Quad->TWidth >> 3;

	for (u32 y = 0; y < Quad->h; ++y)
	{
		u16 *pixel = line;
		u32 x_tx = y_tx;
		u32 x_ty = y_ty;
		for (u32 x = 0; x < Quad->w; ++x)
		{
			u32 Offset;
			u32 tx = x_tx >> 9;
			u32 ty = x_ty >> 9;
			u16 Color = ~0;
			if (Quad->Clamp)
			{
				if (tx > Maskw)
					goto Clamped;
				if (ty > Maskh)
					goto Clamped;
			}
			else
			{
				tx &= Maskw;
				ty &= Maskh;
			}

			if (Quad->Tiled)
			{
				const u32 Index = Quad->Tile[(ty >> 3) * (W) + (tx >> 3)];
				Offset=(Index << 6) + ((ty & 7) << 3) + (tx & 7);
				if (Index == 0) goto Clamped;
			}
			else
				Offset = ty * (Quad->TWidth) + tx;

			u8 Texel;
			switch (Quad->bpp)
			{
			case 4:
				Texel = Quad->u.Imageb[Offset / 2];
				if (Offset & 1)
					Texel &= 0xf;
				else
					Texel = (Texel >> 4) & 0xf;
				Color = Quad->Pal[Texel];
				break;
			case 8:
				Texel = Quad->u.Imageb[Offset];
				Color = Quad->Pal[Texel];
				break;
			case 16:
			default:
				Color = Quad->u.Imagew[Offset];
				break;
			}
			if (Color != TransColor)
			{
				DrawPixel(Quad, pixel, Color);
			}
			Clamped:
			++pixel;
			x_tx += Quad->Txdx;
			x_ty += Quad->Tydx;
		}
		line += Quad->Pitch;
		y_tx += Quad->Txdy;
		y_ty += Quad->Tydy;
	}
}


void vr0video_device::DrawQuadFill(QuadInfo *Quad)
{
	u16 *line = Quad->Dest;
	for (u32 y = 0; y < Quad->h; ++y)
	{
		u16 *pixel = line;
		for (u32 x = 0; x < Quad->w; ++x)
		{
			DrawPixel(Quad, pixel, ~0);
			++pixel;
		}
		line += Quad->Pitch;
	}
}

//Returns true if the operation was a flip (sync or async)
// TODO: async loading actually doesn't stop rendering but just flips the render bank
int vr0video_device::vrender0_ProcessPacket(u32 PacketPtr)
{
	u8 bpp[4] = {4, 8, 16, 16};
	const u16 *Packet = m_packetram;
	const u8 *TEXTURE = m_textureram;

	Packet += PacketPtr;

	const u32 Dx = Packet[1] & 0x3ff;
	const u32 Dy = Packet[2] & 0x1ff;
	const u32 Endx = Packet[3] & 0x3ff;
	const u32 Endy = Packet[4] & 0x1ff;
	const u16 Packet0 = Packet[0];

	if (Packet0 & 0x81) //Sync or ASync flip
	{
		m_LastPalUpdate = 0xffffffff;    //Force update palette next frame
		return 1;
	}

	if (Packet0 & 0x200)
	{
		m_RenderState.Tx = Packet[5] | ((Packet[6] & 0x1f) << 16);
		m_RenderState.Ty = Packet[7] | ((Packet[8] & 0x1f) << 16);
	}
	else
	{
		m_RenderState.Tx = 0;
		m_RenderState.Ty = 0;
	}
	if (Packet0 & 0x400)
	{
		m_RenderState.Txdx = Packet[9]  | ((Packet[10] & 0x1f) << 16);
		m_RenderState.Tydx = Packet[11] | ((Packet[12] & 0x1f) << 16);
		m_RenderState.Txdy = Packet[13] | ((Packet[14] & 0x1f) << 16);
		m_RenderState.Tydy = Packet[15] | ((Packet[16] & 0x1f) << 16);
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
		m_RenderState.SrcAlphaColor = Packet[17] | ((Packet[18] & 0xff) << 16);
		m_RenderState.SrcBlend = (Packet[18] >> 8)  & 0x3f;
		m_RenderState.DstAlphaColor = Packet[19] | ((Packet[20] & 0xff) << 16);
		m_RenderState.DstBlend = (Packet[20] >> 8) & 0x3f;
	}
	if (Packet0 & 0x1000)
		m_RenderState.ShadeColor = Packet[21] | ((Packet[22] & 0xff) << 16);
	if (Packet0 & 0x2000)
		m_RenderState.TransColor = Packet[23] | ((Packet[24] & 0xff) << 16);
	if (Packet0 & 0x4000)
	{
		m_RenderState.TileOffset = Packet[25];
		m_RenderState.FontOffset = Packet[26];
		m_RenderState.PalOffset = Packet[27] >> 3;
		m_RenderState.PaletteBank = (Packet[28] >> 8) & 0xf;
		m_RenderState.TextureMode = (Packet[28] >> 12) & 0x1;
		m_RenderState.PixelFormat = (Packet[28] >> 6) & 3;
		m_RenderState.Width  = 8 << ((Packet[28] >> 0) & 0x7);
		m_RenderState.Height = 8 << ((Packet[28] >> 3) & 0x7);
	}

	if (Packet0 & 0x40 && m_RenderState.PalOffset != m_LastPalUpdate)
	{
		const u32 *Pal = (u32*) (TEXTURE + 1024 * m_RenderState.PalOffset);
		const u16 Trans = RGB32TO16(m_RenderState.TransColor);
		for (int i = 0; i < 256; ++i)
		{
			const u32 p = Pal[i];
			u16 v = RGB32TO16(p);
			// TODO: this is most likely an artifact of not emulating the dither modes,
			//       and it's wrong anyway: topbladv gameplay fighters sports a slighty visible square shadow block.
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

		Quad.Blend = 0;
		Quad.Pitch = 1024;

//      assert(Endx >= Dx && Endy >= Dy);

		if (Packet0 & 2)
		{
			Quad.SrcAlpha = m_RenderState.SrcBlend;
			Quad.DstAlpha = m_RenderState.DstBlend;
			Quad.SrcColor = m_RenderState.SrcAlphaColor;
			Quad.DstColor = m_RenderState.DstAlphaColor;
			Quad.Blend |= ALPHA_BIT;
		}
		else
			Quad.SrcAlpha = 0;

		Quad.w = 1 + Endx - Dx;
		Quad.h = 1 + Endy - Dy;

		Quad.Dest = m_DrawDest;
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
			Quad.Blend |= SHADE_BIT;
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
			Quad.Tiled = m_RenderState.TextureMode;
			Quad.bpp = bpp[m_RenderState.PixelFormat];
			Quad.u.Imageb = TEXTURE + 128 * m_RenderState.FontOffset;
			Quad.Tile = (u16*) (TEXTURE + 128 * m_RenderState.TileOffset);
			if (Quad.bpp == 4)
				Quad.Pal = m_InternalPalette + (m_RenderState.PaletteBank * 16);
			else
				Quad.Pal = m_InternalPalette;

			DrawQuadTexture(&Quad);
		}
		else
			DrawQuadFill(&Quad);
	}
	return 0;
}

void vr0video_device::execute_flipping()
{
	if (m_render_start == false)
		return;

	const u32 B0 = 0x000000;
	const u32 B1 = (m_bank1_select == true ? 0x400000 : 0x100000)/2;
	u16 *Front, *Back;
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

	m_DrawDest = ((m_draw_select == true) ? Front : Back);
	m_DisplayDest = Front;

	while ((m_queue_rear & 0x7ff) != (m_queue_front & 0x7ff))
	{
		DoFlip = vrender0_ProcessPacket(m_queue_rear * 32);
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

u32 vr0video_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const u32 width = cliprect.width();

	u32 const dx = cliprect.left();
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		std::copy_n(&m_DisplayDest[(y * 1024) + dx], width, &bitmap.pix16(y, dx));

	return 0;
}
