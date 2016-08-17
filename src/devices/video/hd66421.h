// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/***************************************************************************

  Hitachi HD66421 LCD Controller

  (c) 2001-2007 Tim Schuerewegen

 ***************************************************************************/

#pragma once

#ifndef __HD66421_H__
#define __HD66421_H__


///*************************************************************************
//  MACROS / CONSTANTS
///*************************************************************************

//#define HD66421_BRIGHTNESS_DOES_NOT_WORK

#define HD66421_WIDTH   160
#define HD66421_HEIGHT  100


/*----------- defined in video/hd66421.c -----------*/

///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_HD66421_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, HD66421, 0)

///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> hd66421_device

class hd66421_device :  public device_t,
						public device_memory_interface
{
public:
	// construction/destruction
	hd66421_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( reg_idx_r );
	DECLARE_WRITE8_MEMBER( reg_idx_w );
	DECLARE_READ8_MEMBER( reg_dat_r );
	DECLARE_WRITE8_MEMBER( reg_dat_w );
	DECLARE_PALETTE_INIT(hd66421);

	UINT32 update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// device_config_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// address space configurations
	const address_space_config      m_space_config;

	inline UINT8 readbyte(offs_t address);
	inline void writebyte(offs_t address, UINT8 data);

	void plot_pixel(bitmap_ind16 &bitmap, int x, int y, UINT32 color);

private:
	UINT8 m_cmd, m_reg[32];
	int m_x, m_y;
	required_device<palette_device> m_palette;
};


// device type definition
extern const device_type HD66421;


#endif
