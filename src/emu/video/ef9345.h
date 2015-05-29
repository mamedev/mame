// license:GPL-2.0+
// copyright-holders:Daniel Coulom,Sandro Ronco
/*********************************************************************

    ef9345.h

    Thomson EF9345 video controller

*********************************************************************/


#pragma once

#ifndef __EF9345_H__
#define __EF9345_H__


#define MCFG_EF9345_PALETTE(_palette_tag) \
	ef9345_device::static_set_palette_tag(*device, "^" _palette_tag);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ef9345_device

class ef9345_device :   public device_t,
						public device_memory_interface,
						public device_video_interface
{
public:
	// construction/destruction
	ef9345_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration
	static void static_set_palette_tag(device_t &device, const char *tag);

	// device interface
	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( data_w );
	void update_scanline(UINT16 scanline);
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_config_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

	// address space configurations
	const address_space_config      m_space_config;

	// inline helper
	inline UINT16 indexram(UINT8 r);
	inline UINT16 indexrom(UINT8 r);
	inline void inc_x(UINT8 r);
	inline void inc_y(UINT8 r);

private:

	void set_busy_flag(int period);
	void draw_char_40(UINT8 *c, UINT16 x, UINT16 y);
	void draw_char_80(UINT8 *c, UINT16 x, UINT16 y);
	void set_video_mode(void);
	void init_accented_chars(void);
	UINT8 read_char(UINT8 index, UINT16 addr);
	UINT8 get_dial(UINT8 x, UINT8 attrib);
	void zoom(UINT8 *pix, UINT16 n);
	UINT16 indexblock(UINT16 x, UINT16 y);
	void bichrome40(UINT8 type, UINT16 address, UINT8 dial, UINT16 iblock, UINT16 x, UINT16 y, UINT8 c0, UINT8 c1, UINT8 insert, UINT8 flash, UINT8 hided, UINT8 negative, UINT8 underline);
	void quadrichrome40(UINT8 c, UINT8 b, UINT8 a, UINT16 x, UINT16 y);
	void bichrome80(UINT8 c, UINT8 a, UINT16 x, UINT16 y);
	void makechar(UINT16 x, UINT16 y);
	void draw_border(UINT16 line);
	void makechar_16x40(UINT16 x, UINT16 y);
	void makechar_24x40(UINT16 x, UINT16 y);
	void makechar_12x80(UINT16 x, UINT16 y);
	void ef9345_exec(UINT8 cmd);

	// internal state
	static const device_timer_id BUSY_TIMER = 0;
	static const device_timer_id BLINKING_TIMER = 1;

	memory_region *m_charset;
	address_space *m_videoram;

	UINT8 m_bf;                             //busy flag
	UINT8 m_char_mode;                      //40 or 80 chars for line
	UINT8 m_acc_char[0x2000];               //accented chars
	UINT8 m_registers[8];                   //registers R0-R7
	UINT8 m_state;                          //status register
	UINT8 m_tgs,m_mat,m_pat,m_dor,m_ror;    //indirect registers
	UINT8 m_border[80];                     //border color
	UINT16 m_block;                         //current memory block
	UINT16 m_ram_base[4];                   //index of ram charset
	UINT8 m_blink;                          //cursor status
	UINT8 m_last_dial[40];                  //last chars dial (for determinate the zoom position)
	UINT8 m_latchc0;                        //background color latch
	UINT8 m_latchm;                         //hided atribute latch
	UINT8 m_latchi;                         //insert atribute latch
	UINT8 m_latchu;                         //underline atribute latch

	bitmap_rgb32 m_screen_out;

	// timers
	emu_timer *m_busy_timer;
	emu_timer *m_blink_timer;

	required_device<palette_device> m_palette;
};

// device type definition
extern const device_type EF9345;

#endif
