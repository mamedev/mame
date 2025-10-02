// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Brooktree Bt481/Bt482

    256 Color, 15-bit, 16-bit, 24-bit RAMDAC

***************************************************************************/

#ifndef MAME_VIDEO_BT48X_H
#define MAME_VIDEO_BT48X_H

#pragma once


class bt481_device : public device_t, public device_palette_interface
{
public:
	bt481_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// color lookup
	rgb_t pixel_select(uint8_t index) const { return pen_color(index & m_read_mask); } // P0-P7
	rgb_t overlay_select(uint8_t index) const { return pen_color((256 + index) & m_overlay_mask); } // OL0-OL3

	void map(address_map &map);

	// accessable with RS0 and RS1
	uint8_t address_r();
	void address_w(uint8_t data);
	uint8_t palette_r();
	void palette_w(uint8_t data);
	virtual uint8_t mask_r();
	virtual void mask_w(uint8_t data);
	void address_read_w(uint8_t data);

	// accessable when RS2 is additionally connected
	uint8_t overlay_address_r();
	void overlay_address_w(uint8_t data);
	void command_w(uint8_t data);
	virtual uint8_t overlay_r();
	virtual void overlay_w(uint8_t data);
	void overlay_address_read_w(uint8_t data);

protected:
	bt481_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;

	// 256 color palette, 1 reserved entry, 15 overlay colors, 1 reserved entry, 3 cursor colors
	virtual uint32_t palette_entries() const noexcept override { return 256 + 1 + 15 + 1 + 3; }

	// indirect registers
	static constexpr int READ_MASK_REGISTER = 0;
	static constexpr int OVERLAY_MASK_REGISTER = 1;
	static constexpr int COMMAND_REGISTER_B = 2;
	static constexpr int CURSOR_REGISTER = 3;

	// cursor modes
	static constexpr int CURSOR_DISABLED = 0;
	static constexpr int CURSOR_3_COLOR = 1;
	static constexpr int CURSOR_COMPLEMENT = 2;
	static constexpr int CURSOR_XWINDOWS = 3;

	uint8_t m_cra;
	uint8_t m_crb;

	uint8_t m_color[3];
	uint8_t m_addr;
	uint8_t m_addr_rgb;

	uint8_t m_indirect_index;

	uint8_t m_read_mask;
	uint8_t m_overlay_mask;
	uint8_t m_cursor;
};

class bt482_device : public bt481_device
{
public:
	bt482_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t mask_r() override;
	virtual void mask_w(uint8_t data) override;
	virtual uint8_t overlay_r() override;
	virtual void overlay_w(uint8_t data) override;

	// renders the hardware cursor
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_start() override ATTR_COLD;

	// additional indirect registers
	static constexpr int CURSOR_X_LOW_REGISTER = 4;
	static constexpr int CURSOR_X_HIGH_REGISTER = 5;
	static constexpr int CURSOR_Y_LOW_REGISTER = 6;
	static constexpr int CURSOR_Y_HIGH_REGISTER = 7;

private:
	uint8_t m_cram[256];
	uint8_t m_cxlr;
	uint8_t m_cxhr;
	uint8_t m_cylr;
	uint8_t m_cyhr;
	uint16_t m_cx;
	uint16_t m_cy;
};

// device type declaration
DECLARE_DEVICE_TYPE(BT481, bt481_device)
DECLARE_DEVICE_TYPE(BT482, bt482_device)

#endif // MAME_VIDEO_BT48X_H
