// license:GPL-2.0+
// copyright-holders:Daniel Coulom,Sandro Ronco
/*********************************************************************

    ef9345.h

    Thomson EF9345 video controller

*********************************************************************/

#ifndef MAME_VIDEO_EF9345_H
#define MAME_VIDEO_EF9345_H

#pragma once

#include "emupal.h"


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
	ef9345_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <typename T> void set_palette_tag(T &&tag) { m_palette.set_tag(std::forward<T>(tag)); }

	// device interface
	uint8_t data_r(offs_t offset);
	void data_w(offs_t offset, uint8_t data);
	void update_scanline(uint16_t scanline);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:

	enum class EF9345_MODE {
		TYPE_EF9345    = 0x001,
		TYPE_TS9347    = 0x002
	};

	// pass-through constructor
	ef9345_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, EF9345_MODE variant);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_config_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// address space configurations
	const address_space_config      m_space_config;

	// inline helpers
	inline uint16_t indexram(uint8_t r);
	inline uint16_t indexrom(uint8_t r);
	inline void inc_x(uint8_t r);
	inline void inc_y(uint8_t r);

	TIMER_CALLBACK_MEMBER(clear_busy_flag);
	TIMER_CALLBACK_MEMBER(blink_tick);

private:
	void set_busy_flag(int period);
	void draw_char_40(uint8_t *c, uint16_t x, uint16_t y);
	void draw_char_80(uint8_t *c, uint16_t x, uint16_t y);
	void set_video_mode(void);
	void init_accented_chars(void);
	uint8_t read_char(uint8_t index, uint16_t addr);
	uint8_t get_dial(uint8_t x, uint8_t attrib);
	void zoom(uint8_t *pix, uint16_t n);
	uint16_t indexblock(uint16_t x, uint16_t y);
	void bichrome40(uint8_t type, uint16_t address, uint8_t dial, uint16_t iblock, uint16_t x, uint16_t y, uint8_t c0, uint8_t c1, uint8_t insert, uint8_t flash, uint8_t hided, uint8_t negative, uint8_t underline);
	void quadrichrome40(uint8_t c, uint8_t b, uint8_t a, uint16_t x, uint16_t y);
	void bichrome80(uint8_t c, uint8_t a, uint16_t x, uint16_t y, uint8_t cursor);
	void makechar(uint16_t x, uint16_t y);
	void draw_border(uint16_t line);
	void makechar_16x40(uint16_t x, uint16_t y);
	void makechar_24x40(uint16_t x, uint16_t y);
	void makechar_12x80(uint16_t x, uint16_t y);
	void ef9345_exec(uint8_t cmd);

	void ef9345(address_map &map) ATTR_COLD;

	// internal state
	required_region_ptr<uint8_t> m_charset;
	address_space *m_videoram;

	uint8_t m_bf;                             //busy flag
	uint8_t m_char_mode;                      //40 or 80 chars for line
	uint8_t m_acc_char[0x2000];               //accented chars
	uint8_t m_registers[8];                   //registers R0-R7
	uint8_t m_state;                          //status register
	uint8_t m_tgs,m_mat,m_pat,m_dor,m_ror;    //indirect registers
	uint8_t m_border[80];                     //border color
	uint16_t m_block;                         //current memory block
	uint16_t m_ram_base[4];                   //index of ram charset
	uint8_t m_blink;                          //cursor status
	uint8_t m_last_dial[40];                  //last chars dial (for determinate the zoom position)
	uint8_t m_latchc0;                        //background color latch
	uint8_t m_latchm;                         //hided attribute latch
	uint8_t m_latchi;                         //insert attribute latch
	uint8_t m_latchu;                         //underline attribute latch

	bitmap_rgb32 m_screen_out;

	// timers
	emu_timer *m_busy_timer;
	emu_timer *m_blink_timer;

	const EF9345_MODE m_variant;

	required_device<palette_device> m_palette;
};

class ts9347_device : public ef9345_device
{
public:
	ts9347_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// device type definition
DECLARE_DEVICE_TYPE(EF9345, ef9345_device)
DECLARE_DEVICE_TYPE(TS9347, ts9347_device)

#endif // MAME_VIDEO_EF9345_H
