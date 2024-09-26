// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_VIDEO_BT431_H
#define MAME_VIDEO_BT431_H

#pragma once

class bt431_device : public device_t
{
public:
	bt431_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	enum register_number : unsigned
	{
		REG_COMMAND     =  0,
		REG_CURSOR_X_LO =  1,
		REG_CURSOR_X_HI =  2,
		REG_CURSOR_Y_LO =  3,
		REG_CURSOR_Y_HI =  4,
		REG_WINDOW_X_LO =  5,
		REG_WINDOW_X_HI =  6,
		REG_WINDOW_Y_LO =  7,
		REG_WINDOW_Y_HI =  8,
		REG_WINDOW_W_LO =  9,
		REG_WINDOW_W_HI = 10,
		REG_WINDOW_H_LO = 11,
		REG_WINDOW_H_HI = 12,
	};

	enum command_mask : u8
	{
		CR_D1D0 = 0x03, // cross hair cursor thickness
		CR_D3D2 = 0x0c, // multiplex control
		CR_D4   = 0x10, // cursor format control
		CR_D5   = 0x20, // cross hair cursor enable
		CR_D6   = 0x40, // 64x64 cursor enable

		CR_WM   = 0x7f, // write mask
	};

	enum cr_d1d0_mask : u8
	{
		CR_D1D0_1PIX = 0x00, // 1 pixel
		CR_D1D0_3PIX = 0x01, // 3 pixels
		CR_D1D0_5PIX = 0x02, // 5 pixels
		CR_D1D0_7PIX = 0x03, // 7 pixels
	};
	enum cr_d3d2_mask : u8
	{
		CR_D3D2_11 = 0x00, // 1:1 multiplexing
		CR_D3D2_41 = 0x04, // 4:1 multiplexing
		CR_D3D2_51 = 0x08, // 5:1 multiplexing
	};

	void map(address_map &map) ATTR_COLD;
	bool cur_r(unsigned x, unsigned y) const;

protected:
	static constexpr u16 ADDRESS_MASK = 0x01ff;

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	template <unsigned S> u8 addr_r() { return m_address >> S; }
	template <unsigned S> void addr_w(u8 data) { m_address = ((m_address & (0xff00 >> S)) | (u16(data) << S)) & ADDRESS_MASK; }

	u8 ram_r();
	void ram_w(u8 data);

	u8 reg_r();
	void reg_w(u8 data);

private:
	void update();

	// registers
	u16 m_address;
	u8 m_command;
	u16 m_cursor_x;
	u16 m_cursor_y;
	u16 m_window_x;
	u16 m_window_y;
	u16 m_window_w;
	u16 m_window_h;

	u8 m_ram[512];

	rectangle m_bm_window;
	rectangle m_ch_h;
	rectangle m_ch_v;
};

DECLARE_DEVICE_TYPE(BT431, bt431_device)

#endif // MAME_VIDEO_BT431_H
