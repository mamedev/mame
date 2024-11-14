// license:BSD-3-Clause
// copyright-holders:Curt Coder, AJR
/**********************************************************************

    Intel 8275/8276 CRT Controller emulation

**********************************************************************
             ____   ____                       ____   ____
    LC3   1 |*   \_/    | 40  Vcc     LC3   1 |*   \_/    | 40  Vcc
    LC2   2 |           | 39  LA0     LC2   2 |           | 39  NC
    LC1   3 |           | 38  LA1     LC1   3 |           | 38  NC
    LC0   4 |           | 37  LTEN    LC0   4 |           | 37  LTEN
    DRQ   5 |           | 36  RVV    BRDY   5 |           | 36  RVV
  _DACK   6 |           | 35  VSP     _BS   6 |           | 35  VSP
   HRTC   7 |           | 34  GPA1   HRTC   7 |           | 34  GPA1
   VRTC   8 |           | 33  GPA0   VRTC   8 |           | 33  GPA0
    _RD   9 |           | 32  HLGT    _RD   9 |           | 32  HLGT
    _WR  10 |    8275   | 31  IRQ     _WR  10 |    8276   | 31  INT
   LPEN  11 |           | 30  CCLK     NC  11 |           | 30  CCLK
    DB0  12 |           | 29  CC6     DB0  12 |           | 29  CC6
    DB1  13 |           | 28  CC5     DB1  13 |           | 28  CC5
    DB2  14 |           | 27  CC4     DB2  14 |           | 27  CC4
    DB3  15 |           | 26  CC3     DB3  15 |           | 26  CC3
    DB4  16 |           | 25  CC2     DB4  16 |           | 25  CC2
    DB5  17 |           | 24  CC1     DB5  17 |           | 24  CC1
    DB6  18 |           | 23  CC0     DB6  18 |           | 23  CC0
    DB7  19 |           | 22  _CS     DB7  19 |           | 22  _CS
    GND  20 |___________| 21  A0      GND  20 |___________| 21  C/_P

**********************************************************************/

#ifndef MAME_VIDEO_I8275_H
#define MAME_VIDEO_I8275_H

#pragma once




//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define I8275_DRAW_CHARACTER_MEMBER(_name) void _name(bitmap_rgb32 &bitmap, int x, int y, uint8_t linecount, uint32_t charcode, uint32_t attrcode)


//**************************************************************************
//  CONSTANTS
//**************************************************************************

namespace i8275_attributes
{
	// Each 8275 outputs eight attribute signals in parallel for each character.
	// Except for LA1 and LA0, the bit assignments provided here match the encoding of character attributes.
	enum
	{
		LA1  = 7, // line attribute code 1
		LA0  = 6, // line attribute code 0
		LTEN = 5, // light enable signal
		RVV  = 4, // reverse video signal
		GPA1 = 3, // general purpose attribute code 1
		GPA0 = 2, // general purpose attribute code 0
		VSP  = 1, // video suppression
		HLGT = 0  // highlight
	};
};


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> i8275_device

class i8275_device :   public device_t,
						public device_video_interface
{
public:
	typedef device_delegate<void (bitmap_rgb32 &bitmap, int x, int y, uint8_t linecount, uint32_t charcode, uint32_t attrcode)> draw_character_delegate;

	// construction/destruction
	i8275_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_character_width(int value) { m_hpixels_per_column = value; }
	void set_refresh_hack(bool hack) { m_refresh_hack = hack; }
	template <typename... T> void set_display_callback(T &&... args) { m_display_cb.set(std::forward<T>(args)...); }
	template <typename T> void set_next_crtc(T &&tag) { m_next_crtc.set_tag(std::forward<T>(tag)); }

	auto drq_wr_callback() { return m_write_drq.bind(); }
	auto irq_wr_callback() { return m_write_irq.bind(); }
	auto hrtc_wr_callback() { return m_write_hrtc.bind(); }
	auto vrtc_wr_callback() { return m_write_vrtc.bind(); }
	auto lc_wr_callback() { return m_write_lc.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void dack_w(uint8_t data);

	void lpen_w(int state);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	i8275_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(hrtc_on);
	TIMER_CALLBACK_MEMBER(drq_on);
	TIMER_CALLBACK_MEMBER(scanline_tick);

	void vrtc_start();
	void vrtc_end();
	void dma_start();

	std::pair<uint8_t, uint8_t> char_from_buffer(int n, int sx, int rc, int lc, int &end_of_row, int blank_row);

	void recompute_parameters();

	bool double_spaced_rows() const { return BIT(m_param[REG_SCN1], 7); }
	uint8_t characters_per_row() const { return (m_param[REG_SCN1] & 0x7f) + 1; }
	uint8_t vrtc_row_count() const { return (m_param[REG_SCN2] >> 6) + 1; }
	uint8_t character_rows_per_frame() const { return (m_param[REG_SCN2] & 0x3f) + 1; }
	uint8_t underline() const { return m_param[REG_SCN3] >> 4; }
	uint8_t scanlines_per_row() const { return (m_param[REG_SCN3] & 0x0f) + 1; }
	bool offset_line_counter() const { return BIT(m_param[REG_SCN4], 7); }
	bool visible_field_attribute() const { return BIT(m_param[REG_SCN4], 6); }
	uint8_t cursor_format() const { return (m_param[REG_SCN4] >> 4) & 0x03; }
	uint8_t hrtc_count() const { return ((m_param[REG_SCN4] & 0x0f) + 1) * 2; }
	uint8_t dma_burst_count() const { return 1 << (m_param[REG_DMA] & 0x03); }
	uint8_t dma_burst_space() const { uint8_t sp = (m_param[REG_DMA] >> 2) & 0x07; return sp ? sp * 8 - 1 : 0; }

	enum : uint8_t
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

	enum : uint8_t
	{
		CA_H =    0x01,
		CA_B =    0x02,
		CA_CCCC = 0x3c,
		CA_LTEN = 0x01,
		CA_VSP =  0x02,
		CA_LA0 =  0x04,
		CA_LA1 =  0x08
	};

	enum : uint8_t
	{
		SCC_END_OF_ROW =        0xf0,
		SCC_END_OF_ROW_DMA =    0xf1,
		SCC_END_OF_SCREEN =     0xf2,
		SCC_END_OF_SCREEN_DMA = 0xf3
	};

	enum : uint8_t
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
	devcb_write8       m_write_lc;

	draw_character_delegate m_display_cb;
	int m_hpixels_per_column;
	bool m_refresh_hack;

	optional_device<i8275_device> m_next_crtc;
	bool m_is_crtc0;

	bitmap_rgb32 m_bitmap;

	uint8_t m_status;
	uint8_t m_param[REG_DMA + 1];
	int m_param_idx;
	int m_param_end;

	uint8_t m_buffer[2][80];
	uint8_t m_fifo[2][16];
	int m_buffer_idx;
	uint8_t m_fifo_idx;
	uint8_t m_fifo_idx_out;
	int m_dma_idx;
	uint8_t m_dma_last_char;
	int m_buffer_dma;

	int m_lpen;

	int m_scanline;
	int m_irq_scanline;
	int m_vrtc_scanline;
	int m_vrtc_drq_scanline;
	bool m_dma_stop;
	bool m_end_of_screen;
	bool m_preset;

	int m_cursor_blink;
	int m_char_blink;
	uint8_t m_stored_attr;
	uint8_t m_field_attr;

	// timers
	emu_timer *m_hrtc_on_timer;
	emu_timer *m_drq_on_timer;
	emu_timer *m_scanline_timer;
};

class i8276_device : public i8275_device
{
public:
	// construction/destruction
	i8276_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
DECLARE_DEVICE_TYPE(I8275, i8275_device)
DECLARE_DEVICE_TYPE(I8276, i8276_device)

#endif // MAME_VIDEO_I8275_H
