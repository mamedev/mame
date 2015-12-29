// license:BSD-3-Clause
// copyright-holders:Jean-François DEL NERO
/*********************************************************************

    ef9365.h

    Thomson EF9365/EF9366 video controller

*********************************************************************/

#pragma once

#ifndef __EF9365_H__
#define __EF9365_H__

#define EF936X_BITPLANE_MAX_SIZE 0x8000
#define EF936X_MAX_BITPLANES  8

#define MCFG_EF936X_PALETTE(_palette_tag) \
	ef9365_device::static_set_palette_tag(*device, "^" _palette_tag);

#define MCFG_EF936X_BITPLANES_CNT(_bitplanes_number) \
	ef9365_device::static_set_nb_bitplanes(*device,_bitplanes_number);

#define MCFG_EF936X_DISPLAYMODE(_display_mode) \
	ef9365_device::static_set_display_mode(*device,_display_mode);

#define MCFG_EF936X_IRQ_HANDLER(_devcb) \
	devcb = &ef9365_device::set_irq_handler(*device, DEVCB_##_devcb);

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
	static void static_set_nb_bitplanes(device_t &device, int nb_bitplanes );
	static void static_set_display_mode(device_t &device, int display_mode );
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<ef9365_device &>(device).m_irq_handler.set_callback(object); }

	// device interface
	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( data_w );

	void update_scanline(UINT16 scanline);
	void set_color_filler( UINT8 color );
	void set_color_entry( int index, UINT8 r, UINT8 g, UINT8 b );

	UINT8 get_last_readback_word(int bitplane_number, int * pixel_offset);

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
	int get_char_pix( unsigned char c, int x, int y );
	void plot(int x_pos,int y_pos);
	int draw_character( unsigned char c, int block, int smallblock );
	int draw_vector(int x1,int y1,int x2,int y2);
	unsigned int get_x_reg();
	unsigned int get_y_reg();
	void set_x_reg(unsigned int x);
	void set_y_reg(unsigned int y);
	void screen_scanning( int force_clear );
	void set_busy_flag(int period);
	void set_video_mode(void);
	void draw_border(UINT16 line);
	void ef9365_exec(UINT8 cmd);
	int  cycles_to_us(int cycles);	
	void dump_bitplanes_word();
	void update_interrupts();

	// internal state
	static const device_timer_id BUSY_TIMER = 0;

	memory_region *m_charset;
	address_space *m_videoram;

	UINT8 m_irq_state;
	UINT8 m_irq_vb;
	UINT8 m_irq_lb;
	UINT8 m_irq_rdy;
	UINT8 m_current_color;
	UINT8 m_bf;                             //busy flag
	UINT8 m_registers[0x10];                //registers
	UINT8 m_state;                          //status register
	UINT8 m_border[80];                     //border color

	rgb_t palette[256];                     // 8 bitplanes max -> 256 colors max
	int   nb_of_bitplanes;
	int   nb_of_colors;
	int   bitplane_xres;
	int   bitplane_yres;
	UINT16 overflow_mask_x;
	UINT16 overflow_mask_y;
	int   vsync_scanline_pos;

	UINT8 m_readback_latch[EF936X_MAX_BITPLANES];   // Last DRAM Readback buffer (Filled after a Direct Memory Access Request command)
	int m_readback_latch_pix_offset;

	UINT32 clock_freq;
	bitmap_rgb32 m_screen_out;

	// timers
	emu_timer *m_busy_timer;

	required_device<palette_device> m_palette;
	devcb_write_line m_irq_handler;
};

// device type definition
extern const device_type EF9365;

#define EF936X_REG_STATUS 0x00
#define EF936X_REG_CMD    0x00
#define EF936X_REG_CTRL1  0x01
#define EF936X_REG_CTRL2  0x02
#define EF936X_REG_CSIZE  0x03
#define EF936X_REG_DELTAX 0x05
#define EF936X_REG_DELTAY 0x07
#define EF936X_REG_X_MSB  0x08
#define EF936X_REG_X_LSB  0x09
#define EF936X_REG_Y_MSB  0x0A
#define EF936X_REG_Y_LSB  0x0B
#define EF936X_REG_XLP    0x0C
#define EF936X_REG_YLP    0x0D

#define EF936X_256x256_DISPLAY_MODE    0x00
#define EF936X_512x512_DISPLAY_MODE    0x01
#define EF936X_512x256_DISPLAY_MODE    0x02
#define EF936X_128x128_DISPLAY_MODE    0x03
#define EF936X_64x64_DISPLAY_MODE    0x04


#endif
