// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Template for skeleton device

***************************************************************************/

#pragma once

#ifndef __JANGOU_BLITTERDEV_H__
#define __JANGOU_BLITTERDEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_JANGOU_BLITTER_ADD(_tag,_freq) \
	MCFG_DEVICE_ADD(_tag, JANGOU_BLITTER, _freq)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> jangou_blitter_device

class jangou_blitter_device : public device_t
{
public:
	// construction/destruction
	jangou_blitter_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O operations
	DECLARE_WRITE8_MEMBER( process_w );
	DECLARE_WRITE8_MEMBER( alt_process_w );
	DECLARE_WRITE8_MEMBER( vregs_w );
	DECLARE_WRITE8_MEMBER( bltflip_w );
	DECLARE_READ_LINE_MEMBER( status_r );

	UINT8        m_blit_buffer[256 * 256];

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void plot_gfx_pixel( UINT8 pix, int x, int y );
	UINT8 gfx_nibble( UINT32 niboffset );
	UINT8 m_pen_data[0x10];
	UINT8 m_blit_data[7];
	UINT8 *m_gfxrom;
	UINT32 m_gfxrommask;
	bool m_bltflip;
};


// device type definition
extern const device_type JANGOU_BLITTER;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
