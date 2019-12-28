// license:BSD-3-Clause
// copyright-holders:ElSemi
#ifndef MAME_VIDEO_VRENDER0_H
#define MAME_VIDEO_VRENDER0_H

#pragma once


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

class vr0video_device : public device_t,
						public device_video_interface
{
public:
	vr0video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void set_areas(u16 *textureram, u16 *frameram);
	void regs_map(address_map &map);
	void execute_flipping();
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	auto idleskip_cb() { return m_idleskip_cb.bind(); }

	DECLARE_READ16_MEMBER(flip_count_r);
	DECLARE_WRITE16_MEMBER(flip_count_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	int vrender0_ProcessPacket(u32 PacketPtr);

	devcb_write_line  m_idleskip_cb;

	struct RenderStateInfo
	{
		u32 Tx;
		u32 Ty;
		u32 Txdx;
		u32 Tydx;
		u32 Txdy;
		u32 Tydy;
		u32 SrcAlphaColor;
		u32 SrcBlend;
		u32 DstAlphaColor;
		u32 DstBlend;
		u32 ShadeColor;
		u32 TransColor;
		u32 TileOffset;
		u32 FontOffset;
		u32 PalOffset;
		u32 PaletteBank;
		u32 TextureMode;
		u32 PixelFormat;
		u32 Width;
		u32 Height;
	};

	struct QuadInfo
	{
		u8 Blend;
		u16 *Dest;
		u32 Pitch;   //in UINT16s
		u32 w,h;
		u32 Tx;
		u32 Ty;
		u32 Txdx;
		u32 Tydx;
		u32 Txdy;
		u32 Tydy;
		u16 TWidth;
		u16 THeight;
		union _u
		{
			const u8 *Imageb;
			const u16 *Imagew;
		} u;
		u16 *Tile;
		u16 *Pal;
		u32 TransColor;
		u32 Shade;
		u8 Clamp;
		u8 Trans;
		u8 SrcAlpha;
		u32 SrcColor;
		u8 DstAlpha;
		u32 DstColor;
	};

	u16 m_InternalPalette[256];
	u32 m_LastPalUpdate;

	RenderStateInfo m_RenderState;

	std::unique_ptr<u8[]> m_multiply_lookup[0x100];
	std::unique_ptr<u8[]> m_additive_lookup[0x100];
	void DrawQuad(u8 bpp, bool tiled, QuadInfo *Quad);
	void DrawQuadFill(QuadInfo *Quad);
	inline void DrawPixel(QuadInfo *Quad, u16 *Dst, u16 Src);
	inline u16 Shade(QuadInfo *Quad, u16 Src);
	u16 Alpha(QuadInfo *Quad, u16 Src, u16 Dst);

	u8 *m_textureram;
	u16 *m_packetram;
	u16 *m_frameram;

	DECLARE_READ16_MEMBER( cmd_queue_front_r );
	DECLARE_WRITE16_MEMBER( cmd_queue_front_w );

	DECLARE_READ16_MEMBER( cmd_queue_rear_r );
	u16 m_queue_rear, m_queue_front;

	DECLARE_READ16_MEMBER( bank1_select_r );
	DECLARE_WRITE16_MEMBER( bank1_select_w );
	bool m_bank1_select;        //!< Select framebuffer bank1 address

	DECLARE_READ16_MEMBER( display_bank_r );
	u8 m_display_bank;     //!< Current display bank

	DECLARE_READ16_MEMBER( render_control_r );
	DECLARE_WRITE16_MEMBER( render_control_w );
	bool m_draw_select;         //!< If true, device draws to Front buffer instead of Back
	bool m_render_reset;        //!< Reset pipeline FIFO
	bool m_render_start;        //!< Enable pipeline processing
	u8 m_dither_mode;      //!< applied on RGB888 to RGB565 conversions (00: 2x2, 01:4x4, 1x disable)
	u8 m_flip_count;       //!< number of framebuffer "syncs" loaded in the parameter RAM,
								//!< a.k.a. how many full (vblank) buffers are ready for the device to parse.

	u16 *m_DrawDest;       //!< frameram pointer to draw buffer area
	u16 *m_DisplayDest;    //!< frameram pointer to display buffer area
};

DECLARE_DEVICE_TYPE(VIDEO_VRENDER0, vr0video_device)

#endif // MAME_VIDEO_VRENDER0_H
