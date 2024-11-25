// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_BFM_BFM_BD1_H
#define MAME_BFM_BFM_BD1_H

#pragma once


class bfm_bd1_device : public device_t
{
public:
	bfm_bd1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint8_t port_val)
		: bfm_bd1_device(mconfig, tag, owner, clock)
	{
		set_port_val(port_val);
	}

	bfm_bd1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration helpers
	void set_port_val(uint8_t val) { m_port_val = val; }

	int write_char(int data);
	virtual void update_display();
	void blank(int data);

	void shift_clock(int state);
	void setdata(int segdata, int data);
	uint16_t set_display(uint16_t segin);
	void sclk(int state);
	void data(int state);
	void por(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	static constexpr uint8_t AT_NORMAL  = 0x00;
	static constexpr uint8_t AT_FLASH   = 0x01;
	static constexpr uint8_t AT_BLANK   = 0x02;
	static constexpr uint8_t AT_FLASHED = 0x80;   // set when character should be blinked off

	std::unique_ptr<output_finder<16> > m_outputs;
	uint8_t m_port_val;

	uint8_t m_cursor_pos = 0;
	uint8_t m_window_start = 0;     // display window start pos 0-15
	uint8_t m_window_end = 0;       // display window end   pos 0-15
	uint8_t m_window_size = 0;      // window  size
	uint8_t m_shift_count = 0;
	uint8_t m_shift_data = 0;
	uint8_t m_pcursor_pos = 0;
	bool m_scroll_active = false;
	uint8_t m_display_mode = 0;
	bool m_flash = false;
	uint8_t m_flash_rate = 0;
	uint8_t m_flash_control = 0;
	uint8_t m_flash_timer = 0;
	uint8_t m_sclk = 0;
	uint8_t m_data = 0;

	uint8_t m_cursor = 0;
	uint16_t m_chars[16]{};
	uint8_t m_attrs[16]{};
	uint16_t m_user_data = 0;         // user defined character data (16 bit)
	uint16_t m_user_def = 0;          // user defined character state
};

DECLARE_DEVICE_TYPE(BFM_BD1, bfm_bd1_device)

#endif // MAME_BFM_BFM_BD1_H
