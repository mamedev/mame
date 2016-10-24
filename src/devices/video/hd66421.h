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
	hd66421_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t reg_idx_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void reg_idx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t reg_dat_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void reg_dat_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void palette_init_hd66421(palette_device &palette);

	uint32_t update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// device_config_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// address space configurations
	const address_space_config      m_space_config;

	inline uint8_t readbyte(offs_t address);
	inline void writebyte(offs_t address, uint8_t data);

	void plot_pixel(bitmap_ind16 &bitmap, int x, int y, uint32_t color);

private:
	uint8_t m_cmd, m_reg[32];
	int m_x, m_y;
	required_device<palette_device> m_palette;
};


// device type definition
extern const device_type HD66421;


#endif
