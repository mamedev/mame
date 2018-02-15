// license:BSD-3-Clause
// copyright-holders:Sean Young, Nathan Woods, Aaron Giles, Wilbert Pol, hap
/*
** File: tms9928a.h -- software implementation of the TMS9928A VDP.
**
** By Sean Young 1999 (sean@msxnet.org).
*/

/*

    Model           Video       Hz

    TMS9918         NTSC        60
    TMS9929?        YPbPr?      50     (not sure. 50Hz non-A model, used in Creativision? or was it a 3rd party clone chip?)

    TMS9918A        NTSC        60
    TMS9928A        YPbPr       60
    TMS9929A        YPbPr       50

    TMS9118         NTSC        60
    TMS9128         YPbPr       60
    TMS9129         YPbPr       50

*/

#ifndef MAME_VIDEO_TMS9928A_H
#define MAME_VIDEO_TMS9928A_H

#pragma once

#include "screen.h"


//  MCFG_DEVICE_ADD(_tag, _variant, XTAL(10'738'635) / 2 )

#define MCFG_TMS9928A_VRAM_SIZE(_size) \
	tms9928a_device::set_vram_size(*device, _size);

#define MCFG_TMS9928A_OUT_INT_LINE_CB(_devcb) \
	devcb = &tms9928a_device::set_out_int_line_callback(*device, DEVCB_##_devcb);

#define MCFG_TMS9928A_SET_SCREEN MCFG_VIDEO_SET_SCREEN

#define MCFG_TMS9928A_OUT_GROMCLK_CB(_devcb) \
	devcb = &tms9928a_device::set_out_gromclk_callback(*device, DEVCB_##_devcb);


#define MCFG_TMS9928A_SCREEN_ADD_NTSC(_screen_tag) \
	MCFG_VIDEO_SET_SCREEN(_screen_tag) \
	MCFG_SCREEN_ADD( _screen_tag, RASTER ) \
	MCFG_SCREEN_RAW_PARAMS( XTAL(10'738'635) / 2, tms9928a_device::TOTAL_HORZ, tms9928a_device::HORZ_DISPLAY_START-12, tms9928a_device::HORZ_DISPLAY_START + 256 + 12, \
			tms9928a_device::TOTAL_VERT_NTSC, tms9928a_device::VERT_DISPLAY_START_NTSC - 12, tms9928a_device::VERT_DISPLAY_START_NTSC + 192 + 12 )


#define MCFG_TMS9928A_SCREEN_ADD_PAL(_screen_tag) \
	MCFG_VIDEO_SET_SCREEN(_screen_tag) \
	MCFG_SCREEN_ADD(_screen_tag, RASTER ) \
	MCFG_SCREEN_RAW_PARAMS( XTAL(10'738'635) / 2, tms9928a_device::TOTAL_HORZ, tms9928a_device::HORZ_DISPLAY_START-12, tms9928a_device::HORZ_DISPLAY_START + 256 + 12, \
			tms9928a_device::TOTAL_VERT_PAL, tms9928a_device::VERT_DISPLAY_START_PAL - 12, tms9928a_device::VERT_DISPLAY_START_PAL + 192 + 12 )


DECLARE_DEVICE_TYPE(TMS9918,  tms9918_device)
DECLARE_DEVICE_TYPE(TMS9918A, tms9918a_device)
DECLARE_DEVICE_TYPE(TMS9118,  tms9118_device)
DECLARE_DEVICE_TYPE(TMS9928A, tms9928a_device)
DECLARE_DEVICE_TYPE(TMS9128,  tms9128_device)
DECLARE_DEVICE_TYPE(TMS9929,  tms9929_device)
DECLARE_DEVICE_TYPE(TMS9929A, tms9929a_device)
DECLARE_DEVICE_TYPE(TMS9129,  tms9129_device)


class tms9928a_device : public device_t,
						public device_memory_interface,
						public device_video_interface
{
public:
	static constexpr unsigned PALETTE_SIZE               = 16;

	/* Some defines used in defining the screens */
	static constexpr unsigned TOTAL_HORZ                 = 342;
	static constexpr unsigned TOTAL_VERT_NTSC            = 262;
	static constexpr unsigned TOTAL_VERT_PAL             = 313;

	static constexpr unsigned HORZ_DISPLAY_START         = 2 + 14 + 8 + 13;
	static constexpr unsigned VERT_DISPLAY_START_PAL     = 13 + 51;
	static constexpr unsigned VERT_DISPLAY_START_NTSC    = 13 + 27;

	// construction/destruction
	tms9928a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void set_vram_size(device_t &device, int vram_size) { downcast<tms9928a_device &>(device).m_vram_size = vram_size; }
	template <class Object> static devcb_base &set_out_int_line_callback(device_t &device, Object &&cb) { return downcast<tms9928a_device &>(device).m_out_int_line_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_out_gromclk_callback(device_t &device, Object &&cb) { return downcast<tms9928a_device &>(device).m_out_gromclk_cb.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_READ8_MEMBER( vram_read );
	DECLARE_WRITE8_MEMBER( vram_write );
	DECLARE_READ8_MEMBER( register_read );
	DECLARE_WRITE8_MEMBER( register_write );

	u8 vram_read();
	void vram_write(u8 data);
	u8 register_read();
	void register_write(u8 data);

	/* update the screen */
	uint32_t screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect );
	bitmap_rgb32 &get_bitmap() { return m_tmpbmp; }

	/* RESET pin */
	void reset_line(int state) { if (state==ASSERT_LINE) device_reset(); }

	void memmap(address_map &map);
protected:
	tms9928a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool is_50hz, bool is_reva, bool is_99);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	void change_register(uint8_t reg, uint8_t val);
	void check_interrupt();
	void update_backdrop();
	void update_table_masks();
	void set_palette();

	static const device_timer_id TIMER_LINE = 0;
	static const device_timer_id GROMCLK = 1;

	int                 m_vram_size;    /* 4K, 8K, or 16K. This should be replaced by fetching data from an address space? */
	devcb_write_line   m_out_int_line_cb; /* Callback is called whenever the state of the INT output changes */
	devcb_write_line    m_out_gromclk_cb; // GROMCLK line is optional; if present, pulse it by XTAL/24 rate

	/* TMS9928A internal settings */
	uint8_t   m_ReadAhead;
	uint8_t   m_Regs[8];
	uint8_t   m_StatusReg;
	uint8_t   m_FifthSprite;
	uint8_t   m_latch;
	uint8_t   m_INT;
	uint16_t  m_Addr;
	uint16_t  m_colour;
	uint16_t  m_pattern;
	uint16_t  m_nametbl;
	uint16_t  m_spriteattribute;
	uint16_t  m_spritepattern;
	int     m_colourmask;
	int     m_patternmask;
	const bool    m_50hz;
	const bool    m_reva;
	const bool    m_99;
	rgb_t   m_palette[16];

	/* memory */
	const address_space_config      m_space_config;
	address_space*                  m_vram_space;

	bitmap_rgb32 m_tmpbmp;
	emu_timer   *m_line_timer;
	emu_timer   *m_gromclk_timer;
	uint8_t       m_mode;

	/* emulation settings */
	int         m_top_border;
	int         m_vertical_size;
};


class tms9918_device : public tms9928a_device
{
public:
	tms9918_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class tms9918a_device : public tms9928a_device
{
public:
	tms9918a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class tms9118_device : public tms9928a_device
{
public:
	tms9118_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class tms9128_device : public tms9928a_device
{
public:
	tms9128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class tms9929_device : public tms9928a_device
{
public:
	tms9929_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class tms9929a_device : public tms9928a_device
{
public:
	tms9929a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class tms9129_device : public tms9928a_device
{
public:
	tms9129_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


#endif // MAME_VIDEO_TMS9928A_H
