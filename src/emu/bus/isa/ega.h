// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#pragma once

#ifndef __ISA_EGA_H__
#define __ISA_EGA_H__

#include "emu.h"
#include "isa.h"
#include "video/crtc_ega.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_ega_device

class isa8_ega_device :
		public device_t,
		public device_isa8_card_interface
{
public:
		// construction/destruction
		isa8_ega_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
		isa8_ega_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const;
		virtual const rom_entry *device_rom_region() const;
		virtual ioport_constructor device_input_ports() const;

		DECLARE_READ8_MEMBER(read);
		DECLARE_WRITE8_MEMBER(write);
		DECLARE_READ8_MEMBER(pc_ega8_3b0_r);
		DECLARE_WRITE8_MEMBER(pc_ega8_3b0_w);
		DECLARE_READ8_MEMBER(pc_ega8_3c0_r);
		DECLARE_WRITE8_MEMBER(pc_ega8_3c0_w);
		DECLARE_READ8_MEMBER(pc_ega8_3d0_r);
		DECLARE_WRITE8_MEMBER(pc_ega8_3d0_w);
		DECLARE_WRITE_LINE_MEMBER(de_changed);
		DECLARE_WRITE_LINE_MEMBER(hsync_changed);
		DECLARE_WRITE_LINE_MEMBER(vsync_changed);
		DECLARE_WRITE_LINE_MEMBER(vblank_changed);

		CRTC_EGA_ROW_UPDATE(ega_update_row);
		CRTC_EGA_ROW_UPDATE(pc_ega_graphics);
		CRTC_EGA_ROW_UPDATE(pc_ega_text);

protected:
		// device-level overrides
		virtual void device_start();
		virtual void device_reset();

		UINT8 alu_op( UINT8 data, UINT8 latch_data );

public:
		crtc_ega_device *m_crtc_ega;

		void install_banks();
		void change_mode();
		DECLARE_WRITE8_MEMBER(pc_ega8_3X0_w);
		DECLARE_READ8_MEMBER(pc_ega8_3X0_r);

		/* Video memory and related variables */
		memory_region   *m_vram;
		UINT8   *m_plane[4];
		UINT8   m_read_latch[4];
		UINT8   *m_videoram;
		UINT8   *m_charA;
		UINT8   *m_charB;

		/* Registers */
		UINT8   m_misc_output;
		UINT8   m_feature_control;

		/* Attribute registers AR00 - AR14
		*/
		struct {
			UINT8   index;
			UINT8   data[32];
			UINT8   index_write;
		} m_attribute;

		/* Sequencer registers SR00 - SR04
		*/
		struct {
			UINT8   index;
			UINT8   data[8];
		} m_sequencer;

		/* Graphics controller registers GR00 - GR08
		*/
		struct {
			UINT8   index;
			UINT8   data[16];
		} m_graphics_controller;

		UINT8   m_frame_cnt;
		UINT8   m_hsync;
		UINT8   m_vsync;
		UINT8   m_vblank;
		UINT8   m_display_enable;
		int     m_video_mode;
		required_device<palette_device> m_palette;
};


// device type definition
extern const device_type ISA8_EGA;

#endif  /* __ISA_EGA_H__ */
