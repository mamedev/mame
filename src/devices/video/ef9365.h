// license:BSD-3-Clause
// copyright-holders:Jean-François DEL NERO
/*********************************************************************

    ef9365.h

    Thomson EF9365 video controller

*********************************************************************/

#pragma once

#ifndef __EF9365_H__
#define __EF9365_H__


#define MCFG_EF9365_PALETTE(_palette_tag) \
	ef9365_device::static_set_palette_tag(*device, "^" _palette_tag);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ef9365_device

class ef9365_device :   public device_t,
						public device_memory_interface,
						public device_video_interface
{
public:
	// construction/destruction
	ef9365_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration
	static void static_set_palette_tag(device_t &device, const char *tag);

	// device interface
	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( data_w );

	void update_scanline(UINT16 scanline);
	void static_set_color_filler( UINT8 color );
	void static_set_color_entry( int index, UINT8 r, UINT8 g, UINT8 b );
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_config_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// address space configurations
	const address_space_config      m_space_config;

	// inline helper

private:
	void draw_character( unsigned char c, int block, int smallblock );
	void screen_scanning( int force_clear );
	void set_busy_flag(int period);
	void set_video_mode(void);
	void draw_border(UINT16 line);
	void ef9365_exec(UINT8 cmd);

	// internal state
	static const device_timer_id BUSY_TIMER = 0;

	memory_region *m_charset;
	address_space *m_videoram;

	UINT8 m_current_color;
	UINT8 m_bf;                             //busy flag
	UINT8 m_registers[0x10];                //registers
	UINT8 m_state;                          //status register
	UINT8 m_border[80];                     //border color
	rgb_t palette[16];

	bitmap_rgb32 m_screen_out;

	// timers
	emu_timer *m_busy_timer;

	required_device<palette_device> m_palette;
};

// device type definition
extern const device_type EF9365;

#define EF9365_REG_STATUS 0x00
#define EF9365_REG_CMD    0x00
#define EF9365_REG_CTRL1  0x01
#define EF9365_REG_CTRL2  0x02
#define EF9365_REG_CSIZE  0x03
#define EF9365_REG_DELTAX 0x05
#define EF9365_REG_DELTAY 0x07
#define EF9365_REG_X_MSB  0x08
#define EF9365_REG_X_LSB  0x09
#define EF9365_REG_Y_MSB  0x0A
#define EF9365_REG_Y_LSB  0x0B
#define EF9365_REG_XLP    0x0C
#define EF9365_REG_YLP    0x0D

#endif
