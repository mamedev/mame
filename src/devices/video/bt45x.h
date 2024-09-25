// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
#ifndef MAME_VIDEO_BT45X_H
#define MAME_VIDEO_BT45X_H

#pragma once

class bt45x_device_base : public device_t
{
public:
	enum address_mask : u8
	{
		REG_READ_MASK  = 0x04,
		REG_BLINK_MASK = 0x05,
		REG_COMMAND    = 0x06,
		REG_CONTROL    = 0x07
	};
	enum command_mask : u8
	{
		CR7 =  0x80, // multiplex select
		CR6 =  0x40, // ram enable
		CR54 = 0x30, // blink rate selection
		CR3 =  0x08, // OL1 blink enable
		CR2 =  0x04, // OL0 blink enable
		CR1 =  0x02, // OL1 display enable
		CR0 =  0x01, // OL0 display enable
	};
	enum control_mask : u8
	{
		D3  = 0x08, // low nibble
		D2  = 0x04, // blue channel enable
		D1  = 0x02, // green channel enable
		D0  = 0x01, // red channel enable

		RGB = 0x07  // rgb mode enable
	};
	enum cr54_mask : u8
	{
		CR54_6464 = 0x30, // 64 on 64 off, 50/50
		CR54_3232 = 0x20, // 32 on 32 off, 50/50
		CR54_1616 = 0x10, // 16 on 16 off, 50/50
		CR54_1648 = 0x00  // 16 on 48 off, 25/75
	};

	virtual void map(address_map &map) ATTR_COLD;

protected:
	bt45x_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const u32 palette_colors, const u32 overlay_colors);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	u8 address_r();
	void address_w(u8 data);
	virtual u8 palette_r(address_space &space) = 0;
	virtual void palette_w(u8 data) = 0;
	u8 register_r(address_space &space);
	void register_w(u8 data);
	virtual u8 overlay_r(address_space &space) = 0;
	virtual void overlay_w(u8 data) = 0;

	// helpers
	virtual void increment_address(const bool side_effects = false);
	virtual u8 get_mask() const { return m_palette_colors - 1; }

	// device state
	u8 m_address;
	u8 m_address_rgb;
	u8 m_read_mask;
	u8 m_blink_mask;
	u8 m_command;
	u8 m_control;

	// internal state
	const u32 m_palette_colors;
	const u32 m_overlay_colors;
	u64 m_blink_start;
};

class bt45x_rgb_device_base : public bt45x_device_base, public device_palette_interface
{
protected:
	bt45x_rgb_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const u32 palette_colors, const u32 overlay_colors);

	virtual void device_start() override ATTR_COLD;

	virtual u32 palette_entries() const noexcept override { return m_palette_colors + m_overlay_colors; }

	virtual u8 palette_r(address_space &space) override;
	virtual void palette_w(u8 data) override;
	virtual u8 overlay_r(address_space &space) override;
	virtual void overlay_w(u8 data) override;

	std::unique_ptr<std::array<u8, 3>[]> m_color_ram;
};

class bt45x_mono_device_base : public bt45x_device_base
{
public:
	// helper instead of device_palette_interface
	u8 lookup(u8 pixel, u8 overlay = 0) const
	{
		if (overlay & 3)
			return m_color_ram[m_palette_colors + (overlay & (m_command & (CR1|CR0)))];
		else
			return (m_command & CR6) ? m_color_ram[pixel & m_read_mask] : m_color_ram[m_palette_colors + 0];
	}

protected:
	bt45x_mono_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const u32 palette_colors, const u32 overlay_colors);

	virtual void device_start() override ATTR_COLD;

	virtual u8 palette_r(address_space &space) override;
	virtual void palette_w(u8 data) override;
	virtual u8 overlay_r(address_space &space) override;
	virtual void overlay_w(u8 data) override;

	std::unique_ptr<u8[]> m_color_ram;
};

class bt451_device : public bt45x_rgb_device_base
{
public:
	bt451_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	/*
	 * The Bt451 is different in having an 8 bit data bus, but 4 bit DACs. The
	 * lower 4 bits are masked off when reading or writing colour data.
	 */
	virtual u8 get_mask() const override { return 0xf0; }
};

class bt453_device : public bt45x_rgb_device_base
{
public:
	bt453_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override ATTR_COLD;
};

class bt454_device : public bt45x_rgb_device_base
{
public:
	bt454_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override ATTR_COLD;
};

class bt455_device : public bt45x_mono_device_base
{
public:
	bt455_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override ATTR_COLD;
};

class bt457_device : public bt45x_mono_device_base
{
public:
	bt457_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual u8 palette_r(address_space &space) override;
	virtual void palette_w(u8 data) override;
	virtual u8 overlay_r(address_space &space) override;
	virtual void overlay_w(u8 data) override;

	virtual void increment_address(const bool side_effects = false) override;
};

class bt458_device : public bt45x_rgb_device_base
{
public:
	bt458_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	bt458_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

class bt467_device : public bt458_device
{
public:
	bt467_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(BT451, bt451_device)
DECLARE_DEVICE_TYPE(BT453, bt453_device)
DECLARE_DEVICE_TYPE(BT454, bt454_device)
DECLARE_DEVICE_TYPE(BT455, bt455_device)
DECLARE_DEVICE_TYPE(BT457, bt457_device)
DECLARE_DEVICE_TYPE(BT458, bt458_device)
DECLARE_DEVICE_TYPE(BT467, bt467_device)

#endif // MAME_VIDEO_BT45X_H
