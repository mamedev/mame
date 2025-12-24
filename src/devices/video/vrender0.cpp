// license:BSD-3-Clause
// copyright-holders:ElSemi, Angelo Salese
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

/*****************************************************************************
 DEVICE INTERFACE
 *****************************************************************************/

DEFINE_DEVICE_TYPE(VIDEO_VRENDER0, vr0video_device, "vr0video", "MagicEyes VRender0 Video Engine")

vr0video_device::vr0video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VIDEO_VRENDER0, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_draw_image{
		{
			&vr0video_device::draw_quad<4, false, 0>,
			&vr0video_device::draw_quad<8, false, 0>,
			&vr0video_device::draw_quad<16, false, 0>,
			&vr0video_device::draw_quad<16, false, 0>,
			&vr0video_device::draw_quad<4, false, 1>,
			&vr0video_device::draw_quad<8, false, 1>,
			&vr0video_device::draw_quad<16, false, 1>,
			&vr0video_device::draw_quad<16, false, 1>,
			&vr0video_device::draw_quad<4, false, 2>,
			&vr0video_device::draw_quad<8, false, 2>,
			&vr0video_device::draw_quad<16, false, 2>,
			&vr0video_device::draw_quad<16, false, 2>,
			&vr0video_device::draw_quad<4, false, 3>,
			&vr0video_device::draw_quad<8, false, 3>,
			&vr0video_device::draw_quad<16, false, 3>,
			&vr0video_device::draw_quad<16, false, 3>
		},
		{
			&vr0video_device::draw_quad<4, true, 0>,
			&vr0video_device::draw_quad<8, true, 0>,
			&vr0video_device::draw_quad<16, true, 0>,
			&vr0video_device::draw_quad<16, true, 0>,
			&vr0video_device::draw_quad<4, true, 1>,
			&vr0video_device::draw_quad<8, true, 1>,
			&vr0video_device::draw_quad<16, true, 1>,
			&vr0video_device::draw_quad<16, true, 1>,
			&vr0video_device::draw_quad<4, true, 2>,
			&vr0video_device::draw_quad<8, true, 2>,
			&vr0video_device::draw_quad<16, true, 2>,
			&vr0video_device::draw_quad<16, true, 2>,
			&vr0video_device::draw_quad<4, true, 3>,
			&vr0video_device::draw_quad<8, true, 3>,
			&vr0video_device::draw_quad<16, true, 3>,
			&vr0video_device::draw_quad<16, true, 3>
		}
	}
	, m_texture_config("texture", ENDIANNESS_LITTLE, 16, 23) // 64 MBit (8 MB) Texture Memory Support
	, m_frame_config("frame", ENDIANNESS_LITTLE, 16, 23) // 64 MBit (8 MB) Framebuffer Memory Support
	, m_idleskip_cb(*this)
	, m_internal_palette{0}
	, m_last_pal_update(0xffffffff)
	, m_render_state(render_state_info())
	, m_queue_rear(0)
	, m_queue_front(0)
	, m_bank1_select(false)
	, m_display_bank(0)
	, m_draw_select(false)
	, m_render_reset(false)
	, m_render_start(false)
	, m_dither_mode(0)
	, m_flip_count(0)
	, m_draw_dest(0)
	, m_display_dest(0)
	, m_flip_sync(false)
	, m_pipeline_timer(nullptr)
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

u16 vr0video_device::cmd_queue_front_r()
{
	return m_queue_front & 0x7ff;
}

void vr0video_device::cmd_queue_front_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_queue_front);
}

u16 vr0video_device::cmd_queue_rear_r()
{
	return m_queue_rear & 0x7ff;
}

u16 vr0video_device::render_control_r()
{
	return (m_draw_select ? 0x80 : 0) | (m_render_reset ? 0x8 : 0) | (m_render_start ? 0x4 : 0) | m_dither_mode;
}

void vr0video_device::render_control_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_draw_select = BIT(data, 7);
		m_render_reset = BIT(data, 3);
		m_render_start = BIT(data, 2);
		m_dither_mode = data & 3;

		// initialize pipeline
		// TODO: what happens if reset and start are both 1? Datasheet advises against it.
		if (m_render_reset)
			m_queue_front = m_queue_rear = 0;
	}
}

u16 vr0video_device::display_bank_r()
{
	return m_display_bank;
}

u16 vr0video_device::bank1_select_r()
{
	return m_bank1_select ? 0x8000 : 0;
}

void vr0video_device::bank1_select_w(u16 data)
{
	m_bank1_select = BIT(data, 15);
}

u16 vr0video_device::flip_count_r()
{
	m_idleskip_cb(m_flip_count != 0);
	return m_flip_count;
}

void vr0video_device::flip_count_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		int const fc = data & 0xff;
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
	// Find our direct access
	space(AS_TEXTURE).cache(m_texcache);
	space(AS_FRAME).cache(m_fbcache);
	space(AS_FRAME).specific(m_fbmem);

	save_item(NAME(m_internal_palette));
	save_item(NAME(m_last_pal_update));

	save_item(NAME(m_render_state.tx));
	save_item(NAME(m_render_state.ty));
	save_item(NAME(m_render_state.txdx));
	save_item(NAME(m_render_state.tydx));
	save_item(NAME(m_render_state.txdy));
	save_item(NAME(m_render_state.tydy));
	save_item(NAME(m_render_state.src_alpha_color));
	save_item(NAME(m_render_state.src_blend));
	save_item(NAME(m_render_state.dst_alpha_color));
	save_item(NAME(m_render_state.dst_blend));
	save_item(NAME(m_render_state.shade_color));
	save_item(NAME(m_render_state.trans_color));
	save_item(NAME(m_render_state.tile_offset));
	save_item(NAME(m_render_state.font_offset));
	save_item(NAME(m_render_state.pal_offset));
	save_item(NAME(m_render_state.palette_bank));
	save_item(NAME(m_render_state.texture_mode));
	save_item(NAME(m_render_state.pixel_format));
	save_item(NAME(m_render_state.Width));
	save_item(NAME(m_render_state.Height));

	save_item(NAME(m_flip_count));
	save_item(NAME(m_queue_rear));
	save_item(NAME(m_queue_front));
	save_item(NAME(m_bank1_select));
	save_item(NAME(m_display_bank));
	save_item(NAME(m_draw_select));
	save_item(NAME(m_render_reset));
	save_item(NAME(m_render_start));
	save_item(NAME(m_dither_mode));
	save_item(NAME(m_flip_sync));

	m_pipeline_timer = timer_alloc(FUNC(vr0video_device::pipeline_cb), this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vr0video_device::device_reset()
{
	std::fill(std::begin(m_internal_palette), std::end(m_internal_palette), 0);
	m_last_pal_update = 0xffffffff;

	m_display_dest = m_draw_dest = 0;
	// 1100 objects per second at ~80 MHz
	m_pipeline_timer->adjust(attotime::from_hz(this->clock() / 1100), 0, attotime::from_hz(this->clock() / 1100));
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector vr0video_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_TEXTURE, &m_texture_config),
		std::make_pair(AS_FRAME, &m_frame_config)
	};
}

/*****************************************************************************
 IMPLEMENTATION
 *****************************************************************************/

constexpr u32 RGB32(u8 r, u8 g, u8 b) { return (r << 16) | (g << 8) | (b << 0); }
constexpr u16 RGB16(u8 r, u8 g, u8 b) { return ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | ((b & 0xf8) >> 3); }

constexpr u16 RGB32TO16(u32 rgb)
{
	return (((rgb >> (16 + 3)) & 0x1f) << 11) | (((rgb >> (8 + 2)) & 0x3f) << 5) | (((rgb >> (3)) & 0x1f) << 0);
}

constexpr u8 EXTRACTR8(u16 src) { return pal5bit(src >> 11); }
constexpr u8 EXTRACTG8(u16 src) { return pal6bit(src >>  5); }
constexpr u8 EXTRACTB8(u16 src) { return pal5bit(src >>  0); }

static inline u16 do_shade(u16 src, u32 shade)
{
	u32 const scr = (EXTRACTR8(src) * ((shade >> 16) & 0xff)) >> 8;
	u32 const scg = (EXTRACTG8(src) * ((shade >>  8) & 0xff)) >> 8;
	u32 const scb = (EXTRACTB8(src) * ((shade >>  0) & 0xff)) >> 8;
	return RGB16(scr, scg, scb);
}

u16 vr0video_device::do_alpha(quad_info &quad, u16 src, u16 dst)
{
	u32 const scr = EXTRACTR8(src);
	u32 const scg = EXTRACTG8(src);
	u32 const scb = EXTRACTB8(src);
	u32 dcr = EXTRACTR8(dst);
	u32 dcg = EXTRACTG8(dst);
	u32 dcb = EXTRACTB8(dst);

	u32 smulr, smulg, smulb;
	u32 dmulr, dmulg, dmulb;

	switch (quad.src_alpha & 0x1f)
	{
		case 0x01:
			smulr = smulg = smulb = 0;
			break;
		case 0x02:
			smulr = (quad.src_color >> 16) & 0xff;
			smulg = (quad.src_color >>  8) & 0xff;
			smulb = (quad.src_color >>  0) & 0xff;
			break;
		case 0x04:
			smulr = scr;
			smulg = scg;
			smulb = scb;
			break;
		case 0x08:
			smulr = (quad.dst_color >> 16) & 0xff;
			smulg = (quad.dst_color >>  8) & 0xff;
			smulb = (quad.dst_color >>  0) & 0xff;
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

	if (BIT(quad.src_alpha, 5))
	{
		smulr = 0x100 - smulr;
		smulg = 0x100 - smulg;
		smulb = 0x100 - smulb;
	}

	switch (quad.dst_alpha & 0x1f)
	{
		case 0x01:
			dmulr = dmulg = dmulb = 0;
			break;
		case 0x02:
			dmulr = (quad.src_color >> 16) & 0xff;
			dmulg = (quad.src_color >>  8) & 0xff;
			dmulb = (quad.src_color >>  0) & 0xff;
			break;
		case 0x04:
			dmulr = scr;
			dmulg = scg;
			dmulb = scb;
			break;
		case 0x08:
			dmulr = (quad.dst_color >> 16) & 0xff;
			dmulg = (quad.dst_color >>  8) & 0xff;
			dmulb = (quad.dst_color >>  0) & 0xff;
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

	if (BIT(quad.dst_alpha, 5))
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

template <u8 Bpp, bool Tiled, u8 Blend>
void vr0video_device::draw_quad(quad_info &quad)
{
	u32 const trans_color = quad.trans ? RGB32TO16(quad.trans_color) : NOTRANSCOLOR;
	u32 const maskw = quad.twidth - 1;
	u32 const maskh = quad.theight - 1;
	u32 const w = quad.twidth >> 3;
	for (int y = quad.dy, y_tx = quad.tx, y_ty = quad.ty; y <= quad.endy; y++, y_tx += quad.txdy, y_ty += quad.tydy)
	{
		for (int x = quad.dx, x_tx = y_tx, x_ty = y_ty; x <= quad.endx; x++, x_tx += quad.txdx, x_ty += quad.tydx)
		{
			u32 const fb_addr = quad.dest + get_fb_addr(x, y);
			u32 tx = x_tx >> 9;
			u32 ty = x_ty >> 9;
			if (quad.clamp)
			{
				if ((tx > maskw) || (ty > maskh))
					continue;
			}
			else
			{
				tx &= maskw;
				ty &= maskh;
			}

			u32 offset = 0;
			if (Tiled)
			{
				u32 const index = m_texcache.read_word(quad.tile + (((ty >> 3) * w + (tx >> 3)) << 1));
				if (index == 0)
					continue;
				offset = (index << 6) + ((ty & 7) << 3) + (tx & 7);
			}
			else
				offset = (ty * quad.twidth) + tx;

			u16 color = 0;
			if (Bpp == 4)
			{
				const u8 texel = m_texcache.read_byte(quad.texaddr + (offset >> 1));
				color = quad.pal[(texel >> (BIT(~offset, 0) << 2)) & 0xf];
			}
			else if (Bpp == 8)
			{
				const u8 texel = m_texcache.read_byte(quad.texaddr + offset);
				color = quad.pal[texel];
			}
			else if (Bpp == 16)
			{
				color = m_texcache.read_word(quad.texaddr + (offset << 1));
			}
			if (color != trans_color)
			{
				u16 pixel = m_fbcache.read_word(fb_addr), prev_pixel = pixel;
				if (BIT(Blend, 1))
					color = do_shade(color, quad.shade);
				if (BIT(Blend, 0))
					pixel = do_alpha(quad, color, pixel);
				else
					pixel = color;
				if (prev_pixel != pixel)
					m_fbmem.write_word(fb_addr, pixel);
			}
		}
	}
}

void vr0video_device::draw_quad_fill(quad_info &quad)
{
	u16 const shade_color = RGB32TO16(quad.shade);
	for (u32 y = quad.dy; y <= quad.endy; y++)
	{
		for (u32 x = quad.dx; x <= quad.endx; x++)
		{
			u32 const fb_addr = quad.dest + get_fb_addr(x, y);
			u16 pixel = m_fbcache.read_word(fb_addr), prev_pixel = pixel;
			if (quad.src_alpha)
				pixel = do_alpha(quad, shade_color, pixel);
			else
				pixel = shade_color;
			if (prev_pixel != pixel)
				m_fbmem.write_word(fb_addr, pixel);
		}
	}
}

// Returns true if the operation was a flip (sync or async)
// TODO: async loading actually doesn't stop rendering but just flips the render bank
int vr0video_device::process_packet(u32 packet_ptr)
{
	u32 const dx = get_packet(packet_ptr + 1) & 0x3ff;
	u32 const dy = get_packet(packet_ptr + 2) & 0x1ff;
	u32 const endx = get_packet(packet_ptr + 3) & 0x3ff;
	u32 const endy = get_packet(packet_ptr + 4) & 0x1ff;
	u8 blend_mode = 0;
	u16 const packet_0 = get_packet(packet_ptr + 0);

	if (packet_0 & 0x81) //Sync or ASync flip
	{
		m_last_pal_update = 0xffffffff;    //Force update palette next frame
		return packet_0 & 0x81;
	}

	if (BIT(packet_0, 9))
	{
		m_render_state.tx = get_packet(packet_ptr + 5) | ((get_packet(packet_ptr + 6) & 0x1f) << 16);
		m_render_state.ty = get_packet(packet_ptr + 7) | ((get_packet(packet_ptr + 8) & 0x1f) << 16);
	}
	else
	{
		m_render_state.tx = 0;
		m_render_state.ty = 0;
	}
	if (BIT(packet_0, 10))
	{
		m_render_state.txdx = get_packet(packet_ptr + 9)  | ((get_packet(packet_ptr + 10) & 0x1f) << 16);
		m_render_state.tydx = get_packet(packet_ptr + 11) | ((get_packet(packet_ptr + 12) & 0x1f) << 16);
		m_render_state.txdy = get_packet(packet_ptr + 13) | ((get_packet(packet_ptr + 14) & 0x1f) << 16);
		m_render_state.tydy = get_packet(packet_ptr + 15) | ((get_packet(packet_ptr + 16) & 0x1f) << 16);
	}
	else
	{
		m_render_state.txdx = 1 << 9;
		m_render_state.tydx = 0;
		m_render_state.txdy = 0;
		m_render_state.tydy = 1 << 9;
	}
	if (BIT(packet_0, 11))
	{
		m_render_state.src_alpha_color = get_packet(packet_ptr + 17) | ((get_packet(packet_ptr + 18) & 0xff) << 16);
		m_render_state.src_blend = (get_packet(packet_ptr + 18) >> 8)  & 0x3f;
		m_render_state.dst_alpha_color = get_packet(packet_ptr + 19) | ((get_packet(packet_ptr + 20) & 0xff) << 16);
		m_render_state.dst_blend = (get_packet(packet_ptr + 20) >> 8) & 0x3f;
	}
	if (BIT(packet_0, 12))
		m_render_state.shade_color = get_packet(packet_ptr + 21) | ((get_packet(packet_ptr + 22) & 0xff) << 16);
	if (BIT(packet_0, 13))
		m_render_state.trans_color = get_packet(packet_ptr + 23) | ((get_packet(packet_ptr + 24) & 0xff) << 16);
	if (BIT(packet_0, 14))
	{
		m_render_state.tile_offset = get_packet(packet_ptr + 25);
		m_render_state.font_offset = get_packet(packet_ptr + 26);
		m_render_state.pal_offset = get_packet(packet_ptr + 27) >> 3;
		m_render_state.palette_bank = (get_packet(packet_ptr + 28) >> 8) & 0xf;
		m_render_state.texture_mode = BIT(get_packet(packet_ptr + 28), 12);
		m_render_state.pixel_format = (get_packet(packet_ptr + 28) >> 6) & 3;
		m_render_state.Width  = 8 << ((get_packet(packet_ptr + 28) >> 0) & 0x7);
		m_render_state.Height = 8 << ((get_packet(packet_ptr + 28) >> 3) & 0x7);
	}

	if (BIT(packet_0, 6) && m_render_state.pal_offset != m_last_pal_update)
	{
		u32 const pal = 1024 * m_render_state.pal_offset;
		u16 const trans = RGB32TO16(m_render_state.trans_color);
		for (int i = 0; i < 256; ++i)
		{
			u32 const p = m_texcache.read_dword(pal + (i << 2));
			u16 v = RGB32TO16(p);
			// TODO: this is most likely an artifact of not emulating the dither modes,
			//       and it's wrong anyway: topbladv gameplay fighters sports a slighty visible square shadow block.
			if ((v == trans && p != m_render_state.trans_color) || v == NOTRANSCOLOR)  //Error due to conversion. caused transparent
			{
				if ((v & 0x1f) != 0x1f)
					v++;                                    //Make the color a bit different (blueish) so it's not
				else
					v--;
			}
			m_internal_palette[i] = v;                        //made transparent by mistake
		}
		m_last_pal_update = m_render_state.pal_offset;
	}

	if (BIT(packet_0, 8))
	{
		quad_info quad;

//      assert(endx >= dx && endy >= dy);

		if (BIT(packet_0, 1))
		{
			quad.src_alpha = m_render_state.src_blend;
			quad.dst_alpha = m_render_state.dst_blend;
			quad.src_color = m_render_state.src_alpha_color;
			quad.dst_color = m_render_state.dst_alpha_color;
			blend_mode |= 1;
		}
		else
			quad.src_alpha = 0;

		quad.dx = dx;
		quad.dy = dy;
		quad.endx = endx;
		quad.endy = endy;

		quad.dest = m_draw_dest;

		quad.tx = m_render_state.tx;
		quad.ty = m_render_state.ty;
		quad.txdx = m_render_state.txdx;
		quad.tydx = m_render_state.tydx;
		quad.txdy = m_render_state.txdy;
		quad.tydy = m_render_state.tydy;
		if (BIT(packet_0, 4))
		{
			quad.shade = m_render_state.shade_color;
			blend_mode |= 2;
			/*
			//simulate shade with alphablend (SLOW!!!)
			if (!quad.src_alpha && BIT(packet_0, 3))
			{
			    quad.src_alpha = 0x21; //1
			    quad.dst_alpha = 0x01; //0
			}*/
		}
		else
			quad.shade = RGB32(255,255,255);
		quad.trans_color = m_render_state.trans_color;
		quad.twidth = m_render_state.Width;
		quad.theight = m_render_state.Height;
		quad.trans = BIT(packet_0, 2);
		//quad.trans = false;
		quad.clamp = BIT(packet_0, 5);

		if (BIT(packet_0, 3))  //Texture Enable
		{
			quad.texaddr = 128 * m_render_state.font_offset;
			quad.tile = 128 * m_render_state.tile_offset;
			if (!m_render_state.pixel_format)
				quad.pal = m_internal_palette + (m_render_state.palette_bank * 16);
			else
				quad.pal = m_internal_palette;

			(this->*(m_draw_image[m_render_state.texture_mode ? 1 : 0][m_render_state.pixel_format + 4 * blend_mode]))(quad);
		}
		else
			draw_quad_fill(quad);
	}
	return 0;
}

TIMER_CALLBACK_MEMBER(vr0video_device::pipeline_cb)
{
	if (!m_render_start)
		return;

	// bail out if we encountered a flip sync command
	// pipeline waits until it receives a vblank signal
	if (m_flip_sync)
		return;

	if ((m_queue_rear & 0x7ff) == (m_queue_front & 0x7ff))
		return;

	int const do_flip = process_packet(m_queue_rear * 32);
	m_queue_rear++;
	m_queue_rear &= 0x7ff;
	if (BIT(do_flip, 0))
		m_flip_sync = true;

	if (BIT(do_flip, 7))
	{
		u32 const B0 = 0x000000;
		u32 const B1 = m_bank1_select ? 0x400000 : 0x100000;
		u32 front, back;

		if (BIT(m_display_bank, 0))
		{
			front = B1;
			back  = B0;
		}
		else
		{
			front = B0;
			back  = B1;
		}

		m_draw_dest = m_draw_select ? back : front;
	}
}

void vr0video_device::execute_flipping()
{
	if (!m_render_start)
		return;

	u32 const B0 = 0x000000;
	u32 const B1 = m_bank1_select ? 0x400000 : 0x100000;
	u32 front, back;

	if (BIT(m_display_bank, 0))
	{
		front = B1;
		back  = B0;
	}
	else
	{
		front = B0;
		back  = B1;
	}

	m_draw_dest = m_draw_select ? front : back;
	m_display_dest = front;

	m_flip_sync = false;
	if (m_flip_count)
	{
		m_flip_count--;
		m_display_bank ^= 1;
	}
}

u32 vr0video_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		u16 *const dest = &bitmap.pix(y);
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
			dest[x] = m_fbcache.read_word(m_display_dest + get_fb_addr(x, y));
	}

	return 0;
}
