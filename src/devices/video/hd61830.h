// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Hitachi HD61830 LCD Timing Controller emulation

**********************************************************************/

#ifndef MAME_VIDEO_HD61830_H
#define MAME_VIDEO_HD61830_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> hd61830_device

class hd61830_device :  public device_t,
						public device_memory_interface,
						public device_video_interface
{
public:
	// construction/destruction
	hd61830_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto rd_rd_callback() { return m_read_rd.bind(); }

	uint8_t status_r();
	void control_w(uint8_t data);

	uint8_t data_r();
	void data_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	uint8_t readbyte(offs_t address) { return space().read_byte(address); }
	void writebyte(offs_t address, uint8_t data) { space().write_byte(address, data); }

	TIMER_CALLBACK_MEMBER(clear_busy_flag);

private:
	enum
	{
		INSTRUCTION_MODE_CONTROL = 0,
		INSTRUCTION_CHARACTER_PITCH,
		INSTRUCTION_NUMBER_OF_CHARACTERS,
		INSTRUCTION_NUMBER_OF_TIME_DIVISIONS,
		INSTRUCTION_CURSOR_POSITION,
		INSTRUCTION_DISPLAY_START_LOW = 8,
		INSTRUCTION_DISPLAY_START_HIGH,
		INSTRUCTION_CURSOR_ADDRESS_LOW,
		INSTRUCTION_CURSOR_ADDRESS_HIGH,
		INSTRUCTION_DISPLAY_DATA_WRITE,
		INSTRUCTION_DISPLAY_DATA_READ,
		INSTRUCTION_CLEAR_BIT,
		INSTRUCTION_SET_BIT
	};

	void set_busy_flag();

	uint16_t draw_scanline(bitmap_ind16 &bitmap, const rectangle &cliprect, int y, uint16_t ra);
	void update_graphics(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_char(bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t ma, int x, int y, uint8_t md);
	void update_text(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void hd61830(address_map &map) ATTR_COLD;

	devcb_read8 m_read_rd;

	emu_timer *m_busy_timer;
	//address_space *m_data;

	bool m_bf;                      // busy flag

	uint8_t m_ir;                     // instruction register
	uint8_t m_mcr;                    // mode control register
	uint8_t m_dor;                    // data output register

	uint16_t m_dsa;                   // display start address
	uint16_t m_cac;                   // cursor address counter

	int m_vp;                       // vertical character pitch
	int m_hp;                       // horizontal character pitch
	int m_hn;                       // horizontal number of characters
	int m_nx;                       // number of time divisions
	int m_cp;                       // cursor position

	int m_blink;                    // blink counter
	int m_cursor;                   // cursor visible

	// address space configurations
	const address_space_config      m_space_config;

	required_region_ptr<uint8_t> m_char_rom;
};


// device type definition
DECLARE_DEVICE_TYPE(HD61830, hd61830_device)
static auto &HD61830B = HD61830;

#endif // MAME_VIDEO_HD61830_H
