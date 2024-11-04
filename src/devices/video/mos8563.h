// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS Technology 8563 VDC emulation

**********************************************************************/

#ifndef MAME_VIDEO_MOS8563_H
#define MAME_VIDEO_MOS8563_H

#pragma once

#include "video/mc6845.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mos8563_device

class mos8563_device : public mc6845_device,
						public device_memory_interface,
						public device_palette_interface
{
public:
	mos8563_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void address_w(uint8_t data);
	uint8_t status_r();
	uint8_t register_r();
	void register_w(uint8_t data);

	inline uint8_t read_videoram(offs_t offset);
	inline void write_videoram(offs_t offset, uint8_t data);

	MC6845_UPDATE_ROW( vdc_update_row );

protected:
	mos8563_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_palette_interface overrides
	virtual uint32_t palette_entries() const noexcept override { return 16; }

	TIMER_CALLBACK_MEMBER(block_copy_tick);

	const address_space_config      m_videoram_space_config;

	uint8_t m_char_buffer[80];
	uint8_t m_attr_buffer[80];

	bool    m_char_blink_state;
	uint8_t   m_char_blink_count;

	/* register file */
	uint16_t  m_attribute_addr;       /* 0x14/0x15 */
	uint8_t   m_horiz_char;           /* 0x16 */
	uint8_t   m_vert_char_disp;       /* 0x17 */
	uint8_t   m_vert_scroll;          /* 0x18 */
	uint8_t   m_horiz_scroll;         /* 0x19 */
	uint8_t   m_color;                /* 0x1a */
	uint8_t   m_row_addr_incr;        /* 0x1b */
	uint8_t   m_char_base_addr;       /* 0x1c */
	uint8_t   m_underline_ras;        /* 0x1d */
	uint8_t   m_word_count;           /* 0x1e */
	uint8_t   m_data;                 /* 0x1f */
	uint16_t  m_block_addr;           /* 0x20/0x21 */
	uint16_t  m_de_begin;             /* 0x22/0x23 */
	uint8_t   m_dram_refresh;         /* 0x24 */
	uint8_t   m_sync_polarity;        /* 0x25 */

	int m_revision;

	virtual void update_cursor_state() override;
	virtual uint8_t draw_scanline(int y, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

	emu_timer *m_block_copy_timer;

	void mos8563_videoram_map(address_map &map) ATTR_COLD;
};


// ======================> mos8568_device

class mos8568_device : public mos8563_device
{
public:
	mos8568_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};


// device type definitions
DECLARE_DEVICE_TYPE(MOS8563, mos8563_device)
DECLARE_DEVICE_TYPE(MOS8568, mos8568_device)

#endif // MAME_VIDEO_MOS8563_H
