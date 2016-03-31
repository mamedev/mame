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

#ifndef __TMS9928A_H__
#define __TMS9928A_H__

#include "emu.h"


#define TMS9928A_PALETTE_SIZE               16


/* Some defines used in defining the screens */
#define TMS9928A_TOTAL_HORZ                 342
#define TMS9928A_TOTAL_VERT_NTSC            262
#define TMS9928A_TOTAL_VERT_PAL             313

#define TMS9928A_HORZ_DISPLAY_START         (2 + 14 + 8 + 13)
#define TMS9928A_VERT_DISPLAY_START_PAL     (13 + 51)
#define TMS9928A_VERT_DISPLAY_START_NTSC    (13 + 27)

//  MCFG_DEVICE_ADD(_tag, _variant, XTAL_10_738635MHz / 2 )

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
	MCFG_SCREEN_RAW_PARAMS( XTAL_10_738635MHz / 2, TMS9928A_TOTAL_HORZ, TMS9928A_HORZ_DISPLAY_START-12, TMS9928A_HORZ_DISPLAY_START + 256 + 12, \
			TMS9928A_TOTAL_VERT_NTSC, TMS9928A_VERT_DISPLAY_START_NTSC - 12, TMS9928A_VERT_DISPLAY_START_NTSC + 192 + 12 )


#define MCFG_TMS9928A_SCREEN_ADD_PAL(_screen_tag) \
	MCFG_VIDEO_SET_SCREEN(_screen_tag) \
	MCFG_SCREEN_ADD(_screen_tag, RASTER ) \
	MCFG_SCREEN_RAW_PARAMS( XTAL_10_738635MHz / 2, TMS9928A_TOTAL_HORZ, TMS9928A_HORZ_DISPLAY_START-12, TMS9928A_HORZ_DISPLAY_START + 256 + 12, \
			TMS9928A_TOTAL_VERT_PAL, TMS9928A_VERT_DISPLAY_START_PAL - 12, TMS9928A_VERT_DISPLAY_START_PAL + 192 + 12 )


extern const device_type TMS9918;
extern const device_type TMS9918A;
extern const device_type TMS9118;
extern const device_type TMS9928A;
extern const device_type TMS9128;
extern const device_type TMS9929;
extern const device_type TMS9929A;
extern const device_type TMS9129;


class tms9928a_device : public device_t,
						public device_memory_interface,
						public device_video_interface
{
public:
	// construction/destruction
	tms9928a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms9928a_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, bool is_50hz, bool is_reva, bool is_99, const char *shortname, const char *source);

	static void set_vram_size(device_t &device, int vram_size) { downcast<tms9928a_device &>(device).m_vram_size = vram_size; }
	template<class _Object> static devcb_base &set_out_int_line_callback(device_t &device, _Object object) { return downcast<tms9928a_device &>(device).m_out_int_line_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_gromclk_callback(device_t &device, _Object object) { return downcast<tms9928a_device &>(device).m_out_gromclk_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( vram_read );
	DECLARE_WRITE8_MEMBER( vram_write );
	DECLARE_READ8_MEMBER( register_read );
	DECLARE_WRITE8_MEMBER( register_write );

	/* update the screen */
	UINT32 screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect );
	bitmap_rgb32 &get_bitmap() { return m_tmpbmp; }

	/* RESET pin */
	void reset_line(int state) { if (state==ASSERT_LINE) device_reset(); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_DATA) const override { return (spacenum == AS_DATA) ? &m_space_config : nullptr; }

private:
	void change_register(UINT8 reg, UINT8 val);
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
	UINT8   m_ReadAhead;
	UINT8   m_Regs[8];
	UINT8   m_StatusReg;
	UINT8   m_FifthSprite;
	UINT8   m_latch;
	UINT8   m_INT;
	UINT16  m_Addr;
	UINT16  m_colour;
	UINT16  m_pattern;
	UINT16  m_nametbl;
	UINT16  m_spriteattribute;
	UINT16  m_spritepattern;
	int     m_colourmask;
	int     m_patternmask;
	bool    m_50hz;
	bool    m_reva;
	bool    m_99;
	rgb_t   m_palette[16];

	/* memory */
	const address_space_config      m_space_config;
	address_space*                  m_vram_space;

	bitmap_rgb32 m_tmpbmp;
	emu_timer   *m_line_timer;
	emu_timer   *m_gromclk_timer;
	UINT8       m_mode;

	/* emulation settings */
	int         m_top_border;
	int         m_vertical_size;
};


class tms9918_device : public tms9928a_device
{
public:
	tms9918_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class tms9918a_device : public tms9928a_device
{
public:
	tms9918a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class tms9118_device : public tms9928a_device
{
public:
	tms9118_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class tms9128_device : public tms9928a_device
{
public:
	tms9128_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class tms9929_device : public tms9928a_device
{
public:
	tms9929_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class tms9929a_device : public tms9928a_device
{
public:
	tms9929a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class tms9129_device : public tms9928a_device
{
public:
	tms9129_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


#endif
