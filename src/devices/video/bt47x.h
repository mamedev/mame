// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
#ifndef MAME_VIDEO_BT47X_H
#define MAME_VIDEO_BT47X_H

#pragma once

class bt47x_device_base
	: public device_t
	, public device_palette_interface
{
public:
	// color lookup
	rgb_t palette_lookup(u8 index) const { return pen_color(index & m_read_mask); }
	rgb_t overlay_lookup(u8 index) const { return pen_color(m_palette_colors + index); }

	virtual void map(address_map &map) ATTR_COLD;
	virtual u8 read(offs_t offset);
	virtual void write(offs_t offset, u8 data);

protected:
	bt47x_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock, unsigned const palette_colors, unsigned const overlay_colors, unsigned const color_bits);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_palette_interface implementation
	virtual u32 palette_entries() const noexcept override { return m_palette_colors + m_overlay_colors; }

	// read/write handlers
	u8 address_r();
	void address_w(u8 data);
	u8 palette_r();
	void palette_w(u8 data);
	u8 mask_r();
	void mask_w(u8 data);
	u8 overlay_r();
	void overlay_w(u8 data);

	// helpers
	virtual unsigned address() const { return m_address; };
	void increment_address(bool const rgb, bool const side_effects = false);
	virtual unsigned color_bits() const { return m_color_bits; }

private:
	// device state
	u8 m_address;
	u8 m_address_rgb;
	u8 m_read_mask;

	// internal state
	unsigned const m_palette_colors;
	unsigned const m_overlay_colors;
	unsigned const m_color_bits;

	std::unique_ptr<std::array<u8, 3>[]> m_color_ram;
};

class bt475_device_base : public bt47x_device_base
{
public:
	virtual void map(address_map &map) override ATTR_COLD;
	virtual u8 read(offs_t offset) override;
	virtual void write(offs_t offset, u8 data) override;

protected:
	bt475_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock, unsigned const palette_colors, unsigned const overlay_colors, unsigned const color_bits);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	u8 command_r();
	void command_w(u8 data);

	enum command_mask : u8
	{
		D0 = 0x01, // sleep enable
		D1 = 0x02, // 8-bit select (Bt477 only)
		D2 = 0x04, // sync on red enable
		D3 = 0x08, // sync on green enable
		D4 = 0x10, // sync on blue enable
		D5 = 0x20, // 7.5 IRE blank pedastal enable
		D6 = 0x40, // reserved (logical one)
		D7 = 0x80, // reserved (logical zero)
	};
	u8 m_command;
};

class bt471_device : public bt47x_device_base
{
public:
	bt471_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
};

class bt475_device : public bt475_device_base
{
public:
	bt475_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
};

class bt476_device : public bt47x_device_base
{
public:
	bt476_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
};

class bt477_device : public bt475_device_base
{
public:
	bt477_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual unsigned color_bits() const override { return (m_command & D1) ? 8 : 6; }
};

class bt478_device : public bt47x_device_base
{
public:
	bt478_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
};

class bt479_device : public bt47x_device_base
{
public:
	bt479_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual void map(address_map &map) override ATTR_COLD;
	virtual u8 read(offs_t offset) override;
	virtual void write(offs_t offset, u8 data) override;

protected:
	virtual void device_start() override ATTR_COLD;

	virtual unsigned address() const override { return BIT(m_command[0], 4, 2) * 0x100 + bt47x_device_base::address(); };
	virtual unsigned color_bits() const override { return BIT(m_command[1], 1) ? 8 : 6; }

	u8 control_r();
	void control_w(u8 data);

private:
	u8 m_window[0x80];
	u8 m_command[2];
	u8 m_flood[2];
};

DECLARE_DEVICE_TYPE(BT471, bt471_device)
//DECLARE_DEVICE_TYPE(BT473, bt473_device)
//DECLARE_DEVICE_TYPE(BT474, bt474_device)
DECLARE_DEVICE_TYPE(BT475, bt475_device)
DECLARE_DEVICE_TYPE(BT476, bt476_device)
DECLARE_DEVICE_TYPE(BT477, bt477_device)
DECLARE_DEVICE_TYPE(BT478, bt478_device)
DECLARE_DEVICE_TYPE(BT479, bt479_device)

#endif // MAME_VIDEO_BT47X_H
