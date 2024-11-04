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

    EFO90501        ?           50?    (uses 10.816 MHz XTAL; TI logo sometimes present)

    XTAL inputs                 10.738098 to 10.739172 MHz (10.738635 MHz typical)
    Pixel clock (internal)      XTAL ÷ 2
    CPUCLK (N/A with TMS992x)   XTAL ÷ 3 (3.58 MHz typical)
    GROMCLK                     XTAL ÷ 24 (447.5 kHz typical)

*/

#ifndef MAME_VIDEO_TMS9928A_H
#define MAME_VIDEO_TMS9928A_H

#pragma once

#include "screen.h"


DECLARE_DEVICE_TYPE(TMS9918,  tms9918_device)
DECLARE_DEVICE_TYPE(TMS9918A, tms9918a_device)
DECLARE_DEVICE_TYPE(TMS9118,  tms9118_device)
DECLARE_DEVICE_TYPE(TMS9928A, tms9928a_device)
DECLARE_DEVICE_TYPE(TMS9128,  tms9128_device)
DECLARE_DEVICE_TYPE(TMS9929,  tms9929_device)
DECLARE_DEVICE_TYPE(TMS9929A, tms9929a_device)
DECLARE_DEVICE_TYPE(TMS9129,  tms9129_device)
DECLARE_DEVICE_TYPE(EFO90501, efo90501_device)


class tms9928a_device : public device_t,
						public device_memory_interface,
						public device_palette_interface,
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

	void set_vram_size(int vram_size) { m_vram_size = vram_size; }
	auto int_callback() { return m_out_int_line_cb.bind(); }
	auto gromclk_callback() { return m_out_gromclk_cb.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	u8 vram_read();
	void vram_write(u8 data);
	u8 register_read();
	void register_write(u8 data);

	/* update the screen */
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	bitmap_rgb32 &get_bitmap() { return m_tmpbmp; }

	/* RESET pin */
	void reset_line(int state) { if (state==ASSERT_LINE) device_reset(); }

protected:
	tms9928a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint16_t horz_total, bool is_50hz, bool is_reva, bool is_99);

	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_palette_interface overrides
	virtual uint32_t palette_entries() const noexcept override { return 16; }

	TIMER_CALLBACK_MEMBER(clock_grom);
	TIMER_CALLBACK_MEMBER(update_line);

private:
	void change_register(uint8_t reg, uint8_t val);
	void check_interrupt();
	void update_backdrop();
	void update_table_masks();
	void set_palette();

	void memmap(address_map &map) ATTR_COLD;

	int                 m_vram_size;    /* 4K, 8K, or 16K. This should be replaced by fetching data from an address space? */
	devcb_write_line    m_out_int_line_cb; /* Callback is called whenever the state of the INT output changes */
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
	int       m_colourmask;
	int       m_patternmask;
	const uint16_t m_total_horz;
	const bool     m_50hz;
	const bool     m_reva;
	const bool     m_99;

	/* memory */
	const address_space_config      m_space_config;
	address_space*                  m_vram_space;

	bitmap_rgb32 m_tmpbmp;
	emu_timer   *m_line_timer;
	emu_timer   *m_gromclk_timer;
	uint8_t      m_mode;

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


class efo90501_device : public tms9928a_device
{
public:
	efo90501_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


#endif // MAME_VIDEO_TMS9928A_H
