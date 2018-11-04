// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intel 8275 Programmable CRT Controller emulation

**********************************************************************
                            _____   _____
                   LC3   1 |*    \_/     | 40  Vcc
                   LC2   2 |             | 39  LA0
                   LC1   3 |             | 38  LA1
                   LC0   4 |             | 37  LTEN
                   DRQ   5 |             | 36  RVV
                 _DACK   6 |             | 35  VSP
                  HRTC   7 |             | 34  GPA1
                  VRTC   8 |             | 33  GPA0
                   _RD   9 |             | 32  HLGT
                   _WR  10 |     8275    | 31  IRQ
                  LPEN  11 |             | 30  CCLK
                   DB0  12 |             | 29  CC6
                   DB1  13 |             | 28  CC5
                   DB2  14 |             | 27  CC4
                   DB3  15 |             | 26  CC3
                   DB4  16 |             | 25  CC2
                   DB5  17 |             | 24  CC1
                   DB6  18 |             | 23  CC0
                   DB7  19 |             | 22  _CS
                   GND  20 |_____________| 21  A0

**********************************************************************/

#ifndef MAME_VIDEO_I8275_H
#define MAME_VIDEO_I8275_H

#pragma once




//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define I8275_DRAW_CHARACTER_MEMBER(_name) void _name(bitmap_rgb32 &bitmap, int x, int y, uint8_t linecount, uint8_t charcode, uint8_t lineattr, uint8_t lten, uint8_t rvv, uint8_t vsp, uint8_t gpa, uint8_t hlgt)


#define MCFG_I8275_CHARACTER_WIDTH(_value) \
	i8275_device::static_set_character_width(*device, _value);

#define MCFG_I8275_DRAW_CHARACTER_CALLBACK_OWNER(_class, _method) \
	i8275_device::static_set_display_callback(*device, i8275_device::draw_character_delegate(&_class::_method, #_class "::" #_method, this));

#define MCFG_I8275_DRQ_CALLBACK(_write) \
	devcb = &i8275_device::set_drq_wr_callback(*device, DEVCB_##_write);

#define MCFG_I8275_IRQ_CALLBACK(_write) \
	devcb = &i8275_device::set_irq_wr_callback(*device, DEVCB_##_write);

#define MCFG_I8275_HRTC_CALLBACK(_write) \
	devcb = &i8275_device::set_hrtc_wr_callback(*device, DEVCB_##_write);

#define MCFG_I8275_VRTC_CALLBACK(_write) \
	devcb = &i8275_device::set_vrtc_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> i8275_device

class i8275_device :   public device_t,
						public device_video_interface
{
public:
	typedef device_delegate<void (bitmap_rgb32 &bitmap, int x, int y, uint8_t linecount, uint8_t charcode, uint8_t lineattr, uint8_t lten, uint8_t rvv, uint8_t vsp, uint8_t gpa, uint8_t hlgt)> draw_character_delegate;

	// construction/destruction
	i8275_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void static_set_character_width(device_t &device, int value) { downcast<i8275_device &>(device).m_hpixels_per_column = value; }
	static void static_set_display_callback(device_t &device, draw_character_delegate &&cb) { downcast<i8275_device &>(device).m_display_cb = std::move(cb); }

	template <class Object> static devcb_base &set_drq_wr_callback(device_t &device, Object &&cb) { return downcast<i8275_device &>(device).m_write_drq.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_irq_wr_callback(device_t &device, Object &&cb) { return downcast<i8275_device &>(device).m_write_irq.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_hrtc_wr_callback(device_t &device, Object &&cb) { return downcast<i8275_device &>(device).m_write_hrtc.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_vrtc_wr_callback(device_t &device, Object &&cb) { return downcast<i8275_device &>(device).m_write_vrtc.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE8_MEMBER( dack_w );

	DECLARE_WRITE_LINE_MEMBER( lpen_w );

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void recompute_parameters();

	enum
	{
		TIMER_HRTC_ON,
		TIMER_DRQ_ON,
		TIMER_SCANLINE
	};

	enum
	{
		ST_IE = 0x40,
		ST_IR = 0x20,
		ST_LP = 0x10,
		ST_IC = 0x08,
		ST_VE = 0x04,
		ST_DU = 0x02,
		ST_FO = 0x01
	};

	enum
	{
		CMD_RESET = 0,
		CMD_START_DISPLAY,
		CMD_STOP_DISPLAY,
		CMD_READ_LIGHT_PEN,
		CMD_LOAD_CURSOR,
		CMD_ENABLE_INTERRUPT,
		CMD_DISABLE_INTERRUPT,
		CMD_PRESET_COUNTERS
	};

	enum
	{
		REG_SCN1 = 0,
		REG_SCN2,
		REG_SCN3,
		REG_SCN4,
		REG_CUR_COL,
		REG_CUR_ROW,
		REG_LPEN_COL,
		REG_LPEN_ROW,
		REG_DMA
	};

	enum
	{
		CA_H =    0x01,
		CA_B =    0x02,
		CA_CCCC = 0x3c,
		CA_LTEN = 0x01,
		CA_VSP =  0x02,
		CA_LA0 =  0x04,
		CA_LA1 =  0x08
	};

	enum
	{
		SCC_END_OF_ROW =        0xf0,
		SCC_END_OF_ROW_DMA =    0xf1,
		SCC_END_OF_SCREEN =     0xf2,
		SCC_END_OF_SCREEN_DMA = 0xf3
	};

	enum
	{
		FAC_H =  0x01,
		FAC_B =  0x02,
		FAC_GG = 0x0c,
		FAC_R =  0x10,
		FAC_U =  0x20
	};

	static const int character_attribute[3][16];

	devcb_write_line   m_write_irq;
	devcb_write_line   m_write_drq;
	devcb_write_line   m_write_hrtc;
	devcb_write_line   m_write_vrtc;

	draw_character_delegate m_display_cb;
	int m_hpixels_per_column;

	bitmap_rgb32 m_bitmap;

	uint8_t m_status;
	uint8_t m_param[REG_DMA + 1];
	int m_param_idx;
	int m_param_end;

	uint8_t m_buffer[2][80];
	uint8_t m_fifo[2][16];
	int m_buffer_idx;
	int m_fifo_idx;
	int m_dma_idx;
	bool m_fifo_next;
	int m_buffer_dma;

	int m_lpen;

	int m_hlgt;
	int m_vsp;
	int m_gpa;
	int m_rvv;
	int m_lten;

	int m_scanline;
	int m_irq_scanline;
	int m_vrtc_scanline;
	int m_vrtc_drq_scanline;
	bool m_du;
	bool m_dma_stop;
	bool m_end_of_screen;
	bool m_preset;

	int m_cursor_blink;
	int m_char_blink;
	uint8_t m_stored_attr;

	// timers
	emu_timer *m_hrtc_on_timer;
	emu_timer *m_drq_on_timer;
	emu_timer *m_scanline_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(I8275, i8275_device)

#endif // MAME_VIDEO_I8275_H
