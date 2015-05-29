// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    OKI MSM6255 Dot Matrix LCD Controller implementation

**********************************************************************/

#pragma once

#ifndef __MSM6255__
#define __MSM6255__

#include "emu.h"



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> msm6255_device

class msm6255_device :  public device_t,
						public device_memory_interface,
						public device_video_interface
{
public:
	// construction/destruction
	msm6255_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual DECLARE_ADDRESS_MAP(map, 8);

	DECLARE_READ8_MEMBER( ir_r );
	DECLARE_WRITE8_MEMBER( ir_w );

	DECLARE_READ8_MEMBER( dr_r );
	DECLARE_WRITE8_MEMBER( dr_w );

	UINT32 screen_update(screen_device &device, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

private:
	// registers
	enum
	{
		REGISTER_MOR = 0,
		REGISTER_PR,
		REGISTER_HNR,
		REGISTER_DVR,
		REGISTER_CPR,
		REGISTER_SLR,
		REGISTER_SUR,
		REGISTER_CLR,
		REGISTER_CUR
	};

	UINT8 read_byte(UINT16 ma, UINT8 ra);

	void update_cursor();
	void draw_scanline(bitmap_ind16 &bitmap, const rectangle &cliprect, int y, UINT16 ma, UINT8 ra = 0);
	void update_graphics(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_text(bitmap_ind16 &bitmap, const rectangle &cliprect);

	const address_space_config m_space_config;

	UINT8 m_ir;                     // instruction register
	UINT8 m_mor;                    // mode control register
	UINT8 m_pr;                     // character pitch register
	UINT8 m_hnr;                    // horizontal character number register
	UINT8 m_dvr;                    // duty number register
	UINT8 m_cpr;                    // cursor form register
	UINT8 m_slr;                    // start address (lower) register
	UINT8 m_sur;                    // start address (upper) register
	UINT8 m_clr;                    // cursor address (lower) register
	UINT8 m_cur;                    // cursor address (upper) register

	int m_cursor;                   // is cursor displayed
	int m_frame;                    // frame counter
};


// device type definition
extern const device_type MSM6255;



#endif
