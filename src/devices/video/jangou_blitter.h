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
	jangou_blitter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	DECLARE_WRITE8_MEMBER( process_w );
	DECLARE_WRITE8_MEMBER( alt_process_w );
	DECLARE_WRITE8_MEMBER( vregs_w );
	DECLARE_WRITE8_MEMBER( bltflip_w );
	DECLARE_READ_LINE_MEMBER( status_r );

	uint8_t        m_blit_buffer[256 * 256];

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void plot_gfx_pixel( uint8_t pix, int x, int y );
	uint8_t gfx_nibble( uint32_t niboffset );
	uint8_t m_pen_data[0x10];
	uint8_t m_blit_data[7];
	uint8_t *m_gfxrom;
	uint32_t m_gfxrommask;
	bool m_bltflip;
};


// device type definition
extern const device_type JANGOU_BLITTER;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
