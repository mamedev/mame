// license:BSD-3-Clause
// copyright-holders:ElSemi, Angelo Salese
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
	vr0video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_areas(uint16_t *textureram, uint16_t *frameram);
	void regs_map(address_map &map) ATTR_COLD;
	void execute_flipping();
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	auto idleskip_cb() { return m_idleskip_cb.bind(); }

	uint16_t flip_count_r();
	void flip_count_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	int vrender0_ProcessPacket(uint32_t PacketPtr);

	devcb_write_line  m_idleskip_cb;

	struct RenderStateInfo
	{
		uint32_t Tx;
		uint32_t Ty;
		uint32_t Txdx;
		uint32_t Tydx;
		uint32_t Txdy;
		uint32_t Tydy;
		uint32_t SrcAlphaColor;
		uint32_t SrcBlend;
		uint32_t DstAlphaColor;
		uint32_t DstBlend;
		uint32_t ShadeColor;
		uint32_t TransColor;
		uint32_t TileOffset;
		uint32_t FontOffset;
		uint32_t PalOffset;
		uint32_t PaletteBank;
		uint32_t TextureMode;
		uint32_t PixelFormat;
		uint32_t Width;
		uint32_t Height;
	};


	uint16_t m_InternalPalette[256];
	uint32_t m_LastPalUpdate;

	RenderStateInfo m_RenderState;

	uint8_t *m_textureram;
	uint16_t *m_packetram;
	uint16_t *m_frameram;

	uint16_t cmd_queue_front_r();
	void cmd_queue_front_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t cmd_queue_rear_r();
	uint16_t m_queue_rear, m_queue_front;

	uint16_t bank1_select_r();
	void bank1_select_w(uint16_t data);
	bool m_bank1_select;        //!< Select framebuffer bank1 address

	uint16_t display_bank_r();
	uint8_t m_display_bank;     //!< Current display bank

	uint16_t render_control_r();
	void render_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	bool m_draw_select;         //!< If true, device draws to Front buffer instead of Back
	bool m_render_reset;        //!< Reset pipeline FIFO
	bool m_render_start;        //!< Enable pipeline processing
	uint8_t m_dither_mode;      //!< applied on RGB888 to RGB565 conversions (00: 2x2, 01:4x4, 1x disable)
	uint8_t m_flip_count;       //!< number of framebuffer "syncs" loaded in the parameter RAM,
								//!< a.k.a. how many full (vblank) buffers are ready for the device to parse.

	uint16_t *m_DrawDest;       //!< frameram pointer to draw buffer area
	uint16_t *m_DisplayDest;    //!< frameram pointer to display buffer area
	bool m_flip_sync = false;

	emu_timer *m_pipeline_timer;
	TIMER_CALLBACK_MEMBER(pipeline_cb);
};

DECLARE_DEVICE_TYPE(VIDEO_VRENDER0, vr0video_device)

#endif // MAME_VIDEO_VRENDER0_H
