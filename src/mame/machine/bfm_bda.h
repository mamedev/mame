// license:BSD-3-Clause
// copyright-holders:David Haywood
#pragma once
#ifndef BFM_BDA_H
#define BFM_BDA_H

#define MCFG_BFMBDA_ADD(_tag,_val) \
		MCFG_DEVICE_ADD(_tag, BFM_BDA,60)\
		MCFG_BDA_PORT(_val)
#define MCFG_BDA_PORT(_val) \
	bfm_bda_t::static_set_value(*device, _val);
#define MCFG_BFMBDA_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)

class bfm_bda_t : public device_t
{
public:
	bfm_bda_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration helpers
	static void static_set_value(device_t &device, int val);

	int write_char(int data);
	virtual void update_display();
	uint8_t   m_port_val;
	void blank(int data);

	void shift_data(int data);
	void setdata(int segdata, int data);
	uint16_t set_display(uint16_t segin);

protected:
	static const uint8_t AT_NORMAL  = 0x00;
	static const uint8_t AT_FLASH   = 0x01;
	static const uint8_t AT_BLANK   = 0x02;
	static const uint8_t AT_FLASHED = 0x80;   // set when character should be blinked off

	int m_cursor_pos;
	int m_window_start;     // display window start pos 0-15
	int m_window_end;       // display window end   pos 0-15
	int m_window_size;      // window  size
	int m_shift_count;
	int m_shift_data;
	int m_pcursor_pos;
	int m_blank_flag;
	int m_flash_flag;
	int m_scroll_active;
	int m_display_mode;
	int m_flash_rate;
	int m_flash_control;
	uint8_t m_cursor;
	uint16_t m_chars[16];
	uint16_t m_outputs[16];
	uint8_t m_attrs[16];
	uint16_t m_user_data;             // user defined character data (16 bit)
	uint16_t m_user_def;          // user defined character state

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

};

extern const device_type BFM_BDA;
#endif
